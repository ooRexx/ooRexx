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
/* A reference to a local or object variable.                                 */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ActivityManager.hpp"
#include "VariableReference.hpp"
#include "RexxVariable.hpp"
#include "VariableDictionary.hpp"
#include "MethodArguments.hpp"
#include "ProtectedObject.hpp"

RexxClass *VariableReference::classInstance = OREF_NULL;   // singleton class instance

/**
 * Create initial bootstrap objects
 */
void VariableReference::createInstance()
{
    CLASS_CREATE(VariableReference);
}


/**
 * Allocate a new VariableReference object
 *
 * @param size   The size of the object.
 *
 * @return The newly allocated object.
 */
void *VariableReference::operator new(size_t size)
{
    return new_object(size, T_VariableReference);
}


/**
 * Constructor for a VariableReference object.
 *
 * @param v      The Variable object we're referencing.
 */
VariableReference::VariableReference(RexxVariable *v)
{
    variable = v;
}


/**
 * The Rexx accessible class NEW method.  This raises an
 * error because VariableReference objects can only be created
 * by the internal interpreter.
 *
 * @param args   The NEW args
 * @param argc   The count of arguments
 *
 * @return Never returns.
 */
RexxObject *VariableReference::newRexx(RexxObject **args, size_t argc)
{
    // we do not allow these to be allocated from Rexx code...
    reportException(Error_Unsupported_new_method, ((RexxClass *)this)->getId());
    return TheNilObject;
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void VariableReference::live(size_t liveMark)
{
    memory_mark(objectVariables);
    memory_mark(variable);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void VariableReference::liveGeneral(MarkReason reason)
{
    memory_mark_general(objectVariables);
    memory_mark_general(variable);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void VariableReference::flatten (Envelope *envelope)
{
    setUpFlatten(VariableReference)

    memory_mark_general(objectVariables);
    memory_mark_general(variable);

    cleanUpFlatten
}


/**
 * Get the name of this variable.
 *
 * @return The name of the referenced variable.
 */
RexxString *VariableReference::getName()
{
    return variable->getName();
}


/**
 * Get the value assigned to this variable
 *
 * @return The value of the referenced variable, or .nil if
 *         unassigned.
 */
RexxObject *VariableReference::getValue()
{
    return variable->getResolvedValue();
}


/**
 * Test if a variable reference is for a simple or stem
 * variable.
 *
 * @return true if this is a reference to a stem variable, false otherwise.
 */
bool VariableReference::isStem()
{
    return variable->isStem();
}


/**
 * Set a new value in the referenced variable
 *
 * @param v      The new variable value.
 */
void VariableReference::setValue(RexxObject *v)
{
    // NB: setValue sorts out the stem vs. simple assignment bits
    variable->setValue(v);
}


/**
 * Set a new value in the referenced variable (rexx exported
 * version)
 *
 * @param v      The new variable value.
 */
void VariableReference::setValueRexx(RexxObject *v)
{
    requiredArgument(v, "VALUE");

    // NB: setValue sorts out the stem vs. simple assignment bits
    variable->setValue(v);
}


/**
 * Process UNKNOWN messages for a variable reference object,
 * forwarding them to the referenced variable value.
 *
 * @param message   The message target.
 * @param arguments The message arguments.
 *
 * @return The message result.
 */
RexxObject *VariableReference::unknownRexx(RexxString *message, ArrayClass *arguments)
{
    Protected<RexxString> messageName = stringArgument(message, ARG_ONE);
    Protected<ArrayClass> argumentList = arrayArgument(arguments, ARG_TWO);

    ProtectedObject result;
    // get the resolved variable value and forward this to the value
    return variable->getResolvedValue()->sendMessage(messageName, argumentList, result);
}


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
void VariableReference::processUnknown(RexxErrorCodes error, RexxString *messageName, RexxObject **arguments, size_t count, ProtectedObject &result)
{
    // just send this as a message indirect variable value
    variable->getResolvedValue()->messageSend(messageName, arguments, count, result);
}


/**
 * Get the string value for the variableReference, which is just
 * the string value of our indirect value.
 *
 * @return The associated string value.
 */
RexxString *VariableReference::stringValue()
{
    return variable->getResolvedValue()->stringValue();
}


/**
 * Convert the stem default value to a numeric value.
 *
 * @param result the place to put the converted number.
 * @param digits The precision to convert under.
 *
 * @return true if this converted, false otherwise.
 */
bool VariableReference::numberValue(wholenumber_t &result, wholenumber_t digits)
{
    return variable->getResolvedValue()->numberValue(result, digits);
}


/**
 * Convert the stem default value to a numeric value.
 *
 * @param result the place to put the converted number.
 *
 * @return true if this converted, false otherwise.
 */
bool VariableReference::numberValue(wholenumber_t &result)
{
    return variable->getResolvedValue()->numberValue(result);
}


/**
 * Convert the stem default value to a numeric value.
 *
 * @param result the place to put the converted number.
 * @param digits The precision to convert under.
 *
 * @return true if this converted, false otherwise.
 */
bool VariableReference::unsignedNumberValue(size_t &result, wholenumber_t digits)
{
    return variable->getResolvedValue()->unsignedNumberValue(result, digits);
}


/**
 * Convert the stem default value to a numeric value.
 *
 * @param result the place to put the converted number.
 *
 * @return true if this converted, false otherwise.
 */
bool VariableReference::unsignedNumberValue(size_t &result)
{
    return variable->getResolvedValue()->unsignedNumberValue(result);
}


/**
 * Convert the stem default value to a numeric value.
 *
 * @param result the place to put the converted number.
 *
 * @return true if this converted, false otherwise.
 */
bool VariableReference::doubleValue(double &result)
{
    return variable->getResolvedValue()->doubleValue(result);
}


/**
 * Convert this stem object to a number string value.
 *
 * @return A number string object, or OREF_NULL if non-numeric
 */
NumberString *VariableReference::numberString()
{
   return variable->getResolvedValue()->numberString();
}


/**
 * Convert the stem to an integer value.
 *
 * @param precision The precision to convert under.
 *
 * @return The Integer object or OREF_NULL if not valid.
 */
RexxInteger *VariableReference::integerValue(wholenumber_t precision)
{
    return variable->getResolvedValue()->integerValue(precision);
}


/**
 * Forward all request requests to the target value.  We
 * handle some of these directly, the rest are passed on
 * to the default value.
 *
 * @param makeclass The name of the class for the request.
 *
 * @return The appropriate result for the request.
 */
RexxObject *VariableReference::request(RexxString *makeclass)
{
    ProtectedObject result;
    // let the default value handle everything else
    variable->getResolvedValue()->sendMessage(GlobalNames::REQUEST, makeclass, result);
    return result;
}


