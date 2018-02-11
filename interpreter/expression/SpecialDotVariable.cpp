/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2018 Rexx Language Association. All rights reserved.         */
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
/* REXX Translator                                                            */
/*                                                                            */
/* Primitive Translator Expression Parsing Dot Variable Reference Class       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "SpecialDotVariable.hpp"


/**
 * Allocate a new Dot Variable expression object.
 *
 * @param size   The size of object.
 *
 * @return Storage for creating a dot variable object.
 */
void *SpecialDotVariable::operator new(size_t size)
{
    return new_object(size, T_SpecialDotVariableTerm);
}


/**
 * Construct a Dot variable retriever.
 *
 * @param variable_name
 *               The name of the symbol.
 * @param value  The value assigned to the name.
 */
SpecialDotVariable::SpecialDotVariable(RexxString *variable_name, RexxObject *value )
{
    variableName = variable_name;
    variableValue = value;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void SpecialDotVariable::live(size_t liveMark)
{
    memory_mark(variableName);
    memory_mark(variableValue);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void SpecialDotVariable::liveGeneral(MarkReason reason)
{
    memory_mark_general(variableName);
    memory_mark_general(variableValue);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void SpecialDotVariable::flatten(Envelope * envelope)
{
    setUpFlatten(SpecialDotVariable)

    flattenRef(variableName);
    flattenRef(variableValue);

    cleanUpFlatten
}


/**
 * Evaluate a dot variable symbol.
 *
 * @param context The current evaluation context.
 * @param stack   The current evaluation stack.
 *
 * @return The dot symbol value.
 */
RexxObject *SpecialDotVariable::evaluate(RexxActivation *context, ExpressionStack *stack )
{
    // evaluate always pushes on the stack.
    stack->push(variableValue);
    // trace this if tracing intermediates
    context->traceSpecialDotVariable(variableName, variableValue);
    return variableValue;
}


/**
 * Just retrieve a value for a dotvariable object without
 * pushing on the stack.
 *
 * @param context The current execution context.
 *
 * @return The dotvariable value.
 */
RexxObject *SpecialDotVariable::getValue(RexxActivation *context)
{
    // this just returns without pushing on the stack
    return variableValue;
}

