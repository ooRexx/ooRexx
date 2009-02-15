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
#include "ActivityManager.hpp"


/******************************************************************************/
/* Function:   Take in an agument passed to a method, convert it to a length  */
/*               object, verifying that the number is a non-negative value.   */
/*               If the argument is omitted, an error is raised.              */
/******************************************************************************/
stringsize_t lengthArgument(
    RexxObject * argument,             /* input argument                    */
    size_t position )                  /* position of the argument          */
{
    if (argument == OREF_NULL)            /* have a real argument?             */
    {
        missingArgument(position);       /* raise an error                    */
    }
    stringsize_t    value;                /* converted number value            */

    if (!argument->unsignedNumberValue(value))
    {
        /* raise the error                   */
        reportException(Error_Incorrect_method_length, argument);
    }
    return value;
}


/******************************************************************************/
/* Function:   Take in an agument passed to a method, convert it to a position*/
/*               value, verifying that the number is a positive value.        */
/*               If the argument is omitted, an error is raised.              */
/******************************************************************************/
stringsize_t positionArgument(
    RexxObject *argument,              /* input argument                    */
    size_t position )                  /* position of the argument          */
{
    if (argument == OREF_NULL)            /* have a real argument?             */
    {
        missingArgument(position);         /* raise an error                    */
    }
    stringsize_t    value;                /* converted number value            */

    if (!argument->unsignedNumberValue(value) || value == 0)
    {
        /* raise the error                   */
        reportException(Error_Incorrect_method_position, argument);
    }
    return value;
}

/******************************************************************************/
/* Function:   Take in an argument passed to the BIF, convert it to a         */
/*               character, if it exists otherwise return the default         */
/*               character as defined (passed in) by the BIF.                 */
/******************************************************************************/
char padArgument(
    RexxObject *argument,              /* method argument                   */
    size_t position )                  /* argument position                 */
{
    RexxString *parameter = (RexxString *)stringArgument(argument, position);
    /* is the string only 1 character?   */
    if (parameter->getLength() != 1)
    {
        /* argument not good, so raise an    */
        /*error                              */
        reportException(Error_Incorrect_method_pad, argument);
    }
    /* yes, return the character.        */
    return parameter->getChar(0);
}

/******************************************************************************/
/* Function:   Take in an argument passed to the BIF, convert it to a         */
/*               character, if it exists otherwise return the default         */
/*               character as defined (passed in) by the BIF.                 */
/******************************************************************************/
char optionArgument(
    RexxObject *argument,              /* method argument                   */
    size_t position )                  /* argument position                 */
{
    /* force option to string            */
    RexxString *parameter = (RexxString *)stringArgument(argument, position);
    /* return the first character        */
    return toupper(parameter->getChar(0));
}
