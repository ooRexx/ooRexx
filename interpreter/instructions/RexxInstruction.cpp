/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Translator Abstract Instruction Code                             */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxInstruction.hpp"
#include "Clause.hpp"
#include "RexxActivation.hpp"
#include "MethodArguments.hpp"


/**
 * Allocate a new instruction object.
 *
 * @param size   The object size.
 *
 * @return A newly allocated instruction object.
 */
void * RexxInstruction::operator new(size_t size)
{
    return new_object(size, T_Instruction);
}


/**
 * Base constructor for an instruction execution object.
 *
 * @param clause The clause this was created from (gives location information).
 * @param type   The instruction keyword type.  Note that the type
 *               does not necessarily map to a keyword name.  There
 *               are special purpose instructions that do not correspond
 *               to specific keywords, and some keyword instructions
 *               may have more that one execution object tailored
 *               to specific subfunctions.
 */
RexxInstruction::RexxInstruction(RexxClause *clause, InstructionKeyword type)
{
    instructionType = type;
    // for instructions that are generated as part of other instructions (for example,
    // internal branching instructions for an IF instruction), we don't have a clause
    // to provide location information.  Just zero the location.
    if (clause != OREF_NULL)
    {
        instructionLocation = clause->getLocation();
    }
    else
    {
        instructionLocation.setStart(0, 0);
    }
}


/**
 * Perform garbage collection on a live object.  This is
 * the default superclass method.  For efficiency, it
 * is recommended that subclasses mark nextInstruction
 * directly (and as the first item marked) rather than
 * forwarding to the superclass method.
 *
 * @param liveMark The current live mark.
 */
void RexxInstruction::live(size_t liveMark)
{
    memory_mark(nextInstruction);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstruction::liveGeneral(MarkReason reason)
{
    memory_mark_general(nextInstruction);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstruction::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstruction)

    flattenRef(nextInstruction);

    cleanUpFlatten
}


/**
 * Common method for evaluating arrays of arguments for
 * the different instruction or expression types that
 * use arguments (CALL, message, functions, etc.)
 *
 * @param context  The current execution context.
 * @param stack    The current evaluation stack
 * @param argArray The pointer to the array of argument expressions.
 * @param argCount The number of argument expressions.
 */
void RexxInstruction::evaluateArguments(RexxActivation *context, ExpressionStack *stack, RexxInternalObject **argArray, size_t argCount)
{
    // evaluate all of the arguments
    for (size_t i = 0; i < argCount; i++)
    {
        // real argument expression
        if (argArray[i] != OREF_NULL)
        {
            // evaluate the expression (and the argument is left on the stack)
            RexxObject *result = argArray[i]->evaluate(context, stack);
            context->traceArgument(result);
        }
        // omitted argument.  Push a null value and trace as a null string
        else
        {
            stack->push(OREF_NULL);
            context->traceArgument(GlobalNames::NULLSTRING);
        }
    }
}


/**
 * Perform garbage collection on a live object.  Note, many
 * subclasses of RexxInstructionExpression do not need to
 * provide their own marking methods unless they have additional
 * fields that require marking.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionExpression::live(size_t liveMark)
{
    memory_mark(nextInstruction);
    memory_mark(expression);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionExpression::liveGeneral(MarkReason reason)
{
    memory_mark_general(nextInstruction);
    memory_mark_general(expression);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionExpression::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionExpression)

    flattenRef(nextInstruction);
    flattenRef(expression);

    cleanUpFlatten
}


/**
 * Common method for evaluating the single expression
 * and tracing the result for subclasses of this
 * class.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 *
 * @return The expression result, or OREF_NULL if there is no
 *         expression to evaluate.
 */
RexxObject *RexxInstructionExpression::evaluateExpression(RexxActivation *context, ExpressionStack *stack)
{
    // if we have an expression value, evaluate it.
    if (expression != OREF_NULL)
    {
        // evaluate this
        RexxObject *result = expression->evaluate(context, stack);
        context->traceResult(result);
        return result;
    }
    // no expression, no result
    return OREF_NULL;
}


/**
 * Common method for evaluating the single expression
 * and tracing the result for subclasses of this
 * class.  The result is forced to string form.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 *
 * @return The expression result, or OREF_NULL if there is no
 *         expression to evaluate.
 */
RexxString *RexxInstructionExpression::evaluateStringExpression(RexxActivation *context, ExpressionStack *stack)
{
    // if we have an expression value, evaluate it.
    if (expression != OREF_NULL)
    {
        // evaluate this
        RexxObject *result = expression->evaluate(context, stack);
        // force to string form, trace, and return the string version
        RexxString *stringResult = result->requestString();
        // protect on the stack
        stack->push(stringResult);
        context->traceResult(stringResult);
        return stringResult;
    }
    // the string expression is required here, so return a NULL string if no
    // expression.  We still need to trace that.
    else
    {
        context->traceResult(GlobalNames::NULLSTRING);
        return GlobalNames::NULLSTRING;
    }
}


