/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
    memory_mark(cachedValue);
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
    // if this is an alert that the image is getting saved,
    // we try to resolve any dot variables we can so that
    // we don't have to add entries to the old-to-new
    // table during normal operations.
    if (reason == PREPARINGIMAGE)
    {
        RexxObject *t = OREF_NULL;   // required for the findClass call

        // any code in the image is part of the Rexx package, so we resolve
        // from that.
        RexxClass *rexxQueue = TheRexxPackage->findClass(variableName, t);
        // set with anything that is returned, including OREF_NULL
        setField(cachedValue, t);
    }

    memory_mark_general(variableName);
    memory_mark_general(cachedValue);
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
    flattenRef(cachedValue);

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
    // if we previously resolved this from a package source, then
    // we saved the value for quick access.
    if (cachedValue != OREF_NULL)
    {
        // evaluate always pushes on the stack.
        stack->push(cachedValue);
        // trace this if tracing intermediates
        context->traceDotVariable(variableName, cachedValue);
        return cachedValue;
    }

    // if this gets set by resolveDotVariable, the value
    // can be cached for quick lookup
    RexxObject *value = OREF_NULL;

    // try first from the environment
    RexxObject *result = context->resolveDotVariable(variableName, value);
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
    else
    {
        // the value returned by resolveDotVariable might
        // be capable to caching, so set this for the next call
        // (this might still be null if it came from a dynamic
        // source)
        setField(cachedValue, value);
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
    // if we previously resolved this from a package source, then
    // we saved the value for quick access.
    if (cachedValue != OREF_NULL)
    {
        return cachedValue;
    }

    // if this gets set by resolveDotVariable, the value
    // can be cached for quick lookup
    RexxObject *value = OREF_NULL;

    // try first from the environment
    RexxObject *result = context->resolveDotVariable(variableName, value);
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
    else
    {
        // the value returned by resolveDotVariable might
        // be capable to caching, so set this for the next call
        // (this might still be null if it came from a dynamic
        // source)
        setField(cachedValue, value);
    }
    // this just returns without pushing on the stack
    return result;
}

