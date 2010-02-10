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
#include "SysActivity.hpp"

/**
 * Close out any resources required by this thread descriptor.
 */
void SysActivity::close()
{
    CloseHandle(this->hThread);
    hThread = NULL;
    threadId = 0;
}


DWORD WINAPI call_thread_function(void * arguments)
{
    // hand this off to the thread object
    ((RexxActivity *)arguments)->runThread();
    return 0;
}


/**
 * Create a real thread for the activity holding this
 * item.
 *
 * @param activity  The activity we're creating on.
 * @param stackSize The required stack size.
 */
void SysActivity::create(RexxActivity *activity, size_t stackSize)
{
    DWORD res;

    hThread = CreateThread(NULL, stackSize, call_thread_function, (void *)activity, 0, &res);
    if (hThread == NULL)
    {
        reportException(Error_System_service_service, "ERROR CREATING THREAD");
    }
    threadId = res;
}


/**
 * Get the ID of the current thread.
 *
 * @return The thread identifer for the current thread.
 */
thread_id_t SysActivity::queryThreadID()
{
    return(thread_id_t)GetCurrentThreadId();
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
    return threadId == queryThreadID();
}


/**
 * Initialize the descriptor for manipulating the current
 * active thread.
 */
void SysActivity::useCurrentThread()
{
    // we need both an identifier and a handle
    threadId = queryThreadID();
    hThread = GetCurrentThread();
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
    return(char *)&temp - stackSize;
}
