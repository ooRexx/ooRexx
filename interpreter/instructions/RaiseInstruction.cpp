/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2023 Rexx Language Association. All rights reserved.    */
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
/* Primitive Raise Parse Class                                                */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"
#include "RaiseInstruction.hpp"
#include "Interpreter.hpp"
#include "MethodArguments.hpp"


/**
 * Construct a RAISE instruction object.
 *
 * @param _condition The condition name.
 * @param _expression
 *                   The RC expression.
 * @param _description
 *                   An optional description string.
 * @param _additional
 *                   An expression to resolve additional information.
 * @param _result    A result value to return when the context exits.
 * @param _arrayCount
 *                   A count of items specified with the ARRAY() option.
 * @param array      The subTerm queue holding the ARRAY() option expressions.
 * @param flags      Additional control flags.
 */
RexxInstructionRaise::RexxInstructionRaise(RexxString *_condition, RexxInternalObject *_expression,
    RexxInternalObject *_description, RexxInternalObject *_additional, RexxInternalObject *_result,
    FlagSet<RaiseInstructionFlags, 32> flags)
{
    // just copy the argument information
    conditionName = _condition;
    rcValue = _expression;
    description = _description;
    resultValue = _result;
    instructionFlags = flags;
    // is this the array form?  We need to copy the expressions
    if (flags[raise_array])
    {
        ArrayClass *arrayItems = (ArrayClass *)_additional;

        arrayCount = arrayItems->size();
        // copy each of the argument expressions
        for (size_t i = 0; i < arrayCount; i++)
        {
            additional[i] = (RexxObject *)arrayItems->get(i + 1);
        }
    }
    // store the expression for retrieving the additional item from the
    // first additional slot.
    else
    {
        // we have just one item
        additional[0] = _additional;
        arrayCount = 1;
    }
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionRaise::live(size_t liveMark)
{
    memory_mark(nextInstruction);  /* must be first one marked          */
    memory_mark(conditionName);
    memory_mark(rcValue);
    memory_mark(description);
    memory_mark(resultValue);
    memory_mark_array(arrayCount, additional);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionRaise::liveGeneral(MarkReason reason)
{
    // this must be the first one marked.
    memory_mark_general(nextInstruction);
    memory_mark_general(conditionName);
    memory_mark_general(rcValue);
    memory_mark_general(description);
    memory_mark_general(resultValue);
    memory_mark_general_array(arrayCount, additional);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionRaise::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionRaise)

    flattenRef(nextInstruction);
    flattenRef(conditionName);
    flattenRef(rcValue);
    flattenRef(description);
    flattenRef(resultValue);
    flattenArrayRefs(arrayCount, additional);

    cleanUpFlatten
}

/**
 * Execute a RAISE instruction.
 *
 * @param context The current execution context.
 * @param stack   The current context evaluation stack.
 */
void RexxInstructionRaise::execute(RexxActivation *context, ExpressionStack *stack)
{
    // trace if needed
    context->traceInstruction(this);

    // set defaults for anything we need to evaluate.
    RexxObject *_additional = OREF_NULL;
    RexxString *_description = OREF_NULL;
    RexxObject *rc = OREF_NULL;
    RexxObject *_result = OREF_NULL;

    // and start evaluating
    // extra RC information (SYNTAX, ERROR, and FAILURE only)
    if (rcValue != OREF_NULL)
    {
                                         /* get the expression value          */
        rc = rcValue->evaluate(context, stack);
        // this is traced using the condition name as the keyword
        context->traceKeywordResult(conditionName, rc);
    }
    // syntax conditions have some special requirements, so process those
    // up front.
    if (instructionFlags[raise_syntax])
    {
        // give this a default additional information of an empty array
        _additional = new_array();
        // The description is a null string
        _description = GlobalNames::NULLSTRING;
        // the RC must have a string value...this is an error if it doesn.
        RexxString *errorcode = rc->requestString();
        if (errorcode == TheNilObject)
        {
            reportException(Error_Conversion_raise, rc);
        }

        // convert this to decimal, then create an integer object
        // that we replace the input rc value with
        wholenumber_t msgNum = Interpreter::messageNumber(errorcode);
        rc = new_integer(msgNum);
    }

    // Reasonable defaults are set up, now see if we have explicit things given
    if (description != OREF_NULL)
    {
        _description = (RexxString *)description->evaluate(context, stack);
        context->traceKeywordResult(GlobalNames::DESCRIPTION, _description);
    }

    // is this the ARRAY form of passing information?
    if (instructionFlags[raise_array])
    {
        // we need to build an array of the additional information
        size_t count = arrayCount;
        _additional = new_array(count);
        // push this on the eval stack for safekeeping
        stack->push(_additional);
        for (size_t i = 0; i < count; i++)
        {
            // we can have ommitted ones here, so only try to evaluate the
            // ones that have been specified
            if (additional[i] != OREF_NULL)
            {
                // trace each of these as arguments
                RexxObject *arg = (additional[i])->evaluate(context, stack);
                // trace each of these as arguments
                context->traceArgument(arg);
                ((ArrayClass *)_additional)->put(arg, i + 1);
                // trace each of these as arguments
                context->traceArgument(arg);
            }
            else
            {
                // just trace a null argument
                context->traceArgument(GlobalNames::NULLSTRING);
            }
        }
        context->traceKeywordResult(GlobalNames::ARRAY, _additional);
    }
    // we might have had this via the ADDITIONAL() option.
    else if (this->additional[0] != OREF_NULL)
    {
        // get this expression value
        _additional = additional[0]->evaluate(context, stack);
        // trace what we got
        context->traceKeywordResult(GlobalNames::ADDITIONAL, _additional);
    }
    // given a return result value to pass back to the caller?
    if (resultValue != OREF_NULL)
    {
        _result = resultValue->evaluate(context, stack);
        context->traceKeywordResult(GlobalNames::RESULT, _result);
    }

    // set a default condition object
    DirectoryClass *conditionobj = OREF_NULL;
    // propagating an existing condition?
    if (instructionFlags[raise_propagate])
    {
        conditionobj = context->getConditionObj();
        if (conditionobj == OREF_NULL)     /* no current active condition?      */
        {
            reportException(Error_Execution_propagate);
        }
    }

    // if we have additional information, fill in a few more things
    if (_additional != OREF_NULL)
    {
        // we may need some additional checks based on the syntax
        // condition.  Since this might come from a PROPAGATE, we
        // have to rely on the string name of the condition.
        RexxString *errorCode = conditionName;
        // if this is a propagate, get CONDITION name from the condition object.
        if (instructionFlags[raise_propagate])
        {
            errorCode = (RexxString *)conditionobj->get(GlobalNames::CONDITION);
        }
        // If this is a SYNTAX condition, than the Additional information MUST
        // be an array of items used for substitutions.
        if (errorCode->strCompare(GlobalNames::SYNTAX))
        {
            // get the array version, and it must be single dimension.
            _additional = _additional->requestArray();
            if (_additional == TheNilObject || ((ArrayClass *)_additional)->isMultiDimensional())
            {
                reportException(Error_Execution_syntax_additional);
            }
        }
    }

    // we have two forms, both of which are handled by context
    // raise return is processed as a return instruction
    if (instructionFlags[raise_return])
    {
        context->raise(conditionName, rc, _description, _additional, _result, conditionobj);
    }
    // the default is handled like an EXIT
    else
    {
        context->raiseExit(conditionName, rc, _description, _additional, _result, conditionobj);
    }
}

