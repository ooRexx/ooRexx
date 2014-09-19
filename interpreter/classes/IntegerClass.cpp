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
    if (Numerics::abs(value) > Numerics::MAX_WHOLENUMBER)
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
bool RexxInteger::truthValue(int errorcode)
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
 * Intercept unknown messages to an integer object and reissue
 * them against the string value.
 *
 * @param msgname   The unknown message name.
 * @param arguments The arguments to the message.
 *
 * @return The message result from the object's string value.
 */
RexxObject *RexxInteger::unknown(RexxString *msgname, ArrayClass *arguments)
{
    return stringValue()->sendMessage(msgname, arguments);
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
    // if using less than the default digits, do the math the hard way
    if (number_digits() < Numerics::DEFAULT_DIGITS)
    {
        return integer_forward(plus, other);
    }
    // if this is a plus operation, we just return this object as the result
    if (other == OREF_NULL)
    {
        return this;
    }
    // binary operation
    else
    {
        // if we have two integers, we can do this very quickly.  However, if we
        // overflow as a result, we fall back to the slow way
        if (isInteger(other))
        {
            wholenumber_t tempVal = value + other->value;
            // fall withing range?  return an integer result
            if (Numerics::isValid(tempVal))
            {
                return new_integer(tempVal);
            }
        }
        // type mismatch or potential overflow...do the math the hard way
        return integer_forward(plus, other);
    }
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
    // if less than default digits in effect, us full arithmetic to generate this
    if (number_digits() < Numerics::DEFAULT_DIGITS )
    {
        return integer_forward(minus, other);
    }

    // the unary minus is easy
    if (other == OREF_NULL)
    {
        return new_integer(-value);
    }
    else
    {
        // if subtracting two integer objects, try this in binary
        if (isInteger(other))
        {
            wholenumber_t tempVal = value - other->value;
            // if this is still in the whole number range, we can return a new Integer result
            if (Numerics::isValid(tempVal))
            {
                return new_integer(tempVal);
            }
        }

        // full number string arithmetic required
        return integer_forward(minus, other);
    }
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
    // if using less than the default digits, do this the slow way
    if (number_digits() < Numerics::DEFAULT_DIGITS )
    {
        return integer_forward(multiply, other);
    }

    // the other argument is required
    requiredArgument(other, ARG_ONE);
    // if the other value is an integer, we can multiply this directly if the other value is
    // an integer, but we need to do this using 64-bit math to detect overflows.
    if (isInteger(other))
    {
        int64_t tempThis = (int64_t)value;
        int64_t tempOther = (int64_t)other->value;

        int64_t tempValue = tempThis * tempOther;

        //.if still in a valid range, return a new integer value for this.
        if (Numerics::isValid64Bit(tempValue))
        {
            return new_integer((wholenumber_t)tempValue);
        }
    }
    // do this via the number string method
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
    // not even worth trying to do this via binary integer means.
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
    // if less than default digits, do this via number string
    if (number_digits() < Numerics::DEFAULT_DIGITS)
    {
        return integer_forward(integerDivide, other);
    }

    // the other argument is required
    requiredArgument(other, ARG_ONE);

    // we can do this via binary means, but need to check for divide by zero here.
    if (isInteger(other))
    {
        if (other->value != 0)
        {
            // do the division directly
            return new_integer(value / other->value);
        }
        else
        {
            reportException(Error_Overflow_zero);
        }
    }

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
    // skip doing this here if under reduced digits
    if (number_digits() < Numerics::DEFAULT_DIGITS)
    {
        return integer_forward(remainder, other);
    }

    requiredArgument(other, ARG_ONE);

    // if we have a pair of integer, we can do this here.
    if (isInteger(other))
    {
        // protect against divide by zero
        if (other->value != 0)
        {
            return new_integer(value % other->value);
        }
        else
        {
            reportException(Error_Overflow_zero);
        }
    }

    return integer_forward(remainder, other);
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
    // we might be able to optimize this a little by trying
    // this using direct binary math, but there are a lot of
    // ways this could go wrong, so just punt and do it via full
    // number string math.  This might work ok with very small
    // exponents...
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
    // if two integers, this is easy to do.
    if (isInteger(other))
    {
        return value - ((RexxInteger *)other)->value;
    }
    // string comparison
    else
    {
        return stringValue()->strictComp((RexxString *)other);
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

    if (isSameType(other) && number_digits() >= Numerics::DEFAULT_DIGITS)
    {
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
    return booleanObject(strictComp(other) == 0);
}


RexxObject *RexxInteger::strictNotEqual(RexxObject *other)
{
    if (other == TheNilObject)           // all conditionals return .false when compared to .nil
    {
        return TheTrueObject;
    }
    return booleanObject(strictComp(other) != 0);
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
 * Take the absolute value of an integer object
 *
 * @return The abs() result.
 */
RexxObject *RexxInteger::abs()
{
    // if working under the default digits or higher, do this here.
    if (number_digits() >= Numerics::DEFAULT_DIGITS)
    {
        // if we're already positive, this is a quick return
        if (value >= 0)
        {
            return this;
        }
        // negate and return as a new integer
        return new_integer(-value);
    }
    else
    {
        // return the numberstring result
        return numberString()->abs();
    }
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
 * @return The Largest of the numbers.
 */
RexxObject *RexxInteger::Max(RexxObject **args, size_t argCount)
{
    // if less than default digits, hand this off to the number string now
    if (number_digits() < Numerics::DEFAULT_DIGITS )
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
    wholenumber_t maxvalue = value;

    // now check all of the numbers in turn
    for (size_t arg = 0; arg < argCount; arg++)
    {
        RexxObject *argument = args[arg];
        requiredArgument(argument, arg);

        // if this is an integer object, we can continue doing this
        if (isInteger(argument))
        {
            wholenumber_t v = ((RexxInteger *)argument)->getValue();
            // get the larger value
            maxvalue = Numerics::maxVal(v, maxvalue);
        }
        // not all integers, so have numberstring figure this out
        else
        {
            return numberString()->Max(args, argCount);
        }
    }

    // return the maximum integer
    return new_integer(maxvalue);
}

RexxObject *RexxInteger::Min(
                            RexxObject **args,                 /* array of comparison values        */
                            size_t argCount)                   /* count of arguments                */
/******************************************************************************/
/* Function:  Perform MAX function on integer objects                         */
/******************************************************************************/
{
    // if less than default digits, hand this off to the number string now
    if (number_digits() < Numerics::DEFAULT_DIGITS )
    {
        return numberString()->Min(args, argCount);
    }

    // if nothing to compare against, we can return this object immediately
    // NOTE: we cannot move this before the digits test because the restricted digits
    // might require reformatting into exponential form or even rounding.
    if (argCount < 1)
    {
        return this;
    }


    // we can try this here as long as all of the numbers really are integers
    wholenumber_t minvalue = value;

    // now check all of the numbers in turn
    for (size_t arg = 0; arg < argCount; arg++)
    {
        RexxObject *argument = args[arg];
        requiredArgument(argument, arg);

        // if this is an integer object, we can continue doing this
        if (isInteger(argument))
        {
            wholenumber_t v = ((RexxInteger *)argument)->getValue();
            // get the larger value
            minvalue = Numerics::minVal(v, minvalue);
        }
        // not all integers, so have numberstring figure this out
        else
        {
            return numberString()->Min(args, argCount);
        }
    }

    // return the maximum integer
    return new_integer(minvalue);
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
    /* just forward to numberstring      */
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
    return this;
}


/**
 * Calculate the ceiling value for a numeric value
 *
 * @return The ceiling value.
 */
RexxObject *RexxInteger::ceiling()
{
    // the ceiling of an integer is always the same value
    return this;
}


/**
 * Calculate the round value for a numeric value
 *
 * @return The round value.
 */
RexxObject *RexxInteger::round()
{
    // the rounding of an integer is always the same value
    return this;
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
RexxObject *RexxInteger::d2c(RexxObject *length)
{
    return numberString()->d2xD2c(length, true);
}


/**
 * Convert a decimal number string into a hexadecimal string
 *
 * @param length The length for the conversion.
 *
 * @return The converted string.
 */
RexxObject *RexxInteger::d2x(RexxObject *length)
{
    return numberString()->d2xD2c(length, false);
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
 * This method will pre-allocate 100 integer objects, 0-99.  These will then
 * be used when ever a request for an integer between 0 and 99 is requested
 * this should help reduce some of our memory requirements and trips through
 * memory_new.
 */
void RexxIntegerClass::initCache()
{
    for (int i = IntegerCacheLow; i < IntegerCacheSize; i++ )
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
    for (int i = IntegerCacheLow; i < IntegerCacheSize ;i++ )
    {
        memory_mark(integercache[i - IntegerCacheLow]);
    }
}

void RexxIntegerClass::liveGeneral(MarkReason reason)
{
    RexxClass::liveGeneral(reason);// do RexxClass level marking

    // mark the cache array
    for (int i = IntegerCacheLow; i < IntegerCacheSize ;i++ )
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
    CLASS_CREATE(Integer, "String", RexxIntegerClass);
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

