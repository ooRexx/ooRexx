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
/* Terminate() after the creating thread called DetachThread() (bug #2107).  */
/*                                                                           */
/* The thread that calls RexxCreateInterpreter() may detach from the new     */
/* instance like any attached thread, and a later Terminate() must still     */
/* return. This creates and terminates instances in each of these cases:     */
/*                                                                           */
/*  0  no detach (the usual case, as a control)                              */
/*  1  the creating thread detaches                                          */
/*  2  it detaches while a Rexx thread started with REPLY is still running   */
/*     (Terminate() has to wait for that thread, which sleeps for 0.2 s)     */
/*  3  it detaches, attaches again, runs some Rexx code and detaches again   */
/*  4  it detaches and another thread calls Terminate()                      */
/*  5  another thread attaches, runs and detaches, then the creating thread  */
/*     detaches                                                              */
/*                                                                           */
/* A Terminate() that never returns cannot be told from a slow one, so a     */
/* watchdog gives up after a fixed wall time and reports a hang. It has to   */
/* run as its own process for the same reason: a hang must not stop the      */
/* process that runs the test suite.                                         */
/*                                                                           */
/* Usage: detach_root_terminate <case> [cycles] [watchdog-seconds]           */
/*                                                                           */
/* Prints "case <n> ok" and exits with 0 when every cycle terminated. Exit   */
/* codes: 1 an instance failed to start or a call failed, 2 bad usage, 99    */
/* the watchdog fired, which means a hang.                                   */
/*                                                                           */
/*****************************************************************************/

#include "oorexxapi.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

static std::atomic<bool> finished(false);


/**
 * Run a few lines of Rexx code on a thread context.
 *
 * @return true if the code ran without raising a condition.
 */
static bool run(RexxThreadContext *context, const char *code)
{
    RexxRoutineObject routine = context->NewRoutine("detach_root_terminate", code, strlen(code));
    if (routine == NULLOBJECT)
    {
        return false;
    }
    context->CallRoutine(routine, NULLOBJECT);
    if (context->CheckCondition())
    {
        context->DisplayCondition();
        return false;
    }
    return true;
}


/**
 * One create/Terminate cycle of the given case.
 *
 * @return true if everything worked; false if a call failed.
 */
static bool cycle(int which)
{
    RexxInstance *instance = NULL;
    RexxThreadContext *context = NULL;

    // NOTE: RexxCreateInterpreter returns 1 for success and 0 for failure,
    // which is inverted from the other RexxReturnCode entry points.
    if (RexxCreateInterpreter(&instance, &context, NULL) != 1 || instance == NULL || context == NULL)
    {
        fprintf(stderr, "could not create an interpreter instance\n");
        return false;
    }

    bool ok = true;
    switch (which)
    {
        case 0:
            break;

        case 1:
            context->DetachThread();
            break;

        case 2:
        {
            // REPLY leaves a Rexx thread running after the method returns, and
            // Terminate() has to wait for it
            ok = run(context, ".sleeper~new~sleep; return\n"
                              "::class sleeper\n"
                              "::method sleep\n"
                              "reply\n"
                              "call SysSleep 0.2\n");
            context->DetachThread();
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            instance->Terminate();
            std::chrono::steady_clock::duration waited = std::chrono::steady_clock::now() - start;
            if (waited < std::chrono::milliseconds(100))
            {
                fprintf(stderr, "Terminate() did not wait for the REPLY thread\n");
                ok = false;
            }
            return ok;
        }

        case 3:
        {
            context->DetachThread();
            RexxThreadContext *again = NULL;
            if (!instance->AttachThread(&again))
            {
                fprintf(stderr, "AttachThread failed\n");
                ok = false;
                break;
            }
            ok = run(again, "x = 1 + 1");
            again->DetachThread();
            break;
        }

        case 4:
        {
            context->DetachThread();
            std::thread other([instance] { instance->Terminate(); });
            other.join();
            // already terminated
            return true;
        }

        case 5:
        {
            std::thread other([instance, &ok]
            {
                RexxThreadContext *attached = NULL;
                if (!instance->AttachThread(&attached))
                {
                    fprintf(stderr, "AttachThread failed\n");
                    ok = false;
                    return;
                }
                if (!run(attached, "y = 2"))
                {
                    ok = false;
                }
                attached->DetachThread();
            });
            other.join();
            context->DetachThread();
            break;
        }
    }

    instance->Terminate();
    return ok;
}


/**
 * Give up after a fixed wall time and report a hang.
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
        fprintf(stderr, "usage: %s <case 0-5> [cycles] [watchdog-seconds]\n", argv[0]);
        return 2;
    }

    int which = atoi(argv[1]);
    int cycles = argc > 2 ? atoi(argv[2]) : 5;
    int watchdogSeconds = argc > 3 ? atoi(argv[3]) : 30;

    if (which < 0 || which > 5 || cycles < 1 || watchdogSeconds < 1)
    {
        fprintf(stderr, "case must be 0 to 5; cycles and watchdog-seconds must be positive\n");
        return 2;
    }

    std::thread(watchdog, watchdogSeconds).detach();

    for (int i = 0; i < cycles; i++)
    {
        if (!cycle(which))
        {
            finished = true;
            return 1;
        }
    }

    finished = true;
    printf("case %d ok\n", which);
    return 0;
}
