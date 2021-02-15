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
/* REXX Translator                                                            */
/*                                                                            */
/* Primitive logical list evaluator                                           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"
#include "ExpressionLogical.hpp"


/**
 * Create a new logical list object.
 *
 * @param size   The size of the class object.
 * @param count  The count of logical expressions.  Used to adjust the
 *               allocated size to the requirements.
 *
 * @return A new RexxExpressionLogical object.
 */
void *RexxExpressionLogical::operator new(size_t size, size_t  count)
{
    return new_object(size + (count - 1) * sizeof(RexxObject *), T_LogicalTerm);
}


/**
 * Constructor for a RexxExpressionLogical object.
 *
 * @param count  The number of expressions in the list.
 * @param list   The accumulated list of expressions.
 */
RexxExpressionLogical::RexxExpressionLogical(size_t count, QueueClass *list)
{
    expressionCount = count;

    // now copy the expressions from the sub term stack
    // NOTE:  The expressionss are in last-to-first order on the stack.
    initializeObjectArray(count, expressions, RexxInternalObject, list);
}


/**
 * The runtime, non-debug live marking routine.
 */
void RexxExpressionLogical::live(size_t liveMark)
{
    memory_mark_array(expressionCount, expressions);
}


/**
 * The generalized live marking routine used for non-performance
 * critical marking operations.
 */
void RexxExpressionLogical::liveGeneral(MarkReason reason)
{
    memory_mark_general_array(expressionCount, expressions);
}


/**
 * The flattening routine, used for serializing object trees.
 *
 * @param envelope The envelope were's flattening into.
 */
void RexxExpressionLogical::flatten(Envelope *envelope)
{
    setUpFlatten(RexxExpressionLogical)

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
RexxObject *RexxExpressionLogical::evaluate(RexxActivation *context, ExpressionStack *stack)
{
    // loop through the expression list evaulating and then testing for the
    // logical value
    size_t count = expressionCount;
    // there are no optional values in the list, so evaluate unconditionally.
    for (size_t i = 0; i < count; i++)
    {
        // evaluate and trace
        RexxObject *value = expressions[i]->evaluate(context, stack);
        context->traceResult(value);

        // the comparison methods return either .true or .false, so we
        // can to a quick test against those.
        if (value != TheTrueObject)    // most of the time, these will be true so test that first.
        {
            if (value == TheFalseObject)
            {
                return TheFalseObject;
            }
            // ok, the either returned a '0' or a '1' that was not the result of returning
            // .true or .false, or we have a bad value.
            if (!value->truthValue(Error_Logical_value_logical_list))
            {
                return TheFalseObject;    // we terminate on the first false condition
            }
        }
    }
    return TheTrueObject;      // all is truth
}

