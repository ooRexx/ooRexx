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
/* When instruction for a SELECT CASE                                         */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "WhenCaseInstruction.hpp"
#include "Token.hpp"


/**
 * Construct a WHEN instructon instance.
 *
 * @param _condition The condition expression to evaluate.
 * @param thenToken  The token for the terminating THEN keyword.
 *                   This is where we mark the end of the
 *                   instruction.
 */
RexxInstructionCaseWhen::RexxInstructionCaseWhen(size_t count, QueueClass *list, RexxToken *thenToken)
{
    expressionCount = count;

    // now copy the expressions from the sub term stack
    // NOTE:  The expressionss are in last-to-first order on the stack.
    initializeObjectArray(count, expressions, RexxInternalObject, list);

    //get the location from the THEN token and use its location to set
    // the end of the instruction.  Note that the THEN is traced on its
    // own, but using the start of the THEN gives a fuller picture of things.
    SourceLocation location = thenToken->getLocation();
    setEnd(location.getLineNumber(), location.getOffset());
}


/**
 * Set the END location for the false branch of an IF
 * instruction.  This will either be an ELSE clause, or
 * the instruction following the instruction on the THEN.
 *
 * @param end_target The new end instruction.
 */
void RexxInstructionCaseWhen::setEndInstruction(RexxInstructionEndIf *end_target)
{
    else_location = end_target;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionCaseWhen::live(size_t liveMark)
{
    // must be first object marked.
    memory_mark(nextInstruction);
    memory_mark(else_location);
    memory_mark_array(expressionCount, expressions);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionCaseWhen::liveGeneral(MarkReason reason)
{
    // must be first object marked.
    memory_mark_general(nextInstruction);
    memory_mark_general(else_location);
    memory_mark_general_array(expressionCount, expressions);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionCaseWhen::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionCaseWhen)

    flattenRef(nextInstruction);
    flattenRef(else_location);
    flattenArrayRefs(expressionCount, expressions);

    cleanUpFlatten
}


/**
 * Execute a WHEN instruction attached to a SELECT CASE.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionCaseWhen::execute(RexxActivation *context, ExpressionStack *stack)
{
    context->traceInstruction(this);

    // This should be us.  It really isn't possible to jump into a middle of a select
    // and get to a WHEN without raising an error.
    DoBlock *doBlock = context->topBlockInstruction();
    // get the case expression
    RexxObject *caseValue = doBlock->getCase();

    for (size_t i = 0; i < expressionCount; i++)
    {
        // and the compare target (which needs tracing, but only as an intermediate
        RexxObject *compareValue = expressions[i]->evaluate(context, stack);
        context->traceResult(compareValue);
        // now perform the compare using the "==" operator method.
        // NOTE that the case value is the left hand side.
        RexxObject *result = caseValue->callOperatorMethod(OPERATOR_STRICT_EQUAL, compareValue);
        context->traceResult(result);

        // Remove the compare object from the stack
        stack->toss();

        // evaluate and decide if we execute this WHEN...we stop with the first true result
        if (result->truthValue(Error_Logical_value_when_case))
        {
            // do a pause and return
            context->pauseInstruction();
            return;
        }
    }

    // we execute the ELSE branch if everything evaluates to false
    context->setNext(else_location->nextInstruction);
    context->pauseInstruction();
}

