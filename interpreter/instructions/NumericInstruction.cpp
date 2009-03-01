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
/* Primitive Numeric Parse Class                                              */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "NumericInstruction.hpp"
#include "Token.hpp"

RexxInstructionNumeric::RexxInstructionNumeric(
    RexxObject *_expression,           /* optional expression               */
    unsigned short type,               /* type of numeric instruction       */
    size_t      flags)                 /* processing flags                  */
/****************************************************************************/
/* Function:  Execute a REXX NUMERIC instruction                            */
/****************************************************************************/
{
                                       /* copy the expression               */
  OrefSet(this, this->expression, _expression);
  instructionFlags = (uint16_t)flags;
  switch (type)
  {
      case SUBKEY_DIGITS:
          instructionFlags |= numeric_digits;
          break;
      case SUBKEY_FUZZ:
          instructionFlags |= numeric_fuzz;
          break;
      case SUBKEY_FORM:
          instructionFlags |= numeric_form;
          break;
  }
}

void RexxInstructionNumeric::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/****************************************************************************/
/* Function:  Execute a REXX LEAVE instruction                              */
/****************************************************************************/
{
    RexxObject  *result;                 /* expression evaluation result      */
    RexxString  *stringResult;           /* converted string                  */
    stringsize_t setting;                /* binary form of the setting        */

    context->traceInstruction(this);     /* trace if necessary                */
                                         /* process the different types of    */
    switch (instructionFlags & numeric_type_mask)           /* numeric instruction               */
    {
        case numeric_digits:               /* NUMERIC DIGITS instruction        */
            /* resetting to default digits?      */
            if (this->expression == OREF_NULL)
            {
                /* just set it to the default        */
                context->setDigits();
            }
            else                           /* need to evaluate an expression    */
            {
                /* get the expression value          */
                result = this->expression->evaluate(context, stack);
                context->traceResult(result);  /* trace if necessary                */
                                               /* bad value?                        */
                if (!result->requestUnsignedNumber(setting, number_digits()) || setting < 1)
                {
                    /* report an exception               */
                    reportException(Error_Invalid_whole_number_digits, result);
                }
                /* problem with the fuzz setting?    */
                if (setting <= context->fuzz())
                {
                    /* this is an error                  */
                    reportException(Error_Expression_result_digits, setting, context->fuzz());
                }
                context->setDigits(setting);   /* now adjust the setting            */
            }
            break;

        case numeric_fuzz:                 /* NUMERIC FUZZ instruction          */
            /* resetting to default fuzz?        */
            if (this->expression == OREF_NULL)
            {
                context->setFuzz();        /* just set it to the default        */
            }
            else                           /* need to evaluate an expression    */
            {
                /* get the expression value          */
                result = this->expression->evaluate(context, stack);
                context->traceResult(result);  /* trace if necessary                */
                                               /* bad value?                        */
                if (!result->requestUnsignedNumber(setting, number_digits()))
                {
                    /* report an exception               */
                    reportException(Error_Invalid_whole_number_fuzz, result);
                }
                /* problem with the digits setting?  */
                if (setting >= context->digits())
                {
                    /* and issue the error               */
                    reportException(Error_Expression_result_digits, context->digits(), setting);
                }
                context->setFuzz(setting);     /* set the new value                 */
            }
            break;

        case numeric_form:                 /* NUMERIC FORM instruction          */
            /* non-VALUE form?                   */
            if (this->expression == OREF_NULL)
            {
                // if default form, set that
                if (instructionFlags&numeric_form_default)
                {
                    context->setForm();
                }
                else
                {

                    // set it to what was specified.
                    context->setForm(instructionFlags&numeric_engineering ? Numerics::FORM_ENGINEERING : Numerics::FORM_SCIENTIFIC);
                }
            }
            else                           /* need to evaluate an expression    */
            {
                /* get the expression value          */
                result = this->expression->evaluate(context, stack);
                /* get the string version            */
                stringResult = REQUEST_STRING(result);
                /* trace if necessary                */
                context->traceResult(stringResult);
                /* Scientific form?                  */
                if (stringResult->strCompare(CHAR_SCIENTIFIC))
                {
                    /* set the proper form               */
                    context->setForm(Numerics::FORM_SCIENTIFIC);
                }
                /* Scientific form?                  */
                else if (stringResult->strCompare(CHAR_ENGINEERING))
                {
                    /* set the engineering form          */
                    context->setForm(Numerics::FORM_ENGINEERING);
                }
                else
                {
                    /* report an exception               */
                    reportException(Error_Invalid_subkeyword_form, result);
                }
            }
            break;
    }
    context->pauseInstruction();         /* do debug pause if necessary       */
}
