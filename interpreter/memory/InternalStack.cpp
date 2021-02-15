/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Internal Use Stack Class                                         */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "InternalStack.hpp"


/**
 * Allocate storage for a new internal stack.
 *
 * @param size      The base object size.
 * @param stackSize The required entries in the stack.
 *
 * @return Storage for a new stack item.
 */
void *InternalStack::operator new(size_t size, size_t stackSize)
{
    return new_object(size + (stackSize * sizeof(RexxInternalObject *)), T_InternalStack);
}


/**
 * Allocate storage for a new internal stack.
 *
 * @param size      The base object size.
 * @param stackSize The required entries in the stack.
 *
 * @return Storage for a new stack item.
 */
InternalStack::InternalStack(size_t stackSize)
{
    size = stackSize;
    top  = stack;
    stack[0] = OREF_NULL;
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void InternalStack::live(size_t liveMark)
{
   for (RexxInternalObject **entry = stack; entry <= top; entry++)
   {
       memory_mark(*entry);
   }
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void InternalStack::liveGeneral(MarkReason reason)
{
   for (RexxInternalObject **entry = stack; entry <= top; entry++)
   {
       memory_mark_general(*entry);
   }
}

