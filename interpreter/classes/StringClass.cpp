/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/*                                                                            */
/* The Rexx STRING object class                                               */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "ProtectedObject.hpp"
#include "StringUtil.hpp"
#include "CompoundVariableTail.hpp"
#include "SystemInterpreter.hpp"
#include "MethodArguments.hpp"
#include "NumberStringClass.hpp"
#include "DirectoryClass.hpp"

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <limits>
#include <cmath>
#include <cfloat>

// singleton class instance
RexxClass *RexxString::classInstance = OREF_NULL;


// character validation sets for the datatype function
const char *RexxString::DIGITS_HEX   = "0123456789ABCDEFabcdef";
const char *RexxString::ALPHANUM     = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const char *RexxString::DIGITS_BIN   = "01";
const char *RexxString::LOWER_ALPHA  = "abcdefghijklmnopqrstuvwxyz";
const char *RexxString::MIXED_ALPHA  = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *RexxString::UPPER_ALPHA  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *RexxString::DIGITS_BASE64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
https://en.wikipedia.org/wiki/Regular_expression#Character_classes
POSIX character ranges returned by XRANGE() and String methods
[:alnum:] [A-Za-z0-9]   Alphanumeric characters
[:alpha:] [A-Za-z]      Alphabetic characters
[:blank:] [ \t]         Space and tab
[:cntrl:] [\x00-\x1F\x7F]       Control characters
[:digit:] [0-9]         Digits
[:graph:] [\x21-\x7E]   Visible characters
[:lower:] [a-z]         Lowercase letters
[:print:] [\x20-\x7E]   Visible characters and the space character
[:punct:] [][!"#$%&'()*+,./:;<=>?@\^_`{|}~-]    Punctuation characters
[:space:] [ \t\r\n\v\f]         Whitespace characters
[:upper:] [A-Z]         Uppercase letters
[:xdigit:] [A-Fa-f0-9]  Hexadecimal digits
*/
// the character ranges are returned in ascending byte order
const char *RexxString::ALNUM = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char *RexxString::ALPHA = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char *RexxString::BLANK = "\t ";
const char *RexxString::CNTRL = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f";
const char *RexxString::DIGIT = "0123456789";
const char *RexxString::GRAPH = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
const char *RexxString::LOWER = RexxString::LOWER_ALPHA;
const char *RexxString::PRINT = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
const char *RexxString::PUNCT = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
const char *RexxString::SPACE = "\t\n\v\f\r ";
const char *RexxString::UPPER = RexxString::UPPER_ALPHA;
const char *RexxString::XDIGIT = "0123456789ABCDEFabcdef";

const char RexxString::ch_PLUS='+';
const char RexxString::ch_MINUS='-';
const char RexxString::ch_PERIOD='.';
const char RexxString::ch_ZERO='0';
const char RexxString::ch_ONE='1';
const char RexxString::ch_FIVE='5';
const char RexxString::ch_NINE='9';

// Mapped character validation sets allow direct lookup.
// We dynamically create mapped sets from their unmapped
// string counterparts here as a one-time setup only.
RexxString::lookupInit RexxString::lookupInitializer;

char RexxString::DIGITS_HEX_LOOKUP[256];
char RexxString::DIGITS_BASE64_LOOKUP[256];
char RexxString::DIGITS_BIN_LOOKUP[256];
char RexxString::ALPHANUM_LOOKUP[256];
char RexxString::LOWER_ALPHA_LOOKUP[256];
char RexxString::MIXED_ALPHA_LOOKUP[256];
char RexxString::UPPER_ALPHA_LOOKUP[256];

/**
 * Convert a validation set string to its mapped form.
 * The mapped form is a 256-byte lookup table with the character as its
 * index (offset) and a value of 0xff signalling an invalid character.
 * All other values are valid characters.
 * The map uses increasing values starting with 0, as this is required
 * for DIGITS_BASE64_LOOKUP and it doesn't matter for others.
 * DIGITS_HEX_LOOKUP has special requirements due to the dual use of
 * lower- and uppercase letters.  See mappedHex() function.
 *
 * @param charSet   The validation string.
 * @param charMap   The returned mapped set.
 *
 * @return void.
 */
void mapped(const char* charSet, char charMap[256])
{
    memset(charMap, '\xff', 256);
    int i = 0;
    unsigned char c;
    while ((c = *charSet++) != '\0')
    {
        charMap[c] = i++;
    }
}

/**
 * Convert the DIGITS_HEX validation set string to its mapped form.
 * This is a special version due to the dual use of lower- and
 * uppercase hexadecimal letters.  See mapped() function for details.
 *
 * @param charSet   The validation string.
 * @param charMap   The returned mapped set.
 *
 * @return void.
 */
void mappedHex(const char* charSet, char charMap[256])
{
    memset(charMap, '\xff', 256);
    char c;
    while ((c = *charSet++) != '\0')
    {
        if (c >= '0' && c <= '9')
        {
            charMap[c] = c - '0';
        }
        else if (c >= 'A' && c <= 'F')
        {
            charMap[c] = c - 'A' + 10;
        }
        else if (c >= 'a' && c <= 'f')
        {
            charMap[c] = c - 'a' + 10;
        }
    }
}

RexxString::lookupInit::lookupInit()
{
    // create mapped character validation sets
    mappedHex(RexxString::DIGITS_HEX, RexxString::DIGITS_HEX_LOOKUP);
    mapped(RexxString::DIGITS_BASE64, RexxString::DIGITS_BASE64_LOOKUP);
    mapped(RexxString::DIGITS_BIN,    RexxString::DIGITS_BIN_LOOKUP);
    mapped(RexxString::ALPHANUM,      RexxString::ALPHANUM_LOOKUP);
    mapped(RexxString::LOWER_ALPHA,   RexxString::LOWER_ALPHA_LOOKUP);
    mapped(RexxString::MIXED_ALPHA,   RexxString::MIXED_ALPHA_LOOKUP);
    mapped(RexxString::UPPER_ALPHA,   RexxString::UPPER_ALPHA_LOOKUP);
}

/**
 * Create initial class object at bootstrap time.
 */
void RexxString::createInstance()
{
    CLASS_CREATE(String);
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
 * Flatten the object contents as part of a saved program.
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
RexxInternalObject *RexxString::unflatten(Envelope *envelope)
{
    // if this has been proxied, then retrieve our target object from the environment
    if (isProxyObject())
    {
        // we have a couple of special cases, everything else should be a core class lookup.
        if (strCompare("NIL"))
        {
            return TheNilObject;
        }
        else if (strCompare("ENVIRONMENT"))
        {
            return TheEnvironment;
        }
        else
        {
            // must be a reference to a core class
            return TheRexxPackage->findClass(this);
        }
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
    if (isBaseClass())
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
bool RexxString::numberValue(wholenumber_t &result, wholenumber_t digits)
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
bool RexxString::unsignedNumberValue(size_t &result, wholenumber_t digits)
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
bool RexxString::unsignedNumberValue(size_t &result)
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
        if (!std::isnan(result))
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


/**
 * Primitive strict equal\not equal method.  This determines
 * only strict equality, not greater or less than values.
 *
 * @param otherObj The other object for comparison.
 *
 * @return true if the objects are equal, false otherwise.
 */
bool RexxString::isEqual(RexxInternalObject *otherObj)
{
    // if not a primitive, we need to go the full == message route.
    if (!isBaseClass())
    {
        ProtectedObject result;
        return sendMessage(GlobalNames::STRICT_EQUAL, (RexxObject *)otherObj, result)->truthValue(Error_Logical_value_method);
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
    return memcmp(getStringData(), other->getStringData(), otherLen) == 0;
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
    return memcmp(getStringData(), other->getStringData(), otherLen) == 0;
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
    size_t otherLen = other->getLength();
    // can't compare equal if different lengths
    if (otherLen != getLength())
    {
        return false;
    }
    // do the actual string compare
    return StringUtil::caselessCompare(getStringData(), other->getStringData(), otherLen) == 0;
}


/**
 * Wrapper around the compareTo() method for doing sort
 * comparisons of strings.
 *
 * @param other  The other comparison object
 *
 * @return -1, 0, 1 depending on the comparison result.
 */
wholenumber_t RexxString::compareTo(RexxInternalObject *other )
{
    if (isBaseClass())
    {
        // there should be a faster version of this...
        return primitiveCompareTo(stringArgument((RexxObject *)other, ARG_ONE));
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
        return firstNum->comp(secondNum, number_fuzz());
    }

    // we're doing a string comparison, so get the string version of the other.
    return stringComp(other->requestString());
}


/**
 * Do a value comparison of two strings for the non-strict
 * comparisons.  This returns for the compares:
 *
 *    a value < 0 when this is smaller than other
 *    a value   0 when this is equal to other
 *    a value > 0 when this is larger than other
 *
 *    NOTE:  This version does not do the numeric portion of the
 *    compare, just the string portion.  This allows
 *    numberstring to directly perform a string comparison
 *    without reattempting numeric conversions on the values.
 *
 * @param other  The object we compare against.
 *
 * @return the relative compare value (<0, 0, or >0)
 */
wholenumber_t RexxString::stringComp(RexxString *second)
{
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

    // done differently depnding on which string is longer
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


/**
 * Do a strict comparison of two strings when called by either
 * the Interger or NumberString class. This returns:
 *
 *    a value < 0 when this is smaller than other
 *    a value   0 when this is equal to other
 *    a value > 0 when this is larger than other
 *
 * @param otherObj The other comparison object.
 *
 * @return The relative comparison result (<0, 0, >0)
 */
wholenumber_t RexxString::primitiveStrictComp(RexxObject *otherObj)
{
    wholenumber_t result;

    // get the string argument and the data/length values
    RexxString *other = otherObj->requestString();
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
 * The String class version of the modulo method.
 *
 * @param divisor The divisor.
 *
 * @return The module result.
 */
RexxObject *RexxString::modulo(RexxObject *divisor)
{
    ArithmeticMethod(modulo(divisor), "MODULO");
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
{
    CompareOperator(comp(other) == 0);
}


RexxObject *RexxString::notEqual(RexxObject *other)
{
    // this one returns true for .nil
    if (other == TheNilObject)
    {
        return TheTrueObject;
    }
    return booleanObject(comp(other) != 0);
}


RexxObject *RexxString::isGreaterThan(RexxObject *other)
{
    CompareOperator(comp(other) > 0);
}


RexxObject *RexxString::isLessThan(RexxObject *other)
{
    CompareOperator(comp(other) < 0);
}


RexxObject *RexxString::isGreaterOrEqual(RexxObject *other)
{
    CompareOperator(comp(other) >= 0);
}


RexxObject *RexxString::isLessOrEqual(RexxObject *other)
{
    CompareOperator(comp(other) <= 0);
}


RexxObject *RexxString::strictGreaterThan(RexxObject *other)
{
    CompareOperator(primitiveStrictComp(other) > 0);
}


RexxObject *RexxString::strictLessThan(RexxObject *other)
{
    CompareOperator(primitiveStrictComp(other) < 0);
}


RexxObject *RexxString::strictGreaterOrEqual(RexxObject *other)
{
    CompareOperator(primitiveStrictComp(other) >= 0);
}


RexxObject *RexxString::strictLessOrEqual(RexxObject *other)
{
    CompareOperator(primitiveStrictComp(other) <= 0);
}


/**
 * Concatenate two strings together.
 *
 * @param other  the other string.
 *
 * @return The concatenation result.
 */
RexxString *RexxString::concat(RexxString *other)
{
    size_t len1 = getLength();
    size_t len2 = other->getLength();

    // if either length is zero, we can avoid creating a new object by
    // just returning the other string value, since strings are immutable.
    if (len2 == 0)
    {
        return this;
    }

    if (len1 == 0)
    {
        return other;
    }

    // create a new string to build the result.
    RexxString *result = raw_string(len1+len2);
    StringBuilder builder(result);

    // just append the two string lengths
    builder.append(getStringData(), len1);
    builder.append(other->getStringData(), len2);
    return result;
}


/**
 * Rexx level concatenate...requires conversion and checking
 *
 * @param otherObj The other object for concatenation.
 *
 * @return The concatenation result.
 */
RexxString *RexxString::concatRexx(RexxObject *otherObj)
{
    requiredArgument(otherObj, ARG_ONE);
    RexxString *other = otherObj->requestString();

    // the following logic also appears in the primitive
    // level concatenation, but since this is such a highly
    // used function, it is repeated here.
    size_t len1 = getLength();
    size_t len2 = other->getLength();

    // if either length is zero, we can avoid creating a new object by
    // just returning the other string value, since strings are immutable.
    if (len2 == 0)
    {
        return this;
    }

    if (len1 == 0)
    {
        return other;
    }

    RexxString *result = raw_string(len1+len2);
    StringBuilder builder(result);

    // just append the two string lengths
    builder.append(getStringData(), len1);
    builder.append(other->getStringData(), len2);
    return result;
}


/**
 * Concatenate a string object onto an ASCII-Z string
 *
 * @param other  The ASCII-Z string we're concatenating with.
 *               Note, these are used internally, so we're
 *               assuming neither part is a null string.
 *
 * @return The concatenate result.
 */
RexxString *RexxString::concatToCstring(const char *other)
{
    size_t len1 = getLength();
    size_t len2 = strlen(other);

    RexxString *result = raw_string(len1+len2);
    StringBuilder builder(result);

    builder.append(other, len2);
    builder.append(getStringData(), len1);
    return result;
}

/**
 * Concatenate an ASCII-Z string onto a string object
 *
 * @param other  The ASCII-Z string to concatenate.
 *
 * @return The concatenated string.
 */
RexxString *RexxString::concatWithCstring(const char *other)
{
    size_t len1 = getLength();
    size_t len2 = strlen(other);

    RexxString *result = raw_string(len1+len2);
    StringBuilder builder(result);

    builder.append(getStringData(), len1);
    builder.append(other, len2);
    return result;
}


/**
 * Concatenate two strings with a blank in between
 *
 * @param otherObj The other string.
 *
 * @return The concatenated strings.
 */
RexxString *RexxString::concatBlank(RexxObject *otherObj)
{
    // note, this version does full string conversions, including the NOSTRING method.
    requiredArgument(otherObj, ARG_ONE);
    RexxString *other = otherObj->requestString();

    size_t len1 = getLength();
    size_t len2 = other->getLength();

    // get a new string of the required size
    RexxString *result = raw_string(len1+len2+1);
    StringBuilder builder(result);

    builder.append(getStringData(), len1);
    // add the blank between
    builder.append(' ');
    builder.append(other->getStringData(), len2);
    return result;
}


/**
 * Determine the truth value of a string object, raising the
 * given error if bad.
 *
 * @param errorCode The error code for bad booleans.
 *
 * @return The true or false value of valid strings.
 */
bool RexxString::truthValue(RexxErrorCodes errorCode)
{
    // get a real string value
    RexxString *testString = baseString();

    // a valid boolean must be one character long
    if (testString->getLength() != 1)
    {
        reportException(errorCode, testString);
    }
    // the only valid values are '1' and '0'.
    if (*(testString->getStringData()) == '0')
    {
        return false;
    }
    else if (!(*(testString->getStringData()) == '1'))
    {
        reportException(errorCode, this);
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
bool RexxString::logicalValue(logical_t &result)
{
    // get a real string value
    RexxString *testString = baseString();

    // wrong size is a convertion error
    if (testString->getLength() != 1)
    {
        return false;
    }

    if (testString->getChar(0) == '0')/* exactly '0'?                      */
    {
        result = false;
        return true;
    }

    else if (testString->getChar(0) == '1')
    {
        result = true;                   // this is true and the conversion worked
        return true;
    }
    return false;       // did not convert correctly
}


/**
 * Tests for existence of lowercase characters
 *
 * @return true if this string contains lowercase characters, false otherwise.
 */
bool RexxString::checkLower()
{
    const char *data = getStringData();
    const char *endData = data + getLength();

    // loop until we find a lower case character
    while (data < endData)
    {
        // if we have a lower case character, mark as having lower
        // case characters and return
        if (Utilities::isLower(*data))
        {
            setHasLower();
            return true;
        }
        data++;
    }
    // we can mark this as having no lower case characters
    setUpperOnly();
    return false;
}


/**
 * Tests for existence of uppercase characters
 *
 * @return true if this string contains uppercase characters,
 *         false otherwise.
 */
bool RexxString::checkUpper()
{
    const char *data = getStringData();
    const char *endData = data + getLength();

    // loop until we find a lower case character
    while (data < endData)
    {
        // if we have a upper case character, mark as having lower
        // case characters and return
        if (Utilities::isUpper(*data))
        {
            setHasUpper();
            return true;
        }
        data++;
    }
    // we can mark this as having no upper case characters
    setLowerOnly();
    return false;
}


/**
 * Translate a string to uppercase...will only create a new
 * string if characters actually have to be translated.
 *
 * @return The string in uppercase.
 */
RexxString *RexxString::upper()
{
    // it is always worth avoiding creating a new object, so if necessary, we'll
    // scan to see if the string contains lowercase characters.
    if (!upperOnly() && (hasLower() || checkLower()))
    {
        RexxString *newstring = raw_string(getLength());
        const char *data = getStringData();

        char *outdata = newstring->getWritableData();
        const char *endData = data + getLength();

        // copy the data over, uppercasing as we go.
        while (data < endData)
        {
            *outdata++ = Utilities::toUpper(*data++);
        }
        // we know this string does not contain lowercase characters
        newstring->setUpperOnly();
        return newstring;
    }

    // known to be uppercase, so we can just return the original string.
    return this;
}


/**
 * Translate a string to "traceable" form, removing non-displayable
 * characters
 *
 * @return The translated string.
 */
RexxString *RexxString::stringTrace()
{
    // NOTE:  since we're doing value comparisons on single character values here,
    // we need to process this as unsigned characters to handle values
    // greater than 0x7f.
    size_t i = getLength();
    const unsigned char *current = (const unsigned char *)getStringData();
    bool nonDisplay = false;

    // loop through, breaking on the first non-displayable character
    for (; i > 0; i--, current++)
    {
        if (*current < ch_SPACE && *current != ch_TAB)
        {
            nonDisplay = true;
            break;
        }
    }

    // no translation required, we can just return this.
    if (!nonDisplay)
    {
        return this;
    }

    RexxString *newCopy = (RexxString *)copy();
    i = newCopy->getLength();
    char *outptr = newCopy->getWritableData();

    // now loop again translating all of the non-displayables
    for (; i > 0; i--)
    {
        // we don't translate tabs either...all other
        // non-displayables become question marks.
        if (*outptr < ch_SPACE && *outptr != ch_TAB)
        {
            *outptr = '?';
        }
        outptr++;
    }
    return newCopy;
}


/**
 * Translate a string to lower case
 *
 * @return The translated string.
 */
RexxString *RexxString::lower()
{
    // it is always worth avoiding creating a new object, so if necessary, we'll
    // scan to see if the string contains uppercase characters.
    if (!lowerOnly() && (hasUpper() || checkUpper()))
    {

        RexxString *newstring = raw_string(getLength());
        const char *data = getStringData();
        const char *endData = data + getLength();
        char *outdata = newstring->getWritableData();

        // copy the data over, lowercasing as we go.
        while (data < endData)
        {
            *outdata++ = Utilities::toLower(*data++);
        }
        // we know this string does not contain uppercase characters
        newstring->setLowerOnly();
        return newstring;
    }
    // nothing to translate, just return the original string
    return this;
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

    rangeLength = std::min(rangeLength, getLength() - startPos);

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

    rangeLength = std::min(rangeLength, getLength() - startPos);

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
    // get a copy of the string...NOTE: we don't use copy because
    // we don't want to copy the flag settings
    RexxString *newstring = extract(0, getLength());

    char *data = newstring->getWritableData() + offset;
    // now lowercase in place
    for (size_t i = 0; i < _length; i++)
    {
        *data = Utilities::toLower(*data);
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
    // get a copy of the string...NOTE: we don't use copy because
    // we don't want to copy the flag settings
    RexxString *newstring = extract(0, getLength());

    char *data = newstring->getWritableData() + offset;
    // now uppercase in place
    for (size_t i = 0; i < _length; i++)
    {
        *data = Utilities::toUpper(*data);
        data++;
    }
    return newstring;
}


/**
 * Convert a string object to an integer.  Returns .nil for
 * failures.
 *
 * @param digits The digits value for the conversion.
 *
 * @return An integer version of this string.
 */
RexxInteger *RexxString::integerValue(wholenumber_t digits)
{
    // try to convert to a number string value...if that converts,
    // try to get an integer object from that.
    NumberString *numberStr = numberString();

    if ((numberStr = numberString()) != OREF_NULL )
    {
        RexxInteger *newInteger = numberStr->integerValue(digits);
        // if it converted, and the converted integer does not already
        // have a string value, set ours
        if (newInteger != TheNilObject && newInteger->getStringrep() == OREF_NULL)
        {
            newInteger->setString(this);
        }
        return newInteger;
    }
    // .nil is returned for all conversion failures.
    else
    {
        return(RexxInteger *)TheNilObject;
    }
}


/**
 * Attach a NumberString value to this string object for
 * arithmetic optimization.
 *
 * @param numberRep The number representation.
 */
void RexxString::setNumberString(NumberString *numberRep)
{

    setField(numberStringValue, numberRep);

    // if we have a number string, we need to turn on the
    // references settings.  If this is NULL, then we no longer
    // have references.
    if (numberRep != OREF_NULL)
    {
        setHasReferences();
    }
    else
    {
        setHasNoReferences();
    }
    return;
}


/**
 * Concatenate two strings with a single character between
 *
 * @param other   The other string.
 * @param between The separator character.
 *
 * @return The concatenated string result.
 */
RexxString *RexxString::concatWith(RexxString *other, char between)
{
    size_t len1 = getLength();
    size_t len2 = other->getLength();

    RexxString *result = raw_string(len1+len2+1);
    StringBuilder builder(result);

    // copy the first string data (if any)
    builder.append(getStringData(), len1);
    // insert the separator character
    builder.append(between);
    // and copy the tail part
    builder.append(other->getStringData(), len2);
    return result;
}


/**
 * Logical AND of a string with another logical value
 *
 * @param other  The other value for the AND.
 *
 * @return Either .true or .false
 */
RexxObject *RexxString::andOp(RexxObject *other)
{
    requiredArgument(other, ARG_ONE);

    bool otherTruth = other->truthValue(Error_Logical_value_method);
    // perform the operation
    return booleanObject(truthValue(Error_Logical_value_method) && otherTruth);
}


/**
 * Logical OR of a string with another logical value
 *
 * @param other  The other OR object.
 *
 * @return Either .true or .false based on the logical result.
 */
RexxObject *RexxString::orOp(RexxObject *other)
{
    requiredArgument(other, ARG_ONE);

    bool otherTruth = other->truthValue(Error_Logical_value_method);

    return booleanObject(truthValue(Error_Logical_value_method) || otherTruth);
}


/**
 * Logical XOR of a string with another logical value
 *
 * @param other  The other value in the logical operation.
 *
 * @return the XOR result.
 */
RexxObject *RexxString::xorOp(RexxObject *other)
{
    requiredArgument(other, ARG_ONE);

    bool otherTruth = other->truthValue(Error_Logical_value_method);

    // != in C++ is essentially an exclusive OR.
    return booleanObject(truthValue(Error_Logical_value_method) != otherTruth);
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
RexxObject *RexxString::choiceRexx(RexxObject *trueResult, RexxObject *falseResult)
{
    requiredArgument(trueResult, "true value");
    requiredArgument(falseResult, "false value");

    return truthValue(Error_Logical_value_method) ? trueResult : falseResult;
}



/**
 * Split string into an array
 *
 * @param div    The separator character.
 *
 * @return An array if the string segments.
 */
ArrayClass *RexxString::makeArrayRexx(RexxString *div)
{
    // same code is used for both string and mutablebuffer.
    return StringUtil::makearray(getStringData(), getLength(), div);
}


/**
 * Logical NOT of a string
 *
 * @return The logical not of the string truth value.
 */
RexxObject *RexxString::notOp()
{
    return booleanObject(!truthValue(Error_Logical_value_method));
}


/**
 * Logical NOT of a string.  This is the operator version, which
 * needs to take an argument that is not used.
 *
 * @param other  The other value (which will be OREF_NULL)
 *
 * @return The not of the string logical value.
 */
RexxObject *RexxString::operatorNot(RexxObject *other)
{
    return booleanObject(!truthValue(Error_Logical_value_method));
}


/**
 * Polymorphic method that makes string a polymorphic expression
 * term for string literals.
 *
 * @param context The current execution context.
 * @param stack   The current expression stack.
 *
 * @return The string value.
 */
RexxObject *RexxString::evaluate(RexxActivation *context, ExpressionStack *stack)
{
    // place the string on the evaluation stack.
    stack->push(this);
    // trace if necessary
    context->traceIntermediate(this, RexxActivation::TRACE_PREFIX_LITERAL);
    return this;
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


/**
 * Polymorphic get_value function used with expression terms
 *
 * @param context The execution context.
 *
 * @return The string object.
 */
RexxObject  *RexxString::getValue(RexxActivation *context)
{
    // strings are their own value
    return this;
}


/**
 * Polymorphic get_value function used with expression terms
 *
 * @param context The method dictionary context.
 *
 * @return The string value.
 */
RexxObject  *RexxString::getValue(VariableDictionary *context)
{
    // strings evaluate to their own value.
    return this;
}


/**
 * Polymorphic get_value function used with expression terms
 *
 * @param context The current exection context.
 *
 * @return The string value.
 */
RexxObject  *RexxString::getRealValue(RexxActivation *context)
{
    // strings evaluate to their own value.
    return this;
}


/**
 * Polymorphic get_value function used with expression terms
 *
 * @param context The method dictionary context.
 *
 * @return The string value.
 */
RexxObject  *RexxString::getRealValue(VariableDictionary *context)
{
    return this;
}


RexxString *RexxString::newString(const char *string, size_t length)
/******************************************************************************/
/* Function:  Allocate (and initialize) a string object                       */
/******************************************************************************/
{
    // these are variable size objects.  We make sure we give some additional space
    // for a terminating null character.
    size_t size2 = sizeof(RexxString) - (sizeof(char) * 3) + length;
    RexxString *newObj = (RexxString *)new_object(size2, T_String);

    // initialize the string fields
    newObj->setLength(length);
    newObj->hashValue = 0;               // make sure the hash value is zeroed

    // add the terminating null
    newObj->putChar(length, '\0');
    // and copy the the string value
    newObj->put(0, string, length);
    // by  default, we don't need live marking.
    newObj->setHasNoReferences();
    return newObj;
}


/**
 * Allocate and initialize a new string object from a
 * pair of data buffer.
 *
 * @param s1     Pointer to the first data buffer
 * @param l1     length of the first buffer
 * @param s2     pointer to the second buffer
 * @param l2     length of the second buffer
 *
 * @return A string object that is the concatenation of the two buffers.
 */
RexxString *RexxString::newString(const char *s1, size_t l1, const char *s2, size_t l2)
{
    // these are variable size objects.  We make sure we give some additional space
    // for a terminating null character.

    size_t size = l1 + l2;
    size_t size2 = sizeof(RexxString) - (sizeof(char) * 3) + size;
    RexxString *newObj = (RexxString *)new_object(size2, T_String);

    // initialize the string fields
    newObj->setLength(size);
    newObj->hashValue = 0;               // make sure the hash value is zeroed

    // add the terminating null
    newObj->putChar(size, '\0');
    // and copy the first string value
    newObj->put(0, s1, l1);
    // and the second
    newObj->put(l1, s2, l2);
    // by  default, we don't need live marking.
    newObj->setHasNoReferences();
    return newObj;
}


/**
 * Allocate (and initialize) an empty string object
 *
 * @param length The required lenth
 *
 * @return A raw string object (no data initialization)
 */
RexxString *RexxString::rawString(size_t length)
{
    // this is idential to newString(), except for copying the data

    // these are variable size objects.  We make sure we give some additional space
    // for a terminating null character.
    size_t size2 = sizeof(RexxString) - (sizeof(char) * 3) + length;
    RexxString *newObj = (RexxString *)new_object(size2, T_String);

    // initialize the string fields
    newObj->setLength(length);
    newObj->hashValue = 0;               // make sure the hash value is zeroed

    // add the terminating null
    newObj->putChar(length, '\0');
    // by  default, we don't need live marking.
    newObj->setHasNoReferences();
    return newObj;
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
RexxString *RexxString::newUpperString(const char * string, size_t length)
{
    // these are variable size objects.  We make sure we give some additional space
    // for a terminating null character.
    size_t size2 = sizeof(RexxString) - (sizeof(char) * 3) + length;
    RexxString *newObj = (RexxString *)new_object(size2, T_String);

    // initialize the string fields
    newObj->setLength(length);
    newObj->hashValue = 0;               // make sure the hash value is zeroed

    char *outdata = newObj->getWritableData();
    // set the input markers
    const char *indata = string;
    const char *endData = indata + length;

    while (indata < endData)
    {
        *outdata++ = Utilities::toUpper(*indata++);
    }
    // flag as containing only uppercase characters
    newObj->setUpperOnly();

    // add the terminating null
    newObj->putChar(length, '\0');
    // by  default, we don't need live marking.
    newObj->setHasNoReferences();
    return newObj;
}


/**
 * Create a string from a double value
 *
 * @param number The starting number.
 *
 * @return A string version of a double value.
 */
RexxString *RexxString::newString(double number)
{
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
RexxString *RexxString::newString(double number, size_t precision)
{
    return new_numberstringFromDouble(number, precision)->stringValue();
}


/**
 * Create a proxy object from this string
 *
 * @param string The name of the proxy.
 *
 * @return A string marked as a proxy object.
 */
RexxString *RexxString::newProxy(const char *string)
{
    RexxString *sref = new_string(string);
    // mark as a proxy and return.
    sref->makeProxyObject();

    return sref;
}


/**
 * Create a new string value (used primarily for subclasses)
 *
 * @param init_args The standard new args.
 * @param argCount  The argument count.
 *
 * @return A new string object of the current class.
 */
RexxString *RexxString::newRexx(RexxObject **init_args, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // break up the arguments
    RexxObject *stringObj;
    RexxClass::processNewArgs(init_args, argCount, init_args, argCount, 1, stringObj, NULL);
    // force argument to string value
    RexxString *string = stringArgument(stringObj, ARG_ONE);
    // we can't use this value directly because we will adjust the
    // class to the target class.  So we create a new string from the data
    // that we can alter.
    string = new_string(string->getStringData(), string->getLength());
    ProtectedObject p(string);

    // handle Rexx class completion
    classThis->completeNewObject(string, init_args, argCount);
    return string;
}


// operator table used for fast evaluation.
PCPPM RexxString::operatorMethods[] =
{
   NULL,                               // first entry not used
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


