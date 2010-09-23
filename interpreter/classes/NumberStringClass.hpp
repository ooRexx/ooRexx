/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
#ifndef Included_RexxNumberString
#define Included_RexxNumberString

#include "Numerics.hpp"

/* Define char data used in OKNUMSTR   */
#define ch_BLANK  ' '                       /* Define a Blank character.            */
#define ch_MINUS  '-'                       /* Define the MINUS character           */
#define ch_PLUS   '+'                       /* Define the PLUS character.           */
#define ch_PERIOD '.'                       /* Define the DOT/PERIOD character.     */
#define ch_ZERO   '0'                       /* Define the Zero  character.          */
#define ch_ONE    '1'                       /* Define the One   character.          */
#define ch_FIVE   '5'                       /* Define the Five  character.          */
#define ch_NINE   '9'                       /* Define the Nine  character.          */
#define ch_TAB    '\t'                      /* Define the alternate whitespace char */

#define NumFormScientific  0x00000001       /* Define Numeric form setting at Object*/
                                            /*  creation time.                      */
#define NumberRounded      0x00000010       /* Indicate the number was rounded once */
                                            /*  at NumDigits, avoid double rounding */

#define OVERFLOWSPACE 2                /* space for numeric buffer overflow */

#define SetNumberStringZero()                                           \
      this->number[0] = '\0';               /* Make value a zero.*/     \
      this->length = 1;                     /* Length is 1       */     \
      this->sign = 0;                       /* Make sign Zero.   */     \
      this->exp = 0;                        /* exponent is zero. */


#define NumberStringRound(s,d) s->roundUp(s,d)

 class RexxNumberStringBase : public RexxObject {
   public:
    inline RexxNumberStringBase() { ; }
    void   mathRound(char *);
    char  *stripLeadingZeros(char *);
    char * adjustNumber(char *, char *, size_t, size_t);

    RexxString *stringObject;          /* converted string value          */
    short NumFlags;                    /* Flags for use by the Numberstring met*/
    short sign;                        /* sign for this number (-1 is neg)     */
    size_t  NumDigits;                 /* Maintain a copy of digits setting of */
                                       /* From when object was created         */
    wholenumber_t exp;
    size_t  length;
 };

 class RexxNumberString : public RexxNumberStringBase {
   public:
    void         *operator new(size_t, size_t);
    inline void  *operator new(size_t size, void *ptr) {return ptr;}
    inline void   operator delete(void *) { ; }
    inline void   operator delete(void *, size_t) { }
    inline void   operator delete(void *, void *) { }


    RexxNumberString(size_t) ;
    RexxNumberString(size_t, size_t) ;
    inline RexxNumberString(RESTORETYPE restoreType) { ; };
    virtual HashCode getHashValue();
    void        live(size_t);
    void        liveGeneral(int reason);
    void        flatten(RexxEnvelope *);

    bool         numberValue(wholenumber_t &result, size_t precision);
    bool         numberValue(wholenumber_t &result);
    bool         unsignedNumberValue(stringsize_t &result, size_t precision);
    bool         unsignedNumberValue(stringsize_t &result);
    bool         doubleValue(double &result);
    inline RexxNumberString *numberString() { return this; }
    RexxInteger *integerValue(size_t);
    RexxString  *makeString();
    RexxInteger *hasMethod(RexxString *);
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

    RexxNumberString *clone();
    void        setString(RexxString *);
    void        roundUp(int);
    RexxString *formatRexx(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
    RexxString *formatInternal(size_t, size_t, size_t, size_t, RexxNumberString *, size_t, bool);
    RexxObject *operatorNot(RexxObject *);
    RexxObject *evaluate(RexxActivation *, RexxExpressionStack *);
    RexxObject *getValue(RexxActivation *context);
    RexxObject *getValue(RexxVariableDictionary *dictionary);
    RexxObject *getRealValue(RexxActivation *);
    RexxObject *getRealValue(RexxVariableDictionary *);
    RexxObject *trunc(RexxObject *);
    RexxObject *truncInternal(size_t);
    RexxObject *unknown(RexxString *, RexxArray *);
    bool        isInstanceOf(RexxClass *);
    RexxMethod   *instanceMethod(RexxString *);
    RexxSupplier *instanceMethods(RexxClass *);
    RexxClass  *classObject();
    inline RexxNumberString *checkNumber(size_t digits, bool rounding)
    {
     if (this->length > digits)            /* is the length larger than digits()*/
                                           /* need to allocate a new number     */
       return this->prepareNumber(digits, rounding);
     return this;                          /* no adjustment required            */
    }

    RexxNumberString *prepareNumber(size_t, bool);
    void              adjustPrecision(char *, size_t);
    void              adjustPrecision();
    inline void       checkPrecision() { if (length > NumDigits) adjustPrecision(); }
    inline void       setNumericSettings(size_t digits, bool form)
    {
        this->NumDigits = digits;
        if (form == Numerics::FORM_SCIENTIFIC)
            this->NumFlags |= NumFormScientific;
        else
            this->NumFlags &= ~NumFormScientific;
    }

    inline void       setupNumber()
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

    RexxNumberString *addSub(RexxNumberString *, unsigned int, size_t);
    RexxNumberString *plus(RexxObject *);
    RexxNumberString *minus(RexxObject *);
    RexxNumberString *multiply(RexxObject *);
    RexxNumberString *divide(RexxObject *);
    RexxNumberString *integerDivide(RexxObject *);
    RexxNumberString *remainder(RexxObject *);
    RexxNumberString *power(RexxObject *);
    RexxNumberString *Multiply(RexxNumberString *);
    RexxNumberString *Division(RexxNumberString *, unsigned int);
    RexxNumberString *abs();
    RexxInteger *Sign();
    RexxObject  *notOp();
    RexxNumberString *Max(RexxObject **, size_t);
    RexxNumberString *Min(RexxObject **, size_t);
    RexxNumberString *maxMin(RexxObject **, size_t, unsigned int);
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
                   this->number[0] = '\0';               /* Make value a zero.*/
                   this->length = 1;                     /* Length is 1       */
                   this->sign = 0;                       /* Make sign Zero.   */
                   this->exp = 0;                        /* exponent is zero. */
                }

    static PCPPM operatorMethods[];

    static RexxNumberString *newInstanceFromDouble(double);
    static RexxNumberString *newInstanceFromDouble(double, size_t);
    static RexxNumberString *newInstanceFromFloat(float);
    static RexxNumberString *newInstanceFromWholenumber(wholenumber_t);
    static RexxNumberString *newInstanceFromInt64(int64_t);
    static RexxNumberString *newInstanceFromUint64(uint64_t);
    static RexxNumberString *newInstanceFromStringsize(stringsize_t);
    static RexxNumberString *newInstance(const char *, stringsize_t);


    static void createInstance();
    static RexxClass *classInstance;

    static size_t highBits(size_t);
    static void  subtractNumbers( RexxNumberString *larger, const char *largerPtr, wholenumber_t aLargerExp,
                                  RexxNumberString *smaller, const char *smallerPtr, wholenumber_t aSmallerExp,
                                  RexxNumberString *result, char **resultPtr);
    static char *addMultiplier( char *, size_t, char *, int);
    static char *subtractDivisor(char *data1, size_t length1, char *data2, size_t length2, char *result, int Mult);
    static char *multiplyPower(char *leftPtr, RexxNumberStringBase *left, char *rightPtr, RexxNumberStringBase *right, char *OutPtr, size_t OutLen, size_t NumberDigits);
    static char *dividePower(char *AccumPtr, RexxNumberStringBase *Accum, char *Output, size_t NumberDigits);
    static char *addToBaseSixteen(int, char *, char *);
    static char *addToBaseTen(int, char *, char *);
    static char *multiplyBaseSixteen(char *, char *);
    static char *multiplyBaseTen(char *, char *);

    char  number[4];
};

void AdjustPrecision(RexxNumberString *, char *, int);

inline RexxNumberString *new_numberstring(const char *s, stringsize_t l)
{
    return RexxNumberString::newInstance(s, l);
}

inline RexxNumberString *new_numberstringFromWholenumber(wholenumber_t n)
{
    return RexxNumberString::newInstanceFromWholenumber(n);
}

inline RexxNumberString *new_numberstringFromStringsize(stringsize_t n)
{
    return RexxNumberString::newInstanceFromStringsize(n);
}

inline RexxNumberString *new_numberstringFromInt64(int64_t n)
{
    return RexxNumberString::newInstanceFromInt64(n);
}

inline RexxNumberString *new_numberstringFromUint64(uint64_t n)
{
    return RexxNumberString::newInstanceFromUint64(n);
}

inline RexxNumberString *new_numberstringFromDouble(double n)
{
    return RexxNumberString::newInstanceFromDouble(n);
}

inline RexxNumberString *new_numberstringFromDouble(double n, size_t p)
{
    return RexxNumberString::newInstanceFromDouble(n, p);
}

inline RexxNumberString *new_numberstringFromFloat(float n)
{
    return RexxNumberString::newInstanceFromFloat(n);
}

#endif
