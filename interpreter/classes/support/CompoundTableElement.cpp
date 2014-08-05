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
/****************************************************************************/
/* REXX Kernel                                    CompoundTableElement.cpp  */
/*                                                                          */
/* Primitive Compound Variable Element Class                                */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "CompoundTableElement.hpp"
#include "Activity.hpp"


/**
 * Allocate a new compound table element object.
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *CompoundTableElement::operator new(size_t size)
{
    return new_object(size, T_CompoundElement);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void CompoundTableElement::live(size_t liveMark)
{
    memory_mark(variableValue);
    memory_mark(variableName);
    memory_mark(dependents);
    memory_mark(parent);
    memory_mark(left);
    memory_mark(right);
    memory_mark(realElement);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void CompoundTableElement::liveGeneral(MarkReason reason)
{
    memory_mark_general(variableValue);
    memory_mark_general(variableName);
    memory_mark_general(dependents);
    memory_mark_general(parent);
    memory_mark_general(left);
    memory_mark_general(right);
    memory_mark_general(realElement);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void CompoundTableElement::flatten(Envelope *envelope)
{
    setUpFlatten(CompoundTableElement)

    flattenRef(variableValue);
    flattenRef(variableName);
    // We do not want to flatten a table of activities, so
    // null that out.
    dependents = OREF_NULL;
    flattenRef(parent);
    flattenRef(left);
    flattenRef(right);
    flattenRef(realElement);

    cleanUpFlatten
}
