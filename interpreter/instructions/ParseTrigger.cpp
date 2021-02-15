/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2017 Rexx Language Association. All rights reserved.    */
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
/* REXX Translator                                          ParseTrigger.cpp  */
/*                                                                            */
/* PARSE instruction trigger operation.                                       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"
#include "ParseTrigger.hpp"
#include "ParseTarget.hpp"
#include "ExpressionBaseVariable.hpp"
#include "MethodArguments.hpp"

/**
 * Initialize a parsing trigger object.
 *
 * @param size   The base size of this object.
 * @param variableCount
 *               The number of variables associated with this trigger.
 *
 * @return A newly allocated Rexx object.
 */
void  *ParseTrigger::operator new(size_t size, size_t variableCount)
{
    return new_object(size + (variableCount - 1) * sizeof(RexxObject *), T_ParseTrigger);
}

/**
 * Construct a PARSING trigger operation.
 *
 * @param type       The type of trigger involved.
 * @param _value     A potential value expression for resolving this
 *                   trigger.
 * @param _variableCount
 *                   The count of variables in this parse section.
 * @param _variables The list of variables to assign.
 */
ParseTrigger::ParseTrigger(ParseTriggerType type, RexxInternalObject *_value, size_t _variableCount,
    QueueClass  *_variables)
{
    triggerType = type;
    variableCount = _variableCount;
    value = _value;
    // now copy any arguments from the sub term stack
    // NOTE:  The arguments are in last-to-first order on the stack.
    initializeObjectArray(_variableCount, variables, RexxVariableBase, _variables);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void ParseTrigger::live(size_t liveMark)
{
    memory_mark(value);
    memory_mark_array(variableCount, variables);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void ParseTrigger::liveGeneral(MarkReason reason)
{
    memory_mark_general(value);
    memory_mark_general_array(variableCount, variables);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void ParseTrigger::flatten(Envelope *envelope)
{
    setUpFlatten(ParseTrigger)

    flattenRef(value);
    flattenArrayRefs(variableCount, variables);

    cleanUpFlatten
}


/**
 * Convert a trigger value to an unsigned integer with
 * appropriate error reporting.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 *
 * @return The converted integer value.
 */
size_t ParseTrigger::integerTrigger(RexxActivation *context, ExpressionStack *stack)
{
    // if we have a value that requires evaluation, get the value and trace.
    RexxObject *trigger = value->evaluate(context, stack);
    context->traceResult(trigger);

    // NOTE:  We leave this on the stack to protect from GC until after we convert.

    size_t result;
    // try to convert to an unsigned number...report an error if this failed.
    if (!trigger->requestUnsignedNumber(result, number_digits()))
    {
        reportException(Error_Invalid_whole_number_parse, trigger);
    }
    // once we have converted this value, we're done with it.  It no longer
    // requires protection.
    stack->pop();
    return result;
}


/**
 * Ensure that a string trigger is an actual string object.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 *
 * @return A true string version of the object.
 */
RexxString *ParseTrigger::stringTrigger(RexxActivation *context, ExpressionStack *stack)
{
    // if we have a value that requires evaluation, get the value and trace.
    RexxObject *trigger = value->evaluate(context, stack);
    context->traceResult(trigger);

    // NOTE:  We leave this on the stack to protect from GC until after we are finished with this.
    // trigger operations that require a string value need to pop this from the stack
    // once they are finished.
   return trigger->requestString();
}


/**
 * Apply a parsing trigger against a parsing context.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 * @param target  The current parsing context.
 */
void ParseTrigger::parse(RexxActivation *context, ExpressionStack *stack,
    RexxTarget *target )
{
    // perform the trigger operaitons
    switch (triggerType)
    {
        // move to the end of the pattern
        case TRIGGER_END:
            target->moveToEnd();
            break;

        // a positive relative movement.
        case TRIGGER_PLUS:
            target->forward(integerTrigger(context, stack));
            break;

        // negative relative movement
        case TRIGGER_MINUS:
            target->backward(integerTrigger(context, stack));
            break;

        // positive relative movement with length semantics (>n)
        case TRIGGER_PLUS_LENGTH:
            target->forwardLength(integerTrigger(context, stack));
            break;

        // negative relative movement with length semantics (<n)
        case TRIGGER_MINUS_LENGTH:
            target->backwardLength(integerTrigger(context, stack));
            break;

        // absolute string positioning
        case TRIGGER_ABSOLUTE:
            target->absolute(integerTrigger(context, stack));
            break;

        // string search
        case TRIGGER_STRING:
            target->search(stringTrigger(context, stack));
            // the string trigger was protected on the stack.  Remove it now
            stack->pop();
            break;

        // caseless string search
        case TRIGGER_MIXED:
            target->caselessSearch(stringTrigger(context, stack));
            // the string trigger was protected on the stack.  Remove it now
            stack->pop();
            break;

        // invalid PARSE trigger type (should really never happen)
        default:
            reportException(Error_Interpretation_switch, "PARSE trigger type", triggerType);
            break;
    }

    // if we are tracing, we need to display each assignment result.  We
    // have two copies of this loop, one optimized for untraced operation and
    // a second one for traced operation.
    if (context->tracingResults())
    {
        for (size_t i = 0; i < variableCount; i++)
        {
            RexxString *variableValue;

            // if this is the last variable on the list, this gets the
            // remainder of the parsing segment.  Otherwise, we just
            // extract the next blank delimited word.
            if (i + 1 == variableCount)
            {
                variableValue = target->remainder();
            }
            else
            {
                variableValue = target->getWord();
            }
            // needs protecting
            ProtectedObject p(variableValue);
            // get the next variable
            RexxVariableBase *variable = variables[i];
            // the '.' dummy placeholder shows up as a NULL value in the list.
            // the dummy placeholder has a special trace form.
            if (variable != OREF_NULL)
            {
                // NOTE:  The different variable types handle their own assignment tracing
                variable->assign(context, variableValue);
                // if only tracing results and not intermediates, then we need to
                // trace this value explicitly.
                if (!context->tracingIntermediates())
                {
                    context->traceResult(variableValue);
                }
            }
            // this is the dummy variable
            else
            {
                context->traceIntermediate(variableValue, RexxActivation::TRACE_PREFIX_DUMMY);
            }
        }
    }
    // not tracing...this version is a bit more optimized.
    else
    {
        for (size_t i = 0; i < variableCount; i++)
        {
            // get the next retriever
            RexxVariableBase *variable = variables[i];
            // if we have a real variable (not a .), extract the string piece and assign.
            if (variable != OREF_NULL)
            {
                RexxObject *variableValue;
                // if this is the last variable in the list, grab the remainder,
                // otherwise, we need to parse word off.
                if (i + 1 == variableCount)
                {
                    variableValue = target->remainder();
                }
                else
                {
                    variableValue = target->getWord();
                }
                // needs protecting if the assignment is a compound var or a message
                // target.
                ProtectedObject p(variableValue);
                // do the assignment                 */
                variable->assign(context, variableValue);
            }
            // dummy variable, we just skip the assignment
            else
            {
                // we need to figure out if we're skipping a word, or skipping everything.
                if (i + 1 == variableCount)
                {
                    target->skipRemainder();
                }
                else
                {
                    target->skipWord();
                }
            }
        }
    }
}

