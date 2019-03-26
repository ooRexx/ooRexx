/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                          NativeActivation.cpp  */
/*                                                                            */
/* Primitive Native Activation Class                                          */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "MethodClass.hpp"
#include "NativeCode.hpp"
#include "RexxActivation.hpp"
#include "NativeActivation.hpp"
#include "BufferClass.hpp"
#include "MessageClass.hpp"
#include "VariableDictionary.hpp"
#include "RexxCode.hpp"
#include "RexxInstruction.hpp"
#include "ExpressionBaseVariable.hpp"
#include "ProtectedObject.hpp"
#include "PointerClass.hpp"
#include "ActivityDispatcher.hpp"
#include "CallbackDispatcher.hpp"
#include "TrappingDispatcher.hpp"
#include "Interpreter.hpp"
#include "SystemInterpreter.hpp"
#include "ActivationFrame.hpp"
#include "StackFrameClass.hpp"
#include "MutableBufferClass.hpp"
#include "MethodArguments.hpp"
#include "ExpressionStem.hpp"
#include "VariableReference.hpp"

#include <stdio.h>


/**
 * Allocate a new native Activation.
 *
 * @param size   the allocation size.
 *
 * @return A pointer to the newly allocated object.
 */
void * NativeActivation::operator new(size_t size)
{
    return new_object(size, T_NativeActivation);
}


/**
 * Initialize an activation for direct caching in the activation
 * cache.  At this time, this is not an executable activation
 */
NativeActivation::NativeActivation()
{
    setHasNoReferences();          // nothing referenced from this either
}


/**
 * Constructor for a new native activation used to create a
 * callback context for exit call outs.
 *
 * @param _activity The activity we're running under.
 */
NativeActivation::NativeActivation(Activity *_activity, RexxActivation*_activation)
{
    clearObject();             // start with a fresh object
    activity = _activity;      // the activity we're running
    activation = _activation;  // our parent context
}


/**
 * Constructor for a new native activation used to create a
 * callback context for exit call outs.
 *
 * @param _activity The activity we're running under.
 */
NativeActivation::NativeActivation(Activity *_activity)
{
    clearObject();
    activity = _activity;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void NativeActivation::live(size_t liveMark)
{
    memory_mark(previous);
    memory_mark(executable);
    memory_mark(argArray);
    memory_mark(receiver);
    memory_mark(activity);
    memory_mark(activation);
    memory_mark(messageName);
    memory_mark(firstSavedObject);
    memory_mark(saveList);
    memory_mark(result);
    memory_mark(objectVariables);
    memory_mark(conditionObj);
    memory_mark(securityManager);

    // We're hold a pointer back to our arguments directly where they
    // are created.  Since in some places, this argument list comes
    // from the C stack, we need to handle the marker ourselves.
    memory_mark_array(argCount, argList);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void NativeActivation::liveGeneral(MarkReason reason)
{
    memory_mark_general(previous);
    memory_mark_general(executable);
    memory_mark_general(argArray);
    memory_mark_general(receiver);
    memory_mark_general(activity);
    memory_mark_general(activation);
    memory_mark_general(messageName);
    memory_mark_general(firstSavedObject);
    memory_mark_general(saveList);
    memory_mark_general(result);
    memory_mark_general(objectVariables);
    memory_mark_general(conditionObj);
    memory_mark_general(securityManager);

    // We're hold a pointer back to our arguments directly where they
    // are created.  Since in some places, this argument list comes
    // from the C stack, we need to handle the marker ourselves.
    memory_mark_general_array(argCount, argList);
}


/**
 * Report a method/function signature error
 */
void NativeActivation::reportSignatureError()
{
    reportException(isMethod() ? Error_Incorrect_method_signature : Error_Incorrect_call_signature);
}


/**
 * Report a stem argument error.
 *
 * @param position The argument position
 * @param object   The object in error.
 */
void NativeActivation::reportStemError(size_t position, RexxObject *object)
{
    reportException(isMethod() ? Error_Incorrect_method_nostem : Error_Incorrect_call_nostem, position + 1, object);
}


/**
 * Process the arguments for a typed function/method call.
 *
 * @param argcount The count of arguments.
 * @param arglist  The original Rexx arguments.
 * @param argumentTypes
 *                 The type descriptor from the target.
 * @param descriptors
 *                 The maximum argument count for the target.
 * @param maximumArgumentCount
 */
void NativeActivation::processArguments(size_t _argcount, RexxObject **_arglist, uint16_t *argumentTypes,
    ValueDescriptor *descriptors, size_t maximumArgumentCount)
{
    size_t inputIndex = 0;            // no arguments used yet             */
    size_t outputIndex = 1;           // we start filling in from the second (first is return value)
    bool usedArglist = false;         // if we request the argument list, then there's no requirement
                                      // to use up all of the passed arguments individually.

    // set up the return value descriptor
    descriptors[0].type = *argumentTypes;
    descriptors[0].value.value_int64_t = 0;
    // the first type in the signature is the return value, which we skip for now
    uint16_t *currentType = argumentTypes + 1;
    // now loop through the type information.  Some of the types are "created"
    // arguments that don't appear directly in the method/function invocation.
    // We can't directly do a 1-for-1 mapping
    for (; *currentType != REXX_ARGUMENT_TERMINATOR; currentType++)
    {
        // make sure we don't get a buffer overload
        if (outputIndex >= maximumArgumentCount)
        {
            reportSignatureError();
        }

        uint16_t type = ARGUMENT_TYPE(*currentType);
        bool isOptional = IS_OPTIONAL_ARGUMENT(*currentType);

        descriptors[outputIndex].type = type;           // fill in the type
        switch (type)
        {
            // reference to the receiver object...if this is a function call,
            // then this will be OREF NULL.
            case REXX_VALUE_OSELF:
            {
                // this doesn't make any sense for a function call
                if (!isMethod())
                {
                    reportSignatureError();
                }
                // fill in the receiver object and mark it...
                descriptors[outputIndex].value.value_RexxObjectPtr = (RexxObjectPtr)receiver;
                descriptors[outputIndex].flags = ARGUMENT_EXISTS | SPECIAL_ARGUMENT;
                break;
            }

            // reference to the method scope...if this is a function call,
            // then this will be OREF NULL.
            case REXX_VALUE_SCOPE:
            {
                // this doesn't make any sense for a function call
                if (!isMethod())
                {
                    reportSignatureError();
                }
                // fill in the receiver object and mark it...
                descriptors[outputIndex].value.value_RexxObjectPtr = (RexxObjectPtr)getScope();
                descriptors[outputIndex].flags = ARGUMENT_EXISTS | SPECIAL_ARGUMENT;
                break;
            }

            // reference to the superclass scope...if this is a function call,
            // then this will be OREF NULL.
            case REXX_VALUE_SUPER:
            {
                // this doesn't make any sense for a function call
                if (!isMethod())
                {
                    reportSignatureError();
                }
                // fill in the receiver object and mark it...
                descriptors[outputIndex].value.value_RexxObjectPtr = (RexxClassObject)getSuper();
                descriptors[outputIndex].flags = ARGUMENT_EXISTS | SPECIAL_ARGUMENT;
                break;
            }

            case REXX_VALUE_CSELF:                /* reference to CSELF                */
            {
                // this doesn't make any sense for a function call
                if (!isMethod())
                {
                    reportSignatureError();
                }
                descriptors[outputIndex].value.value_POINTER = cself();
                descriptors[outputIndex].flags = ARGUMENT_EXISTS | SPECIAL_ARGUMENT;
                break;
            }

            case REXX_VALUE_ARGLIST:              /* need the argument list            */
            {
                descriptors[outputIndex].flags = ARGUMENT_EXISTS | SPECIAL_ARGUMENT;
                descriptors[outputIndex].value.value_RexxArrayObject = (RexxArrayObject)getArguments();
                // we've used this
                usedArglist = true;
                break;
            }

            // This is either the message name used to invoke a method or
            // the name used to call a function
            case REXX_VALUE_NAME:
            {
                descriptors[outputIndex].flags = ARGUMENT_EXISTS | SPECIAL_ARGUMENT;
                descriptors[outputIndex].value.value_CSTRING = (CSTRING)messageName->getStringData();
                break;
            }

            // this is a real argument taken from the argument list
            default:
            {
                if (inputIndex < _argcount && _arglist[inputIndex] != OREF_NULL)
                {
                    // all of these arguments exist
                    descriptors[outputIndex].flags = ARGUMENT_EXISTS;
                    RexxObject *argument = _arglist[inputIndex];
                    switch (type)
                    {
                        // arbitrary object reference
                        case REXX_VALUE_RexxObjectPtr:
                        {
                            descriptors[outputIndex].value.value_RexxObjectPtr = (RexxObjectPtr)argument;
                            break;
                        }

                        case REXX_VALUE_int:
                        {
                            // convert and copy                  */
                            descriptors[outputIndex].value.value_int = (int)signedIntegerValue(argument, inputIndex, INT_MAX, INT_MIN);
                            break;
                        }

                        case REXX_VALUE_int8_t:
                        {
                            descriptors[outputIndex].value.value_int8_t = (int8_t)signedIntegerValue(argument, inputIndex, INT8_MAX, INT8_MIN);
                            break;
                        }

                        case REXX_VALUE_int16_t:
                        {
                            descriptors[outputIndex].value.value_int16_t = (int16_t)signedIntegerValue(argument, inputIndex, INT16_MAX, INT16_MIN);
                            break;
                        }

                        case REXX_VALUE_int32_t:
                        {
                            descriptors[outputIndex].value.value_int32_t = (int32_t)signedIntegerValue(argument, inputIndex, INT32_MAX, INT32_MIN);
                            break;
                        }

                        case REXX_VALUE_int64_t:
                        {
                            descriptors[outputIndex].value.value_int64_t = (int64_t)int64Value(argument, inputIndex);
                            break;
                        }

                        case REXX_VALUE_ssize_t:
                        {
                            descriptors[outputIndex].value.value_ssize_t = (ssize_t)signedIntegerValue(argument, inputIndex, SSIZE_MAX, -SSIZE_MAX - 1);
                            break;
                        }

                        case REXX_VALUE_intptr_t:
                        {
                            descriptors[outputIndex].value.value_intptr_t = (intptr_t)signedIntegerValue(argument, inputIndex, INTPTR_MAX, INTPTR_MIN);
                            break;
                        }

                        case REXX_VALUE_uint8_t:
                        {
                            descriptors[outputIndex].value.value_uint8_t = (uint8_t)unsignedIntegerValue(argument, inputIndex, UINT8_MAX);
                            break;
                        }

                        case REXX_VALUE_uint16_t:
                        {
                            descriptors[outputIndex].value.value_uint16_t = (uint16_t)unsignedIntegerValue(argument, inputIndex, UINT16_MAX);
                            break;
                        }

                        case REXX_VALUE_uint32_t:
                        {
                            descriptors[outputIndex].value.value_uint32_t = (uint32_t)unsignedIntegerValue(argument, inputIndex, UINT32_MAX);
                            break;
                        }

                        case REXX_VALUE_uint64_t:
                        {
                            descriptors[outputIndex].value.value_uint64_t = (uint64_t)unsignedInt64Value(argument, inputIndex);
                            break;
                        }

                        case REXX_VALUE_size_t:
                        {
                            descriptors[outputIndex].value.value_size_t = (size_t)unsignedIntegerValue(argument, inputIndex, SIZE_MAX);
                            break;
                        }

                        case REXX_VALUE_uintptr_t:
                        {
                            descriptors[outputIndex].value.value_uintptr_t = (uintptr_t)unsignedIntegerValue(argument, inputIndex, UINTPTR_MAX);
                            break;
                        }

                        case REXX_VALUE_logical_t:
                        {
                            descriptors[outputIndex].value.value_logical_t = argument->truthValue(Error_Logical_value_method);
                            break;
                        }

                        // The Rexx whole number one is checked against the human digits limit
                        case REXX_VALUE_wholenumber_t:
                        {
                            descriptors[outputIndex].value.value_wholenumber_t = (wholenumber_t)signedIntegerValue(argument, inputIndex, Numerics::MAX_WHOLENUMBER, Numerics::MIN_WHOLENUMBER);
                            break;
                        }

                        // The Rexx whole number one is checked against the human digits limit
                        case REXX_VALUE_positive_wholenumber_t:
                        {
                            descriptors[outputIndex].value.value_wholenumber_t = positiveWholeNumberValue(argument, inputIndex);
                            break;
                        }

                        // The Rexx whole number one is checked against the human digits limit
                        case REXX_VALUE_nonnegative_wholenumber_t:
                        {
                            descriptors[outputIndex].value.value_wholenumber_t = nonnegativeWholeNumberValue(argument, inputIndex);
                            break;
                        }

                        // an unsigned string number value
                        case REXX_VALUE_stringsize_t:
                        {
                            descriptors[outputIndex].value.value_stringsize_t = (stringsize_t)unsignedIntegerValue(argument, inputIndex, Numerics::MAX_STRINGSIZE);
                            break;
                        }

                        case REXX_VALUE_double:
                        {
                            descriptors[outputIndex].value.value_double = getDoubleValue(argument, inputIndex);
                            break;
                        }


                        case REXX_VALUE_float:
                        {
                            descriptors[outputIndex].value.value_float = (float)getDoubleValue(argument, inputIndex);
                            break;
                        }

                        case REXX_VALUE_CSTRING:
                        {
                            descriptors[outputIndex].value.value_CSTRING = cstring(argument);
                            break;
                        }

                        case REXX_VALUE_RexxStringObject:
                        {
                            // force to a string value
                            RexxString *temp = stringArgument(argument, inputIndex + 1) ;
                            // if this forced a string object to be created,
                            // we need to protect it here.
                            if (temp != argument)
                            {
                                createLocalReference(temp);
                            }
                            descriptors[outputIndex].value.value_RexxStringObject = (RexxStringObject)temp;
                            break;

                        }

                        case REXX_VALUE_RexxArrayObject:
                        {
                            ArrayClass *temp = arrayArgument(argument, inputIndex + 1) ;
                            // if this forced a string object to be created,
                            // we need to protect it here.
                            if (temp != argument)
                            {
                                createLocalReference(temp);
                            }
                            descriptors[outputIndex].value.value_RexxArrayObject = (RexxArrayObject)temp;
                            break;

                        }

                        case REXX_VALUE_RexxStemObject:
                        {
                            // Stem arguments get special handling.  If the argument
                            // object is already a stem object, we're done.  Otherwise,
                            // we get the string value of the argument and use that
                            // to resolve a stem name in the current context.  If the
                            // trailing period is not given on the name, one will be added.
                            // Note that the second form is only available if this is a
                            // call context.  This is an error for a method context.

                            // is this a stem already?
                            if (isStem(argument))
                            {
                                descriptors[outputIndex].value.value_RexxStemObject = (RexxStemObject)argument;
                                break;
                            }

                            // this doesn't make any sense for a method call...requires
                            // variable pool access
                            if (isMethod())
                            {
                                reportStemError(inputIndex, argument);
                            }

                            RexxString *temp = argument->requestString();
                            if ((RexxObject *)temp == TheNilObject)
                            {
                                reportStemError(inputIndex, argument);
                            }
                            // if this forced a string object to be created,
                            // we need to protect it here.
                            if (temp != argument)
                            {

                                createLocalReference(temp);
                            }

                            // see if we can retrieve this stem
                            RexxObject *stem = getContextStem(temp);
                            if (stem == OREF_NULL)
                            {
                                reportStemError(inputIndex, argument);
                            }
                            descriptors[outputIndex].value.value_RexxStemObject = (RexxStemObject)stem;
                            break;
                        }

                        case REXX_VALUE_RexxClassObject: // required class object
                        {
                            // this must be a class object
                            if (!argument->isInstanceOf(TheClassClass))
                            {
                                reportException(Error_Invalid_argument_noclass, inputIndex + 1, TheClassClass->getId());
                            }
                            descriptors[outputIndex].value.value_RexxClassObject = (RexxClassObject)argument;
                            break;
                        }

                        case REXX_VALUE_POINTER:
                        {
                            // this must be a pointer object
                            if (!argument->isInstanceOf(ThePointerClass))
                            {
                                reportException(Error_Invalid_argument_noclass, inputIndex + 1, ThePointerClass->getId());
                            }
                            descriptors[outputIndex].value.value_POINTER = pointer(argument);
                            break;
                        }

                        case REXX_VALUE_POINTERSTRING:
                        {
                            descriptors[outputIndex].value.value_POINTERSTRING = pointerString(argument, inputIndex);
                            break;
                        }

                        case REXX_VALUE_RexxMutableBufferObject:
                        {
                            // this must be a pointer object
                            if (!argument->isInstanceOf(TheMutableBufferClass))
                            {
                                reportException(Error_Invalid_argument_noclass, inputIndex + 1, TheMutableBufferClass->getId());
                            }
                            descriptors[outputIndex].value.value_RexxMutableBufferObject = (RexxMutableBufferObject)argument;
                            break;
                        }

                        case REXX_VALUE_RexxVariableReferenceObject:
                        {
                            // this must be a pointer object
                            if (!argument->isInstanceOf(TheVariableReferenceClass))
                            {
                                reportException(Error_Invalid_argument_noclass, inputIndex + 1, TheVariableReferenceClass->getId());
                            }
                            descriptors[outputIndex].value.value_RexxVariableReferenceObject = (RexxVariableReferenceObject)argument;
                            break;
                        }

                        default:
                        {
                            reportSignatureError();
                            break;
                        }
                    }
                }
                else
                {
                    // if this was not an option argument
                    if (!isOptional)
                    {
                        reportException(Error_Invalid_argument_noarg, inputIndex + 1);
                    }

                    // this is a non-specified argument
                    descriptors[outputIndex].flags = 0;
                    switch (type)
                    {

                        case REXX_VALUE_RexxObjectPtr:     // no object here
                        case REXX_VALUE_int:               // non-integer value
                        case REXX_VALUE_wholenumber_t:     // non-existent long
                        case REXX_VALUE_positive_wholenumber_t:     // non-existent long
                        case REXX_VALUE_nonnegative_wholenumber_t:  // non-existent long
                        case REXX_VALUE_CSTRING:           // missing character string
                        case REXX_VALUE_POINTER:
                        case REXX_VALUE_RexxStringObject:  // no object here
                        case REXX_VALUE_stringsize_t:      // non-existent long
                        case REXX_VALUE_int8_t:            // non-integer value
                        case REXX_VALUE_int16_t:           // non-integer value
                        case REXX_VALUE_int32_t:           // non-integer value
                        case REXX_VALUE_int64_t:           // non-integer value
                        case REXX_VALUE_uint8_t:           // non-integer value
                        case REXX_VALUE_uint16_t:          // non-integer value
                        case REXX_VALUE_uint32_t:          // non-integer value
                        case REXX_VALUE_uint64_t:          // non-integer value
                        case REXX_VALUE_size_t:
                        case REXX_VALUE_ssize_t:
                        case REXX_VALUE_intptr_t:
                        case REXX_VALUE_uintptr_t:
                        case REXX_VALUE_logical_t:         // this must be a boolean value
                        case REXX_VALUE_RexxArrayObject:   // no object here
                        case REXX_VALUE_RexxStemObject:
                        case REXX_VALUE_RexxClassObject:
                        case REXX_VALUE_RexxMutableBufferObject:
                        case REXX_VALUE_POINTERSTRING:
                        {
                            // set this as a 64-bit value to clear everything out
                            descriptors[outputIndex].value.value_int64_t = 0;
                            break;
                        }
                        // non-existent double
                        case REXX_VALUE_double:
                        {
                            descriptors[outputIndex].value.value_double = 0.0;
                            break;
                        }
                        // non-existent float
                        case REXX_VALUE_float:
                        {
                            descriptors[outputIndex].value.value_float = 0.0;
                            break;
                        }

                        // a bad signature
                        default:
                        {
                            reportSignatureError();
                            break;
                        }
                    }
                }
                inputIndex++;              // we've used up one more input argument
                break;
            }
        }
        outputIndex++;                 // step to the next argument
        argumentTypes++;               // and the next output position pointer
    }
    // do we have additional unwanted arguments?  that's an error
    if (inputIndex < _argcount && !usedArglist)    /* extra, unwanted arguments?        */
    {
        reportException(Error_Invalid_argument_maxarg, inputIndex);
    }
}


/**
 * Convert an collection of value descriptors into a Rexx array
 * object.  This is useful for creating arrays of arguments for
 * the various call APIs.
 *
 * @param value  The self-describing value descriptors.
 * @param count  The number of descriptors in the list.
 *
 * @return The described value converted to an appropriate Rexx object.
 */
ArrayClass *NativeActivation::valuesToObject(ValueDescriptor *value, size_t count)
{
    ArrayClass *r = new_array(count);
    ProtectedObject p(r);

    for (size_t i = 0; i < count; i++)
    {
        // convert each of the values in turn
        r->put(valueToObject(value++), i);
    }
    return r;
}


/**
 * Convert an API value descriptor into a Rexx object.
 *
 * @param value  The self-describing value descriptor.
 *
 * @return The described value converted to an appropriate Rexx object.
 */
RexxObject *NativeActivation::valueToObject(ValueDescriptor *value)
{
    switch (value->type)
    {
        case REXX_VALUE_RexxObjectPtr:          // object reference.  All object types get
        case REXX_VALUE_RexxStringObject:       // returned as a Rexx object
        case REXX_VALUE_RexxArrayObject:
        case REXX_VALUE_RexxClassObject:
        case REXX_VALUE_RexxStemObject:
        case REXX_VALUE_RexxMutableBufferObject:
        case REXX_VALUE_RexxVariableReferenceObject:
        {
            return (RexxObject *)value->value.value_RexxObjectPtr; // just return the object value
        }

        case REXX_VALUE_int:
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_int);
        }

        case REXX_VALUE_int8_t:
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_int8_t);
        }

        case REXX_VALUE_int16_t:
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_int16_t);
        }

        case REXX_VALUE_int32_t:
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_int32_t);
        }

        case REXX_VALUE_int64_t:
        {
            return Numerics::int64ToObject(value->value.value_int64_t);
        }

        case REXX_VALUE_intptr_t:
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_intptr_t);
        }

        case REXX_VALUE_uint8_t:
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_uint8_t);
        }

        case REXX_VALUE_uint16_t:
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_uint16_t);
        }

        case REXX_VALUE_uint32_t:
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_uint32_t);
        }

        case REXX_VALUE_uint64_t:
        {
            return Numerics::uint64ToObject(value->value.value_uint64_t);
        }

        case REXX_VALUE_uintptr_t:
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_uintptr_t);
        }

        case REXX_VALUE_logical_t:
        {
            return booleanObject(value->value.value_logical_t != 0);
        }

        case REXX_VALUE_size_t:
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_size_t);
        }

        case REXX_VALUE_ssize_t:
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_size_t);
        }

        case REXX_VALUE_positive_wholenumber_t:
        case REXX_VALUE_nonnegative_wholenumber_t:
        case REXX_VALUE_wholenumber_t:
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_wholenumber_t);
        }

        case REXX_VALUE_stringsize_t:
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_stringsize_t);
        }

        case REXX_VALUE_double:
        {
            return new_string(value->value.value_double);
        }

        case REXX_VALUE_float:
        {
            return new_string(value->value.value_float);
        }

        case REXX_VALUE_CSTRING:
        {
            const char *string = value->value.value_CSTRING;
            // return return nothing if a null pointer is returned.
            if (string == NULL)
            {
                return OREF_NULL;
            }
            return new_string(string);             // make a Rexx string from this
        }

        case REXX_VALUE_POINTER:
        {
            // just wrap the pointer in a pointer object
            return new_pointer(value->value.value_POINTER);
        }

        case REXX_VALUE_POINTERSTRING:
        {
            // format this into a chracter string
            return Numerics::pointerToString(value->value.value_POINTER);
        }

        case 0:
        {
            // useful for creating argument lists.  This is an omitted value, so just return
            // a null value
            return OREF_NULL;
        }

        default:
        {
            reportSignatureError();      // bad method signature problem
            break;
        }
    }
    return OREF_NULL;
}


/**
 * Convert a Rexx object into the requested value type, if possible.
 *
 * @param o      The source object.
 * @param value  The receiving value structure, which also defines the type.
 *
 * @return true if the value was convertable, false otherwise.
 */
bool NativeActivation::objectToValue(RexxObject *o, ValueDescriptor *value)
{
    switch (value->type)
    {
        case REXX_VALUE_RexxObjectPtr:          // Object reference
        {
            // silly, but this always works.
            value->value.value_RexxObjectPtr = (RexxObjectPtr)o;
            return true;
        }
        case REXX_VALUE_RexxClassObject: // required class object
        {
            // this must be a class object
            if (!o->isInstanceOf(TheClassClass))
            {
                return false;
            }
            value->value.value_RexxClassObject = (RexxClassObject)o;
            return true;
        }
        case REXX_VALUE_RexxMutableBufferObject: // required mutablebuffer object
        {
            // this must be a mutablebuffer object
            if (!o->isInstanceOf(TheMutableBufferClass))
            {
                return false;
            }
            value->value.value_RexxMutableBufferObject = (RexxMutableBufferObject)o;
            return true;
        }
        case REXX_VALUE_RexxVariableReferenceObject: // required variable reference object
        {
            // this must be a variablereference object
            if (!o->isInstanceOf(TheVariableReferenceClass))
            {
                return false;
            }
            value->value.value_RexxVariableReferenceObject = (RexxVariableReferenceObject)o;
            return true;
        }
        case REXX_VALUE_int:
        {
            ssize_t temp = 0;
            bool success = Numerics::objectToSignedInteger(o, temp, INT_MAX, INT_MIN);
            value->value.value_int = (int)temp;
            return success;
        }

        case REXX_VALUE_int8_t:
        {
            ssize_t temp = 0;
            bool success = Numerics::objectToSignedInteger(o, temp, INT8_MAX, INT8_MIN);
            value->value.value_int8_t = (int8_t)temp;
            return success;
        }

        case REXX_VALUE_int16_t:
        {
            ssize_t temp = 0;
            bool success = Numerics::objectToSignedInteger(o, temp, INT16_MAX, INT16_MIN);
            value->value.value_int16_t = (int16_t)temp;
            return success;
        }

        case REXX_VALUE_int32_t:
        {
            ssize_t temp = 0;
            bool success = Numerics::objectToSignedInteger(o, temp, INT32_MAX, INT32_MIN);
            value->value.value_int32_t = (int32_t)temp;
            return success;
        }

        case REXX_VALUE_intptr_t:
        {
            intptr_t temp = 0;
            bool success = Numerics::objectToIntptr(o, temp);
            value->value.value_intptr_t = (intptr_t)temp;
            return success;
        }

        case REXX_VALUE_int64_t:
        {
            int64_t temp = 0;
            bool success = Numerics::objectToInt64(o, temp);
            value->value.value_int64_t = (int64_t)temp;
            return success;
        }

        case REXX_VALUE_uint8_t:
        {
            size_t temp = 0;
            bool success = Numerics::objectToUnsignedInteger(o, temp, UINT8_MAX);
            value->value.value_uint8_t = (uint8_t)temp;
            return success;
        }

        case REXX_VALUE_uint16_t:
        {
            size_t temp = 0;
            bool success = Numerics::objectToUnsignedInteger(o, temp, UINT16_MAX);
            value->value.value_uint16_t = (uint16_t)temp;
            return success;
        }

        case REXX_VALUE_uint32_t:
        {
            size_t temp = 0;
            bool success = Numerics::objectToUnsignedInteger(o, temp, UINT32_MAX);
            value->value.value_uint32_t = (uint32_t)temp;
            return success;
        }

        case REXX_VALUE_uintptr_t:
        {
            uintptr_t temp = 0;
            bool success = Numerics::objectToUintptr(o, temp);
            value->value.value_uintptr_t = (uintptr_t)temp;
            return success;
        }

        case REXX_VALUE_uint64_t:
        {
            uint64_t temp = 0;
            bool success = Numerics::objectToUnsignedInt64(o, temp);
            value->value.value_uint64_t = (uint64_t)temp;
            return success;
        }

        case REXX_VALUE_size_t:
        {
            size_t temp = 0;
            bool success = Numerics::objectToUnsignedInteger(o, temp, SIZE_MAX);
            value->value.value_size_t = (size_t)temp;
            return success;
        }

        case REXX_VALUE_logical_t:
        {
            return o->logicalValue(value->value.value_logical_t);
        }

        // The Rexx whole number one is checked against the human digits limit
        case REXX_VALUE_wholenumber_t:
        {
            wholenumber_t temp = 0;
            bool success = Numerics::objectToWholeNumber(o, temp, Numerics::MAX_WHOLENUMBER, Numerics::MIN_WHOLENUMBER);
            value->value.value_wholenumber_t = (wholenumber_t)temp;
            return success;
        }

        // The Rexx whole number one is checked against the human digits limit
        case REXX_VALUE_positive_wholenumber_t:
        {
            wholenumber_t temp = 0;
            bool success = Numerics::objectToWholeNumber(o, temp, Numerics::MAX_WHOLENUMBER, 1);
            value->value.value_positive_wholenumber_t = (wholenumber_t)temp;
            return success;
        }

        // The Rexx whole number one is checked against the human digits limit
        case REXX_VALUE_nonnegative_wholenumber_t:
        {
            wholenumber_t temp = 0;
            bool success = Numerics::objectToWholeNumber(o, temp, Numerics::MAX_WHOLENUMBER, 0);
            value->value.value_wholenumber_t = (wholenumber_t)temp;
            return success;
        }

        case REXX_VALUE_ssize_t:
        {
            ssize_t temp = 0;
            // NB:  SSIZE_MIN appears to be defined as 0 for some bizarre reason on some platforms,
            // so we'll make things relative to SIZE_MAX.
            bool success = Numerics::objectToSignedInteger(o, temp, SSIZE_MAX, (-SSIZE_MAX) - 1);
            value->value.value_nonnegative_wholenumber_t = (wholenumber_t)temp;
            return success;
        }

        // an unsigned string number value
        case REXX_VALUE_stringsize_t:
        {
            stringsize_t temp = 0;
            bool success = Numerics::objectToStringSize(o, temp, Numerics::MAX_STRINGSIZE);
            value->value.value_stringsize_t = temp;
            return success;
        }

        case REXX_VALUE_double:
        {
            return o->doubleValue(value->value.value_double);
        }


        case REXX_VALUE_float:
        {
            double temp = 0.0;
            bool success = o->doubleValue(temp);
            value->value.value_float = (float)temp;
            return success;
        }

        case REXX_VALUE_CSTRING:
        {
            value->value.value_CSTRING = cstring(o);
            return true;
        }

        case REXX_VALUE_RexxStringObject:
        {
            RexxString *temp = stringArgument(o, 1) ;
            // if this forced a string object to be created,
            // we need to protect it here.
            if (temp != o)
            {
                createLocalReference(temp);
            }
            value->value.value_RexxStringObject = (RexxStringObject)temp;
            return true;

        }

        case REXX_VALUE_RexxArrayObject:
        {
            ArrayClass *temp = arrayArgument(o, 1) ;
            // if this forced a new object to be created,
            // we need to protect it here.
            if (temp != o)
            {
                createLocalReference(temp);
            }
            value->value.value_RexxArrayObject = (RexxArrayObject)temp;
            return true;

        }

        case REXX_VALUE_RexxStemObject:
        {
            // Stems get special handling.  If the
            // object is already a stem object, we're done.  Otherwise,
            // we get the string value of the value and use that
            // to resolve a stem name in the current context.  If the
            // trailing period is not given on the name, one will be added.
            // Note that the second form is only available if this is a
            // call context.  This is an error for a method context.

            // is this a stem already?
            if (isStem(o))
            {
                value->value.value_RexxStemObject = (RexxStemObject)o;
                return true;
            }

            // this doesn't make any sense for a method call
            if (isMethod())
            {
                return false;
            }

            RexxString *temp = stringArgument(o, 1) ;
            // if this forced a string object to be created,
            // we need to protect it here.
            if (temp != o)
            {
                createLocalReference(temp);
            }

            // see if we can retrieve this stem
            RexxObject *stem = getContextStem(temp);
            if (stem == OREF_NULL)
            {
                return false;
            }
            value->value.value_RexxStemObject = (RexxStemObject)stem;
            return true;
        }

        case REXX_VALUE_POINTER:
        {
            value->value.value_POINTER = pointer(o);
            return true;
        }

        case REXX_VALUE_POINTERSTRING:
        {
            RexxString *string = o->stringValue();

            void *pointerVal;
            if (sscanf(string->getStringData(), "0x%p", &pointerVal) != 1)
            {
                return false;
            }

            value->value.value_POINTER = pointerVal;
            return true;
        }

        // something we don't recognize or support.
        default:
        {
            return false;
        }
    }
    return false;
}


/**
 * Create a local reference for an object.  The protects the object
 * from GC until the environment terminates.
 *
 * @param objr   The object to protect.
 */
void NativeActivation::createLocalReference(RexxInternalObject *objr)
{
    // if we have a real object, then add to the list
    if (objr != OREF_NULL)
    {
        // It is a fairly common pattern that a single object gets allocated
        // by a method call that is being returned as a result. This is common if
        // a string value is getting returned. This pushes the allocation of the
        // save list off until it is needed for more than one object.
        if (firstSavedObject == OREF_NULL)
        {
            firstSavedObject = objr;
        }

        else
        {
            // make sure we protect this from a GC triggered by this table creation.
            ProtectedObject p1(objr);
            // create an identity table if this is the first reference we need to protect.
            if (saveList == OREF_NULL)
            {
                saveList = new_identity_table();
            }
            saveList->put(objr, objr);
        }
    }
}


/**
 * Remove an object from the local reference table.
 *
 * @param objr   The object to remove.
 */
void NativeActivation::removeLocalReference(RexxInternalObject *objr)
{
    // if the reference is non-null
    if (objr != OREF_NULL)
    {
        // if this is our first object we can just null this out
        if (firstSavedObject == objr)
        {
            firstSavedObject = OREF_NULL;
        }
        else
        {

            // make sure we have a savelist before trying to remove this
            if (saveList != OREF_NULL)
            {
                // NB...this is a special remove that functions using the object
                // identify to avoid false positives or potential exceptions caused
                // by calling EQUALS methods.
                saveList->remove(objr);
            }
        }
    }
}


/**
 * Run a native method or function under the context of this activation.
 *
 * @param _receiver The receiver object (NULL if not a method invocation).
 * @param _msgname  The message name.
 * @param _argcount The argument count
 * @param _arglist  The list of arguments.
 * @param resultObj The returned result object.
 */
void NativeActivation::run(MethodClass *_method, NativeMethod *_code, RexxObject  *_receiver,
                           RexxString  *_msgname, RexxObject **_arglist, size_t _argcount, ProtectedObject &resultObj)
{
    // add the frame to the execution stack
    NativeActivationFrame frame(activity, this);
    // anchor the relevant context information
    executable = _method;
    receiver = _receiver;
    messageName = _msgname;
    argList = _arglist;
    argCount = _argcount;
    activationType = METHOD_ACTIVATION;      // this is for running a method

    ValueDescriptor arguments[MaxNativeArguments];

    MethodContext context;               // the passed out method context

    // sort out our active security manager
    securityManager = _code->getSecurityManager();
    if (securityManager == OREF_NULL)
    {
        securityManager = activity->getInstanceSecurityManager();
    }

    // build a context pointer to pass out
    activity->createMethodContext(context, this);

    context.threadContext.arguments = arguments;

    // get the entry point address of the target method
    PNATIVEMETHOD methp = _code->getEntry();

    // retrieve the argument signatures and process them
    uint16_t *types = (*methp)((RexxMethodContext *)&context, NULL);
    processArguments(argCount, argList, types, arguments, MaxNativeArguments);

    size_t activityLevel = activity->getActivationLevel();
    trapErrors = true;                       // we trap errors from here
    try
    {
        activity->releaseAccess();           /* force this to "safe" mode         */
        /* process the method call           */
        (*methp)((RexxMethodContext *)&context, arguments);
        activity->requestAccess();           /* now in unsafe mode again          */

        // process the returned result
        result = valueToObject(arguments);
    }
    catch (NativeActivation *)
    {
    }
    // there is a subtle interaction between native and rexx activations when
    // native calls make calls to APIs that in turn invoke Rexx code.  When conditions
    // occur and the stack is being unwound, the ApiContext destructors will release
    // the kernel access, which can leave us with no active Activity. Bad things happen
    // when that occurs. We'll grab the exception when it goes past and make sure
    // things remain consistent.
    catch (RexxActivation *r)
    {
        // if we're not the current kernel holder when things return, make sure we
        // get the lock before we continue
        if (ActivityManager::currentActivity != activity)
        {
            activity->requestAccess();
        }
        throw r;
    }

    // if we're not the current kernel holder when things return, make sure we
    // get the lock before we continue
    if (ActivityManager::currentActivity != activity)
    {
        activity->requestAccess();
    }
    guardOff();                  // release any variable locks
    argCount = 0;                // make sure we don't try to mark any arguments after this

    // the lock holder gets here by a throw from a kernel reentry.  We need to
    // make sure the activation count gets reset, else we'll accumulate bogus
    // nesting levels that will make it look like this activity is still in use
    // when in fact we're done with it.
    activity->restoreActivationLevel(activityLevel);

    // give up reference to receiver so that it can be garbage collected
    receiver = OREF_NULL;

    checkConditions();                   // see if we have conditions to raise now

    // set the return value and get outta here
    resultObj = result;

    // disable the stack frame used to generate the tracebacks since we are
    // noh longer active. An error in an uninit method might pick up bogus information
    // if we're still in the list
    frame.disableFrame();

    // good place to check for uninits
    memoryObject.checkUninitQueue();

    activity->popStackFrame(this); // pop this from the activity
    setHasNoReferences();          // mark this as not having references in case we get marked
}


/**
 * Process a native function call.
 *
 * @param routine   The routine we're executing (used for context resolution).
 * @param code      The code object.
 * @param functionName
 *                  The name of the function.
 * @param list      The list of arguments.
 * @param count     The number of arguments.
 * @param resultObj The return value.
 */
void NativeActivation::callNativeRoutine(RoutineClass *_routine, NativeRoutine *_code, RexxString *functionName,
                                         RexxObject **list, size_t count, ProtectedObject &resultObj)
{
    NativeActivationFrame frame(activity, this);

    // anchor the context stuff
    executable = _routine;
    messageName = functionName;
    argList = list;
    argCount = count;
    activationType = FUNCTION_ACTIVATION;      // this is for running a method
    accessCallerContext();                     // we need this to access the caller's context

    ValueDescriptor arguments[MaxNativeArguments];

    CallContext context;               // the passed out method context

    // sort out our active security manager
    securityManager = _code->getSecurityManager();
    if (securityManager == OREF_NULL)
    {
        securityManager = activity->getInstanceSecurityManager();
    }

    // build a context pointer to pass out
    activity->createCallContext(context, this);

    context.threadContext.arguments = arguments;

    // get the entry point address of the target method
    PNATIVEROUTINE methp = _code->getEntry();

    // retrieve the argument signatures and process them
    uint16_t *types = (*methp)((RexxCallContext *)&context, NULL);
    processArguments(argCount, argList, types, arguments, MaxNativeArguments);

    size_t activityLevel = activity->getActivationLevel();
    trapErrors = true;                       // we trap error conditions now
    try
    {
        enableVariablepool();                // enable the variable pool interface here
        activity->releaseAccess();           // force this to "safe" mode
                                             // process the method call
        (*methp)((RexxCallContext *)&context, arguments);
        activity->requestAccess();           // now in unsafe mode again

        // process the returned result
        result = valueToObject(arguments);
    }
    catch (NativeActivation *)
    {
    }

    // if we're not the current kernel holder when things return, make sure we
    // get the lock before we continue
    if (ActivityManager::currentActivity != activity)
    {
        activity->requestAccess();
    }

    disableVariablepool();                // disable the variable pool from here
    // belt and braces...this restores the activity level to whatever
    // level we had when we made the callout.
    activity->restoreActivationLevel(activityLevel);

    // give up reference to receiver so that it can be garbage collected
    receiver = OREF_NULL;

    checkConditions();                   // see if we have conditions to raise now

    // set the return value and get outta here.
    resultObj = result;
    argCount = 0;                  // make sure we don't try to mark any arguments

    activity->popStackFrame(this); // pop this from the activity
    setHasNoReferences();          // mark this as not having references in case we get marked
}


/**
 * Process a native function call.
 *
 * @param entryPoint The target function entry point.
 * @param list       The list of arguments.
 * @param count      The number of arguments.
 * @param result     A protected object to receive the function result.
 */
void NativeActivation::callRegisteredRoutine(RoutineClass *_routine, RegisteredRoutine *_code, RexxString *functionName,
                                             RexxObject **list, size_t count, ProtectedObject &resultObj)
{
    NativeActivationFrame frame(activity, this);

    // anchor the context stuff
    messageName = functionName;
    executable = _routine;
    argList = list;
    argCount = count;
    accessCallerContext();                   // we need this to access the caller's context

    activationType = FUNCTION_ACTIVATION;      // this is for running a method
    // use the default security manager
    securityManager = activity->getInstanceSecurityManager();


    // get the entry point address of the target method
    RexxRoutineHandler *methp = _code->getEntry();

    CONSTRXSTRING  arguments[MaxNativeArguments];
    CONSTRXSTRING *argPtr = arguments;

    // unlike the other variants, we don't have a cap on arguments.  If we have more than the preallocated
    // set, then dynamically allocate a buffer object to hold the memory and anchor it in the
    // activation saved objects.
    if (count > MaxNativeArguments)
    {
        BufferClass *argBuffer = new_buffer(sizeof(CONSTRXSTRING) * count);
        // this keeps the buffer alive until the activation is popped.
        createLocalReference(argBuffer);
        argPtr = (CONSTRXSTRING *)argBuffer->getData();
    }

    // all of the arguments now need to be converted to string arguments
    for (size_t argindex = 0; argindex < count; argindex++)
    {
        /* get the next argument             */
        RexxObject *argument = list[argindex];
        if (argument != OREF_NULL)       /* have an argument?                 */
        {
            /* force to string form              */
            RexxString *stringArgument = argument->stringValue();
            // if the string version is not the same, we're going to need to make
            // sure the string version is protected from garbage collection until
            // the call is finished
            if (stringArgument != argument)
            {
                // make sure this is protected
                createLocalReference(stringArgument);
            }
            stringArgument->toRxstring(argPtr[argindex]);
        }
        // have an omitted argument...the rxstring is all zeroed
        else
        {
            argPtr[argindex].strlength = 0;
            argPtr[argindex].strptr = NULL;
        }
    }
    // get the current queue name
    const char *queuename = Interpreter::getCurrentQueue()->getStringData();
    RXSTRING funcresult;
    int functionrc;
    char default_return_buffer[DEFRXSTRING];

    // make the RXSTRING result
    MAKERXSTRING(funcresult, default_return_buffer, sizeof(default_return_buffer));

    size_t activityLevel = activity->getActivationLevel();

    trapErrors = true;                       // trap errors from here
    try
    {
        enableVariablepool();                // enable the variable pool interface here
        activity->releaseAccess();           // force this to "safe" mode
        // now process the function call
        functionrc = (int)(*methp)(functionName->getStringData(), count, argPtr, queuename, &funcresult);
        activity->requestAccess();           // now in unsafe mode again
    }
    catch (RexxActivation *)
    {
        // if we're not the current kernel holder when things return, make sure we
        // get the lock before we continue
        if (ActivityManager::currentActivity != activity)
        {
            activity->requestAccess();
        }

        argCount = 0;                // make sure we don't try to mark any arguments
        // the lock holder gets here by a throw from a kernel reentry.  We need to
        // make sure the activation count gets reset, else we'll accumulate bogus
        // nesting levels that will make it look like this activity is still in use
        // when in fact we're done with it.
        activity->restoreActivationLevel(activityLevel);
        // IMPORTANT NOTE:  We don't pop our activation off the stack.  This will be
        // handled by the catcher.  Attempting to pop the stack when an error or condition has
        // occurred can munge the activation stack, resulting bad stuff.
        setHasNoReferences();        // mark this as not having references in case we get marked
        // now rethrow the trapped condition so that real target can handle this.
        throw;
    }
    catch (NativeActivation *)
    {
        // if we're not the current kernel holder when things return, make sure we
        // get the lock before we continue
        if (ActivityManager::currentActivity != activity)
        {
            activity->requestAccess();
        }
        argCount = 0;                // make sure we don't try to mark any arguments
        // the lock holder gets here by throw from a kernel reentry.  We need to
        // make sure the activation count gets reset, else we'll accumulate bogus
        // nesting levels that will make it look like this activity is still in use
        // when in fact we're done with it.
        activity->restoreActivationLevel(activityLevel);
        activity->popStackFrame(this);   // pop this from the activity
        setHasNoReferences();        // mark this as not having references in case we get marked
        // set the return value and get outta here
        resultObj = result;
        return;
    }

    trapErrors = false;                   // no more error trapping
    disableVariablepool();                // disable the variable pool from here

    // belt and braces...this restores the activity level to whatever
    // level we had when we made the callout.
    activity->restoreActivationLevel(activityLevel);

    // now process the function return value
    if (functionrc == 0)
    {
        // if we have a result, return it as a string object
        if (funcresult.strptr != NULL)
        {
            resultObj = new_string(funcresult);
            // free the buffer if the user allocated a new one.
            if (funcresult.strptr != default_return_buffer)
            {
                SystemInterpreter::releaseResultMemory(funcresult.strptr);
            }
        }
    }
    else
    {
        reportException(Error_Incorrect_call_external, functionName);
    }

    argCount = 0;                  // make sure we don't try to mark any arguments
    activity->popStackFrame(this); // pop this from the activity
    setHasNoReferences();          // mark this as not having references in case we get marked
}


/**
 * Run a task under the scope of a native activation.  This is
 * generally a bootstrapping call, such as a top-level program call,
 * method translation, etc.
 *
 * @param dispatcher The dispatcher instance we're going to run.
 */
void NativeActivation::run(ActivityDispatcher &dispatcher)
{
    activationType = DISPATCHER_ACTIVATION;  // this is for running a dispatcher
    size_t activityLevel = activity->getActivationLevel();
    // use the default security manager
    securityManager = activity->getInstanceSecurityManager();

    // make the activation hookup
    dispatcher.setContext(activity, this);
    trapErrors = true;                       // we trap errors from here
    try
    {
        // we run this under a callback trap so that the exceptions get processed.
        dispatcher.run();
    }
    catch (ActivityException)
    {
    }
    catch (NativeActivation *)
    {
    }
    trapErrors = false;                      // disable the error trapping

    // if we're not the current kernel holder when things return, make sure we
    // get the lock before we continue
    if (ActivityManager::currentActivity != activity)
    {
        activity->requestAccess();
    }

    // belt and braces...this restores the activity level to whatever
    // level we had when we made the callout.
    activity->restoreActivationLevel(activityLevel);
    if (conditionObj != OREF_NULL)
    {
        // pass the condition information on to the dispatch unig
        dispatcher.handleError(conditionObj);
    }

    activity->popStackFrame(this); // pop this from the activity
    setHasNoReferences();          // mark this as not having references in case we get marked
    return;
}


/**
 * Run a callback under the scope of a native actvation. This is
 * generally a call out, such as a system exit, argument
 * callback, etc.
 *
 * @param dispatcher The dispatcher instance we're going to run.
 */
void NativeActivation::run(CallbackDispatcher &dispatcher)
{
    activationType = CALLBACK_ACTIVATION;    // we're handling a callback
    // use the default security manager
    securityManager = activity->getInstanceSecurityManager();
    size_t activityLevel = activity->getActivationLevel();
    trapErrors = true;               // trap errors on
    try
    {
        // make the activation hookup
        dispatcher.setContext(activity, this);
        activity->releaseAccess();           /* force this to "safe" mode         */
        dispatcher.run();
        activity->requestAccess();           /* now in unsafe mode again          */
    }
    catch (ActivityException)
    {
    }
    catch (NativeActivation *)
    {
    }
    // if we're not the current kernel holder when things return, make sure we
    // get the lock before we continue
    if (ActivityManager::currentActivity != activity)
    {
        activity->requestAccess();
    }

    trapErrors = false;          // back to normal mode for error trapping

    // belt and braces...this restores the activity level to whatever
    // level we had when we made the callout.
    activity->restoreActivationLevel(activityLevel);
    // make sure we handle the error notifications
    if (conditionObj != OREF_NULL)
    {
        // pass the condition information on to the dispatch unig
        dispatcher.handleError(conditionObj);
    }
    return;                             /* and finished                      */
}


/**
 * Run a some type of activity using a fresh activation stack.
 * This generally us a method call such as an uninit method
 * where we wish to run the method and ignore errors.  This runs
 * without releasing the kernel lock.
 *
 * @param dispatcher The dispatcher instance we're going to run.
 */
void NativeActivation::run(TrappingDispatcher &dispatcher)
{
    activationType = TRAPPING_ACTIVATION;    // we're handling a callback
    size_t activityLevel = activity->getActivationLevel();
    trapErrors = true;               // trap errors on
    // the caller may want conditions trapped as well.
    trapConditions = dispatcher.trapConditions();
    try
    {
        // make the activation hookup and run it.  Note that this
        // version does not release the kernel lock
        dispatcher.setContext(activity, this);
        dispatcher.run();
    }
    catch (ActivityException)
    {
    }
    catch (NativeActivation *)
    {
    }
    // if we're not the current kernel holder when things return, make sure we
    // get the lock before we continue
    if (ActivityManager::currentActivity != activity)
    {
        activity->requestAccess();
    }

    trapErrors = false;          // back to normal mode for error trapping

    // belt and braces...this restores the activity level to whatever
    // level we had when we made the callout.
    activity->restoreActivationLevel(activityLevel);
    // make sure we handle the error notifications
    if (conditionObj != OREF_NULL)
    {
        // pass the condition information on to the dispatch unig
        dispatcher.handleError(conditionObj);
    }
    return;                             /* and finished                      */
}


/**
 * Establish the caller's context for native activations
 * that require access to the caller's context.
 */
void NativeActivation::accessCallerContext()
{
    activation = (RexxActivation *)getPreviousStackFrame();
}


/**
 * Check to see if there are deferred syntax errors that need
 * to be raised on return from a native activation.
 */
void NativeActivation::checkConditions()
{
    trapErrors = false;                // no more error trapping

    // if we have a stashed condition object, we need to raise this now
    if (conditionObj != OREF_NULL)
    {
        // we're raising this in the previous stack frame.  If we're actually the
        // base of the stack, there's nothing left to handle this.
        if (!isStackBase())
        {
            // syntax errors are fatal...we need to reraise this
            if (conditionName->strCompare(GlobalNames::SYNTAX))
            {
                // this prevents us from trying to trap this again
                trapErrors = false;
                activity->reraiseException(conditionObj);
            }
            // some other condition (like NOTREADY)
            else
            {
                // find a predecessor Rexx activation
                ActivationBase *_sender = getPreviousStackFrame();
                // raise this in our caller's context.
                if (_sender != OREF_NULL)
                {
                    _sender->trap(conditionName, conditionObj);
                }
                // if the trap is not handled, then we return directly.  The return
                // value (if any) is stored in the condition object
                result = (RexxObject *)conditionObj->get(GlobalNames::RESULT);
            }
        }
    }
}


/**
 * Clear any non-syntax conditions that have been trapped
 * so that they are not propagated to a caller.
 */
void NativeActivation::clearCondition()
{
    // if we have a stashed condition object, we need to see if this is a
    // non-syntax condition
    if (conditionObj != OREF_NULL)
    {
        // syntax errors need to be propagated, only clear other types
        if (!conditionName->strCompare(GlobalNames::SYNTAX))
        {
            clearException();
        }
    }
}


/**
 * Check if a condition of the given type has been trapped.
 *
 * @param name   The condition name.
 *
 * @return True if this activation has trapped a condition of the
 *         indicated type.
 */
bool NativeActivation::checkCondition(RexxString *name)
{
    return conditionName != OREF_NULL && conditionName->strCompare(name);
}


/**
 * Retrieve the set of object variables for the method
 * scope.
 *
 * @return The associated variable dictionary.
 */
VariableDictionary *NativeActivation::methodVariables()
{
    // first retrieval?  Locate the correct variables
    if (objectVariables == OREF_NULL)
    {
        // not a method invocation?
        if (receiver == OREF_NULL)
        {
            // retrieve the variable dictionary from the local context.
            objectVariables = ((RexxActivation *)receiver)->getLocalVariables();
        }
        else
        {
            MethodClass *method = (MethodClass *)executable;
            // ok, we need to locate the object variable dictionary in the method's scope
            objectVariables = receiver->getObjectVariables(getScope());
            // if we are a guarded method, grab the variable lock
            if (objectScope == SCOPE_RELEASED && method->isGuarded())
            {
                objectVariables->reserve(activity);
                objectScope = SCOPE_RESERVED;
            }
        }
    }
    return objectVariables;
}


/**
 * Convert a value to a wholenumber value.
 *
 * @param o        The object to convert.
 * @param position The argument position.
 * @param maxValue The maximum value allowed in the range.
 * @param minValue The minimum range value.
 *
 * @return The converted number.
 */
wholenumber_t NativeActivation::signedIntegerValue(RexxObject *o, size_t position, wholenumber_t maxValue, wholenumber_t minValue)
{
    ssize_t temp;

    // convert using the whole value range
    if (!Numerics::objectToSignedInteger(o, temp, maxValue, minValue))
    {
        reportException(Error_Invalid_argument_range, new_array(new_integer(position + 1), Numerics::wholenumberToObject(minValue), Numerics::wholenumberToObject(maxValue), o));
    }
    return temp;
}


/**
 * Convert a value to a positive wholenumber value.
 *
 * @param o        The object to convert.
 * @param position The argument position.
 *
 * @return The converted number.
 */
wholenumber_t NativeActivation::positiveWholeNumberValue(RexxObject *o, size_t position)
{
    ssize_t temp;

    // convert using the whole value range
    if (!Numerics::objectToSignedInteger(o, temp, Numerics::MAX_WHOLENUMBER, 1))
    {
        reportException(Error_Invalid_argument_positive, position + 1, o);
    }
    return temp;
}


/**
 * Convert a value to a nonnegative wholenumber value.
 *
 * @param o        The object to convert.
 * @param position The argument position.
 *
 * @return The converted number.
 */
wholenumber_t NativeActivation::nonnegativeWholeNumberValue(RexxObject *o, size_t position)
{
    ssize_t temp;

    // convert using the whole value range
    if (!Numerics::objectToSignedInteger(o, temp, Numerics::MAX_WHOLENUMBER, 0))
    {
        reportException(Error_Invalid_argument_nonnegative, position + 1, o);
    }
    return temp;
}



/**
 * Convert a value to a size_t value.
 *
 * @param o        The object to convert.
 * @param position The argument position.
 * @param maxValue The maximum value allowed in the range.
 *
 * @return The converted number.
 */
size_t NativeActivation::unsignedIntegerValue(RexxObject *o, size_t position, stringsize_t maxValue)
{
    size_t temp;

    // convert using the whole value range
    if (!Numerics::objectToUnsignedInteger(o, temp, maxValue))
    {
        reportException(Error_Invalid_argument_range, new_array(new_integer(position + 1), IntegerZero, Numerics::stringsizeToObject(maxValue), o));
    }
    return temp;
}


/**
 * Convert a value to an int64_t value
 *
 * @param o        The object to convert.
 * @param position The argument position.
 *
 * @return The converted number.
 */
int64_t NativeActivation::int64Value(RexxObject *o, size_t position)
{
    int64_t temp;

    // convert using the whole value range
    if (!Numerics::objectToInt64(o, temp))
    {
        reportException(Error_Invalid_argument_range, new_array(new_integer(position + 1), Numerics::int64ToObject(INT64_MIN), Numerics::int64ToObject(INT64_MAX), o));
    }
    return temp;
}


/**
 * Convert a value to a  uint64_t value
 *
 * @param o        The object to convert.
 * @param position The argument position.
 *
 * @return The converted number.
 */
uint64_t NativeActivation::unsignedInt64Value(RexxObject *o, size_t position)
{
    uint64_t temp;

    // convert using the whole value range
    if (!Numerics::objectToUnsignedInt64(o, temp))
    {
        reportException(Error_Invalid_argument_range, new_array(new_integer(position + 1), IntegerZero, Numerics::uint64ToObject(UINT64_MAX), o));
    }
    return temp;
}


/**
 * Convert an object into a CSTRING pointer.
 *
 * @param object The source object.
 *
 * @return The CSTRING version of the object.
 */
const char *NativeActivation::cstring(RexxObject *object)
{
    // force to a string value, making sure to protect the string
    // if a different object is returned.
    RexxString *string = (RexxString *)object->stringValue();
    if (string != object)
    {
        createLocalReference(string);
    }
    // return the pointer to the string data
    return string->getStringData();
}


/**
 * Convert a string in the format 0xnnnnnnnnn into a pointer value.
 *
 * @param object The object to convert.
 *
 * @return The pointer value.
 */
void *NativeActivation::pointerString(RexxObject *object, size_t position)
{
    /* force to a string value           */
    RexxString *string = (RexxString *)object->stringValue();

    void *pointerVal;
    if (sscanf(string->getStringData(), "0x%p", &pointerVal) != 1)
    {
        reportException(Error_Invalid_argument_pointer, position + 1, string);
    }

    return pointerVal;
}


/**
 * Convert an object to a double
 *
 * @param object   The object to convert.
 * @param position The argument position for error reporting.
 *
 * @return The converted double value.
 */
double NativeActivation::getDoubleValue(RexxObject *object, size_t position)
{
    double r;
    // convert to a double, raising an error if this is invalid.
    if (!object->doubleValue(r))
    {
        reportException(Error_Invalid_argument_double, position + 1, object);
    }
    return r;
}


/**
 * Returns "unwrapped" C or C++ object associated with this
 * object instance.  If the variable CSELF does not exist, then
 * NULL is returned.
 *
 * @return The unwrapped value.
 */
void *NativeActivation::cself()
{
    // if this is a method invocation, ask the receiver object to figure this out.
    if (receiver != OREF_NULL)
    {
        // this is necessary to get turn on a guard lock if the method
        // is guarded.  Failure to do this can cause multithreading problems.
        methodVariables();
        return receiver->getCSelf(getScope());
    }
    // nope, call context doesn't allow this
    return OREF_NULL;
}


/**
 * Dereference the pointer value inside a pointer object.
 *
 * @param object The source pointer object.
 *
 * @return The pointer value, or NULL if this is not a pointer object.
 */
void *NativeActivation::pointer(RexxObject *object)
{
    if (!object->isInstanceOf(ThePointerClass))
    {
        return NULL;
    }
    // just unwrap thee pointer
    return ((PointerClass *)object)->pointer();
}


/**
 * Redispatch an activation on a different activity
 *
 * @return The dispatched method result.
 */
RexxObject *NativeActivation::dispatch()
{
    ProtectedObject r;
    if (code != OREF_NULL)
    {
        // just run the method
        run((MethodClass *)executable, (NativeMethod *)code, receiver, messageName, argList, argCount, r);
    }
    return r;
}


/**
 * Return the current digits setting for the context.
 *
 * @return The digits value
 */
wholenumber_t NativeActivation::digits()
{
    // if we have a parent context, return the value from that, otherwise
    // just return the defalue.
    if (activation == OREF_NULL)
    {
        return Numerics::DEFAULT_DIGITS;
    }
    else
    {
        return activation->digits();
    }
}


/**
 * Return the context fuzz() setting.
 *
 * @return The current fuzz setting.
 */
wholenumber_t NativeActivation::fuzz()
{
    // if we have a parent context, return the value from that, otherwise
    // just return the defalue.
    if (activation == OREF_NULL)
    {
        return Numerics::DEFAULT_FUZZ;
    }
    else
    {
        return activation->fuzz();
    }
}


/**
 * Return the current context form setting.
 *
 * @return The form setting.
 */
bool NativeActivation::form()
{
    // if we have a parent context, return the value from that, otherwise
    // just return the defalue.
    if (activation == OREF_NULL)
    {
        return Numerics::DEFAULT_FORM;
    }
    else
    {
        return activation->form();
    }
}


/**
 * Set the digits setting in the calling context.
 *
 * @param _digits The new digits setting.
 */
void NativeActivation::setDigits(wholenumber_t _digits)
{
    // if we're in a call context, set this
    if (activation != OREF_NULL)
    {
        activation->setDigits(_digits);
    }
}

/**
 * Set a new fuzz setting in the current context.
 *
 * @param _fuzz  The new fuzz value.
 */
void NativeActivation::setFuzz(wholenumber_t _fuzz )
{
    // if we're in a call context, set this
    if (activation != OREF_NULL)
    {
        activation->setFuzz(_fuzz);
    }
}


/**
 * update the numeric form setting in the current call context.
 *
 * @param _form  The new form setting.
 */
void NativeActivation::setForm(bool _form)
{
    // only process if we're in a call context.
    if (activation != OREF_NULL)
    {
        activation->setForm(_form);
    }
}


/**
 * Get the numeric settings for this native activation.  If we're
 * running in the direct call context from a Rexx activation, then
 * the settings are those inherited from the Rexx context.  Otherwise,
 * we just return the default numeric settings.
 *
 * @return The current Numeric settings.
 */
const NumericSettings *NativeActivation::getNumericSettings()
{
    if (activation == OREF_NULL)
    {
        return Numerics::getDefaultSettings();
    }
    else
    {
        return activation->getNumericSettings();
    }
}


/**
 * Indicate whether this activation represents the base of the call
 * stack.
 *
 * @return true if this is a base activation.
 */
bool NativeActivation::isStackBase()
{
    return stackBase;
}


/**
 * Return the Rexx context this operates under.  Depending on the
 * context, this could be null.
 *
 * @return The parent Rexx context.
 */
RexxActivation *NativeActivation::getRexxContext()
{
    return activation;    // this might be null
}


/**
 * Return the Rexx executable context that is our immediate
 * caller. Depending on the
 * context, this could be null.
 *
 * @return The parent Rexx context.
 */
BaseExecutable *NativeActivation::getRexxContextExecutable()
{
    // since this should only be used for exit calls, in theory, we always
    // have one of these.
    if (activation != OREF_NULL)
    {
        return activation->getExecutable();
    }
    return OREF_NULL;
}


/**
 * Return the Rexx context object for our immediate caller.
 * Depending on the context, this could be null.
 *
 * @return The parent Rexx context.
 */
RexxObject *NativeActivation::getRexxContextObject()
{
    // since this should only be used for exit calls, in theory, we always
    // have one of these.
    if (activation != OREF_NULL)
    {
        return activation->getContextObject();
    }
    return OREF_NULL;
}


/**
 * Return the Rexx context this operates under.  Depending on the
 * context, this could be null.
 *
 * @return The parent Rexx context.
 */
RexxActivation *NativeActivation::findRexxContext()
{
    // if this is attached to a Rexx context, we can stop here
    if (activation != OREF_NULL)
    {
        return activation;
    }
    // otherwise, have our predecessor see if it can figure this out.  It's likely
    // that is the one we need.
    if (previous != OREF_NULL)
    {
        return previous->findRexxContext();
    }
    // at the base of the stack, no context.
    return OREF_NULL;
}


/**
 * Get the message receiver
 *
 * @return The message receiver.  Returns OREF_NULL if this is not
 *         a message activation.
 */
RexxObject *NativeActivation::getReceiver()
{
    return receiver;
}


/**
 * Get the active method
 *
 * @return The method object for this frame..  Returns OREF_NULL
 *         if this is not a message activation.
 */
MethodClass *NativeActivation::getMethod()

{
    return isMethod() ? (MethodClass *)executable : OREF_NULL;
}


/**
 * Get the security manager context
 *
 * @return The security manager, if there is one.
 */
SecurityManager *NativeActivation::getSecurityManager()
{
    PackageClass *s = getPackageObject();
    if (s != OREF_NULL)
    {
        return s->getSecurityManager();
    }
    return OREF_NULL;     // no security manager on this context.
}


/**
 * Release a variable pool guard lock
 */
void NativeActivation::guardOff()
{
    // if we currently have the lock, release the variables.
    if (objectScope == SCOPE_RESERVED)
    {
        objectVariables->release(activity);
        objectScope = SCOPE_RELEASED;
    }
}


/**
 * Acquire a variable pool guard lock
 */
void NativeActivation::guardOn()
{
    // if there's no receiver object, then this is not a true method call.
    // there's nothing to lock
    if (!isMethod())
    {
        return;
    }
    // first retrieval?   We need to grab the object variables.
    if (objectVariables == OREF_NULL)
    {
        // grab the object variables associated with this object
        objectVariables = receiver->getObjectVariables(getScope());
    }
    // only reserve these if we don't already have them reserved.
    if (objectScope == SCOPE_RELEASED)
    {
        objectVariables->reserve(activity);
        objectScope = SCOPE_RESERVED;
    }
}


/**
 * Wait for a variable in a guard expression to get updated.
 */
void NativeActivation::guardWait()
{
    // we need to wait without locking the variables.  If we
    // held the lock before the wait, we reaquire it after we wake up.
    GuardStatus initial_state = objectScope;

    if (objectScope == SCOPE_RESERVED)
    {
        objectVariables->release(activity);
        objectScope = SCOPE_RELEASED;
    }
    // clear the guard sem on the activity
    activity->guardSet();
    // wait to be woken up by an update
    activity->guardWait();

    // and reset once we get control back
    activity->guardSet();
    // if we released the scope before waiting, then we need to get it
    // back before proceeding.
    if (initial_state == SCOPE_RESERVED)
    {
        objectVariables->reserve(activity);
        objectScope = SCOPE_RESERVED;
    }
}


/**
 * Wait for an object variable to be updated and return the new value. The guard state will be ON on return.
 *
 * @param name   The name of the variable (only simple or stem variables allowed)
 *
 * @return The new value of the variable.
 */
RexxObject *NativeActivation::guardOnWhenUpdated(const char *name)
{
    // if there's no receiver object, then this is not a true method call.
    // there's nothing to lock
    if (!isMethod())
    {
        return OREF_NULL;
    }

    // get a retriever for this variable
    Protected<RexxVariableBase> retriever = getObjectVariableRetriever(name);
    // if this didn't parse, it's an illegal name
    // we also don't allow compound variables here because the source for
    // resolving the tail pieces is not defined.
    if (retriever == OREF_NULL)
    {
        return OREF_NULL;
    }
    // now use this to set the inform status on the variable
    retriever->setGuard(objectVariables);
    // clear the guard sem on the activity
    activity->guardSet();
    // our desired state is to have the guard, so set it now. The wait will release it and
    // reaquire when the variable is updated
    guardOn();

    // now go perform the wait
    guardWait();

    // retrieve the value
    return retriever->getRealValue(methodVariables());
}


/**
 * Wait for an object variable to be updated and return the new
 * value. The guard state will be OFF on return.
 *
 * @param name   The name of the variable (only simple or stem variables allowed)
 *
 * @return The new value of the variable.
 */
RexxObject *NativeActivation::guardOffWhenUpdated(const char *name)
{
    // if there's no receiver object, then this is not a true method call.
    // there's nothing to lock
    if (!isMethod())
    {
        return OREF_NULL;
    }

    // get a retriever for this variable
    Protected<RexxVariableBase> retriever = getObjectVariableRetriever(name);
    // if this didn't parse, it's an illegal name
    // we also don't allow compound variables here because the source for
    // resolving the tail pieces is not defined.
    if (retriever == OREF_NULL)
    {
        return OREF_NULL;
    }
    // now use this to set the inform status on the variable
    retriever->setGuard(objectVariables);
    // clear the guard sem on the activity
    activity->guardSet();
    // our desired state is to have the guard off, so set it now.
    guardOff();

    // now go perform the wait
    guardWait();

    // retrieve the value
    return retriever->getRealValue(methodVariables());
}


/**
 * Enable the variable pool for access.
 */
void NativeActivation::enableVariablepool()
{
    // fetch next calls need to start out fresh
    resetNext();
    variablePoolEnabled = true;
}


/**
 * Turn off variable pool access.
 */
void NativeActivation::disableVariablepool()
{
    resetNext();
    variablePoolEnabled = false;
}


/**
 * Reset the next state of the variable pool.
 */
void NativeActivation::resetNext()
{
    // clear the iterator
    iterator.terminate();
}


/**
 * Fetch the next variable of a variable pool traversal
 *
 * @param name   The returned variable name.
 * @param value  The returned variable value.
 *
 * @return true if we have a variable, false if we've reached the
 *         iteration end.
 */
bool NativeActivation::fetchNext(RexxString *&name, RexxObject *&value)
{
    // is this the first new request?
    if (!iterator.isActive())
    {
        // get the top activation frame and get an iterator from the dictionary
        RexxActivation *activation = activity->getCurrentRexxFrame();
        iterator = activation->getLocalVariables()->iterator();
    }
    // we have an active iterator, and we left it at the previous position
    // at the end of the last request.  Advance it now.
    else
    {
        iterator.next();
    }

    // nothing available?  clear this iterator so the next fetch request will
    // start a new iteration and return a failure
    if (!iterator.isAvailable())
    {
        iterator.terminate();
        return false;
    }

    name = iterator.name();
    value = (RexxObject *)iterator.value();
    return true;
}


/**
 * Trap a condition at this level of the activation stack.
 *
 * @param condition The name of the condition.
 * @param exception_object
 *                  The exception object containing the specifics of the condition.
 *
 * @return false if this activation takes a pass on the condition.  Does not
 *         return at all if the condition is handled.
 */
bool NativeActivation::trap(RexxString *condition, DirectoryClass *exception_object)
{
    // There are two possibilities here.  We're either seeing this because of a
    // propagating syntax condition.  for this case, we trap this and hold it.
    // The other possibility is a condition being raised by an API callback.  That should
    // be the only situation where we see any other condition type.  We also trap that
    // one so it can be raised in the caller's context.

    // we end up seeing this a second time if we're raising the exception on
    // return from an external call or method.
    if (condition->isEqual(GlobalNames::SYNTAX))
    {
        if (trapErrors)
        {
            // record this in case any callers want to know about it.
            setConditionInfo(condition, exception_object);
            // this will unwind back to the calling level, with the
            // exception information recorded.
            throw this;
        }

    }
    else if (trapConditions)
    {
        // record this in case any callers want to know about it.
        setConditionInfo(condition, exception_object);

        // in some contexts, we need to capture without propagating (ie,
        // command redirection handers. We also allow more than one condition
        // to occur, we'll just overwrite the last one.
        if (captureConditions)
        {
            // we handled this
            return true;
        }

        // pretty much the same deal, but we're only handling conditions, and
        // only one condtion, so reset the trap flag
        trapConditions = false;

        // this will unwind back to the calling level, with the
        // exception information recorded.
        throw this;
    }
    return false;
}


/**
 * Transfer a trapped condition into an activation context.
 *
 * @param c      The condition object.
 */
void NativeActivation::setConditionInfo(DirectoryClass *c)
{
    conditionObj = c;
    // we also need to retrieve the condition name from the object
    conditionName = (RexxString *)conditionObj->get(GlobalNames::CONDITION);
}


/**
 * Test if a condition is trapped at this level of the
 * activation stack.
 *
 * @param condition The name of the condition.
 *
 * @return false if this activation takes a pass on the
 *         condition, true if this activation will handle the
 *         condition.
 *         handled.
 */
bool NativeActivation::willTrap(RexxString *condition)
{
    // There are two possibilities here.  We're either seeing this because of a
    // propagating syntax condition.  for this case, we trap this and hold it.
    // The other possibility is a condition being raised by an API callback or we've
    // been created by a trapping dispatcher. In either case, we will trap and hold
    // the condition.

    // we end up seeing this a second time if we're raising the exception on
    // return from an external call or method.
    if (condition->isEqual(GlobalNames::SYNTAX))
    {
        // indicate if error trapping is enabled
        return trapErrors;
    }

    // not a syntax error, so indicate if we have condition trapping
    // enabled
    return trapConditions;
}


/**
 * Raise a condition on behalf of a native method.  This method
 * does not return.
 *
 * @param condition  The condition type to raise.
 * @param description
 *                   The condition description string.
 * @param additional The additional information associated with this condition.
 * @param result     The result object.
 */
void NativeActivation::raiseCondition(RexxString *condition, RexxString *description, RexxObject *additional, RexxObject *_result)
{
    result = _result;
    activity->raiseCondition(condition, OREF_NULL, description, additional, result);

    // We only return here if no activation above us has trapped this.  If we do return, then
    // we terminate the call by throw this up the stack.
    throw this;
}


/**
 * Return the method context arguments as an array.
 *
 * @return An array containing the method arguments.
 */
ArrayClass *NativeActivation::getArguments()
{
    // if we've not requested this before, convert the arguments to an
    // array object.
    if (argArray == OREF_NULL)
    {
        /* create the argument array */
        argArray = new_array(argCount, argList);
        // make sure the array is anchored in our activation
        createLocalReference(argArray);
    }
    return argArray;
}

/**
 * Retrieve a specific argument from the method invocation context.
 *
 * @param index  The argument of interest.
 *
 * @return The object mapped to that argument.  Returns OREF_NULL for
 *         omitted arguments.
 */
RexxObject *NativeActivation::getArgument(size_t index)
{
    if (index <= argCount)
    {
        return argList[index - 1];
    }
    return OREF_NULL;
}

/**
 * Return the super class scope of the current method context.
 *
 * @return The superclass scope object.
 */
RexxClass *NativeActivation::getSuper()
{
    return receiver->superScope(getScope());
}

/**
 * Return the current method scope.
 *
 * @return The current method scope object.
 */
RexxClass *NativeActivation::getScope()
{
    return ((MethodClass *)executable)->getScope();
}

/**
 * Resolve an argument object into a stem object.  The argument
 * object may already be a stem, or may the a variable name
 * used to resolve a stem.
 *
 * @param s      The source object used for the resolution.
 *
 * @return A resolved stem object.  Returns a NULLOBJECT if there is
 *         a resolution problem.
 */
StemClass *NativeActivation::resolveStemVariable(RexxObject *s)
{
    // a NULL argument is not resolvable
    if (s == OREF_NULL)
    {
        return OREF_NULL;
    }


    // is this a stem already?
    if (isStem(s))
    {
        return (StemClass *)s;
    }

    /* force to a string value           */
    RexxString *temp = stringArgument(s, 1);
    // see if we can retrieve this stem
    return (StemClass *)getContextStem(temp);
}


/**
 * retrieve a stem variable stem from the current context.
 *
 * @param name   The name, which does not need to end in a period.
 *
 * @return The stem object associated with that name.
 */
RexxObject* NativeActivation::getContextStem(RexxString *name)
{
    // if this is not a stem name, add it now
    if (!name->endsWith('.'))
    {
        name = name->concatWithCstring(".");
    }

    RexxVariableBase *retriever = VariableDictionary::getVariableRetriever(name);
    // if this didn't parse, it's an illegal name
    // it must also resolve to a stem type...this could be a compound one
    if (retriever == OREF_NULL || !isOfClass(StemVariableTerm, retriever))
    {
        return OREF_NULL;
    }
    // get the variable value
    return retriever->getValue(activation);
}


/**
 * Retrieve the value of a context variable via the API calls.
 * Returns OREF_NULL if the variable does not exist.
 *
 * @param name   The variable name.
 *
 * @return The variable value, if any.
 */
RexxObject* NativeActivation::getContextVariable(const char *name)
{
    RexxVariableBase *retriever = VariableDictionary::getVariableRetriever(new_string(name));
    // if this didn't parse, it's an illegal name
    if (retriever == OREF_NULL)
    {
        return OREF_NULL;
    }
    // all next operations must be reset
    resetNext();

    // have a non-name retriever?
    if (isString(retriever))
    {
        // the value is the retriever
        return (RexxObject *)retriever;
    }
    else
    {
        // get the variable value
        return retriever->getRealValue(activation);
    }
}


/**
 * Retrieve a context variable as a VariableReference instance
 * via the API calls. This will create the variable if the
 * variable does not exist.
 *
 * @param name   The variable name.
 *
 * @return The variable value, if any.
 */
VariableReference *NativeActivation::getContextVariableReference(const char *name)
{
    RexxVariableBase *retriever = VariableDictionary::getVariableRetriever(new_string(name));
    // if this didn't parse, it's an illegal name
    if (retriever == OREF_NULL)
    {
        return OREF_NULL;
    }
    // all next operations must be reset
    resetNext();

    // get the reference. This will return NULL for all non-referencable
    // retriever types
    return retriever->getVariableReference(activation);
}



/**
 * Set a context variable on behalf of an API call.
 *
 * @param name   The name of the variable.
 * @param value  The variable value.
 */
void NativeActivation::setContextVariable(const char *name, RexxObject *value)
{
    // get the REXX activation for the target context
    RexxVariableBase *retriever = VariableDictionary::getVariableRetriever(new_string(name));
    // if this didn't parse, it's an illegal name
    if (retriever == OREF_NULL || isString(retriever))
    {
        return;
    }
    resetNext();               // all next operations must be reset

    // do the assignment
    retriever->set(activation, value);
}


/**
 * Drop a context variable for an API call.
 *
 * @param name   The target variable name.
 */
void NativeActivation::dropContextVariable(const char *name)
{
    // get the REXX activation for the target context
    RexxVariableBase *retriever = VariableDictionary::getVariableRetriever(new_string(name));
    // if this didn't parse, it's an illegal name
    if (retriever == OREF_NULL || isString(retriever))
    {
        return;
    }
    resetNext();               // all next operations must be reset

    // perform the drop
    retriever->drop(activation);
}


/**
 * Retrieve a list of all variables in the current context.
 *
 * @return A directory containing the context variables.
 */
DirectoryClass *NativeActivation::getAllContextVariables()
{
    resetNext();               // all next operations must be reset
    return activation->getAllLocalVariables();
}


/**
 * Get an object variable in the current method scope.  Returns
 * a NULL object reference if the variable does not exist.
 *
 * @param name   The variable name.
 *
 * @return The variable value or OREF_NULL if the variable does not
 *         exist.
 */
RexxObject *NativeActivation::getObjectVariable(const char *name)
{
    // get a retriever for this variable
    Protected<RexxVariableBase> retriever = getObjectVariableRetriever(name);
    // if this didn't parse, it's an illegal name
    // we also don't allow compound variables here because the source for
    // resolving the tail pieces is not defined.
    if (retriever == OREF_NULL)
    {
        return OREF_NULL;
    }
    // retrieve the value
    return retriever->getRealValue(methodVariables());
}


/**
 * Validate and retrieve an accessor for an object variable. Note, this only retrieves top-level variables (simple and stem variables).
 *
 * @param name   The name of the required variable.
 *
 * @return The variable retriever or OREF_NULL if this was not a valid variable.
 */
RexxVariableBase *NativeActivation::getObjectVariableRetriever(const char *name)
{
    Protected<RexxString> target = new_string(name);
    // get the REXX activation for the target context
    Protected<RexxVariableBase>retriever = VariableDictionary::getVariableRetriever(target);
    // if this didn't parse, it's an illegal name
    // we also don't allow compound variables here because the source for
    // resolving the tail pieces is not defined.
    if (retriever == OREF_NULL || isString(retriever) || isOfClassType(CompoundVariableTerm, retriever))
    {
        return OREF_NULL;
    }
    // return the retriever
    return retriever;
}


/**
 * Get a reference to an object variable in the current method
 * scope. This will create the object if it does not already
 * exist.
 *
 * @param name   The variable name.
 *
 * @return The variable reference object.
 */
VariableReference *NativeActivation::getObjectVariableReference(const char *name)
{
    // get a retriever for this variable
    Protected<RexxVariableBase> retriever = getObjectVariableRetriever(name);
    // if this didn't parse, it's an illegal name
    // we also don't allow compound variables here because the source for
    // resolving the tail pieces is not defined.
    if (retriever == OREF_NULL)
    {
        return OREF_NULL;
    }
    // create a reference object for this variable
    return retriever->getVariableReference(methodVariables());
}


/**
 * Set an object variable to a new value.
 *
 * @param name   The name of the variable.
 * @param value  The new variable value.
 */
void NativeActivation::setObjectVariable(const char *name, RexxObject *value)
{
    // get a retriever for this variable
    Protected<RexxVariableBase> retriever = getObjectVariableRetriever(name);
    // if this didn't parse, it's an illegal name
    // we also don't allow compound variables here because the source for
    // resolving the tail pieces is not defined.
    if (retriever == OREF_NULL)
    {
        return;
    }
    // do the assignment
    retriever->set(methodVariables(), value);
}


/**
 * Drop an object variable in the current method scope.
 *
 * @param name   The name of the variable.
 */
void NativeActivation::dropObjectVariable(const char *name)
{
    // get a retriever for this variable
    Protected<RexxVariableBase> retriever = getObjectVariableRetriever(name);
    // if this didn't parse, it's an illegal name
    // we also don't allow compound variables here because the source for
    // resolving the tail pieces is not defined.
    if (retriever == OREF_NULL)
    {
        return;
    }
    // drop the variable
    retriever->drop(methodVariables());
}


/**
 * Resolve a class in the context of the current execution context.
 *
 * @param className The target class name.
 *
 * @return The resolved class (if any).
 */
RexxClass *NativeActivation::findClass(RexxString *className)
{
    RexxClass *classObject;

    // if we have an executable context, use that as the context.
    if (executable != OREF_NULL)
    {
        classObject = executable->findClass(className);
    }
    else
    {
        classObject = Interpreter::findClass(className);
    }

    // we need to filter this to always return a class object
    if (classObject != OREF_NULL && classObject->isInstanceOf(TheClassClass))
    {
        return classObject;
    }
    return OREF_NULL;
}


/**
 * Resolve a class in the context of the caller's execution
 * context.
 *
 * @param className The target class name.
 *
 * @return The resolved class (if any).
 */
RexxClass *NativeActivation::findCallerClass(RexxString *className)
{
    RexxClass *classObject;
    // have a caller context?  if not, just do the default environment searches
    if (activation == OREF_NULL)
    {
        classObject = Interpreter::findClass(className);
    }
    else
    {
        // use the caller activation to resolve this
        classObject = activation->findClass(className);
    }
    // we need to filter this to always return a class object
    if (classObject != OREF_NULL && classObject->isInstanceOf(TheClassClass))
    {
        return classObject;
    }
    return OREF_NULL;
}


/**
 * Retrieve the source object for the current context, if there is one.
 *
 * @return The source object associated with any Method or Routine currently
 *         being run.
 */
PackageClass *NativeActivation::getPackageObject()
{
    if (executable != OREF_NULL)
    {
        return executable->getPackageObject();
    }
    return OREF_NULL;
}


/**
 * Handle a request chain for the variable pool interface API.
 *
 * @param pshvblock The shared variable block for the request.
 *
 * @return The composit return code for the chain of requests.
 */
RexxReturnCode NativeActivation::variablePoolInterface(PSHVBLOCK pshvblock)
{
    // this is not allowed asynchronously
    if (!isVariablePoolEnabled())
    {
        return RXSHV_NOAVL;
    }

    // this is a composit return code for all operations we're asked to perform
    RexxReturnCode retcode = 0;

    try
    {
        // variable pool requests can be chained, so we process
        // all of the requests
        while (pshvblock)
        {
            variablePoolRequest(pshvblock);
            // the main return code is a composite, so
            // OR in the return value from each block.
            retcode |= pshvblock->shvret;
            pshvblock = pshvblock->shvnext;
        }
        return retcode;

    }
    // intercept any termination failures
    catch (ActivityException)
    {
        pshvblock->shvret |= (uint8_t)RXSHV_MEMFL;
        retcode |= pshvblock->shvret;
        return retcode;
    }
}


/**
 * Get a variable retriever for the target variable.
 *
 * @param pshvblock The variable pool request block.
 * @param symbolic  The symbolic vs. direct indicator.
 *
 * @return A variable retriever for the variable.  Returns OREF_NULL
 *         if the variable is not resolvable from the name.
 */
RexxVariableBase *NativeActivation::variablePoolGetVariable(PSHVBLOCK pshvblock, bool symbolic)
{
    // no name given?  Thats a failure for this request
    if (pshvblock->shvname.strptr==NULL)
    {
        pshvblock->shvret|=RXSHV_BADN;
    }
    else
    {
        // get the variable as a string
        RexxString *variable = new_string(pshvblock->shvname);
        RexxVariableBase *retriever = OREF_NULL;
        // the type of retriever and the valid name rules depend on
        // whether this is a symbolic or direct request
        if (symbolic)
        {
            retriever = VariableDictionary::getVariableRetriever(variable);
        }
        else
        {
            retriever = VariableDictionary::getDirectVariableRetriever(variable);
        }
        // mark as a failure if this did not parse to a valid variable name.
        if (retriever == OREF_NULL || isString(retriever))
        {
            pshvblock->shvret|=RXSHV_BADN;
            return OREF_NULL;
        }
        else
        {
            // non-next requests of any type reset the variable iterator.
            resetNext();
            return retriever;
        }
    }
    return OREF_NULL;
}


/**
 * Perform a variable pool fetch operation.
 *
 * @param pshvblock The operation shared variable block.
 */
void NativeActivation::variablePoolFetchVariable(PSHVBLOCK pshvblock)
{
    RexxVariableBase *retriever = variablePoolGetVariable(pshvblock, pshvblock->shvcode == RXSHV_SYFET);
    RexxObject *value = OREF_NULL;
    if (retriever != OREF_NULL)
    {
        // have a real variable retriever and the variable does
        // exist currently?  We flag new variables in the request block
        if (!retriever->exists(activation))
        {
            pshvblock->shvret |= RXSHV_NEWV;
        }
        // get the variable value
        value = retriever->getValue(activation);

        // copy the value into the block
        pshvblock->shvret |= copyValue(value, &pshvblock->shvvalue, (size_t *)&pshvblock->shvvaluelen);
    }
}


/**
 * Perform a variable pool set operation.
 *
 * @param pshvblock The operation shared variable block.
 */
void NativeActivation::variablePoolSetVariable(PSHVBLOCK pshvblock)
{
    // get a retriever for this variable name.
    RexxVariableBase *retriever = variablePoolGetVariable(pshvblock, pshvblock->shvcode == RXSHV_SYSET);
    if (retriever != OREF_NULL)
    {
        // if assigning to a new variable, mark this as a new one.
        if (!retriever->exists(activation))
        {
            pshvblock->shvret |= RXSHV_NEWV;
        }
        // do the assignment
        retriever->set(activation, new_string(pshvblock->shvvalue));
    }
}


/**
 * Perform a variable pool drop operation.
 *
 * @param pshvblock The operation shared variable block.
 */
void NativeActivation::variablePoolDropVariable(PSHVBLOCK pshvblock)
{
    RexxVariableBase *retriever = variablePoolGetVariable(pshvblock, pshvblock->shvcode == RXSHV_SYDRO);
    if (retriever != OREF_NULL)
    {
        // dropping an unassigned variable also sets the new flag.
        if (!retriever->exists(activation))
        {
            pshvblock->shvret |= RXSHV_NEWV;
        }
        retriever->drop(activation);
    }
}


/**
 * Perform a variable pool fetch next operation.
 *
 * @param pshvblock The operation shared variable block.
 */
void NativeActivation::variablePoolNextVariable(PSHVBLOCK pshvblock)
{
    RexxString *name;
    RexxObject *value;

    // get the next variable from the iterator.  Flag
    // as the last variable if this fails
    if (!fetchNext(name, value))
    {
        pshvblock->shvret |= RXSHV_LVAR;
    }
    else
    {
        // need to copy both the name and the value into the block.
        pshvblock->shvret |= copyValue(name, &pshvblock->shvname, &pshvblock->shvnamelen);
        pshvblock->shvret |= copyValue(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
    }
}


/**
 * Perform a variable pool fetch private operation.
 *
 * @param pshvblock The operation shared variable block.
 */
void NativeActivation::variablePoolFetchPrivate(PSHVBLOCK pshvblock)
{
    // must have a name
    if (pshvblock->shvname.strptr==NULL)
    {
        pshvblock->shvret|=RXSHV_BADN;   /* this is bad                       */
    }
    else
    {
        // get the variable as a string
        const char *variable = pshvblock->shvname.strptr;
        // Interpreter VERSION string?
        if (strcmp(variable, "VERSION") == 0)
        {
            pshvblock->shvret |= copyValue(Interpreter::getVersionString(), &pshvblock->shvvalue, &pshvblock->shvvaluelen);
        }
        // the current queue name?
        else if (strcmp(variable, "QUENAME") == 0)
        {
            pshvblock->shvret |= copyValue(Interpreter::getCurrentQueue(), &pshvblock->shvvalue, &pshvblock->shvvaluelen);
        }
        // the SOURCE string?
        else if (strcmp(variable, "SOURCE") == 0)
        {
            RexxString *value = activation->sourceString();
            pshvblock->shvret |= copyValue(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
        }
        // the parameter count (arguments in internal parlance)
        else if (strcmp(variable, "PARM") == 0)
        {
            RexxInteger *value = new_integer(activation->getProgramArgumentCount());
            pshvblock->shvret |= copyValue(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
        }
        // request for a specific argument?
        else if (!memcmp(variable, "PARM.", sizeof("PARM.") - 1))
        {
            wholenumber_t value_position;
            // extract the tail piece and try to convert to an integer.
            // any failure is a bad name.
            RexxString *tail = new_string(variable + strlen("PARM."));
            if (!tail->numberValue(value_position) || value_position <= 0)
            {
                pshvblock->shvret|=RXSHV_BADN;
            }
            else
            {
                // get the argument from the parent Rexx activation.
                RexxObject *value = activation->getProgramArgument(value_position);
                // non-existant args are always returned as a null string
                if (value == OREF_NULL)
                {
                    value = GlobalNames::NULLSTRING;
                }
                pshvblock->shvret |= copyValue(value, &pshvblock->shvvalue, (size_t *)&pshvblock->shvvaluelen);
            }
        }
        // an unknown name
        else
        {
            pshvblock->shvret|=RXSHV_BADN;
        }
    }
}


/**
 * Process a single variable pool request.
 *
 * @param pshvblock The request block for this request.
 */
void NativeActivation::variablePoolRequest(PSHVBLOCK pshvblock)
{
    // clear any return code in this block.  Return codes get
    // OR'ed into this value.
    pshvblock->shvret = 0;

    switch (pshvblock->shvcode)
    {
        case RXSHV_FETCH:
        case RXSHV_SYFET:
        {
            variablePoolFetchVariable(pshvblock);
            break;
        }
        case RXSHV_SET:
        case RXSHV_SYSET:
        {
            variablePoolSetVariable(pshvblock);
            break;
        }
        case RXSHV_DROPV:
        case RXSHV_SYDRO:
        {
            variablePoolDropVariable(pshvblock);
            break;
        }
        case RXSHV_NEXTV:
        {
            variablePoolNextVariable(pshvblock);
            break;
        }
        case RXSHV_PRIV:
        {
            variablePoolFetchPrivate(pshvblock);
            break;
        }
        // an unknown function
        default:
        {
            pshvblock->shvret |= RXSHV_BADF;
            break;
        }
    }
}


/**
 * Copy a value for a variable pool request, with checks for
 * truncation.
 *
 * @param value    The value to copy.
 * @param rxstring The target RXSTRING.
 * @param length   The max length we can copy.
 *
 * @return A return code to be included in the composite return value.
 */
RexxReturnCode NativeActivation::copyValue(RexxObject * value, CONSTRXSTRING *rxstring, size_t *length)
{
    RXSTRING temp;

    temp.strptr = const_cast<char *>(rxstring->strptr);
    temp.strlength = rxstring->strlength;

    RexxReturnCode rc = copyValue(value, &temp, length);

    rxstring->strptr = temp.strptr;
    rxstring->strlength = temp.strlength;
    return rc;
}


/**
 * Copy a value for a variable pool request, with checks for
 * truncation.
 *
 * @param value    The value to copy.
 * @param rxstring The target RXSTRING.
 * @param length   The max length we can copy.
 *
 * @return A return code to be included in the composite return value.
 */
RexxReturnCode NativeActivation::copyValue(RexxObject * value, RXSTRING *rxstring, size_t *length)
{
    uint32_t rc = 0;

    RexxString *stringVal = value->stringValue();
    size_t string_length = stringVal->getLength();
    // caller allowing us to allocate this?
    if (rxstring->strptr == NULL)
    {
        rxstring->strptr = (char *)SystemInterpreter::allocateResultMemory(string_length + 1);
        if (rxstring->strptr == NULL)
        {
            return RXSHV_MEMFL;
        }
        rxstring->strlength = string_length + 1;
    }
    // is the buffer too short for the value?  We just copy what we can
    // and set the truncation flag.
    if (string_length > rxstring->strlength)
    {
        rc = RXSHV_TRUNC;                      /* set truncated return code      */
                                               /* copy the short piece           */
        memcpy(rxstring->strptr, stringVal->getStringData(), rxstring->strlength);
    }
    else
    {
        // can copy everything...add a terminating null if there is room.  Not
        // having room for the null is not cause for setting the trunc flag.  The
        // null is just a convenience, not part of the value.
        memcpy(rxstring->strptr, stringVal->getStringData(), string_length);
        if (rxstring->strlength > string_length)
        {
            rxstring->strptr[string_length] = '\0';
        }
        // the returned length does not include the null.
        rxstring->strlength = string_length;
    }
    *length = string_length;
    return rc;
}


/**
 * Perform a sort on stem data.  If everything works correctly,
 * this returns zero, otherwise an appropriate error value.
 *
 * @param stemname The name of the stem variable.
 * @param order    The sort order (ascending or descending).
 * @param type     The type of sort.
 * @param start    The starting element.
 * @param end      The ending element.
 * @param firstcol The first comparison column.
 * @param lastcol  The last comparison column.
 *
 * @return The completion code.
 */
int NativeActivation::stemSort(StemClass *stem, const char *tailExtension, int order, int type, wholenumber_t start, wholenumber_t end, wholenumber_t firstcol, wholenumber_t lastcol)
{
    Protected<RexxString> tail;

    // Damn, someone is trying to sort a subsection.  We need to pass that
    // tail pieces.
    if (tailExtension != NULL)
    {
        tail = new_upper_string(tailExtension);
    }

    // go perform the sort operation.
    return stem->sort(tail, order, type, (size_t)start, (size_t)end, (size_t)firstcol, (size_t)lastcol);
}


/**
 * Implement the equivalent of a FORWARD CONTINUE instruction
 * as an API call.  All null arguments are filled in with
 * the method context args.
 *
 * @param to     The target object.  Defaults to SELF
 * @param msg    The target message name.  Defaults to current message.
 * @param super  Any superclass override.  Defaults to none.
 * @param args   The message arguments.  Defaults to current argument set.
 *
 * @return The message send result.
 */
void NativeActivation::forwardMessage(RexxObject *to, RexxString *msg, RexxClass *super, ArrayClass *args, ProtectedObject &_result)
{
    // process all of the non-overridden values
    if (to == OREF_NULL)
    {
        to = getSelf();
    }
    if (msg == OREF_NULL)
    {
        msg = getMessageName();
    }
    if (args == OREF_NULL)
    {
        args = getArguments();
    }

    // no super class override?  Normal message send
    if (super == OREF_NULL)
    {
        to->messageSend(msg, args->messageArgs(), args->messageArgCount(), _result);
    }
    else
    {
        to->messageSend(msg, args->messageArgs(), args->messageArgCount(), super, _result);
    }
}


/**
 * Create a stack frame for exception tracebacks.
 *
 * @return A StackFrame instance for this activation.
 */
StackFrameClass *NativeActivation::createStackFrame()
{
    if (receiver == OREF_NULL)
    {
        ArrayClass *info = new_array(getMessageName());
        ProtectedObject p(info);

        RexxString *message = activity->buildMessage(Message_Translations_compiled_routine_invocation, info);
        p = message;
        return new StackFrameClass(StackFrameClass::FRAME_ROUTINE, getMessageName(), (BaseExecutable *)getExecutableObject(), NULL, getArguments(), message, SIZE_MAX);
    }
    else
    {
        ArrayClass *info = new_array(getMessageName(), ((MethodClass *)executable)->getScopeName());
        ProtectedObject p(info);

        RexxString *message = activity->buildMessage(Message_Translations_compiled_method_invocation, info);
        p = message;
        return new StackFrameClass(StackFrameClass::FRAME_METHOD, getMessageName(), (BaseExecutable *)getExecutableObject(), receiver, getArguments(), message, SIZE_MAX);
    }
}


/**
 * Allocate memory in the current object context.
 *
 * @param size   The requested memory size.
 *
 * @return The newly allocated data buffer.
 */
void *NativeActivation::allocateObjectMemory(size_t size)
{
    return receiver->allocateObjectMemory(size);
}


/**
 * Rellocate memory in the current object context.
 *
 * @param pointer The data buffer to reallocate
 * @param size    The requested memory size.
 *
 * @return The new data pointer.
 */
void *NativeActivation::reallocateObjectMemory(void *pointer, size_t size)
{
    return receiver->reallocateObjectMemory(pointer, size);
}


/**
 * Free previously allocated object memory.
 *
 * @param pointer The memory to release.
 */
void NativeActivation::freeObjectMemory(void *pointer)
{
    return receiver->freeObjectMemory(pointer);
}


