/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"
#include "RexxInstruction.hpp"
#include "ExpressionLogical.hpp"
#include "StackClass.hpp"
#include "RexxActivity.hpp"
#include "BuiltinFunctions.hpp"
#include "SourceFile.hpp"

/**
 * Constructor for a RexxExpressionLogical object.
 *
 * @param source The source parsing context (used for raising errors)
 * @param count  The number of expressions in the list.
 * @param list   The accumulated list of expressions.
 */
RexxExpressionLogical::RexxExpressionLogical(RexxSource *source, size_t count, RexxQueue  *list)
{
    expressionCount = count;

    // the parsed expressions are stored in a queue, so we process them in
    // reverse order.
    while (count > 0)
    {
        RexxObject *condition = list->pop();
        if (condition == OREF_NULL)
        {
            source->syntaxError(Error_Invalid_expression_logical_list);
        }
        OrefSet(this, this->expressions[--count], condition);
    }
}

/**
 * The runtime, non-debug live marking routine.
 */
void RexxExpressionLogical::live(size_t liveMark)
{
  size_t  i;                           /* loop counter                      */
  size_t  count;                       /* argument count                    */

  for (i = 0, count = this->expressionCount; i < count; i++)
  {
      memory_mark(this->expressions[i]);
  }
}

/**
 * The generalized live marking routine used for non-performance
 * critical marking operations.
 */
void RexxExpressionLogical::liveGeneral(int reason)
{
  size_t  i;                           /* loop counter                      */
  size_t  count;                       /* argument count                    */

  for (i = 0, count = this->expressionCount; i < count; i++)
  {
      memory_mark_general(this->expressions[i]);
  }
}

/**
 * The flattening routine, used for serializing object trees.
 *
 * @param envelope The envelope were's flattening into.
 */
void RexxExpressionLogical::flatten(RexxEnvelope *envelope)
{
  size_t  i;                           /* loop counter                      */
  size_t  count;                       /* argument count                    */

  setUpFlatten(RexxExpressionLogical)

  for (i = 0, count = this->expressionCount; i < count; i++)
  {
      flatten_reference(newThis->expressions[i], envelope);
  }

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
RexxObject *RexxExpressionLogical::evaluate(RexxActivation *context, RexxExpressionStack *stack)
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
                                         /* Get new object                    */
    return new_object(size + (count - 1) * sizeof(RexxObject *), T_LogicalTerm);
}

