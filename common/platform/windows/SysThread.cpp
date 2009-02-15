/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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

DWORD WINAPI call_thread_function(void * arguments)
{
    ((SysThread *)arguments)->dispatch();
   return 0;
}

// create a new thread and attach to an activity
void SysThread::createThread()
{
    _threadHandle = CreateThread(NULL, THREAD_STACK_SIZE, call_thread_function, this, 0, &_threadID);
}

void SysThread::dispatch()
{
    // default dispatch returns immediately
}

// attach an activity to an existing thread
void SysThread::attachThread()
{
    // initialize the thread basics
    _threadID = GetCurrentThreadId();
    _threadHandle = GetCurrentThread();
}

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


char *SysThread::getStackBase()
/******************************************************************************/
/* Function:  Return a pointer to the current stack base                      */
/******************************************************************************/
{
   int32_t temp;
   return ((char *)(&temp)) - THREAD_STACK_SIZE;
}


void SysThread::terminate()
/******************************************************************************/
/* Function:  Do any platform specific thread termination                     */
/******************************************************************************/
{
    CloseHandle(_threadHandle);     // close the thread handle
}


void SysThread::startup()
/******************************************************************************/
/* Function:  Do any platform specific thread initialization                  */
/******************************************************************************/
{
    // this is a nop on Windows;
}


void SysThread::shutdown()
/******************************************************************************/
/* Function:  Do any platform specific thread shutdown activities.  This      */
/* replaces the WindowEnv stuff in prior releases.                            */
/******************************************************************************/
{
    // this is a nop on Windows;
}


void SysThread::yield()
/******************************************************************************/
/* Function:  Yield control to other threads.                                 */
/******************************************************************************/
{
    // this is a nop on Windows;
}


bool SysThread::equals(SysThread &other)
/******************************************************************************/
/* Function:  Yield dispatching control to other threads.                     */
/******************************************************************************/
{
    return _threadID == other._threadID;
}
