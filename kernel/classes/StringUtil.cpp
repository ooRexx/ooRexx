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
/* REXX Kernel                                                                */
/*                                                                            */
/* String Utilities shared between String class and MutableBuffer class       */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>

#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxBuiltinFunctions.h"
#include "ProtectedObject.hpp"
#include "StringUtil.hpp"
#include "QueueClass.hpp"


/**
 * Extract a substring from a data buffer.
 *
 * @param string    The data buffer.
 * @param stringLength
 *                  The length of the buffer.
 * @param _position The position argument for the starting position.
 * @param _length   The substring length argument.
 * @param pad       The padding argument.
 *
 * @return The extracted substring.
 */
RexxString *StringUtil::substr(const char *string, size_t stringLength, RexxInteger *_position,
    RexxInteger *_length, RexxString  *pad)
{
    size_t position = get_position(_position, ARG_ONE) - 1;
    // assume nothing is pulled from this string
    size_t length = 0;
    // is the position within the string bounds?
    if (stringLength >= position)
    {
        // we extract everything from the position to the end (potentially)
        length = stringLength - position;
    }
    // now we process any overrides on this
    length = optional_length(_length, length, ARG_TWO);
    // get a padding character (blank is default)
    char padChar = get_pad(pad, ' ', ARG_THREE);

    // if our target length is zero, we can just return the null string singleton
    if (length == 0)
    {
        return OREF_NULLSTRING;
    }

    size_t substrLength = 0;
    size_t padCount = 0;

    // starting past the end of the string?
    // this will be all pad characters
    if (position > stringLength)
    {
        padCount = length;
    }
    else
    {
        // we have a combination of source string and pad characters
        substrLength = Numerics::minVal(length, stringLength - position);
        padCount = length - substrLength;
    }
    RexxString *retval = raw_string(length);       /* get result string                 */
    if (substrLength)                  /* data to copy?                     */
    {
        // copy over the string portion
        retval->put(0, string + position, substrLength);
    }
    // add any needed padding characters
    if (padCount != 0)
    {
        retval->set(substrLength, padChar, padCount);
    }
    // and return the final result
    return retval;
}


/**
 * Locate a string within the designated string buffer.
 *
 * @param stringData The stringData to search within.
 * @param length     The length of the string data.
 * @param needle     The needle to search for.
 * @param pstart     The starting position.
 *
 * @return An integer object giving the located position.
 */
RexxInteger *StringUtil::posRexx(const char *stringData, size_t length, RexxString *needle, RexxInteger *pstart)
{
    /* force needle to a string          */
    needle = REQUIRED_STRING(needle, ARG_ONE);
    /* get the starting position         */
    size_t _start = optional_position(pstart, 1, ARG_TWO);
    /* pass on to the primitive function */
    /* and return as an integer object   */
    return new_integer(pos(stringData, length, needle, _start - 1));
}


/**
 * Primitive level search withint a string buffer.
 *
 * @param stringData The maystack buffer.
 * @param haystack_length
 *                   The length of the haystack.
 * @param needle     The search needle.
 * @param _start     The starting position.
 *
 * @return The offset of the located needle, or 0 if the needle doesn't exist.
 */
size_t StringUtil::pos(const char *stringData, size_t haystack_length, RexxString *needle, size_t _start)
{
    // get the two working lengths
    size_t needle_length = needle->getLength();

    // ok, there are a few quick checks we can perform.  If the needle is
    // bigger than the haystack, or the needle is a null string or
    // our haystack length after adjusting to the starting position
    // zero, then we can quickly return zero.
    if (needle_length > haystack_length + _start || needle_length == 0 || _start + needle_length > haystack_length)
    {
        return 0;
    }

    // address the string value
    const char *haypointer = stringData + _start;
    const char *needlepointer = needle->getStringData();
    size_t location = _start + 1;         // this is the match location as an index
    // calculate the number of probes we can make in this string
    size_t count = (haystack_length - _start) - needle_length + 1;

    // now scan
    while (count--)
    {
                                           /* get a hit?                        */
        if (memcmp(haypointer, needlepointer, needle_length) == 0)
        {
            return location;
        }
        // step our pointers accordingly
        location++;
        haypointer++;
    }
    return 0;  // we got nothing...
}


/**
 * Locate the last positon of a string within the designated
 * string buffer.
 *
 * @param stringData The stringData to search within.
 * @param length     The length of the string data.
 * @param needle     The needle to search for.
 * @param pstart     The starting position.
 *
 * @return An integer object giving the located position.
 */
RexxInteger *StringUtil::lastPosRexx(const char *stringData, size_t haystackLen, RexxString  *needle, RexxInteger *_start)
{
    needle = REQUIRED_STRING(needle, ARG_ONE);
    // find out where to start the search. The default is at the very end.
    size_t startPos = optional_position(_start, haystackLen, ARG_TWO);
    // now perform the actual search.
    return new_integer(lastPos(stringData, haystackLen, needle, startPos));
}


/**
 * Primitive level lastpos search within a string buffer.
 *
 * @param stringData The maystack buffer.
 * @param haystack_length
 *                   The length of the haystack.
 * @param needle     The search needle.
 * @param _start     The starting position.
 *
 * @return The offset of the located needle, or 0 if the needle doesn't exist.
 */
size_t StringUtil::lastPos(const char *stringData, size_t haystackLen, RexxString  *needle, size_t _start)
{
    size_t needleLen = needle->getLength();          /* and get the length too            */

    // no match possible if either string is null
    if (needleLen == 0 || haystackLen == 0)
    {
        return 0;
    }
    else
    {
        // get the start position for the search.
        haystackLen = Numerics::minVal(_start, haystackLen);
                                         /* do the search                     */
        const char *matchLocation = lastPos(needle->getStringData(), needleLen, stringData, haystackLen);
        if (matchLocation == NULL)
        {
            return 0;
        }
        else
        {
            return matchLocation - stringData + 1;
        }
    }
}


/**
 * Absolutely most primitive version of a lastpos search.  This
 * version searches directly in a buffer rather than a Rexx
 * String.
 *
 * @param needle    Pointer to the needle string.
 * @param needleLen Length of the needle string.
 * @param haystack  The pointer to the haystack string.
 * @param haystackLen
 *                  The length of the haystack string.
 *
 * @return A pointer to the match location or NULL if there is no match.
 */
const char *StringUtil::lastPos(const char *needle, size_t needleLen, const char *haystack, size_t haystackLen)
{
    // if the needle's longer than the haystack, no chance of a match
    if (needleLen > haystackLen)
    {
        return NULL;
    }
    // set the search startpoing point relative to the end of the search string
    haystack = haystack + haystackLen - needleLen;
    // this is the possible number of compares we might need to perform
    size_t count = haystackLen - needleLen + 1;
    // now scan backward
    while (count > 0)
    {
        // got a match at this position, return it directly
        if (memcmp(haystack, needle, needleLen) == 0)
        {
            return haystack;
        }
        // decrement count and position
        count--;
        haystack--;
    }
    return NULL;   // nothing to see here folks, move along
}


/**
 * Absolutely most primitive version of a caseless lastpos
 * search. This version searches directly in a buffer rather
 * than a Rexx String.
 *
 * @param needle    Pointer to the needle string.
 * @param needleLen Length of the needle string.
 * @param haystack  The pointer to the haystack string.
 * @param haystackLen
 *                  The length of the haystack string.
 *
 * @return A pointer to the match location or NULL if there is no match.
 */
const char *StringUtil::caselessLastPos(const char *needle, size_t needleLen, const char *haystack, size_t haystackLen)
{
    // if the needle's longer than the haystack, no chance of a match
    if (needleLen > haystackLen)
    {
        return NULL;
    }
    // set the search startpoing point relative to the end of the search string
    haystack = haystack + haystackLen - needleLen;
    // this is the possible number of compares we might need to perform
    size_t count = haystackLen - needleLen + 1;
    // now scan backward
    while (count > 0)
    {
        // got a match at this position, return it directly
        if (caselessCompare(haystack, needle, needleLen) == 0)
        {
            return haystack;
        }
        // decrement count and position
        count--;
        haystack--;
    }
    return NULL;   // nothing to see here folks, move along
}


/**
 * Extract an individual character from a string buffer, returned
 * as a string object.
 *
 * @param stringData The string buffer.
 * @param stringLength
 *                   The length of the buffer.
 * @param positionArg
 *                   The target position.
 *
 * @return The target character, as a string value.
 */
RexxString *StringUtil::subchar(const char *stringData, size_t stringLength, RexxInteger *positionArg)
{
    // the starting position isn't optional
    size_t position = get_position(positionArg, ARG_ONE) - 1;

    // beyond the bounds, this is a null string
    if (position >= stringLength)
    {
        return OREF_NULLSTRING;
    }
    // return the single character
    return new_string(stringData + position, 1);
}


/**
 * Carve the string buffer up into an array of string values.
 *
 * @param start     The starting position of the buffer.
 * @param length    The length of the buffer.
 * @param separator The optional separator character.
 *
 * @return An array of all strings within the buffer, with the target
 *         delimiter removed.
 */
RexxArray *StringUtil::makearray(const char *start, size_t length, RexxString *separator)
{
    const char *sepData = "\n";          // set our default separator
    size_t sepSize = 1;
    bool checkCR = true;                 // by default, we look for either separator

    // if we have an explicit separator, use it instead
    if (separator != OREF_NULL)
    {
        // make sure this is really a string value
        separator = REQUIRED_STRING(separator, ARG_ONE);
        sepData = separator->getStringData();
        sepSize = separator->getLength();
        checkCR = false;                 // if explicitly given, only use the given one
    }


    // the Null string gets special handling
    if (sepSize == 0)
    {
        // we need an array the size of the string
        RexxArray *array = new_array(length);
        ProtectedObject p1(array);
        // create a string for each character and poke into the array
        for (size_t i = 0; i < length; i++, start++)
        {
            array->put(new_string(start, 1), i + 1);
        }
        return array;
    }


    RexxQueue *strings = new_queue();    /* save each string in a queue       */
    ProtectedObject p2(strings);         /* which we need to protect */

    // set our end marker
    const char *end = start + length - sepSize + 1;

    while (start < end)
    {
        const char *tmp = start;
        /* search for separator character    */
        while (tmp < end && memcmp(tmp, sepData, sepSize) != 0)
        {
            tmp++;
        }
        size_t stringLen = tmp - start;
        // if checking for either linend possibility, reduce the length if we had
        // a leading CR character
        if (checkCR && *(tmp - 1) == '\r')
        {
            stringLen--;
        }
        strings->queue(new_string(start, stringLen));
        // step to the next scan position
        start = tmp + sepSize;
    }
    // now convert this to an array
    return strings->makeArray();
}


/**
 * Perform a caseless comparison of two strings
 *
 * @param string1 The first string to compare.
 * @param string2 The second string.
 * @param length  The length to compare.
 *
 * @return 0 if the two strings are equal, -1 if the first is less than the
 *         second, and 1 if the first string is the greater.
 */
int  StringUtil::caselessCompare(const char *string1, const char *string2, size_t length)
{
    /* totally equal?                    */
    if (!memcmp(string1, string2, length))
    {
        return 0;                          /* return equality indicator         */
    }

    while (length--)                     /* need to do it the hardway         */
    {
        /* not equal?                        */
        if (toupper(*string1) != toupper(*string2))
        {
            /* first one less?                   */
            if (toupper(*string1) < toupper(*string2))
            {
                return -1;                     /* return less than indicator        */
            }
            else
            {
                return 1;                      /* first is larger                   */
            }
        }
        string1++;                         /* step first pointer                */
        string2++;                         /* and second pointer also           */
    }
    return 0;                            /* fall through, these are equal     */
}
