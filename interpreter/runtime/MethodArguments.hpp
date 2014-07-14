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
#define MethodArguments_Include



/**
 * Check for required arguments and raise a missing argument
 * error for the given position.
 *
 * @param object   The reference to check.
 * @param position the position of the argument for the error message.
 */
inline void requiredArgument(RexxInternalObject *object, size_t position)
{
    if (object == OREF_NULL)
    {
        missingArgument(position);
    }
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
inline RexxString * stringArgument(RexxInternalObject *object, size_t position)
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
inline RexxString *stringArgument(RexxInternalObject *object, const char *name)
{
    if (object == OREF_NULL)
    {
        reportException(Error_Invalid_argument_noarg, name);
    }

    return object->requiredString(name);
}

// handle an option string argument where a default argument value is provided.
inline RexxString *optionalStringArgument(RexxInternalObject *o, RexxString *d, size_t p)
{
    return (o == OREF_NULL ? d : stringArgument(o, p));
}


// handle an option string argument where a default argument value is provided.
inline RexxString *optionalStringArgument(RexxInternalObject *o, RexxString *d, const char *p)
{
    return (o == OREF_NULL ? d : stringArgument(o, p));
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
size_t lengthArgument(RexxInternalObject *o, size_t p);


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
inline size_t optionalLengthArgument(RexxInternalObject *o, size_t d, size_t p)
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
size_t positionArgument(RexxInternalObject *o, size_t p);


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
inline size_t optionalPositionArgument(RexxInternalObject *o, size_t d, size_t p)
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
char padArgument(RexxInternalObject *o, size_t p);



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
inline char optionalPadArgument(RexxInternalObject *o, char d, size_t p)
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
char optionArgument(RexxInternalObject *o, size_t p);


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
inline char optionalOptionArgument(RexxInternalObject *o, char d, size_t p)
{
    return (o == OREF_NULL ? d : optionArgument(o, p));
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
inline size_t optionalNonNegative(RexxInternalObject *o, size_t d, size_t p)
{
    return (o == OREF_NULL ? d : o->requiredNonNegative(p));
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
inline size_t optionalPositive(RexxInternalObject *o, size_t d, size_t p)
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
inline RexxArray *arrayArgument(RexxInternalObject *object, size_t position)
{
    // this is required.
    if (object == OREF_NULL)
    {
        missingArgument(position);
    }
    // force to array form
    RexxArray *array = object->requestArray();
    // not an array or not single dimension?  Error!
    if (array == TheNilObject || array->getDimension() != 1)
    {
        reportException(Error_Execution_noarray, object);
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
inline RexxArray * arrayArgument(RexxInternalObject *object, const char *name)
{
    if (object == OREF_NULL)
    {
        reportException(Error_Invalid_argument_noarg, name);
    }

    // get the array form and verify we got a single-dimension array back.
    RexxArray *array = object->requestArray();
    if (array == TheNilObject || array->getDimension() != 1)
    {
        /* raise an error                    */
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
inline void classArgument(RexxInternalObject *object, RexxClass *clazz, const char *name)
{
    if (object == OREF_NULL)             /* missing argument?                 */
    {
        reportException(Error_Invalid_argument_noarg, name);
    }

    if (!object->isInstanceOf(clazz))
    {
        reportException(Error_Invalid_argument_noclass, name, clazz->getId());
    }
}


/**
 * A function specifically for REQUESTing a STRING, since there are
 * four primitive classes that are equivalents for strings.  It will trap on
 * OREF_NULL.  This always returns a string value, going all the
 * way down the various methods of providing a string value.
 * Will also raise NOSTRING conditions.
 *
 * @param object The object we need a string value from.
 *
 * @return The string value of the object.
 */
inline RexxString *REQUEST_STRING(RexxInternalObject *object)
{
    return (isOfClass(String, object) ? (RexxString *)object : (object)->requestString());
}

/**
 * Request an array version for an argument.  Will perform
 * makearray processing on the object, if needed.
 *
 * @param obj    The object to request.
 *
 * @return The converted array value of the object or TheNilObject if
 *         if did not convert.
 */
inline RexxArray * REQUEST_ARRAY(RexxInternalObject *obj) { return ((obj)->requestArray()); }

/**
 * Request an object to be converted to a RexxInteger
 * object.  Return TheNilObject if it could not be converted.
 *
 * @param obj    The object to convert.
 *
 * @return An Integer object instance representing this object or
 *         .nil if it cannot be converted.
 */
inline RexxInteger * REQUEST_INTEGER(RexxInternalObject *obj) { return ((obj)->requestInteger(Numerics::ARGUMENT_DIGITS));}

#endif

