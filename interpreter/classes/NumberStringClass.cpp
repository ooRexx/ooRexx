/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/* Primitive NumberString Class                                               */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "NumberStringClass.hpp"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "RexxActivation.hpp"
#include "Numerics.hpp"
#include "StringUtil.hpp"
#include "MethodArguments.hpp"
#include "Interpreter.hpp"

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include <locale.h>   // localeconv
#ifdef HAVE_XLOCALE_H
# include <xlocale.h> // localeconv on BSD
#endif
#include <cmath>
#include <cfloat>

// singleton class instance
RexxClass *NumberString::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void NumberString::createInstance()
{
    CLASS_CREATE_SPECIAL(NumberString, "String", RexxClass);
}


/**
 * Constructor for a new number string value.
 *
 * @param len    The length we require for the value.
 */
NumberString::NumberString(size_t len)
{
    setNumericSettings(number_digits(), number_form());
    numberSign = 1;
    digitsCount = len;
}


/**
 * Create a number string for a given digits precision
 *
 * @param len       The length of value we need to accomodate
 * @param precision The precision to be used for formatting.
 */
NumberString::NumberString(size_t len, size_t precision)
{
    setNumericSettings(precision, number_form());
    numberSign = 1;
    digitsCount = len;
}


/**
 * Special clone code for a number string object.
 *
 * @return A new numberstring with extra fields nulled out.
 */
NumberString *NumberString::clone()
{
    NumberString *newObj = (NumberString *)RexxInternalObject::clone();
    // clear all other fields
    newObj->stringObject = OREF_NULL;
    newObj->objectVariables = OREF_NULL;
    return newObj;
}


/**
 * Get the primitive hash value of this String object.
 *
 * @return The calculated string hash for the string.
 */
HashCode NumberString::getHashValue()
{
    // we need to use the string hash so that we can show up in
    // the same hash buckets as the equivalent string object.
    return stringValue()->getHashValue();
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void NumberString::live(size_t liveMark)
{
    memory_mark(objectVariables);
    memory_mark(stringObject);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void NumberString::liveGeneral(MarkReason reason)
{
    memory_mark_general(objectVariables);
    memory_mark_general(stringObject);
}


/**
 * Flatten the object contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void NumberString::flatten(Envelope *envelope)
{
    setUpFlatten(NumberString)

    flattenRef(objectVariables);
    flattenRef(stringObject);

    cleanUpFlatten
}


/**
 * Set the number string's string value
 *
 * @param stringObj The formatted string value.
 */
void NumberString::setString(RexxString *stringObj)
{
    setField(stringObject, stringObj);
    // we need to mark when garbage collecting
    setHasReferences();
}


/**
 * Handle a REQUEST('STRING') request for a REXX numberstring
 *
 * @return Return the backing string value.
 */
RexxString *NumberString::makeString()
{
    return stringValue();
}


/**
 * Override for the default object makearray method.
 *
 * @return The results of our string representation's makearray.
 */
ArrayClass *NumberString::makeArray()
{
  return stringValue()->makeArray();
}


/**
 * Handle a HASMETHOD request for a numberstring.  Since this
 * class is hidden, we get the string value and return
 * its hasmethod result.
 *
 * @param methodName The target method name.
 *
 * @return The hasmethod result.
 */
bool NumberString::hasMethod(RexxString *methodName)
{
    return stringValue()->hasMethod(methodName);
}


/**
 * primitive level string conversion request.
 *
 * @return The object's string value
 */
RexxString *NumberString::primitiveMakeString()
{
    // if we have a string value created already, return it now, otherwise
    // we need to format the value.
    if (stringObject != OREF_NULL)
    {
        return stringObject;
    }
    return stringValue();
}


/**
 * Test if a number has any significant (i.e., non-zero)
 * decimal values within a given digits setting.
 *
 * @param digits The digits setting to test.
 *
 * @return true if this number has significant decimals.
 */
bool NumberString::hasSignificantDecimals(wholenumber_t digits)
{
    // if the exponent is not negative, then we can't have decimals.
    if (!hasDecimals())
    {
        return false;
    }

    // the first digit to check
    const char *tempPtr = numberDigits + digitsCount + numberExponent;
    wholenumber_t checkLength = -numberExponent;
    const char *highDigit = numberDigits + digits;
    // while more decimals and we're still within the digits value
    while (checkLength > 0 && tempPtr < highDigit)
    {
        // we found a digit
        if (*tempPtr++ != 0)
        {
            return true;
        }
        checkLength--;
    }

    // if we hit the digits limit before running of digits to check,
    // we need to test this digit to see if this will round up into
    // the area we already checked.
    if (checkLength > 0)
    {
        return *tempPtr >= 5;
    }
    // all insigificant
    return false;
}


/**
 * Format an exponent value into a buffer, including the
 * "E" marker and the exponent sign.
 *
 * @param exponent The exponent value.
 * @param buffer   The buffer for formatting.
 */
void NumberString::formatExponent(wholenumber_t exponent, char *buffer)
{
    // if this is a negative value, the sign is created using the
    // number formatting
    if (exponent < 0)
    {
        *buffer = 'E';
        Numerics::formatWholeNumber(exponent, buffer + 1);
    }
    else if (exponent > 0)
    {
        // add both the "E" and the "+", then format the number
        strcpy(buffer, "E+");
        Numerics::formatWholeNumber(exponent, buffer + 2);
    }
    else
    {
        // make this a null C string
        *buffer = '\0';
    }
}


/**
 * Check for an overflow situation with the exponent.
 */
void NumberStringBase::checkOverflow()
{
    // if the adjusted exponent is too large
    if (numberExponent + digitsCount - 1 > Numerics::MAX_EXPONENT)
    {
        reportException(Error_Overflow_expoverflow, numberExponent + digitsCount - 1, Numerics::DEFAULT_DIGITS);
    }

    // or just straight out too small, these are errors.
    if (numberExponent < Numerics::MIN_EXPONENT)
    {
        reportException(Error_Overflow_expunderflow, numberExponent, Numerics::DEFAULT_DIGITS);
    }
    return;
}


/**
 * Convert a number string to a string object
 *
 * @return The formatted string version of this number string.
 */
RexxString *NumberString::stringValue()
{
    // if we've done this already, just return the formatted value.
    if (stringObject != OREF_NULL)
    {
        return stringObject;
    }

    // if the sign is zero, we are exactly zero...this is an easy
    // format.
    if (isZero())
    {
        setString(GlobalNames::ZERO);
        return stringObject;
    }

    // if the exponent value is zero, this is all integer digits.  This
    // is a fast path for conversion.
    if (isAllInteger())
    {
        wholenumber_t maxNumberSize = digitsCount + isNegative();

        // get a new string of the exact length needed
        RexxString *stringObj = raw_string(maxNumberSize);
        NumberBuilder builder(stringObj);

        // just build as an integer
        builder.addIntegerPart(isNegative(), numberDigits, digitsCount, 0);
        // set and return this string representation
        setString(stringObj);
        stringObj->setNumberString(this);
        return stringObj;
    }


    // we likely have some sort of decimals or exponent value.  This is
    // a slightly more complicated formatting job.

    // if we have an overflow situation, we cannot format
    checkOverflow();

    // the number is not exponential yet.
    wholenumber_t expFactor = 0;
    wholenumber_t adjustedSize = numberExponent + digitsCount - 1;
    wholenumber_t adjustedExponent = numberExponent;

    // null out the exponent string value for now.
    char  expstring[17];
    expstring[0] = '\0';

    // have we hit the trigger value either
    // A) the number of digits to the left of the decimal exceeds the digits setting or
    // B) the number of digits to the right of the decimal point exceed twice the digits setting.
    if ((adjustedSize >= createdDigits) || (std::abs(numberExponent) > (createdDigits * 2)))
    {
        // we need to go exponential.  Need to make a number of adjustments based
        // on the formatting.
        if (isEngineering())
        {
            // if the adjusted size is negative, we have nothing to the
            // left of the decimal point.  Force a 2 character adjustment to the left.
            if (adjustedSize < 0)
            {
                adjustedSize -= 2;
            }
            // get this adjusted in units of 3.
            adjustedSize = (adjustedSize / 3) * 3;
        }

        // our adjustment may end up with an overflow or underflow, so raise the error
        if (std::abs(adjustedSize) > Numerics::MAX_EXPONENT)
        {
            reportException(adjustedSize > 0 ? Error_Overflow_expoverflow : Error_Overflow_expunderflow, adjustedSize, Numerics::DEFAULT_DIGITS);
        }

        // adjust the exponent for formatting
        adjustedExponent -= adjustedSize;

        // make sure we still have an exponent after the adjustments
        expFactor = adjustedSize != 0;

        // format the string form of the exponent
        formatExponent(adjustedSize, expstring);
        // get the exponent value as a positive number from here.
        adjustedSize = std::abs(adjustedSize);
    }

    wholenumber_t maxNumberSize;

    // if the adjusted exponent is not negative, then we are
    // shifting everything to the left, so the number is longer
    // than the size.
    if (adjustedExponent >= 0)
    {
        // the size is the length of the number plus the exponent
        maxNumberSize = (size_t)adjustedExponent + digitsCount;
    }
    // the exponent is negative, so we're shifting stuff to the right.
    // if the absolute value of the exponent is larger than the
    // length of the number, then we'll need to add additional zeros between
    // the decimal point and the first digit.
    else if (std::abs(adjustedExponent) >= digitsCount)
    {
        // we add to characters for a leading zero and decimal point
        maxNumberSize = std::abs(adjustedExponent) + 2;
    }
    // not adding any digits, the decimal is in the middle, so our
    // result value is the length plus one for a decimal point
    else
    {
        maxNumberSize = digitsCount + 1;
    }

    // if we're using exponential notation, add in the size of the
    // exponent string
    if (expFactor)
    {
        maxNumberSize += strlen(expstring);
    }
    // add in space for a sign
    if (isNegative())
    {
        maxNumberSize++;
    }

    RexxString *stringObj = raw_string(maxNumberSize);
    NumberBuilder builder(stringObj);

    wholenumber_t adjustedLength = adjustedExponent + digitsCount;

    // if the adjusted length is not positive, then
    // we have nothing to the left of the decimal.  All
    // digits are include, plus we start with a leading
    // "0." and potential leading 0s before the start of the digits.
    // looking something like this "0.00000xxxxx"
    if (adjustedLength <= 0)
    {
        // build the integer part as just the sign and a single pad character
        builder.addIntegerPart(isNegative(), numberDigits, 0, 1);
        // build the decimal portion
        builder.addDecimalPart(numberDigits, digitsCount, -adjustedLength);

        // add the exponent, if any
        builder.addExponent(expstring);
    }
    // we might need to add zeros after the digits,
    // something like "xxxx00000"
    else if (adjustedLength >= digitsCount)
    {
        // build the integer section
        builder.addIntegerPart(isNegative(), numberDigits, digitsCount, adjustedLength - digitsCount);
        // add the exponent, if any
        builder.addExponent(expstring);
    }
    // partial decimal number
    else
    {
        // do the integer and decimal parts separately
        builder.addIntegerPart(isNegative(), numberDigits, adjustedLength);
        builder.addDecimalPart(numberDigits + adjustedLength, digitsCount - adjustedLength);
        // and the exponent, if we have one.
        builder.addExponent(expstring);
    }

    // since this was created from the number string, we can set this as
    // the string value and also set the number string value the string value
    stringObj->setNumberString(this);
    setString(stringObj);
    return stringObj;
}


/**
 * Convert a number string to a wholenumber value under the default digits.
 *
 * @param result The returned converted number.
 *
 * @return true if this could be converted, false otherwise.
 */
bool NumberString::numberValue(wholenumber_t &result)
{
    // convert using the default digits version
    return numberValue(result, Numerics::DEFAULT_DIGITS);
}


/**
 * Convert a number string to an unsigned whole number value
 *
 * @param result The converted value.
 *
 * @return true if this converted ok, false otherwise.
 */
bool NumberString::unsignedNumberValue(size_t &result)
{
    // convert using the default digits version
    return unsignedNumberValue(result, Numerics::DEFAULT_DIGITS);
}


/**
 * Convert a number string to a number value
 *
 * @param result    The returned result.
 * @param numDigits The precision used for the conversion.
 *
 * @return The converted value.
 */
bool NumberString::numberValue(wholenumber_t &result, wholenumber_t numDigits)
{
    // set up the default values
    bool carry = false;
    // we work off of copies of these values so that adjustments do not alter
    // the original object.
    wholenumber_t numberExp = numberExponent;
    wholenumber_t length = digitsCount;

    // if the number is exactly zero, then this is easy
    if (isZero())
    {
        result = 0;
        return true;
    }

    // used to return conversion results.
    size_t intnum;

    // is this easily within limits (very common)?
    if (length <= numDigits && numberExp >= 0)
    {
        // go convert...we need to make sure it stays in range.
        if (!createUnsignedValue(numberDigits, length, false, numberExp, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // too big to handle
        }

        // adjust for the sign
        result = ((wholenumber_t)intnum) * numberSign;
        return true;
    }

    // this number either has decimals, or needs to be truncated/rounded because of
    // the conversion digits value.  We need to make adjustments.

    if (!checkIntegerDigits(numDigits, length, numberExp, carry))
    {
        return false;
    }

    // if because of this adjustment, the decimal point lies to the left
    // of our first digit, then this value truncates to 0 (or 1, if a carry condition
    // resulted).
    if (-numberExp >= length)
    {
        // since we know a) this number is all decimals, and b) the
        // remaining decimals are either all 0 or all 9s with a carry,
        // this result is either 0 or 1.
        result = carry ? 1 : 0;
        return true;
    }

    // we process different bits depending on whether this is a negative or positive
    // exponent
    if (numberExp < 0)
    {
        // now convert this into an unsigned value
        if (!createUnsignedValue(numberDigits, length + numberExp, carry, 0, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // to big to handle
        }
    }
    // a straight out number, we can compute just from the digits.
    else
    {
        if (!createUnsignedValue(numberDigits, length, carry, numberExp, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // to big to handle
        }
    }

    // adjust for the sign
    result = ((wholenumber_t)intnum) * numberSign;
    return true;
}


/**
 * Convert a number string to an unsigned number value
 *
 * @param result    The returned result.
 * @param numDigits The precision for converting this value.
 *
 * @return true if the number could be converted, false otherwise.
 */
bool NumberString::unsignedNumberValue(size_t &result, wholenumber_t numDigits)
{
    // set up the default values

    bool carry = false;
    // we work off of copies of these values so that adjustments do not alter
    // the original object.
    wholenumber_t numberExp = numberExponent;
    wholenumber_t length = digitsCount;

    // if the number is exactly zero, then this is easy
    if (isZero())
    {
        result = 0;
        return true;
    }

    // we can't convert negative values into an unsigned one
    if (isNegative())
    {
        return false;
    }

    // is this easily within limits (very common)?
    if (length <= numDigits && numberExp >= 0)
    {
        // convert and directly return the value
        return createUnsignedValue(numberDigits, length, false, numberExp, Numerics::maxValueForDigits(numDigits), result);
    }


    // this number either has decimals, or needs to be truncated/rounded because of
    // the conversion digits value.  We need to make adjustments.
    if (!checkIntegerDigits(numDigits, length, numberExp, carry))
    {
        return false;
    }


    // if because of this adjustment, the decimal point lies to the left
    // of our first digit, then this value truncates to 0 (or 1, if a carry condition
    // resulted).
    if (-numberExp >= digitsCount)
    {
        // since we know a) this number is all decimals, and b) the
        // remaining decimals are either all 0 or all 9s with a carry,
        // this result is either 0 or 1.
        result = carry ? 1 : 0;
        return true;
    }

    // we process different bits depending on whether this is a negative or positive
    // exponent
    if (numberExp < 0)
    {
        // now convert this into an unsigned value
        return createUnsignedValue(numberDigits, length + numberExp, carry, 0, Numerics::maxValueForDigits(numDigits), result);
    }
    else
    {
        return createUnsignedValue(numberDigits, length, carry, numberExp, Numerics::maxValueForDigits(numDigits), result);
    }
}


/**
 * Convert a number string to a double value.
 *
 * @param result The returned conversion value.
 *
 * @return true if this converted, false otherwise.
 */
bool NumberString::doubleValue(double &result)
{
    // build this from the string value
    const char *string = stringValue()->getStringData();
    // use the C library routine to convert this to a double value since we
    // use compatible formats

    // strtod() is locale-dependent, and we cannot be sure that we run under
    // the default "C" locale, as some badly behaved native code (a known
    // example being BSF.CLS) may have called setlocale(LC_ALL, "") or similar.

    // strtod_l() is available on many platforms, but e. g. Openindiana is
    // missing it.  Switching back and forth with uselocale() would work
    // (maybe slow), but uselocale() isn't readily available on Windws,
    // where it would require setlocale() together with
    // _configthreadlocale(_ENABLE_PER_THREAD_LOCALE).  We instead employ a
    // hack: should the current locale not have the dot as decimal radix, we
    // replace any dot with the current locale radix before conversion.

    char localeRadix = *localeconv()->decimal_point;

    // if the current locale uses a dot as radix, just do a straight conversion
    // (very common)
    if (localeRadix == '.')
    {
        result = strtod(string, NULL);
    }
    else
    {
        // The current locale uses no dot.  Change any dot to locale radix
        // before conversion, but don't modify the Rexx string itself.
        char *str = strdup(string);
        if (str == NULL)
        {
            return false;
        }
        char *dotPos = strchr(str, '.');
        if (dotPos != NULL)
        {
            *dotPos = localeRadix;
        }
        result = strtod(str, NULL);
        free(str);
    }

    // and let pass all of the special cases
    return true;
}


/**
 * convert a number string to an integer object
 *
 * @param digits The digits setting for the conversion.
 *
 * @return The value converted into an Integer object, or .nil
 *         if it does not convert.
 */
RexxInteger *NumberString::integerValue(wholenumber_t digits)
{
    wholenumber_t integerNumber;

    // try to convert and return .nil for any failures
    if (!numberValue(integerNumber, digits))
    {
        return (RexxInteger *)TheNilObject;
    }
    // return as an integer
    return new_integer(integerNumber);
}


/**
 * Convert the numberstring to unsigned value
 *
 * @param thisnum   The number digits.
 * @param intlength The length of the digits to process
 * @param carry     The returned carry flag if this rounds out.
 * @param exponent  The current exponent.
 * @param maxValue  The maximum value for this conversion.
 * @param result    The returned result.
 *
 * @return true if this converted, false for any failures.
 */
bool  NumberString::createUnsignedValue(const char *thisnum, size_t intlength, int carry, wholenumber_t exponent, size_t maxValue, size_t &result)
{
    // if the exponent multiplier would cause an overflow, there's no point in doing
    // anything here
    if (exponent + intlength > Numerics::ARGUMENT_DIGITS)
    {
        return false;
    }

    // our converted value
    size_t intNumber = 0;

    for (size_t numpos = 1; numpos <= intlength; numpos++ )
    {
        // add in the next digit value
        size_t newNumber = (intNumber * 10) + (size_t)*thisnum++;
        // if an overflow occurs, then the new number will wrap around and be
        // smaller that the starting value.
        if (newNumber < intNumber)
        {
            return false;
        }
        // make this the current value and continue
        intNumber = newNumber;
    }

    // do we need to add in a carry value because of a rounding situation?
    if (carry)
    {
        // add in the carry bit and check for an overflow, again
        size_t newNumber = intNumber + 1;
        if (newNumber < intNumber)
        {
            return false;
        }
        intNumber = newNumber;
    }

    // have an exponent to process?
    for (wholenumber_t exp = 1; exp <= exponent; exp++ )
    {
        size_t newNumber = intNumber * 10;

        // did this wrap?
        if (newNumber < intNumber)
        {
            return false;
        }
        intNumber = newNumber;
    }

    // was ths out of range for this conversion?
    if (intNumber > maxValue)
    {
        return false;
    }

    // return the converted number and indicate success.
    result = intNumber;
    return true;
}


/**
 * Convert the numberstring to unsigned INT64 value
 *
 * @param thisnum   The digits for the conversion.
 * @param intlength The number of digits to process.
 * @param carry     A potential carried out digit.
 * @param exponent  The exponent value for the number.
 * @param maxValue  The maximum value for the conversion.
 * @param result    The returned conversion value.
 *
 * @return true if this converted, false otherwise.
 */
bool  NumberString::createUnsignedInt64Value(const char *thisnum, size_t intlength, int carry, wholenumber_t exponent, uint64_t maxValue, uint64_t &result)
{
    // if the exponent multiplier would cause an overflow, there's no point in doing
    // anything here
    if (exponent + intlength > Numerics::DIGITS64)
    {
        return false;
    }

    // our converted value
    uint64_t intNumber = 0;

    for (size_t numpos = 1; numpos <= intlength; numpos++ )
    {
        // add in the next digit value
        uint64_t newNumber = (intNumber * 10) + (uint64_t)*thisnum++;
        // if an overflow occurs, then the new number will wrap around and be
        // smaller that the starting value.
        if (newNumber < intNumber)
        {
            return false;
        }
        // make this the current value and continue
        intNumber = newNumber;
    }

    // do we need to add in a carry value because of a rounding situation?
    if (carry)
    {
        // add in the carry bit and check for an overflow, again
        uint64_t newNumber = intNumber + 1;
        if (newNumber < intNumber)
        {
            return false;
        }
        intNumber = newNumber;
    }

    // have an exponent to process?
    for (wholenumber_t exp = 1; exp <= exponent; exp++ )
    {
        uint64_t newNumber = intNumber * 10;

        // did this wrap?
        if (newNumber < intNumber)
        {
            return false;
        }
        intNumber = newNumber;
    }

    // was ths out of range for this conversion?
    if (intNumber > maxValue)
    {
        return false;
    }

    result = intNumber;                  /* Assign return value.              */
    return true;                         /* Indicate sucessfule converison.   */
}


/**
 * Check that a numberstring is convertable into an integer value
 *
 * @param numDigits The digits() setting for the check.
 * @param numberLength
 *                  The length of the value (which can be updated by the check)
 * @param numberExponent
 *                  The exponent value (also updatable)
 * @param carry     A carry out flag.
 *
 * @return true if this can be converted to an integer, false if it fails.
 */
bool NumberString::checkIntegerDigits(wholenumber_t numDigits, wholenumber_t &length,
    wholenumber_t &exponent, bool &carry)
{
    // set the initial values
    carry = false;
    exponent = numberExponent;
    length = digitsCount;

    // is this number longer than the digits value?
    // this is going to be truncated or rounded, so we
    // need to see if a carry is required, and also
    // adjust the exponent value.
    if (length > numDigits)
    {
        // adjust the effective exponent up by the difference
        exponent += (length - numDigits);
        // and override the converted length to be just the digits length
        length = numDigits;

        // now check to see if the first excluded digit will cause rounding
        // if it does, we need to worry about a carry value when converting
        if (*(numberDigits + numDigits) >= 5)
        {
            carry = true;
        }
    }

    // if we have a negative exponent, then we need to look at
    // the values below the decimal point
    if (exponent < 0)
    {
        // the position of the decimal is the negation of the exponent
        wholenumber_t decimalPos = -exponent;
        // everything to the right of the decimal must be a zero.
        char compareChar = 0;
        // if we had a previous carry condition, this changes things a
        // bit.  Since the carry will add 1 to the right-most digit,
        // in order for all of the decimals to end up zero, then all of
        // the digits there must actually be '9's.
        if (carry)
        {
            // if the decimal position will result in at least one padding
            // zero, then the carry makes it impossible for all of the decimals
            // to reduce to zero.  This cannot be a whole number.
            if (decimalPos > length)
            {
                return false;
            }
            // switch the checking value
            compareChar = 9;
        }

        const char *numberData;
        if (decimalPos >= length )
        {
            // decimal is somewhere to the left of everything...
            // all of these digits must be checked
            decimalPos = length;
            numberData = numberDigits;
        }
        else
        {
            // get the exponent adjusted position
            numberData = numberDigits + length + exponent;
        }

        for ( ; decimalPos > 0 ; decimalPos--)
        {
            // a bad digits data means a conversion failure
            if ( *numberData++ != compareChar)
            {
                return false;
            }
        }
    }
    return true;
}


/**
 * Convert a number string to a signed int64 value
 *
 * @param result    The returned result.
 * @param numDigits The digits value for the conversion.
 *
 * @return true if this converted ok, false otherwise.
 */
bool NumberString::int64Value(int64_t *result, wholenumber_t numDigits)
{
    // set up the default values
    bool carry = false;
    // we work off of copies of these values so that adjustments do not alter
    // the original object.
    wholenumber_t numberExp = numberExponent;
    wholenumber_t numberLength = digitsCount;
    uint64_t intnum;

    // if the number is exactly zero, then this is easy
    if (numberSign == 0)
    {
        *result = 0;
        return true;
    }

    // is this easily within limits (very common)?
    if (numberLength <= numDigits && numberExp >= 0)
    {
        // the minimum negative value requires one more than the max positive
        if (!createUnsignedInt64Value(numberDigits, numberLength, false, numberExp, ((uint64_t)INT64_MAX) + 1, intnum))
        {
            return false;                   // too big to handle
        }
        // this edge case can be a problem, so check for it specifically
        if (intnum == ((uint64_t)INT64_MAX) + 1)
        {
            // if at the limit, this must be a negative number
            if (numberSign != -1)
            {
                return false;
            }
            *result = INT64_MIN;
        }
        else
        {
            // adjust for the sign
            *result = ((int64_t)intnum) * numberSign;
        }
        return true;
    }

    // this number either has decimals, or needs to be truncated/rounded because of
    // the conversion digits value.  We need to make adjustments.

    if (!checkIntegerDigits(numDigits, numberLength, numberExp, carry))
    {
        return false;
    }

    // if because of this adjustment, the decimal point lies to the left
    // of our first digit, then this value truncates to 0 (or 1, if a carry condition
    // resulted).
    if (-numberExp>= numberLength)
    {
        // since we know a) this number is all decimals, and b) the
        // remaining decimals are either all 0 or all 9s with a carry,
        // this result is either 0 or 1.
        *result = carry ? 1 : 0;
        return true;
    }

    // we process different bits depending on whether this is a negative or positive
    // exponent
    if (numberExp < 0)
    {
        // now convert this into an unsigned value
        if (!createUnsignedInt64Value(numberDigits, numberLength + numberExp, carry, 0, ((uint64_t)INT64_MAX), intnum))
        {
            return false;                   // to big to handle
        }
    }
    else
    {
        if (!createUnsignedInt64Value(numberDigits, numberLength, carry, numberExp, ((uint64_t)INT64_MAX), intnum))
        {
            return false;                   // to big to handle
        }
    }

    // the edge case is a problem, so handle it directly
    if (intnum == ((uint64_t)INT64_MAX) + 1)
    {
        // if at the limit, this must be a negative number
        if (numberSign != -1)
        {
            return false;
        }
        *result = INT64_MAX;
    }
    else
    {
        // adjust for the sign
        *result = ((int64_t)intnum) * numberSign;
    }
    return true;
}


/**
 * Convert a number string to an unsigned int64 value
 *
 * @param result    The returned result.
 * @param numDigits The digis setting for the conversion.
 *
 * @return true if the value converted, false otherwise.
 */
bool NumberString::unsignedInt64Value(uint64_t *result, wholenumber_t numDigits)
{
    // set up the default values

    bool carry = false;
    // we work off of copies of these values so that adjustments do not alter
    // the original object.
    wholenumber_t numberExp = numberExponent;
    wholenumber_t numberLength = digitsCount;

    // if the number is exactly zero, then this is easy
    if (isZero())
    {
        *result = 0;
        return true;
    }

    // no signed values allowed
    if (isNegative())
    {
        return false;
    }

    // is this easily within limits (very common)?
    if (numberLength <= numDigits && numberExp >= 0)
    {
        if (!createUnsignedInt64Value(numberDigits, numberLength, false, numberExp, UINT64_MAX, *result))
        {
            return false;                   // too big to handle
        }
        return true;
    }

    // this number either has decimals, or needs to be truncated/rounded because of
    // the conversion digits value.  We need to make adjustments.

    if (!checkIntegerDigits(numDigits, numberLength, numberExp, carry))
    {
        return false;
    }

    // if because of this adjustment, the decimal point lies to the left
    // of our first digit, then this value truncates to 0 (or 1, if a carry condition
    // resulted).
    if (-numberExp >= numberLength)
    {
        // since we know a) this number is all decimals, and b) the
        // remaining decimals are either all 0 or all 9s with a carry,
        // this result is either 0 or 1.
        *result = carry ? 1 : 0;
        return true;
    }

    // we process different bits depending on whether this is a negative or positive
    // exponent
    if (numberExp < 0)
    {
        // now convert this into an unsigned value
        return createUnsignedInt64Value(numberDigits, numberLength + numberExp, carry, 0, UINT64_MAX, *result);
    }
    else
    {
        return createUnsignedInt64Value(numberDigits, numberLength, carry, numberExp, UINT64_MAX, *result);
    }
}


/**
 * Return a truth value boolean for a number string
 *
 * @param errorcode The error to raise if this is not valid.
 *
 * @return true or false if this is a valid logical.
 */
bool  NumberString::truthValue(RexxErrorCodes errorcode)
{
    // a zero value is easy
    if (isZero())
    {
        return false;
    }
    // only other choice is exactly 1.  This means 1) the size is positive,
    // 2) the exponent is zero, 3) the number has exactly one digits, and finally
    // 4) that single digit is a 1.  Thankfully, we rarely need to perform this test
    // on number strings!
    else if (!isOne())
    {
        reportException(errorcode, this);
    }
    // we have a true result.
    return true;
}


/**
 * Convert an object to a logical value without raising an
 * error.
 *
 * @param result The converted value.
 *
 * @return true if this converted ok, false for an invalid logical.
 */
bool NumberString::logicalValue(logical_t &result)
{
    // zero is and easy test.
    if (isZero())
    {
        result = false;
        return true;
    }
    // only other choice is exactly 1.  This means 1) the size is positive,
    // 2) the exponent is zero, 3) the number has exactly one digits, and finally
    // 4) that single digit is a 1.  Thankfully, we rarely need to perform this test
    // on number strings!
    else if (isOne())
    {
        result = true;                   // this is true and the conversion worked
        return true;
    }
    return false;                        // bad conversion
}


/**
 * Scan a string value to determine if it is a valid number.
 *
 * @param number The pointer to the string data.
 * @param length The length of the string data.
 *
 * @return true if this is NOT a valid number, false if it is a
 *         number.
 */
bool numberStringScan(const char *number, size_t length)
{
    // for efficiency, this code takes advantage of the fact that REXX
    // string objects all have a guard null character on the end

    // null strings are not a number
    if (length == 0)
    {
        return true;
    }

    bool hadPeriod = false;
    const char *inPtr = number;
    const char *endData = inPtr + length;

    // skip any leading blanks
    while (*inPtr == RexxString::ch_SPACE || *inPtr == RexxString::ch_TAB)
    {
        inPtr++;
    }

    // now start looking for real data
    char ch = *inPtr;
    // start with a sign
    if (ch == RexxString::ch_MINUS || ch == RexxString::ch_PLUS)
    {
        inPtr++;
        // spaces are allowed after a sign, so skip them too
        while (*inPtr == RexxString::ch_SPACE || *inPtr == RexxString::ch_TAB)
        {
            inPtr++;
        }
    }

    // we've stepped over any blanks or sign, now check to see if
    // this number starts with a period
    if (*inPtr == RexxString::ch_PERIOD)
    {
        inPtr++;
        hadPeriod = true;
    }

    // now start validating content.  We've already checked for
    // a period, so at this point, we should only see digits.
    while (*inPtr >= RexxString::ch_ZERO && *inPtr <= RexxString::ch_NINE)
    {
        inPtr++;
    }

    // have we reached the end (common case for integer values).  This is
    // a valid number
    if (inPtr >= endData)
    {
        return false;
    }

    // we've found something other than a digit.  First check is a period
    if (*inPtr == RexxString::ch_PERIOD)
    {
        // if we had a previous period, this is bad
        if (hadPeriod)
        {
            return true;
        }
        inPtr++;
        // scan off digits again
        while (*inPtr >= RexxString::ch_ZERO && *inPtr <= RexxString::ch_NINE)
        {
            inPtr++;
        }
        // use up the rest of the string (also common)...we have a good number
        if (inPtr >= endData)
        {
            return false;
        }
    }

    // we're at a non-digit.  Check for the start of an exponent.
    if (*inPtr == 'E' || *inPtr == 'e')
    {
        // we must have digits after this...fail if this the end of the string.
        if (++inPtr >= endData)
        {
            return true;
        }

        // the sign is optional after the E
        if ((*inPtr == RexxString::ch_MINUS) || (*inPtr == RexxString::ch_PLUS))
        {
            // but we still need something after the sign
            inPtr++;
            if (inPtr >= endData)
            {
                return true;
            }
        }

        // we need at least one valid digit at this point
        if (*inPtr < RexxString::ch_ZERO || *inPtr > RexxString::ch_NINE)
        {
            return true;
        }
        // we have at least one digit in the exponent, skip over all digits
        while (*inPtr >= RexxString::ch_ZERO && *inPtr <= RexxString::ch_NINE)
        {
            inPtr++;
        }
    }
    // trailing blanks are permitted at this point, so step over any we find
    while (*inPtr == RexxString::ch_SPACE || *inPtr == RexxString::ch_TAB)
    {
        inPtr++;
    }
    // at this point, we need to be at the end of the string
    if (inPtr >= endData)
    {
        return false;
    }
    // so close, but still invalid
    return true;
}


/**
 * Truncate a number to given decimal digit count
 *
 * @param decimal_digits
 *               The optional count (default is 0 decimals)
 *
 * @return The truncated number.
 */
RexxObject *NumberString::trunc(RexxObject *decimal_digits)
{
    size_t needed_digits = optionalNonNegative(decimal_digits, 0, ARG_ONE);

    // we need to round this number to the current digits setting, then perform
    // the truncation on the rounded version.  The original target is left unchanged.
    return prepareNumber(number_digits(), ROUND)->truncInternal(needed_digits);
}


/**
 * Truncate a number to given decimal digit count
 *
 * @param needed_digits
 *               The required number of decimals.
 *
 * @return The truncated number.
 */
RexxObject *NumberString::truncInternal(wholenumber_t needed_digits)
{
    // zero is easy, depending on the how many decimals we need
    if (isZero())
    {
        // if a complete truncation, return an integer zero
        if (needed_digits == 0)
        {
            return IntegerZero;
        }
        // we need to pad out with zero decimals.
        else
        {
            // we need space for "0." and the requested number of zero pads.
            RexxString *result = raw_string(needed_digits + 2);
            NumberBuilder builder(result);

            // add the zero position
            builder.addZeroDecimal();
            // add the zero padding
            builder.addZeros(needed_digits);
            return result;
        }
    }


    // we have to do some real formatting

    // get some counters for the different sections as we figure
    // this out.  Everything not found to be needed later will be zero.
    wholenumber_t integerPadding = 0;
    wholenumber_t integerDigits = 0;
    wholenumber_t leadDecimalPadding = 0;
    wholenumber_t trailingDecimalPadding = 0;
    wholenumber_t decimalDigits = 0;
    // include the period if we need a decimal part here.
    size_t overHead = needed_digits != 0;
    // we use the initial sign, but based on how things playout, this
    // value could truncate to something like "0.000", which loses the sign.
    // we will need to clear this out
    size_t signOverHead = isNegative();

    // if the exponent is greater than zero, then we will need to add some padding zeros
    //
    if (numberExponent > 0)
    {
        // all of the digits are on the integer side
        integerDigits = digitsCount;
        // we need to add addtional zeros
        integerPadding = numberExponent;
        // if we have decimals requested, these will all be

        // if we have digits requested, this will be all padding.
        if (needed_digits != 0)
        {
            leadDecimalPadding = needed_digits;
        }
    }
    // the number itself has a decimal part
    else
    {
        // this will be our integer part
        integerDigits = digitsCount + numberExponent;
        // at least part of this number is to the left of
        // the decimal.  Figure out how much goes to the right
        // and whether we need to add padding after the real
        // number decimals
        if (integerDigits > 0)
        {
            // figure out how many decimal digits we need from the number
            decimalDigits = std::min(digitsCount - integerDigits, needed_digits);
            // if we need more than are available, calculate how much trailing pad we need.
            if (decimalDigits < needed_digits)
            {
                trailingDecimalPadding = needed_digits - decimalDigits;
            }
        }

        // there is no leading part.
        else
        {
            // if we need to truncate all decimals, then this result is exactly zero.
            if (needed_digits == 0)
            {
                return IntegerZero;
            }

            // we need to add a zero before the decimal.
            integerPadding = 1;
            // this is the length of the whole decimal part, made
            // up of digits from the number and some number of padding zeros
            // between the period and the actual digits.
            wholenumber_t decimalLength = -numberExponent;
            // we have no real integer digits.
            integerDigits = 0;

            // split this into the padding and the actual digits.
            // if we need less that this, we'll adjust these values.
            leadDecimalPadding = decimalLength - digitsCount;
            decimalDigits = digitsCount;

            // will we need to add additional zeros beyond what is provided?
            if (needed_digits >= decimalLength)
            {
                // we use all of the number value, plus some potential trailing pad zeros.
                trailingDecimalPadding = needed_digits - decimalLength;
            }
            // we really are truncating part of this
            else
            {
                // truncating in the leading section?  We don't add
                // any digits from the number and cap the padding.
                if (needed_digits <= leadDecimalPadding)
                {
                    // we're going from something like "-0.0001234" to "-0.00".
                    // the sign needs to disappear
                    signOverHead = 0;
                    decimalDigits = 0;
                    leadDecimalPadding = needed_digits;
                }
                // we're using all of the padding and at least one of the digits
                else
                {
                    decimalDigits = std::min(decimalDigits, needed_digits - leadDecimalPadding);
                }
            }
        }
    }

    // let's see if we can return this as an integer.  this requires
    // - no digits requested,
    // - have at least one digit,
    // - but no more than REXXINTEGER_DIGITS in result
    if (needed_digits == 0 && integerDigits >= 1 && integerDigits + integerPadding <= Numerics::REXXINTEGER_DIGITS)
    {
        return new_integer(signOverHead != 0, numberDigits, integerDigits, integerPadding);
    }

    // add up the whole lot to get the result size
    wholenumber_t resultSize = overHead + signOverHead + integerDigits + integerPadding + leadDecimalPadding + decimalDigits + trailingDecimalPadding;
    RexxString *result = raw_string(resultSize);
    NumberBuilder builder(result);

    // build the integer part
    builder.addIntegerPart(signOverHead != 0, numberDigits, integerDigits, integerPadding);
    // if we have any decimal part, add in the period and the trailing sections
    if (needed_digits != 0)
    {
        builder.addDecimalPart(numberDigits + integerDigits, decimalDigits, leadDecimalPadding, trailingDecimalPadding);
    }
    // and return the result
    return result;
}


/**
 * Return the floor value for a decimal numeric value.  The floor
 * is the first full number value less that the current
 * value.  If the number has no significant decimal positions,
 * this returns the same value (minus any trailing decimal zeros).
 * NOTE:  If the number is negative, the value goes toward the
 * lower number (e.g., the floor of -3.5 is -4, not -3).
 *
 * @return The numeric floor of this value.
 */
RexxObject *NumberString::floor()
{
    /* round to current digits setting   */
    return prepareNumber(number_digits(), ROUND)->floorInternal();
}


/**
 * Calculate the floor value for a number.
 *
 * @return A string value of the floor, with appropriate
 *         formatting for the function.
 */
RexxObject *NumberString::floorInternal()
{
    // NOTE:  this is the internal version that is done after calling
    // prepare number, which will return a version we can make alterations
    // to.

    // if this is exactly zero, then the floor is always zero
    if (isZero())
    {
        return IntegerZero;
    }
    // if this is a positive number, then this is the same as trunc
    else if (isPositive())
    {
        return truncInternal(0);
    }
    else
    {
        // since we have to go lower, first check to see if there are
        // any non-zero decimals.  If not, then we can call trunc
        // directly to format.  If there are non-zero decimals, we add one
        // to the value and then let trunc do the heavy lifting.

        // not have a decimal part?  If no decimal part, just pass to trunc
        if (numberExponent >= 0)
        {
            return truncInternal(0);
        }
        else
        {
            // number has a decimal part, so we need to see if there are
            // any non-zero decimals

            // get the number of decimals we need to scan
            size_t decimals = std::min(digitsCount, -numberExponent);
            // get the position to start the scan
            size_t lastDecimal = digitsCount - 1;
            bool foundNonZero = false;
            for (size_t i = decimals; i > 0; i--)
            {
                // if we found a non-zero, we have to do this the hard way
                if (numberDigits[lastDecimal--] != 0)
                {
                    foundNonZero = true;
                    break;
                }
            }

            // no-nonzero values, we can allow trunc to handle it from here
            if (!foundNonZero)
            {
                return truncInternal(0);
            }

            // ok, we need to add 1 to this value, and then allow trunc to finish the
            // job.  Unfortunately, this might require us to round out of the top of
            // the number, so we need to account for this

            // get the leading digits of the value
            wholenumber_t integer_digits = digitsCount + numberExponent;
            // if there are no integer digits, then this number is between 0 and -1.
            // we can just punt here and return -1.
            if (integer_digits <= 0)
            {
                return IntegerMinusOne;
            }
            else
            {
                // ok, we have integer digits.  Let's make this easy at this
                // point by chopping this down to just the integer digits and
                // throwing away the exponent
                digitsCount = integer_digits;
                numberExponent = 0;

                // point to the first digit, and start adding until
                // we no longer round
                char *current = numberDigits + integer_digits - 1;

                while (current >= numberDigits)
                {
                    int ch = *current + 1;
                    // did this round?
                    if (ch > 9)
                    {
                        // overwrite with a zero and keep looping
                        *current-- = 0;
                    }
                    else
                    {
                        // overwrite this value and have the trunc function
                        // format the value
                        *current = ch;
                        return truncInternal(0);
                    }
                }

                // ok, we rounded all the way out.  At this point, every digit in
                // the buffer is a zero.  Doing the rounding is easy here, just
                // stuff a 1 in the first digit and bump the exponent by 1
                *numberDigits = 1;
                numberExponent += 1;
                // and again, the trunc code can handle all of the formatting
                return truncInternal(0);
            }
        }
    }
}


/**
 * Return the ceiling value for a decimal numeric value.  The
 * ceiling is the first full number value greater that the
 * current value. If the number has no significant decimal
 * positions, this returns the same value (minus any trailing
 * decimal zeros). NOTE:  If the number is negative, the value
 * goes toward the higher number (e.g., the ceiling of -3.5 is
 * -3, not
 * -4).
 *
 * @return The numeric ceiling of this value.
 */
RexxObject *NumberString::ceiling()
{
    return prepareNumber(number_digits(), ROUND)->ceilingInternal();
}

/**
 * Calculate the ceiling value for a number.
 *
 * @return A string value of the ceiling, with appropriate
 *         formatting for the function.
 */
RexxObject *NumberString::ceilingInternal()
{
    // NOTE:  this is the internal version that is done after calling
    // prepare number, which will return a version we can make alterations
    // to.

    // if this is exactly zero, then the ceiling is always zero
    if (isZero())
    {
        return IntegerZero;
    }
    // if this is a negative number, then this is the same as trunc
    else if (isNegative())
    {
        return truncInternal(0);
    }
    else
    {
        // since we have to go higher, first check to see if there are
        // any non-zero decimals.  If not, then we can call trunc
        // directly to format.  If there are non-zero decimals, we add one
        // to the value and then let trunc do the heavy lifting.

        // not have a decimal part?  If no decimal part, just pass to trunc
        if (!hasDecimals())
        {
            return truncInternal(0);
        }
        else
        {
            // number has a decimal part, so we need to see if there are
            // any non-zero decimals

            // get the number of decimals we need to scan
            size_t decimals = std::min(digitsCount, -numberExponent);
            // get the position to start the scan
            wholenumber_t lastDecimal = digitsCount - 1;
            bool foundNonZero = false;
            for (size_t i = decimals; i > 0; i--)
            {
                // if we found a non-zero, we have to do this the hard way
                if (numberDigits[lastDecimal--] != 0)
                {
                    foundNonZero = true;
                    break;
                }
            }

            // no-nonzero values, we can allow trunc to handle it from here
            if (!foundNonZero)
            {
                return truncInternal(0);
            }

            // ok, we need to add 1 to this value, and then allow trunc to finish the
            // job.  Unfortunately, this might require us to round out of the top of
            // the number, so we need to account for this

            // get the leading digits of the value
            wholenumber_t integer_digits = digitsCount + numberExponent;
            // if there are no integer digits, then this number is between 0 and 1.
            // we can just punt here and return 1.
            if (integer_digits <= 0)
            {
                return IntegerOne;
            }
            else
            {
                // ok, we have integer digits.  Let's make like easy at this
                // point by chopping this down to just the integer digits and
                // throwing away the exponent
                digitsCount = integer_digits;
                numberExponent = 0;

                // point to the first digit, and start adding until
                // we no longer round
                char *current = numberDigits + integer_digits - 1;

                while (current >= numberDigits)
                {
                    int ch = *current + 1;
                    // did this round?
                    if (ch > 9)
                    {
                        // overwrite with a zero and keep looping
                        *current-- = 0;
                    }
                    else
                    {
                        // overwrite this value and have the trunc function
                        // format the value
                        *current = ch;
                        return truncInternal(0);
                    }
                }

                // ok, we rounded all the way out.  At this point, every digit in
                // the buffer is a zero.  Doing the rounding is easy here, just
                // stuff a 1 in the first digit and bump the exponent by 1
                *numberDigits = 1;
                numberExponent += 1;
                // and again, the trunc code can handle all of the formatting
                return truncInternal(0);
            }
        }
    }
}


/**
 * Round the number value depending on the value of the first
 * decimal position using standard rounding rules.
 * NOTE: this uses the same rules for floor and ceiling when
 * determining how things are rounded.  This is really defined
 * as floor(number + .5).  Thus 3.4 and -3.4 round to 3 and -3,
 * since they end up calculating floor(3.9) and floor(-2.9),
 * repectively.
 *
 * @return The rounded value
 */
RexxObject *NumberString::round()
{
    return prepareNumber(number_digits(), ROUND)->roundInternal();
}

/**
 * Calculate the rounded number value.
 *
 * @return A string value of the rounded number.
 */
RexxObject *NumberString::roundInternal()
{
    // NOTE:  this is the internal version that is done after calling
    // prepare number, which will return a version we can make alterations
    // to.

    // if this is exactly zero, then the rounded value is always zero
    if (isZero())
    {
        return IntegerZero;
    }
    // ok, regardless of the sign, we first decide based on the first
    // decimal which way we are rounding.
    else
    {
        // since we might have to go higher, first check to see if there are
        // any non-zero decimals.  If not, then we can call trunc
        // directly to format.  Otherwise, we need to look at the first
        // decimal position and see which way we go.

        // not have a decimal part?  If no decimal part, just pass to trunc
        if (numberExponent >= 0)
        {
            return truncInternal(0);
        }
        else
        {
            // number has a decimal part, so we need to look at the first
            // decimal position

            // get the leading digits of the value
            wholenumber_t integer_digits = digitsCount + numberExponent;
            // if the exponent is larger than the number of significant digits, then
            // the first digit to the right of the decimal is zero, so this will always
            // round to zero
            if (integer_digits < 0)
            {
                return IntegerZero;
            }
            else
            {
                // ok, we have integer digits and decimals.  The returned value will
                // be an integer, so we can just set the exponent to zero now and
                // throw away the decimal part
                digitsCount = integer_digits;
                numberExponent = 0;

                // Now we need to look at the first decimal and see if
                // we're rounding up
                char *current = numberDigits + integer_digits;
                // no rounding needed, go do the formatting
                if (*current < 5)
                {
                    return truncInternal(0);
                }

                // we need to add one to the integer part...which of course
                // might round all the way out.
                current--;

                while (current >= numberDigits)
                {
                    int ch = *current + 1;
                    // did this round?
                    if (ch > 9)
                    {
                        // overwrite with a zero and keep looping
                        *current-- = 0;
                    }
                    else
                    {
                        // overwrite this value and have the trunc function
                        // format the value
                        *current = ch;
                        return truncInternal(0);
                    }
                }
                // no integer digits in the rounding?, then the result will be one
                // or minus one)
                if (digitsCount == 0)
                {
                    return isNegative() ? IntegerMinusOne : IntegerOne;
                }
                // ok, we rounded all the way out.  At this point, every digit in
                // the buffer is a zero.  Doing the rounding is easy here, just
                // stuff a 1 in the first digit and bump the exponent by 1
                *numberDigits = 1;
                numberExponent += 1;
                // and again, the trunc code can handle all of the formatting
                return truncInternal(0);
            }
        }
    }
}


/**
 * Format the numberstring data according to the format
 * function controls.
 *
 * @param Integers   The size of the integer section.
 * @param Decimals   The size of the decimals section.
 * @param MathExp    The size allocated to the exponent.
 * @param ExpTrigger The exponent trigger size.
 *
 * @return the formatted number.
 */
RexxString  *NumberString::formatRexx(RexxObject *Integers, RexxObject *Decimals,
  RexxObject *MathExp, RexxObject *ExpTrigger )
{
    wholenumber_t digits = number_digits();
    bool form = number_form();

    wholenumber_t integers = optionalNonNegative(Integers, -1, ARG_ONE);
    wholenumber_t decimals = optionalNonNegative(Decimals, -1, ARG_TWO);
    wholenumber_t mathExp = optionalNonNegative(MathExp, -1, ARG_THREE);
    wholenumber_t expTrigger = optionalNonNegative(ExpTrigger, digits, ARG_FOUR);
    // round to the current digits setting and format
    return prepareNumber(digits, ROUND)->formatInternal(integers, decimals, mathExp, expTrigger, this, digits, form);
}


/**
 * Format the numberstring data according to the format
 * function controls.
 *
 * @param integers   The space allocated for the integers.
 * @param decimals   The space allocated for the decimals.
 * @param mathexp    The space allocated for the exponent.
 * @param exptrigger The exponent trigger value.
 * @param original   The original numberstring we're formatting from (used for error reporting)
 * @param digits     The digits value we're formatting to.
 * @param form       The required ENGINEERING/SCIENTIFIC form.
 *
 * @return The formatted number.
 */
RexxString *NumberString::formatInternal(wholenumber_t integers, wholenumber_t decimals, wholenumber_t mathexp,
    wholenumber_t exptrigger, NumberString *original, wholenumber_t digits, bool form)
{

    // if we have an exponent, we will format this early
    // so that we know the length.  Set this up as a null string
    // value to start.
    char   stringExponent[15];
    stringExponent[0] = '\0';

    // This is the value displayed as an exponent...default this to zero
    wholenumber_t displayedExponent = 0;
    // we only turn this on if the number needs to have an exponent displayed.  If
    // the calculated exponent
    bool showExponent = false;

    // we need to calculate a whole bunch of size sections.  Initialize
    // them all to zero for now.
    wholenumber_t size = 0;
    wholenumber_t leadingSpaces = 0;
    wholenumber_t integerDigits = 0;
    wholenumber_t trailingIntegerZeros = 0;
    wholenumber_t leadingDecimalZeros = 0;
    wholenumber_t decimalDigits = 0;
    wholenumber_t decimalSpace = 0;
    wholenumber_t leadingExpZeros = 0;
    wholenumber_t trailingDecimalZeros = 0;
    wholenumber_t exponentSize = 0;

    // are we allowed to have exponential form?  Need to figure out
    // if we need to use this.  NOTE:  The only not-allowed version is
    // an explicit 0. The default value of -1 also applies here.
    if (mathexp != 0)
    {
        wholenumber_t adjustedLength = numberExponent + digitsCount - 1;

        // our default trigger is the digits setting, but this can be set lower
        // than that on the call.  If the number of decimals is greater than the
        // trigger value, or the space required to format a pure decimal number or more than
        // twice the trigger value, we're in exponential form
        if (adjustedLength >= exptrigger || (adjustedLength < 0 && std::abs(numberExponent) > exptrigger * 2))
        {
            // we only handle this is the trigger is not explicitly zero
            // we need to show the exponent if this is specified (so far)
            showExponent = true;
            if (form == Numerics::FORM_ENGINEERING)
            {
                // if the whole portion is decimals, force an adjustment two characters to the left.
                if (adjustedLength < 0)
                {
                    adjustedLength = adjustedLength - 2;
                }
                // and round this to multiples of 3
                adjustedLength = (adjustedLength / 3) * 3;
            }
            // adjust the exponent value
            numberExponent = numberExponent - adjustedLength;
            // the exponent factor will be added in to the exponent when formatted
            displayedExponent = adjustedLength;
            // format the exponent to a string value now.
            Numerics::formatWholeNumber(std::abs(displayedExponent), stringExponent);
            exponentSize = strlen(stringExponent);
            // if the exponent size is not defaulted, then test that we have space
            // to fit this.
            if (mathexp != -1)
            {
                leadingExpZeros = mathexp - exponentSize;
                // not enough space for this exponent value?  That is an error.
                if (exponentSize > mathexp)
                {
                    reportException(Error_Incorrect_method_exponent_oversize, this, mathexp);
                }
            }
        }
    }

    // using the default for the decimals?
    if (decimals == -1)
    {
        // a negative exponent determines how many decimal positions there are
        if (numberExponent < 0)
        {
            decimalDigits = -numberExponent;
            // if this is a very large right shift, we may need some leading zeros.
            if (decimalDigits > digitsCount)
            {
                leadingDecimalZeros = decimalDigits - digitsCount;
            }
            // record the total decimal space (including the period)
            decimalSpace = decimalDigits + leadingDecimalZeros;
        }
    }
    else
    {
        // we only have decimals in the number if we have a negative exponent.
        if (numberExponent < 0)
        {
            // get the number of decimals we'll have
            wholenumber_t adjustedDecimals = -numberExponent;
            // will we have more decimals than requested?  We'll need to
            // round or truncate.
            if (adjustedDecimals > decimals)
            {
                // get the amount we need to change by
                adjustedDecimals = adjustedDecimals - decimals;
                // we're going to chop the lengths a bit, so we also tweak the
                // exponent
                numberExponent = numberExponent + adjustedDecimals;
                // are we going to lose all of the digits?  Then we need to
                // round.
                if (adjustedDecimals >= digitsCount)
                {
                    // if the delta is exactly equal to the number of digits in
                    // the number, then we might need to round.  We become just a
                    // single digit number.
                    if (adjustedDecimals == digitsCount && numberDigits[0] >= 5)
                    {
                        numberDigits[0] = 1;
                    }
                    // truncating and becoming zero
                    else
                    {
                        numberDigits[0] = 0;
                        numberExponent = 0;
                        numberSign = 0;
                    }
                    // we become a single digit long regardless
                    digitsCount = 1;
                }
                // we're only losing some of the digits, need to handle
                // a slightly more complex rounding sitiuation.
                else
                {
                    // reduce the length
                    digitsCount -= adjustedDecimals;
                    // go round the number digit
                    mathRound(numberDigits);

                    // [bugs:#1474] has brought up a case, where although the initial
                    // exponential notation trigger had been true, when re-doing
                    // "the whole trigger thing" (see below), it became false.
                    // Neither do we want format() to switch from exponential to
                    // non-exponential just because of format() rounding reasons, nor
                    // can the code further down build the result string for this case
                    // without crashing (because trailingDecimalZeros becomes negative).
                    // To fix we apply some sort of kludge here:
                    // we simply remember the triggered exponential state and let it
                    // override the outcome of the second trigger calulation
                    bool showExponentWasTrue = showExponent;

                    // calculate new adjusted value, which means we have to redo
                    // the previous exponent calculation.
                    // needed for format(.999999,,4,2,2)
                    if (mathexp != 0 && displayedExponent != 0)
                    {
                        // adjust the exponent back to the orignal and set our display
                        // exponent back to zero.
                        numberExponent += displayedExponent;
                        displayedExponent = 0;
                        strcpy(stringExponent, "0");
                        exponentSize = 1;
                        // turn this all off for the moment
                        showExponent = false;
                    }

                    wholenumber_t adjustedExponent = numberExponent + digitsCount - 1;

                    // redo the whole trigger thing
                    if (showExponentWasTrue | (mathexp != 0 && (adjustedExponent >= exptrigger || (adjustedExponent < 0 && std::abs(numberExponent) > exptrigger * 2))))
                    {
                        // this might not have been set on originally, but only occurring because of
                        // the rounding.
                        showExponent = true;
                        if (form == Numerics::FORM_ENGINEERING)
                        {
                            if (adjustedExponent < 0)
                            {
                                adjustedExponent = adjustedExponent - 2;
                            }
                            adjustedExponent = (adjustedExponent / 3) * 3;
                        }

                        // reapply the exponent adjustment
                        numberExponent -= adjustedExponent;
                        displayedExponent = adjustedExponent;
                        // format exponent to a string
                        Numerics::formatWholeNumber(std::abs(displayedExponent), stringExponent);
                        // and get the new exponent size
                        exponentSize = strlen(stringExponent);
                        // if we have an explicit size, get the fill zeros as well.
                        if (mathexp != -1)
                        {
                            leadingExpZeros = mathexp - exponentSize;
                        }

                        // if we had an explicit exponent size, then make sure we can fit this
                        // exponent in that space.
                        if (mathexp != -1)
                        {
                            // not enough space for this exponent value?  That is an error.
                            if (exponentSize > mathexp)
                            {
                                reportException(Error_Incorrect_method_exponent_oversize, this, mathexp);
                            }
                        }
                    }
                }
            }

            // ok, all changes to the numbers have been made.  We need to
            // redo the space calculations.
            adjustedDecimals = -numberExponent;
            // more decimal positions than we have digits?  We're going to have
            // to add leading zeros for pad.
            if (adjustedDecimals > digitsCount)
            {
                leadingDecimalZeros = adjustedDecimals - digitsCount;
                decimalDigits = digitsCount;
            }
            // no leading zeros, and we have fewer real digits
            else
            {
                decimalDigits = adjustedDecimals;
            }

            // this can happen if there is a round out on the top digit when we
            // are truncating and exponential notation is now required. In that situation
            // we have more decimals that we are allowed to use
            if (adjustedDecimals > decimals)
            {
                adjustedDecimals = decimals;
                decimalDigits = decimals;
            }
            // in theory, everything has been adjusted to the point where
            // decimals is >= to the adjusted size
            trailingDecimalZeros = decimals - adjustedDecimals;
        }
        // we have an explicit size specified, but the number itself has no decimals.
        // these all become trailing zeros
        else
        {
            trailingDecimalZeros = decimals;
        }

        // record the total decimal space (including the period)
        decimalSpace = decimalDigits + leadingDecimalZeros + trailingDecimalZeros;
    }

    // using the default integers value?
    if (integers == -1)
    {
        // the integer digits is determined by adding the
        // count of digits to the exponent.  If we end up losing
        // all of the integers, then our value is 1 (for the leading "0").
        integers = digitsCount + numberExponent;
        if (integers <= 0)
        {
            integers = 1;
            integerDigits = 0;
            trailingIntegerZeros = 1;
        }
        else
        {
            integerDigits = integers;
            // we might need to add some trailing zeros when this is expanded out and also
            // cap the digits we use
            if (integers > digitsCount)
            {
                integerDigits = digitsCount;
                trailingIntegerZeros = integers - digitsCount;
            }
        }
    }
    //. working with a user-requested size.
    else
    {
        wholenumber_t reqIntegers = integers;
        // the integer size value includes the sign, so reduce by one if we are negative
        if (isNegative())
        {
            integers = integers - 1;
        }

        // the integer digits is determined by adding the
        // count of digits to the exponent.  If we end up losing
        // all of the integers, then our value is 1 (for the leading "0").
        wholenumber_t neededIntegers = digitsCount + numberExponent;

        if (neededIntegers <= 0)
        {
            neededIntegers = 1;
            integerDigits = 0;
            trailingIntegerZeros = 1;
        }
        else
        {
            integerDigits = neededIntegers;
            // if we're need more than the digits we have, get the count of trailing
            // zeros.
            if (neededIntegers > digitsCount)
            {
                integerDigits = digitsCount;
                trailingIntegerZeros = neededIntegers - digitsCount;
            }

        }

        // not enough room for this number?  this is an error
        if (integers < neededIntegers)
        {
            reportException(Error_Incorrect_method_before_oversize, this, reqIntegers);
        }
        // calculate the leading spaces now
        leadingSpaces = integers - neededIntegers;
    }

    // all of the adjustments have been made.  Now start calculating the
    // result size so we can allocate a string object to build this up.
    size = 0;

    if (isNegative())
    {
        size++;
    }

    // add in the size for the integer stuff
    size += leadingSpaces;
    size += integerDigits + trailingIntegerZeros;
    // and the decimal part...don't forget to add in the period.
    if (decimalSpace > 0)
    {
        size += decimalSpace + 1;
    }

    // do we have an exponent to add?
    if (showExponent)
    {
        if (mathexp != -1 || displayedExponent != 0)
        {
            // this size is the same regardless of whether the exponent value is zero
            // add two for the E and the sign,
            size += 2;
            // we might need some leading zeros on this if we have
            // an explicit exponent size
            // the mathexp size is what we want.
            size += exponentSize + leadingExpZeros;
        }
    }

    RexxString *result = raw_string(size);
    NumberBuilder builder(result);

    // add the leading spaces
    builder.addSpaces(leadingSpaces);
    // build the integer part
    builder.addIntegerPart(isNegative(), numberDigits, integerDigits, trailingIntegerZeros);

    // if we have a decimal portion, add that now
    if (decimalSpace > 0)
    {
        builder.addDecimalPart(numberDigits + integerDigits, decimalDigits, leadingDecimalZeros, trailingDecimalZeros);
    }

    // have an exponent to add?
    if (showExponent)
    {
        // if we have a non-zero exponent, then format it
        if (displayedExponent != 0)
        {
            // add the exponent marker
            builder.append('E');
            builder.addExponentSign(displayedExponent < 0);
            // add any padding zeros needed.
            builder.addZeros(leadingExpZeros);
            // add the real exponent part
            builder.append(stringExponent, exponentSize);
        }
        // a zero exponent is always expressed as spaces, but only if we have an explicit size
        else if (mathexp != -1)
        {
            builder.addSpaces(leadingExpZeros + exponentSize + 2);
        }
    }
    return result;
}


// different scanning states for scanning numeric symbols
typedef enum
{
    NUMBER_START,
    NUMBER_SIGN,
    NUMBER_SIGN_WHITESPACE,
    NUMBER_DIGIT,
    NUMBER_SPOINT,
    NUMBER_POINT,
    NUMBER_E,
    NUMBER_ESIGN,
    NUMBER_EDIGIT,
    NUMBER_TRAILING_WHITESPACE,
} NumberScanState;


/**
 * Simple class to make scanning the string value a little easier.
 */
class StringScanner
{
public:
    StringScanner(const char *s, size_t l)
    {
        start = s;
        current = s;
        end = s + l;
        length = l;
    }

    inline bool atEnd() { return current >= end; }
    inline char getChar() { return *current;}
    inline char nextChar() { return *current++; }
    inline void stepPosition() { current++; }
    inline bool moreChars() { return current < end; }

protected:
    const char *start;
    const char *current;
    const char *end;
    size_t length;
};


/**
 * A simple class to assist with the process of scanning
 * a string value and creating a number string.
 */
class NumberStringBuilder
{
public:
    NumberStringBuilder(NumberString *s)
    {
        number = s;
        // pointers for adding to the digits buffer
        current = s->numberDigits;


        // default to a positive exponent
        exponentSign = 1;
        // the exponent is zero at the start
        exponent = 0;
        // no decimals at the start
        decimals = 0;
        // we don't accumulate digits until we hit some sort of
        // digit.
        scannedDigits = 0;
        haveNonZero = false;
        invalidExponent = false;
    }

    void addDigit(char d)
    {
        // if this is a zero digit, only add this if
        // we have previously seen a non-zero digit.
        if (d == RexxString::ch_ZERO)
        {
            // only add if this is a significant zero
            if (haveNonZero)
            {
                *current++ = '\0';
            }
        }
        // a non zero digit is always significant
        else
        {
            // zeros are significant now
            haveNonZero = true;
            // store in binary form
            *current++ = d - '0';
        }
        // in case this reduces to just zero, remember that we've
        // seen some sort of digit in the number
        scannedDigits++;
    }

    // add an integer digit before the decimal point
    void addIntegerDigit(char d)
    {
        // nothing much extra here.
        addDigit(d);
    }

    // add a digit found after the decimal point.  We also keep
    // count of the number of these for the final exponent value
    void addDecimalDigit(char d)
    {
        // add the digit
        addDigit(d);
        // even if this was a non-significant zero digit, it still
        // contributes to the exponent information
        decimals++;
    }

    void setSign(char s)
    {
        // set the number sign
        number->numberSign = s == RexxString::ch_MINUS ? -1 : 1;
    }

    void setExponentSign(char s)
    {
        exponentSign = s == RexxString::ch_MINUS ? -1 : 1;
    }

    void addExponentDigit(char d)
    {
        wholenumber_t new_exponent = (exponent * 10) + (d - RexxString::ch_ZERO);
        // now we need to check for an exponent that is too large or one that
        // caused an overflow
        if (new_exponent > Numerics::MAX_EXPONENT || new_exponent < exponent)
        {
            // set this to an invalid exponent so we can flag this as invalid later
            invalidExponent = true;
        }
        else
        {
            // this is a good value (so far)
            exponent = new_exponent;
        }
    }

    bool finish()
    {
        // with all of the state changes, it is actually possible to
        // not encounter a single digit in the number...make sure we
        // added at least one digits, even if it was zero.  We're not picky
        // about where the digit appears, but we much have at least one.
        if (scannedDigits == 0)
        {
            return false;
        }
        // if all we've seen are zeros, make this exactly zero
        if (!haveNonZero)
        {
            number->setZero();
            return true;
        }

        // if the exponent was larger than 9-nines, this is invalid
        if (invalidExponent)
        {
            return false;
        }

        // set the count of digits in the number
        number->digitsCount = current - number->numberDigits;
        // get the final exponent value
        number->numberExponent = (exponent * exponentSign) - decimals;
        // a couple of final exponent checks
        // since the number of decimals enters into the exponent, we need to
        // verify that the calculated exponent did not cause an underflow
        if (std::abs(number->numberExponent) > Numerics::MAX_EXPONENT)
        {
            return false;
        }

        // we can also go the other way, where the number of digits can cause the exponent
        // to overflow if expressed in scientific notation.
        if ((number->numberExponent + number->digitsCount - 1) > Numerics::MAX_EXPONENT)
        {
            return false;
        }
        // we have a good number
        return true;
    }

protected:
    NumberString *number;               // the number we're building
    char *current;                      // the current spot for adding a new digit
    int exponentSign;                   // any sign for our exponent
    wholenumber_t exponent;             // our exponent accumulator
    wholenumber_t decimals;             // the decimal shift on the exponent
    wholenumber_t scannedDigits;        // indicate we've seen digits
    bool haveNonZero;                   // we've seen a non-zero digit
    bool invalidExponent;               // the exponent is larger than 9-nines
};



/**
 * Format a string value into a numberString object.
 *
 * @param number The source character string.
 * @param length The length of the character string.
 *
 * @return true if this was a valid number, false for an invalid
 *         number.
 */
bool NumberString::parseNumber(const char *number, size_t length)
{

    // we're in a clean scan state now
    NumberScanState state = NUMBER_START;
    StringScanner scanner(number, length);

    // get a builder to accumulate the information
    NumberStringBuilder builder(this);

    // get the current character to start this off
    char inch = scanner.getChar();
    // ok, loop through the token until we've consumed it all.
    for (;;)
    {
        // finite state machine to establish numeric constant (with possible
        // included sign in exponential form)

        switch (state)
        {
            // this is our beginning state...we know nothing about this number yet.
            case NUMBER_START:
            {
                // have whitespace at the start?  We're removing those.  The
                // state remains NUMBER_START until we get something real.
                if (inch == RexxString::ch_SPACE || inch == RexxString::ch_TAB)
                {
                    state = NUMBER_START;
                }
                else if (inch == RexxString::ch_PLUS || inch == RexxString::ch_MINUS)
                {
                    // set the sign appropriately in the number
                    builder.setSign(inch);
                    state = NUMBER_SIGN;
                }
                // have a digit at the start?  Potential number, so
                // we're looking for digits here.
                else if (inch >= RexxString::ch_ZERO && inch <= RexxString::ch_NINE)
                {
                    // this is an integer digit, add it to the number
                    builder.addIntegerDigit(inch);
                    state = NUMBER_DIGIT;
                }
                // if this is a dot, then we've got a starting decimal
                // point.  This could be a number or an environment symbol
                else if (inch == RexxString::ch_PERIOD)
                {
                    state = NUMBER_SPOINT;
                }
                // a non-numeric character.  A number is not possible.
                else
                {
                    return false;
                }
                break;
            }

            // removing whitespace after a sign character
            case NUMBER_SIGN_WHITESPACE:
            {
                // another whitespace character?  continue in this state
                // state remains NUMBER_START until we get something real.
                if (inch == RexxString::ch_SPACE || inch == RexxString::ch_TAB)
                {
                    break;
                }
                // have a digit at the start?  Potential number, so
                // we're looking for digits here.
                else if (inch >= RexxString::ch_ZERO && inch <= RexxString::ch_NINE)
                {
                    // this is an integer digit, add it to the number
                    builder.addIntegerDigit(inch);
                    state = NUMBER_DIGIT;
                }
                // if this is a dot, then we've got a starting decimal
                // point.  This could be a number or an environment symbol
                else if (inch == RexxString::ch_PERIOD)
                {
                    state = NUMBER_SPOINT;
                }
                // a non-numeric character.  A number is not possible.
                else
                {
                    return false;
                }
                break;
            }


            // we just had a sign character.  Possibilities
            // here are 1) digits, 2) a period, or 3) whitespace after the
            // sign but before the number
            case NUMBER_SIGN:
            {
                // have a digit at the start?  Potential number, so
                // we're looking for digits here.
                if (inch >= RexxString::ch_ZERO && inch <= RexxString::ch_NINE)
                {
                    // this is an integer digit, add it to the number
                    builder.addIntegerDigit(inch);
                    state = NUMBER_DIGIT;
                }
                // if this is a dot, then we've got a starting decimal
                // point.  This could be a number or an environment symbol
                else if (inch == RexxString::ch_PERIOD)
                {
                    state = NUMBER_SPOINT;
                }
                // have whitespace after the sign?  need to scan that off
                else if (inch == RexxString::ch_SPACE || inch == RexxString::ch_TAB)
                {
                    state = NUMBER_SIGN_WHITESPACE;
                }
                // a non-numeric character.  A number is not possible.
                else
                {
                    return false;
                }
                break;
            }

            // we're scanning digits, still potentially a number.
            case NUMBER_DIGIT:
            {
                // other non-digit?  We're no longer scanning a number.
                if (inch >= RexxString::ch_ZERO && inch <= RexxString::ch_NINE)
                {
                    // add this digit to the number, the state is unchanged
                    builder.addIntegerDigit(inch);
                }
                // is this a period?  Since we're scanning digits, this
                // is must be the first period and is a decimal point.
                // switch to scanning the part after the decimal.
                else if (inch == RexxString::ch_PERIOD)
                {
                    state = NUMBER_POINT;
                }
                // So far, the form is "digitsE"...this can still be a number,
                // but know we're looking for an exponent.
                else if (inch=='E' || inch == 'e')
                {
                    state = NUMBER_E;
                }
                // could be trailing blanks here
                else if (inch == RexxString::ch_SPACE || inch == RexxString::ch_TAB)
                {
                    state = NUMBER_TRAILING_WHITESPACE;
                }
                // something else
                else
                {
                    return false;
                }
                break;
            }


            // we're scanning from a leading decimal point.  How we
            // go from here depends on the next character.
            case NUMBER_SPOINT:
            {
                // not a digit immediately after the period, we don't have a
                // number any more
                if (inch < RexxString::ch_ZERO || inch > RexxString::ch_NINE)
                {
                    return false;
                }
                // second character is a digit, so we're scanning the
                // part after the decimal.
                else
                {
                    // this is a decimal digit, add it to the number
                    builder.addDecimalDigit(inch);
                    state = NUMBER_POINT;
                }
                break;
            }

            // scanning after a decimal point.  From here, we could hit
            // the 'E' for exponential notation.
            case NUMBER_POINT:
            {
                // found a digit?  add to the number and continue in the same state
                if (inch >= RexxString::ch_ZERO && inch <= RexxString::ch_NINE)
                {
                    builder.addDecimalDigit(inch);
                }
                // potential exponential, switch scan to the exponent part.
                else if (inch == 'E' || inch == 'e')
                {
                    state = NUMBER_E;
                }
                // could be trailing blanks here
                else if (inch == RexxString::ch_SPACE || inch == RexxString::ch_TAB)
                {
                    state = NUMBER_TRAILING_WHITESPACE;
                }
                // non-digit other than an 'E'?, no longer a valid numeric.
                else
                {
                    return false;
                }
                break;
            }

            // we have a valid number up to an 'E'...now we can have digits or
            // a sign for the exponent.
            case NUMBER_E:
            {
                // switching to process the exponent digits
                if (inch >= RexxString::ch_ZERO && inch <= RexxString::ch_NINE)
                {
                    // consume the exponent digit
                    builder.addExponentDigit(inch);
                    state = NUMBER_EDIGIT;
                }
                // potential sign for the exponent
                else if (inch == RexxString::ch_PLUS || inch == RexxString::ch_MINUS)
                {
                    // set the sign appropriately in the number
                    builder.setExponentSign(inch);
                    state = NUMBER_ESIGN;
                }
                // something invalid after the "E"
                else
                {
                    return false;
                }
                break;
            }

            // we're scanning a potential numeric value, and we've just
            // had the sign, so we're looking for digits after that.   If there
            // are no digits, we have a problem
            case NUMBER_ESIGN:
            {
                // found a digit here?  switching into exponent scan mode.
                if (inch >= RexxString::ch_ZERO && inch <= RexxString::ch_NINE)
                {
                    // add the first digit here
                    builder.addExponentDigit(inch);
                    state = NUMBER_EDIGIT;
                }
                else
                {
                    // non-digit cannot be a number.
                    return false;
                }
                break;
            }

            // scanning for exponent digits.  No longer numeric if we find a non-digit,
            // although we can switch to whitespace
            case NUMBER_EDIGIT:
            {
                if (inch >= RexxString::ch_ZERO && inch <= RexxString::ch_NINE)
                {
                    // process the digit
                    builder.addExponentDigit(inch);
                }
                // have whitespace after the sign?  need to scan that off
                else if (inch == RexxString::ch_SPACE || inch == RexxString::ch_TAB)
                {
                    state = NUMBER_TRAILING_WHITESPACE;
                }
                else
                {
                    return false;
                }
                break;
            }

            // removing trailing whitespace.  Only whitespace is allowed from here.
            case NUMBER_TRAILING_WHITESPACE:
            {
                // non-whitespace not allowed.
                if (inch != RexxString::ch_SPACE && inch != RexxString::ch_TAB)
                {
                    return false;
                }
            }
        }

        // handled all of the states, now handle the termination checks.
        scanner.stepPosition();

        // have we reached the end of the line?  Also done.
        if (!scanner.moreChars())
        {
            break;
        }

        // get the next character
        inch = scanner.getChar();
    }

    // if this fails final validation checks, return the failure.
    return builder.finish();
}


/**
 * Create a numberstring object from a whole number value.
 *
 * @param integer The integer value.
 */
void NumberString::formatNumber(wholenumber_t integer)
{
    // if the value is zero, this is easy
    if (integer == 0)
    {
        setZero();
    }
    else
    {
        numberSign = integer < 0 ? -1 : 1;
        // format this into the digits buffer
        digitsCount = Numerics::normalizeWholeNumber(integer, numberDigits);
    }
}


/**
 * Format the integer num into a numberstring.
 *
 * @param integer The unsigned integer value.
 */
void NumberString::formatUnsignedNumber(size_t integer)
{
    // zero is easy
    if (integer == 0)
    {
        setZero();
    }
    else
    {
        // this is a positive value
        numberSign = 1;
        // format the string value into our buffer
        Numerics::formatStringSize(integer, numberDigits);

        // normalize all of the digits
        char *current = numberDigits;
        while (*current != '\0')
        {
            *current -= '0';
            current++;
        }
        digitsCount = current - numberDigits;
    }
}


/**
 * Format a 64-bit number into a numberstring.
 *
 * @param integer The value to format.
 */
void NumberString::formatInt64(int64_t integer)
{
    // zero is easy
    if (integer == 0)
    {
        setZero();
    }
    else
    {
        // we convert this directly because portable numeric-to-ascii routines
        // don't really exist for the various 32/64 bit values.
        char buffer[32];
        size_t index = sizeof(buffer);

        // negative number?  copy a negative sign, and take the abs value
        if (integer < 0)
        {
            // work from an unsigned version that can hold all of the digits
            // we need to use a version we can negate first, then add the
            // digit back in
            uint64_t working = (uint64_t)(-(integer + 1));
            working++;      // undoes the +1 above
            numberSign = -1;      // negative number

            while (working > 0)
            {
                // get the digit and reduce the size of the integer
                int digit = (int)(working % 10);
                working = working / 10;
                // store the digit
                buffer[--index] = digit;
            }
        }
        else
        {
            numberSign = 1;              // positive number
            while (integer > 0)
            {
                // get the digit and reduce the size of the integer
                int digit = (int)(integer % 10);
                integer = integer / 10;
                // store the digit
                buffer[--index] = digit;
            }
        }

        // copy into the buffer and set the length
        digitsCount = sizeof(buffer) - index;
        memcpy(numberDigits, &buffer[index], digitsCount);
    }
}


/**
 * Format an unsigned 64-bit integer num into a numberstring.
 *
 * @param integer The integer value.
 */
void NumberString::formatUnsignedInt64(uint64_t integer)
{
    if (integer == 0)
    {
        setZero();
    }
    else
    {
        // we convert this directly because A)  we need to post-process the numbers
        // to make them zero based, and B) portable numeric-to-ascii routines
        // don't really exist for the various 32/64 bit values.
        char buffer[32];
        size_t index = sizeof(buffer);

        while (integer > 0)
        {
            // get the digit and reduce the size of the integer
            int digit = (int)(integer % 10);
            integer = integer / 10;
            // store the digit
            buffer[--index] = digit;
        }

        // copy into the buffer and set the length
        digitsCount = sizeof(buffer) - index;
        memcpy(numberDigits, &buffer[index], digitsCount);
    }
}


/**
 * Process an unknown message condition on an object.  This is
 * an optimized bypass for the Object default method that can
 * bypass creating an array for the arguments and sending the
 * UNKNOWN message to the object.  Since many things funnel
 * through the integer unknown method, this is a big
 * optimization.
 *
 * @param messageName
 *                  The target message name.
 * @param arguments The message arguments.
 * @param count     The count of arguments.
 * @param result    The return result protected object.
 */
void NumberString::processUnknown(RexxErrorCodes error, RexxString *messageName, RexxObject **arguments, size_t count, ProtectedObject &result)
{
    // just send this as a message directly to the string object.
    stringValue()->messageSend(messageName, arguments, count, result);
}


/**
 * Wrapper around the compareTo() method that does string sort
 * comparisons.
 *
 * @param other  The other comparison object
 *
 * @return -1, 0, 1 depending on the comparison result.
 */
wholenumber_t NumberString::compareTo(RexxInternalObject *other )
{
    // just send this as a message directly to the string object.
    return stringValue()->compareTo(other);
}


/**
 * Override for the normal isinstanceof method.  This version
 * allows the NumberStringClass to "lie" about being a string.
 *
 * @param other  The comparison class
 *
 * @return True if the string value is an instance of the target class.
 */
bool NumberString::isInstanceOf(RexxClass *other)
{
    return stringValue()->isInstanceOf(other);
}


/**
 * Retrieve the method instance for an object's defined method.
 *
 * @param method_name
 *               The method name.
 *
 * @return The method object that implements the object method.
 */
MethodClass *NumberString::instanceMethod(RexxString  *method_name)
{
    return stringValue()->instanceMethod(method_name);
}


/**
 * Return a supplier containing the methods implemented by an
 * object.  Depending on the argument, this is either A) all of
 * the methods, B) just the explicitly set instance methods, or
 * C) the methods provided by a given class.
 *
 * @param class_object
 *               The target class object (optional).
 *
 * @return A supplier with the appropriate method set.
 */
SupplierClass *NumberString::instanceMethods(RexxClass *class_object)
{
    return stringValue()->instanceMethods(class_object);
}

// start of numberstring operators that just forward to the string object.

RexxString *NumberString::concatBlank(RexxObject *other)
{
    return stringValue()->concatBlank(other);
}

RexxString *NumberString::concat(RexxObject *other)
{
    return stringValue()->concatRexx(other);
}

RexxObject *NumberString::orOp(RexxObject *operand)
{
  return stringValue()->orOp(operand);
}

RexxObject *NumberString::andOp(RexxObject *operand)
{
  return stringValue()->andOp(operand);
}

RexxObject *NumberString::xorOp(RexxObject *operand)
{
  return stringValue()->xorOp(operand);
}


/**
 * Primitive strict equal\not equal method.  This determines
 * only strict equality, not greater or less than values.
 *
 * @param other  The other object.
 *
 * @return true if they are equal, false otherwise.
 */
bool NumberString::isEqual(RexxInternalObject *other)
{
    // perform the comparison using the string value because this is "==".
    return stringValue()->isEqual(other);
}


/**
 * strict comparison of two objects.
 *
 * @param other  The other object.
 *
 * @return The string compare result returned by the string representation.
 */
wholenumber_t NumberString::strictComp(RexxObject *other)
{
    // do this using the string form
    return stringValue()->primitiveStrictComp(other);
}


/**
 * Test for whether a LOSTDIGITS condition needs to be raised.
 *
 * @param digits The digits setting to test.
 */
void NumberString::checkLostDigits(wholenumber_t digits)
{
    if (digitsCount > digits)
    {
        reportCondition(GlobalNames::LOSTDIGITS, this);
    }
}


/**
 * To a comparison test for two number strings for
 * non-strict comparisons.
 *
 * @param right  The other comparison value.
 * @param fuzz   The fuzz value for the comparison.
 *
 * @return A value < 0 when this is smaller than the other.
 *         A value   0 when this is equal to the other
 *         A value > 0 when this is greater than the other.
 */
wholenumber_t NumberString::comp(RexxObject *right, size_t fuzz)
{
    // the compare is done by subtracting the two numbers, the
    // sign of the result obj will be our return value.
    requiredArgument(right, ARG_ONE);

    // get the numberstring value from the right side.  If this does
    // not convert, we need to handle this as a string comparison.
    NumberString *rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)
    {
        // just go directly to the string comparison part, since we know
        // numeric comparison is not possible.
        return stringValue()->stringComp(right->requestString());
    }

    // unfortunately, we need to perform any lostdigits checks before
    // handling any of the short cuts
    wholenumber_t digits = number_digits();

    checkLostDigits(digits);
    rightNumber->checkLostDigits(digits);

    // if the signs are different, this is an easy comparison.
    if (numberSign != rightNumber->numberSign)
    {
        return (numberSign < rightNumber->numberSign) ? -1 : 1;
    }

    // the signs are equal, so if this number is zero, so is the other
    // and they are equal
    if (isZero())
    {
        return 0;
    }

    // get the minimum exponent
    wholenumber_t minExponent = std::min(numberExponent, rightNumber->numberExponent);

    // get values adjusted for the relative magnatudes of the numbers.  This can
    // allow us to avoid performing the actual subtraction.
    wholenumber_t adjustedLeftExponent = numberExponent - minExponent;
    wholenumber_t adjustedRightExponent = rightNumber->numberExponent - minExponent;

    wholenumber_t adjustedLeftLength = digitsCount + adjustedLeftExponent;
    wholenumber_t adjustedRightLength = rightNumber->digitsCount + adjustedRightExponent;

    // get the digits value adjusted for the current fuzz.
    digits -= fuzz;

    // if both of these are in the fuzz range, the longer number is the largest,
    // although we need to take the sign into account.
    if (adjustedLeftLength <= digits && adjustedRightLength <= digits)
    {
        // the longer number is the winner
        if (adjustedLeftLength > adjustedRightLength)
        {
            return numberSign;
        }
        else if (adjustedRightLength > adjustedLeftLength)
        {
            return -numberSign;
        }
        // the numbers are of the same adjusted length.  For these
        // values, because they are within the fuzzy digits limit, we
        // can directly compare the digits rather than subtracting.
        else
        {
            // are the lengths the same (common)
            if (digitsCount == rightNumber->digitsCount)
            {

                return memcmp(numberDigits, rightNumber->numberDigits, digitsCount) * numberSign;
            }
            // the right one shorter?  Since the adjusted sizes are the
            // same, the digits line up in the buffer and we can compare for
            // the shorter length.  If they compare equal, then we need to scan
            // the longer string to see if there are any non-zero characters in
            // non-compared section
            else if (digitsCount > rightNumber->digitsCount)
            {
                int rc = memcmp(numberDigits, rightNumber->numberDigits, rightNumber->digitsCount) * numberSign;
                // if still equal, scan the remainder of our digits after the compared length
                if (rc == 0)
                {
                    const char *scan = numberDigits + rightNumber->digitsCount;
                    wholenumber_t count = digitsCount - rightNumber->digitsCount;
                    while (count--)
                    {
                        // if we have a non-zero digit, the left side is the larger
                        if (*scan++ != 0)
                        {
                            return numberSign;
                        }
                    }
                    // all zeros after the compared section, these are equal.
                    return 0;
                }
                // data compared differently
                return rc;
            }
            // inverse of the above compare...the left side is shorter
            else
            {
                int rc = memcmp(numberDigits, rightNumber->numberDigits, digitsCount) * numberSign;
                // if still equal, scan the remainder of our digits after the compared length
                if (rc == 0)
                {
                    const char *scan = rightNumber->numberDigits + digitsCount;
                    wholenumber_t count = rightNumber->digitsCount - digitsCount;
                    while (count--)
                    {
                        // if we have a non-zero digit, the right side is the larger
                        if (*scan++ != 0)
                        {
                            // we negate the sign for this return value
                            return -numberSign;
                        }
                    }
                    // all zeros after the compared section, these are equal.
                    return 0;
                }
                // data compared differently
                return rc;
            }
        }
    }
    // we need to subtract to get the result
    else
    {
        return addSub(rightNumber, OT_MINUS, digits)->numberSign;
    }
}


/**
 * non-strict "=" operator
 *
 * @param other  The other object for comparison.
 *
 * @return The comparison result.
 */
RexxObject *NumberString::equal(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other, number_fuzz()) == 0);
}


/**
 * non-strict "\=" operator
 *
 * @param other  The other comparison value.
 *
 * @return The compare result.
 */
RexxObject *NumberString::notEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals will not compare to .nil
    {
        return TheTrueObject;
    }
    return booleanObject(comp(other, number_fuzz()) != 0);
}


// macro for generating common comparison operators.
#define CompareOperator(comp)  \
    if (other == TheNilObject)  \
    {                           \
        return TheFalseObject;  \
    }                           \
    return booleanObject(comp);


RexxObject *NumberString::isGreaterThan(RexxObject *other)
{
    CompareOperator(comp(other, number_fuzz()) > 0);
}

RexxObject *NumberString::isLessThan(RexxObject *other)
{
    CompareOperator(comp(other, number_fuzz()) < 0);
}

RexxObject *NumberString::isGreaterOrEqual(RexxObject *other)
{
    CompareOperator(comp(other, number_fuzz()) >= 0);
}

RexxObject *NumberString::isLessOrEqual(RexxObject *other)
{
    CompareOperator(comp(other, number_fuzz()) <= 0);
}


/**
 * Exported version of the HASHCODE method for retrieving the
 * object's hash.
 *
 * @return A string version of the hash (generally holds binary characters).
 */
RexxObject *NumberString::hashCode()
{
    // get the hash value, which is actually derived from the integer string value
    HashCode h = hash();
    return new_string((const char *)&h, sizeof(HashCode));
}


/**
 * Perform the primitive level "==" compare
 *
 * @param other  The other for comparison.
 *
 * @return The compare result.
 */
RexxObject *NumberString::strictEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictComp(other) == 0);
}


/**
 * Strict inequality operation
 *
 * @param other  The other value for comparison.
 *
 * @return The comparison result.
 */
RexxObject *NumberString::strictNotEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheTrueObject;
    }
    return booleanObject(strictComp(other) != 0);
}


RexxObject *NumberString::strictGreaterThan(RexxObject *other)
{
    CompareOperator(strictComp(other) > 0);
}

RexxObject *NumberString::strictLessThan(RexxObject *other)
{
    CompareOperator(strictComp(other) < 0);
}

RexxObject *NumberString::strictGreaterOrEqual(RexxObject *other)
{
    CompareOperator(strictComp(other) >= 0);
}

RexxObject *NumberString::strictLessOrEqual(RexxObject *other)
{
    CompareOperator(strictComp(other) <= 0);
}


/**
 * Common method used to process the right-side arguments
 * for arithmetic operators.
 *
 * @param right  The right-hand-side of the operator.
 *
 * @return The converted numberstring value.
 */
NumberString *NumberString::operatorArgument(RexxObject *right)
{
    requiredArgument(right, ARG_ONE);
    // get the operand as a Number string.  If this does not
    // convert, we have a conversion error.
    NumberString *rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)
    {
        reportException(Error_Conversion_operator, right);
    }
    return rightNumber;
}

/**
 * Add two number strings
 *
 * @param right  The other string to add.
 *
 * @return The addition result.
 */
NumberString *NumberString::plus(RexxObject *right)
{
    // this can be either a dyadic operation or a prefix
    // operation, depending on whether we haave an argument.
    if (right != OREF_NULL)
    {
        // get the argument as a number string
        NumberString *rightNumber = operatorArgument(right);

        // addsub() does the computation
        return addSub(rightNumber, OT_PLUS, number_digits());
    }
    else
    {
        // need to format under different precision?
        if (stringObject != OREF_NULL || createdDigits != number_digits() ||
            (number_form() == Numerics::FORM_SCIENTIFIC && isEngineering()) ||
            (number_form() == Numerics::FORM_ENGINEERING && isScientific()))
        {
            // create a new number and round appropriately.
            return prepareOperatorNumber(number_digits(), number_digits(), ROUND);
        }
        // we can just return this unchanged
        else
        {
            return this;
        }
    }
}


/**
 * Subtraction between two numbers
 *
 * @param right  The subtraction argument.
 *
 * @return The subtraction result.
 */
NumberString *NumberString::minus(RexxObject *right)
{
    // if we have an argument, this is a full subtraction operation.
    if (right != OREF_NULL)
    {
        // get the argument as a number string
        NumberString *rightNumber = operatorArgument(right);

        return addSub(rightNumber, OT_MINUS, number_digits());
    }
    else
    {
        // need to copy and reformat, then negate the sign
        NumberString *result = prepareOperatorNumber(number_digits(), number_digits(), ROUND);
        result->numberSign = -(result->numberSign);
        return result;
    }
}


/**
 * Multiply operation.
 *
 * @param right  the other operand
 *
 * @return The multiplication result.
 */
NumberString *NumberString::multiply(RexxObject *right)
{
    // get the argument as a number string
    NumberString *rightNumber = operatorArgument(right);
    // and multiply the number strings
    return Multiply(rightNumber);
}


/**
 * Divide two numbers
 *
 * @param right  The operator divisor
 *
 * @return The division result.
 */
NumberString *NumberString::divide(RexxObject *right)
{
    // get the argument as a number string
    NumberString *rightNumber = operatorArgument(right);
    // go do the division
    return Division(rightNumber, OT_DIVIDE);
}


/**
 * Integer division between two numbers
 *
 * @param right  The division operand.
 *
 * @return The division result.
 */
NumberString *NumberString::integerDivide(RexxObject *right)
{
    // get the argument as a number string
    NumberString *rightNumber = operatorArgument(right);
    // do the division
    return Division(rightNumber, OT_INT_DIVIDE);
}


/**
 * Remainder division between two numbers
 *
 * @param right  The right side of the operator
 *
 * @return The remainder of the division.
 */
NumberString *NumberString::remainder(RexxObject *right)
{
    // get the argument as a number string
    NumberString *rightNumber = operatorArgument(right);
    // go do the division
    return Division(rightNumber, OT_REMAINDER);
}


/**
 * Perform modulo operation for two numbers.
 * There is no universal agreement how to calculate modulo for
 * floating point arguments or for negative divisors, so we restrict
 * the dividend to a whole number and the divisor to a positive whole number.
 *
 * @param right  The divisor object.
 *
 * @return The module result.
 */
NumberString *NumberString::modulo(RexxObject *divisorObj)
{
    // modulo requires the dividend to be an integer
    if (!this->isInteger())
    {
        reportException(Error_Incorrect_method_string_no_whole_number, "MODULO", this);
    }

    requiredArgument(divisorObj, ARG_ONE);
    NumberString *divisor = divisorObj->numberString();
    // the divisor is required to be a positive integer
    if (divisor == OREF_NULL || !divisor->isInteger() || divisor->numberSign != 1)
    {
        reportException(Error_Incorrect_method_positive, 1, divisorObj);
    }
    // calculate the remainder
    NumberString *module = Division(divisor, OT_REMAINDER);
    // if negative, correct by adding the divisor
    return module->numberSign >= 0 ? module : module->plus(divisor);
}


/**
 * Perform conditional rounding of a number value.
 *
 * @param digits The current digits value.
 * @param form   The current form.
 *
 * @return Either the same numberstring value or a new one.
 */
NumberString *NumberString::copyIfNecessary()
{
    wholenumber_t digits = number_digits();
    bool form = number_form();

    // if this number has problems with the current digits settings,
    // we need to make a copy and potentially round.  There are also issues
    // if the numberstring digits or form settings are different from the
    // current values.  We will need a string that will format the correct way.
    if (digitsCount > digits || createdDigits != digits || isScientific() != form)
    {
        NumberString *newNumber = clone();
        // inherit the current numeric settings and perform rounding, if
        // necessary
        newNumber->setupNumber(digits, form);
        return newNumber;
    }
    // we can just return this unchanged
    else
    {
        return this;
    }
}


/**
 * Perform conditional rounding of a number value.
 *
 * @param digits The current digits value.
 * @param form   The current form.
 *
 * @return Either the same numberstring value or a new one.
 */
NumberString *NumberString::copyForCurrentSettings()
{
    NumberString *newNumber = clone();
    // inherit the current numeric settings and perform rounding, if
    // necessary
    newNumber->setupNumber(number_digits(), number_form());
    return newNumber;
}


/**
 * Return the absolute value of a number
 *
 * @return The absolute value result.
 */
NumberString *NumberString::abs()
{
    // if this is a positive number already, we can potentially return the same
    // object.
    if (isPositive())
    {
        // see if we need to create a new version because the numeric
        // settings have changed
        return copyIfNecessary();
    }

    // get a copy of the number because we might need to round.
    NumberString *newNumber = copyForCurrentSettings();
    newNumber->numberSign = (short)::abs(newNumber->numberSign);
    return newNumber;
}


/**
 * Return the sign of a number
 *
 * @return The sign value, as an integer.
 */
RexxInteger *NumberString::Sign()
{
    // it is possible that a change in numeric settings might require
    // some rounding that could affect the result.  See if we
    // need to copy (likely not).
    NumberString *newNumber = copyIfNecessary();
    // return the new sign
    return new_integer(newNumber->numberSign);
}


/**
 * Logical not of a number string value
 *
 * @return The logical inversion result.
 */
RexxObject  *NumberString::notOp()
{
    // exactly zero?  We can handle this.
    if (isZero())
    {
        return TheTrueObject;
    }
    // a test for one is pretty easy also
    if (isOne())
    {
        return TheFalseObject;
    }

    // it is possible that there are some more complicated formatting
    // results that might produce a valid logical, so we'll allow the
    // string version to be the final arbiter and raise the error.
    return stringValue()->notOp();
}


/**
 * Logical not of a number string value (operator method version)
 *
 * @param right  Dummy unused argument just to have the same signature as other operators.
 *
 * @return The logical inversion result.
 */
RexxObject  *NumberString::operatorNot(RexxObject *right)
{
    // exactly zero?  We can handle this.
    if (isZero())
    {
        return TheTrueObject;
    }
    // a test for one is pretty easy also
    if (isOne())
    {
        return TheFalseObject;
    }

    // it is possible that there are some more complicated formatting
    // results that might produce a valid logical, so we'll allow the
    // string version to be the final arbiter and raise the error.
    return stringValue()->notOp();
}


/**
 * Process MAX function
 *
 * @param args     The array of other comparison arguments.
 * @param argCount The count of the arguments.
 *
 * @return The largest of the numbers.
 */
NumberString *NumberString::Max(RexxObject **args, size_t argCount)
{
   return maxMin(args, argCount, OT_MAX);
}


/**
 * Process Min function
 *
 * @param args     The array of other comparison arguments.
 * @param argCount The count of the arguments.
 *
 * @return The smallest of the numbers.
 */
NumberString *NumberString::Min(RexxObject **args, size_t argCount)
{
   return maxMin(args, argCount, OT_MIN);
}


/**
 * This method determines if the formatted numberstring is a true integer
 * string.  That is, its not of the form 1.00E3 but 1000
 *
 * @return true if this can be represented as a true whole number value,
 *         false otherwise.
 */
bool NumberString::isInteger()
{
    // easiest case...the number zero.
    if (numberSign == 0)
    {
        return true;
    }

    // zero exponents is a good case too...we would only need
    // exponential format if too long...and we've already determined
    // that situation.
    if (numberExponent == 0)
    {
        return true;
    }

    // get size of the integer part of this number
    wholenumber_t adjustedLength = numberExponent + digitsCount;
    // ok, now do the exponent check...if we need one, not an integer
    if ((adjustedLength > createdDigits) || adjustedLength <= 0)
    {
        return false;
    }

    // we don't need exponential notation, and this exponent value is positive, which
    // means there are no decimals.  This is a good integer
    if (numberExponent > 0)
    {
        return true;
    }

    // validate that all decimal positions are zero
    for (wholenumber_t numIndex = adjustedLength; numIndex < digitsCount; numIndex++)
    {
        if (numberDigits[numIndex] != 0)
        {
            return false;
        }
    }
    return true;
}


/**
 * Convert a valid numberstring to a hex string.
 *
 * @param length The output length (required if this is a
 *               negative number)
 *
 * @return The converted object.
 */
RexxString *NumberString::d2x(RexxObject *length)
{
    return d2xD2c(length, false);
}


/**
 * Convert a valid numberstring to a character string.
 *
 * @param length The output length (required for negative
 *               numbers)
 *
 * @return The converted value.
 */
RexxString *NumberString::d2c(RexxObject *length)
{
    return d2xD2c(length, true);
}


/**
 * Polymorphic method that makes numberstring a polymorphic
 * expression term for literals
 *
 * @param context The current execution context.
 * @param stack   The current expression stack.
 *
 * @return The evaluated result.
 */
RexxObject *NumberString::evaluate(RexxActivation *context, ExpressionStack *stack)
{
    // evaluate both returns the value and places it on the expression stack.
    stack->push(this);
    context->traceIntermediate(this, RexxActivation::TRACE_PREFIX_LITERAL);
    return this;
}


/**
 * Polymorphic get_value function used with expression terms
 *
 * @param context The evaluation context.
 *
 * @return Just directly returns the number string.
 */
RexxObject  *NumberString::getValue(RexxActivation *context)
{
    // just return the value
    return this;
}


/**
 * Polymorphic get_value function used with expression terms
 *
 * @param context The evaluation context.
 *
 * @return Just directly returns the number string.
 */
RexxObject  *NumberString::getValue(VariableDictionary *context)
{
    // just return the value
    return this;
}


/**
 * Polymorphic get_value function used with expression terms
 *
 * @param context The evaluation context.
 *
 * @return Just directly returns the number string.
 */
RexxObject  *NumberString::getRealValue(RexxActivation *context)
{
    // just return the value
    return this;
}


/**
 * Polymorphic get_value function used with expression terms
 *
 * @param context The evaluation context.
 *
 * @return Just directly returns the number string.
 */
RexxObject  *NumberString::getRealValue(VariableDictionary *context)
{
    // just return the value
    return this;
}


/**
 * Return the String class object for numberstring instances
 *
 * @return Always returns the String class.
 */
RexxClass   *NumberString::classObject()
{
    // we're pretending to be a string, so return the string class.
    return TheStringClass;
}


/**
 * Allocate memory for a new numberstring object.
 *
 * @param size   The base object size.
 * @param length The number of digits to allocate space for.
 *
 * @return Storage for an object.
 */
void  *NumberString::operator new(size_t size, size_t length)
{
    NumberString *newNumber = (NumberString *)new_object(size + length, T_NumberString);
    // until we have a string value, this object has no references.
    newNumber->setHasNoReferences();
    return newNumber;
}


/**
 * Create a new number string object from a character string value.
 *
 * @param number The number character data.
 * @param len    The number length.
 *
 * @return A new number string object.
 */
NumberString *NumberString::newInstance(const char *number, size_t len)
{
    // sometimes we just create an empty string with a given length
    if (number == NULL)
    {
        NumberString *newNumber = new (len) NumberString (len);
        // give this a zero value.
        newNumber->setZero();
        return newNumber;
    }

    // this is frequently called to see if a string can be converted to a
    // numeric value.  We can frequently determine this fairly quickly, so
    // we do a quick validation scan first to eliminate the obvious cases
    // so that we don't spew dead objects all over the place when doing these
    // checks.
    if (numberStringScan(number, len))
    {
        return OREF_NULL;
    }

    // this is a valid number at first blush.  We might still
    // find something wrong (underflow/overflow problems most likely)
    NumberString *newNumber = new (len) NumberString (len);
    // now see if the data actually is a number and fill in the numeric values.
    // NOTE: even though a scan has been we still may not have a thorough
    // enough check.
    if (!newNumber->parseNumber(number, len))
    {
        return OREF_NULL;
    }
    return newNumber;
}


/**
 * Create a numberstring object from a floating point number
 *
 * @param num    The source number.
 *
 * @return The number string object.
 */
NumberString *NumberString::newInstanceFromFloat(float num)
{
    return newInstanceFromDouble((double)num, number_digits());
}


/**
 * Create a NumberString from a double value
 *
 * @param number The number to convert.
 *
 * @return A numberstring object representing this number.
 */
NumberString *NumberString::newInstanceFromDouble(double number)
{
    return newInstanceFromDouble(number, number_digits());
}


/**
 * Create a numberstring from a double value using a given
 * formatting precision.
 *
 * @param number    The double number to convert
 * @param precision The precision to apply to the result.
 *
 * @return The formatted number, as a numberstring value.
 */
NumberString *NumberString::newInstanceFromDouble(double number, wholenumber_t precision)
{
    // There are some special double values involved here.  We just return some
    // special strings for those.
    if (std::isnan(number))
    {
        return (NumberString *)GlobalNames::NAN_VAL;
    }
    else if (number == +HUGE_VAL)
    {
        return (NumberString *)GlobalNames::INFINITY_PLUS;
    }
    else if (number == -HUGE_VAL)
    {
        return (NumberString *)GlobalNames::INFINITY_MINUS;
    }

    // with precision restricted to a maximum of 16, the length of a %.*g
    // string can be up to 23 characters, e. g. -1.234567890123457e-308
    char doubleStr[32]; // allow for + 2 precision, NUL, and margin

    // get double as a string value, using the provided precision (16 at most)
    snprintf(doubleStr, sizeof(doubleStr), "%.*g", std::min(16, (int)precision) + 2, number);

    // snprintf() is locale-dependent, and we cannot be sure that we run under
    // the default "C" locale, as some badly behaved native code (a known
    // example being BSF.CLS) may have called setlocale(LC_ALL, "") or similar.
    // As snprintf_l() only exists on BSD-based and Windows systems and
    // uselocale() isn't readily available on Windws, where it would require
    // setlocale() together with _configthreadlocale(_ENABLE_PER_THREAD_LOCALE),
    // we employ a hack: should the current locale not have the dot as decimal
    // radix, we replace the actual radix with a dot in the converted string.
    char radixChar = *localeconv()->decimal_point;
    if (radixChar != '.')
    {
        // find the locale radix position in the converted string
        char *radixPos = strchr(doubleStr, radixChar);
        if (radixPos != NULL)
        {
            // we found a locale radix .. convert it to a dot
            *radixPos = '.';
        }
    }

    size_t resultLen = strlen(doubleStr);
    // create a new number string with this size
    NumberString *result = new (resultLen) NumberString (resultLen, precision);
    // now format as a numberstring
    result->parseNumber(doubleStr, resultLen);
    // and perform any rounding that might be required for this precision.
    return result->prepareNumber(precision, ROUND);
}


/**
 * Create a NumberString object from a wholenumber_t value
 *
 * @param integer The value to convert.
 *
 * @return A NumberString object for this value.
 */
NumberString *NumberString::newInstanceFromWholenumber(wholenumber_t integer)
{
    // the size of the integer depends on the platform, 32-bit or 64-bit.
    // ARGUMENT_DIGITS ensures the correct value
    NumberString *newNumber = new (Numerics::ARGUMENT_DIGITS) NumberString (Numerics::ARGUMENT_DIGITS);
    newNumber->formatNumber(integer);      /* format the number               */
    return newNumber;
}


/**
 * Create a NumberString object from a size_t value
 *
 * @param integer The integer to convert.
 *
 * @return The numberstring value.
 */
NumberString *NumberString::newInstanceFromStringsize(size_t integer)
{
    // the size of the integer depends on the platform, 32-bit or 64-bit.
    // ARGUMENT_DIGITS ensures the correct value
    NumberString *newNumber = new (Numerics::ARGUMENT_DIGITS) NumberString (Numerics::ARGUMENT_DIGITS);
    newNumber->formatUnsignedNumber(integer);
    return newNumber;
}


/**
 * Create a NumberString object from a signed 64 bit number
 *
 * @param integer The integer value to convert.
 *
 * @return A numberstring value for this.
 */
NumberString *NumberString::newInstanceFromInt64(int64_t integer)
{
    // this give us space for entire binary range of the int64_t number.
    NumberString *newNumber = new (Numerics::DIGITS64) NumberString (Numerics::DIGITS64);
    newNumber->formatInt64(integer);
    return newNumber;
}


/**
 * Create a NumberString object from an unsigned 64 bit number
 *
 * @param integer The integer value to convert.
 *
 * @return A new numberstring for the value.
 */
NumberString *NumberString::newInstanceFromUint64(uint64_t integer)
{
    // this give us space for entire binary range of the uint64_t number.
    NumberString *newNumber = new (Numerics::DIGITS64) NumberString (Numerics::DIGITS64);
    newNumber->formatUnsignedInt64(integer);
    return newNumber;
}

// the numberstring operator table
PCPPM NumberString::operatorMethods[] =
{
   NULL,                               // first entry not used
   (PCPPM)&NumberString::plus,
   (PCPPM)&NumberString::minus,
   (PCPPM)&NumberString::multiply,
   (PCPPM)&NumberString::divide,
   (PCPPM)&NumberString::integerDivide,
   (PCPPM)&NumberString::remainder,
   (PCPPM)&NumberString::power,
   (PCPPM)&NumberString::concat,
   (PCPPM)&NumberString::concat,
   (PCPPM)&NumberString::concatBlank,
   (PCPPM)&NumberString::equal,
   (PCPPM)&NumberString::notEqual,
   (PCPPM)&NumberString::isGreaterThan,
   (PCPPM)&NumberString::isLessOrEqual,
   (PCPPM)&NumberString::isLessThan,
   (PCPPM)&NumberString::isGreaterOrEqual,

   (PCPPM)&NumberString::isGreaterOrEqual,
   (PCPPM)&NumberString::isLessOrEqual,
   (PCPPM)&NumberString::strictEqual,
   (PCPPM)&NumberString::strictNotEqual,
   (PCPPM)&NumberString::strictGreaterThan,
   (PCPPM)&NumberString::strictLessOrEqual,
   (PCPPM)&NumberString::strictLessThan,
   (PCPPM)&NumberString::strictGreaterOrEqual,

   (PCPPM)&NumberString::strictGreaterOrEqual,
   (PCPPM)&NumberString::strictLessOrEqual,
   (PCPPM)&NumberString::notEqual,

   (PCPPM)&NumberString::notEqual,
   (PCPPM)&NumberString::andOp,
   (PCPPM)&NumberString::orOp,
   (PCPPM)&NumberString::xorOp,
   (PCPPM)&NumberString::operatorNot,
};
