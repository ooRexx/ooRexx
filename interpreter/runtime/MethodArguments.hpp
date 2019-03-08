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
/* utility functions for argument validation.                                 */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Globally required include files                                            */
/******************************************************************************/
#ifndef MethodArguments_Included
#define MethodArguments_Included

#include "ActivityManager.hpp"

/******************************************************************************/
/* Defines for argument error reporting                                       */
/******************************************************************************/

const int ARG_ONE    = 1;
const int ARG_TWO    = 2;
const int ARG_THREE  = 3;
const int ARG_FOUR   = 4;
const int ARG_FIVE   = 5;
const int ARG_SIX    = 6;
const int ARG_SEVEN  = 7;
const int ARG_EIGHT  = 8;
const int ARG_NINE   = 9;
const int ARG_TEN    = 10;


/**
 * Report an error with a missing argument.
 *
 * @param argumentPosition
 *               The argument position.
 */
inline void missingArgument(size_t argumentPosition)
{
    reportException(Error_Incorrect_method_noarg, argumentPosition);
}


/**
 * Report an error with a missing argument.
 *
 * @param argumentPosition
 *               The argument position.
 */
inline void missingArgument(const char *argumentPosition)
{
    reportException(Error_Invalid_argument_noarg, argumentPosition);
}


/**
 * Check for required arguments and raise a missing argument
 * error for the given position.
 *
 * @param object   The reference to check.
 * @param position the position of the argument for the error message.
 */
inline RexxObject *requiredArgument(RexxObject *object, size_t position)
{
    if (object == OREF_NULL)
    {
        missingArgument(position);
    }
    return object;
}


/**
 * Check for required arguments and raise a missing argument
 * error for the given position.
 *
 * @param object   The reference to check.
 * @param position the position of the argument for the error message.
 */
inline RexxObject *requiredArgument(RexxObject *object, const char *position)
{
    if (object == OREF_NULL)
    {
        missingArgument(position);
    }
    return object;
}


/**
 * REQUEST a STRING needed as a method
 * argument.  This raises an error if the object cannot be converted to a
 * string value.
 *
 * @param object   The object argument to check.
 * @param position The argument position, used for any error messages.
 *
 * @return The String value of the object, if it really has a string value.
 */
inline RexxString *stringArgument(RexxObject *object, size_t position)
{
    if (object == OREF_NULL)
    {
        missingArgument(position);
    }
    return object->requiredString(position);
}


/**
 * REQUEST a STRING needed as a method
 * argument.  This raises an error if the object cannot be converted to a
 * string value.
 *
 * @param object The object to check.
 * @param name   The parameter name of the argument (used for error reporting.)
 *
 * @return The string value of the object if it truely has a string value.
 */
inline RexxString *stringArgument(RexxObject *object, const char *name)
{
    if (object == OREF_NULL)
    {
        reportException(Error_Invalid_argument_noarg, name);
    }

    return object->requiredString(name);
}


/**
 * handle an optional string argument where a default argument value is provided.
 *
 * @param o      The argument object to test.
 * @param d      The default argument value (can be null)
 * @param p      The argument position.
 *
 * @return The argument value, in string form.
 */
inline RexxString *optionalStringArgument(RexxObject *o, RexxString *d, size_t p)
{
    return (o == OREF_NULL ? d : stringArgument(o, p));
}




/**
 * handle an optional string argument where a default argument value is provided.
 *
 * @param o      The argument object to test.
 * @param d      The default argument value (can be null)
 * @param p      The argument position.
 *
 * @return The argument value, in string form.
 */
inline RexxString* optionalStringArgument(RexxObject *o, RexxString *d, const char *p)
{
    return (o == OREF_NULL ? d : stringArgument(o, p));
}


/**
 * Take in an agument passed to a method, convert it to a
 * numeric object. If the argument is omitted, an error is
 * raised.
 *
 * @param argument The argument reference to test.
 * @param position The position of the argument (used for error reporting.)
 *
 * @return The argument converted to an integer value.
 */
wholenumber_t numberArgument(RexxObject *argument, size_t position);


/**
 * Take in an agument passed to a method, convert it to a
 * numeric object. If the argument is omitted, an error is
 * raised.
 *
 * @param argument The argument reference to test.
 * @param position The position of the argument (used for error reporting.)
 *
 * @return The argument converted to an integer value.
 */
wholenumber_t numberArgument(RexxObject *argument, const char *position);


/**
 * Parse an optional whole number method argument.  this must be
 * a whole number.  Raises a number if the argument not a
 * numeric value.
 *
 * @param o      The object to check.
 * @param d      The default value to return if the argument was omitted.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
inline wholenumber_t optionalNumberArgument(RexxObject *o, wholenumber_t d, size_t p)
{
    return (o == OREF_NULL ? d : numberArgument(o, p));
}


/**
 * Parse an optional whole number method argument.  this must be
 * a whole number.  Raises a number if the argument not a
 * numeric value.
 *
 * @param o      The object to check.
 * @param d      The default value to return if the argument was omitted.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
inline wholenumber_t optionalNumberArgument(RexxObject *o, wholenumber_t d, const char *p)
{
    return (o == OREF_NULL ? d : numberArgument(o, p));
}


/**
 * Parse a length method argument.  this must be a non-negative
 * whole number.  Raises a number if the argument was omitted or
 * is not a length numeric value.
 *
 * @param o      The object to check.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
size_t lengthArgument(RexxObject *o, size_t p);


/**
 * Parse a length method argument.  this must be a non-negative
 * whole number.  Raises a number if the argument was omitted or
 * is not a length numeric value.
 *
 * @param o      The object to check.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
size_t lengthArgument(RexxObject *o, const char *p);


/**
 * Parse a non-negative method argument.  this must be a
 * non-negative whole number.  Raises a number if the argument
 * was omitted or is not a length numeric value.
 *
 * @param o      The object to check.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
size_t nonNegativeArgument(RexxObject *o, size_t p);


/**
 * Parse a non-negative method argument.  this must be a
 * non-negative whole number.  Raises a number if the argument
 * was omitted or is not a length numeric value.
 *
 * @param o      The object to check.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
size_t nonNegativeArgument(RexxObject *o, const char *p);


/**
 * Parse an optional length method argument.  this must be a
 * non-negative whole number.  Raises a number if the argument
 * not a length numeric value.
 *
 * @param o      The object to check.
 * @param d      The default value to return if the argument was omitted.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
inline size_t optionalLengthArgument(RexxObject *o, size_t d, size_t p)
{
    return (o == OREF_NULL ? d : lengthArgument(o, p));
}


/**
 * Parse an optional length method argument.  this must be a
 * non-negative whole number.  Raises a number if the argument
 * not a length numeric value.
 *
 * @param o      The object to check.
 * @param d      The default value to return if the argument was omitted.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
inline size_t optionalLengthArgument(RexxObject *o, size_t d, const char *p)
{
    return (o == OREF_NULL ? d : lengthArgument(o, p));
}


/**
 * Parse a position method argument.  this must be a positive
 * whole number.  Raises a number if the argument was omitted or
 * is not a length numeric value.
 *
 * @param o      The object to check.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
size_t positionArgument(RexxObject *o, size_t p);


/**
 * Parse a position method argument.  this must be a positive
 * whole number.  Raises a number if the argument was omitted or
 * is not a length numeric value.
 *
 * @param o      The object to check.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
size_t positionArgument(RexxObject *o, const char *p);


/**
 * Parse an optional position method argument.  this must be a
 * positive whole number.  Raises a number if the argument not a
 * length numeric value.
 *
 * @param o      The object to check.
 * @param d      The default value to return if the argument was omitted.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
inline size_t optionalPositionArgument(RexxObject *o, size_t d, size_t p)
{
    return (o == OREF_NULL ? d : positionArgument(o, p));
}


/**
 * Parse an optional position method argument.  this must be a
 * positive whole number.  Raises a number if the argument not a
 * length numeric value.
 *
 * @param o      The object to check.
 * @param d      The default value to return if the argument was omitted.
 * @param p      The argument position.
 *
 * @return The converted numeric value of the object.
 */
inline size_t optionalPositionArgument(RexxObject *o, size_t d, const char *p)
{
    return (o == OREF_NULL ? d : positionArgument(o, p));
}


/**
 * Parse a pad argument.  This must a string object and
 * only a single character long.
 *
 * @param o      The object argument to check.  Raises an error if this was
 *               omitted.
 * @param p      The argument position, for error reporting.
 *
 * @return The pad character from the argument string.
 */
char padArgument(RexxObject *o, size_t p);


/**
 * Parse a pad argument.  This must a string object and
 * only a single character long.
 *
 * @param o      The object argument to check.  Raises an error if this was
 *               omitted.
 * @param p      The argument position, for error reporting.
 *
 * @return The pad character from the argument string.
 */
char padArgument(RexxObject *o, const char *p);


/**
 * Parse an optional pad argument.  This must a string object
 * and only a single character long.
 *
 * @param o      The object argument to check.
 * @param d      The default pad character if the argument was omitted.
 * @param p      The argument position, for error reporting.
 *
 * @return The pad character from the argument string.
 */
inline char optionalPadArgument(RexxObject *o, char d, size_t p)
{
    return (o == OREF_NULL ? d : padArgument(o, p));
}


/**
 * Parse an optional pad argument.  This must a string object
 * and only a single character long.
 *
 * @param o      The object argument to check.
 * @param d      The default pad character if the argument was omitted.
 * @param p      The argument position, for error reporting.
 *
 * @return The pad character from the argument string.
 */
inline char optionalPadArgument(RexxObject *o, char d, const char *p)
{
    return (o == OREF_NULL ? d : padArgument(o, p));
}


/**
 * Parse an option argument.  This must be a non-zero length string.
 *
 * @param o      The object to check.
 * @param p      The argument position for error messages.
 *
 * @return The first character of the option string.
 */
char optionArgument(RexxObject *o, size_t p);


/**
 * Parse an option argument.  This must be a non-zero length string.
 *
 * @param o      The object to check.
 * @param p      The argument position for error messages.
 *
 * @return The first character of the option string.
 */
char optionArgument(RexxObject *o, const char *p);


/**
 * Parse an option argument.  This must be a non-zero length string.
 *
 * @param o      The object to check.
 * @param validOptions
 *               The list of valid option characters as an ASCII-Z string.
 * @param p      The argument position for error messages.
 *
 * @return The first character of the option string.
 */
char optionArgument(RexxObject *o, const char *validOptions, size_t p);


/**
 * Parse an option argument.  This must be a non-zero length string.
 *
 * @param o      The object to check.
 * @param validOptions
 *               The list of valid option characters as an ASCII-Z string.
 * @param p      The argument position for error messages.
 *
 * @return The first character of the option string.
 */
char optionArgument(RexxObject *o, const char *validOptions, const char *p);


/**
 * Validate that an array is non-space up to the last item
 * and contains nothing but string values.
 *
 * @param argArray The argument array.
 * @param position The argument name for error reporting.
 */
void stringArrayArgument(ArrayClass *argArray, const char *position);


/**
 * Parse an optional option argument.  This must be a non-zero
 * length string.
 *
 * @param o      The object to check.
 * @param d      The default option if this was an omitted argument.
 * @param p      The argument position for error messages.
 *
 * @return The first character of the option string.
 */
inline char optionalOptionArgument(RexxObject *o, char d, size_t p)
{
    return (o == OREF_NULL ? d : optionArgument(o, p));
}


/**
 * Parse an optional option argument.  This must be a non-zero
 * length string.
 *
 * @param o      The object to check.
 * @param d      The default option if this was an omitted argument.
 * @param p      The argument position for error messages.
 *
 * @return The first character of the option string.
 */
inline char optionalOptionArgument(RexxObject *o, char d, const char *p)
{
    return (o == OREF_NULL ? d : optionArgument(o, p));
}


/**
 * Parse an optional option argument.  This must be a non-zero
 * length string.
 *
 * @param o      The object to check.
 * @param v      The list of valid option characters.
 * @param d      The default option if this was an omitted argument.
 * @param p      The argument position for error messages.
 *
 * @return The first character of the option string.
 */
inline char optionalOptionArgument(RexxObject *o, const char *v, char d, size_t p)
{
    return (o == OREF_NULL ? d : optionArgument(o, v, p));
}


/**
 * Parse an optional option argument.  This must be a non-zero
 * length string.
 *
 * @param o      The object to check.
 * @param v      The list of valid option characters.
 * @param d      The default option if this was an omitted argument.
 * @param p      The argument position for error messages.
 *
 * @return The first character of the option string.
 */
inline char optionalOptionArgument(RexxObject *o, const char *v, char d, const char *p)
{
    return (o == OREF_NULL ? d : optionArgument(o, v, p));
}


/**
 * Handle an optional non-negative numeric option.
 *
 * @param o      The object to check.
 * @param d      The default value to return if the argument was omitted.
 * @param p      The argument position used for error reporting.
 *
 * @return The converted numeric value.
 */
inline size_t optionalNonNegative(RexxObject *o, size_t d, size_t p)
{
    return o == OREF_NULL ? d : nonNegativeArgument(o, p);
}


/**
 * Handle an optional non-negative numeric option.
 *
 * @param o      The object to check.
 * @param d      The default value to return if the argument was omitted.
 * @param p      The argument position used for error reporting.
 *
 * @return The converted numeric value.
 */
inline size_t optionalNonNegative(RexxObject *o, size_t d, const char *p)
{
    return o == OREF_NULL ? d : nonNegativeArgument(o, p);
}


/**
 * Handle an optional positive numeric option.
 *
 * @param o      The object to check.
 * @param d      The default value to return if the argument was omitted.
 * @param p      The argument position used for error reporting.
 *
 * @return The converted numeric value.
 */
inline size_t optionalPositive(RexxObject *o, size_t d, size_t p)
{
    return (o == OREF_NULL ? d : o->requiredPositive(p));
}


/**
 * Handle an optional positive numeric option.
 *
 * @param o      The object to check.
 * @param d      The default value to return if the argument was omitted.
 * @param p      The argument position used for error reporting.
 *
 * @return The converted numeric value.
 */
inline size_t optionalPositive(RexxObject *o, size_t d, const char *p)
{
    return (o == OREF_NULL ? d : o->requiredPositive(p));
}


/**
 * REQUEST an ARRAY needed as a method
 * argument.  This raises an error if the object cannot be converted to a
 * single dimensional array item.
 *
 * @param object   The argument object.
 * @param position The argument position (used for error reporting.)
 *
 * @return A converted single-dimension array.
 */
inline ArrayClass *arrayArgument(RexxObject *object, size_t position)
{
    // this is required.
    if (object == OREF_NULL)
    {
        missingArgument(position);
    }
    // force to array form
    ArrayClass *array = object->requestArray();
    // not an array or not single dimension?  Error!
    if (array == TheNilObject || array->isMultiDimensional())
    {
        reportException(Error_Execution_noarray, (RexxObject *)object);
    }
    return array;
}


/**
 * REQUEST an ARRAY needed as a method
 * argument.  This raises an error if the object cannot be converted to a
 * single dimensional array item.
 *
 * @param object   The argument object.
 * @param position The argument name (used for error reporting.)
 *
 * @return A converted single-dimension array.
 */
inline ArrayClass *arrayArgument(RexxObject *object, const char *name)
{
    if (object == OREF_NULL)
    {
        reportException(Error_Invalid_argument_noarg, name);
    }

    // get the array form and verify we got a single-dimension array back.
    ArrayClass *array = object->requestArray();
    if (array == TheNilObject || array->isMultiDimensional())
    {
        reportException(Error_Invalid_argument_noarray, name);
    }
    return array;
}


/**
 * Validate that an argument is an instance of a specific class.
 *
 * @param object The argument to test.
 * @param clazz  The class type to check it against.
 * @param name   The argument name for error reporting.
 */
inline void classArgument(RexxObject *object, RexxClass *clazz, const char *name)
{
    if (object == OREF_NULL)
    {
        reportException(Error_Invalid_argument_noarg, name);
    }
    // verify this is an instance of the class we require.
    if (!object->isInstanceOf(clazz))
    {
        reportException(Error_Invalid_argument_noclass, name, clazz->getId());
    }
}


/**
 * Handy function for situations where a NULL results needs
 * translation into .nil.
 *
 * @param o      The result value.
 *
 * @return Either the result object, or TheNilObject.
 */
inline RexxObject *resultOrNil(RexxInternalObject *o) { return o != OREF_NULL ? (RexxObject *)o : TheNilObject; }


/**
 * Handy method for transforming a boolean value into Rexx method
 * boolean return values (i.e., .true or .false).
 *
 * @param v      The boolean value.
 *
 * @return Either TheTrueObject or TheFalseObject, depending on
 *         the argument value.
 */
inline RexxObject *booleanObject(bool v) { return v ? TheTrueObject : TheFalseObject; }

#endif

