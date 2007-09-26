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
/* REXX Kernel                                                  IntegerClass.c       */
/*                                                                            */
/* Primitive Integer Class                                                    */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "RexxBuiltinFunctions.h"

extern long validMaxWhole[];
                                       /* current global settings           */
extern ACTIVATION_SETTINGS *current_settings;
                                       /* define an operator forwarding     */
                                       /* method                            */

#define string_forwarder_cpp(method)\
RexxObject *RexxInteger::##method(RexxObject *operand)\
 {\
     return (RexxObject *)this->string()->method(operand);\
 }

ULONG RexxInteger::hash()
/******************************************************************************/
/* Function:  retrieve the hash value of an integer object                    */
/******************************************************************************/
{
  RexxString * string;                 /* integer string value              */

  if (!OTYPE(Integer, this))           /*  a nonprimitive object?           */
                                       /* see if == overridden.             */
    return this->sendMessage(OREF_STRICT_EQUAL)->requestString()->hash();
  else {
    if (this->hashvalue == 0) {        /* no hash generated yet?            */
      string = this->stringValue();    /* generate the string value         */
                                       /* get the string's hash value       */
      this->hashvalue = string->getHashValue();
    }
    return HASHVALUE(this);            /* return the string hash            */
  }
}

void RexxInteger::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->objectVariables);
  memory_mark(this->stringrep);
  cleanUpMemoryMark
}

void RexxInteger::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->objectVariables);
  memory_mark_general(this->stringrep);
  cleanUpMemoryMarkGeneral
}

void RexxInteger::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInteger)

   flatten_reference(newThis->objectVariables, envelope);
   flatten_reference(newThis->stringrep, envelope);

  cleanUpFlatten
}

RexxString *RexxInteger::makeString()
/******************************************************************************/
/* Function:  Handle a REQUEST('STRING') request for a REXX integer object    */
/******************************************************************************/
{
  return this->stringValue();          /* return the string value           */
}

RexxInteger *RexxInteger::hasMethod(RexxString *methodName)
/******************************************************************************/
/* Function:  Handle a HASMETHOD request for an integer                       */
/******************************************************************************/
{
                                       /* return the string value's answer  */
  return this->stringValue()->hasMethod(methodName);
}

RexxString *RexxInteger::primitiveMakeString()
/******************************************************************************/
/* Function:  Handle a REQUEST('STRING') request for a REXX integer object    */
/******************************************************************************/
{
  char        stringBuffer[11];        /* integer formatting buffer         */
  RexxString *string;                  /* string value                      */

  if (this->stringrep != OREF_NULL)    /* have a string already?            */
    return this->stringrep;            /* return it directly                */
                                       /* convert value into string         */
  _ltoa((long)this->value, stringBuffer, 10);
                                       /* return as a string                */
  string = new_string(stringBuffer, strlen(stringBuffer));
                                       /* cache this away for later         */
  OrefSet(this, this->stringrep, string);
  SetObjectHasReferences(this);        /* we now have references            */
  return string;                       /* return the new string             */
}

RexxString *RexxInteger::stringValue()
/******************************************************************************/
/* Function:  Return the string value for an integer                          */
/******************************************************************************/
{
  char        stringBuffer[11];        /* integer formatting buffer         */
  RexxString *string;                  /* string value                      */

  if (this->stringrep != OREF_NULL)    /* have a string already?            */
    return this->stringrep;            /* return it directly                */
                                       /* convert value into string         */
  _ltoa((long)this->value, stringBuffer, 10);
                                       /* return as a string                */
  string = new_string(stringBuffer, strlen(stringBuffer));
                                       /* cache this away for later         */
  OrefSet(this, this->stringrep, string);
  SetObjectHasReferences(this);        /* we now have references            */
  return string;                       /* return the new string             */
}

void RexxInteger::copyIntoTail(RexxCompoundTail *tail)
/******************************************************************************/
/* Function:  Copy the value of an integer into a compound variable name      */
/******************************************************************************/
{
  char        stringBuffer[11];        /* integer formatting buffer         */

  if (this->stringrep != OREF_NULL) {  /* have a string already?            */
      /* copying directly from an existing string rep is faster */
      /* than formatting a new value and copying. */
      tail->append((PUCHAR)stringrep->stringData, stringrep->length);
      return;
  }
  _ltoa((long)this->value, stringBuffer, 10);
                                       /* append this to the buffer         */
  tail->append((UCHAR *)stringBuffer, strlen(stringBuffer));
}

double  RexxInteger::doubleValue()
/******************************************************************************/
/* Function:  Convert a string value to a double value;                       */
/******************************************************************************/
{
  return (double)this->value;          /* just let the compiler convert     */
}

RexxNumberString *RexxInteger::numberString()
/******************************************************************************/
/* Function:  Convert an integer into a numberstring value                    */
/******************************************************************************/
{
  if (this->stringrep != OREF_NULL)    /* have a cached string value?       */
                                       /* use its numberstring value        */
    return this->stringrep->numberString();
  else                                 /* create a new numberstring         */
    return (RexxNumberString *)new_numberstring((wholenumber_t)this->value);
}

long RexxInteger::longValue(
    size_t digits)                     /* digits of precision to use        */
/******************************************************************************/
/* Function:  Convert an integer object to a long value under the current     */
/*            precision.                                                      */
/******************************************************************************/
{
  if (digits == NO_LONG) {             /* Do we need to get current digits  */
    digits = number_digits();          /*  Yes, go get it.                  */
    digits = min(digits, 9);           /* 9 is max for default digits.      */
  }
                                       /* is the long value expressable as a*/
                                       /*  whole number in REXX term.       */
  if (digits < 9 && labs((INT)this->value) >= validMaxWhole[digits - 1]) {
      return NO_LONG;                  /* nope, not a valid long.           */
  }
  return this->value;                  /* return the value directly         */
}

RexxInteger *RexxInteger::integerValue(
    size_t digits)                     /* required precision (ignored)      */
/******************************************************************************/
/* Function:  Convert an integer to an integer (real easy!)                   */
/******************************************************************************/
{
  return this;                         /* just return directly              */
}

void RexxInteger::setString(
    RexxString *string )               /* new string value                  */
/******************************************************************************/
/* Function:  Add a string value to the string look-a-side.                   */
/******************************************************************************/
{
                                       /* set the strign                    */
   OrefSet(this, this->stringrep, string);
   SetObjectHasReferences(this);       /* we now have references            */
}

BOOL RexxInteger::truthValue(
    LONG  errorcode )                  /* error to raise if not good        */
/******************************************************************************/
/* Function:  Determine the truth value of an integer object                  */
/******************************************************************************/
{
  if (this->value == 0L)               /* have a zero?                      */
    return FALSE;                      /* this is false                     */
  else if (this->value != 1L)          /* how about a one?                  */
    report_exception1(errorcode, this);/* report the error                  */
  return TRUE;                         /* this is true                      */
}

/******************************************************************************/
/* Function:   Macro to forward a method against the numberstring value of    */
/*             an integer object.                                             */
/******************************************************************************/

#define integer_forward(s,m,o) ((s)->numberString()->m(o))

RexxObject *RexxInteger::unknown(
    RexxString *msgname,               /* unknown message name              */
    RexxArray *arguments)              /* arguments to the unknown message  */
/******************************************************************************/
/* Function:  Intercept unknown messages to an integer object and reissue     */
/*            them against the string value.                                  */
/******************************************************************************/
{
                                       /* just reissue this                 */
  return this->stringValue()->sendMessage(msgname, arguments);
}


/**
 * Override for the normal isinstanceof method.  This version
 * allows the IntegerClass to "lie" about being a string.
 *
 * @param other  The comparison class
 *
 * @return True if the string value is an instance of the target class.
 */
bool RexxInteger::isInstanceOf(RexxClass *other)
{
    return stringValue()->isInstanceOf(other);
}


/**
 * Retrieve the method instance for an object's defined method.
 *
 * @param method_name
 *               The method name.
 *
 * @return The method object that implements the object method.
 */
RexxMethod *RexxInteger::instanceMethod(RexxString  *method_name)
{
    return stringValue()->instanceMethod(method_name);
}


/**
 * Return a supplier containing the methods implemented by an
 * object.  Depending on the argument, this is either A) all of
 * the methods, B) just the explicitly set instance methods, or
 * C) the methods provided by a given class.
 *
 * @param class_object
 *               The target class object (optional).
 *
 * @return A supplier with the appropriate method set.
 */
RexxSupplier *RexxInteger::instanceMethods(RexxClass *class_object)
{
    return stringValue()->instanceMethods(class_object);
}


RexxString *RexxInteger::concatBlank(
    RexxString *other )                /* other object for concatenation    */
/******************************************************************************/
/* Function:  Concatenate an object to an integer                             */
/******************************************************************************/
{
  required_arg(other, ONE);            /* this is required                  */
  other = REQUEST_STRING(other);       /* ensure a string value             */
                                       /* do the concatenate                */
  return this->stringValue()->concatWith(other, ' ');
}

RexxString *RexxInteger::concat(
    RexxString *other )                /* other object for concatenation    */
/******************************************************************************/
/* Function:  Concatenate an object to an integer                             */
/******************************************************************************/
{
  required_arg(other, ONE);            /* this is required                  */
  other = REQUEST_STRING(other);       /* ensure a string value             */
                                       /* do the concatenate                */
  return this->stringValue()->concat(other);
}

RexxObject *RexxInteger::plus(
    RexxInteger *other)                /* target other object               */
/******************************************************************************/
/* Function:  Add an integer to another object                                */
/******************************************************************************/
{
  long tempVal;                        /* addition result                   */

                                       /* are we using default digits?      */
  if (number_digits() != DEFAULT_DIGITS )
                                       /* nope, we can't do integer arith   */
    return integer_forward(this, plus, other);
  if (other == OREF_NULL)              /* unary                             */
    return this;                       /* just return ourselves             */
  else {                               /* binary                            */
    if (OTYPE(Integer, other)) {       /* adding two integers together?     */
                                       /* add the numbers                   */
      tempVal = this->value + other->value;
                                       /* result still within range?        */
      if (tempVal <= 999999999 && tempVal >= -999999999)
        return new_integer(tempVal);   /* return as an integer number       */
    }
                                       /* need to do full arithmetic        */
    return integer_forward(this, plus, other);
  }
}

RexxObject *RexxInteger::minus(
    RexxInteger *other)                /* target other object               */
/******************************************************************************/
/* Function:  Subtract another object from an integer                         */
/******************************************************************************/
{
  long tempVal;                        /* subtraction result                */

                                       /* are we using default digits?      */
  if (number_digits() != DEFAULT_DIGITS )
                                       /* nope, then we can do integer arith*/
    return integer_forward(this, minus, other);

  if (other == OREF_NULL) {            /* unary subtraction operator        */
    tempVal = -this->value;            /* negate the current number         */
    return new_integer(tempVal);       /* and return a new integer          */
  }
  else {                               /* binary subtraction operation      */
    if (OTYPE(Integer, other)) {       /* subtracting two integers?         */
                                       /* subtract the numbers              */
      tempVal = this->value - other->value;
                                       /* result still within range?        */
      if (tempVal <= 999999999 && tempVal >= -999999999)
        return new_integer(tempVal);   /* return as an integer number       */
    }
                                       /* need to do full arithmetic        */
    return integer_forward(this, minus, other);
  }
}

RexxObject *RexxInteger::multiply(
    RexxInteger *other)                /* target other object               */
/******************************************************************************/
/* Function:  Multiply an integer by another object                           */
/******************************************************************************/
{
  long otherval;                       /* long value of other object        */
  long tempVal;                        /* temp result value                 */

                                       /* are we using default digits?      */
  if (number_digits() != DEFAULT_DIGITS )
                                       /* nope, we can't do integer math    */
    return integer_forward(this, multiply, other);
  required_arg(other, ONE);            /* make sure the argument is there   */
                                       /* is the other an integer and will  */
                                       /* the result be in a good range?    */
  if (OTYPE(Integer, other) && labs((INT)this->value) <= 99999 &&
      labs((INT)(otherval = other->value)) <= 9999) {
    tempVal = this->value * otherval;  /* multiply directly                 */
    return new_integer(tempVal);       /* and return as an integer          */
  }
  else                                 /* do the slow way                   */
    return integer_forward(this, multiply, other);
}

RexxObject *RexxInteger::divide(
    RexxInteger *other)                /* target other object               */
/******************************************************************************/
/* Function:  Perform division (actually done as numberstring math)           */
/******************************************************************************/
{
                                       /* just forward this                 */
  return integer_forward(this, divide, other);
}

RexxObject *RexxInteger::integerDivide(
    RexxInteger *other)                /* target other object               */
/******************************************************************************/
/* Function:  Perform integer division                                        */
/******************************************************************************/
{
  long otherval;                       /* long value of the other object    */
  long tempVal;                        /* temporary result value            */

                                       /* are we using default digits?      */
  if (number_digits() != DEFAULT_DIGITS )
                                       /* nope, we can't do integer arith   */
    return integer_forward(this, integerDivide, other);
  required_arg(other, ONE);            /* make sure this is really there    */

  if (OTYPE(Integer, other)) {         /* is right object an integer?       */
                                       /* is right number 0?                */
    if ((otherval = other->value) != 0) {
      tempVal = this->value / otherval;/* nope, do the division....         */
      return new_integer(tempVal);     /* and return as an integer object   */
    }
    else                               /* yes, raise error.                 */
      report_exception(Error_Overflow_zero);
  }
                                       /* not integer, forward to           */
                                       /*numberstring.                      */
  return integer_forward(this, integerDivide, other);
}

RexxObject *RexxInteger::remainder(
    RexxInteger *other)                /* target other object               */
/******************************************************************************/
/* Function:  Perform remainder division                                      */
/******************************************************************************/
{
  long otherval;                       /* long value of the other object    */
  long tempVal;                        /* temporary result value            */

                                       /* are we using default digits?      */
  if (number_digits() != DEFAULT_DIGITS )
                                       /* nope, we can't do integer arith   */
    return integer_forward(this, remainder, other);
  required_arg(other, ONE);            /* make sure this is really there    */

  if (OTYPE(Integer, other)) {         /* is right object an integer?       */
                                       /* is right number 0?                */
    if ((otherval = other->value) != 0) {
      tempVal = this->value % otherval;/* nope, do the division....         */
      return new_integer(tempVal);     /* and return as an integer object   */
    }
    else                               /* yes, raise error.                 */
      report_exception(Error_Overflow_zero);
  }
                                       /* not integer, forward to           */
                                       /*numberstring.                      */
  return integer_forward(this, remainder, other);
}

RexxObject *RexxInteger::power(
    RexxObject *other)                 /* power exponent value              */
/******************************************************************************/
/* Function:  Process integer power operator (actually just a forwarder)      */
/******************************************************************************/
{
                                       /* just send along                   */
  return integer_forward(this, power, other);
}

BOOL RexxInteger::isEqual(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Primitive strict equal\not equal method.  This determines       */
/*            only strict equality, not greater or less than values.          */
/******************************************************************************/
{
  if (!isPrimitive(this))              /* not a primitive?                  */
                                       /* do the full lookup compare        */
    return this->sendMessage(OREF_STRICT_EQUAL, other)->truthValue(Error_Logical_value_method);

  if (OTYPE(Integer, other))           /* two integers?                     */
                                       /* just directly compare the values  */
    return this->value == ((RexxInteger *)other)->value;
                                       /* go do a string compare            */
  return this->stringValue()->isEqual(other);
}

long RexxInteger::strictComp(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  Compare the two values.                                         */
/*                                                                            */
/*  Returned:  return <0 if other is greater than this                        */
/*             return  0 if this equals other                                 */
/*             return >0 if this is greater than other                        */
/******************************************************************************/
{
  required_arg(other, ONE);            /* make sure this is really there    */
  if (OTYPE(Integer, other))           /* string compare is simple          */
                                       /* just return their difference      */
    return this->value - ((RexxInteger *)other)->value;
  else                                 /* go do a string compare            */
    return this->stringValue()->strictComp((RexxString *)other);
}


/**
 * Exported version of the HASHCODE method for retrieving the
 * object's hash.
 *
 * @return A string version of the hash (generally holds binary characters).
 */
RexxObject *RexxInteger::hashCode()
{
    // get the hash value, which is actually derived from the integer string value
    unsigned long hash = this->hash();
    return new_string((char *)&hash, sizeof(unsigned long));
}


RexxInteger *RexxInteger::strictEqual(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  Perform the primitive level "==" compare, including the hash    */
/*            value processing.                                               */
/******************************************************************************/
{
    return this->strictComp(other) == 0 ? TheTrueObject : TheFalseObject;
}


RexxInteger *RexxInteger::strictNotEqual(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  Strict inequality operation                                     */
/******************************************************************************/
{
                                       /* return strict compare result      */
  return (this->strictComp(other) != 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxInteger::equal(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  non-strict "=" operator                                         */
/******************************************************************************/
{
  return this->comp(other) == 0 ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxInteger::notEqual(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  non-strict "\=" operator                                        */
/******************************************************************************/
{
  return this->comp(other) != 0 ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxInteger::isGreaterThan(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  non-strict ">" operator                                         */
/******************************************************************************/
{
  return this->comp(other) > 0 ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxInteger::isLessThan(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  non-strict "<" operator                                         */
/******************************************************************************/
{
  return this->comp(other) < 0 ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxInteger::isGreaterOrEqual(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  non-strict ">=" operator                                        */
/******************************************************************************/
{
  return this->comp(other) >= 0 ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxInteger::isLessOrEqual(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  non-strict "<=" operator                                        */
/******************************************************************************/
{
  return this->comp(other) <= 0 ? TheTrueObject : TheFalseObject;
}


RexxInteger *RexxInteger::strictGreaterThan(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  strict ">>" operator                                            */
/******************************************************************************/
{
  return this->strictComp(other) > 0 ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxInteger::strictLessThan(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  strict "<<" operator                                            */
/******************************************************************************/
{
  return this->strictComp(other) < 0 ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxInteger::strictGreaterOrEqual(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  strict ">>=" operator                                           */
/******************************************************************************/
{
  return this->strictComp(other) >= 0 ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxInteger::strictLessOrEqual(
    RexxObject *other)                 /* other comparison value            */
/******************************************************************************/
/* Function:  strict "<<=" operator                                           */
/******************************************************************************/
{
  return this->strictComp(other) <= 0 ? TheTrueObject : TheFalseObject;
}

RexxObject *RexxInteger::notOp()
/******************************************************************************/
/* Function:  Perform the logical not of an integer object                    */
/******************************************************************************/
{
                                       /* perform the operation             */
  return this->truthValue(Error_Logical_value_method) ? TheFalseObject : TheTrueObject;
}

RexxObject *RexxInteger::operatorNot(
    RexxObject *dummy )                /* dummy for polymorphic operators   */
/******************************************************************************/
/* Function:  Perform the logical not of an integer object                    */
/******************************************************************************/
{
                                       /* perform the operation             */
  return this->truthValue(Error_Logical_value_method) ? TheFalseObject : TheTrueObject;
}

RexxObject *RexxInteger::andOp(
    RexxObject *other)                 /* other logical value               */
/******************************************************************************/
/* Function:  Logically AND two objects together                              */
/******************************************************************************/
{
  RexxObject *otherTruth;              /* truth value of the other object   */

  required_arg(other, ONE);            /* make sure the argument is there   */
                                       /* validate the boolean              */
  otherTruth = other->truthValue(Error_Logical_value_method) ? TheTrueObject : TheFalseObject;
                                       /* perform the operation             */
  return (!this->truthValue(Error_Logical_value_method)) ? TheFalseObject : otherTruth;
}

RexxObject *RexxInteger::orOp(
    RexxObject *other)                 /* other logical value               */
/******************************************************************************/
/* Function:  Logically OR two objects together                               */
/******************************************************************************/
{
  RexxObject *otherTruth;              /* truth value of the other object   */

  required_arg(other, ONE);            /* make sure the argument is there   */
                                       /* validate the boolean              */
  otherTruth = other->truthValue(Error_Logical_value_method) ? TheTrueObject : TheFalseObject;
                                       /* perform the operation             */
  return (this->truthValue(Error_Logical_value_method)) ? TheTrueObject : otherTruth;
}

RexxObject *RexxInteger::xorOp(
    RexxObject *other)                 /* other logical value               */
/******************************************************************************/
/* Function:  Logically XOR two objects together                              */
/******************************************************************************/
{
  BOOL truth;                          /* converted other argument          */

  required_arg(other, ONE);            /* make sure the argument is there   */
                                       /* get as a boolean                  */
  truth = other->truthValue(Error_Logical_value_method);
                                       /* first one false?                  */
  if (!this->truthValue(Error_Logical_value_method))
                                       /* value is always the second        */
    return truth ? TheTrueObject : TheFalseObject;
  else                                 /* value is inverse of second        */
    return (truth) ? TheFalseObject : TheTrueObject;
}

RexxObject *RexxInteger::abs()
/******************************************************************************/
/* Function:  Take the absolute value of an integer object                    */
/******************************************************************************/
{
                                       /* working under the default digits? */
 if (number_digits() == DEFAULT_DIGITS) {
     /* if we're already positive, this is a quick return */
     if (value >= 0) {
         return this;
     }
     return new_integer(-value);       /* return as an integer object       */
 }
 else
   return this->numberString()->abs(); /* do a numberstring operation       */
}

RexxObject *RexxInteger::sign()
/******************************************************************************/
/* Function:  SIGN() function on an integer object                            */
/******************************************************************************/
{
  RexxObject *result;                  /* returned result                   */

 if (this->value > 0L)                 /* positive number?                  */
   result = IntegerOne;                /* result is "1"                     */
 else if (this->value < 0L)            /* negative number?                  */
  result = new_integer(-1);            /* result is "-1"                    */
 else
  result = IntegerZero;                /* exactly zero                      */
 return result;                        /* return the value                  */
}

RexxObject *RexxInteger::Max(
    RexxObject **args,                 /* array of comparison values        */
    size_t argCount)                   /* count of arguments                */
/******************************************************************************/
/* Function:  Perform MAX function on integer objects                         */
/******************************************************************************/
{
  long         value;                  /* current working value             */
  long         maxvalue;               /* current maximum                   */
  size_t       arg;                    /* current arg position              */
  RexxObject * argument;               /* current argument object           */
  BOOL         AllIntegers;            /* have all integers                 */


                                       /* are we using default digits?      */
  if (number_digits() != DEFAULT_DIGITS )
                                       /* nope, we can't do integer max.    */
   return this->numberString()->Max(args, argCount);

  if (argCount < 1)                    /* no comparisons to do?             */
    return (RexxObject *)this;         /* just return this as the result    */

  maxvalue = this->value;              /* assume first number is our max.   */

  AllIntegers = TRUE;                  /* assume all numbers are integers.. */
                                       /* check each numbers to see if      */
                                       /* larger than the max.              */
  for (arg = 0; arg < argCount && AllIntegers; arg++) {
    argument = args[arg];              /* get next argument element         */

    if (argument == OREF_NULL)         /* was argument missging ?           */
                                       /* Yes, report the error.            */
      report_exception1(Error_Incorrect_method_noarg, new_integer(arg));

    if (OTYPE(Integer, argument)) {    /* is this an INTEGER object?        */
                                       /* yes, gets its value.              */
      value = ((RexxInteger *)argument)->value;
      if (value > maxvalue)            /* is this number larger than max?   */
        maxvalue = value;              /* yes, it is our new max.           */
    }
    else {                             /* not an integer, compare isn't     */
      AllIntegers = FALSE;             /* so simple.                        */
      break;
    }
  }

  if (AllIntegers) {                   /* were all the objects integers?    */
    return new_integer(maxvalue);      /* yes, then max is our max number   */
  }
  else {
                                       /* not all integers, convert into a  */
                                       /* NumberString, and let NumberString*/
                                       /* figure this out.                  */
    return this->numberString()->Max(args, argCount);
  }
}

RexxObject *RexxInteger::Min(
    RexxObject **args,                 /* array of comparison values        */
    size_t argCount)                   /* count of arguments                */
/******************************************************************************/
/* Function:  Perform MAX function on integer objects                         */
/******************************************************************************/
{
  long         value;                  /* current working value             */
  long         minvalue;               /* current minimum                   */
  size_t       arg;                    /* current arg position              */
  RexxObject * argument;               /* current argument object           */
  BOOL         AllIntegers;            /* have all integers                 */


                                       /* are we using default digits?      */
  if (number_digits() != DEFAULT_DIGITS )
                                       /* nope, we can't do integer max.    */
   return this->numberString()->Min(args, argCount);

  if (argCount < 1)                    /* no comparisons to do?             */
    return (RexxObject *)this;         /* just return this as the result    */

  minvalue = this->value;              /* assume first number is our min.   */

  AllIntegers = TRUE;                  /* assume all numbers are integers.. */
                                       /* check each numbers to see if      */
                                       /* larger than the max.              */
  for (arg = 0; arg < argCount && AllIntegers; arg++) {
    argument = args[arg];              /* get next argument element         */

    if (argument == OREF_NULL)         /* was argument missging ?           */
                                       /* Yes, report the error.            */
      report_exception1(Error_Incorrect_method_noarg, new_integer(arg));

    if (OTYPE(Integer, argument)) {    /* is this an INTEGER object?        */
                                       /* yes, gets its value.              */
      value = ((RexxInteger *)argument)->value;
      if (value < minvalue)            /* is this number larger than min?   */
        minvalue = value;              /* yes, it is our new max.           */
    }
    else {                             /* not an integer, compare isn't     */
      AllIntegers = FALSE;             /* so simple.                        */
      break;
    }
  }

  if (AllIntegers) {                   /* were all the objects integers?    */
    return new_integer(minvalue);      /* yes, then max is our max number   */
  }
  else {
                                       /* not all integers, convert into a  */
                                       /* NumberString, and let NumberString*/
                                       /* figure this out.                  */
    return this->numberString()->Min(args, argCount);
  }
}

RexxObject *RexxInteger::trunc(
    RexxObject *decimals)              /* number of decimal digits requested*/
/******************************************************************************/
/* Function:  Act as a front end for the actual TRUNC REXX method             */
/******************************************************************************/
{
                                       /* just forward to numberstring      */
  return this->numberString()->trunc(decimals);
}

RexxObject *RexxInteger::format(
  RexxObject *Integers,                /* space for integer part            */
  RexxObject *Decimals,                /* number of decimals required       */
  RexxObject *MathExp,                 /* the exponent size                 */
  RexxObject *ExpTrigger )             /* the exponent trigger              */
/******************************************************************************/
/* Function:  Act as a front end for the actual FORMAT REXX method            */
/******************************************************************************/
{
  return this->numberString()->formatRexx(Integers, Decimals, MathExp, ExpTrigger);
}

RexxObject *RexxInteger::d2c(
    RexxObject *length)                /* length of result                  */
/******************************************************************************/
/* Function:  Convert a decimal number string into a character string         */
/******************************************************************************/
{
                                       /* format as a string value          */
  return this->numberString()->d2xD2c(length, TRUE);
}

RexxObject *RexxInteger::d2x(
    RexxObject *length)                /* length of result                  */
/******************************************************************************/
/* Function:  Convert a decimal number string into a hexadecimal string       */
/******************************************************************************/
{
                                       /* format as a string value          */
  return this->numberString()->d2xD2c(length, FALSE);
}

RexxObject  *RexxInteger::evaluate(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/******************************************************************************/
/* Function:  Polymorphic method that makes integer a polymorphic expression  */
/*            term for literals                                               */
/******************************************************************************/
{
  stack->push(this);                   /* place on the evaluation stack     */
                                       /* trace if necessary                */
  context->traceIntermediate(this, TRACE_PREFIX_LITERAL);
  return this;                         /* also return the result            */
}


RexxObject  *RexxInteger::getValue(
    RexxActivation *context)           /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
  return (RexxObject *)this;           /* just return this value            */
}


RexxObject  *RexxInteger::getValue(
    RexxVariableDictionary *context)   /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
  return (RexxObject *)this;           /* just return this value            */
}

/* **************************************** */
/*  Integer class methods begin here .....  */
/* **************************************** */

RexxIntegerClass::RexxIntegerClass()
/******************************************************************************/
/* This method will pre-allocate 100 integer objects, 0-99.  These will then  */
/*  be used when ever a request for an integer between 0 and 99 is requested  */
/*  this should help reduce some of our memory requirements and trips through */
/*  memory_new.                                                               */
/******************************************************************************/
{
 long i;                               /* loop counter                      */

 for (i=INTEGERCACHELOW; i<INTEGERCACHESIZE; i++ ) {  /* now create all our cached integers*/
   OrefSet(this, this->integercache[i - INTEGERCACHELOW], new  RexxInteger (i));
   /* force the item to create its string value too.  This can save */
   /* us a lot of time when string indices are used for compound */
   /* variables and also eliminate a bunch of old-new table */
   /* references. */
   this->integercache[i - INTEGERCACHELOW]->stringValue();
 }
}

void RexxIntegerClass::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  long i;                              /* loop counter                      */

  this->RexxClass::live();             /* do RexxClass level marking        */

  setUpMemoryMark
                                       /* now mark the cached integers      */
  for (i = INTEGERCACHELOW; i < INTEGERCACHESIZE ;i++ ) {
   memory_mark(this->integercache[i - INTEGERCACHELOW]);
  }
  cleanUpMemoryMark
}

void RexxIntegerClass::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  long i;                              /* loop counter                      */

  this->RexxClass::liveGeneral();      /* do RexxClass level marking        */

  setUpMemoryMarkGeneral
                                       /* now mark the cached integers      */
  for (i = INTEGERCACHELOW; i < INTEGERCACHESIZE ;i++ ) {
   memory_mark_general(this->integercache[i - INTEGERCACHELOW]);
  }
  cleanUpMemoryMarkGeneral
}

void *RexxInteger::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new integer object                                     */
/******************************************************************************/
{
  RexxObject * newObject;              /* newly create object               */

  newObject = new_object(size);        /* get a new object                  */
                                       /* add in the integer behaviour, and */
                                       /* make sure old2new knows about it  */
  BehaviourSet(newObject, TheIntegerBehaviour);
  ClearObject(newObject);              /* clear the object                  */
  SetObjectHasNoReferences(newObject); /* Tell GC, not to bother with Live  */
  return newObject;                    /* return the new object.            */
}

void integer_create (void)
/******************************************************************************/
/* Function:  Create the integer class and set up the integer cache           */
/******************************************************************************/
{
                                       /* Create the Integer class object   */
                                       /*  its asubclass of the CLASS class,*/
                                       /*  and needs to override the NEW    */
                                       /*  method to provide caching        */
                                       /*  support for integers.            */
  create_udsubClass(Integer, RexxIntegerClass);
                                       /*  initialize our static array of   */
                                       /*  cached integers                  */
  new (TheIntegerClass) RexxIntegerClass();

}

#include "RexxNativeAPI.h"
#undef RexxInteger

native0 (long, INTEGER_VALUE)
/******************************************************************************/
/* Function:  External interface to the object method                         */
/******************************************************************************/
{
/******************************************************************************/
/* NOTE:  This method does not reaquire kernel access                         */
/******************************************************************************/
                                       /* forward the method                */
  return ((RexxInteger *)self)->value;
}

nativei1 (REXXOBJECT, INTEGER_NEW,
         long, value)                  /* integer value                     */
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(new_integer(value));
}
