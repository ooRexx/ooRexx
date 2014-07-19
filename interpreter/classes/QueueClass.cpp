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
/* REXX Kernel                                               QueueClass.cpp   */
/*                                                                            */
/* Primitive Queue Class                                                      */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "QueueClass.hpp"
#include "IntegerClass.hpp"
#include "SupplierClass.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *RexxQueue::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxQueue::createInstance()
{
    CLASS_CREATE(Queue, "Queue", RexxClass);
}


/**
 * Create an instance of the Queue class.
 *
 * @param size   The base size of the object.
 *
 * @return An initialized queue object.
 */
void *RexxQueue::operator new(size_t size)
{
    RexxQueue *newQueue = (RexxQueue *)new (INITIAL_LIST_SIZE, size) ListTable;
    // these are handled a little differently, so we have to explicitly set our behavior (for now)
    newQueue->setBehaviour(TheQueueBehaviour);
    newQueue->init();
    return newQueue;
}


/**
 * Pull an item off of the stack, returning .nil if no items
 * are available
 *
 * @return The popped item, or .nil if the queue is empty.
 */
RexxObject *RexxQueue::pullRexx()
{
    return resultOrNil(pop());
}


/**
 * Push an item on to the queue.
 *
 * @param item   The item to add.
 *
 * @return Returns nothing.
 */
RexxObject *RexxQueue::pushRexx(RexxObject *item)
{
    requiredArgument(item, ARG_ONE);
    push(item);
    return OREF_NULL;
}


RexxObject *RexxQueue::queueRexx(RexxObject *item)
/******************************************************************************/
/* Function:  Push an item onto the queue                                     */
/******************************************************************************/
{
  requiredArgument(item, ARG_ONE);             /* make sure we have an argument     */
                                       /* add to the end of the queue       */
  this->queue(item);
  return OREF_NULL;                    /* return nothing                    */
}


RexxObject *RexxQueue::peek()
/******************************************************************************/
/* Function:  Return the first element of the queue without removing it       */
/******************************************************************************/
{
    return firstItem();
}


/**
 * Create an instance of a queue object from Rexx code.
 *
 * @param init_args The pointer to the arguments.
 * @param argCount  The count of arguments.
 *
 * @return A new instance of the queue class.
 */
RexxObject *RexxQueue::newRexx(RexxObject **init_args, size_t argCount)
/******************************************************************************/
/* Function:  Create an instance of a queue                                   */
/******************************************************************************/
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxObject *newObj =  new RexxQueue;
    ProtectedObject p(newObj);

    // handle Rexx class completion
    classThis->completeNewObject(newObj, init_args, argCount);
    return newObj;
}


/**
 * Create an instance of the queue class and fill it with objects.
 *
 * @param args     The argument pointer.
 * @param argCount The count of arguments.
 *
 * @return An instance of the queue class populated with objects.
 */
RexxQueue *RexxQueue::ofRexx(RexxObject **args, size_t argCount)
{
    // create a queue object.
    RexxQueue *newQueue = (RexxQueue *)RexxQueue::newRexx(NULL, 0);
    ProtectedObject p(newQueue);

    for (size_t i = 0; i < argCount; i++)
    {
        // add each item, but ommitted arguments are an error.
        RexxObject *item = args[i];
        if (item == OREF_NULL)
        {
            reportException(Error_Incorrect_method_noarg, i + 1);
        }
        newQueue->addLast(item);
    }
    return newQueue;
}




