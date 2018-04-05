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
/* Primitive Numeric Parse Class                                              */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "NumericInstruction.hpp"
#include "Token.hpp"
#include "MethodArguments.hpp"

/**
 * Constructor for a NUMERIC instruction
 *
 * @param _expression
 *               An expression that needs evaluation for the specific
 *               function.
 * @param flags  A set of flags that drive the execution function.
 */
RexxInstructionNumeric::RexxInstructionNumeric(RexxInternalObject *_expression, FlagSet<NumericInstructionFlags, 32> flags)
{
    expression = _expression;
    numericFlags = flags;
}

/**
 * Execute a NUMERIC instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionNumeric::execute(RexxActivation *context, ExpressionStack *stack )
{
    // trace if necessary
    context->traceInstruction(this);

    // now process the specific functions encoded in the flags

    // NUMERIC DIGITS
    if (numericFlags[numeric_digits])
    {
        // no expression?  Just set digits back to default
        if (expression == OREF_NULL)
        {
            context->setDigits(Numerics::DEFAULT_DIGITS);
        }
        // expression version
        else
        {
            // need to evaluate
            RexxObject *result = expression->evaluate(context, stack);
            context->traceKeywordResult(GlobalNames::DIGITS, result);

            size_t setting;

            // this must be an a positive numeric value
            if (!result->requestUnsignedNumber(setting, number_digits()) || setting < 1)
            {
                reportException(Error_Invalid_whole_number_digits, result);
            }
            // digits cannot be less than or equal to fuzz
            if ((wholenumber_t)setting <= context->fuzz())
            {
                reportException(Error_Expression_result_digits, setting, context->fuzz());
            }
            // set the value
            context->setDigits(setting);
        }
    }
    // NUMERIC FUZZ
    else if (numericFlags[numeric_fuzz])
    {
        // no expression resets to default
        if (expression == OREF_NULL)
        {
            context->setFuzz(Numerics::DEFAULT_FUZZ);
        }
        else
        {
            // get the expression value and convert to a numeric
            RexxObject *result = expression->evaluate(context, stack);
            context->traceKeywordResult(GlobalNames::FUZZ, result);

            size_t setting;
                                           /* bad value?                        */
            if (!result->requestUnsignedNumber(setting, number_digits()))
            {
                reportException(Error_Invalid_whole_number_fuzz, result);
            }

            // cannot be greater than or equal to digits
            if ((wholenumber_t)setting >= context->digits())
            {
                reportException(Error_Expression_result_digits, context->digits(), setting);
            }
            // change the setting
            context->setFuzz(setting);
        }
    }
    // NUMERIC FORM
    else if (numericFlags[numeric_form])
    {
        // NON VALUE form?
        if (expression == OREF_NULL)
        {
            // if default form, set that
            if (numericFlags[numeric_form_default])
            {
                context->setForm(Numerics::DEFAULT_FORM);
            }
            else
            {
                // set it to what was specified.
                context->setForm(numericFlags[numeric_engineering] ? Numerics::FORM_ENGINEERING : Numerics::FORM_SCIENTIFIC);
            }
        }
        else
        {
            // evaluate the expression and get as a string value.
            RexxObject *result = expression->evaluate(context, stack);
            context->traceKeywordResult(GlobalNames::FORM, result);
            RexxString *stringResult = result->requestString();

            //  Scientific form?
            if (stringResult->strCompare(GlobalNames::SCIENTIFIC))
            {
                context->setForm(Numerics::FORM_SCIENTIFIC);
            }
            // Engineering form?
            else if (stringResult->strCompare(GlobalNames::ENGINEERING))
            {
                context->setForm(Numerics::FORM_ENGINEERING);
            }
            else
            {
                reportException(Error_Invalid_subkeyword_form, result);
            }
        }
    }
    context->pauseInstruction();         // do debug pause if necessary
}
