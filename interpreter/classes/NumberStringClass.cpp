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
/* REXX Kernel                                                                */
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
#include "BufferClass.hpp"
#include "RexxActivation.hpp"
#include "NumberStringMath.hpp"
#include "Numerics.hpp"
#include "StringUtil.hpp"



// singleton class instance
RexxClass *RexxNumberString::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxNumberString::createInstance()
{
    CLASS_CREATE(NumberString, "String", RexxClass);
}


/* MHES 20050108 deprecated */
#define string_forwarder(method)\
RexxObject *RexxNumberString::##method(RexxObject *operand)\
 {\
     return (RexxObject *)this->stringValue()->##method(operand);\
 }


/**
 * Constructor for a new number string value.
 *
 * @param len    The length we require for the value.
 */
RexxNumberString::RexxNumberString(size_t len)
{
    this->NumDigits = number_digits();
    this->sign = 1;
    this->length = len;
    if (number_form() == Numerics::FORM_SCIENTIFIC)
    {
        this->NumFlags |= NumFormScientific;
    }
}


/**
 * Create a number string for a given digits precision
 *
 * @param len       The length of value we need to accomodate
 * @param precision The precision to be used for formatting.
 */
RexxNumberString::RexxNumberString(size_t len, size_t precision)
{
    this->NumDigits = precision;
    this->sign = 1;
    this->length = len;
    if (number_form() == Numerics::FORM_SCIENTIFIC)
    {
        this->NumFlags |= NumFormScientific;
    }
}

RexxNumberString *RexxNumberString::clone()
/******************************************************************************/
/* Function:  low level copy of a number string object                        */
/******************************************************************************/
{
    /* first clone ourselves             */
    RexxNumberString *newObj = (RexxNumberString *)this->RexxInternalObject::clone();
    /* don't keep the original string    */
    OrefSet(newObj, newObj->stringObject, OREF_NULL);
    /* or the OVD fields                 */
    OrefSet(newObj, newObj->objectVariables, OREF_NULL);
    return newObj;                       /* return this                       */
}


/**
 * Get the primitive hash value of this String object.
 *
 * @return The calculated string hash for the string.
 */
HashCode RexxNumberString::getHashValue()
{
    return stringValue()->getHashValue();
}

void RexxNumberString::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->objectVariables);
    memory_mark(this->stringObject);
}

void RexxNumberString::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->objectVariables);
    memory_mark_general(this->stringObject);
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
   this->setHasReferences();           /* we now have to garbage collect    */
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
    {
        return this->stringObject;         /* all finished                      */
    }
    return this->stringValue();          /* return the string value           */
}

RexxString *RexxNumberString::stringValue()
/******************************************************************************/
/* Function:  Convert a number string to a string object                      */
/******************************************************************************/
{
    char  expstring[17], num;
    int    carry;
    size_t createdDigits;
    size_t MaxNumSize, LenValue;
    wholenumber_t numindex;
    wholenumber_t temp, ExpValue, ExpFactor;
    size_t charpos;
    RexxString *StringObj;

    if (this->stringObject != OREF_NULL) /* already converted?                */
    {
        return this->stringObject;         /* all finished                      */
    }
                                           /* Start converting the number.      */

    if (this->sign == 0  )
    {              /* is the number zero?               */
                   /* Yes, return a 0 string.           */
        OrefSet(this, this->stringObject, OREF_ZERO_STRING);
        this->setHasReferences();           /* we now have to garbage collect    */
        return this->stringObject;          /* and return now                    */
    }
    else
    {                                /*  No, convert the number.          */
                                     /* convert the exponent numeber into */
                                     /* string.                           */

        createdDigits = this->NumDigits;    /* Get Digits settings at creation   */

        ExpValue = this->exp;               /* Working copy of exponent          */
        LenValue = this->length;            /* Working copy of the length.       */

                                            /* If no exponent                    */
        if (ExpValue == 0)
        {
            /* Yes, we can fast path conversion  */
            MaxNumSize = LenValue;             /* Size of result is length of number*/
            if (this->sign <0)                 /* if number is negative.            */
            {
                MaxNumSize++;                     /* result one bigger for - sign.     */
            }
                                                  /* Get new string, of exact length,  */
            StringObj = (RexxString *)raw_string(MaxNumSize);
            charpos = 0;                       /* data in string to start.          */
            if (this->sign < 0)
            {              /* If number is neagative            */
                           /*  add negative sign and bump index */
                StringObj->putChar(charpos++, ch_MINUS);
            }
            /* For each number digits in number  */
            for (numindex=0; (size_t)numindex < LenValue; numindex++)
            {
                /* place char rep in NumString       */
                num = this->number[numindex] + ch_ZERO;
                StringObj->putChar(charpos++, num);
            }                                  /* Done with Fast Path....           */
        }
        else
        {                              /* We need to do this the long way   */
            carry = 0;                         /* assume rounding-up NOT necessary  */

                                               /*is number just flat out too big?   */
            if ((( ExpValue + (wholenumber_t)LenValue - 1) > Numerics::MAX_EXPONENT) ||
                (ExpValue < (Numerics::MIN_EXPONENT)) )       /* Yes, report Overflow error.       */
            {
                reportException(Error_Conversion_operator, this);
            }


            ExpFactor = 0;                     /* number not eponential yet..       */
            temp = ExpValue + (wholenumber_t)LenValue - 1;  /* get size of this number           */
            expstring[0] = '\0';               /* string vlaue of exp factor, Null  */
                                               /* is left of decimal > NumDigits or */
            if ((temp >= (wholenumber_t)createdDigits) ||  /* exponent twice numDigits          */
                ((size_t)Numerics::abs(ExpValue) > (createdDigits*2)) )
            {
                /* Yes, we need to go exponential.   */
                /* we need Engineering format?       */
                if (!(this->NumFlags & NumFormScientific))
                {
                    if (temp < 0)                    /* Yes, is number a whole number?    */
                    {
                        temp -= 2;                      /* force 2 char adjustment to left   */
                    }
                    temp = (temp/3) * 3;             /* get count to the right of Decimal */
                }
                if (Numerics::abs(temp) > Numerics::MAX_EXPONENT)
                {        /* is adjusted number too big?       */
                    if (temp > Numerics::MAX_EXPONENT)              /* did it overflow?                  */
                    {
                        /* Yes, report overflow error.       */
                        reportException(Error_Overflow_expoverflow, temp, Numerics::DEFAULT_DIGITS);
                    }
                    else
                    {
                        /* Actually an underflow error.      */
                        reportException(Error_Overflow_expunderflow, temp, Numerics::DEFAULT_DIGITS);
                    }
                }
                ExpValue -= temp;                 /* adjust the exponent               */
                if ( temp != 0 )
                {                /* do we still have exponent ?       */
                    ExpFactor = true;              /* Save the factor                   */
                }
                else
                {
                    ExpFactor = false;             /* no need to save the factor        */
                }

                if (temp < 0)
                {
                    *expstring = 'E';
                    /* convert exponent value into string*/
                    Numerics::formatWholeNumber(temp, expstring + 1);
                }
                else if (temp > 0)
                {
                    strcpy(expstring, "E+");
                    /* convert exponent value into string*/
                    Numerics::formatWholeNumber(temp, expstring + 2);
                }
                temp = Numerics::abs(temp);           /* get positive exponent factor      */

            }
            /* Now compute size of result string */
            if (ExpValue >= 0 )                /* if the Exponent is positive       */
            {
                /* Size is length of number plus exp.*/
                MaxNumSize = (size_t)ExpValue + LenValue;
            }
            /*is exponent larger than length,    */
            else if ((size_t)Numerics::abs(ExpValue) >= LenValue)
            {
                /* Yes, we will need to add zeros to */
                /* the front plus a 0.               */
                MaxNumSize = Numerics::abs(ExpValue) + 2;
            }

            else                               /*Won't be adding any digits, just   */
            {
                MaxNumSize = LenValue + 1;        /* length of number + 1 for decimal  */
            }

            if (ExpFactor)                     /* Are we using Exponential notation?*/
            {
                /* Yes, need to add in size of the   */
                MaxNumSize += strlen(expstring);  /* exponent stuff.                   */
            }

            if (this->sign <0)                 /* is number negative?               */
            {
                MaxNumSize++;                     /* yes, add one to size for - sign   */
            }
                                                  /* get new string of appropriate size*/
            StringObj = (RexxString *)raw_string(MaxNumSize);

            charpos = 0;                       /* set starting position             */
            if (this->sign < 0)
            {              /* Is the number negative?           */
                           /* Yes, add in the negative sign.    */
                StringObj->putChar(charpos, ch_MINUS);
            }
            temp = ExpValue + (wholenumber_t)LenValue;   /* get the adjusted length.          */

                                               /* Since we may have carry from round*/
                                               /* we'll fill in the number from the */
                                               /* back and make our way forward.    */

                                               /* Strindex points to exponent start */
                                               /* part of string.                   */
            charpos  = MaxNumSize - strlen(expstring);

            if (ExpFactor)                     /* will result have Exponent?        */
            {
                /* Yes, put the data into the string.*/
                StringObj->put(charpos, expstring, strlen(expstring));
            }

            /* Next series of If's will determine*/
            /* if we need to add zeros to end    */
            /* of the number and where to place  */
            /* decimal, fill in string as we     */
            /* go, also need to check for a carry*/
            /* if we rounded the number early on.*/

            if (temp <= 0)
            {                   /* Is ther an Integer portion?       */
                                /*   0.000000xxxx result.            */
                                /*   ^^^^^^^^     filler             */

                                /* Start filling in digits           */
                                /*   from the back....               */
                for (numindex = (wholenumber_t)(LenValue-1);numindex >= 0 ;numindex-- )
                {
                    /* are we carry from round?          */
                    num = this->number[numindex];   /* working copy of this Digit.       */
                    num = num + ch_ZERO;            /* now put the number as a character */
                    StringObj->putChar(--charpos, num);
                }
                temp = -temp;                    /* make the number positive...       */

                if (temp)
                {
                    charpos  -= temp;               /* yes, point to starting pos to fill*/
                                                    /* now fill in the leading Zeros.    */
                    StringObj->set(charpos, ch_ZERO, temp);
                }
                StringObj->putChar(--charpos, ch_PERIOD);
                if (carry)                       /* now put in the leading 1. is carry*/
                {
                    StringObj->putChar(--charpos, ch_ONE);
                }
                else                             /* or 0. if no carry.                */
                {
                    StringObj->putChar(--charpos, ch_ZERO);
                }
            }
            /* do we need to add zeros at end?   */
            else if ((size_t)temp >= LenValue)
            {
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
                for (numindex = (wholenumber_t)LenValue-1;numindex >= 0 ;numindex-- )
                {
                    num = this->number[numindex];   /* working copy of this Digit.       */
                    num = num + ch_ZERO;            /* now put the number as a character */
                    StringObj->putChar(--charpos, num);
                }
            }                                  /* done with this case....           */

            else
            {                             /* we have a partial Decimal number  */
                                          /* add in the first digit a 1.       */
                                          /* Start filling in digits           */
                for (numindex = (wholenumber_t)LenValue - 1; numindex > temp - 1 ;numindex-- )
                {
                    num = this->number[numindex];   /* working copy of this Digit.       */
                                                    /* now put the number as a character */
                    num += ch_ZERO;
                    StringObj->putChar(--charpos, num);
                }
                /* add in the decimal point.         */
                StringObj->putChar(--charpos, ch_PERIOD);

                /* Start filling in digits           */
                /* add numbers before decimal point  */
                for (numindex = temp - 1; numindex >= 0; numindex-- )
                {
                    num = this->number[numindex];   /* working copy of this Digit.       */
                    num += ch_ZERO;
                    StringObj->putChar(--charpos, num);
                }
                /* end of final case, conversion done*/
            }
        }                                   /* end of non-fast path conversion.  */
    }                                    /* End of conversion of number       */
                                         /* since string is created from      */
                                         /* number string, we can set the     */
    StringObj->setNumberString(this);    /* lookaside right away              */
                                         /* and also save this string         */
    OrefSet(this, this->stringObject, StringObj);
    this->setHasReferences();            /* we now have to garbage collect    */
    return StringObj;                    /* all done, return new string       */
}


bool RexxNumberString::numberValue(wholenumber_t &result)
/******************************************************************************/
/* Function:  Convert a number string to a wholenumber value                  */
/******************************************************************************/
{
    // convert using the default digits version
    return this->numberValue(result, Numerics::DEFAULT_DIGITS);
}

bool RexxNumberString::unsignedNumberValue(stringsize_t &result)
/******************************************************************************/
/* Function:  Convert a number string to a unsigned whole number value        */
/******************************************************************************/
{
    // convert using the default digits version
    return this->unsignedNumberValue(result, Numerics::DEFAULT_DIGITS);
}

bool RexxNumberString::numberValue(wholenumber_t &result, size_t numDigits)
/******************************************************************************/
/* Function:  Convert a number string to a number value                       */
/******************************************************************************/
{
    // set up the default values

    bool carry = false;
    wholenumber_t numberExp = this->exp;
    stringsize_t numberLength = this->length;
    size_t intnum;

    // if the number is exactly zero, then this is easy
    if (this->sign == 0)
    {
        result = 0;
        return true;
    }
    // is this easily within limits (very common)?
    if (length <= numDigits && numberExp >= 0)
    {
        if (!createUnsignedValue(number, length, false, numberExp, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // too big to handle
        }
        // adjust for the sign
        result = ((wholenumber_t)intnum) * sign;
        return true;
    }

    // this number either has decimals, or needs to be truncated/rounded because of
    // the conversion digits value.  We need to make adjustments.

    if (!checkIntegerDigits(numDigits, numberLength, numberExp, carry))
    {
        return false;
    }

    // if because of this adjustment, the decimal point lies to the left
    // of our first digit, then this value truncates to 0 (or 1, if a carry condition
    // resulted).
    if (-numberExp>= (wholenumber_t)numberLength)
    {
        // since we know a) this number is all decimals, and b) the
        // remaining decimals are either all 0 or all 9s with a carry,
        // this result is either 0 or 1.
        result = carry ? 1 : 0;
        return true;
    }

    // we process different bits depending on whether this is a negative or positive
    // exponent
    if (numberExp < 0)
    {
        // now convert this into an unsigned value
        if (!createUnsignedValue(number, numberLength + numberExp, carry, 0, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // to big to handle
        }
    }
    else
    {                             /* straight out number. just compute.*/
        if (!createUnsignedValue(number, numberLength, carry, numberExp, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // to big to handle
        }
    }

    // adjust for the sign
    result = ((wholenumber_t)intnum) * sign;
    return true;
}

bool RexxNumberString::unsignedNumberValue(stringsize_t &result, size_t numDigits)
/******************************************************************************/
/* Function:  Convert a number string to an unsigned number value             */
/******************************************************************************/
{
    // set up the default values

    bool carry = false;
    wholenumber_t numberExp = this->exp;
    stringsize_t numberLength = this->length;
    size_t intnum;

    // if the number is exactly zero, then this is easy
    if (this->sign == 0)
    {
        result = 0;
        return true;
    }
    // we can't convert negative values into an unsigned one
    if (sign < 0)
    {
        return false;
    }

    // is this easily within limits (very common)?
    if (length <= numDigits && numberExp >= 0)
    {
        if (!createUnsignedValue(number, length, false, numberExp, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // too big to handle
        }
        // we can just return this directly.
        result = intnum;
        return true;
    }

    // this number either has decimals, or needs to be truncated/rounded because of
    // the conversion digits value.  We need to make adjustments.
    if (!checkIntegerDigits(numDigits, numberLength, numberExp, carry))
    {
        return false;
    }

    // if because of this adjustment, the decimal point lies to the left
    // of our first digit, then this value truncates to 0 (or 1, if a carry condition
    // resulted).
    if (-numberExp>= (wholenumber_t)numberLength)
    {
        // since we know a) this number is all decimals, and b) the
        // remaining decimals are either all 0 or all 9s with a carry,
        // this result is either 0 or 1.
        result = carry ? 1 : 0;
        return true;
    }

    // we process different bits depending on whether this is a negative or positive
    // exponent
    if (numberExp < 0)
    {
        // now convert this into an unsigned value
        if (!createUnsignedValue(number, numberLength + numberExp, carry, 0, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // to big to handle
        }
    }
    else
    {                             /* straight out number. just compute.*/
        if (!createUnsignedValue(number, numberLength, carry, numberExp, Numerics::maxValueForDigits(numDigits), intnum))
        {
            return false;                   // to big to handle
        }
    }

    // adjust for the sign
    result = intnum;
    return true;
}

bool RexxNumberString::doubleValue(double &result)
/******************************************************************************/
/* Function:  Convert a number string to a double                             */
/******************************************************************************/
{
    RexxString *string;                   /* string version of the number      */

    string = this->stringValue();         /* get the string value              */
                                          /* convert the number                */
    result = strtod(string->getStringData(), NULL);
    // and let pass all of the special cases
    return true;
}

RexxInteger *RexxNumberString::integerValue(
    size_t digits )                    /* required precision                */
/******************************************************************************/
/* Function:  convert a number string to an integer object                    */
/******************************************************************************/
{

    wholenumber_t integerNumber;       /* converted value                   */

    if (!numberValue(integerNumber, number_digits()))
    {
        return (RexxInteger *)TheNilObject;/* just return .nil                  */
    }

    return new_integer(integerNumber);
}


/*********************************************************************/
/*   Function:  Convert the numberstring to unsigned value           */
/*********************************************************************/
bool  RexxNumberString::createUnsignedValue(const char *thisnum, stringsize_t intlength, int carry, wholenumber_t exponent, size_t maxValue, size_t &result)
{
    // if the exponent multiplier would cause an overflow, there's no point in doing
    // anything here
    if (exponent > (wholenumber_t)Numerics::ARGUMENT_DIGITS)
    {
        return false;
    }

    // our converted value
    size_t intNumber = 0;

    for (stringsize_t numpos = 1; numpos <= intlength; numpos++ )
    {
        // add in the next digit value
        size_t newNumber = (intNumber * 10) + (size_t)*thisnum++;
        // if an overflow occurs, then the new number will wrap around and be
        // smaller that the starting value.
        if (newNumber < intNumber)
        {
            return false;
        }
        // make this the current value and continue
        intNumber = newNumber;
    }

    // do we need to add in a carry value because of a rounding situation?
    if (carry)
    {
        // add in the carry bit and check for an overflow, again
        size_t newNumber = intNumber + 1;
        if (newNumber < intNumber)
        {
            return false;
        }
        intNumber = newNumber;
    }

    // have an exponent to process?
    if (exponent > 0)
    {
        // get this as a multipler value
        size_t exponentMultiplier = 1;
        while (exponent > 0)
        {
            exponentMultiplier *= 10;
            exponent--;
        }
        // get this as a multipler value
        size_t newNumber = intNumber * exponentMultiplier;

        // did this wrap?  This is a safe test, since we capped
        // the maximum exponent size we can multiply by.
        if (newNumber < intNumber)
        {
            return false;
        }
        intNumber = newNumber;
    }

    // was ths out of range for this conversion?
    if (intNumber >= maxValue)
    {
        return false;
    }

    result = intNumber;                  /* Assign return value.              */
    return true;                         /* Indicate sucessfule converison.   */
}


/*********************************************************************/
/*   Function:  Convert the numberstring to unsigned value           */
/*********************************************************************/
bool  RexxNumberString::createUnsignedInt64Value(const char *thisnum, stringsize_t intlength, int carry, wholenumber_t exponent, uint64_t maxValue, uint64_t &result)
{
    // if the exponent multiplier would cause an overflow, there's no point in doing
    // anything here
    if (exponent > (wholenumber_t)Numerics::DIGITS64)
    {
        return false;
    }

    // our converted value
    uint64_t intNumber = 0;

    for (stringsize_t numpos = 1; numpos <= intlength; numpos++ )
    {
        // add in the next digit value
        uint64_t newNumber = (intNumber * 10) + (uint64_t)*thisnum++;
        // if an overflow occurs, then the new number will wrap around and be
        // smaller that the starting value.
        if (newNumber < intNumber)
        {
            return false;
        }
        // make this the current value and continue
        intNumber = newNumber;
    }

    // do we need to add in a carry value because of a rounding situation?
    if (carry)
    {
        // add in the carry bit and check for an overflow, again
        uint64_t newNumber = intNumber + 1;
        if (newNumber < intNumber)
        {
            return false;
        }
        intNumber = newNumber;
    }

    // have an exponent to process?
    if (exponent > 0)
    {
        // get this as a multipler value
        uint64_t exponentMultiplier = 1;
        while (exponent > 0)
        {
            exponentMultiplier *= 10;
            exponent--;
        }


        uint64_t newNumber = intNumber * exponentMultiplier;

        // did this wrap?  This is a safe test, since we capped
        // the maximum exponent size we can multiply by.
        if (newNumber < intNumber)
        {
            return false;
        }
        intNumber = newNumber;
    }

    // was ths out of range for this conversion?
    if (intNumber > maxValue)
    {
        return false;
    }

    result = intNumber;                  /* Assign return value.              */
    return true;                         /* Indicate sucessfule converison.   */
}


bool RexxNumberString::checkIntegerDigits(stringsize_t numDigits, stringsize_t &numberLength,
    wholenumber_t &numberExponent, bool &carry)
/******************************************************************************/
/* Function:  Check that a numberstring is convertable into an integer value  */
/******************************************************************************/
{
    carry = false;
    numberExponent = this->exp;
    numberLength = this->length;

    // is this number longer than the digits value?
    // this is going to be truncated or rounded, so we
    // need to see if a carry is required, and also
    // adjust the exponent value.
    if (this->length > numDigits)
    {
        // adjust the effective exponent up by the difference
        numberExponent += (this->length - numDigits);
        // and override the converted length to be just the digits length
        numberLength = numDigits;

        // now check to see if the first excluded digit will cause rounding
        // if it does, we need to worry about a carry value when converting
        if (*(this->number + numberLength) >= 5)
        {
            carry = true;
        }
    }
    // if we have a negative exponent, then we need to look at
    // the values below the decimal point
    if (numberExponent < 0)
    {
        // the position of the decimal is the negation of the exponent
        stringsize_t decimalPos = (stringsize_t)(-numberExponent);
        // everything to the right of the decimal must be a zero.
        char compareChar = 0;
        // if we had a previous carry condition, this changes things a
        // bit.  Since the carry will add 1 to the right-most digit,
        // in order for all of the decimals to end up zero, then all of
        // the digits there must actually be '9's.
        if (carry)
        {
            // if the decimal position will result in at least one padding
            // zero, then the carry makes it impossible for all of the decimals
            // to reduce to zero.  This cannot be a whole number.
            if (decimalPos > numberLength)
            {
                return false;
            }
            // switch the checking value
            compareChar = 9;
        }

        const char *numberData;
        if (decimalPos >= numberLength )
        {
            // decimal is somewhere to the left of everything...
            // all of these digits must be checked
            decimalPos = numberLength;
            numberData = this->number;
        }
        else
        {
            // get the exponent adjusted position
            numberData = this->number + numberLength + numberExponent;
        }

        for ( ; decimalPos > 0 ; decimalPos--)
        {
            // a bad digits data means a conversion failure
            if ( *numberData++ != compareChar)
            {
                return false;
            }
        }
    }
    return true;
}


bool RexxNumberString::int64Value(int64_t *result, stringsize_t numDigits)
/******************************************************************************/
/* Function:  Convert a number string to a int64 value                        */
/******************************************************************************/
{
    // set up the default values

    bool carry = false;
    wholenumber_t numberExp = this->exp;
    stringsize_t numberLength = this->length;
    uint64_t intnum;

    // if the number is exactly zero, then this is easy
    if (this->sign == 0)
    {
        *result = 0;
        return true;
    }
    // is this easily within limits (very common)?
    if (length <= numDigits && numberExp >= 0)
    {
        // the minimum negative value requires one more than the max positive
        if (!createUnsignedInt64Value(number, length, false, numberExp, ((uint64_t)INT64_MAX) + 1, intnum))
        {
            return false;                   // too big to handle
        }
        // this edge case can be a problem, so check for it specifically
        if (intnum == ((uint64_t)INT64_MAX) + 1)
        {
            // if at the limit, this must be a negative number
            if (sign != -1)
            {
                return false;
            }
            *result = INT64_MIN;
        }
        else
        {
            // adjust for the sign
            *result = ((int64_t)intnum) * sign;
        }
        return true;
    }

    // this number either has decimals, or needs to be truncated/rounded because of
    // the conversion digits value.  We need to make adjustments.

    if (!checkIntegerDigits(numDigits, numberLength, numberExp, carry))
    {
        return false;
    }

    // if because of this adjustment, the decimal point lies to the left
    // of our first digit, then this value truncates to 0 (or 1, if a carry condition
    // resulted).
    if (-numberExp>= (wholenumber_t)numberLength)
    {
        // since we know a) this number is all decimals, and b) the
        // remaining decimals are either all 0 or all 9s with a carry,
        // this result is either 0 or 1.
        *result = carry ? 1 : 0;
        return true;
    }

    // we process different bits depending on whether this is a negative or positive
    // exponent
    if (numberExp < 0)
    {
        // now convert this into an unsigned value
        if (!createUnsignedInt64Value(number, numberLength + numberExp, carry, 0, ((uint64_t)INT64_MAX) + 1, intnum))
        {
            return false;                   // to big to handle
        }
    }
    else
    {                             /* straight out number. just compute.*/
        if (!createUnsignedInt64Value(number, numberLength, carry, numberExp, ((uint64_t)INT64_MAX) + 1, intnum))
        {
            return false;                   // to big to handle
        }
    }
    // the edge case is a problem, so handle it directly
    if (intnum == ((uint64_t)INT64_MAX) + 1)
    {
        // if at the limit, this must be a negative number
        if (sign != -1)
        {
            return false;
        }
        *result = INT64_MAX;
    }
    else
    {
        // adjust for the sign
        *result = ((int64_t)intnum) * sign;
    }
    return true;
}


bool RexxNumberString::unsignedInt64Value(uint64_t *result, stringsize_t numDigits)
/******************************************************************************/
/* Function:  Convert a number string to a int64 value                        */
/******************************************************************************/
{
    // set up the default values

    bool carry = false;
    wholenumber_t numberExp = this->exp;
    stringsize_t numberLength = this->length;

    // if the number is exactly zero, then this is easy
    if (this->sign == 0)
    {
        *result = 0;
        return true;
    }

    // no signed values allowed
    if (sign == -1)
    {
        return false;
    }

    // is this easily within limits (very common)?
    if (length <= numDigits && numberExp >= 0)
    {
        if (!createUnsignedInt64Value(number, length, false, numberExp, UINT64_MAX, *result))
        {
            return false;                   // too big to handle
        }
        return true;
    }

    // this number either has decimals, or needs to be truncated/rounded because of
    // the conversion digits value.  We need to make adjustments.

    if (!checkIntegerDigits(numDigits, numberLength, numberExp, carry))
    {
        return false;
    }

    // if because of this adjustment, the decimal point lies to the left
    // of our first digit, then this value truncates to 0 (or 1, if a carry condition
    // resulted).
    if (-numberExp>= (wholenumber_t)numberLength)
    {
        // since we know a) this number is all decimals, and b) the
        // remaining decimals are either all 0 or all 9s with a carry,
        // this result is either 0 or 1.
        *result = carry ? 1 : 0;
        return true;
    }

    // we process different bits depending on whether this is a negative or positive
    // exponent
    if (numberExp < 0)
    {
        // now convert this into an unsigned value
        return createUnsignedInt64Value(number, numberLength + numberExp, carry, 0, UINT64_MAX, *result);
    }
    else
    {                             /* straight out number. just compute.*/
        return createUnsignedInt64Value(number, numberLength, carry, numberExp, UINT64_MAX, *result);
    }
}


bool  RexxNumberString::truthValue(
    int   errorcode )                  /* error to raise if not good        */
/******************************************************************************/
/* Function:  Return a truth value boolean for a number string                */
/******************************************************************************/
{
    if (this->sign == 0 )                /* exactly zero?                     */
    {
        return false;                      /* done quickly                      */
    }
                                           /* not exactly 1?                    */
    else if (!(this->sign == 1 && this->exp == 0 && this->length == 1L && *(this->number) == 1))
    {
        reportException(errorcode, this);/* report the error                  */
    }
    return true;                         /* this is true                      */
}


/**
 * Convert an object to a logical value without raising an
 * error.
 *
 * @param result The converted value.
 *
 * @return true if this converted ok, false for an invalid logical.
 */
bool RexxNumberString::logicalValue(logical_t &result)
{
    if (this->sign == 0 )                /* exactly zero?                     */
    {
        result = false;                  // this is false and the conversion worked
        return true;
    }
                                           /* exactly 1?                    */
    else if (this->sign == 1 && this->exp == 0 && this->length == 1 && *(this->number) == 1)
    {
        result = true;                   // this is true and the conversion worked
        return true;
    }
    else
    {
        return false;                    // bad conversion
    }
}

bool numberStringScan(const char *number, size_t length)
/******************************************************************************/
/* Arguments:  Number data, number length                                     */
/* Function :Scan the string to determine if its a valid number               */
/* Returned :  0 if input was valid number                                    */
/*             1 if input was invalid number                                  */
/******************************************************************************/
{
    char      ch;                         /* current character                 */
    const char *InPtr;                    /* Input Data Pointer                */
    const char *EndData;                  /* Scan end position                 */
    bool      hadPeriod;                  /* had a decimal point already       */

                                          /* for efficiency, this code takes   */
                                          /* advantage of the fact that REXX   */
                                          /* string object's all have a guard  */
                                          /* null character on the end         */

    if (!length)
    {                      /* Length zero not a number?         */
        return true;                       /* a null string is not a number     */
    }

    hadPeriod = false;                  /* period yet                        */
    InPtr = number;                     /*Point to start of input string.    */
    EndData = InPtr + length;           /*Point to end of Data + 1.          */

    while (*InPtr == ch_BLANK || *InPtr == ch_TAB)  /* Skip all leading blanks.          */
    {
        InPtr++;                          /* Skip it, and go on to next char   */
    }
                                          /* Is this a sign Character?         */
    if ((ch = *InPtr) == ch_MINUS || ch == ch_PLUS)
    {
        InPtr++;                          /* Yes, skip it.                     */
        while (*InPtr == ch_BLANK || *InPtr == ch_TAB)   /* Ship all leading blanks.          */
        {
            InPtr++;                        /* Skip it, and go on to next char   */
        }
    }

    if (*InPtr == ch_PERIOD)
    {          /* got a leading period?             */
        InPtr++;                          /* step over it                      */
        hadPeriod = true;                 /* got the decimal point already     */
    }

    ch = *InPtr;                        /* Get 1st Digit.                    */
    if (ch < ch_ZERO || ch > ch_NINE)   /* Is this a valid digit?            */
    {
        return true;                      /* Nope, bad number                  */
    }
    else
    {
        /*Skip all leading Zero's            */
        while (*InPtr == ch_ZERO)         /* While 1st Digit is a 0            */
        {
            InPtr++;                        /* Go to next character.             */
        }
                                            /* Have we reach end of number,num   */
                                            /*zero?                              */
        if (InPtr >= EndData)
        {
            return false;                   /* valid number... all Zeros         */
        }
    }
    /* While the character is a Digit.   */
    while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE)
    {
        InPtr++;                          /* Go to next digit                  */
    }
    if (InPtr >= EndData)               /* Did we reach end of data?         */
    {
        return false;                   /* all done, just return valid number*/
    }

    if (*InPtr == ch_PERIOD)
    {          /*Decimal point???                   */
        if (hadPeriod)                    /* already had one?                  */
        {
            return true;                    /* yep, this is a bad number         */
        }
        InPtr++;                          /* yes, skip it.                     */
                                          /* While the character is a Digit.   */
        while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE)
        {
            InPtr++;                        /* Go to next digit                  */
        }
        if (InPtr >= EndData)             /* Did we reach end of data          */
        {
            return false;                   /* this was fine                     */
        }
    }

    if (toupper(*InPtr) == 'E')
    {       /* See if this char is an exponent?  */
        if (++InPtr >= EndData)            /* Yes, but did we reach end of input*/
        {
            /* Yes, invalid number.              */
            return true;
        }
        /* If this a plus/minus sign?        */
        if ((*InPtr == ch_MINUS) || (*InPtr == ch_PLUS))
        {
            InPtr++;                         /*  go on to next char.              */
        }
        if (InPtr >= EndData)              /* reach end of Input ?              */
        {
            return true;                     /* Yes, invalid number.              */
        }
                                             /* If this char a valid digit?       */
        if (*InPtr < ch_ZERO || *InPtr > ch_NINE)
        {
            return true;                     /* No,  invalid number.              */
        }
                                             /* Do while we have a valid digit    */
        while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE)
        {
            InPtr++;                         /* Yup, go to next one and check     */
        }
    }
    /* At this point all that should be  */
    /* left Are trailing blanks.         */
    while (*InPtr == ch_BLANK || *InPtr == ch_TAB)  /* Skip all trailing blanks          */
    {
        InPtr++;                          /* Skip it, and go on to next char   */
    }
    if (InPtr >= EndData)               /* Did we reach end of data          */
    {
        return false;                     /* this was fine                     */
    }
    return true;                        /* wasn't a valid number             */
}

void fill_digits(                      /* create a string of digits         */
  char *outPtr,                        /* output location                   */
  const char *number,                  /* start of string of digits         */
  size_t count )                       /* size of resulting string          */
/******************************************************************************/
/* Function : Copy "count" digits of a number to the desired location,        */
/*            converting them back to character digits                        */
/******************************************************************************/
{
    while (count--)                      /* while still have characters       */
    {
        *outPtr++ = *number++ + '0';       /* convert back to character         */
    }
}

RexxObject *RexxNumberString::trunc(
  RexxObject *decimal_digits)          /* number of decimal digits        */
/******************************************************************************/
/* Function:  Truncate a number to given decimal digit count                  */
/******************************************************************************/
{
    /* get the decimal count             */
    size_t needed_digits = optionalNonNegative(decimal_digits, 0, ARG_ONE);
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
    wholenumber_t    temp;               /* temporary string value            */
    wholenumber_t    integer_digits;     /* leading integer digits            */
    size_t  size;                        /* total size of the result          */
    int     signValue;                   /* current sign indicator            */
    char   *resultPtr;                   /* result pointer                    */

    if (this->sign == 0)
    {               /* is the number zero?               */
        if (needed_digits == 0)            /* no decimals requested?            */
        {
            /* just return a zero                */
            return IntegerZero;
        }
        else
        {                             /* need to pad                       */
                                      /* get an empty string               */
            result = (RexxString *)raw_string(needed_digits + 2);
            /* get a data pointer                */
            resultPtr = result->getWritableData();
            strcpy(resultPtr, "0.");         /* copy the leading part             */
                                             /* fill in the trailing zeros        */
            memset(resultPtr + 2, '0', needed_digits);
            return result;                   /* return the result                 */
        }
    }
    else
    {                               /* have to do real formatting        */
        size = 0;                          /* start with nothing                */
        signValue = this->sign;            /* copy the sign                     */
                                           /* calculate the leading part        */
                                           /* number have a decimal part?       */
        if (this->exp > 0)
        {
            /* add in both length and exponent   */
            size += this->length + this->exp;
            if (needed_digits != 0)          /* have digits required?             */
            {
                size += needed_digits + 1;     /* add in the digits and the decimal */
            }
        }
        else
        {                             /* number has a decimal part.        */
                                      /* get the leading part              */
            integer_digits = (wholenumber_t)this->length + this->exp;
            if (integer_digits > 0)
            {        /* something on the left hand side?  */
                size += integer_digits;        /* add in these digits               */
                if (needed_digits != 0)        /* decimals requested?               */
                {
                    size += needed_digits + 1;   /* add in the digits and the decimal */
                }
            }
            else
            {                           /* no leading part                   */
                if (needed_digits == 0)        /* nothing wanted after decimal?     */
                {
                    return IntegerZero;          /* this is just zero then            */
                }
                                                 /* do we need to pad more zeros than */
                                                 /*  number we want after the decimal?*/
                if ((wholenumber_t)needed_digits <= -integer_digits)
                {
                    size = needed_digits + 2;    /* result is formatted zero...no sign*/
                    signValue = 0;               /* force the sign out                */
                }
                else
                {
                    size += needed_digits + 2;   /* calculate the decimal size        */
                }
            }
        }
        if (signValue < 0)                 /* negative number?                  */
        {
            size++;                          /* start with a sign                 */
        }
                                             /* get an empty pointer              */
        result = (RexxString *)raw_string(size);
        /* point to the data part            */
        resultPtr = result->getWritableData();
        if (signValue < 0)                 /* negative number?                  */
        {
            *resultPtr++ = '-';              /* start with a sign                 */
        }
                                             /* calculate the leading part        */
                                             /* number have a decimal part?       */
        if (this->exp > 0)
        {
            /* fill in the digits                */
            fill_digits(resultPtr, this->number, this->length);
            resultPtr += this->length;       /* step over the length              */
                                             /* now fill in the extra zeros       */
            memset(resultPtr, '0', this->exp);
            resultPtr += this->exp;          /* and the exponent                  */
            if (needed_digits != 0)
            {        /* decimals requested?               */
                *resultPtr++ = '.';            /* add a trailing decimal point      */
                                               /* copy on the trailers              */
                memset(resultPtr, '0', needed_digits);
            }
        }
        else
        {                             /* number has a decimal part.        */
            integer_digits = this->length + this->exp;
            if (integer_digits > 0)
            {        /* something on the left hand side?  */
                     /* add the integer digits            */
                fill_digits(resultPtr, this->number, integer_digits);
                resultPtr += integer_digits;   /* step over the digits              */
                if (needed_digits != 0)
                {      /* decimals requested?               */
                    *resultPtr++ = '.';          /* add a trailing decimal point      */
                                                 /* get count to add                  */
                    temp = Numerics::minVal(needed_digits, this->length - integer_digits);
                    /* fill in the digits                */
                    fill_digits(resultPtr, this->number + integer_digits, temp);
                    resultPtr += temp;           /* step over the digits              */
                    needed_digits -= temp;       /* adjust the length                 */
                    if (needed_digits != 0)      /* still need more?                  */
                    {
                        /* copy on the trailers              */
                        memset(resultPtr, '0', needed_digits);
                    }
                }
            }
            else
            {                           /* no leading part                   */
                                        /* do we need to pad more zeros than */
                                        /*  number we want after the decimal?*/
                if ((wholenumber_t)needed_digits <= -integer_digits)
                {
                    strcpy(resultPtr, "0.");     /* copy a leading zero part          */
                    resultPtr += 2;              /* step over                         */
                                                 /* copy on the trailers              */
                    memset(resultPtr, '0', needed_digits);
                }
                else
                {
                    strcpy(resultPtr, "0.");     /* copy a leading zero part          */
                    resultPtr += 2;              /* step over                         */
                                                 /* copy on the trailers              */
                    memset(resultPtr, '0', -integer_digits);
                    resultPtr += -integer_digits;/* step over the digits              */
                    needed_digits += integer_digits; /* reduce needed_digits          */
                    /* get count to add                  */
                    temp = Numerics::minVal(needed_digits, this->length);
                    /* fill in the digits                */
                    fill_digits(resultPtr, this->number, temp);
                    resultPtr += temp;           /* step over the digits              */
                    needed_digits -= temp;       /* adjust the length                 */
                    if (needed_digits != 0)      /* still need more?                  */
                    {
                        /* copy on the trailers              */
                        memset(resultPtr, '0', needed_digits);
                    }
                }
            }
        }
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
    bool   form;                         /* current numeric form              */

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
    bool        form)                    /* form to format to               */
/******************************************************************************/
/* Function : Format the numberstring data according to the format            */
/*            function controls.                                              */
/******************************************************************************/
{
    wholenumber_t    expfactor;          /* actual used exponent              */
    wholenumber_t    temp;               /* temporary calculation holder      */
    size_t exponentsize = 0;             /* size of the exponent              */
    char   exponent[15];                 /* character exponent value          */
    wholenumber_t adjust;                /* exponent adjustment factor        */
    size_t size;                         /* total size of the result          */
    size_t leadingSpaces;                /* number of leading spaces          */
    size_t leadingZeros = 0;             /* number of leading zeros           */
    size_t leadingExpZeros = 0;          /* number of leading zeros in exp    */
    size_t trailingZeros;                /* number of trailing zeros          */
    size_t reqIntegers;                  /* requested integers                */
    RexxString *result;                  /* final formatted number            */
    char  *resultPtr;                    /* pointer within the result         */
    bool   defaultexpsize = false;       /* default exponent size             */

    expfactor = 0;                       /* not exponential yet               */

    if (mathexp != 0)
    {                  /* Is exponential allowed?           */
                       /* calculate the exponent factor     */
        temp = this->exp + this->length - 1;
        /* is left of dec>digits             */
        /* or twice digits on right          */
        if (temp >= (wholenumber_t)exptrigger || Numerics::abs(this->exp) > (wholenumber_t)(exptrigger * 2))
        {
            if (form == Numerics::FORM_ENGINEERING)
            {  /* request for Engineering notation? */
                if (temp < 0)                  /* yes, is it a whole number?        */
                {
                    temp = temp - 2;             /* no, force two char left adjustment  -2 instead of -1 */
                }
                temp = (temp / 3) * 3;         /* get count right of decimal point  */
            }
            this->exp = this->exp - temp;    /* adjust the exponent               */
            expfactor = temp;                /* save the factor                   */
            temp = Numerics::abs(temp);      /* get positive exponent value       */
                                             /* format exponent to a string       */
            Numerics::formatWholeNumber(temp, exponent);
            /* get the number of digits needed   */
            exponentsize = strlen(exponent);
            if (mathexp == (size_t)-1)
            {     /* default exponent size?            */
                mathexp = exponentsize;        /* use actual length                 */
                defaultexpsize = true;         /* default exponent size on          */
            }
            if (exponentsize > mathexp)      /* not enough room?                  */
            {
                reportException(Error_Incorrect_method_exponent_oversize, (RexxObject *)original, mathexp);
            }
        }
    }

    if (decimals == (size_t)-1)
    {        /* default decimal processing?       */
        if (this->exp < 0)                 /* negative exponent?                */
        {
            decimals = -this->exp;           /* get number of decimals            */
        }
    }
    else
    {
        if (this->exp < 0)
        {               /* have actual decimals?             */
            adjust = -this->exp;             /* get absolute value                */
            if ((size_t)adjust > decimals)
            { /* need to round or truncate?        */
                adjust = adjust - decimals;    /* get the difference                */
                                               /* adjust exponent                   */
                this->exp = this->exp + adjust;
                if (adjust >= (wholenumber_t)this->length)
                {  /* Losing all digits?  need rounding */
                    /* is rounding needed?               */
                    if (adjust == (wholenumber_t)this->length && this->number[0] >= 5)
                    {
                        this->number[0] = 1;       /* round up                          */
                    }
                    else
                    {
                        this->number[0] = 0;       /* round down                        */
                        this->exp = 0;             /* nothing left at all               */
                        this->sign = 1;            /* suppress a negative sign          */
                    }
                    this->length = 1;            /* just one digit left               */
                }
                /* Need to round?                    */
                else
                {                         /* truncating, need to check rounding*/
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
                        strcpy(exponent, "0");
                        exponentsize = strlen(exponent);
                    }

                    temp = this->exp + this->length - 1;

                    /* did rounding trigger the          */
                    /* exponential form?                 */
                    if (mathexp != 0 && (temp >= (wholenumber_t)exptrigger || (size_t)Numerics::abs(this->exp) > exptrigger * 2))
                    {
                        /* yes, request for                  */
                        if (form == Numerics::FORM_ENGINEERING)
                        {
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
                        Numerics::formatWholeNumber(expfactor, exponent);
                        /* get the number of digits needed   */
                        exponentsize = strlen(exponent);

                        if (mathexp == (size_t)-1) /* default exponent size?            */
                        {
                            mathexp = exponentsize;  /* use actual length                 */
                        }
                        if (exponentsize > mathexp)/* not enough room?                  */
                        {
                            reportException(Error_Incorrect_method_exponent_oversize, original, mathexp);
                        }
                    }
                }
            }
        }
    }

    if (integers == (size_t)-1)
    {        /* default integers requested        */
        if (this->exp >= 0)                /* non-negative exponent?            */
        {
            /* use all of number                 */
            integers = this->length + this->exp;
        }
        else
        {
            /* no integer part?                  */
            if ((size_t)Numerics::abs(this->exp) > this->length)
            {
                integers = 1;                  /* just the leading zero             */
            }
            else                             /* get the integer part              */
            {
                integers = this->length + this->exp;
            }
        }
    }
    else
    {                               /* user requested size               */
        reqIntegers = integers;            /* save integers                     */
        if (this->sign == -1)              /* negative number?                  */
        {
            integers = integers - 1;         /* the sign takes up one spot        */
        }
        if (this->exp >= 0)                /* non-negative exponent?            */
        {
            temp = this->length + this->exp; /* use all of number                 */
        }
        else
        {
            /* no integer part?                  */
            if ((size_t)Numerics::abs(this->exp) > this->length)
            {
                temp = 1;                      /* just the leading zero             */
            }
            else
            {
                /* get the integer part              */
                temp = this->length + this->exp;
            }
        }
        if ((wholenumber_t)integers < temp)  /* not enough room?                  */
        {
            /* this is an error                  */
            reportException(Error_Incorrect_method_before_oversize, original, reqIntegers);
        }
    }

    size = 0;                            /* start with a null string          */
    leadingSpaces = 0;                   /* no leading spaces yet             */
    temp = this->exp + this->length;     /* get adjusted length               */
    if (temp != (wholenumber_t)integers)
    {        /* need leading spaces?              */
        if (temp > 0)                      /* have leading part?                */
        {
            leadingSpaces = integers - temp; /* get leading length                */
        }
        else
        {
            leadingSpaces = integers - 1;    /* leave space for leading 0         */
        }
        size += leadingSpaces;             /* fill in the spaces                */
    }
    if (this->sign == -1)                /* negative number?                  */
    {
        size++;                            /* space for the sign                */
    }

    if (temp <= 0)
    {                     /* no integer portion?               */
        size += 2;                         /* add room for zero and decimal     */
        leadingZeros = -temp;              /* get positive of zeros             */
        size += leadingZeros;              /* add in the zeros size             */
        if (this->length > 0)              /* have a length?                    */
        {
            size += this->length;            /* add on the actual data            */
        }
                                             /* need more zeros?                  */
        if (leadingZeros + this->length < decimals)
        {
            /* get the trailing count            */
            trailingZeros = decimals - (leadingZeros + this->length);
            size += trailingZeros;           /* add them on                       */
        }
        else
        {
            trailingZeros = 0;               /* no trailing zeros                 */
        }
    }
    else if (temp >= (wholenumber_t)this->length)
    { /* all integer data?                 */
        size += this->length;              /* add on the digits                 */
                                           /* reduce total length               */
        trailingZeros = temp - this->length;
        size += trailingZeros;             /* add this to the total size        */
        if (decimals > 0)                  /* decimals needed?                  */
        {
            size += decimals + 1;            /* add decimal point and trailers    */
        }
    }
    else
    {                               /* partial decimal number            */
        size += this->length + 1;          /* need the length plus a decimal    */
                                           /* get needed extra zeroes           */
        trailingZeros = decimals - (this->length - temp);
        size += trailingZeros;             /* add that to the size              */
        if ((wholenumber_t)trailingZeros < 0)
        {
            this->length += trailingZeros;
            this->exp -= trailingZeros;
            trailingZeros = 0;
        }
    }

    if (expfactor != 0)
    {                /* exponential value?                */
        size += 2;                         /* add on the E and the sign         */
                                           /* get extra size needed             */
        leadingExpZeros = mathexp - exponentsize;
        size += mathexp;                   /* add on the total exponent size    */
    }
    /* spaces needed for exp.?           */
    else if (mathexp > 0 && !defaultexpsize && temp > (wholenumber_t)exptrigger)
    {
        size += mathexp + 2;               /* add on the spaces needed          */
    }
    result = raw_string(size);           /* get an empty string to start      */

    resultPtr = result->getWritableData();
    temp = this->exp + this->length;     /* get adjusted length               */
    if (leadingSpaces != 0)
    {            /* need leading spaces?              */
                 /* fill them in                      */
        memset(resultPtr, ' ', leadingSpaces);
        resultPtr += leadingSpaces;        /* and step past them                */
    }
    if (this->sign == -1)                /* negative number?                  */
    {
        *resultPtr++ = '-';                /* add the sign                      */
    }

    if (temp <= 0)
    {                     /* no integer portion?               */
        strcpy(resultPtr, "0.");           /* add the leading zero and decimal  */
        resultPtr += 2;                    /* and step past them                */
        if (leadingZeros != 0)
        {           /* zeroes needed?                    */
                    /* fill them in                      */
            memset(resultPtr, '0', leadingZeros);
            resultPtr += leadingZeros;       /* and step past them                */
        }
        if (this->length > 0)
        {            /* have a length?                    */
                     /* fill in the remaining part        */
            fill_digits(resultPtr, this->number, this->length);
            resultPtr += this->length;       /* step over the digits              */
        }
        if (trailingZeros != 0)
        {          /* need more zeros?                  */
                   /* fill them in                      */
            memset(resultPtr, '0', trailingZeros);
            resultPtr += trailingZeros;      /* and step past them                */
        }
    }
    else if (temp >= (wholenumber_t)this->length)
    {/* all integer data?                 */
        /* fill in the remaining part        */
        fill_digits(resultPtr, this->number, this->length);
        resultPtr += this->length;         /* step over the digits              */
        if (trailingZeros != 0)
        {          /* need more zeros?                  */
                   /* fill them in                      */
            memset(resultPtr, '0', trailingZeros);
            resultPtr += trailingZeros;      /* and step past them                */
        }
        if ((wholenumber_t)decimals > 0)
        { /* decimals needed?                  */
            *resultPtr++ = '.';              /* add the period                    */
            memset(resultPtr, '0', decimals);/* fill them in                      */
            resultPtr += decimals;           /* and step past them                */
        }
    }
    else
    {                               /* partial decimal number            */
                                    /* fill in the leading part          */
        fill_digits(resultPtr, this->number, temp);
        resultPtr += temp;                 /* step over the digits              */
        *resultPtr++ = '.';                /* add the period                    */
                                           /* fill in the trailing part         */
        fill_digits(resultPtr, this->number + temp, this->length - temp);
        resultPtr += this->length - temp;  /* step over the extra part          */
        if ((wholenumber_t)trailingZeros > 0)
        {           /* extra decimals needed?            */
            /* fill them in                      */
            memset(resultPtr, '0', trailingZeros);
            resultPtr += trailingZeros;      /* and step past them                */
        }
    }

    if (expfactor != 0)
    {                /* exponential value?                */
        *resultPtr++ = 'E';                /* fill in the notation character    */
        if (expfactor > 0)                 /* positive exponent?                */
        {
            *resultPtr++ = '+';              /* add the plus sign                 */
        }
        else
        {
            *resultPtr++ = '-';              /* a minus sign is required          */
        }
        if (leadingExpZeros > 0)
        {         /* need extras?                      */
                  /* fill them in                      */
            memset(resultPtr, '0', leadingExpZeros);
            resultPtr += leadingExpZeros;    /* and step past them                */
        }
        /* now add on the exponent           */
        memcpy(resultPtr, exponent, exponentsize);
    }
    /* blanks needed instead?            */
    else if (mathexp > 0 && !defaultexpsize && temp > (wholenumber_t)exptrigger)
    {
        /* fill them in                      */
        memset(resultPtr, ' ', mathexp + 2);
        resultPtr += mathexp;              /* and step past them                */
                                           /* add on the spaces                 */
    }
    return result;                       /* return the result                 */
}

int RexxNumberString::format(const char *_number, size_t _length)
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
    wholenumber_t  ExpValue;              /* Exponent Value                    */
    size_t    MaxDigits;                  /* Maximum number size               */
    char      ch;                         /* current character                 */
    char      MSDigit = 0;                /* Most Significant digit truncated  */
    const char *InPtr;                    /* Input Data Pointer                */
    char     *OutPtr;                     /* Output Data Pointer               */
    const char *EndData;                  /* Scan end position                 */
    bool      isZero;                     /* Number is zero if true            */
    size_t    resultDigits;               /* Number of digits in result        */


    ExpValue = 0;                       /* Initial Exponent.                 */
    ExpSign = 0;                        /* set exponent sign                 */
    isZero = true;                      /* Assume number will be zero.       */

    InPtr = _number;                    /*Point to start of input string.    */
    EndData = InPtr + _length;          /*Point to end of Data + 1.          */

    while (*InPtr == ch_BLANK || *InPtr == ch_TAB)    /* Ship all leading blanks.          */
    {
        InPtr++;                          /* Skip it, and go on to next char   */
    }
                                          /* Is this a sign Character?         */
    if ((ch = *InPtr) == ch_MINUS || ch == ch_PLUS)
    {
        InPtr++;                           /* Yes, skip it.                     */
        if (ch == ch_MINUS)                /* is it a Minus sign?               */
        {
            this->sign   = -1;                /* Yup, indicate a negative number.  */
        }
    }
    while (*InPtr == ch_BLANK || *InPtr == ch_TAB)   /* Ship all leading blanks.          */
    {
        InPtr++;                          /* Skip it, and go on to next char   */
    }
    ch = *InPtr;                        /* Get 1st Digit.                    */
    MaxDigits = resultDigits = _length; /* Set our max digits counter.       */
    OutPtr = this->number;              /* Point to Output area.             */

                                        /*Skip all leading Zero's            */
    while (*InPtr == ch_ZERO)           /* While 1st Digit is a 0            */
    {
        InPtr++;                          /* Go to next character.             */
    }

                                          /* Have we reach end of number,num   */
                                          /*zero?                              */
    if (InPtr >= EndData)
    {
        SetNumberStringZero();            /* Make value a zero.                */
        return 0;
    }
    /* Now process real digits.          */
    ExpValue = 0;                       /* Start accumulating exponent       */

    if (*InPtr > ch_ZERO && *InPtr <= ch_NINE)
    {
        isZero = false;                    /* found the first non-zero digit    */
    }
    /* While the character is a Digit.   */
    while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE)
    {
        if (MaxDigits)
        {                  /* Still room to add digits          */
                           /* Move digit into output.           */
            *OutPtr++ = (char)(*InPtr++ - '0');
            MaxDigits--;                    /* Now room for one less.            */
        }
        else
        {
            /* Have we found our most Sig Digit  */
            /* and have we not run out of data?  */
            if ((!MSDigit) && (InPtr < EndData))
            {
                MSDigit = *InPtr;             /* Nope, now we have MSD.            */
            }
            InPtr++;                        /* Point to next char                */
            ExpValue++;                     /* Increment the exponent value.     */
        }
    }
    if (InPtr >= EndData)
    {             /* Did we reach end of data?         */
                  /* compute length.                   */
        this->length = (size_t) (resultDigits - MaxDigits);
        this->exp = ExpValue;             /* set exponent value                */
        this->roundUp(MSDigit);
        this->roundUp(MSDigit);           /* Round up the number if necessary  */
        return 0;                         /* all done, just return             */
    }
    /* compute length.                   */
    this->length = (resultDigits - MaxDigits);
    this->exp = ExpValue;               /* set exponent value                */

    if (*InPtr == ch_PERIOD)
    {          /*Decimal point???                   */
        InPtr++;                          /* yes, skip it.                     */
        if (InPtr >= EndData)
        {           /* Did we reach end of data          */
                    /* Yes,  valid digits continue.      */
                    /*is it "0.", or number Zero         */
            if (MaxDigits == resultDigits || isZero)
            {
                this->setZero();              /* make number just zero.            */
            }
            else
            {
                /* Round up the number if necessary  */
                this->roundUp(MSDigit);
            }
            return 0;                       /* All done, exit.                   */
        }
        if (MaxDigits == resultDigits)
        {  /*Any significant digits?            */
           /* No, Ship leading Zeros            */
            while (*InPtr == ch_ZERO)
            {     /* While 1st Digit is a 0            */
                ExpValue--;                   /* decrement exponent.               */
                InPtr++;                      /* Go to next character.             */
                                              /* Have we reach end of number,num   */
                                              /*zero?                              */
                if (InPtr >= EndData)
                {
                    SetNumberStringZero();      /* Make value a zero.                */
                    return 0;
                }
            }
        }
        /* in the range 1-9?                 */
        if (*InPtr > ch_ZERO && *InPtr <= ch_NINE)
        {
            isZero = false;                 /* found the first non-zero digit    */
        }
        /*While there are still digits       */
        while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE)
        {
            if (MaxDigits)
            {                /*if still room for digits           */
                ExpValue--;                   /* Decrement Exponent                */
                                              /* Move char to output               */
                *OutPtr++ = (char)(*InPtr++ - '0');
                MaxDigits--;                  /* Room for one less digit.          */
            }
            else
            {
                if (!MSDigit)                 /* not gotten a most sig digit yet?  */
                {
                    MSDigit = *InPtr;           /* record this one                   */
                }
                InPtr++;                      /* No more room, go to next digit    */
            }
        }
        if (InPtr >= EndData)
        {           /*Are we at end of data?             */
                    /* Compute length of number.         */
            this->length = (resultDigits - MaxDigits);
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
    this->length = resultDigits - MaxDigits;
    if (!this->length)
    {                /* No digits, number is Zero.        */
                     /* Have we reached the end of the    */
                     /*string                             */
        if (InPtr >= EndData)
        {
            /* Yes, all done.                    */
            this->setZero();                 /* make number just zero.            */
            return 0;                        /* All done, exit.                   */
        }
    }
    this->exp = ExpValue;               /* get current exponent value.       */

    if (toupper(*InPtr) == 'E')
    {       /* See if this char is an exponent?  */
        ExpSign = 1;                      /* Assume sign of exponent to '+'    */
        InPtr++;                          /* step over the 'E'                 */
        if (*InPtr == ch_MINUS)
        {         /* If this a minus sign?             */
            ExpSign = -1;                   /*  Yes, make sign of exp '-'        */
            InPtr++;                        /*  go on to next char.              */
        }
        else if (*InPtr == ch_PLUS)       /* If this a plus  sign?             */
        {
            InPtr++;                        /* Yes, skip it and go to next char. */
        }
        ExpValue = 0;                     /* Start of exponent clear work area.*/
        MaxDigits = 0;                    /* claer digit counter.              */

                                          /* Do while we have a valid digit    */
        while (*InPtr >= ch_ZERO && *InPtr <= ch_NINE)
        {
            /* Add this digit to Exponent value. */
            ExpValue = ExpValue * 10 + ((*InPtr++) - '0');
            if (ExpValue > Numerics::MAX_EXPONENT)   /* Exponent can only be 9 digits long*/
            {
                return 1;                     /* if more than that, indicate error.*/
            }
            if (ExpValue)                   /* Any significance in the Exponent? */
            {
                MaxDigits++;                  /*  Yes, bump up the digits counter. */
            }
        }
        this->exp += (ExpValue * ExpSign);/* Compute real exponent.            */
                                          /* Is it bigger than allowed max     */
        if (Numerics::abs(this->exp) > Numerics::MAX_EXPONENT)
        {
            return 1;                       /* yes, indicate error.              */
        }
    }

    if (this->sign == 0 || isZero)
    {    /* Was this really a zero number?    */
        this->setZero();                  /* make number just zero.            */
    }

    this->roundUp(MSDigit);             /* Round up the number if necessary  */
                                        /*is number just flat out too big?   */
    if ((this->exp + (wholenumber_t)this->length - 1) > Numerics::MAX_EXPONENT)
    {
        return 1;                         /* also bad                          */
    }
    return 0;                           /* All done !!                       */
}

void RexxNumberString::formatNumber(wholenumber_t integer)
/******************************************************************************/
/* Function : Format the integer num into a numberstring.                     */
/******************************************************************************/
{
    if (integer == 0)
    {                  /* is integer 0?                     */
                       /* indicate this.                    */
        this->setZero();
    }
    else
    {                               /* number is non-zero                */
                                    /* Format the number                 */
        if (integer < 0 )
        {                /* Negative integer number?          */
            this->sign = -1;
        }
        /* convert value into string         */
        this->length = Numerics::normalizeWholeNumber(integer, (char *)this->number);
    }
}

void RexxNumberString::formatUnsignedNumber(size_t integer)
/******************************************************************************/
/* Function : Format the integer num into a numberstring.                     */
/******************************************************************************/
{
    char  *current;                      /* current position                  */

    if (integer == 0)
    {                  /* is integer 0?                     */
                       /* indicate this.                    */
        this->setZero();
    }
    else
    {                               /* number is non-zero                */
                                    /* Format the number                 */
                                    /* convert value into string         */
        Numerics::formatStringSize(integer, (char *)this->number);
        current = this->number;            /* point to the data start           */
        while (*current != '\0')
        {         /* while still have digits           */
            *current -= '0';                 /* make zero based                   */
            current++;                       /* step to the next one              */
        }
        /* set the proper length             */
        this->length = current - this->number;
    }
}


void RexxNumberString::formatInt64(int64_t integer)
/******************************************************************************/
/* Function : Format the integer num into a numberstring.                     */
/******************************************************************************/
{
    if (integer == 0)
    {                  /* is integer 0?                     */
                       /* indicate this.                    */
        this->setZero();
    }
    else
    {                               /* number is non-zero                */
        // we convert this directly because portable numeric-to-ascii routines
        // don't really exist for the various 32/64 bit values.
        char buffer[32];
        size_t index = sizeof(buffer);

        // negative number?  copy a negative sign, and take the abs value
        if (integer < 0)
        {
            // work from an unsigned version that can hold all of the digits
            // we need to use a version we can negate first, then add the
            // digit back in
            uint64_t working = (uint64_t)(-(integer + 1));
            working++;      // undoes the +1 above
            sign = -1;      // negative number

            while (working > 0)
            {
                // get the digit and reduce the size of the integer
                int digit = (int)(working % 10);
                working = working / 10;
                // store the digit
                buffer[--index] = digit;
            }
        }
        else
        {
            sign = 1;              // positive number
            while (integer > 0)
            {
                // get the digit and reduce the size of the integer
                int digit = (int)(integer % 10);
                integer = integer / 10;
                // store the digit
                buffer[--index] = digit;
            }
        }

        // copy into the buffer and set the length
        this->length = sizeof(buffer) - index;
        memcpy(this->number, &buffer[index], this->length);
    }
}


void RexxNumberString::formatUnsignedInt64(uint64_t integer)
/******************************************************************************/
/* Function : Format the integer num into a numberstring.                     */
/******************************************************************************/
{
    if (integer == 0)
    {                  /* is integer 0?                     */
                       /* indicate this.                    */
        this->setZero();
    }
    else
    {                               /* number is non-zero                */
        // we convert this directly because A)  we need to post-process the numbers
        // to make them zero based, and B) portable numeric-to-ascii routines
        // don't really exist for the various 32/64 bit values.
        char buffer[32];
        size_t index = sizeof(buffer);

        while (integer > 0)
        {
            // get the digit and reduce the size of the integer
            int digit = (int)(integer % 10);
            integer = integer / 10;
            // store the digit
            buffer[--index] = digit;
        }

        // copy into the buffer and set the length
        this->length = sizeof(buffer) - index;
        memcpy(this->number, &buffer[index], this->length);
    }
}


RexxObject *RexxNumberString::unknown(RexxString *msgname, RexxArray *arguments)
/******************************************************************************/
/* Function:  Forward all unknown messages to the numberstring's string       */
/*            representation                                                  */
/******************************************************************************/
{
    return this->stringValue()->sendMessage(msgname, arguments);
}


/**
 * Override for the normal isinstanceof method.  This version
 * allows the NumberStringClass to "lie" about being a string.
 *
 * @param other  The comparison class
 *
 * @return True if the string value is an instance of the target class.
 */
bool RexxNumberString::isInstanceOf(RexxClass *other)
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
RexxMethod *RexxNumberString::instanceMethod(RexxString  *method_name)
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
RexxSupplier *RexxNumberString::instanceMethods(RexxClass *class_object)
{
    return stringValue()->instanceMethods(class_object);
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

bool RexxNumberString::isEqual(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Primitive strict equal\not equal method.  This determines       */
/*            only strict equality, not greater or less than values.          */
/******************************************************************************/
{
    if (this->isSubClassOrEnhanced())      /* not a primitive?                  */
    {
                                           /* do the full lookup compare        */
        return this->sendMessage(OREF_STRICT_EQUAL, other)->truthValue(Error_Logical_value_method);
    }
                                       /* go do a string compare            */
    return this->stringValue()->isEqual(other);
}

wholenumber_t RexxNumberString::strictComp(RexxObject *other)
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

wholenumber_t RexxNumberString::comp(
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
    wholenumber_t      aLexp;            /* adjusted left exponent            */
    wholenumber_t     aRexp;             /* adjusted right exponent           */
    size_t    aLlen;                     /* adjusted left length              */
    size_t    aRlen;                     /* adjusted right length             */
    wholenumber_t      MinExp;           /* minimum exponent                  */
    size_t    NumberDigits;              /* current digits setting            */
    char     *scan;                      /* scan pointer                      */
    wholenumber_t rc;                    /* compare result                    */

                                         /* the compare is acually done by    */
                                         /* subtracting the two numbers, the  */
                                         /* sign of the result obj will be our*/
                                         /* return value.                     */
    requiredArgument(right, ARG_ONE);            /* make sure we have a real value    */
                                         /* get a numberstring object from    */
                                         /*right                              */
    rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)        /* didn't convert?                   */
    {
        /* numbers couldn't be compared      */
        /* numerically, do a string compare. */
        return this->stringValue()->comp(right);
    }

    if (this->sign != rightNumber->sign) /* are numbers the same sign?        */
    {
        /* no, this is easy                  */
        return(this->sign < rightNumber->sign) ? -1 : 1;
    }
    if (rightNumber->sign == 0)          /* right one is zero?                */
    {
        return this->sign;                 /* use this sign                     */
    }
    if (this->sign == 0)                 /* am I zero?                        */
    {
        return rightNumber->sign;          /* return the right sign             */
    }
                                           /* set smaller exponent              */
    MinExp = (rightNumber->exp < this->exp)? rightNumber->exp : this->exp;
    aLexp = this->exp - MinExp;          /* get adjusted left size            */
    aRexp = rightNumber->exp - MinExp;   /* get adjusted right size           */
    aLlen = aLexp + this->length;        /* get adjusted left size            */
    aRlen = aRexp + rightNumber->length; /* get adjusted right size           */
    NumberDigits = number_fuzzydigits(); /* get precision for comparisons.    */
                                         /* can we do a fast exit?            */
    if (aLlen <= NumberDigits && aRlen <= NumberDigits)
    {
        /* longer number is the winner       */
        if (aLlen > aRlen)                 /* left longer?                      */
        {
            return this->sign;               /* use left sign                     */
        }
        else if (aRlen > aLlen)            /* right longer?                     */
        {
            return -this->sign;              /* use inverse of the sign           */
        }
        else
        {
            /* actual lengths the same?          */
            if (this->length == rightNumber->length)
            {
                /* return the comparison result      */
                /* adjusted by the sign value        */
                return memcmp(this->number, rightNumber->number, this->length) * this->sign;
            }
            /* right one shorter?                */
            else if (this->length > rightNumber->length)
            {
                /* compare for shorter length        */
                rc = memcmp(this->number, rightNumber->number, rightNumber->length) * this->sign;
                if (rc == 0)
                {                 /* equal for that length?            */
                                  /* point to the remainder part       */
                    scan = this->number + rightNumber->length;
                    /* get the remainder length          */
                    aRlen = this->length - rightNumber->length;
                    while (aRlen--)
                    {            /* scan the remainder                */
                        if (*scan++ != 0)          /* found a non-zero one?             */
                        {
                            return this->sign;       /* left side is greater              */
                        }
                    }
                    return 0;                    /* these are equal                   */
                }
                return rc;                     /* return compare result             */
            }
            else
            {                           /* left one is shorter               */
                                        /* compare for shorter length        */
                rc = memcmp(this->number, rightNumber->number, this->length) * this->sign;
                if (rc == 0)
                {                 /* equal for that length?            */
                                  /* point to the remainder part       */
                    scan = rightNumber->number + this->length;
                    /* get the remainder length          */
                    aRlen = rightNumber->length - this->length;
                    while (aRlen--)
                    {            /* scan the remainder                */
                        if (*scan++ != 0)          /* found a non-zero one?             */
                        {
                            return -this->sign;      /* right side is greater             */
                        }
                    }
                    return 0;                    /* these are equal                   */
                }
                return rc;                     /* return compare result             */
            }
        }
    }
    else
    {                               /* need to subtract these            */
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


/**
 * Exported version of the HASHCODE method for retrieving the
 * object's hash.
 *
 * @return A string version of the hash (generally holds binary characters).
 */
RexxObject *RexxNumberString::hashCode()
{
    // get the hash value, which is actually derived from the integer string value
    HashCode h = this->hash();
    return new_string((const char *)&h, sizeof(HashCode));
}

RexxInteger *RexxNumberString::strictEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Perform the primitive level "==" compare, including the hash    */
/*            value processing.                                               */
/******************************************************************************/
{
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
    if (right != OREF_NULL)
    {            /* Is this a dyadic operation?       */
                 /* get a numberstring object from    */
                 /*right                              */
        RexxNumberString *rightNumber = right->numberString();
        if (rightNumber == OREF_NULL)      /* is the operand numeric?           */
        {
            /* nope, this is an error            */
            reportException(Error_Conversion_operator, right);
        }
        /* call addsub to do computation     */
        return this->addSub(rightNumber, OT_PLUS, number_digits());
    }
    else
    {
        /* need to format under different    */
        /* precision?                        */
        if (this->stringObject != OREF_NULL || this->NumDigits != number_digits() ||
            (number_form() == Numerics::FORM_SCIENTIFIC && !(this->NumFlags&NumFormScientific)) ||
            (number_form() == Numerics::FORM_ENGINEERING && this->NumFlags&NumFormScientific))
        {
            /* need to copy and reformat         */
            return this->prepareNumber(number_digits(), ROUND);
        }
        else
        {
            return this;                 /* just return the same value        */
        }
    }
}

RexxNumberString *RexxNumberString::minus(RexxObject *right)
/********************************************************************/
/* Function:  Subtraction between two numbers                       */
/********************************************************************/
{
    if (right != OREF_NULL)
    {            /* Is this a dyadic operation?       */
                 /* get a numberstring object from    */
                 /*right                              */
        RexxNumberString *rightNumber = right->numberString();
        if (rightNumber == OREF_NULL)      /* is the operand numeric?           */
        {
            /* nope, this is an error            */
            reportException(Error_Conversion_operator, right);
        }
        /* call addsub to do computation     */
        return this->addSub(rightNumber, OT_MINUS, number_digits());
    }
    else
    {
        /* need to copy and reformat         */
        RexxNumberString *result = this->prepareNumber(number_digits(), ROUND);
        /* invert the sign of our copy.      */
        result->sign = -(result->sign);
        return result;                       /* return addition result            */
    }
}

RexxNumberString *RexxNumberString::multiply(RexxObject *right)
/********************************************************************/
/* Function:  Multiply two numbers                                  */
/********************************************************************/
{
    requiredArgument(right, ARG_ONE);            /* must have an argument             */
                                         /* get a numberstring object from    */
                                         /*right                              */
    RexxNumberString *rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
    {
        /* nope, this is an error            */
        reportException(Error_Conversion_operator, right);
    }
    return this->Multiply(rightNumber);  /* go do the multiply                */
}

RexxNumberString *RexxNumberString::divide(RexxObject *right)
/********************************************************************/
/* Function:  Divide two numbers                                    */
/********************************************************************/
{
    requiredArgument(right, ARG_ONE);            /* must have an argument             */

                                         /* get a numberstring object from    */
                                         /*right                              */
    RexxNumberString *rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
    {
        /* nope, this is an error            */
        reportException(Error_Conversion_operator, right);
    }
    /* go do the division                */
    return this->Division(rightNumber, OT_DIVIDE);
}

RexxNumberString *RexxNumberString::integerDivide(RexxObject *right)
/********************************************************************/
/* Function:  Integer division between two numbers                  */
/********************************************************************/
{
    requiredArgument(right, ARG_ONE);            /* must have an argument             */
                                         /* get a numberstring object from    */
                                         /*right                              */
    RexxNumberString *rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
    {
        /* nope, this is an error            */
        reportException(Error_Conversion_operator, right);
    }
    /* go do the division                */
    return this->Division(rightNumber, OT_INT_DIVIDE);
}

RexxNumberString *RexxNumberString::remainder(RexxObject *right)
/********************************************************************/
/* Function:  Remainder division between two numbers                */
/********************************************************************/
{
    requiredArgument(right, ARG_ONE);            /* must have an argument             */

                                         /* get a numberstring object from    */
                                         /*right                              */
    RexxNumberString *rightNumber = right->numberString();
    if (rightNumber == OREF_NULL)        /* is the operand numeric?           */
    {
        /* nope, this is an error            */
        reportException(Error_Conversion_operator, right);
    }
    /* go do the division                */
    return this->Division(rightNumber, OT_REMAINDER);
}

RexxNumberString *RexxNumberString::abs()
/********************************************************************/
/* Function:  Return the absolute value of a number                 */
/********************************************************************/
{
    RexxNumberString *NewNumber = this->clone();            /* copy the number                   */
    /* inherit the current numeric settings and perform rounding, if */
    /* necessary */
    NewNumber->setupNumber();
    /* switch the sign                   */
    NewNumber->sign = (short)::abs(NewNumber->sign);
    return NewNumber;                     /* and return                        */
}

RexxInteger *RexxNumberString::Sign()
/********************************************************************/
/* Function:  Return the sign of a number                           */
/********************************************************************/
{
    RexxNumberString *NewNumber = this->clone();            /* copy the number                   */
    /* inherit the current numeric settings and perform rounding, if */
    /* necessary */
    NewNumber->setupNumber();
    return new_integer(NewNumber->sign);  /* just return the sign value        */
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
void RexxNumberString::roundUp(int MSDigit)
{
    int  carry;
    char *InPtr;
    char ch;
    /* Did we chop off digits and is it  */
    /* greater/equal to 5, rounding?     */
    if (MSDigit >= ch_FIVE && MSDigit <= ch_NINE)
    {
        /* yes, we have to round up digits   */

        carry = 1;                         /* indicate we have a carry.         */
                                           /* point to last digit.              */
        InPtr = this->number + this->length - 1;

        /* all digits and still have a carry */
        while ((InPtr >= this->number) && carry)
        {
            if (*InPtr == 9)                  /* Is this digit a 9?                */
            {
                ch = 0;                          /* make digit 0, still have carry    */
            }
            else
            {
                ch = *InPtr + 1;                 /* Not nine, just add one to digit.  */
                carry = 0;                       /* No more carry.                    */
            }
            *InPtr-- = ch;                    /* replace digit with new value.     */
        }                                  /* All done rounding.                */

        if (carry)
        {                       /* Do we still have a carry?         */
                                /* yes, carry rippled all the way    */
            *this->number = 1;                /*  set 1st digit to a 1.            */
            this->exp += 1;                   /* increment exponent by one.        */
        }
    }
}

RexxString *RexxNumberString::d2x(
     RexxObject *_length)               /* result length                     */
/******************************************************************************/
/* Function:  Convert a valid numberstring to a hex string.                   */
/******************************************************************************/
{
                                       /* forward to the formatting routine */
    return this->d2xD2c(_length, false);
}

RexxString *RexxNumberString::d2c(
     RexxObject *_length)               /* result length                     */
/******************************************************************************/
/* Function:  Convert a valid numberstring to a character string.             */
/******************************************************************************/
{
                                       /* forward to the formatting routine */
    return this->d2xD2c(_length, true);
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
     RexxObject *_length,              /* result length                     */
     bool  type )                      /* D2C or D2X flag                   */
/******************************************************************************/
/* Function:  Convert a valid numberstring to a hex or character string.      */
/******************************************************************************/

{
    char       PadChar;                  /* needed padding character          */
    stringsize_t ResultSize;             /* size of result string             */
    size_t     HexLength;                /* length of hex characters          */
    size_t     BufferLength;             /* length of the buffer              */
    char     * Scan;                     /* scan pointer                      */
    char     * HighDigit;                /* highest digit location            */
    char     * Accumulator;              /* accumulator pointer               */
    char     * TempPtr;                  /* temporary pointer value           */
    size_t     PadSize;                  /* needed padding                    */
    size_t     CurrentDigits;            /* current digits setting            */
    size_t     TargetLength;             /* length of current number          */
    RexxBuffer *Target;                  /* formatted number                  */
    RexxString *Retval;                  /* returned result                   */


                                         /* get the target length             */
    ResultSize = optionalLengthArgument(_length, SIZE_MAX, ARG_ONE);
    CurrentDigits = number_digits();     /* get the current digits setting    */
    TargetLength = this->length;         /* copy the length                   */
                                         /* too big to process?               */
    if (this->exp + this->length > CurrentDigits)
    {
        if (type == true)                  /* d2c form?                         */
        {
            /* use that message                  */
            reportException(Error_Incorrect_method_d2c, this);
        }
        else                               /* use d2x form                      */
        {
            reportException(Error_Incorrect_method_d2x, this);
        }
    }
    else if (this->exp < 0)
    {            /* may have trailing zeros           */
                 /* point to the decimal part         */
        TempPtr = this->number + this->length + this->exp;
        HexLength = -this->exp;            /* get the length to check           */
                                           /* point to the rounding digit       */
        HighDigit = this->number + CurrentDigits;
        /* while more decimals               */
        while (HexLength -- && TempPtr <= HighDigit)
        {
            if (*TempPtr != 0)
            {             /* non-zero decimal?                 */
                          /* this may be non-significant       */
                if (TargetLength > CurrentDigits)
                {
                    /* this the "rounding" digit?        */
                    if (TempPtr == HighDigit && *TempPtr < 5)
                    {
                        break;                     /* insignificant digit found         */
                    }
                }
                if (type == true)              /* d2c form?                         */
                {
                    /* use that message                  */
                    reportException(Error_Incorrect_method_d2c, this);
                }
                else                           /* use d2x form                      */
                {
                    reportException(Error_Incorrect_method_d2x, this);
                }
            }
            TempPtr++;                       /* step the pointer                  */
        }
        /* adjust the length                 */
        TargetLength = this->length + this->exp;
    }
    /* negative without a size           */
    if (this->sign < 0 && ResultSize == SIZE_MAX)
    {
        /* this is an error                  */
        reportException(Error_Incorrect_method_d2xd2c);
    }
    if (ResultSize == SIZE_MAX)          /* using default size?               */
    {
        /* allocate buffer based on digits   */
        BufferLength = CurrentDigits + OVERFLOWSPACE;
    }
    else if (type == true)
    {             /* X2C function?                     */
        if (ResultSize * 2 < CurrentDigits)/* smaller than digits setting?      */
        {
            /* allocate buffer based on digits   */
            BufferLength = CurrentDigits + OVERFLOWSPACE;
        }
        else                               /* allocate a large buffer           */
        {
            BufferLength = (ResultSize * 2) + OVERFLOWSPACE;
        }
    }
    else
    {                               /* D2X function                      */
        if (ResultSize < CurrentDigits)    /* smaller than digits setting?      */
        {
            /* allocate buffer based on digits   */
            BufferLength = CurrentDigits + OVERFLOWSPACE;
        }
        else                               /* allocate a large buffer           */
        {
            BufferLength = ResultSize + OVERFLOWSPACE;
        }
    }
    Target = new_buffer(BufferLength);   /* set up format buffer              */
    Scan = this->number;                 /* point to first digit              */
                                         /* set accumulator pointer           */
    Accumulator = Target->getData() + BufferLength - 2;
    HighDigit = Accumulator - 1;         /* set initial high position         */
                                         /* clear the accumulator             */
    memset(Target->getData(), '\0', BufferLength);
    while (TargetLength--)
    {             /* while more digits                 */
                  /* add next digit                    */
        HighDigit = addToBaseSixteen(*Scan++, Accumulator, HighDigit);
        if (TargetLength != 0)             /* not last digit?                   */
        {
            /* do another multiply               */
            HighDigit = multiplyBaseSixteen(Accumulator, HighDigit);
        }
    }
    if (this->exp > 0)
    {                 /* have extra digits to worry about? */
                      /* do another multiply               */
        HighDigit = multiplyBaseSixteen(Accumulator, HighDigit);
        TargetLength = this->exp;          /* copy the exponent                 */
        while (TargetLength--)
        {           /* while more digits                 */
                    /* add next zero digit               */
            HighDigit = addToBaseSixteen('\0', Accumulator, HighDigit);
            if (TargetLength != 0)           /* not last digit?                   */
            {
                /* do the multiply                   */
                HighDigit = multiplyBaseSixteen(Accumulator, HighDigit);
            }
        }
    }
    HexLength = Accumulator - HighDigit; /* get accumulator length            */
    if (this->sign < 0)
    {                /* have a negative number?           */
                     /* take twos complement              */
        PadChar = 'F';                     /* pad negatives with foxes          */
        Scan = Accumulator;                /* point to last digit               */
        while (!*Scan)                     /* handle any borrows                */
        {
            *Scan-- = 0xf;                   /* make digit a 15                   */
        }
        *Scan = *Scan - 1;                 /* subtract the 1                    */
        Scan = Accumulator;                /* start at first digit again        */
        while (Scan > HighDigit)
        {         /* invert all the bits               */
                  /* one digit at a time               */
            *Scan = (char)(*Scan ^ (unsigned)0x0f);
            Scan--;                          /* step to next digit                */
        }
    }
    else
    {
        PadChar = '0';                     /* pad positives with zero           */
    }
                                           /* now make number printable         */
    Scan = Accumulator;                  /* start at first digit again        */
    while (Scan > HighDigit)
    {           /* convert all the nibbles           */
        *Scan = IntToHexDigit(*Scan);      /* one digit at a time               */
        Scan--;                            /* step to next digit                */
    }
    Scan = HighDigit + 1;                /* point to first digit              */

    if (type == false)
    {                 /* d2x function ?                    */
        if (ResultSize == SIZE_MAX)        /* using default length?             */
        {
            ResultSize = HexLength;          /* use actual data length            */
        }
    }
    else
    {                               /* d2c function                      */
        if (ResultSize == SIZE_MAX)        /* using default length?             */
        {
            ResultSize = HexLength;          /* use actual data length            */
        }
        else
        {
            ResultSize += ResultSize;        /* double the size                   */
        }
    }
    if (ResultSize < HexLength)
    {        /* need to truncate?                 */
        PadSize = 0;                       /* no Padding                        */
        Scan += HexLength - ResultSize;    /* step the pointer                  */
        HexLength = ResultSize;            /* adjust number of digits           */
    }
    else                                 /* possible padding                  */
    {
        PadSize = ResultSize - HexLength;  /* calculate needed padding          */
    }
    if (PadSize)
    {                       /* padding needed?                   */
        Scan -= PadSize;                   /* step back the pointer             */
        memset(Scan, PadChar, PadSize);    /* pad in front                      */
    }
    if (type == true)                    /* need to pack?                     */
    {
        Retval = StringUtil::packHex(Scan, ResultSize);/* yes, pack to character            */
    }
    else
    {
        /* allocate result string            */
        Retval = new_string(Scan, ResultSize);
    }
    return Retval;                       /* return proper result              */
}


RexxObject  *RexxNumberString::getValue(
    RexxActivation *context)           /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return (RexxObject *)this;           /* just return this value            */
}


RexxObject  *RexxNumberString::getValue(
    RexxVariableDictionary *context)   /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return (RexxObject *)this;           /* just return this value            */
}


RexxObject  *RexxNumberString::getRealValue(
    RexxActivation *context)           /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return (RexxObject *)this;           /* just return this value            */
}


RexxObject  *RexxNumberString::getRealValue(
    RexxVariableDictionary *context)   /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return (RexxObject *)this;           /* just return this value            */
}

void  *RexxNumberString::operator new(size_t size, size_t length)
/******************************************************************************/
/* Function:  Create a new NumberString object                                */
/******************************************************************************/
{
    RexxNumberString *newNumber = (RexxNumberString *)new_object(size + length, T_NumberString);
    /* initialize the new object         */
    newNumber->setHasNoReferences();     /* Let GC know no to bother with LIVE*/
    return newNumber;                    /* return the new numberstring       */
}

RexxNumberString *RexxNumberString::newInstance(const char *number, stringsize_t len)
/******************************************************************************/
/* Function:  Create a new number string object                               */
/******************************************************************************/
{
    RexxNumberString *newNumber;

    if (number == NULL)
    {                /* asking for a dummy string?        */
                     /* allocate a new string             */
        newNumber = new (len) RexxNumberString (len);
        /* make it a zero value              */
        newNumber->setZero();
        return newNumber;                  /* return this now                   */
    }
    /* scan the string 1st to see if its */
    /*valid                              */
    if (numberStringScan(number, len))
    {
        newNumber = OREF_NULL;             /* not a valid number, get out now.  */
    }
    else
    {
        /* looks to be valid.  get a new     */
        /* format it                         */
        newNumber = new (len) RexxNumberString (len);
        /* now see if the data actually is   */
        /*  a number and fill in actual data */
        /* NOTE: even though a scan has been */
        /*   we still may not have a thorough*/
        /*   enough check.                   */
        if (newNumber->format(number, len))
        {
            /* number didn't convert,            */
            newNumber = OREF_NULL;
        }
    }
    return newNumber;
}

RexxNumberString *RexxNumberString::newInstanceFromFloat(float num)
/******************************************************************************/
/* Function:  Create a numberstring object from a floating point number       */
/******************************************************************************/
{
    return newInstanceFromDouble((double)num, number_digits());
}

RexxNumberString *RexxNumberString::newInstanceFromDouble(double number)
/******************************************************************************/
/* Function:  Create a NumberString from a double value                       */
/******************************************************************************/
{
    return newInstanceFromDouble(number, number_digits());
}


/**
 * Create a numberstring from a double value using a given
 * formatting precision.
 *
 * @param number    The double number to convert
 * @param precision The precision to apply to the result.
 *
 * @return The formatted number, as a numberstring value.
 */
RexxNumberString *RexxNumberString::newInstanceFromDouble(double number, size_t precision)
{
    // make a nan value a string value
    if (isnan(number))
    {
        return (RexxNumberString *)new_string("nan");
    }
    else if (number == +HUGE_VAL)
    {
        return (RexxNumberString *)new_string("+infinity");
    }
    else if (number == -HUGE_VAL)
    {
        return (RexxNumberString *)new_string("-infinity");
    }

    RexxNumberString *result;
    size_t resultLen;
    /* Max length of double str is       */
    /*  22, make 30 just to be safe      */
    char doubleStr[30];
    /* get double as a string value.     */
    /* Use digits as precision.          */
    sprintf(doubleStr, "%.*g", (int)(precision + 2), number);
    resultLen = strlen(doubleStr);       /* Compute length of floatString     */
                                         /* Create new NumberString           */
    result = new (resultLen) RexxNumberString (resultLen, precision);
    /* now format as a numberstring      */
    result->format(doubleStr, resultLen);
    return result->prepareNumber(precision, ROUND);
}

RexxNumberString *RexxNumberString::newInstanceFromWholenumber(wholenumber_t integer)
/******************************************************************************/
/* Function:  Create a NumberString object from a wholenumber_t value         */
/******************************************************************************/
{
    // the size of the integer depends on the platform, 32-bit or 64-bit.
    // ARGUMENT_DIGITS ensures the correct value
    RexxNumberString *newNumber = new (Numerics::ARGUMENT_DIGITS) RexxNumberString (Numerics::ARGUMENT_DIGITS);
    newNumber->formatNumber(integer);      /* format the number               */
    return newNumber;
}

RexxNumberString *RexxNumberString::newInstanceFromStringsize(stringsize_t integer)
/******************************************************************************/
/* Function:  Create a NumberString object from a size_t value                */
/******************************************************************************/
{
    // the size of the integer depends on the platform, 32-bit or 64-bit.
    // ARGUMENT_DIGITS ensures the correct value
    RexxNumberString *newNumber = new (Numerics::ARGUMENT_DIGITS) RexxNumberString (Numerics::ARGUMENT_DIGITS);
    newNumber->formatUnsignedNumber(integer);     /* format the number        */
    return newNumber;
}


RexxNumberString *RexxNumberString::newInstanceFromInt64(int64_t integer)
/******************************************************************************/
/* Function:  Create a NumberString object from a signed 64 bit number        */
/******************************************************************************/
{
    // this give us space for entire binary range of the int64_t number.
    RexxNumberString *newNumber = new (Numerics::DIGITS64) RexxNumberString (Numerics::DIGITS64);
    newNumber->formatInt64(integer);  /* format the number                    */
    return newNumber;
}


RexxNumberString *RexxNumberString::newInstanceFromUint64(uint64_t integer)
/******************************************************************************/
/* Function:  Create a NumberString object from an unsigned 64 bit number     */
/******************************************************************************/
{
    // this give us space for entire binary range of the uint64_t number.
    RexxNumberString *newNumber = new (Numerics::DIGITS64) RexxNumberString (Numerics::DIGITS64);
    newNumber->formatUnsignedInt64(integer);  /* format the number            */
    return newNumber;
}

                                       /* numberstring operator methods     */
PCPPM RexxNumberString::operatorMethods[] =
{
   NULL,                               /* first entry not used              */
   (PCPPM)&RexxNumberString::plus,
   (PCPPM)&RexxNumberString::minus,
   (PCPPM)&RexxNumberString::multiply,
   (PCPPM)&RexxNumberString::divide,
   (PCPPM)&RexxNumberString::integerDivide,
   (PCPPM)&RexxNumberString::remainder,
   (PCPPM)&RexxNumberString::power,
   (PCPPM)&RexxNumberString::concat,
   (PCPPM)&RexxNumberString::concat, /* Duplicate entry neccessary        */
   (PCPPM)&RexxNumberString::concatBlank,
   (PCPPM)&RexxNumberString::equal,
   (PCPPM)&RexxNumberString::notEqual,
   (PCPPM)&RexxNumberString::isGreaterThan,
   (PCPPM)&RexxNumberString::isLessOrEqual,
   (PCPPM)&RexxNumberString::isLessThan,
   (PCPPM)&RexxNumberString::isGreaterOrEqual,
                           /* Duplicate entry neccessary        */
   (PCPPM)&RexxNumberString::isGreaterOrEqual,
   (PCPPM)&RexxNumberString::isLessOrEqual,
   (PCPPM)&RexxNumberString::strictEqual,
   (PCPPM)&RexxNumberString::strictNotEqual,
   (PCPPM)&RexxNumberString::strictGreaterThan,
   (PCPPM)&RexxNumberString::strictLessOrEqual,
   (PCPPM)&RexxNumberString::strictLessThan,
   (PCPPM)&RexxNumberString::strictGreaterOrEqual,
                           /* Duplicate entry neccessary        */
   (PCPPM)&RexxNumberString::strictGreaterOrEqual,
   (PCPPM)&RexxNumberString::strictLessOrEqual,
   (PCPPM)&RexxNumberString::notEqual,
                           /* Duplicate entry neccessary        */
   (PCPPM)&RexxNumberString::notEqual,
   (PCPPM)&RexxNumberString::andOp,
   (PCPPM)&RexxNumberString::orOp,
   (PCPPM)&RexxNumberString::xorOp,
   (PCPPM)&RexxNumberString::operatorNot,
};
