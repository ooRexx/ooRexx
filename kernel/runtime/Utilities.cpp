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
/****************************************************************************/
/* REXX Kernel                                                  okutil.c    */
/*                                                                          */
/* Utility Functions                                                        */
/*                                                                          */
/****************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivity.hpp"
#include "ArrayClass.hpp"
#include "RexxNativeActivation.hpp"

void logic_error (char *desc)
/******************************************************************************/
/* Function:  Raise a fatal logic error                                       */
/******************************************************************************/
{
  printf("Logic error: %s\n",desc);
  exit(RC_LOGIC_ERROR);
}

void report_halt (                     /* report a halt condition           */
    RexxString *description )          /* condition descriptive information */
/******************************************************************************/
/* Function:   Report a HALT condition, raising an error if not trapped.      */
/******************************************************************************/
{
                                       /* process as common condition       */
  if (!CurrentActivity->raiseCondition(OREF_HALT, OREF_NULL, description, OREF_NULL, OREF_NULL, OREF_NULL))
                                       /* raise as a syntax error           */
    report_exception1(Error_Program_interrupted_condition, OREF_HALT);
}

void report_nomethod(                  /* report a NOMETHOD condition       */
    RexxString *message,               /* message that wasn't found         */
    RexxObject *receiver )             /* object that received the message  */
/******************************************************************************/
/* Function:   Report a NOMETHOD condition, raising an error if not trapped.  */
/******************************************************************************/
{
                                       /* process as common condition       */
  if (!CurrentActivity->raiseCondition(OREF_NOMETHOD, OREF_NULL, message, receiver, OREF_NULL, OREF_NULL))
                                       /* raise as a syntax error           */
    report_exception2(Error_No_method_name, receiver, message);
}

long message_number(
    RexxString *errorcode)             /* REXX error code as string         */
/******************************************************************************/
/* Function:  Parse out the error code string into the messagecode valuey     */
/******************************************************************************/
{
  char *decimalPoint;                  /* location of decimalPoint in errorcode*/
  long primary;                        /* Primary part of error code, major */
  long secondary;                      /* Secondary protion (minor code)    */
  long count;

                                       /* make sure we get errorcode as str */
  errorcode = (RexxString *)errorcode->stringValue();
                                       /* scan to decimal Point or end of   */
                                       /* error code.                       */
  for (decimalPoint = errorcode->stringData, count = 0; *decimalPoint && *decimalPoint != '.'; decimalPoint++, count++);
                                       /* get the primary portion of code   */
  primary = (new_string(errorcode->stringData, count)->longValue(9)) * 1000;
                                       /* did major code compute to long    */
                                       /* and within range                  */
  if (primary == NO_LONG || primary < 1 || primary >= 100000) {
                                       /* Nope raise an error.              */
    report_exception(Error_Expression_result_raise);
  }

  if (*decimalPoint) {                 /* Was there a decimal point specified?*/
                                       /* Yes, compute its decimal value.   */
    secondary = new_string(decimalPoint + 1, errorcode->length - count -1)->longValue(9);
                                       /* is the subcode invalid or too big?*/
    if (secondary == NO_LONG || secondary < 0  || secondary >= 1000) {
                                       /* Yes, raise an error.              */
      report_exception(Error_Expression_result_raise);
    }
  }
  else {
    secondary = 0;
  }
  return primary + secondary;          /* add two protions together, return */
}

void process_new_args(
    RexxObject **arg_array,            /* source argument array             */
    size_t       argCount,             /* size of the argument array        */
    RexxObject***init_args,            /* remainder arguments               */
    size_t      *remainderSize,        /* remaining count of arguments      */
    size_t       required,             /* number of arguments we require    */
    RexxObject **argument1,            /* first returned argument           */
    RexxObject **argument2 )           /* second return argument            */
/******************************************************************************/
/* Function:  Divide up a class new arglist into new arguments and init args  */
/******************************************************************************/
{
  *argument1 = OREF_NULL;              /* clear the first argument          */
  if (argCount >= 1)                   /* have at least one argument?       */
    *argument1 = arg_array[0];         /* get the first argument            */
  if (required == 2) {                 /* processing two arguments?         */
    if (argCount >= 2)                 /* get at least 2?                   */
      *argument2 = arg_array[1];       /* get the second argument           */
    else
      *argument2 = OREF_NULL;          /* clear the second argument         */
  }
                                       /* get the init args part            */
  *init_args = arg_array + required;
  /* if we have at least the required arguments, reduce the count. */
  /* Otherwise, set this to zero. */
  if (argCount >= required) {
      *remainderSize = argCount - required;
  }
  else {
      *remainderSize = 0;
  }
}

void missing_argument(
    LONG  argumentPosition)            /* position of the missing argument  */
/******************************************************************************/
/* Function:  Raise an error for a missing argument, given the target         */
/*            position.                                                       */
/******************************************************************************/
{
                                       /* just raise the error              */
  report_exception1(Error_Incorrect_method_noarg, new_integer(argumentPosition));
}

INT  CaselessCompare(                  /* do a caseless memory comparison   */
     PUCHAR    string1,                /* first compare string              */
     PUCHAR    string2,                /* second compare string             */
     LONG      length)                 /* comparison length                 */
/******************************************************************************/
/* Function:  Compare two strings, ignoring differences in case               */
/******************************************************************************/
{
                                       /* totally equal?                    */
  if (!memcmp((PCHAR)string1, (PCHAR)string2, length))
    return 0;                          /* return equality indicator         */

  while (length--) {                   /* need to do it the hardway         */
                                       /* not equal?                        */
    if (toupper(*string1) != toupper(*string2)) {
                                       /* first one less?                   */
      if (toupper(*string1) < toupper(*string2))
        return -1;                     /* return less than indicator        */
      else
        return 1;                      /* first is larger                   */
    }
    string1++;                         /* step first pointer                */
    string2++;                         /* and second pointer also           */
  }
  return 0;                            /* fall through, these are equal     */
}

PCHAR mempbrk(
  PCHAR     String,                    /* search string                     */
  PCHAR     Set,                       /* reference set                     */
  LONG      Length )                   /* size of string                    */
/*********************************************************************/
/*  Function:  Find first occurence of set member in memory          */
/*********************************************************************/
{
  PCHAR    Retval;                     /* returned value                    */

  Retval = NULL;                       /* nothing found yet                 */
  while (Length-- > 0) {               /* search through string             */

    if (strchr(Set, *String)) {        /* find a match in ref set?          */
      Retval = String;                 /* copy position                     */
      break;                           /* quit the loop                     */
    }
    String++;                          /* step the pointer                  */
  }
  return Retval;                       /* return matched position           */
}

RexxObject *native_release(
    RexxObject *result )               /* potential return value            */
/***************************************************************/
/* Function:  Release kernel access, providing locking on      */
/*            the return value, if any                         */
/***************************************************************/
{
  RexxNativeActivation *activation;    /* current activation                */

  if (result != OREF_NULL) {           /* only save real references!        */
                                       /* get the current activation        */
    activation = (RexxNativeActivation *)CurrentActivity->current();
                                       /* protect the result                */
    save(result);                      /* protect from GC while saving!     */
    result = activation->saveObject(result);
    discard(result);
  }
  ReleaseKernelAccess(CurrentActivity);/* release the kernel lock           */
  return result;                       /* return the result object          */
}
