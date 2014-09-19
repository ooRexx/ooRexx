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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Stack Class                                                      */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ObjectClass.hpp"
#include "MemoryStack.hpp"

/**
 * Create a new stack for the memory object from temporary
 * memory.
 *
 * @param size      The base size of the object.
 * @param stksize   The number of items we wish to have in the stack.
 * @param temporary A dummy argument just to get this new method invoked...
 *                  causes the stack to be allocated from temporary memory.
 *
 * @return A newly allocated stack object.
 */
void *LiveStack::operator new(size_t size, size_t stksize, bool temporary)
{
    // This is a special allocation.  We use this if we need to expand the livestack
    // during a GC operation, which of course is when we are not able to allocate from
    // the Rexx heap.
    RexxInternalObject *newObject = memoryObject.temporaryObject(size + ((stksize-1) * sizeof(RexxObject *)));
    // set the behaviour
    newObject->setBehaviour(TheLiveStackBehaviour);
    return newObject;
}


/**
 * Allocate a live stack with the given number of slots.
 *
 * @param _size  The stack size.
 */
LiveStack::LiveStack(size_t _size)
{
    // we can get created via means other than normal memory allocation, so
    // ensure we're completely cleared out
    clearObject();
    // set the size and top element
    size = _size;
    top = 0;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void LiveStack::live(size_t liveMark)
{
    // note, we only mark the array up to (but not including) the top position.
    if (top > 0)
    {
        memory_mark_array(top - 1, stack);
    }
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void LiveStack::liveGeneral(MarkReason reason)
{
    // note, we only mark the array up to (but not including) the top position.
    if (top > 0)
    {
        memory_mark_general_array(top - 1, stack);
    }
}


/**
 * Reallocate the stack to one that is larger by a given multiplier.
 *
 * @param multiplier The multiplier value.
 *
 * @return A newly allocated stack with the entries from this
 *         stack copied over to it.
 */
LiveStack *LiveStack::reallocate(size_t multiplier)
{
    // create a new stack that is larger by the given multiplier
    LiveStack *newStack = new (size * multiplier, true) LiveStack (size * multiplier);
    // copy the entries over to the new stack
    newStack->copyEntries(this);
    return newStack;
}


/**
 * Create a new push through stack
 *
 * @param size    The base size of the object.
 * @param stksize The number of items we wish to have in the stack.
 *
 * @return A newly allocated stack object.
 */
void *PushThroughStack::operator new(size_t size, size_t stksize)
{
    return new_object(size + ((stksize-1) * sizeof(RexxObject *)), T_PushThroughStack);
}


/**
 * Allocate a live stack with the given number of slots.
 *
 * @param _size  The stack size.
 */
PushThroughStack::PushThroughStack(size_t _size)
{
    // we can get created via means other than normal memory allocation, so
    // ensure we're completely cleared out
    clearObject();
    // set the size and first element (we're essentially starting
    // out as if we pushed OREF_NULL on to the stack)
    size = _size;
    current = 0;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void PushThroughStack::live(size_t liveMark)
{
    // note, because we are a push-through stack, we mark everything.
    memory_mark_array(size, stack);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void PushThroughStack::liveGeneral(MarkReason reason)
{
    // note, because we are a push-through stack, we mark everything.
    memory_mark_general_array(size, stack);
}

/**
 * Remove an element from the stack and NULL out the stack slot.
 *
 * @param element The element to remove.
 * @param search  indicates whether we should search the stack for the item.
 */
void PushThroughStack::remove(RexxInternalObject *element, bool search)
{
    // first check the top item (which is easy)...we just leave the empty slot
    // if we found it.
    if (stack[current] == element)
    {
        stack[current] = OREF_NULL;
    }
    else
    {
        // not top element, search it if requested
        if (search)
        {
            for (size_t i= 0; i < size; i++)
            {
                if (stack[i] == element)
                {
                    stack[i] = OREF_NULL;
                    break;
                }
            }
        }
    }
}


