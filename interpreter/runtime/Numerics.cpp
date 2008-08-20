/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/* Utility class to manage the various sorts of numeric conversions required  */
/* by Rexx.  These conversions are all just static methods.                   */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "Numerics.hpp"
#include "NumberStringClass.hpp"
#include "IntegerClass.hpp"
#include <limits.h>

#ifdef __REXX64__
wholenumber_t Numerics::MAX_WHOLENUMBER = __INT64_C(999999999999999999);
wholenumber_t Numerics::MIN_WHOLENUMBER = __INT64_C(-999999999999999999);
wholenumber_t Numerics::MAX_EXPONENT = __INT64_C(999999999999999999);
wholenumber_t Numerics::MIN_EXPONENT = __INT64_C(-999999999999999999);
stringsize_t Numerics::DEFAULT_DIGITS  = ((stringsize_t)18);
    // the digits setting used internally for function/method arguments to allow
    // for the full range
stringsize_t Numerics::ARGUMENT_DIGITS  = ((stringsize_t)20);


/* Array for valid whole number at various digits settings */
/*  for value 1-18.                                         */
wholenumber_t Numerics::validMaxWhole[] = {10,
                                           100,
                                           1000,
                                           10000,
                                           100000,
                                           1000000,
                                           10000000,
                                           100000000,
                                           1000000000,
                                           10000000000,
                                           100000000000,
                                           1000000000000,
                                           10000000000000,
                                           100000000000000,
                                           1000000000000000,
                                           10000000000000000,
                                           100000000000000000,
                                           1000000000000000000};
#else
wholenumber_t Numerics::MAX_WHOLENUMBER = 999999999;
wholenumber_t Numerics::MIN_WHOLENUMBER = -999999999;
wholenumber_t Numerics::MAX_EXPONENT = 999999999;
wholenumber_t Numerics::MIN_EXPONENT = -999999999;
stringsize_t Numerics::DEFAULT_DIGITS  = ((stringsize_t)9);
    // the digits setting used internally for function/method arguments to allow
    // for the full binary value range
stringsize_t Numerics::ARGUMENT_DIGITS  = ((stringsize_t)10);


/* Array for valid whole number at various digits settings */
/*  for value 1-9.                                         */
wholenumber_t Numerics::validMaxWhole[] = {10,
                                           100,
                                           1000,
                                           10000,
                                           100000,
                                           1000000,
                                           10000000,
                                           100000000,
                                           1000000000};
#endif
stringsize_t  Numerics::MAX_STRINGSIZE = SIZE_MAX;
    // max numeric digits value for explicit 64-bit conversions
stringsize_t Numerics::DIGITS64 = ((stringsize_t)20);
bool Numerics::FORM_SCIENTIFIC    = false;
bool Numerics::FORM_ENGINEERING   = true;

stringsize_t Numerics::DEFAULT_FUZZ    = ((stringsize_t)0); /* default numeric fuzz setting      */
                                     /* default numeric form setting      */
bool Numerics::DEFAULT_FORM = Numerics::FORM_SCIENTIFIC;

NumericSettings Numerics::defaultSettings;
NumericSettings *Numerics::settings = &Numerics::defaultSettings;


NumericSettings::NumericSettings()
{
    digits = Numerics::DEFAULT_DIGITS;
    fuzz = Numerics::DEFAULT_FUZZ;
    form = Numerics::DEFAULT_FORM;
}


/**
 * Convert a signed int64 object into the appropriate Rexx
 * object type.
 *
 * @param v      The value to convert.
 *
 * @return The Rexx object version of this number.
 */
RexxObject *Numerics::int64ToObject(int64_t v)
{
    // in the range for an integer object?
    if (v <= MAX_WHOLENUMBER && v >= MIN_WHOLENUMBER)
    {
        return new_integer((wholenumber_t)v);
    }
    // out of range, we need to use a numberstring for this, using the full
    // allowable digits range
    return new_numberstringFromInt64(v);
}


/**
 * Convert a signed int64 object into the appropriate Rexx
 * object type.
 *
 * @param v      The value to convert.
 *
 * @return The Rexx object version of this number.
 */
RexxObject *Numerics::uint64ToObject(uint64_t v)
{
    // in the range for an integer object?
    if (v <= (uint64_t)MAX_WHOLENUMBER)
    {
        return new_integer((stringsize_t)v);
    }
    // out of range, we need to use a numberstring for this, using the full
    // allowable digits range
    return new_numberstringFromUint64(v);
}


/**
 * Convert an signed number value into the appropriate Rexx
 * object type.
 *
 * @param v      The value to convert.
 *
 * @return The Rexx object version of this number.
 */
RexxObject *Numerics::wholenumberToObject(wholenumber_t v)
{
    // in the range for an integer object?
    if (v <= MAX_WHOLENUMBER && v >= MIN_WHOLENUMBER)
    {
        return new_integer((wholenumber_t)v);
    }
    // out of range, we need to use a numberstring for this, using the full
    // allowable digits range
    return new_numberstringFromWholenumber(v);
}


/**
 * Convert an unsigned number value into the appropriate Rexx
 * object type.
 *
 * @param v      The value to convert.
 *
 * @return The Rexx object version of this number.
 */
RexxObject *Numerics::stringsizeToObject(stringsize_t v)
{
    // in the range for an integer object?
    if (v <= (stringsize_t)MAX_WHOLENUMBER)
    {
        return new_integer((stringsize_t)v);
    }
    // out of range, we need to use a numberstring for this, using the full
    // allowable digits range
    return new_numberstringFromStringsize(v);
}

/**
 * Convert an object into a whole number value.
 *
 * @param source   The source object.
 * @param result   The returned converted value.
 * @param maxValue The maximum range value for the target.
 * @param minValue The minimum range value for this number.
 *
 * @return true if the number converted properly, false for any
 *         conversion errors.
 */
bool Numerics::objectToWholeNumber(RexxObject *source, wholenumber_t &result, wholenumber_t maxValue, wholenumber_t minValue)
{
    wholenumber_t temp;
    // if not a valid whole number, reject this too
    if (!source->numberValue(temp, ARGUMENT_DIGITS))
    {
        return false;
    }

    // outside of the minimum range?  reject this too.
    if (temp > maxValue || temp < minValue)
    {
        return false;
    }

    // pass this back and indicate success.
    result = temp;
    return true;
}


/**
 * Convert an object into an unsigned number value.
 *
 * @param source   The source object.
 * @param result   The returned converted value.
 * @param maxValue The maximum range value for the target.
 *
 * @return true if the number converted properly, false for any
 *         conversion errors.
 */
bool Numerics::objectToStringSize(RexxObject *source, stringsize_t &result, stringsize_t maxValue)
{
    // is this an integer value (very common)
    if (isInteger(source))
    {
        result = ((RexxInteger *)source)->stringSize();
        return result <= maxValue ? true : false;
    }
    else
    {
        // get this as a numberstring (which it might already be)
        RexxNumberString *nString = source->numberString();
        // not convertible to number string?  get out now
        if (nString == OREF_NULL)
        {
            return false;
        }
        uint64_t temp;

        // if not valid or outside of the minimum range, reject this too
        if (nString->unsignedInt64Value(&temp, ARGUMENT_DIGITS))
        {
            if ( temp <= maxValue )
            {
                result = (stringsize_t)temp;
                return true;
            }
        }
        return false;
    }
}


/**
 * Convert an object into a signed int64 value.
 *
 * @param source   The source object.
 * @param result   The returned converted value.
 *
 * @return true if the number converted properly, false for any
 *         conversion errors.
 */
bool Numerics::objectToInt64(RexxObject *source, int64_t &result)
/******************************************************************************/
/* Function:  Convert a Rexx object into a numeric value within the specified */
/* value range.  If the value is not convertable to an integer value or is    */
/* outside of the specified range, false is returned.                         */
/******************************************************************************/
{
    // is this an integer value (very common)
    if (isInteger(source))
    {
        result = ((RexxInteger *)source)->wholeNumber();
        return true;
    }
    else
    {
        // get this as a numberstring (which it might already be)
        RexxNumberString *nString = source->numberString();
        // not convertible to number string?  get out now
        if (nString == OREF_NULL)
        {
            return false;
        }

        // if not a valid whole number, reject this too
        return nString->int64Value(&result, DIGITS64);
    }
}


/**
 * Convert an object into an unsigned int64 value.
 *
 * @param source   The source object.
 * @param result   The returned converted value.
 *
 * @return true if the number converted properly, false for any
 *         conversion errors.
 */
bool Numerics::objectToUnsignedInt64(RexxObject *source, uint64_t &result)
/******************************************************************************/
/* Function:  Convert a Rexx object into a numeric value within the specified */
/* value range.  If the value is not convertable to an integer value or is    */
/* outside of the specified range, false is returned.                         */
/******************************************************************************/
{
    // is this an integer value (very common)
    if (isInteger(source))
    {
        result = ((RexxInteger *)source)->stringSize();
        return true;
    }
    else
    {
        // get this as a numberstring (which it might already be)
        RexxNumberString *nString = source->numberString();
        // not convertible to number string?  get out now
        if (nString == OREF_NULL)
        {
            return false;
        }

        // if not a valid whole number, reject this too
        return nString->unsignedInt64Value(&result, DIGITS64);
    }
}


/**
 * Convert an object into an uintptr_t values.  Used for values
 * that are numbers masking as pointers.
 *
 * @param source The source object.
 * @param result The returned value.
 *
 * @return true if this converted, false for any conversion failures.
 */
bool Numerics::objectToUintptr(RexxObject *source, uintptr_t &result)
{
    // is this an integer value (very common)
    if (isInteger(source))
    {
        result = ((RexxInteger *)source)->stringSize();
        return true;
    }
    else
    {
        // get this as a numberstring (which it might already be)
        RexxNumberString *nString = source->numberString();
        // not convertible to number string?  get out now
        if (nString == OREF_NULL)
        {
            return false;
        }
        uint64_t temp;

        // if not a valid whole number, reject this too
        if (nString->unsignedInt64Value(&temp, ARGUMENT_DIGITS))
        {
            result = (uintptr_t)temp;
            return true;
        }
        return false;
    }
}


/**
 * Convert an object into an intptr_t values.  Used for values
 * that are numbers masking as pointers.
 *
 * @param source The source object.
 * @param result The returned value.
 *
 * @return true if this converted, false for any conversion failures.
 */
bool Numerics::objectToIntptr(RexxObject *source, intptr_t &result)
{
    // is this an integer value (very common)
    if (isInteger(source))
    {
        result = ((RexxInteger *)source)->wholeNumber();
        return true;
    }
    else
    {
        // get this as a numberstring (which it might already be)
        RexxNumberString *nString = source->numberString();
        // not convertible to number string?  get out now
        if (nString == OREF_NULL)
        {
            return false;
        }
        int64_t temp;

        // if not a valid whole number, reject this too
        if (nString->int64Value(&temp, ARGUMENT_DIGITS))
        {
            result = (intptr_t)temp;
            return true;
        }
        return false;
    }
}


/**
 * Do portable formatting of a wholenumber value into an ascii
 * string.
 *
 * @param integer The value to convert.
 * @param dest    The location to store the formatted string.
 *
 * @return The length of the converted number.
 */
stringsize_t Numerics::formatWholeNumber(wholenumber_t integer, char *dest)
{
    // zero? this is pretty easy
    if (integer == 0)
    {
        strcpy((char *)dest, "0");
        return 1;
    }

    size_t sign = 0;
    // negative number?  copy a negative sign, and take the abs value
    if (integer < 0)
    {
        *dest++ = '-';
        integer = -integer;
        sign = 1;   // added in to the length
    }

    // we convert this directly because portable numeric-to-ascii routines
    // don't really exist for the various 32/64 bit values.
    char buffer[24];
    size_t index = sizeof(buffer);

    while (integer > 0)
    {
        // get the digit and reduce the size of the integer
        int digit = (int)(integer % 10) + '0';
        integer = integer / 10;
        // store the digit
        buffer[--index] = digit;
    }

    // copy into the buffer and set the length
    stringsize_t length = sizeof(buffer) - index;
    memcpy(dest, &buffer[index], length);
    // make sure we have a terminating null
    dest[length] = '\0';
    return length + sign;
}

/**
 * Do portable formatting of a stringsize value into an ascii
 * string.
 *
 * @param integer The value to convert.
 * @param dest    The location to store the formatted string.
 *
 * @return The length of the converted number.
 */
stringsize_t Numerics::formatStringSize(stringsize_t integer, char *dest)
{
    // zero? this is pretty easy
    if (integer == 0)
    {
        strcpy((char *)dest, "0");
        return 1;
    }

    // we convert this directly because portable numeric-to-ascii routines
    // don't really exist for the various 32/64 bit values.
    char buffer[24];
    size_t index = sizeof(buffer);

    while (integer > 0)
    {
        // get the digit and reduce the size of the integer
        int digit = (int)(integer % 10) + '0';
        integer = integer / 10;
        // store the digit
        buffer[--index] = digit;
    }

    // copy into the buffer and set the length
    stringsize_t length = sizeof(buffer) - index;
    memcpy(dest, &buffer[index], length);
    // make sure we have a terminating null
    dest[length] = '\0';
    return length;
}


/**
 * Do portable formatting of an int64_t value into an ascii
 * string.
 *
 * @param integer The value to convert.
 * @param dest    The location to store the formatted string.
 *
 * @return The length of the converted number.
 */
stringsize_t Numerics::formatInt64(int64_t integer, char *dest)
{
    // zero? this is pretty easy
    if (integer == 0)
    {
        strcpy((char *)dest, "0");
        return 1;
    }

    size_t sign = 0;
    // negative number?  copy a negative sign, and take the abs value
    if (integer < 0)
    {
        *dest++ = '-';
        integer = -integer;
        sign = 1;   // added in to the length
    }

    // we convert this directly because portable numeric-to-ascii routines
    // don't really exist for the various 32/64 bit values.
    char buffer[32];
    size_t index = sizeof(buffer);

    while (integer > 0)
    {
        // get the digit and reduce the size of the integer
        int digit = (int)(integer % 10) + '0';
        integer = integer / 10;
        // store the digit
        buffer[--index] = digit;
    }

    // copy into the buffer and set the length
    stringsize_t length = sizeof(buffer) - index;
    memcpy(dest, &buffer[index], length);
    // make sure we have a terminating null
    dest[length] = '\0';
    return length + sign;
}


/**
 * Do portable formatting of a uint64_t value into an ascii
 * string.
 *
 * @param integer The value to convert.
 * @param dest    The location to store the formatted string.
 *
 * @return The length of the converted number.
 */
stringsize_t Numerics::formatUnsignedInt64(uint64_t integer, char *dest)
{
    // zero? this is pretty easy
    if (integer == 0)
    {
        strcpy(dest, "0");
        return 1;
    }

    // we convert this directly because portable numeric-to-ascii routines
    // don't really exist for the various 32/64 bit values.
    char buffer[32];
    size_t index = sizeof(buffer);

    while (integer > 0)
    {
        // get the digit and reduce the size of the integer
        int digit = (int)(integer % 10) + '0';
        integer = integer / 10;
        // store the digit
        buffer[--index] = digit;
    }

    // copy into the buffer and set the length
    stringsize_t length = sizeof(buffer) - index;
    memcpy(dest, &buffer[index], length);
    // make sure we have a terminating null
    dest[length] = '\0';
    return length;
}


/**
 * Convert an unsigned ptr value into the appropriate Rexx
 * object type.
 *
 * @param v      The value to convert.
 *
 * @return The Rexx object version of this number.
 */
RexxObject *Numerics::uintptrToObject(uintptr_t v)
{
    // in the range for an integer object?
    if (v <= (uintptr_t)MAX_WHOLENUMBER)
    {
        return new_integer((wholenumber_t)v);
    }
    // out of range, we need to use a numberstring for this, using the full
    // allowable digits range.  Note that this assumes we maintain the connection
    // that a wholenumber_t is the same size as an intptr_t.
    return new_numberstringFromStringsize((stringsize_t)v);
}


/**
 * Convert an signed ptr value into the appropriate Rexx object
 * type.
 *
 * @param v      The value to convert.
 *
 * @return The Rexx object version of this number.
 */
RexxObject *Numerics::intptrToObject(intptr_t v)
{
    // in the range for an integer object?
    if (v <= (intptr_t)MAX_WHOLENUMBER && v >=(intptr_t)MIN_WHOLENUMBER)
    {
        return new_integer((wholenumber_t)v);
    }
    // out of range, we need to use a numberstring for this, using the full
    // allowable digits range.  Note that this assumes we maintain the connection
    // that a wholenumber_t is the same size as an intptr_t.
    return new_numberstringFromWholenumber((wholenumber_t)v);
}
