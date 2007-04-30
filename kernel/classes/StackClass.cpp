/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* REXX Kernel                                                  StackClass.c     */
/*                                                                            */
/* Primitive Stack Class                                                      */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StackClass.hpp"

RexxStack::RexxStack(
    size_t size)                       /* elements in the stack             */
/******************************************************************************/
/* Function:  Initialize a primitive stack.                                   */
/******************************************************************************/
{
  ClearObject(this);                   /* clear entire stack                */
  this->u_size = size;                 /* set the size                      */
  this->top = 0;                       /* and we're set at the top          */
}


void RexxStack::init(
    size_t size)                       /* elements in the stack             */
/******************************************************************************/
/* Function:  Initialize a primitive stack early in memory set up             */
/******************************************************************************/
{
  ClearObject(this);                   /* clear entire stack                */
  this->u_size = size;                 /* set the size                      */
  this->top = 0;                       /* and we're set at the top          */
}

void RexxStack::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  RexxObject **rp;

  setUpMemoryMark
  for (rp = this->stack; rp < this->stack+this->stackSize(); rp++)
    memory_mark(*rp);
  cleanUpMemoryMark
}

void RexxStack::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  RexxObject **rp;

  setUpMemoryMarkGeneral
  for (rp = this->stack; rp < this->stack+this->stackSize(); rp++)
    memory_mark_general(*rp);
  cleanUpMemoryMarkGeneral
}


void RexxStack::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxStack)
  size_t i;

   for (i=0; i < this->stackSize(); i++ ) {
    flatten_reference(newThis->stack[i], envelope);
   }
  cleanUpFlatten
}

RexxObject  *RexxStack::get(size_t pos)
/******************************************************************************/
/* Function:  Get a specific stack element                                    */
/******************************************************************************/
{
  if (pos < this->stackSize())
    return *(this->stack+(this->stackSize()+this->top-pos)%this->stackSize());
  else
    return OREF_NULL;
}

RexxObject  *RexxStack::pop()
/******************************************************************************/
/* Function:  Pop an element from the stack                                   */
/******************************************************************************/
{
  RexxObject *object;                  /* object removed from the stack     */

  object = *(this->stack + this->top); /* get the new item                  */
                                       /* needed by memory_alloc            */
  OrefSet(this, *(this->stack+this->top), OREF_NULL);
  decrementTop();                      /* move the top pointer (and potentially wrap) */
  return object;                       /* return the object                 */
}

RexxObject  *RexxStack::fpop()
/******************************************************************************/
/* Function:  Get top stack element, which is removed from the stack          */
/*                                                                            */
/* This "fast pop" method doesn't null out the popped stack entry and can     */
/* therefore leave spurious object references in the stack.  It is used by    */
/* memory_collect for mobj.livestack, where speed is of the essence and the   */
/* spurious references have no effect.                                        */
/******************************************************************************/
{
  RexxObject *object;                  /* object removed from the stack     */

  object = *(this->stack + this->top); /* get the top item                  */
  decrementTop();                      /* move the top pointer (and potentially wrap) */
  return object;                       /* return the object                 */
}

void *RexxStack::operator new(
     size_t size,                      /* Object size                       */
     size_t stksize,                   /* stack size                        */
     BOOL   temporary )                /* this is a temporary one           */
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
  RexxObject *newObject;               /* the new atack                     */

  if (!temporary)                      /* normal stack object?              */
                                       /* Get new object                    */
    newObject = new_object(size + ((stksize-1) * sizeof(RexxObject *)));
  else
                                       /* Get new object                    */
    newObject = memoryObject.temporaryObject(size + ((stksize-1) * sizeof(RexxObject *)));
                                       /* set the behaviour                 */
  BehaviourSet(newObject, TheStackBehaviour);
  return newObject;                    /* return the new object             */
}


RexxSaveStack::RexxSaveStack(
    size_t size,                       /* elements in the stack             */
    size_t allocSize)                  /* size to allocate!                 */
      : RexxStack(size)
/******************************************************************************/
/* Function:  Initialize a primitive stack.                                   */
/******************************************************************************/
{
  this->allocSize = allocSize;
}

void RexxSaveStack::init(
    size_t size,                       /* elements in the stack             */
    size_t allocSize)                  /* size to allocate                  */
/******************************************************************************/
/* Function:  Initialize a primitive stack early in memory set up             */
/******************************************************************************/
{
  ClearObject(this);                   /* clear entire stack                */
  this->u_size = size;                 /* set the size                      */
  this->top = 0;                       /* set the element to the top        */
  this->allocSize = allocSize;
}

void *RexxSaveStack::operator new(
     size_t size,                      /* Object size                       */
     size_t allocSize )                /* size to allocate                  */
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
  RexxObject *newObject;               /* the new atack                     */

                                       /* Get new object                    */
  newObject = new_object(size + ((allocSize-1) * sizeof(RexxObject *)));

                                       /* set the behaviour                 */
  BehaviourSet(newObject, TheStackBehaviour);
  return newObject;                    /* return the new object             */
}

void RexxSaveStack::extend(
    size_t newSize)                    /* new size to use                   */
/******************************************************************************/
/* Function:  Extend the usable size of the save stack                        */
/******************************************************************************/
{
   if (newSize < this->allocSize)
     this->u_size = newSize;
}

void RexxSaveStack::remove(
    RexxObject *element,               /* object to remove from save stack  */
    BOOL search)                       /* search through whole savestack?   */
{
   size_t i;

   /* I remember top element so that this operation is not disturbed by */
   /* another thread pushing something onto the savestack               */

   /* first check top element */
   i = this->top;

   if (this->stack[i] == element) {
     this->stack[i] = OREF_NULL;
     if (i == this->top)
        this->top--;
   } else
     /* not top element, search it if requested */
     if (search)
       for (i=0; i<this->u_size; i++)
         if (this->stack[i] == element) {
           this->stack[i] = OREF_NULL;
           break;
         }
}

#define SAVE_THRESHOLD 5

void RexxSaveStack::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  RexxObject **rp;

  setUpMemoryMark

  for (rp = this->stack; rp < this->stack+this->stackSize(); rp++) {
    RexxObject *thisObject = *rp;      /* get the next object in the stack */
    if (thisObject == OREF_NULL) {
        continue;                      /* an empty entry? just go on */
    }
                                       /* if the object has already been marked, */
    else if (ObjectIsMarked(thisObject)) {
        *rp = OREF_NULL;               /* we can clear this out now, rather than keeping it in the stack */
    }
    else {
                                       /* if this is a large, unmarked object, we */
                                       /* do some additional tests.  If this is a String */
                                       /* then we check the position within the stack.  If */
                                       /* it is not near the top of the stack, then this is */
                                       /* an already dead object.  We will clear this out */
                                       /* now to prevent GC thrashing */
        if (IsLargeObject(thisObject)) {
//          printf("Large object on save stack encountered of size %d\n", ObjectSize(thisObject));
                                       /* is this a large string? */
            if (OTYPE(String, thisObject)) {
                RexxString *string = (RexxString *)thisObject;
//              printf("Examining string object of length%d\n", string->length);
                                       /* if the pointer is less than the top, we don't */
                                       /* need to worry about wrapping now */
                if (rp < this->stack + this->top) {
                                       /* if in the save range, continue */
                    if (rp < this->stack + this->top - SAVE_THRESHOLD) {
                        *rp = OREF_NULL;  /* we can clear this out now, rather than keeping it in the stack */
//                      printf("Zeroing out SaveStack reference to marked object %p\n", thisObject);
                        continue;
                    }
                }
                else {
                                       /* see if we've wrapped recently */
                    long wrap = this->top - SAVE_THRESHOLD;
                    if (wrap < 0) {    /* check to see if this is in the top range */
                        if (rp < this->stack + (this->top - wrap)) {
                            *rp = OREF_NULL;  /* we can clear this out now, rather than keeping it in the stack */
//                          printf("Zeroing out SaveStack reference to unmarked object %p\n", thisObject);
                            continue;
                        }
                    }
                }
            }
        }
        /* this is an object we need to keep alive, but we'll only */
        /* do this for one GC cycle.  We'll clear this out now, to */
        /* make sure we don't keep this pinned longer than */
        /* necessary. */
        memory_mark(*rp);
        *rp = OREF_NULL;
    }
  }
  cleanUpMemoryMark
}

