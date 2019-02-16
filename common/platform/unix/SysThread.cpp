/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* Unix implementation of the SysThread class.                                */
/*                                                                            */
/******************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "SysThread.hpp"
#include <stdio.h>
#include <unistd.h>

// attach an activity to an existing thread
void SysThread::attachThread()
{
    // initialize the thread basics
    _threadID = pthread_self();
    attached = true;      // we didn't create this one
}

void SysThread::dispatch()
{
    // default dispatch returns immediately
}

void SysThread::terminate()
{
    if (!attached && _threadID != 0)
    {
        pthread_detach(_threadID);
        _threadID = 0;
    }
}


void SysThread::startup()
{
    // this is a nop on Unix.
}


void SysThread::shutdown()
{
    // this is a nop on Unix.
}


void SysThread::yield()
{
// sched_yield doesn't really seem to do a yield unless this is a
// real time scheduling priority, so force this thread to sleep for a
// sort interval to give other threads a chance to run
    usleep(1);
}


bool SysThread::equals(SysThread &other)
{
    return pthread_equal(_threadID, other._threadID);
}


static void * call_thread_function(void *argument)
{
    ((SysThread *)argument)->dispatch();
    return NULL;
}


// create a new thread and attach to an activity
void SysThread::createThread()
{
    // we own this thread vs. running on an exising thread
    attached = false;

    // create the thread now
    int rc = createThread(_threadID, THREAD_STACK_SIZE, call_thread_function, (void *)this);
    if (rc != 0)
    {
        _threadID = 0;
        fprintf(stderr," *** ERROR: At SysThread(), createThread - RC = %d !\n", rc);
    }
    return;
}


/**
 * Create a new thread.
 *
 * @param threadNumber
 *                  The returned thread ID
 * @param stackSize The required stack size
 * @param startRoutine
 *                  The thread startup routine.
 * @param startArgument
 *                  The typeless argument passed to the startup routine.
 *
 * @return The success/failure return code.
 */
int SysThread::createThread(pthread_t &threadNumber, size_t stackSize, void *(*startRoutine)(void *), void *startArgument)
{
    pthread_attr_t  newThreadAttr;

    // Create an attr block for Thread.
    pthread_attr_init(&newThreadAttr);

    // Set the stack size.
    pthread_attr_setstacksize(&newThreadAttr, stackSize);

    // Now create the thread
    int rc = pthread_create(&threadNumber, &newThreadAttr, startRoutine, (void *)startArgument);
    pthread_attr_destroy(&newThreadAttr);
    return rc;
}


// wait for the thread to terminatre
void SysThread::waitForTermination()
{
    if (!attached && _threadID != 0)
    {
        void *res;
        pthread_join(_threadID, &res);
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
    // usleep uses micro seconds, so we need to multiply.
    usleep(msecs*1000);
}


/**
 * Platform wrapper around a sleep function that can sleep for long periods of time. .
 *
 * @param microseconds
 *               The number of microseconds to delay.
 */
void SysThread::longSleep(uint64_t microseconds)
{
    // split into two part: secs and nanoseconds
    long secs = (long)microseconds / 1000000;
    long nanoseconds = (long)(microseconds % 1000000) * 1000;

#if defined( HAVE_NANOSLEEP )
    struct timespec    Rqtp, Rmtp;
    Rqtp.tv_sec = secs;
    Rqtp.tv_nsec = nanoseconds;
    nanosleep(&Rqtp, &Rmtp);
#elif defined( HAVE_NSLEEP )
    struct timestruc_t Rqtp, Rmtp;
    Rqtp.tv_sec = secs;
    Rqtp.tv_nsec = nanoseconds;
    nsleep(&Rqtp, &Rmtp);
#else
    usleep(microseconds);
#endif
}


