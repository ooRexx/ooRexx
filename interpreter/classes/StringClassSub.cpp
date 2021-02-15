/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/* substring oriented REXX string methods                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ActivityManager.hpp"
#include "StringUtil.hpp"
#include "MethodArguments.hpp"


/**
 * The center/centre method for the string class.
 *
 * @param _length The length to center in.
 * @param pad     The optional pad character
 *
 * @return The centered string.
 */
RexxString *RexxString::center(RexxInteger *_length, RexxString *pad)
{
    // this will be our final length
    size_t width = lengthArgument(_length, ARG_ONE);

    // the pad character is optional, with a blank default
    char padChar = optionalPadArgument(pad, ' ', ARG_TWO);
    size_t len = getLength();
    // if the width and the length are the same, we just return the target
    // string unchanged
    if (width == len)
    {
        return this;
    }

    // centered in zero characters?  This is a null string
    if (width == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // if the width is bigger than the length, we need to add pad characters
    if (width > len)
    {
        // half the pad is on the left side
        size_t leftPad = (width - len) / 2;
        // the remainder on the right, which also gets the extra if an odd number
        // is required
        size_t rightPad = (width - len) - leftPad;
        RexxString *retval = raw_string(width);

        StringBuilder builder(retval);
        // set left pad characters
        builder.pad(padChar, leftPad);
        // add the string data
        builder.append(getStringData(), len);
        // now the trailing pad chars
        builder.pad(padChar, rightPad);
        return retval;
    }
    // the request width is smaller than the input, so we have to truncate
    else
    {
        // we really only need to calculate the left side truncation
        size_t leftPad = (len - width) / 2;
        return new_string(getStringData() + leftPad, width);
    }
}


/**
 * The delstr() method of the string class.
 *
 * @param position The starting postion
 * @param _length  The length to delete.
 *
 * @return The string after the deletion.
 */
RexxString *RexxString::delstr(RexxInteger *position, RexxInteger *_length)
{
    size_t stringLen = getLength();
    size_t deletePos = optionalPositionArgument(position, 1, ARG_ONE);
    size_t deleteLen = optionalLengthArgument(_length, stringLen - deletePos + 1, ARG_TWO);

    // if the delete position is beyond the
    // length of the string, just return unchanged
    if (deletePos > stringLen)
    {
        return this;
    }

    // delete all characters?  This is a null string
    if (deletePos == 1 && deleteLen >= stringLen)
    {
        return GlobalNames::NULLSTRING;
    }

    // easier to do if origin zero
    deletePos--;

    size_t backLen = 0;
    // if we're not deleting up to or past the end, calculate the
    // size of the trailing section
    if (deleteLen < (stringLen - deletePos))
    {
        backLen = stringLen - (deletePos + deleteLen);
    }

    RexxString *retval = raw_string(deletePos + backLen);
    StringBuilder builder(retval);

    // copy any leading part, unless we're deleting from the
    // start of the string
    builder.append(getStringData(), deletePos);
    // add the trailing section
    builder.append(getStringData() + deletePos + deleteLen, backLen);

    return retval;
}


/**
 * The String insert method.
 *
 * @param newStrObj The string to insert.
 * @param position  The insert postion.
 * @param _length   The optional insert length.
 * @param pad       The optional padding character.
 *
 * @return The string with the new string inserted.
 */
RexxString *RexxString::insert(RexxString  *newStrObj, RexxInteger *position, RexxInteger *_length, RexxString  *pad)
{
    size_t targetLength = getLength();

    newStrObj = stringArgument(newStrObj, ARG_ONE);
    size_t newStringLength = newStrObj->getLength();

    // we're parsing this as a length argument because a postion of zero
    // is valid for insert
    size_t insertPosition = optionalLengthArgument(position, 0, ARG_TWO);
    size_t insertLength = optionalLengthArgument(_length, newStringLength, ARG_THREE);

    // default pad character is a blank
    char padChar = optionalPadArgument(pad, ' ', ARG_FOUR);
    size_t leadPad = 0;
    size_t frontLength;
    size_t backLength;

    // inserting at the beginning?  No leading pad required, no leading part
    // to copy, the back length is the entire string
    if (insertPosition == 0)
    {
        leadPad = 0;
        frontLength = 0;
        backLength = targetLength;
    }
    // insert position beyond the length?  We might need to insert some leading
    // pad characters, the front part is everything and there is no back part to copy
    else if (insertPosition >= targetLength)
    {
        leadPad = insertPosition - targetLength;
        frontLength = targetLength;
        backLength = 0;
    }

    // we're inserting in the middle.  No leading pad, the insert position
    // determines the length of the front and back sections
    else
    {
        leadPad = 0;
        frontLength = insertPosition;
        backLength = targetLength - insertPosition;
    }

    // we might need to truncate the inserted string if the specified length is
    // shorter than the inserted string
    newStringLength = std::min(newStringLength, insertLength);
    size_t padLength = insertLength - newStringLength;

    size_t resultLength = targetLength + insertLength + leadPad;
    RexxString *retval = raw_string(resultLength);
    StringBuilder builder(retval);

    // if we have a front length, copy it now
    builder.append(getStringData(), frontLength);

    // if there are leading pad characters required, add them now
    builder.pad(padChar, leadPad);

    // if we have new string data to copy, this is next
    builder.append(newStrObj->getStringData(), newStringLength);

    // and trailing pad required?
    builder.pad(padChar, padLength);

    // and the back length value
    builder.append(getStringData() + frontLength, backLength);
    return retval;
}



/**
 * Extract a substring relative to the left side of the target string.
 *
 * @param _length The length to extract.
 * @param pad     The pad character for padding out to the given length.
 *
 * @return The extracted string.
 */
RexxString *RexxString::left(RexxInteger *_length, RexxString *pad)
{
    size_t size = lengthArgument(_length, ARG_ONE);

    // default padd is a blank
    char padChar = optionalPadArgument(pad, ' ', ARG_TWO);

    // a request size if zero is a simple null string
    if (size == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    size_t length = getLength();
    RexxString *retval = raw_string(size);
    StringBuilder builder(retval);

    // cap the length copied from the existing string to its length
    size_t copyLength = std::min(length, size);

    // if we have data to copy, add to the result
    builder.append(getStringData(), copyLength);

    // if the requested length is longer than the string, we need to add
    // pad characters.

    if (size > length)
    {
        builder.pad(padChar, size - length);
    }
    return retval;
}


/**
 * Perform an overlay operation on a string.
 *
 * @param newStrObj The string being overlayed on the target string.
 * @param position  The overlay position.
 * @param _length   The optional length of the overlay.
 * @param pad       The optional pad character
 *
 * @return A new string with the overlay performed.
 */
RexxString *RexxString::overlay(RexxString *newStrObj, RexxInteger *position, RexxInteger *_length, RexxString  *pad)
{
    size_t targetLen = getLength();
    // make sure we have a real string value for the overlay
    newStrObj = stringArgument(newStrObj, ARG_ONE);
    size_t newLen = newStrObj->getLength();

    // the overlay postion defaults to the first character
    size_t overlayPos = optionalPositionArgument(position, 1, ARG_TWO);
    // the length default is the overlay string length
    size_t overlayLen = optionalLengthArgument(_length, newLen, ARG_THREE);
    // default pad is a blank
    char padChar = optionalPadArgument(pad, ' ', ARG_FOUR);

    size_t backPad = 0;

    // if the requested length is larger than the string length, we
    // need to pad
    if (overlayLen > newLen)
    {
        backPad = overlayLen - newLen;
    }
    // might be shorter than the string, so reduce the copy length to
    // the requested size
    else
    {
        newLen = overlayLen;
    }

    size_t frontPad = 0;
    // calculate the default split positions
    size_t frontLen = overlayPos - 1;
    size_t backLen = targetLen - (overlayPos + overlayLen - 1);

    // if the overlay position is beyond the length of the target
    // string, we need to add padding in front of the new string and
    // copy just the target string length
    if (overlayPos > targetLen)
    {
        frontPad = overlayPos - targetLen - 1;
        frontLen = targetLen;
    }

    // if the end of the overlay position extends past the
    // end of the target string, there's no trailing part to copy
    if (overlayPos + overlayLen > targetLen)
    {
        backLen = 0;
    }


    RexxString *retval = raw_string(frontLen + backLen + frontPad + overlayLen);
    StringBuilder builder(retval);

    // copy the front part
    builder.append(getStringData(), frontLen);
    // add any leading padding
    builder.pad(padChar, frontPad);
    // copy the overlay string (or portion of the overlay string)
    builder.append(newStrObj->getStringData(), newLen);
    // add any possible back padding
    builder.pad(padChar, backPad);
    // and finally the trailing section
    builder.append(getStringData() + overlayPos + overlayLen - 1, backLen);

    return retval;
}


/**
 * Replace a substring starting at a given position and
 * length with another string.  This operation is essentially
 * a delstr() followed by an insert() operation.
 *
 * @param newStrObj The replacement string
 * @param position  The replacement position (required)
 * @param _length   The length of string to replace (optional).  If omitted,
 *                  the length of the replacement string is used and this
 *                  is essentially an overlay operation.
 * @param pad       The padding character if padding is required.  The default
 *                  is a ' '
 *
 * @return A new instance of the string with the value replace.
 */
RexxString *RexxString::replaceAt(RexxString  *newStrObj, RexxInteger *position, RexxInteger *_length, RexxString  *pad)
{
    size_t targetLen = getLength();
    // the replacement value is required and must be a string
    RexxString *newStr = stringArgument(newStrObj, ARG_ONE);

    // the length of the replacement string is the default replacement length
    size_t newLen = newStr->getLength();
    // the overlay position is required
    size_t replacePos = positionArgument(position, ARG_TWO);
    // the replacement length is optional, and defaults to the length of the replacement string
    size_t replaceLen = optionalLengthArgument(_length, newLen, ARG_THREE);

    // we only pad if the start position is past the end of the string
    char padChar = optionalPadArgument(pad, ' ', ARG_FOUR);

    size_t padding = 0;
    size_t frontLen = 0;
    size_t backLen = 0;

    // the only time we need to pad is if the replacement position is past the
    // end of the string
    if (replacePos > targetLen)
    {
        padding = replacePos - targetLen - 1;
        frontLen = targetLen;
    }
    else
    {
        // this is within bounds, so we copy up to that position
        frontLen = replacePos - 1;
    }

    // is this within the bounds of the string?
    if (replacePos + replaceLen - 1 < targetLen)
    {
        // calculate the back part we need to copy
        backLen = targetLen - (replacePos + replaceLen - 1);
    }

    // allocate a result string
    RexxString *retval = raw_string(frontLen + backLen + padding + newLen);
    StringBuilder builder(retval);

    // add the front section
    builder.append(getStringData(), frontLen);

    // padding only happens if we've copied the entire front portion
    builder.pad(padChar, padding);

    // copy the replacement string
    builder.append(newStr->getStringData(), newLen);

    // the remainder, if there is any, gets copied after the
    // replacement string with no padding
    builder.append(getStringData() + replacePos + replaceLen - 1, backLen);
    return retval;
}


/**
 * Reverse the byte positions in a string object.
 *
 * @return The target string, reversed.
 */
RexxString *RexxString::reverse()
{
    size_t length = getLength();
    // if this is a null string or just a single character,
    // there's nothing to reverse
    if (length <= 1)
    {
        return this;
    }

    RexxString *retval = raw_string(length);
    char *string = retval->getWritableData();

    // copy from the end in the original
    const char *end = getStringData() + length - 1;

    // now perform the whole length copy
    while (length--)
    {
        *string++ = *end--;
    }

    return retval;
}


/**
 * Extract a substring of the target relative to the
 * right end.
 *
 * @param _length The extract length.
 * @param pad     The optional padding character.
 *
 * @return The extracted substring.
 */
RexxString *RexxString::right(RexxInteger *_length, RexxString  *pad)
{
    size_t size = lengthArgument(_length, ARG_ONE);

    char padChar = optionalPadArgument(pad, ' ', ARG_TWO);
    size_t sourceLength = getLength();

    // if the extraction length is zero, return a null string
    if (size == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // asking for the same size (not uncommon, really), no extraction necessary
    if (sourceLength == size)
    {
        return this;
    }

    RexxString *retval = raw_string(size);
    StringBuilder builder(retval);

    // the requested length might be longer than the target string, so
    // cap at that size
    size_t copyLength = std::min(sourceLength, size);
    size_t padLength = size - copyLength;

    // padding, if required, occurs before the extracted string piece
    builder.pad(padChar, padLength);
    // copy on the string portion, copying from the end of the string
    builder.append(getStringData() + sourceLength - copyLength, copyLength);
    return retval;
}


/**
 * Strip a set of leading and/or trailing characters from a string,
 * returning a new string value, or if unchanged, the original string.
 *
 * @param optionString The option indicating where to strip characters.
 * @param stripchar    The set of characters to strip.
 *
 * @return A string instance with the target characters removed.
 */
RexxString *RexxString::strip(RexxString *optionString, RexxString *stripchar)
{
    // get the option character
    char option = optionalOptionArgument(optionString, "BLT", STRIP_BOTH, ARG_ONE);

    // get the strip character set
    stripchar = optionalStringArgument(stripchar, OREF_NULL, ARG_TWO);

    // the default is to remove spaces and horizontal tabs
    const char *chars = stripchar == OREF_NULL ? " \t" : stripchar->getStringData();
    size_t charsLen = stripchar == OREF_NULL ? strlen(" \t") : stripchar->getLength();

    const char *front = getStringData();
    size_t length = getLength();

    if (option == STRIP_LEADING || option == STRIP_BOTH)
    {
        // loop while more string or we don't find one of the stripped characters
        while (length > 0)
        {
            if (!StringUtil::matchCharacter(*front, chars, charsLen))
            {
                break;
            }
            front++;
            length--;
        }
    }

    // need to strip trailing?
    if (option == STRIP_TRAILING || option == STRIP_BOTH)
    {
        // point to the end and scan backwards now
        const char *back = front + length - 1;
        while (length > 0)
        {
            if (!StringUtil::matchCharacter(*back, chars, charsLen))
            {
                break;
            }
            back--;
            length--;
        }
    }

    // if there is anything left, extract the remaining part
    if (length > 0)
    {
        // it's quite common that nothing was stripped, in which case we can
        // just return the original string instead of creating a new one
        return length == getLength() ? this : new_string(front, length);
    }
    else
    {
        // null string, everything stripped away
        return GlobalNames::NULLSTRING;
    }
}


/**
 * Extract a substring from a target string.
 *
 * @param position The starting position of the extracted string.
 * @param _length  The length to extract.
 * @param pad      A padding character for padding out to the length, if necessary.
 *
 * @return The extracted string value.
 */
RexxString *RexxString::substr(RexxInteger *position, RexxInteger *_length, RexxString  *pad)
{
    // use the common code shared with MutableBuffer
    return StringUtil::substr(getStringData(), getLength(), position, _length, pad);
}


/**
 * Extract a substring from a target string, using brackets
 * syntax.
 *
 * @param position The starting position of the extracted string.
 * @param _length  The length to extract.
 * @param pad      A padding character for padding out to the length, if necessary.
 *
 * @return The extracted string value.
 */
RexxString *RexxString::brackets(RexxInteger *position, RexxInteger *_length)
{
    // use the common code shared with MutableBuffer
    return StringUtil::substr(getStringData(), getLength(), position, _length);
}


/**
 * Extract a single character from a string object.
 * Returns a null string if the specified position is
 * beyond the bounds of the string.
 *
 * @param positionArg
 *               The position of the target  character.  Must be a positive
 *               whole number.
 *
 * @return Returns the single character at the target position.
 *         Returns a null string if the position is beyond the end
 *         of the string.
 */
RexxString *RexxString::subchar(RexxInteger *positionArg)
{
    // use the common extraction code.
    return StringUtil::subchar(getStringData(), getLength(), positionArg);
}

