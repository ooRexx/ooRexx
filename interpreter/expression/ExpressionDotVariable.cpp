/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
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
/* REXX Translator                                                            */
/*                                                                            */
/* Primitive Translator Expression Parsing Dot Variable Reference Class       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "ExpressionDotVariable.hpp"


/**
 * Allocate a new Dot Variable expression object.
 *
 * @param size   The size of object.
 *
 * @return Storage for creating a dot variable object.
 */
void * RexxDotVariable::operator new(size_t size)
{
    return new_object(size, T_DotVariableTerm);
}


/**
 * Construct a Dot variable retriever.
 *
 * @param variable_name
 *               The name of the symbol.
 */
RexxDotVariable::RexxDotVariable(RexxString *variable_name )
{
    variableName = variable_name;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxDotVariable::live(size_t liveMark)
{
    memory_mark(variableName);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxDotVariable::liveGeneral(MarkReason reason)
{
    memory_mark_general(variableName);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxDotVariable::flatten(Envelope * envelope)
{
    setUpFlatten(RexxDotVariable)

    flattenRef(variableName);

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
RexxObject * RexxDotVariable::evaluate(RexxActivation *context, ExpressionStack *stack )
{
    RexxObject *result;

    // we handle .nil, .true, and .false as a special case here to
    // ensure we're getting the real stuff rather than overrides
    // somebody has poked into the environment
    size_t length = variableName->getLength();
    const char *name = variableName->getStringData();
    if (length == 3 && memcmp(name, "NIL", 3) == 0)
    {
        result = TheNilObject;
    }
    else if (length == 4 && memcmp(name, "TRUE", 4) == 0)
    {
        result = TheTrueObject;
    }
    else if (length == 5 && memcmp(name, "FALSE", 5) == 0)
    {
        result = TheFalseObject;
    }
    else
    {
        // try from the environment
        result = context->resolveDotVariable(variableName);
        if (result == OREF_NULL)
        {
            // might be a special rexx name
            result = context->rexxVariable(variableName);

            if (result == OREF_NULL)
            {
                // add a period to the name
                result = variableName->concatToCstring(".");
            }
        }
    }
    // evaluate always pushes on the stack.
    stack->push(result);
    // trace this if tracing intermediates
    context->traceDotVariable(variableName, result);
    return result;
}


/**
 * Just retrieve a value for a dotvariable object without
 * pushing on the stack.
 *
 * @param context The current execution context.
 *
 * @return The dotvariable value.
 */
RexxObject * RexxDotVariable::getValue(RexxActivation *context)
{
    // try first from the environment
    RexxObject *result = context->resolveDotVariable(variableName);
    if (result == OREF_NULL)
    {
        // might be a special rexx name
        result = context->rexxVariable(variableName);

        if (result == OREF_NULL)
        {
            // add a period to the name
            result = variableName->concatToCstring(".");
        }
    }
    // this just returns without pusing on the stack
    return result;
}

