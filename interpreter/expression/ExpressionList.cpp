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
/* Primitive logical list evaluator                                           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ExpressionList.hpp"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"


/**
 * Create a new expression list object.
 *
 * @param size   The size of the class object.
 * @param count  The count of logical expressions.  Used to adjust the
 *               allocated size to the requirements.
 *
 * @return A new RexxExpressionLogical object.
 */
void *RexxExpressionList::operator new(size_t size, size_t  count)
{
    return new_object(size + (count - 1) * sizeof(RexxObject *), T_ListTerm);
}


/**
 * Constructor for a RexxExpressionList object.
 *
 * @param count  The number of expressions in the list.
 * @param list   The accumulated list of expressions.
 */
RexxExpressionList::RexxExpressionList(size_t count, QueueClass *list)
{
    expressionCount = count;

    // now copy the expressions from the sub term stack
    // NOTE:  The expressionss are in last-to-first order on the stack.
    initializeObjectArray(count, expressions, RexxInternalObject, list);
}


/**
 * The runtime, non-debug live marking routine.
 */
void RexxExpressionList::live(size_t liveMark)
{
    memory_mark_array(expressionCount, expressions);
}


/**
 * The generalized live marking routine used for non-performance
 * critical marking operations.
 */
void RexxExpressionList::liveGeneral(MarkReason reason)
{
    memory_mark_general_array(expressionCount, expressions);
}


/**
 * The flattening routine, used for serializing object trees.
 *
 * @param envelope The envelope were's flattening into.
 */
void RexxExpressionList::flatten(Envelope *envelope)
{
    setUpFlatten(RexxExpressionList)

    flattenArrayRefs(expressionCount, expressions);

    cleanUpFlatten
}

/**
 * Evaluate a logical expresion list.
 *
 * @param context The execution context.
 * @param stack   The evaluation stack.
 *
 * @return The result of the operation, either .true or .false.
 */
RexxObject *RexxExpressionList::evaluate(RexxActivation *context, ExpressionStack *stack)
{
    // loop through the expression list evaulating and then testing for the
    // logical value
    size_t count = expressionCount;

    // save the top of the stack for popping values off later.
    size_t stacktop = stack->location();

    // create a result array with a matching size
    Protected<ArrayClass> result = new_array(expressionCount);

    // there are no optional values in the list, so evaluate unconditionally.
    for (size_t i = 0; i < count; i++)
    {
        // evaluate and trace
        RexxInternalObject *expr = expressions[i];
        // if this is a real expression (omitted expressions are permitted)
        if (expr != OREF_NULL)
        {
            RexxObject *value = expr->evaluate(context, stack);
            // trace this as an argument value
            context->traceArgument(value);

            // add this to the created array
            result->put(value, i + 1);
        }
    }

    // remove the arguments from the stack and push our result
    stack->setTop(stacktop);
    stack->push(result);

    // TODO:  Need to reassess how this final result is traced.
    // trace the array result and return it
    context->traceResult(result);
    return result;
}

