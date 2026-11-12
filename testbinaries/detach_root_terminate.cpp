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
/*     (Terminate() has to wait for that thread, which sleeps for 0.2 s and  */
/*     then calls an external function of this program)                      */
/*  3  it detaches, attaches again, runs some Rexx code and detaches again   */
/*  4  it detaches and another thread calls Terminate()                      */
/*  5  another thread attaches, runs and detaches, then the creating thread  */
/*     detaches                                                              */
/*  6  as 4, but an uninit method run by that Terminate() has to wait for    */
/*     the guard of an object whose guarded method is running on a thread    */
/*     of another instance (bug #1650: the uninit kept the kernel lock while */
/*     waiting, and the guard holder could never get it back)                */
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


// case 2: set by the REPLY thread as its last action
static std::atomic<bool> replyDone(false);

/**
 * External function DRT_REPLY_DONE, called by the REPLY thread of case 2
 * when it has finished its work.
 */
static size_t REXXENTRY replyDoneFunction(CONSTANT_STRING, size_t, PCONSTRXSTRING, CONSTANT_STRING, PRXSTRING result)
{
    replyDone = true;
    result->strlength = 0;
    return 0;
}


// case 6: an object of another instance, with a guarded method that sleeps
// and a guarded method that an uninit method of the terminated instance calls
static const char *holderCode =
    "::class holder public\n"
    "::method init\n"
    "expose marked\n"
    "marked = 0\n"
    "::method hold\n"                     // guarded: keeps the guard while it sleeps
    "call SysSleep 0.5\n"                 // native code, run without the kernel lock
    "::method mark\n"                     // guarded: waits while hold runs
    "expose marked\n"
    "marked = 1\n"
    "::attribute marked get\n";

static const char *readerCode =
    "::class reader public\n"
    "::method init\n"
    "expose holder\n"
    "use arg holder\n"
    "::method uninit\n"
    "expose holder\n"
    "holder~mark\n";


/**
 * Case 6. The holder runs on its own thread in its own instance.
 *
 * @return true if the uninit method ran and Terminate() returned.
 */
static bool uninitWaitsForGuard(RexxInstance *instance, RexxThreadContext *context)
{
    std::atomic<RexxObjectPtr> holder(NULLOBJECT);
    std::atomic<bool> go(false);
    std::atomic<bool> terminated(false);
    std::atomic<int> marked(-1);
    bool ok = true;

    std::thread holderThread([&]
    {
        RexxInstance *other = NULL;
        RexxThreadContext *otherContext = NULL;
        if (RexxCreateInterpreter(&other, &otherContext, NULL) != 1)
        {
            fprintf(stderr, "could not create the holder's instance\n");
            holder = (RexxObjectPtr)-1;
            return;
        }
        RexxPackageObject package = otherContext->LoadPackageFromData("holder", holderCode, strlen(holderCode));
        RexxObjectPtr h = otherContext->SendMessage0(otherContext->FindPackageClass(package, "HOLDER"), "NEW");
        otherContext->RequestGlobalReference(h);
        holder = h;
        while (!go)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        otherContext->SendMessage0(h, "HOLD");
        while (!terminated)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        wholenumber_t value = -1;
        otherContext->ObjectToWholeNumber(otherContext->SendMessage0(h, "MARKED"), &value);
        marked = (int)value;
        otherContext->ReleaseGlobalReference(h);
        other->Terminate();
    });

    while (holder == NULLOBJECT)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (holder == (RexxObjectPtr)-1)
    {
        holderThread.join();
        context->DetachThread();
        instance->Terminate();
        return false;
    }

    // an object nobody references, whose uninit method calls holder~mark
    RexxPackageObject package = context->LoadPackageFromData("reader", readerCode, strlen(readerCode));
    RexxObjectPtr reader = context->SendMessage1(context->FindPackageClass(package, "READER"), "NEW", holder);
    context->ReleaseLocalReference(reader);
    context->DetachThread();

    // start hold, give it time to get the guard, and terminate on a thread
    // that was never attached to the instance
    go = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread terminator([instance] { instance->Terminate(); });
    terminator.join();
    terminated = true;
    holderThread.join();

    if (marked != 1)
    {
        fprintf(stderr, "the uninit method did not run (marked = %d)\n", (int)marked);
        ok = false;
    }
    return ok;
}


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
            // Terminate() has to wait for it. That thread calls DRT_REPLY_DONE
            // as its last action, so if Terminate() waited, replyDone is set
            // when it returns. (Timing Terminate() is not enough: on a slow
            // machine most of the sleep can be over before it is called.)
            replyDone = false;
            ok = run(context, ".sleeper~new~sleep; return\n"
                              "::class sleeper\n"
                              "::method sleep\n"
                              "reply\n"
                              "call SysSleep 0.2\n"
                              "call DRT_REPLY_DONE\n");
            context->DetachThread();
            instance->Terminate();
            if (!replyDone)
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

        case 6:
            // already terminated
            return uninitWaitsForGuard(instance, context);
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
        fprintf(stderr, "usage: %s <case 0-6> [cycles] [watchdog-seconds]\n", argv[0]);
        return 2;
    }

    int which = atoi(argv[1]);
    int cycles = argc > 2 ? atoi(argv[2]) : 5;
    int watchdogSeconds = argc > 3 ? atoi(argv[3]) : 30;

    if (which < 0 || which > 6 || cycles < 1 || watchdogSeconds < 1)
    {
        fprintf(stderr, "case must be 0 to 6; cycles and watchdog-seconds must be positive\n");
        return 2;
    }

    if (which == 2 && RexxRegisterFunctionExe("DRT_REPLY_DONE", (REXXPFN)replyDoneFunction) != RXFUNC_OK)
    {
        fprintf(stderr, "could not register DRT_REPLY_DONE\n");
        return 1;
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
