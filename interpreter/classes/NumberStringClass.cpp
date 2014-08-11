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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive NumberString Class                                               */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "RexxActivation.hpp"
#include "NumberStringMath.hpp"
#include "Numerics.hpp"
#include "StringUtil.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *NumberString::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void NumberString::createInstance()
{
    CLASS_CREATE(NumberString, "String", RexxClass);
}


/**
 * Constructor for a new number string value.
 *
 * @param len    The length we require for the value.
 */
NumberString::NumberString(size_t len)
{
    setNumericSettings(number_digits(), number_form());
    sign = 1;
    length = len;
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
    sign = 1;
    length = len;
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
        Numerics::formatWholeNumber(exponent), buffer + 1);
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
    if (isInteger())
    {
        size_t maxNumberSize = numberLength + isNegative();

        // get a new string of the exact length needed
        RexxString *stringObj = raw_string(maxNumberSize);
        NumberBuilder builder(stringObj);

        // just build as an integer
        builder.addIntegerPart(isNegative(), numberDigits, numberLength, 0);
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
    wholenumber_t adjustedSize = numberExponent + numberLength - 1;
    size_t adjustedExponent = numberExponent;

    // null out the exponent string value for now.
    char  expstring[17];

    // have we hit the trigger value either
    // A) the number of digits to the left of the decimal exceeds the digits setting or
    // B) the number of digits to the right of the decimal point exceed twice the digits setting.
    if ((adjustedSize >= (wholenumber_t)createdDigits) || ((size_t)Numerics::abs(numberExponent) > (createdDigits*2)) )
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
        if (Numerics::abs(adjustedSize) > Numerics::MAX_EXPONENT)
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
        adjustedSize = Numerics::abs(adjustedSize);
    }

    size_t maxNumberSize;

    // if the adjusted exponent is not negative, then we are
    // shifting everything to the left, so the number is longer
    // than the size.
    if (adjustedExponent >= 0)
    {
        // the size is the length of the number plus the exponent
        maxNumberSize = (size_t)adjustedExponent + numberLength;
    }
    // the exponent is negative, so we're shifting stuff to the right.
    // if the absolute value of the exponent is larger than the
    // length of the number, then we'll need to add additional zeros between
    // the decimal point and the first digit.
    else if (Numerics::abs(adjustedExponent) >= numberLength)
    {
        // we add to characters for a leading zero and decimal point
        maxNumberSize = Numerics::abs(adjustedExponent) + 2;
    }
    // not adding any digits, the decimal is in the middle, so our
    // result value is the length plus one for a decimal point
    else
    {
        maxNumberSize = numberLength + 1;
    }

    // if we're using exponential notation, add in the size of the
    // exponent string
    if (expFactor)
    {
        maxNumbersize += strlen(expstring);
    }
    // add in space for a sign
    if (isNegative())
    {
        maxNumberSize++;
    }

    RexxString *stringObj = raw_string(maxNumberSize);
    NumberBuilder bulder(stringObj);

    wholenumber_t adjustedLength = adjustedExponent + numberLength;

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
        builder.addDecimalPart(numberDigits, numberLength, -adjustedLength);

        // add the exponent, if any
        builder.addExponent(expstring);
    }
    // we might need to add zeros after the digits,
    // something like "xxxx00000"
    else if (adjustedLength >= numberLength)
    {
        // build the integer section
        builder.addIntegerPart(isNegative(), numberDigits, numberLength, adjustedLength);
        // add the exponent, if any
        builder.addExponent(expstring);
    }
    // partial decimal number
    else
    {
        wholenumber_t integers = numberLength - temp;
        // do the integer and decimal parts separately
        builder.addIntegerPart(isNegative(), numberDigits, integers);
        builder.addDecimalPart(numberDigits + integers, numberLength - digits);
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
bool NumberString::numberValue(wholenumber_t &result, size_t numDigits)
{
    // set up the default values
    bool carry = false;
    wholenumber_t numberExp = numberExponent;
    size_t length = numberLength;

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
        if (!createUnsignedValue(number, length, false, numberExp, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // too big to handle
        }

        // adjust for the sign
        result = ((wholenumber_t)intnum) * sign;
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
    if (-numberExp>= (wholenumber_t)length)
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
        if (!createUnsignedValue(number, length + numberExp, carry, 0, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // to big to handle
        }
    }
    // a straight out number, we can compute just from the digits.
    else
    {
        if (!createUnsignedValue(number, length, carry, numberExp, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // to big to handle
        }
    }

    // adjust for the sign
    result = ((wholenumber_t)intnum) * sign;
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
bool NumberString::unsignedNumberValue(size_t &result, size_t numDigits)
{
    // set up the default values

    bool carry = false;
    wholenumber_t numberExp = exp;
    size_t length = numberLength;

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
        return createUnsignedValue(number, length, false, numberExp, Numerics::maxValueForDigits(numDigits), result);
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
    if (-numberExp >= numberLength)
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
        return createUnsignedValue(number, numberLength + numberExp, carry, 0, Numerics::maxValueForDigits(numDigits), result);
    }
    else
    {                             /* straight out number. just compute.*/
        return createUnsignedValue(number, numberLength, carry, numberExp, Numerics::maxValueForDigits(numDigits), result);
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
    RexxString *string = stringValue();
    // use the C library routine to convert this to a double value since we
    // use compatible formats
    result = strtod(string->getStringData(), NULL);
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
RexxInteger *NumberString::integerValue(size_t digits)
{

    wholenumber_t integerNumber;

    // try to convert and return .nil for any failures
    if (!numberValue(integerNumber, number_digits()))
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
    if (exponent > (wholenumber_t)Numerics::ARGUMENT_DIGITS)
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
    if (exponent > 0)
    {
        // get this as a multipler value
        size_t exponentMultiplier = 1;
        while (exponent > 0)
        {
            exponentMultiplier *= 10;
            exponent--;
        }
        // get this as a multipler value
        size_t newNumber = intNumber * exponentMultiplier;

        // did this wrap?  This is a safe test, since we capped
        // the maximum exponent size we can multiply by.
        if (newNumber < intNumber)
        {
            return false;
        }
        intNumber = newNumber;
    }

    // was ths out of range for this conversion?
    if (intNumber >= maxValue)
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
    if (exponent > (wholenumber_t)Numerics::DIGITS64)
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
    if (exponent > 0)
    {
        // get this as a multipler value
        uint64_t exponentMultiplier = 1;
        while (exponent > 0)
        {
            exponentMultiplier *= 10;
            exponent--;
        }


        uint64_t newNumber = intNumber * exponentMultiplier;

        // did this wrap?  This is a safe test, since we capped
        // the maximum exponent size we can multiply by.
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
bool NumberString::checkIntegerDigits(size_t numDigits, size_t &length,
    wholenumber_t &exponent, bool &carry)
{
    // set the initial values
    carry = false;
    cxponent = numberExponent;
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
        if (*(number + numberLength) >= 5)
        {
            carry = true;
        }
    }

    // if we have a negative exponent, then we need to look at
    // the values below the decimal point
    if (exponent < 0)
    {
        // the position of the decimal is the negation of the exponent
        size_t decimalPos = (size_t)(-exponent);
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
            numberData = number;
        }
        else
        {
            // get the exponent adjusted position
            numberData = number + length + exponent;
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
bool NumberString::int64Value(int64_t *result, size_t numDigits)
{
    // set up the default values
    bool carry = false;
    wholenumber_t numberExp = numberExponent;
    size_t numberLength = digitsCount;
    uint64_t intnum;

    // if the number is exactly zero, then this is easy
    if (sign == 0)
    {
        *result = 0;
        return true;
    }

    // is this easily within limits (very common)?
    if (numberLength <= numDigits && numberExp >= 0)
    {
        // the minimum negative value requires one more than the max positive
        if (!createUnsignedInt64Value(number, numberLength, false, numberExp, ((uint64_t)INT64_MAX) + 1, intnum))
        {
            return false;                   // too big to handle
        }
        // this edge case can be a problem, so check for it specifically
        if (intnum == ((uint64_t)INT64_MAX) + 1)
        {
            // if at the limit, this must be a negative number
            if (sign != -1)
            {
                return false;
            }
            *result = INT64_MIN;
        }
        else
        {
            // adjust for the sign
            *result = ((int64_t)intnum) * sign;
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
    if (-numberExp>= (wholenumber_t)numberLength)
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
        if (!createUnsignedInt64Value(number, numberLength + numberExp, carry, 0, ((uint64_t)INT64_MAX) + 1, intnum))
        {
            return false;                   // to big to handle
        }
    }
    else
    {                             /* straight out number. just compute.*/
        if (!createUnsignedInt64Value(number, numberLength, carry, numberExp, ((uint64_t)INT64_MAX) + 1, intnum))
        {
            return false;                   // to big to handle
        }
    }

    // the edge case is a problem, so handle it directly
    if (intnum == ((uint64_t)INT64_MAX) + 1)
    {
        // if at the limit, this must be a negative number
        if (sign != -1)
        {
            return false;
        }
        *result = INT64_MAX;
    }
    else
    {
        // adjust for the sign
        *result = ((int64_t)intnum) * sign;
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
bool NumberString::unsignedInt64Value(uint64_t *result, size_t numDigits)
{
    // set up the default values

    bool carry = false;
    wholenumber_t numberExp = numberExponent;
    size_t numberLength = digitsCount;

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
        if (!createUnsignedInt64Value(number, numberLength, false, numberExp, UINT64_MAX, *result))
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
    if (-numberExp>= (wholenumber_t)numberLength)
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
        return createUnsignedInt64Value(number, numberLength + numberExp, carry, 0, UINT64_MAX, *result);
    }
    else
    {                             /* straight out number. just compute.*/
        return createUnsignedInt64Value(number, numberLength, carry, numberExp, UINT64_MAX, *result);
    }
}


/**
 * Return a truth value boolean for a number string
 *
 * @param errorcode The error to raise if this is not valid.
 *
 * @return true or false if this is a valid logical.
 */
bool  NumberString::truthValue(int errorcode)
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
    else if (!(numberSign == 1 && numberExponent == 0 && digitsCount == 1 && *(number) == 1))
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
    else if (numberSign == 1 && numberExponent == 0 && digitsCount == 1 && *(number) == 1)
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
        while (*nPtr == RexxString::ch_SPACE || *inPtr == RexxString::ch_TAB)
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
        if (inPtr >= indData)
        {
            return false;
        }
    }

    // we're at a non-digit.  Check for the start of an exponent.
    if (toupper(*inPtr) == 'E')
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
RexxObject *NumberString::truncInternal(size_t needed_digits)
{
    RexxString *result;                  /* returned result                   */
    wholenumber_t    temp;               /* temporary string value            */
    wholenumber_t    integer_digits;     /* leading integer digits            */
    size_t  size;                        /* total size of the result          */
    int     signValue;                   /* current sign indicator            */
    char   *resultPtr;                   /* result pointer                    */

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
    size_t signOverhead = isNegative();

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
            decimalDigits = Numerics::maxVal(digitsCount - integerDigits, needed_digits);
            // if we need more than are available, calculate how much trailing pad we need.
            if (decimalDigits < needed_digits)
            {
                trailingDecimalPad = needed_digits - decimalDigits;
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
            wholenumber_t decimalLength = -integerDigits;
            // we have no real integer digits.
            integerDigits = 0;

            // split this into the padding and the actual digits.
            // if we need less that this, we'll adjust these values.
            leadDecimalPad = decimalLength - digitsCount;
            decimalDigits = digitsCount;

            // will we need to add additional zeros beyond what is provided?
            if (needed_digits >= decimalLength)
            {
                // we use all of the number value, plus some potential trailing pad zeros.
                trailingDecimalPad - needed_digits - decimalLength;
            }
            // we really are truncating part of this
            else
            {
                // truncating in the leading section?  We don't add
                // any digits from the number and cap the padding.
                if (needed_digits <= leadDecimalPad)
                {
                    // we're going from something like "-0.0001234" to "-0.00".
                    // the sign needs to disappear
                    signOverhead = 0;
                    decimalDigits = 0;
                    leadDecimalPad = Numerics::maxVal(leadDecimalPad, needed_digits);
                }
                // we're using all of the padding and at least one of the digits
                else
                {
                    decimalDigits = Numerics::maxVal(decimalDigits, needed_digits - leadDecimalPad);
                }
            }
        }
    }


    // add up the whole lot to get the result size
    wholenumber_t resultSize = overhead + signOverhead + integerDigits + integerPadding + leadDecimalPadding + decimalDigits + trailingDecimalPad;
    RexxString *result = raw_string(resultSize);
    NumberBuilder builder(result);

    // build the integer part
    builder.addIntegerPart(signOverHead != 0, number, integerDigits, integerPad);
    // if we have any decimal part, add in the period and the trailing sections
    if (needed_digits != 0)
    {
        addDecimalPart(number + integerDigits, decimalDigits, leadDecimalPad, trailingDecimalPad);
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
    else if (sign > 0)
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
            size_t decimals = Numerics::minVal(length, (size_t)(-exp));
            // get the position to start the scan
            size_t lastDecimal = length - 1;
            bool foundNonZero = false;
            for (size_t i = decimals; i > 0; i--)
            {
                // if we found a non-zero, we have to do this the hard way
                if (number[lastDecimal--] != 0)
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
            wholenumber_t integer_digits = (wholenumber_t)length + exp;
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
                length = integer_digits;
                exp = 0;

                // point to the first digit, and start adding until
                // we no longer round
                char *current = number + integer_digits - 1;

                while (current >= number)
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
                *number = 1;
                exp += 1;
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
        if (exp >= 0)
        {
            return truncInternal(0);
        }
        else
        {
            // number has a decimal part, so we need to see if there are
            // any non-zero decimals

            // get the number of decimals we need to scan
            size_t decimals = Numerics::minVal((size_t)length, (size_t)(-exp));
            // get the position to start the scan
            size_t lastDecimal = length - 1;
            bool foundNonZero = false;
            for (size_t i = decimals; i > 0; i--)
            {
                // if we found a non-zero, we have to do this the hard way
                if (number[lastDecimal--] != 0)
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
            wholenumber_t integer_digits = (wholenumber_t)length + exp;
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
                length = integer_digits;
                exp = 0;

                // point to the first digit, and start adding until
                // we no longer round
                char *current = number + integer_digits - 1;

                while (current >= number)
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
                *number = 1;
                exp += 1;
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
                length = integer_digits;
                exp = 0;

                // Now we need to look at the first decimal and see if
                // we're rounding up
                char *current = number + integer_digits;
                // no rounding needed, go do the formatting
                if (*current < 5)
                {
                    return truncInternal(0);
                }

                // we need to add one to the integer part...which of course
                // might round all the way out.
                current--;

                while (current >= number)
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
                *number = 1;
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
    size_t integers;                     /* integer space requested           */
    size_t decimals;                     /* decimal space requested           */
    size_t mathexp;                      /* exponent space requested          */
    size_t exptrigger;                   /* exponential notation trigger      */
    size_t digits;                       /* current numeric digits            */
    bool   form;                         /* current numeric form              */

    size_t digits = number_digits();
    size_t form = number_form();

    wholenumber_t integers = optionalNonNegative(Integers, -1, ARG_ONE);
    wholenumber_t = optionalNonNegative(Decimals, -1, ARG_TWO);
    wholenumber_t = optionalNonNegative(MathExp, -1, ARG_THREE);
    wholenumber_t exptrigger = optionalNonNegative(ExpTrigger, digits, ARG_FOUR);
    // round to the current digits setting and format
    return prepareNumber(digits, ROUND)->formatInternal(integers, decimals, mathexp, exptrigger, this, digits, form);
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
    wholenumber_t exptrigger, NumberString *original, size_t digits, bool form)
{
    bool   defaultExpSize = false;

    // if we have an exponent, we will format this early
    // so that we know the length.  Set this up as a null string
    // value to start.
    char   stringExponent[15];
    stringExponent[0] = '\0';

    // no exponent factor yet.
    wholenumber_t expFactor = 0;

    // we need to calculate a whole bunch of size sections.  Initialize
    // them all to zero for now.
    wholenumber size = 0;
    wholenumber_t leadingSpaces = 0;
    wholenumber_t integerDigits = 0;
    wholenumber_t trailingIntegerZeros = 0;
    wholenumber_t leadingDecimalZeros = 0;
    wholenumber_t decimalDigits = 0;
    wholenumber_t leadingExpZeros = 0;
    wholenumber_t exponentSpaces = 0;
    wholenumber_t trailingDecimalZeros = 0;
    wholenumber_t exponentSize = 0;

    // are we allowed to have exponential form?  Need to figure out
    // if we need to use this.
    if (mathexp != 0)
    {
        wholenumber_t adjustedLength = numberExponent + digitsCount - 1;

        // our default trigger is the digits setting, but this can be set lower
        // than that on the call.  If the number of decimals is greater than the
        // trigger value, or the space required to format a pure decimal number or more than
        // twice the trigger value, we're in exponential form
        if (adjustedLength >= exptrigger || Numerics::abs(exp) > exptrigger * 2)
        {
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
            expfactor = adjustedLength;
            adjustedLength = Numerics::abs(adjustedLength);
            // format the exponent to a string value now.
            Numerics::formatWholeNumber(adjustedLength, stringExponent);
            /* get the number of digits needed   */
            exponentSize = strlen(exponent);
            // if the exponent size is defaulted, then reset to
            // the actual size we have.
            if (mathexp == -1)
            {
                mathexp = exponentSize;
                defaultExpSize = true;
            }
            // not enough space for this exponent value?  That is an error.
            if (exponentSize > mathexp)
            {
                reportException(Error_Incorrect_method_exponent_oversize, original, mathexp);
            }
        }
    }

    // using the default for the decimals?
    if (decimals == -1)
    {
        // a negative exponent determines how many decimal positions there are
        if (numberExponent < 0)
        {
            decimals = -numberExponent;
            // if this is a very large right shift, we may need some leading zeros.
            if (decimals > digitsCount)
            {
                leadingDecimalZeros = decimals - digitsCount;
            }
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
            if ((adjustedDecimals > decimals)
            {
                // get the amount we need to change by
                adjustedDecimals = adjustedDecimals - decimals;
                // we're going to chop the lengths a bit, so we also tweak the
                // exponent
                numberExponent = numberExponent + adjust;
                // are we going to lose all of the digits?  Then we need to
                // round.
                if (adjustedDecimals >= digitsCount)
                {
                    // if the delta is exactly equal to the number of digits in
                    // the number, then we might need to round.  We become just a
                    // single digit number.
                    if (adjust == digitsCount && number[0] >= 5)
                    {
                        number[0] = 1;
                    }
                    // truncating and becoming zero
                    else
                    {
                        number[0] = 0;
                        numberExponent = 0;
                        sign = 0;
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
                    mathRound(number);
                    // calculate new adjusted value, which means we have to redo
                    // the previous exponent calculation.
                    // needed for format(.999999,,4,2,2)
                    if (mathexp != 0 && expfactor != 0)
                    {
                        // adjust the exponent back to the orignal
                        numberExponent += expfactor;
                        expfactor = 0;
                        strcpy(stringExponent, "0");
                        exponentSize = strlen(exponent);
                    }

                    wholenumber_t adjustedExponent = numberExponent + digitsCount - 1;

                    // redo the whole trigger thing
                    if (mathexp != 0 && (adjustedExponent >= exptrigger || Numerics::abs(adjustedExponent) > exptrigger * 2))
                    {
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
                        expfactor = adjustedExponent;
                        // format exponent to a string
                        Numerics::formatWholeNumber(Numerics::abs(expfactor), stringExponent);
                        // and get the new exponent size     */
                        exponentSize = strlen(exponent);

                        // TODO:  This is not really correct...we've wiped out mathexp
                        // earlier, so this will no longer be -1.  Need to decouple this.
                        if (mathexp == -1)
                        {
                            mathexp = exponentSize;
                        }

                        // check for an overflow
                        if (exponentSize > mathexp)
                        {
                            reportException(Error_Incorrect_method_exponent_oversize, original, mathexp);
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
                decimalDicits = digitsCount - adjustedDecimals;
            }
            // in theory, everything has been adjusted to the point where
            // decimals is >= to the adjusted size
            trailingDecimalZeros = decimals - adjustedDecimals;
        }
    }

    // using the default integers value?
    if (integers == -1)
    {
        // the integer digits is determined by adding the
        // count of digits to the exponent.  If we end up losing
        // all of the integers, then our value is 1 (for the leading "0").
        integers = digitsCount + numberExponent;
        if (integers < 0)
        {
            integers = 1;
            integerDigits = 0;
            trailingIntegerZeros = 1
        }
        else
        {
            integerDigits = integers;
            // we might need to add some trailing zeros when this is expanded out
            if (integers > digitsCount)
            {
                trailingIntegerZeros = integers - digitsCount;
            }
        }
    }
    //. working with a user-requested size.
    else
    {
        wholenumber_T reqIntegers = integers;
        // the integer size value includes the sign, so reduce by one if we are negative
        if (isNegative())
        {
            integers = integers - 1;
        }

        // the integer digits is determined by adding the
        // count of digits to the exponent.  If we end up losing
        // all of the integers, then our value is 1 (for the leading "0").
        wholenumber_t neededIntegers = digitsCount + numberExponent;

        if (neededIntegers < 0)
        {
            neededIntegers = 1;
            integerDigits = 0;
            trailingIntegerZeros = 1
        }
        else
        {
            integerDigits = neededIntegers;
            // if we're need more than the digits we have, get the count of trailing
            // zeros.
            if (neededIntegers > digitsCount)
            {
                trailingDecimalZeros = neededIntegers - digitsCount;
            }

        }

        // not enough room for this number?  this is an error
        if (integers < neededIntegers)
        {
            reportException(Error_Incorrect_method_before_oversize, original, reqIntegers);
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
    size += integers;

    // if we have decimals needed, add in the decimals space and the period.
    if (decimals > 0)
    {
        size += decimals + 1;
    }

    // do we have an exponent to add?
    if (expfactor != 0)
    {
        // add two for the E and the sign,
        size += 2;
        // we might need some leading zeros on this
        leadingExpZeros = mathexp - exponentSize;
        // the mathexp size is what we want.
        size += mathexp;
    }
    // spaces needed for exp
    else if (mathexp > 0 && !defaultExpSize)
    {
        // this is all spaces
        exponentSpaces = mathexp + 2;
        size += exponentSpaces;
    }

    RexxString *result = raw_string(size);
    NumberBuilder builder(result);

    // add the leading spaces
    builder.addSpaces(leadingSpaces);
    // build the integer part
    builder.addIntegerPart(isNegative(), number, integerDigits, trailingIntegerZeros);

    // if we have a decimal portion, add that now
    if (decimals > 0)
    {
        builder.addDecimalPart(number + integerDigits, decimalDigits, leadingDecimalZeros, trailingDecimalZeros);
    }

    // have an exponent to add?
    if (expfactor != 0)
    {
        // add the exponent marker
        builder.append('E');
        builder.addExponentSign(expfactor < 0);
        // add any padding zeros needed.
        builder.addZeros(leadingExpZeros);
        // add the real exponent part
        builder.append(stringExponent, exponentSize);
    }
    // we might have some spaces to pad if an explict exponent size was requested.
    else
    {
        builder.addSpaces(exponentSpaces);
    }
    return result;
}


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
    wholenumber_t expValue = 0;
    int expSign = 0;
    bool isZeroValue = true;

    const char *inPtr = number;
    const char *endData = inPtr + length;

    numberSign = 0;

    // skip all leading blanks
    while (*inPtr == RexxString::ch_SPACE || *inPtr == RexxString::ch_TAB)
    {
        inPtr++;
    }

    // are we looking at a sign character?
    char ch = *inPtr;
    if (ch == RexxString::ch_MINUS || ch == RexxString::ch_PLUS)
    {
        inPtr++;
        // if this is a negative value, set the sign immediately
        if (ch == RexxString::ch_MINUS)
        {
            numberSign = -1;
        }
        // skip any spaces after the sign
        while (*inPtr == RexxString::ch_SPACE || *inPtr == RexxString::ch_TAB)
        {
            inPtr++;
        }
    }

    size_t maxDigits = resultDigits = length;
    char *outPtr = numberDigits;

    // if the number starts with a zero, skip any leading zeros until we
    // see something real.
    if (*inPtr == RexxString::ch_ZERO)
    {
        // skip any leading zeros
        while (*inPtr == RexxString::ch_ZERO)
        {
            inPtr++;
        }

        // we coull leading blanks
        while (*inPtr == RexxString::ch_SPACE || *inPtr == RexxString::ch_TAB)
        {
            inPtr++;
        }

        // we had at least one zero...if we've reached the end of the number
        // while skipping digits, this is exactly zero.
        if (inPtr >= endData)
        {
            setZero();
            return true;
        }
    }

    expValue = 0;

    if (*inPtr > RexxString::ch_ZERO && *inPtr <= RexxString::ch_NINE)
    {
        isZero = false;
    }

    // now scan all digit values
    while (*InPtr >= RexxString::ch_ZERO && *InPtr <= RexxString::ch_NINE)
    {
        // do we still have room for more digits in this object?
        if (maxDigits > 0)
        {
            // copy into the buffer reduced to addable form
            *outPtr++ = (char)(*inPtr++ - '0');
            maxDigits--;
        }
        // we've run out of space for more digits.
        else
        {
            // have we found our most significant digit yet and
            // and have not run out of data?  Save this for a later
            // rounding check.
            if (MSDigit == 0 && (inPtr < endData))
            {
                MSDigit = *inPtr;
            }
            inPtr++;
            expValue++;
        }
    }

    // did we reach the end of the data just on the digits scan?
    if (inPtr >= endData)
    {
        digitsCount = resultDigits - maxDigits;
        numberExponent = expValue;
        // perform a round using the most signficant digit
        roundUp(MSDigit);
        return 0;
    }

    // compute the length...so far
    digitsCount = resultDigits - maxDigits;
    numberExponent = expValue;

    // did we hit a period?
    if (*inPtr == RexxString::ch_PERIOD)
    {
        inPtr++;
        if (inPtr >= endData)
        {           /* Did we reach end of data          */
                    /* Yes,  valid digits continue.      */
                    /*is it "0.", or number Zero         */
            if (MaxDigits == resultDigits || isZero)
            {
                setZero();              /* make number just zero.            */
            }
            else
            {
                /* Round up the number if necessary  */
                roundUp(MSDigit);
            }
            return 0;                       /* All done, exit.                   */
        }
        if (MaxDigits == resultDigits)
        {  /*Any significant digits?            */
           /* No, Ship leading Zeros            */
            while (*InPtr == RexxString::ch_ZERO)
            {     /* While 1st Digit is a 0            */
                ExpValue--;                   /* decrement exponent.               */
                InPtr++;                      /* Go to next character.             */
                                              /* Have we reach end of number,num   */
                                              /*zero?                              */
                if (InPtr >= EndData)
                {
                    setZero();                  /* Make value a zero.                */
                    return 0;
                }
            }
        }
        /* in the range 1-9?                 */
        if (*InPtr > RexxString::ch_ZERO && *InPtr <= RexxString::ch_NINE)
        {
            isZero = false;                 /* found the first non-zero digit    */
        }
        /*While there are still digits       */
        while (*InPtr >= RexxString::ch_ZERO && *InPtr <= RexxString::ch_NINE)
        {
            if (MaxDigits)
            {                /*if still room for digits           */
                ExpValue--;                   /* Decrement Exponent                */
                                              /* Move char to output               */
                *OutPtr++ = (char)(*InPtr++ - '0');
                MaxDigits--;                  /* Room for one less digit.          */
            }
            else
            {
                if (!MSDigit)                 /* not gotten a most sig digit yet?  */
                {
                    MSDigit = *InPtr;           /* record this one                   */
                }
                InPtr++;                      /* No more room, go to next digit    */
            }
        }
        if (InPtr >= EndData)
        {           /*Are we at end of data?             */
                    /* Compute length of number.         */
            length = (resultDigits - MaxDigits);
            exp = ExpValue;           /* get exponent.                     */
                                            /* Round up the number if necessary  */
            roundUp(MSDigit);
            return 0;                       /* All done, return                  */
        }
    }                                   /* End is it a Decimal point.        */

    /* At this point we are don copying  */
    /* digits.  We are just looking for  */
    /* exponent value if any and any     */
    /*trailing blanks                    */

    /* Get  final length of number.      */
    length = resultDigits - MaxDigits;
    if (!length)
    {                /* No digits, number is Zero.        */
                     /* Have we reached the end of the    */
                     /*string                             */
        if (InPtr >= EndData)
        {
            /* Yes, all done.                    */
            setZero();                 /* make number just zero.            */
            return 0;                        /* All done, exit.                   */
        }
    }
    exp = ExpValue;               /* get current exponent value.       */

    if (toupper(*InPtr) == 'E')
    {       /* See if this char is an exponent?  */
        ExpSign = 1;                      /* Assume sign of exponent to '+'    */
        InPtr++;                          /* step over the 'E'                 */
        if (*InPtr == RexxString::ch_MINUS)
        {         /* If this a minus sign?             */
            ExpSign = -1;                   /*  Yes, make sign of exp '-'        */
            InPtr++;                        /*  go on to next char.              */
        }
        else if (*InPtr == RexxString::ch_PLUS)       /* If this a plus  sign?             */
        {
            InPtr++;                        /* Yes, skip it and go to next char. */
        }
        ExpValue = 0;                     /* Start of exponent clear work area.*/
        MaxDigits = 0;                    /* claer digit counter.              */

                                          /* Do while we have a valid digit    */
        while (*InPtr >= RexxString::ch_ZERO && *InPtr <= RexxString::ch_NINE)
        {
            /* Add this digit to Exponent value. */
            ExpValue = ExpValue * 10 + ((*InPtr++) - '0');
            if (ExpValue > Numerics::MAX_EXPONENT)   /* Exponent can only be 9 digits long*/
            {
                return 1;                     /* if more than that, indicate error.*/
            }
            if (ExpValue)                   /* Any significance in the Exponent? */
            {
                MaxDigits++;                  /*  Yes, bump up the digits counter. */
            }
        }
        exp += (ExpValue * ExpSign);/* Compute real exponent.            */
                                          /* Is it bigger than allowed max     */
        if (Numerics::abs(exp) > Numerics::MAX_EXPONENT)
        {
            return 1;                       /* yes, indicate error.              */
        }
    }

    if (sign == 0 || isZero)
    {    /* Was this really a zero number?    */
        setZero();                  /* make number just zero.            */
    }

    roundUp(MSDigit);             /* Round up the number if necessary  */
                                        /*is number just flat out too big?   */
    if ((exp + (wholenumber_t)length - 1) > Numerics::MAX_EXPONENT)
    {
        return 1;                         /* also bad                          */
    }
    return 0;                           /* All done !!                       */
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
    if (integer == 0)
    {

        setZero();
    }
    else
    {
        // this is a positive value
        numberSign = 1;
        // format the string value into our buffer
        Numerics::formatStringSize(integer, number);

        // normalize all of the digits
        char *current = number;
        while (*current != '\0')
        {
            *current -= '0';
            current++;
        }
        digitsCount = current - number;
    }
}


void NumberString::formatInt64(int64_t integer)
/******************************************************************************/
/* Function : Format the integer num into a numberstring.                     */
/******************************************************************************/
{
    if (integer == 0)
    {                  /* is integer 0?                     */
                       /* indicate this.                    */
        setZero();
    }
    else
    {                               /* number is non-zero                */
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
            sign = -1;      // negative number

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
            sign = 1;              // positive number
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
        length = sizeof(buffer) - index;
        memcpy(number, &buffer[index], length);
    }
}


void NumberString::formatUnsignedInt64(uint64_t integer)
/******************************************************************************/
/* Function : Format the integer num into a numberstring.                     */
/******************************************************************************/
{
    if (integer == 0)
    {                  /* is integer 0?                     */
                       /* indicate this.                    */
        setZero();
    }
    else
    {                               /* number is non-zero                */
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
        length = sizeof(buffer) - index;
        memcpy(number, &buffer[index], length);
    }
}


RexxObject *NumberString::unknown(RexxString *msgname, ArrayClass *arguments)
/******************************************************************************/
/* Function:  Forward all unknown messages to the numberstring's string       */
/*            representation                                                  */
/******************************************************************************/
{
    return stringValue()->sendMessage(msgname, arguments);
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


RexxString *NumberString::concatBlank(RexxObject *other)
/******************************************************************************/
/* Function:  Blank concatenation operator                                    */
/******************************************************************************/
{
    return stringValue()->concatBlank(other);
}

RexxString *NumberString::concat(RexxObject *other)
/******************************************************************************/
/* Function:  Normal concatentation operator                                  */
/******************************************************************************/
{
    return stringValue()->concatRexx(other);
}
                                       /* numberstring operator forwarders  */
                                       /* to process string operators       */

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

bool NumberString::isEqual(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Primitive strict equal\not equal method.  This determines       */
/*            only strict equality, not greater or less than values.          */
/******************************************************************************/
{
    if (isSubClassOrEnhanced())      /* not a primitive?                  */
    {
                                           /* do the full lookup compare        */
        return sendMessage(GlobalNames::STRICT_EQUAL, other)->truthValue(Error_Logical_value_method);
    }
                                       /* go do a string compare            */
    return stringValue()->isEqual(other);
}

wholenumber_t NumberString::strictComp(RexxObject *other)
/******************************************************************************/
/* Function:  Compare the two values.                                         */
/*                                                                            */
/*  Returned:  return <0 if other is greater than this                        */
/*             return  0 if this equals other                                 */
/*             return >0 if this is greater than other                        */
/******************************************************************************/
{
                                       /* the strict compare is done against*/
                                       /* strings only, so convert to string*/
                                       /* and let string do this compare.   */
   return stringValue()->strictComp(other);
}

wholenumber_t NumberString::comp(
    RexxObject *right)                 /* right hand side of compare      */
/******************************************************************************/
/* Function:  Do a value comparison of two number strings for the non-strict  */
/*            comparisons.  This returns for the compares:                    */
/*                                                                            */
/*             a value < 0 when this is smaller than other                    */
/*             a value   0 when this is equal to other                        */
/*             a value > 0 when this is larger than other                     */
/******************************************************************************/
{
    NumberString *rightNumber;       /* converted right hand number     */
    wholenumber_t      aLexp;            /* adjusted left exponent            */
    wholenumber_t     aRexp;             /* adjusted right exponent           */
    size_t    aLlen;                     /* adjusted left length              */
    size_t    aRlen;                     /* adjusted right length             */
    wholenumber_t      MinExp;           /* minimum exponent                  */
    size_t    NumberDigits;              /* current digits setting            */
    char     *scan;                      /* scan pointer                      */
    wholenumber_t rc;                    /* compare result                    */

                                         /* the compare is acually done by    */
                                         /* subtracting the two numbers, the  */
                                         /* sign of the result obj will be our*/
                                         /* return value.                     */
    requiredArgument(right, ARG_ONE);            /* make sure we have a real value    */
                                         /* get a numberstring object from    */
                                         /*right                              */
    rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)        /* didn't convert?                   */
    {
        /* numbers couldn't be compared      */
        /* numerically, do a string compare. */
        return stringValue()->comp(right);
    }

    // unfortunately, we need to perform any lostdigits checks before
    // handling any of the short cuts
    NumberDigits = number_digits();

    if (length > NumberDigits)
    {
        reportCondition(GlobalNames::LOSTDIGITS, (RexxString *)this);
    }
    if (rightNumber->length > NumberDigits)
    {
        reportCondition(GlobalNames::LOSTDIGITS, (RexxString *)rightNumber);
    }

    if (sign != rightNumber->sign) /* are numbers the same sign?        */
    {
        /* no, this is easy                  */
        return(sign < rightNumber->sign) ? -1 : 1;
    }
    if (rightNumber->sign == 0)          /* right one is zero?                */
    {
        return sign;                 /* use this sign                     */
    }
    if (sign == 0)                 /* am I zero?                        */
    {
        return rightNumber->sign;          /* return the right sign             */
    }
                                           /* set smaller exponent              */
    MinExp = (rightNumber->exp < exp)? rightNumber->exp : exp;
    aLexp = exp - MinExp;          /* get adjusted left size            */
    aRexp = rightNumber->exp - MinExp;   /* get adjusted right size           */
    aLlen = aLexp + length;        /* get adjusted left size            */
    aRlen = aRexp + rightNumber->length; /* get adjusted right size           */
    NumberDigits = number_fuzzydigits(); /* get precision for comparisons.    */
                                         /* can we do a fast exit?            */
    if (aLlen <= NumberDigits && aRlen <= NumberDigits)
    {
        /* longer number is the winner       */
        if (aLlen > aRlen)                 /* left longer?                      */
        {
            return sign;               /* use left sign                     */
        }
        else if (aRlen > aLlen)            /* right longer?                     */
        {
            return -sign;              /* use inverse of the sign           */
        }
        else
        {
            /* actual lengths the same?          */
            if (length == rightNumber->length)
            {
                /* return the comparison result      */
                /* adjusted by the sign value        */
                return memcmp(number, rightNumber->number, length) * sign;
            }
            /* right one shorter?                */
            else if (length > rightNumber->length)
            {
                /* compare for shorter length        */
                rc = memcmp(number, rightNumber->number, rightNumber->length) * sign;
                if (rc == 0)
                {                 /* equal for that length?            */
                                  /* point to the remainder part       */
                    scan = number + rightNumber->length;
                    /* get the remainder length          */
                    aRlen = length - rightNumber->length;
                    while (aRlen--)
                    {            /* scan the remainder                */
                        if (*scan++ != 0)          /* found a non-zero one?             */
                        {
                            return sign;       /* left side is greater              */
                        }
                    }
                    return 0;                    /* these are equal                   */
                }
                return rc;                     /* return compare result             */
            }
            else
            {                           /* left one is shorter               */
                                        /* compare for shorter length        */
                rc = memcmp(number, rightNumber->number, length) * sign;
                if (rc == 0)
                {                 /* equal for that length?            */
                                  /* point to the remainder part       */
                    scan = rightNumber->number + length;
                    /* get the remainder length          */
                    aRlen = rightNumber->length - length;
                    while (aRlen--)
                    {            /* scan the remainder                */
                        if (*scan++ != 0)          /* found a non-zero one?             */
                        {
                            return -sign;      /* right side is greater             */
                        }
                    }
                    return 0;                    /* these are equal                   */
                }
                return rc;                     /* return compare result             */
            }
        }
    }
    else
    {                               /* need to subtract these            */
                                    /* call addsub to do computation     */
        rightNumber = addSub(rightNumber, OT_MINUS, number_fuzzydigits());
        return rightNumber->sign;          /* compare result is subtract sign   */
    }
}

RexxObject *NumberString::equal(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict "=" operator                                         */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other) == 0);
}

RexxObject *NumberString::notEqual(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict "\=" operator                                        */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheTrueObject;
    }
    return booleanObject(comp(other) != 0);
}

RexxObject *NumberString::isGreaterThan(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict ">" operator                                         */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other) > 0);
}

RexxObject *NumberString::isLessThan(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict "<" operator                                         */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other) < 0);
}

RexxObject *NumberString::isGreaterOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict ">=" operator                                        */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other) >= 0);
}

RexxObject *NumberString::isLessOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict "<=" operator                                        */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other) <= 0);
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

RexxObject *NumberString::strictEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Perform the primitive level "==" compare, including the hash    */
/*            value processing.                                               */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictComp(other) == 0);
}

RexxObject *NumberString::strictNotEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Strict inequality operation                                     */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheTrueObject;
    }
    return booleanObject(strictComp(other) != 0);
}


RexxObject *NumberString::strictGreaterThan(RexxObject *other)
/******************************************************************************/
/* Function:  strict ">>" operator                                            */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictComp(other) > 0);
}

RexxObject *NumberString::strictLessThan(RexxObject *other)
/******************************************************************************/
/* Function:  strict "<<" operator                                            */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictComp(other) < 0);
}

RexxObject *NumberString::strictGreaterOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  strict ">>=" operator                                           */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictComp(other) >= 0);
}

RexxObject *NumberString::strictLessOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  strict "<<=" operator                                           */
/******************************************************************************/
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictComp(other) <= 0);
}

NumberString *NumberString::plus(RexxObject *right)
/********************************************************************/
/* Function:  Add two number strings                                */
/********************************************************************/
{
    if (right != OREF_NULL)
    {            /* Is this a dyadic operation?       */
                 /* get a numberstring object from    */
                 /*right                              */
        NumberString *rightNumber = right->numberString();
        if (rightNumber == OREF_NULL)      /* is the operand numeric?           */
        {
            /* nope, this is an error            */
            reportException(Error_Conversion_operator, right);
        }
        /* call addsub to do computation     */
        return addSub(rightNumber, OT_PLUS, number_digits());
    }
    else
    {
        /* need to format under different    */
        /* precision?                        */
        if (stringObject != OREF_NULL || numDigits != number_digits() ||
            (number_form() == Numerics::FORM_SCIENTIFIC && isEngineering()) ||
            (number_form() == Numerics::FORM_ENGINEERING && isScientific()))
        {
            /* need to copy and reformat         */
            return prepareOperatorNumber(number_digits(), number_digits(), ROUND);
        }
        else
        {
            return this;                 /* just return the same value        */
        }
    }
}

NumberString *NumberString::minus(RexxObject *right)
/********************************************************************/
/* Function:  Subtraction between two numbers                       */
/********************************************************************/
{
    if (right != OREF_NULL)
    {            /* Is this a dyadic operation?       */
                 /* get a numberstring object from    */
                 /*right                              */
        NumberString *rightNumber = right->numberString();
        if (rightNumber == OREF_NULL)      /* is the operand numeric?           */
        {
            /* nope, this is an error            */
            reportException(Error_Conversion_operator, right);
        }
        /* call addsub to do computation     */
        return addSub(rightNumber, OT_MINUS, number_digits());
    }
    else
    {
        /* need to copy and reformat         */
        NumberString *result = prepareOperatorNumber(number_digits(), number_digits(), ROUND);
        /* invert the sign of our copy.      */
        result->sign = -(result->sign);
        return result;                       /* return addition result            */
    }
}

NumberString *NumberString::multiply(RexxObject *right)
/********************************************************************/
/* Function:  Multiply two numbers                                  */
/********************************************************************/
{
    requiredArgument(right, ARG_ONE);            /* must have an argument             */
                                         /* get a numberstring object from    */
                                         /*right                              */
    NumberString *rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
    {
        /* nope, this is an error            */
        reportException(Error_Conversion_operator, right);
    }
    return Multiply(rightNumber);  /* go do the multiply                */
}

NumberString *NumberString::divide(RexxObject *right)
/********************************************************************/
/* Function:  Divide two numbers                                    */
/********************************************************************/
{
    requiredArgument(right, ARG_ONE);            /* must have an argument             */

                                         /* get a numberstring object from    */
                                         /*right                              */
    NumberString *rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
    {
        /* nope, this is an error            */
        reportException(Error_Conversion_operator, right);
    }
    /* go do the division                */
    return Division(rightNumber, OT_DIVIDE);
}

NumberString *NumberString::integerDivide(RexxObject *right)
/********************************************************************/
/* Function:  Integer division between two numbers                  */
/********************************************************************/
{
    requiredArgument(right, ARG_ONE);            /* must have an argument             */
                                         /* get a numberstring object from    */
                                         /*right                              */
    NumberString *rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
    {
        /* nope, this is an error            */
        reportException(Error_Conversion_operator, right);
    }
    /* go do the division                */
    return Division(rightNumber, OT_INT_DIVIDE);
}

NumberString *NumberString::remainder(RexxObject *right)
/********************************************************************/
/* Function:  Remainder division between two numbers                */
/********************************************************************/
{
    requiredArgument(right, ARG_ONE);            /* must have an argument             */

                                         /* get a numberstring object from    */
                                         /*right                              */
    NumberString *rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
    {
        /* nope, this is an error            */
        reportException(Error_Conversion_operator, right);
    }
    /* go do the division                */
    return Division(rightNumber, OT_REMAINDER);
}

NumberString *NumberString::abs()
/********************************************************************/
/* Function:  Return the absolute value of a number                 */
/********************************************************************/
{
    NumberString *NewNumber = clone();            /* copy the number                   */
    /* inherit the current numeric settings and perform rounding, if */
    /* necessary */
    NewNumber->setupNumber();
    /* switch the sign                   */
    NewNumber->sign = (short)::abs(NewNumber->sign);
    return NewNumber;                     /* and return                        */
}

RexxInteger *NumberString::Sign()
/********************************************************************/
/* Function:  Return the sign of a number                           */
/********************************************************************/
{
    NumberString *NewNumber = clone();            /* copy the number                   */
    /* inherit the current numeric settings and perform rounding, if */
    /* necessary */
    NewNumber->setupNumber();
    return new_integer(NewNumber->sign);  /* just return the sign value        */
}

RexxObject  *NumberString::notOp()
/********************************************************************/
/* Function:  Logical not of a number string value                  */
/********************************************************************/
{
   return stringValue()->notOp();
}

RexxObject  *NumberString::operatorNot(RexxObject *right)
/********************************************************************/
/* Function:  Polymorphic NOT operator method                       */
/********************************************************************/
{
   return stringValue()->notOp();
}

NumberString *NumberString::Max(
    RexxObject **args,                 /* array of comparison values        */
    size_t argCount)                   /* count of arguments                */
/********************************************************************/
/* Function:  Process MAX function                                  */
/********************************************************************/
{
   return maxMin(args, argCount, OT_MAX);
}

NumberString *NumberString::Min(
    RexxObject **args,                 /* array of comparison values        */
    size_t argCount)                   /* count of arguments                */
/********************************************************************/
/* Function:  Process the MIN function                              */
/********************************************************************/
{
   return maxMin(args, argCount, OT_MIN);
}


/**
 * This method determines if the formatted numberstring is s true integer
 * string.  That is, its not of the form 1.00E3 but 1000
 *
 * @return true if this can be represented as a true whole number value,
 *         false otherwise.
 */
bool NumberString::isInteger()
{
    // easiest case...the number zero.
    if (sign == 0)
    {
        return true;
    }

    // get working values of the exponent and length
    wholenumber_t expValue = exp;
    size_t lenValue = length;

    // zero exponents is a good case too...we would only need
    // exponential format if too long...and we've already determined
    // that situation.
    if (expValue == 0)
    {
        return true;
    }

    wholenumber_t expFactor = 0;
    // get size of the integer part of this number
    wholenumber_t temp = expValue + (wholenumber_t)lenValue - 1;
    // ok, now do the exponent check...if we need one, not an integer
    if ((temp >= (wholenumber_t)numDigits) || ((size_t)Numerics::abs(expValue) > (numDigits * 2)) )
    {
        return false;
    }

    // we don't need exponential notation, and this exponent value is positive, which
    // means there are no decimals.  This is a good integer
    if (expValue > 0)
    {
        return true;
    }

    // get the adjusted length (expValue is negative, so this will
    // be less than length of the string).
    wholenumber_t integers = expValue + (wholenumber_t)lenValue;
    wholenumber_t decimals = lenValue - integers;
    // we can have a number of leading zeros for a decimal number,
    // so it is possible all of our digits are decimals.
    if (integers < 0)
    {
        integers = 0;
        decimals = lenValue;
    }

    // validate that all decimal positions are zero
    for (size_t numIndex = (size_t)integers; numIndex < lenValue; numIndex++)
    {
        if (number[numIndex] != 0)
        {
            return false;
        }
    }
    return true;
}


/*********************************************************************/
/*   Function:          Round up a number as a result of the chopping*/
/*                        digits off of the number during init.      */
/*********************************************************************/
void NumberString::roundUp(int MSDigit)
{
    int  carry;
    char *InPtr;
    char ch;
    /* Did we chop off digits and is it  */
    /* greater/equal to 5, rounding?     */
    if (MSDigit >= RexxString::ch_FIVE && MSDigit <= RexxString::ch_NINE)
    {
        /* yes, we have to round up digits   */

        carry = 1;                         /* indicate we have a carry.         */
                                           /* point to last digit.              */
        InPtr = number + length - 1;

        /* all digits and still have a carry */
        while ((InPtr >= number) && carry)
        {
            if (*InPtr == 9)                  /* Is this digit a 9?                */
            {
                ch = 0;                          /* make digit 0, still have carry    */
            }
            else
            {
                ch = *InPtr + 1;                 /* Not nine, just add one to digit.  */
                carry = 0;                       /* No more carry.                    */
            }
            *InPtr-- = ch;                    /* replace digit with new value.     */
        }                                  /* All done rounding.                */

        if (carry)
        {                       /* Do we still have a carry?         */
                                /* yes, carry rippled all the way    */
            *number = 1;                /*  set 1st digit to a 1.            */
            exp += 1;                   /* increment exponent by one.        */
        }
    }
}

RexxString *NumberString::d2x(
     RexxObject *_length)               /* result length                     */
/******************************************************************************/
/* Function:  Convert a valid numberstring to a hex string.                   */
/******************************************************************************/
{
                                       /* forward to the formatting routine */
    return d2xD2c(_length, false);
}

RexxString *NumberString::d2c(
     RexxObject *_length)               /* result length                     */
/******************************************************************************/
/* Function:  Convert a valid numberstring to a character string.             */
/******************************************************************************/
{
                                       /* forward to the formatting routine */
    return d2xD2c(_length, true);
}


RexxObject *NumberString::evaluate(
     RexxActivation *context,          /* current activation context        */
     ExpressionStack *stack )      /* evaluation stack                  */
/******************************************************************************/
/* Function:  Polymorphic method that makes numberstring a polymorphic        */
/*            expression term for literals                                    */
/******************************************************************************/
{
    stack->push(this);                   /* place on the evaluation stack     */
                                         /* trace if necessary                */
    context->traceIntermediate(this, RexxActivation::TRACE_PREFIX_LITERAL);
    return this;                         /* also return the result            */
}

RexxString *NumberString::d2xD2c(
     RexxObject *_length,              /* result length                     */
     bool  type )                      /* D2C or D2X flag                   */
/******************************************************************************/
/* Function:  Convert a valid numberstring to a hex or character string.      */
/******************************************************************************/

{
    char       PadChar;                  /* needed padding character          */
    size_t ResultSize;             /* size of result string             */
    size_t     HexLength;                /* length of hex characters          */
    size_t     BufferLength;             /* length of the buffer              */
    char     * Scan;                     /* scan pointer                      */
    char     * HighDigit;                /* highest digit location            */
    char     * Accumulator;              /* accumulator pointer               */
    char     * TempPtr;                  /* temporary pointer value           */
    size_t     PadSize;                  /* needed padding                    */
    size_t     CurrentDigits;            /* current digits setting            */
    size_t     TargetLength;             /* length of current number          */
    BufferClass *Target;                  /* formatted number                  */
    RexxString *Retval;                  /* returned result                   */


                                         /* get the target length             */
    ResultSize = optionalLengthArgument(_length, SIZE_MAX, ARG_ONE);
    CurrentDigits = number_digits();     /* get the current digits setting    */
    TargetLength = length;         /* copy the length                   */
                                         /* too big to process?               */
    if (exp + length > CurrentDigits)
    {
        if (type == true)                  /* d2c form?                         */
        {
            /* use that message                  */
            reportException(Error_Incorrect_method_d2c, this);
        }
        else                               /* use d2x form                      */
        {
            reportException(Error_Incorrect_method_d2x, this);
        }
    }
    else if (exp < 0)
    {            /* may have trailing zeros           */
                 /* point to the decimal part         */
        TempPtr = number + length + exp;
        HexLength = -exp;            /* get the length to check           */
                                           /* point to the rounding digit       */
        HighDigit = number + CurrentDigits;
        /* while more decimals               */
        while (HexLength -- && TempPtr <= HighDigit)
        {
            if (*TempPtr != 0)
            {             /* non-zero decimal?                 */
                          /* this may be non-significant       */
                if (TargetLength > CurrentDigits)
                {
                    /* this the "rounding" digit?        */
                    if (TempPtr == HighDigit && *TempPtr < 5)
                    {
                        break;                     /* insignificant digit found         */
                    }
                }
                if (type == true)              /* d2c form?                         */
                {
                    /* use that message                  */
                    reportException(Error_Incorrect_method_d2c, this);
                }
                else                           /* use d2x form                      */
                {
                    reportException(Error_Incorrect_method_d2x, this);
                }
            }
            TempPtr++;                       /* step the pointer                  */
        }
        /* adjust the length                 */
        TargetLength = length + exp;
    }
    /* negative without a size           */
    if (sign < 0 && ResultSize == SIZE_MAX)
    {
        /* this is an error                  */
        reportException(Error_Incorrect_method_d2xd2c);
    }
    if (ResultSize == SIZE_MAX)          /* using default size?               */
    {
        /* allocate buffer based on digits   */
        BufferLength = CurrentDigits + OVERFLOWSPACE;
    }
    else if (type == true)
    {             /* X2C function?                     */
        if (ResultSize * 2 < CurrentDigits)/* smaller than digits setting?      */
        {
            /* allocate buffer based on digits   */
            BufferLength = CurrentDigits + OVERFLOWSPACE;
        }
        else                               /* allocate a large buffer           */
        {
            BufferLength = (ResultSize * 2) + OVERFLOWSPACE;
        }
    }
    else
    {                               /* D2X function                      */
        if (ResultSize < CurrentDigits)    /* smaller than digits setting?      */
        {
            /* allocate buffer based on digits   */
            BufferLength = CurrentDigits + OVERFLOWSPACE;
        }
        else                               /* allocate a large buffer           */
        {
            BufferLength = ResultSize + OVERFLOWSPACE;
        }
    }
    Target = new_buffer(BufferLength);   /* set up format buffer              */
    Scan = number;                 /* point to first digit              */
                                         /* set accumulator pointer           */
    Accumulator = Target->getData() + BufferLength - 2;
    HighDigit = Accumulator - 1;         /* set initial high position         */
                                         /* clear the accumulator             */
    memset(Target->getData(), '\0', BufferLength);
    while (TargetLength--)
    {             /* while more digits                 */
                  /* add next digit                    */
        HighDigit = addToBaseSixteen(*Scan++, Accumulator, HighDigit);
        if (TargetLength != 0)             /* not last digit?                   */
        {
            /* do another multiply               */
            HighDigit = multiplyBaseSixteen(Accumulator, HighDigit);
        }
    }
    if (exp > 0)
    {                 /* have extra digits to worry about? */
                      /* do another multiply               */
        HighDigit = multiplyBaseSixteen(Accumulator, HighDigit);
        TargetLength = exp;          /* copy the exponent                 */
        while (TargetLength--)
        {           /* while more digits                 */
                    /* add next zero digit               */
            HighDigit = addToBaseSixteen('\0', Accumulator, HighDigit);
            if (TargetLength != 0)           /* not last digit?                   */
            {
                /* do the multiply                   */
                HighDigit = multiplyBaseSixteen(Accumulator, HighDigit);
            }
        }
    }
    HexLength = Accumulator - HighDigit; /* get accumulator length            */
    if (sign < 0)
    {                /* have a negative number?           */
                     /* take twos complement              */
        PadChar = 'F';                     /* pad negatives with foxes          */
        Scan = Accumulator;                /* point to last digit               */
        while (!*Scan)                     /* handle any borrows                */
        {
            *Scan-- = 0xf;                   /* make digit a 15                   */
        }
        *Scan = *Scan - 1;                 /* subtract the 1                    */
        Scan = Accumulator;                /* start at first digit again        */
        while (Scan > HighDigit)
        {         /* invert all the bits               */
                  /* one digit at a time               */
            *Scan = (char)(*Scan ^ (unsigned)0x0f);
            Scan--;                          /* step to next digit                */
        }
    }
    else
    {
        PadChar = '0';                     /* pad positives with zero           */
    }
                                           /* now make number printable         */
    Scan = Accumulator;                  /* start at first digit again        */
    while (Scan > HighDigit)
    {           /* convert all the nibbles           */
        *Scan = RexxString::intToHexDigit(*Scan);      /* one digit at a time               */
        Scan--;                            /* step to next digit                */
    }
    Scan = HighDigit + 1;                /* point to first digit              */

    if (type == false)
    {                 /* d2x function ?                    */
        if (ResultSize == SIZE_MAX)        /* using default length?             */
        {
            ResultSize = HexLength;          /* use actual data length            */
        }
    }
    else
    {                               /* d2c function                      */
        if (ResultSize == SIZE_MAX)        /* using default length?             */
        {
            ResultSize = HexLength;          /* use actual data length            */
        }
        else
        {
            ResultSize += ResultSize;        /* double the size                   */
        }
    }
    if (ResultSize < HexLength)
    {        /* need to truncate?                 */
        PadSize = 0;                       /* no Padding                        */
        Scan += HexLength - ResultSize;    /* step the pointer                  */
        HexLength = ResultSize;            /* adjust number of digits           */
    }
    else                                 /* possible padding                  */
    {
        PadSize = ResultSize - HexLength;  /* calculate needed padding          */
    }
    if (PadSize)
    {                       /* padding needed?                   */
        Scan -= PadSize;                   /* step back the pointer             */
        memset(Scan, PadChar, PadSize);    /* pad in front                      */
    }
    if (type == true)                    /* need to pack?                     */
    {
        Retval = StringUtil::packHex(Scan, ResultSize);/* yes, pack to character            */
    }
    else
    {
        /* allocate result string            */
        Retval = new_string(Scan, ResultSize);
    }
    return Retval;                       /* return proper result              */
}


RexxObject  *NumberString::getValue(
    RexxActivation *context)           /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return (RexxObject *)this;           /* just return this value            */
}


RexxObject  *NumberString::getValue(
    VariableDictionary *context)   /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return (RexxObject *)this;           /* just return this value            */
}


RexxObject  *NumberString::getRealValue(
    RexxActivation *context)           /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return (RexxObject *)this;           /* just return this value            */
}


RexxObject  *NumberString::getRealValue(
    VariableDictionary *context)   /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return (RexxObject *)this;           /* just return this value            */
}

RexxClass   *NumberString::classObject()
/******************************************************************************/
/* Function:  Return the String class object for numberstring instances       */
/******************************************************************************/
{
                                       /* just return class from behaviour  */
  return TheStringClass;
}

void  *NumberString::operator new(size_t size, size_t length)
/******************************************************************************/
/* Function:  Create a new NumberString object                                */
/******************************************************************************/
{
    NumberString *newNumber = (NumberString *)new_object(size + length, T_NumberString);
    /* initialize the new object         */
    newNumber->setHasNoReferences();     /* Let GC know no to bother with LIVE*/
    return newNumber;                    /* return the new numberstring       */
}

NumberString *NumberString::newInstance(const char *number, size_t len)
/******************************************************************************/
/* Function:  Create a new number string object                               */
/******************************************************************************/
{
    NumberString *newNumber;

    if (number == NULL)
    {                /* asking for a dummy string?        */
                     /* allocate a new string             */
        newNumber = new (len) NumberString (len);
        /* make it a zero value              */
        newNumber->setZero();
        return newNumber;                  /* return this now                   */
    }
    /* scan the string 1st to see if its */
    /*valid                              */
    if (numberStringScan(number, len))
    {
        newNumber = OREF_NULL;             /* not a valid number, get out now.  */
    }
    else
    {
        /* looks to be valid.  get a new     */
        /* format it                         */
        newNumber = new (len) NumberString (len);
        /* now see if the data actually is   */
        /*  a number and fill in actual data */
        /* NOTE: even though a scan has been */
        /*   we still may not have a thorough*/
        /*   enough check.                   */
        if (newNumber->parseNumber(number, len))
        {
            /* number didn't convert,            */
            newNumber = OREF_NULL;
        }
    }
    return newNumber;
}

NumberString *NumberString::newInstanceFromFloat(float num)
/******************************************************************************/
/* Function:  Create a numberstring object from a floating point number       */
/******************************************************************************/
{
    return newInstanceFromDouble((double)num, number_digits());
}

NumberString *NumberString::newInstanceFromDouble(double number)
/******************************************************************************/
/* Function:  Create a NumberString from a double value                       */
/******************************************************************************/
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
NumberString *NumberString::newInstanceFromDouble(double number, size_t precision)
{
    // make a nan value a string value
    if (isnan(number))
    {
        return (NumberString *)new_string("nan");
    }
    else if (number == +HUGE_VAL)
    {
        return (NumberString *)new_string("+infinity");
    }
    else if (number == -HUGE_VAL)
    {
        return (NumberString *)new_string("-infinity");
    }

    NumberString *result;
    size_t resultLen;
    /* Max length of double str is       */
    /*  22, make 30 just to be safe      */
    char doubleStr[30];
    /* get double as a string value.     */
    /* Use digits as precision.          */
    sprintf(doubleStr, "%.*g", (int)(precision + 2), number);
    resultLen = strlen(doubleStr);       /* Compute length of floatString     */
                                         /* Create new NumberString           */
    result = new (resultLen) NumberString (resultLen, precision);
    /* now format as a numberstring      */
    result->format(doubleStr, resultLen);
    return result->prepareNumber(precision, ROUND);
}

NumberString *NumberString::newInstanceFromWholenumber(wholenumber_t integer)
/******************************************************************************/
/* Function:  Create a NumberString object from a wholenumber_t value         */
/******************************************************************************/
{
    // the size of the integer depends on the platform, 32-bit or 64-bit.
    // ARGUMENT_DIGITS ensures the correct value
    NumberString *newNumber = new (Numerics::ARGUMENT_DIGITS) NumberString (Numerics::ARGUMENT_DIGITS);
    newNumber->formatNumber(integer);      /* format the number               */
    return newNumber;
}

NumberString *NumberString::newInstanceFromStringsize(size_t integer)
/******************************************************************************/
/* Function:  Create a NumberString object from a size_t value                */
/******************************************************************************/
{
    // the size of the integer depends on the platform, 32-bit or 64-bit.
    // ARGUMENT_DIGITS ensures the correct value
    NumberString *newNumber = new (Numerics::ARGUMENT_DIGITS) NumberString (Numerics::ARGUMENT_DIGITS);
    newNumber->formatUnsignedNumber(integer);     /* format the number        */
    return newNumber;
}


NumberString *NumberString::newInstanceFromInt64(int64_t integer)
/******************************************************************************/
/* Function:  Create a NumberString object from a signed 64 bit number        */
/******************************************************************************/
{
    // this give us space for entire binary range of the int64_t number.
    NumberString *newNumber = new (Numerics::DIGITS64) NumberString (Numerics::DIGITS64);
    newNumber->formatInt64(integer);  /* format the number                    */
    return newNumber;
}


NumberString *NumberString::newInstanceFromUint64(uint64_t integer)
/******************************************************************************/
/* Function:  Create a NumberString object from an unsigned 64 bit number     */
/******************************************************************************/
{
    // this give us space for entire binary range of the uint64_t number.
    NumberString *newNumber = new (Numerics::DIGITS64) NumberString (Numerics::DIGITS64);
    newNumber->formatUnsignedInt64(integer);  /* format the number            */
    return newNumber;
}

                                       /* numberstring operator methods     */
PCPPM NumberString::operatorMethods[] =
{
   NULL,                               /* first entry not used              */
   (PCPPM)&NumberString::plus,
   (PCPPM)&NumberString::minus,
   (PCPPM)&NumberString::multiply,
   (PCPPM)&NumberString::divide,
   (PCPPM)&NumberString::integerDivide,
   (PCPPM)&NumberString::remainder,
   (PCPPM)&NumberString::power,
   (PCPPM)&NumberString::concat,
   (PCPPM)&NumberString::concat, /* Duplicate entry neccessary        */
   (PCPPM)&NumberString::concatBlank,
   (PCPPM)&NumberString::equal,
   (PCPPM)&NumberString::notEqual,
   (PCPPM)&NumberString::isGreaterThan,
   (PCPPM)&NumberString::isLessOrEqual,
   (PCPPM)&NumberString::isLessThan,
   (PCPPM)&NumberString::isGreaterOrEqual,
                           /* Duplicate entry neccessary        */
   (PCPPM)&NumberString::isGreaterOrEqual,
   (PCPPM)&NumberString::isLessOrEqual,
   (PCPPM)&NumberString::strictEqual,
   (PCPPM)&NumberString::strictNotEqual,
   (PCPPM)&NumberString::strictGreaterThan,
   (PCPPM)&NumberString::strictLessOrEqual,
   (PCPPM)&NumberString::strictLessThan,
   (PCPPM)&NumberString::strictGreaterOrEqual,
                           /* Duplicate entry neccessary        */
   (PCPPM)&NumberString::strictGreaterOrEqual,
   (PCPPM)&NumberString::strictLessOrEqual,
   (PCPPM)&NumberString::notEqual,
                           /* Duplicate entry neccessary        */
   (PCPPM)&NumberString::notEqual,
   (PCPPM)&NumberString::andOp,
   (PCPPM)&NumberString::orOp,
   (PCPPM)&NumberString::xorOp,
   (PCPPM)&NumberString::operatorNot,
};
