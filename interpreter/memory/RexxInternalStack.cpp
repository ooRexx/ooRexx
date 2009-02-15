/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                         RexxInternalStack.c    */
/*                                                                            */
/* Primitive Internal Use Stack Class                                         */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxInternalStack.hpp"

void RexxInternalStack::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
   RexxObject **entry;                 /* marked stack entry                */

                                       /* loop through the stack entries    */
   for (entry = this->stack; entry <= this->top; entry++)
   {
       memory_mark(*entry);              /* marking each one                  */
   }
}

void RexxInternalStack::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
   RexxObject **entry;                 /* marked stack entry                */

                                       /* loop through the stack entries    */
   for (entry = this->stack; entry <= this->top; entry++)
   {
       memory_mark_general(*entry);      /* marking each one                  */
   }
}

void RexxInternalStack::flatten(RexxEnvelope * envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInternalStack)

  size_t i;                            /* pointer for scanning stack entries*/
  size_t count;                        /* number of elements                */

  count = this->top - this->stack;     /* get the total count               */
                                       /* loop through the stack entries    */
   for (i = 0; i < count; i++)
   {
       flatten_reference(newThis->stack[i], envelope);
   }
  cleanUpFlatten
}

RexxInternalStack *RexxInternalStack::newInstance(
    size_t stackSize)                 /* stack size                        */
/******************************************************************************/
/* Function:  Create a new expression stack                                   */
/******************************************************************************/
{
  RexxInternalStack* newObj;          /* newly create stack                */

                                       /* Get new object                    */
  newObj = (RexxInternalStack *)new_object(sizeof(RexxInternalStack) + (stackSize * sizeof(RexxObject *)), T_InternalStack);
  newObj->size = stackSize;            /* set the size                      */
  newObj->top  = newObj->stack;        /* set the top element               */
                                       /* set marker for "end of object" to */
                                       /* protect against stack overflow    */
  newObj->stack[0] = OREF_NULL;        /* clear out the first element       */
  return newObj;                       /* return the new stack item         */
}

