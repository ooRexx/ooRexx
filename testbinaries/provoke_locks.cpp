/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026 Rexx Language Association. All rights reserved.         */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*****************************************************************************/
/*                                                                           */
/* Concurrent interpreter instance creation and termination.                 */
/*                                                                           */
/* Every thread calls RexxCreateInterpreter at the same moment, runs a       */
/* program that spawns reply threads, and terminates its instance. Repeated  */
/* for several rounds so that instance creation on one thread overlaps reply */
/* threads winding down on another.                                          */
/*                                                                           */
/* This covers two things the Rexx test suite cannot reach, because the rexx */
/* command line tool only ever creates one instance on one thread:           */
/*                                                                           */
/*  1. One-time interpreter bootstrap. Interpreter::startInterpreter() guards*/
/*     it with "ResourceSection lock; if (!isActive())". If that lock does   */
/*     not actually lock, every thread passes the test and every thread      */
/*     restores the image into the same global memoryObject. Before the      */
/*     resource lock was made immune to static initialization order this     */
/*     failed here 15 times out of 15, with all worker threads inside        */
/*     MemoryObject::restoreImage simultaneously.  See bug #2078.            */
/*                                                                           */
/*  2. The resourceLock/kernel lock order. The bootstrap requests the kernel */
/*     lock while holding the resource lock, which Interpreter.hpp forbids.  */
/*     A second thread holding the kernel lock and wanting the resource lock */
/*     (InterpreterInstance::poolActivity, on a reply thread) would then     */
/*     deadlock. That is the lock pair in the stack traces on bug #1734.     */
/*     It does not deadlock today only because the bootstrap body runs once  */
/*     per process, so this is a guard against that ceasing to be true.      */
/*     See bug #2079.                                                        */
/*                                                                           */
/* Usage: provoke_locks <program.rex> [threads] [rounds] [watchdog-seconds]  */
/*                                                                           */
/* Exit codes: 0 completed, 1 an instance failed to start, 2 bad usage,      */
/* 99 the watchdog fired, which means a hang. A crash shows up as whatever   */
/* the platform reports for the signal.                                      */
/*                                                                           */
/*****************************************************************************/

#include "oorexxapi.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <vector>

/**
 * A reusable thread barrier. std::barrier is C++20, which is newer than this
 * code base assumes, so this is the classic two-phase version.
 */
class Barrier
{
public:
    explicit Barrier(size_t count) : threshold(count), waiting(count), generation(0) { }

    void wait()
    {
        std::unique_lock<std::mutex> guard(barrierMutex);
        size_t thisGeneration = generation;
        if (--waiting == 0)
        {
            generation++;
            waiting = threshold;
            barrierCondition.notify_all();
        }
        else
        {
            barrierCondition.wait(guard, [this, thisGeneration] { return thisGeneration != generation; });
        }
    }

private:
    std::mutex barrierMutex;
    std::condition_variable barrierCondition;
    size_t threshold;
    size_t waiting;
    size_t generation;
};


static const char *programName = NULL;
static int rounds = 0;
static Barrier *startLine = NULL;
static std::atomic<bool> finished(false);
static std::atomic<int> failures(0);


static void worker(int id)
{
    // line every thread up so that the first RexxCreateInterpreter calls, the
    // ones that race the one-time bootstrap, land together
    startLine->wait();

    for (int round = 0; round < rounds; round++)
    {
        RexxInstance *instance = NULL;
        RexxThreadContext *context = NULL;

        // NOTE: RexxCreateInterpreter returns 1 for success and 0 for failure,
        // which is inverted from the other RexxReturnCode entry points.
        if (RexxCreateInterpreter(&instance, &context, NULL) != 1 || instance == NULL || context == NULL)
        {
            fprintf(stderr, "thread %d round %d: could not create an interpreter instance\n", id, round);
            failures++;
            return;
        }

        RexxArrayObject args = context->NewArray(0);
        context->CallProgram(programName, args);
        if (context->CheckCondition())
        {
            context->DisplayCondition();
        }

        instance->Terminate();
    }
}


/**
 * If the interpreter deadlocks nothing ever returns, so a hang would look the
 * same as a test that simply takes a long time. Give up after a fixed wall
 * time and report it as a hang.
 */
static void watchdog(int seconds)
{
    for (int tenths = 0; tenths < seconds * 10; tenths++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (finished)
        {
            return;
        }
    }
    fprintf(stderr, "WATCHDOG: no progress after %d seconds, treating this as a hang\n", seconds);
    fflush(stderr);
    std::_Exit(99);
}


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <program.rex> [threads] [rounds] [watchdog-seconds]\n", argv[0]);
        return 2;
    }

    programName = argv[1];
    int threadCount = argc > 2 ? atoi(argv[2]) : 8;
    rounds = argc > 3 ? atoi(argv[3]) : 10;
    int watchdogSeconds = argc > 4 ? atoi(argv[4]) : 60;

    if (threadCount < 1 || rounds < 1 || watchdogSeconds < 1)
    {
        fprintf(stderr, "threads, rounds and watchdog-seconds must all be positive\n");
        return 2;
    }

    std::thread(watchdog, watchdogSeconds).detach();

    Barrier barrier(threadCount);
    startLine = &barrier;

    std::vector<std::thread> threads;
    for (int i = 0; i < threadCount; i++)
    {
        threads.push_back(std::thread(worker, i));
    }
    for (size_t i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }

    finished = true;
    return failures == 0 ? 0 : 1;
}
