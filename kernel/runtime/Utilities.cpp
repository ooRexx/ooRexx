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
#include <ctype.h>
#include <sys/types.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "ArrayClass.hpp"
#include "RexxNativeActivation.hpp"
#include "ProtectedObject.hpp"
#include "Utilities.hpp"

void logic_error (const char *desc)
/******************************************************************************/
/* Function:  Raise a fatal logic error                                       */
/******************************************************************************/
{
  printf("Logic error: %s\n",desc);
  exit(RC_LOGIC_ERROR);
}


wholenumber_t message_number(
    RexxString *errorcode)             /* REXX error code as string         */
/******************************************************************************/
/* Function:  Parse out the error code string into the messagecode valuey     */
/******************************************************************************/
{
  const char *decimalPoint;            /* location of decimalPoint in errorcode*/
  wholenumber_t  primary = 0;          /* Primary part of error code, major */
  wholenumber_t  secondary = 0;        /* Secondary protion (minor code)    */
  wholenumber_t  count;

                                       /* make sure we get errorcode as str */
  errorcode = (RexxString *)errorcode->stringValue();
                                       /* scan to decimal Point or end of   */
                                       /* error code.                       */
  for (decimalPoint = errorcode->getStringData(), count = 0; *decimalPoint && *decimalPoint != '.'; decimalPoint++, count++);

  // must be a whole number in the correct range
  if (!new_string(errorcode->getStringData(), count)->numberValue(primary) || primary < 1 || primary >= 100)
  {
                                       /* Nope raise an error.              */
      reportException(Error_Expression_result_raise);

  }
  // now shift over the decimal position.
  primary *= 1000;


  if (*decimalPoint) {                 /* Was there a decimal point specified?*/
                                       /* is the subcode invalid or too big?*/
    if (!new_string(decimalPoint + 1, errorcode->getLength() - count -1)->numberValue(secondary) || secondary < 0  || secondary >= 1000) {
                                       /* Yes, raise an error.              */
        reportException(Error_Expression_result_raise);
    }
  }
  return primary + secondary;          /* add two portions together, return */
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
    size_t argumentPosition)           /* position of the missing argument  */
/******************************************************************************/
/* Function:  Raise an error for a missing argument, given the target         */
/*            position.                                                       */
/******************************************************************************/
{
                                       /* just raise the error              */
    reportException(Error_Incorrect_method_noarg, argumentPosition);
}

const char *mempbrk(
  const char *String,                  /* search string                     */
  const char *Set,                     /* reference set                     */
  size_t      Length )                 /* size of string                    */
/*********************************************************************/
/*  Function:  Find first occurence of set member in memory          */
/*********************************************************************/
{
  while (Length-- > 0) {               /* search through string             */

    if (strchr(Set, *String)) {        /* find a match in ref set?          */
        return String;
    }
    String++;                          /* step the pointer                  */
  }
  return NULL;                         /* return matched position           */
}


/**
 * Portable implementation of an ascii-z caseless string compare.
 *
 * @param opt1   First string argument
 * @param opt2   Second string argument.
 *
 * @return The compare result.  Returns 0, negative, or positive depending
 *         one the ordering compare result.
 */
int Utilities::strCaselessCompare(const char *op1, const char *op2)
{
    while (tolower(*op1) == tolower(*op2))
    {
        if (*op1 == 0)
        {
            return 0;
        }
        op1++;
        op2++;
    }

    return(tolower(*op1) - tolower(*op2));
}

/**
 * Portable implementation of a caseless memory compare.
 *
 * @param opt1   First memory location to compare.
 * @param opt2   Second memory location.
 * @param len    Length to compare.
 *
 * @return The compare result.  Returns 0, negative, or positive depending
 *         one the ordering compare result.
 */
int Utilities::memicmp(const void *mem1, const void *mem2, size_t len)
{
    const char *op1 = (const char *)mem1;
    const char *op2 = (const char *)mem2;
    while(len != 0)
    {
        if (tolower(*op1) != tolower(*op2))
        {
            return tolower(*op1) - tolower(*op2);

        }
        op1++;
        op2++;
        len--;
    }
    return 0;
}

/**
 * Portable implementation of an ascii-z string to uppercase (in place).
 *
 * @param str    String argument
 *
 * @return The address of the str unput argument.
 */
void Utilities::strupper(char *str)
{
    while (*str)
    {
        *str = toupper(*str);
        str++;
    }

    return;
}


/**
 * Portable implementation of an ascii-z string to uppercase (in place).
 *
 * @param str    String argument
 *
 * @return The address of the str unput argument.
 */
void Utilities::strlower(char *str)
{
    while (*str)
    {
        *str = tolower(*str);
        str++;
    }

    return;
}


/**
 * Bounded strchr() function.
 *
 * @param data   The data pointer.
 * @param n      The maximum length to scan.
 * @param ch     The character of interest.
 *
 * @return The pointer to the located character, or NULL if it isn't found.
 */
const char *Utilities::strnchr(const char *data, size_t n, char ch)
{
    const char *endPtr = data + n;
    while (data < endPtr && *data != '\0')
    {
        if (*data == ch)
        {
            return data;
        }
        data++;
    }
    return NULL;
}
