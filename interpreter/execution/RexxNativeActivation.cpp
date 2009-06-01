/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                      RexxNativeActivation.cpp  */
/*                                                                            */
/* Primitive Native Activation Class                                          */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "MethodClass.hpp"
#include "RexxNativeCode.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "BufferClass.hpp"
#include "MessageClass.hpp"
#include "RexxVariableDictionary.hpp"
#include "SourceFile.hpp"
#include "RexxCode.hpp"
#include "RexxInstruction.hpp"
#include "ExpressionBaseVariable.hpp"
#include "ProtectedObject.hpp"
#include "PointerClass.hpp"
#include "ActivityDispatcher.hpp"
#include "CallbackDispatcher.hpp"
#include "Interpreter.hpp"
#include "SystemInterpreter.hpp"

#include <math.h>
#include <limits.h>




/**
 * Initialize an activation for direct caching in the activation
 * cache.  At this time, this is not an executable activation
 */
RexxNativeActivation::RexxNativeActivation()
{
    this->setHasNoReferences();          // nothing referenced from this either
}


/**
 * Constructor for a new native activation used to create a
 * callback context for exit call outs.
 *
 * @param _activity The activity we're running under.
 */
RexxNativeActivation::RexxNativeActivation(RexxActivity *_activity, RexxActivation*_activation)
{
    this->clearObject();                 /* start with a fresh object         */
    this->activity = _activity;      /* the activity running on           */
    this->activation = _activation;  // our parent context
}


/**
 * Constructor for a new native activation used to create a
 * callback context for exit call outs.
 *
 * @param _activity The activity we're running under.
 */
RexxNativeActivation::RexxNativeActivation(RexxActivity *_activity)
{
    this->clearObject();                 /* start with a fresh object         */
    this->activity = _activity;      /* the activity running on           */
}


void RexxNativeActivation::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->previous);
    memory_mark(this->executable);
    memory_mark(this->argArray);
    memory_mark(this->receiver);
    memory_mark(this->activity);
    memory_mark(this->activation);
    memory_mark(this->msgname);
    memory_mark(this->savelist);
    memory_mark(this->result);
    memory_mark(this->nextstem);
    memory_mark(this->compoundelement);
    memory_mark(this->nextcurrent);
    memory_mark(this->objectVariables);
    memory_mark(this->conditionObj);
    memory_mark(this->securityManager);

    /* We're hold a pointer back to our arguments directly where they */
    /* are created.  Since in some places, this argument list comes */
    /* from the C stack, we need to handle the marker ourselves. */
    size_t i;
    for (i = 0; i < argcount; i++)
    {
        memory_mark(arglist[i]);
    }
}

void RexxNativeActivation::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->previous);
    memory_mark_general(this->executable);
    memory_mark_general(this->argArray);
    memory_mark_general(this->receiver);
    memory_mark_general(this->activity);
    memory_mark_general(this->activation);
    memory_mark_general(this->msgname);
    memory_mark_general(this->savelist);
    memory_mark_general(this->result);
    memory_mark_general(this->nextstem);
    memory_mark_general(this->compoundelement);
    memory_mark_general(this->nextcurrent);
    memory_mark_general(this->objectVariables);
    memory_mark_general(this->conditionObj);
    memory_mark_general(this->securityManager);

    /* We're hold a pointer back to our arguments directly where they */
    /* are created.  Since in some places, this argument list comes */
    /* from the C stack, we need to handle the marker ourselves. */
    size_t i;
    for (i = 0; i < argcount; i++)
    {
        memory_mark_general(arglist[i]);
    }
}


void RexxNativeActivation::reportSignatureError()
/******************************************************************************/
/* Function:  Report a method signature error                                 */
/******************************************************************************/
{
    if (activationType == METHOD_ACTIVATION)
    {
        reportException(Error_Incorrect_method_signature);
    }
    else
    {
        reportException(Error_Incorrect_call_signature);
    }
}


void RexxNativeActivation::reportStemError(size_t position, RexxObject *object)
/******************************************************************************/
/* Function:  Report a method signature error                                 */
/******************************************************************************/
{
    if (activationType == METHOD_ACTIVATION)
    {
        reportException(Error_Incorrect_method_nostem, position + 1, object);
    }
    else
    {
        reportException(Error_Incorrect_call_nostem, position + 1, object);
    }
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
void RexxNativeActivation::processArguments(size_t _argcount, RexxObject **_arglist, uint16_t *argumentTypes,
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
            case REXX_VALUE_OSELF:                /* reference to SELF                 */
            {
                // this doesn't make any sense for a function call
                if (activationType != METHOD_ACTIVATION)
                {
                    reportSignatureError();
                }
                // fill in the receiver object and mark it...
                descriptors[outputIndex].value.value_RexxObjectPtr = (RexxObjectPtr)this->receiver;
                descriptors[outputIndex].flags = ARGUMENT_EXISTS | SPECIAL_ARGUMENT;
                break;
            }

            // reference to the method scope...if this is a function call,
            // then this will be OREF NULL.
            case REXX_VALUE_SCOPE:
            {
                // this doesn't make any sense for a function call
                if (activationType != METHOD_ACTIVATION)
                {
                    reportSignatureError();
                }
                // fill in the receiver object and mark it...
                descriptors[outputIndex].value.value_RexxObjectPtr = (RexxObjectPtr)this->getScope();
                descriptors[outputIndex].flags = ARGUMENT_EXISTS | SPECIAL_ARGUMENT;
                break;
            }

            // reference to the superclass scope...if this is a function call,
            // then this will be OREF NULL.
            case REXX_VALUE_SUPER:
            {
                // this doesn't make any sense for a function call
                if (activationType != METHOD_ACTIVATION)
                {
                    reportSignatureError();
                }
                // fill in the receiver object and mark it...
                descriptors[outputIndex].value.value_RexxObjectPtr = (RexxClassObject)this->getSuper();
                descriptors[outputIndex].flags = ARGUMENT_EXISTS | SPECIAL_ARGUMENT;
                break;
            }

            case REXX_VALUE_CSELF:                /* reference to CSELF                */
            {
                // this doesn't make any sense for a function call
                if (activationType != METHOD_ACTIVATION)
                {
                    reportSignatureError();
                }
                descriptors[outputIndex].value.value_POINTER = this->cself();
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
                descriptors[outputIndex].value.value_CSTRING = (CSTRING)this->msgname->getStringData();
                break;
            }

            // this is a real argument taken from the argument list
            default:                         /* still within argument bounds?     */
            {
                if (inputIndex < _argcount && _arglist[inputIndex] != OREF_NULL)
                {
                    // all of these arguments exist
                    descriptors[outputIndex].flags = ARGUMENT_EXISTS;
                    RexxObject *argument = _arglist[inputIndex];    /* get the next argument             */
                    switch (type)
                    {               /* process this type                 */

                        case REXX_VALUE_RexxObjectPtr:  /* arbitrary object reference        */
                        {
                            descriptors[outputIndex].value.value_RexxObjectPtr = (RexxObjectPtr)argument;
                            break;
                        }

                        case REXX_VALUE_int:            /* integer value                     */
                        {
                            // convert and copy                  */
                            descriptors[outputIndex].value.value_int = (int)signedIntegerValue(argument, inputIndex, INT_MAX, INT_MIN);
                            break;
                        }

                        case REXX_VALUE_int8_t:            /* 8-bit integer value               */
                        {
                            descriptors[outputIndex].value.value_int8_t = (int8_t)signedIntegerValue(argument, inputIndex, INT8_MAX, INT8_MIN);
                            break;
                        }

                        case REXX_VALUE_int16_t:            /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_int16_t = (int16_t)signedIntegerValue(argument, inputIndex, INT16_MAX, INT16_MIN);
                            break;
                        }

                        case REXX_VALUE_int32_t:            /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_int32_t = (int32_t)signedIntegerValue(argument, inputIndex, INT32_MAX, INT32_MIN);
                            break;
                        }

                        case REXX_VALUE_int64_t:            /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_int64_t = (int64_t)int64Value(argument, inputIndex);
                            break;
                        }

                        case REXX_VALUE_ssize_t:            /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_ssize_t = (ssize_t)signedIntegerValue(argument, inputIndex, SSIZE_MAX, -SSIZE_MAX - 1);
                            break;
                        }

                        case REXX_VALUE_intptr_t:         /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_intptr_t = (intptr_t)signedIntegerValue(argument, inputIndex, INTPTR_MAX, INTPTR_MIN);
                            break;
                        }

                        case REXX_VALUE_uint8_t:            /* 8-bit integer value               */
                        {
                            descriptors[outputIndex].value.value_uint8_t = (uint8_t)unsignedIntegerValue(argument, inputIndex, UINT8_MAX);
                            break;
                        }

                        case REXX_VALUE_uint16_t:            /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_uint16_t = (uint16_t)unsignedIntegerValue(argument, inputIndex, UINT16_MAX);
                            break;
                        }

                        case REXX_VALUE_uint32_t:            /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_uint32_t = (uint32_t)unsignedIntegerValue(argument, inputIndex, UINT32_MAX);
                            break;
                        }

                        case REXX_VALUE_uint64_t:            /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_uint64_t = (uint64_t)unsignedInt64Value(argument, inputIndex);
                            break;
                        }

                        case REXX_VALUE_size_t:            /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_size_t = (size_t)unsignedIntegerValue(argument, inputIndex, SIZE_MAX);
                            break;
                        }

                        case REXX_VALUE_uintptr_t:         /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_uintptr_t = (uintptr_t)unsignedIntegerValue(argument, inputIndex, UINTPTR_MAX);
                            break;
                        }

                        case REXX_VALUE_logical_t:         /* integer value                     */
                        {
                            descriptors[outputIndex].value.value_logical_t = argument->truthValue(Error_Logical_value_method);
                            break;
                        }

                        // The Rexx whole number one is checked against the human digits limit
                        case REXX_VALUE_wholenumber_t:  /* number value                      */
                        {
                            descriptors[outputIndex].value.value_wholenumber_t = (wholenumber_t)signedIntegerValue(argument, inputIndex, Numerics::MAX_WHOLENUMBER, Numerics::MIN_WHOLENUMBER);
                            break;
                        }

                        // an unsigned string number value
                        case REXX_VALUE_stringsize_t:
                        {
                            descriptors[outputIndex].value.value_stringsize_t = (stringsize_t)unsignedIntegerValue(argument, inputIndex, Numerics::MAX_STRINGSIZE);
                            break;
                        }

                        case REXX_VALUE_double:         /* double value                      */
                        {
                            descriptors[outputIndex].value.value_double = this->getDoubleValue(argument, inputIndex);
                            break;
                        }


                        case REXX_VALUE_float:          /* float value                      */
                        {
                            descriptors[outputIndex].value.value_float = (float)this->getDoubleValue(argument, inputIndex);
                            break;
                        }

                        case REXX_VALUE_CSTRING:        /* ASCII-Z string value              */
                        {
                            descriptors[outputIndex].value.value_CSTRING = this->cstring(argument);
                            break;
                        }

                        case REXX_VALUE_RexxStringObject: /* Required STRING object            */
                        {
                            /* force to a string value           */
                            RexxString *temp = stringArgument(argument, inputIndex + 1) ;
                            // if this forced a string object to be created,
                            // we need to protect it here.
                            if (temp != argument)
                            {
                                                     /* make it safe                      */
                                createLocalReference(temp);
                            }
                            /* set the result in                 */
                            descriptors[outputIndex].value.value_RexxStringObject = (RexxStringObject)temp;
                            break;

                        }

                        case REXX_VALUE_RexxArrayObject: /* Required ARRAY object            */
                        {
                            /* force to a string value           */
                            RexxArray *temp = arrayArgument(argument, inputIndex + 1) ;
                            // if this forced a string object to be created,
                            // we need to protect it here.
                            if (temp != argument)
                            {
                                                     /* make it safe                      */
                                createLocalReference(temp);
                            }
                            /* set the result in                 */
                            descriptors[outputIndex].value.value_RexxArrayObject = (RexxArrayObject)temp;
                            break;

                        }

                        case REXX_VALUE_RexxStemObject: /* Required Stem object            */
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
                                /* set the result in                 */
                                descriptors[outputIndex].value.value_RexxStemObject = (RexxStemObject)argument;
                                break;
                            }

                            // this spesn't make any sense for a function call
                            if (activationType == METHOD_ACTIVATION)
                            {
                                reportStemError(inputIndex, argument);
                            }

                            /* force to a string value           */
                            RexxString *temp = argument->requestString();
                            if ((RexxObject *)temp == TheNilObject)
                            {
                                reportStemError(inputIndex, argument);
                            }
                            // if this forced a string object to be created,
                            // we need to protect it here.
                            if (temp != argument)
                            {
                                                     /* make it safe                      */
                                createLocalReference(temp);
                            }

                            // see if we can retrieve this stem
                            RexxObject *stem = getContextStem(temp);
                            if (stem == OREF_NULL)
                            {
                                reportStemError(inputIndex, argument);
                            }
                            /* set the result in                 */
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
                            /* set the result in                 */
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
                            descriptors[outputIndex].value.value_POINTER = this->pointer(argument);
                            break;
                        }

                        case REXX_VALUE_POINTERSTRING:
                        {
                            descriptors[outputIndex].value.value_POINTERSTRING = this->pointerString(argument, inputIndex);
                            break;
                        }

                        default:                   /* something messed up               */
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
                                       /* just raise the error              */
                        reportException(Error_Invalid_argument_noarg, inputIndex + 1);
                    }

                    // this is a non-specified argument
                    descriptors[outputIndex].flags = 0;
                    switch (type)
                    {

                        case REXX_VALUE_RexxObjectPtr:     // no object here
                        case REXX_VALUE_int:               // non-integer value
                        case REXX_VALUE_wholenumber_t:     // non-existent long
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
                        case REXX_VALUE_POINTERSTRING:
                        {
                            // set this as a 64-bit value to clear everything out
                            descriptors[outputIndex].value.value_int64_t = 0;
                            break;
                        }
                        case REXX_VALUE_double:         /* non-existent double               */
                        {
                            descriptors[outputIndex].value.value_double = 0.0;
                            break;
                        }
                        case REXX_VALUE_float:          /* non-existent double               */
                        {
                            descriptors[outputIndex].value.value_float = 0.0;
                            break;
                        }
                                                   /* still an error if not there       */
                        default:                   /* something messed up               */
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
        outputIndex++;                 /* step to the next argument         */
        argumentTypes++;               // and the next output position pointer
    }
    if (inputIndex < _argcount && !usedArglist)    /* extra, unwanted arguments?        */
    {
                                         /* got too many                      */
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
RexxArray *RexxNativeActivation::valuesToObject(ValueDescriptor *value, size_t count)
{
    RexxArray *r = new_array(count);
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
RexxObject *RexxNativeActivation::valueToObject(ValueDescriptor *value)
{
    switch (value->type)
    {
        case REXX_VALUE_RexxObjectPtr:          // object reference.  All object types get
        case REXX_VALUE_RexxStringObject:       // returned as a Rexx object
        case REXX_VALUE_RexxArrayObject:
        case REXX_VALUE_RexxClassObject:
        case REXX_VALUE_RexxStemObject:
        {
            return (RexxObject *)value->value.value_RexxObjectPtr; // just return the object value
        }

        case REXX_VALUE_int:                    /* integer value                     */
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_int);
        }

        case REXX_VALUE_int8_t:                         /* integer value                     */
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_int8_t);
        }

        case REXX_VALUE_int16_t:                        /* integer value                     */
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_int16_t);
        }

        case REXX_VALUE_int32_t:                        /* integer value                     */
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_int32_t);
        }

        case REXX_VALUE_int64_t:                        /* integer value                     */
        {
            return Numerics::int64ToObject(value->value.value_int64_t);
        }

        case REXX_VALUE_intptr_t:                       /* integer value                     */
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_intptr_t);
        }

        case REXX_VALUE_uint8_t:                         /* integer value                     */
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_uint8_t);
        }

        case REXX_VALUE_uint16_t:                        /* integer value                     */
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_uint16_t);
        }

        case REXX_VALUE_uint32_t:                        /* integer value                     */
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_uint32_t);
        }

        case REXX_VALUE_uint64_t:                        /* integer value                     */
        {
            return Numerics::uint64ToObject(value->value.value_uint64_t);
        }

        case REXX_VALUE_uintptr_t:                       /* integer value                     */
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_uintptr_t);
        }

        case REXX_VALUE_logical_t:                        /* logical value                     */
        {
            return value->value.value_logical_t == 0 ? TheFalseObject : TheTrueObject;
        }

        case REXX_VALUE_size_t:                        /* integer value                     */
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_size_t);
        }

        case REXX_VALUE_ssize_t:                        /* integer value                     */
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_size_t);
        }

        case REXX_VALUE_wholenumber_t:        /* long integer value                */
        {
            return Numerics::wholenumberToObject((wholenumber_t)value->value.value_wholenumber_t);
        }

        case REXX_VALUE_stringsize_t:     /* long integer value                */
        {
            return Numerics::stringsizeToObject((stringsize_t)value->value.value_stringsize_t);
        }

        case REXX_VALUE_double:                 /* double value                      */
        {
            return new_string(value->value.value_double);
        }

        case REXX_VALUE_float:                  /* float value                      */
        {
            return new_string(value->value.value_float);
        }

        case REXX_VALUE_CSTRING:                /* ASCII-Z string                    */
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
bool RexxNativeActivation::objectToValue(RexxObject *o, ValueDescriptor *value)
{
    switch (value->type)
    {
        case REXX_VALUE_RexxObjectPtr:          /* Object reference                  */
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
            /* set the result in                 */
            value->value.value_RexxClassObject = (RexxClassObject)o;
            return true;
        }
        case REXX_VALUE_int:            /* integer value                     */
        {
            ssize_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToSignedInteger(o, temp, INT_MAX, INT_MIN);
            value->value.value_int = (int)temp;
            return success;
        }

        case REXX_VALUE_int8_t:            /* 8-bit integer value               */
        {
            ssize_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToSignedInteger(o, temp, INT8_MAX, INT8_MIN);
            value->value.value_int8_t = (int8_t)temp;
            return success;
        }

        case REXX_VALUE_int16_t:            /* integer value                     */
        {
            ssize_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToSignedInteger(o, temp, INT16_MAX, INT16_MIN);
            value->value.value_int16_t = (int16_t)temp;
            return success;
        }

        case REXX_VALUE_int32_t:            /* integer value                     */
        {
            ssize_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToSignedInteger(o, temp, INT32_MAX, INT32_MIN);
            value->value.value_int32_t = (int32_t)temp;
            return success;
        }

        case REXX_VALUE_intptr_t:           /* integer value                     */
        {
            intptr_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToIntptr(o, temp);
            value->value.value_intptr_t = (intptr_t)temp;
            return success;
        }

        case REXX_VALUE_int64_t:            /* integer value                     */
        {
            int64_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToInt64(o, temp);
            value->value.value_int64_t = (int64_t)temp;
            return success;
        }

        case REXX_VALUE_uint8_t:            /* 8-bit integer value               */
        {
            size_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToUnsignedInteger(o, temp, UINT8_MAX);
            value->value.value_uint8_t = (uint8_t)temp;
            return success;
        }

        case REXX_VALUE_uint16_t:            /* integer value                     */
        {
            size_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToUnsignedInteger(o, temp, UINT16_MAX);
            value->value.value_uint16_t = (uint16_t)temp;
            return success;
        }

        case REXX_VALUE_uint32_t:            /* integer value                     */
        {
            size_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToUnsignedInteger(o, temp, UINT32_MAX);
            value->value.value_uint32_t = (uint32_t)temp;
            return success;
        }

        case REXX_VALUE_uintptr_t:         /* integer value                     */
        {
            uintptr_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToUintptr(o, temp);
            value->value.value_uintptr_t = (uintptr_t)temp;
            return success;
        }

        case REXX_VALUE_uint64_t:            /* integer value                     */
        {
            uint64_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToUnsignedInt64(o, temp);
            value->value.value_uint64_t = (uint64_t)temp;
            return success;
        }

        case REXX_VALUE_size_t:            /* integer value                     */
        {
            size_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToUnsignedInteger(o, temp, SIZE_MAX);
            value->value.value_size_t = (size_t)temp;
            return success;
        }

        case REXX_VALUE_logical_t:         /* integer value                     */
        {
            // this converts without raising an error
            return o->logicalValue(value->value.value_logical_t);
        }

        // The Rexx whole number one is checked against the human digits limit
        case REXX_VALUE_wholenumber_t:  /* number value                      */
        {
            wholenumber_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToWholeNumber(o, temp, Numerics::MAX_WHOLENUMBER, Numerics::MIN_WHOLENUMBER);
            value->value.value_wholenumber_t = (wholenumber_t)temp;
            return success;
        }

        // The Rexx whole number one is checked against the human digits limit
        case REXX_VALUE_ssize_t:  /* ssize_t value                     */
        {
            ssize_t temp = 0;
            // convert and copy                  */
            // NB:  SSIZE_MIN appears to be defined as 0 for some bizarre reason on some platforms,
            // so we'll make things relative to SIZE_MAX.
            bool success = Numerics::objectToSignedInteger(o, temp, SSIZE_MAX, (-SSIZE_MAX) - 1);
            value->value.value_wholenumber_t = (wholenumber_t)temp;
            return success;
        }

        // an unsigned string number value
        case REXX_VALUE_stringsize_t:
        {
            stringsize_t temp = 0;
            // convert and copy                  */
            bool success = Numerics::objectToStringSize(o, temp, Numerics::MAX_STRINGSIZE);
            value->value.value_stringsize_t = temp;
            return success;
        }

        case REXX_VALUE_double:         /* double value                      */
        {
            return o->doubleValue(value->value.value_double);
        }


        case REXX_VALUE_float:          /* float value                      */
        {
            double temp = 0.0;
            bool success = o->doubleValue(temp);
            value->value.value_float = (float)temp;
            return success;
        }

        case REXX_VALUE_CSTRING:        /* ASCII-Z string value              */
        {
            value->value.value_CSTRING = this->cstring(o);
            return true;
        }

        case REXX_VALUE_RexxStringObject: /* Required STRING object            */
        {
            /* force to a string value           */
            RexxString *temp = stringArgument(o, 1) ;
            // if this forced a string object to be created,
            // we need to protect it here.
            if (temp != o)
            {
                                     /* make it safe                      */
                createLocalReference(temp);
            }
            /* set the result in                 */
            value->value.value_RexxStringObject = (RexxStringObject)temp;
            return true;

        }

        case REXX_VALUE_RexxArrayObject: /* Required ARRAY object            */
        {
            /* force to a string value           */
            RexxArray *temp = arrayArgument(o, 1) ;
            // if this forced a string object to be created,
            // we need to protect it here.
            if (temp != o)
            {
                                     /* make it safe                      */
                createLocalReference(temp);
            }
            /* set the result in                 */
            value->value.value_RexxArrayObject = (RexxArrayObject)temp;
            return true;

        }

        case REXX_VALUE_RexxStemObject: /* Required Stem object            */
        {
            // Stem os get special handling.  If the o
            // object is already a stem object, we're done.  Otherwise,
            // we get the string value of the o and use that
            // to resolve a stem name in the current context.  If the
            // trailing period is not given on the name, one will be added.
            // Note that the second form is only available if this is a
            // call context.  This is an error for a method context.

            // is this a stem already?
            if (isStem(o))
            {
                /* set the result in                 */
                value->value.value_RexxStemObject = (RexxStemObject)o;
                return true;
            }

            // this doesn't make any sense for a function call
            if (activationType == METHOD_ACTIVATION)
            {
                return false;
            }

            /* force to a string value           */
            RexxString *temp = stringArgument(o, 1) ;
            // if this forced a string object to be created,
            // we need to protect it here.
            if (temp != o)
            {
                                     /* make it safe                      */
                createLocalReference(temp);
            }

            // see if we can retrieve this stem
            RexxObject *stem = getContextStem(temp);
            if (stem == OREF_NULL)
            {
                return false;
            }
            /* set the result in                 */
            value->value.value_RexxStemObject = (RexxStemObject)stem;
            return true;
        }

        case REXX_VALUE_POINTER:
        {
            value->value.value_POINTER = this->pointer(o);
            return true;
        }

        case REXX_VALUE_POINTERSTRING:
        {
            /* force to a string value           */
            RexxString *string = o->stringValue();

            void *pointerVal;
            if (sscanf(string->getStringData(), "0x%p", &pointerVal) != 1)
            {
                return false;
            }

            value->value.value_POINTER = pointerVal;
            return true;
        }

        default:                   /* something messed up               */
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
void RexxNativeActivation::createLocalReference(RexxObject *objr)
{
    // if we have a real object, then add to the list
    if (objr != OREF_NULL)
    {
        // make sure we protect this from a GC triggered by this table creation.
        ProtectedObject p1(objr);
        if (this->savelist == OREF_NULL)     /* first saved object?               */
        {
            /* create the save list now          */
            this->savelist = new_list();
        }
        /* add to the save table             */
        this->savelist->append(objr);
    }
}


/**
 * Remove an object from the local reference table.
 *
 * @param objr   The object to remove.
 */
void RexxNativeActivation::removeLocalReference(RexxObject *objr)
{
    // if the reference is non-null
  if (objr != OREF_NULL)
  {
      // make sure we have a savelist before trying to remove this
      if (savelist != OREF_NULL)
      {
          savelist->removeItem(objr);
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
void RexxNativeActivation::run(RexxMethod *_method, RexxNativeMethod *_code, RexxObject  *_receiver,
    RexxString  *_msgname, RexxObject **_arglist, size_t _argcount, ProtectedObject &resultObj)
{
    // anchor the relevant context information
    executable = _method;
    receiver = _receiver;
    msgname = _msgname;
    arglist = _arglist;
    argcount = _argcount;
    activationType = METHOD_ACTIVATION;      // this is for running a method

    ValueDescriptor arguments[MAX_NATIVE_ARGUMENTS];

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
    processArguments(argcount, arglist, types, arguments, MAX_NATIVE_ARGUMENTS);

    size_t activityLevel = this->activity->getActivationLevel();
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
    catch (RexxNativeActivation *)
    {
    }

    // if we're not the current kernel holder when things return, make sure we
    // get the lock before we continue
    if (ActivityManager::currentActivity != activity)
    {
        activity->requestAccess();
    }
    this->guardOff();                  /* release any variable locks        */
    this->argcount = 0;                /* make sure we don't try to mark any arguments */
    // the lock holder gets here by longjmp from a kernel reentry.  We need to
    // make sure the activation count gets reset, else we'll accumulate bogus
    // nesting levels that will make it look like this activity is still in use
    // when in fact we're done with it.
    this->activity->restoreActivationLevel(activityLevel);

    /* give up reference to receiver so that it can be garbage collected */
    this->receiver = OREF_NULL;

    checkConditions();                   // see if we have conditions to raise now

    // set the return value and get outta here
    resultObj = this->result;
    this->argcount = 0;                  /* make sure we don't try to mark any arguments */


    this->activity->popStackFrame(this); /* pop this from the activity        */
    this->setHasNoReferences();          /* mark this as not having references in case we get marked */
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
void RexxNativeActivation::callNativeRoutine(RoutineClass *_routine, RexxNativeRoutine *_code, RexxString *functionName,
    RexxObject **list, size_t count, ProtectedObject &resultObj)
{
    // anchor the context stuff
    executable = _routine;
    msgname = functionName;
    arglist = list;
    argcount = count;
    activationType = FUNCTION_ACTIVATION;      // this is for running a method
    accessCallerContext();                   // we need this to access the caller's context

    ValueDescriptor arguments[MAX_NATIVE_ARGUMENTS];

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
    processArguments(argcount, arglist, types, arguments, MAX_NATIVE_ARGUMENTS);

    size_t activityLevel = this->activity->getActivationLevel();
    trapErrors = true;                       // we trap error conditions now
    try
    {
        enableVariablepool();                // enable the variable pool interface here
        activity->releaseAccess();           /* force this to "safe" mode         */
                                             /* process the method call           */
        (*methp)((RexxCallContext *)&context, arguments);
        activity->requestAccess();           /* now in unsafe mode again          */

        // process the returned result
        result = valueToObject(arguments);
    }
    catch (RexxNativeActivation *)
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
    this->activity->restoreActivationLevel(activityLevel);

    /* give up reference to receiver so that it can be garbage collected */
    this->receiver = OREF_NULL;

    checkConditions();                   // see if we have conditions to raise now

    // set the return value and get outta here.
    resultObj = this->result;
    this->argcount = 0;                  /* make sure we don't try to mark any arguments */

    this->activity->popStackFrame(this); /* pop this from the activity        */
    this->setHasNoReferences();          /* mark this as not having references in case we get marked */
}


/**
 * Process a native function call.
 *
 * @param entryPoint The target function entry point.
 * @param list       The list of arguments.
 * @param count      The number of arguments.
 * @param result     A protected object to receive the function result.
 */
void RexxNativeActivation::callRegisteredRoutine(RoutineClass *_routine, RegisteredRoutine *_code, RexxString *functionName,
    RexxObject **list, size_t count, ProtectedObject &resultObj)
{
    // anchor the context stuff
    msgname = functionName;
    executable = _routine;
    arglist = list;
    argcount = count;
    accessCallerContext();                   // we need this to access the caller's context

    activationType = FUNCTION_ACTIVATION;      // this is for running a method
    // use the default security manager
    securityManager = activity->getInstanceSecurityManager();


    // get the entry point address of the target method
    RexxRoutineHandler *methp = _code->getEntry();

    CONSTRXSTRING   arguments[MAX_NATIVE_ARGUMENTS];
    CONSTRXSTRING *argPtr = arguments;

    // unlike the other variants, we don't have a cap on arguments.  If we have more than the preallocated
    // set, then dynamically allocate a buffer object to hold the memory and anchor it in the
    // activation saved objects.
    if (count > MAX_NATIVE_ARGUMENTS)
    {
        RexxBuffer *argBuffer = new_buffer(sizeof(CONSTRXSTRING) * count);
        // this keeps the buffer alive until the activation is popped.
        createLocalReference(argBuffer);
        argPtr = (CONSTRXSTRING *)argBuffer->getData();
    }

    // all of the arguments now need to be converted to string arguments
    for (size_t argindex=0; argindex < count; argindex++)
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
        else                             /* have an omitted argument          */
        {
            /* give it zero length               */
            argPtr[argindex].strlength = 0;
            /* and a zero pointer                */
            argPtr[argindex].strptr = NULL;
        }
    }
    /* get the current queue name        */
    const char *queuename = Interpreter::getCurrentQueue()->getStringData();
    RXSTRING funcresult;
    int functionrc;                      /* Return code from function         */
    /* default return code buffer        */
    char default_return_buffer[DEFRXSTRING];

    /* make the RXSTRING result          */
    MAKERXSTRING(funcresult, default_return_buffer, sizeof(default_return_buffer));

    size_t activityLevel = this->activity->getActivationLevel();

    trapErrors = true;                       // trap errors from here
    try
    {
        enableVariablepool();                // enable the variable pool interface here
        activity->releaseAccess();           /* force this to "safe" mode         */
        // now process the function call
        functionrc = (*methp)(functionName->getStringData(), count, argPtr, queuename, &funcresult);
        activity->requestAccess();           /* now in unsafe mode again          */
    }
    catch (RexxActivation *)
    {
        // if we're not the current kernel holder when things return, make sure we
        // get the lock before we continue
        if (ActivityManager::currentActivity != activity)
        {
            activity->requestAccess();
        }

        this->argcount = 0;                /* make sure we don't try to mark any arguments */
        // the lock holder gets here by longjmp from a kernel reentry.  We need to
        // make sure the activation count gets reset, else we'll accumulate bogus
        // nesting levels that will make it look like this activity is still in use
        // when in fact we're done with it.
        this->activity->restoreActivationLevel(activityLevel);
        // IMPORTANT NOTE:  We don't pop our activation off the stack.  This will be
        // handled by the catcher.  Attempting to pop the stack when an error or condition has
        // occurred can munge the activation stack, resulting bad stuff.
        this->setHasNoReferences();        /* mark this as not having references in case we get marked */
        // now rethrow the trapped condition so that real target can handle this.
        throw;
    }
    catch (RexxNativeActivation *)
    {
        // if we're not the current kernel holder when things return, make sure we
        // get the lock before we continue
        if (ActivityManager::currentActivity != activity)
        {
            activity->requestAccess();
        }
        this->argcount = 0;                /* make sure we don't try to mark any arguments */
        // the lock holder gets here by longjmp from a kernel reentry.  We need to
        // make sure the activation count gets reset, else we'll accumulate bogus
        // nesting levels that will make it look like this activity is still in use
        // when in fact we're done with it.
        this->activity->restoreActivationLevel(activityLevel);
        this->activity->popStackFrame(this);   /* pop this from the activity        */
        this->setHasNoReferences();        /* mark this as not having references in case we get marked */
        // set the return value and get outta here
        resultObj = this->result;
        return;
    }

    trapErrors = false;                   // no more error trapping
    disableVariablepool();                // disable the variable pool from here

    // belt and braces...this restores the activity level to whatever
    // level we had when we made the callout.
    this->activity->restoreActivationLevel(activityLevel);

    // now process the function return value
    if (functionrc == 0)             /* If good rc from function          */
    {
        if (funcresult.strptr != NULL)         /* If we have a result, return it    */
        {
            /* make a string result              */
            resultObj = new_string(funcresult);
            /* user give us a new buffer?        */
            if (funcresult.strptr != default_return_buffer )
            {
                /* free it                           */
                SystemInterpreter::releaseResultMemory(funcresult.strptr);
            }
        }
    }
    else                             /* Bad rc from function, signal      */
    {
        /* error                             */
        reportException(Error_Incorrect_call_external, functionName);
    }

    this->argcount = 0;                  /* make sure we don't try to mark any arguments */
    this->activity->popStackFrame(this); /* pop this from the activity        */
    this->setHasNoReferences();          /* mark this as not having references in case we get marked */
}


/**
 * Run a task under the scope of a native activation.  This is
 * generally a bootstrapping call, such as a top-level program call,
 * method translation, etc.
 *
 * @param dispatcher The dispatcher instance we're going to run.
 */
void RexxNativeActivation::run(ActivityDispatcher &dispatcher)
{
    activationType = DISPATCHER_ACTIVATION;  // this is for running a dispatcher
    size_t activityLevel = this->activity->getActivationLevel();
    // use the default security manager
    securityManager = activity->getInstanceSecurityManager();

    // make the activation hookup
    dispatcher.setContext(activity, this);
    trapErrors = true;                       // we trap errors from here
    try
    {
        // we run this under a callback trap so that the exceptions get processed.
        dispatcher.run();
        // make sure we clear any outstanding trapped conditions for this
        // activity before we clear.  If an untrapped condition occurrect, this
        // will return to one of the catch positions
        activity->clearCurrentCondition();
    }
    catch (ActivityException)
    {
    }
    catch (RexxNativeActivation *)
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
    this->activity->restoreActivationLevel(activityLevel);
    if (conditionObj != OREF_NULL)
    {
        // pass the condition information on to the dispatch unig
        dispatcher.handleError(conditionObj);
    }

    this->activity->popStackFrame(this); /* pop this from the activity        */
    this->setHasNoReferences();          /* mark this as not having references in case we get marked */
    return;                              /* and finished                      */
}


/**
 * Run a callback under the scope of a native actvation. This is
 * generally a call out, such as a system exit, argument
 * callback, etc.
 *
 * @param dispatcher The dispatcher instance we're going to run.
 */
void RexxNativeActivation::run(CallbackDispatcher &dispatcher)
{
    activationType = CALLBACK_ACTIVATION;    // we're handling a callback
    // use the default security manager
    securityManager = activity->getInstanceSecurityManager();
    size_t activityLevel = this->activity->getActivationLevel();
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
    catch (RexxNativeActivation *)
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
    this->activity->restoreActivationLevel(activityLevel);
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
void RexxNativeActivation::accessCallerContext()
{
    activation = (RexxActivation *)getPreviousStackFrame();
}


/**
 * Check to see if there are deferred syntax errors that need
 * to be raised on return from a native activation.
 */
void RexxNativeActivation::checkConditions()
{
    trapErrors = false;                // no more error trapping

    // if we have a stashed condition object, we need to raise this now
    if (conditionObj != OREF_NULL)
    {
        // we're raising this in the previous stack frame.  If we're actually the
        // base of the stack, there's nothing left to handle this.
        if (!isStackBase())
        {
            /* get the original condition name   */
            RexxString *condition = (RexxString *)conditionObj->at(OREF_CONDITION);

            /* fatal SYNTAX error?               */
            if (condition->strCompare(CHAR_SYNTAX))
            {
                // this prevents us from trying to trap this again
                trapErrors = false;
                                                 /* go propagate the condition        */
                activity->reraiseException(conditionObj);
            }
            else
            {                               /* normal condition trapping         */
                                            /* get the sender object (if any)    */
                // find a predecessor Rexx activation
                RexxActivationBase *_sender = this->getPreviousStackFrame();
                /* do we have a sender that is       */
                /* trapping this condition?          */
                /* do we have a sender?              */

                if (_sender != OREF_NULL)
                {
                    /* "tickle them" with this           */
                    _sender->trap(condition, conditionObj);
                }
                // if the trap is not handled, then we return directly.  The return
                // value (if any) is stored in the condition object
                result = conditionObj->at(OREF_RESULT);
            }
        }
    }
}


RexxVariableDictionary *RexxNativeActivation::methodVariables()
/******************************************************************************/
/* Function:  Retrieve the set of method variables                            */
/******************************************************************************/
{
    /* first retrieval?                  */
    if (this->objectVariables == OREF_NULL)
    {
        // not a method invocation?
        if (receiver == OREF_NULL)
        {
            /* retrieve the method variables     */
            this->objectVariables = ((RexxActivation *)this->receiver)->getLocalVariables();
        }
        else
        {
            RexxMethod *method = (RexxMethod *)executable;
            /* must be wanting the ovd set of    */
            /*variables                          */
            this->objectVariables = this->receiver->getObjectVariables(method->getScope());
            /* guarded method?                   */
            if (this->object_scope == SCOPE_RELEASED && method->isGuarded())
            {
                /* reserve the variable scope        */
                this->objectVariables->reserve(this->activity);
                /* and remember for later            */
                this->object_scope = SCOPE_RESERVED;
            }
        }
    }
    return this->objectVariables;        /* return the dictionary             */
}


bool RexxNativeActivation::isInteger(
    RexxObject *object)                /* object to validate                */
/******************************************************************************/
/* Function:  Validate that an object has an integer value                    */
/******************************************************************************/
{
    wholenumber_t temp;
    return object->numberValue(temp, this->digits());
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
wholenumber_t RexxNativeActivation::signedIntegerValue(RexxObject *o, size_t position, wholenumber_t maxValue, wholenumber_t minValue)
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
 * Convert a value to a size_t value.
 *
 * @param o        The object to convert.
 * @param position The argument position.
 * @param maxValue The maximum value allowed in the range.
 *
 * @return The converted number.
 */
size_t RexxNativeActivation::unsignedIntegerValue(RexxObject *o, size_t position, stringsize_t maxValue)
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
int64_t RexxNativeActivation::int64Value(RexxObject *o, size_t position)
{
    int64_t temp;

    // convert using the whole value range
    if (!Numerics::objectToInt64(o, temp))
    {
        reportException(Error_Invalid_argument_range, new_array(new_integer(position + 1), Numerics::int64ToObject(INT64_MAX), Numerics::int64ToObject(INT64_MIN), o));
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
uint64_t RexxNativeActivation::unsignedInt64Value(RexxObject *o, size_t position)
{
    uint64_t temp;

    // convert using the whole value range
    if (!Numerics::objectToUnsignedInt64(o, temp))
    {
        reportException(Error_Invalid_argument_range, new_array(new_integer(position + 1), IntegerZero, Numerics::int64ToObject(INT64_MAX), o));
    }
    return temp;
}


const char *RexxNativeActivation::cstring(
    RexxObject *object)                /* object to convert                 */
/******************************************************************************/
/* Function:  Return an object as a CSTRING                                   */
/******************************************************************************/
{
    /* force to a string value           */
    RexxString *string = (RexxString *)object->stringValue();
    if (string != object)                /* different value?                  */
    {
        /* make it safe                      */
        createLocalReference(string);
    }
    return string->getStringData();           /* just point to the string data     */
}


/**
 * Convert a string in the format 0xnnnnnnnnn into a pointer value.
 *
 * @param object The object to convert.
 *
 * @return The pointer value.
 */
void *RexxNativeActivation::pointerString(RexxObject *object, size_t position)
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


double RexxNativeActivation::getDoubleValue(RexxObject *object, size_t position)
/******************************************************************************/
/* Function:  Convert an object to a double                                   */
/******************************************************************************/
{
    double r;                            /* returned result                   */
                                         /* convert and check result          */
    if (!object->doubleValue(r))
    {
        /* conversion error                  */
        reportException(Error_Invalid_argument_double, position + 1, object);
    }
    return r;                            /* return converted number           */
}


bool RexxNativeActivation::isDouble(
    RexxObject *object)                /* object to check                   */
/******************************************************************************/
/* Function:  Test to see if an object is a valid double                      */
/******************************************************************************/
{
    double r;                            /* returned result                   */
                                         /* convert and check result          */
    return object->doubleValue(r);
}

void *RexxNativeActivation::cself()
/******************************************************************************/
/* Function:  Returns "unwrapped" C or C++ object associated with this        */
/*            object instance.  If the variable CSELF does not exist, then    */
/*            NULL is returned.                                               */
/******************************************************************************/
{
    // if this is a method invocation, ask the receiver object to figure this out.
    if (receiver != OREF_NULL)
    {
        // this is necessary to get turn on a guard lock if the method
        // is guarded.  Failure to do this can cause multithreading problems.
        methodVariables();
        return receiver->getCSelf();
    }
    // nope, call context doesn't allow this
    return OREF_NULL;
}


void *RexxNativeActivation::pointer(
    RexxObject *object)                /* object to convert                 */
/******************************************************************************/
/* Function:  Return as a pointer the value of an integer                     */
/******************************************************************************/
{
    if (!object->isInstanceOf(ThePointerClass))
    {
        return NULL;
    }
    // just unwrap thee pointer
    return ((RexxPointer *)object)->pointer();
}


RexxObject *RexxNativeActivation::dispatch()
/******************************************************************************/
/* Function:  Redispatch an activation on a different activity                */
/******************************************************************************/
{
    ProtectedObject r;
    this->run((RexxMethod *)executable, (RexxNativeMethod *)code, receiver, msgname, arglist, argcount, r);  /* just do a method run              */
    return (RexxObject *)r;
}


void RexxNativeActivation::traceBack(
    RexxList *traceback_list)          /* list of traceback items           */
/******************************************************************************/
/* Function:  Add a trace back item to the list.  For native activations,     */
/*            this is a no-op.                                                */
/******************************************************************************/
{
    return;                              /* just return                       */
}

size_t RexxNativeActivation::digits()
/******************************************************************************/
/* Function:  Return the current digits setting                               */
/******************************************************************************/
{
    /* have a real one?                  */
    if (activation == OREF_NULL)
    {
        return Numerics::DEFAULT_DIGITS;   /*  no, just return default value    */
    }
    else
    {
        return activation->digits();       /* pass on the the sender            */
    }
}

size_t RexxNativeActivation::fuzz()
/******************************************************************************/
/* Function:  Return the current fuzz setting                                 */
/******************************************************************************/
{
    /* have a real one?                  */
    if (activation == OREF_NULL)
    {
        return Numerics::DEFAULT_FUZZ;     /*  no, just return default value    */
    }
    else
    {
        return activation->fuzz();         /* pass on the the sender            */
    }
}

bool RexxNativeActivation::form()
/******************************************************************************/
/* Function:  Return the curren form setting                                  */
/******************************************************************************/
{
    /* have a real one?                  */
    if (activation == OREF_NULL)
    {
        return Numerics::DEFAULT_FORM;     /*  no, just return default value    */
    }
    else
    {
        return activation->form();         /* pass on the the sender            */
    }
}

void RexxNativeActivation::setDigits(
    size_t _digits)                     /* new NUMERIC DIGITS value          */
/******************************************************************************/
/* Function:  Set a new numeric digits setting                                */
/******************************************************************************/
{
    /* have a real one?                  */
    if (activation == OREF_NULL)
    {
        activation->setDigits(_digits);      /* just forward the set              */
    }
}

void RexxNativeActivation::setFuzz(
    size_t _fuzz )                     /* new NUMERIC FUZZ value            */
/******************************************************************************/
/* Function:  Set a new numeric fuzz setting                                  */
/******************************************************************************/
{
    /* have a real one?                  */
    if (activation != OREF_NULL)
    {
        activation->setFuzz(_fuzz);         /* just forward the set              */
    }
}

void RexxNativeActivation::setForm(
    bool _form )                        /* new NUMERIC FORM value            */
/******************************************************************************/
/* Function:  Set a new numeric form setting                                  */
/******************************************************************************/
{
    /* have a real one?                  */
    if (activation == OREF_NULL)
    {
        activation->setForm(_form);        /* just forward the set              */
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
NumericSettings *RexxNativeActivation::getNumericSettings()
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
bool RexxNativeActivation::isStackBase()
{
    return stackBase;
}


/**
 * Return the Rexx context this operates under.  Depending on the
 * context, this could be null.
 *
 * @return The parent Rexx context.
 */
RexxActivation *RexxNativeActivation::getRexxContext()
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
BaseExecutable *RexxNativeActivation::getRexxContextExecutable()
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
RexxObject *RexxNativeActivation::getRexxContextObject()
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
RexxActivation *RexxNativeActivation::findRexxContext()
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
RexxObject *RexxNativeActivation::getReceiver()
{
    return receiver;
}


/**
 * Get the security manager context
 *
 * @return The security manager, if there is one.
 */
SecurityManager *RexxNativeActivation::getSecurityManager()
{
    RexxSource *s = getSourceObject();
    if (s != OREF_NULL)
    {
        return s->getSecurityManager();
    }
    return OREF_NULL;     // no security manager on this context.
}


void RexxNativeActivation::guardOff()
/******************************************************************************/
/* Function:  Release a variable pool guard lock                              */
/******************************************************************************/
{
    /* currently locked?                 */
    if (this->object_scope == SCOPE_RESERVED)
    {
        /* release the variable dictionary   */
        this->objectVariables->release(this->activity);
        /* set the state to released         */
        this->object_scope = SCOPE_RELEASED;
    }
}

void RexxNativeActivation::guardOn()
/******************************************************************************/
/* Function:  Acquire a variable pool guard lock                              */
/******************************************************************************/
{
    // if there's no receiver object, then this is not a true method call.
    // there's nothing to lock
    if (receiver == OREF_NULL)
    {
        return;
    }
    /* first retrieval? */
    if (this->objectVariables == OREF_NULL)
    {
        /* grab the object variables associated with this object */
        this->objectVariables = this->receiver->getObjectVariables(((RexxMethod *)executable)->getScope());
    }
    /* not currently holding the lock? */
    if (this->object_scope == SCOPE_RELEASED)
    {
        /* reserve the variable scope */
        this->objectVariables->reserve(this->activity);
        /* and remember for later */
        this->object_scope = SCOPE_RESERVED;
    }
}

void RexxNativeActivation::enableVariablepool()
/******************************************************************************/
/* Function:  Enable the variable pool                                        */
/******************************************************************************/
{
  this->resetNext();                   /* reset fetch next calls            */
  this->vpavailable = true;            /* allow the calls                   */
}

void RexxNativeActivation::disableVariablepool()
/******************************************************************************/
/* Function:  Disable the variable pool                                       */
/******************************************************************************/
{
  this->resetNext();                   /* reset fetch next calls            */
  this->vpavailable = false;           /* no more external calls            */
}

void RexxNativeActivation::resetNext()
/******************************************************************************/
/* Function: Reset the next state of the variable pool                        */
/******************************************************************************/
{
  this->nextvariable = SIZE_MAX;       /* turn off next index               */
  this->nextcurrent = OREF_NULL;       /* clear the next value              */
  this->nextstem = OREF_NULL;          /* clear the secondary pointer       */
  this->compoundelement = OREF_NULL;
}


bool RexxNativeActivation::fetchNext(
    RexxString **name,                 /* the returned name                 */
    RexxObject **value)                /* the return value                  */
/******************************************************************************/
/* Function:  Fetch the next variable of a variable pool traversal            */
/******************************************************************************/
{
    RexxVariable *variable;              /* retrieved variable value          */
    RexxCompoundElement *compound;       /* retrieved variable value          */
    RexxStem     *stemVar;               /* a potential stem variable collection */

                                         /* starting off fresh?               */
    if (nextCurrent() == OREF_NULL)
    {
        /* grab the activation context */
        RexxActivation *act = activity->getCurrentRexxFrame();
        setNextVariable(SIZE_MAX);         /* request the first item            */
        /* Get the local variable dictionary from the context. */
        setNextCurrent(act->getLocalVariables());
        /* we are not on a stem              */
        setNextStem(OREF_NULL);
        setCompoundElement(OREF_NULL);
    }

    for (;;)                             /* loop until we get something       */
    {
        stemVar = nextStem();              /* see if we're processing a stem variable */
        if (stemVar != OREF_NULL)          /* were we in the middle of processing a stem? */
        {
            compound = stemVar->nextVariable(this);
            if (compound != OREF_NULL)     /* if we still have elements here */
            {
                /* create a full stem name           */
                *name = compound->createCompoundName(stemVar->getName());
                /* get the value                     */
                *value = compound->getVariableValue();
                return true;
            }
            else                           /* we've reached the end of the stem, reset */
            {
                /* to the main dictionary and continue */
                setNextStem(OREF_NULL);
                setCompoundElement(OREF_NULL);
            }
        }
        /* get the next variable             */
        variable = nextCurrent()->nextVariable(this);
        if (variable == OREF_NULL)         /* reached the end of the table      */
        {
            return false;
        }
        else                               /* have a real variable              */
        {
            /* get the value                     */
            RexxObject *variable_value = variable->getVariableValue();
            /* found a stem item?                */
            if (isOfClass(Stem, variable_value))
            {
                /* we are not on a stem              */
                setNextStem((RexxStem *)variable_value);
                setCompoundElement(((RexxStem *)variable_value)->first());
                /* set up an iterator for the stem   */
            }
            else                             /* found a real variable             */
            {
                *value = variable_value;       /* pass back the value (name already set) */
                *name = variable->getName();
                return true;                   /* we have another variable to return */
            }
        }
    }
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
bool RexxNativeActivation::trap(RexxString *condition, RexxDirectory * exception_object)
{
    // There are two possibilities here.  We're either seeing this because of a
    // propagating syntax condition.  for this case, we trap this and hold it.
    // The other possibility is a condition being raised by an API callback.  That should
    // be the only situation where we see any other condition type.  We also trap that
    // one so it can be raised in the caller's context.

    // we end up seeing this a second time if we're raising the exception on
    // return from an external call or method.
    if (condition->isEqual(OREF_SYNTAX))
    {
        if (trapErrors)
        {
            // record this in case any callers want to know about it.
            setConditionInfo(exception_object);
            // this will unwind back to the calling level, with the
            // exception information recorded.
            throw this;
        }

    }
    else if (trapConditions)
    {
        // pretty much the same deal, but we're only handling conditions, and
        // only one condtion, so reset the trap flag
        trapConditions = false;
        // record this in case any callers want to know about it.
        setConditionInfo(exception_object);
        // this will unwind back to the calling level, with the
        // exception information recorded.
        throw this;
    }
    return false;                        /* this wasn't handled               */
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
void RexxNativeActivation::raiseCondition(RexxString *condition, RexxString *description, RexxObject *additional, RexxObject *_result)
{
    this->result = (RexxObject *)_result; /* save the result                   */
                                         /* go raise the condition            */
    this->activity->raiseCondition(condition, OREF_NULL, description, additional, result);

    // We only return here if no activation above us has trapped this.  If we do return, then
    // we terminate the call by throw this up the stack.
    throw this;
}


/**
 * Return the method context arguments as an array.
 *
 * @return An array containing the method arguments.
 */
RexxArray *RexxNativeActivation::getArguments()
{
    // if we've not requested this before, convert the arguments to an
    // array object.
    if (argArray == OREF_NULL)
    {
        /* create the argument array */
        argArray = new (argcount, arglist) RexxArray;
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
RexxObject *RexxNativeActivation::getArgument(size_t index)
{
    if (index <= argcount)
    {
        return arglist[index - 1];
    }
    return OREF_NULL;
}

/**
 * Return the super class scope of the current method context.
 *
 * @return The superclass scope object.
 */
RexxObject *RexxNativeActivation::getSuper()
{
    return receiver->superScope(((RexxMethod *)executable)->getScope());
}

/**
 * Return the current method scope.
 *
 * @return The current method scope object.
 */
RexxObject *RexxNativeActivation::getScope()
{
    return ((RexxMethod *)executable)->getScope();
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
RexxStem *RexxNativeActivation::resolveStemVariable(RexxObject *s)
{
    // is this a stem already?
    if (isStem(s))
    {
        return (RexxStem *)s;
    }

    /* force to a string value           */
    RexxString *temp = stringArgument(s, 1);
    // see if we can retrieve this stem
    return (RexxStem *)getContextStem(temp);
}


RexxObject *RexxNativeActivation::getContextStem(RexxString *name)
/******************************************************************************/
/* Function:  retrieve a stem variable stem from the current context.         */
/******************************************************************************/
{
    // if this is not a stem name, add it now
    if (name->getChar(name->getLength() - 1) != '.')
    {
        name = name->concatWithCstring(".");
    }

    RexxVariableBase *retriever = RexxVariableDictionary::getVariableRetriever(name);
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
RexxObject *RexxNativeActivation::getContextVariable(const char *name)
{
    RexxVariableBase *retriever = RexxVariableDictionary::getVariableRetriever(new_string(name));
    // if this didn't parse, it's an illegal name
    if (retriever == OREF_NULL)
    {
        return OREF_NULL;
    }
    this->resetNext();               // all next operations must be reset

    // have a non-name retriever?
    if (isString((RexxObject *)retriever))
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
 * Set a context variable on behalf of an API call.
 *
 * @param name   The name of the variable.
 * @param value  The variable value.
 */
void RexxNativeActivation::setContextVariable(const char *name, RexxObject *value)
{
    // get the REXX activation for the target context
    RexxVariableBase *retriever = RexxVariableDictionary::getVariableRetriever(new_string(name));
    // if this didn't parse, it's an illegal name
    if (retriever == OREF_NULL || isString((RexxObject *)retriever))
    {
        return;
    }
    this->resetNext();               // all next operations must be reset

    // do the assignment
    retriever->set(activation, value);
}


/**
 * Drop a context variable for an API call.
 *
 * @param name   The target variable name.
 */
void RexxNativeActivation::dropContextVariable(const char *name)
{
    // get the REXX activation for the target context
    RexxVariableBase *retriever = RexxVariableDictionary::getVariableRetriever(new_string(name));
    // if this didn't parse, it's an illegal name
    if (retriever == OREF_NULL || isString((RexxObject *)retriever))
    {
        return;
    }
    this->resetNext();               // all next operations must be reset

    // perform the drop
    retriever->drop(activation);
}


RexxDirectory *RexxNativeActivation::getAllContextVariables()
/******************************************************************************/
/* Function:  Retriev a list of all variables in the current context.         */
/******************************************************************************/
{
    this->resetNext();               // all next operations must be reset
    return activation->getAllLocalVariables();
}


/**
 * Get nn object variable in the current method scope.  Returns
 * a NULL object reference if the variable does not exist.
 *
 * @param name   The variable name.
 *
 * @return The variable value or OREF_NULL if the variable does not
 *         exist.
 */
RexxObject *RexxNativeActivation::getObjectVariable(const char *name)
{
    // get the REXX activation for the target context
    RexxVariableBase *retriever = RexxVariableDictionary::getVariableRetriever(new_string(name));
    // if this didn't parse, it's an illegal name
    // we also don't allow compound variables here because the source for
    // resolving the tail pieces is not defined.
    if (retriever == OREF_NULL || isString((RexxObject *)retriever) || isOfClassType(CompoundVariableTerm, retriever))
    {
        return OREF_NULL;
    }
    // retrieve the value
    return retriever->getRealValue(methodVariables());
}

/**
 * The an object variable to a new value.
 *
 * @param name   The name of the variable.
 * @param value  The new variable value.
 */
void RexxNativeActivation::setObjectVariable(const char *name, RexxObject *value)
{
    // get the REXX activation for the target context
    RexxVariableBase *retriever = RexxVariableDictionary::getVariableRetriever(new_string(name));
    // if this didn't parse, it's an illegal name
    // we also don't allow compound variables here because the source for
    // resolving the tail pieces is not defined.
    if (retriever == OREF_NULL || isString((RexxObject *)retriever) || isOfClassType(CompoundVariableTerm, retriever))
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
void RexxNativeActivation::dropObjectVariable(const char *name)
{
    // get the REXX activation for the target context
    RexxVariableBase *retriever = RexxVariableDictionary::getVariableRetriever(new_string(name));
    // if this didn't parse, it's an illegal name
    // we also don't allow compound variables here because the source for
    // resolving the tail pieces is not defined.
    if (retriever == OREF_NULL || isString((RexxObject *)retriever) || isOfClassType(CompoundVariableTerm, retriever))
    {
        return;
    }
    // do the assignment
    retriever->drop(methodVariables());
}


/**
 * Resolve a class in the context of the current execution context.
 *
 * @param className The target class name.
 *
 * @return The resolved class (if any).
 */
RexxClass *RexxNativeActivation::findClass(RexxString *className)
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
RexxClass *RexxNativeActivation::findCallerClass(RexxString *className)
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
RexxSource *RexxNativeActivation::getSourceObject()
{
    if (executable != OREF_NULL)
    {
        return executable->getSourceObject();
    }
    return OREF_NULL;
}


/**
 * Allocate a new native Activation.
 *
 * @param size   the allocation size.
 *
 * @return A pointer to the newly allocated object.
 */
void * RexxNativeActivation::operator new(size_t size)
{
                                       /* Get new object                    */
  RexxObject *newObject = new_object(size, T_NativeActivation);
  newObject->clearObject();            /* clear out at start                */
  return newObject;                    /* return the new object             */
}


/**
 * Handle a request chain for the variable pool interface API.
 *
 * @param pshvblock The shared variable block for the request.
 *
 * @return The composit return code for the chain of requests.
 */
RexxReturnCode RexxNativeActivation::variablePoolInterface(PSHVBLOCK pshvblock)
{
    // this is not allowed asynchronously
    if (!getVpavailable())
    {
        return RXSHV_NOAVL;
    }

    RexxReturnCode retcode = 0;          /* initialize composite rc           */

    try
    {
        while (pshvblock)
        {                   /* while more request blocks         */
            variablePoolRequest(pshvblock);
            retcode |= pshvblock->shvret;        /* accumulate the return code        */
            pshvblock = pshvblock->shvnext;      /* step to the next block            */
        }
        return retcode;                       /* return composite return code      */

    }
    // intercept any termination failures
    catch (ActivityException)
    {
        /* set failure in current            */
        pshvblock->shvret |= (uint8_t)RXSHV_MEMFL;
        retcode |= pshvblock->shvret;       /* OR with the composite             */
        return retcode;                     /* stop processing requests now      */
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
RexxVariableBase *RexxNativeActivation::variablePoolGetVariable(PSHVBLOCK pshvblock, bool symbolic)
{
    /* no name given?                    */
    if (pshvblock->shvname.strptr==NULL)
    {
        pshvblock->shvret|=RXSHV_BADN;   /* this is bad                       */
    }
    else
    {
        /* get the variable as a string      */
        RexxString *variable = new_string(pshvblock->shvname);
        RexxVariableBase *retriever = OREF_NULL;
        /* symbolic access?                  */
        if (symbolic)
        {
            /* get a symbolic retriever          */
            retriever = RexxVariableDictionary::getVariableRetriever(variable);
        }
        else                             /* need a direct retriever           */
        {
            retriever = RexxVariableDictionary::getDirectVariableRetriever(variable);
        }
        if (retriever == OREF_NULL)      /* have a bad name?                  */
        {
            pshvblock->shvret|=RXSHV_BADN; /* this is bad                       */
        }
        else
        {
            resetNext();             /* reset any next operations         */
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
void RexxNativeActivation::variablePoolFetchVariable(PSHVBLOCK pshvblock)
{
    RexxVariableBase *retriever = variablePoolGetVariable(pshvblock, pshvblock->shvcode == RXSHV_SYFET);
    RexxObject *value = OREF_NULL;
    if (retriever != OREF_NULL)
    {
        /* have a non-name retriever?        */
        if (isString((RexxObject *)retriever))
        {
            /* the value is the retriever        */
            value = (RexxObject *)retriever;
        }
        else
        {
                                           /* have a non-name retriever         */
                                           /* and a new variable?               */
            if (!retriever->exists(activation))
            {
                /* flag this in the block            */
                pshvblock->shvret |= RXSHV_NEWV;
            }
            /* get the variable value            */
            value = retriever->getValue(activation);
        }

        /* copy the value                    */
        pshvblock->shvret |= copyValue(value, &pshvblock->shvvalue, (size_t *)&pshvblock->shvvaluelen);
    }
    else
    {
        /* this is bad                       */
        pshvblock->shvret = RXSHV_BADN;
    }
}


/**
 * Perform a variable pool set operation.
 *
 * @param pshvblock The operation shared variable block.
 */
void RexxNativeActivation::variablePoolSetVariable(PSHVBLOCK pshvblock)
{
    RexxVariableBase *retriever = variablePoolGetVariable(pshvblock, pshvblock->shvcode == RXSHV_SYSET);
    if (retriever != OREF_NULL)
    {
        /* have a non-name retriever?        */
        if (isString((RexxObject *)retriever))
        {
            /* this is bad                       */
            pshvblock->shvret = RXSHV_BADN;
        }
        else
        {
                                           /* have a non-name retriever         */
                                           /* and a new variable?               */
            if (!retriever->exists(activation))
            {
                /* flag this in the block            */
                pshvblock->shvret |= RXSHV_NEWV;
            }
            /* do the assignment                 */
            retriever->set(activation, new_string(pshvblock->shvvalue));
        }
    }
}


/**
 * Perform a variable pool drop operation.
 *
 * @param pshvblock The operation shared variable block.
 */
void RexxNativeActivation::variablePoolDropVariable(PSHVBLOCK pshvblock)
{
    RexxVariableBase *retriever = variablePoolGetVariable(pshvblock, pshvblock->shvcode == RXSHV_SYDRO);
    if (retriever != OREF_NULL)
    {
        /* have a non-name retriever?        */
        if (isString((RexxObject *)retriever))
        {
            /* this is bad                       */
            pshvblock->shvret = RXSHV_BADN;
        }
        else
        {
                                           /* have a non-name retriever         */
                                           /* and a new variable?               */
            if (!retriever->exists(activation))
            {
                /* flag this in the block            */
                pshvblock->shvret |= RXSHV_NEWV;
            }
            /* perform the drop                  */
            retriever->drop(activation);
        }
    }
}


/**
 * Perform a variable pool fetch next operation.
 *
 * @param pshvblock The operation shared variable block.
 */
void RexxNativeActivation::variablePoolNextVariable(PSHVBLOCK pshvblock)
{
    RexxString *name;
    RexxObject *value;
    /* get the next variable             */
    if (!this->fetchNext(&name, &value))
    {
        pshvblock->shvret |= RXSHV_LVAR; /* flag as such                      */
    }
    else
    {                             /* need to copy the name and value   */
                                  /* copy the name                     */
        pshvblock->shvret |= copyValue(name, &pshvblock->shvname, &pshvblock->shvnamelen);
        /* copy the value                    */
        pshvblock->shvret |= copyValue(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
    }
}


/**
 * Perform a variable pool fetch private operation.
 *
 * @param pshvblock The operation shared variable block.
 */
void RexxNativeActivation::variablePoolFetchPrivate(PSHVBLOCK pshvblock)
{
    /* and VP is enabled                 */
    /* private block should always be enabled */
    /* no name given?                    */
    if (pshvblock->shvname.strptr==NULL)
    {
        pshvblock->shvret|=RXSHV_BADN;   /* this is bad                       */
    }
    else
    {
        /* get the variable as a string      */
        const char *variable = pshvblock->shvname.strptr;
        /* want the version string?          */
        if (strcmp(variable, "VERSION") == 0)
        {
            /* copy the value                    */
            pshvblock->shvret |= copyValue(Interpreter::getVersionNumber(), &pshvblock->shvvalue, &pshvblock->shvvaluelen);
        }
        /* want the the current queue?       */
        else if (strcmp(variable, "QUENAME") == 0)
        {
            /* copy the value                    */
            pshvblock->shvret |= copyValue(Interpreter::getCurrentQueue(), &pshvblock->shvvalue, &pshvblock->shvvaluelen);
        }
        /* want the version string?          */
        else if (strcmp(variable, "SOURCE") == 0)
        {
            /* retrieve the source string        */
            RexxString *value = activation->sourceString();
            /* copy the value                    */
            pshvblock->shvret |= copyValue(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
        }
        /* want the parameter count?         */
        else if (strcmp(variable, "PARM") == 0)
        {
            RexxInteger *value = new_integer(activation->getProgramArgumentCount());
            /* copy the value                    */
            pshvblock->shvret |= copyValue(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
        }
        /* some other parm form              */
        else if (!memcmp(variable, "PARM.", sizeof("PARM.") - 1))
        {
            wholenumber_t value_position;
            /* extract the numeric piece         */
            RexxString *tail = new_string(variable + strlen("PARM."));
            /* get the binary value              */
            /* not a good number?                */
            if (!tail->numberValue(value_position) || value_position <= 0)
            {
                /* this is a bad name                */
                pshvblock->shvret|=RXSHV_BADN;
            }
            else
            {
                /* get the arcgument from the parent activation */
                RexxObject *value = activation->getProgramArgument(value_position);
                if (value == OREF_NULL)
                {    /* doesn't exist?                    */
                    value = OREF_NULLSTRING; /* return a null string              */
                }
                /* copy the value                    */
                pshvblock->shvret |= copyValue(value, &pshvblock->shvvalue, (size_t *)&pshvblock->shvvaluelen);
            }
        }
        else
        {
            pshvblock->shvret|=RXSHV_BADN; /* this is a bad name                */
        }
    }
}


/**
 * Process a single variable pool request.
 *
 * @param pshvblock The request block for this request.
 */
void RexxNativeActivation::variablePoolRequest(PSHVBLOCK pshvblock)
{
    pshvblock->shvret = 0;               /* set the block return code         */

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
        default:
        {
            pshvblock->shvret |= RXSHV_BADF;   /* bad function                      */
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
RexxReturnCode RexxNativeActivation::copyValue(RexxObject * value, CONSTRXSTRING *rxstring, size_t *length)
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
RexxReturnCode RexxNativeActivation::copyValue(RexxObject * value, RXSTRING *rxstring, size_t *length)
{
    RexxString * stringVal;             /* converted object value            */
    stringsize_t string_length;         /* length of the string              */
    uint32_t     rc;                    /* return code                       */

    rc = 0;                             /* default to success                */
                                        /* get the string value              */
    stringVal = value->stringValue();
    string_length = stringVal->getLength();/* get the string length             */
    // caller allowing use to allocate this?
    if (rxstring->strptr == NULL)
    {
        rxstring->strptr = (char *)SystemInterpreter::allocateResultMemory(string_length + 1);
        if (rxstring->strptr == NULL)
        {
            return RXSHV_MEMFL;                  /* couldn't allocate, return flag */
        }
        rxstring->strlength = string_length + 1;
    }
    /* buffer too short?              */
    if (string_length > rxstring->strlength)
    {
        rc = RXSHV_TRUNC;                      /* set truncated return code      */
                                               /* copy the short piece           */
        memcpy(rxstring->strptr, stringVal->getStringData(), rxstring->strlength);
    }
    else
    {
        /* copy entire string             */
        memcpy(rxstring->strptr, stringVal->getStringData(), string_length);
        /* room for a null?               */
        if (rxstring->strlength > string_length)
        {
            /* yes, add one                   */
            rxstring->strptr[string_length] = '\0';
        }
        rxstring->strlength = string_length;   /* string length doesn't include terminating 0 */
    }
    *length = string_length;                 /* return actual string length    */
    return rc;                               /* give back the return code      */
}

int RexxNativeActivation::stemSort(const char *stemname, int order, int type, size_t start, size_t end, size_t firstcol, size_t lastcol)
/******************************************************************************/
/* Function:  Perform a sort on stem data.  If everything works correctly,    */
/*             this returns zero, otherwise an appropriate error value.       */
/******************************************************************************/
{
    size_t  position;                    /* scan position within compound name */
    size_t  length;                      /* length of tail section            */

                                         /* if access is enabled              */
    // NB:  The braces here are to ensure the ProtectedObjects get released before the
    // currentActivity gets zeroed out.
    {
        /* get the stem name as a string */
        RexxString *variable = new_string(stemname);
        ProtectedObject p1(variable);
        /* and get a retriever for this variable */
        RexxStemVariable *retriever = (RexxStemVariable *)RexxVariableDictionary::getVariableRetriever(variable);

        /* this must be a stem variable in order for the sorting to work. */

        if ( (!isOfClass(StemVariableTerm, retriever)) && (!isOfClass(CompoundVariableTerm, retriever)) )
        {
            return false;
        }

        RexxString *tail = OREF_NULLSTRING ;
        ProtectedObject p2(tail);

        if (isOfClass(CompoundVariableTerm, retriever))
        {
            length = variable->getLength();      /* get the string length             */
            position = 0;                        /* start scanning at first character */
            /* scan to the first period          */
            while (variable->getChar(position) != '.')
            {
                position++;                        /* step to the next character        */
                length--;                          /* reduce the length also            */
            }
            position++;                          /* step past previous period         */
            length--;                            /* adjust the length                 */
            tail = variable->extract(position, length);
            tail = tail->upper();
        }

        return retriever->sort(activation, tail, order, type, start, end, firstcol, lastcol);
    }
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
void RexxNativeActivation::forwardMessage(RexxObject *to, RexxString *msg, RexxClass *super, RexxArray *args, ProtectedObject &_result)
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
        to->messageSend(msg, args->data(), args->size(), _result);
    }
    else
    {
        to->messageSend(msg, args->data(), args->size(), super, _result);
    }
}


