/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
/* Primitive Message Instruction Parse Class                                  */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"
#include "ExpressionMessage.hpp"
#include "Token.hpp"
#include "ProtectedObject.hpp"


/**
 * Allocate memory for a message expression object.
 *
 * @param size     The base object size.
 * @param argCount The count of arguments for the message.
 *
 * @return Storage for building a message object.
 */
void *RexxExpressionMessage::operator new(size_t size, size_t argCount)
{
    if (argCount == 0)
    {
        // allocate with singleton item chopped off
        return new_object(size - sizeof(RexxObject *), T_MessageSendTerm);
    }
    else
    {
        // allocate with the space needed for the arguments
        return new_object(size + (argCount - 1) * sizeof(RexxObject *), T_MessageSendTerm);
    }
}


/**
 * Construct an expression message object.
 *
 * @param _target  The target of the message send.
 * @param name     The name of the message.
 * @param _super   A potential superclass override.
 * @param argCount The count of message arguments.
 * @param arglist  The argument list (a queue where the arguments can be pulled off)
 * @param double_form
 *                 A flag indicating if this is a ~ or ~~ operation.
 */
RexxExpressionMessage::RexxExpressionMessage(RexxInternalObject *_target, RexxString *name,
    RexxInternalObject *_super, size_t argCount, QueueClass *arglist, bool double_form)
{
    messageName = name;
    target = _target;
    super = _super;
    doubleTilde = double_form;
    argumentCount = argCount;
    initializeObjectArray(argCount, arguments, RexxInternalObject, arglist);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxExpressionMessage::live(size_t liveMark)
{
    memory_mark(messageName);
    memory_mark(target);
    memory_mark(super);
    memory_mark_array(argumentCount, arguments);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxExpressionMessage::liveGeneral(MarkReason reason)
{
    memory_mark_general(this->messageName);
    memory_mark_general(this->target);
    memory_mark_general(this->super);
    memory_mark_general_array(argumentCount, arguments);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxExpressionMessage::flatten(Envelope *envelope)
{
    setUpFlatten(RexxExpressionMessage)

    flattenRef(messageName);
    flattenRef(target);
    flattenRef(super);
    flattenArrayRefs(argumentCount, arguments);

    cleanUpFlatten
}


/**
 * Evaluate a message expression term.
 *
 * @param context The current execution context.
 * @param stack   The evaluation stack.
 *
 * @return The message result.
 */
RexxObject *RexxExpressionMessage::evaluate(RexxActivation *context, ExpressionStack *stack)
{
    // evaluate the target object
    RexxObject *_target = target->evaluate(context, stack);
    RexxClass *_super = OREF_NULL;

    // do we have a super class override?
    if (super != OREF_NULL)
    {
        _super = (RexxClass *)super->evaluate(context, stack);
        // _super an instance of TheClassClass
        if (!_super->isInstanceOf(TheClassClass))
        {
            reportException(Error_Invalid_argument_noclass, "SCOPE", "Class");
        }
        // validate the starting scope
        _target->validateScopeOverride(_super);
        // we send the message using the stack, which
        // expects to find the target and the arguments
        // on the stack, but not the super.  We need to
        // pop this item off after evaluation.  Since this
        // comes either from a variable or the environment, this
        // is already protected from GC.
        stack->toss();
    }

    // evaluate the arguments first
    RexxInstruction::evaluateArguments(context, stack, arguments, argumentCount);

    ProtectedObject result;

    // issue based on whether we have the override
    if (_super == OREF_NULL)
    {
        stack->send(messageName, argumentCount, result);
    }
    else
    {
        stack->send(messageName, _super, argumentCount, result);
    }

    // remove any arguments from the stack
    stack->popn(argumentCount);

    // double twidde form?  replace the result with the target .
    // NOTE:  currently, the target is the top item on the stack, so
    // we don't need to fix the stack
    if (doubleTilde)
    {
        result = _target;
    }
    // use the actual return value, replace it on the stack
    else
    {
        stack->prefixResult(result);
    }

    // we're in an expression here, so a result is required.
    if (result.isNull())
    {
        reportException(Error_No_result_object_message, messageName);
    }

    // trace if necessary
    context->traceMessage(messageName, result);

    return result;
}


/**
 * Perform an assignment operation for a message term
 * used in USE ARG or PARSE situation.
 *
 * @param context The current activation context.
 * @param value   The new value.
 */
void RexxExpressionMessage::assign(RexxActivation *context, RexxObject *value)
{
    // the stack is not passed to assignment operations but fortunately, we can
    // get that from the context.
    ExpressionStack *stack = context->getStack();

    // evaluate the target (protected on the stack)
    RexxObject *_target = target->evaluate(context, stack);
    RexxClass *_super = OREF_NULL;

    // message override?
    if (super != OREF_NULL)
    {
        // evaluate the superclass override
        _super = (RexxClass *)super->evaluate(context, stack);
        // we need to remove this from the stack for the send operation to work.
        stack->toss();
    }

    // push the assignment value on to the stack as the firt argument
    stack->push(value);
    // now push the rest of the arguments.  This might be something like a[1,2,3,4] as
    // an assignment term.  The assignment value is the first argument, followed by
    // any other arguments that are part of the encoded message term.
    size_t argcount = argumentCount;

    for (size_t i = 0; i < argcount; i++)
    {
        // non-omitted argument?
        if (arguments[i] != OREF_NULL)
        {
            // evaluate and potentially trace
            RexxObject *resultArg = arguments[i]->evaluate(context, stack);
            context->traceResult(resultArg);
        }
        else
        {
            // non existant arg....we may still need to trace that
            stack->push(OREF_NULL);
            context->traceResult(GlobalNames::NULLSTRING);
        }
    }

    ProtectedObject result;

    // now send the message the appropriate way.  Note we
    // have an extra arg from the assignment target.
    if (_super == OREF_NULL)
    {
        // normal message send
        stack->send(messageName, argcount + 1, result);
    }
    else
    {
        // send with an override
        stack->send(messageName, _super, argcount + 1, result);
    }

    context->traceAssignment(messageName, result);
    // remove all arguments (arguments + target + assignment value)
    stack->popn(argcount + 2);
}


/**
 * Convert a message into an assignment message by adding "="
 * to the end of the message name.
 *
 * @param source The current source context.
 */
void RexxExpressionMessage::makeAssignment(LanguageParser *parser)
{
    // add an equal sign to the name
    messageName = parser->commonString(messageName->concat(GlobalNames::EQUAL));
}

