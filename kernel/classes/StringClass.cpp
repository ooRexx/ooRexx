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
/* REXX Kernel                                                  StringClass.c    */
/*                                                                            */
/* Primitive String Class                                                     */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
extern INT  lookup[];
                                       /* current global settings           */
extern ACTIVATION_SETTINGS *current_settings;

#include "ASCIIDBCSStrings.hpp"
#include "RexxBuiltinFunctions.h"                          /* Gneral purpose BIF Header file       */

ULONG RexxString::hash()
/******************************************************************************/
/* Function:  retrieve the hash value of a string object                      */
/******************************************************************************/
{
  if (!OTYPE(String, this))            /*  a nonprimitive object?           */
                                       /* see if == overridden.             */
    return this->sendMessage(OREF_STRICT_EQUAL)->requestString()->getHashValue();
  else
    return HASHVALUE(this);            /* return the string hash            */
}

void RexxString::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->NumberString);
  memory_mark(this->objectVariables);
  cleanUpMemoryMark
}

void RexxString::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->NumberString);
  memory_mark_general(this->objectVariables);
  cleanUpMemoryMarkGeneral
}

void RexxString::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxString)

   flatten_reference(newThis->NumberString, envelope);
   flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}

RexxObject *RexxString::unflatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  unflatten an object                                             */
/******************************************************************************/
{
  RexxObject *newSelf;

  if (this->header & ProxyObject) {         /* is this a proxy object?              */
                                            /* Yes, then we need to run the code    */
                                            /*  to generate a new object            */
    newSelf = envelope->queryProxy(this);
    if (!newSelf) {                         /* not in the table?                    */
                                            /* get the proxy from the environment   */
      newSelf = TheEnvironment->entry(this);
                                            /* and add to the proxy table           */
      envelope->addProxy(this, newSelf);
    }
  }
  else
    newSelf = this->RexxObject::unflatten(envelope);
  return newSelf;                           /* return the new version               */
}

RexxString *RexxString::stringValue()
/******************************************************************************/
/* Function:  Return the primitive string value of this object                */
/******************************************************************************/
{
  if (OTYPE(String, this))             /* already a primitive string?       */
    return this;                       /* just return our selves            */
  else                                 /* need to build a new string        */
    return new_string(this->stringData, this->length);
}

RexxString  *RexxString::makeString()
/******************************************************************************/
/* Function:  Handle a REQUEST('STRING') request for a REXX string object     */
/******************************************************************************/
{
  if (isPrimitive(this))               /* really a primitive string?        */
    return this;                       /* this is easy                      */
  else                                 /* need to create a new string       */
    return new_string(this->stringData, this->length);
}


void RexxString::copyIntoTail(RexxCompoundTail *tail)
/******************************************************************************/
/* Function:  Handle a tail construction request for an internal object       */
/******************************************************************************/
{
                                       /* copy this directly into the tail */
    tail->append((UCHAR *)this->stringData, this->length);
}


RexxString  *RexxString::primitiveMakeString()
/******************************************************************************/
/* Function:  Handle a REQUEST('STRING') request for a REXX string object     */
/******************************************************************************/
{
  return this;                         /* this is easy                      */
}

long RexxString::longValue(
     size_t digits)                    /* digits to use                     */
/******************************************************************************/
/* Function:  Convert a string object to a long value.  Returns NO_LONG if    */
/*            it will not convert.                                            */
/******************************************************************************/
{
  RexxNumberString *numberstring;      /* converted numberstring version    */

  if (!(OTYPE(String, this)))          /* subclassed string object?         */
                                       /* get the string value's long value */
    return this->requestString()->longValue(digits);
  numberstring = this->fastNumberString(); /* get the number string version     */
  if (numberstring != OREF_NULL )      /* convert ok?                       */
                                       /* convert to integer with proper    */
                                       /* precision                         */
    return numberstring->longValue(digits);
  else
    return NO_LONG;                    /* return the "not value long" value */
}

double RexxString::doubleValue()
/******************************************************************************/
/* Function:  Convert a string object to a double value                       */
/******************************************************************************/
{
  RexxNumberString *numberDouble;      /* converted number string           */

  numberDouble = this->fastNumberString(); /* convert String to Numberstring    */
  if (numberDouble != OREF_NULL)       /* Did we get a numberstring?        */
    return numberDouble->doubleValue();/* Yup, convert it to double         */
  else
    return NO_DOUBLE;                  /* not number string, so NODOUBLE    */
}

RexxNumberString *RexxString::numberString()
/******************************************************************************/
/* Function:   Convert a String Object into a Number Object                   */
/******************************************************************************/
{
  RexxString       *newSelf;           /* converted string value            */
  RexxNumberString *numberString;      /* numberstring value of string      */

  if (this->nonNumeric())              /* Did we already try and convert to */
                                       /* to a numberstring and fail?       */
   return OREF_NULL;                   /* Yes, no need to try agian.        */

  if (this->NumberString != OREF_NULL) /* see if we have already converted  */
    return this->NumberString;         /* return the numberString Object.   */

  if (!OTYPE(String, this)) {          /* not truly a string type?          */
    newSelf = this->requestString();   /* do the conversion                 */
                                       /* get a new numberstring Obj        */
    OrefSet(newSelf, newSelf->NumberString, (RexxNumberString *)new_numberstring(newSelf->stringData, newSelf->length));
                                       /* save the number string            */
    numberString = newSelf->NumberString;
    if (numberString != OREF_NULL)     /* Did number convert OK?            */
      SetObjectHasReferences(newSelf); /* Make sure we are sent Live...     */
  }
  else {                               /* real primitive string             */
                                       /* get a new numberstring Obj        */
    OrefSet(this, this->NumberString, (RexxNumberString *)new_numberstring(this->stringData, this->length));
    numberString = this->NumberString; /* save the number string            */
    if (numberString == OREF_NULL)     /* Did number convert OK?            */
      this->setNonNumeric();           /* mark as a nonnumeric              */
    else {
      SetObjectHasReferences(this);    /* Make sure we are sent Live...     */
                                       /* connect the string and number     */
      this->NumberString->setString(this);
    }
  }
  return numberString;                 /* return the numberString Object.   */
}

RexxNumberString *RexxString::createNumberString()
/******************************************************************************/
/* Function:   Convert a String Object into a Number Object                   */
/******************************************************************************/
{
  RexxString       *newSelf;           /* converted string value            */
  RexxNumberString *numberString;      /* numberstring value of string      */

  if (!OTYPE(String, this)) {          /* not truly a string type?          */
    newSelf = this->requestString();   /* do the conversion                 */
                                       /* get a new numberstring Obj        */
    OrefSet(newSelf, newSelf->NumberString, (RexxNumberString *)new_numberstring(newSelf->stringData, newSelf->length));
                                       /* save the number string            */
    numberString = newSelf->NumberString;
    if (numberString != OREF_NULL)     /* Did number convert OK?            */
      SetObjectHasReferences(newSelf); /* Make sure we are sent Live...     */
  }
  else {                               /* real primitive string             */
                                       /* get a new numberstring Obj        */
    OrefSet(this, this->NumberString, (RexxNumberString *)new_numberstring(this->stringData, this->length));
    numberString = this->NumberString; /* save the number string            */
    if (numberString == OREF_NULL)     /* Did number convert OK?            */
      this->setNonNumeric();           /* mark as a nonnumeric              */
    else {
      SetObjectHasReferences(this);    /* Make sure we are sent Live...     */
                                       /* connect the string and number     */
      this->NumberString->setString(this);
    }
  }
  return numberString;                 /* return the numberString Object.   */
}


size_t RexxString::get(size_t start, PCHAR  buffer, size_t bufl)
/******************************************************************************/
/* Function:  Get a section of a string and copy it into a buffer             */
/******************************************************************************/
{
    size_t copylen = 0;

    if (start < this->length)
    {
        if (bufl <= this->length-start)
        {
            copylen = bufl - 1;
        }
        else
        {
            copylen = this->length-start;
            *(buffer+copylen) = '\0';
        }
        memcpy(buffer,this->stringData+start,(size_t)copylen);
    }
    else
    {
        *buffer = '\0';
    }

    return copylen + 1;
}

RexxObject *RexxString::lengthRexx()
/******************************************************************************/
/* Function:  Return the length of a string as an integer object              */
/******************************************************************************/
{
  size_t tempLen;

  if (DBCS_SELF)                       /* need to use DBCS?                 */
                                       /* return the DBCS count             */
    return this->DBCSlength();
  tempLen = this->length;
                                       /* return string byte length         */
  return (RexxObject *) new_integer(tempLen);
}

BOOL RexxString::isEqual(
    RexxObject *otherObj)              /* other comparison object           */
/******************************************************************************/
/* Function:  Primitive strict equal\not equal method.  This determines       */
/*            only strict equality, not greater or less than values.          */
/******************************************************************************/
{
  size_t  otherLen;                    /* length of the other string        */
  RexxString *other;                   /* converted string object           */

  required_arg(otherObj, ONE);         /* this is required.                 */
  if (!isPrimitive(this))              /* not a primitive?                  */
                                       /* do the full lookup compare        */
    return this->sendMessage(OREF_STRICT_EQUAL, otherObj)->truthValue(Error_Logical_value_method);

  other = REQUEST_STRING(otherObj);    /* force into string form            */
  if (DBCS_MODE) {                     /* need to use DBCS?                 */
    ValidDBCS(this);                   /* validate the string               */
    ValidDBCS(other);                  /* and the other string too          */
  }

  otherLen = other->length;            /* get length of second string.      */
                                       /* do quick compare on the hash      */
  if (this->hashvalue != other->hashvalue)
    return FALSE;                      /* can't be equal                    */
  if (otherLen != this->length)        /* lengths different?                */
    return FALSE;                      /* also unequal                      */
                                       /* now compare the actual string     */
  return !memcmp(this->stringData, other->stringData, otherLen);
}

BOOL RexxString::primitiveIsEqual(
    RexxObject *otherObj)              /* other comparison object           */
/******************************************************************************/
/* Function:  Primitive strict equal\not equal method.  This determines       */
/*            only strict equality, not greater or less than values.          */
/******************************************************************************/
{
  size_t  otherLen;                    /* length of the other string        */
  RexxString *other;                   /* converted string object           */

  required_arg(otherObj, ONE);         /* this is required.                 */
  if (otherObj == TheNilObject)        // strings never compare equal to the NIL object
  {
      return FALSE;
  }

  other = REQUEST_STRING(otherObj);    /* force into string form            */
  if (DBCS_MODE) {                     /* need to use DBCS?                 */
    ValidDBCS(this);                   /* validate the string               */
    ValidDBCS(other);                  /* and the other string too          */
  }

  otherLen = other->length;            /* get length of second string.      */
                                       /* do quick compare on the hash      */
  if (this->hashvalue != other->hashvalue)
    return FALSE;                      /* can't be equal                    */
  if (otherLen != this->length)        /* lengths different?                */
    return FALSE;                      /* also unequal                      */
                                       /* now compare the actual string     */
  return !memcmp(this->stringData, other->stringData, otherLen);
}


/**
 * Primitive string caseless comparison.
 *
 * @param otherObj The other string to compare.
 *
 * @return true if the strings compare, false otherwise.
 */
bool RexxString::primitiveCaselessIsEqual(RexxObject *otherObj)
{
    // we have one required string object
    required_arg(otherObj, ONE);
    RexxString *other = REQUEST_STRING(otherObj);
    stringsize_t otherLen = other->getLength();
    // can't compare equal if different lengths
    if (otherLen != this->getLength())
    {
        return false;
    }
    // do the actual string compare
    return CaselessCompare((PUCHAR)this->getStringData(), (PUCHAR)other->getStringData(), otherLen) == 0;
}


/**
 * Wrapper around the compareTo() method that validates and
 * extracts integer value.
 *
 * @param other  The other comparison object
 *
 * @return -1, 0, 1 depending on the comparison result.
 */
wholenumber_t RexxString::compareTo(RexxObject *other )
{
    if (isPrimitive(this))
    {
        return compareToRexx((RexxString *)other, OREF_NULL, OREF_NULL)->value;
    }
    else
    {
        return RexxObject::compareTo(other);
    }
}


long RexxString::comp(RexxObject *other)
/******************************************************************************/
/* Function:  Do a value comparison of two strings for the non-strict         */
/*            comparisons.  This returns for the compares:                    */
/*                                                                            */
/*             a value < 0 when this is smaller than other                    */
/*             a value   0 when this is equal to other                        */
/*             a value > 0 when this is larger than other                     */
/******************************************************************************/
{
  RexxString *second;                  /* string value of other             */
  RexxNumberString *firstNum;          /* numberstring value of this        */
  RexxNumberString *secondNum;         /* numberstring value of other       */
  PUCHAR firstStart;                   /* comparison start pointer          */
  PUCHAR secondStart;                  /* other start pointer               */
  size_t firstLen;                     /* this compare length               */
  size_t secondLen;                    /* other compare length              */
  int  result;                         /* compare result                    */

                                       /* We need to see if the objects can */
                                       /* be Converted to NumberString Objs */
                                       /* 1st, this way we know if the COMP */
                                       /* method of number String will      */
                                       /* succeed.  Will only fail if an    */
                                       /* object cannot be represented as a */
                                       /* number.  This is important since  */
                                       /* NumberString calls String to do   */
                                       /* the compare if it can't, since    */
                                       /* this is the method NumberString   */
                                       /* will call, we must make sure a    */
                                       /* call to NumberString succeeds or  */
                                       /* we will get into a loop.          */
  required_arg(other, ONE);            /* make sure we have a real argument */
                                       /* try and convert both numbers      */
  if (((firstNum = this->fastNumberString()) != OREF_NULL) && ((secondNum = other->numberString()) != OREF_NULL ))
                                       /* yes, send converted numbers and do*/
                                       /* the compare                       */
    return (long) firstNum->comp(secondNum);
  second = REQUEST_STRING(other);      /* yes, get a string object.         */
  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS comparison            */
     return this->DBCSstringCompare(second);
                                       /* objects are converted.  now strip */
                                       /* any leading/trailing blanks.      */

  firstLen = this->length;             /* get the initial length            */
  firstStart = (PUCHAR)this->stringData; /* and starting position           */

  secondLen = second->length;          /* get length of second string.      */
  secondStart = (PUCHAR)second->stringData; /* get pointer to start of data */

                                       /* while we have leading blanks.     */
  while (firstLen > 0 && (*firstStart == ch_BLANK || *firstStart == ch_TAB)) {
   firstStart++;                       /* ignore character and look at next */
   firstLen--;                         /* and string is now one char less.  */
  }
                                       /* while we have leading blanks.     */
  while (secondLen > 0 && (*secondStart == ch_BLANK || *secondStart == ch_TAB)) {
   secondStart++;                      /* ignore character and look at next */
   secondLen--;                        /* and string is now one char less.  */
  }

  if (firstLen >= secondLen) {         /* determine the longer string.      */
                                       /* first string is larger,           */

                                       /* do a memory compare of strings,   */
                                       /* use length of smaller string.     */
    result = memcmp(firstStart, secondStart, (size_t) secondLen);
                                       /* equal but different lengths?      */
    if ((result == 0) && (firstLen != secondLen)) {
                                       /* point to first remainder char     */
      firstStart = (PUCHAR) firstStart + secondLen;
      while (firstLen-- > secondLen) { /* while still have more to compare  */
          unsigned char current = *firstStart++;
          if (current != ch_BLANK && current != ch_TAB)
          {
              return current - ch_BLANK;
          }
      }
    }
  }

  else {                               /* The length of second obj is longer*/
                                       /* do memory compare of strings, use */
                                       /*  length of smaller string.        */
    result = memcmp(firstStart, secondStart, (size_t) firstLen);
    if  (result == 0) {                /* if strings compared equal, we have*/
                                       /* we need to compare the trailing   */
                                       /* part with blanks                  */
      secondStart = (PUCHAR) secondStart + firstLen;
      while (secondLen-- > firstLen) { /* while the longer string stills has*/
        unsigned char current = *secondStart++;
        if (current != ch_BLANK && current != ch_TAB)
        {
            return ch_BLANK - current;
        }
      }
    }
  }
  return result;                       /* return the compare result         */
}

long RexxString::strictComp(RexxObject *otherObj)
/******************************************************************************/
/* Function:  Do a strict comparison of two strings.  This returns:           */
/*                                                                            */
/*             a value < 0 when this is smaller than other                    */
/*             a value   0 when this is equal to other                        */
/*             a value > 0 when this is larger than other                     */
/******************************************************************************/
{
  char *otherData;                     /* the other character data          */
  size_t otherLen;                     /* length of the other string        */
  int  result;                         /* compare result                    */
  RexxString *other;                   /* converted string value            */

  required_arg(otherObj, ONE);         /* this is required.                 */
  other = REQUEST_STRING(otherObj);    /* force into string form            */
  if (DBCS_MODE) {                     /* need to use DBCS?                 */
    ValidDBCS(this);                   /* validate the string               */
    ValidDBCS(other);                  /* and the other string too          */
  }

  otherLen = other->length;            /* get length of second string.      */
  otherData = other->stringData;       /* get pointer to start of data.     */

  if (this->length >= otherLen) {      /* determine the longer string.      */
                                       /* first string is larger,           */
                                       /* do a memory compare of strings,   */
                                       /* use length of smaller string.     */
    result = memcmp(this->stringData, otherData, (size_t) otherLen);
                                       /* if strings are equal, and         */
                                       /* are not equal, the self is greater*/
    if ((result == 0) && (this->length > otherLen))
      result = 1;                      /* otherwise they are equal.         */
  }
  else {                               /* The length of second obj is longer*/
                                       /* do memory compare of strings, use */
                                       /*  length of smaller string.        */
    result = memcmp(this->stringData, otherData, (size_t) this->length);
    if  (result == 0)                  /* if stings compared equal,         */
      result = -1;                     /*  then the other string is bigger. */
  }
  return result;                       /* finished, return our result       */
}

RexxObject *RexxString::plus(RexxObject *right)
/******************************************************************************/
/* Function:  String addition...performed by RexxNumberString                 */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception1(Error_Conversion_operator, this);
  return numstr->plus(right);          /* have numberstring do this         */
}

RexxObject *RexxString::minus(RexxObject *right)
/******************************************************************************/
/* Function:  String subtraction...performed by RexxNumberString              */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception1(Error_Conversion_operator, this);
  return numstr->minus(right);         /* have numberstring do this         */
}

RexxObject *RexxString::multiply(RexxObject *right)
/******************************************************************************/
/* Function:  String multiplication...performed by RexxNumberString           */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception1(Error_Conversion_operator, this);
  return numstr->multiply(right);      /* have numberstring do this         */
}

RexxObject *RexxString::divide(RexxObject *right)
/******************************************************************************/
/* Function:  String division...performed by RexxNumberString                 */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception1(Error_Conversion_operator, this);
  return numstr->divide(right);        /* have numberstring do this         */
}

RexxObject *RexxString::integerDivide(RexxObject *right)
/******************************************************************************/
/* Function:  String division...performed by RexxNumberString                 */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception1(Error_Conversion_operator, this);
  return numstr->integerDivide(right); /* have numberstring do this         */
}

RexxObject *RexxString::remainder(RexxObject *right)
/******************************************************************************/
/* Function:  String division...performed by RexxNumberString                 */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception1(Error_Conversion_operator, this);
  return numstr->remainder(right);     /* have numberstring do this         */
}

RexxObject *RexxString::power(RexxObject *right)
/******************************************************************************/
/* Function:  String division...performed by RexxNumberString                 */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception1(Error_Conversion_operator, this);
  return numstr->power(right);         /* have numberstring do this         */
}

RexxObject *RexxString::abs(void)
/******************************************************************************/
/* Function:  String absolute value...performed by RexxNumberString           */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception2(Error_Incorrect_method_string_nonumber, new_cstring("ABS"), this);
  return numstr->abs();                /* have numberstring do this         */
}

RexxObject *RexxString::sign(void)
/******************************************************************************/
/* Function:  String sign value...performed by RexxNumberString               */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception2(Error_Incorrect_method_string_nonumber, new_cstring("SIGN"), this);
  return numstr->Sign();               /* have numberstring do this         */
}

RexxObject *RexxString::Max(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  String max value...performed by RexxNumberString                */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception2(Error_Incorrect_method_string_nonumber, new_cstring("MAX"), this);
  /* have numberstring do this         */
  return numstr->Max(arguments, argCount);
}

RexxObject *RexxString::Min(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  String min value...performed by RexxNumberString                */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception2(Error_Incorrect_method_string_nonumber, new_cstring("MIN"), this);
  /* have numberstring do this         */
  return numstr->Min(arguments, argCount);
}

RexxObject *RexxString::trunc(RexxInteger *decimals)
/******************************************************************************/
/* Function:  String Trunc...performed by RexxNumberString                    */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception2(Error_Incorrect_method_string_nonumber, new_cstring("TRUNC"), this);
  return numstr->trunc(decimals);      /* have numberstring do this         */
}

RexxObject *RexxString::format(RexxObject *Integers, RexxObject *Decimals, RexxObject *MathExp, RexxObject *ExpTrigger)
/******************************************************************************/
/* Function:  String Format...performed by RexxNumberString                   */
/******************************************************************************/
{
  RexxNumberString *numstr;            /* converted number string           */

                                       /* non-numeric?                      */
  if ((numstr = this->fastNumberString()) == OREF_NULL)
                                       /* this is a conversion error        */
    report_exception2(Error_Incorrect_method_string_nonumber, new_cstring("FORMAT"), this);
                                       /* have numberstring do this         */
  return numstr->formatRexx(Integers, Decimals, MathExp, ExpTrigger);
}


/**
 * The string equals() method, which does a strict compare with
 * another string object.
 *
 * @param other  The other string object.
 *
 * @return True if the strings are equal, false for inequality.
 */
RexxInteger *RexxString::equals(RexxString *other)
{
    return this->primitiveIsEqual(other) ? TheTrueObject : TheFalseObject;
}

/**
 * The string equals() method, which does a strict caseless
 * compare with another string object.
 *
 * @param other  The other string object.
 *
 * @return True if the strings are equal, false for inequality.
 */
RexxInteger *RexxString::caselessEquals(RexxString *other)
{
    return this->primitiveCaselessIsEqual(other) ? TheTrueObject : TheFalseObject;
}


RexxInteger *RexxString::strictEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Strict ("==") equality operator...also returns the hash value   */
/*            if sent with no other object                                    */
/******************************************************************************/
{
    return this->primitiveIsEqual(other) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxString::strictNotEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Strict ("\==") inequality operator                              */
/******************************************************************************/
{
  return !this->primitiveIsEqual(other) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxString::equal(RexxObject *other)
/******************************************************************************/
/* Function:  Non-strict ("=") string equality operator                       */
/******************************************************************************/
{
  // Strings never compare equal to the .nil object
  if (other == TheNilObject)
  {
      return TheFalseObject;
  }
  return ((this->comp(other) == 0) ? TheTrueObject : TheFalseObject);
}

RexxInteger *RexxString::notEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Non-Strict ("\=") string inequality operator                    */
/******************************************************************************/
{
  // Strings never compare equal to the .nil object
  if (other == TheNilObject)
  {
      return TheTrueObject;
  }
  return ((this->comp(other) != 0) ? TheTrueObject : TheFalseObject);
}

RexxInteger *RexxString::isGreaterThan(RexxObject *other)
/******************************************************************************/
/* Function:  Non-strict greater than operator (">")                          */
/******************************************************************************/
{
  return ((this->comp(other) > 0) ? TheTrueObject : TheFalseObject);
}

RexxInteger *RexxString::isLessThan(RexxObject *other)
/******************************************************************************/
/* Function:  Non-strict less than operatore ("<")                            */
/******************************************************************************/
{
  return ((this->comp(other) < 0) ? TheTrueObject : TheFalseObject);
}

RexxInteger *RexxString::isGreaterOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Non-strict greater than or equal operator (">=" or "\<")        */
/******************************************************************************/
{
  return ((this->comp(other) >= 0) ? TheTrueObject : TheFalseObject);
}

RexxInteger *RexxString::isLessOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Non-strict less than or equal operator ("<=" or "\>")           */
/******************************************************************************/
{
  return ((this->comp(other) <= 0) ? TheTrueObject : TheFalseObject);
}

RexxInteger *RexxString::strictGreaterThan(RexxObject *other)
/******************************************************************************/
/* Function:  Strict greater than comparison (">>")                           */
/******************************************************************************/
{
  return (this->strictComp(other) > 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxString::strictLessThan(RexxObject *other)
/******************************************************************************/
/* Function:  Strict less than comparison ("<<")                              */
/******************************************************************************/
{
  return (this->strictComp(other) < 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxString::strictGreaterOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Strict greater than or equal to comparison (">>=" or "\<<")     */
/******************************************************************************/
{
  return (this->strictComp(other) >= 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxString::strictLessOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Strict less than or equal to operatore ("<<=" or "\>>")         */
/******************************************************************************/
{
  return (this->strictComp(other) <= 0) ? TheTrueObject : TheFalseObject;
}

RexxString *RexxString::concat(RexxString *other)
/******************************************************************************/
/* Function:  Concatenate two strings together                                */
/******************************************************************************/
{
  size_t len1;                         /* length of first string            */
  size_t len2;                         /* length of second string           */
  RexxString *result;                  /* result string                     */
  PCHAR data;                          /* character pointer                 */

  len1 = this->length;                 /* get this length                   */
  len2 = other->length;                /* and the other length              */

  if (len2 == 0)                       // some people have taken to using a''b
  {                                    // to perform concatenation operations
      return this;                     // it makes sense to optimize concatenation
  }                                    // with a null string by just returning
  if (len1 == 0)                       // the non-null object.
  {
      return other;
  }
                                       /* create a new string               */
  result = (RexxString *)raw_string(len1+len2);
  data = result->stringData;           /* point to the string data          */

  // both lengths are non-zero because of the test above, so we can
  // unconditionally copy
                                       /* copy the front part               */
  memcpy(data, this->stringData, len1);
  memcpy(data + len1, other->stringData, len2);
  result->generateHash();              /* done building the string          */
  return result;                       /* return the result                 */

}

RexxString *RexxString::concatRexx(RexxObject *otherObj)
/******************************************************************************/
/* Function:  Rexx level concatenate...requires conversion and checking       */
/******************************************************************************/
{
  size_t len1;                         /* length of first string            */
  size_t len2;                         /* length of second string           */
  RexxString *result;                  /* result string                     */
  RexxString *other;
  PCHAR data;                          /* character pointer                 */

  required_arg(otherObj, ONE);         /* this is required.                 */
                                       /* ensure a string value             */
  other = (RexxString *)REQUEST_STRING(otherObj);

  /* added error checking for NULL pointer (from NilObject) */
  if (other == OREF_NULL)
    report_exception1(Error_Incorrect_method_nostring, IntegerOne);

  if (DBCS_MODE) {                     /* need to use DBCS?                 */
    ValidDBCS(this);                   /* validate the string               */
    ValidDBCS(other);                  /* and the other string too          */
  }

                                       /* the following logic also appears  */
                                       /* in string_concat, but is repeated */
                                       /* here because this is a VERY high  */
                                       /* use function                      */
  len1 = this->length;                 /* get this length                   */
  len2 = other->length;                /* and the other length              */
                                       /* create a new string               */
  result = (RexxString *)raw_string(len1+len2);
  data = result->stringData;           /* point to the string data          */
  if (len1 != 0) {                     /* have real data?                   */
                                       /* copy the front part               */
    memcpy(data, this->stringData, len1);
    data += len1;                      /* step past the length              */
  }
  if (len2 != 0)                       /* have a second length              */
                                       /* and the second part               */
    memcpy(data, other->stringData, len2);
  result->generateHash();              /* done building the string          */
  return result;                       /* return the result                 */
}

RexxString *RexxString::concatToCstring(PCHAR  other)
/******************************************************************************/
/* Function:  Concatenate a string object onto an ASCII-Z string              */
/******************************************************************************/
{
  size_t len1;                         /* length of first string            */
  size_t len2;                         /* length of ASCII-Z string          */
  RexxString *result;                  /* result string                     */

  len1 = this->length;                 /* get this length                   */
  len2 = strlen(other);                /* and the other length              */
                                       /* create a new string               */
  result = (RexxString *)raw_string(len1+len2);
                                       /* copy the front part               */
  memcpy(result->stringData, other, len2);
                                       /* and the second part               */
  memcpy(result->stringData + len2, this->stringData, len1);
  result->generateHash();              /* done building the string          */
  return result;
}

RexxString *RexxString::concatWithCstring(PCHAR  other)
/******************************************************************************/
/* Function:  Concatenate an ASCII-Z string onto a string object              */
/******************************************************************************/
{
  size_t len1;                         /* length of first string            */
  size_t len2;                         /* length of ASCII-Z string          */
  RexxString *result;                  /* result string                     */

  len1 = this->length;                 /* get this length                   */
  len2 = strlen(other);                /* and the other length              */
                                       /* create a new string               */
  result = (RexxString *)raw_string(len1+len2);
                                       /* copy the string object            */
  memcpy(result->stringData, this->stringData, len1);
                                       /* copy the ASCII-Z string           */
  memcpy(result->stringData + len1, other, len2);
  result->generateHash();              /* done building the string          */
  return result;
}

RexxString *RexxString::concatBlank(RexxObject *otherObj)
/******************************************************************************/
/* Function:  Concatenate two strings with a blank in between                 */
/******************************************************************************/
{
  size_t len1;                         /* length of first string            */
  size_t len2;                         /* length of second string           */
  RexxString *result;                  /* result string                     */
  RexxString *other;                   /* result string                     */
  PCHAR data;                          /* character pointer                 */

  required_arg(otherObj, ONE);         /* this is required.                 */
                                       /* ensure a string value             */
  other = (RexxString *)REQUEST_STRING(otherObj);

  /* added error checking for NULL pointer (from NilObject) */
  if (other == OREF_NULL)
    report_exception1(Error_Incorrect_method_nostring, IntegerOne);

  if (DBCS_MODE) {                     /* need to use DBCS?                 */
    ValidDBCS(this);                   /* validate the string               */
    ValidDBCS(other);                  /* and the other string too          */
  }

                                       /* ensure a string value             */
  other = (RexxString *)REQUEST_STRING(otherObj);

  /* added error checking for NULL pointer (from NilObject) */
  if (other == OREF_NULL)
    report_exception1(Error_Incorrect_method_nostring, IntegerOne);
                                       /* the following logic also appears  */
                                       /* in string_concat_with, but is     */
                                       /* repeated here because this is a   */
                                       /* VERY high use function            */
  len1 = this->length;                 /* get this length                   */
  len2 = other->length;                /* and the other length              */
                                       /* create a new string               */
  result = (RexxString *)raw_string(len1+len2+1);
  data = result->stringData;           /* point to the string data          */
  if (len1 != 0) {                     /* have a first string?              */
                                       /* copy the front part               */
    memcpy(data, this->stringData, len1);
    data += len1;                      /* step past the length              */
  }
  *data++ = ' ';                       /* stuff in the seperating blank     */
  if (len2 != 0)                       /* have a second string?             */
                                       /* and the second part               */
    memcpy(data, other->stringData, len2);
  result->generateHash();              /* rebuild the hash value            */
  return result;
}

BOOL RexxString::truthValue(long errorCode)
/******************************************************************************/
/* Function:  Determine the truth value of a string object, raising the       */
/*            given error if bad.                                             */
/******************************************************************************/
{
  RexxString *testString;              /* string to test                    */

  if (!OTYPE(String, this))            /*  a nonprimitive object?           */
    testString = this->requestString();/* get the real string value         */
  else
    testString = this;                 /* just use the string directly      */
  if (testString->length != 1L)        /* not exactly 1 character long?     */
                                       /* report the error                  */
    report_exception1(errorCode, testString);
  if (*(testString->stringData) == '0')/* exactly '0'?                      */
      return FALSE;                    /* have a false                      */
                                       /* not exactly '1'?                  */
  else if (!(*(testString->stringData) == '1'))
    report_exception1(errorCode, this);/* report the error                  */
  return TRUE;                         /* this is true                      */
}

BOOL RexxString::checkLower()
/******************************************************************************/
/* Function:  Tests for existence of lowercase characters                     */
/******************************************************************************/
{
  PUCHAR data;                         /* current data pointer              */
  PUCHAR endData;                      /* end location                      */

  data = (PUCHAR)this->stringData;     /* point to the string               */
  endData = data + this->length;       /* set the end point                 */

  while (data < endData) {             /* loop through entire string        */
    if (*data != toupper(*data)) {     /* have something to uppercase?      */
      this->setHasLower();             /* remember we have this             */
      return TRUE;                     /* just return now                   */
    }
    data++;                            /* step the position                 */
  }
                                       /* no lowercase?                     */
  this->setUpperOnly();                /* set the upper only attribute      */
  return FALSE;                        /* return then translation flag      */
}

RexxString *RexxString::upper()
/******************************************************************************/
/* Function:  Translate a string to uppercase...will only create a new        */
/*            string if characters actually have to be translated.            */
/******************************************************************************/
{
  RexxString *newstring;               /* newly created string              */
  PUCHAR data;                         /* current data pointer              */
  PUCHAR outdata;                      /* output data                       */
  PUCHAR endData;                      /* end of the data                   */

  if (DBCS_SELF) {                     /* need to use DBCS?                 */
    ValidDBCS(this);                   /* validate the string               */
                                       /* actually have DBCS and need to    */
                                       /* uppercase?                        */
    if (!NoDBCS(this) & !this->upperOnly()) {
                                       /* create a new string               */
      newstring = new_string((PCHAR)this->stringData, this->length);
                                       /* do DBCS uppercasing               */
      DBCS_MemUpper(STRPTR(newstring), STRLEN(newstring));
                                       /* rebuild the hash value            */
      newstring->generateHash();
                                       /* flag the string as uppercased     */
      newstring->setUpperOnly();
      return newstring;                /* return the new string             */
    }                                  /* (single byte only falls through)  */
  }
                                       /* something to uppercase?           */
  if (!this->upperOnly() && (this->hasLower() || this->checkLower())) {
                                       /* create a new string               */
    newstring = (RexxString *)raw_string(this->length);
    data = (PUCHAR)this->stringData;   /* point to the data start           */
                                       /* point to output data              */
    outdata = (PUCHAR)newstring->stringData;
    endData = data + this->length;     /* set the loop terminator           */
    while (data < endData) {           /* loop through entire string        */
      *outdata = toupper(*data);       /* copy the uppercase character      */
      data++;                          /* step the position                 */
      outdata++;                       /* and the output position           */
    }
    newstring->generateHash();         /* rebuild the hash value            */
    newstring->setUpperOnly();         /* flag the string as uppercased     */
    return newstring;                  /* return the new string             */
  }
  return this;                         /* return this unchanged             */
}

RexxString *RexxString::stringTrace()
/******************************************************************************/
/* Function:  Translate a string to "traceable" form, removing non-displayable*/
/*            characters                                                      */
/******************************************************************************/
{
  RexxString *newCopy;                 /* new copy of string                */
  PUCHAR    Current;                   /* current string location           */
  size_t    i;                         /* string length                     */
  BOOL      NonDisplay;                /* have non-displayables             */

  i = this->length;                    /* get the length                    */
  Current = (PUCHAR)this->stringData;  /* point to the start                */
  NonDisplay = FALSE;                  /* no non-displayable characters     */

  for (; i > 0; i--) {                 /* loop for the entire string        */
    if (DBCS_SELF) {                   /* this string have DBCS characters  */
      if (IsDBCS(*Current)) {          /* leave DBCS alone                  */
        i--;                           /* reduce length one more            */
        Current++;                     /* and the pointer too               */
      }
                                       /* control character?                */
      else if ((UCHAR)*Current < (UCHAR)' ') {
        NonDisplay = TRUE;             /* got a non-displayable             */
        break;                         /* get out of here                   */
       }
    }
    else                               /*no DBCS characters                 */
                                       /* control character?                */
    if ((UCHAR)*Current < (UCHAR)' ') {
      NonDisplay = TRUE;               /* got a non-displayable             */
      break;                           /* get out of here                   */
    }
    Current++;                         /* step the pointer                  */
  }
  if (!NonDisplay)                     /* all displayable?                  */
    return this;                       /* leave unchanged                   */
                                       /* copy the string                   */
  newCopy = (RexxString *) this->copy();
  i = newCopy->length;                 /* get the length                    */
                                       /* point to the start                */
  Current = (PUCHAR)newCopy->stringData;

  for (; i > 0; i--) {                 /* loop for the entire string        */
    if(DBCS_MODE) {
      if (IsDBCS(*Current)) {          /* leave DBCS alone                  */
        i--;                           /* reduce length one more            */
        Current++;                     /* and the pointer too               */
       }
                                       /* control character?                */
      else if ((UCHAR)*Current < (UCHAR)' ' && (UCHAR)*Current != '\t')
        *Current = '?';                /* yes, change to question           */
      Current++;                       /* step the pointer                  */
    }
                                       /* no DBCS characters                */
                                       /* control character?                */
    else if ((UCHAR)*Current < (UCHAR)' ' && (UCHAR)*Current != '\t')
        *Current = '?';                /* yes, change to question           */
      Current++;                       /* step the pointer                  */
  }
  newCopy->generateHash();             /* rebuild the hash value            */
  return newCopy;                      /* return the converted string       */
}


RexxString *RexxString::lower()
/******************************************************************************/
/* Function:  Translate a string to lower case                                */
/******************************************************************************/
{
  RexxString *newstring;               /* newly created string              */
  PUCHAR         data;                 /* current data pointer              */
  PUCHAR         outdata;              /* output data                       */
  size_t i;                            /* loop counter                      */
  bool   translate;                    /* translation required              */

  if (DBCS_SELF) {                     /* need to use DBCS?                 */
    ValidDBCS(this);                   /* validate the string               */
    if (!NoDBCS(this)) {               /* actually have DBCS characters?    */
                                       /* create a new string               */
      newstring = (RexxString *)raw_string(this->getLength());
                                       /* copy the data                     */
      memcpy(STRPTR(newstring), STRPTR(this), STRLEN(this));
                                       /* do DBCS lowercasing               */
      DBCS_MemLower(STRPTR(newstring), STRLEN(newstring));
                                       /* rebuild the hash value            */
      return newstring;                /* return the new string             */
    }                                  /* (single byte only falls through)  */
  }

  data = (PUCHAR)this->getStringData();  /* point to the string               */
  translate = false;                   /* no translation required           */

  for (i = 0; i < this->getLength(); i++) { /* loop through entire string        */
    if (*data != tolower(*data)) {     /* have something to lowercase?      */
      translate = true;                /* flag it                           */
      break;                           /* stop at the first one             */
    }
    data++;                            /* step the position                 */
  }
  if (translate) {                     /* something to uppercase?           */
                                       /* create a new string               */
    newstring = (RexxString *)raw_string(this->getLength());
    data = (PUCHAR)this->getStringData();   /* point to the data start           */
                                       /* point to output data              */
    outdata = (PUCHAR)newstring->getStringData();
                                       /* loop through entire string        */
    for (i = 0; i < this->getLength(); i++) {
      *outdata = tolower(*data);       /* copy the lowercase character      */
      data++;                          /* step the position                 */
      outdata++;                       /* and the output position           */
    }
    newstring->generateHash();         /* rebuild the hash value            */
  }
  else
    newstring = this;                  /* return untranslated string        */
  return newstring;                    /* return the new copy               */
}


/**
 * Rexx exported method stub for the lower() method.
 *
 * @param start  The optional starting location.  Defaults to the first character
 *               if not specified.
 * @param length The length to convert.  Defaults to the segment from the start
 *               position to the end of the string.
 *
 * @return A new string object with the case conversion applied.
 */
RexxString *RexxString::lowerRexx(RexxInteger *start, RexxInteger *length)
{
    size_t startPos = optional_position(start, 1, ARG_ONE) - 1;
    size_t rangeLength = optional_length(length, getLength(), ARG_TWO);

    // if we're starting beyond the end bounds, return unchanged
    if (startPos >= getLength())
    {
        return this;
    }

    rangeLength = min(rangeLength, getLength() - startPos);

    // a zero length value is also a non-change.
    if (rangeLength == 0)
    {
        return this;
    }

    return lower(startPos, rangeLength);
}


/**
 * Rexx exported method stub for the upper() method.
 *
 * @param start  The optional starting location.  Defaults to the first character
 *               if not specified.
 * @param length The length to convert.  Defaults to the segment from the start
 *               position to the end of the string.
 *
 * @return A new string object with the case conversion applied.
 */
RexxString *RexxString::upperRexx(RexxInteger *start, RexxInteger *length)
{
    size_t startPos = optional_position(start, 1, ARG_ONE) - 1;
    size_t rangeLength = optional_length(length, getLength(), ARG_TWO);

    // if we're starting beyond the end bounds, return unchanged
    if (startPos >= getLength())
    {
        return this;
    }

    rangeLength = min(rangeLength, getLength() - startPos);

    // a zero length value is also a non-change.
    if (rangeLength == 0)
    {
        return this;
    }

    return upper(startPos, rangeLength);
}



/**
 * Lowercase a portion of a Rexx string, returning a new string object.  This
 * method assumes the offset and length are already valid
 * for this string object.
 *
 * @param start  The starting offset of the segment to lowercase (origin 0).
 *
 * @param length The length to lowercase.
 *
 * @return A new string object with the case conversion applied.
 */
RexxString *RexxString::lower(size_t offset, size_t length)
{
    // get a copy of the string
    RexxString *newstring = extract(0, getLength());

    PUCHAR data = (PUCHAR)newstring->getStringData() + offset;
    // now uppercase in place
    for (size_t i = 0; i < length; i++) {
        *data = tolower(*data);
        data++;
    }
    newstring->generateHash();              /* done building the string          */
    return newstring;
}



/**
 * Uppercase a portion of a Rexx string, returning a new string
 * object.  This method assumes the offset and length are
 * already valid for this string object.
 *
 * @param start  The starting offset of the segment to uppercase
 *               (origin 0).
 *
 * @param length The length to lowercase.
 *
 * @return A new string object with the case conversion applied.
 */
RexxString *RexxString::upper(size_t offset, size_t length)
{
    // get a copy of the string
    RexxString *newstring = extract(0, getLength());

    PUCHAR data = (PUCHAR)newstring->getStringData() + offset;
    // now uppercase in place
    for (size_t i = 0; i < length; i++) {
        *data = toupper(*data);
        data++;
    }
    newstring->generateHash();              /* done building the string          */
    return newstring;
}

RexxInteger *RexxString::integerValue(
    size_t digits)                     /* precision to use                  */
/******************************************************************************/
/* Function:  Convert a string object to an integer.  Returns .nil for        */
/*            failures.                                                       */
/******************************************************************************/
{
  RexxNumberString *numberString;      /* string's numberstring version     */
  RexxInteger *newInteger;             /* returned integer string           */

                                       /* Force String conversion through   */
                                       /* NumberString                      */
                                       /* get the number string version     */
  if ((numberString = this->fastNumberString()) != OREF_NULL ) {
                                       /* try for an integer                */
    newInteger = numberString->integerValue(digits);
                                       /* did it convert?                   */
    if (newInteger != TheNilObject && newInteger->stringrep == OREF_NULL)
      newInteger->setString(this);     /* connect the string value          */
    return newInteger;                 /* return the new integer            */
  }
  else
    return (RexxInteger *)TheNilObject;/* return .nil for failures          */
}

void RexxString::setNumberString(RexxObject *NumberRep)
/******************************************************************************/
/* Function:  Set a number string value on to the string                      */
/******************************************************************************/
{

  OrefSet(this, this->NumberString, (RexxNumberString *)NumberRep);

  if (NumberRep != OREF_NULL)          /* actually get one?                 */
   SetObjectHasReferences(this);       /* Make sure we are sent Live...     */
  else
   SetObjectHasNoReferences(this);     /* no more references                */
  return;
}

RexxString *RexxString::concatWith(RexxString *other,
                                   char        between)
/******************************************************************************/
/* Function:  Concatenate two strings with a single character between         */
/******************************************************************************/
{
  size_t len1;                         /* length of first string            */
  size_t len2;                         /* length of second string           */
  RexxString *result;                  /* result string                     */
  PCHAR data;                          /* character pointer                 */

  len1 = this->length;                 /* get this length                   */
  len2 = other->length;                /* and the other length              */
                                       /* create a new string               */
  result = (RexxString *)raw_string(len1+len2+1);
  data = result->stringData;           /* point to the string data          */
  if (len1 != 0) {                     /* have a first string?              */
                                       /* copy the front part               */
    memcpy(data, this->stringData, len1);
    data += len1;                      /* step past the length              */
  }
  *data++ = between;                   /* stuff in the seperating char      */
  if (len2 != 0)                       /* have a second string?             */
                                       /* and the second part               */
    memcpy(data, other->stringData, len2);
  result->generateHash();              /* rebuild the hash value            */
  return result;
}

RexxObject *RexxString::andOp(RexxObject *other)
/******************************************************************************/
/* Function:  Logical AND of a string with another logical value              */
/******************************************************************************/
{
  RexxObject *otherTruth;              /* truth value of the other object   */

  required_arg(other, ONE);            /* make sure the argument is there   */
                                       /* validate the boolean              */
  otherTruth = other->truthValue(Error_Logical_value_method) ? TheTrueObject : TheFalseObject;
                                       /* perform the operation             */
  return (!this->truthValue(Error_Logical_value_method)) ? TheFalseObject : otherTruth;
}

RexxObject *RexxString::orOp(RexxObject *other)
/******************************************************************************/
/* Function:  Logical OR of a string with another logical value               */
/******************************************************************************/
{
  RexxObject *otherTruth;              /* truth value of the other object   */

  required_arg(other, ONE);            /* make sure the argument is there   */
                                       /* validate the boolean              */
  otherTruth = other->truthValue(Error_Logical_value_method) ? TheTrueObject : TheFalseObject;
                                       /* perform the operation             */
  return (this->truthValue(Error_Logical_value_method)) ? TheTrueObject : otherTruth;
}

RexxObject *RexxString::xorOp(RexxObject *other)
/******************************************************************************/
/* Function:  Logical XOR of a string with another logical value              */
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

RexxArray *RexxString::makeArray(RexxString *div)
/******************************************************************************/
/* Function:  Split string into an array                                      */
/******************************************************************************/
{
  size_t size = 32;                    /* initial size                      */
  size_t linecount = 0;                /* number of lines                   */
  size_t len;                          /* target string length              */
  char   **lines = OREF_NULL;          /* array holding the single lines    */
  char   *start = this->stringData;
  char   *end   = this->stringData + this->length;
  RexxArray  *result;
  RexxString *entry;
  char   *tmp;                         /* temporary character pointer       */
  char   separator = '\n';             /* default separator                 */

                                       /* special separator given?          */
  if (div != OREF_NULL) {
                                       /* it must be a string object        */
    if (!OTYPE(String, div)) {
      report_exception1(Error_Incorrect_method_nostring, IntegerOne);
    }
                                       /* it must only contain one character*/
    if (div->length > 1) {
      report_exception1(Error_Incorrect_method_pad, div);
    }

                                       /* use the first character           */
    separator = div->stringData[0];
  }
                                       /* if the string to be worked on is  */
                                       /* larger than 2K, try to calculate  */
                                       /* an initial size by assuming an    */
                                       /* average 64 characters per line    */
                                       /* this will reduce the number of    */
                                       /* reallocations...                  */
  if (this->length > 2048) {
    size = this->length / 64;
  }

                                       /* allocate the initial array        */
  lines = (char**) calloc(size,sizeof(char*));

                                       /* dissect string                    */
  while (start < end) {
    tmp = start;
                                       /* search for separator character    */
    while (tmp < end && *tmp != separator) tmp++;
                                       /* insert address of line start      */
    lines[linecount++] = start;
    start = tmp+1;
                                       /* if array is full, double it       */
    if (linecount == size) {
      lines = (char**) realloc(lines,size*2*sizeof(char*));
                                       /* clear out 2nd half                */
      memset(lines+size,0x00,sizeof(char*)*size);
      size *= 2;
    }

  }
  lines[linecount] = end;

  result = new_array(linecount);       /* create the array                  */
  save(result);                        /* protect from garbage collection   */

  for (size = 0; size < linecount; size++) {
    len = lines[size+1] - lines[size];
                                       /* ends with separator?              */
    if (lines[size][len-1] == separator) {
      len--;                           /* don't include separator           */
                                       /* newline separator? remove 0x0d... */
      if (separator == '\n' && lines[size][len-1] == 0x0d) {
        len--;
      }
    }
    entry = new_string(lines[size], len);
    result->put(entry, size+1);
  }

  free(lines);

  discard_hold(result);                /* release the lock                  */

  return result;
}

RexxObject *RexxString::notOp()
/******************************************************************************/
/* Function:  Logical NOT of a string                                         */
/******************************************************************************/
{
  return this->truthValue(Error_Logical_value_method) ? (RexxObject *)TheFalseObject : (RexxObject *)TheTrueObject;
}

RexxObject *RexxString::operatorNot(RexxObject *other)
/******************************************************************************/
/* Function:  Logical NOT of a string                                         */
/******************************************************************************/
{
  return this->truthValue(Error_Logical_value_method) ? (RexxObject *)TheFalseObject : (RexxObject *)TheTrueObject;
}

RexxObject *RexxString::isInteger()
/******************************************************************************/
/* Function:  Test if this string is an integer value                         */
/******************************************************************************/
{
  UCHAR *digitPtr;
  size_t digitsLeft;

    digitPtr = (UCHAR *)this->stringData;
    digitsLeft = this->length;

                                       /* Skip all leading blanks           */
    for (; digitsLeft && (*digitPtr == ch_BLANK || *digitPtr == ch_TAB); ++digitPtr, --digitsLeft) ;

    if (digitsLeft){                   /* Still Digits left ?               */
      if (*digitPtr == ch_PLUS || *digitPtr == ch_MINUS) {
                                       /* need to move past the sign and    */
                                       /*  remove any remaining blanks.     */
       for (++digitPtr, --digitsLeft;
            digitsLeft && (*digitPtr == ch_BLANK || *digitPtr == ch_TAB);
            ++digitPtr, --digitsLeft) ;
                                       /* Yes, skip any blanks              */
       if (!digitsLeft)                /* Did we reach end of data ?        */
                                       /* Yes, not valid                    */
        return (RexxObject *) TheFalseObject;
      }
                                       /* we are now ready to check for     */
                                       /*digits                             */
     for (; digitsLeft && *digitPtr >= ch_ZERO && *digitPtr <= ch_NINE;
           ++digitPtr, --digitsLeft) ;
                                       /* found our first non-digit, or end */
                                       /* is it a decimal point?            */
     if ( digitsLeft && *digitPtr == ch_PERIOD) {
      digitPtr++;                      /* Yes, see if remaining digits are 0*/
      digitsLeft--;
      for (; digitsLeft && *digitPtr == ch_ZERO; ++digitPtr, --digitsLeft) ;
     }
                                       /* if chars left make sure all are   */
                                       /* blanks.                           */
     for (; digitsLeft && (*digitPtr == ch_BLANK || *digitPtr == ch_TAB); ++digitPtr, --digitsLeft) ;
                                       /* skipped all trailing blanks.      */
                                       /* we better be at the end of the    */
                                       /* string, otherwise its invalid.    */
     if (!digitsLeft)
                                       /* yes its the end, return true      */
      return (RexxObject *) TheTrueObject;
    }

                                       /* all other cases are invalid....   */
    return (RexxObject *) TheFalseObject;
}

RexxObject *RexxString::evaluate(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/******************************************************************************/
/* Function:  Polymorphic method that makes string a polymorphic expression   */
/*            term for string literals.                                       */
/******************************************************************************/
{

  stack->push((RexxObject *)this);     /* place on the evaluation stack     */
                                       /* trace if necessary                */
  context->traceIntermediate((RexxObject *)this, TRACE_PREFIX_LITERAL);
  return this;                         /* also return the result            */
}

RexxString *RexxStringClass::newString(const PCHAR string, size_t length)
/******************************************************************************/
/* Function:  Allocate (and initialize) a string object                       */
/******************************************************************************/
{
  RexxString *newString;               /* new string object                 */
  size_t      size2;                   /* allocated size                    */

                                       /* calculate the size                */
                                       /* STRINGOBJ - excess chars (3)      */
                                       /* + length. only sub 3 to allow     */
                                       /* for terminating NULL              */
  size2 = sizeof(RexxString) - (sizeof(char) * 3) + length;
                                       /* allocate the new object           */
  newString = (RexxString *)new_object(size2);
                                       /* set the behaviour from the class*/
  BehaviourSet(newString, TheStringBehaviour);
                                       /* set the virtual function table    */
  setVirtualFunctions(newString, T_string);
                                       /* clear the front part              */
  ClearObjectLength(newString, sizeof(RexxString));
  newString->length = length;          /* save the length                   */
                                       /* Null terminate, allows faster     */
                                       /* conversion to ASCII-Z string      */
  *(newString->stringData + length) = '\0';
                                       /* copy it over                      */
  memcpy(newString->stringData,string,(size_t)length);
  newString->generateHash();           /* generate the hash value           */
                                       /* by  default, we don't need Live   */
  SetObjectHasNoReferences(newString); /*sent                               */
                                       /* NOTE: That if we can set          */
                                       /*  this->NumebrString elsewhere     */
                                       /*we need to mark ourselves as       */
  return newString;                    /*having OREFs                       */
}

RexxString *RexxStringClass::rawString(size_t length)
/******************************************************************************/
/* Function:  Allocate (and initialize) an empty string object                */
/******************************************************************************/
{
  RexxString *newString;               /* new string object                 */
  size_t      size2;                   /* allocated size                    */

                                       /* calculate the size                */
                                       /* STRINGOBJ - excess chars (3)      */
                                       /* + length. only sub 3 to allow     */
                                       /* for terminating NULL              */
  size2 = sizeof(RexxString) - (sizeof(char) * 3) + length;
                                       /* allocate the new object           */
  newString = (RexxString *)new_object(size2);
                                       /* set the behaviour from the class*/
  BehaviourSet(newString, TheStringBehaviour);
                                       /* set the virtual function table    */
  setVirtualFunctions(newString, T_string);
                                       /* clear the front part              */
  ClearObjectLength(newString, sizeof(RexxString));
  newString->length = length;          /* save the length                   */
                                       /* Null terminate, allows faster     */
                                       /* conversion to ASCII-Z string      */
  *(newString->stringData + length) = '\0';
                                       /* by  default, we don't need Live   */
  SetObjectHasNoReferences(newString); /*sent                               */
                                       /* NOTE: That if we can set          */
                                       /*  this->NumebrString elsewhere     */
                                       /*we need to mark ourselves as       */
  return newString;                    /*having OREFs                       */
}

RexxString *RexxStringClass::newCstring(const PCHAR string)
/******************************************************************************/
/* Function:  Create a new string object from a CSTRING                       */
/******************************************************************************/
{                                      /* just return the string value      */
  return (RexxString *)new_string(string, strlen(string));
}


RexxString *RexxStringClass::newDouble(PDBL  number)
/******************************************************************************/
/* Function: Create a string from a double value                              */
/******************************************************************************/
{
  RexxNumberString *numberStringDouble;
                                       /* get double as a number string.    */
  numberStringDouble = (RexxNumberString *)new_numberstring(*number);
                                       /* now convert to string object      */
  return numberStringDouble->stringValue();
}

RexxString *RexxStringClass::newProxy(const PCHAR  string)
/******************************************************************************/
/* Function:  Create a proxy object from this string                          */
/******************************************************************************/
{
  RexxString *sref;
                                       /* The provided source string is null*/
                                       /*  terminated so let class_new      */
                                       /*  compute the length.              */
                                       /* get a new string object           */
  sref = (RexxString *)new_cstring(string);
                                       /* here we need to identify this     */
                                       /*string                             */
  sref->header |= ProxyObject;         /*  as being a proxy object          */

  return sref;
}

RexxString *RexxStringClass::newRexx(RexxObject **init_args, size_t argCount)
/******************************************************************************/
/* Arguments: Subclass init arguments                                         */
/* Function:  Create a new string value (used primarily for subclasses)       */
/******************************************************************************/
{
  RexxString *string;                  /* string value                      */

                                       /* break up the arguments            */
  process_new_args(init_args, argCount, &init_args, &argCount, 1, (RexxObject **)&string, NULL);
                                       /* force argument to string value    */
  string = (RexxString *)REQUIRED_STRING(string, ARG_ONE);
                                       /* create a new string object        */
  string = new_string(string->stringData, string->length);
  BehaviourSet(string, this->instanceBehaviour);
  if (((RexxClass *)this)->uninitDefined()) {
    string->hasUninit();
  }
                                       /* Initialize the new instance       */
  string->sendMessage(OREF_INIT, init_args, argCount);
  return string;                       /* return the new string             */
}

#include "RexxNativeAPI.h"

#define this ((RexxString *)self)

native0 (size_t, STRING_LENGTH)
/******************************************************************************/
/* Function:  External interface to the object method                         */
/******************************************************************************/
{
/******************************************************************************/
/* NOTE:  This method does not reaquire kernel access                         */
/******************************************************************************/
                                       /* forward the method                */
  return this->length;
}

native0 (PCHAR, STRING_DATA)
/******************************************************************************/
/* Function:  External interface to the object method                         */
/******************************************************************************/
{
/******************************************************************************/
/* NOTE:  This method does not reaquire kernel access                         */
/******************************************************************************/
                                       /* forward the method                */
  return this->stringData;
}

nativei1 (REXXOBJECT, STRING_NEWD, PDBL, number)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(new_stringd(number));
}

nativei1 (REXXOBJECT, STRING_NEW_UPPER, PCHAR, string)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(((RexxString *)new_cstring(string))->upper());
}

nativei2 (REXXOBJECT, STRING_NEW, PCHAR, string, size_t, length)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(new_string(string, length));
}

native3 (size_t, STRING_GET, size_t, start, PCHAR, buffer, size_t, bufl)
/******************************************************************************/
/* Function:  External interface to the object method                         */
/******************************************************************************/
{
/******************************************************************************/
/* NOTE:  This method does not reaquire kernel access                         */
/******************************************************************************/
                                       /* just forward and return           */
  return this->get(start, buffer, bufl);
}

