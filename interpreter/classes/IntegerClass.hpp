/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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

#include "NumberStringClass.hpp"

void integer_create (void);
#define INTEGERCACHELOW  -10
#define INTEGERCACHESIZE 100
#define MAX_INTEGER_LENGTH 10

class RexxIntegerClass;

class RexxInteger : public RexxObject {
 public:
  inline RexxInteger(RESTORETYPE restoreType) { ; };
  inline RexxInteger(wholenumber_t intValue) { this->value = intValue; };
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  void *operator new(size_t);
  void live(size_t);
  void liveGeneral(int reason);
  void flatten(RexxEnvelope*);
  virtual HashCode getHashValue();

  bool         numberValue(wholenumber_t &result, size_t precision);
  bool         numberValue(wholenumber_t &result);
  bool         unsignedNumberValue(stringsize_t &result, size_t precision);
  bool         unsignedNumberValue(stringsize_t &result);
  bool         doubleValue(double &result);
  RexxNumberString *numberString();
  RexxInteger *integerValue(size_t);
  RexxString  *makeString();
  void         copyIntoTail(RexxCompoundTail *);
  RexxInteger *hasMethod(RexxString *);
  RexxString  *primitiveMakeString();
  RexxString  *stringValue();
  bool         truthValue(int);
  virtual bool logicalValue(logical_t &);
  bool         isInstanceOf(RexxClass *);
  RexxMethod   *instanceMethod(RexxString *);
  RexxSupplier *instanceMethods(RexxClass *);

  bool        isEqual(RexxObject *);
  wholenumber_t strictComp(RexxObject *);
  wholenumber_t comp(RexxObject *other);

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

  RexxObject *unknown(RexxString *, RexxArray *);
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
  RexxObject *format(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
  RexxObject *d2c(RexxObject *);
  RexxObject *d2x(RexxObject *);
  RexxObject *evaluate(RexxActivation *, RexxExpressionStack *);
  RexxObject *getValue(RexxActivation *);
  RexxObject *getValue(RexxVariableDictionary *);
  RexxObject *getRealValue(RexxActivation *);
  RexxObject *getRealValue(RexxVariableDictionary *);
  RexxString *concat(RexxString *);
  RexxString *concatBlank(RexxString *);
  void        setString(RexxString *string);
                                       /* numberstring operator forwarders  */
  koper (integer_operator_not)

  inline wholenumber_t getValue() {return this->value;}
  inline wholenumber_t wholeNumber() {return this->value;}
  inline stringsize_t stringSize() {return (stringsize_t)this->value;}
  inline wholenumber_t incrementValue() {return ++this->value;}
  inline wholenumber_t decrementValue() {return --this->value;}
  inline RexxString *getStringrep() {return this->stringrep;}

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

class RexxIntegerClass : public RexxClass {
 public:
  RexxIntegerClass(RESTORETYPE restoreType) { ; };
  void *operator new(size_t size, void *ptr) {return ptr;};
  void *operator new (size_t);
  void *operator new(size_t size, size_t size1, const char *className, RexxBehaviour *classBehave, RexxBehaviour *instance) { return new (size, className, classBehave, instance) RexxClass; }
  inline void operator delete(void *, void *ptr) { }
  inline void operator delete (void *) { }
  inline void operator delete(void *, size_t, const char *, RexxBehaviour *, RexxBehaviour *) { }

  RexxIntegerClass();
  inline RexxInteger *newCache(wholenumber_t value) {if (value >= INTEGERCACHELOW && value < INTEGERCACHESIZE)
                                       return this->integercache[value - INTEGERCACHELOW];
                                     else
                                       return new RexxInteger (value); };
  void live(size_t);
  void liveGeneral(int reason);
                                     /* array of fast aloocation integers 0-99      */
  RexxInteger *integercache[INTEGERCACHESIZE - INTEGERCACHELOW];
};


inline RexxInteger *new_integer(wholenumber_t v) { return TheIntegerClass->newCache(v); }
#endif
