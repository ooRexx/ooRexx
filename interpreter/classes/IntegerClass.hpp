/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2017 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                           IntegerClass.hpp     */
/*                                                                            */
/* Primitive Integer Class Definitions                                        */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInteger
#define Included_RexxInteger

#include "ClassClass.hpp"

void integer_create ();

class RexxIntegerClass;

class RexxInteger : public RexxObject
{
 public:
    inline RexxInteger(RESTORETYPE restoreType) { ; };
    inline RexxInteger(wholenumber_t intValue) { value = intValue; };

    void *operator new(size_t);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope*) override;
    HashCode getHashValue() override;

    bool numberValue(wholenumber_t &result, wholenumber_t precision) override;
    bool numberValue(wholenumber_t &result) override;
    bool unsignedNumberValue(size_t &result, wholenumber_t precision) override;
    bool unsignedNumberValue(size_t &result) override;
    bool doubleValue(double &result) override;
    void processUnknown(RexxErrorCodes error, RexxString *, RexxObject **, size_t, ProtectedObject &) override;
    wholenumber_t compareTo(RexxInternalObject *) override;

    NumberString *numberString() override;
    RexxInteger *integerValue(wholenumber_t) override;
    RexxString  *makeString() override;
    void         copyIntoTail(CompoundVariableTail *) override;
    bool         hasMethod(RexxString *) override;
    RexxString  *primitiveMakeString() override;
    RexxString  *stringValue() override;
    ArrayClass  *makeArray() override;
    bool truthValue(RexxErrorCodes) override;
    bool logicalValue(logical_t &) override;
    bool isInstanceOf(RexxClass *) override;
    MethodClass *instanceMethod(RexxString *) override;
    SupplierClass *instanceMethods(RexxClass *) override;

    bool  isEqual(RexxInternalObject *) override;
    wholenumber_t strictComp(RexxObject *);
    bool          strictEquality(RexxObject *);
    wholenumber_t comp(RexxObject *other);

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

    RexxObject *plus(RexxInteger *);
    RexxObject *minus(RexxInteger *);
    RexxObject *multiply(RexxInteger *);
    RexxObject *divide(RexxInteger *);
    RexxObject *integerDivide(RexxInteger *);
    RexxObject *remainder(RexxInteger *);
    RexxObject *modulo(RexxInteger *);
    RexxObject *power(RexxObject *);
    RexxObject *notOp();
    RexxObject *operatorNot(RexxObject *);
    RexxObject *andOp(RexxObject *);
    RexxObject *orOp(RexxObject *);
    RexxObject *xorOp(RexxObject *);
    RexxObject *choiceRexx(RexxObject *, RexxObject *);

    RexxObject *abs();
    RexxObject *sign();
    RexxObject *Max(RexxObject **, size_t);
    RexxObject *Min(RexxObject **, size_t);
    RexxObject *trunc(RexxObject *);
    RexxObject *floor();
    RexxObject *ceiling();
    RexxObject *round();
    RexxObject *format(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
    RexxObject *d2c(RexxInteger *);
    RexxObject *d2x(RexxInteger *);

    RexxObject *evaluate(RexxActivation *, ExpressionStack *) override;
    RexxObject *getValue(RexxActivation *) override;
    RexxObject *getValue(VariableDictionary *) override;
    RexxObject *getRealValue(RexxActivation *) override;
    RexxObject *getRealValue(VariableDictionary *) override;

    RexxString *concat(RexxString *);
    RexxString *concatBlank(RexxString *);
    void        setString(RexxString *string);
    RexxClass  *classObject();

    inline wholenumber_t getValue() {return value;}
    inline wholenumber_t wholeNumber() {return value;}
    inline size_t stringSize() {return (size_t)value;}
    inline RexxString *getStringrep() {return stringrep;}

    static void createInstance();
    static PCPPM operatorMethods[];
    static RexxIntegerClass *classInstance;

    static RexxInteger *falseObject;
    static RexxInteger *trueObject;
    static RexxInteger *nullPointer;

    static RexxInteger *integerZero;
    static RexxInteger *integerOne;
    static RexxInteger *integerTwo;
    static RexxInteger *integerThree;
    static RexxInteger *integerFour;
    static RexxInteger *integerFive;
    static RexxInteger *integerSix;
    static RexxInteger *integerSeven;
    static RexxInteger *integerEight;
    static RexxInteger *integerNine;
    static RexxInteger *integerMinusOne;

protected:

    RexxString *stringrep;              /* integer string representation     */
    wholenumber_t value;                /* actual integer value              */

    static wholenumber_t validMaxWhole[];  // table of maximum values per digits setting
};

class RexxIntegerClass : public RexxClass
{
 public:
    RexxIntegerClass(RESTORETYPE restoreType) { ; };
    inline RexxIntegerClass(const char *id , RexxBehaviour *classBehaviour, RexxBehaviour *instanceBehaviour) :
        RexxClass(id, classBehaviour, instanceBehaviour) { }

    static const int IntegerCacheLow = -10;
    static const int IntegerCacheHigh = 100;

    inline RexxInteger *newCache(wholenumber_t value)
    {
        if (value >= IntegerCacheLow && value <= IntegerCacheHigh)
        {
            return integercache[value - IntegerCacheLow];
        }
        else
        {
            return new RexxInteger (value);
        }
    }

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void initCache();

    // array of fast allocation integers -10 to 100
    RexxInteger *integercache[IntegerCacheHigh - IntegerCacheLow + 1];
};


inline RexxInteger *new_integer(wholenumber_t v) { return TheIntegerClass->newCache(v); }
inline RexxInteger *new_integer(bool sign, const char *digits, wholenumber_t intDigits, wholenumber_t pad = 0)
{
    wholenumber_t v, i;
    v = digits[0];
    for (i = 1; i < intDigits; i++)
    {
        // add digits
        v = v * 10 + digits[i];
    }
    for (i = 1; i <= pad; i++)
    {
        // add zeros
        v  = v * 10;
    }
    // return value with correct sign
    return TheIntegerClass->newCache(sign ? -v : v);
}

inline wholenumber_t length_in_bits(wholenumber_t v)
{
    wholenumber_t r = 0;

    if (v < 0) v = -v;
#ifdef __REXX64__
    if (v & 0xFFFFFFFF00000000) { v >>= 32; r |= 32; }
#endif
    if (v &         0xFFFF0000) { v >>= 16; r |= 16; }
    if (v &             0xFF00) { v >>=  8; r |=  8; }
    if (v &               0xF0) { v >>=  4; r |=  4; }
    if (v &                0xC) { v >>=  2; r |=  2; }
    if (v &                0x2) { v >>=  1; r |=  1; }
    return r + 1;
}

// ignoring bases or powers < 3, this is the maximum base/power a RexxInteger can handle
#ifdef __REXX64__
    // 999999 ** 3 = 999997000002999999
    #define RexxIntegerMaxBase  999999
    // 3 ** 37 = 450283905890997363
    #define RexxIntegerMaxPower 37
#else
    // 999 ** 3 = 997002999
    #define RexxIntegerMaxBase  999
    // 3 ** 18 = 387420489
    #define RexxIntegerMaxPower 18
#endif


#endif
