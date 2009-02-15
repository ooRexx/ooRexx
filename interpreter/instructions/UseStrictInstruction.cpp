/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* Primitive USE STRICT instruction class                                     */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "ArrayClass.hpp"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"
#include "UseStrictInstruction.hpp"
#include "ExpressionBaseVariable.hpp"


RexxInstructionUseStrict::RexxInstructionUseStrict(size_t count, bool strict, bool extraAllowed, RexxQueue *variable_list, RexxQueue *defaults)
{
    // set the variable count and the option flag
    variableCount = count;
    variableSize = extraAllowed; // we might allow an unchecked number of additional arguments
    minimumRequired = 0;         // do don't necessarily require any of these.
    strictChecking = strict;     // record if this is the strict form

    // items are added to the queues in reverse order, so we pop them off and add
    // them to the end of the list as we go.
    while (count > 0)     // loop through our variable set, adding everything in.
    {
        // decrement first, so we store at the correct offset.
        count--;
        OrefSet(this, variables[count].variable, (RexxVariableBase *)variable_list->pop());
        OrefSet(this, variables[count].defaultValue, defaults->pop());

        // if this is a real variable, see if this is the last of the required ones.
        if (minimumRequired < count + 1 && variables[count].variable != OREF_NULL)
        {
            // no default value means this is a required argument, this is the min we'll accept.
            if (variables[count].defaultValue == OREF_NULL)
            {
                minimumRequired = count + 1;
            }
        }
    }
}


/**
 * The runtime, non-debug live marking routine.
 */
void RexxInstructionUseStrict::live(size_t liveMark)
{
  size_t i;                            /* loop counter                      */
  size_t count;                        /* argument count                    */

  memory_mark(this->nextInstruction);  /* must be first one marked          */
  for (i = 0, count = variableCount; i < count; i++)
  {
      memory_mark(this->variables[i].variable);
      memory_mark(this->variables[i].defaultValue);
  }
}


/**
 * The generalized live marking routine used for non-performance
 * critical marking operations.
 */
void RexxInstructionUseStrict::liveGeneral(int reason)
{
  size_t i;                            /* loop counter                      */
  size_t count;                        /* argument count                    */

                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  for (i = 0, count = variableCount; i < count; i++)
  {
      memory_mark_general(this->variables[i].variable);
      memory_mark_general(this->variables[i].defaultValue);
  }
}


/**
 * The flattening routine, used for serializing object trees.
 *
 * @param envelope The envelope were's flattening into.
 */
void RexxInstructionUseStrict::flatten(RexxEnvelope *envelope)
{
  size_t i;                            /* loop counter                      */
  size_t count;                        /* argument count                    */

  setUpFlatten(RexxInstructionUseStrict)

  flatten_reference(newThis->nextInstruction, envelope);
  for (i = 0, count = variableCount; i < count; i++)
  {
      flatten_reference(newThis->variables[i].variable, envelope);
      flatten_reference(newThis->variables[i].defaultValue, envelope);
  }
  cleanUpFlatten
}


void RexxInstructionUseStrict::execute(RexxActivation *context, RexxExpressionStack *stack)
{
    context->traceInstruction(this);     // trace if necessary
    // get the argument information from the context
    RexxObject **arglist = context->getMethodArgumentList();
    size_t argcount = context->getMethodArgumentCount();
    // strict checking means we need to enforce min/max limits
    if (strictChecking)
    {
        // not enough of the required arguments?  That's an error
        if (argcount < minimumRequired)
        {
            // this is a pain, but there are different errors for method errors vs. call errors.
            if (context->inMethod())
            {
                reportException(Error_Incorrect_method_minarg, minimumRequired);
            }
            else
            {
                reportException(Error_Incorrect_call_minarg, context->getCallname(), minimumRequired);
            }
        }
        // potentially too many?
        if (!variableSize && argcount > variableCount)
        {
            if (context->inMethod())
            {
                reportException(Error_Incorrect_method_maxarg, variableCount);
            }
            else
            {
                reportException(Error_Incorrect_call_maxarg, context->getCallname(), variableCount);
            }
        }
    }

    // now we process each of the variable definitions left-to-right
    for (size_t i = 0; i < variableCount; i++)
    {
        // get our current variable.  We're allowed to skip over variables, so
        // there might not be anything here.
        RexxVariableBase *variable = variables[i].variable;
        if (variable != OREF_NULL)
        {
            // get the corresponding argument
            RexxObject *argument = getArgument(arglist, argcount, i);
            if (argument != OREF_NULL)
            {
                context->traceResult(argument);  // trace if necessary
                // assign the value
                variable->assign(context, stack, argument);
            }
            else
            {
                // grab a potential default value
                RexxObject *defaultValue = variables[i].defaultValue;

                // and omitted argument is only value if we've marked it as optional
                // by giving it a default value
                if (defaultValue != OREF_NULL)
                {
                    // evaluate the default value now
                    defaultValue = defaultValue->evaluate(context, stack);
                    context->traceResult(defaultValue);  // trace if necessary
                    // assign the value
                    variable->assign(context, stack, defaultValue);
                    stack->pop();    // remove the value from the stack
                }
                else
                {
                    // not doing strict checks, revert to old rules and drop the variable.
                    if (!strictChecking)
                    {
                        variable->drop(context);

                    }
                    else
                    {
                        if (context->inMethod())
                        {
                            reportException(Error_Incorrect_method_noarg, i + 1);
                        }
                        else
                        {
                            reportException(Error_Incorrect_call_noarg, context->getCallname(), i + 1);
                        }
                    }
                }
            }
        }
    }
    context->pauseInstruction();    // do debug pause if necessary
}


/**
 * Get the argument corresponding to a given argument position.
 *
 * @param arglist The argument list for the method.
 * @param count   The argument count.
 * @param target  The target argument offset.
 *
 * @return The argument corresponding to the position.  Returns OREF_NULL
 *         if the argument doesn't exist.
 */
RexxObject *RexxInstructionUseStrict::getArgument(RexxObject **arglist, size_t count, size_t target)
{
    // is this beyond what we've been provided with?
    if (target + 1 > count)
    {
        return OREF_NULL;
    }
    // return the target item
    return arglist[target];
}


