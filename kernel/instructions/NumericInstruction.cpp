/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* REXX Translator                                              NumericInstruction.c      */
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
    RexxObject *expression,            /* optional expression               */
    USHORT      type,                  /* type of numeric instruction       */
    UCHAR       flags)                 /* processing flags                  */
/****************************************************************************/
/* Function:  Execute a REXX NUMERIC instruction                            */
/****************************************************************************/
{
                                       /* copy the expression               */
  OrefSet(this, this->expression, expression);
  numeric_type = type;                 /* the type                          */
  i_flags = flags;                     /* and the flag settings             */
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
  LONG         setting;                /* binary form of the setting        */
  LONG         tempVal;                /* temporary value for errors        */

  context->traceInstruction(this);     /* trace if necessary                */
                                       /* process the different types of    */
  switch (numeric_type) {              /* numeric instruction               */

    case SUBKEY_DIGITS:                /* NUMERIC DIGITS instruction        */
                                       /* resetting to default digits?      */
      if (this->expression == OREF_NULL)
                                       /* just set it to the default        */
        context->setDigits(DEFAULT_DIGITS);
      else {                           /* need to evaluate an expression    */
                                       /* get the expression value          */
        result = this->expression->evaluate(context, stack);
        context->traceResult(result);  /* trace if necessary                */
                                       /* convert the value                 */
        setting = REQUEST_LONG(result, NO_LONG);
                                       /* bad value?                        */
        if (setting == NO_LONG || setting < 1)
                                       /* report an exception               */
          report_exception1(Error_Invalid_whole_number_digits, result);
                                       /* problem with the fuzz setting?    */
        if (setting <= context->fuzz()) {
          tempVal = context->fuzz();   /* get the value                     */
                                       /* this is an error                  */
          report_exception2(Error_Expression_result_digits, new_integer(setting), new_integer(tempVal));
        }
        context->setDigits(setting);   /* now adjust the setting            */
      }
      break;

    case SUBKEY_FUZZ:                  /* NUMERIC FUZZ instruction          */
                                       /* resetting to default fuzz?        */
      if (this->expression == OREF_NULL)
        context->setFuzz(DEFAULT_FUZZ);/* just set it to the default        */
      else {                           /* need to evaluate an expression    */
                                       /* get the expression value          */
        result = this->expression->evaluate(context, stack);
        context->traceResult(result);  /* trace if necessary                */
                                       /* convert the value                 */
        setting = REQUEST_LONG(result, NO_LONG);
                                       /* bad value?                        */
        if (setting == NO_LONG || setting < 0)
                                       /* report an exception               */
          report_exception1(Error_Invalid_whole_number_fuzz, result);
                                       /* problem with the digits setting?  */
        if (setting >= context->digits()) {
          tempVal = context->digits(); /* Get the value                     */
                                       /* and issue the error               */
          report_exception2(Error_Expression_result_digits, new_integer(tempVal), new_integer(setting));
        }
        context->setFuzz(setting);     /* set the new value                 */
      }
      break;

    case SUBKEY_FORM:                  /* NUMERIC FORM instruction          */
                                       /* non-VALUE form?                   */
      if (this->expression == OREF_NULL)
                                       /* just set it to the default        */
        context->setForm(i_flags&numeric_engineering ? FORM_ENGINEERING : FORM_SCIENTIFIC);
      else {                           /* need to evaluate an expression    */
                                       /* get the expression value          */
        result = this->expression->evaluate(context, stack);
                                       /* get the string version            */
        stringResult = REQUEST_STRING(result);
                                       /* trace if necessary                */
        context->traceResult(stringResult);
                                       /* Scientific form?                  */
        if (stringResult->strCompare(CHAR_SCIENTIFIC))
                                       /* set the proper form               */
          context->setForm(FORM_SCIENTIFIC);
                                       /* Scientific form?                  */
        else if (stringResult->strCompare(CHAR_ENGINEERING))
                                       /* set the engineering form          */
          context->setForm(FORM_ENGINEERING);
        else
                                       /* report an exception               */
          report_exception1(Error_Invalid_subkeyword_form, result);
      }
      break;
  }
  context->pauseInstruction();         /* do debug pause if necessary       */
}
