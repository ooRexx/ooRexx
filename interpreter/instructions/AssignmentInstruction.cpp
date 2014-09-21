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
/* Primitive Assignment Parse Class                                           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ExpressionBaseVariable.hpp"
#include "RexxActivation.hpp"
#include "AssignmentInstruction.hpp"

RexxInstructionAssignment::RexxInstructionAssignment(RexxVariableBase *target, RexxInternalObject *_expression)
{
    variable = target;
    expression = _expression;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionAssignment::live(size_t liveMark)
{
    memory_mark(nextInstruction);
    memory_mark(variable);
    memory_mark(expression);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionAssignment::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(variable);
    memory_mark_general(expression);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionAssignment::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionAssignment)

    flattenRef(nextInstruction);
    flattenRef(variable);
    flattenRef(expression);

    cleanUpFlatten
}

/**
 * Execute a REXX assignment instruction
 * NOTE:  This instruction is implemented using two seperate paths
 * for traced vs. non-traced execution.  This reduces the checks
 * for non-traced execution to a single check in this very
 * heavily executed instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionAssignment::execute(RexxActivation *context, ExpressionStack *stack)
{
    // if tracing?  handle this via the slower path
    if (context->tracingInstructions())
    {
        context->traceInstruction(this);
        // get the expression value
        RexxObject *result = expression->evaluate(context, stack);
        // trace the result
        context->traceResult(result);
        // assign the variable
        variable->assign(context, result);
        // do debug pause
        context->pauseInstruction();
    }
    // fast path for non-traced execution
    else
    {
        variable->assign(context, expression->evaluate(context, stack));
    }
}

