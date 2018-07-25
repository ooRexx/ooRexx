/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
    _threadHandle = CreateThread(NULL, THREAD_STACK_SIZE, call_thread_function, this, 0, &_threadID);
    // we created this one
    attached = false;
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
 * Set the thread priority.
 *
 * @param priority The thread priority value.
 */
void SysThread::setPriority(ThreadPriority priority)
{
    int pri = THREAD_PRIORITY_NORMAL;
                                         /* critical priority?                */
    switch (priority)
    {
        case HIGH_PRIORITY:       // critical priority
            pri= THREAD_PRIORITY_ABOVE_NORMAL+1; /* the class is regular, but move    */
                                                 /* to the head of the class          */
                                                 /* medium priority                   */
            break;

        case GUARDED_PRIORITY:
            pri = THREAD_PRIORITY_NORMAL+1;    /* guard priority is just above normal*/
            break;

        case MEDIUM_PRIORITY:
            pri = THREAD_PRIORITY_NORMAL;      /* normal class,                     */
                                               /* dead in the middle of it all      */
            break;

        case LOW_PRIORITY:
            pri = THREAD_PRIORITY_IDLE +1;     /* give us idle only, but make it    */
            break;
                                               /* important idle time only          */
    }
    SetThreadPriority(_threadHandle, pri);
}


/**
 * Return a pointer to the current stack base.
 *
 * @return A pointer to the current stack position
 */
char *SysThread::getStackBase()
{
   int32_t temp;
   return ((char *)(&temp)) - THREAD_STACK_SIZE;
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
    // this is a nop on Windows;
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
