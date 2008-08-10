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
#ifndef Included_Numerics
#define Included_Numerics


class NumericSettings                  // "global" numeric settings         */
{
    public:
      NumericSettings();
      size_t digits;                       /* numeric digits setting            */
      size_t fuzz;                         /* numeric fuzz setting              */
      bool form;                           /* numeric form setting              */
};                                         /* global activation settings        */


class Numerics
{
public:
    static wholenumber_t MAX_WHOLENUMBER;
    static wholenumber_t MIN_WHOLENUMBER;
    static wholenumber_t MAX_EXPONENT;
    static wholenumber_t MIN_EXPONENT;
    static stringsize_t DEFAULT_DIGITS;
    // the digits setting used internally for function/method arguments to allow
    // for the full range
    static stringsize_t ARGUMENT_DIGITS;
    static stringsize_t  MAX_STRINGSIZE;

    // max numeric digits value for explicit 64-bit conversions
    static stringsize_t DIGITS64;
    static bool FORM_SCIENTIFIC;
    static bool FORM_ENGINEERING;

    static stringsize_t DEFAULT_FUZZ;
                                     /* default numeric form setting      */
    static bool DEFAULT_FORM;

    static RexxObject *wholenumberToObject(wholenumber_t v);
    static RexxObject *stringsizeToObject(stringsize_t v);
    static RexxObject *int64ToObject(int64_t v);
    static RexxObject *uint64ToObject(uint64_t v);
    static RexxObject *ptrToObject(uintptr_t v);

    static bool objectToWholeNumber(RexxObject *o, wholenumber_t &result, wholenumber_t max, wholenumber_t min);
    static bool objectToStringSize(RexxObject *o, stringsize_t &result, stringsize_t max);
    static bool objectToInt64(RexxObject *o, int64_t &result);
    static bool objectToUnsignedInt64(RexxObject *o, uint64_t &result);
    static bool objectToUintptr(RexxObject *source, uintptr_t &result);

    static stringsize_t formatWholeNumber(wholenumber_t integer, char *dest);
    static stringsize_t formatStringSize(stringsize_t integer, char *dest);
    static stringsize_t formatInt64(int64_t integer, char *dest);
    static stringsize_t formatUnsignedInt64(uint64_t integer, char *dest);

    static wholenumber_t validMaxWhole[];          // table of maximum values per digits setting
    static size_t digits() { return settings->digits; }
    static size_t fuzz()   { return settings->fuzz; }
    static bool   form()   { return settings->form; }
    static void   setCurrentSettings(NumericSettings *s) { settings = s; }
    static NumericSettings *setDefaultSettings() { settings = &defaultSettings; return settings; }
    static NumericSettings *getDefaultSettings() { return &defaultSettings; }
    static inline wholenumber_t abs(wholenumber_t n) { return n < 0 ? -n : n; }
    static inline wholenumber_t minVal(wholenumber_t n1, wholenumber_t n2) { return n2 > n1 ? n1 : n2; }
    static inline stringsize_t minVal(stringsize_t n1, stringsize_t n2) { return n2 > n1 ? n1 : n2; }
    static inline wholenumber_t maxVal(wholenumber_t n1, wholenumber_t n2) { return n2 > n1 ? n2 : n1; }
    static inline stringsize_t maxVal(stringsize_t n1, stringsize_t n2) { return n2 > n1 ? n2 : n1; }
    static inline wholenumber_t maxValueForDigits(size_t d)
    {
        if (d > DEFAULT_DIGITS)
        {
            return validMaxWhole[DEFAULT_DIGITS - 1];
        }
        else
        {
            return validMaxWhole[d - 1];
        }
    }

    static inline wholenumber_t multiplierForExponent(size_t e)
    {
        return validMaxWhole[e - 1];
    }


protected:

    static NumericSettings *settings;
    static NumericSettings  defaultSettings;
};


inline size_t number_digits() { return Numerics::digits(); }
inline size_t number_fuzz()   { return Numerics::fuzz(); }
inline bool   number_form()   { return Numerics::form(); }
inline size_t number_fuzzydigits()   { return number_digits() - number_fuzz(); }
#endif

