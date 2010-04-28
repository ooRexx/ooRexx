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
/* REXX Kernel                                                  okmath.c      */
/*                                                                            */
/* Arithemtic function for the NumberString Class                             */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "RexxCore.h"
#include "StringClass.hpp"
#include "NumberStringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivity.hpp"
#include "BufferClass.hpp"
#include "NumberStringMath.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"


RexxNumberString *RexxNumberString::maxMin(RexxObject **args, size_t argCount, unsigned int operation)
/*********************************************************************/
/* Function:  Process the MAX and MIN builtin functions and methods  */
/*********************************************************************/
{
    size_t arg;
    RexxNumberString *compobj, *maxminobj;
    RexxInteger *compResult;
    RexxObject *nextObject;
    size_t saveFuzz, saveDigits;

    if (argCount == 0) return this;       /* any arguments to ccompare?        */

                                          /* Get a reference to our current act*/
    RexxActivationBase *CurrentActivation = ActivityManager::currentActivity->getTopStackFrame();

    saveFuzz = CurrentActivation->fuzz(); /* get the current fuzz and digits   */
    saveDigits = CurrentActivation->digits();
    CurrentActivation->setFuzz(0);        /* set the fuzz setting to 0         */

                                          /* assume 1st operand (self) is the  */
                                          /*  one we want !                    */
    maxminobj = this->prepareNumber(saveDigits, ROUND);
    ProtectedObject p(maxminobj);
    for (arg=0; arg < argCount; arg++)
    {  /* Loop through all args             */
        nextObject = args[arg];              /* Get next argument.                */

        if (nextObject == OREF_NULL)
        {       /* Was this arg omitted?             */
                /*  Yes, report the error.           */
                /* but restore the fuzz first        */
            CurrentActivation->setFuzz(saveFuzz);
            if (operation == OT_MAX)            /*  doing a MAX operation?           */
            {
                reportException(Error_Incorrect_call_noarg, CHAR_ORXMAX, arg + 1);
            }
            else                                /*  nope must be min.                */
            {
                reportException(Error_Incorrect_call_noarg, CHAR_ORXMIN, arg + 1);
            }
        }

        compobj = nextObject->numberString();/* get a numberstring object         */
        if (compobj != OREF_NULL)
        {          /* Was conversion sucessfull?        */
                   /* get new comp object in right      */
                   /* digits                            */
            compobj = compobj->prepareNumber(saveDigits, ROUND);

            /* Just compare the two NumberStrings*/
            /*  See if new number is greater than*/
            /* See if we have a new MAX          */
            if (operation == OT_MAX)
            {
                compResult = compobj->isGreaterThan(maxminobj);

            }
            else
            {                              /* must be looking for MIN           */
                                           /*  is new less than old maxmin      */
                                           /*  if so we have a new maxmin.      */
                compResult = compobj->isLessThan(maxminobj);
            }


            if (compResult == TheTrueObject)
            {  /* Do we have a new MAX/MIN ?        */
               /* Yes, no need to save old MAX/MIN  */
               /*  assign and protect our next      */
               /*  MAX/MIN                          */
                p = compobj;
                maxminobj = (RexxNumberString *)compobj;
            }
        }
        else
        {                               /* invalid object type.              */
                                        /* be sure we restore original Fuzz  */
            CurrentActivation->setFuzz(saveFuzz);
            /* keep maxminobj around just a      */
            reportException(Error_Incorrect_method_number, arg + 1, args[arg]);
        }
    }
    CurrentActivation->setFuzz(saveFuzz); /* be sure we restore original Fuzz  */
                                          /* We have already made a            */
                                          /* copy/converted the MaxMin object  */
                                          /* and it is adjusted to the correct */
                                          /* precision, so we have nothing     */
                                          /* left to do.                       */
    return maxminobj;                     /* now return it.                    */
}

void RexxNumberStringBase::mathRound(
    char  *NumPtr)                     /* first digit to round up           */
/*********************************************************************/
/* Function:  Adjust a a number string object to correct NUMERIC     */
/*            DIGITS setting                                         */
/*********************************************************************/
{
    int carry, RoundNum;
    size_t resultDigits;
    wholenumber_t numVal;

    resultDigits = this->length;          /* get number of digits in number    */
    NumPtr += this->length;               /* point one past last digit, this   */
                                          /* gives us most significant digit of*/
                                          /* digits being truncated.           */

    if (*NumPtr-- >= 5 )
    {                /* is this digit grater_equal to 5   */
        carry = 1;                           /* yes, we need to round up next dig */
        while (carry && resultDigits-- > 0)
        {/* do while we have carry and digits */
         /* left to round up in number        */
            if (*NumPtr == 9)                  /* is this digit 9?                  */
            {
                RoundNum = 0;                     /* Yes, then this digit will be 0    */
            }
            else
            {
                RoundNum = *NumPtr + carry;       /* Not 9, add 1 (carry) to digit     */
                carry = 0;                        /* no more carry.                    */
            }
            *NumPtr-- = (char)RoundNum;        /* Set this digit in data area.      */
        }

        if (carry )
        {                        /* Done will all numbers, do we still*/
                                 /* have a carry (Did carry propogate */
                                 /* all the way through?              */
            *++NumPtr = 1;                      /* yes, set high digit to 1          */
            this->exp += 1;                     /*      increment exponent by one.   */
        }
    }

    /* At this point number is all setup,*/
    /*  Check for overflow               */
    numVal = this->exp + this->length - 1;
    if (numVal > Numerics::MAX_EXPONENT)
    {
        reportException(Error_Overflow_expoverflow, numVal, Numerics::DEFAULT_DIGITS);
    }
    /*  Check for underflow.             */
    else if (this->exp < Numerics::MIN_EXPONENT)
    {
        reportException(Error_Overflow_expunderflow, this->exp, Numerics::DEFAULT_DIGITS);
    }
    return;
}

void RexxNumberString::adjustPrecision()
/*********************************************************************/
/* Function:  Adjust the precision of a number to the given digits   */
/*********************************************************************/
{
    wholenumber_t resultVal;
    /* is length of number too big?      */
    if (this->length > NumDigits)
    {
        size_t extra = this->length - NumDigits;
        this->length = NumDigits;         /* Yes, make length equal precision  */
        this->exp += extra;               /* adjust exponent by amount over    */
        this->mathRound(number);          /* Round the adjusted number         */
    }

    /* Was number reduced to zero?       */
    if (*number == 0 && this->length == 1)
    {
        this->setZero();                /* Yes, make it so...                */
    }
    else
    {

        /* At this point number is all setup,*/
        /*  Check for overflow               */
        resultVal = this->exp + this->length - 1;
        if (resultVal > Numerics::MAX_EXPONENT)
        {
            reportException(Error_Overflow_expoverflow, resultVal, Numerics::DEFAULT_DIGITS);
        }
        else if (this->exp < Numerics::MIN_EXPONENT)
        {      /*  Check for underflow.             */
            reportException(Error_Overflow_expunderflow, this->exp, Numerics::DEFAULT_DIGITS);
        }
    }
    return;                               /* just return to caller.            */
}

size_t RexxNumberString::highBits(size_t number)
/*********************************************************************/
/* Function:  Determine high order bit position of an unsigned       */
/*            number setting.                                        */
/*********************************************************************/
{
    size_t HighBit;

    if (number == 0)                      /* is number 0?                      */
    {
        return 0;                         /*  Yes, just return 0, no high bit  */
    }

    HighBit = LONGBITS;                   /* get number of digits in number    */

    while ((number & HIBIT) == 0)
    {     /* loops though all bit positions    */
          /*  until first 1 bit is found.      */
        number <<= 1;                        /* shift number one bit pos left.    */
        HighBit--;                           /* decrement i, high bit not found   */
    }

    return HighBit;                       /* return count.                     */
}

char *RexxNumberStringBase::adjustNumber(
    char *NumPtr,                      /* pointer to number data            */
    char  *result,                     /* result location                   */
    size_t resultLen,                  /* result length                     */
    size_t NumberDigits)               /* required digits setting           */
/*********************************************************************/
/* Function:  Adjust the number data to be with the correct NUMERIC  */
/*            DIGITS setting.                                        */
/*********************************************************************/
{
    /* Remove all leading zeros          */
    NumPtr = this->stripLeadingZeros(NumPtr);

    /* is length of number too big?      */
    if (this->length > NumberDigits)
    {
        this->length = NumberDigits;         /* Yes, make length equal precision  */
                                             /* adjust exponent by amount over    */
        this->exp += this->length - NumberDigits;
        this->mathRound(NumPtr);             /* Round the adjusted number         */
    }

    if (resultLen == 0)                   /* See if there is anything to copy? */
    {
        return result;                       /* Nope, just return result Pointer. */
    }
    else
    {
        /* Copy the data into the result area*/
        /*  and pointer to start of data     */
        return(char *)memcpy(((result + resultLen - 1) - this->length), NumPtr, this->length);
    }
}

char *RexxNumberStringBase::stripLeadingZeros(
    char *AccumPtr)                    /* current accumulator position      */
/*********************************************************************/
/* Function:  Remove all leading zeros from a number                 */
/*********************************************************************/
{
    /* while leading digit is zero and   */
    /* still data in object              */
    while (!*AccumPtr && this->length>1)
    {
        AccumPtr++;                         /* Point to next digit.              */
        this->length--;                     /* length od number is one less.     */
    }
    return AccumPtr;                      /* return pointer to 1st non-zero    */
}

void RexxNumberString::adjustPrecision(char *resultPtr, size_t NumberDigits)
/*********************************************************************/
/* Function:  Adjust the precision of a number to the given digits   */
/*********************************************************************/
{
    bool  CopyData;
    wholenumber_t resultVal;
    /* resultPtr will either point to    */
    /* the actual result data or be      */
    /* NULL, if the data is already in   */
    /* result obj.                       */
    if (resultPtr == NULL)
    {              /* Is resultPtr NULL, that is data   */
                   /*  already in result object?        */
        CopyData = false;                    /* Yes, don't copy data.             */
                                             /* have data pointer point to data in*/
        resultPtr = this->number;            /* The result object.                */
    }
    else
    {
        CopyData = true;                     /* resultPtr not null, need to copy  */
    }
    /* is length of number too big?      */
    if (this->length > NumberDigits)
    {
        size_t extra = this->length - NumberDigits;
        this->length = NumberDigits;        /* Yes, make length equal precision  */
        this->exp += extra;                 /* adjust exponent by amount over    */
        this->mathRound(resultPtr);         /* Round the adjusted number         */
    }

    if (CopyData)
    {                       /* only remove leading zeros if      */
                            /* data note already in the object.  */
                            /* remove any leading zeros          */
        resultPtr = this->stripLeadingZeros(resultPtr);
        /* Move result data into object      */
        memcpy(this->number, resultPtr, (size_t)this->length);
    }

    /* make sure this number has the correct numeric settings */
    setNumericSettings(NumberDigits, number_form());

    if (!*resultPtr && this->length == 1) /* Was number reduced to zero?       */
        this->setZero();                     /* Yes, make it so...                */
    else
    {

        /* At this point number is all setup,*/
        /*  Check for overflow               */
        resultVal = this->exp + this->length - 1;
        if (resultVal > Numerics::MAX_EXPONENT)
        {
            reportException(Error_Overflow_expoverflow, resultVal, Numerics::DEFAULT_DIGITS);
        }
        else if (this->exp < Numerics::MIN_EXPONENT)
        {      /*  Check for underflow.             */
            reportException(Error_Overflow_expunderflow, this->exp, Numerics::DEFAULT_DIGITS);
        }
    }
    return;                               /* just return to caller.            */
}

RexxNumberString *RexxNumberString::prepareNumber(size_t NumberDigits, bool rounding)
/*********************************************************************/
/* Function:  Create new copy of supplied object and make sure the   */
/*            number is computed to correct digits setting           */
/*********************************************************************/
{
    /* clone ourselves                   */
    RexxNumberString *newObj = this->clone();
    if (newObj->length > NumberDigits)
    {  /* is the length larger than digits()*/
       /* raise a numeric condition, may    */
       /*  not return from this.            */
        reportCondition(OREF_LOSTDIGITS, (RexxString *)newObj);
        /* adjust exponet by amount over     */
        /* precision                         */
        newObj->exp += newObj->length - NumberDigits;
        newObj->length = NumberDigits;       /* make length equal precision       */
        if (rounding == ROUND)
        {             /* are we to perform rounding?       */
                      /* Round the adjusted number         */
            newObj->mathRound(newObj->number);
        }
    }
    /* make sure this has the correct settings */
    newObj->setNumericSettings(NumberDigits, number_form());
    return newObj;                        /* return new object to caller.      */
}


RexxNumberString *RexxNumberString::addSub(
  RexxNumberString *other,             /* other addition/subtract target    */
  unsigned int operation,              /* add or subtract operation         */
  size_t NumberDigits )                /* precision to use                  */
/*********************************************************************/
/* Function:  Add or subtract two normalized numbers                 */
/*********************************************************************/
{
    RexxNumberString *left, *right, *result, *temp1;
    char  *leftPtr,*rightPtr,*resultPtr;
    char  *resultBuffer = NULL;
    int   right_sign, carry, addDigit, rc;
    wholenumber_t  LadjustDigits, RadjustDigits;
    wholenumber_t  adjustDigits;
    size_t ResultSize;
    wholenumber_t aLeftExp, aRightExp, rightExp, leftExp, MinExp;
    size_t rightLength, leftLength;
    char  resultBufFast[FASTDIGITS*2+1];  /* for fast allocation if default    */
    size_t maxLength = NumberDigits + 1;

    /* Make copies of the input object   */
    /* since we may need to adjust       */
    /* values, and make these new        */
    /*objects safe.                      */
    left = this;
    right = other;
    leftExp = left->exp;                  /* maintain our own copy of exponent */
    rightExp = right->exp;                /* these may be adjusted below.      */

    leftLength = left->length;            /* maintain our own copy of lengths  */
    rightLength = right->length;          /* these may be adjusted below.      */

    if (leftLength > maxLength)
    {
        /* raise a numeric condition, may,   */
        reportCondition(OREF_LOSTDIGITS, (RexxString *)this);
        /*  not return from this.            */
        leftExp += leftLength - maxLength;
        leftLength = maxLength;
    }
    if (rightLength > maxLength)
    {
        /* raise a numeric condition, may    */
        reportCondition(OREF_LOSTDIGITS, (RexxString *)other);
        /*  not return from this.            */
        rightExp += rightLength - maxLength;
        rightLength = maxLength;
    }

    if (leftExp <= rightExp)              /* Find the smaller of the two exps. */
    {
        MinExp = leftExp;
    }
    else
    {
        MinExp = rightExp;
    }


    aLeftExp  = leftExp - MinExp;         /* Compute adjusted Left exponent    */
    aRightExp = rightExp - MinExp;        /* Compute adjusted Right exponent   */
    right_sign = right->sign;             /* make a copy of the right objs sign*/
                                          /* Now check if either number is     */
                                          /*  zero or has dominance over       */
                                          /*  the other number.                */
    temp1=NULL;
    if (left->sign==0)
    {                 /* Is left number zero               */
        temp1=right;                         /*  Yes, set up to return right      */
    }
    else if (right->sign==0)
    {          /* Is right number zero ?            */
        temp1=left;                          /*  Yes, Setup up to return left     */

                                             /* Is the left number completely     */
                                             /*  dominant, that is can the right  */
                                             /*  number affect the outcome of the */
                                             /*  arithmetic operation?            */
    }
    else if ((aLeftExp + (wholenumber_t)leftLength) > (wholenumber_t)(rightLength + NumberDigits))
    {
        temp1=left;                          /*   Yes,  so return the left number.*/
                                             /* Is the right number completely    */
                                             /*  dominant, that is can the left   */
                                             /*  number affect the outcome of the */
                                             /*  arithmetic operation?            */
    }
    else if ((aRightExp + (wholenumber_t)rightLength) > (wholenumber_t)(leftLength + NumberDigits))
    {
        temp1=right;                         /*   Yes, so setup to return right   */
    }
    /* Temp1 will either point to null   */
    /* or to the number object that      */
    /* should be returned as the result. */
    /* A null means we really do need to */
    /* do that math.                     */
    if (temp1)
    {                         /* Can we do a quick return?         */
        result = temp1->clone();             /* Yes, copy temp1 for result        */

                                             /* are we doing subtraction and      */
                                             /* the right operand is our result?  */
        if ((temp1 == right) && (operation == OT_MINUS))
        {
            result->sign = -result->sign;       /* yes, make sign of result oposite  */
        }
                                                /* of the right operand.             */
                                                /* Get result into correct precision.*/
        result->setupNumber();
        return result;                       /* and return new result object      */
    }
    /* We really need to perfrom the     */
    /*  arithmetic, so lets do it.       */
    if (operation == OT_MINUS)
    {          /* is this a subtraction request?    */
        right_sign  = -right_sign;           /*  yes, invert sign of right number */
                                             /*  to make addition.                */
    }

    if (right_sign != left->sign)
    {       /* are the two signs equal?          */
            /*  nope, see if numbers are equal?  */
        if (((leftLength == rightLength) && (leftExp == rightExp)) &&
            !(memcmp(left->number, right->number, leftLength)) )
        {

            return new_numberstring("0", 1);   /* Yes, return Zero as result.       */
        }
        operation = OT_MINUS;                /* We are now doing a subtraction.   */
    }
    else
    {                                /* signs are equal, it is addition   */
        operation = OT_PLUS;                 /* We will now do a addition.        */
    }

    ResultSize = maxLength;               /* size will be NUMERIC DIGITS + 1   */
    result = (RexxNumberString *) new (ResultSize) RexxNumberString (ResultSize);

    result->sign = 0;                     /* make sure all values are zero     */
    result->exp=0;                        /* to start with ...                 */
    result->length=0;

    /* LeftPtr points to end of number   */
    leftPtr = left->number + leftLength - 1;
    /* RightPtr points to end of number  */
    rightPtr = right->number + rightLength - 1;
    resultPtr = NULL;                     /* Initialize Result Ptr to 0;       */


                                          /* See if we need oto adjust the     */
                                          /* number with respect to current    */
                                          /* Digits setting.                   */
                                          /* Compute the amount we may need to */
                                          /* adjust for each number. Using the */
                                          /* Adjusted exponents ....           */
                                          /* a value of 0 or less means we are */
                                          /*OK.                                */
    LadjustDigits = ((wholenumber_t)leftLength + aLeftExp) - (wholenumber_t)(maxLength);
    RadjustDigits = ((wholenumber_t)rightLength + aRightExp) - (wholenumber_t)(maxLength);
    /* Do we need to adjust any numbers? */
    if (LadjustDigits > 0 || RadjustDigits >0 )
    {
        if (LadjustDigits >= RadjustDigits)  /* yes, find which number needs to be*/
        {
            /* adjusted the most and assign the  */
            adjustDigits = LadjustDigits;       /* adjustment value to adjustDigits. */
        }
        else
        {
            adjustDigits = RadjustDigits;
        }
        /* now we adjust the adjust exp      */
        /* and real length/exp of each no.   */


        if (aLeftExp)
        {                      /* Was left exp larger than right    */
            if ((wholenumber_t)adjustDigits < aLeftExp)
            { /* Do we need to adjust for more than*/
                /* our adjusted exponent?            */
                aLeftExp -= adjustDigits;         /* Yes, decrease adj exp             */
                                                  /* Right number is now shorter       */
                rightPtr = rightPtr-adjustDigits;
                rightExp += adjustDigits;         /* Right exponent adjusted           */
                rightLength -= adjustDigits;      /* update the length to reflect      */
                adjustDigits = 0;                 /* adjustment done.                  */
            }
            else
            {                             /* decrease by adjusted exp value    */
                                          /* Right number is now shorter       */
                rightPtr = rightPtr-aLeftExp;
                rightExp += aLeftExp;             /* Right exponent adjusted           */
                rightLength -= adjustDigits;      /* update the length to reflect      */
                adjustDigits -= aLeftExp;         /* adjustment partially done.        */
                aLeftExp = 0;                     /* the adjusted exponent is now zero.*/
            }
        }
        else if (aRightExp)
        {                /* Was right exp larger than left    */
            if ((wholenumber_t)adjustDigits < aRightExp)
            {    /* Do we adjust for more than        */
                /* our adjusted exponent?            */
                aRightExp -= adjustDigits;        /* Yes, decrease adj exp             */
                                                  /* Left  number is now shorter       */
                leftPtr = leftPtr-adjustDigits;
                leftExp += adjustDigits;          /* Left  exponent adjusted           */
                leftLength -= adjustDigits;       /* update the length to reflect      */
                adjustDigits = 0;                 /* adjustment done.                  */
            }
            else
            {                             /* decrease by adjusted exp value    */
                                          /* Right number is now shorter       */
                leftPtr = leftPtr-aRightExp;
                leftExp += aRightExp;             /* Right exponent adjusted           */
                leftLength -= adjustDigits;       /* update the length to reflect      */
                adjustDigits -= aRightExp;        /* adjustment partially done.        */
                aRightExp = 0;                    /* the adjusted exponent is now zero.*/
            }
        }

        if (adjustDigits)
        {                  /* Is there still adjusting needed   */
            leftExp += adjustDigits;            /* So adjust length and exp of each  */
                                                /* bumber by the remaining adjust    */
            leftPtr = leftPtr-adjustDigits;
            rightExp += adjustDigits;
            rightPtr = rightPtr - adjustDigits;
        }
    }                                     /* Done with adjusting the numbers,  */
    if (NumberDigits <= FASTDIGITS)       /* Get a buffer for the reuslt       */
    {
        resultBuffer = resultBufFast;
    }
    else
    {
        resultBuffer = buffer_alloc(NumberDigits*2 + 1);
    }

    /* Have Result Ptr point to end      */
    resultPtr = resultBuffer + NumberDigits*2;

    if (operation == OT_PLUS)
    {           /* Are we doing addition?            */
        carry = 0;                           /* no carry to start with.           */

                                             /* we need check and see if there    */
                                             /* are values in the adjusted exps   */
                                             /* that is do we pretend there       */
                                             /* are extra zeros at the end        */
                                             /* of the numbers?                   */
        result->sign = left->sign;           /* we are doing addition, both signs */
                                             /* are the same, assign use left for */
                                             /* result.                           */
        if (aLeftExp)
        {
            while (aLeftExp--)
            {               /* does left need zeros "added"?     */
                            /* yes, move digits of right         */
                            /* number into result                */
                            /*  xxxxxxx000   <- aLeftExp = 3     */
                            /*   yyyyyyyyy   <- aRightExp = 0    */
                if (rightPtr >= right->number)    /* Is there still data to move?      */
                {
                    *resultPtr-- = *rightPtr--;      /* Yes, move in correct digit.       */
                }
                else                              /* nope,                             */
                {
                    *resultPtr-- = '\0';             /* nope, move in a zero. we remove   */
                }
                                                     /*  leading zeros from orig number   */
                result->length++;                 /* Length is one greater.            */
            }
            result->exp = rightExp;            /* Right is smaller, assign to resul */
        }
        else if (aRightExp)
        {
            while (aRightExp--)
            {              /* does right need zeros "added"?    */
                           /* yes, move digits of left          */
                           /* number into result                */
                           /*  xxxxxxxxx    <- aLeftExp = 0     */
                           /*   yyyyyy00    <- aRightExp = 2    */

                if (leftPtr >= left->number)      /* Is there still data to move?      */
                {
                    *resultPtr-- = *leftPtr--;       /* Yes, move in correct digit.       */
                }
                else                              /* nope,                             */
                {
                    *resultPtr-- = '\0';             /* nope, move in a zero.             */
                }
                result->length++;                 /* Result length is now one more.    */
            }
            result->exp = leftExp;             /* left is smaller, assign to result */
        }
        else
        {
            result->exp = leftExp;             /* otherwise same exp. take left     */
        }

        while ( (leftPtr >= left->number) &&/* While we still have data to add.  */
                (rightPtr >= right->number) )
        {
            /* add the two current digits with a */
            /* possibly carry bit, and update the*/
            /* left and right ptr to point to    */
            /* next (actually previous) digit.   */
            addDigit = carry + (*leftPtr--) + *(rightPtr--);
            if (addDigit >= 10)
            {             /* result greater than 10? we have a */
                          /* carry for our next addition.      */
                carry = 1;                       /* indicate carry.                   */
                addDigit -= 10;                  /* subtract 10 from our result.      */
            }
            else
            {                            /* not bigger than 10.               */
                carry = 0;                       /* cancel any former carry           */
            }
            *resultPtr-- = (char)addDigit;    /* move result into the result       */
            result->length++;                 /* length of result is one more.     */
        }                                   /* go back and do next set of digits */

        /* One of the numbers is finished,   */
        /* check the other.                  */

        while (leftPtr >= left->number)
        {   /* While still digits in the         */
            /* left number.                      */
            addDigit = carry + (*leftPtr--);   /* add in any carry, and move to     */
                                               /* next digit to add.                */
            if (addDigit >= 10)
            {              /* do we still have a carry          */
                carry = 1;                        /* yes, indicate carry.              */
                addDigit -= 10;                   /* subtract 10 from our result.      */
            }
            else
            {                             /* not bigger than 10.               */
                carry = 0;                        /* make sure we cancel former carry  */
            }
            *resultPtr-- = (char)addDigit;     /* move result into the result       */
            result->length++;                  /* length of result is one more.     */
        }                                   /* all done with left number.        */

        while (rightPtr >= right->number)
        { /* While there is still digits in    */
          /* right number.                     */
            addDigit = carry + (*rightPtr--);  /* add in any carry, and move to the */
                                               /* next digit to add.                */
            if (addDigit >= 10)
            {              /* do we still have a carry          */
                carry = 1;                        /* indicate carry.                   */
                addDigit -= 10;                   /* subtract 10 from our result.      */
            }
            else
            {                             /* not bigger than 10.               */
                carry = 0;                        /* make sure we cancel former carry  */
            }
            *resultPtr-- = (char)addDigit;     /* move result into the result       */
            result->length++;                  /* length of result is one more.     */
        }                                   /* all done with left number.        */

        if (carry)
        {                        /* Do we still have a carry over?    */
            result->length++;                  /*  yes, number is now one more      */
            *resultPtr-- = 1;                  /*  set the high digit to 1.         */
        }
    }                                     /* end of addition.                  */

    else
    {                                /* we are doing subtraction.         */

                                     /* is left number bigger than right  */
        if ((aLeftExp + leftLength)>(aRightExp + rightLength))
        {
            /* yes, subtract right from left     */
            subtractNumbers(left, leftPtr, aLeftExp, right, rightPtr, aRightExp, result, &resultPtr);
            /* result exp is the adjusted exp of */
            /* smaller number.                   */
            if (aLeftExp)                     /* if adjusted left exp has a value  */
            {
                result->exp = rightExp;          /* the the right exp was smaller     */
            }
            else
            {
                result->exp = leftExp;           /* otherwise left was smaller/equal  */
            }

            result->sign = left->sign;         /* result sign is that of left.      */
        }
        /* is right number bigger than left  */
        else if ((aLeftExp + leftLength)<(aRightExp + rightLength))
        {
            /* yes, subtract left from right     */
            subtractNumbers(right, rightPtr, aRightExp, left, leftPtr, aLeftExp, result, &resultPtr);
            /* result exp is adjusted exp of the */
            /* smaller number.                   */
            if (aLeftExp)                     /* if adjusted left exp has a value  */
            {
                result->exp = rightExp;          /* the the right exp was smaller     */
            }
            else
            {
                result->exp = leftExp;           /* otherwise left was smaller/equal  */
            }

            result->sign = right_sign;         /* result sign is that of right.     */
        }

        /* here both numbers have same       */
        /* adjusted lexponent + length       */
        /* so see which number is longer and */
        /* each number for length of smaller.*/
        /* if they compare equal the result  */
        /* is zero                           */
        else if (rightLength < leftLength)
        {/* Does right number have a smaller  */
         /* length.                           */

         /* Yes, compare the digits....       */
         /* and see if right is larger.       */
            if ((rc = memcmp(right->number, left->number, rightLength)) > 0)
            {
                /* yes, subtract left from right     */
                subtractNumbers(right, rightPtr, aRightExp, left, leftPtr, aLeftExp, result, &resultPtr);
                result->sign = right_sign;        /* result sign is that of right.     */
            }
            else
            {
                /* no,  subtract right from left     */
                subtractNumbers(left, leftPtr, aLeftExp, right, rightPtr, aRightExp, result, &resultPtr);
                result->sign = left->sign;        /* result sign is that of left.      */
            }
            result->exp = leftExp;             /* result exp is that of the longer  */
                                               /* number (smaller Exp.) which is the*/
                                               /* left number in this case.         */
        }
        else
        {                              /* left length is smaller or equal to*/
                                       /* the length of right.              */
                                       /* see if left number is larger      */
            if ((rc = memcmp(left->number, right->number, leftLength)) > 0)
            {
                /* yes, subtract right from left     */
                subtractNumbers(left, leftPtr, aLeftExp, right, rightPtr, aRightExp, result, &resultPtr);
                result->exp = rightExp;          /* result exp is that of the longer  */
                                                 /* number (smaller Exp.) which is    */
                                                 /* right number in this case.        */

                result->sign = left->sign;        /* result sign is that of left.      */
            }
            else if (rc)
            {                     /* is the right number larger?       */
                                  /* no, subtract left from right      */
                subtractNumbers(right, rightPtr, aRightExp, left, leftPtr, aLeftExp, result, &resultPtr);
                result->exp = rightExp;          /* result exp is that of the longer  */
                                                 /* number (smaller Exp.) which is the*/
                                                 /* right number in this case.        */
                result->sign = right_sign;        /* result sign is that of right.     */
            }
            else
            {                              /* numbers compared are equal.       */
                if (leftLength < rightLength)
                {   /* does left have fewer digits?      */
                    /* Yes, then left is actuall smaller */
                    /*  so subtract left from right      */
                    subtractNumbers(right, rightPtr, aRightExp, left, leftPtr, aLeftExp, result, &resultPtr);
                    result->exp = rightExp;          /* result exp is that of the longer  */
                                                     /* number (smaller Exp.) which is    */
                                                     /* left number in this case.         */
                    result->sign = right_sign;       /* result sign is that of right.     */
                }
                else
                {                            /* Lengths are also eqaul, so numbers*/
                                             /* are identical.                    */
                    result->sign=0;                  /* number equal, return 0            */
                    result->exp = 0;
                    result->length = 1;
                    result->number[0] = '0';
                }
            }
        }
    }                                     /* End of subtraction.               */

    /* pointer resultPtr always points   */
    /* to next position to add a digit   */
    /* thereofre we will bump the pointer*/
    /* 1st before doing any cleanup      */
    /* we end up pointing to the most sig*/
    /* digit on exit of loop instead of  */
    /* the 1st digit.                    */

    /*make sure result end up with       */
    /* correct precision.                */
    result->adjustPrecision(++resultPtr, NumberDigits);
    return result;                        /* all done, return the result       */

}

void RexxNumberString::subtractNumbers(
    RexxNumberString *larger,          /* larger numberstring object        */
    const char       *largerPtr,       /* pointer to last digit in larger   */
    wholenumber_t     aLargerExp,      /* adjusted exponent of larger       */
    RexxNumberString *smaller,         /* smaller numberstring object       */
    const char       *smallerPtr,      /* pointer to last digit in smaller  */
    wholenumber_t     aSmallerExp,     /* adjusted exponent of smaller      */
    RexxNumberString *result,          /* result numberstring object        */
    char            **presultPtr)      /* last number in result             */
/*********************************************************************/
/* Function:  Subtract two numbers.  The second number is subtracted */
/*            from the first.  The absolute value of the 1st number  */
/*            must be larger than the absolute value of the second.  */
/*********************************************************************/
{
    int borrow, subDigit;
    char *resultPtr;

    resultPtr = *presultPtr;              /* since we need to update value of  */
                                          /* resultPtr and return value to     */
                                          /* caller, we are passed a pointer   */
                                          /* pointer,  we'll just use Ptr to   */
                                          /* data. (avoid reference problems)  */
                                          /* we will update this pointer value */
                                          /* before exiting, so the Ptr        */
                                          /* is correct in caller.             */
    borrow = 0;                          /* no carry to start with.           */

                                         /* 1st we need to make both adjusted */
                                         /* exponents zero.  values are either*/
                                         /* positive or zero on entry.        */
                                         /* one of the adjusted exponents will*/
                                         /* always be zero on entry.          */
    while (aLargerExp--)
    {               /* if larger number had a value for  */
                    /* for adjusted exponent that means  */
                    /* that smaller number had a smaller */
                    /* exponent (less significant digit  */
                    /* so we pretend to add extra zeros  */
                    /* to the larger number and do the   */
                    /* subtraction for required number   */
                    /* of digits, (aLargerExp).          */
                    /*   xxxxx00                         */
                    /*    yyyyyy                         */

        if (smallerPtr >= smaller->number) /* Is there still data to subtract   */
        {
            subDigit = *smallerPtr--;        /* Yes, set value for subtraction    */
        }
        else                               /* nope,                             */
        {
            subDigit = '\0';                 /* nope, use a Zero for subtraction. */
        }

                                             /* subtract 10 from smaller number,  */
                                             /* any borrow.                       */
                                             /* and point to next digit.          */
        subDigit = borrow + 10 - subDigit;
        if (subDigit == 10)
        {              /* was result 10, this can only occur*/
                       /* the bottom digit was a zero and we*/
                       /* didn't borrow anyting.            */
            subDigit -= 10;                   /* Yes, result will be zero and      */
            borrow = 0;                       /*   no borrow.                      */
        }
        else
        {
            borrow = -1;                      /* nope, result digit ok, and we     */
        }
                                              /* need to borrow.                   */
        *resultPtr-- = (char)subDigit;     /* place this digit in result number */
        result->length++;                  /* length of result is now one more. */
    }

    while (aSmallerExp--)
    {              /* if smaller number had a value for */
                   /* for adjusted exponent that means  */
                   /* that larger  number had a smaller */
                   /* exponent (more significant digit  */
                   /* so we pretend to add extra zeros  */
                   /* to smaller number and do the      */
                   /* subtraction for required number   */
                   /* of digits, (aSmallerExp).         */
                   /*   xxxxxxx                         */
                   /*    yyyy00                         */

                   /* here we just move the the digits  */
                   /* from larger into result.          */
                   /* and point to next digit.          */

        if (largerPtr >= larger->number)   /* Is there still data to move?      */
        {
            *resultPtr-- = *largerPtr--;     /* Yes, move in correct digit.       */
        }
        else                               /* nope,                             */
        {
            *resultPtr-- = '\0';             /* nope, move in a zero. we remove   */
        }
                                             /*  leading zeros from orig number   */
        result->length++;                  /* length of result is now one more. */
    }

    /* Now we are ready to do subtract   */
    /* of our digits.                    */
    /* While still have data to subtract */
    while (smallerPtr >= smaller->number)
    {
        /* Sub two current digits with a     */
        /* possibly borrow bit, and update   */
        /* larger and smaller pointer to     */
        /* to next (actually previous) digit */
        subDigit = borrow + (*largerPtr--) - *(smallerPtr--);
        if (subDigit < 0  )
        {              /* result less than 0, Do we need to */
                       /* borrow for next subtraction       */
            borrow = -1;                      /* yes, indicate borrow.             */
            subDigit += 10;                   /* add 10 to result from borrow.     */
        }
        else
        {                             /* not less than 0                   */
            borrow = 0;                       /* make sure we cancel former borrow */
        }
        *resultPtr-- = (char)subDigit;     /* move result into result object.   */
        result->length++;                  /* length of result is one more.     */
    }                                    /* go back and do next set of digits */

    /* One number is finished, need      */
    /* to check the other.               */

    while (largerPtr >= larger->number)
    {/* While still digits in the larger  */
        subDigit = borrow + (*largerPtr--);/* sub any borrow,   and move to the */
                                           /* next digit to subtract.           */
        if (subDigit < 0  )
        {              /* do we still need to borrow?       */
            borrow = -1;                      /* yes, indicate borrow              */
            subDigit += 10;                   /* add 10 to result for borrow.      */
        }
        else
        {                             /* not bigger than 10.               */
            borrow = 0;                       /* make sure we cancel former carry  */
        }
        *resultPtr-- = (char)subDigit;     /* move result into result object.   */
        result->length++;                  /* length of result is one more.     */
    }                                    /* all done with larger number.      */

    *presultPtr = resultPtr;              /* update result PTR for our return  */
    return;
}

char *RexxNumberString::addToBaseSixteen(
  int      Digit,                      /* digit to add                      */
  char    *Value,                      /* number to add                     */
  char    *HighDigit )                 /* highest digit location            */
/*********************************************************************/
/*   Function:          Adds a base ten digit to string number in    */
/*                      base 16 (used by the d2x and d2c functions)  */
/*********************************************************************/
{
    while (Digit)
    {                      /* while something to add            */
        Digit += *Value;                   /* add in current number             */
        if (Digit > 15)
        {                  /* carry?                            */
            Digit -= 16;                     /* reduce digit                      */
            *Value-- = (char)Digit;          /* set digit and step pointer        */
            Digit = 1;                       /* just a carry digit now            */
        }
        else
        {
            *Value-- = (char)Digit;          /* set the digit                     */
            Digit = 0;                       /* no more carry                     */
        }
    }
    if (Value < HighDigit)               /* new high water mark?              */
    {
        return Value;                      /* return high location              */
    }
    else
    {
        return HighDigit;                  /* return the old one                */
    }
}

char *RexxNumberString::multiplyBaseSixteen(
  char *     Accum,                    /* number to multiply                */
  char *     HighDigit )               /* current high water mark           */
/*********************************************************************/
/*   Function:          multiplies a base 16 number by 10, placing   */
/*                      the result in the same buffer.               */
/*********************************************************************/
{
    int          Carry;                  /* multiplication carry              */
    unsigned int Digit;                  /* current digit                     */
    char *       OutPtr;                 /* output pointer                    */

    OutPtr = Accum;                      /* point to first digit              */
    Carry = 0;                           /* no carry yet                      */
    while (OutPtr > HighDigit)
    {         /* while more digits                 */
              /* multiply digit by 10              */
        Digit = (unsigned int )((*OutPtr * 10) + Carry);
        if (Digit > 15)
        {                  /* carry?                            */
            Carry = (int)(Digit / 16);       /* get carry value                   */
            Digit &= (unsigned int )0x0000000f; /* keep just lower nibble         */
        }
        else                               /* no carry here                     */
        {
            Carry = 0;                       /* no carry                          */
        }
        *OutPtr-- = (char)Digit;           /* set the digit                     */
    }
    if (Carry)                           /* carried out?                      */
    {
        *OutPtr-- = (char)Carry;           /* set the carry pointer             */
    }
    return OutPtr;                       /* return new high water mark        */
}

char *RexxNumberString::addToBaseTen(
  int      Digit,                      /* digit to add                      */
  char    *Accum,                      /* number to add                     */
  char    *HighDigit )                 /* highest digit location            */
/*********************************************************************/
/*   Function:          Adds a base sixteen digit to a string number */
/*                      base 10 (used by the d2x and d2c functions)  */
/*********************************************************************/
{
    int      Carry;                      /* carry number                      */

    Carry = 0;                           /* no carry yet                      */
    while (Digit || Carry)
    {             /* while something to add            */
        Digit += *Accum + Carry;           /* add in current number             */
        if (Digit > 9)
        {                   /* carry?                            */
            Carry = Digit / 10;              /* get the carry out                 */
            Digit %= 10;                     /* reduce digit                      */
            *Accum-- = (char)Digit;          /* set digit and step pointer        */
        }
        else
        {
            *Accum-- = (char)Digit;          /* set the digit                     */
            Carry = 0;                       /* no more carry                     */
        }
        Digit = 0;                         /* no addition after first           */
    }
    if (Accum < HighDigit)               /* new high water mark?              */
    {
        return Accum;                      /* return high location              */
    }
    else
    {
        return HighDigit;                  /* return the old one                */
    }
}

char *RexxNumberString::multiplyBaseTen(
  char *      Accum,                    /* number to multiply                */
  char *      HighDigit )               /* current high water mark           */

/*********************************************************************/
/*   Function:          multiplies a base 16 number by 10, placing   */
/*                      the number in a different buffer.            */
/*********************************************************************/
{
    char *       OutPtr;                 /* addition pointer                  */
    int          Carry;                  /* multiplication carry              */
    unsigned int Digit;                  /* current digit                     */

    OutPtr = Accum;                      /* point to output buffer            */
    Carry = 0;                           /* no carry yet                      */

    while (OutPtr > HighDigit)
    {         /* while more digits                 */
              /* multiply digit by 16              */
        Digit = (unsigned int )((*OutPtr * 16) + Carry);
        if (Digit > 9)
        {                   /* carry?                            */
            Carry = (int)(Digit / 10);       /* get carry value                   */
            Digit %= 10;                     /* get the digit value               */
        }
        else                               /* no carry here                     */
        {
            Carry = 0;                       /* no carry                          */
        }
        *OutPtr-- = (char)Digit;           /* set the digit                     */
    }
    while (Carry)
    {                      /* carried out?                      */
        Digit = Carry % 10;                /* get first carry digit             */
        Carry = Carry / 10;                /* get the next digit                */
        *OutPtr-- = (char)Digit;           /* set the digit                     */
    }
    return OutPtr;                       /* return new high water mark        */
}
