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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Expression Stack Class                                           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ExpressionStack.hpp"
#include "ActivityManager.hpp"


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void ExpressionStack::live(size_t liveMark)
{
    // mark all current entries on the stack.
    for (RexxInternalObject **entry = stack; entry <= top; entry++)
    {
        memory_mark(*entry);
    }
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void ExpressionStack::liveGeneral(MarkReason reason)
{
    // mark all current entries on the stack.
    for (RexxInternalObject **entry = stack; entry <= top; entry++)
    {
        memory_mark_general(*entry);
    }
}


/**
 * Migrate this expression stack to a new activity.
 *
 * @param activity The new activity.
 */
void ExpressionStack::migrate(Activity *activity)
{
    RexxInternalObject **oldFrame = stack;
    // allocate a new frame
    activity->allocateStackFrame(this, size);
    // copy the enties over to the new stack.
    memcpy(stack, oldFrame, sizeof(RexxInternalObject *) * size);
}


/**
 * Verify that a function has received all of its required
 * arguments, and did not receive extras.
 *
 * @param argcount The argument count.
 * @param min      The min count for this function.
 * @param max      The max count for this expression.
 * @param function The function name.
 */
void ExpressionStack::expandArgs(size_t argcount, size_t min, size_t max, const char *function )
{
    // handle checks for min and max argument counts
    if (argcount < min)
    {
        reportException(Error_Incorrect_call_minarg, function, min);
    }
    else if (argcount > max)
    {
        reportException(Error_Incorrect_call_maxarg, function, max);
    }
    else
    {
        // now check all of the arguments up to the min count
        // to verify none of them have been omitted.
        RexxInternalObject **current = pointer(argcount - 1);
        for (size_t i = min; i; i--)
        {
            // an omitted value?
            if (*current++ == OREF_NULL)
            {
                reportException(Error_Incorrect_call_noarg, function, min - i + 1);
            }
        }
    }
}


/**
 * Retrieve an object from the expression stack and potentially
 * convert it into a string argument.
 *
 * @param position The argument position.
 *
 * @return The argument String value.
 */
RexxString *ExpressionStack::requiredStringArg(size_t position)
{
    // get the argument from the stack.  If this is a true
    // string, just return directly.
    RexxInternalObject *argument = peek(position);
    if (isString(argument))
    {
        return (RexxString *)argument;
    }
    // get the string form, raising a NOSTRING condition if necessary
    RexxString *newStr = argument->requestString();
    // we replace the original stack object with the string value
    replace(position, newStr);
    return newStr;
}


/**
 * Retrieve an object from the expression stack and potentially
 * convert it into a string argument.
 *
 * @param position The argument position.
 *
 * @return The string version of this object.
 */
RexxString *ExpressionStack::optionalStringArg(size_t  position)
{
    // this is an optional argument, we just return the null value if it
    // isn't there.
    RexxInternalObject *argument = peek(position);
    if (argument == OREF_NULL)
    {
        return OREF_NULL;
    }

    // quick return if this is a string
    if (isString(argument))
    {
        return (RexxString *)argument;
    }

    // get the string form, raising a NOSTRING condition if necessary
    RexxString *newStr = argument->requestString();
    // we replace the original stack object with the string value
    replace(position, newStr);
    return newStr;
}


/**
 * Retrieve an object from the expression stack and potentially
 * convert it into an integer argument.
 *
 * @param position The argument position.
 * @param argcount The argument count (used for error reporting)
 * @param function The function name.
 *
 * @return The argument as an integer object.
 */
RexxInteger *ExpressionStack::requiredIntegerArg(size_t position,
     size_t argcount, const char *function)
{
    // if the argument is an integer already, this is a quick return.
    RexxInternalObject *argument = peek(position);
    if (isInteger(argument))
    {
        return (RexxInteger *)argument;
    }

    // convert to an integer value, give an error if it did not convert.
    wholenumber_t numberValue;
    if (!argument->requestNumber(numberValue, Numerics::ARGUMENT_DIGITS))
    {
        reportException(Error_Incorrect_call_whole, function, argcount - position, (RexxObject *)argument);
    }

    // replace the slot with the new value
    RexxInteger *newInt = new_integer(numberValue);
    replace(position, newInt);
    return newInt;
}




/**
 * Retrieve an object from the expression stack and potentially
 * convert it into an integer argument.
 *
 * @param position The argument position.
 * @param argcount The argument count (used for error reporting)
 * @param function The function name.
 *
 * @return The argument as an integer object.
 */
RexxInteger *ExpressionStack::optionalIntegerArg(size_t position, size_t argcount, const char *function)
{
    // this is optional, so we can return null.
    RexxInternalObject *argument = peek(position);
    if (argument == OREF_NULL)
    {
        return OREF_NULL;
    }
    if (isInteger(argument))
    {
        return(RexxInteger *)argument;
    }

    // convert to an integer value, give an error if it did not convert.
    wholenumber_t numberValue;
    if (!argument->requestNumber(numberValue, Numerics::ARGUMENT_DIGITS))
    {
        reportException(Error_Incorrect_call_whole, function, argcount - position, (RexxObject *)argument);
    }

    // replace the slot with the new value
    RexxInteger *newInt = new_integer(numberValue);
    replace(position, newInt);
    return newInt;
}


/**
 * Process an argument and ensure it is a valid integer
 * that can be expressed as a 64-bit value.
 *
 * @param position The argument position for any error messages.
 * @param argcount The number of arguments passed to the function.
 * @param function The function name
 *
 * @return An object that can be converted to a 64-bit value for
 *         pass-on to a native function.
 */
RexxObject *ExpressionStack::requiredBigIntegerArg(size_t position, size_t argcount, const char *function)
{
    RexxObject *argument = (RexxObject *)peek(position);
    // get this in the form of an object that is valid as a 64-bit integer, ready to
    // be passed along as an argument to native code.
    RexxObject *newArgument = Numerics::int64Object(argument);
    // returns a null value if it doesn't convert properly
    if (newArgument == OREF_NULL)
    {
        reportException(Error_Incorrect_call_whole, function, argcount - position, argument);
    }
    // replace original object on the stack
    replace(position, newArgument);
    return newArgument;
}


/**
 * Process an argument and ensure it is a valid integer
 * that can be expressed as a 64-bit value.
 *
 * @param position The argument position for any error messages.
 * @param argcount The number of arguments passed to the function.
 * @param function The function name
 *
 * @return An object that can be converted to a 64-bit value for
 *         pass-on to a native function.
 */
RexxObject *ExpressionStack::optionalBigIntegerArg(size_t position, size_t argcount, const char *function)
{
    RexxObject *argument = (RexxObject *)peek(position);
    if (argument == OREF_NULL)
    {
        return OREF_NULL;
    }
    // get this in the form of an object that is valid as a 64-bit integer, ready to
    // be passed along as an argument to native code.
    RexxObject *newArgument = Numerics::int64Object(argument);
    // returns a null value if it doesn't convert properly
    if (newArgument == OREF_NULL)
    {
        reportException(Error_Incorrect_call_whole, function, argcount - position, argument);
    }
    // replace original object on the stack
    replace(position, newArgument);
    return newArgument;
}
