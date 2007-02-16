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
/* REXX Kernel                                                  NumberStringClass.c    */
/*                                                                            */
/* Primitive NumberString Class                                               */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxBuffer.hpp"
#include "RexxActivation.hpp"
#include "NumberStringMath.hpp"
#include "RexxBuiltinFunctions.h"                     /* Gneral purpose BIF Header file    */

                                       /* current global settings           */
extern ACTIVATION_SETTINGS *current_settings;
extern long validMaxWhole[];

/* MHES 20050108 deprecated */
#define string_forwarder(method)\
RexxObject *RexxNumberString::##method(RexxObject *operand)\
 {\
     return (RexxObject *)this->stringValue()->##method(operand);\
 }


/*********************************************************************/
/*  Function:  Convert a number string into a RexxInteger object     */
/*********************************************************************/
long number_create_integer(UCHAR *thisnum, long intlength, int carry, int sign)
{
  long intnum;
  int  numpos;

  if (intlength > 10) {                /* too many digits for integer????   */
    return NO_LONG;
  }
  else {
    intnum = 0;                        /* initialize integer value to 0;    */
    for (numpos = 1 ; (numpos <= intlength); numpos++ ) {
                                       /* compute next digit part.          */
     intnum = (intnum * 10) + (int) *thisnum++ ;
     // on an overflow, this will wrap to a negative number.
     if (intnum < 0)
     {
         return NO_LONG;
     }
    }
  }
  if (carry)                           /* have a carry out condition?       */
    intnum++;                          /* step the number                   */
                                       /* too big?   can't convert          */
  if (intnum < 0 || sign == -1 && intnum > MAXNEGNUM) {
    return NO_LONG;
  }
  else {
    return intnum;                     /* return converted number           */
  }
}

/*********************************************************************/
/*   Function:  Convert the numberstring to ULONG value              */
/*********************************************************************/
int  number_create_uinteger(UCHAR *thisnum, long intlength, int carry, int sign, ULONG *value)
{
  long intnum;
  int  numpos;

  if (intlength > 10) {                /* too many digits for integer????   */
    return FALSE;
  }
  else {
    intnum = 0;                        /* initialize integer value to 0;    */
    for (numpos = 1 ; (numpos <= intlength); numpos++ ) {
                                       /* is number about to overflow       */
     if (intnum > MAXPOSBASE || ((intnum == MAXPOSBASE) && (*thisnum > 5))) {
       return FALSE;                   /* YES, return unconvertable         */
     }
                                       /* compute next digit part.          */
     intnum = (intnum * 10) + (int) *thisnum++ ;
    }
  }

  if (carry)                           /* have a carry out condition?       */
    if (intnum == MAXPOSNUM) {         /* Already at max value?             */
      return FALSE;                    /* Yup, number too big.              */
   }
   else {
    intnum++;                          /* step the number                   */
   }
  *value = intnum;                     /* Assign return value.              */
  return TRUE;                         /* Indicate sucessfule converison.   */
}

RexxNumberString::RexxNumberString(size_t len)
/******************************************************************************/
/* Function:  low level copy of a number string object                        */
/******************************************************************************/
{
   ClearObject(this);
   this->NumDigits = number_digits();
   this->sign = 1;
   this->length = len;
   if (number_form() == FORM_SCIENTIFIC)
      this->NumFlags |= NumFormScientific;
}

RexxNumberString *RexxNumberString::clone()
/******************************************************************************/
/* Function:  low level copy of a number string object                        */
/******************************************************************************/
{
  RexxNumberString *newObject;         /* new copy of the object            */

                                       /* first clone ourselves             */
  newObject = (RexxNumberString *)memoryObject.clone(this);
                                       /* don't keep the original string    */
  OrefSet(newObject, newObject->stringObject, OREF_NULL);
                                       /* or the OVD fields                 */
  OrefSet(newObject, newObject->objectVariables, OREF_NULL);
  return newObject;                    /* return this                       */
}

ULONG   RexxNumberString::hash()
/******************************************************************************/
/* Function:  retrieve the hash value of an integer object                    */
/******************************************************************************/
{
  RexxString * string;                 /* integer string value              */

  if (!OTYPE(NumberString, this))      /*  a nonprimitive object?           */
                                       /* see if == overridden.             */
    return this->sendMessage(OREF_STRICT_EQUAL)->requestString()->hash();
  else {
    if (this->hashvalue == 0) {        /* no hash generated yet?            */
      string = this->stringValue();    /* generate the string value         */
      this->hashvalue = string->hash();/* get the string's hash value       */
    }
    return HASHVALUE(this);            /* return the string hash            */
  }
}

void RexxNumberString::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->objectVariables);
  memory_mark(this->stringObject);
  cleanUpMemoryMark
}

void RexxNumberString::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->objectVariables);
  memory_mark_general(this->stringObject);
  cleanUpMemoryMarkGeneral
}

void RexxNumberString::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxNumberString)

   flatten_reference(newThis->objectVariables, envelope);
   flatten_reference(newThis->stringObject, envelope);

  cleanUpFlatten
}

void RexxNumberString::setString(
    RexxString *stringObj )            /* new string value                  */
/******************************************************************************/
/* Function:  Set the number string's string value                            */
/******************************************************************************/
{
                                       /* set the new string value          */
   OrefSet(this, this->stringObject, stringObj);
   SetObjectHasReferences(this);       /* we now have to garbage collect    */
}

RexxString *RexxNumberString::makeString()
/******************************************************************************/
/* Function:  Handle a REQUEST('STRING') request for a REXX numberstring      */
/******************************************************************************/
{
  return this->stringValue();          /* return the string value           */
}

RexxInteger *RexxNumberString::hasMethod(RexxString *methodName)
/******************************************************************************/
/* Function:  Handle a HASMETHOD request for an integer                       */
/******************************************************************************/
{
                                       /* return the string value's answer  */
  return this->stringValue()->hasMethod(methodName);
}

RexxString *RexxNumberString::primitiveMakeString()
/******************************************************************************/
/* Function:  Handle a REQUEST('STRING') request for a REXX numberstring      */
/******************************************************************************/
{
  if (this->stringObject != OREF_NULL) /* already converted?                */
    return this->stringObject;         /* all finished                      */
  return this->stringValue();          /* return the string value           */
}

RexxString *RexxNumberString::stringValue()
/******************************************************************************/
/* Function:  Convert a number string to a string object                      */
/******************************************************************************/
{
  CHAR  expstring[17], num;
  int    carry;
  size_t NumDigits;
  size_t MaxNumSize, LenValue;
  long   numindex;
  long  temp, ExpValue, ExpFactor;
  size_t charpos;
  RexxString *StringObj;

  if (this->stringObject != OREF_NULL) /* already converted?                */
    return this->stringObject;         /* all finished                      */
                                       /* Start converting the number.      */

 if (this->sign == 0  ) {              /* is the number zero?               */
                                       /* Yes, return a 0 string.           */
   OrefSet(this, this->stringObject, OREF_ZERO_STRING);
   SetObjectHasReferences(this);       /* we now have to garbage collect    */
   return this->stringObject;          /* and return now                    */
 }
 else {                                /*  No, convert the number.          */
                                       /* convert the exponent numeber into */
                                       /* string.                           */

   NumDigits = this->NumDigits;        /* Get Digits settings at creation   */

   ExpValue = this->exp;               /* Working copy of exponent          */
   LenValue = this->length;            /* Working copy of the length.       */

                                       /* If no exponent                    */
   if (ExpValue == 0) {
                                       /* Yes, we can fast path conversion  */
    MaxNumSize = LenValue;             /* Size of result is length of number*/
    if (this->sign <0)                 /* if number is negative.            */
     MaxNumSize++;                     /* result one bigger for - sign.     */
                                       /* Get new string, of exact length,  */
    StringObj = (RexxString *)raw_string(MaxNumSize);
    charpos = 0;                       /* data in string to start.          */
    if (this->sign < 0) {              /* If number is neagative            */
                                       /*  add negative sign and bump index */
     StringObj->putChar(charpos++, ch_MINUS);
    }
                                       /* For each number digits in number  */
    for(numindex=0; (size_t)numindex < LenValue; numindex++) {
                                       /* place char rep in NumString       */
     num = this->number[numindex] + ch_ZERO;
     StringObj->putChar(charpos++, num);
    }                                  /* Done with Fast Path....           */
    StringObj->generateHash();         /* done building the string          */
   }
   else {                              /* We need to do this the long way   */
    carry = 0;                         /* assume rounding-up NOT necessary  */

                                       /*is number just flat out too big?   */
    if ((( ExpValue + (long)LenValue - 1) > MAXNUM) ||
        (ExpValue < (-MAXNUM)) )       /* Yes, report Overflow error.       */
      report_exception1(Error_Conversion_operator, this);


    ExpFactor = 0;                     /* number not eponential yet..       */
    temp = ExpValue + (long)LenValue - 1;  /* get size of this number           */
    expstring[0] = '\0';               /* string vlaue of exp factor, Null  */
                                       /* is left of decimal > NumDigits or */
    if ((temp >= (long)NumDigits) ||   /* exponent twice numDigits          */
        ((size_t)labs(ExpValue) > (NumDigits*2)) ) {
                                       /* Yes, we need to go exponential.   */
                                       /* we need Engineering format?       */
     if (!(this->NumFlags & NumFormScientific)) {
      if (temp < 0)                    /* Yes, is number a whole number?    */
       temp -= 2;                      /* force 2 char adjustment to left   */
      temp = (temp/3) * 3;             /* get count to the right of Decimal */
     }
     if (labs(temp) > MAXNUM) {        /* is adjusted number too big?       */
       if (temp > MAXNUM)              /* did it overflow?                  */
                                       /* Yes, report overflow error.       */
         report_exception2(Error_Overflow_expoverflow, new_integer(temp), IntegerNine);
       else
                                       /* Actually an underflow error.      */
         report_exception2(Error_Overflow_expunderflow, new_integer(temp), IntegerNine);
     }
     ExpValue -= temp;                 /* adjust the exponent               */
     if ( temp != 0 ) {                /* do we still have exponent ?       */
        ExpFactor = TRUE;              /* Save the factor                   */
     } else {
        ExpFactor = FALSE;             /* no need to save the factor        */
     } /* endif */

     if (temp < 0) {
       strcpy(expstring, "E");
                                       /* convert exponent value into string*/
       _ltoa((long)temp, expstring+1, 10);
     }
     else if (temp > 0) {
       strcpy(expstring, "E+");
                                       /* convert exponent value into string*/
       _ltoa((long)temp, expstring+2, 10);
     }
     temp = labs((INT)temp);           /* get positive exponent factor      */

    }
                                       /* Now compute size of result string */
    if (ExpValue >= 0 )                /* if the Exponent is positive       */
                                       /* Size is length of number plus exp.*/
     MaxNumSize = (size_t)ExpValue + LenValue;
                                       /*is exponent larger than length,    */
    else if ((size_t)labs(ExpValue) >= LenValue)
                                       /* Yes, we will need to add zeros to */
                                       /* the front plus a 0.               */
      MaxNumSize = labs(ExpValue) + 2;

    else                               /*Won't be adding any digits, just   */
     MaxNumSize = LenValue + 1;        /* length of number + 1 for decimal  */

    if (ExpFactor)                     /* Are we using Exponential notation?*/
                                       /* Yes, need to add in size of the   */
     MaxNumSize += strlen(expstring);  /* exponent stuff.                   */

    if (this->sign <0)                 /* is number negative?               */
     MaxNumSize++;                     /* yes, add one to size for - sign   */
                                       /* get new string of appropriate size*/
    StringObj = (RexxString *)raw_string(MaxNumSize);

    charpos = 0;                       /* set starting position             */
    if (this->sign < 0) {              /* Is the number negative?           */
                                       /* Yes, add in the negative sign.    */
     StringObj->putChar(charpos, ch_MINUS);
    }
    temp = ExpValue + (long)LenValue;  /* get the adjusted length.          */

                                       /* Since we may have carry from round*/
                                       /* we'll fill in the number from the */
                                       /* back and make our way forward.    */

                                       /* Strindex points to exponent start */
                                       /* part of string.                   */
    charpos  = MaxNumSize - strlen(expstring);

    if (ExpFactor)                     /* will result have Exponent?        */
                                       /* Yes, put the data into the string.*/
     StringObj->put(charpos, expstring, strlen(expstring));

                                       /* Next series of If's will determine*/
                                       /* if we need to add zeros to end    */
                                       /* of the number and where to place  */
                                       /* decimal, fill in string as we     */
                                       /* go, also need to check for a carry*/
                                       /* if we rounded the number early on.*/

    if (temp <= 0) {                   /* Is ther an Integer portion?       */
                                       /*   0.000000xxxx result.            */
                                       /*   ^^^^^^^^     filler             */

                                       /* Start filling in digits           */
                                       /*   from the back....               */
      for (numindex = (long)(LenValue-1);numindex >= 0 ;numindex-- ) {
                                       /* are we carry from round?          */
       num = this->number[numindex];   /* working copy of this Digit.       */
       num = num + ch_ZERO;            /* now put the number as a character */
       StringObj->putChar(--charpos, num);
      }
      temp = -temp;                    /* make the number positive...       */

      if (temp) {
       charpos  -= temp;               /* yes, point to starting pos to fill*/
                                       /* now fill in the leading Zeros.    */
       StringObj->set(charpos, ch_ZERO, temp);
      }
      StringObj->putChar(--charpos, ch_PERIOD);
      if (carry)                       /* now put in the leading 1. is carry*/
       StringObj->putChar(--charpos, ch_ONE);
      else                             /* or 0. if no carry.                */
       StringObj->putChar(--charpos, ch_ZERO);
      StringObj->generateHash();       /* done building the string          */
    }
    /* do we need to add zeros at end?   */
    else if ((size_t)temp >= LenValue) {
                                       /*  xxxxxx000000  result             */
                                       /*        ^^^^^^  filler             */

                                       /* Yes, add zeros first.             */
     temp -= LenValue;                 /* see how many zeros we need to add.*/
     charpos  -= temp;                 /* point to starting pos to fill     */
                                       /* now fill in the leading Zeros.    */
     StringObj->set(charpos, ch_ZERO, temp);
                                       /* done w/ trailing zeros start      */
                                       /* adding digits (from back)         */


                                       /* start filling in digits           */
      for (numindex = (long)LenValue-1;numindex >= 0 ;numindex-- ) {
       num = this->number[numindex];   /* working copy of this Digit.       */
       num = num + ch_ZERO;            /* now put the number as a character */
       StringObj->putChar(--charpos, num);
      }
    }                                  /* done with this case....           */

    else {                             /* we have a partial Decimal number  */
                                       /* add in the first digit a 1.       */
                                       /* Start filling in digits           */
      for (numindex = (long)LenValue - 1;numindex > temp - 1 ;numindex-- ) {
       num = this->number[numindex];   /* working copy of this Digit.       */
                                       /* now put the number as a character */
       num += ch_ZERO;
       StringObj->putChar(--charpos, num);
      }
                                       /* add in the decimal point.         */
     StringObj->putChar(--charpos, ch_PERIOD);

                                       /* Start filling in digits           */
                                       /* add numbers before decimal point  */
      for (numindex = temp - 1;numindex >= 0 ;numindex-- ) {
       num = this->number[numindex];   /* working copy of this Digit.       */
       num += ch_ZERO;
       StringObj->putChar(--charpos, num);
      }
                                       /* end of final case, conversion done*/
    }
   }                                   /* end of non-fast path conversion.  */
  }                                    /* End of conversion of number       */
  StringObj->generateHash();           /* force the object to have a hash   */
                                       /* since string is created from      */
                                       /* number string, we can set the     */
  StringObj->setNumberString(this);    /* lookaside right away              */
                                       /* and also save this string         */
  OrefSet(this, this->stringObject, StringObj);
  SetObjectHasReferences(this);        /* we now have to garbage collect    */
  return StringObj;                    /* all done, return new string       */
}

long RexxNumberString::longValue(size_t digits)
/******************************************************************************/
/* Function:  Convert a number string to a long value                         */
/******************************************************************************/
{
  UCHAR *num;
  BOOL carry;
  long intnum;
  long numexp;
  size_t numlength, numpos, NumDigits;
  UCHAR compareChar;

  if (this->sign == 0 )                /* is the number zero ??             */
    return 0;                          /*  yes, return right away           */
  else {                               /*  no, check and make sure number is*/
                                       /*   a integer type number           */

    if (digits == NO_LONG) {           /* were we passed a value to use for */
                                       /* digits?                           */
      NumDigits = number_digits();     /* No, Get current digits setting.   */
      NumDigits = min(NumDigits, 9);   /* 9 is max for default digits.      */
    } else {
      NumDigits = digits;              /* Yes, we will use this setting.    */
    } /* endif */

    if (this->length > NumDigits) {    /* is number bigger than Digits      */
                                       /* Yes need to adjust number down.   */
     numexp = this->exp + (this->length-NumDigits);
     numlength = NumDigits;

                                       /* is MSD of numbers being trunc     */
     if (*(this->number + numlength) >= 5)
                                       /* Great/equal to 5?  Rounding?      */
      carry = TRUE;                    /* Yes, indocate rounding for later. */
     else
      carry = FALSE;                   /* Nope, make sure we don't round    */


    } else {                           /* current length is OK. set         */
     numlength = this->length;         /*  length and                       */
     numexp = this->exp;               /*  exp values to numbers.           */
     carry = FALSE;                    /* We don't have a carry.            */
    }
    if (numexp < 0)  {                 /* Is the exponet negative?          */
     numpos    = -numexp;              /* Get length of num after decimal   */

     if (carry)                        /* Did we have a carry condition?    */
                                       /* Any 'implied' zero's between      */
                                       /* decimal point and 1st digit?      */
       if (numpos == numlength)
         compareChar = 9;              /* all digits after decimal must     */
                                       /* be 9. to carry forward            */
       else
         return NO_LONG;               /* yes, not a valid whole number.    */
     else
       compareChar = '\0';             /* other wise all must be 0.         */

     if (numpos >= numlength ) {       /* all of number after the decimanl  */
       numpos = numlength;
       num = (PUCHAR) this->number;    /* start looking at 1st digit        */
     }
     else
                                       /* set num to 1st digit after decimal*/
       num = (PUCHAR) this->number + numlength + numexp;

     for ( ; numpos ; numpos--) {      /* look at all digits for vaility.   */
      if ( *num++ != compareChar)      /* this the one we're looking for    */
        return  NO_LONG;               /*   nope, not a valid integer.      */
     }

     if (-numexp >= (long)numlength)   /* are we carrying                   */
                                       /* and since we now know all digits  */
                                       /* are 0 or 9 (if carry), we know the*/
      if (carry)                       /* answer now.                       */
        return 1;                      /* if carry then its a 1.            */
      else
        return 0;                      /* otherwise its a zero.             */

     intnum = number_create_integer((PUCHAR) this->number, numlength + numexp, carry, this->sign);
     if (intnum == NO_LONG) {          /* Is numebr too big ????            */
      return NO_LONG;                  /* Yes, return invalid integer....   */
     }

    }
    else {                             /* straight out number. just compute.*/
     intnum = number_create_integer((PUCHAR) this->number, numlength, carry, this->sign);
     if (intnum == NO_LONG) {          /* Is numebr too big ????            */
      return NO_LONG;                  /* Yes, return invalid integer....   */
     }

     if (numexp > 0 ) {                /* need to add zeros to the end?     */
                                       /* Yes, see how many we need to add  */
                                       /* make sure don't expand past size  */
                                       /*  of a long....                    */
      for (numpos = 1 ;numpos <= (size_t)numexp &&
                       ((this->sign == 1 && intnum < MAXPOSBASE) ||
                        (this->sign == -1 && intnum < MAXNEGBASE));
           numpos++ ) {
       intnum *= 10;                   /*  Add one zero to end of integer   */
      }

      if (numpos <= (size_t)numexp) {  /* did number exceed limit??         */
       return NO_LONG;                 /* yes, return error.                */
      }
     }
    }
                                       /* is long value expressable as a    */
                                       /*  whole number in REXX term.       */
    if (NumDigits <= 9 && intnum >= validMaxWhole[NumDigits -1]) {
      return NO_LONG;                  /* nope, not a valid long.           */
    }

                                       /* If number is negative, make int   */
                                       /* neg                               */
    if (this->sign == -1)
     intnum = -intnum;
    return intnum;                     /* return INTEGER object.            */
  }
}

RexxInteger *RexxNumberString::integerValue(
    size_t digits )                    /* required precision                */
/******************************************************************************/
/* Function:  convert a number string to an integer object                    */
/******************************************************************************/
{
  long integerNumber;                  /* converted value                   */

                                       /* get the long value?               */
  integerNumber = this->longValue(digits);
  if (integerNumber == NO_LONG)        /* no good?                          */
    return (RexxInteger *)TheNilObject;/* just return .nil                  */
  else
    return new_integer(integerNumber); /* convert to an integer object      */
}

double   RexxNumberString::doubleValue()
/******************************************************************************/
/* Function:  Convert a number string to a double                             */
/******************************************************************************/
{
 RexxString *string;                   /* string version of the number      */
 double doubleNumber;                  /* converted value                   */

 string = this->stringValue();         /* get the string value              */
                                       /* convert the number                */
 doubleNumber = strtod(string->stringData, NULL);
                                       /* out of range?                     */
 if (doubleNumber == +HUGE_VAL || doubleNumber == -HUGE_VAL)
   return NO_DOUBLE;                   /* got a bad value                   */
 else
   return doubleNumber;                /* return the converted value        */
}

BOOL  RexxNumberString::truthValue(
    LONG  errorcode )                  /* error to raise if not good        */
/******************************************************************************/
/* Function:  Return a truth value boolean for a number string                */
/******************************************************************************/
{
  if (this->sign == 0 )                /* exactly zero?                     */
    return FALSE;                      /* done quickly                      */
                                       /* not exactly 1?                    */
  else if (!(this->sign == 1 && this->exp == 0 && this->length == 1L && *(this->number) == 1))
    report_exception1(errorcode, this);/* report the error                  */
  return TRUE;                         /* this is TRUE                      */
}

BOOL numberStringScan(PCHAR number, size_t length)
/******************************************************************************/
/* Arguments:  Number data, number length                                     */
/* Function :Scan the string to determine if its a valid number               */
/* Returned :  0 if input was valid number                                    */
/*             1 if input was invalid number                                  */
/******************************************************************************/
{
 UCHAR     ch;                         /* current character                 */
 PUCHAR    InPtr;                      /* Input Data Pointer                */
 PUCHAR    EndData;                    /* Scan end position                 */
 BOOL      hadPeriod;                  /* had a decimal point already       */

                                       /* for efficiency, this code takes   */
                                       /* advantage of the fact that REXX   */
                                       /* string object's all have a guard  */
                                       /* null character on the end         */

   if (!length) {                      /* Length zero not a number?         */
    return TRUE;                       /* a null string is not a number     */
   }

   hadPeriod = FALSE;                  /* period yet                        */
   InPtr = (PUCHAR) number;            /*Point to start of input string.    */
   EndData = InPtr + length;           /*Point to end of Data + 1.          */

   while (*InPtr == ch_BLANK)          /* Skip all leading blanks.          */
     InPtr++;                          /* Skip it, and go on to next char   */
                                       /* Is this a sign Character?         */
   if ((ch = *InPtr) == ch_MINUS || ch == ch_PLUS) {
     InPtr++;                          /* Yes, skip it.                     */
     while (*InPtr == ch_BLANK)        /* Ship all leading blanks.          */
       InPtr++;                        /* Skip it, and go on to next char   */
   }

   if (*InPtr == ch_PERIOD) {          /* got a leading period?             */
     InPtr++;                          /* step over it                      */
     hadPeriod = TRUE;                 /* got the decimal point already     */
   }

   ch = *InPtr;                        /* Get 1st Digit.                    */
   if (ch < ch_ZERO || ch > ch_NINE)   /* Is this a valid digit?            */
     return TRUE;                      /* Nope, bad number                  */
   else {
                                       /*Skip all leading Zero's            */
     while (*InPtr == ch_ZERO)         /* While 1st Digit is a 0            */
       InPtr++;                        /* Go to next character.             */
                                       /* Have we reach end of number,num   */
                                       /*zero?                              */
     if (InPtr >= EndData)
       return FALSE;                   /* valid number... all Zeros         */
   }
                                       /* While the character is a Digit.   */
   while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE)
     InPtr++;                          /* Go to next digit                  */
   if (InPtr >= EndData)               /* Did we reach end of data?         */
       return FALSE;                   /* all done, just return valid number*/

   if (*InPtr == ch_PERIOD) {          /*Decimal point???                   */
     if (hadPeriod)                    /* already had one?                  */
       return TRUE;                    /* yep, this is a bad number         */
     InPtr++;                          /* yes, skip it.                     */
                                       /* While the character is a Digit.   */
     while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE)
       InPtr++;                        /* Go to next digit                  */
     if (InPtr >= EndData)             /* Did we reach end of data          */
       return FALSE;                   /* this was fine                     */
   }

   if (toupper(*InPtr) == 'E') {       /* See if this char is an exponent?  */
    if (++InPtr >= EndData)            /* Yes, but did we reach end of input*/
                                       /* Yes, invalid number.              */
      return TRUE;
                                       /* If this a plus/minus sign?        */
    if ((*InPtr == ch_MINUS) || (*InPtr == ch_PLUS))
      InPtr++;                         /*  go on to next char.              */
    if (InPtr >= EndData)              /* reach end of Input ?              */
      return TRUE;                     /* Yes, invalid number.              */
                                       /* If this char a valid digit?       */
    if (*InPtr < ch_ZERO || *InPtr > ch_NINE)
      return TRUE;                     /* No,  invalid number.              */
                                       /* Do while we have a valid digit    */
    while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE)
      InPtr++;                         /* Yup, go to next one and check     */
   }
                                       /* At this point all that should be  */
                                       /* left Are trailing blanks.         */
   while (*InPtr == ch_BLANK)          /* Skip all trailing blanks          */
     InPtr++;                          /* Skip it, and go on to next char   */
   if (InPtr >= EndData)               /* Did we reach end of data          */
     return FALSE;                     /* this was fine                     */
   return TRUE;                        /* wasn't a valid number             */
}

void fill_digits(                      /* create a string of digits         */
  PCHAR  outPtr,                       /* output location                   */
  PCHAR  number,                       /* start of string of digits         */
  size_t count )                       /* size of resulting string          */
/******************************************************************************/
/* Function : Copy "count" digits of a number to the desired location,        */
/*            converting them back to character digits                        */
/******************************************************************************/
{
  while (count--)                      /* while still have characters       */
    *outPtr++ = *number++ + '0';       /* convert back to character         */
}

RexxObject *RexxNumberString::trunc(
  RexxObject *decimal_digits)          /* number of decimal digits        */
/******************************************************************************/
/* Function:  Truncate a number to given decimal digit count                  */
/******************************************************************************/
{
  size_t  needed_digits;               /* digits to truncate to             */

                                       /* get the decimal count             */
  needed_digits = optionalNonNegative(decimal_digits, 0, ARG_ONE);
                                       /* round to current digits setting   */
  return this->prepareNumber(number_digits(), ROUND)->truncInternal(needed_digits);
}

RexxObject *RexxNumberString::truncInternal(
  size_t needed_digits)                /* number of decimal digits          */
/******************************************************************************/
/* Function:  Truncate a number to given decimal digit count                  */
/******************************************************************************/
{
  RexxString *result;                  /* returned result                   */
  LONG    temp;                        /* temporary string value            */
  long    integer_digits;              /* leading integer digits            */
  size_t  size;                        /* total size of the result          */
  LONG    sign;                        /* current sign indicator            */
  PCHAR   resultPtr;                   /* result pointer                    */

  if (this->sign == 0) {               /* is the number zero?               */
    if (needed_digits == 0)            /* no decimals requested?            */
                                       /* just return a zero                */
      return IntegerZero;
    else {                             /* need to pad                       */
                                       /* get an empty string               */
      result = (RexxString *)raw_string(needed_digits + 2);
                                       /* get a data pointer                */
      resultPtr = result->stringData;
      strcpy(resultPtr, "0.");         /* copy the leading part             */
                                       /* fill in the trailing zeros        */
      memset(resultPtr + 2, '0', needed_digits);
                                       /* generate a hash value             */
      result->generateHash();
      return result;                   /* return the result                 */
    }
  }
  else {                               /* have to do real formatting        */
    size = 0;                          /* start with nothing                */
    sign = this->sign;                 /* copy the sign                     */
                                       /* calculate the leading part        */
                                       /* number have a decimal part?       */
    if (this->exp > 0) {
                                       /* add in both length and exponent   */
      size += this->length + this->exp;
      if (needed_digits != 0)          /* have digits required?             */
        size += needed_digits + 1;     /* add in the digits and the decimal */
    }
    else {                             /* number has a decimal part.        */
                                       /* get the leading part              */
      integer_digits = (long)this->length + this->exp;
      if (integer_digits > 0) {        /* something on the left hand side?  */
        size += integer_digits;        /* add in these digits               */
        if (needed_digits != 0)        /* decimals requested?               */
          size += needed_digits + 1;   /* add in the digits and the decimal */
      }
      else {                           /* no leading part                   */
        if (needed_digits == 0)        /* nothing wanted after decimal?     */
          return IntegerZero;          /* this is just zero then            */
                                       /* do we need to pad more zeros than */
                                       /*  number we want after the decimal?*/
        if ((long)needed_digits <= -integer_digits) {
          size = needed_digits + 2;    /* result is formatted zero...no sign*/
          sign = 0;                    /* force the sign out                */
        }
        else
          size += needed_digits + 2;   /* calculate the decimal size        */
      }
    }
    if (sign < 0)                      /* negative number?                  */
      size++;                          /* start with a sign                 */
                                       /* get an empty pointer              */
    result = (RexxString *)raw_string(size);
                                       /* point to the data part            */
    resultPtr = result->stringData;
    if (sign < 0)                      /* negative number?                  */
      *resultPtr++ = '-';              /* start with a sign                 */
                                       /* calculate the leading part        */
                                       /* number have a decimal part?       */
    if (this->exp > 0) {
                                       /* fill in the digits                */
      fill_digits(resultPtr, (PCHAR)this->number, this->length);
      resultPtr += this->length;       /* step over the length              */
                                       /* now fill in the extra zeros       */
      memset(resultPtr, '0', this->exp);
      resultPtr += this->exp;          /* and the exponent                  */
      if (needed_digits != 0) {        /* decimals requested?               */
        *resultPtr++ = '.';            /* add a trailing decimal point      */
                                       /* copy on the trailers              */
        memset(resultPtr, '0', needed_digits);
      }
    }
    else {                             /* number has a decimal part.        */
      integer_digits = this->length + this->exp;
      if (integer_digits > 0) {        /* something on the left hand side?  */
                                       /* add the integer digits            */
        fill_digits(resultPtr, (PCHAR)this->number, integer_digits);
        resultPtr += integer_digits;   /* step over the digits              */
        if (needed_digits != 0) {      /* decimals requested?               */
          *resultPtr++ = '.';          /* add a trailing decimal point      */
                                       /* get count to add                  */
          temp = min(needed_digits, this->length - integer_digits);
                                       /* fill in the digits                */
          fill_digits(resultPtr, (PCHAR)this->number + integer_digits, temp);
          resultPtr += temp;           /* step over the digits              */
          needed_digits -= temp;       /* adjust the length                 */
          if (needed_digits != 0)      /* still need more?                  */
                                       /* copy on the trailers              */
            memset(resultPtr, '0', needed_digits);
        }
      }
      else {                           /* no leading part                   */
                                       /* do we need to pad more zeros than */
                                       /*  number we want after the decimal?*/
        if ((long)needed_digits <= -integer_digits) {
          strcpy(resultPtr, "0.");     /* copy a leading zero part          */
          resultPtr += 2;              /* step over                         */
                                       /* copy on the trailers              */
          memset(resultPtr, '0', needed_digits);
        }
        else {
          strcpy(resultPtr, "0.");     /* copy a leading zero part          */
          resultPtr += 2;              /* step over                         */
                                       /* copy on the trailers              */
          memset(resultPtr, '0', -integer_digits);
          resultPtr += -integer_digits;/* step over the digits              */
          needed_digits += integer_digits; /* reduce needed_digits          */
                                       /* get count to add                  */
          temp = min(needed_digits, this->length);
                                       /* fill in the digits                */
          fill_digits(resultPtr, (PCHAR)this->number, temp);
          resultPtr += temp;           /* step over the digits              */
          needed_digits -= temp;       /* adjust the length                 */
          if (needed_digits != 0)      /* still need more?                  */
                                       /* copy on the trailers              */
            memset(resultPtr, '0', needed_digits);
        }
      }
    }
                                       /* go finish off the string          */
    result->generateHash();
  }
  return result;                       /* return the formatted number       */
}

RexxString  *RexxNumberString::formatRexx(
  RexxObject *Integers,                /* space for integer part            */
  RexxObject *Decimals,                /* number of decimals required       */
  RexxObject *MathExp,                 /* the exponent size                 */
  RexxObject *ExpTrigger )             /* the exponent trigger              */
/******************************************************************************/
/* Function : Format the numberstring data according to the format            */
/*            function controls.                                              */
/******************************************************************************/
{
  size_t integers;                     /* integer space requested           */
  size_t decimals;                     /* decimal space requested           */
  size_t mathexp;                      /* exponent space requested          */
  size_t exptrigger;                   /* exponential notation trigger      */
  size_t digits;                       /* current numeric digits            */
  LONG   form;                         /* current numeric form              */

  digits = number_digits();            /* get the current digits value      */
  form = number_form();                /* and the exponential form          */
                                       /* get the space for integer part    */
  integers = optionalNonNegative(Integers, -1, ARG_ONE);
                                       /* and the decimal part              */
  decimals = optionalNonNegative(Decimals, -1, ARG_TWO);
                                       /* also the exponent size            */
  mathexp = optionalNonNegative(MathExp, -1, ARG_THREE);
                                       /* and the trigger                   */
  exptrigger = optionalNonNegative(ExpTrigger, digits, ARG_FOUR);
                                       /* round to current digits setting   */
  return this->prepareNumber(digits, ROUND)->formatInternal(integers, decimals, mathexp, exptrigger, this, digits, form);
}

RexxString *RexxNumberString::formatInternal(
  size_t      integers,                /* space for integer part          */
  size_t      decimals,                /* number of decimals required     */
  size_t      mathexp,                 /* the exponent size               */
  size_t      exptrigger,              /* the exponent trigger            */
  RexxNumberString *original,          /* oringial NumStr                 */
  size_t      digits,                  /* digits to format to             */
  LONG        form)                    /* form to format to               */
/******************************************************************************/
/* Function : Format the numberstring data according to the format            */
/*            function controls.                                              */
/******************************************************************************/
{
  LONG   expfactor;                    /* actual used exponent              */
  LONG   temp;                         /* temporary calculation holder      */
  size_t exponentsize;                 /* size of the exponent              */
  UCHAR  exponent[15];                 /* character exponent value          */
  LONG   adjust;                       /* exponent adjustment factor        */
  size_t size;                         /* total size of the result          */
  size_t leadingSpaces;                /* number of leading spaces          */
  size_t leadingZeros;                 /* number of leading zeros           */
  size_t leadingExpZeros;              /* number of leading zeros in exp    */
  size_t trailingZeros;                /* number of trailing zeros          */
  size_t reqIntegers;                  /* requested integers                */
  RexxString *result;                  /* final formatted number            */
  PCHAR  resultPtr;                    /* pointer within the result         */
  BOOL   defaultexpsize = FALSE;       /* default exponent size             */

  expfactor = 0;                       /* not exponential yet               */

  if (mathexp != 0) {                  /* Is exponential allowed?           */
                                       /* calculate the exponent factor     */
    temp = this->exp + this->length - 1;
                                       /* is left of dec>digits             */
                                       /* or twice digits on right          */
    if (temp >= (long)exptrigger || labs(this->exp) > (long)(exptrigger * 2)) {
      if (form == FORM_ENGINEERING) {  /* request for Engineering notation? */
        if (temp < 0)                  /* yes, is it a whole number?        */
          temp = temp - 2;             /* no, force two char left adjustment  -2 instead of -1 */
        temp = (temp / 3) * 3;         /* get count right of decimal point  */
      }
      this->exp = this->exp - temp;    /* adjust the exponent               */
      expfactor = temp;                /* save the factor                   */
      temp = labs((INT)temp);          /* get positive exponent value       */
                                       /* format exponent to a string       */
      _ltoa((long)temp, (PCHAR)exponent, 10);
                                       /* get the number of digits needed   */
      exponentsize = strlen((PCHAR)exponent);
      if (mathexp == -1) {             /* default exponent size?            */
        mathexp = exponentsize;        /* use actual length                 */
        defaultexpsize = TRUE;         /* default exponent size on          */
      }
      if (exponentsize > mathexp)      /* not enough room?                  */
        report_exception2(Error_Incorrect_method_exponent_oversize, original, new_integer(mathexp));
    }
  }

  if (decimals == -1) {                /* default decimal processing?       */
    if (this->exp < 0)                 /* negative exponent?                */
      decimals = -this->exp;           /* get number of decimals            */
  }
  else {
    if (this->exp < 0) {               /* have actual decimals?             */
      adjust = -this->exp;             /* get absolute value                */
      if ((size_t)adjust > decimals) { /* need to round or truncate?        */
        adjust = adjust - decimals;    /* get the difference                */
                                       /* adjust exponent                   */
        this->exp = this->exp + adjust;
        if (adjust >= (long)this->length) {  /* Losing all digits?  need rounding */
                                       /* is rounding needed?               */
          if (adjust == (long)this->length && this->number[0] >= 5)
            this->number[0] = 1;       /* round up                          */
          else {
            this->number[0] = 0;       /* round down                        */
            this->exp = 0;             /* nothing left at all               */
            this->sign = 1;            /* suppress a negative sign          */
          }
          this->length = 1;            /* just one digit left               */
        }
                                       /* Need to round?                    */
        else {                         /* truncating, need to check rounding*/
          temp = decimals - adjust;    /* get the difference                */
                                       /* adjust the length                 */
          this->length = this->length - adjust;
                                       /* go round this number              */
          this->mathRound(this->number);
                                       /* calculate new adjusted value      */
          /* undo the previous exponent calculation                         */
                                       /* needed for format(.999999,,4,2,2) */
          if (mathexp != 0 && expfactor != 0)
          {
            this->exp += expfactor;
            expfactor = 0;
            _ltoa((long)0, (PCHAR)exponent, 10);
            exponentsize = strlen((PCHAR)exponent);
          }

          temp = this->exp + this->length - 1;

                                       /* did rounding trigger the          */
                                       /* exponential form?                 */
          if (mathexp != 0 && (temp >= (long)exptrigger || (size_t)labs(this->exp) > exptrigger * 2)) {
                                       /* yes, request for                  */
            if (form == FORM_ENGINEERING) {
                                       /* Engineering notation fmt?         */
              if (temp < 0)            /* yes, is it a whole number?        */
                temp = temp - 2;       /* no, force two char adjust to left */
              temp = (temp / 3) * 3;   /* get count right of decimal point  */
            }
                                       /* adjust the exponent               */
            this->exp = this->exp - temp;
                                       /* adjust the exponent factor        */
            expfactor = expfactor + temp;
                                       /* format exponent to a string       */
            _ltoa(labs((INT)expfactor), (PCHAR)exponent, 10);
                                       /* get the number of digits needed   */
            exponentsize = strlen((PCHAR)exponent);

            if (mathexp == -1)         /* default exponent size?            */
              mathexp = exponentsize;  /* use actual length                 */
            if (exponentsize > mathexp)/* not enough room?                  */
               report_exception2(Error_Incorrect_method_exponent_oversize, original, new_integer(mathexp));
          }
        }
      }
    }
  }

  if (integers == -1) {                /* default integers requested        */
    if (this->exp >= 0)                /* non-negative exponent?            */
                                       /* use all of number                 */
      integers = this->length + this->exp;
    else {
                                       /* no integer part?                  */
      if ((size_t)labs(this->exp) > this->length)
        integers = 1;                  /* just the leading zero             */
      else                             /* get the integer part              */
        integers = this->length + this->exp;
    }
  }
  else {                               /* user requested size               */
    reqIntegers = integers;            /* save integers                     */
    if (this->sign == -1)              /* negative number?                  */
      integers = integers - 1;         /* the sign takes up one spot        */
    if (this->exp >= 0)                /* non-negative exponent?            */
      temp = this->length + this->exp; /* use all of number                 */
    else {
                                       /* no integer part?                  */
      if ((size_t)labs(this->exp) > this->length)
        temp = 1;                      /* just the leading zero             */
      else
                                       /* get the integer part              */
        temp = this->length + this->exp;
    }
    if ((long)integers < temp)         /* not enough room?                  */
                                       /* this is an error                  */
      report_exception2(Error_Incorrect_method_before_oversize, original, new_integer(reqIntegers));
  }

  size = 0;                            /* start with a null string          */
  leadingSpaces = 0;                   /* no leading spaces yet             */
  temp = this->exp + this->length;     /* get adjusted length               */
  if (temp != (long)integers) {        /* need leading spaces?              */
    if (temp > 0)                      /* have leading part?                */
      leadingSpaces = integers - temp; /* get leading length                */
    else
      leadingSpaces = integers - 1;    /* leave space for leading 0         */
    size += leadingSpaces;             /* fill in the spaces                */
  }
  if (this->sign == -1)                /* negative number?                  */
    size++;                            /* space for the sign                */

  if (temp <= 0) {                     /* no integer portion?               */
    size += 2;                         /* add room for zero and decimal     */
    leadingZeros = -temp;              /* get positive of zeros             */
    size += leadingZeros;              /* add in the zeros size             */
    if (this->length > 0)              /* have a length?                    */
      size += this->length;            /* add on the actual data            */
                                       /* need more zeros?                  */
    if (leadingZeros + this->length < decimals) {
                                       /* get the trailing count            */
      trailingZeros = decimals - (leadingZeros + this->length);
      size += trailingZeros;           /* add them on                       */
    }
    else
      trailingZeros = 0;               /* no trailing zeros                 */
  }
  else if (temp >= (long)this->length) { /* all integer data?                 */
    size += this->length;              /* add on the digits                 */
                                       /* reduce total length               */
    trailingZeros = temp - this->length;
    size += trailingZeros;             /* add this to the total size        */
    if (decimals > 0)                  /* decimals needed?                  */
      size += decimals + 1;            /* add decimal point and trailers    */
  }
  else {                               /* partial decimal number            */
    size += this->length + 1;          /* need the length plus a decimal    */
                                       /* get needed extra zeroes           */
    trailingZeros = decimals - (this->length - temp);
    size += trailingZeros;             /* add that to the size              */
    if ((long) trailingZeros<0)
    {
      this->length += trailingZeros;
      this->exp -= trailingZeros;
      trailingZeros = 0;
    }
  }

  if (expfactor != 0) {                /* exponential value?                */
    size += 2;                         /* add on the E and the sign         */
                                       /* get extra size needed             */
    leadingExpZeros = mathexp - exponentsize;
    size += mathexp;                   /* add on the total exponent size    */
  }
                                       /* spaces needed for exp.?           */
  else if (mathexp > 0 && !defaultexpsize && temp > (long)exptrigger)
    size += mathexp + 2;               /* add on the spaces needed          */
  result = raw_string(size);           /* get an empty string to start      */

  resultPtr = result->stringData;
  temp = this->exp + this->length;     /* get adjusted length               */
  if (leadingSpaces != 0) {            /* need leading spaces?              */
                                       /* fill them in                      */
    memset(resultPtr, ' ', leadingSpaces);
    resultPtr += leadingSpaces;        /* and step past them                */
  }
  if (this->sign == -1)                /* negative number?                  */
    *resultPtr++ = '-';                /* add the sign                      */

  if (temp <= 0) {                     /* no integer portion?               */
    strcpy(resultPtr, "0.");           /* add the leading zero and decimal  */
    resultPtr += 2;                    /* and step past them                */
    if (leadingZeros != 0) {           /* zeroes needed?                    */
                                       /* fill them in                      */
      memset(resultPtr, '0', leadingZeros);
      resultPtr += leadingZeros;       /* and step past them                */
    }
    if (this->length > 0) {            /* have a length?                    */
                                       /* fill in the remaining part        */
      fill_digits(resultPtr, (PCHAR)this->number, this->length);
      resultPtr += this->length;       /* step over the digits              */
    }
    if (trailingZeros != 0) {          /* need more zeros?                  */
                                       /* fill them in                      */
      memset(resultPtr, '0', trailingZeros);
      resultPtr += trailingZeros;      /* and step past them                */
    }
  }
  else if (temp >= (long)this->length) {/* all integer data?                 */
                                       /* fill in the remaining part        */
    fill_digits(resultPtr, (PCHAR)this->number, this->length);
    resultPtr += this->length;         /* step over the digits              */
    if (trailingZeros != 0) {          /* need more zeros?                  */
                                       /* fill them in                      */
      memset(resultPtr, '0', trailingZeros);
      resultPtr += trailingZeros;      /* and step past them                */
    }
    if ((long) decimals > 0) {                /* decimals needed?                  */
      *resultPtr++ = '.';              /* add the period                    */
      memset(resultPtr, '0', decimals);/* fill them in                      */
      resultPtr += decimals;           /* and step past them                */
    }
  }
  else {                               /* partial decimal number            */
                                       /* fill in the leading part          */
    fill_digits(resultPtr, (PCHAR)this->number, temp);
    resultPtr += temp;                 /* step over the digits              */
    *resultPtr++ = '.';                /* add the period                    */
                                       /* fill in the trailing part         */
    fill_digits(resultPtr, (PCHAR)this->number + temp, this->length - temp);
    resultPtr += this->length - temp;  /* step over the extra part          */
    if ((long) trailingZeros > 0) {           /* extra decimals needed?            */
                                       /* fill them in                      */
      memset(resultPtr, '0', trailingZeros);
      resultPtr += trailingZeros;      /* and step past them                */
    }
  }

  if (expfactor != 0) {                /* exponential value?                */
    *resultPtr++ = 'E';                /* fill in the notation character    */
    if (expfactor > 0)                 /* positive exponent?                */
      *resultPtr++ = '+';              /* add the plus sign                 */
    else
      *resultPtr++ = '-';              /* a minus sign is required          */
    if (leadingExpZeros > 0) {         /* need extras?                      */
                                       /* fill them in                      */
      memset(resultPtr, '0', leadingExpZeros);
      resultPtr += leadingExpZeros;    /* and step past them                */
    }
                                       /* now add on the exponent           */
    memcpy(resultPtr, exponent, exponentsize);
  }
                                       /* blanks needed instead?            */
  else if (mathexp > 0 && !defaultexpsize && temp > (long)exptrigger) {
                                       /* fill them in                      */
    memset(resultPtr, ' ', mathexp + 2);
    resultPtr += mathexp;              /* and step past them                */
                                       /* add on the spaces                 */
  }
  result->generateHash();              /* go generate the hash              */
  return result;                       /* return the result                 */
}

long RexxNumberString::format(PCHAR number, size_t length)
/******************************************************************************/
/* Function : Format the string data into a numberstring.                     */
/*            NOTE: now that a scan is done first the is some cleanup that can*/
/*               be done, since we can make some assumptions about certain    */
/*               data/chars being valid.                                      */
/*                                                                            */
/*  Returned:  0 if input was valid number                                    */
/*             1 if input was invalid number                                  */
/******************************************************************************/
{

 int       ExpSign;                    /* Exponent Sign                     */
 int       ExpValue;                   /* Exponent Value                    */
 size_t    MaxDigits;                  /* Maximum number size               */
 UCHAR     ch;                         /* current character                 */
 UCHAR     MSDigit = 0;                /* Most Significant digit truncated  */
 PUCHAR    InPtr;                      /* Input Data Pointer                */
 PUCHAR    OutPtr;                     /* Output Data Pointer               */
 PUCHAR    EndData;                    /* Scan end position                 */
 BOOL      isZero;                     /* Number is zero if TRUE            */
 size_t    NumDigits;                  /* Number of digits in result        */


   ExpValue = 0;                       /* Initial Exponent.                 */
   ExpSign = 0;                        /* set exponent sign                 */
   isZero = TRUE;                      /* Assume number will be zero.       */

   InPtr = (PUCHAR) number;            /*Point to start of input string.    */
   EndData = InPtr + length;           /*Point to end of Data + 1.          */

   while (*InPtr == ch_BLANK)          /* Ship all leading blanks.          */
     InPtr++;                          /* Skip it, and go on to next char   */
                                       /* Is this a sign Character?         */
   if ((ch = *InPtr) == ch_MINUS || ch == ch_PLUS) {
    InPtr++;                           /* Yes, skip it.                     */
    if (ch == ch_MINUS)                /* is it a Minus sign?               */
     this->sign   = -1;                /* Yup, indicate a negative number.  */
   }
   while (*InPtr == ch_BLANK)          /* Ship all leading blanks.          */
     InPtr++;                          /* Skip it, and go on to next char   */
   ch = *InPtr;                        /* Get 1st Digit.                    */
   MaxDigits = NumDigits = length;     /* Set our max digits counter.       */
   OutPtr = this->number;              /* Point to Output area.             */

                                       /*Skip all leading Zero's            */
   while (*InPtr == ch_ZERO)           /* While 1st Digit is a 0            */
     InPtr++;                          /* Go to next character.             */

                                       /* Have we reach end of number,num   */
                                       /*zero?                              */
   if (InPtr >= EndData) {
     SetNumberStringZero();            /* Make value a zero.                */
     return 0;
   }
                                       /* Now process real digits.          */
   ExpValue = 0;                       /* Start accumulating exponent       */

   if (*InPtr > ch_ZERO && *InPtr <= ch_NINE) {
    isZero = FALSE;                    /* found the first non-zero digit    */
   }
                                       /* While the character is a Digit.   */
   while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE) {
     if (MaxDigits) {                  /* Still room to add digits          */
                                       /* Move digit into output.           */
       *OutPtr++ = (UCHAR)(*InPtr++ - '0');
       MaxDigits--;                    /* Now room for one less.            */
     }
     else {
                                       /* Have we found our most Sig Digit  */
                                       /* and have we not run out of data?  */
       if ((!MSDigit) && (InPtr < EndData))
         MSDigit = *InPtr;             /* Nope, now we have MSD.            */
       InPtr++;                        /* Point to next char                */
       ExpValue++;                     /* Increment the exponent value.     */
     }
   }
   if (InPtr >= EndData) {             /* Did we reach end of data?         */
                                       /* compute length.                   */
     this->length = (UINT) (NumDigits - MaxDigits);
     this->exp = ExpValue;             /* set exponent value                */
     this->roundUp(MSDigit);
     this->roundUp(MSDigit);           /* Round up the number if necessary  */
     return 0;                         /* all done, just return             */
   }
                                       /* compute length.                   */
   this->length = (NumDigits - MaxDigits);
   this->exp = ExpValue;               /* set exponent value                */

   if (*InPtr == ch_PERIOD) {          /*Decimal point???                   */
     InPtr++;                          /* yes, skip it.                     */
     if (InPtr >= EndData) {           /* Did we reach end of data          */
                                       /* Yes,  valid digits continue.      */
                                       /*is it "0.", or number Zero         */
       if (MaxDigits == NumDigits || isZero){
         this->setZero();              /* make number just zero.            */
       }
       else
                                       /* Round up the number if necessary  */
         this->roundUp(MSDigit);
       return 0;                       /* All done, exit.                   */
     }
     if (MaxDigits == NumDigits) {     /*Any significant digits?            */
                                       /* No, Ship leading Zeros            */
       while (*InPtr == ch_ZERO) {     /* While 1st Digit is a 0            */
         ExpValue--;                   /* decrement exponent.               */
         InPtr++;                      /* Go to next character.             */
                                       /* Have we reach end of number,num   */
                                       /*zero?                              */
         if (InPtr >= EndData) {
           SetNumberStringZero();      /* Make value a zero.                */
           return 0;
         }
       }
     }
                                       /* in the range 1-9?                 */
     if (*InPtr > ch_ZERO && *InPtr <= ch_NINE) {
       isZero = FALSE;                 /* found the first non-zero digit    */
     }
                                       /*While there are still digits       */
     while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE) {
       if (MaxDigits) {                /*if still room for digits           */
         ExpValue--;                   /* Decrement Exponent                */
                                       /* Move char to output               */
         *OutPtr++ = (UCHAR)(*InPtr++ - '0');
         MaxDigits--;                  /* Room for one less digit.          */
       }
       else {
         if (!MSDigit)                 /* not gotten a most sig digit yet?  */
           MSDigit = *InPtr;           /* record this one                   */
         InPtr++;                      /* No more room, go to next digit    */
       }
     }
     if (InPtr >= EndData) {           /*Are we at end of data?             */
                                       /* Compute length of number.         */
       this->length = (NumDigits - MaxDigits);
       this->exp = ExpValue;           /* get exponent.                     */
                                       /* Round up the number if necessary  */
       this->roundUp(MSDigit);
       return 0;                       /* All done, return                  */
     }
   }                                   /* End is it a Decimal point.        */

                                       /* At this point we are don copying  */
                                       /* digits.  We are just looking for  */
                                       /* exponent value if any and any     */
                                       /*trailing blanks                    */

                                       /* Get  final length of number.      */
   this->length = NumDigits - MaxDigits;
   if (!this->length) {                /* No digits, number is Zero.        */
                                       /* Have we reached the end of the    */
                                       /*string                             */
     if (InPtr >= EndData) {
                                       /* Yes, all done.                    */
      this->setZero();                 /* make number just zero.            */
      return 0;                        /* All done, exit.                   */
     }
   }
   this->exp = ExpValue;               /* get current exponent value.       */

   if (toupper(*InPtr) == 'E') {       /* See if this char is an exponent?  */
     ExpSign = 1;                      /* Assume sign of exponent to '+'    */
     InPtr++;                          /* step over the 'E'                 */
     if (*InPtr == ch_MINUS) {         /* If this a minus sign?             */
       ExpSign = -1;                   /*  Yes, make sign of exp '-'        */
       InPtr++;                        /*  go on to next char.              */
     }
     else if (*InPtr == ch_PLUS)       /* If this a plus  sign?             */
       InPtr++;                        /* Yes, skip it and go to next char. */
     ExpValue = 0;                     /* Start of exponent clear work area.*/
     MaxDigits = 0;                    /* claer digit counter.              */

                                       /* Do while we have a valid digit    */
     while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE) {
                                       /* Add this digit to Exponent value. */
       ExpValue = ExpValue * 10 + ((*InPtr++) - '0');
       if (ExpValue > 999999999L)      /* Exponent can only be 9 digits long*/
         return 1;                     /* if more than that, indicate error.*/
       if (ExpValue)                   /* Any significance in the Exponent? */
         MaxDigits++;                  /*  Yes, bump up the digits counter. */
     }
     this->exp += (ExpValue * ExpSign);/* Compute real exponent.            */
                                       /* Is it bigger than allowed max     */
     if (labs(this->exp) > 999999999L)
       return 1;                       /* yes, indicate error.              */
   }

   if (this->sign == 0 || isZero) {    /* Was this really a zero number?    */
     this->setZero();                  /* make number just zero.            */
   }

   this->roundUp(MSDigit);             /* Round up the number if necessary  */
                                       /*is number just flat out too big?   */
   if ((this->exp + (long)this->length - 1) > MAXNUM)
     return 1;                         /* also bad                          */
   return 0;                           /* All done !!                       */
}

void RexxNumberString::formatLong(long integer)
/******************************************************************************/
/* Function : Format the integer num into a numberstring.                     */
/******************************************************************************/
{
  PCHAR  current;                      /* current position                  */

  if (integer == 0) {                  /* is integer 0?                     */
                                       /* indicate this.                    */
   this->setZero();
  }
  else {                               /* number is non-zero                */
                                       /* Format the number                 */
    if (integer < 0 ) {                /* Negative integer number?          */
      this->sign = -1;
      integer = -integer;              /* take the positive version         */
    }
                                       /* convert value into string         */
    _ltoa((long)integer, (PCHAR)this->number, 10);
    current = (PCHAR)this->number;     /* point to the data start           */
    while (*current != '\0') {         /* while still have digits           */
      *current -= '0';                 /* make zero based                   */
      current++;                       /* step to the next one              */
    }
                                       /* set the proper length             */
    this->length = current - (PCHAR)this->number;
  }
}

void RexxNumberString::formatULong(ULONG integer)
/******************************************************************************/
/* Function : Format the integer num into a numberstring.                     */
/******************************************************************************/
{
  PCHAR  current;                      /* current position                  */

  if (integer == 0) {                  /* is integer 0?                     */
                                       /* indicate this.                    */
   this->setZero();
  }
  else {                               /* number is non-zero                */
                                       /* Format the number                 */
                                       /* convert value into string         */
    _ultoa((long)integer, (PCHAR)this->number, 10);
    current = (PCHAR)this->number;     /* point to the data start           */
    while (*current != '\0') {         /* while still have digits           */
      *current -= '0';                 /* make zero based                   */
      current++;                       /* step to the next one              */
    }
                                       /* set the proper length             */
    this->length = current - (PCHAR)this->number;
  }
}

RexxObject *RexxNumberString::unknown(RexxString *msgname, RexxArray *arguments)
/******************************************************************************/
/* Function:  Forward all unknown messages to the numberstring's string       */
/*            representation                                                  */
/******************************************************************************/
{
  return send_message((RexxObject *)this->stringValue(), msgname, arguments);
}

RexxString *RexxNumberString::concatBlank(RexxObject *other)
/******************************************************************************/
/* Function:  Blank concatenation operator                                    */
/******************************************************************************/
{
  return this->stringValue()->concatBlank(other);
}

RexxString *RexxNumberString::concat(RexxObject *other)
/******************************************************************************/
/* Function:  Normal concatentation operator                                  */
/******************************************************************************/
{
  return this->stringValue()->concatRexx(other);
}
                                       /* numberstring operator forwarders  */
                                       /* to process string operators       */

/* MHES 20050108 deprecated */
/*string_forwarder(orOp)
string_forwarder(andOp)
string_forwarder(xorOp) */

RexxObject *RexxNumberString::orOp(RexxObject *operand)
{
  return (RexxObject *)this->stringValue()->orOp(operand);
}

RexxObject *RexxNumberString::andOp(RexxObject *operand)
{
  return (RexxObject *)this->stringValue()->andOp(operand);
}

RexxObject *RexxNumberString::xorOp(RexxObject *operand)
{
  return (RexxObject *)this->stringValue()->xorOp(operand);
}

BOOL RexxNumberString::isEqual(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Primitive strict equal\not equal method.  This determines       */
/*            only strict equality, not greater or less than values.          */
/******************************************************************************/
{
  if (!isPrimitive(this))              /* not a primitive?                  */
                                       /* do the full lookup compare        */
    return this->sendMessage(OREF_STRICT_EQUAL, other)->truthValue(Error_Logical_value_method);
                                       /* go do a string compare            */
  return this->stringValue()->isEqual(other);
}

long RexxNumberString::strictComp(RexxObject *other)
/******************************************************************************/
/* Function:  Compare the two values.                                         */
/*                                                                            */
/*  Returned:  return <0 if other is greater than this                        */
/*             return  0 if this equals other                                 */
/*             return >0 if this is greater than other                        */
/******************************************************************************/
{
                                       /* the strict compare is done against*/
                                       /* strings only, so convert to string*/
                                       /* and let string do this compare.   */
 return this->stringValue()->strictComp(other);
}

long RexxNumberString::comp(
    RexxObject *right)                 /* right hand side of compare      */
/******************************************************************************/
/* Function:  Do a value comparison of two number strings for the non-strict  */
/*            comparisons.  This returns for the compares:                    */
/*                                                                            */
/*             a value < 0 when this is smaller than other                    */
/*             a value   0 when this is equal to other                        */
/*             a value > 0 when this is larger than other                     */
/******************************************************************************/
{
  RexxNumberString *rightNumber;       /* converted right hand number     */
  LONG      aLexp;                     /* adjusted left exponent            */
  LONG      aRexp;                     /* adjusted right exponent           */
  size_t    aLlen;                     /* adjusted left length              */
  size_t    aRlen;                     /* adjusted right length             */
  LONG      MinExp;                    /* minimum exponent                  */
  size_t    NumberDigits;              /* current digits setting            */
  PCHAR     scan;                      /* scan pointer                      */
  LONG      rc;                        /* compare result                    */

                                       /* the compare is acually done by    */
                                       /* subtracting the two numbers, the  */
                                       /* sign of the result obj will be our*/
                                       /* return value.                     */
  required_arg(right, ONE);            /* make sure we have a real value    */
                                       /* get a numberstring object from    */
                                       /*right                              */
  rightNumber = right->numberString();
  if (rightNumber == OREF_NULL)        /* didn't convert?                   */
                                       /* numbers couldn't be compared      */
                                       /* numerically, do a string compare. */
    return this->stringValue()->comp(right);

  if (this->sign != rightNumber->sign) /* are numbers the same sign?        */
                                       /* no, this is easy                  */
    return (this->sign < rightNumber->sign) ? -1 : 1;
  if (rightNumber->sign == 0)          /* right one is zero?                */
    return this->sign;                 /* use this sign                     */
  if (this->sign == 0)                 /* am I zero?                        */
    return rightNumber->sign;          /* return the right sign             */
                                       /* set smaller exponent              */
  MinExp = (rightNumber->exp < this->exp)? rightNumber->exp : this->exp;
  aLexp = this->exp - MinExp;          /* get adjusted left size            */
  aRexp = rightNumber->exp - MinExp;   /* get adjusted right size           */
  aLlen = aLexp + this->length;        /* get adjusted left size            */
  aRlen = aRexp + rightNumber->length; /* get adjusted right size           */
  NumberDigits = number_fuzzydigits(); /* get precision for comparisons.    */
                                       /* can we do a fast exit?            */
  if (aLlen <= NumberDigits && aRlen <= NumberDigits) {
                                       /* longer number is the winner       */
    if (aLlen > aRlen)                 /* left longer?                      */
      return this->sign;               /* use left sign                     */
    else if (aRlen > aLlen)            /* right longer?                     */
      return -this->sign;              /* use inverse of the sign           */
    else {
                                       /* actual lengths the same?          */
      if (this->length == rightNumber->length)
                                       /* return the comparison result      */
                                       /* adjusted by the sign value        */
        return memcmp(this->number, rightNumber->number, this->length) * this->sign;
                                       /* right one shorter?                */
      else if (this->length > rightNumber->length) {
                                       /* compare for shorter length        */
        rc = memcmp(this->number, rightNumber->number, rightNumber->length) * this->sign;
        if (rc == 0) {                 /* equal for that length?            */
                                       /* point to the remainder part       */
          scan = (PCHAR)this->number + rightNumber->length;
                                       /* get the remainder length          */
          aRlen = this->length - rightNumber->length;
          while (aRlen--) {            /* scan the remainder                */
            if (*scan++ != 0)          /* found a non-zero one?             */
              return this->sign;       /* left side is greater              */
          }
          return 0;                    /* these are equal                   */
        }
        return rc;                     /* return compare result             */
      }
      else {                           /* left one is shorter               */
                                       /* compare for shorter length        */
        rc = memcmp(this->number, rightNumber->number, this->length) * this->sign;
        if (rc == 0) {                 /* equal for that length?            */
                                       /* point to the remainder part       */
          scan = (PCHAR)rightNumber->number + this->length;
                                       /* get the remainder length          */
          aRlen = rightNumber->length - this->length;
          while (aRlen--) {            /* scan the remainder                */
            if (*scan++ != 0)          /* found a non-zero one?             */
              return -this->sign;      /* right side is greater             */
          }
          return 0;                    /* these are equal                   */
        }
        return rc;                     /* return compare result             */
      }
    }
  }
  else {                               /* need to subtract these            */
                                       /* call addsub to do computation     */
    rightNumber = this->addSub(rightNumber, OT_MINUS, number_fuzzydigits());
    return rightNumber->sign;          /* compare result is subtract sign   */
  }
}

RexxInteger *RexxNumberString::equal(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict "=" operator                                         */
/******************************************************************************/
{
  return (this->comp(other) == 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxNumberString::notEqual(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict "\=" operator                                        */
/******************************************************************************/
{
  return (this->comp(other) != 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxNumberString::isGreaterThan(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict ">" operator                                         */
/******************************************************************************/
{
  return (this->comp(other) > 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxNumberString::isLessThan(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict "<" operator                                         */
/******************************************************************************/
{
  return (this->comp(other) < 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxNumberString::isGreaterOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict ">=" operator                                        */
/******************************************************************************/
{
  return (this->comp(other) >= 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxNumberString::isLessOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  non-strict "<=" operator                                        */
/******************************************************************************/
{
  return (this->comp(other) <= 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxNumberString::strictEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Perform the primitive level "==" compare, including the hash    */
/*            value processing.                                               */
/******************************************************************************/
{
  LONG    hash;                        /* retrieved hash                    */

  if (other == OREF_NULL) {            /* asking for the hash value?        */
    hash = this->hash();               /* get the hash value                */
                                       /* create a string value             */
    return (RexxInteger *)new_string((PCHAR)&hash, sizeof(LONG));
  }
  else                                 /* do the actual comparison          */
    return (this->strictComp(other) == 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxNumberString::strictNotEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Strict inequality operation                                     */
/******************************************************************************/
{
  return (this->strictComp(other) != 0) ? TheTrueObject : TheFalseObject;
}


RexxInteger *RexxNumberString::strictGreaterThan(RexxObject *other)
/******************************************************************************/
/* Function:  strict ">>" operator                                            */
/******************************************************************************/
{
  return (this->strictComp(other) > 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxNumberString::strictLessThan(RexxObject *other)
/******************************************************************************/
/* Function:  strict "<<" operator                                            */
/******************************************************************************/
{
  return (this->strictComp(other) < 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxNumberString::strictGreaterOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  strict ">>=" operator                                           */
/******************************************************************************/
{
  return (this->strictComp(other) >= 0) ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxNumberString::strictLessOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  strict "<<=" operator                                           */
/******************************************************************************/
{
  return (this->strictComp(other) <= 0) ? TheTrueObject : TheFalseObject;
}

RexxNumberString *RexxNumberString::plus(RexxObject *right)
/********************************************************************/
/* Function:  Add two number strings                                */
/********************************************************************/
{
  RexxNumberString *   result;         /* returned result                   */
  RexxNumberString *   rightNumber;    /* converted operand                 */

  if (right != OREF_NULL) {            /* Is this a dyadic operation?       */
                                       /* get a numberstring object from    */
                                       /*right                              */
    rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)      /* is the operand numeric?           */
                                       /* nope, this is an error            */
      report_exception1(Error_Conversion_operator, right);
                                       /* call addsub to do computation     */
    result = this->addSub(rightNumber, OT_PLUS, number_digits());
  }
  else {
                                       /* need to format under different    */
                                       /* precision?                        */
    if (this->stringObject != OREF_NULL || this->NumDigits != number_digits() ||
       (number_form() == FORM_SCIENTIFIC && !(this->NumFlags&NumFormScientific)) ||
       (number_form() == FORM_ENGINEERING && this->NumFlags&NumFormScientific))
                                       /* need to copy and reformat         */
      result = this->prepareNumber(number_digits(), ROUND);
    else
      result = this;                   /* just return the same value        */
  }
  return result;                       /* return addition result            */
}

RexxNumberString *RexxNumberString::minus(RexxObject *right)
/********************************************************************/
/* Function:  Subtraction between two numbers                       */
/********************************************************************/
{
  RexxNumberString *   result;         /* returned result                   */
  RexxNumberString *   rightNumber;    /* converted operand                 */

  if (right != OREF_NULL) {            /* Is this a dyadic operation?       */
                                       /* get a numberstring object from    */
                                       /*right                              */
    rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)      /* is the operand numeric?           */
                                       /* nope, this is an error            */
      report_exception1(Error_Conversion_operator, right);
                                       /* call addsub to do computation     */
    result = this->addSub(rightNumber, OT_MINUS, number_digits());
  }
  else {
                                       /* need to copy and reformat         */
    result = this->prepareNumber(number_digits(), ROUND);
                                       /* invert the sign of our copy.      */
    result->sign = -(result->sign);
  }
  return result;                       /* return addition result            */
}

RexxNumberString *RexxNumberString::multiply(RexxObject *right)
/********************************************************************/
/* Function:  Multiply two numbers                                  */
/********************************************************************/
{
  RexxNumberString *   rightNumber;    /* converted operand                 */

  required_arg(right, ONE);            /* must have an argument             */

                                       /* get a numberstring object from    */
                                       /*right                              */
  rightNumber = right->numberString();
  if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
                                       /* nope, this is an error            */
    report_exception1(Error_Conversion_operator, right);
  return this->Multiply(rightNumber);  /* go do the multiply                */
}

RexxNumberString *RexxNumberString::divide(RexxObject *right)
/********************************************************************/
/* Function:  Divide two numbers                                    */
/********************************************************************/
{
  RexxNumberString *rightNumber;       /* converted operand                 */

  required_arg(right, ONE);            /* must have an argument             */

                                       /* get a numberstring object from    */
                                       /*right                              */
  rightNumber = right->numberString();
  if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
                                       /* nope, this is an error            */
    report_exception1(Error_Conversion_operator, right);
                                       /* go do the division                */
  return this->Division(rightNumber, OT_DIVIDE);
}

RexxNumberString *RexxNumberString::integerDivide(RexxObject *right)
/********************************************************************/
/* Function:  Integer division between two numbers                  */
/********************************************************************/
{
  RexxNumberString *rightNumber;       /* converted operand                 */

  required_arg(right, ONE);            /* must have an argument             */
                                       /* get a numberstring object from    */
                                       /*right                              */
  rightNumber = right->numberString();
  if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
                                       /* nope, this is an error            */
    report_exception1(Error_Conversion_operator, right);
                                       /* go do the division                */
  return this->Division(rightNumber, OT_INT_DIVIDE);
}

RexxNumberString *RexxNumberString::remainder(RexxObject *right)
/********************************************************************/
/* Function:  Remainder division between two numbers                */
/********************************************************************/
{
  RexxNumberString *rightNumber;       /* converted operand                 */

  required_arg(right, ONE);            /* must have an argument             */

                                       /* get a numberstring object from    */
                                       /*right                              */
  rightNumber = right->numberString();
  if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
                                       /* nope, this is an error            */
    report_exception1(Error_Conversion_operator, right);
                                       /* go do the division                */
  return this->Division(rightNumber, OT_REMAINDER);
}

RexxNumberString *RexxNumberString::abs()
/********************************************************************/
/* Function:  Return the absolute value of a number                 */
/********************************************************************/
{

 RexxNumberString *NewNumber;
 size_t digits = number_digits();

 NewNumber = this->clone();            /* copy the number                   */
 /* inherit the current numeric settings and perform rounding, if */
 /* necessary */
 NewNumber->setupNumber();
                                       /* switch the sign                   */
 NewNumber->sign = (SHORT)labs((INT)NewNumber->sign);
 return NewNumber;                     /* and return                        */
}

RexxInteger *RexxNumberString::Sign()
/********************************************************************/
/* Function:  Return the sign of a number                           */
/********************************************************************/
{
 RexxNumberString *NewNumber;          /* rounded number                    */
 LONG tempSign;

 NewNumber = this->clone();            /* copy the number                   */
 /* inherit the current numeric settings and perform rounding, if */
 /* necessary */
 NewNumber->setupNumber();
 tempSign = NewNumber->sign;           /* return the rounded sign           */
 return new_integer(tempSign);         /* just return the sign value        */
}

RexxObject  *RexxNumberString::notOp()
/********************************************************************/
/* Function:  Logical not of a number string value                  */
/********************************************************************/
{
 return this->stringValue()->notOp();
}

RexxObject  *RexxNumberString::operatorNot(RexxObject *right)
/********************************************************************/
/* Function:  Polymorphic NOT operator method                       */
/********************************************************************/
{
 return this->stringValue()->notOp();
}

RexxNumberString *RexxNumberString::Max(
    RexxObject **args,                 /* array of comparison values        */
    size_t argCount)                   /* count of arguments                */
/********************************************************************/
/* Function:  Process MAX function                                  */
/********************************************************************/
{
 return this->maxMin(args, argCount, OT_MAX);
}

RexxNumberString *RexxNumberString::Min(
    RexxObject **args,                 /* array of comparison values        */
    size_t argCount)                   /* count of arguments                */
/********************************************************************/
/* Function:  Process the MIN function                              */
/********************************************************************/
{
 return this->maxMin(args, argCount, OT_MIN);
}

RexxObject *RexxNumberString::isInteger()
/******************************************************************************/
/* This method determines if the formatted numberstring is s true integer     */
/* string.  That is, its not of the form 1.00E3 but 10000                     */
/******************************************************************************/
{
                                       /* first convert number to formatted */
                                       /* objetc, "make it a string"        */
                                       /* now call string to check for      */
  return this->stringValue()->isInteger();
}

/*********************************************************************/
/*   Function:          Round up a number as a result of the chopping*/
/*                        digits off of the number during init.      */
/*********************************************************************/
void RexxNumberString::roundUp(ULONG MSDigit)
{
  int  carry;
  UCHAR *InPtr, ch;
                                       /* Did we chop off digits and is it  */
                                       /* greater/equal to 5, rounding?     */
   if (MSDigit >= ch_FIVE && MSDigit <= ch_NINE) {
                                       /* yes, we have to round up digits   */

    carry = 1;                         /* indicate we have a carry.         */
                                       /* point to last digit.              */
    InPtr = this->number + this->length - 1;

                                       /* all digits and still have a carry */
    while ((InPtr >= this->number) && carry){
     if (*InPtr == 9)                  /* Is this digit a 9?                */
      ch = 0;                          /* make digit 0, still have carry    */
     else {
      ch = *InPtr + 1;                 /* Not nine, just add one to digit.  */
      carry = 0;                       /* No more carry.                    */
     }
     *InPtr-- = ch;                    /* replace digit with new value.     */
    }                                  /* All done rounding.                */

    if (carry) {                       /* Do we still have a carry?         */
                                       /* yes, carry rippled all the way    */
     *this->number = 1;                /*  set 1st digit to a 1.            */
     this->exp += 1;                   /* increment exponent by one.        */
    }
   }
}

RexxString *RexxNumberString::d2x(
     RexxObject *length)               /* result length                     */
/******************************************************************************/
/* Function:  Convert a valid numberstring to a hex string.                   */
/******************************************************************************/
{
                                       /* forward to the formatting routine */
  return this->d2xD2c(length, FALSE);
}

RexxString *RexxNumberString::d2c(
     RexxObject *length)               /* result length                     */
/******************************************************************************/
/* Function:  Convert a valid numberstring to a character string.             */
/******************************************************************************/
{
                                       /* forward to the formatting routine */
  return this->d2xD2c(length, TRUE);
}


RexxObject *RexxNumberString::evaluate(
     RexxActivation *context,          /* current activation context        */
     RexxExpressionStack *stack )      /* evaluation stack                  */
/******************************************************************************/
/* Function:  Polymorphic method that makes numberstring a polymorphic        */
/*            expression term for literals                                    */
/******************************************************************************/
{
  stack->push(this);                   /* place on the evaluation stack     */
                                       /* trace if necessary                */
  context->traceIntermediate(this, TRACE_PREFIX_LITERAL);
  return this;                         /* also return the result            */
}

RexxString *RexxNumberString::d2xD2c(
     RexxObject *length,               /* result length                     */
     BOOL  type )                      /* D2C or D2X flag                   */
/******************************************************************************/
/* Function:  Convert a valid numberstring to a hex or character string.      */
/******************************************************************************/

{
  UCHAR      PadChar;                  /* needed padding character          */
  long       ResultSize;               /* size of result string             */
  size_t     HexLength;                /* length of hex characters          */
  size_t     BufferLength;             /* length of the buffer              */
  PCHAR      Scan;                     /* scan pointer                      */
  PCHAR      HighDigit;                /* highest digit location            */
  PCHAR      Accumulator;              /* accumulator pointer               */
  PCHAR      TempPtr;                  /* temporary pointer value           */
  size_t     PadSize;                  /* needed padding                    */
  size_t     CurrentDigits;            /* current digits setting            */
  size_t     TargetLength;             /* length of current number          */
  RexxBuffer *Target;                  /* formatted number                  */
  RexxString *Retval;                  /* returned result                   */


                                       /* get the target length             */
  ResultSize = optional_length(length, -1, ARG_ONE);
  CurrentDigits = number_digits();     /* get the current digits setting    */
  TargetLength = this->length;         /* copy the length                   */
                                       /* too big to process?               */
  if (this->exp + this->length > CurrentDigits) {
    if (type == TRUE)                  /* d2c form?                         */
                                       /* use that message                  */
      report_exception1(Error_Incorrect_method_d2c, this);
    else                               /* use d2x form                      */
      report_exception1(Error_Incorrect_method_d2x, this);
  }
  else if (this->exp < 0) {            /* may have trailing zeros           */
                                       /* point to the decimal part         */
    TempPtr = (PCHAR)this->number + this->length + this->exp;
    HexLength = -this->exp;            /* get the length to check           */
                                       /* point to the rounding digit       */
    HighDigit = (PCHAR)this->number + CurrentDigits;
                                       /* while more decimals               */
    while (HexLength -- && TempPtr <= HighDigit) {
      if (*TempPtr != 0) {             /* non-zero decimal?                 */
                                       /* this may be non-significant       */
        if (TargetLength > CurrentDigits) {
                                       /* this the "rounding" digit?        */
          if (TempPtr == HighDigit && *TempPtr < 5)
            break;                     /* insignificant digit found         */
        }
        if (type == TRUE)              /* d2c form?                         */
                                       /* use that message                  */
          report_exception1(Error_Incorrect_method_d2c, this);
        else                           /* use d2x form                      */
          report_exception1(Error_Incorrect_method_d2x, this);
      }
      TempPtr++;                       /* step the pointer                  */
    }
                                       /* adjust the length                 */
    TargetLength = this->length + this->exp;
  }
                                       /* negative without a size           */
  if (this->sign < 0 && ResultSize == -1)
                                       /* this is an error                  */
    report_exception(Error_Incorrect_method_d2xd2c);
  if (ResultSize == -1)                /* using default size?               */
                                       /* allocate buffer based on digits   */
    BufferLength = CurrentDigits + OVERFLOWSPACE;
  else if (type == TRUE)      {             /* X2C function?                     */
    if (ResultSize * 2 < (long)CurrentDigits)/* smaller than digits setting?      */
                                       /* allocate buffer based on digits   */
      BufferLength = CurrentDigits + OVERFLOWSPACE;
    else                               /* allocate a large buffer           */
      BufferLength = (ResultSize * 2) + OVERFLOWSPACE;
  }
  else {                               /* D2X function                      */
    if (ResultSize < (long)CurrentDigits)    /* smaller than digits setting?      */
                                       /* allocate buffer based on digits   */
      BufferLength = CurrentDigits + OVERFLOWSPACE;
    else                               /* allocate a large buffer           */
      BufferLength = ResultSize + OVERFLOWSPACE;
  }
  Target = new_buffer(BufferLength);   /* set up format buffer              */
  Scan = (PCHAR)this->number;          /* point to first digit              */
                                       /* set accumulator pointer           */
  Accumulator = Target->data + BufferLength - 2;
  HighDigit = Accumulator - 1;         /* set initial high position         */
                                       /* clear the accumulator             */
  memset(Target->data, '\0', BufferLength);
  while (TargetLength--) {             /* while more digits                 */
                                       /* add next digit                    */
    HighDigit = AddToBaseSixteen(*Scan++, Accumulator, HighDigit);
    if (TargetLength != 0)             /* not last digit?                   */
                                       /* do another multiply               */
      HighDigit = MultiplyBaseSixteen(Accumulator, HighDigit);
  }
  if (this->exp > 0) {                 /* have extra digits to worry about? */
                                       /* do another multiply               */
    HighDigit = MultiplyBaseSixteen(Accumulator, HighDigit);
    TargetLength = this->exp;          /* copy the exponent                 */
    while (TargetLength--) {           /* while more digits                 */
                                       /* add next zero digit               */
      HighDigit = AddToBaseSixteen('\0', Accumulator, HighDigit);
      if (TargetLength != 0)           /* not last digit?                   */
                                       /* do the multiply                   */
        HighDigit = MultiplyBaseSixteen(Accumulator, HighDigit);
    }
  }
  HexLength = Accumulator - HighDigit; /* get accumulator length            */
  if (this->sign < 0) {                /* have a negative number?           */
                                       /* take twos complement              */
    PadChar = 'F';                     /* pad negatives with foxes          */
    Scan = Accumulator;                /* point to last digit               */
    while (!*Scan)                     /* handle any borrows                */
      *Scan-- = 0xf;                   /* make digit a 15                   */
    *Scan = (UCHAR)(*Scan - 1);        /* subtract the 1                    */
    Scan = Accumulator;                /* start at first digit again        */
    while (Scan > HighDigit) {         /* invert all the bits               */
                                       /* one digit at a time               */
      *Scan = (UCHAR)(*Scan ^ (unsigned)0x0f);
      Scan--;                          /* step to next digit                */
    }
  }
  else
    PadChar = '0';                     /* pad positives with zero           */
                                       /* now make number printable         */
  Scan = Accumulator;                  /* start at first digit again        */
  while (Scan > HighDigit) {           /* convert all the nibbles           */
    *Scan = IntToHexDigit(*Scan);      /* one digit at a time               */
    Scan--;                            /* step to next digit                */
  }
  Scan = HighDigit + 1;                /* point to first digit              */

  if (type == FALSE) {                 /* d2x function ?                    */
    if (ResultSize == -1)              /* using default length?             */
      ResultSize = HexLength;          /* use actual data length            */
  }
  else {                               /* d2c function                      */
    if (ResultSize == -1)              /* using default length?             */
      ResultSize = HexLength;          /* use actual data length            */
    else
      ResultSize += ResultSize;        /* double the size                   */
  }
  if (ResultSize < (long)HexLength) {  /* need to truncate?                 */
    PadSize = 0;                       /* no Padding                        */
    Scan += HexLength - ResultSize;    /* step the pointer                  */
    HexLength = ResultSize;            /* adjust number of digits           */
  }
  else                                 /* possible padding                  */
    PadSize = ResultSize - HexLength;  /* calculate needed padding          */
  if (PadSize) {                       /* padding needed?                   */
    Scan -= PadSize;                   /* step back the pointer             */
    memset(Scan, PadChar, PadSize);    /* pad in front                      */
  }
  if (type == TRUE)                    /* need to pack?                     */
    Retval = PackHex(Scan, ResultSize);/* yes, pack to character            */
  else
                                       /* allocate result string            */
    Retval = (RexxString *)new_string(Scan, ResultSize);
  return Retval;                       /* return proper result              */
}

int RexxNumberString::ULong(
     ULONG *  value)                   /* result length                     */
/******************************************************************************/
/* Function:  Convert a valid numberstring to a ULONG value.                  */
/*   returns TRUE for a sucessfule conversion, FALSE otherwise.               */
/******************************************************************************/
{
  UCHAR *num;
  BOOL  carry;
  size_t numlength, numpos;
  long  numexp;
  ULONG intnum;
  size_t  NumDigits;
  char  compareChar;
  int   rc;


   if (this->sign == -1) {             /*  Anegative numebr?                */
     return FALSE;
   }
   else if (this->sign == 0) {         /* Is number 0?                      */
     *value = 0;                       /* Yup, return 0 .                   */
     return TRUE;
   }
   else {                              /* positive number convert it.       */
    NumDigits = 10;



    if (this->length > NumDigits) {    /* is number bigger than max Digits. */
                                       /* Yes need to adjust number down.   */
     numexp = this->exp + (this->length-NumDigits);
     numlength = NumDigits;

                                       /* is MSD of the numbers being trunc */
     if (*(this->number + numlength) >= 5)
                                       /* Great/equal to 5?  Rounding?      */
      carry = TRUE;                    /* Yes, indocate rounding for later. */
     else
      carry = FALSE;                   /* Nope, make sure don't round later */


    }
    else {                             /* current length is OK. set         */
     numlength = this->length;         /*  length and                       */
     numexp = this->exp;               /*  exp values to numbers.           */
     carry = FALSE;                    /* We don't have a carry.            */
    }

    if (numexp < 0)  {                 /* Is the exponet negative?          */
     numpos    = -numexp;              /*    Get length of num after decimal*/
     if (carry)                        /* Did we have a carry condition?    */
       if (numpos == numlength)        /* 'implied' zero's between decimal  */
                                       /* point and 1st digit?              */
         compareChar = 9;              /* digits after decimap point must   */
                                       /* be 9 to carry forward into integer*/
       else
         return FALSE;                 /* yes, not a valid whole number.    */
     else
       compareChar = '\0';             /* other wise all must be 0.         */

     if (numpos >= numlength ) {       /* all of number after the decimanl  */
       numpos = numlength;
       num = (PUCHAR) this->number;    /* start looking at 1st digit        */
     } else
                                       /* set num to 1st digit after decimal*/
       num = (PUCHAR) this->number + numlength + numexp;

     for ( ; numpos ; numpos--) {      /* look at all digits for vaility.   */
      if ( *num++ != compareChar)      /* this digit what we're looking for */
        return  FALSE;                 /*   nope, not a valid integer.      */
     }

     if (-numexp >= (long)numlength) { /* carrying and all digits after     */
                                       /* and since we know all those digit */
                                       /* are 0 or 9 (if carry), we know the*/
      if (carry)                       /* answer now.                       */
        *value = 1;                    /* if carry then its a 1.            */
      else
        *value = 0;                    /* otherwise its a zero.             */
      return TRUE;
     }

     rc = number_create_uinteger((PUCHAR) this->number, numlength + numexp, carry, this->sign, &intnum);
     if (!rc) {                        /* Is numebr too big ????            */
      return FALSE;                    /* Yes, return invalid integer....   */
     }
    }
    else {                             /* straight out number. just compute.*/
     rc = number_create_uinteger((PUCHAR) this->number, numlength, carry, this->sign, &intnum);
     if (!rc) {                        /* Is numebr too big ????            */
      return FALSE;                    /* Yes, return invalid integer....   */
     }

     if (numexp > 0 ) {                /* do we need to add zeros to end?   */
                                       /* Yes, see how many we need to add  */
                                       /* make sure don't expand past size  */
                                       /*  of a long....                    */
      for (numpos = 1 ;numpos <= (size_t)numexp && intnum < MAXPOSBASE; numpos++ ) {
       intnum *= 10;                   /*  Add one zero to end of integer   */
      }

      if (numpos <= (size_t)numexp) {  /* did number exceed limit??         */
       return FALSE;                   /* yes, return error.                */
      }
     }
    }
   *value = intnum;
   return TRUE;                        /* return INTEGER object.            */
  }
}

void  *RexxNumberString::operator new(size_t size, size_t length)
/******************************************************************************/
/* Function:  Create a new NumberString object                                */
/******************************************************************************/
{
  RexxNumberString *newNumber;

  newNumber = (RexxNumberString *)new_object(size + length);
  newNumber->hashvalue = 0;            /* Undefine the hash value           */
                                       /* Give new object its behaviour     */
  BehaviourSet(newNumber, TheNumberStringBehaviour);
                                       /* initialize the new object         */
  SetObjectHasNoReferences(newNumber); /* Let GC know no to bother with LIVE*/
  return newNumber;                    /* return the new numberstring       */
}

RexxNumberString *RexxNumberStringClass::classNew(PCHAR number, size_t len)
/******************************************************************************/
/* Function:  Create a new number string object                               */
/******************************************************************************/
{
  RexxNumberString *newNumber;

  if (number == NULL) {                /* asking for a dummy string?        */
                                       /* allocate a new string             */
    newNumber = new (len) RexxNumberString (len);
                                       /* make it a zero value              */
    newNumber->setZero();
    return newNumber;                  /* return this now                   */
  }
                                       /* scan the string 1st to see if its */
                                       /*valid                              */
  if (numberStringScan(number, len))
    newNumber = OREF_NULL;             /* not a valid number, get out now.  */
  else {
                                       /* looks to be valid.  get a new     */
                                       /* format it                         */
     newNumber = new (len) RexxNumberString (len);
                                       /* now see if the data actually is   */
                                       /*  a number and fill in actual data */
                                       /* NOTE: even though a scan has been */
                                       /*   we still may not have a thorough*/
                                       /*   enough check.                   */
     if (newNumber->format(number, len))
                                       /* number didn't convert,            */
       newNumber = OREF_NULL;
  }
  return newNumber;
}

RexxNumberString *RexxNumberStringClass::newFloat(float num)
/******************************************************************************/
/* Function:  Create a numberstring object from a floating point number       */
/******************************************************************************/
{
  RexxNumberString *result;
  size_t resultLen;
                                       /* Max length of double str is       */
                                       /*  10, make 15 just to be safe      */
  char floatStr[30];
                                       /* get float  as a string value.     */
                                       /* Use digits as precision.          */
  sprintf(floatStr, "%.*g", number_digits() + 1, num);
  resultLen = strlen(floatStr);        /* Compute length of floatString     */
                                       /* Create new NumberString           */
  result = new (resultLen) RexxNumberString (resultLen);
                                       /* now format as a numberstring      */
  result->format(floatStr, resultLen);
  return result;
}

RexxNumberString *RexxNumberStringClass::newDouble(PDBL number)
/******************************************************************************/
/* Function:  Create a NumberString from a double value                       */
/******************************************************************************/
{
  RexxNumberString *result;
  size_t resultLen;
                                       /* Max length of double str is       */
                                       /*  22, make 30 just to be safe      */
  char doubleStr[30];
                                       /* get double as a string value.     */
                                       /* Use digits as precision.          */
  sprintf(doubleStr, "%.*g", number_digits() + 1, *number);
  resultLen = strlen(doubleStr);       /* Compute length of floatString     */
                                       /* Create new NumberString           */
  result = new (resultLen) RexxNumberString (resultLen);
                                       /* now format as a numberstring      */
  result->format(doubleStr, resultLen);
  return result;
}

RexxNumberString *RexxNumberStringClass::newLong(long integer)
/******************************************************************************/
/* Function:  Create a NumberString object from a long value                  */
/******************************************************************************/
{
  RexxNumberString *newNumber;
                                       /* at most an integer will be 9      */
                                       /*  digits long.                     */
  newNumber = new (10) RexxNumberString (10);
  newNumber->formatLong(integer);      /* format the integer                */
  return newNumber;
}

RexxNumberString *RexxNumberStringClass::newULong(ULONG integer)
/******************************************************************************/
/* Function:  Create a NumberString object from an unsigned long value        */
/******************************************************************************/
{
  RexxNumberString *newNumber;

                                       /* at most an integer will be 9      */
                                       /*  digits long.                     */
  newNumber = new (10) RexxNumberString (10);
  newNumber->formatULong(integer);     /* format the integer                */
  return newNumber;
}
