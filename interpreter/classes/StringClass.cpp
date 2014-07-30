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
/* REXX Kernel                                               StringClass.c    */
/*                                                                            */
/* Primitive String Class                                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxActivation.hpp"
#include "Activity.hpp"
#include "ProtectedObject.hpp"
#include "StringUtil.hpp"
#include "CompoundVariableTail.hpp"
#include "SystemInterpreter.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *RexxString::classInstance = OREF_NULL;


// character validation sets for the datatype function
const char *RexxString::HEX_CHAR_STR = "0123456789ABCDEFabcdef";
const char *RexxString::ALPHANUM     = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const char *RexxString::BINARY       = "01";
const char *RexxString::LOWER_ALPHA  = "abcdefghijklmnopqrstuvwxyz";
const char *RexxString::MIXED_ALPHA  = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *RexxString::UPPER_ALPHA  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";


/**
 * Create initial class object at bootstrap time.
 */
void RexxString::createInstance()
{
    CLASS_CREATE(String, "String", RexxClass);
}


/**
 * Get the primitive hash value of this String object.
 *
 * @return The calculated string hash for the string.
 */
HashCode RexxString::getHashValue()
{
    // this will calculate the hash if it hasn't been done yet
    return getStringHash();
}


/**
 * Convert a string returned from an object HashCode() method into
 * a binary hashcode suitable for the hash collections.
 *
 * @return A binary hash code from a string value.
 */
HashCode RexxString::getObjectHashCode()
{
    HashCode h;

    // ok, we need to pick this string apart and turn this into a numeric code
    // a null string is simple.
    if (getLength() == 0)
    {
        h = 1;
    }

    // if we have at least 4 characters, use them as binary, since that's
    // what is normally returned here.
    else if (getLength() >= sizeof(HashCode))
    {
        h = *((HashCode *)getStringData());
    }
    else
    {
        // either 1 or 2 characters.  Just pick up a short value, which will
        // also pick up terminating null if only a single character
        h = *((short *)getStringData());
    }
    return h;
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void RexxString::live(size_t liveMark)
{
    memory_mark(numberStringValue);
    memory_mark(objectVariables);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void RexxString::liveGeneral(MarkReason reason)
{
    // if this string is part of the image, generate the numberstring value now
    // if possible to avoid oldspace/newspace cross references at trun time.
    if (reason == PREPARINGIMAGE)
    {
        numberString();
    }
    memory_mark_general(numberStringValue);
    memory_mark_general(objectVariables);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void RexxString::flatten(Envelope *envelope)
{
    setUpFlatten(RexxString)

    flattenRef(numberStringValue);
    flattenRef(objectVariables);

    cleanUpFlatten
}


/**
 * unflatten an object
 *
 * @param envelope The envelope owning the object currently.
 *
 * @return A potential replacement object if this is a proxy.
 */
RexxObject *RexxString::unflatten(Envelope *envelope)
{
    // if this has been proxied, then retrieve our target object from the environment
    if (isProxyObject())
    {
        return (RexxObject *)TheEnvironment->entry(this);
    }
    else
    {
        // perform a normal default unflatten op.
        return RexxObject::unflatten(envelope);
    }
}


/**
 * Return the primitive string value of this object
 *
 * @return Unless subclassed, directly returns the string value.
 */
RexxString *RexxString::stringValue()
{
    if (!isBaseClass())
    {
        return this;
    }
    else
    {
        // need to return a real string object, so create one from the string data.
        return new_string(getStringData(), getLength());
    }
}


/**
 * Handle a REQUEST('STRING') request for a REXX string object
 *
 * @return A string version of this string.
 */
RexxString  *RexxString::makeString()
{
    // the base class is easy
    if (isBaseClass())
    {
        return this;
    }
    else
    {
        // return a true string object if this is a subclass.
        return new_string(getStringData(), getLength());
    }
}


/**
 * Baseclass optimization for handling request array calls.
 *
 * @return The string object converted to an array using default arguments.
 */
ArrayClass  *RexxString::makeArray()
{
    // forward to the Rexx version with default arguments
    return makeArrayRexx(OREF_NULL);
}


/**
 * Copy a string value into to a compound variable tail.
 *
 * @param tail   The target tail object.
 */
void RexxString::copyIntoTail(CompoundVariableTail *tail)
{
    tail->append(getStringData(), getLength());
}


/**
 * Handle a REQUEST('STRING') request for a REXX string object
 *
 * @return Just returns the target string object.
 */
RexxString  *RexxString::primitiveMakeString()
{
    return this;
}


/**
 * Convert a string object to a whole number value.  Returns false if
 * it will not convert.
 *
 * @param result the return whole number result
 * @param digits the digits value for the conversion.
 *
 * @return true if this converted successfully.
 */
bool RexxString::numberValue(wholenumber_t &result, stringsize_t digits)
{
    // convert based off of requested string value.
    if (!isBaseClass())
    {
        return requestString()->numberValue(result, digits);
    }

    // get a number string for this string.  If we validated ok,
    // then convert that to an integer value.
    NumberString *numberstring = numberString();
    if (numberstring != OREF_NULL )
    {
        return numberstring->numberValue(result, digits);
    }
    return false;
}


/**
 * Convert a string object to a whole number value.  Returns false if
 * it will not convert.
 *
 * @param result the return whole number result
 *
 * @return true if this converted successfully.
 */
bool RexxString::numberValue(wholenumber_t &result)
{
    if (!isBaseClass())
    {
        return requestString()->numberValue(result);
    }

    NumberString *numberstring = numberString();
    if (numberstring != OREF_NULL )
    {
        return numberstring->numberValue(result);
    }
    return false;
}


/**
 * Convert a string object to an unsigned number value.  Returns
 * false if it will not convert.
 *
 * @param result the return whole number result
 * @param digits the digits value for the conversion.
 *
 * @return true if this converted successfully.
 */
bool RexxString::unsignedNumberValue(stringsize_t &result, stringsize_t digits)
{
    if (!isBaseClass())
    {
        return requestString()->unsignedNumberValue(result, digits);
    }
    NumberString *numberstring = numberString();
    if (numberstring != OREF_NULL )
    {
        return numberstring->unsignedNumberValue(result, digits);
    }
    return false;
}


/**
 * Convert a string object to an unsigned number value.  Returns
 * false if it will not convert.
 *
 * @param result the return whole number result
 *
 * @return true if this converted successfully.
 */
bool RexxString::unsignedNumberValue(stringsize_t &result)
{
    if (!isBaseClass())
    {
        return requestString()->unsignedNumberValue(result);
    }

    NumberString *numberstring = numberString();
    if (numberstring != OREF_NULL )
    {
        return numberstring->unsignedNumberValue(result);
    }
    return false;
}


/**
 * Convert a string object to a double value.  Returns false if
 * it will not convert.
 *
 * @param result the return whole number result
 *
 * @return true if this converted successfully.
 */
bool RexxString::doubleValue(double &result)
{
    NumberString *numberDouble = numberString();
    if (numberDouble != OREF_NULL)
    {
        return numberDouble->doubleValue(result);
    }
    // non numeric, so this could be one of the special cases
    if (strCompare("nan"))
    {
        result = std::numeric_limits<double>::signaling_NaN();
        // this will be false if this is really a NaN value. If true,
        // then fall back and use the quiet version.
        if (!isnan(result))
        {
          result = std::numeric_limits<double>::quiet_NaN();
        }
        return true;
    }
    if (strCompare("+infinity"))
    {
        result = +HUGE_VAL;
        return true;
    }
    if (strCompare("-infinity"))
    {
        result = -HUGE_VAL;
        return true;
    }
    return false;
}


/**
 * Convert a String Object into a Number Object
 *
 * @return The converted numeric value.
 */
NumberString *RexxString::numberString()
{
    // we might have one already, so reuse it.
    if (numberStringValue != OREF_NULL)
    {
        return numberStringValue;
    }

    // already tried and failed?  Return failure immediately.
    if (nonNumeric())
    {
        return OREF_NULL;
    }
    // a subclassed type?  Do this the long way
    if (!isBaseClass())
    {
        // get the request string value and create the number string from that value.
        // we set this in our value
        RexxString *newSelf = requestString();
        setField(numberStringValue, (NumberString *)new_numberstring(newSelf->getStringData(), newSelf->getLength()));
    }
    else
    {
        // generate the numberstring directly
        setField(numberStringValue, (NumberString *)new_numberstring(getStringData(), getLength()));
    }
    // mark as non-numeric if we could not create this so we won't try again.
    if (numberStringValue == OREF_NULL)
    {
        setNonNumeric();
    }
    else
    {
        // we have a numberstring now, so turn on reference marking for this object.
        setHasReferences();
        // set our string value into the numberstring as well.
        numberStringValue->setString(this);
    }
    return numberStringValue;
}


/**
 * Get a section of a string and copy it into a buffer
 *
 * @param startPos The starting offset within the string.
 * @param buffer   the location to copy to.
 * @param bufl     The size of the buffer.
 *
 * @return The actualy length copied.
 */
size_t RexxString::copyData(size_t startPos, char *buffer, size_t bufl)
{
    size_t copylen = 0;

    if (startPos < getLength())
    {
        if (bufl <= getLength() - startPos)
        {
            copylen = bufl;
        }
        else
        {
            copylen = getLength() - startPos;
        }
        memcpy(buffer, getStringData() + startPos, (size_t)copylen);
    }

    return copylen;
}


/**
 * Return the length of a string as an integer object.
 *
 * @return The string length.
 */
RexxObject *RexxString::lengthRexx()
{
    return new_integer(getLength());
}


/******************************************************************************/
/* Function:  Primitive strict equal\not equal method.  This determines       */
/*            only strict equality, not greater or less than values.          */
/******************************************************************************/
bool RexxString::isEqual(RexxObject *otherObj)
{
    requiredArgument(otherObj, ARG_ONE);
    // if not a primitive, we need to go the full == message route.
    if (!isBaseClass())
    {
        return sendMessage(OREF_STRICT_EQUAL, otherObj)->truthValue(Error_Logical_value_method);
    }

    if (otherObj == TheNilObject)        // strings never compare equal to the NIL object
    {
        return false;
    }

    RexxString *other = otherObj->requestString();
    size_t otherLen = other->getLength();
    // the length comparison is the easiest path to failure
    if (otherLen != getLength())
    {
        return false;
    }
    // compare the string data.
    return !memcmp(getStringData(), other->getStringData(), otherLen);
}


/**
 * Primitive strict equal\not equal method.  This determines
 * only strict equality, not greater or less than values.
 *
 * @param otherObj The comparison object.
 *
 * @return true or false, depending on the string equality.
 */
bool RexxString::primitiveIsEqual(RexxObject *otherObj)
{
    requiredArgument(otherObj, ARG_ONE);
    if (otherObj == TheNilObject)
    {
        return false;
    }

    RexxString *other = otherObj->requestString();
    size_t otherLen = other->getLength();
    if (otherLen != getLength())
    {
        return false;
    }
    return !memcmp(getStringData(), other->getStringData(), otherLen);
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
    requiredArgument(otherObj, ARG_ONE);
    if (otherObj == TheNilObject)        // strings never compare equal to the NIL object
    {
        return false;
    }
    RexxString *other = otherObj->requestString();
    stringsize_t otherLen = other->getLength();
    // can't compare equal if different lengths
    if (otherLen != getLength())
    {
        return false;
    }
    // do the actual string compare
    return StringUtil::caselessCompare(getStringData(), other->getStringData(), otherLen) == 0;
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
    if (isBaseClass())
    {
        // there should be a faster version of this...
        return primitiveCompareTo(stringArgument(other, ARG_ONE));
    }
    else
    {
        return RexxObject::compareTo(other);
    }
}


/**
 * Do a value comparison of two strings for the non-strict
 * comparisons.  This returns for the compares:
 *
 *    a value < 0 when this is smaller than other
 *    a value   0 when this is equal to other
 *    a value > 0 when this is larger than other
 *
 * @param other  The object we compare against.
 *
 * @return the relative compare value (<0, 0, or >0)
 */
wholenumber_t RexxString::comp(RexxObject *other)
{
    // We need to see if the objects can be Converted to NumberString Objs
    // 1st, this way we know if the COMP method of number String will
    // succeed.  Will only fail if an object cannot be represented as a
    // number.  This is important since NumberString calls String to do
    // the compare if it can't, since this is the method NumberString
    // will call, we must make sure a call to NumberString succeeds or
    // we will get into a loop.
    requiredArgument(other, ARG_ONE);
    // try and convert both numbers first.
    NumberString *firstNum = numberString();
    NumberString *secondNum = other->numberString();

    // if both are valid numbers, this is a numeric comparison.
    if (firstNum != OREF_NULL && secondNum != OREF_NULL)
    {
        return firstNum->comp(secondNum);
    }

    // we're doing a string comparison, so get the string version of the other.
    RexxString *second = other->requestString();

    // get the string specifics
    size_t firstLen = getLength();
    const char *firstStart = getStringData();

    size_t secondLen = second->getLength();
    const char *secondStart = second->getStringData();

    // skip over the leading white space characters in our string
    while (firstLen > 0 && (*firstStart == ch_SPACE || *firstStart == ch_TAB))
    {
        firstStart++;
        firstLen--;
    }
    // and the same for the second string
    while (secondLen > 0 && (*secondStart == ch_SPACE || *secondStart == ch_TAB))
    {
        secondStart++;
        secondLen--;
    }

    // done differntly depnding on which string is longer
    if (firstLen >= secondLen)
    {
        // compare for the shorter length
        wholenumber_t result = memcmp(firstStart, secondStart, (size_t) secondLen);
        // equal but differnt lengths?   We compare all the rest of the characters
        // using blank padding.
        if ((result == 0) && (firstLen != secondLen))
        {
            // point to first remainder char
            firstStart = firstStart + secondLen;
            while (firstLen-- > secondLen)
            {
                // Need unsigned char or chars above 0x7f will compare as less than
                // blank.
                unsigned char current = *firstStart++;
                if (current != ch_SPACE && current != ch_TAB)
                {
                    return current - ch_SPACE;
                }
            }
        }
        return result;
    }

    // same as above, but we reverse the blank compare result
    else
    {
        wholenumber_t result = memcmp(firstStart, secondStart, (size_t) firstLen);
        if (result == 0)
        {
            secondStart = secondStart + firstLen;
            while (secondLen-- > firstLen)
            {
                unsigned char current = *secondStart++;
                if (current != ch_SPACE && current != ch_TAB)
                {
                    return ch_SPACE - current;
                }
            }
        }
        return result;
    }
}


/**
 * Do a strict comparison of two strings.  This returns:
 *
 *    a value < 0 when this is smaller than other
 *    a value   0 when this is equal to other
 *    a value > 0 when this is larger than other
 *
 * @param otherObj The other comparison object.
 *
 * @return The relative comparison result (<0, 0, >0)
 */
wholenumber_t RexxString::strictComp(RexxObject *otherObj)
{
    wholenumber_t result;

    // get the string argument and the data/length values
    RexxString *other = stringArgument(otherObj, ARG_ONE);
    size_t otherLen = other->getLength();
    const char *otherData = other->getStringData();

    // if we are the longer string string, compare using the other length.  the
    // lengths are the tie breaker.
    if (getLength() >= otherLen)
    {
        result = memcmp(getStringData(), otherData, (size_t) otherLen);
        if ((result == 0) && (getLength() > otherLen))
        {
            result = 1;                      /* otherwise they are equal.         */
        }
    }
    // compare using our length...
    else
    {
        result = memcmp(getStringData(), otherData, (size_t) getLength());
        // since the other length is longer, we cannot be equal.  The other string
        // is longer, and is considered the greater of the two.
        if (result == 0)
        {
            result = -1;
        }
    }
    return result;
}


// simple macro for generating the arithmetic operator methods, which
// are essentially identical except for the final method call.
#define ArithmeticOperator(method)  \
    NumberString *numstr = numberString(); \
    if (numstr == OREF_NULL)              \
    {                                     \
        reportException(Error_Conversion_operator, this); \
    }                                     \
    return numstr->method(right_term);


/**
 * String addition...performed by NumberString
 *
 * @param right_term The other operator term.
 *
 * @return The operator result
 */
RexxObject *RexxString::plus(RexxObject *right_term)
{
    ArithmeticOperator(plus);
}


/**
 * String subtraction...performed by NumberString
 *
 * @param right_term The other operator term.
 *
 * @return The operator result
 */
RexxObject *RexxString::minus(RexxObject *right_term)
{
    ArithmeticOperator(minus);
}


/**
 * String multiplication...performed by NumberString
 *
 * @param right_term The other operator term.
 *
 * @return The operator result
 */
RexxObject *RexxString::multiply(RexxObject *right_term)
{
    ArithmeticOperator(multiply);
}


/**
 * String division...performed by NumberString
 *
 * @param right_term The other operator term.
 *
 * @return The operator result
 */
RexxObject *RexxString::divide(RexxObject *right_term)
{
    ArithmeticOperator(divide);
}


/**
 * String integer division...performed by NumberString
 *
 * @param right_term The other operator term.
 *
 * @return The operator result
 */
RexxObject *RexxString::integerDivide(RexxObject *right_term)
{
    ArithmeticOperator(integerDivide);
}


/**
 * String remainder division...performed by NumberString
 *
 * @param right_term The other operator term.
 *
 * @return The operator result
 */
RexxObject *RexxString::remainder(RexxObject *right_term)
{
    ArithmeticOperator(remainder);
}


/**
 * String power operator...performed by NumberString
 *
 * @param right_term The other operator term.
 *
 * @return The operator result
 */
RexxObject *RexxString::power(RexxObject *right_term)
{
    ArithmeticOperator(power);
}


// simple macro for generating the arithmetic methods, which
// are essentially identical except for the final method call.
#define ArithmeticMethod(method, name)  \
    NumberString *numstr = numberString(); \
    if (numstr == OREF_NULL)              \
    {                                     \
        reportException(Error_Incorrect_method_string_nonumber, name, this); \
    }                                     \
    return numstr->method;



/**
 * String absolute value...performed by NumberString
 *
 * @return The absolute value result.
 */
RexxObject *RexxString::abs()
{
    ArithmeticMethod(abs(), "ABS");
}


/**
 * String sign value...performed by NumberString
 *
 * @return The sign result.
 */
RexxObject *RexxString::sign()
{
    ArithmeticMethod(Sign(), "SIGN");
}


/**
 * String max value...performed by NumberString
 *
 * @param arguments The variable list of Max arguments.
 * @param argCount  The count of arguments.
 *
 * @return The Max result.
 */
RexxObject *RexxString::Max(RexxObject **arguments, size_t argCount)
{
    ArithmeticMethod(Max(arguments, argCount), "MAX");
}


/**
 * String min value...performed by NumberString
 *
 * @param arguments The variable list of Min arguments.
 * @param argCount  The count of arguments.
 *
 * @return The Min result.
 */
RexxObject *RexxString::Min(RexxObject **arguments, size_t argCount)
{
    ArithmeticMethod(Min(arguments, argCount), "MIN");
}


/**
 * String Trunc...performed by NumberString
 *
 * @param decimals The number of decimals to truncate to.
 *
 * @return The trunc result.
 */
RexxObject *RexxString::trunc(RexxInteger *decimals)
{
    ArithmeticMethod(trunc(decimals), "TRUNC");
}


/**
 * The String class version of the floor method.
 *
 * @return The formatted numeric version.
 */
RexxObject *RexxString::floor()
{
    ArithmeticMethod(floor(), "FLOOR");
}


/**
 * The String class version of the ceiling method.
 *
 * @return The formatted numeric version.
 */
RexxObject *RexxString::ceiling()
{
    ArithmeticMethod(ceiling(), "CEILING");
}

/**
 * The String class version of the round method.
 *
 * @return The formatted numeric version.
 */
RexxObject *RexxString::round()
{
    ArithmeticMethod(round(), "ROUND");
}


/**
 * String Format...performed by NumberString
 *
 * @param integers   The number if integer positions.
 * @param decimals   The number of decimals requested.
 * @param mathExp    The size of the exponent
 * @param expTrigger The expontent trigger value.
 *
 * @return The formatted number.
 */
RexxObject *RexxString::format(RexxObject *integers, RexxObject *decimals, RexxObject *mathExp, RexxObject *expTrigger)
{
    ArithmeticMethod(formatRexx(integers, decimals, mathExp, expTrigger), "FORMAT");
}


/**
 * The string equals() method, which does a strict compare with
 * another string object.
 *
 * @param other  The other string object.
 *
 * @return True if the strings are equal, false for inequality.
 */
RexxObject *RexxString::equals(RexxString *other)
{
    return booleanObject(primitiveIsEqual(other));
}


/**
 * The string equals() method, which does a strict caseless
 * compare with another string object.
 *
 * @param other  The other string object.
 *
 * @return True if the strings are equal, false for inequality.
 */
RexxObject *RexxString::caselessEquals(RexxString *other)
{
    return booleanObject(primitiveCaselessIsEqual(other));
}


/**
 * Strict ("==") equality operator...also returns the hash value
 * if sent with no other object
 *
 * @param other  The other comparison result.
 *
 * @return .true if equal, .false otherwise.
 */
RexxObject *RexxString::strictEqual(RexxObject *other)
{
    return booleanObject(primitiveIsEqual(other));
}


/**
 * Strict ("\==") inequality operator
 *
 * @param other  The other comparison operator.
 *
 * @return .true if not equal, .false if equal.
 */
RexxObject *RexxString::strictNotEqual(RexxObject *other)
{
    return booleanObject(!primitiveIsEqual(other));
}


// macro for generating common comparison operators.
#define CompareOperator(comp)  \
    if (other == TheNilObject)  \
    {                           \
        return TheFalseObject;  \
    }                           \
    return booleanObject(comp);


RexxObject *RexxString::equal(RexxObject *other)
/******************************************************************************/
/* Function:  Non-strict ("=") string equality operator                       */
/******************************************************************************/
{
    CompareOperator(comp(other) == 0);
}

RexxObject *RexxString::notEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Non-Strict ("\=") string inequality operator                    */
/******************************************************************************/
{
    CompareOperator(comp(other) != 0);
}

RexxObject *RexxString::isGreaterThan(RexxObject *other)
/******************************************************************************/
/* Function:  Non-strict greater than operator (">")                          */
/******************************************************************************/
{
    CompareOperator(comp(other) > 0);
}

RexxObject *RexxString::isLessThan(RexxObject *other)
/******************************************************************************/
/* Function:  Non-strict less than operatore ("<")                            */
/******************************************************************************/
{
    CompareOperator(comp(other) < 0);
}

RexxObject *RexxString::isGreaterOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Non-strict greater than or equal operator (">=" or "\<")        */
/******************************************************************************/
{
    CompareOperator(comp(other) >= 0);
}

RexxObject *RexxString::isLessOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Non-strict less than or equal operator ("<=" or "\>")           */
/******************************************************************************/
{
    CompareOperator(comp(other) <= 0);
}

RexxObject *RexxString::strictGreaterThan(RexxObject *other)
/******************************************************************************/
/* Function:  Strict greater than comparison (">>")                           */
/******************************************************************************/
{
    CompareOperator(strictComp(other) > 0);
}

RexxObject *RexxString::strictLessThan(RexxObject *other)
/******************************************************************************/
/* Function:  Strict less than comparison ("<<")                              */
/******************************************************************************/
{
    CompareOperator(strictComp(other) < 0);
}

RexxObject *RexxString::strictGreaterOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Strict greater than or equal to comparison (">>=" or "\<<")     */
/******************************************************************************/
{
    CompareOperator(strictComp(other) >= 0);
}


RexxObject *RexxString::strictLessOrEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Strict less than or equal to operatore ("<<=" or "\>>")         */
/******************************************************************************/
{
    CompareOperator(strictComp(other) <= 0);
}

RexxString *RexxString::concat(RexxString *other)
/******************************************************************************/
/* Function:  Concatenate two strings together                                */
/******************************************************************************/
{
    size_t len1;                         /* length of first string            */
    size_t len2;                         /* length of second string           */
    RexxString *result;                  /* result string                     */
    char *data;                          /* character pointer                 */

    len1 = getLength();            /* get this length                   */
    len2 = other->getLength();           /* and the other length              */

    if (len2 == 0)                       // some people have taken to using a''b
    {
        // to perform concatenation operations
        return this;                     // it makes sense to optimize concatenation
    }                                    // with a null string by just returning
    if (len1 == 0)                       // the non-null object.
    {
        return other;
    }
    /* create a new string               */
    result = (RexxString *)raw_string(len1+len2);
    data = result->getWritableData();    /* point to the string data          */

    // both lengths are non-zero because of the test above, so we can
    // unconditionally copy
    /* copy the front part               */
    memcpy(data, getStringData(), len1);
    memcpy(data + len1, other->getStringData(), len2);
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
    char *data;                          /* character pointer                 */

    requiredArgument(otherObj, ARG_ONE);         /* this is required.                 */
                                         /* ensure a string value             */
    other = otherObj->requestString();

    /* added error checking for NULL pointer (from NilObject) */
    if (other == OREF_NULL)
    {
        reportException(Error_Incorrect_method_nostring, IntegerOne);
    }

    /* the following logic also appears  */
    /* in string_concat, but is repeated */
    /* here because this is a VERY high  */
    /* use function                      */
    len1 = getLength();                 /* get this length                   */
    len2 = other->getLength();                /* and the other length              */
    /* create a new string               */
    result = (RexxString *)raw_string(len1+len2);
    data = result->getWritableData();    /* point to the string data          */
    if (len1 != 0)
    {                     /* have real data?                   */
                          /* copy the front part               */
        memcpy(data, getStringData(), len1);
        data += len1;                      /* step past the length              */
    }
    if (len2 != 0)                       /* have a second length              */
    {
        /* and the second part               */
        memcpy(data, other->getStringData(), len2);
    }
    return result;                       /* return the result                 */
}

RexxString *RexxString::concatToCstring(const char *other)
/******************************************************************************/
/* Function:  Concatenate a string object onto an ASCII-Z string              */
/******************************************************************************/
{
    size_t len1;                         /* length of first string            */
    size_t len2;                         /* length of ASCII-Z string          */
    RexxString *result;                  /* result string                     */

    len1 = getLength();                 /* get this length                   */
    len2 = strlen(other);                /* and the other length              */
                                         /* create a new string               */
    result = (RexxString *)raw_string(len1+len2);
    /* copy the front part               */
    memcpy(result->getWritableData(), other, len2);
    /* and the second part               */
    memcpy(result->getWritableData() + len2, getStringData(), len1);
    return result;
}

RexxString *RexxString::concatWithCstring(const char *other)
/******************************************************************************/
/* Function:  Concatenate an ASCII-Z string onto a string object              */
/******************************************************************************/
{
    size_t len1;                         /* length of first string            */
    size_t len2;                         /* length of ASCII-Z string          */
    RexxString *result;                  /* result string                     */

    len1 = getLength();                 /* get this length                   */
    len2 = strlen(other);                /* and the other length              */
                                         /* create a new string               */
    result = (RexxString *)raw_string(len1+len2);
    /* copy the string object            */
    memcpy(result->getWritableData(), getStringData(), len1);
    /* copy the ASCII-Z string           */
    memcpy(result->getWritableData() + len1, other, len2);
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
    char *data;                          /* character pointer                 */

    requiredArgument(otherObj, ARG_ONE);         /* this is required.                 */
                                         /* ensure a string value             */
    other = otherObj->requestString();

    /* the following logic also appears  */
    /* in string_concat_with, but is     */
    /* repeated here because this is a   */
    /* VERY high use function            */
    len1 = getLength();                 /* get this length                   */
    len2 = other->getLength();                /* and the other length              */
    /* create a new string               */
    result = (RexxString *)raw_string(len1+len2+1);
    data = result->getWritableData();    /* point to the string data          */
    if (len1 != 0)
    {                     /* have a first string?              */
                          /* copy the front part               */
        memcpy(data, getStringData(), len1);
        data += len1;                      /* step past the length              */
    }
    *data++ = ' ';                       /* stuff in the seperating blank     */
    if (len2 != 0)                       /* have a second string?             */
    {
        /* and the second part               */
        memcpy(data, other->getStringData(), len2);
    }
    return result;
}

bool RexxString::truthValue(int errorCode)
/******************************************************************************/
/* Function:  Determine the truth value of a string object, raising the       */
/*            given error if bad.                                             */
/******************************************************************************/
{
    RexxString *testString;              /* string to test                    */

    if (!isBaseClass())                 /*  a nonprimitive object?           */
    {
        testString = requestString();/* get the real string value         */
    }
    else
    {
        testString = this;                 /* just use the string directly      */
    }
    if (testString->getLength() != 1)    /* not exactly 1 character long?     */
    {
        /* report the error                  */
        reportException(errorCode, testString);
    }
    if (*(testString->getStringData()) == '0')/* exactly '0'?                      */
    {
        return false;                    /* have a false                      */
    }
                                         /* not exactly '1'?                  */
    else if (!(*(testString->getStringData()) == '1'))
    {
        reportException(errorCode, this);/* report the error                  */
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
bool RexxString::logicalValue(logical_t &result)
{
    RexxString *testString;              /* string to test                    */

    if (!isBaseClass())                  /*  a nonprimitive object?           */
    {
        testString = requestString();/* get the real string value         */
    }
    else
    {
        testString = this;                 /* just use the string directly      */
    }

    if (testString->getLength() != 1)    /* not exactly 1 character long?     */
    {
        return false;     // not a valid logical
    }
    if (testString->getChar(0) == '0')/* exactly '0'?                      */
    {
        result = false;                  // this is false and the conversion worked
        return true;
    }
                                         /* exactly '1'?                  */
    else if (testString->getChar(0) == '1')
    {
        result = true;                   // this is true and the conversion worked
        return true;
    }
    return false;       // did not convert correctly
}

bool RexxString::checkLower()
/******************************************************************************/
/* Function:  Tests for existence of lowercase characters                     */
/******************************************************************************/
{
    const char *data;                    /* current data pointer              */
    const char *endData;                 /* end location                      */

    data = getStringData();        /* point to the string               */
    endData = data + getLength();  /* set the end point                 */

    while (data < endData)
    {             /* loop through entire string        */
        if (*data != toupper(*data))
        {     /* have something to uppercase?      */
            setHasLower();             /* remember we have this             */
            return true;                     /* just return now                   */
        }
        data++;                            /* step the position                 */
    }
    /* no lowercase?                     */
    setUpperOnly();                /* set the upper only attribute      */
    return false;                        /* return then translation flag      */
}

RexxString *RexxString::upper()
/******************************************************************************/
/* Function:  Translate a string to uppercase...will only create a new        */
/*            string if characters actually have to be translated.            */
/******************************************************************************/
{
    RexxString *newstring;               /* newly created string              */
    const char *data;                    /* current data pointer              */
    char * outdata;                      /* output data                       */
    const char *endData;                 /* end of the data                   */

                                         /* something to uppercase?           */
    if (!upperOnly() && (hasLower() || checkLower()))
    {
        /* create a new string               */
        newstring = (RexxString *)raw_string(getLength());
        data = getStringData();      /* point to the data start           */
                                           /* point to output data              */
        outdata = newstring->getWritableData();
        endData = data + getLength();     /* set the loop terminator           */
        while (data < endData)
        {           /* loop through entire string        */
            *outdata = toupper(*data);       /* copy the uppercase character      */
            data++;                          /* step the position                 */
            outdata++;                       /* and the output position           */
        }
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
    // NOTE:  since we're doing value comparisons on single character values here,
    // we need to process this as unsigned characters to handle values
    // greater than 0x7f.
    const unsigned char *Current;        /* current string location           */
    size_t    i;                         /* string length                     */
    bool      NonDisplay;                /* have non-displayables             */

    i = getLength();               /* get the length                    */
                                         /* point to the start                */
    Current = (const unsigned char *)getStringData();
    NonDisplay = false;                  /* no non-displayable characters     */

    for (; i > 0; i--)
    {                 /* loop for the entire string        */
                      /* control character?                */
        if (*Current < ch_SPACE)
        {
            NonDisplay = true;               /* got a non-displayable             */
            break;                           /* get out of here                   */
        }
        Current++;                         /* step the pointer                  */
    }
    if (!NonDisplay)                     /* all displayable?                  */
    {
        return this;                       /* leave unchanged                   */
    }
                                           /* copy the string                   */
    newCopy = (RexxString *) copy();
    i = newCopy->getLength();                 /* get the length                    */
    /* point to the start                */
    char *outptr = newCopy->getWritableData();

    for (; i > 0; i--)
    {                 /* loop for the entire string        */
                      /* control character?                */
        if (*outptr < ch_SPACE && *outptr != ch_TAB)
        {
            *outptr = '?';                 /* yes, change to question           */
        }
        outptr++;                        /* step the pointer                  */
    }
    return newCopy;                      /* return the converted string       */
}


RexxString *RexxString::lower()
/******************************************************************************/
/* Function:  Translate a string to lower case                                */
/******************************************************************************/
{
    RexxString *newstring;               /* newly created string              */
    const char *   data;                 /* current data pointer              */
    char *         outdata;              /* output data                       */
    size_t i;                            /* loop counter                      */
    bool   needTranslation;              /* translation required              */

    data = getStringData();        /* point to the string               */
    needTranslation = false;             /* no translation required           */

    for (i = 0; i < getLength(); i++)
    { /* loop through entire string        */
        if (*data != tolower(*data))
        {     /* have something to lowercase?      */
            needTranslation = true;          /* flag it                           */
            break;                           /* stop at the first one             */
        }
        data++;                            /* step the position                 */
    }
    if (needTranslation)
    {               /* something to uppercase?           */
                    /* create a new string               */
        newstring = (RexxString *)raw_string(getLength());
        data = getStringData();      /* point to the data start           */
                                           /* point to output data              */
        outdata = newstring->getWritableData();
        /* loop through entire string        */
        for (i = 0; i < getLength(); i++)
        {
            *outdata = tolower(*data);       /* copy the lowercase character      */
            data++;                          /* step the position                 */
            outdata++;                       /* and the output position           */
        }
    }
    else
    {
        newstring = this;                  /* return untranslated string        */
    }
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
RexxString *RexxString::lowerRexx(RexxInteger *_start, RexxInteger *_length)
{
    size_t startPos = optionalPositionArgument(_start, 1, ARG_ONE) - 1;
    size_t rangeLength = optionalLengthArgument(_length, getLength(), ARG_TWO);

    // if we're starting beyond the end bounds, return unchanged
    if (startPos >= getLength())
    {
        return this;
    }

    rangeLength = Numerics::minVal(rangeLength, getLength() - startPos);

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
RexxString *RexxString::upperRexx(RexxInteger *_start, RexxInteger *_length)
{
    size_t startPos = optionalPositionArgument(_start, 1, ARG_ONE) - 1;
    size_t rangeLength = optionalLengthArgument(_length, getLength(), ARG_TWO);

    // if we're starting beyond the end bounds, return unchanged
    if (startPos >= getLength())
    {
        return this;
    }

    rangeLength = Numerics::minVal(rangeLength, getLength() - startPos);

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
RexxString *RexxString::lower(size_t offset, size_t _length)
{
    // get a copy of the string
    RexxString *newstring = extract(0, getLength());

    char *data = newstring->getWritableData() + offset;
    // now uppercase in place
    for (size_t i = 0; i < _length; i++)
    {
        *data = tolower(*data);
        data++;
    }
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
RexxString *RexxString::upper(size_t offset, size_t _length)
{
    // get a copy of the string
    RexxString *newstring = extract(0, getLength());

    char *data = newstring->getWritableData() + offset;
    // now uppercase in place
    for (size_t i = 0; i < _length; i++)
    {
        *data = toupper(*data);
        data++;
    }
    return newstring;
}

RexxInteger *RexxString::integerValue(
    size_t digits)                     /* precision to use                  */
/******************************************************************************/
/* Function:  Convert a string object to an integer.  Returns .nil for        */
/*            failures.                                                       */
/******************************************************************************/
{
    NumberString *numberStr;         /* string's numberstring version     */
    RexxInteger *newInteger;             /* returned integer string           */

                                         /* Force String conversion through   */
                                         /* NumberString                      */
                                         /* get the number string version     */
    if ((numberStr = numberString()) != OREF_NULL )
    {
        /* try for an integer                */
        newInteger = numberStr->integerValue(digits);
        /* did it convert?                   */
        if (newInteger != TheNilObject && newInteger->getStringrep() == OREF_NULL)
        {
            newInteger->setString(this);     /* connect the string value          */
        }
        return newInteger;                 /* return the new integer            */
    }
    else
    {
        return(RexxInteger *)TheNilObject;/* return .nil for failures          */
    }
}

void RexxString::setNumberString(RexxObject *NumberRep)
/******************************************************************************/
/* Function:  Set a number string value on to the string                      */
/******************************************************************************/
{

    OrefSet(this, numberStringValue, (NumberString *)NumberRep);

    if (NumberRep != OREF_NULL)          /* actually get one?                 */
    {
        setHasReferences();           /* Make sure we are sent Live...     */
    }
    else
    {
        setHasNoReferences();         /* no more references                */
    }
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
    char *data;                          /* character pointer                 */

    len1 = getLength();                 /* get this length                   */
    len2 = other->getLength();                /* and the other length              */
    /* create a new string               */
    result = (RexxString *)raw_string(len1+len2+1);
    data = result->getWritableData();         /* point to the string data          */
    if (len1 != 0)
    {                     /* have a first string?              */
                          /* copy the front part               */
        memcpy(data, getStringData(), len1);
        data += len1;                      /* step past the length              */
    }
    *data++ = between;                   /* stuff in the seperating char      */
    if (len2 != 0)                       /* have a second string?             */
    {
        /* and the second part               */
        memcpy(data, other->getStringData(), len2);
    }
    return result;
}

RexxObject *RexxString::andOp(RexxObject *other)
/******************************************************************************/
/* Function:  Logical AND of a string with another logical value              */
/******************************************************************************/
{
    RexxObject *otherTruth;              /* truth value of the other object   */

    requiredArgument(other, ARG_ONE);            /* make sure the argument is there   */
                                         /* validate the boolean              */
    otherTruth = booleanObject(other->truthValue(Error_Logical_value_method));
    /* perform the operation             */
    return(!truthValue(Error_Logical_value_method)) ? TheFalseObject : otherTruth;
}

RexxObject *RexxString::orOp(RexxObject *other)
/******************************************************************************/
/* Function:  Logical OR of a string with another logical value               */
/******************************************************************************/
{
    RexxObject *otherTruth;              /* truth value of the other object   */

    requiredArgument(other, ARG_ONE);            /* make sure the argument is there   */
                                         /* validate the boolean              */
    otherTruth = booleanObject(other->truthValue(Error_Logical_value_method));
    /* perform the operation             */
    return(truthValue(Error_Logical_value_method)) ? TheTrueObject : otherTruth;
}

RexxObject *RexxString::xorOp(RexxObject *other)
/******************************************************************************/
/* Function:  Logical XOR of a string with another logical value              */
/******************************************************************************/
{
    requiredArgument(other, ARG_ONE);            /* make sure the argument is there   */
                                         /* get as a boolean                  */
    bool truth = other->truthValue(Error_Logical_value_method);
    /* first one false?                  */
    if (!truthValue(Error_Logical_value_method))
    {
        /* value is always the second        */
        return booleanObject(truth);
    }
    else                                 /* value is inverse of second        */
    {
        return booleanObject(!truth);
    }
}

ArrayClass *RexxString::makeArrayRexx(RexxString *div)
/******************************************************************************/
/* Function:  Split string into an array                                      */
/******************************************************************************/
{
    return StringUtil::makearray(getStringData(), getLength(), div);
}


RexxObject *RexxString::notOp()
/******************************************************************************/
/* Function:  Logical NOT of a string                                         */
/******************************************************************************/
{
    return booleanObject(!truthValue(Error_Logical_value_method));
}

RexxObject *RexxString::operatorNot(RexxObject *other)
/******************************************************************************/
/* Function:  Logical NOT of a string                                         */
/******************************************************************************/
{
    return booleanObject(!truthValue(Error_Logical_value_method));
}

RexxObject *RexxString::evaluate(
    RexxActivation      *context,      /* current activation context        */
    ExpressionStack *stack )       /* evaluation stack                  */
/******************************************************************************/
/* Function:  Polymorphic method that makes string a polymorphic expression   */
/*            term for string literals.                                       */
/******************************************************************************/
{

    stack->push((RexxObject *)this);     /* place on the evaluation stack     */
                                       /* trace if necessary                */
    context->traceIntermediate((RexxObject *)this, RexxActivation::TRACE_PREFIX_LITERAL);
    return this;                         /* also return the result            */
}


/**
 * Copy a string to an RXSTRING, with appropriate allocation
 * of a new buffer if required.
 *
 * @param r
 */
void RexxString::copyToRxstring(RXSTRING &r)
{
    size_t result_length = getLength() + 1;
    if (r.strptr == NULL || r.strlength < result_length)
    {
        r.strptr = (char *)SystemInterpreter::allocateResultMemory(result_length);
    }
    // copy all of the data + the terminating null
    memcpy(r.strptr, getStringData(), result_length);
    // fill in the length too
    r.strlength = getLength();
}


RexxObject  *RexxString::getValue(
    RexxActivation *context)           /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return this;
}


RexxObject  *RexxString::getValue(
    VariableDictionary *context)   /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return this;
}


RexxObject  *RexxString::getRealValue(
    RexxActivation *context)           /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return this;
}


RexxObject  *RexxString::getRealValue(
    VariableDictionary *context)   /* current activation context        */
/******************************************************************************/
/* Function:  Polymorphic get_value function used with expression terms       */
/******************************************************************************/
{
    return this;
}


RexxString *RexxString::newString(const char *string, size_t length)
/******************************************************************************/
/* Function:  Allocate (and initialize) a string object                       */
/******************************************************************************/
{
    /* calculate the size                */
    /* STRINGOBJ - excess chars (3)      */
    /* + length. only sub 3 to allow     */
    /* for terminating NULL              */
    size_t size2 = sizeof(RexxString) - (sizeof(char) * 3) + length;
    /* allocate the new object           */
    RexxString *newObj = (RexxString *)new_object(size2, T_String);
    /* clear the front part              */
    newObj->setLength(length);           /* save the length                   */
    newObj->hashValue = 0;               // make sure the hash value is zeroed
                                         /* Null terminate, allows faster     */
                                         /* conversion to ASCII-Z string      */
    newObj->putChar(length, '\0');
    /* copy it over                      */
    newObj->put(0, string, length);
    /* by  default, we don't need Live   */
    newObj->setHasNoReferences();        /*sent                               */
                                         /* NOTE: That if we can set          */
                                         /*  NumebrString elsewhere     */
                                         /*we need to mark ourselves as       */
    return newObj;                       /*having OREFs                       */
}

RexxString *RexxString::rawString(size_t length)
/******************************************************************************/
/* Function:  Allocate (and initialize) an empty string object                */
/******************************************************************************/
{
                                       /* calculate the size                */
                                       /* STRINGOBJ - excess chars (3)      */
                                       /* + length. only sub 3 to allow     */
                                       /* for terminating NULL              */
  size_t size2 = sizeof(RexxString) - (sizeof(char) * 3) + length;
                                       /* allocate the new object           */
  RexxString *newObj = (RexxString *)new_object(size2, T_String);
  newObj->setLength(length);           /* save the length                   */
  newObj->hashValue = 0;               // make sure the hash value is zeroed
                                       /* Null terminate, allows faster     */
                                       /* conversion to ASCII-Z string      */
  newObj->putChar(length, '\0');
                                       /* by  default, we don't need Live   */
  newObj->setHasNoReferences();        /*sent                               */
                                       /* NOTE: That if we can set          */
                                       /*  NumebrString elsewhere     */
                                       /*we need to mark ourselves as       */
  return newObj;                       /*having OREFs                       */
}


/**
 * Allocate an initialize a string object that will also
 * contain only uppercase characters.  This allows a creation
 * and uppercase operation to be done in one shot, without
 * requiring two string objects to be created.
 *
 * @param string The source string data.
 * @param length The length of the string data.
 *
 * @return A newly constructed string object.
 */
RexxString *RexxString::newUpperString(const char * string, stringsize_t length)
{
    /* calculate the size                */
    /* STRINGOBJ - excess chars (3)      */
    /* + length. only sub 3 to allow     */
    /* for terminating NULL              */
    size_t size2 = sizeof(RexxString) - (sizeof(char) * 3) + length;
    /* allocate the new object           */
    RexxString *newObj = (RexxString *)new_object(size2, T_String);
    newObj->length = length;             /* save the length                   */
    newObj->hashValue = 0;               // make sure the hash value is zeroed
                                         /* create a new string               */
                                         /* point to output data              */
    char *outdata = newObj->getWritableData();
    // set the input markers
    const char *indata = string;
    const char *endData = indata + length;
    while (indata < endData)             /* loop through entire string        */
    {
        *outdata = toupper(*indata);     /* copy the uppercase character      */
        indata++;                        /* step the position                 */
        outdata++;                       /* and the output position           */
    }
    newObj->setUpperOnly();              /* flag the string as uppercased     */
                                         /* Null terminate, allows faster     */
                                         /* conversion to ASCII-Z string      */
    newObj->putChar(length, '\0');
    /* by  default, we don't need Live   */
    newObj->setHasNoReferences();        /*sent                               */
                                         /* NOTE: That if we can set          */
                                         /*  NumebrString elsewhere     */
                                         /*we need to mark ourselves as       */
    return newObj;                       /*having OREFs                       */
}

RexxString *RexxString::newString(double number)
/******************************************************************************/
/* Function: Create a string from a double value                              */
/******************************************************************************/
{
                                       /* get double as a number string.    */
  return new_numberstringFromDouble(number)->stringValue();
}


/**
 * Convert a double value to a string using the provided
 * precision.
 *
 * @param number    The number to convert.
 * @param precision The precision requested for the result.
 *
 * @return A string value of the converted result.
 */
RexxString *RexxString::newString(double number, stringsize_t precision)
{
    if (number == 0)                     /* zero result?               */
    {
        return new_string("0");
    }
    else
    {
        char buffer[64];
        // format as a string
        sprintf(buffer, "%.*g", (int)precision, number);
        size_t len = strlen(buffer);
        // if the last character is a decimal, we remove that
        if (buffer[len - 1] == '.')
        {
            len--;
        }
        return new_string(buffer, len);
    }
}


RexxString *RexxString::newProxy(const char *string)
/******************************************************************************/
/* Function:  Create a proxy object from this string                          */
/******************************************************************************/
{
  RexxString *sref;
                                       /* The provided source string is null*/
                                       /*  terminated so let class_new      */
                                       /*  compute the length.              */
                                       /* get a new string object           */
  sref = (RexxString *)new_string(string);
                                       /* here we need to identify this     */
                                       /*string                             */
  sref->makeProxiedObject();           /*  as being a proxy object          */

  return sref;
}

RexxString *RexxString::newRexx(RexxObject **init_args, size_t argCount)
/******************************************************************************/
/* Arguments: Subclass init arguments                                         */
/* Function:  Create a new string value (used primarily for subclasses)       */
/******************************************************************************/
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxObject *stringObj;               /* string value                      */

                                         /* break up the arguments            */
    RexxClass::processNewArgs(init_args, argCount, &init_args, &argCount, 1, (RexxObject **)&stringObj, NULL);
    /* force argument to string value    */
    RexxString *string = (RexxString *)stringArgument(stringObj, ARG_ONE);
    /* create a new string object        */
    string = new_string(string->getStringData(), string->getLength());
    ProtectedObject p(string);

    // handle Rexx class completion
    classThis->completeNewObject(string, init_args, argCount);
    return string;
}


PCPPM RexxString::operatorMethods[] =
{
   NULL,                               /* first entry not used              */
   (PCPPM)&RexxString::plus,
   (PCPPM)&RexxString::minus,
   (PCPPM)&RexxString::multiply,
   (PCPPM)&RexxString::divide,
   (PCPPM)&RexxString::integerDivide,
   (PCPPM)&RexxString::remainder,
   (PCPPM)&RexxString::power,
   (PCPPM)&RexxString::concatRexx,
   (PCPPM)&RexxString::concatRexx,
   (PCPPM)&RexxString::concatBlank,
   (PCPPM)&RexxString::equal,
   (PCPPM)&RexxString::notEqual,
   (PCPPM)&RexxString::isGreaterThan,
   (PCPPM)&RexxString::isLessOrEqual,
   (PCPPM)&RexxString::isLessThan,
   (PCPPM)&RexxString::isGreaterOrEqual,
                              /* Duplicate entry neccessary        */
   (PCPPM)&RexxString::isGreaterOrEqual,
   (PCPPM)&RexxString::isLessOrEqual,
   (PCPPM)&RexxString::strictEqual,
   (PCPPM)&RexxString::strictNotEqual,
   (PCPPM)&RexxString::strictGreaterThan,
   (PCPPM)&RexxString::strictLessOrEqual,
   (PCPPM)&RexxString::strictLessThan,
   (PCPPM)&RexxString::strictGreaterOrEqual,
                              /* Duplicate entry neccessary        */
   (PCPPM)&RexxString::strictGreaterOrEqual,
   (PCPPM)&RexxString::strictLessOrEqual,
   (PCPPM)&RexxString::notEqual,
   (PCPPM)&RexxString::notEqual, /* Duplicate entry neccessary        */
   (PCPPM)&RexxString::andOp,
   (PCPPM)&RexxString::orOp,
   (PCPPM)&RexxString::xorOp,
   (PCPPM)&RexxString::operatorNot,
};


