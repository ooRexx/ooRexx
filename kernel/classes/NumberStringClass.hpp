/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                  NumberStringClass.hpp  */
/*                                                                            */
/* Primitive NumberString Class Definitions                                   */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxNumberString
#define Included_RexxNumberString

#define MAXNUM      999999999               /* maximum size of 9 digits int         */
#define MAXPOSNUM  4294967294               /* maximum size of a ULONG              */
#define MAXNEGNUM  2147483647               /* maximum size of a negative long      */
#define MAXPOSBASE  429496729               /* maximum size value ULONG before * 10 */
#define MAXNEGBASE  214748365               /* maximum size neg long before * 10    */
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

#define DEFAULTDIGITS  9                    /* Define the default digits setting.   */
#define DEFAULTFUZZ    0                    /* Define the default fuzz   setting.   */

#define NumFormScientific  0x00000001       /* Define Numeric form setting at Object*/
                                            /*  creation time.                      */
#define NumberRounded      0x00000010       /* Indicate the number was rounded once */
                                            /*  at NumDigits, avoid double rounding */

#define SetNumberStringZero()                                           \
      this->number[0] = '\0';               /* Make value a zero.*/     \
      this->length = 1;                     /* Length is 1       */     \
      this->sign = 0;                       /* Make sign Zero.   */     \
      this->exp = 0;                        /* exponent is zero. */

long number_create_integer(UCHAR *, long, int, int);


#define NumberStringRound(s,d) s->roundUp(s,d)
#define number_digits() current_settings->digits
#define number_fuzz()   current_settings->fuzz
#define number_form()   current_settings->form
#define number_fuzzydigits() (current_settings->digits - current_settings->fuzz)

 class RexxNumberStringBase : public RexxObject {
   public:
    inline RexxNumberStringBase() { ; }
    void   mathRound(UCHAR *);
    PUCHAR stripLeadingZeros(UCHAR *);
    PUCHAR adjustNumber(UCHAR *, UCHAR *, size_t, size_t);

    RexxString *stringObject;          /* converted string value          */
    short NumFlags;                    /* Flags for use by the Numberstring met*/
    short sign;                        /* sign for this number (-1 is neg)     */
    size_t  NumDigits;                 /* Maintain a copy of digits setting of */
                                       /* From when object was created         */
    long    exp;
    size_t  length;
 };

 class RexxNumberString : public RexxNumberStringBase {
   public:
    void         *operator new(size_t, size_t);
    inline void  *operator new(size_t size, void *ptr) {return ptr;};
    RexxNumberString(size_t) ;
    inline RexxNumberString(RESTORETYPE restoreType) { ; };
    ULONG       hash();
    void        live();
    void        liveGeneral();
    void        flatten(RexxEnvelope *);

    long        longValue(size_t);
    inline RexxNumberString *numberString() { return this; }
    double      doubleValue();
    RexxInteger *integerValue(size_t);
    RexxString  *makeString();
    RexxInteger *hasMethod(RexxString *);
    RexxString  *primitiveMakeString();
    RexxString  *stringValue();
    BOOL         truthValue(LONG);

    BOOL        isEqual(RexxObject *);
    long        strictComp(RexxObject *);
    long        comp(RexxObject *);
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

    int         ULong(ULONG *);
    RexxNumberString *clone();
    void        setString(RexxString *);
    void        roundUp(ULONG);
    RexxString *formatRexx(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
    RexxString *formatInternal(size_t, size_t, size_t, size_t, RexxNumberString *, size_t, long);
    RexxObject *operatorNot(RexxObject *);
    RexxObject *evaluate(RexxActivation *, RexxExpressionStack *);
    RexxObject *getValue(RexxActivation *context){return this;}
    RexxObject *getValue(RexxVariableDictionary *dictionary){return this;}
    RexxObject *trunc(RexxObject *);
    RexxObject *truncInternal(size_t);
    RexxObject *unknown(RexxString *, RexxArray *);
    bool        isInstanceOf(RexxClass *);
    RexxMethod   *instanceMethod(RexxString *);
    RexxSupplier *instanceMethods(RexxClass *);
    inline RexxNumberString *checkNumber(size_t digits, BOOL rounding)
    {
     if (this->length > digits)            /* is the length larger than digits()*/
                                           /* need to allocate a new number     */
       return this->prepareNumber(digits, rounding);
     return this;                          /* no adjustment required            */
    }

    RexxNumberString *prepareNumber(size_t, BOOL);
    void              adjustPrecision(UCHAR *, size_t);
    void              adjustPrecision();
    inline void       checkPrecision() { if (length > NumDigits) adjustPrecision(); }
    inline void       setNumericSettings(size_t digits, int form)
    {
        this->NumDigits = digits;
        if (form == FORM_SCIENTIFIC)
            this->NumFlags |= NumFormScientific;
        else
            this->NumFlags &= ~NumFormScientific;
    }

    inline void       setupNumber() {
                                       /* current global settings           */
        extern ACTIVATION_SETTINGS *current_settings;
        size_t digits = number_digits();
        /* inherit the current numeric settings */
        setNumericSettings(digits, number_form());
        /* check for any required rounding */
        checkPrecision();
    }
    bool  createUnsignedInt64Value(stringchar_t *thisnum, stringsize_t intlength, int carry, wholenumber_t exponent, uint64_t maxValue, uint64_t &result);
    bool  checkIntegerDigits(stringsize_t numDigits, stringsize_t &numberLength, wholenumber_t &numberExponent, bool &carry);
    bool  int64Value(int64_t *result, stringsize_t numDigits);
    void  formatInt64(int64_t integer);

    RexxNumberString *addSub(RexxNumberString *, UINT, size_t);
    RexxNumberString *plus(RexxObject *);
    RexxNumberString *minus(RexxObject *);
    RexxNumberString *multiply(RexxObject *);
    RexxNumberString *divide(RexxObject *);
    RexxNumberString *integerDivide(RexxObject *);
    RexxNumberString *remainder(RexxObject *);
    RexxNumberString *power(RexxObject *);
    RexxNumberString *Multiply(RexxNumberString *);
    RexxNumberString *Division(RexxNumberString *, UINT);
    RexxNumberString *abs();
    RexxInteger *Sign();
    RexxObject  *notOp();
    RexxNumberString *Max(RexxObject **, size_t);
    RexxNumberString *Min(RexxObject **, size_t);
    RexxNumberString *maxMin(RexxObject **, size_t, UINT);
    RexxObject *isInteger();
    RexxString *d2c(RexxObject *);
    RexxString *d2x(RexxObject *);
    RexxString *d2xD2c(RexxObject *, BOOL);
    RexxString *concat(RexxObject *);
    RexxString *concatBlank(RexxObject *);
    RexxObject *orOp(RexxObject *);
    RexxObject *andOp(RexxObject *);
    RexxObject *xorOp(RexxObject *);
    void        formatLong(long);
    void        formatULong(ULONG);
    long        format(PCHAR, size_t);
    inline void        setZero() {
                   this->number[0] = '\0';               /* Make value a zero.*/
                   this->length = 1;                     /* Length is 1       */
                   this->sign = 0;                       /* Make sign Zero.   */
                   this->exp = 0;                        /* exponent is zero. */
                }
    UCHAR number[4];
 };

class RexxNumberStringClass : public RexxClass {
 public:
    static RexxNumberString *newInstance(double);
    static RexxNumberString *newInstance(float);
    static RexxNumberString *newInstance(wholenumber_t);
    static RexxNumberString *newInstance(int64_t);
    static RexxNumberString *newInstance(stringsize_t);
    static RexxNumberString *newInstance(char *, stringsize_t);

};

void AdjustPrecision(RexxNumberString *, UCHAR *, int);

inline RexxNumberString *new_numberstring(char *s, stringsize_t l)
{
    return RexxNumberStringClass::newInstance(s, l);
}

inline RexxNumberString *new_numberstring(wholenumber_t n)
{
    return RexxNumberStringClass::newInstance(n);
}

inline RexxNumberString *new_numberstring(stringsize_t n)
{
    return RexxNumberStringClass::newInstance(n);
}

inline RexxNumberString *new_numberstring(int64_t n)
{
    return RexxNumberStringClass::newInstance(n);
}

inline RexxNumberString *new_numberstring(double n)
{
    return RexxNumberStringClass::newInstance(n);
}

inline RexxNumberString *new_numberstring(float n)
{
    return RexxNumberStringClass::newInstance((double)n);
}

#endif
