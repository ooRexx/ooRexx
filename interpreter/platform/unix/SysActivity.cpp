/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2023 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                            SysActivity.hpp     */
/*                                                                            */
/* System support for Thread operations                                       */
/*                                                                            */
/******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "RexxCore.h"
#include "ActivityManager.hpp"
#include <errno.h>

#include "RexxCore.h"
#include "SysActivity.hpp"
#include "SysThread.hpp"


/**
 * Top launcher function for spinning off a new activity.
 *
 * @param args   The thread arguments (just the pointer to the activity).
 */
void *threadFnc(void *args)
{
    ((Activity *)args)->runThread();
    return NULL;
}

/**
 * Close out any resources required by this thread descriptor.
 */
void SysActivity::close()
{
    threadId = 0;
}

/**
 * Create a new thread for an activity.
 *
 * @param activity  The activity that will run on this thread.
 * @param stackSize The desired stack size.
 */
void SysActivity::create(Activity *activity, size_t stackSize)
{
    // try to create the thread and raise an exception for any failure
    int rc = SysThread::createThread(threadId, valid, stackSize, threadFnc, (void *)activity);
    if (rc != 0)
    {
        reportException(Error_System_service_service, "ERROR CREATING THREAD");
    }
}


/**
 * Get the ID of the current thread.
 *
 * @return The thread identifer for the current thread.
 */
thread_id_t SysActivity::queryThreadID()
{
    return (thread_id_t)pthread_self();      /* just call the correct function */
}


/**
 * Check if this activity is getting used on the correct
 * thread.
 *
 * @return true if the current thread is the same as the one
 *         the activity was created for.
 */
bool SysActivity::validateThread()
{
    return threadId == pthread_self();
}


/**
 * Initialize the descriptor for manipulating the current
 * active thread.
 */
void SysActivity::useCurrentThread()
{
    // we need both an identifier and a handle
    threadId = pthread_self();
}


/**
 * Return the pointer to the base of the current stack.
 * This is used for checking recursion overflows.
 *
 * @return The character pointer for the stack base.
 */
char* SysActivity::getStackBase()
{
#ifdef HAVE_PTHREAD_GETATTR_NP
    pthread_attr_t attrs;
    pthread_getattr_np(pthread_self(), &attrs);
    void   *stackAddr;
    size_t  stackSize;
    pthread_attr_getstack(&attrs, &stackAddr, &stackSize);
    pthread_attr_destroy(&attrs);
    return (char *)stackAddr;
#else
    pthread_t thread = pthread_self();

#ifdef __APPLE__
    // on the Mac pthread_get_stacksize_np() returns an incorrect smaller value, so if we're the main
    // thread just assume the default
    size_t size = pthread_main_np() ? (8 * 1024 * 1024) : pthread_get_stacksize_np(thread);
#else
    size_t size = pthread_get_stacksize_np(pthread_self());
#endif
    // stack address points to the start of the stack (the high address), not the base, how it's returned by pthread_get_stackaddr_np
    return (char *)pthread_get_stackaddr_np(thread) - size;
#endif
}


/**
 * Return the size of the stack used by the current thread.
 *
 * @return The size of the current stack
 */
size_t SysActivity::getStackSize()
{
#ifdef HAVE_PTHREAD_GETATTR_NP
    pthread_attr_t attrs;
    pthread_getattr_np(pthread_self(), &attrs);
    void   *stackAddr;
    size_t  stackSize;
    pthread_attr_getstack(&attrs, &stackAddr, &stackSize);
    pthread_attr_destroy(&attrs);
    return stackSize;
#else

#ifdef __APPLE__
    // pthread_get_stacksize_np() returns an incorrect value on the Mac, so we fall back on
    // the default stack size if we're the main thread.
    return pthread_main_np() ? (8 * 1024 * 1024) : pthread_get_stacksize_np(pthread_self());
#else
    return pthread_get_stacksize_np(pthread_self());
#endif
#endif
}
