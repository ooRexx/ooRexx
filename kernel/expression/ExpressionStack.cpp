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
/* REXX Kernel                                                  ExpressionStack.c    */
/*                                                                            */
/* Primitive Expression Stack Class                                           */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ExpressionStack.hpp"

void RexxExpressionStack::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
   RexxObject **entry;                 /* marked stack entry                */

  setUpMemoryMark
                                       /* loop through the stack entries    */
   for (entry = this->stack; entry <= this->top; entry++)
     memory_mark(*entry);              /* marking each one                  */
  cleanUpMemoryMark
}

void RexxExpressionStack::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
   RexxObject **entry;                 /* marked stack entry                */

   setUpMemoryMarkGeneral
                                       /* loop through the stack entries    */
   for (entry = this->stack; entry <= this->top; entry++)
     memory_mark_general(*entry);      /* marking each one                  */
   cleanUpMemoryMarkGeneral
}

void RexxExpressionStack::flatten(RexxEnvelope * envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxExpressionStack)

  long i;                              /* pointer for scanning stack entries*/
  long count;                          /* number of elements                */

  count = this->top - this->stack;     /* get the total count               */
                                       /* loop through the stack entries    */
   for (i = 0; i < count; i++)
     flatten_reference(newThis->stack[i], envelope);

  cleanUpFlatten
}

void RexxExpressionStack::migrate(RexxActivity *activity)
/******************************************************************************/
/* Function:  Migrate the expression stack to a new activity                  */
/******************************************************************************/
{
    RexxObject **oldFrame = stack;
    /* allocate a new frame */
    activity->allocateStackFrame(this, size);
    /* copy the enties over to the new stack. */
    memcpy(stack, oldFrame, sizeof(RexxObject *) * size);
}

void RexxExpressionStack::expandArgs(
     INT    argcount,                  /* count of arguments                */
     INT    min,                       /* minimum required arguments        */
     INT    max,                       /* maximum required arguments        */
     PCHAR  function )                 /* function being processed          */
/******************************************************************************/
/* Function:  Verify that a function has received all of its required         */
/*            arguments, and did not receive extras.                          */
/******************************************************************************/
{
  INT          i;                      /* loop counter                      */
  LONG         j;
  RexxObject **current;                /* pointer to the current stack item */

  if (argcount < min)                  /* too few arguments?                */
                                       /* report an error                   */
    report_exception2(Error_Incorrect_call_minarg, new_cstring(function), new_integer(min));
  else if (argcount > max)             /* too many arguments?               */
                                       /* report an error                   */
    report_exception2(Error_Incorrect_call_maxarg, new_cstring(function), new_integer(max));
  else {                               /* need to expand number of args     */
                                       /* address the stack elements        */
    current = this->pointer(argcount - 1);
    for (i = min; i; i--) {            /* check on required arguments       */
      if (*current++ == OREF_NULL) {   /* omitted argument?                 */
        j = min - i + 1;               /* argument location                 */
                                       /* missing required argument         */
        report_exception2(Error_Incorrect_call_noarg, new_cstring(function), new_integer(j));
      }
    }
  }
}

RexxString *RexxExpressionStack::requiredStringArg(
    INT    position)                   /* position of argument              */
/******************************************************************************/
/* Function:  Retrieve an object from the expression stack and potentially    */
/*            convert it into a string argument.                              */
/******************************************************************************/
{
  RexxObject *argument;                /* processed argument                */
  RexxString *newStr;                  /* converted value                   */

  argument = this->peek(position);     /* get the argument in question      */
  if (OTYPE(String, argument))         /* string object already?            */
    return (RexxString *)argument;     /* finished                          */
                                       /* get the string form, raising a    */
                                       /* NOSTRING condition if necessary   */
  newStr = argument->requestString();
  this->replace(position, newStr);     /* replace the argument              */
  return newStr;                       /* return the replacement value      */
}

RexxString *RexxExpressionStack::optionalStringArg(
    INT    position)                   /* position of argument              */
/******************************************************************************/
/* Function:  Retrieve an object from the expression stack and potentially    */
/*            convert it into a string argument.                              */
/******************************************************************************/
{
  RexxObject *argument;                /* processed argument                */
  RexxString *newStr;                  /* converted value                   */

  argument = this->peek(position);     /* get the argument in question      */
  if (argument == OREF_NULL)           /* missing a required argument?      */
    return OREF_NULL;                  /* finished already                  */
  if (OTYPE(String, argument))         /* string object already?            */
    return (RexxString *)argument;     /* finished                          */
                                       /* get the string form, raising a    */
                                       /* NOSTRING condition if necessary   */
  newStr = argument->requestString();
  this->replace(position, newStr);     /* replace the argument              */
  return newStr;                       /* return the replacement value      */
}

RexxInteger *RexxExpressionStack::requiredIntegerArg(
     int    position,                  /* position of argument              */
     int    argcount,                  /* count of arguments                */
     PCHAR  function)                  /* function being processed          */
/******************************************************************************/
/* Function:  Retrieve an object from the expression stack and potentially    */
/*            convert it into an integer argument.                            */
/******************************************************************************/
{
  RexxObject   *argument;              /* processed argument                */
  RexxInteger  *newInt;                /* converted value                   */
  LONG          long_value;            /* converted long value              */
  LONG          tempCount;             /* actual function argument position */

  argument = this->peek(position);     /* get the argument in question      */
  if (OTYPE(Integer, argument))        /* integer object already?           */
    return (RexxInteger *)argument;    /* finished                          */
                                       /* return the string form of argument*/
  long_value = REQUEST_LONG(argument, DEFAULT_DIGITS);
  if (long_value == NO_LONG) {         /* not convertable?                  */
    tempCount = argcount - position;   /* get the actual argument number    */
                                       /* report an exception               */
    report_exception3(Error_Incorrect_call_whole, new_cstring(function), new_integer(tempCount), argument);
  }
  newInt = new_integer(long_value);    /* create an integer object          */
  this->replace(position, newInt);     /* replace the argument              */
  return newInt;                       /* return the replacement value      */
}

RexxInteger *RexxExpressionStack::optionalIntegerArg(
     int    position,                  /* position of argument              */
     int    argcount,                  /* count of arguments                */
     PCHAR  function)                  /* function being processed          */
/******************************************************************************/
/* Function:  Retrieve an object from the expression stack and potentially    */
/*            convert it into an integer argument.                            */
/******************************************************************************/
{
  RexxObject   *argument;              /* processed argument                */
  RexxInteger  *newInt;                /* converted value                   */
  LONG          long_value;            /* converted long value              */
  LONG          tempCount;             /* actual function argument position */

  argument = this->peek(position);     /* get the argument in question      */
  if (argument == OREF_NULL)           /* missing an optional argument?     */
    return OREF_NULL;                  /* nothing there                     */
  if (OTYPE(Integer, argument))        /* integer object already?           */
    return (RexxInteger *)argument;    /* finished                          */
                                       /* return the string form of argument*/
  long_value = REQUEST_LONG(argument, DEFAULT_DIGITS);
  if (long_value == NO_LONG) {         /* not convertable?                  */
    tempCount = argcount - position;   /* get the actual argument number    */
                                       /* report an exception               */
    report_exception3(Error_Incorrect_call_whole, new_cstring(function), new_integer(tempCount), argument);
  }
  newInt = new_integer(long_value);    /* create an integer object          */
  this->replace(position, newInt);     /* replace the argument              */
  return newInt;                       /* return the replacement value      */
}

