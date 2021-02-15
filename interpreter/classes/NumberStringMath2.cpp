/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Arithmetic function for the NumberString Class                             */
/*  Multiply/Divide/Power                                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "NumberStringClass.hpp"
#include "ArrayClass.hpp"
#include "BufferClass.hpp"
#include "Activity.hpp"
#include "ActivityManager.hpp"
#include "MethodArguments.hpp"
#include "ProtectedObject.hpp"


/**
 * Multiply current digit through "top" number and add
 * result to the accumulator.
 *
 * @param top      the pointer to the top number first digit
 * @param topLen   The length of the top number.
 * @param accumPtr The pointer to the last digit of the accumulator.
 * @param multChar The character to multiply by.
 *
 * @return The new accumulator top bounds.
 */
char *NumberString::addMultiplier(const char *top, wholenumber_t topLen, char *accumPtr, int multChar)
{
    // no carry at the start
    int carry = 0;
    // we do this starting from the least significant digit of the top number
    top += (topLen - 1);

    // while we have a more digits to process, do the
    // multiplication addition.
    while (topLen-- )
    {
        // Multiply char by top digit and add the accumvalue for position and
        // and carry. and adjust pointer to the next digit positions.
        int resultChar = carry + *accumPtr + (multChar * *top--);
        // and handle possible carries (the carry can be more than just
        // one for a multiply)
        carry = resultChar / 10;
        resultChar = resultChar % 10;

        // add the character to the accumulator buffer
        *accumPtr-- =  resultChar;

    }
    // if we still had a carry, add this to the accumulator in the front position
    if (carry != 0)
    {
        *accumPtr-- = (char)carry;
    }
    // return a pointer to the first accumulator character
    return ++accumPtr;
}


/**
 * Multiply two NumberString objects
 *
 * @param other  The other object.
 *
 * @return A new numberstring that is the product of the two number strings.
 */
NumberString *NumberString::Multiply(NumberString *other)
{
    Protected<BufferClass> outputBuffer;

    wholenumber_t digits = number_digits();

    // prepare both numbers, copying and rounding if necessary.
    NumberString *left = checkNumber(digits);
    NumberString *right = other->checkNumber(digits);


    // if either string is zero, then the result is also zero
    if (left->isZero() || right->isZero())
    {
        return new_numberstring("0", 1);
    }

    // we can optimize things by using the smaller number for
    // the individual digit multiplcation, which will need fewer passes.
    NumberString *largeNum = left;
    NumberString *smallNum = right;

    if (left->digitsCount < right->digitsCount)
    {
        largeNum = right;
        smallNum = left;
    }

    // get our buffer size
    size_t totalDigits = ((digits +1) * 2) + 1;

    // fast allocation buffer
    char resultBufFast[FAST_BUFFER];
    char *outPtr = resultBufFast;

    // if the digits are really big, then we need to allocate a larger buffer.
    // just allocate a buffer object and allow it to get garbage collected after
    // we're done.
    if (totalDigits > FAST_BUFFER)
    {
        outputBuffer = new_buffer(totalDigits);
        outPtr = outputBuffer->getData();
    }
    // make sure this is cleared out
    memset(outPtr, '\0', totalDigits);

    // set up the initial accumulator
    char *accumPtr = outPtr;
    wholenumber_t accumLen = 0;

    // this is where we start laying out the data...starting from the
    // far end of the buffer.
    char *resultPtr = accumPtr + totalDigits - 1;
    // we iterate through the small number multiplying with each of the
    // digits in the smaller number
    const char *current = smallNum->numberDigits + smallNum->digitsCount;

    // now process all of the digits
    for (size_t i = smallNum->digitsCount ; i > 0 ; i-- )
    {
        current--;
        // get the current multiplier character
        int multChar = *current;
        // we don't need to do anything with zero digits.  Other digits
        // we multiply and add
        if (multChar != 0)
        {
            // multiply the larger number by the current digit and add to the accumulator
            accumPtr = addMultiplier(largeNum->numberDigits, largeNum->digitsCount, resultPtr,  multChar);
        }

        // back up the result pointer for the next add position and handle the next digit.
        resultPtr--;
    }
    // update the accumulator length for the final result.
    accumLen = (++resultPtr - accumPtr) + smallNum->digitsCount;

    // accumPtr now points to result,
    //  the len of result is in accumLen

    wholenumber_t extraDigits = 0;
    // if this is longer than the current digits, the excess is added into the
    // exponent
    if (accumLen > digits)
    {
        // we also need to chop the length to digits + 1 (we'll use that to round
        // the final result)
        extraDigits = accumLen -(digits + 1);
        accumLen = digits + 1;
    }

    // now get a numberstring object large enough to hold this result
    NumberString *result = new_numberstring(NULL, accumLen);
    // the result exponent is the sum of the two operand exponents + the adjustment amount
    result->numberExponent = largeNum->numberExponent + smallNum->numberExponent + extraDigits;
    // the sign is computed by multiplying the signs
    result->numberSign = largeNum->numberSign * smallNum->numberSign;
    result->digitsCount = accumLen;

    // make sure this is in the correct precision (also copies the digits into
    // the result object).
    result->adjustPrecision(accumPtr, digits);
    return result;
}


/**
 * Perform a subtraction during division.
 *
 * @param data1   The data to subtract from.
 * @param length1 The length of the string subtract from.
 * @param data2   The dividend string.
 * @param length2 The length of the dividend.
 * @param result  The location to place the subtraction result.
 * @param mult    The multiplier for the subtraction.
 *
 * @return the pointer to the first character of the result.
 */
char *NumberString::subtractDivisor(const char *divisor, wholenumber_t divisorLength,
    const char *dividend, wholenumber_t dividendLength, char *result, int mult)
{
    // This rountine actually does the divide of the Best Guess
    //  Mult.  This Best guess is a guess at how many times the
    //  dividend will go into the divisor, since it is only a
    //  guess we make sure we guess to the low side, if low we
    //  always adjust our guess and recompute.
    // We multiply the Dividend(Data2) by mult and then subtract
    //  the result from the Divisor (Data1) and this result is
    //  put into RESULT.  Since Mult is guaranteed to be correct
    //  or low, the result is guaranteed to never be negative.
    //
    //            xxMxxx           <- M signifies this Mult
    //          ______________
    // dividend ) divisor
    //           -iiiiiiiii        <- result of M x Divisor
    //           ===========
    //            rrrrrrrrr        <- result returned


    // point to the least significant character of both operands
    const char *divisorData = divisor + (divisorLength - 1);
    const char *dividendData = dividend + (dividendLength -1);

    // set up the result pointer for the subtraction
    char *outPtr = result + 1;
    int carry = 0;
    // get the differench in length
    wholenumber_t extra = divisorLength - dividendLength;

    // process all digits in the dividend number
    while (dividendLength--)
    {
        int divChar = carry + *divisorData-- - (*dividendData-- * mult);
        // if this went negative, we've got a borrow, and it could be a
        // pretty large borrow (the mult result can be as large as 81)
        if (divChar < 0)
        {
            // make positive by adding 100
            divChar += 100;
            // calculate the borrow out (will be a negative number after subtracting 10)
            carry = (divChar/10) - 10;
            // the character at this position is the remainder
            divChar %= 10;
        }
        // no carry on this iteration
        else
        {
            carry = 0;
        }
        // set the result character
        *--outPtr = (char)divChar;
    }

    // are their extra characters to process?
    // we need to propagate any carries until this stops
    if (extra > 0)
    {
        // if the carry is zero, we can just copy the remaining
        // digits over
        if (carry == 0)
        {
            while (extra--)
            {
                *--outPtr = (char)*divisorData--;
            }
        }
        else
        {
            // need to copy and take the carry into account.
            while (extra--)
            {
                int divChar = carry + *divisorData--;
                // This is a straightforward subtraction now, so the borrow
                // will just be a single digit
                if (divChar < 0)
                {
                    divChar += 10;
                    carry = -1;
                    *--outPtr = (char)divChar;
                }
                // had the first non-carry, so now we can copy the data
                else
                {
                    *--outPtr = (char)divChar;
                    while (extra--)
                    {
                        *--outPtr = *divisorData--;
                    }
                    break;
                }
            }
        }
    }
    return outPtr;
}


/**
 * Divide two numbers
 *
 * @param other  The left-hand-side of the division operation.
 * @param DivOP  The type of division operation (divide, integer divide, or remainder)
 *
 * @return The division result.
 */
NumberString *NumberString::Division(NumberString *other, ArithmeticOperator divOP)
{
    // buffers for intermediate results (just the numeric data)
    NumberStringBase accumBuffer;
    NumberStringBase saveLeftBuffer;
    NumberStringBase saveRightBuffer;
    Protected<BufferClass> outputBuffer;

    // static sized buffers for typical calculation sizes.
    char leftBufFast[FAST_BUFFER];
    char rightBufFast[FAST_BUFFER];
    char outBufFast[FAST_BUFFER];

    // NOTE: this is very similiar to the PowerDivide
    //   method, these we kept as seperate routines since there
    //   are enough subtile differences between the objectPointer operations
    //   that combining them would make an already complex
    //   algorithm even more so.  When fixing/updating/adding to
    //   this code also check PowerDivide for similiar updates

    // handle the zero cases, which are pretty quick.
    // if the other is zero, this is an error
    if (other->isZero())
    {
        reportException(Error_Overflow_zero);
    }
    // if this number is zero, the result is also zero.
    if (isZero())
    {
        return (NumberString *)IntegerZero;
    }

    // set of pointers for our temporary values
    NumberStringBase *accum = &accumBuffer;
    NumberStringBase *saveLeft = &saveLeftBuffer;
    NumberStringBase *saveRight = &saveRightBuffer;
    wholenumber_t digits = number_digits();

    // either of these values might require rounding before starting the
    // operation, which might copy the values
    NumberString *left = checkNumber(digits);
    NumberString *right = other->checkNumber(digits);

    // calculate the probable result exponent from the two operand
    // exponends and the length difference.
    wholenumber_t calcExp = left->numberExponent - right->numberExponent;
    calcExp += left->digitsCount - right->digitsCount;

    // a negative exponent means this value will be less than
    // zero.  If we are doing integer divide or remainder, we
    //
    // is exp < 0 and doing // or %
    if (calcExp < 0 && divOP != OT_DIVIDE)
    {
        // if this is integer division, the result is zero
        if (divOP == OT_INT_DIVIDE)
        {
            return (NumberString *)IntegerZero;
        }
        // for the remainder operation, this is the result (with suitable rounding,
        // of course)
        else
        {
            // this must be a new number value set with the current numeric values
            NumberString *result = left->prepareOperatorNumber(digits + 1, digits, NOROUND);
            result->setupNumber();
            return result;
        }
    }

    size_t totalDigits = ((digits + 1) * 2) + 1;

    char *leftNum  = leftBufFast;
    char *rightNum = rightBufFast;
    char *output   = outBufFast;

    // we have automatic buffers for these that will handle typical
    // sizes.  If larger than that, we allocate real buffer objects
    // and just allow them to be garbage collected at the end
    if (totalDigits > FAST_BUFFER)
    {
        // we can use a single buffer and chop it up
        outputBuffer = new_buffer(totalDigits * 3);
        leftNum = outputBuffer->getData();
        rightNum = leftNum + totalDigits;
        output = rightNum + totalDigits;
    }

    // make a copy of the input data into the buffers and pad the
    // rest of the buffer with zeros
    memcpy(leftNum, left->numberDigits, left->digitsCount);
    memset(leftNum + left->digitsCount, '\0', totalDigits - left->digitsCount);

    memcpy(rightNum, right->numberDigits, right->digitsCount);
    memset(rightNum + right->digitsCount, '\0', totalDigits - right->digitsCount);
    char *resultPtr = output;

    // copy the numberstring information as well
    *saveRight = *right;
    *saveLeft = *left;

    char *saveLeftPtr = NULL;
    char *saveRightPtr = NULL;

    // if this is a remainder, we need to save the pointers to
    // the original data
    if (divOP == OT_REMAINDER)
    {
        saveLeftPtr  = leftNum;
        saveRightPtr = rightNum;
        // force the dividend sign to be positive.
        saveRight->numberSign = 1;
    }

    // the result sign is easy to compute (note, we use the saved
    // right sign, which might have been made positive for the
    // remainder operation
    accum->numberSign = left->numberSign * saveRight->numberSign;

    wholenumber_t rightPadding = 0;

    // we might need to pad the right number if it is shorter.
    // if the right is longer, we pad the left number to the same length
    if (saveRight->digitsCount > saveLeft->digitsCount)
    {
        // we're making the left number longer, which pads the number
        // because the rest of the number buffer is zeros.
        saveLeft->digitsCount = saveRight->digitsCount;
    }
    // the right number is shorter...we'll need to pad that out by the difference
    else
    {

        rightPadding = saveLeft->digitsCount - saveRight->digitsCount;
        // we want both numbers the same
        saveRight->digitsCount = saveLeft->digitsCount;
    }

    // set the new exponents using relative forms
    saveLeft->numberExponent = digits * 2 - saveLeft->digitsCount + 1;
    saveRight->numberExponent = rightPadding;
    wholenumber_t adjustLeft = 0;

    // When generating a best guess digits for result we will look
    // use the 1st 2 digits of the dividend (if there are 2)
    // we then add 1 to this DivChar to ensure than when we gues
    // we either guess correctly of under guess.
    //           _______________
    //  aabbbbbb ) xxyyyyyyyy
    //
    //     DivChar = aa + 1

    // the division character is the first two digits, or
    // if there is only one digit, the second digit is effectively
    // a zero.
    int divChar = *rightNum * 10;
    if (saveRight->digitsCount > 1)
    {
        divChar += *(rightNum + 1);
    }
    // and add 1 to our division characters so that we will err on the
    // low side
    divChar++;

    // the count of digits in the result
    wholenumber_t resultDigits = 0;
    int thisDigit = 0;

    // We are now to enter 2 do forever loops, inside the loops
    //  we test for ending conditions. and will exit the loops
    //  when needed. This inner loop may need to break out of
    //  both loops, if our divisor is reduced to zero(all finish
    //  if this happens to do the no-no nad use a GOTO.
    // The outer loop is used to obtain all digits for the resul
    //  We continue in this loop while the divisor has NOT been
    //  reduced to zero and we have not reach the maximum number
    //  of digits to be in the result (NumDigits + 1), we add
    //  one to NumDigits so we can round if necessary.
    // The inner loop conputs each digits of the result and
    //  breaks to the outer loop when the next digit of result
    //  is found.
    // We compute a digit of result by continually taking best
    //  guesses at how many times the dividend can go into the
    //  divisor. Once The divisor becomes less than the dividend
    //  we found this digit and we exit the inner loop. If the
    //  divisor = dividend then we know dividend will go into
    //  1 more than last guess, so bump up the last guess and
    //  exit both loops (ALL DONE !!), if neither of the above
    //  conditions are met our last guess was low, compute a new
    //  guess using result of last one, and go though inner loop
    //  again.

    // outer loop
    for (; ; )
    {
        int multiplier;
        // inner loop
        for (; ; )
        {
            // are the two numbers now of equal length?
            if (saveLeft->digitsCount == saveRight->digitsCount)
            {
                // directly compare the two numbers
                int rc = memcmp(leftNum, rightNum, saveLeft->digitsCount);
                // if the left number is smaller, we're done with the inner
                // loop.
                if (rc < 0)
                {
                    break;
                }
                // if the inner numbers are equal and this is not a remainder
                // op, we can terminate the entire loop
                else if (rc == 0 && divOP != OT_REMAINDER)
                {
                    // divided evenly, if you add in one more to the guess
                    *resultPtr++ = (char)(thisDigit + 1);
                    resultDigits++;
                    // this breaks out of both loops
                    goto PowerDivideDone;
                }
                // either the number is greater or we're doing a remainder
                else
                {
                    // just use one digit from the divisor for this next guess
                    multiplier = *leftNum;
                }
            }
            // the left number is longer than the accumulator?
            else if (saveLeft->digitsCount > saveRight->digitsCount)
            {
                // calculate multiplier using the next two digits
                // note, since the left number is longer than the right,
                // we know we have at least two digis.
                multiplier = *leftNum * 10 + *(leftNum + 1);
            }
            // the divisor is smaller than the dividend, so we break out of the loop
            else
            {
                break;
            }

            // we found this digit of result, go to outer loop and finish up
            // processing for this digit. Compute Multiplier for actual divide
            multiplier = multiplier * 10 / divChar;

            // that is how many times will digit
            // of dividend go into divisor, using
            // the 1st 2 digits of each number
            // compute our Best Guess for this
            // digit

            // if this computed to zero, make it 1
            if (multiplier == 0)
            {
                multiplier = 1;
            }

            // we know the divident goes into divisor at least one more time.
            thisDigit += multiplier;


            // divide the digit through and see if we guessed correctly.
            leftNum = subtractDivisor(leftNum, saveLeft->digitsCount, rightNum, saveRight->digitsCount, leftNum + saveLeft->digitsCount - 1, multiplier);
            // skip over any leading zeros
            leftNum = saveLeft->stripLeadingZeros(leftNum);
            // end of inner loop, go back and guess again !! This might have been the right guess.
        }
        // we only add zero digits if we have other digits.  non-zero
        // digits always get added
        if (resultDigits != 0 || thisDigit != 0)
        {
            // add this to the result
            *resultPtr++ = (char) thisDigit;
            thisDigit = 0;
            resultDigits++;

            // we stop processing of A) the left number has been reduced
            // to zero or B) The result has reached our digits limit.
            if (*leftNum == '\0' || resultDigits > digits)
            {
                break;
            }
        }

        // we have different termination rules for int divide and remainder operations.
        if (divOP != OT_DIVIDE)
        {
            // have we generated all of the integer digits yet?
            // then we are done.
            if (calcExp <= 0)
            {
                break;
            }
        }
        // Was number reduced to zero?  We divided evenly
        if (saveLeft->digitsCount == 1 && *leftNum == '\0')
        {
            break;
        }


        // we're not done dividing yet, we
        //  need to adjust expected exponent
        //  by one to the left
        calcExp--;
        // if we are still padding the right number, use one less digit on the
        // next pass
        if (rightPadding > 0)
        {
            saveRight->digitsCount--;
            rightPadding--;
        }
        // we decreased the left value to to the size of the right
        // before starting, so we add the digits back in as we progress.
        else
        {
            saveLeft->digitsCount++;
        }
    }

    // We've ended the entire division because we've hit equality.
    PowerDivideDone:
    ;

    // if this is a // or % operation, the result might be bad.
    // This might not be expressible as a whole number because of
    // the relative size of the operands, or we have a result with
    // no integer portion
    if ((divOP != OT_DIVIDE) && ((calcExp >= 0 &&
           ( resultDigits + calcExp) > digits) ||
         (calcExp < 0  && std::abs(calcExp) > resultDigits)))
    {
        reportException(divOP == OT_REMAINDER ? Error_Invalid_whole_number_rem : Error_Invalid_whole_number_intdiv);
    }

    // if we're doing a remainder operation, we've really done an integer divide to this
    // point and now need to figure out the remainder portion
    if (divOP == OT_REMAINDER)
    {
        // if we managed to generate any result digits
        if (resultDigits != 0)
        {
            // the left number is the remainder.  If the first digit
            // is zero, there is no remainder
            if (*leftNum != 0)
            {
                // this is our result
                resultPtr = leftNum;
                // now we need to calculate the exponent, adjusting for any added zeros
                saveLeftPtr += left->digitsCount;
                saveRightPtr = resultPtr + saveLeft->digitsCount + adjustLeft;
                accum->numberExponent = left->numberExponent - (saveRightPtr - saveLeftPtr);
                // the result length is the remaining digits count
                accum->digitsCount = saveLeft->digitsCount;
            }
            // we have a zero result, just return a zero
            else
            {
                return (NumberString *)IntegerZero;
            }
        }
        // no digits in result, remainder is the left number (this)
        else
        {
            // we return a copy of the left number
            NumberString *result = clone();
            result->setupNumber();
            return result;
        }
    }
    // this is a real division (but possibly integer division) so we need to finish up the result
    else
    {
        // if we have some sort of result digits, set up for building the final number string
        if (resultDigits != 0)
        {
            resultPtr = output;
            accum->digitsCount = resultDigits;
            accum->numberExponent = calcExp;
            // if the result is too big, we need to round to the digits setting
            if (accum->digitsCount > digits)
            {
                // we shorten the length and increase the exponent by the delta
                accum->numberExponent += (accum->digitsCount - digits);
                accum->digitsCount = digits;
                // see if we need to round
                accum->mathRound(resultPtr);
            }

            // We now remove any trailing zeros in the result.
            leftNum = resultPtr + accum->digitsCount - 1;

            // NOTE:  We know we have at least one non-zero digit, so this loop
            // will not remove the entire result
            while (*leftNum == 0 && accum->digitsCount > 0)
            {
                leftNum--;
                // changes in length must be reflected with an equal and
                // opposite change in exponent.
                accum->digitsCount--;
                accum->numberExponent++;
            }
        }
        // no digits in the result, the answer is zero.  This generally
        // only happens with integer division.
        else
        {
            return(NumberString *)IntegerZero;
        }
    }

    // if this has been an integer divide, we can return the result as a
    // RexxInteger if we have at most Numerics::REXXINTEGER_DIGITS digits
    // no need to check the current numeric digits setting, as an
    // integer divide will have raised
    // Error 26.11:  Result of % operation did not result in a whole number
    // for any result outside of the current digits setting
    if (divOP == OT_INT_DIVIDE &&
        accum->digitsCount + accum->numberExponent <= Numerics::REXXINTEGER_DIGITS &&
        // we also require accum->numberExponent >= 0 .. not sure if this might happen
        accum->numberExponent >= 0)
    {
      return (NumberString *)new_integer(accum->numberSign < 0, resultPtr, accum->digitsCount, accum->numberExponent);
    }

    // ok, accum is the number data, resultPtr is the result data
    NumberString *result = new (accum->digitsCount) NumberString (accum->digitsCount);

    result->digitsCount = accum->digitsCount;
    result->numberExponent = accum->numberExponent;
    result->numberSign = accum->numberSign;
    // note, we have already rounded, so this should work without rounding now.
    result->adjustPrecision(resultPtr, digits);
    return result;
}


/**
 * Adjust the number data to be with the correct NUMERIC
 * DIGITS setting.
 *
 * @param numPtr    The pointer to the start of the current numeric data.
 * @param result    Where this should be copied back to after adjustment.
 * @param resultLen The length of the result area.  Data is copied relative
 *                  to the end of the data.
 * @param digits    The digits setting for the adjustment.
 *
 * @return The new number ptr after the copy.
 */
char *NumberStringBase::adjustNumber(char *numPtr, char *result, wholenumber_t resultLen, wholenumber_t digits)
{
    // remove all leading zeros that might have occurred after the operation.
    numPtr = stripLeadingZeros(numPtr);

    // after stripping, is the length of the number larger than the digits setting?
    if (digitsCount > digits)
    {
        // NOTE:  the original version had a bug where it was attempting to adjust the
        // exponent, but because it updated the length first, the net adjustment was zero.
        // I "fixed" this and completely broke the power operation.  I really don't understand
        // why the exponent does not need adjusting here, but it appears it doesn't.

        // adjust the length down to the digits value
        digitsCount = digits;
        // round the number.
        mathRound(numPtr);
    }
    // copy the data into the result area, aligned with the end of the
    // buffer.  We return the pointer to the new start of the number
    return (char *)memcpy(((result + resultLen - 1) - digitsCount), numPtr, digitsCount);
}


/**
 * Perform the Arithmetic power operation
 *
 * @param PowerObj The power we're raising this number to.
 *
 * @return The power result.
 */
NumberString *NumberString::power(RexxObject *powerObj)
{
    requiredArgument(powerObj, ARG_ONE);
    wholenumber_t powerValue;
    Protected<BufferClass> outputBuffer;

    if (!powerObj->numberValue(powerValue, number_digits()))
    {
        reportException(Error_Invalid_whole_number_power, powerObj);
    }

    bool negativePower = false;

    // if the power is negative, we'll first calculate the power
    // as a positive and take the reciprical at the end.
    if (powerValue < 0)
    {
        negativePower = true;
        powerValue = -powerValue;
    }

    wholenumber_t digits = number_digits();

    // get a potential copy of this, truncated to have at most digits + 1 digits.
    NumberString *left = prepareOperatorNumber(digits + 1, digits, NOROUND);

    // if we are raising zero to a power, there are some special rules in play here.
    if (left->isZero())
    {
        // if the power is negative, this is an overflow error
        if (negativePower)
        {
            reportException(Error_Overflow_power);
        }
        // mathematically, 0**0 is undefined.  Rexx defines this as 1
        else if (powerValue == 0)
        {
            return (NumberString *)IntegerOne;
        }
        // zero to a non-zero power is zero
        else
        {
            return (NumberString *)IntegerZero;
        }
    }

    // we figure out ahead of time if this will overflow without having to
    // do all of the calculation work.  This tests if we can even start the
    // process because it will required too many bits to calculate.  This is
    // more likely to fail with 32-bit compiles than 64-bit because we have
    // fewer bits to work with.
    if ((highBits(std::abs(left->numberExponent + left->digitsCount - 1)) +
         highBits(std::abs(powerValue)) + 1) > SIZEBITS )
    {
        reportException(Error_Overflow_overflow, this, GlobalNames::POWER, powerObj);
    }

    // we can also calculate the exponent magnitude ahead of time and fail this early.
    if (std::abs(left->numberExponent + left->digitsCount - 1) * powerValue > Numerics::MAX_EXPONENT)
    {
        reportException(Error_Overflow_overflow, this, GlobalNames::POWER, powerObj);
    }

    // anything raised to the 0th power is 1, this is an easy one.
    if (powerValue == 0)
    {
        return (NumberString *)IntegerOne;
    }

    // we create a dummy numberstring object and initialize it from the target number
    NumberStringBase accumNumber = *left;
    NumberStringBase *accumObj = &accumNumber;

    // Find out how many digits are in power value, needed for actual
    //  precision value to be used in the computation.
    wholenumber_t extra = 0;
    wholenumber_t oldNorm = powerValue;

    // keep dividing the value by 10 until we hit zero.  That will be the number of
    // powers of 10 we have in the number.
    for (; oldNorm != 0; extra++)
    {
        oldNorm /= 10;
    }

    // add these extra digits into the working digits setting
    digits += (extra + 1);
    // and this is the size of the buffers we need for our accumulators
    wholenumber_t accumLen = (2 * (digits + 1)) + 1;

    // get a single buffer object with space for two values of this size
    outputBuffer = new_buffer(accumLen * 2);
    char *outPtr = outputBuffer->getData();
    char *accumBuffer = outPtr + accumLen;
    char *accumPtr = accumBuffer;

    // copy the the initial number data into the buffer
    memcpy(accumPtr, left->numberDigits, left->digitsCount);

    // the power operation is defined to use bitwise reduction
    size_t numBits = SIZEBITS;

    // get the first non-zero bit shifted to the top of the number
    // this will both position us to start the process and
    // also give us the a count of how many bits we're working with.
    while (!((size_t)powerValue & HIBIT))
    {
        powerValue <<= 1;
        numBits--;
    }

    // turn off this 1st 1-bit, already take care of by
    // the starting accumulator (essentially, a multiply by 1)
    powerValue = (wholenumber_t) ((size_t)powerValue & LOWBITS);

    // ok, we know how many passes we need to make on this number.
    while (numBits--)
    {
        // if this bit is on, then we need to multiply the
        // number by the accumulator.
        if ((size_t) powerValue & HIBIT)
        {
            // do the multiply and get the new high position back
            accumPtr = multiplyPower(accumPtr, accumObj, left->numberDigits, left, outPtr, accumLen, digits);
            // We now call AdjustNumber to make sure we stay within the required
            // precision and move the Accum data back to Accum.
            accumPtr = accumObj->adjustNumber(accumPtr, accumBuffer, accumLen, digits);
        }
        // if we will be making another pass through this loop, we need
        // to square the accumulator
        if (numBits > 0)
        {
            accumPtr = multiplyPower(accumPtr, accumObj, accumPtr, accumObj,  outPtr, accumLen, digits);
            // and adjust the result again
            accumPtr = accumObj->adjustNumber(accumPtr, accumBuffer, accumLen, digits);
        }
        // and shift to the next bit position
        powerValue <<= 1;
    }

    // if this was actually a negative power, take the reciprical now
    if (negativePower)
    {
        accumPtr = dividePower(accumPtr, accumObj, accumBuffer, digits);
    }

    // reset the digits to the original and remove all leading zeros
    digits -= (extra +1);               // reset digits setting to original;
    accumPtr = accumObj->stripLeadingZeros(accumPtr);

    // Is result bigger than digits?
    if (accumObj->digitsCount > digits)
    {
        // adjust to the shorter length and round if needed
        accumObj->numberExponent += (accumObj->digitsCount - digits);
        accumObj->digitsCount = digits;
        accumObj->mathRound(accumPtr);
    }

    // we now remove any trailing zeros in the result (same rules as
    // division)
    char *tempPtr = accumPtr + accumObj->digitsCount - 1;
    /* While there are trailing zeros    */
    while (*tempPtr == 0 && accumObj->digitsCount > 0)
    {
        tempPtr--;
        accumObj->digitsCount--;
        accumObj->numberExponent++;
    }

    // finally build a result object.
    NumberString *result = new (accumObj->digitsCount) NumberString (accumObj->digitsCount);

    result->numberSign = accumObj->numberSign;
    result->numberExponent  = accumObj->numberExponent;
    result->digitsCount = accumObj->digitsCount;
    memcpy(result->numberDigits, accumPtr, result->digitsCount);
    return result;
}


/**
 * Multiply numbers for the power operation
 *
 * @param leftPtr  The left number data
 * @param left     The left number numberstring data
 * @param rightPtr The right pointer data
 * @param right    The right number numberstring info.
 * @param outPtr   The ouput pointer.
 * @param outLen   The output buffer length
 * @param digits   the current digits setting
 *
 * @return The pointer to the new start of the accumulator data.
 */
char *NumberString::multiplyPower(const char *leftPtr, NumberStringBase *left,
                     const char *rightPtr, NumberStringBase *right,
                     char *outPtr, wholenumber_t outLen, wholenumber_t digits)
{
    // clear the output buffer of any previous results
    memset(outPtr, '\0', outLen);

    wholenumber_t accumLen = 0;
    char *accumPtr = NULL;
    // build the result from the end of the output location.
    char *resultPtr = outPtr + outLen - 1;
    // we iterate through the right number
    const char *current = rightPtr + right->digitsCount;

    // process all of the multiplier digits
    for (size_t i = right->digitsCount; i ; i-- )
    {
        current--;
        int multChar = *current;
        // only process non-zero digits
        if (multChar != 0)
        {
            // add in the left operand multiplied by the digit
            accumPtr = addMultiplier(leftPtr, left->digitsCount, resultPtr,  multChar);
        }
        // back up the result pointer for the next digit
        resultPtr--;
    }
    // get our new length
    accumLen = (++resultPtr - accumPtr) + right->digitsCount;

    // we might need to truncate to our digits setting
    wholenumber_t extraDigits = accumLen > digits ? accumLen - digits : 0;


    left->numberExponent += (right->numberExponent + extraDigits);
    // we recompute the sign every time we multiply
    left->numberSign *= right->numberSign;
    // we still use the full length in the accumulator
    left->digitsCount = accumLen;
    return accumPtr;
}


/**
 * Invert number from the power operation when a negative power is used.
 *
 * @param accumPtr The accumulator number.
 * @param accum    The accumulator number
 * @param output   The output buffer location.
 * @param digits   The digits count we're operating under.
 *
 * @return
 */
char *NumberString::dividePower(const char *accumPtr, NumberStringBase *accum, char *output, wholenumber_t digits)
{
    // NOTE: this routine is very similiar to the Division
    //   routine, these we kept as seperate routines since there
    //   are enough subtile differences between the operations
    //   that combining them would make an already complex
    //   routine even more so.  When fixing/updating/adding to
    //   this routin also check Division for similiar updates

    wholenumber_t totalDigits = ((digits + 1) * 2) + 1;
    Protected<BufferClass> outputBuffer;

    // set up temporary buffers for the calculations
    NumberStringBase leftBuffer;
    outputBuffer = new_buffer(totalDigits * 2);
    char *leftPtr = outputBuffer->getData();
    char *result = leftPtr + totalDigits;

    NumberStringBase *left = &leftBuffer;

    // length of left starts same as the accumulator
    left->digitsCount = accum->digitsCount;
    // no exponent to start with, but we start with a 1, followed by a lot of zeros.
    left->numberExponent = 0;
    *leftPtr = 1;
    memset(leftPtr + 1, '\0', totalDigits - 1);

    // this is our expected result exponent
    wholenumber_t calcExp = -accum->numberExponent - accum->digitsCount + 1;

    char *leftNumber = leftPtr;
    const char *rightNumber = accumPtr;

    // When generate a best guess digits for result we will look
    // use the 1st 2 digits of the dividend (if there are 2)
    // we then add 1 to this divChar to ensure than when we guess
    // we either guess correctly or under guess.
    //           _______________
    //  aabbbbbb ) xxyyyyyyyy
    //
    //     DivChar = aa + 1

    int divChar = *rightNumber * 10;
    if (accum->digitsCount > 1)
    {
        divChar += *(rightNumber + 1);
    }
    // add 1 to this pairing so that we will err on the high side.
    divChar++;

    wholenumber_t resultDigits = 0;
    int thisDigit = 0;

    // We are now to enter 2 do forever loops, inside the loops
    //  we test for ending conditions. and will exit the loops
    //  when needed. This inner loop may need to break out of
    //  both loops, if our divisor is reduced to zero(all finish
    //  if this happens to do the no-no nad use a GOTO.
    // The outer loop is used to obtain all digits for the result
    //  We continue in this loop while the divisor has NOT been
    //  reduced to zero and we have not reach the maximum number
    //  of digits to be in the result (NumDigits + 1), we add
    //  one to NumDigits so we can round if necessary.
    // The inner loop conputs each digits of the result and
    //  breaks to the outer loop when the next digit of result
    //  is found.
    // We compute a digit of result by continually taking best
    //  guesses at how many times the dividend can go into the
    //  divisor. Once The divisor becomes less than the dividend
    //  we found this digit and we exit the inner loop. If the
    //  divisor = dividend then we know dividend will go into
    //  1 more than last guess, so bump up the last guess and
    //  exit both loops (ALL DONE !!), if neither of the above
    //  conditions are met our last guess was low, compute a new
    //  guess using result of last one, and go though inner loop
    //  again.
    for (; ; )
    {
        int multiplier;
        for (; ; )
        {
            // if we've reached the point where the two numbers are
            // of equal length, we've reached the point where we can
            // directly compare to see where we are at.
            if (left->digitsCount == accum->digitsCount)
            {
                // do a direct comparison of the digits
                // if the left is smaller, we've got a good estimate
                // for this digit
                int rc = memcmp(leftNumber, rightNumber, left->digitsCount);
                if (rc < 0)
                {
                    break;
                }
                // the number is equal.  Our current digit is exactly one
                // too small.  Adjust it and exit the entire
                // division loop
                else if (rc == 0)
                {

                    *result++ = (char)(thisDigit + 1);
                    resultDigits++;
                    goto PowerDivideDone;
                }
                // we can pick up the next multiplier and continue
                else
                {
                    multiplier = *leftNumber;
                }
            }
            // unequal lengths...if the left is still longer than the accumulator, we
            // get the next set of digits
            else if (left->digitsCount > accum->digitsCount)
            {
                // NB:  since the left number is longer, we know there are
                // still at least 2 digits here
                multiplier = *leftNumber * 10 + *(leftNumber + 1);
            }
            // the left is smaller, break the inner loop
            else
            {
                break;
            }

            // get the multiplier...a zero multiplier gets wrapped to 1
            multiplier = multiplier * 10 / divChar;
            if (multiplier == 0)
            {
                multiplier = 1;
            }

            // add to the current digit and subtract
            thisDigit += multiplier;

            leftNumber = subtractDivisor(leftNumber, left->digitsCount, rightNumber, accum->digitsCount, leftNumber + left->digitsCount - 1, multiplier);
            // and strip off all leading zeros so we're working with real lengths
            leftNumber = left->stripLeadingZeros(leftNumber);
        }

        // any non-zero digit gets added to the result.  Zeros only get
        // added if we have a previous non-zero.
        if (resultDigits > 0 || thisDigit != 0)
        {
            *result++ = (char) thisDigit;
            thisDigit = 0;
            resultDigits++;

            // if the left number has gone to zero or we've hit our digits limit, we're
            // done processing
            if (*leftNumber == '\0' || resultDigits > digits)
            {
                break;
            }

        }

        // have we reduced to exactly zero?  we're done
        if (left->digitsCount == 1 && *leftNumber == '\0')
        {
            break;
        }

        calcExp--;
        left->digitsCount++;
        // shift the left number back to the front of the buffer and padd with zeros again
        leftNumber = (char *)memmove(leftPtr, leftNumber, left->digitsCount);
        memset(leftNumber + left->digitsCount, '\0', totalDigits - left->digitsCount);
    }

    // finished with the divide, so final result cleanup
    PowerDivideDone:
    ;

    accum->digitsCount = resultDigits;
    accum->numberExponent = calcExp;
    // copy to the output buffer
    memcpy(output, result - resultDigits, resultDigits);
    return output;
}
