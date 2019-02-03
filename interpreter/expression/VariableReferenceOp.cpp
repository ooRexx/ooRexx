/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/*                                                                            */
/* Primitive Translator Expression VariableReference operator class           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "VariableReferenceOp.hpp"
#include "ExpressionBaseVariable.hpp"
#include "VariableReference.hpp"
#include "RexxActivation.hpp"


/**
 * Allocate storage for a simple variable retriever.
 *
 * @param size   The base object size.
 *
 * @return Rexx object storage for the object.
 */
void *VariableReferenceOp::operator new(size_t size)
{
    return new_object(size, T_VariableReferenceOp);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void VariableReferenceOp::live(size_t liveMark)
{
    memory_mark(variable);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void VariableReferenceOp::liveGeneral(MarkReason reason)
{
    memory_mark_general(variable);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void VariableReferenceOp::flatten(Envelope *envelope)
{
    setUpFlatten(VariableReferenceOp)

    flattenRef(variable);

    cleanUpFlatten
}


/**
 * Evaluate a variable reference in an expression.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 *
 * @return The variable reference object (also pushed on the
 *         stack)
 */
RexxObject *VariableReferenceOp::evaluate(RexxActivation *context, ExpressionStack *stack)
{
    // look up the variable
    VariableReference *value = variable->getVariableReference(context);
    stack->push(value);
    // trace as an operator
    context->traceOperator(">", value->getName());
    return value;
}





