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
/* REXX Kernel                                            SysActivity.hpp     */
/*                                                                            */
/* System support for Thread operations                                       */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "ActivityManager.hpp"
#include <errno.h>

#define THREAD_PRIORITY 100

#include "RexxCore.h"
#include "SysActivity.hpp"


/**
 * Top launcher function for spinning off a new activity.
 *
 * @param args   The thread arguments (just the pointer to the activity).
 */
void *threadFnc(void *args)
{
    ((RexxActivity *)args)->runThread();
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
void SysActivity::create(RexxActivity *activity, size_t stackSize)
{
    int             rc;
    pthread_attr_t  newThreadAttr;
    int schedpolicy;
    struct sched_param schedparam;

                               // Create an attr block for Thread.
    rc = pthread_attr_init(&newThreadAttr);
                               // Set the stack size.
 #if defined(LINUX) || defined(OPSYS_SUN) || defined(AIX)

 /* scheduling on two threads controlled by the result method of the message object */
 /* do not work proper without an enhanced priority                                 */

    pthread_getschedparam(pthread_self(), &schedpolicy, &schedparam);
    schedparam.sched_priority = 100;

 #if defined(OPSYS_SUN)
 /* PTHREAD_EXPLICIT_SCHED ==> use scheduling attributes of the new object    */

    rc = pthread_attr_setinheritsched(&newThreadAttr, PTHREAD_EXPLICIT_SCHED);

 /* Performance measurements show massive performance improvements > 50 %     */
 /* using Round Robin scheduling instead of FIFO scheduling                   */
    rc = pthread_attr_setschedpolicy(&newThreadAttr, SCHED_RR);
 #endif
    rc = pthread_attr_setschedparam(&newThreadAttr, &schedparam);

 #endif
    rc = pthread_attr_setstacksize(&newThreadAttr, stackSize);
                                               // Now create the thread
    rc = pthread_create(&threadId, &newThreadAttr, threadFnc, (void *)activity);
                               // Bumop thread count by one. Threadid
    if (rc != 0)
    {
        reportException(Error_System_service_service, "ERROR CREATING THREAD");
    }
    rc = pthread_attr_destroy(&newThreadAttr);
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
char *SysActivity::getStackBase(size_t stackSize)
{
    size_t temp;
    return (char *)&temp - stackSize;
}
