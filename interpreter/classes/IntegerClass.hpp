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

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope*);
    virtual HashCode getHashValue();

    virtual bool numberValue(wholenumber_t &result, wholenumber_t precision);
    virtual bool numberValue(wholenumber_t &result);
    virtual bool unsignedNumberValue(size_t &result, wholenumber_t precision);
    virtual bool unsignedNumberValue(size_t &result);
    virtual bool doubleValue(double &result);
    virtual void processUnknown(RexxString *, RexxObject **, size_t, ProtectedObject &);
    virtual wholenumber_t compareTo(RexxInternalObject *);

    virtual NumberString *numberString();
    virtual RexxInteger *integerValue(wholenumber_t);
    virtual RexxString  *makeString();
    virtual void         copyIntoTail(CompoundVariableTail *);
    virtual bool         hasMethod(RexxString *);
    virtual RexxString  *primitiveMakeString();
    virtual RexxString  *stringValue();
    virtual ArrayClass  *makeArray();
    virtual bool truthValue(int);
    virtual bool logicalValue(logical_t &);
    virtual bool isInstanceOf(RexxClass *);
    virtual MethodClass *instanceMethod(RexxString *);
    virtual SupplierClass *instanceMethods(RexxClass *);

    virtual bool  isEqual(RexxInternalObject *);
    wholenumber_t strictComp(RexxObject *);
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
    RexxObject *power(RexxObject *);
    RexxObject *notOp();
    RexxObject *operatorNot(RexxObject *);
    RexxObject *andOp(RexxObject *);
    RexxObject *orOp(RexxObject *);
    RexxObject *xorOp(RexxObject *);

    RexxObject *abs();
    RexxObject *sign();
    RexxObject *Max(RexxObject **, size_t);
    RexxObject *Min(RexxObject **, size_t);
    RexxObject *trunc(RexxObject *);
    RexxObject *floor();
    RexxObject *ceiling();
    RexxObject *round();
    RexxObject *format(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
    RexxObject *d2c(RexxObject *);
    RexxObject *d2x(RexxObject *);
    virtual RexxObject *evaluate(RexxActivation *, ExpressionStack *);
    virtual RexxObject *getValue(RexxActivation *);
    virtual RexxObject *getValue(VariableDictionary *);
    virtual RexxObject *getRealValue(RexxActivation *);
    virtual RexxObject *getRealValue(VariableDictionary *);
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
    static const int IntegerCacheSize = 100;

    inline RexxInteger *newCache(wholenumber_t value)
    {
        if (value >= IntegerCacheLow && value < IntegerCacheSize)
        {
            return integercache[value - IntegerCacheLow];
        }
        else
        {
            return new RexxInteger (value);
        }
    }

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);

    void initCache();

    // array of fast aloocation integers -10 to 90
    RexxInteger *integercache[IntegerCacheSize - IntegerCacheLow];
};


inline RexxInteger *new_integer(wholenumber_t v) { return TheIntegerClass->newCache(v); }
#endif
