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
/* REXX Kernel                                      StringClassConversion.cpp */
/*                                                                            */
/* REXX string conversion methods                                             */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "BufferClass.hpp"

#include "ActivityManager.hpp"
#include "StringUtil.hpp"
#include "MethodArguments.hpp"
#include "NumberStringClass.hpp"


/**
 * Convert the character string into the same string with the
 * characters converted into a Base64 encoding.
 *
 * @return The string reformatted into a Base64 encoding.
 */
RexxString *RexxString::encodeBase64()
{
    // if we're encoding a null string, the result is a
    // null string.
    size_t inputLength = getLength();
    if (inputLength == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // figure out the output string length (this will be roughly
    // 4/3 the length of the input string (3 characters will encode
    // into 4 digits)
    size_t outputLength = (inputLength / 3) * 4;

    // The encoding will always use 4-digit sequences, so if this
    // was not evenly divisible by 3, add a complete 4-digit piece
    // at the end.
    if (inputLength % 3 > 0)
    {
        outputLength += 4;
    }

    // build directly into an empty string object
    RexxString *retval = raw_string(outputLength);
    const char *source = getStringData();

    char *destination = retval->getWritableData();

    // loop through the entire string
    while (inputLength > 0)
    {
        size_t inc[3];    // digit accumulator
        int buflen = 0;
        // the encoding is done 3 characters at a time.
        for (int i = 0; i < 3; i++)
        {
            // we always do 3 characters, even at the end
            // of the string.  If we still have characters
            // left, grab that characters into our accumulator
            // buffer
            if (inputLength > 0)
            {
                // make sure we just get 8 bits
                inc[i] = *source & 0xff;
                inputLength--;
                source++;
                buflen++;
            }
            // this piece is just zero.  We also
            // do not add this to the buffer length
            else
            {
                inc[i] = '\0';
            }
        }
        if (buflen > 0)
        {
            // now perform the base64 conversion to the next 4 output string chars.
            // we are picking up 6 bits at a time from the 24 bits of input characters
            // and converting those bits to a single base-64 digit.

            // 6 bits from first character
            *destination = DIGITS_BASE64[ inc[0] >> 2 ];
            destination++;

            // remaining 2 bits from first character, plus 4 bits from the second character
            *destination = DIGITS_BASE64[ ((inc[0] & 0x03) << 4) | ((inc[1] & 0xf0) >> 4) ];
            destination++;

            // remaining bits from second char, plus 2 bits from the last character.  If we don't have
            // a character here, use "="
            *destination = (char) (buflen > 1 ? DIGITS_BASE64[ ((inc[1] & 0x0f) << 2) | ((inc[2] & 0xc0) >> 6) ] : '=');
            destination++;

            // the final 6 bits...again, using "=" if we did not have a character here at the end.
            *destination = (char) (buflen > 2 ? DIGITS_BASE64[ inc[2] & 0x3f ] : '=');
            destination++;
        }
    }
    return retval;
}


/**
 * Reverse the effect of an encodebase64 operation, converting
 * a string in Base64 format into a "normal" character string.
 * This supports Base64 encoding as described by RFC 2045, but
 * does not allow (ignore) characters outside of the base64 alphabet.
 * See https://tools.ietf.org/html/rfc2045#page-24
 *
 * @return The converted character string.
 */
RexxString *RexxString::decodeBase64()
{
    size_t inputLength = getLength();
    // a null string remains a null string.
    if (inputLength == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // to be a valid base64 encoding, the digits must be
    // a encoded in 4-character units.
    if (inputLength % 4 > 0)
    {
        reportException(Error_Incorrect_method_invbase64);
    }

    const char *source = getStringData();

    // figure out the string length.  The last one or two characters
    // might be the '=' placeholders, so we reduce the output
    // length if we have those.
    size_t outputLength = (inputLength / 4) * 3;
    if (*(source + inputLength - 1) == '=')
    {
        outputLength--;
    }
    if (*(source + inputLength - 2) == '=')
    {
        outputLength--;
    }

    // and allocate a result string for the decoding.
    RexxString *retval = raw_string(outputLength);
    char *destination = retval->getWritableData();

    // now loop through the input string 4 digits at a time.
    while (inputLength > 0)
    {
        for (int i = 0; i < 4; i++)
        {
            unsigned char ch = *source++;

            // first, find the matching character
            unsigned char digitValue = DIGITS_BASE64_LOOKUP[ch];
            // if we did not find a match, this could be
            // an end of buffer filler characters
            if (digitValue == (unsigned char)'\xff')
            {
                // if this is '=' and we're looking at
                // one of the last two digits, we've hit the
                // end
                if (ch == '=' && inputLength <= 4 && (i == 3 || (i == 2 && *source == '=')))
                {
                    break;
                }

                // found an invalid character
                reportException(Error_Incorrect_method_invbase64);
            }

            // digit value is the binary value of this digit.  Now, based
            // on which digit of the input set we're working on, we update
            // the values in the output buffer.  We only have 6 bits of
            // character data at this point.
            switch (i)
            {
                // first digit, all 6 bits go into the current position, shifted
                case 0:
                    *destination = digitValue << 2;
                    break;
                // second digit.  2 bits are used to complete the current
                // character, 4 bits are inserted into the next character
                case 1:
                    *destination |= digitValue >> 4;
                    destination++;
                    *destination = digitValue << 4;
                    break;
                // third digit.  4 bits are used to complete the
                // current character, the remaining 2 bits go into the next one.
                case 2:
                    *destination |= digitValue >> 2;
                    destination++;
                    *destination = digitValue << 6;
                    break;
                // last character of the set.  All 6 bits are inserted into
                // the current output position.
                case 3:
                    *destination |= digitValue;
                    destination++;
                    break;
            }
        }
        // reduce the length by the quartet we just processed
        inputLength -= 4;
    }

    return retval;
}


/**
 * Process the string C2X method/function
 *
 * @return The character string, converted into its hex equivalent.
 */
RexxString *RexxString::c2x()
{
    // a null string is still a null string
    size_t inputLength = getLength();
    if (inputLength == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // this is an easy calculation.  The output string will be twice as large.
    RexxString *retval = raw_string(inputLength * 2);
    const char *source = getStringData();

    char *destination = retval->getWritableData();

    // process the entire string length
    while (inputLength--)
    {
        char ch = *source++;
        // get upper nibble after shifting out lower nibble and do
        // logical ANDING with F to convert to integer then convert
        // to hex value and put it in destination
        *destination++ = intToHexDigit((ch>>4) & 0xF);

        // logical AND with F to convert lower nibble to integer
        // then convert to hex value and put it in destination
        *destination++ = intToHexDigit(ch  & 0xF);
    }

    return retval;
}


/**
 * Process the string D2C method/function
 *
 * @param _length The optional length argument.
 *
 * @return The converted string.
 */
RexxString *RexxString::d2c(RexxInteger *_length)
{
    // The string must be a valid number for this conversion, so get the
    // numberstring value and raise an error if not a valid number
    NumberString *numberstring = numberString();
    if (numberstring == OREF_NULL)
    {
        reportException(Error_Incorrect_method_d2c, this);
    }

    // number string does all the real magic here.
    return numberstring->d2xD2c(_length, true);
}


/**
 * Process the string D2X method/function
 *
 * @param _length The optional length argument.
 *
 * @return The converted string value.
 */
RexxString *RexxString::d2x(RexxInteger *_length)
{
    // The string must be a valid number for this conversion, so get the
    // numberstring value and raise an error if not a valid number
    NumberString *numberstring = numberString();
    if (numberstring == OREF_NULL)
    {
        reportException(Error_Incorrect_method_d2x, this);
    }

    // number string handles the conversion
    return numberstring->d2xD2c(_length, false);
}


/**
 * Process the string X2C method/function
 *
 * @return The converted string value.
 */
RexxString *RexxString::x2c()
{
    // null strings are always null string
    size_t inputLength = getLength();
    if (inputLength== 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // try to pack the data
    return StringUtil::packHex(getStringData(), inputLength);
}


/**
 * Process the string X2D method/function
 *
 * @param _length The output string length (required for negative values)
 *
 * @return The converted string value.
 */
RexxString *RexxString::x2d(RexxInteger *_length)
{
    // handled by a common routine
    return x2dC2d(_length, false);
}


/**
 * Common C2D processing routine
 *
 * @param _length The optional explicit length
 *
 * @return The converted string.
 */
RexxString *RexxString::c2d(RexxInteger *_length)
{
    // handled by a common routine
    return x2dC2d(_length, true);
}


/**
 * Common X2D/X2C processing routine
 *
 * @param _length The output length
 * @param type    The conversion type.  True is c2d, false is x2d.
 *
 * @return The converted string.
 */
RexxString *RexxString::x2dC2d(RexxInteger *_length, bool type )
{
    wholenumber_t currentDigits = number_digits();
    size_t stringLength = getLength();

    // get the result size...
    size_t resultSize = optionalLengthArgument(_length, stringLength, ARG_ONE);
    // a result size of zero always returns zero...that was easy
    if (resultSize == 0)
    {
        return (RexxString *)IntegerZero;
    }

    // use this string directly
    RexxString *string = this;
    // NOTE:  if this is a negative value, we directly modify the object
    // in question, so we need a non-constant pointer to the data.  We will be
    // performing the operation on a copy of the string (either a packed hex string
    // or just a temporary copy we will discard).
    char *stringPtr = getWritableData();
    // assume an even nibble position
    size_t nibblePosition = 0;

    // default to no negative values allowed.
    bool negative = false;

    // if we are dealing with character data, this is easier because
    // we don't need to validate any data or allow for blanks, etc.
    if (type == true)
    {
        // if we don't have an explicit length specified, we use the
        // input string length as the result size
        if (_length != OREF_NULL)
        {
            // if the requested size is less than or equal to the
            // length of the input string, then we need to worry about
            // negative values
            if (resultSize <= stringLength)
            {
                // move the string pointer to the requested top location
                stringPtr += stringLength - resultSize;
                // pretend we have a shorter string
                stringLength = resultSize;

                // if the high order bit is on, we are dealing with a negative number
                if (*stringPtr & 0x80)
                {
                    negative = true;

                    // work off of a copy of the string
                    string = (RexxString *)copy();
                    // and get a new pointer into the value
                    stringPtr = string->getWritableData() + getLength() - resultSize;
                }
            }
        }
    }
    // x2d function...we first need to pack the next string down to its character value
    // and process based on that
    else
    {
        string = StringUtil::packHex(stringPtr, stringLength);
        // redo everything using the packed length
        stringLength = string->getLength();
        stringPtr = string->getWritableData();
        // if a length is specified, we need to check on whether this is negative.
        if (_length != OREF_NULL)
        {
            // this is a little more complicated that the c2d case.  The
            // specified length is the length of the hex characters, so we
            // need to be checking in a specific nibble in the packed hex string.
            size_t bytePosition = resultSize / 2;
            nibblePosition = resultSize % 2;

            // now we have a byte length for the target length.
            resultSize = bytePosition + nibblePosition;
            // if the string is at least as long as the result, then we
            // can have a negative value.
            if (resultSize <= stringLength)
            {
                // step to the start position specified by the length
                stringPtr += stringLength - resultSize;
                stringLength = resultSize;

                // where we check depends on whether the specified length was odd or even
                if ((nibblePosition != 0 && *stringPtr & 0x08) ||
                    (nibblePosition == 0 && *stringPtr & 0x80))
                {
                    negative = true;
                }
            }
        }
    }

    // if this is a negative value, we perform the 2s complement negation operation
    // in place.  Note that we are working off of a copy of the original data, so this
    // is fine.
    if (negative)
    {
        char *scan = stringPtr;
        size_t tempSize = stringLength;

        // reverse each byte using an exclusive OR
        while (tempSize--)
        {
            *scan = *scan ^ 0xff;
            scan++;
        }

        // point to the least significant byte (the last byte of the string)
        scan = stringPtr + stringLength - 1;
        tempSize = stringLength;

        // now we need to add 1 to the number, which might end up carrying
        // past the first byte.  We cannot get a carry all the way to the end,
        // since the only string that would allow that is all zeros, but we would
        // not be negating that string because it doesn't have the high order bit
        // set.
        while (tempSize--)
        {
            int ch = (*scan & 0xff);
            ch++;
            // if this did not carry, we're done (yay!)
            if (ch <= 0xff)
            {
                *scan = ch;
                break;
            }
            // set this byte to 0 and continue adding 1 until we
            // don't have a carry any longer.
            else
            {
                *scan = 0;
                scan--;
            }
        }
    }

    // if we had an odd number of nibbles, make sure the top nibble is
    // zeroed out.  This can only happen with X2D, but we want to make
    // sure no bits in the other nibble can influence the result.
    // (but only do this if the result size doesn't exceed the string size)
    if (resultSize <= stringLength && nibblePosition != 0)
    {
        *stringPtr &= 0x0f;
    }

    char *scan = stringPtr;

    // all of this is done using the current digits setting, so get a buffer with
    // sufficient space to perform the math
    BufferClass *buffer = new_buffer(currentDigits + NumberString::OVERFLOWSPACE + 1);
    // we start building from the end forward
    char *accumulator = buffer->getData() + currentDigits + NumberString::OVERFLOWSPACE;
    // make sure the buffer is cleared out
    memset(buffer->getData(), '\0', currentDigits + NumberString::OVERFLOWSPACE + 1);
    char *highDigit = accumulator - 1;

    // now perform the arithmetic and accumulate this
    while (stringLength--)
    {
        // ok, so for each nibble, we add to the accumulator under base 10, then
        // we multiply by 16 to perform a shift.
        int ch = *scan++;

        highDigit = NumberString::addToBaseTen((ch & 0xf0) >> 4, accumulator, highDigit);
        // multiply by 16
        highDigit = NumberString::multiplyBaseTen(accumulator, highDigit);
        // get the new accumulator length...if we're beyond the current digits
        // value, then we have an error
        wholenumber_t decLength = (accumulator - highDigit);
        if (decLength > currentDigits)
        {
            reportException(type == true ? Error_Incorrect_method_c2dbig : Error_Incorrect_method_x2dbig, currentDigits);
        }

        // now repeat for the lower part of the nibble
        highDigit = NumberString::addToBaseTen(ch & 0x0f, accumulator, highDigit);
        // if this is not the last byte, do the multiply to prepare for the next one
        if (stringLength != 0)
        {
            highDigit = NumberString::multiplyBaseTen(accumulator, highDigit);
        }
        // need to perform the overflow check again
        decLength = (accumulator - highDigit);
        if (decLength > currentDigits)
        {
            reportException(type == true ? Error_Incorrect_method_c2dbig : Error_Incorrect_method_x2dbig, currentDigits);
        }
    }

    // get accumulator length...this will be our final result length
    wholenumber_t decLength = (accumulator - highDigit);

    // we can return this result as a RexxInteger if we have at most
    // Numerics::REXXINTEGER_DIGITS digits
    // no need to check the current numeric digits setting, as C2D/X2D raises
    // Error 93.935/6:  X/C2D result is not a valid whole number with NUMERIC DIGITS nn
    // for any result outside of the current digits setting
    if (decLength <= Numerics::REXXINTEGER_DIGITS)
    {
      return (RexxString *)new_integer(negative, highDigit + 1, decLength, 0);
    }

    // now we need to turn the math digits into real characters for the result
    wholenumber_t tempLength = decLength;
    scan = highDigit + 1;
    while (tempLength--)
    {
        *scan = *scan + '0';
        scan++;
    }

    resultSize = decLength;
    // if we have a negative result, add an extra character to the result for the sign.
    if (negative)
    {
        resultSize++;
    }
    RexxString *retval = raw_string(resultSize);
    scan = retval->getWritableData();
    // add the sign if needed
    if (negative)
    {
        *scan++ = '-';
    }
    // copy the rest of the data
    memcpy(scan, accumulator - decLength + 1, decLength);
    return retval;
}


/**
 * Perform a B2X conversion on a character string.
 *
 * @return The converted string value.
 */
RexxString *RexxString::b2x()
{
    // a null bit string converts to a null string
    if (isNullString())
    {
        return GlobalNames::NULLSTRING;
    }

    // validate the string content, getting a bit count back.
    size_t bits = StringUtil::validateGroupedSet(getStringData(), getLength(), RexxString::DIGITS_BIN_LOOKUP, 4, false);
    // every 4 bits will be one hex character in the result
    RexxString *retval = raw_string((bits + 3) / 4);

    char *destination = retval->getWritableData();
    const char *source = getStringData();
    size_t length = getLength();

    // we know the string conforms to the rules for the string, so we just
    // need to process all of the bits and convert
    while (bits > 0)
    {
        char nibble[4];
        // this will really on impact the first nibble, since we will be working
        // in groups of 4 after that.  Find out how many bit digits we need to
        // process for the next nibble
        size_t excess = bits % 4;
        // zero means we have a full nibble to process
        if (excess == 0)
        {
            excess = 4;
        }
        // fill the nibble with zero bits for the non-used ones
        else
        {
            memset(nibble, '0', 4);
        }
        size_t jump;        // string movement offset
        // get the next nibble worth of characters
        StringUtil::copyGroupedChars(&nibble[0] + (4 - excess), source, length, excess, RexxString::DIGITS_BIN_LOOKUP, jump);
        // insert into the destination as a hex character
        *destination++ = StringUtil::packNibble(nibble);
        source += jump;
        length -= jump;
        // we should be working with multiples of 4 after the first one.
        bits -= excess;
    }
    return retval;
}


/**
 * A hex-to-binary conversion of a character string value.
 *
 * @return The converted string.
 */
RexxString *RexxString::x2b()
{
    // a null string converts to a null string
    if (getLength() == 0)
    {
        return GlobalNames::NULLSTRING;
    }
    // validate the content and grouping of the string, returning the count of set characters
    size_t nibbles = StringUtil::validateGroupedSet(getStringData(), getLength(), RexxString::DIGITS_HEX_LOOKUP, 2, true);
    // every hex nibble will expand to 4 bit characters
    RexxString *retval = raw_string(nibbles * 4);

    char *destination = retval->getWritableData();
    const char *source = getStringData();

    // process all of the nibbles and unpack into the result
    while (nibbles > 0)
    {
        // if not a white space character, we're at a hex digit
        char ch = *source++;
        if (ch != ch_SPACE && ch != ch_TAB)
        {
            // convert to a decimal number first, then unpack that into the nibble
            int val = RexxString::hexDigitToInt(ch);
            StringUtil::unpackNibble(val, destination);
            destination += 4;
            nibbles--;
        }
    }
    return retval;
}
