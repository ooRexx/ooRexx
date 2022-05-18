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
/* Forward instruction implementation class                                   */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "VariableDictionary.hpp"
#include "RexxActivation.hpp"
#include "ForwardInstruction.hpp"
#include "MethodArguments.hpp"

/**
 * Initialize a FORWARD instruction.
 *
 * @param t      The target object.
 * @param m      The message name.
 * @param s      The superclass override
 * @param args   The args specified via ARGUMENTS option.
 * @param a      The args specified via the ARRAY() option.
 * @param c      the continue/return flag.
 */
RexxInstructionForward::RexxInstructionForward(RexxInternalObject * t, RexxInternalObject *m, RexxInternalObject *s, RexxInternalObject *args, ArrayClass *a, bool c)
{
    target = t;
    message = m;
    superClass = s;
    arguments = args;
    array = a;
    continueExecution = c;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionForward::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(target);
    memory_mark(message);
    memory_mark(superClass);
    memory_mark(arguments);
    memory_mark(array);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionForward::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(target);
    memory_mark_general(message);
    memory_mark_general(superClass);
    memory_mark_general(arguments);
    memory_mark_general(array);
}

void RexxInstructionForward::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionForward)

    flattenRef(nextInstruction);
    flattenRef(target);
    flattenRef(message);
    flattenRef(superClass);
    flattenRef(arguments);
    flattenRef(array);

    cleanUpFlatten
}

/**
 * Execute a FORWARD instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionForward::execute(RexxActivation *context, ExpressionStack *stack)
{
    context->traceInstruction(this);
    // only allowed in method contexts.
    if (!context->inMethod())
    {
        reportException(Error_Execution_forward);
    }

    // intialize each of the pieces and process what has been specified on the command.
    // the context figures out what has been overridden from the current context, so
    // we only set the pieces we've been given
    RexxObject *_target = OREF_NULL;
    RexxObject *_message = OREF_NULL;
    RexxClass  *_superClass = OREF_NULL;
    RexxObject **_arguments = OREF_NULL;
    size_t      count = 0;

    // sent to a different object target?
    if (target != OREF_NULL)
    {
        _target = target->evaluate(context, stack);
        context->traceKeywordResult(GlobalNames::TO, _target);
    }

    // sending a different message?
    if (message != OREF_NULL)
    {
        // we need this as a string value...and in upper case.
        RexxObject *temp = message->evaluate(context, stack);
        // needs tracing too
        context->traceKeywordResult(GlobalNames::MESSAGE, temp);
        _message = temp->requestString()->upper();
        // leave this on the stack.
        stack->push(_message);
    }

    // have a superclass override?
    if (superClass != OREF_NULL)
    {
        _superClass = (RexxClass *)superClass->evaluate(context, stack);
        if (_superClass != OREF_NULL )
        {
            if (!_superClass->isInstanceOf(TheClassClass))
            {
                reportException(Error_Invalid_argument_noclass, "SCOPE", "Class");
            }
        }
        context->traceKeywordResult(GlobalNames::CLASS, _superClass);
    }

    // overriding the arguments?
    if (arguments != OREF_NULL)
    {
        // we need to evaluate this argument, then get as an array
        RexxObject *temp = arguments->evaluate(context, stack);
        context->traceKeywordResult(GlobalNames::ARRAY, temp);
        ArrayClass *argArray = temp->requestArray();
        // protect this on the stack too
        stack->push(argArray);
        // make sure we got an acceptable array back.
        if (argArray == TheNilObject || argArray->isMultiDimensional())
        {
            reportException(Error_Execution_forward_arguments);
        }
        // get the size...
        count = argArray->size();
        // now we need to find the last real argument in this array
        if (count != 0 && argArray->get(count) == OREF_NULL)
        {
            count--;
            while (count > 0)
            {
                // done if we find a non-null argument
                if (argArray->get(count) != OREF_NULL)
                {
                    break;
                }
                count--;
            }
        }
        // point to the data in the argument array.
        _arguments = argArray->messageArgs();
    }

    // have we been overridden via the ARRAY keyword?  We only have
    // one of ARGUMENTS or ARRAY.
    if (array != OREF_NULL)
    {
        // this is an array of expressions, so we need to evaluate all of these
        count = array->size();
        for (size_t i = 1; i <= count; i++)
        {
            RexxObject *argElement = (RexxObject *)array->get(i);
            // a real argument?                  */
            if (argElement != OREF_NULL)
            {
                // trace each of these as arguments
                RexxObject *arg = argElement->evaluate(context, stack);
                // trace each of these as arguments
                context->traceArgument(arg);
            }
            else
            {
                // just push a null reference for the missing ones
                stack->push(OREF_NULL);
                context->traceArgument(GlobalNames::NULLSTRING);
            }
        }
        // note that we evaluated this one last, so that other
        // values pushed on the stack will not interfere with the arguments.
        _arguments = stack->arguments(count);
    }
    // now have the context forward this
    ProtectedObject result = context->forward(_target, (RexxString *)_message, _superClass, _arguments, count, continueExecution);
    // if we continued, then we may need to set the RESULT variable in the current context.
    if (continueExecution)
    {
        // if we have a result, then we need to trace this and set the result variable
        if (!result.isNull())
        {
            context->traceResult(result);
            context->setLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT, result);
        }
        // ne result returned, so we drop the RESULT variable
        else
        {
            context->dropLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT);
        }
        // and finally, pause
        context->pauseInstruction();
    }
}

