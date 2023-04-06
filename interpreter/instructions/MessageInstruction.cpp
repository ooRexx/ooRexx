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
#include "RexxActivation.hpp"
#include "VariableDictionary.hpp"
#include "MessageInstruction.hpp"
#include "ExpressionMessage.hpp"
#include "ProtectedObject.hpp"

/**
 * Construct a Message instruction from an already
 * constructed message expression.
 *
 * @param message The source message expression object.
 */
RexxInstructionMessage::RexxInstructionMessage(RexxExpressionMessage *message)
{
    target = message->target;
    super = message->super;
    name = message->messageName;
    argumentCount = message->argumentCount;
    for (size_t i = 0; i < argumentCount; i++)
    {
        arguments[i] = message->arguments[i];
    }
}

/**
 * Create an assignment message object from a source message expression object.
 *
 * @param message    The source message expression object.
 * @param expression The expression assignment value (passed as the first object).
 */
RexxInstructionMessage::RexxInstructionMessage(RexxExpressionMessage *message, RexxInternalObject *expression)
{
    target = message->target;
    super = message->super;
    name = message->messageName;
    // we add an additional first argument here, so add one to the argument count
    argumentCount = message->argumentCount + 1;
    // the assignment expression is the first argument
    arguments[0] = expression;
    for (size_t i = 1; i < argumentCount; i++)
    {
        arguments[i] = message->arguments[i - 1];
    }
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionMessage::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(name);
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
void RexxInstructionMessage::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(name);
    memory_mark_general(target);
    memory_mark_general(super);
    memory_mark_general_array(argumentCount, arguments);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionMessage::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionMessage)

    flattenRef(nextInstruction);
    flattenRef(name);
    flattenRef(target);
    flattenRef(super);
    flattenArrayRefs(argumentCount, arguments);

    cleanUpFlatten
}


/**
 * Execute a message instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionMessage::execute(RexxActivation *context, ExpressionStack *stack)
{
    context->traceInstruction(this);

    // evaluate the target object
    RexxObject *_target = target->evaluate(context, stack);
    RexxClass  *_super = OREF_NULL;

    // do we have a superclass override?
    if (super != OREF_NULL)
    {
        // get the superclass target
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
    // issue the send with or without a superclass override
    if (super == OREF_NULL)
    {
        stack->send(name, argumentCount, result);
    }
    else
    {
        stack->send(name, _super, argumentCount, result);
    }

    // for the instruction version, we don't worry about popping
    // anything off of the evaluation stack...that will be cleared automatically
    // when we complete.

    // if this is the double message version, replace the result object
    // with the target object.
    if (instructionType == KEYWORD_MESSAGE_DOUBLE)
    {
        result = _target;
    }

    // if we have a result, trace it and assign it to the variable result.
    if (!result.isNull())
    {
        // trace the message name and result
        context->traceMessage(name, result);
        context->setLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT, result);
    }
    // for no result, we drop the RESULT variable
    else
    {
        context->dropLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT);
    }

    context->pauseInstruction();
}

