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
/* REXX Kernel                                                  okbutil.c     */
/*                                                                            */
/* Word-related REXX string method utility functions                          */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxBuiltinFunctions.h"                     /* include BIF util prototype/macros */
                                       /* current global settings           */
extern ACTIVATION_SETTINGS *current_settings;

/******************************************************************************/
/* Function:   Take in an agument passed to a method, convert it to a length  */
/*               object, verifying that the number is a non-negative value.   */
/*               If the argument is omitted, an error is raised.              */
/******************************************************************************/
size_t get_length(
    RexxObject * argument,             /* input argument                    */
    size_t position )                  /* position of the argument          */
{
 LONG   value;                         /* converted number value            */

 if (argument == OREF_NULL)            /* have a real argument?             */
   missing_argument(position);         /* raise an error                    */
 if (OTYPE(Integer, argument))         /* already an integer?               */
                                       /* get this directly                 */
   value = ((RexxInteger *)argument)->value;
 else
                                       /* convert the length                */
   value = REQUIRED_LONG(argument, DEFAULT_DIGITS, position);
 if (value < 0)                        /* not a good length argument?       */
                                       /* this is an error                  */
   report_exception1(Error_Incorrect_method_length, argument);
 return (size_t)value;                 /* return converted value            */
}

/******************************************************************************/
/* Function:   Take in an agument passed to a method, convert it to a position*/
/*               value, verifying that the number is a positive value.        */
/*               If the argument is omitted, an error is raised.              */
/******************************************************************************/
size_t get_position(
    RexxObject *argument,              /* input argument                    */
    size_t position )                  /* position of the argument          */
{
 LONG   value;                         /* converted number value            */

 if (argument == OREF_NULL)            /* have a real argument?             */
   missing_argument(position);         /* raise an error                    */
 if (OTYPE(Integer, argument))         /* already an integer?               */
                                       /* get this directly                 */
   value = ((RexxInteger *)argument)->value;
 else
                                       /* convert the length                */
   value = REQUIRED_LONG(argument, DEFAULT_DIGITS, position);
 if (value <= 0)                       /* not a good position argument?     */
                                       /* this is an error                  */
   report_exception1(Error_Incorrect_method_position, argument);
 return (size_t)value;                 /* return converted value            */
}

/******************************************************************************/
/* Function:   Take in an argument passed to the BIF, convert it to a         */
/*               character, if it exists otherwise return the default         */
/*               character as defined (passed in) by the BIF.                 */
/******************************************************************************/
char get_pad_character(
    RexxObject *argument,              /* method argument                   */
    size_t position )                  /* argument position                 */
{
 RexxString *parameter;                /* converted string argument         */
                                       /* convert parameter to a string     */
 parameter = (RexxString *)REQUIRED_STRING(argument, position);
                                       /* is the string only 1 character?   */
 if (parameter->length != 1)
                                       /* argument not good, so raise an    */
                                       /*error                              */
   report_exception1(Error_Incorrect_method_pad, argument);
                                       /* yes, return the character.        */
 return (CHAR)*(parameter->stringData);
}

/******************************************************************************/
/* Function:   Take in an argument passed to the BIF, convert it to a         */
/*               character, if it exists otherwise return the default         */
/*               character as defined (passed in) by the BIF.                 */
/******************************************************************************/
CHAR get_option_character(
    RexxObject *argument,              /* method argument                   */
    size_t position )                  /* argument position                 */
{
 RexxString *parameter;                /* converted string argument         */

                                       /* force option to string            */
 parameter = (RexxString *)REQUIRED_STRING(argument, position);
                                       /* return the first character        */
 return toupper(*parameter->stringData);
}
