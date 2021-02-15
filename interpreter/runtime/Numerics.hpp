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
/* Utility class to manage the various sorts of numeric conversions required  */
/* by Rexx.  These conversions are all just static methods.                   */
/*                                                                            */
/******************************************************************************/
#ifndef Included_Numerics
#define Included_Numerics

#include <algorithm>
#include "RexxCore.h"


/**
 * A class for processing different numeric settings
 */
class NumericSettings
{
 public:

    NumericSettings();

    void setDefault();

    inline void   setDigits(wholenumber_t d) { digits = d; }
    inline wholenumber_t getDigits() const { return digits; }
    inline void   setForm(bool f) { form = f; }
    inline bool   getForm() const { return form; }
    inline void   setFuzz(wholenumber_t f) { fuzz = f; }
    inline wholenumber_t getFuzz() const { return fuzz; }

protected:

    wholenumber_t digits;                 // numeric digits setting
    wholenumber_t fuzz;                   // numeric fuzz setting
    bool   form;                          // numeric form setting
};


/**
 * A class for holding all numeric-based settings and
 * some numeric oriented static methods.
 */
class Numerics
{
public:
#ifdef __REXX64__
    static const wholenumber_t MAX_WHOLENUMBER = __INT64_C(999999999999999999);
    static const wholenumber_t MIN_WHOLENUMBER = __INT64_C(-999999999999999999);
    // the digits setting used internally for function/method arguments to allow
    // for the full range
    static const wholenumber_t ARGUMENT_DIGITS  = 18;
    // this is the digits setting for full size binary settings
    static const wholenumber_t SIZE_DIGITS  = 20;
#else
    static const wholenumber_t MAX_WHOLENUMBER = 999999999;
    static const wholenumber_t MIN_WHOLENUMBER = -999999999;
        // the digits setting used internally for function/method arguments to allow
        // for the full binary value range
    static const wholenumber_t ARGUMENT_DIGITS  = 9;
    // this is the digits setting for full size binary settings
    static const wholenumber_t SIZE_DIGITS  = 10;
#endif

    // RexxInteger size limits
    static const wholenumber_t REXXINTEGER_DIGITS;
    static const wholenumber_t MIN_REXXINTEGER = MIN_WHOLENUMBER;
    static const wholenumber_t MAX_REXXINTEGER = MAX_WHOLENUMBER;
#ifdef __REXX64__
    static const wholenumber_t REXXINTEGER_BITS = 64;
#else
    static const wholenumber_t REXXINTEGER_BITS = 32;
#endif

    static const wholenumber_t MAX_EXPONENT = 999999999;
    static const wholenumber_t MIN_EXPONENT = -999999999;
    static const wholenumber_t DEFAULT_DIGITS  = 9;
    // a digits setting for full range integer conversion
    static const size_t  MAX_STRINGSIZE = MAX_WHOLENUMBER;

    // max numeric digits value for explicit 64-bit conversions
    static const wholenumber_t DIGITS64 = 20;
    static const bool FORM_SCIENTIFIC;
    static const bool FORM_ENGINEERING;

    static const wholenumber_t DEFAULT_FUZZ = 0;
    static const bool DEFAULT_FORM;

    static const wholenumber_t validMaxWhole[];      // table of maximum values per digits setting
    static const wholenumber_t validMaxWholeBits[];  // table of bit sizes of above values

    static RexxObject *wholenumberToObject(wholenumber_t v);
    static RexxObject *stringsizeToObject(size_t v);
    static RexxObject *int64ToObject(int64_t v);
    static RexxObject *uint64ToObject(uint64_t v);
    static RexxObject *uintptrToObject(uintptr_t v);
    static RexxObject *intptrToObject(intptr_t v);

    static bool objectToWholeNumber(RexxObject *o, wholenumber_t &result, wholenumber_t max, wholenumber_t min);
    static bool objectToStringSize(RexxObject *o, size_t &result, size_t max);
    static bool objectToSignedInteger(RexxObject *o, ssize_t &result, ssize_t max, ssize_t min);
    static bool objectToUnsignedInteger(RexxObject *o, size_t &result, size_t max);
    static bool objectToInt64(RexxObject *o, int64_t &result);
    static bool objectToUnsignedInt64(RexxObject *o, uint64_t &result);
    static bool objectToUintptr(RexxObject *source, uintptr_t &result);
    static bool objectToIntptr(RexxObject *source, intptr_t &result);
    static RexxObject *int64Object(RexxObject *source);

    static size_t formatWholeNumber(wholenumber_t integer, char *dest);
    static size_t formatStringSize(size_t integer, char *dest);
    static size_t formatInt64(int64_t integer, char *dest);
    static size_t formatUnsignedInt64(uint64_t integer, char *dest);

    static size_t normalizeWholeNumber(wholenumber_t integer, char *dest);

    static wholenumber_t digits() { return settings->getDigits(); }
    static wholenumber_t fuzz()   { return settings->getFuzz(); }
    static bool   form()   { return settings->getForm(); }
    static void   setCurrentSettings(const NumericSettings *s) { settings = s; }
    static const NumericSettings *setDefaultSettings() { settings = &defaultSettings; return settings; }
    static const NumericSettings *getDefaultSettings() { return &defaultSettings; }
    static inline wholenumber_t maxValueForDigits(wholenumber_t d)
    {
        if (d > REXXINTEGER_DIGITS) // 9 for 32-bit, 18 for 64-bit
        {
            return validMaxWhole[REXXINTEGER_DIGITS];
        }
        else
        {
            return validMaxWhole[d];
        }
    }

    static inline wholenumber_t maxBitsForDigits(wholenumber_t d)
    {
        if (d > REXXINTEGER_DIGITS) // 9 for 32-bit, 18 for 64-bit
        {
            return validMaxWholeBits[REXXINTEGER_DIGITS];
        }
        else
        {
            return validMaxWholeBits[d];
        }
    }

    static RexxString *pointerToString(void *);

    static inline bool isValid(wholenumber_t v) { return v <= MAX_WHOLENUMBER && v >= MIN_WHOLENUMBER; }
    // this has a different name because when compiling for 64-bit, wholenumber_t and int64_t are the same and
    // the compiler complains.  The first is for validating a whole number value, the second is for validating an
    // explicit 64-bit value.
    static inline bool isValid32Bit(int64_t v) { return v <= INT32_MAX && v >= INT32_MIN; }
    static inline bool isValid(wholenumber_t v, wholenumber_t digits)  { return std::abs(v) <= validMaxWhole[std::min(digits, REXXINTEGER_DIGITS)]; }
    static inline bool isValid64Bit(int64_t v, wholenumber_t digits)  { digits = std::min(digits, REXXINTEGER_DIGITS); return v <= (int64_t)validMaxWhole[digits] && v >= -(int64_t)validMaxWhole[digits]; }


protected:

    static const NumericSettings *settings;
    static const NumericSettings  defaultSettings;
};


inline wholenumber_t number_digits() { return Numerics::digits(); }
inline wholenumber_t number_fuzz()   { return Numerics::fuzz(); }
inline bool   number_form()   { return Numerics::form(); }
inline wholenumber_t number_fuzzydigits()   { return number_digits() - number_fuzz(); }
#endif

