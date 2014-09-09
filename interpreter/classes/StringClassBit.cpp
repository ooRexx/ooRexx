/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                            StringClassBit.cpp  */
/*                                                                            */
/* REXX string BITxxx methods                                                 */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "StringClass.hpp"
#include "MethodArguments.hpp"


/**
 * String method for performing a BITAND operation.
 *
 * @param string2 The string to AND with (optional)
 * @param pad     The optional pad character.
 *
 * @return The ANDed string.
 */
RexxString *RexxString::bitAnd(RexxString *string2, RexxString *pad)
{
    // the other string is optional...the operation can be performed using just
    // pad characters.
    string2 = optionalStringArgument(string2, GlobalNames::NULLSTRING, ARG_ONE);

    size_t string2Len = string2->getLength();
    // get the pad character...the default is 'ff'x.
    char padChar = optionalPadArgument(pad, (char)0xff, ARG_TWO);

    size_t string1Len = getLength();

    size_t minLength;
    size_t maxLength;
    const char *padString;
    const char *source;

    // we adjust for whichever string is longer
    if (string1Len <= string2Len)
    {
        minLength = string1Len;
        maxLength = string2Len;
        padString = string2->getStringData();
        source = getStringData();
    }
    else
    {
        minLength = string2Len;
        maxLength = string1Len;
        padString = getStringData();
        source = string2->getStringData();
    }

    // calculate the length of padding required
    size_t padLength = maxLength - minLength;

    // get a result string the length of the longer string.
    RexxString *retval = raw_string(maxLength);
    char *target = retval->getWritableData();

    // copy in the longer string
    memcpy(target, padString, maxLength);

    // now peform the AND operation between the two strings
    while (minLength--)
    {
        *target = *target & *source++;
        target++;
    }

    // now do the end part with the pad character
    while (padLength--)
    {
        *target = *target & padChar;
        target++;
    }

    return retval;
}


/**
 * String method for performing a BITOR operation.
 *
 * @param string2 The string to OR with (optional)
 * @param pad     The optional pad character.
 *
 * @return The ORed string.
 */
RexxString *RexxString::bitOr(RexxString *string2, RexxString *pad)
{
    // the other string is optional...the operation can be performed using just
    // pad characters.
    string2 = optionalStringArgument(string2, GlobalNames::NULLSTRING, ARG_ONE);

    size_t string2Len = string2->getLength();
    // get the pad character...the default is '00'x.
    char padChar = optionalPadArgument(pad, (char)0x00, ARG_TWO);

    size_t string1Len = getLength();

    size_t minLength;
    size_t maxLength;
    const char *padString;
    const char *source;

    // we adjust for whichever string is longer
    if (string1Len <= string2Len)
    {
        minLength = string1Len;
        maxLength = string2Len;
        padString = string2->getStringData();
        source = getStringData();
    }
    else
    {
        minLength = string2Len;
        maxLength = string1Len;
        padString = getStringData();
        source = string2->getStringData();
    }

    // calculate the length of padding required
    size_t padLength = maxLength - minLength;

    // get a result string the length of the longer string.
    RexxString *retval = raw_string(maxLength);
    char *target = retval->getWritableData();

    // copy in the longer string
    memcpy(target, padString, maxLength);

    // now peform the AND operation between the two strings
    while (minLength--)
    {
        *target = *target | *source++;
        target++;
    }

    // now do the end part with the pad character
    while (padLength--)
    {
        *target = *target | padChar;
        target++;
    }

    return retval;
}
/**
 * String method for performing a BITXOR operation.
 *
 * @param string2 The string to XOR with (optional)
 * @param pad     The optional pad character.
 *
 * @return The XORed string.
 */
RexxString *RexxString::bitXor(RexxString *string2, RexxString *pad)
{
    // the other string is optional...the operation can be performed using just
    // pad characters.
    string2 = optionalStringArgument(string2, GlobalNames::NULLSTRING, ARG_ONE);

    size_t string2Len = string2->getLength();
    // get the pad character...the default is '00'x.
    char padChar = optionalPadArgument(pad, (char)0x00, ARG_TWO);

    size_t string1Len = getLength();

    size_t minLength;
    size_t maxLength;
    const char *padString;
    const char *source;

    // we adjust for whichever string is longer
    if (string1Len <= string2Len)
    {
        minLength = string1Len;
        maxLength = string2Len;
        padString = string2->getStringData();
        source = getStringData();
    }
    else
    {
        minLength = string2Len;
        maxLength = string1Len;
        padString = getStringData();
        source = string2->getStringData();
    }

    // calculate the length of padding required
    size_t padLength = maxLength - minLength;

    // get a result string the length of the longer string.
    RexxString *retval = raw_string(maxLength);
    char *target = retval->getWritableData();

    // copy in the longer string
    memcpy(target, padString, maxLength);

    // now peform the XOR operation between the two strings
    while (minLength--)
    {
        *target = *target ^ *source++;
        target++;
    }

    // now do the end part with the pad character
    while (padLength--)
    {
        *target = *target ^ padChar;
        target++;
    }

    return retval;
}

