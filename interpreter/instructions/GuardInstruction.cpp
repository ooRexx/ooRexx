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
/* Primitive Guard Parse Class                                                */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ArrayClass.hpp"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "GuardInstruction.hpp"
#include "ExpressionBaseVariable.hpp"

/**
 * Initialize a GUARD instruction instance.
 *
 * @param _expression
 *               Optional guard expression.
 * @param variable_list
 *               the list of guard variables in the expression.
 * @param on_off indicates whether this is GUARD ON or GUARD OFF.
 */
RexxInstructionGuard::RexxInstructionGuard(RexxInternalObject *_expression,
    ArrayClass  *variable_list, bool on_off)
{
    expression = _expression;
    guardOn = on_off;
    // set a default for this.
    variableCount = 0;
    // do we have a guard expression?  We need to copy the guard variable
    // list.
    if (variable_list != OREF_NULL)
    {
        variableCount = variable_list->items();
        /* loop through the variable list    */
        for (size_t i = 0; i < variableCount; i++)
        {
            variables[i] = (RexxVariableBase *)(variable_list->get(i + 1));
        }
    }
}


void RexxInstructionGuard::live(size_t liveMark)
{
    memory_mark(nextInstruction);
    memory_mark(expression);
    memory_mark_array(variableCount, variables);
}



void RexxInstructionGuard::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general(expression);
    memory_mark_general_array(variableCount, variables);
}

void RexxInstructionGuard::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionGuard)

    flattenRef(nextInstruction);
    flattenRef(expression);
    flattenArrayRefs(variableCount, variables);

    cleanUpFlatten
}


/**
 * Execute a GUARD instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionGuard::execute(RexxActivation *context, ExpressionStack *stack )
{
    context->traceInstruction(this);

    // only valid in a method context.
    if (!context->inMethod())
    {
        reportException(Error_Translation_guard_guard);
    }

    // non-expression form?
    else if (expression == OREF_NULL)
    {
        // handle the ON/OFF status
        if (guardOn)
        {
            context->guardOn();
        }
        else
        {
            context->guardOff();
        }
    }
    else
    {
        // we need to set a guard watch on all variables used in the GUARD expression
        for (size_t i = 0; i < variableCount; i++)
        {
            variables[i]->setGuard(context);
        }

        // ok, we set the GUARD state to the the state we want.  Then we evaluate
        // the expression and if false, we wait for a change in guard stage.  Every
        // time we wait, we release the guard lock, and when we wake up again, we
        // return to our pre-wait state.
        if (guardOn)
        {
            context->guardOn();
        }
        else
        {
            context->guardOff();
        }

        // initialize the guard SEM
        context->getActivity()->guardSet();
        // get the expression result
        RexxObject *result = expression->evaluate(context, stack);
        context->traceKeywordResult(GlobalNames::WHEN, result);

        // do first evaluation without establishing doing any waits
        if (!result->truthValue(Error_Logical_value_guard))
        {
            // ok, we're currently false on the guard expression.  We need to loop
            // until this evaluates to true.
            do
            {
                // clean the expression stack so we don't overrun this doing
                // multiple checks
                stack->clear();
                // now perform the guard wait
                context->guardWait();
                // reset our activity guard semaphore.
                ActivityManager::currentActivity->guardSet();
                // try the expression again
                result = expression->evaluate(context, stack);
                context->traceKeywordResult(GlobalNames::WHEN, result);
                // and continue until we get a true result
            } while (!result->truthValue(Error_Logical_value_guard));
        }
        // ok, we're good now...remove the variable watch points
        for (size_t i = 0; i < variableCount; i++)
        {
            variables[i]->clearGuard(context);
        }
    }
}

