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
/* REXX Kernel                                                    okmath2.c   */
/*                                                                            */
/* Arithmetic function for the NumberString Class                             */
/*  Multiply/Divide/Power                                                     */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "RexxCore.h"
#include "NumberStringClass.hpp"
#include "ArrayClass.hpp"
#include "BufferClass.hpp"
#include "RexxActivity.hpp"
#include "NumberStringMath.hpp"
#include "ActivityManager.hpp"

char *RexxNumberString::addMultiplier(
    char *top,                        /* data pointer of "top" number      */
    size_t topLen,                     /* length of the top number          */
    char *AccumPtr,                   /* output accumulator location       */
    int MultChar)                      /* multiplier value                  */
/*********************************************************************/
/* Function:  Multiply current digit through "top" number and add    */
/*            result to the accumulator.                             */
/*********************************************************************/
{
    int carry, ResultChar;

    carry = 0;                           /* no carry at this point.           */
    top += (topLen - 1);                 /* move data point to end of data.   */

                                         /* while there are digits left to    */
                                         /* multiply and there is room to put */
                                         /* more digits.                      */
    while (topLen-- )
    {
        /* Multiply char by top digit and add*/
        /* the accumvalue for position and   */
        /* and carry. and adjust pointer to  */
        /* the next digit positions.         */
        ResultChar = carry + *AccumPtr + (MultChar * *top--);
        if (ResultChar >= 10)
        {            /* Do we have carry to worry about?  */
            carry = ResultChar / 10;         /* Yes, calculate the carry over.    */
            ResultChar -= carry * 10;        /* adjust number down.               */
        }
        else
        {
            carry = 0;                       /* no carry,                         */
        }
        *AccumPtr-- =  ResultChar;         /* Set result char to the accum pos  */
                                           /* and point accum to next position. */
    }
    if (carry)
    {                         /* still room to put a digit         */
        *AccumPtr-- = (char)carry;        /* yes, put carry into next pos.     */
    }
    return ++AccumPtr;                    /* return pointer to start of Accum. */
}

RexxNumberString *RexxNumberString::Multiply(RexxNumberString *other)
/*********************************************************************/
/* Function:  Multiply two NumberString objects                      */
/*********************************************************************/
{
    RexxNumberString *left, *right, *result, *LargeNum, *SmallNum;
    char *ResultPtr, *AccumPtr, *Current, *OutPtr;
    char MultChar;
    size_t AccumLen;
    size_t i;
    size_t NumberDigits, TotalDigits, ExtraDigit;
    char resultBufFast[FASTDIGITS];      /* fast allocation if default digits */

    NumberDigits = number_digits();       /* Get the current Numeric Digits    */

                                          /* prepare both numbers              */
    left = this->checkNumber(NumberDigits + 1, NOROUND);
    right = other->checkNumber(NumberDigits + 1, NOROUND);
    /* either number 0 to begin with?    */
    if (left->sign == 0 || right->sign == 0)
    {
        return new_numberstring("0", 1);   /* Yes, then result is Zero.         */
    }

    if (left->length > right->length)
    {   /* Determine the large number        */
        LargeNum = left;                     /* left is larger, set up large and  */
        SmallNum = right;                    /*  small for right                  */
    }
    else
    {
        LargeNum = right;                    /* right is larger, set up large and */
        SmallNum = left;                     /*  small for left.                  */
    }

    TotalDigits = ((NumberDigits+1) * 2) + 1;
    /* working with large numbers?       */
    if (TotalDigits > FASTDIGITS)
    {
        /* get a work are for result digits. */
        OutPtr = buffer_alloc(TotalDigits);
    }
    else
    {
        OutPtr = resultBufFast;             /* use the local version             */
    }
    memset(OutPtr,'\0',TotalDigits);      /* Make sure work area is zero       */

    AccumPtr = OutPtr;                    /* Set up our acummulator            */
    AccumLen = 0;                         /* no data at this time.             */
                                          /* Set up result Pointer. Point to   */
                                          /* the end of the data.              */
    ResultPtr = AccumPtr + TotalDigits - 1;
    /* Set up current digit ptr.         */
    Current = SmallNum->number + SmallNum->length;

    /* do for all multiplier digits      */
    for (i = SmallNum->length ; i ; i-- )
    {
        Current--;                          /* shift add location by 1.          */
        MultChar = *Current;                /* get new multiplier digit.         */
        if (MultChar)
        {                     /* is this new digit a Zero?         */
                              /* nope, go do multiplication of     */
                              /* digit                             */
            AccumPtr = addMultiplier(LargeNum->number, LargeNum->length, ResultPtr,  MultChar);
        }
        /* If number is zero we don't need   */
        /* to do anything.                   */
        ResultPtr--;                        /* Backup Result Ptr, for next digit */
    }                                     /* go do next digit.                 */
    /* Get length of computed number.    */
    AccumLen = (++ResultPtr - AccumPtr) + SmallNum->length;

    /* AccumPtr now points to result,    */
    /*  the len of result is in AccumLen */

    /* now get a real Object for dat     */
    if (AccumLen > NumberDigits)
    {        /* Is result len greater then Digits */
             /* save amount over digits for exp   */
        ExtraDigit = AccumLen -(NumberDigits + 1);
        AccumLen = NumberDigits + 1;         /* we will only use Digits + 1       */
    }
    else
    {
        ExtraDigit = 0;                      /* Length OK, no adjusting Exp.      */
    }

                                             /* go get the new object.            */
    result = (RexxNumberString *)new_numberstring(NULL, AccumLen);
    /* get the result exponent           */
    result->exp = LargeNum->exp + SmallNum->exp + ExtraDigit;
    /* Compute the Sign                  */
    result->sign = LargeNum->sign * SmallNum->sign;
    result->length = AccumLen;            /* Set length of result.             */
                                          /* Make sure result is in correct    */
                                          /* precision                         */
    result->adjustPrecision(AccumPtr, NumberDigits);
    return result;                        /* return computed value.            */
}                                      /* All done,                         */

char *RexxNumberString::subtractDivisor(char *data1, size_t length1,
                       char *data2, size_t length2,
                       char *result, int Mult)
/*********************************************************************/
/* Function:  Subtraction routine for division                       */
/*********************************************************************/
{
    char *OutPtr;
    int   carry, DivChar;
    size_t extra;
    /* This rountine actually does the divide of the Best Guess */
    /*  Mult.  This Best guess is a guess at how many times the */
    /*  dividend will go into the divisor, since it is only a   */
    /*  guess we make sure we guess to the low side, if low we  */
    /*  always adjust our guess and recompute.                  */
    /* We multiply the Dividend(Data2) by mult and then subtract*/
    /*  the result from the Divisor (Data1) and this result is  */
    /*  put into RESULT.  Since Mult is guaranteed to be correct*/
    /*  or low, the result is guaranteed to never be negative.  */

    /*            xxMxxx           <- M signifies this Mult     */
    /*          ______________                                  */
    /* dividend ) divisor                                       */
    /*           -iiiiiiiii        <- result of M x Divisor     */
    /*           ===========                                    */
    /*            rrrrrrrrr        <- result returned           */


    data1 += (length1 -1);               /* point to end of data1             */
    data2 += (length2 -1);               /* point to end of data2             */

    OutPtr = result + 1;                 /* setup output pointer              */
    carry = 0;                           /* no carry at this point.           */
    extra = length1 - length2;           /* get extra byte count.             */

    while (length2--)
    {                  /* do all digits in second number    */
                       /* compute this div value, and bump  */
                       /* data pointer to the next digit.   */
        DivChar = carry + *data1-- - (*data2-- * Mult);
        if (DivChar < 0)
        {                  /* is this div value negative?       */
            DivChar += 100;                    /* make it positive, by adding 100   */
            carry = (DivChar/10) -10;          /* calculate borrow out.             */
            DivChar %= 10;                     /* compute real result remainder     */
        }
        else                                /* div value is not negative.        */
        {
            carry = 0;                         /* clear out carry value.            */
        }
        *--OutPtr = (char)DivChar;         /* set this digit in output          */
    }                                    /* go back and do next divide.       */

    if (extra)
    {                         /* is ther more to process?          */
        if (!carry)
        {                       /* is there a carry left over?       */
            while (extra--)                    /*  no, just copy each remaining     */
            {
                *--OutPtr = (char)*data1--;      /*   digit from data1.               */
            }
        }
        else
        {
            while (extra--)
            {                  /* carry left over, do for all extra */
                DivChar = carry + *data1--;       /* add carry to digit.               */
                if (DivChar < 0)
                {                /* is result negative?               */
                    DivChar += 10;                   /* add 10(borrow) to digit value     */
                    carry = -1;                      /* have another carry.               */
                    *--OutPtr = (char)DivChar;      /* put digit into result             */
                }
                else
                {
                    *--OutPtr = (char)DivChar;      /* finished w/ carry place in result */
                    while (extra--)                  /* now just copy rest of digits      */
                    {
                        *--OutPtr = *data1--;           /*  and adjust for next digit.       */
                    }
                    break;                           /* all done, break out of loop       */
                }
            }
        }
    }
    return OutPtr;                       /* return pointer to start of result */
}


RexxNumberString *RexxNumberString::Division(RexxNumberString *other, unsigned int DivOP)
/*********************************************************************/
/* Function:  Divide two numbers                                     */
/*********************************************************************/
{
    RexxNumberString *left, *right;
    RexxNumberStringBase *Accum;          /* dummy accumulator object          */
    RexxNumberStringBase *SaveLeft;       /* dummy operator object             */
    RexxNumberStringBase *SaveRight;      /* dummy operator object             */
                                          /* buffers for dummy arguments       */
    char AccumBuffer[sizeof(RexxNumberStringBase)];
    char SaveLeftBuffer[sizeof(RexxNumberStringBase)];
    char SaveRightBuffer[sizeof(RexxNumberStringBase)];
    RexxNumberString   *result;
    char *Num1, *Num2;
    char *resultPtr, *Output, *rightPtr, *leftPtr, *SaveLeftPtr, *SaveRightPtr;
    wholenumber_t   multiplier, rc;
    wholenumber_t   DivChar, thisDigit;
    wholenumber_t  CalcExp;

    size_t  NumberDigits, totalDigits, resultDigits;
    size_t adjustNum1;
    char leftBufFast[FASTDIGITS];        /* fast allocation if default        */
    char rightBufFast[FASTDIGITS];       /* fast allocation if default        */
    char outBufFast[FASTDIGITS];         /* fast allocation if default        */
    size_t  rightPadding;                 /* amount right side is padded by    */

    SaveLeftPtr = NULL;

    /* NOTE: this routine if very similiar to the PowerDivide   */
    /*   routine, these we kept as seperate routines since there*/
    /*   are enough subtile differences between the objectPointererations  */
    /*   that combining them would make an already complex      */
    /*   routine even more so.  When fixing/updating/adding to  */
    /*   this routin also check PowerDivide for similiar updates*/

    if (!other->sign)
    {               /* is the right number zero?         */
                    /* yes, divide by Zero.              */
        reportException(Error_Overflow_zero);
    }
    else if (!this->sign)
    {               /* is left number Zero?              */
                    /*  yes, just return a zero.         */
        return(RexxNumberString *)IntegerZero;
    }
    /* set up address of temporaries     */
    Accum = (RexxNumberStringBase *)AccumBuffer;
    SaveLeft = (RexxNumberStringBase *)SaveLeftBuffer;
    SaveRight = (RexxNumberStringBase *)SaveRightBuffer;
    NumberDigits = number_digits();       /* get current digits setting.       */
                                          /* make sure we've got good copy of  */
                                          /*  our working numbers              */
    left = this->checkNumber(NumberDigits + 1, NOROUND);
    right = other->checkNumber(NumberDigits + 1, NOROUND);
    CalcExp = left->exp - right->exp;     /* compute the new exponents         */
                                          /* calculate expected resultant exp  */
    CalcExp += (wholenumber_t)left->length - (wholenumber_t)right->length;
    /* is exp < 0 and doing // or %      */
    if (CalcExp < 0 && DivOP != OT_DIVIDE)
    {
        if (DivOP == OT_INT_DIVIDE)
        {       /* Are we doing % (integer Divide)?  */
                /* yes, result is zero.              */
            return(RexxNumberString *)IntegerZero;
        }
        else
        {
            /* We are doing //, return left(this)*/
            result = left->prepareNumber(NumberDigits + 1, NOROUND);
            result->setupNumber();
            return result;
        }
    }

    totalDigits = ((NumberDigits + 1) * 2) + 1;
    if (totalDigits > FASTDIGITS)
    {       /* working with large numbers?       */
            /* No fast path here, do Division    */
            /* get buffer for left digit data.   */
        leftPtr  = buffer_alloc(totalDigits);
        /* and right digit data              */
        rightPtr  = buffer_alloc(totalDigits);
        /* get buffer for result digit data. */
        Output   = buffer_alloc(totalDigits);
    }
    else
    {
        leftPtr  = leftBufFast;             /* use non-allocated version for the */
        rightPtr = rightBufFast;
        Output   = outBufFast;
    }
    /* now copy the data itself into     */
    /*  the temp data buffers.           */
    memcpy(leftPtr,left->number, left->length);
    /* pad the output area with zeros.   */
    memset(leftPtr + left->length, '\0', totalDigits - left->length);
    memcpy(rightPtr, right->number, right->length);
    /* pad the output area with zeros.   */
    memset(rightPtr + right->length, '\0', totalDigits - right->length);
    resultPtr = Output;                   /* Set up result, point to end of    */
                                          /* make copies of right number info  */
    memcpy(SaveRight, right, sizeof(RexxNumberStringBase));
    /* make copies of left number info   */
    memcpy(SaveLeft, left, sizeof(RexxNumberStringBase));
    if (DivOP == OT_REMAINDER)
    {          /* Are we doing remainder divide?    */
        SaveLeftPtr  = leftPtr;              /* Save initial pointers to left     */
        SaveRightPtr = rightPtr;             /*  and right numbers.               */
        SaveRight->sign = 1;                 /* force dividend sign positive.     */
    }
    /* compute sign of result.           */
    Accum->sign = left->sign * SaveRight->sign;

    /* is right numebr longer than left? */
    if (SaveRight->length > SaveLeft->length)
    {
        SaveLeft->length = SaveRight->length;/* set both numbers to same length   */
        rightPadding = 0;                    /* no padding needed for right number*/
    }
    else
    {                                /* Left number is longer.            */
                                     /* no padding needed for right number*/
        rightPadding = SaveLeft->length - SaveRight->length;
        SaveRight->length = SaveLeft->length;/* set both numbers to same length   */
    }

    /* Set the new exponents of two      */
    SaveLeft->exp = NumberDigits * 2 - SaveLeft->length + 1;
    SaveRight->exp = rightPadding;        /* work numbers.                     */
    adjustNum1 = 0;                       /* Set to 0 to start with.           */
    Num1 = leftPtr;                       /* Num1 is ldivisor digit pointer    */
    Num2 = rightPtr;                      /* Num2 is dividend digit pointer    */

    /* When generate a best guess digits for result we will look*/
    /* use the 1st 2 digits of the dividend (if there are 2)    */
    /* we then add 1 to this DivChar to ensure than when we gues*/
    /* we either guess correctly of under guess.                */
    /*           _______________                                */
    /*  aabbbbbb ) xxyyyyyyyy                                   */
    /*                                                          */
    /*     DivChar = aa + 1                                     */

    DivChar = *Num2 * 10;                 /* Divide char is the 1st 2 digits + 1  */
    if (SaveRight->length > 1)            /* more than 1 digit in Accum?       */
    {
        DivChar += *(Num2 + 1);              /*  yes, get second digit for Div    */
    }
    DivChar++;                            /* add 1 to Div number               */
    resultDigits = 0;         /* initializes digit values to zero. */
    thisDigit = 0;

    /* We are now to enter 2 do forever loops, inside the loops */
    /*  we test for ending conditions. and will exit the loops  */
    /*  when needed. This inner loop may need to break out of   */
    /*  both loops, if our divisor is reduced to zero(all finish*/
    /*  if this happens to do the no-no nad use a GOTO.         */
    /* The outer loop is used to obtain all digits for the resul*/
    /*  We continue in this loop while the divisor has NOT been */
    /*  reduced to zero and we have not reach the maximum number*/
    /*  of digits to be in the result (NumDigits + 1), we add   */
    /*  one to NumDigits so we can round if necessary.          */
    /* The inner loop conputs each digits of the result and     */
    /*  breaks to the outer loop when the next digit of result  */
    /*  is found.                                               */
    /* We compute a digit of result by continually taking best  */
    /*  guesses at how many times the dividend can go into the  */
    /*  divisor. Once The divisor becomes less than the dividend*/
    /*  we found this digit and we exit the inner loop. If the  */
    /*  divisor = dividend then we know dividend will go into   */
    /*  1 more than last guess, so bump up the last guess and   */
    /*  exit both loops (ALL DONE !!), if neither of the above  */
    /*  conditions are met our last guess was low, compute a new*/
    /*  guess using result of last one, and go though inner loop*/
    /*  again.                                                  */
    for (; ; )
    {                          /* do forever (outer loop)           */
        for (; ; )
        {                        /* do forever (inner loop)           */
                                 /* are two numbers equal in length?  */
            if (SaveLeft->length == SaveRight->length)
            {
                /* yes, then compare two numbers     */
                rc = memcmp(Num1, Num2, SaveLeft->length);
                if (rc < 0)                       /* is Num1(left) smaller?            */
                {
                    break;                           /* yes, break out of inner loop.     */
                }

                                                     /* are the two numebrs equal and not */
                                                     /* doing //                          */
                else if (rc == 0 && DivOP != OT_REMAINDER)
                {
                    /* yes, done with Division, cleanup  */
                    *resultPtr++ = (char)(thisDigit + 1);
                    resultDigits++;                  /* one more digit in result          */
                    goto PowerDivideDone;            /* break out of both loops.          */
                }
                else                              /* Either rc >0 or doing //          */
                {
                    multiplier = *Num1;              /* Lengths of nums are equal we only */
                    /* need to use 1 digits from divisor */
                    /* to this next guess.               */
                }
            }
            /* is left longer than Accum?        */
            else if (SaveLeft->length > SaveRight->length)
            {
                /* calculate multiplier, next two    */
                /*digits                             */
                multiplier = *Num1 * 10 + *(Num1 + 1);
            }
            else
            {
                break;                           /* Divisor is smaller than dividend, */
            }
                                                 /* we found this digit of result, go */
                                                 /* to outer loop and finish up       */
                                                 /* processing for this digit.        */
                                                 /* compute Multiplier for actual     */
                                                 /*divide                             */
            multiplier = multiplier * 10 / DivChar;
            /* that is how many times will digit */
            /* of dividend go into divisor, using*/
            /* the 1st 2 digits of each number   */
            /* compute our Best Guess for this   */
            /* digit                             */
            if (multiplier == 0)               /* did it compute to 0?              */
            {
                multiplier = 1;                   /*  yes, can't be zero make it one.  */
            }
                                                  /* we know dividend goes into        */
                                                  /* divisor at least one more time.   */
            thisDigit += multiplier;           /* add multiplier to this digit.     */


                                               /* Go and actualy see if we guessed  */
                                               /* correctly, Divide digit through   */
            Num1 = subtractDivisor(Num1, SaveLeft->length, Num2, SaveRight->length, Num1 + SaveLeft->length - 1, (int)multiplier);
            /* while we have leading zeros       */
            while (*Num1 == 0 && SaveLeft->length > 1)
            {
                Num1++;                          /* step to the next digit            */
                SaveLeft->length--;              /* and reduce the length also        */
            }
            /* end of inner loop, go back and    */
            /* guess again !!                     */
        }
        if (resultDigits || thisDigit)
        {    /* Have a digit for result?          */
            *resultPtr++ = (char) thisDigit;  /* yes, place digit in result.       */
            thisDigit = 0;                     /* reset digit value to zero;        */
            resultDigits++;                    /* one more digit in result;         */

                                               /* has dividend reduced to zero,     */
                                               /*  run out of room for additional?  */
            if (*Num1 == '\0' || resultDigits > NumberDigits)
            {
                break;                            /* yes, were done, exit outer loop   */
            }

        }

        if (DivOP != OT_DIVIDE)
        {           /* Are we doing // or %              */
            if (CalcExp <= 0)                  /*  have we finished integer part?   */
            {
                break;                            /* yes, all done here, break out     */
            }
        }
        /* Was number reduced to zero?       */
        if (SaveLeft->length == 1 && *Num1 == '\0')
        {
            break;                             /* yes, all done exit outer loop     */
        }


                                               /* we're not done dividing yet, we   */
                                               /*  need to adjust expected exponent */
                                               /*  by one to the left               */
        CalcExp--;                          /* result exponent is one less.      */
        if (rightPadding > 0)
        {             /* are we still "padding" number for */
                      /*  right number?                    */
            SaveRight->length--;               /* yes, length of right is one less. */
            rightPadding--;                    /*  now padding one less digit.      */
        }
        else
        {
            SaveLeft->length++;                /* length of left is now one more.   */
        }
    }                                     /* end of outer loop                 */

    PowerDivideDone:                       /* done doing actual divide now do   */
    ;                       /*  the cleanup stuff.               */

    if ((DivOP != OT_DIVIDE) &&          /* Is this a // or % operation, and  */
        (( CalcExp >= 0 &&               /*   and is the result bad?          */
           ( resultDigits + CalcExp) > NumberDigits) ||
         (CalcExp < 0  && (size_t)Numerics::abs(CalcExp) > resultDigits)))
    {
        /* yes, report the error and get out.*/
        if (DivOP == OT_REMAINDER)         /* remainder operation?              */
        {
            reportException(Error_Invalid_whole_number_rem);
        }
        else
        {
            reportException(Error_Invalid_whole_number_intdiv);
        }
    }
    if (DivOP == OT_REMAINDER)
    {         /* Are we doing //                   */
        if (resultDigits)
        {                 /* any numbers in result?            */
            if (*Num1)
            {                       /* yes, but was it Zero?             */
                                    /* nope, we got a real remainder     */
                resultPtr = Num1;                 /* set result to point to remainder  */
                                                  /* we need to compute the exponent   */
                                                  /* of our result.                    */
                SaveLeftPtr += left->length;      /* point to end of input.            */
                                                  /* point to existing location.       */
                SaveRightPtr = resultPtr + SaveLeft->length + adjustNum1;
                /* Adjust for added Zeros.           */
                Accum->exp = left->exp - (SaveRightPtr - SaveLeftPtr);
                Accum->length = SaveLeft->length; /* length of result is that of the   */
                                                  /* remaining divisor digits.         */
            }
            else
            {
                /* result is 0, just return it.      */
                return(RexxNumberString *)IntegerZero;
            }
        }
        /* no digits in result, remainder is */
        /* the left number (this)            */
        else
        {
            /* return a copy of Div(left) number */
            result = this->clone();
            result->setupNumber();
            return result;
        }
    }
    else
    {                               /* real division... compute answer.  */
        if (resultDigits)
        {                 /* any number in result?             */
                          /* Set resultPtr to start of our     */
                          /* buffer                            */
            resultPtr = Output;
            Accum->length = resultDigits;      /* length is digits in result.       */
            Accum->exp = CalcExp;              /* set exp to that calculated above. */
            if (Accum->length > NumberDigits)
            {/* is result too big?                */
             /* Yes, we need to adjust result     */
             /* increase exponent by amount over. */
                Accum->exp += (Accum->length - NumberDigits);
                Accum->length = NumberDigits;     /* Length is same as Digits          */
                Accum->mathRound(resultPtr);      /* round result if necessary.        */
            }
            /* We now remove any trailing zeros  */
            /* point to last digit in result     */
            Num1 = resultPtr + Accum->length - 1;
            while (!*Num1 && Accum->length)
            {  /* While there are trailing zeros    */
                Num1--;                           /*  point to next character.         */
                Accum->length--;                  /*  Result is one digit less.        */
                Accum->exp++;                     /*  Adjust expont up one             */
            }
        }
        else
        {
            /* no digits in result answer is     */
            /* zero.                             */
            return(RexxNumberString *)IntegerZero;
        }
    }                                    /* End final processing              */
    result = new (Accum->length) RexxNumberString (Accum->length);

    result->length = Accum->length;      /* set length of result              */
    result->exp = Accum->exp;            /* set exponent of result.           */
    result->sign = Accum->sign;          /* set sign of result.               */
                                         /* move result data to result area   */
    result->adjustPrecision(resultPtr, NumberDigits);
    return result;                       /* all done, return to caller.       */
}

RexxNumberString *RexxNumberString::power(RexxObject *PowerObj)
/*********************************************************************/
/*   Function:          Perform the Arithmetic power operation       */
/*********************************************************************/
{
    wholenumber_t powerValue;
    wholenumber_t extra, OldNorm;
    size_t  NumberDigits;
    char   *Accum, *AccumPtr, *OutPtr, *TempPtr;
    bool    NegativePower;
    RexxNumberStringBase *AccumObj;
    RexxNumberString     *left;
    RexxNumberString     *result;
    size_t NumBits;
    size_t    AccumLen;

    NegativePower = false;               /* Initialize the flags.             */
    requiredArgument(PowerObj, ARG_ONE);         /* must have one argument            */
                                         /* get the whole number value        */
    if (!PowerObj->numberValue(powerValue, number_digits()))
    {
        reportException(Error_Invalid_whole_number_power, PowerObj);
    }

    if (powerValue < 0)
    {                /* is the power negative?            */
        NegativePower = true;               /*  yes, mark for later.             */
        powerValue = -powerValue;           /*  make power positive, we first do */
                                            /*    power as if positive then      */
                                            /*    invert value (1/x)             */
    }
    NumberDigits = number_digits();      /* get the current Digits Setting.   */
                                         /* make a copy of self, since we may */
                                         /* need to adjust some of its data.  */
    left = this->prepareNumber(NumberDigits+1, NOROUND);

    if (left->sign == 0)
    {               /* Is the base number Zero?          */
        if (NegativePower)                  /* was power negative?               */
        {
            /*  this is a no no, report error.   */
            reportException(Error_Overflow_power);
        }
        else if (powerValue == 0)           /* Is power value zero?              */
        {
            /*  yes, return value of one         */
            return(RexxNumberString *)IntegerOne;
        }
        else                                /* otherwise power was positive      */
        {
            /*       return value of zero        */
            return(RexxNumberString *)IntegerZero;
        }

    }                                    /* Will the result exponent overflow?*/
    if ((highBits(Numerics::abs(left->exp + left->length - 1)) +
         highBits(Numerics::abs(powerValue)) + 1) > LONGBITS )
    {
        /* yes, report error and return.     */
        reportException(Error_Overflow_overflow, this, OREF_POWER, PowerObj);
    }
    /* Will the result overflow ?        */
    if (Numerics::abs((wholenumber_t)(left->exp + left->length - 1)) * powerValue > Numerics::MAX_EXPONENT)
    {
        /* yes, report error and return.     */
        reportException(Error_Overflow_overflow, this, OREF_POWER, PowerObj);
    }

    if (powerValue != 0)
    {               /* a non-zero power value?           */
                    /* yes, do the power operation.      */
                    /* get storage for Accumulator data. */
        AccumObj = (RexxNumberStringBase *)buffer_alloc(sizeof(RexxNumberStringBase));
        memcpy(AccumObj, left, sizeof(RexxNumberStringBase));
        /* initialize the Accumulator object.*/
        /* this has all data of NumberString */
        /* except the digits data            */

        /* Find out how many digits are in   */
        /*  power value, needed for actual   */
        /*  precision value to be used in    */
        /*  the computation.                 */
        for (extra=0, OldNorm = powerValue; OldNorm ;extra++ )
        {
            OldNorm /= 10;                  /* Divide value by ten, keeping int  */
        }
        NumberDigits += (extra + 1);        /* adjust digits setting to reflect  */

                                            /* size of buffers, for              */
                                            /*multiplication                     */
        AccumLen = (2 * (NumberDigits+1)) + 1;
        /* get storage for Output data       */
        OutPtr = buffer_alloc(AccumLen);
        /* get storage for Accumulator Data  */
        Accum = buffer_alloc(AccumLen);
        AccumPtr = Accum;                   /* Accum will point to start of      */
                                            /* storage block that AccumPtr is in.*/

                                            /* Initialize Accumulator digit data */
                                            /*  start with initial data.         */
        memcpy(AccumPtr, left->number, left->length);
        /* The power operation is defined    */
        /* to use bitwise reduction          */

        NumBits = LONGBITS;                 /* Get total number of bits in long  */

                                            /* Find first non-zero left most bit */
        while (!((size_t)powerValue & HIBIT))
        {
            powerValue <<= 1;                 /*  bit is zero shift bits 1 to left */
            NumBits--;                        /*  one less bit.                    */
        }                                   /* endwhile                          */

        /* turn off this 1st 1-bit, already  */
        /* taken care of. Skip 1st Multiply  */
        powerValue = (wholenumber_t) ((size_t)powerValue & LOWBITS);

        while (NumBits--)
        {                 /* while 1-bits remain in power.     */
            if ((size_t) powerValue & HIBIT)
            { /* is left most bit a 1? */
                /* yes, we need to multiply number by*/
                /*  Acummulator.                     */
                /* go do multiply.  AccumPtr will get*/
                /* assigned result of multiply       */
                AccumPtr = multiplyPower(AccumPtr, AccumObj, left->number, (RexxNumberStringBase *) left, OutPtr, AccumLen, NumberDigits);
                /* We now call AdjustNumber to make  */
                /* sure we stay within the required  */
                /* precision and move the Accum      */
                /* data back to Accum.               */
                AccumPtr = AccumObj->adjustNumber(AccumPtr, Accum, AccumLen, NumberDigits);
            }
            if (NumBits)
            {                     /* any 1-bits left in power?         */
                                  /* yes, we need to Square the Accum  */
                                  /* go do multiply.  AccumPtr will get*/
                                  /* assigned result of squaring       */
                AccumPtr = multiplyPower(AccumPtr, AccumObj, AccumPtr, AccumObj,  OutPtr, AccumLen, NumberDigits);
                /* We now call AdjustNumber to make  */
                /* sure we stay within the required  */
                /* precision and move the Accum      */
                /* data back to Accum.               */
                AccumPtr = AccumObj->adjustNumber(AccumPtr, Accum, AccumLen, NumberDigits);
            }
            powerValue <<= 1;                  /* shift power bits one to the left  */
        }                                   /* Finished with Power 1st step.     */

        if (NegativePower)
        {                /* is this a negative power operation*/
                         /* yes, so we need to invert value.  */
            AccumPtr = dividePower(AccumPtr, AccumObj, Accum, NumberDigits);
        }

        NumberDigits -= (extra + 1);        /* reset digits setting to original; */
                                            /* Remove all leading zeros.         */
        AccumPtr = AccumObj->stripLeadingZeros(AccumPtr);

        /* Is result bigger than digits?     */
        if (AccumObj->length > NumberDigits)
        {
            /* Yes, we need to adjust result     */
            /* increase exponent by amount over. */
            AccumObj->exp += (AccumObj->length - NumberDigits);
            AccumObj->length = NumberDigits;   /* Length is same as Digits          */
            AccumObj->mathRound(AccumPtr);     /* round result if necessary.        */
        }
        /* We now remove any trailing blanks */
        /* point to last digit in result     */
        TempPtr = AccumPtr + AccumObj->length -1;
        /* While there are trailing zeros    */
        while (!*TempPtr && AccumObj->length)
        {
            TempPtr--;                        /*  point to next character.         */
            AccumObj->length--;               /*  Result is one digit less.        */
            AccumObj->exp++;                  /*  Adjust expont up one             */
        }

        /* get new numberString Object for   */
        /* result length. No initial Data    */
        result = new (AccumObj->length) RexxNumberString (AccumObj->length);

        result->sign = AccumObj->sign;      /* fill in the data of result from   */
        result->exp  = AccumObj->exp;       /*  AccumObj.                        */
        result->length = AccumObj->length;
        /* copy digit data from AccumPtr.    */
        memcpy(result->number, AccumPtr, result->length);
    }
    else
    {                             /* Power value is zero.              */
                                  /* result is 1.                      */
        result = (RexxNumberString *)IntegerOne;
    }
    return result;                       /* return our result object.         */
}

char *RexxNumberString::multiplyPower(char *leftPtr, RexxNumberStringBase *left,
                     char *rightPtr, RexxNumberStringBase *right,
                     char *OutPtr, size_t OutLen, size_t NumberDigits)
/*********************************************************************/
/*   Function:  Multiply numbers for the power operation             */
/*********************************************************************/
{
    char *current, *resultPtr;
    char *AccumPtr = NULL;
    char MultChar;
    size_t  AccumLen;
    size_t  i;
    size_t ExtraDigit;


    memset(OutPtr, '\0', OutLen);         /* make output area is all zeros.    */

    AccumLen = 0;                         /* no data at this time.             */
    resultPtr = OutPtr + OutLen - 1;      /* Set up result, point to end of    */
                                          /*  data.                            */
                                          /* Set up digit ptr. small num       */
    current = rightPtr + right->length;   /* get last digit of number.         */

    for (i = right->length ; i ; i-- )
    {  /* do for all multiplier digits      */
        current--;                          /* shift add location by 1.          */
        MultChar = *current;                /* get new multiplier digit.         */
        if (MultChar)                       /* is this new digit a Zero?         */
        {
            /* nope, do multiplication of this   */
            /* digit                             */
            AccumPtr = addMultiplier(leftPtr, left->length, resultPtr,  MultChar);
        }
        resultPtr--;                        /* Backup Result Ptr, for next digit */
    }                                     /* go do next digit.                 */
    /* Get length of computed number.    */
    AccumLen = (size_t)(++resultPtr - AccumPtr) + right->length;

    /* AccumPtr now points to result, and*/
    /*  the len of result is in AccumLen */

    /* We will now get a real Object     */
    if (AccumLen > NumberDigits)
    {        /* Is result len greater then Digits */
        ExtraDigit = AccumLen - NumberDigits;/* Yes, save amount over for exp     */
    }
    else
    {
        ExtraDigit = 0;                      /*Length OK, no adjusting Exp.       */
    }

                                             /* compute the resulting Exponent    */
    left->exp += (right->exp + ExtraDigit);
    left->sign *= right->sign;            /* Compute the Sign                  */
    left->length = AccumLen;              /* Set length of result.             */


    return AccumPtr;                      /* return Pointer to result digits.  */
}                                      /* All done,                         */

char *RexxNumberString::dividePower(char *AccumPtr, RexxNumberStringBase *Accum, char *Output, size_t NumberDigits)
/*********************************************************************/
/*   Function:  Invert number from the power operation               */
/*********************************************************************/
{
    RexxNumberStringBase *left;
    char  leftBuffer[sizeof(RexxNumberStringBase)];
    char *Num1, *Num2;
    char *resultPtr, *leftPtr, *result;
    int   multiplier, rc;
    int   DivChar, thisDigit;
    wholenumber_t  CalcExp;
    size_t resultDigits;
    size_t  totalDigits;

    /* NOTE: this routine if very similiar to the Division      */
    /*   routine, these we kept as seperate routines since there*/
    /*   are enough subtile differences between the operations  */
    /*   that combining them would make an already complex      */
    /*   routine even more so.  When fixing/updating/adding to  */
    /*   this routin also check Division    for similiar updates*/

    totalDigits = ((NumberDigits + 1) * 2) + 1;
    /* get buffer for left digit data.   */
    leftPtr = buffer_alloc(totalDigits);
    /* get buffer for result digit data. */
    result  = buffer_alloc(totalDigits);
    resultPtr = result;                   /* Set up the result, point to end of*/
                                          /*  data.                            */
                                          /* address the static part           */
    left = (RexxNumberStringBase *)leftBuffer;

    /* length of left starts same as     */
    /* Accum                             */
    left->length = Accum->length;
    left->exp = 0;                        /* no exponent to start with.        */
    *leftPtr = 1;                         /* place the digit 1 into left.      */
                                          /* fill the rest of data with Zero   */
    memset(leftPtr + 1, '\0', totalDigits - 1);
    /* calculate expected resultant exp  */
    CalcExp = -Accum->exp - (wholenumber_t)Accum->length + 1;

    Num1 = leftPtr;                       /* Num1 will be left digit pointer.  */
    Num2 = AccumPtr;                      /* Num2 is our input digit pointer   */

    /* When generate a best guess digits for result we will look*/
    /* use the 1st 2 digits of the dividend (if there are 2)    */
    /* we then add 1 to this DivChar to ensure than when we gues*/
    /* we either guess correctly of under guess.                */
    /*           _______________                                */
    /*  aabbbbbb ) xxyyyyyyyy                                   */
    /*                                                          */
    /*     DivChar = aa + 1                                     */

    DivChar = *Num2 * 10;                 /* Divide char is 1st 2 digits + 1   */
    if (Accum->length > 1)                /* more than 1 digit in Accum?       */
        DivChar += *(Num2 + 1);              /*  yes, get second digit for Div    */
    DivChar++;                            /* add 1 to Div number               */

    resultDigits = 0;         /* initializes digit values to zero. */
    thisDigit = 0;

    /* We are now to enter 2 do forever loops, inside the loops */
    /*  we test for ending conditions. and will exit the loops  */
    /*  when needed. This inner loop may need to break out of   */
    /*  both loops, if our divisor is reduced to zero(all finish*/
    /*  if this happens to do the no-no nad use a GOTO.         */
    /* The outer loop is used to obtain all digits for the resul*/
    /*  We continue in this loop while the divisor has NOT been */
    /*  reduced to zero and we have not reach the maximum number*/
    /*  of digits to be in the result (NumDigits + 1), we add   */
    /*  one to NumDigits so we can round if necessary.          */
    /* The inner loop conputs each digits of the result and     */
    /*  breaks to the outer loop when the next digit of result  */
    /*  is found.                                               */
    /* We compute a digit of result by continually taking best  */
    /*  guesses at how many times the dividend can go into the  */
    /*  divisor. Once The divisor becomes less than the dividend*/
    /*  we found this digit and we exit the inner loop. If the  */
    /*  divisor = dividend then we know dividend will go into   */
    /*  1 more than last guess, so bump up the last guess and   */
    /*  exit both loops (ALL DONE !!), if neither of the above  */
    /*  conditions are met our last guess was low, compute a new*/
    /*  guess using result of last one, and go though inner loop*/
    /*  again.                                                  */
    for (; ; )
    {                          /* do forever (outer loop)           */
        for (; ; )
        {                        /* do forever (inner loop)           */
                                 /* are two numbers equal in length?  */
            if (left->length == Accum->length)
            {
                /* yes, then compare the two numbers */
                rc = memcmp(Num1, Num2, left->length);
                if (rc < 0)                       /* is Num1(left) smaller?            */
                {
                    break;                           /* yes, break out of inner loop.     */
                }

                else if (rc == 0)
                {               /* are the two numebrs equal         */
                                /* yes, done with Division, cleanup  */
                    *resultPtr++ = (char)(thisDigit + 1);
                    resultDigits++;                  /* one more digit in result          */
                    goto PowerDivideDone;            /* break out of both loops.          */
                }
                else
                {
                    multiplier = *Num1;              /* Num2(Accum) is smaller,           */
                }
            }
            /* is left longer than Accum?        */
            else if (left->length > Accum->length)
            {
                /* calculate multiplier, next two    */
                /*digits                             */
                multiplier = *Num1 * 10 + *(Num1 + 1);
            }
            else
            {
                break;                           /* left is smaller, break inner loop */
            }

                                                 /* compute Multiplier for divide     */
            multiplier = multiplier * 10 / DivChar;
            if (multiplier == 0)               /* did it compute to 0?              */
            {
                multiplier = 1;                   /*  yes, can't be zero make it one.  */
            }

            thisDigit += multiplier;           /* add multiplier to this digit.     */
                                               /* now subtract                      */
            Num1 = subtractDivisor(Num1, left->length, Num2, Accum->length, Num1 + left->length - 1, multiplier);
            /* Strip off all leading zeros       */
            Num1 = left->stripLeadingZeros(Num1);
        }                                   /* end of inner loop                 */

        if (resultDigits || thisDigit)
        {    /* Have a digit for result?          */
            *resultPtr++ = (char) thisDigit;  /* yes, place digit in result.       */
            thisDigit = 0;                     /* reset digit value to zero;        */
            resultDigits++;                    /* one more digit in result;         */

                                               /* is there been reduced to zero     */
            if (*Num1 == '\0' || resultDigits > NumberDigits)
            {
                break;                            /* yes, were done, exit outer loop   */
            }

        }
        /* Was number reduced to zero?       */
        if (left->length == 1 && *Num1 == '\0')
        {
            break;                             /* yes, all done exit outer loop     */
        }

        CalcExp--;                          /* result exponent is one less.      */
        left->length++;                     /* length is one more.               */

                                            /* reset the number ptr to beginning */
                                            /* of the buffer.                    */
        Num1 = (char *)memmove(leftPtr, Num1, left->length);
        /* make sure traling end of buffer   */
        /* is reset to 0's.                  */
        memset(Num1 + left->length, '\0', totalDigits - left->length);
    }                                     /* end of outer loop                 */

    PowerDivideDone:                       /*All done doing divide now do       */
    ;                       /*  the cleanup stuff.               */

    Accum->length = resultDigits;         /* set length of result              */
    Accum->exp = CalcExp;                 /* set exponent of result.           */
    memcpy(Output, result, resultDigits); /* move result data to result area   */
    return Output;                        /* all done, return to caller.       */
}
