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
/* REXX Translator                                                            */
/*                                                                            */
/* Primitive Function Invocation Class                                        */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "QueueClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxActivation.hpp"
#include "RexxInstruction.hpp"
#include "ExpressionFunction.hpp"
#include "Token.hpp"
#include "StackClass.hpp"
#include "RexxActivity.hpp"
#include "ProtectedObject.hpp"


/**
 * Allocate a new function call object.
 *
 * @param size     The base object size
 * @param argCount The count of function arguments need to allocate space.
 *
 * @return The storage for creating a function object.
 */
void *RexxExpressionFunction::operator new(size_t size, size_t argCount)
{
    if (argCount == 0)
    {
        // allocate with singleton item chopped off
        return new_object(size - sizeof(RexxObject *), T_FunctionCallTerm);
    }
    else
    {
        // allocate with the space needed for the arguments
        return new_object(size + (argCount - 1) * sizeof(RexxObject *), T_FunctionCallTerm);
    }
}

/**
 * Construct a function call expression object.
 *
 * @param function_name
 *                   The name of the function.
 * @param argCount   The argument count.
 * @param arglist    The source arguments.
 * @param index      A potential builtin function index.
 */
RexxExpressionFunction::RexxExpressionFunction(RexxString *function_name,
    size_t argCount, RexxQueue *arglist, size_t index)
{
    functionName = function_name;
    builtinIndex = index;
    argumentCount = argCount;

    // now copy any arguments from the sub term stack
    // NOTE:  The arguments are in last-to-first order on the stack.
    initializeObjectArray(argCount, arguments, RexxObject, argList);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxExpressionFunction::live(size_t liveMark)
{
    memory_mark(target);
    memory_mark(functionName);
    memory_mark_array(argumentCount, arguments);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxExpressionFunction::liveGeneral(int reason)
{
    memory_mark_general(target);
    memory_mark_general(functionName);
    memory_mark_general_array(argumentCount, arguments);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxExpressionFunction::flatten(RexxEnvelope *envelope)
{
    setUpFlatten(RexxExpressionFunction)

    flattenRef(target);
    flattenRef(functionName);
    flattenArrayRefs(argumentCount, arguments);

    cleanUpFlatten
}


/**
 * Delayed resolution of function calls.
 *
 * @param labels The table of label instructions in the current code block.
 */
void RexxExpressionFunction::resolve(RexxDirectory *labels)
{
    // Note, if we are not allowed to have internal calls, we never get added to the
    // resolution list.  If we get a resolve call, then we need to check for this.
    // if there is a labels table, see if we can find a label object from the context.
    if (labels != OREF_NULL)
    {
        // see if there is a matching label.  If we get something,
        // we're finished.
        targetInstruction = (RexxInstruction *)labels->at((RexxString *)targetName));
    }

    // really nothing else required here.  If we did not resolve a label location, then
    // the next step depends on whether we have a valid builtin index or not.
}


/**
 * Evaluate a function call.
 *
 * @param context The current execution context.
 * @param stack   the current evaluation stack.
 *
 * @return The function return value.
 */
RexxObject *RexxExpressionFunction::evaluate(RexxActivation *context, RexxExpressionStack *stack )
{
    // save the top of the stack for popping values off later.
    size_t stacktop = stack->location();

    // evaluate all of the arguments
    for (size_t i = 0; i < argumentCount; i++)
    {
        // real argument expression
        if (arguments[i] != OREF_NULL)
        {
            // evaluate the expression (and the argument is left on the stack)
            RexxObject *result = arguments[i]->evaluate(context, stack);
            context->traceArgument(result);
        }
        // omitted argument.  Push a null value and trace as a null string
        else
        {
            stack->push(OREF_NULL);
            context->traceArgument(OREF_NULLSTRING);
        }
    }

    ProtectedObject   result;            // returned result

    // if we resolved to an internal label, call that now
    if (target != OREF_NULL)
    {
        context->internalCall(functionName, target, argumentCount, stack, result);
    }
    // if this was resolved to a builtin, call directly
    else if (builtinIndex != NO_BUILTIN)
    {
        result = (RexxObject *) (*(RexxSource::builtinTable[builtinIndex]))(context, argumentCount, stack);
    }
    else
    {
        context->externalCall(functionName, argumentCount, stack, OREF_FUNCTIONNAME, result);
    }

    // functions must have a return result
    if ((RexxObject *)result == OREF_NULL)
    {
         reportException(Error_Function_no_data_function, functionName);
    }

    // remove the arguments from the stack and push our result
    stack->setTop(stacktop);
    stack->push((RexxObject *)result);

    // trace if needed and return the result
    context->traceFunction(functionName, (RexxObject *)result);
    return(RexxObject *)result;
}
