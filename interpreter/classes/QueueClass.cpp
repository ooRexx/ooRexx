/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
RexxClass *QueueClass::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void QueueClass::createInstance()
{
    CLASS_CREATE(Queue);
}


/**
 * Create an instance of the Queue class.
 *
 * @param size   The base size of the object.
 *
 * @return An initialized queue object.
 */
void *QueueClass::operator new(size_t size, size_t capacity, size_t maxSize)
{
    // we're really allocating an array item
    return (void *)ArrayClass::allocateNewObject(size, capacity, maxSize, T_Queue);
}


/**
 * Process and validate a queue index.  This overrides the base
 * array class validation to disable the multi-dimensional
 * support.  If the index is out of bounds in any dimension it
 * will either return false or raise an error, depending on the
 * bounds checking parameter.
 *
 * @param index      The array of objects representing the index.
 * @param indexCount The count of index items.
 * @param argPosition  The argument position (used for error
 *               reporting)
 * @param boundsError Flags indicating how bounds errors should
 *                   be handled.
 * @param position   The returned flattened index pointing to the actual
 *                   item in the array.
 *
 * @return true if this was a valid index (within bounds) based
 *         on the bounds checking flags.
 */
bool QueueClass::validateIndex(RexxObject **index, size_t indexCount,
    size_t argPosition, size_t boundsError, size_t &position)
{
    return validateSingleDimensionIndex(index, indexCount, argPosition, boundsError, position);
}


/**
 * Validate the insertion index for a queue collection.  For
 * queues, the position must be location of an existing item
 * within the bounds of the queue, unlike an array which
 * can insert at empty slots or beyond the existing bounds.
 *
 * @param position The insertion index position.
 */
void QueueClass::checkInsertIndex(size_t position)
{
    if (position > lastItem)
    {
        reportException(Error_Incorrect_method_queue_index, position);
    }
}


/**
 * Pull an item off of the stack, returning .nil if no items
 * are available
 *
 * @return The popped item, or .nil if the queue is empty.
 */
RexxObject *QueueClass::pullRexx()
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
RexxObject *QueueClass::pushRexx(RexxObject *item)
{
    requiredArgument(item, ARG_ONE);
    push(item);
    return OREF_NULL;
}


/**
 * Queue an item on this queue.
 *
 * @param item   The item to add to the queue.
 *
 * @return Returns nothing.
 */
RexxObject *QueueClass::queueRexx(RexxObject *item)
{
    requiredArgument(item, ARG_ONE);

    queue(item);
    return OREF_NULL;
}


/**
 * Return the first item of the queue without removing it.
 *
 * @return The first object from the queue.
 */
RexxInternalObject *QueueClass::peek()
{
    return getFirstItem();
}


/**
 * The Rexx stub for the Queue PUT method. replaces the Array
 * version because it has slightly different semantics for index
 * validation.  The only valid indexes are those within range
 * and the size is not adjusted for a put out of bounds.
 *
 * @param arguments The array of all arguments sent to the
 *                  method (variable arguments allowed
 *                  here...the first argument is the item being
 *                  added, all other arguments are the index).
 * @param argCount  The number of arguments in the method call.
 *
 * @return Always return nothing.
 */
RexxObject *QueueClass::putRexx(RexxObject *value, RexxObject *index)
{
    requiredArgument(value, ARG_ONE);
    // make sure we have an index specified before trying to decode this.
    requiredArgument(index, ARG_TWO);

    // Validate the index argument, but don't allow expansion.
    size_t position;

    if (!validateIndex(&index, 1, ARG_TWO, IndexAccess, position))
    {
        reportException(Error_Incorrect_method_index, index);
    }

    // we can only update assigned items, so make sure this is within bounds.
    checkInsertIndex(position);

    // set the new value and return nothing
    put(value, position);
    return OREF_NULL;
}


/**
 * Remove an object from the array.  This redefines remove() to
 * be a delete operation.
 *
 * @param index  The target index position.
 *
 * @return The removed object, if any.  Returns OREF_NULL if there
 *         is no item at that index.
 */
RexxInternalObject *QueueClass::remove(size_t index)
{
    return deleteItem(index);
}


/**
 * The init method for this class.  This does delayed
 * initialization of this object until a INIT message is
 * received during initialization.
 *
 * @param initialSize
 *               The initial list size (optional)
 *
 * @return Always returns nothing
 */
RexxObject *QueueClass::initRexx(RexxObject *initialSize)
{
    // It would be nice to do this expansion in the new method, but it sort
    // of messes up subclasses (e.g. CircularQueue) if we steal the first new
    // argument.  We will set the capacity here, even if it means an immediate expansion

    // the capacity is optional, but must be a positive numeric value
    size_t capacity = optionalLengthArgument(initialSize, DefaultArraySize, ARG_ONE);
    ensureSpace(capacity);
    return OREF_NULL;
}


/**
 * Create an instance of a queue object from Rexx code.
 *
 * @param init_args The pointer to the arguments.
 * @param argCount  The count of arguments.
 *
 * @return A new instance of the queue class.
 */
RexxObject *QueueClass::newRexx(RexxObject **init_args, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    Protected<QueueClass> newObj = new QueueClass;

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
QueueClass *QueueClass::ofRexx(RexxObject **args, size_t argCount)
{
    // create a queue object.
    Protected<QueueClass> newQueue = (QueueClass *)QueueClass::newRexx(NULL, 0);

    for (size_t i = 0; i < argCount; i++)
    {
        // add each item, but ommitted arguments are an error.
        RexxObject *item = args[i];
        if (item == OREF_NULL)
        {
            reportException(Error_Incorrect_method_noarg, i + 1);
        }
        newQueue->append(item);
    }
    return newQueue;
}


/**
 * Handle a string conversion REQUEST for a Queue object
 * we need to override this because Queue doesn't define a makeString() method
 *
 * @return Always return .nil
 */
RexxString *QueueClass::primitiveMakeString()
{
    return (RexxString *)TheNilObject;
}


/**
 * Handle a string conversion REQUEST for a Queue object
 * we need to override this because Queue doesn't define a makeString() method
 *
 * @return Always return .nil
 */
RexxString *QueueClass::makeString()
{
    return (RexxString *)TheNilObject;
}


