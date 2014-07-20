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
/* REXX Kernel                                         NumberStringClass.hpp  */
/*                                                                            */
/* Primitive NumberString Class Definitions                                   */
/*                                                                            */
/******************************************************************************/
#ifndef Included_NumberString
#define Included_NumberString

#include "Numerics.hpp"
#include "NumberStringMath.hpp"
#include "FlagSet.hpp"


                                            /*  at NumDigits, avoid double rounding */


class NumberStringBase : public RexxObject
{
 friend class NumberString;
public:
    typedef enum
    {
        NumberFormScientific,
        NumberRounded,
    } NumberFlag;


    inline NumberStringBase() { ; }

    void   mathRound(char *);
    char  *stripLeadingZeros(char *);
    char * adjustNumber(char *, char *, size_t, size_t);

  protected:

    RexxString *stringObject;          // converted string value
    FlagSet<NumberFlag, 16> numFlags;  // Flags for use by the Numberstring methods
    short sign;                        // sign for this number (-1 is neg)
    size_t  numDigits;                 // Maintain a copy of digits setting of from when object was created
    wholenumber_t exp;                 // the exponent value
    size_t  length;                    // the length of the number data
};


class NumberString : public NumberStringBase
{
   public:
    void         *operator new(size_t, size_t);
    inline void  *operator new(size_t size, void *ptr) {return ptr;}
    inline void   operator delete(void *) { ; }
    inline void   operator delete(void *, size_t) { }
    inline void   operator delete(void *, void *) { }


    NumberString(size_t) ;
    NumberString(size_t, size_t) ;
    inline NumberString(RESTORETYPE restoreType) { ; };
    virtual HashCode getHashValue();
    void        live(size_t);
    void        liveGeneral(MarkReason reason);
    void        flatten(Envelope *);

    bool         numberValue(wholenumber_t &result, size_t precision);
    bool         numberValue(wholenumber_t &result);
    bool         unsignedNumberValue(stringsize_t &result, size_t precision);
    bool         unsignedNumberValue(stringsize_t &result);
    bool         doubleValue(double &result);
    inline NumberString *numberString() { return this; }
    RexxInteger *integerValue(size_t);
    RexxString  *makeString();
    ArrayClass   *makeArray();
    bool         hasMethod(RexxString *);
    RexxString  *primitiveMakeString();
    RexxString  *stringValue();
    bool         truthValue(int);
    virtual bool logicalValue(logical_t &);

    bool        isEqual(RexxObject *);
    wholenumber_t strictComp(RexxObject *);
    wholenumber_t comp(RexxObject *);
    RexxInteger *equal(RexxObject *);
    RexxInteger *strictEqual(RexxObject *);
    RexxInteger *notEqual(RexxObject *);
    RexxInteger *strictNotEqual(RexxObject *);
    RexxInteger *isGreaterThan(RexxObject *);
    RexxInteger *isLessThan(RexxObject *);
    RexxInteger *isGreaterOrEqual(RexxObject *);
    RexxInteger *isLessOrEqual(RexxObject *);
    RexxInteger *strictGreaterThan(RexxObject *);
    RexxInteger *strictLessThan(RexxObject *);
    RexxInteger *strictGreaterOrEqual(RexxObject *);
    RexxInteger *strictLessOrEqual(RexxObject *);
    RexxObject  *hashCode();

    NumberString *clone();
    void        setString(RexxString *);
    void        roundUp(int);
    RexxString *formatRexx(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
    RexxString *formatInternal(size_t, size_t, size_t, size_t, NumberString *, size_t, bool);
    RexxObject *operatorNot(RexxObject *);
    RexxObject *evaluate(RexxActivation *, RexxExpressionStack *);
    RexxObject *getValue(RexxActivation *context);
    RexxObject *getValue(RexxVariableDictionary *dictionary);
    RexxObject *getRealValue(RexxActivation *);
    RexxObject *getRealValue(RexxVariableDictionary *);
    RexxObject *trunc(RexxObject *);
    RexxObject *truncInternal(size_t);
    RexxObject *floor();
    RexxObject *floorInternal();
    RexxObject *ceiling();
    RexxObject *ceilingInternal();
    RexxObject *round();
    RexxObject *roundInternal();
    RexxObject *unknown(RexxString *, ArrayClass *);
    bool        isInstanceOf(RexxClass *);
    MethodClass   *instanceMethod(RexxString *);
    SupplierClass *instanceMethods(RexxClass *);
    RexxClass  *classObject();
    inline NumberString *checkNumber(size_t digits)
    {
       if (length > digits)            // is the length larger than digits()?
       {
                                             // need to allocate a new number, but
                                             // we chop to digits + 1
           return prepareOperatorNumber(digits + 1, digits, NOROUND);
       }
       return this;                          // no adjustment required
    }

    NumberString *prepareNumber(size_t, bool);
    NumberString *prepareOperatorNumber(size_t, size_t, bool);
    void              adjustPrecision(char *, size_t);
    void              adjustPrecision();
    inline void       checkPrecision() { if (length > numDigits) adjustPrecision(); }
    inline void       setNumericSettings(size_t digits, bool form)
    {
        numDigits = digits;
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
        /* inherit the current numeric settings */
        setNumericSettings(number_digits(), number_form());
        /* check for any required rounding */
        checkPrecision();
    }

    bool  createUnsignedValue(const char *thisnum, stringsize_t intlength, int carry, wholenumber_t exponent, size_t maxValue, size_t &result);
    bool  createUnsignedInt64Value(const char *thisnum, stringsize_t intlength, int carry, wholenumber_t exponent, uint64_t maxValue, uint64_t &result);
    bool  checkIntegerDigits(stringsize_t numDigits, stringsize_t &numberLength, wholenumber_t &numberExponent, bool &carry);
    bool  int64Value(int64_t *result, stringsize_t numDigits);
    bool  unsignedInt64Value(uint64_t *result, stringsize_t numDigits);
    void  formatInt64(int64_t integer);
    void  formatUnsignedInt64(uint64_t integer);

    NumberString *addSub(NumberString *, unsigned int, size_t);
    NumberString *plus(RexxObject *);
    NumberString *minus(RexxObject *);
    NumberString *multiply(RexxObject *);
    NumberString *divide(RexxObject *);
    NumberString *integerDivide(RexxObject *);
    NumberString *remainder(RexxObject *);
    NumberString *power(RexxObject *);
    NumberString *Multiply(NumberString *);
    NumberString *Division(NumberString *, unsigned int);
    NumberString *abs();
    RexxInteger *Sign();
    RexxObject  *notOp();
    NumberString *Max(RexxObject **, size_t);
    NumberString *Min(RexxObject **, size_t);
    NumberString *maxMin(RexxObject **, size_t, unsigned int);
    RexxObject *isInteger();
    RexxString *d2c(RexxObject *);
    RexxString *d2x(RexxObject *);
    RexxString *d2xD2c(RexxObject *, bool);
    RexxString *concat(RexxObject *);
    RexxString *concatBlank(RexxObject *);
    RexxObject *orOp(RexxObject *);
    RexxObject *andOp(RexxObject *);
    RexxObject *xorOp(RexxObject *);
    void        formatNumber(wholenumber_t);
    void        formatUnsignedNumber(size_t);
    int         format(const char *, size_t);
    inline void        setZero() {
                   number[0] = '\0';               /* Make value a zero.*/
                   length = 1;                     /* Length is 1       */
                   sign = 0;                       /* Make sign Zero.   */
                   exp = 0;                        /* exponent is zero. */
                }

    static PCPPM operatorMethods[];

    static NumberString *newInstanceFromDouble(double);
    static NumberString *newInstanceFromDouble(double, size_t);
    static NumberString *newInstanceFromFloat(float);
    static NumberString *newInstanceFromWholenumber(wholenumber_t);
    static NumberString *newInstanceFromInt64(int64_t);
    static NumberString *newInstanceFromUint64(uint64_t);
    static NumberString *newInstanceFromStringsize(stringsize_t);
    static NumberString *newInstance(const char *, stringsize_t);


    static void createInstance();
    static RexxClass *classInstance;

    static size_t highBits(size_t);
    static void  subtractNumbers( NumberString *larger, const char *largerPtr, wholenumber_t aLargerExp,
                                  NumberString *smaller, const char *smallerPtr, wholenumber_t aSmallerExp,
                                  NumberString *result, char **resultPtr);
    static char *addMultiplier( char *, size_t, char *, int);
    static char *subtractDivisor(char *data1, size_t length1, char *data2, size_t length2, char *result, int Mult);
    static char *multiplyPower(char *leftPtr, NumberStringBase *left, char *rightPtr, NumberStringBase *right, char *OutPtr, size_t OutLen, size_t NumberDigits);
    static char *dividePower(char *AccumPtr, NumberStringBase *Accum, char *Output, size_t NumberDigits);
    static char *addToBaseSixteen(int, char *, char *);
    static char *addToBaseTen(int, char *, char *);
    static char *multiplyBaseSixteen(char *, char *);
    static char *multiplyBaseTen(char *, char *);

    static const size_t OVERFLOWSPACE = 2;   // space for numeric buffer overflow

    char  number[4];
};


inline NumberString *new_numberstring(const char *s, stringsize_t l)
{
    return NumberString::newInstance(s, l);
}

inline NumberString *new_numberstringFromWholenumber(wholenumber_t n)
{
    return NumberString::newInstanceFromWholenumber(n);
}

inline NumberString *new_numberstringFromStringsize(stringsize_t n)
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
