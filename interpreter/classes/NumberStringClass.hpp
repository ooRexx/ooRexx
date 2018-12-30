/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2017 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                         NumberStringClass.hpp  */
/*                                                                            */
/* Primitive NumberString Class Definitions                                   */
/*                                                                            */
/******************************************************************************/
#ifndef Included_NumberString
#define Included_NumberString

#include "FlagSet.hpp"
#include "Numerics.hpp"
#include "StringClass.hpp"


/**
 * A base NumberString object.  Occasionally, we create
 * number string instances with no data part, so we have a base
 * class for some of the portions.
 */
class NumberStringBase : public RexxObject
{
 friend class NumberString;
 friend class NumberStringBuilder;
public:
    typedef enum
    {
        NumberFormScientific,
        NumberRounded,
    } NumberFlag;


    inline NumberStringBase() { ; }
    // special constructor for temporary working copies
    inline NumberStringBase(bool opt) : stringObject(OREF_NULL), numberSign(0),
        createdDigits(0), numberExponent(0), digitsCount(0) { }

    void   mathRound(char *);
    char  *stripLeadingZeros(char *);
    char  *adjustNumber(char *, char *, wholenumber_t, wholenumber_t);
    void   truncateToDigits(wholenumber_t digits, char *digitsPtr, bool round);
    //quick test for a numeric overflow
    void checkOverflow();

  protected:

    RexxString *stringObject;          // converted string value
    FlagSet<NumberFlag, 16> numFlags;  // Flags for use by the Numberstring methods
    short numberSign;                  // sign for this number (-1 is neg)
    wholenumber_t  createdDigits;      // the digits setting of from when object was created
    wholenumber_t numberExponent;      // the exponent value
    wholenumber_t digitsCount;         // the length of the number data (more conveniently managed as a signed number)
};


/**
 * The "Full Monty" NumberString.  This implements most of the
 * functions, and directly includes the string data.
 */
class NumberString : public NumberStringBase
{
 public:
   /**
    * Identifiers for different arithmetic operators.
    */
   typedef enum
   {
       OT_PLUS,
       OT_MINUS,
       OT_MULTIPLY,
       OT_DIVIDE,
       OT_INT_DIVIDE,
       OT_REMAINDER,
       OT_POWER,
       OT_MAX,
       OT_MIN,
   } ArithmeticOperator;


   /**
    * A class for constructing a number value from a sequence of
    * append steps.
    */
   class NumberBuilder
   {
   public:
       inline NumberBuilder(RexxString *s) : current(s->getWritableData()) {}

       inline void addSign(bool isNegative) { if (isNegative) { *current = RexxString::ch_MINUS; current++; } }
       inline void addExponentSign(bool isNegative) { *current = isNegative ? RexxString::ch_MINUS : RexxString::ch_PLUS; current++; }
       inline void addDecimal() { append(RexxString::ch_PERIOD); }
       inline void addExponent(const char *exp)
       {
           size_t len = strlen(exp);
           memcpy(current, exp, len);
           current += len;
       }
       inline void append(const char *d, size_t l)  { memcpy(current, d, l); current += l; }
       inline void append(char c) { *current++ = c; }
       inline void addDigits(const char *d, wholenumber_t len)
       {
           for (wholenumber_t i = 0; i < len; i++)
           {
               append(d[i] + RexxString::ch_ZERO);
           }
       }
       inline void addZeroDecimal()  { append('0'); append('.'); }
       inline void addZeros(wholenumber_t count)
       {
           memset(current, '0', count);
           current += count;
       }

       inline void addSpaces(wholenumber_t count)
       {
           memset(current, ' ', count);
           current += count;
       }

       inline void addIntegerPart(bool sign, const char *digits, wholenumber_t intDigits, wholenumber_t pad = 0)
       {
           addSign(sign);
           addDigits(digits, intDigits);
           addZeros(pad);
       }

       inline void addDecimalPart(const char *digits, wholenumber_t decimalDigits, wholenumber_t leadPad = 0, wholenumber_t trailingPad = 0)
       {
           addDecimal();
           addZeros(leadPad);
           addDigits(digits, decimalDigits);
           addZeros(trailingPad);
       }


   protected:
       char *current;   // current output pointer
   };

    void         *operator new(size_t, size_t);
    inline void   operator delete(void *) { }

    NumberString(size_t) ;
    NumberString(size_t, size_t) ;
    inline NumberString(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    HashCode getHashValue() override;

    bool numberValue(wholenumber_t &result, wholenumber_t precision) override;
    bool numberValue(wholenumber_t &result) override;
    bool unsignedNumberValue(size_t &result, wholenumber_t precision) override;
    bool unsignedNumberValue(size_t &result) override;
    bool doubleValue(double &result) override;
    inline NumberString *numberString() override { return this; }
    RexxInteger *integerValue(wholenumber_t) override;
    RexxString  *makeString() override;
    ArrayClass  *makeArray() override;
    bool         hasMethod(RexxString *) override;
    RexxString  *primitiveMakeString() override;
    RexxString  *stringValue() override;
    bool         truthValue(RexxErrorCodes) override;
    bool logicalValue(logical_t &) override;

    bool  isEqual(RexxInternalObject *) override;
    void processUnknown(RexxErrorCodes error, RexxString *, RexxObject **, size_t, ProtectedObject &) override;
    wholenumber_t compareTo(RexxInternalObject *) override;

    wholenumber_t strictComp(RexxObject *);
    wholenumber_t comp(RexxObject *, size_t fuzz);
    RexxObject  *equal(RexxObject *);
    RexxObject  *strictEqual(RexxObject *);
    RexxObject  *notEqual(RexxObject *);
    RexxObject  *strictNotEqual(RexxObject *);
    RexxObject  *isGreaterThan(RexxObject *);
    RexxObject  *isLessThan(RexxObject *);
    RexxObject  *isGreaterOrEqual(RexxObject *);
    RexxObject  *isLessOrEqual(RexxObject *);
    RexxObject  *strictGreaterThan(RexxObject *);
    RexxObject  *strictLessThan(RexxObject *);
    RexxObject  *strictGreaterOrEqual(RexxObject *);
    RexxObject  *strictLessOrEqual(RexxObject *);
    RexxObject  *hashCode();

    NumberString *clone();
    void        setString(RexxString *);
    RexxString *formatRexx(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
    RexxString *formatInternal(wholenumber_t, wholenumber_t, wholenumber_t, wholenumber_t, NumberString *, wholenumber_t, bool);
    RexxObject *operatorNot(RexxObject *);

    RexxObject *evaluate(RexxActivation *, ExpressionStack *) override;
    RexxObject *getValue(RexxActivation *context) override;
    RexxObject *getValue(VariableDictionary *dictionary) override;
    RexxObject *getRealValue(RexxActivation *) override;
    RexxObject *getRealValue(VariableDictionary *) override;
    bool isInstanceOf(RexxClass *) override;

    RexxObject *trunc(RexxObject *);
    RexxObject *truncInternal(wholenumber_t);
    RexxObject *floor();
    RexxObject *floorInternal();
    RexxObject *ceiling();
    RexxObject *ceilingInternal();
    RexxObject *round();
    RexxObject *roundInternal();

    MethodClass *instanceMethod(RexxString *) override;
    SupplierClass *instanceMethods(RexxClass *) override;

    RexxClass  *classObject();
    inline NumberString *checkNumber(wholenumber_t digits)
    {
       if (digitsCount > digits)             // is the length larger than digits()?
       {
                                             // need to allocate a new number, but
                                             // we chop to digits + 1
           return prepareOperatorNumber(digits + 1, digits, NOROUND);
       }
       return this;                          // no adjustment required
    }

    //quick test for a numeric overflow
    void checkLostDigits(wholenumber_t digits);
    NumberString *operatorArgument(RexxObject *right);

    NumberString *prepareNumber(wholenumber_t, bool);
    NumberString *prepareOperatorNumber(wholenumber_t, wholenumber_t, bool);
    NumberString *copyIfNecessary();
    NumberString *copyForCurrentSettings();
    void              adjustPrecision(char *, wholenumber_t);
    void              adjustPrecision();
    inline void       checkPrecision() { if (digitsCount > createdDigits) adjustPrecision(); }
    inline void       setNumericSettings(wholenumber_t digits, bool form)
    {
        createdDigits = digits;
        if (form == Numerics::FORM_SCIENTIFIC)
        {
            numFlags.set(NumberFormScientific);
        }
        else
        {
            numFlags.reset(NumberFormScientific);
        }
    }

    inline bool isScientific() { return numFlags[NumberFormScientific]; }
    inline bool isEngineering() { return !numFlags[NumberFormScientific]; }

    inline void setupNumber()
    {
        // inherit the current numeric settings
        setNumericSettings(number_digits(), number_form());
        // check for any required rounding
        checkPrecision();
    }

    inline void setupNumber(wholenumber_t digits, bool form)
    {
        // inherit the current numeric settings
        setNumericSettings(digits, form);
        // check for any required rounding
        checkPrecision();
    }

    bool  createUnsignedValue(const char *thisnum, size_t intlength, int carry, wholenumber_t exponent, size_t maxValue, size_t &result);
    bool  createUnsignedInt64Value(const char *thisnum, size_t intlength, int carry, wholenumber_t exponent, uint64_t maxValue, uint64_t &result);
    bool  checkIntegerDigits(wholenumber_t numDigits, wholenumber_t &numberLength, wholenumber_t &numberExponent, bool &carry);
    bool  int64Value(int64_t *result, wholenumber_t numDigits);
    bool  unsignedInt64Value(uint64_t *result, wholenumber_t numDigits);
    void  formatInt64(int64_t integer);
    void  formatUnsignedInt64(uint64_t integer);

    NumberString *addSub(NumberString *, ArithmeticOperator, wholenumber_t);
    NumberString *plus(RexxObject *);
    NumberString *minus(RexxObject *);
    NumberString *multiply(RexxObject *);
    NumberString *divide(RexxObject *);
    NumberString *integerDivide(RexxObject *);
    NumberString *remainder(RexxObject *);
    NumberString *modulo(RexxObject *);
    NumberString *power(RexxObject *);
    NumberString *Multiply(NumberString *);
    NumberString *Division(NumberString *, ArithmeticOperator);
    NumberString *abs();
    RexxInteger *Sign();
    RexxObject  *notOp();
    NumberString *Max(RexxObject **, size_t);
    NumberString *Min(RexxObject **, size_t);
    NumberString *maxMin(RexxObject **, size_t, ArithmeticOperator);
    bool        isInteger();
    RexxString *d2c(RexxObject *);
    RexxString *d2x(RexxObject *);
    RexxString *d2xD2c(RexxObject *, bool);
    RexxString *concat(RexxObject *);
    RexxString *concatBlank(RexxObject *);
    RexxObject *orOp(RexxObject *);
    RexxObject *andOp(RexxObject *);
    RexxObject *xorOp(RexxObject *);
    bool        parseNumber(const char *number, size_t length);
    void        formatNumber(wholenumber_t integer);
    void        formatUnsignedNumber(size_t);

    inline void setZero()
    {
        numberDigits[0] = '\0';         // Make value a zero.
        digitsCount = 1;                // Length is 1
        numberSign = 0;                 // Make sign Zero.
        numberExponent = 0;             // exponent is zero.
    }

    inline bool isZero() { return numberSign == 0; }
    inline bool isOne() { return digitsCount == 1 && numberSign == 1 && numberExponent == 0 && numberDigits[0] == 1; }
    inline bool isNegative() { return numberSign < 0; }
    inline bool isPositive() { return numberSign > 0; }
    inline bool isAllInteger() { return numberExponent == 0; }
    inline bool hasDecimals() { return numberExponent < 0; }
           bool hasSignificantDecimals(wholenumber_t digits);
           void formatExponent(wholenumber_t exponent, char *buffer);

    static PCPPM operatorMethods[];

    static NumberString *newInstanceFromDouble(double);
    static NumberString *newInstanceFromDouble(double, wholenumber_t);
    static NumberString *newInstanceFromFloat(float);
    static NumberString *newInstanceFromWholenumber(wholenumber_t);
    static NumberString *newInstanceFromInt64(int64_t);
    static NumberString *newInstanceFromUint64(uint64_t);
    static NumberString *newInstanceFromStringsize(size_t);
    static NumberString *newInstance(const char *, size_t);


    static void createInstance();
    static RexxClass *classInstance;

    static size_t highBits(size_t);
    static void  subtractNumbers(NumberString *larger, const char *largerPtr, wholenumber_t aLargerExp,
                                  NumberString *smaller, const char *smallerPtr, wholenumber_t aSmallerExp,
                                  NumberString *result, char *&resultPtr);
    static char *addMultiplier(const char *, wholenumber_t, char *, int);
    static char *subtractDivisor(const char *data1, wholenumber_t length1, const char *data2, wholenumber_t length2, char *result, int Mult);
    static char *multiplyPower(const char *leftPtr, NumberStringBase *left, const char *rightPtr, NumberStringBase *right, char *OutPtr, wholenumber_t OutLen, wholenumber_t NumberDigits);
    static char *dividePower(const char *AccumPtr, NumberStringBase *Accum, char *Output, wholenumber_t NumberDigits);
    static char *addToBaseSixteen(int, char *, char *);
    static char *addToBaseTen(int, char *, char *);
    static char *multiplyBaseSixteen(char *, char *);
    static char *multiplyBaseTen(char *, char *);

    static const size_t OVERFLOWSPACE = 2;   // space for numeric buffer overflow

    static const size_t BYTE_SIZE = 8;
    static const size_t SIZEBITS = (sizeof(size_t) * BYTE_SIZE);
    static const bool ROUND = true;
    static const bool NOROUND = false;

    // these are used for masking the power bits
    static const size_t HIBIT = ~SSIZE_MAX;
    static const size_t LOWBITS = SSIZE_MAX;

    // special buffer allocation size.  Since we occasionally use numeric
    // digits 20 for date and time calculations, make sure we can handle at least
    // that size for most operations.  That requires at least twice the digits value
    // plus an extra.  For alignment purposes, makea multiple of 8 also.
    static const size_t FAST_BUFFER = 48;

    char  numberDigits[4];                   // the digits for the number
};


inline NumberString *new_numberstring(const char *s, size_t l)
{
    return NumberString::newInstance(s, l);
}

inline NumberString *new_numberstringFromWholenumber(wholenumber_t n)
{
    return NumberString::newInstanceFromWholenumber(n);
}

inline NumberString *new_numberstringFromStringsize(size_t n)
{
    return NumberString::newInstanceFromStringsize(n);
}

inline NumberString *new_numberstringFromInt64(int64_t n)
{
    return NumberString::newInstanceFromInt64(n);
}

inline NumberString *new_numberstringFromUint64(uint64_t n)
{
    return NumberString::newInstanceFromUint64(n);
}

inline NumberString *new_numberstringFromDouble(double n)
{
    return NumberString::newInstanceFromDouble(n);
}

inline NumberString *new_numberstringFromDouble(double n, size_t p)
{
    return NumberString::newInstanceFromDouble(n, p);
}

inline NumberString *new_numberstringFromFloat(float n)
{
    return NumberString::newInstanceFromFloat(n);
}

#endif
