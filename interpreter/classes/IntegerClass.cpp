/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                           IntegerClass.cpp     */
/*                                                                            */
/* Primitive Integer Class                                                    */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "Activity.hpp"
#include "Numerics.hpp"
#include "CompoundVariableTail.hpp"
#include "MethodArguments.hpp"
#include "NumberStringClass.hpp"



// singleton class instance
RexxIntegerClass *RexxInteger::classInstance = OREF_NULL;

RexxInteger *RexxInteger::falseObject = OREF_NULL;
RexxInteger *RexxInteger::trueObject = OREF_NULL;
RexxInteger *RexxInteger::nullPointer = OREF_NULL;

RexxInteger *RexxInteger::integerZero = OREF_NULL;
RexxInteger *RexxInteger::integerOne = OREF_NULL;
RexxInteger *RexxInteger::integerTwo = OREF_NULL;
RexxInteger *RexxInteger::integerThree = OREF_NULL;
RexxInteger *RexxInteger::integerFour = OREF_NULL;
RexxInteger *RexxInteger::integerFive = OREF_NULL;
RexxInteger *RexxInteger::integerSix = OREF_NULL;
RexxInteger *RexxInteger::integerSeven = OREF_NULL;
RexxInteger *RexxInteger::integerEight = OREF_NULL;
RexxInteger *RexxInteger::integerNine = OREF_NULL;
RexxInteger *RexxInteger::integerMinusOne = OREF_NULL;


/**
 * Get the primitive hash value of this String object.
 *
 * @return The calculated string hash for the string.
 */
HashCode RexxInteger::getHashValue()
{
    return stringValue()->getHashValue();
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInteger::live(size_t liveMark)
{
    memory_mark(objectVariables);
    memory_mark(stringrep);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInteger::liveGeneral(MarkReason reason)
{
    // if this integer is part of the image, force the
    // string rep and the numberstring version to be created.  This
    // avoids issues with old-to-new references with common objects.
    if (reason == PREPARINGIMAGE)
    {
        stringValue()->numberString();
    }

    memory_mark_general(objectVariables);
    memory_mark_general(stringrep);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInteger::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInteger)

    flattenRef(objectVariables);
    flattenRef(stringrep);

    cleanUpFlatten
}


/**
 * Handle a REQUEST('STRING') request for a REXX integer object
 *
 * @return The string value of the integer object.
 */
RexxString *RexxInteger::makeString()
{
    return stringValue();
}


/**
 * Override for the default object makearray method.
 *
 * @return The results of our string representation's makearray.
 */
ArrayClass *RexxInteger::makeArray()
{
  return stringValue()->makeArray();     // have the string value handle this
}


/**
 * Spoofed version of the standard hasMethod method.  This
 * will forward the request to the object's string representation.
 *
 * @param methodName The target name of the method.
 *
 * @return .true if the object has the method, .false otherwise.
 */
bool RexxInteger::hasMethod(RexxString *methodName)
{
                                       /* return the string value's answer  */
    return stringValue()->hasMethod(methodName);
}


/**
 * Low level processing for a REQUEST('STRING') request.
 *
 * @return The string representation for this object.
 */
RexxString *RexxInteger::primitiveMakeString()
{
    // if we've already created a string representation, just return it immediately
    if (stringrep != OREF_NULL)
    {
        return stringrep;
    }

    // convert this into an ASCII-Z string, then into a string object.
    char        stringBuffer[32];
    Numerics::formatWholeNumber(value, stringBuffer);

    // and make the object version of this
    RexxString *string = new_string(stringBuffer, strlen(stringBuffer));
    // save this for later requests.  If used once, we're likely to use it
    // again.
    setField(stringrep, string);
    setHasReferences();            // now have references that need marking
    return string;
}


/**
 * Return a string value for an integer object.
 *
 * @return The object string value.
 */
RexxString *RexxInteger::stringValue()
{
    // funnel to the common method.
    return primitiveMakeString();
}


/**
 * Copy the value of an integer into a compound variable name
 *
 * @param tail   The compound tail we're adding to.
 */
void RexxInteger::copyIntoTail(CompoundVariableTail *tail)
{
    // if we have a string already, just have it copy itself
    if (stringrep != OREF_NULL)
    {
        stringrep->copyIntoTail(tail);
        return;
    }
    // we will format as a string, but skip creating a string object
    char        stringBuffer[32];
    // convert value into ASCII-Z string and append to the buffer.
    Numerics::formatWholeNumber(value, stringBuffer);
    tail->append(stringBuffer, strlen(stringBuffer));
}


/**
 * Convert an integer into a number string.
 *
 * @return The number string version of the integer.
 */
NumberString *RexxInteger::numberString()
{
    // if we have a string representation, use its numberstring value to
    // all values remain in sync.
    if (stringrep != OREF_NULL)    /* have a cached string value?       */
    {
        return stringrep->numberString();
    }
    // create a numberstring version directly from the integer.
    else
    {
        return(NumberString *)new_numberstringFromWholenumber((wholenumber_t)value);
    }
}


/**
 * Convert an integer object into a double value.
 *
 * @param value  The returned value.
 *
 * @return true if this converted ok, false for any errors.  This always
 *         returns true for the Integer class.
 */
bool RexxInteger::doubleValue(double &result)
{
    // just let the compiler convert
    result = (double)wholeNumber();
    return true;
}


/**
 * Convert an integer object into a whole number value using the
 * default digits setting.
 *
 * @param result The returned result.
 *
 * @return true if the number converts ok under the current digits setting.  false
 *         for any conversion errors.
 */
bool RexxInteger::numberValue(wholenumber_t &result)
{
    // is the long value expressable as a whole number in REXX term.
    if (std::abs(value) > Numerics::MAX_WHOLENUMBER)
    {
        return false;                    // nope, not a valid long.
    }
    result = value;                      // return the value
    return true;                         // this was convertable
}


/**
 * Convert an integer object into a whole number value using the
 * current digits setting.
 *
 * @param result The returned result.
 * @param digits The digits setting to apply to the conversion.
 *
 * @return true if the number converts ok under the current digits setting.  false
 *         for any conversion errors.
 */
bool RexxInteger::numberValue(wholenumber_t &result, wholenumber_t digits)
{
    // is this expressable as a number under the current digits value?
    if (!Numerics::isValid(value, digits))
    {
        return false;                    // nope, not able to convert under this setting
    }
    result = value;                      // return the value
    return true;                         // this was convertable
}


/**
 * Convert an integer object into an unsigned whole number value
 * using the default digits setting.
 *
 * @param result The returned result.
 *
 * @return true if the number converts ok under the current digits setting.  false
 *         for any conversion errors.
 */
bool RexxInteger::unsignedNumberValue(size_t &result)
{
    // this must be non-negative and not out of range
    if (value < 0  || value > Numerics::MAX_WHOLENUMBER)
    {
        return false;
    }
    result = wholeNumber();              // return the value
    return true;                         // this was convertable
}


/**
 * Convert an integer object into an unsigned whole number value
 * using the current digits setting.
 *
 * @param result The returned result.
 * @param digits The digits setting to apply to the conversion.
 *
 * @return true if the number converts ok under the current digits setting.  false
 *         for any conversion errors.
 */
bool RexxInteger::unsignedNumberValue(size_t &result, wholenumber_t digits)
{
    // valid as a unsigned number in the current digits range?
    if (value < 0  || !Numerics::isValid(value, digits))
    {
        return false;
    }
    result = wholeNumber();              // return the value
    return true;                         // this was convertable
}


/**
 * Convert an integer to an integer (real easy!)
 *
 * @param digits The digits setting.
 *
 * @return The integer value of this integer...which is us.
 */
RexxInteger *RexxInteger::integerValue(wholenumber_t digits)
{
    return this;
}


/**
 * Set the integer string value.
 *
 * @param string The new string value.
 */
void RexxInteger::setString(RexxString *string )
{
    setField(stringrep, string);
    setHasReferences();
}


/**
 * Determine the truth value of an integer object
 *
 * @param errorcode The error code to issue.
 *
 * @return either true or false, based on the validation.
 */
bool RexxInteger::truthValue(RexxErrorCodes errorcode)
{
    if (value == 0)
    {
        return false;
    }
    else if (value != 1)
    {
        reportException(errorcode, this);
    }
    return true;
}


/**
 * Convert an object to a logical value without raising an
 * error.
 *
 * @param result The converted value.
 *
 * @return true if this converted ok, false for an invalid logical.
 */
bool RexxInteger::logicalValue(logical_t &result)
{
    if (value == 0)                /* have a zero?                      */
    {
        result = false;                  // this is false and the conversion worked
        return true;
    }
    else if (value == 1)           /* how about a one?                  */
    {
        result = true;                   // this is true and the conversion worked
        return true;
    }
    else
    {
        return false;                    // bad conversion
    }
}


/**
 * Macro to forward a method against the numberstring value of
 * an integer object.
 */
#define integer_forward(m,o) ((this)->numberString()->m(o))


/**
 * Process an unknown message condition on an object.  This is
 * an optimized bypass for the Object default method that can
 * bypass creating an array for the arguments and sending the
 * UNKNOWN message to the object.  Since many things funnel
 * through the integer unknown method, this is a big
 * optimization.
 *
 * @param messageName
 *                  The target message name.
 * @param arguments The message arguments.
 * @param count     The count of arguments.
 * @param result    The return result protected object.
 */
void RexxInteger::processUnknown(RexxErrorCodes error, RexxString *messageName, RexxObject **arguments, size_t count, ProtectedObject &result)
{
    // just send this as a message directly to the string object.
    stringValue()->messageSend(messageName, arguments, count, result);
}


/**
 * Wrapper around the compareTo() method that does string sort
 * comparisons.
 *
 * @param other  The other comparison object
 *
 * @return -1, 0, 1 depending on the comparison result.
 */
wholenumber_t RexxInteger::compareTo(RexxInternalObject *other )
{
    // just send this as a message directly to the string object.
    return stringValue()->compareTo(other);
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
MethodClass *RexxInteger::instanceMethod(RexxString  *method_name)
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
SupplierClass *RexxInteger::instanceMethods(RexxClass *class_object)
{
    return stringValue()->instanceMethods(class_object);
}


/**
 * Blank concatenation method for an integer object.
 *
 * @param other  The other concatenation value.
 *
 * @return
 */
RexxString *RexxInteger::concatBlank(RexxString *other)
{
    // concatenate with the string value
    return stringValue()->concatBlank(other);
}


/**
 * Concatenate an object to an Integer
 *
 * @param other  The other object for the concatenation.
 *
 * @return The concatenation result.
 */
RexxString *RexxInteger::concat(RexxString *other )
{
    return stringValue()->concatRexx(other);
}


/**
 * Add an integer to another object.
 *
 * @param other  The argument object.
 *
 * @return The addition result.
 */
RexxObject *RexxInteger::plus(RexxInteger *other)
{
    // if we want to do this with binary math, our operand must be valid
    // under the current numeric digits
    if (Numerics::isValid(value, number_digits()))
    {
        // if this is an prefix plus, we just return this object as the result
        if (other == OREF_NULL)
        {
            return this;
        }
        // binary operation
        else
        {
            // to calculate the sum with binary math, also the second operand
            // must be a RexxInteger that is valid under the current numeric digits
            if (isInteger(other) && Numerics::isValid(other->value, number_digits()))
            {
                // neither in the 32-bit, nor in the 64-bit case, will the sum
                // overflow wholenumber_t: for 32-bit, wholenumber_t accepts up to
                // 2^31-1 = 2147483647, which is more than twice as large as the
                // maximum of 999999999 for a RexxInteger
                // for the 64-bit case, wholenumber_t accepts up to
                // 2^63 -1 = 9223372036854775807 which is also more than twice as
                // large as   999999999999999999, the maximum for a RexxInteger
                wholenumber_t result = value + other->value;

                // though no wholenumber_t overflow is possible, the sum may
                // still be too large for a RexxInteger under current numeric digits
                if (Numerics::isValid(result, number_digits()))
                {
                    return new_integer(result);
                }
            }
        }
    }

    // we will have to forward to NumberString::plus
    return integer_forward(plus, other);
}


/**
 * Subtract another object from this integer.
 *
 * @param other  The other object.
 *
 * @return The subtraction or unary minus result.
 */
RexxObject *RexxInteger::minus(RexxInteger *other)
{
    // if we want to do this with binary math, our operand must be valid
    // under the current numeric digits
    if (Numerics::isValid(value, number_digits()))
    {
        // if this is an prefix minus, we just return the negated value
        if (other == OREF_NULL)
        {
            return new_integer(-value);
        }
        // binary operation
        else
        {
            // to calculate the difference with binary math, also the second operand
            // must be a RexxInteger that is valid under the current numeric digits
            if (isInteger(other) && Numerics::isValid(other->value, number_digits()))
            {
                // neither in the 32-bit, nor in the 64-bit case, will the difference
                // overflow wholenumber_t: for 32-bit, wholenumber_t accepts up to
                // 2^31-1 = 2147483647, which is more than twice as large as the
                // maximum of 999999999 for a RexxInteger
                // for the 64-bit case, wholenumber_t accepts up to
                // 2^63 -1 = 9223372036854775807 which is also more than twice as
                // large as   999999999999999999, the maximum for a RexxInteger
                wholenumber_t result = value - other->value;

                // though no wholenumber_t overflow is possible, the difference may
                // still be too large for a RexxInteger under current numeric digits
                if (Numerics::isValid(result, number_digits()))
                {
                    return new_integer(result);
                }
            }
        }
    }

    // we will have to forward to NumberString::minus
    return integer_forward(minus, other);
}


/**
 * Multiply an integer by another object.
 *
 * @param other  The other object.
 *
 * @return The multiplication result.
 */
RexxObject *RexxInteger::multiply(RexxInteger *other)
{
    // we'll try to multiply with binary math if both factors
    // are RexxIntegers that are valid under the current numeric digits
    if (Numerics::isValid(value, number_digits()) &&
        other != OREF_NULL && isInteger(other))
    {
        wholenumber_t multiplier = other->getValue();
        wholenumber_t result;
        if (Numerics::isValid(multiplier, number_digits()))
        {
            // check if we can cut this short
            switch(multiplier)
            {
                case 0:
                    // n * 0 is 0, always
                    return IntegerZero;
                case 1:
                    // n * 1 is n, always
                    return this;
                case -1:
                    // we just negate
                    return new_integer(-value);
                case -2: case 2:
                    // we'll evaluate n * 2 by bit shifting
                    // neither in the 32-bit, nor in the 64-bit case, will the shift
                    // overflow wholenumber_t: for 32-bit, wholenumber_t accepts up to
                    // 2^31-1 = 2147483647, which is more than twice as large as the
                    // maximum of 999999999 for a RexxInteger
                    // for the 64-bit case, wholenumber_t accepts up to
                    // 2^63 -1 = 9223372036854775807 which is also more than twice as
                    // large as   999999999999999999, the maximum for a RexxInteger
                    result = value << 1;

                    // though no wholenumber_t overflow is possible, the shift may
                    // still be too large for a RexxInteger under current numeric digits
                    if (Numerics::isValid(result, number_digits()))
                    {
                        // for a multiplier of -2, result will be negative
                        return new_integer(multiplier == -2 ? -result : result);
                    }
                    break;
            }

            // the product should be a valid integer under the current numeric
            // digits; if we know it won't fit, there's no need to multiply
            // we can estimate this: multiplying an m-bit number with an n-bit
            // number yields a product of either (m + n - 1) or (m + n) bits
            // we test (m + n - 1) <= 30 (for 32-bit) and 60 (for 64-bit),
            // which means (m + n) <= 31 (32-bit) and <= 61 (64-bit), which in
            // turn makes sure there will be no wholenumber_t overflow
            if (length_in_bits(value) + length_in_bits(multiplier) - 1 <= Numerics::maxBitsForDigits(number_digits()))
            {
                result = value * multiplier;
                // the result may still be slightly too large; need to check
                if (Numerics::isValid(result, number_digits()))
                {
                    return new_integer(result);
                }
            }
        }
    }

    // we will have to forward to NumberString::multiply
    return integer_forward(multiply, other);
}


/**
 * Divide an integer object by another object.
 *
 * @param other  The other object.
 *
 * @return The divide result
 */
RexxObject *RexxInteger::divide(RexxInteger *other)
{
    // we'll try to divide with binary math if both dividend and divisor
    // are RexxIntegers that are valid under the current numeric digits
    if (Numerics::isValid(value, number_digits()) &&
        other != OREF_NULL && isInteger(other))
    {
        wholenumber_t divisor = other->getValue();
        if (Numerics::isValid(divisor, number_digits()))
        {
            // this may be an integer division, which we can do in binary math
            // if so, there's no need to check for the size of the resulting quotient
            switch(divisor)
            {
                case 0:
                    break; // let NumberString::divide handle this
                case -1:
                    return new_integer(-value);
                case 1:
                    return this;
                case -2:
                case 2:
                    // if even, dividing by 2 (or -2) is easy
                    if (!(value & 1))
                    {
                        return new_integer(value / divisor);
                    }
                    break;
                case -4:
                case 4:
                    // if multiple of four, dividing is easy
                    if (!(value & 3))
                    {
                        return new_integer(value / divisor);
                    }
                    break;
                default:
                    // generally, if there's no remainder, we can divide here
                    if (value % divisor == 0)
                    {
                        return new_integer(value / divisor);
                    }
                    break;
            }
        }
    }

    // else we will have to forward to NumberString::divide
    return integer_forward(divide, other);
}


/**
 * Perform an integer division operation.
 *
 * @param other  The other value for the divide.
 *
 * @return The division result
 */
RexxObject *RexxInteger::integerDivide(RexxInteger *other)
{
    // we'll try to divide with binary math if both dividend and divisor
    // are RexxIntegers that are valid under the current numeric digits
    if (Numerics::isValid(value, number_digits()) &&
        other != OREF_NULL && isInteger(other))
    {
        wholenumber_t divisor = other->getValue();
        if (Numerics::isValid(divisor, number_digits()))
        {
            // no need to check for the size of the resulting quotient
            // let NumberString:integerDivide handle any divide-by-zero
            if (divisor != 0)
            {
                return new_integer(value / divisor);
            }
        }
    }

    // else we will have to forward to NumberString:integerDivide
    return integer_forward(integerDivide, other);
}


/**
 * Integer object remainder operation.
 *
 * @param other  The divider object.
 *
 * @return The remainder result.
 */
RexxObject *RexxInteger::remainder(RexxInteger *other)
{
    // we'll try to calculate the remainder with binary math if both operands
    // are RexxIntegers that are valid under the current numeric digits
    if (Numerics::isValid(value, number_digits()) &&
        other != OREF_NULL && isInteger(other))
    {
        wholenumber_t divisor = other->getValue();
        if (Numerics::isValid(divisor, number_digits()))
        {
            // no need to check for the size of the result
            switch(divisor)
            {
                case 0:
                    break; // let NumberString::remainder handle this
                case -1:
                case 1:
                    return IntegerZero;
                case -2:
                case 2:
                    // if odd, remainder is +/-1, otherwise 0
                    // the sign of the remainder is the sign of the dividend
                    return (value & 1) ? (value < 0 ? IntegerMinusOne : IntegerOne) : IntegerZero;
                default:
                    return new_integer(value % divisor);
            }
        }
    }

    // else we will have to forward to NumberString::remainder
    return integer_forward(remainder, other);
}


/**
 * Integer object modulo operation.
 * There is no universal agreement how to calculate modulo for
 * floating point arguments or for negative divisors, so we restrict
 * the dividend to a whole number and the divisor to a positive whole number.
 *
 * @param other  The divisor object.
 *
 * @return The module result.
 */
RexxObject *RexxInteger::modulo(RexxInteger *other)
{
    // we'll try to calculate the module with binary math if both operands
    // are RexxIntegers that are valid under the current numeric digits
    if (Numerics::isValid(value, number_digits()) &&
        other != OREF_NULL && isInteger(other))
    {
        wholenumber_t divisor = other->getValue();
        // our divisor must be a positive whole number
        if (Numerics::isValid(divisor, number_digits()) && divisor >= 1)
        {
            // no need to check for the size of the result
            switch(divisor)
            {
                case 1:
                    // a mod 1 is zero, always
                    return IntegerZero;
                case 2:
                    // if odd, the module is 1, otherwise 0
                    return (value & 1) ? IntegerOne : IntegerZero;
                default:
                    wholenumber_t module = value % divisor;
                    return module >= 0 ? new_integer(module) : new_integer(divisor + module);
            }
        }
    }

    // else we will have to forward to NumberString::modulo
    return integer_forward(modulo, other);
}


/**
 * Integer power operation.
 *
 * @param other  The power exponent
 *
 * @return the exponent result.
 */
RexxObject *RexxInteger::power(RexxObject *other)
{
    // we'll try to do this with binary math if both base and power
    // are RexxIntegers that are valid under the current numeric digits
    if (Numerics::isValid(value, number_digits()) &&
        other != OREF_NULL && isInteger(other))
    {
        wholenumber_t power = ((RexxInteger *)other)->getValue();
        if (Numerics::isValid(power, number_digits()))
        {
            // handle some common bases
            switch (value)
            {
                case 0:
                    // base 0 returns integers for positive powers only
                    // 0 ^ n is 0 for n >= 1
                    if (power >= 1)
                    {
                      return integerZero;
                    }
                    // handle power = 0 case and other powers later
                    break;

                case 1:
                    // base 1 returns integers with both positive and negative powers
                    // 1 ^ n is 1, always
                    return integerOne;

                case -1:
                    // base -1 returns integers with both positive and negative powers
                    // (-1) ^ n is -1 if n is odd, and +1 if n is even
                    return (power & 1) ? integerMinusOne : integerOne;

                case 2: case -2:
                    // bases 2 and -2 return integers for positive powers only
                    // we'll evaluate 2 ^ n by bit shifting
                    // the number of bits to shift left must be less than the
                    // maximum bits allowed under the current numeric digits
                    // 2 ** 59 = 576460752303423488 is maximum for 64-bits
                    // 2 ** 29 = 536870912 is maximum for 32-bits
                    // base -2 results will be negative if power is odd
                    if (power >= 0 && power < Numerics::maxBitsForDigits(number_digits()))
                    {
                        // no wholenumber_t overflow possible
                        wholenumber_t result = (wholenumber_t)1 << power;

                        // base -2 results will be negative if power is odd
                        return (value == -2 && power & 1) ? new_integer(-result) : new_integer(result);
                    }
                    // can't use RexxInteger, have to fall back to NumberString
                    return integer_forward(power, other);

                case 4: case -4:
                case 8: case -8:
                case 16: case -16:
                case 32: case -32:
                case 64: case -64:
                case 128: case -128:
                case 256: case -256:
                    // similar to above base 2 case
                    wholenumber_t baseFactor;
                    baseFactor = length_in_bits(value) - 1;
                    if (power >= 0 && power * baseFactor < Numerics::maxBitsForDigits(number_digits()))
                    {
                        // no wholenumber_t overflow possible
                        wholenumber_t result = (wholenumber_t)1 << (power * baseFactor);

                        // result for negative bases will be negative if power is odd
                        return (value < 0 && power & 1) ? new_integer(-result) : new_integer(result);
                    }
                    // can't use RexxInteger, have to fall back to NumberString
                    return integer_forward(power, other);

                case 10:
                case -10:
                    // bases 10 and -10 return integers for positive powers only
                    // we'll evaluate 10 ^ n by lookup (n less than REXXINTEGER_DIGITS)
                    // to keep the result valid under the current numeric digits,
                    // n must be also less than the current numeric digits
                    if (power >= 0 &&
                        power < Numerics::REXXINTEGER_DIGITS &&
                        power < number_digits())
                    {
                        // we abuse our table of maximum values for specified digits
                        // e. g. 10^9 = 999 999 999 (maximum for numeric digits 9) plus 1
                        wholenumber_t result = Numerics::maxValueForDigits(power) + 1;
                        // base -10 results will be negative if power is odd
                        return (value == -10 && power & 1) ? new_integer(-result) : new_integer(result);
                    }
                    // can't use RexxInteger, have to fall back to NumberString
                    return integer_forward(power, other);
            }

            // handle some common powers
            switch (power)
            {
                case 0:
                    // n ^ 0 is 1, even 0 ^ 0 is defined as 1 in ooRexx
                    return integerOne;

                case 1:
                    // n ^ 1 is n, of course
                    return this;

                case 2:
                    // we'll evaluate n ^ 2 as n * n
                    // the result should be a valid integer under the current numeric
                    // digits; if we know it won't fit, there's no need to multiply
                    // we can estimate this: squaring an n-bit number yields a result
                    // of either (2n - 1) or (2n) bits
                    // we test (2n - 1) <= 30 (for 32-bit) and 60 (for 64-bit),
                    // which means (2n) <= 31 (32-bit) and <= 61 (64-bit), which in
                    // turn makes sure there will be no wholenumber_t overflow
                    if (length_in_bits(value) * 2 - 1 <= Numerics::maxBitsForDigits(number_digits()))
                    {
                        wholenumber_t result;
                        result = value * value;
                        // the result may still be slightly too large; need to check
                        if (Numerics::isValid(result, number_digits()))
                        {
                            return new_integer(result);
                        }
                    }
                    // we cannot use NumberString::multiply to finish this
                    //     return integer_forward(multiply, this);
                    // because "*" and "**" behave differently as documented
                    // "**" will remove trailing zeros as if divided by 1
                    // "*" will keep any trailing zeros
                    // there's a test case in EXPONENT.testGroup which shows this:
                    //     ::method "test_25"
                    //       numeric digits 18
                    //       self~assertSame(12345678900000 ** 2, 1.5241578750190521E+26)
                    //     Expected: [[1.52415787501905210E+26]
                    //     Actual:   [[1.5241578750190521E+26],
                    // another example to reproduce this issue is
                    //    a = 123000; say a ** 2 a * a -> 1.5129E+10 1.51290000E+10
                    // we instead forward to NumberString::power
                    return integer_forward(power, other);
            }

            // at this point, with bases 0, 1, 2, and powers 0, 1, 2 checked,
            // we know that (no matter what the current numeric digits setting is),
            // a RexxInteger cannot handle
            // - negative powers,
            // - any power larger than 18 (32-bit) or 37 (64-bit), and
            // - any base larger than +/-999 (32-bit) or +/-999999 (64-bit)
            if (power < 0 || power > RexxIntegerMaxPower ||
                std::abs(value) > RexxIntegerMaxBase)
            {
                return integer_forward(power, other);
            }

            // no common base or power .. try to do the full calculation
            // the result should be a valid integer under the current numeric digits
            // if we know it won't fit, there's no need to try
            // we can estimate the result size: if base has b bits, the result
            // will require between (b * power - b + 1) and (b * power) bits
            wholenumber_t maxBits = Numerics::maxBitsForDigits(number_digits());
            if ((length_in_bits(value) - 1) * power + 1 <= maxBits)
            {
                wholenumber_t base = value;
                wholenumber_t result = 1;
                bool overflow = 0;

                // https://en.wikipedia.org/wiki/Exponentiation_by_squaring#Basic_method
                while (power > 1)
                {
                    if (power & 1)
                    {
                        // make sure that result * base will not overflow
                        if (length_in_bits(result) + length_in_bits(base) - 1 > maxBits)
                        {
                            overflow = 1;
                            break;
                        }
                        result *= base;
                        power = (power - 1) / 2;
                    }
                    else
                    {
                        power /= 2;
                    }
                    // make sure that base * base will not overflow
                    if (length_in_bits(base) * 2 - 1 > maxBits)
                    {
                        overflow = 1;
                        break;
                    }
                    base *= base;
                }
                // check that we didn't break out of the loop due to overflow
                if (!overflow &&
                    // and make sure that result * base will not overflow
                    length_in_bits(result) + length_in_bits(base) - 1 <= maxBits)
                {
                    result *= base;
                    // our result may still be slightly too large; need to check
                    if (Numerics::isValid(result, number_digits()))
                    {
                        return new_integer(result);
                    }
                }
            }
        }
    }

    // else we fall back using NumberString's power()
    return integer_forward(power, other);
}


/**
 * Primitive strict equal\not equal method.  This determines
 * only strict equality, not greater or less than values.
 *
 * @param other  the other object.
 *
 * @return The comparison result
 */
bool RexxInteger::isEqual(RexxInternalObject *other)
{
    // primitive version, no argument checking here

    // compare directly if we have two integers
    if (isInteger(other))
    {
        return value == ((RexxInteger *)other)->value;
    }

    // do a string compare
    return stringValue()->isEqual(other);
}


/**
 * Compare the two values.
 *
 * return <0 if other is greater than this
 * return  0 if this equals other
 * return >0 if this is greater than other
 *
 * @param other  The other object for the comparison.
 *
 * @return The comparison result.
 */
wholenumber_t RexxInteger::strictComp(RexxObject *other)
{
    // though ostensibly a low level call, this is used from the
    // comparison operators, so the argument checking is done here.
    requiredArgument(other, ARG_ONE);
    // this must always be done using the string version, since values of
    // different decimal lengths need to use string rules.  Thus
    // 12<<2 is true because string rules are used
    return stringValue()->primitiveStrictComp(other);
}


/**
 * Compare the two values, testing just for strict equality
 *
 * @param other  The other object for the comparison.
 *
 * @return The comparison result.
 */
bool RexxInteger::strictEquality(RexxObject *other)
{
    // though ostensibly a low level call, this is used from the
    // comparison operators, so the argument checking is done here.
    requiredArgument(other, ARG_ONE);
    // if two integers, this is easy to do.
    if (isInteger(other))
    {
        return value == ((RexxInteger *)other)->value;
    }
    // string comparison
    else
    {
        return stringValue()->primitiveIsEqual(other);
    }
}


/**
 * Do a comparison operation between an Integer object and
 * another object.
 *
 * @param other  The other object.
 *
 * @return <0, 0, or >0 to give appropriate ordering
 */
wholenumber_t RexxInteger::comp(RexxObject *other)
{
    // also used from multiple arguments
    requiredArgument(other, ARG_ONE);

    // if we want to do a binary compare, both operands must be valid
    // under the current numeric digits, and FUZZ must be zero
    if (Numerics::isValid(value, number_digits()) &&
        isInteger(other) &&
        Numerics::isValid(((RexxInteger *)other)->value, number_digits()) &&
        number_fuzz() == 0)
    {
        // the difference of two RexxIntegers may overflow a RexxInteger, but no
        // wholenumber_t overflow will occur - see dicussion at RexxInteger::minus
        // as the comp() result will just be used for further binary comparisons
        // (<0, >0, =0, etc.) and not be converted into a RexxInteger, that's ok
        return value - ((RexxInteger *)other)->value;
    }
    else
    {
        return numberString()->comp(other, number_fuzz());
    }
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
    HashCode hashVal = hash();
    return new_string((char *)&hashVal, sizeof(HashCode));
}


/**
 * Perform the primitive level "==" compare, including the hash
 * value processing.
 *
 * @param other  The other comparison result.
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::strictEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictEquality(other));
}


RexxObject *RexxInteger::strictNotEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheTrueObject;
    }
    return booleanObject(!strictEquality(other));
}


/**
 * A compare operation
 *
 * @param other  The other comparison object
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::equal(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other) == 0);
}


/**
 * A compare operation
 *
 * @param other  The other comparison object
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::notEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheTrueObject;
    }
    return booleanObject(comp(other) != 0);
}


/**
 * A compare operation
 *
 * @param other  The other comparison object
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::isGreaterThan(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other) > 0);
}


/**
 * A compare operation
 *
 * @param other  The other comparison object
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::isLessThan(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other) < 0);
}


/**
 * A compare operation
 *
 * @param other  The other comparison object
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::isGreaterOrEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other) >= 0);
}


/**
 * A compare operation
 *
 * @param other  The other comparison object
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::isLessOrEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(comp(other) <= 0);
}


/**
 * A compare operation
 *
 * @param other  The other comparison object
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::strictGreaterThan(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictComp(other) > 0);
}


/**
 * A compare operation
 *
 * @param other  The other comparison object
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::strictLessThan(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictComp(other) < 0);
}


/**
 * A compare operation
 *
 * @param other  The other comparison object
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::strictGreaterOrEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictComp(other) >= 0);
}


/**
 * A compare operation
 *
 * @param other  The other comparison object
 *
 * @return .true or .false
 */
RexxObject *RexxInteger::strictLessOrEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheFalseObject;
    }
    return booleanObject(strictComp(other) <= 0);
}


/**
 * Perform the logical not of an integer object
 *
 * @return The logical not on the object truth value.
 */
RexxObject *RexxInteger::notOp()
{
    return booleanObject(truthValue(Error_Logical_value_method));
}


/**
 * Perform the logical not of an integer object
 *
 * @return The logical not on the object truth value.
 */
RexxObject *RexxInteger::operatorNot(RexxObject *dummy)
{
    return booleanObject(!truthValue(Error_Logical_value_method));
}


/**
 * Logically AND two objects together
 *
 * @param other  The other object for the AND
 *
 * @return The logical result.
 */
RexxObject *RexxInteger::andOp(RexxObject *other)
{
    requiredArgument(other, ARG_ONE);
    // there's no short cutting in Rexx, so we need  to validate all arguments.
    RexxObject *otherTruth = booleanObject(other->truthValue(Error_Logical_value_method));
    return (!truthValue(Error_Logical_value_method)) ? TheFalseObject : otherTruth;
}


/**
 * Logically OR two objects together
 *
 * @param other  The OR operand
 *
 * @return The logical result.
 */
RexxObject *RexxInteger::orOp(RexxObject *other)
{
    requiredArgument(other, ARG_ONE);
    RexxObject *otherTruth = booleanObject(other->truthValue(Error_Logical_value_method));
    return truthValue(Error_Logical_value_method) ? TheTrueObject : otherTruth;
}


/**
 * Logically XOR two objects together
 *
 * @param other  The XOR operand
 *
 * @return The logical result.
 */
RexxObject *RexxInteger::xorOp(RexxObject *other)
{
    requiredArgument(other, ARG_ONE);
    bool truth = other->truthValue(Error_Logical_value_method);

    if (!truthValue(Error_Logical_value_method))
    {
        return booleanObject(truth);
    }
    else
    {
        return booleanObject(!truth);
    }
}


/**
 * Implement an expression choice method.  The target object
 * is evaluated as a logical value, and if it evaluates to true,
 * returns the first argument as a result.  If it evaluates to
 * false, it returns the second argument.
 *
 * @param trueResult The value to return for a true result.
 * @param falseResult
 *                   The falue to return for a false result
 *
 * @return Either the true value or the false value based on the value of the target object.
 */
RexxObject *RexxInteger::choiceRexx(RexxObject *trueResult, RexxObject *falseResult)
{
    requiredArgument(trueResult, "true value");
    requiredArgument(falseResult, "false value");

    return truthValue(Error_Logical_value_method) ? trueResult : falseResult;
}


/**
 * Take the absolute value of an integer object
 *
 * @return The abs() result.
 */
RexxObject *RexxInteger::abs()
{
    // we need to check if we have a value that's valid under the
    // current numeric digits
    if (Numerics::isValid(value, number_digits()))
    {
        // if we're already positive, this is a quick return
        if (value >= 0)
        {
            return this;
        }
        // negate and return as a new integer
        return new_integer(-value);
    }
    // else forward to NumberString
    return numberString()->abs();
}


/**
 * SIGN() function on an integer object
 *
 * @return One, zero, or minus one.
 */
RexxObject *RexxInteger::sign()
{
    if (value > 0)
    {
        return IntegerOne;
    }
    else if (value < 0)
    {
        return IntegerMinusOne;
    }
    else
    {
        return IntegerZero;
    }
}


/**
 * Perform MAX function on integer objects
 *
 * @param args     The array of arguments
 * @param argCount The count of arguments
 *
 * @return The largest of the numbers.
 */
RexxObject *RexxInteger::Max(RexxObject **args, size_t argCount)
{
    // we must have a RexxInteger valid under the current numeric digits
    if (!Numerics::isValid(value, number_digits()))
    {
        return numberString()->Max(args, argCount);
    }

    // if nothing to compare against, we can return this object immediately
    // NOTE: we cannot move this before the digits test because the restricted digits
    // might require reformatting into exponential form or even rounding.
    if (argCount < 1)
    {
        return this;
    }

    // we can try this here as long as all of the numbers really are integers
    wholenumber_t maxValue = value;
    RexxObject *maxObject = this;

    // now check all of the numbers in turn
    for (size_t arg = 0; arg < argCount; arg++)
    {
        RexxObject *argument = args[arg];
        requiredArgument(argument, arg);

        // if this is an integer object, we can continue doing this
        if (isInteger(argument))
        {
            wholenumber_t v = ((RexxInteger *)argument)->getValue();
            // remember the larger value and the corresponding object
            if (v > maxValue)
            {
                maxValue = v;
                maxObject = argument;
            }
        }
        // not all integers, so have numberstring figure this out
        else
        {
            return numberString()->Max(args, argCount);
        }
    }
    // return the maximum object
    return maxObject;
}


/**
 * Perform MIN function on integer objects
 *
 * @param args     The array of arguments
 * @param argCount The count of arguments
 *
 * @return The smallest of the numbers.
 */
RexxObject *RexxInteger::Min(RexxObject **args, size_t argCount)
{
    // if nothing to compare against, we can return this object immediately
    // NOTE: we've intentionally moved this before the digits test because
    // it gives us the ability to distinguish between RexxInteger and a
    // NumberString from within ooRexx (see e. g. RexxInteger.testGroup)
    // "n~min" is a very unlikely expression, and it is extremely unlikely
    // that this will show up as a bug in real code
    if (argCount < 1)
    {
        return this;
    }

    // we must have a RexxInteger valid under the current numeric digits
    if (!Numerics::isValid(value, number_digits()))
    {
        return numberString()->Min(args, argCount);
    }

    // we can try this here as long as all of the numbers really are integers
    wholenumber_t minValue = value;
    RexxObject *minObject = this;

    // now check all of the numbers in turn
    for (size_t arg = 0; arg < argCount; arg++)
    {
        RexxObject *argument = args[arg];
        requiredArgument(argument, arg);

        // if this is an integer object, we can continue doing this
        if (isInteger(argument))
        {
            wholenumber_t v = ((RexxInteger *)argument)->getValue();
            // remember the smaller value and the corresponding object
            if (v < minValue)
            {
                minValue = v;
                minObject = argument;
            }
        }
        // not all integers, so have numberstring figure this out
        else
        {
            return numberString()->Min(args, argCount);
        }
    }
    // return the minimum object
    return minObject;
}


/**
 * Act as a front end for the actual TRUNC REXX method
 *
 * @param decimals The number of decimals to truncate to.
 *
 * @return The truncation result.
 */
RexxObject *RexxInteger::trunc(RexxObject *decimals)
{
    // if decimals are omitted or zero, this is the same value
    // we also need to check if we have a value that's valid under the
    // current numeric digits
    if ((decimals == OREF_NULL ||
       (isInteger(decimals) && ((RexxInteger *)decimals)->getValue() == 0)) &&
       Numerics::isValid(value, number_digits()))
    {
      return this;
    }
    // else forward to NumberString
    return numberString()->trunc(decimals);
}


/**
 * Calculate the floor value for a numeric value
 *
 * @return The floor value.
 */
RexxObject *RexxInteger::floor()
{
    // the floor of an integer is always the same value
    // we still need to check if we have a value that's valid under the
    // current numeric digits
    if (Numerics::isValid(value, number_digits()))
    {
      return this;
    }
    // else forward to NumberString
    return numberString()->floor();
}


/**
 * Calculate the ceiling value for a numeric value
 *
 * @return The ceiling value.
 */
RexxObject *RexxInteger::ceiling()
{
    // the ceiling of an integer is always the same value
    // we still need to check if we have a value that's valid under the
    // current numeric digits
    if (Numerics::isValid(value, number_digits()))
    {
      return this;
    }
    // else forward to NumberString
    return numberString()->ceiling();
}


/**
 * Calculate the round value for a numeric value
 *
 * @return The round value.
 */
RexxObject *RexxInteger::round()
{
    // the rounding of an integer is always the same value
    // we still need to check if we have a value that's valid under the
    // current numeric digits
    if (Numerics::isValid(value, number_digits()))
    {
      return this;
    }
    // else forward to NumberString
    return numberString()->round();
}

/**
 * Act as a front end for the actual FORMAT REXX method
 *
 * @param integers   The number of integers requested
 * @param decimals   The number decimals to include
 * @param mathExp    The mathexp value.
 * @param expTrigger The exptrigger result.
 *
 * @return The formatted number.
 */
RexxObject *RexxInteger::format(RexxObject *integers, RexxObject *decimals, RexxObject *mathExp, RexxObject *expTrigger)
{
    // just forward to the numberstring version
    return numberString()->formatRexx(integers, decimals, mathExp, expTrigger);
}


/**
 * Convert a decimal number string into a character string
 *
 * @param length The target length for the converted string.
 *
 * @return The converted string value.
 */
RexxObject *RexxInteger::d2c(RexxInteger *lengthObject)
{
    // we'll try the conversion with binary math if both the value and
    // (if specified) the length argument are RexxIntegers that are valid
    // under the current numeric digits; in addition we require either:
    //     value >= 0, length omitted, or
    //     any value, length specified and > 0
    // we'll forward anything else to NumberString to deal with
    if (Numerics::isValid(value, number_digits()) &&
       ((value >= 0 && lengthObject == OREF_NULL) ||
        (lengthObject != OREF_NULL && isInteger(lengthObject) && lengthObject->getValue() > 0)))
    {
        wholenumber_t length;
        if (lengthObject == OREF_NULL)
        {
            // we know that value >= 0
            // calculate length = number of characters, e.g.
            // value = 1, length_in_bits() = 1, length = 1
            // value = 256, length_in_bits() = 9, length = 2
            // value = 65535, length_in_bits() = 16, length = 3
            length = (length_in_bits(value) + 7) / 8;
        }
        else
        {
            length = lengthObject->getValue();
        }

        // get a result string large enough to fit our length
        RexxString *result = raw_string(length);

        // add hex nibbles from right to left
        RexxString::StringBuilderRtL builder(result);

        // we must not change our value instance variable .. copy it
        wholenumber_t val = value;

        // looping for length will both automatically
        // - take care of leading '00'x or 'FF'x characters, and
        // - left-truncate for short lengths
        while (length--)
        {
            builder.put((unsigned char)(val & 0xFF));
            val >>= 8;
        }
        return result;
    }
    // else we will have to forward to NumberString
    return numberString()->d2xD2c(lengthObject, true);
}


/**
 * Convert a decimal number string into a hexadecimal string
 *
 * @param length The length for the conversion.
 *
 * @return The converted string.
 */
RexxObject *RexxInteger::d2x(RexxInteger *lengthObject)
{
    // we'll try the conversion with binary math if both the value and
    // (if specified) the length argument are RexxIntegers that are valid
    // under the current numeric digits; in addition we require either:
    //     value >= 0, length omitted, or
    //     any value, length specified and > 0
    // we'll forward anything else to NumberString to deal with
    if (Numerics::isValid(value, number_digits()) &&
       ((value >= 0 && lengthObject == OREF_NULL) ||
        (lengthObject != OREF_NULL && isInteger(lengthObject) && lengthObject->getValue() > 0)))
    {
        wholenumber_t length;
        if (lengthObject == OREF_NULL)
        {
            // we know that value >= 0
            // calculate length = number of hex nibbles, e.g.
            // value = 1, length_in_bits() = 1, length = 1
            // value = 256, length_in_bits() = 9, length = 3
            // value = 65535, length_in_bits() = 16, length = 4
            length = (length_in_bits(value) + 3) / 4;
        }
        else
        {
            length = lengthObject->getValue();
        }

        // if length is 1, and value between 0..9, just return this object
        if (length == 1 && value >= 0 && value <= 9)
        {
            return this;
        }

        // get a result string large enough to fit our length
        RexxString *result = raw_string(length);

        // add hex nibbles from right to left
        RexxString::StringBuilderRtL builder(result);

        // we must not change our value instance variable .. copy it
        wholenumber_t val = value;

        // looping for length will both automatically
        // - take care of leading '0' or 'F' characters, and
        // - left-truncate for short lengths
        while (length--)
        {
            builder.put(RexxString::intToHexDigit(val & 0xF));
            val >>= 4;
        }
        return result;
    }
    // else we will have to forward to NumberString
    return numberString()->d2xD2c(lengthObject, false);
}


/**
 * Perform expression evaluation for an integer term.
 *
 * @param context The current exection context.
 * @param stack   The current evaluation stack.
 *
 * @return The evaluation result.
 */
RexxObject  *RexxInteger::evaluate(RexxActivation *context, ExpressionStack *stack )
{
    // evaluate always leaves results on the stack
    stack->push(this);
    context->traceIntermediate(this, RexxActivation::TRACE_PREFIX_LITERAL);
    // this is a literal result.
    return this;
}


/**
 * Polymorphic getValue function used with expression terms
 *
 * @param context The execution context.
 *
 * @return The term result (which is this object).
 */
RexxObject  *RexxInteger::getValue(RexxActivation *context)
{
    // no stack, no trace, just return the value.
    return this;
}


/**
 * Polymorphic getValue function used with expression terms
 *
 * @param context The variable dictionary used to resolve
 *                variables.
 *
 * @return The term result (which is this object).
 */
RexxObject  *RexxInteger::getValue(VariableDictionary *context)
{
    // again, this is a literal result.
    return this;
}




/**
 * Polymorphic getRealValue function used with expression terms
 *
 * @param context The execution context.
 *
 * @return The term result (which is this object).
 */
RexxObject *RexxInteger::getRealValue(RexxActivation *context)
{
    return this;
}


/**
 * Polymorphic getRealValue function used with expression terms
 *
 * @param context The variable dictionary used to resolve
 *                variables.
 *
 * @return The term result (which is this object).
 */
RexxObject  *RexxInteger::getRealValue(VariableDictionary *context)
{
    return this;
}


// Integer Class class methods start here.


/**
 * This method will pre-allocate 111 integer objects, -10 .. 100.  These will then
 * be used whenever a request for an integer between -10 and 100 is requested
 * this should help reduce some of our memory requirements and trips through
 * memory_new.
 */
void RexxIntegerClass::initCache()
{
    for (int i = IntegerCacheLow; i <= IntegerCacheHigh; i++)
    {
        integercache[i - IntegerCacheLow] = new  RexxInteger (i);
        // force the item to create its string value too.  This can save
        // us a lot of time when string indices are used for compound
        // variables and also eliminate a bunch of old-new table
        // references.

        // because the numberstring value is required for operations, we
        // also generate that as well to avoid the oldspace/newspace operations.
        integercache[i - IntegerCacheLow]->stringValue()->numberString();
    }
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxIntegerClass::live(size_t liveMark)
{
    RexxClass::live(liveMark);     // do RexxClass level marking

    // mark the cache array
    for (int i = IntegerCacheLow; i <= IntegerCacheHigh; i++)
    {
        memory_mark(integercache[i - IntegerCacheLow]);
    }
}

void RexxIntegerClass::liveGeneral(MarkReason reason)
{
    RexxClass::liveGeneral(reason);// do RexxClass level marking

    // mark the cache array
    for (int i = IntegerCacheLow; i <= IntegerCacheHigh; i++)
    {
        memory_mark_general(integercache[i - IntegerCacheLow]);
    }
}


/**
 * An override for retrieving the class object.  We
 * lie and pretend we're the string class.
 *
 * @return The associated class object.
 */
RexxClass *RexxInteger::classObject()
{
    return TheStringClass;
}


/**
 * Create a new integer object
 *
 * @param size   The size of the object.
 *
 * @return The storage for a new integer object.
 */
void *RexxInteger::operator new(size_t size)
{
    RexxInternalObject *newObject = new_object(size, T_Integer);
    // initially, no references.
    newObject->setHasNoReferences();
    return newObject;
}


/**
 * Create the integer class and set up the integer cache
 */
void RexxInteger::createInstance()
{
    CLASS_CREATE_SPECIAL(Integer, "String", RexxIntegerClass);
    TheIntegerClass->initCache();
}

// table of operator methods for quick expression dispatch
PCPPM RexxInteger::operatorMethods[] =
{
   NULL,
   (PCPPM)&RexxInteger::plus,
   (PCPPM)&RexxInteger::minus,
   (PCPPM)&RexxInteger::multiply,
   (PCPPM)&RexxInteger::divide,
   (PCPPM)&RexxInteger::integerDivide,
   (PCPPM)&RexxInteger::remainder,
   (PCPPM)&RexxInteger::power,
   (PCPPM)&RexxInteger::concat,
   (PCPPM)&RexxInteger::concat, /* Duplicate entry neccessary        */
   (PCPPM)&RexxInteger::concatBlank,
   (PCPPM)&RexxInteger::equal,
   (PCPPM)&RexxInteger::notEqual,
   (PCPPM)&RexxInteger::isGreaterThan,
   (PCPPM)&RexxInteger::isLessOrEqual,
   (PCPPM)&RexxInteger::isLessThan,
   (PCPPM)&RexxInteger::isGreaterOrEqual,
                              /* Duplicate entry neccessary        */
   (PCPPM)&RexxInteger::isGreaterOrEqual,
   (PCPPM)&RexxInteger::isLessOrEqual,
   (PCPPM)&RexxInteger::strictEqual,
   (PCPPM)&RexxInteger::strictNotEqual,
   (PCPPM)&RexxInteger::strictGreaterThan,
   (PCPPM)&RexxInteger::strictLessOrEqual,
   (PCPPM)&RexxInteger::strictLessThan,
   (PCPPM)&RexxInteger::strictGreaterOrEqual,
                              /* Duplicate entry neccessary        */
   (PCPPM)&RexxInteger::strictGreaterOrEqual,
   (PCPPM)&RexxInteger::strictLessOrEqual,
   (PCPPM)&RexxInteger::notEqual,
   (PCPPM)&RexxInteger::notEqual,
   (PCPPM)&RexxInteger::andOp,
   (PCPPM)&RexxInteger::orOp,
   (PCPPM)&RexxInteger::xorOp,
   (PCPPM)&RexxInteger::operatorNot,
};

