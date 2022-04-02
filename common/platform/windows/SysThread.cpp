/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX Kernel                                              SysThread.cpp     */
/*                                                                            */
/* Windows implementation of the SysThread class.                             */
/*                                                                            */
/******************************************************************************/

#include "windows.h"
#include "SysThread.hpp"

/**
 * Stub function for handling the intial startup on a new thread.
 *
 * @param arguments The void argument object, which is just a pointer to the
 *                  SysThread object.
 *
 * @return Always returns 0.
 */
DWORD WINAPI call_thread_function(void * arguments)
{
    ((SysThread *)arguments)->dispatch();
   return 0;
}

/**
 * create a new thread
 */
void SysThread::createThread()
{

    createThread(_threadHandle, _threadID, THREAD_STACK_SIZE, call_thread_function, (void *)this);
    // we created this one
    attached = false;
}


/**
 * Create a new thread.
 *
 * @param threadHandle
 *                  The handle of the created thread
 * @param threadId  The id of the created thread
 * @param stackSize The required stack size
 * @param startRoutine
 *                  The thread startup routine.
 * @param startArgument
 *                  The typeless argument passed to the startup routine.
 *
 * @return The success/failure return code.
 */
int SysThread::createThread(HANDLE &threadHandle, DWORD &threadId, size_t stackSize, LPTHREAD_START_ROUTINE startRoutine, void *startArgument)
{
    threadHandle = CreateThread(NULL, stackSize, startRoutine, startArgument, 0, &threadId);
    return threadHandle == INVALID_HANDLE_VALUE;
}

/**
 * This is a dummy method intended for subclass overrides. This
 * dispatches the code on the new thread.
 */
void SysThread::dispatch()
{
    // default dispatch returns immediately
}

/**
 * attach an activity to an existing thread
 */
void SysThread::attachThread()
{
    // initialize the thread basics
    _threadID = GetCurrentThreadId();
    _threadHandle = GetCurrentThread();
    attached = true;           // we don't own this one (and don't terminate it)
}


/**
 * Do any platform specific termination
 */
void SysThread::terminate()
{
    if (!attached && _threadHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(_threadHandle);     // close the thread handle
        _threadHandle = INVALID_HANDLE_VALUE;
    }
}


/**
 * Do any thread-specific startup activity.
 */
void SysThread::startup()
{
    // this is a nop on Windows;
}


/**
 * Perform any platform-specific thread shutdown activities.
 */
void SysThread::shutdown()
{
    // this is a nop on Windows;
}


/**
 * yield control to other threads.
 */
void SysThread::yield()
{
    // just give up the time slice
    Sleep(1);
}


/**
 * Test whether two threads are the same.
 *
 * @param other  The other thread object.
 *
 * @return True if they are the same thread, false if not.
 */
bool SysThread::equals(SysThread &other)
{
    return _threadID == other._threadID;
}


/**
 * Wait for the thread to terminate
 */
void SysThread::waitForTermination()
{
    if (!attached && _threadID != 0)
    {
        WaitForSingleObject(_threadHandle, INFINITE);
        _threadID = 0;
    }
}


/**
 * Platform wrapper around a simple sleep function.
 *
 * @param msecs  The number of milliseconds to sleep.
 */
void SysThread::sleep(int msecs)
{
    Sleep(msecs);
}


#define OM_WAKEUP (WM_USER+10)

/**
 * callback routine for SetTimer set in longSleep
 *
 * @param hwnd    The window handle
 * @param uMsg    the message parameter
 * @param idEvent the event id
 * @param dwTime  the time
 */
VOID CALLBACK SleepTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    DWORD ThreadId;
    KillTimer(NULL, idEvent);       /* kill the timer that just ended */
    ThreadId = GetCurrentThreadId();
    PostThreadMessage(ThreadId, OM_WAKEUP, 0, 0L); /* send ourself the wakeup message*/
}


/**
 * Platform wrapper around a sleep function that can sleep for long periods of time. .
 *
 * @param microseconds
 *               The number of microseconds to delay.
 */
int SysThread::longSleep(uint64_t microseconds)
{

    // convert to milliseconds, no overflow possible
    long milliseconds = (long)(microseconds / 1000);

    /** Using Sleep with a long timeout risks sleeping on a thread with a message
     *  queue, which can make the system sluggish, or possibly deadlocked.  If the
     *  sleep is longer than 333 milliseconds use a window timer to avoid this
     *  risk.
     */
    if (milliseconds > 333)
    {
        if (!(SetTimer(NULL, 0, milliseconds, (TIMERPROC)SleepTimerProc)))
        {
            return GetLastError();
        }

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (msg.message == OM_WAKEUP)  /* If our message, exit loop       */
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    else
    {
        Sleep(milliseconds);
    }
    return 0;
}

