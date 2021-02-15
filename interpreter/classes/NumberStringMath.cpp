/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* Arithemtic function for the NumberString Class                             */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "NumberStringClass.hpp"
#include "ArrayClass.hpp"
#include "Activity.hpp"
#include "BufferClass.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"


/**
 * Convert a valid numberstring to a hex or character string.
 *
 * @param length The output length (required for negative
 *               numbers).
 * @param type   The type of conversion (hex or character)
 *
 * @return The converted value.
 */
RexxString *NumberString::d2xD2c(RexxObject *length, bool type)
{
    // get the target length
    wholenumber_t resultSize = optionalLengthArgument(length, SIZE_MAX, ARG_ONE);
    wholenumber_t currentDigits = number_digits();
    wholenumber_t targetLength = digitsCount;

    // already larger than the current digits setting?  That's an
    // error
    if (numberExponent + digitsCount > currentDigits)
    {
        reportException(type ? Error_Incorrect_method_d2c : Error_Incorrect_method_d2x, this);
    }

    // if this has decimals, we need to see if we have non-zero decimals.
    if (hasDecimals())
    {
        // if we have non-zero decimals, this is an error.  This must be a whole
        // number.
        if (hasSignificantDecimals(currentDigits))
        {
            reportException(type ? Error_Incorrect_method_d2c : Error_Incorrect_method_d2x, this);
        }
        // adjust the scan length to eliminate the decimal part
        targetLength += numberExponent;
    }

    // if the value is negative and we don't have an explicit size, this is an error
    if (isNegative() && resultSize == SIZE_MAX)
    {
        reportException(Error_Incorrect_method_d2xd2c);
    }

    wholenumber_t bufferLength = currentDigits + OVERFLOWSPACE;

    if (resultSize != SIZE_MAX)
    {
        // if this is the character function, we need more space, so double it
        if (type)
        {
            resultSize += resultSize;
        }
        bufferLength = std::max(currentDigits, resultSize) + OVERFLOWSPACE;
    }

    Protected<BufferClass> target = new_buffer(bufferLength);
    char *scan = numberDigits;

    // we start accumulating from the end of the buffer
    char *accumulator = target->getData() + bufferLength - 2;
    // and this is our high water mark for the math so far.
    char *highDigit = accumulator - 1;
    // clear the buffer before we start
    memset(target->getData(), '\0', bufferLength);

    // now process all of the digits in the integer portion of the number string.
    while (targetLength--)
    {
        // add to the buffer using base 16 math
        highDigit = addToBaseSixteen(*scan++, accumulator, highDigit);
        if (targetLength != 0)
        {
            // multiply by base 16 to shift everything
            highDigit = multiplyBaseSixteen(accumulator, highDigit);
        }
    }

    // we have extra digits to worry about, so we need to
    if (numberExponent > 0)
    {
        // we need to multiply the number by 16 for each of the excess values.
        // Note that normally we would not multiply for the last one, but we
        // skipped the last one while processing the digits, so this works out
        // to be the same.
        for (wholenumber_t i = 0; i < numberExponent; i++)
        {
            // we additional multiplications required
            highDigit = multiplyBaseSixteen(accumulator, highDigit);
        }
    }
    // this is our current length in terms of hex digits
    wholenumber_t hexLength = accumulator - highDigit;

    // this is our default padding character.  We need to switch this
    // if the value is negative
    char padChar = '0';

    if (isNegative())
    {
        // we pad with foxes to perform sign extending
        padChar = 'F';
        scan = accumulator;
        // start with the back of the result and transform any zeros
        // into foxes.  This is basically the equivalent of subtracting
        // 1 with a wrap and then carrying the subtraction out.  The first
        // non-zero byte is where the borrowing stops.
        while (*scan == 0)
        {
            *scan-- = 0xf;
        }

        // subtract a one from the first non-zero digit
        *scan = *scan - 1;

        // now we need to invert all of the bits for the result length.
        scan = accumulator;
        while (scan > highDigit)
        {
            *scan = (char)(*scan ^ (unsigned)0x0f);
            scan--;
        }
    }

    // now we need to convert all of the result digits into printable characters
    scan = accumulator;
    while (scan > highDigit)
    {
        *scan = RexxString::intToHexDigit(*scan);
        scan--;
    }
    scan = highDigit + 1;

    // now calculate the final result size.
    if (resultSize == SIZE_MAX)
    {
        resultSize = hexLength;
    }

    size_t padSize = 0;

    // we might actually need to truncate
    if (resultSize < hexLength)
    {
        // no padding added, we are truncating on the left end, so
        // shift the pointer by the excess.
        padSize = 0;
        scan += hexLength - resultSize;
        hexLength = resultSize;
    }
    // we have possible padding
    else
    {
        padSize = resultSize - hexLength;
    }

    // do we need to pad?
    if (padSize != 0)
    {
        // step the result pointer back and add the padding character (either '0' or 'F',
        // depending on the sign of the number)
        scan -= padSize;
        memset(scan, padChar, padSize);
    }

    // if this is d2c, we need to pack the hex digits into character values.
    if (type == true)
    {
        return StringUtil::packHex(scan, resultSize);
    }
    // for d2x, we can just create the string version now
    else
    {
        return new_string(scan, resultSize);
    }
}


/**
 * Process the MAX and MIN builtin functions and methods
 *
 * @param args      The variable length array of arguments.
 * @param argCount  The count of arguments.
 * @param operation Indicates which function we are processing.
 *
 * @return The largest or smallest of the set of numbers.
 */
NumberString *NumberString::maxMin(RexxObject **args, size_t argCount, ArithmeticOperator operation)
{
    const char *methodName = operation == OT_MAX ? "MAX" : "MIN";
    // this is the result from comp() that indicates we have
    // a greater comparison.
    wholenumber_t compResult = operation == OT_MAX ? 1 : -1;


    wholenumber_t saveFuzz = number_fuzz();
    wholenumber_t saveDigits = number_digits();

    // NB:  The min and max methods are defined as rounding the values to the
    // current digits settings, which bypasses the LOSTDIGITS condition
    // The target number gets rounded also
    Protected<NumberString> maxminobj = prepareNumber(saveDigits, ROUND);

    // if there are no other numbers to compare against, this is the result.
    if (argCount == 0)
    {
        return maxminobj;
    }

    // now looop through all of the arguments
    for (size_t arg = 0; arg < argCount; arg++)
    {
        // get the next argument.  Omitted arguments are not allowed
        RexxObject *nextObject = args[arg];
        if (nextObject == OREF_NULL)
        {
            reportException(Error_Incorrect_call_noarg, methodName, arg + 1);
        }

        // try to get as a numberstring object
        NumberString *compobj = nextObject->numberString();
        if (compobj != OREF_NULL)
        {
            // we need this rounded to the current digits setting.
            compobj = compobj->prepareOperatorNumber(saveDigits, saveDigits, ROUND);

            // if we had the desired comparison trigger,
            // we have a new min or max object.  Swap it
            // (NOTE, comp does not necessarily return 1 or -1 because of
            // platform differences with memcmp.  This funny comparison ensures
            // we get the correct result
            wholenumber_t rc = compobj->comp(maxminobj, saveFuzz);

            if (rc < 0 && compResult < 0)
            {
                maxminobj = compobj;
            }
            else if (rc > 0 && compResult > 0)
            {
                maxminobj = compobj;
            }
        }
        // we have an invalid object type...that's an error
        else
        {
            reportException(Error_Incorrect_method_number, arg + 1, args[arg]);
        }
    }

    // return the max or min object.  This is already rounded to the
    // current digits ans is ready to go.
    return maxminobj;
}


/**
 * Adjust a a number string object to correct NUMERIC
 * DIGITS setting
 *
 * @param numPtr The first digit of the number (not the rounding
 *               position).
 */
void NumberStringBase::mathRound(char *numPtr)
{
    // point one past the last digit of the number string.  This is
    // the point we round from.
    wholenumber_t resultDigits = digitsCount;
    numPtr += digitsCount;

    // if this digit is greater than 5, we have to round
    if (*numPtr-- >= 5 )
    {
        // we have a carry out, which is our trigger to keep rounding
        int carry = 1;
        // loop through all of the digits while we still have carry outs
        // happening.
        while (carry != 0 && resultDigits-- > 0)
        {
            // if the current digit is a 9, make it zero and continue rounding.
            if (*numPtr == 9)
            {
                *numPtr-- = 0;
            }
            else
            {
                // adjust for the carry value and turn off the carray flag
                // to stop the rounding process
                *numPtr = *numPtr + 1;
                carry = 0;
            }
        }

        // did we carry all the way out?  All of
        // our digits became zero.  We replace the
        // first digit of the number with a 1 and
        // bump the exponent value up one for the shift
        if (carry != 0)
        {
            *++numPtr = 1;
            numberExponent += 1;
        }
    }

    // do some overflow checks
    checkOverflow();
}


/**
 * Adjust the precision of a number to the digits setting
 * it was created under.
 */
void NumberString::adjustPrecision()
{
    // is the length of the number too big?
    if (digitsCount > createdDigits)
    {
        // perform truncation and rounding.
        truncateToDigits(createdDigits, numberDigits, true);
    }
    // it is possible that we have a zero number not marked as zero
    else if (numberDigits[0] == 0 && digitsCount == 1)
    {
        // make sure it is fully zero
        setZero();
    }
    // check to make sure this is really a valid number
    else
    {
        checkOverflow();
    }
}


/**
 * Determine high order bit position of an unsigned
 * number setting.
 *
 * @param number The number we're checking
 *
 * @return The index position of the high bit.
 */
size_t NumberString::highBits(size_t number)
{
    // if the number is zero, there is no high bit.
    if (number == 0)
    {
        return 0;
    }

    size_t highBit = SIZEBITS;

    // keep shifting the number until we find the high bit on
    while ((number & HIBIT) == 0)
    {
        number <<= 1;
        highBit--;
    }

    return highBit;
}


/**
 * Remove all leading zeros from a number after an arithmetic
 * operation.
 *
 * @param accumPtr The pointer to the start of the number digits
 *
 * @return The new first digit pointer.  The length of the number
 *         has been adjusted accordingly.  This does not touch
 *         the number exponent.
 */
char *NumberStringBase::stripLeadingZeros(char *accumPtr)
{
    // we only strip down to the last zero...we'll leave a single one for
    // a real zero result.
    while (*accumPtr == 0 && digitsCount > 1)
    {
        accumPtr++;
        digitsCount--;
    }
    return accumPtr;
}


/**
 * Adjust the precision of a number to the given digits.  Also
 * copies the data to the start of the number string buffer
 * after adjustment.
 *
 * @param resultPtr The start of the digits data to
 *                  process.
 * @param digits    The precision for making the adjustment.
 */
void NumberString::adjustPrecision(char *resultPtr, wholenumber_t digits)
{
    // is the length of the number too big?
    if (digitsCount > digits)
    {
        // adjust the length and the exponent for the truncation amount
        wholenumber_t extra = digitsCount - digits;
        digitsCount = digits;
        numberExponent += extra;
        // round the adjusted number, if necessary
        mathRound(resultPtr);
    }

    // strip any leading zeros
    resultPtr = stripLeadingZeros(resultPtr);
    // now move into the object
    memcpy(numberDigits, resultPtr, digitsCount);

    // make sure this has the correct digits creation settings
    setNumericSettings(digits, number_form());

    // if this reduced to zero, make it exactly zero
    if (*resultPtr == 0 && digitsCount == 1)
    {
        setZero();
    }
    else
    {
        // perform overflow checks on the result
        checkOverflow();
    }
    return;
}


/**
 * Truncate a number to the current digits setting, with optional rounding.
 *
 * @param digits The target digits (already determined that truncation is needed)
 * @param round
 */
void NumberStringBase::truncateToDigits(wholenumber_t digits, char *digitsPtr, bool round)
{
    numberExponent += digitsCount - digits;
    digitsCount = digits;
    if (round)
    {
        mathRound(digitsPtr);
    }
}


/**
 * Create new copy of supplied object and make sure the
 * number is computed to correct digits setting
 *
 * @param NumberDigits
 *                 The precision we're at.
 * @param rounding true if we should round, false if we just truncate.
 *
 * @return The new number that is now safe for modification and also
 *         rounded to the current digits setting.
 */
NumberString *NumberString::prepareNumber(wholenumber_t digits, bool rounding)
{
    // make a copy of the number with a fresh setting of numeric creation information.
    NumberString *newObj = clone();
    if (newObj->digitsCount > digits)
    {
        // NOTE:  This version does NOT raise a LOSTDIGITS condition, since it
        // is used for formatting results from functions that are used to create
        // intentionally shortened numbers.

        // perform truncation and rounding.
        newObj->truncateToDigits(digits, newObj->numberDigits, rounding);
    }
    // update the numeric metrics for the creation time
    newObj->setNumericSettings(digits, number_form());
    return newObj;
}


/**
 * Prepare an operator numberstring to a given length,
 * raising a LOSTDIGITS condition if the starting number
 * will cause lost digits
 *
 * @param targetLength
 *                 The target preparation length (>= numberDigits)
 * @param digits The digits setting used to determine LOSTDIGITS
 *               conditions
 * @param rounding Inidicates whether rounding is to be performed if the
 *                 target object is longer than the targetLength
 *
 * @return A new number object no longer than the target length.
 */
NumberString *NumberString::prepareOperatorNumber(wholenumber_t targetLength, wholenumber_t digits, bool rounding)
{
    // clone the starting number
    NumberString *newObj = clone();
    if (newObj->digitsCount > digits)
    {
        // if we are going to lose digits because we're longer than the
        // current digits setting, raise the LOSTDIGITS condition
        reportCondition(GlobalNames::LOSTDIGITS, (RexxString *)newObj);
        // if we're longer than the target length, chop and potentially round
        if (newObj->digitsCount > targetLength)
        {
            newObj->truncateToDigits(targetLength, newObj->numberDigits, rounding);
        }
    }
    // make sure this has the correct settings
    newObj->setNumericSettings(digits, number_form());
    return newObj;
}


/**
 * Add or subtract two normalized numbers
 *
 * @param other     The other number for the operation.
 * @param operation The operation identifier (addtion or subtraction)
 * @param digits    The digits this operation is being performed under.
 *
 * @return The operation result.
 */
NumberString *NumberString::addSub(NumberString *other, ArithmeticOperator operation, wholenumber_t digits)
{
    // when performing the operations, we allow up to digits + 1 digits to particpate
    wholenumber_t maxLength = digits  + 1;

    // get local variables of the input object metrics since we might
    // need to copy the objects and adjust some settings.
    NumberString *left = this;
    NumberString *right = other;
    wholenumber_t leftExp = left->numberExponent;
    wholenumber_t rightExp = right->numberExponent;

    wholenumber_t leftLength = left->digitsCount;
    wholenumber_t rightLength = right->digitsCount;

    // now we need to check for truncation, including raising LOSTDIGITS
    // conditions if we do need to truncate
    if (leftLength > digits)
    {
        // raise a numeric condition, which might not return
        reportCondition(GlobalNames::LOSTDIGITS, (RexxString *)this);

        // adjust this for the maximum length
        if (leftLength > maxLength)
        {
            leftExp += leftLength - maxLength;
            leftLength = maxLength;
        }
    }

    // and repeat for the right number
    if (rightLength > digits)
    {
        reportCondition(GlobalNames::LOSTDIGITS, (RexxString *)other);
        if (rightLength > maxLength)
        {
            rightExp += rightLength - maxLength;
            rightLength = maxLength;
        }
    }

    // get the minimum exponent from these values
    wholenumber_t minExp = std::min(leftExp, rightExp);

    // we create an adjusted exponent that gives a good relative size indicator
    // one of these will be zero, the other will be the difference in magniture
    // between the two numbers
    wholenumber_t adjustedLeftExp  = leftExp - minExp;
    wholenumber_t adjustedRightExp = rightExp - minExp;

    // we might be able to quickly return in certain circumstances.
    NumberString *result = OREF_NULL;

    // if the left number is zero, we can just return the right number (with some
    // adjustments.
    if (left->isZero())
    {
        result = right;
    }
    // if the right number is zero, we can just return the left
    else if (right->isZero())
    {
        result = left;
    }

    // is the size difference between these numbers so large that the
    // right number cannot impact the calculation result?  We can just
    // use the left number without subtracting
    else if ((adjustedLeftExp + leftLength) > (rightLength + digits))
    {
        result = left;
    }
    // same test for the right side
    else if ((adjustedRightExp + rightLength) > (leftLength + digits))
    {
        result = right;
    }

    // if we have a fast path exit, then we can do a quick return without
    // performing any actual operation.  We return a copy of the value, and
    // we might have to change the sign of this is a subtraction operation.
    if (result != OREF_NULL)
    {
        // copy the original number
        NumberString *resultCopy = result->clone();

        // if this is a subtraction operation, we need to negate the sign
        // of the result number.
        if ((result == right) && (operation == OT_MINUS))
        {
            resultCopy->numberSign = -resultCopy->numberSign;
        }
        // get into the correct precision and record the settings at the
        // time this was created.
        resultCopy->setupNumber();
        return resultCopy;
    }

    // subtraction is basically addition with the negation of the number.
    // so if we're subtracting, negate the sign of the right hand side.
    wholenumber_t rightSign = operation == OT_MINUS ? -right->numberSign : right->numberSign;

    // if two signs are not equal, check to see if the specifics of the numbers
    // are equal...we might be able to return an exact zero
    if (rightSign != left->numberSign)
    {
        // these are equal if the lengths, exponents, and all digits are the same.
        // this is more common than you might think.  We can make this zero immediately.
        if (((leftLength == rightLength) && (leftExp == rightExp)) &&
            !(memcmp(left->numberDigits, right->numberDigits, leftLength)) )
        {
            return new_numberstring("0", 1);
        }
        // if the signs are different, make this a subtraction.
        operation = OT_MINUS;
    }
    else
    {
        // the signs are equal, so handle as an addition.
        operation = OT_PLUS;
    }

    // get a result object big enough to handle our maximum size result
    result = new (maxLength) NumberString (maxLength);

    // make sure all values are zero to start with
    result->numberSign = 0;
    result->numberExponent = 0;
    result->digitsCount = 0;


    // point to the end of both numbers.
    char *leftPtr = left->numberDigits + leftLength - 1;
    char *rightPtr = right->numberDigits + rightLength - 1;

    // See if we need oto adjust the number with respect to current
    // Digits setting.  Compute the amount we may need to
    // adjust for each number. Using the adjusted exponents ....
    // a value of 0 or less means we are OK.
    wholenumber_t adjustedLeftDigits = (leftLength + adjustedLeftExp) - maxLength;
    wholenumber_t adjustedRightDigits = (rightLength + adjustedRightExp) - maxLength;
    wholenumber_t adjustDigits = 0;
    // Do we need to adjust any numbers?
    if (adjustedLeftDigits > 0 || adjustedRightDigits > 0 )
    {
        // we need some adjustment on the digit alignments, figure
        // out which way
        if (adjustedLeftDigits >= adjustedRightDigits)
        {
            adjustDigits = adjustedLeftDigits;
        }
        else
        {
            adjustDigits = adjustedRightDigits;
        }

        // one of these adjusted exponents will be zero, which is the smaller
        // of the two numbers because the adjustment was made using the smaller of
        // the two exponents.  This number will contribute less to the sum or difference.
        if (adjustedLeftExp != 0)
        {
            // the left number has the larger exponent.  Do we need to adjust
            // for more than the adjusted exponent here?
            if (adjustDigits < adjustedLeftExp)
            {
                // adjust the exponent by the digits adjustment (obtained originally by
                // subtracting the two exponents...)
                adjustedLeftExp -= adjustDigits;
                // rightPtr points to the end of the right number, so shorten by the
                // asjustment amount and also reduce the length and the real exponent.
                rightPtr = rightPtr - adjustDigits;
                rightExp += adjustDigits;
                rightLength -= adjustDigits;
                // no longer need the adjustment amount
                adjustDigits = 0;
            }
            // the adjustment is larger than the exponent difference, so
            // adjust the right information by the exponent difference.
            else
            {
                rightPtr = rightPtr - adjustedLeftExp;
                rightExp += adjustedLeftExp;
                // we reduce the length to add by the full adjustment amound
                rightLength -= adjustDigits;
                adjustDigits -= adjustedLeftExp;
                // the adjusted left exponent is no longer needed
                adjustedLeftExp = 0;
            }
        }
        // ok, the right exponent is larger
        else if (adjustedRightExp != 0)
        {
            // same process we just did for the other case
            if (adjustDigits < adjustedRightExp)
            {
                adjustedRightExp -= adjustDigits;
                leftPtr = leftPtr - adjustDigits;
                leftExp += adjustDigits;
                leftLength -= adjustDigits;
                adjustDigits = 0;
            }
            else
            {

                leftPtr = leftPtr - adjustedRightExp;
                leftExp += adjustedRightExp;
                leftLength -= adjustDigits;
                adjustDigits -= adjustedRightExp;
                adjustedRightExp = 0;
            }
        }

        // if we still have adjustments to make after all of that,
        if (adjustDigits)
        {
            // reduce the length of both numbers by the adjustment amount and
            // increase both exponents accordingly
            leftExp += adjustDigits;
            leftPtr = leftPtr - adjustDigits;
            rightExp += adjustDigits;
            rightPtr = rightPtr - adjustDigits;
        }
    }

    // we can allocate here, if we're in a "reasonable" digits setting.
    // we can go up to 36 digits with the default, which probably gets most
    // cases.
    char resultBufFast[FAST_BUFFER * 2 + 1];
    char *resultBuffer = resultBufFast;
    Protected<BufferClass> outputBuffer;

    // if the digits are really big, then we need to allocate a larger buffer.
    // just allocate a buffer object and allow it to get garbage collected after
    // we're done.
    if (digits > FAST_BUFFER)
    {
        outputBuffer = new_buffer(digits * 2 + 1);
        resultBuffer = outputBuffer->getData();
    }

    // point to the end, which will be the start of where we accumulate
    char *resultPtr = resultBuffer + digits * 2;

    // finally time to do some arithmetic.  We have different paths based on
    // whether this is addition or subtraction
    if (operation == OT_PLUS)
    {
        wholenumber_t carry = 0;

        // we are doing addition, which means both signs are the same.  Take the
        // result sign from the left number.
        result->numberSign = left->numberSign;
        // we need check and see if there are values in the adjusted exps
        // that is do we pretend there  are extra zeros at the end
        // of the numbers?


        // if the left exponent needs adjusting, we need to have
        // zeros added.  Handle this by moving digits of the right number into
        // the result.  For example,
        // xxxxxxx000     <- adjustedLeftExp = 3
        //  yyyyyyyyy     <- adjustedRightExp = 0
        if (adjustedLeftExp != 0)
        {
            // while we have exponent left
            while (adjustedLeftExp--)
            {
                // if there is still data to move, copy from the right number and shorten it
                if (rightPtr >= right->numberDigits)
                {
                    *resultPtr-- = *rightPtr--;
                }
                // move in a zero if we've run out of right digits.
                else
                {
                    *resultPtr-- = '\0';
                }
                // note the length addition in the result number
                result->digitsCount++;
            }
            // the right is smaller, so use the right exponent for the result
            result->numberExponent = rightExp;
        }
        // the right exponent is greater...same process, but copying left digits
        // into the result.
        else if (adjustedRightExp != 0)
        {
            while (adjustedRightExp--)
            {
                if (leftPtr >= left->numberDigits)
                {
                    *resultPtr-- = *leftPtr--;
                }
                else
                {
                    *resultPtr-- = '\0';
                }
                result->digitsCount++;
            }
            result->numberExponent = leftExp;
        }
        // these have same exponent, so take the exponent from the left
        else
        {
            result->numberExponent = leftExp;
        }

        // ok, while both numbers still have digits, we finally get to add something
        // together!
        while ( (leftPtr >= left->numberDigits) && (rightPtr >= right->numberDigits))
        {
            // add the two current digits with possible carry bit, and update the
            // left and right ptr to point to next (actually previous) digit.
            wholenumber_t addDigit = carry + (*leftPtr--) + *(rightPtr--);
            // check for a carry on this addition
            if (addDigit >= 10)
            {
                carry = 1;
                addDigit -= 10;
            }
            else
            {
                carry = 0;
            }
            // move this into the result and bump the result length
            *resultPtr-- = (char)addDigit;
            result->digitsCount++;
        }


        // one of the numbers is finished, but we might have digits in the
        // other number to handle.

        // left number?  we need to handle this like an addition to zero,
        // with carry outs still possible.
        while (leftPtr >= left->numberDigits)
        {
            wholenumber_t addDigit = carry + (*leftPtr--);
            if (addDigit >= 10)
            {
                carry = 1;
                addDigit -= 10;
            }
            else
            {
                carry = 0;
            }
            *resultPtr-- = (char)addDigit;
            result->digitsCount++;
        }

        // and repeat with the right number
        while (rightPtr >= right->numberDigits)
        {
            wholenumber_t addDigit = carry + (*rightPtr--);
            if (addDigit >= 10)
            {
                carry = 1;
                addDigit -= 10;
            }
            else
            {
                carry = 0;
            }
            *resultPtr-- = (char)addDigit;
            result->digitsCount++;
        }

        // if we still have a carry from this, add an extra "1" at the front
        if (carry != 0)
        {
            result->digitsCount++;
            *resultPtr-- = 1;
        }
    }
    // we're dealing with a subtraction operation.
    else
    {
        // if the left side is the larger number based on the length/exponent
        // compbination, then subtract the right from the left.
        if ((adjustedLeftExp + leftLength) > (adjustedRightExp + rightLength))
        {
            subtractNumbers(left, leftPtr, adjustedLeftExp, right, rightPtr, adjustedRightExp, result, resultPtr);
            // the result exponent is the exponent of the smaller number
            result->numberExponent = adjustedLeftExp != 0 ? rightExp : leftExp;
            // the result sign is that of the left
            result->numberSign = left->numberSign;
        }
        // right number bigger than the left?   We subtract the left from the right
        else if ((adjustedLeftExp + leftLength) < (adjustedRightExp + rightLength))
        {
            subtractNumbers(right, rightPtr, adjustedRightExp, left, leftPtr, adjustedLeftExp, result, resultPtr);
            // TODO:  double check this one to make sure this test is right.
            result->numberExponent = adjustedLeftExp != 0 ? rightExp : leftExp;
            // we use the right sign for this (which might have been flipped for the operation)
            result->numberSign = (short)rightSign;
        }

        // here both numbers have same adjusted lexponent + length
        // so see which number is longer and compare each number for length of smaller.
        // if they compare equal the result is zero
        else if (rightLength < leftLength)
        {
            // the right number is shorter.  Because of the adjusted exponents,
            // these digits actually line up at the same positions.  Things work
            // our more efficiently if we subtract the smaller number from the larger
            if (memcmp(right->numberDigits, left->numberDigits, rightLength) > 0)
            {
                // subtract the left from the right and keep the sign from the right.
                subtractNumbers(right, rightPtr, adjustedRightExp, left, leftPtr, adjustedLeftExp, result, resultPtr);
                result->numberSign = (short)rightSign;
            }
            else
            {
                // subtracting right from left and using the sign from the left.
                subtractNumbers(left, leftPtr, adjustedLeftExp, right, rightPtr, adjustedRightExp, result, resultPtr);
                result->numberSign = left->numberSign;
            }
            // we use the exponent from the left (which must be larger because it is the shorter number).
            result->numberExponent = leftExp;
        }
        // the left length is less than or equal to the right.  Reverse the process
        // from above.
        else
        {
            int rc;
            // if the left number is larger, subtract the right from the left
            if ((rc = memcmp(left->numberDigits, right->numberDigits, leftLength)) > 0)
            {
                subtractNumbers(left, leftPtr, adjustedLeftExp, right, rightPtr, adjustedRightExp, result, resultPtr);
                result->numberExponent = rightExp;
                result->numberSign = left->numberSign;
            }
            // left number is smaller
            else if (rc != 0)
            {
                subtractNumbers(right, rightPtr, adjustedRightExp, left, leftPtr, adjustedLeftExp, result, resultPtr);
                result->numberExponent = rightExp;
                result->numberSign = (short)rightSign;
            }
            // numbers compare equal for their lengths
            else
            {
                // if the left length is shorter, then subtract the left from the right
                if (leftLength < rightLength)
                {
                    subtractNumbers(right, rightPtr, adjustedRightExp, left, leftPtr, adjustedLeftExp, result, resultPtr);
                    result->numberExponent = rightExp;
                    result->numberSign = (short)rightSign;
                }
                // the lengths are equal and they compare equal, so this result is zero.
                else
                {
                    result->setZero();
                    // no adjustment necessary, we can return this now.
                    return result;
                }
            }
        }
    }

    // pointer resultPtr always points to next position to add a digit
    // thereofre we will bump the pointer 1st before doing any cleanup
    // we end up pointing to the most sig digit on exit of loop instead of
    // the 1st digit.

    //make sure result ends up with correct precision.
    result->adjustPrecision(++resultPtr, digits);
    return result;
}


/**
 * Subtract two numbers.  The second number is subtracted
 * from the first.  The absolute value of the 1st number
 * must be larger than the absolute value of the second.
 *
 * @param larger     The pointer to the larger number.
 * @param largerPtr  The larger number digits pointer.
 * @param adjustedLargerExp
 *                   The adjusted exponent for the larger number.
 * @param smaller    The smaller number value.
 * @param smallerPtr The pointer to the digit data for the smaller number.
 * @param adjustedSmallerExp
 *                   The adjusted exponent for the smaller number.
 * @param result     The result number.
 * @param resultPtr  The pointer for writing the result data.
 */
void NumberString::subtractNumbers(NumberString *larger, const char *largerPtr, wholenumber_t adjustedLargerExp,
    NumberString *smaller, const char *smallerPtr, wholenumber_t adjustedSmallerExp, NumberString *result, char *&resultPtr)
{
    // no carry from the start
    int borrow = 0;

    // 1st we need to make both adjusted exponents zero.  values are either
    // positive or zero on entry. one of the adjusted exponents will
    // always be zero on entry.


    // if larger number had a value for for adjusted exponent that means
    // that smaller number had a smaller exponent (less significant digit
    // so we pretend to add extra zeros to the larger number and do the
    // subtraction for required number of digits, (aLargerExp).
    //   xxxxx00
    //    yyyyyy

    int subDigit;

    while (adjustedLargerExp--)
    {
        if (smallerPtr >= smaller->numberDigits)
        {
            subDigit = *smallerPtr--;
        }
        else
        {
            subDigit = '\0';
        }
        // we're subtracting from zero, so we need to borrow.  We also
        // need to worry about a borrow from a previous iteration
        subDigit = borrow + 10 - subDigit;
        // if the result was 10, then the bottom digit was zero
        // and we didn't borrow anything.  This is a zero digit
        // and there's no borrow.
        if (subDigit == 10)
        {
            subDigit = 0;
            borrow = 0;
        }
        // the digit is ok, but we need to borrow on the next digit
        else
        {
            borrow = -1;
        }

        // add this to our result number
        *resultPtr-- = (char)subDigit;
        result->digitsCount++;
    }

    // same process, but the smaller number needs the padding.  In this
    // case, we're subtracting zero from each of the positions...an extremely
    // easy process!
    while (adjustedSmallerExp--)
    {
        // if we still have digits here, just copy the digit into the
        // result, otherwise pad with a zero.
        if (largerPtr >= larger->numberDigits)
        {
            *resultPtr-- = *largerPtr--;
        }
        else
        {
            *resultPtr-- = '\0';
        }
        result->digitsCount++;
    }

    // ok, our strings have had the exponent mismatches handled.
    // strings of digits, do the actual subtraction.
    while (smallerPtr >= smaller->numberDigits)
    {
        // standard subtraction process.  Subtract two digits
        // and account for the borrows
        subDigit = borrow + (*largerPtr--) - *(smallerPtr--);
        // result less than zero means we need to borrow on the next
        // iteration.  Add 10 to this digit to reflect the borrow.
        if (subDigit < 0  )
        {
            borrow = -1;
            subDigit += 10;
        }
        // a positive result means no borrow
        else
        {
            borrow = 0;
        }
        // add this to the result buffer
        *resultPtr-- = (char)subDigit;
        result->digitsCount++;
    }

    // we might still have digits left on the larger number
    while (largerPtr >= larger->numberDigits)
    {
        // we have nothing to subtract here but the borrow
        subDigit = borrow + (*largerPtr--);

        if (subDigit < 0)
        {
            borrow = -1;
            subDigit += 10;
        }
        else
        {
            borrow = 0;
        }
        *resultPtr-- = (char)subDigit;
        result->digitsCount++;
    }
    // because we arranged these so that we always subtract the smaller from the larger, we
    // don't need to worry about a borrow out of the top digit.
}

/**
 * Adds a base ten digit to string number in
 * base 16 (used by the d2x and d2c functions)
 *
 * @param digit     The digit to add.
 * @param value     The number we're adding to.
 * @param highDigit The highest digit location
 *
 * @return The new highest digit location if this carrys out.
 */
char *NumberString::addToBaseSixteen(int digit, char *value, char *highDigit )
{
    // while we have something to add
    while (digit != 0)
    {
        // add in to the current value location
        digit += *value;
        // we're doing base-16 math here, so check for carries
        // in base 16.
        if (digit > 15)
        {
            digit -= 16;
            *value-- = (char)digit;
            digit = 1;
        }
        else
        {
            *value-- = (char)digit;
            digit = 0;
        }
    }

    // return the highest digit position
    return value < highDigit ? value : highDigit;
}


/**
 * multiplies a base 16 number by decimal 10, placing
 * the result in the same buffer.
 *
 * @param Accum     The current accumulator position.
 * @param HighDigit The current high digit location for the number.
 *
 * @return The new high digit location.
 */
char *NumberString::multiplyBaseSixteen(char *accum, char * highDigit )
{
    // get a working output pointer
    char *outPtr = accum;
    // no carry at the start
    unsigned int carry = 0;

    // until we run out of digits, do the multiply.
    while (outPtr > highDigit)
    {
        // multiply the current digit by 10
        unsigned int digit = (unsigned int )((*outPtr * 10) + carry);
        // the only digits that won't carry here are 0 and 1.
        if (digit > 15)
        {
            // the carry value is the number of units base 16.  For example,
            // if the current digit is 4, the result is 40.  Our carry value
            // is 2 and the new digit at this location is 8
            carry = digit / 16;
            // the digit is the lower nibble.
            digit &= (unsigned int )0x0000000f;
        }
        // no carry over to the next
        else
        {
            carry = 0;
        }
        *outPtr-- = (char)digit;
    }

    // did we carry out of the top digit (pretty likely).
    if (carry)
    {
        *outPtr-- = (char)carry;
    }
    // return the new high water mark.
    return outPtr;
}


/**
 * Adds a base sixteen digit to a string number
 * base 10 (used by the x2d and c2d functions)
 *
 * @param digit     The base-16 digit to add.
 * @param accum     The accumulator pointer.
 * @param highDigit The highest digit of our result so far.
 *
 * @return The new highest digit position.
 */
char *NumberString::addToBaseTen(int digit, char *accum, char *highDigit)
{
    int carry = 0;
    // continue as long as we have something to add in
    while (digit != 0 || carry != 0)
    {
        digit += *accum + carry;
        if (digit > 9)
        {
            carry = digit / 10;
            digit %= 10;
            *accum-- = (char)digit;
        }
        else
        {
            *accum-- = (char)digit;
            carry = 0;
        }
        digit = 0;
    }
    // return the current high water mark
    return accum < highDigit ? accum : highDigit;

}

/**
 * multiplys a base 16 number by 10, placing
 * the number in the same buffer
 *
 * @param Accum     The current accumulator position
 * @param HighDigit The high digit of the result.
 *
 * @return The new high digit position.
 */
char *NumberString::multiplyBaseTen(char *accum, char *highDigit)
{
    char *outPtr = accum;
    unsigned int carry = 0;

    // while we have more digits to process
    while (outPtr > highDigit)
    {
        // multiplying by 16, but checking for the carry in decimal
        unsigned int digit = (unsigned int )((*outPtr * 16) + carry);
        if (digit > 9)
        {
            // the carry value can be larger than 1
            carry = digit / 10;
            // the digit is the remainder
            digit %= 10;
        }
        // no carry out on this one
        else
        {
            carry = 0;
        }
        *outPtr-- = (char)digit;
    }

    // handle carry outs...
    while (carry)
    {
        unsigned int digit = carry % 10;
        carry = carry / 10;
        *outPtr-- = (char)digit;
    }
    // this is the new high water mark
    return outPtr;
}
