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
/* REXX Kernel                                                                */
/*                                                                            */
/* String Utilities shared between String class and MutableBuffer class       */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>

#include "RexxCore.h"
#include "StringClass.hpp"
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
    size_t position = positionArgument(_position, ARG_ONE) - 1;
    // assume nothing is pulled from this string
    size_t length = 0;
    // is the position within the string bounds?
    if (stringLength >= position)
    {
        // we extract everything from the position to the end (potentially)
        length = stringLength - position;
    }
    // now we process any overrides on this
    length = optionalLengthArgument(_length, length, ARG_TWO);
    // get a padding character (blank is default)
    char padChar = optionalPadArgument(pad, ' ', ARG_THREE);

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
RexxInteger *StringUtil::posRexx(const char *stringData, size_t length, RexxString *needle, RexxInteger *pstart, RexxInteger *range)
{
    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    /* get the starting position         */
    size_t _start = optionalPositionArgument(pstart, 1, ARG_TWO);
    size_t _range = optionalLengthArgument(range, length - _start + 1, ARG_THREE);
    /* pass on to the primitive function */
    /* and return as an integer object   */
    return new_integer(pos(stringData, length, needle, _start - 1, _range));
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
size_t StringUtil::pos(const char *stringData, size_t haystack_length, RexxString *needle, size_t _start, size_t _range)
{
    // get the two working lengths
    size_t needle_length = needle->getLength();
    // make sure the range is capped
    _range = Numerics::minVal(_range, haystack_length - _start + 1);

    // ok, there are a few quick checks we can perform.  If the needle is
    // bigger than the haystack, or the needle is a null string or
    // our haystack length after adjusting to the starting position
    // zero, then we can quickly return zero.
    if (_start > haystack_length || needle_length > _range || needle_length == 0)
    {
        return 0;
    }

    // address the string value
    const char *haypointer = stringData + _start;
    const char *needlepointer = needle->getStringData();
    size_t location = _start + 1;         // this is the match location as an index
    // calculate the number of probes we can make in this string
    size_t count = _range - needle_length + 1;

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
size_t StringUtil::caselessPos(const char *stringData, size_t haystack_length, RexxString *needle, size_t _start, size_t _range)
{
    // get the two working lengths
    size_t needle_length = needle->getLength();
    // make sure the range is capped
    _range = Numerics::minVal(_range, haystack_length - _start + 1);

    // ok, there are a few quick checks we can perform.  If the needle is
    // bigger than the haystack, or the needle is a null string or
    // our haystack length after adjusting to the starting position
    // zero, then we can quickly return zero.
    if (_start > haystack_length || needle_length > _range || needle_length == 0)
    {
        return 0;
    }

    // address the string value
    const char *haypointer = stringData + _start;
    const char *needlepointer = needle->getStringData();
    size_t location = _start + 1;         // this is the match location as an index
    // calculate the number of probes we can make in this string
    size_t count = _range - needle_length + 1;

    // now scan
    while (count--)
    {
                                           /* get a hit?                        */
        if (caselessCompare(haypointer, needlepointer, needle_length) == 0)
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
RexxInteger *StringUtil::lastPosRexx(const char *stringData, size_t haystackLen, RexxString  *needle, RexxInteger *_start, RexxInteger *_range)
{
    needle = stringArgument(needle, ARG_ONE);
    // find out where to start the search. The default is at the very end.
    size_t startPos = optionalPositionArgument(_start, haystackLen, ARG_TWO);
    size_t range = optionalLengthArgument(_range, haystackLen, ARG_THREE);
    // now perform the actual search.
    return new_integer(lastPos(stringData, haystackLen, needle, startPos, range));
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
size_t StringUtil::lastPos(const char *stringData, size_t haystackLen, RexxString  *needle, size_t _start, size_t range)
{
    size_t needleLen = needle->getLength();          /* and get the length too            */

    // no match possible if either string is null
    if (needleLen == 0 || haystackLen == 0 || needleLen > range)
    {
        return 0;
    }
    else
    {
        // get the start position for the search.
        haystackLen = Numerics::minVal(_start, haystackLen);
        range = Numerics::minVal(range, haystackLen);
        // adjust the starting point
        const char *startPoint = stringData + haystackLen - range;
                                         /* do the search                     */
        const char *matchLocation = lastPos(needle->getStringData(), needleLen, startPoint, haystackLen);
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
 * Primitive level caseless lastpos search within a string
 * buffer.
 *
 * @param stringData The maystack buffer.
 * @param haystack_length
 *                   The length of the haystack.
 * @param needle     The search needle.
 * @param _start     The starting position.
 *
 * @return The offset of the located needle, or 0 if the needle doesn't exist.
 */
size_t StringUtil::caselessLastPos(const char *stringData, size_t haystackLen, RexxString *needle, size_t _start, size_t range)
{
    size_t needleLen = needle->getLength();          /* and get the length too            */

    // no match possible if either string is null
    if (needleLen == 0 || haystackLen == 0 || needleLen > range)
    {
        return 0;
    }
    else
    {
        // get the start position for the search.
        haystackLen = Numerics::minVal(_start, haystackLen);
        range = Numerics::minVal(range, haystackLen);
        // adjust the starting point
        const char *startPoint = stringData + haystackLen - range;
                                         /* do the search                     */
        const char *matchLocation = caselessLastPos(needle->getStringData(), needleLen, startPoint, haystackLen);
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
    size_t position = positionArgument(positionArg, ARG_ONE) - 1;

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
        separator = stringArgument(separator, ARG_ONE);
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



/**
 * Convert a hex digit to it's integer value equivalent.
 *
 * @param ch     The input character.
 *
 * @return the integer value of the digit.
 */
int StringUtil::hexDigitToInt(char  ch)
{
    int   Retval;                        /* return value                      */

    if (isdigit(ch))                     /* if real digit                     */
    {
        Retval = ch - '0';                 /* convert that                      */
    }
    else
    {
        Retval = toupper(ch) - 'A' + 10;   /* convert alphabetic                */
    }
    return Retval;                       /* return conversion                 */
}

/**
 * The value of the buffer contents
 * interpreted as the binary expansion
 * of a byte, with most significant
 * bit in s[0] and least significant
 * bit in s[7].
 *
 * @param String The string to pack
 *
 * @return The single packed character.
 */
char StringUtil::packByte(const char *String )
{
    char Result = 0;                   /* start off at zero                 */
    for (int i = 0; i < 8; i++)        /* loop thru 8 chars                 */
    {
        if (String[i] == '1')          /* if 'bit' set                      */
        {
            Result |= (1<<(7-i));      /* or with mask                      */
        }
    }
    return Result;                     /* return packed byte                */
}

/**
 * The value of the buffer contents
 * interpreted as the binary expansion
 * of a byte, with most significant
 * bit in s[0] and least significant
 * bit in s[7].
 *
 * @param String Pack 4 characters into a hex string value.
 *
 * @return The hex character representing the nibble value.
 */
char StringUtil::packNibble(const char *String)
{
    char  Buf[8];                        /* temporary buffer                  */
    int   i;                             /* table index                       */

    memset(Buf, '0', 4);                 /* set first 4 bytes to zero         */
    memcpy(Buf+4, String, 4);            /* copy next 4 bytes                 */
    i = packByte(Buf);                   /* pack to a single byte             */
    return "0123456789ABCDEF"[i];        /* convert to a printable character  */
}

/**
 * Pack 2 0123456789ABCDEFabcdef chars into
 * byte
 *
 * The value of the buffer contents
 * interpreted as the hex expansion
 * of a byte, with most significant
 * nibble in s[0] and least significant
 * nibble in s[2].
 *
 * @param Byte   The pointer to the hex digit pair to pack.
 *
 * @return The single byte encoding of the pair of digits.
 */
char StringUtil::packByte2(const char *Byte)
{
    int      Nibble1;                    /* first nibble                      */
    int      Nibble2;                    /* second nibble                     */

                                         /* convert each digit                */
    Nibble1 = hexDigitToInt(Byte[0]);
    Nibble2 = hexDigitToInt(Byte[1]);
    /* combine the two digits            */

    return((Nibble1 << 4) | Nibble2);
}

/**
 * Validate blocks in string
 *
 * A string is considered valid if consists
 * of zero or more characters belonging to
 * the null-terminated C string set in
 * groups of size modulus.  The first group
 * may have fewer than modulus characters.
 * The groups are optionally separated by
 * one or more blanks.
 *
 * @param String  The string to validate.
 * @param Length  The string length.
 * @param Set     The valid characters in the set.
 * @param Modulus The size of the smallest allowed grouping.
 * @param Hex     Indicates this is a hex or binary string.  Used for issuing
 *                the correct error type.
 *
 * @return The number of valid digits found.
 */
size_t StringUtil::validateSet(const char *String, size_t Length, const char *Set, int Modulus, bool Hex)
{
    char     c;                          /* current character                 */
    size_t   Count;                      /* # set members found               */
    const char *Current;                 /* current location                  */
    const char *SpaceLocation = NULL;    /* location of last space            */
    int      SpaceFound;                 /* space found yet?                  */
    size_t   Residue = 0;                /* if space_found, # set             */
                                         /* members                           */

    if (*String == ch_SPACE)             /* if no leading blank               */
    {
        if (Hex)                           /* hex version?                      */
        {
            /* raise the hex message             */
            reportException(Error_Incorrect_method_hexblank, IntegerOne);
        }
        else
        {
            /* need the binary version           */
            reportException(Error_Incorrect_method_binblank, IntegerOne);
        }
    }
    SpaceFound = 0;                      /* set initial space flag            */
    Count = 0;                           /* start count with zero             */
    Current = String;                    /* point to start                    */

    for (; Length; Length--)
    {           /* process entire string             */
        c = *Current++;                    /* get char and step pointer         */
                                           /* if c in set                       */
        if (c != '\0' && strchr(Set, c) != NULL)
        {
            Count++;                         /* bump count                        */
        }
        else
        {
            if (c == ch_SPACE)
            {             /* if c blank                        */
                SpaceLocation = Current;       /* save the space location           */
                if (!SpaceFound)
                {             /* if 1st blank                      */
                              /* save position                     */
                    Residue = (Count % Modulus);
                    SpaceFound = 1;              /* we have the first space           */
                }
                /* else if bad position              */
                else if (Residue != (Count % Modulus))
                {
                    if (Hex)                     /* hex version?                      */
                    {
                        /* raise the hex message             */
                        reportException(Error_Incorrect_method_hexblank, SpaceLocation - String);
                    }
                    else
                    {
                        /* need the binary version           */
                        reportException(Error_Incorrect_method_binblank, SpaceLocation - String);
                    }
                }
            }
            else
            {

                if (Hex)                       /* hex version?                      */
                {
                    /* raise the hex message             */
                    reportException(Error_Incorrect_method_invhex, new_string((char *)&c, 1));
                }
                else
                {
                    reportException(Error_Incorrect_method_invbin, new_string((char *)&c, 1));
                }
            }
        }
    }
    /* if trailing blank or grouping bad */
    if ((c == ch_SPACE) || (SpaceFound && ((Count % Modulus) != Residue)))
    {
        if (Hex)                           /* hex version?                      */
        {
            /* raise the hex message             */
            reportException(Error_Incorrect_method_hexblank, SpaceLocation - String);
        }
        else
        {
            /* need the binary version           */
            reportException(Error_Incorrect_method_binblank, SpaceLocation - String);
        }
    }
    return Count;                        /* return count of chars             */
}

/**
 * Scan string for next members of
 * character set
 *
 * @param Destination
 *               The string where the characters are packed.
 * @param Source The source for the string data.
 * @param Length The length of the input string.
 * @param Count  The number of valid characters in the string.
 * @param Set    The set of allowed characters.
 * @param ScannedSize
 *               The returned scan size.
 *
 * @return
 */
size_t  StringUtil::chGetSm(char *Destination, const char *Source, size_t Length, size_t Count, const char *Set, size_t *ScannedSize)
{
    char      c;                         /* current scanned character  */
    const char *Current;                 /* current scan pointer       */
    size_t    Found;                     /* number of characters found */
    size_t    Scanned;                   /* number of character scanned*/

    Scanned = 0;                         /* nothing scanned yet        */
    Found = 0;                           /* nothing found yet          */
    Current = Source;                    /* get pointer to string      */

    for (; Length; Length--)
    {           /* scan entire string         */
        c = *Current++;                    /* get char and step pointer  */
        Scanned++;                         /* remember scan count        */
                                           /* if c in set                       */
        if (c != '\0' && strchr(Set, c) != NULL)
        {
            *Destination++ = c;              /* copy c                     */
            if (++Found == Count)            /* if all found               */
            {
                break;                         /* we are all done            */
            }
        }
    }
    *ScannedSize = Scanned;              /* return characters scanned  */
    return Found;                        /* and number found           */
}

/**
 * pack a string of 'hex' digits in place
 *
 * take two alpha chars and make into one byte
 *
 * @param String The string to pack
 * @param StringLength
 *               The length of the string.
 *
 * @return The resulting packed string.
 */
RexxString *StringUtil::packHex(const char *String, size_t StringLength)
{
    size_t   Nibbles;                    /* count of nibbles to pack          */
    size_t   n;
    const char *Source;                  /* pack source                       */
    char *    Destination;               /* packing destination               */
    size_t   b;                          /* nibble odd count                  */
    char     Buf[8];                     /* temp pack buffer                  */
    size_t   jjj;                        /* copies nibbles                    */
    RexxString *Retval;                  /* result value                      */

    if (StringLength)
    {                  /* if not a null string              */
        Source = String;                   /* get pointer                       */
        /* validate the information          */
        Nibbles = validateSet(Source, StringLength, "0123456789ABCDEFabcdef", 2, true);
        /* get a result string               */
        Retval = raw_string((Nibbles + 1) / 2);
        /* initialize destination            */
        Destination = Retval->getWritableData();

        while (Nibbles > 0)
        {              /* while chars to process            */

            b = Nibbles%2;                   /* get nibbles for next byte         */
            if (b == 0)                      /* even number                       */
            {
                b = 2;                         /* use two bytes                     */
            }
            else                             /* odd number,                       */
            {
                memset(Buf, '0', 2);           /* pad with zeroes                   */
            }

            jjj = 2 - b;                     /* copy nibbles into buff            */
            chGetSm(Buf+jjj, Source, StringLength, b, "0123456789ABCDEFabcdef", &n);
            *Destination++ = packByte2(Buf); /* pack into destination             */
            Source += n;                     /* advance source location           */
            StringLength -= n;               /* reduce the length                 */
            Nibbles -= b;                    /* decrement the count               */
        }
    }
    else
    {
        /* this is a null string             */
        Retval = OREF_NULLSTRING;
    }
    return Retval;                       /* return the packed string          */
}

/**
 * convert nibble to 4 '0'/'1' chars
 *
 * p[0], ..., p[3]: the four '0'/'1'
 * chars representing the nibble
 *
 * No terminating null character is
 * produced
 *
 * @param Val    The nibble to unpack.
 * @param p      The location to unpack into.
 */
void StringUtil::unpackNibble(int Val, char *p)
{
    p[0] = (Val & 0x08) != 0 ?'1':'0';
    p[1] = (Val & 0x04) != 0 ?'1':'0';
    p[2] = (Val & 0x02) != 0 ?'1':'0';
    p[3] = (Val & 0x01) != 0 ?'1':'0';
}


/**
 * Find the first occurrence of the set non-member in a string.
 *
 * @param String The string to search.
 * @param Set    The character set.
 * @param Length The length to search.
 *
 * @return The position of a match.
 */
const char *StringUtil::memcpbrk(const char *String, const char *Set, size_t Length)
{
    const char *Retval;                  /* returned value                    */

    Retval = NULL;                       /* nothing found yet                 */
    while (Length--)
    {                   /* search through string             */
                        /* find a match in ref set?          */
        if (*String == '\0' || !strchr(Set, *String))
        {
            Retval = String;                 /* copy position                     */
            break;                           /* quit the loop                     */
        }
        String++;                          /* step the pointer                  */
    }
    return Retval;                       /* return matched position           */
}


/**
 * Validate blocks in string
 *
 * A string is considered valid if consists
 * of zero or more characters belonging to
 * the null-terminated C string set in
 * groups of size modulus.  The first group
 * may have fewer than modulus characters.
 * The groups are optionally separated by
 * one or more blanks.
 *
 * @param String     The string to validate.
 * @param Length     The string length.
 * @param Set        The validation set.
 * @param Modulus    The set modulus
 * @param PackedSize The final packed size.
 *
 * @return The count of located characters.
 */
int StringUtil::valSet(const char *String, size_t Length, const char *Set, int Modulus, size_t *PackedSize )
{
    char     c = '\0';                   /* current character                 */
    size_t   Count;                      /* # set members found               */
    const char *Current;                 /* current location                  */
    int      SpaceFound;                 /* space found yet?                  */
    size_t   Residue = 0;                /* if space_found, # set members     */
    int      rc;                         /* return code                       */

    rc = false;                          /* default to failure                */
    if (*String != ' ' && *String != '\t')
    {    /* if no leading blank               */
        SpaceFound = 0;                    /* set initial space flag            */
        Count = 0;                         /* start count with zero             */
        Current = String;                  /* point to start                    */

        rc = true;                         /* default to good now               */
        for (; Length; Length--)
        {         /* process entire string             */
            c = *Current++;                  /* get char and step pointer         */
                                             /* if c in set                       */
            if (c != '\0' && strchr(Set, c) != NULL)
            {
                Count++;                       /* bump count                        */
            }
            else
            {
                if (c == ' ' || c == '\t')
                {   /* if c blank                        */
                    if (!SpaceFound)
                    {           /* if 1st blank                      */
                                /* save position                     */
                        Residue = (Count % Modulus);
                        SpaceFound = 1;            /* we have the first space           */
                    }
                    /* else if bad position              */
                    else if (Residue != (Count % Modulus))
                    {
                        rc = false;                /* this is an error                  */
                        break;                     /* report error                      */
                    }
                }
                else
                {
                    rc = false;                  /* this is an error                  */
                    break;                       /* report error                      */
                }
            }
        }
        if (rc)
        {                          /* still good?                       */
            if (c == ' ' || c == '\t')       /* if trailing blank                 */
            {
                rc = false;                    /* report error                      */
            }
            else if (SpaceFound && (Count % Modulus) != Residue)
            {
                rc = false;                    /* grouping problem                  */
            }
            else
            {
                *PackedSize = Count;           /* return count of chars             */
            }
        }
    }
    return rc;                           /* return success/failure            */
}


/**
 * Perform primitive datatype validation.
 *
 * @param String The target string.
 * @param Option The type of data to validate.
 *
 * @return True if this is of the indicated type, false for any mismatch.
 */
RexxObject *StringUtil::dataType(RexxString *String, char Option )
{
    size_t      Len;                     /* validated string length           */
    RexxObject *Answer;                  /* validation result                 */
    RexxObject *Temp;                    /* temporary value                   */
    const char *Scanp;                   /* string data pointer               */
    size_t      Count;                   /* hex nibble count                  */
    int         Type;                    /* validated symbol type             */
    RexxNumberString *TempNum;

    Len = String->getLength();           /* get validated string len          */
    Option = toupper(Option);            /* get the first character           */

                                         /* assume failure on checking        */
    Answer = TheFalseObject;
    /* get a scan pointer                */
    Scanp = String->getStringData();

    switch (Option)
    {                    /* based on type to confirm          */

        case DATATYPE_ALPHANUMERIC:        /* Alphanumeric                      */
            /* all in the set?                   */
            if (Len != 0 && !memcpbrk(Scanp, ALPHANUM, Len))
            {
                /* this is a good string             */
                Answer = TheTrueObject;
            }
            break;

        case DATATYPE_BINARY:              /* Binary string                     */
            /* validate the string               */
            if (Len == 0 || valSet(Scanp, Len, BINARI, 4, &Count))
            {
                /* this is a good string             */
                Answer = TheTrueObject;
            }
            break;

        case DATATYPE_LOWERCASE:           /* Lowercase                         */
            if (Len != 0 && !memcpbrk(Scanp, LOWER_ALPHA, Len))
            {
                /* this is a good string             */
                Answer = TheTrueObject;
            }
            break;

        case DATATYPE_UPPERCASE:           /* Uppercase                         */
            if (Len != 0 && !memcpbrk(Scanp, UPPER_ALPHA, Len))
            {
                /* this is a good string             */
                Answer = TheTrueObject;
            }
            break;

        case DATATYPE_MIXEDCASE:           /* Mixed case                        */
            if (Len != 0 && !memcpbrk(Scanp, MIXED_ALPHA, Len))
            {
                /* this is a good string             */
                Answer = TheTrueObject;
            }
            break;

        case DATATYPE_WHOLE_NUMBER:        /* Whole number                      */
            /* validate as a number              */
            TempNum = String->numberString();
            if (TempNum != OREF_NULL)
            {      /* valid number?                     */
                   /* force rounding to current digits  */
                TempNum = (RexxNumberString *)TempNum->plus(IntegerZero);
                /* check for integer then            */
                Answer = TempNum->isInteger();
            }
            break;

        case DATATYPE_NUMBER:              /* Number                            */
            /* validate as a number              */
            Temp = (RexxObject *)String->numberString();
            if (Temp != OREF_NULL)           /* valid number?                     */
            {
                /* got a good one                    */
                Answer = TheTrueObject;
            }
            break;

        case DATATYPE_9DIGITS:             /* NUMERIC DIGITS 9 number           */
            {                                  /* good long number                  */
                wholenumber_t temp;
                if (String->numberValue(temp))
                {
                    Answer = TheTrueObject;
                }
                break;
            }

        case DATATYPE_HEX:                 /* heXadecimal                       */
            /* validate the string               */
            if (Len == 0 || valSet(Scanp, Len, HEX_CHAR_STR, 2, &Count))
            {
                /* valid hexadecimal                 */
                Answer = TheTrueObject;
            }
            break;

        case DATATYPE_SYMBOL:              /* Symbol                            */
            /* validate the symbol               */
            if (String->isSymbol() != STRING_BAD_VARIABLE)
            {
                /* is a valid symbol                 */
                Answer = TheTrueObject;
            }
            break;

        case DATATYPE_VARIABLE:            /* Variable                          */

            /* validate the symbol               */
            Type = String->isSymbol();
            /* a valid variable type?            */
            if (Type == STRING_NAME ||
                Type == STRING_STEM ||
                Type == STRING_COMPOUND_NAME)
            {
                /* is a valid symbol                 */
                Answer = TheTrueObject;
            }
            break;

        case DATATYPE_LOGICAL:           // Test for a valid logical.
            if (Len != 1 || (*Scanp != '1' && *Scanp != '0'))
            {
                Answer = TheFalseObject;
            }
            else
            {
                Answer = TheTrueObject;
            }

            break;

        default  :                         /* unsupported option                */
            reportException(Error_Incorrect_method_option, "ABCDLMNOSUVWX9", new_string((const char *)&Option,1));
    }
    return Answer;                       /* return validation answer          */
}


/**
 * Skip leading blanks in a string.
 *
 * @param String The target string.
 * @param StringLength
 *               The length of the string segment.
 */
void StringUtil::skipBlanks(const char **String, size_t *StringLength )
{
    const char *Scan;                    /* scan pointer               */
    size_t   Length;                     /* length to scan             */

    Scan = *String;                      /* point to data              */
    Length = *StringLength;              /* get the length             */

    for (;Length; Length--)
    {            /* scan entire string         */
        if (*Scan != ' ' && *Scan != '\t') /* if not a space             */
        {
            break;                           /* just quit the loop         */
        }
        Scan++;                            /* step to next character     */
    }
    /* fell through, all blanks   */
    *String = Scan;                      /* set pointer one past       */
    *StringLength = Length;              /* update the length          */
}

/**
 * Skip non-blank characters to the next whitespace char.
 *
 * @param String The source string.
 * @param StringLength
 *               The string length (update on return);
 */
void StringUtil::skipNonBlanks(const char **String, size_t   *StringLength )
{
    const char *Scan;                    /* scan pointer               */
    size_t   Length;                     /* length to scan             */

    Scan = *String;                      /* point to data              */
    Length = *StringLength;              /* get the length             */

    for (;Length; Length--)
    {            /* scan entire string         */
        if (*Scan == ' ' || *Scan == '\t') /* if not a space             */
        {
            break;                           /* just quit the loop         */
        }
        Scan++;                            /* step to next character     */
    }
    /* fell through, all blanks   */
    *String = Scan;                      /* set pointer one past       */
    *StringLength = Length;              /* update the length          */
}


/**
 * Count the number of words in a string.
 *
 * @param String The string to count.
 * @param StringLength
 *               The length of the string.
 *
 * @return The count of white-space delimited words.
 */
size_t  StringUtil::wordCount(const char *String, size_t   StringLength )
{
    size_t Count = 0;                           /* default to nothing         */
    if (StringLength)
    {                  /* if not a null string       */
        skipBlanks(&String, &StringLength);/* skip any leading blanks    */

        while (StringLength)
        {             /* while still string ...     */
            Count++;                         /* account for this word      */
                                             /* now skip the non-blanks    */
            skipNonBlanks(&String, &StringLength);
            if (!StringLength)               /* if done with the string    */
            {
                break;                         /* we are finished            */
            }
                                               /* skip to the next word      */
            skipBlanks(&String, &StringLength);
        }                                  /* loop while still have chars*/
    }
    return  Count;                       /* done looping, return the   */
                                         /* count of words             */
}


/**
 * Find the next word in the string.
 *
 * @param String     The source string.
 * @param StringLength
 *                   The length of the string (update on return).
 * @param NextString The next word position.
 *
 * @return The length of the located word.
 */
size_t StringUtil::nextWord(const char **String, size_t *StringLength, const char **NextString )
{
    size_t WordStart = 0;                       /* nothing moved yet          */
    if (*StringLength)
    {                 /* Something there?           */
        skipBlanks(String, StringLength);  /* skip any leading blanks    */

        if (*StringLength)
        {               /* if still string ...        */
            WordStart = *StringLength;       /* save current length        */
            *NextString = *String;           /* save start position now    */
                                             /* skip the non-blanks        */
            skipNonBlanks(NextString, StringLength);
            WordStart -= *StringLength;      /* adjust the word length     */
        }
    }
    return WordStart;                    /* return word length         */
}


/**
 * Count the occurences of a string within another string.
 *
 * @param hayStack Pointer to the haystack data.
 * @param hayStackLength
 *                 Length of the haystack data.
 * @param needle   The needle we're searching for
 *
 * @return The count of needle occurrences located in the string.
 */
size_t StringUtil::countStr(const char *hayStack, size_t hayStackLength, RexxString *needle)
{
    size_t count = 0;                           /* no matches yet                    */
    /* get the first match position      */
    size_t matchPos = pos(hayStack, hayStackLength, needle, 0, hayStackLength);
    while (matchPos != 0)
    {
        count = count + 1;                 /* count this match                  */
        // step to the new position and search
        matchPos = pos(hayStack, hayStackLength, needle, matchPos + needle->getLength() - 1, hayStackLength);
    }
    return count;                        /* return the match count            */
}


/**
 * Count the occurences of a string within another string.
 *
 * @param hayStack Pointer to the haystack data.
 * @param hayStackLength
 *                 Length of the haystack data.
 * @param needle   The needle we're searching for
 *
 * @return The count of needle occurrences located in the string.
 */
size_t StringUtil::caselessCountStr(const char *hayStack, size_t hayStackLength, RexxString *needle)
{
    size_t count = 0;                           /* no matches yet                    */
    /* get the first match position      */
    size_t matchPos = caselessPos(hayStack, hayStackLength, needle, 0, hayStackLength);
    while (matchPos != 0)
    {
        count = count + 1;                 /* count this match                  */
        // step to the new position and search
        matchPos = caselessPos(hayStack, hayStackLength, needle, matchPos + needle->getLength() - 1, hayStackLength);
    }
    return count;                        /* return the match count            */
}


size_t StringUtil::memPos(
  const char *string,                  /* search string                     */
  size_t length,                       /* string length                     */
  char   target )                      /* target character                  */
/*********************************************************************/
/*  Function:  offset of first occurrence of char in string          */
/*********************************************************************/
{
                                         /* while in the string               */
    for (const char *scan = string; length; length--)
    {
        // if we have a match, return the offset
        if (*scan == target)
        {
            return scan - string;
        }
        scan++;                            /* step the position                 */
    }
    return -1;                   // no match position
}


/**
 * Perform a verify operation on a section of data.
 *
 * @param data      The data pointer
 * @param stringLen The length of the string to match
 * @param ref       The reference search string.
 * @param option    The match/nomatch option.
 * @param _start    The starting offset for the match.
 *
 * @return The match/nomatch position, or 0 if nothing was found.
 */
RexxInteger *StringUtil::verify(const char *data, size_t stringLen, RexxString  *ref, RexxString  *option, RexxInteger *_start, RexxInteger *range)
{
    // get the reference string information
    ref = stringArgument(ref, ARG_ONE);
    size_t referenceLen = ref->getLength();
                                         /* get the option, default 'Nomatch' */
    char opt = optionalOptionArgument(option, VERIFY_NOMATCH, ARG_TWO);
    // validate the possibilities
    if (opt != VERIFY_MATCH && opt != VERIFY_NOMATCH)
    {
        /* not that either, then its an error*/
        reportException(Error_Incorrect_method_option, "MN", option);
    }

    /* get starting position             */
    size_t startPos = optionalPositionArgument(_start, 1, ARG_THREE);
    size_t stringRange = optionalLengthArgument(range, stringLen - startPos + 1, ARG_FOUR);
    if (startPos > stringLen)            /* beyond end of string?             */
    {
        return IntegerZero;              /* couldn't find it                  */
    }
    else
    {
        // adjust the range for seaching
        stringRange = Numerics::minVal(stringRange, stringLen - startPos + 1);

        /* point at start position           */
        const char *current = data + startPos - 1;
        if (referenceLen == 0)
        {               /* if verifying a nullstring         */
            if (opt == VERIFY_MATCH)      /* can't match at all                */
            {
                return IntegerZero;          /* so return zero                    */
            }
            else
            {
                return new_integer(startPos);/* non-match at start position       */
            }
        }
        else
        {
            // we're verifying that all characters are members of the reference set, so
            // return the first non-matching character
            if (opt == VERIFY_NOMATCH)
            {
                while (stringRange-- != 0)
                {            /* while input left                  */
                    char ch = *current++;          /* get next char                     */
                                                   /* get reference string              */
                    const char *reference = ref->getStringData();
                    size_t temp = referenceLen;           /* copy the reference length         */

                    while (temp != 0)
                    {               /* spin thru reference               */
                        if (ch == *reference++)
                        {
                            // we have a match, so we can leave
                            break;
                        }
                        temp--;
                    }
                    // terminate because we tested all characters?
                    if (temp == 0)
                    {
                        // mismatch at this offset
                        return new_integer(current - data);
                    }
                }
                // this is always a non matching situation to get here
                return IntegerZero;
            }
            else
            {
                while (stringLen-- != 0)
                {            /* while input left                  */
                    char ch = *current++;          /* get next char                     */
                                                   /* get reference string              */
                    const char *reference = ref->getStringData();
                    size_t temp = referenceLen;           /* copy the reference length         */

                    while (temp != 0)
                    {               /* spin thru reference               */
                        if (ch == *reference++)
                        {
                            // we found a matching character, return that position
                            return new_integer(current - data);
                        }
                        temp--;
                    }
                }
                // this is always a non matching situation to get here
                return IntegerZero;
            }
        }
    }
}


/**
 * Do a subword operation on a buffer of data
 *
 * @param data     The start of the data buffer.
 * @param length   The length of the buffer
 * @param position The starting word position.
 * @param plength  the count of words to return.
 *
 * @return The string containing the indicated subwords.
 */
RexxString *StringUtil::subWord(const char *data, size_t length, RexxInteger *position, RexxInteger *plength)
{
                                         /* convert position to binary        */
    size_t wordPos = positionArgument(position, ARG_ONE);
    // get num of words to extract.  The default is a "very large number
    size_t count = optionalLengthArgument(plength, Numerics::MAX_WHOLENUMBER, ARG_TWO);

    // handle cases that will always result in a null string
    if (length == 0 || count == 0)
    {
        return OREF_NULLSTRING;
    }
    const char *nextSite = NULL;
    const char *word = data;
                                       /* get the first word                */
    size_t wordLength = nextWord(&word, &length, &nextSite);
    while (--wordPos > 0 && wordLength != 0)
    {  /* loop until we reach tArget        */
        word = nextSite;                 /* copy the start pointer            */
                                         /* get the next word                 */
        wordLength = nextWord(&word, &length, &nextSite);
    }
    // we terminated because there was no word found before we reached the
    // count position
    if (wordPos != 0)
    {
        return OREF_NULLSTRING;        /* again a null string               */
    }

    const char *wordStart = word;                /* save start position               */
    const char *wordEnd = word;                  /* default end is the same           */
                                     /* loop until we reach tArget        */
    while (count-- > 0 && wordLength != 0)
    {
        wordEnd = word + wordLength;   /* point to the word end             */
        word = nextSite;               /* copy the start pointer            */
                                       /* get the next word                 */
        wordLength = nextWord(&word, &length, &nextSite);
    }
    /* extract the substring             */
    return new_string(wordStart, wordEnd - wordStart);
}


/**
 * Extract a word from a buffer
 *
 * @param data     The data pointer
 * @param length   the length of the data buffer.
 * @param position the target word position.
 *
 * @return The string value of the word at the indicated position.
 */
RexxString *StringUtil::word(const char *data, size_t length, RexxInteger *position)
{
                                         /* convert position to binary        */
    size_t wordPos = positionArgument(position, ARG_ONE);

    if (length == 0)                     /* null string?                      */
    {
        return OREF_NULLSTRING;          /* result is null also               */
    }
    const char *word = data;             /* point to the string               */
    const char *nextSite = NULL;
                                       /* get the first word                */
    size_t wordLength = nextWord(&word, &length, &nextSite);
    while (--wordPos > 0 && wordLength != 0)
    {  /* loop until we reach target        */
        word = nextSite;                 /* copy the start pointer            */
                                         /* get the next word                 */
        wordLength = nextWord(&word, &length, &nextSite);
    }
    if (wordLength != 0)               /* have a word                       */
    {
        /* extract the string                */
        return new_string(word, wordLength);
    }
    return OREF_NULLSTRING;        /* no word, return a null            */
}


/**
 * Extract all words from a buffer
 *
 * @param data     The data pointer
 * @param length   the length of the data buffer.
 * @param position the target word position.
 *
 * @return The string value of the word at the indicated position.
 */
RexxArray *StringUtil::words(const char *data, size_t length)
{
    const char *word = data;             /* point to the string               */
    const char *nextSite = NULL;

    RexxArray *result = new_array((size_t)0);
    ProtectedObject p(result);
                                       /* get the first word                */
    size_t wordLength = nextWord(&word, &length, &nextSite);
    while (wordLength != 0)
    {
        // add to the result array
        result->append(new_string(word, wordLength));
        word = nextSite;                 /* copy the start pointer            */
                                         /* get the next word                 */
        wordLength = nextWord(&word, &length, &nextSite);
    }
    return result;      // return whatever we've accumulated
}


/**
 * Return the index position for a given word
 *
 * @param data     The data containing the words
 * @param length   The length of the data buffer
 * @param position The target word position
 *
 * @return The offset of the start of the indicated word.
 */
RexxInteger *StringUtil::wordIndex(const char *data, size_t length, RexxInteger *position)
{
                                         /* convert count to binary           */
    size_t wordPos = positionArgument(position, ARG_ONE);
    const char *word = data;             /* point to word data                */
    const char *nextSite = NULL;

                                         /* get the first word                */
    size_t wordLength = nextWord(&word, &length, &nextSite);
    while (--wordPos > 0 && wordLength != 0)
    {    /* loop until we reach target        */
        word = nextSite;                   /* copy the start pointer            */
                                           /* get the next word                 */
        wordLength = nextWord(&word, &length, &nextSite);
    }

    if (wordLength == 0)                 /* ran out of string                 */
    {
        return IntegerZero;              /* no index                          */
    }
    return new_integer(word - data + 1);
}


/**
 * Return the length of the word located at a given index.
 *
 * @param data     The data containing the word list.
 * @param length   The length of the data buffer
 * @param position The target word position.
 *
 * @return The length of the given word at the target index.  Returns
 *         0 if no word is found.
 */
RexxInteger *StringUtil::wordLength(const char *data, size_t length, RexxInteger *position)
{
    /* convert count to binary           */
    size_t wordPos = positionArgument(position , ARG_ONE);
    const char *word = data;             /* point to word data                */
    const char *nextSite = NULL;

    /* get the first word                */
    size_t wordLength = nextWord(&word, &length, &nextSite);
    while (--wordPos > 0 && wordLength != 0)
    {    /* loop until we reach target        */
        word = nextSite;                   /* copy the start pointer            */
                                           /* get the next word                 */
        wordLength = nextWord(&word, &length, &nextSite);
    }
    return new_integer(wordLength);      /* return the word length            */
}


/**
 * Execute a wordpos search on a buffer of data.
 *
 * @param data   the source data buffer.
 * @param length the length of the buffer
 * @param phrase the search phrase.
 * @param pstart the starting position.
 *
 * @return the location of the start of the search phrase.
 */
RexxInteger *StringUtil::wordPos(const char *data, size_t length, RexxString  *phrase, RexxInteger *pstart)
{
    phrase = stringArgument(phrase, ARG_ONE);/* get the phrase we are looking for */
    stringsize_t needleLength = phrase->getLength();       /* get the length also               */
                                         /* get starting position, the default*/
                                         /* is the first word                 */
    stringsize_t count = optionalPositionArgument(pstart, 1, ARG_TWO);

    const char *needle = phrase->getStringData();  /* get friendly pointer              */
    const char *haystack = data;                   /* and the second also               */
    stringsize_t haystackLength = length;          /* get the haystack length           */
                                                 /* count the words in needle         */
    stringsize_t needleWords = wordCount(needle, needleLength);
                                         /* and haystack                      */
    stringsize_t haystackWords = wordCount(haystack, haystackLength);
                                         /* if search string is longer        */
                                         /* or no words in search             */
                                         /* or count is longer than           */
                                         /* haystack, this is a failure       */
    if (needleWords > (haystackWords - count + 1) || needleWords == 0 || count > haystackWords)
    {
        return IntegerZero;
    }

    const char *nextHaystack;
    const char *nextNeedle;
                                       /* point at first word               */
    stringsize_t haystackWordLength = nextWord(&haystack, &haystackLength, &nextHaystack);
                                       /* now skip over count-1             */
    for (stringsize_t i = count - 1; i && haystackWordLength != 0; i--)
    {
        haystack = nextHaystack;         /* step past current word            */
                                       /* find the next word                */
        haystackWordLength = nextWord(&haystack, &haystackLength, &nextHaystack);
    }
                                       /* get number of searches            */
    stringsize_t searchCount = (haystackWords - needleWords - count) + 2;
                                       /* position at first needle          */
    stringsize_t firstNeedle = nextWord(&needle, &needleLength, &nextNeedle);
                                       /* loop for the possible times       */
    for (; searchCount; searchCount--)
    {
        stringsize_t needleWordLength = firstNeedle;   /* set the length                    */
        const char *needlePosition = needle;         /* get the start of phrase           */
        const char *haystackPosition = haystack;     /* and the target string loop        */
                                         /* for needlewords                   */
        const char *nextHaystackPtr = nextHaystack;  /* copy nextword information         */
        const char *nextNeedlePtr = nextNeedle;
                                         /* including the lengths             */
        stringsize_t haystackScanLength = haystackLength;
        stringsize_t needleScanLength = needleLength;

        stringsize_t i;

        for (i = needleWords; i; i--)
        {
            // length mismatch, can't be a match

            if (haystackWordLength != needleWordLength)
            {
                break;
            }

            // now compare the two words, using a caseless comparison
            // if the words don't match, terminate now
            if (memcmp(needlePosition, haystackPosition, needleWordLength) != 0)
            {
                break;                       /* get out fast.                     */
            }

                                           /* the last words matched, so        */
                                           /* continue searching.               */

                                           /* set new search information        */
            haystackPosition = nextHaystackPtr;
            needlePosition = nextNeedlePtr;
                                           /* Scan off the next word            */
            haystackWordLength = nextWord(&haystackPosition, &haystackScanLength, &nextHaystackPtr);
                                           /* repeat for the needle             */
            needleWordLength = nextWord(&needlePosition, &needleScanLength, &nextNeedlePtr);
        }

        if (i == 0)                      /* all words matched, we             */
        {
            return new_integer(count);   // return the position
        }
        haystack = nextHaystack;         /* set the search position           */
                                         /* step to next haytack pos          */
        haystackWordLength = nextWord(&haystack, &haystackLength, &nextHaystack);
        count++;                         /* remember the word position        */
    }

    return IntegerZero;                // not found
}


/**
 * Execute a caseless wordpos search on a buffer of data.
 *
 * @param data   the source data buffer.
 * @param length the length of the buffer
 * @param phrase the search phrase.
 * @param pstart the starting position.
 *
 * @return the location of the start of the search phrase.
 */
RexxInteger *StringUtil::caselessWordPos(const char *data, size_t length, RexxString  *phrase, RexxInteger *pstart)
{
    phrase = stringArgument(phrase, ARG_ONE);/* get the phrase we are looking for */
    stringsize_t needleLength = phrase->getLength();       /* get the length also               */
                                         /* get starting position, the default*/
                                         /* is the first word                 */
    stringsize_t count = optionalPositionArgument(pstart, 1, ARG_TWO);

    const char *needle = phrase->getStringData();  /* get friendly pointer              */
    const char *haystack = data;                   /* and the second also               */
    stringsize_t haystackLength = length;          /* get the haystack length           */
                                                 /* count the words in needle         */
    stringsize_t needleWords = wordCount(needle, needleLength);
                                         /* and haystack                      */
    stringsize_t haystackWords = wordCount(haystack, haystackLength);
                                         /* if search string is longer        */
                                         /* or no words in search             */
                                         /* or count is longer than           */
                                         /* haystack, this is a failure       */
    if (needleWords > (haystackWords - count + 1) || needleWords == 0 || count > haystackWords)
    {
        return IntegerZero;
    }

    const char *nextHaystack;
    const char *nextNeedle;
                                       /* point at first word               */
    stringsize_t haystackWordLength = nextWord(&haystack, &haystackLength, &nextHaystack);
                                       /* now skip over count-1             */
    for (stringsize_t i = count - 1; i && haystackWordLength != 0; i--)
    {
        haystack = nextHaystack;         /* step past current word            */
                                       /* find the next word                */
        haystackWordLength = nextWord(&haystack, &haystackLength, &nextHaystack);
    }
                                       /* get number of searches            */
    stringsize_t searchCount = (haystackWords - needleWords - count) + 2;
                                       /* position at first needle          */
    stringsize_t firstNeedle = nextWord(&needle, &needleLength, &nextNeedle);
                                       /* loop for the possible times       */
    for (; searchCount; searchCount--)
    {
        stringsize_t needleWordLength = firstNeedle;   /* set the length                    */
        const char *needlePosition = needle;         /* get the start of phrase           */
        const char *haystackPosition = haystack;     /* and the target string loop        */
                                         /* for needlewords                   */
        const char *nextHaystackPtr = nextHaystack;  /* copy nextword information         */
        const char *nextNeedlePtr = nextNeedle;
                                         /* including the lengths             */
        stringsize_t haystackScanLength = haystackLength;
        stringsize_t needleScanLength = needleLength;

        stringsize_t i;

        for (i = needleWords; i; i--)
        {
            // length mismatch, can't be a match

            if (haystackWordLength != needleWordLength)
            {
                break;
            }

            // now compare the two words, using a caseless comparison
            // if the words don't match, terminate now
            if (caselessCompare(needlePosition, haystackPosition, needleWordLength))
            {
                break;                       /* get out fast.                     */
            }

                                           /* the last words matched, so        */
                                           /* continue searching.               */

                                           /* set new search information        */
            haystackPosition = nextHaystackPtr;
            needlePosition = nextNeedlePtr;
                                           /* Scan off the next word            */
            haystackWordLength = nextWord(&haystackPosition, &haystackScanLength, &nextHaystackPtr);
                                           /* repeat for the needle             */
            needleWordLength = nextWord(&needlePosition, &needleScanLength, &nextNeedlePtr);
        }

        if (i == 0)                      /* all words matched, we             */
        {
            return new_integer(count);   // return the position
        }
        haystack = nextHaystack;         /* set the search position           */
                                         /* step to next haytack pos          */
        haystackWordLength = nextWord(&haystack, &haystackLength, &nextHaystack);
        count++;                         /* remember the word position        */
    }

    return IntegerZero;                // not found
}
