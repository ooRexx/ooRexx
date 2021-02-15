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
/* REXX Translator                                  ExpressionVariable.cpp    */
/*                                                                            */
/* Primitive Translator Expression Parsing Variable Reference Class           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "Activity.hpp"
#include "VariableDictionary.hpp"
#include "RexxVariable.hpp"
#include "ExpressionVariable.hpp"
#include "VariableReference.hpp"


/**
 * Allocate storage for a simple variable retriever.
 *
 * @param size   The base object size.
 *
 * @return Rexx object storage for the object.
 */
void *RexxSimpleVariable::operator new(size_t size)
{
    return new_object(size, T_VariableTerm);
}


/**
 * Construct a simple variable instance.
 *
 * @param variable_name
 *                  The name of the variable.
 * @param var_index The variable index in the local context.
 */
RexxSimpleVariable::RexxSimpleVariable(RexxString *variable_name, size_t var_index)
{
    variableName = variable_name;
    index = var_index;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxSimpleVariable::live(size_t liveMark)
{
    memory_mark(variableName);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxSimpleVariable::liveGeneral(MarkReason reason)
{
    memory_mark_general(variableName);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxSimpleVariable::flatten(Envelope *envelope)
{
    setUpFlatten(RexxSimpleVariable)

    flattenRef(variableName);

    cleanUpFlatten
}


/**
 * Evaluate a simple variable in an expression.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 *
 * @return The variable value (also pushed on stack)
 */
RexxObject *RexxSimpleVariable::evaluate(RexxActivation *context, ExpressionStack *stack)
{
    // look up the variable
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    RexxObject *value = variable->getVariableValue();
    // we need to do novalue checks.
    if (value == OREF_NULL)
    {
        // try the various novalue mechanisms
        value = context->handleNovalueEvent(variableName, variableName, variable);
    }
    // stack, trace, and return
    stack->push(value);

    context->traceVariable(variableName, value);
    return value;
}


/**
 * retrieve a simple variable's value (notready condition will
 * not be raised)
 *
 * @param dictionary The source variable dictionary.
 *
 * @return The variable value.
 */
RexxObject *RexxSimpleVariable::getValue(VariableDictionary *dictionary)
{
    RexxVariable *variable = dictionary->getVariable(variableName);
    RexxObject *value = variable->getVariableValue();
    // if no variable yet, return the name.
    if (value == OREF_NULL)
    {
        value = variableName;
    }
    return value;
}


/**
 * retrieve a simple variable's value (notready condition will
 * not be raised)
 *
 * @param context The current execution context.
 *
 * @return The variable value.
 */
RexxObject  *RexxSimpleVariable::getValue(RexxActivation *context)
{
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    RexxObject *value = variable->getVariableValue();
    // use the variable name if not set.
    if (value == OREF_NULL)
    {
        value = variableName;
    }
    return value;
}


/**
 * Retrieve an object variable value, returning OREF_NULL if
 * the variable does not have a value.
 *
 * @param dictionary The source variable dictionary.
 *
 * @return The variable value, or OREF_NULL if the variable is not
 *         assigned.
 */
RexxObject *RexxSimpleVariable::getRealValue(VariableDictionary *dictionary)
{
    RexxVariable *variable = dictionary->getVariable(variableName);
    return variable->getVariableValue();
}


/**
 * Retrieve a VariableReference object for this variable from
 * the given source dictionary.
 *
 * @param dictionary The source variable dictionary.
 *
 * @return The variable reference for this variable.
 */
VariableReference *RexxSimpleVariable::getVariableReference(VariableDictionary *dictionary)
{
    RexxVariable *variable = dictionary->getVariable(variableName);
    return variable->createReference();
}


/**
 * Get a variable reference to a variable from the given
 * activation context.
 *
 * @param context The current context.
 *
 * @return A variable reference object for the variable.
 */
VariableReference *RexxSimpleVariable::getVariableReference(RexxActivation *context)
{
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    return variable->createReference();
}


/**
 * Get the value of a variable without applying a default value
 * to it.  Used in the apis so the caller can more easily
 * detect an uninitialized variable.
 *
 * @param context The current context.
 *
 * @return The value of the variable.  Returns OREF_NULL if the variable
 *         has not been assigned a value.
 */
RexxObject  *RexxSimpleVariable::getRealValue(RexxActivation *context)
{
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    return variable->getVariableValue();
}


/**
 * Set a simple variable.
 *
 * @param dictionary The target variable dictionary.
 * @param value      The new value.
 */
void RexxSimpleVariable::set(VariableDictionary *dictionary, RexxObject *value )
{
    RexxVariable *variable = dictionary->getVariable(variableName);
    variable->set(value);
}

/**
 * Set a variable value in the current context.
 *
 * @param context The current execution context.
 * @param value   The value to set.
 */
void RexxSimpleVariable::set(RexxActivation *context, RexxObject *value )
{
    // The context handles the details of this
    context->setLocalVariable(variableName, index, value);
}


/**
 * Test if a variable exists in the current context.
 *
 * @param context The target execution context.
 *
 * @return true if the variable exists, false otherwise.
 */
bool RexxSimpleVariable::exists(RexxActivation *context)
{
    return context->localVariableExists(variableName, index);
}

/**
 * Assign a value to a simple variable.
 *
 * @param context The current execution context.
 * @param value   The value to assign.
 */
void RexxSimpleVariable::assign(RexxActivation *context, RexxObject *value)
{
    // The context handles the details of this
    context->setLocalVariable(variableName, index, value);
    context->traceAssignment(variableName, value);
}


/**
 * Drop a variable in the current execution context.
 *
 * @param context The current execution context.
 */
void RexxSimpleVariable::drop(RexxActivation *context)
{
    // the context handles all of this
    context->dropLocalVariable(variableName, index);
}


/**
 * Drop a variable that's directly in a variable dictionary.
 *
 * @param dictionary The target dictionary
 */
void RexxSimpleVariable::drop(VariableDictionary *dictionary)
{
    RexxVariable *variable = dictionary->getVariable(variableName);
    variable->drop();
}


/**
 * Set a guard notification on a simple variable.
 *
 * @param context The current execution context.
 */
void RexxSimpleVariable::setGuard(RexxActivation *context )
{
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    variable->inform(ActivityManager::currentActivity);
}


/**
 * Remove a GUARD WHEN watch from a simple variable.
 *
 * @param context The current execution context.
 */
void RexxSimpleVariable::clearGuard(RexxActivation *context)
{
                                       /* look up the name                  */
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    variable->uninform(ActivityManager::currentActivity); /* remove the notification           */
}


/**
 * Set a guard notification on a simple variable.
 *
 * @param dictionary The target dictionary
 */
void RexxSimpleVariable::setGuard(VariableDictionary *dictionary)
{
    RexxVariable *variable = dictionary->getVariable(variableName);
    variable->inform(ActivityManager::currentActivity);
}


/**
 * Remove a GUARD WHEN watch from a simple variable.
 *
 * @param dictionary The target dictionary
 */
void RexxSimpleVariable::clearGuard(VariableDictionary *dictionary)
{
    RexxVariable *variable = dictionary->getVariable(variableName);
    variable->uninform(ActivityManager::currentActivity); /* remove the notification           */
}


/**
 * Perform a PROCEDURE EXPOSE operation on a simple variable.
 *
 * @param context The current execution context.
 * @param parent  The parent execution context.
 */
void RexxSimpleVariable::procedureExpose(RexxActivation *context, RexxActivation *parent)
{
    // get this from the old context.
    RexxVariable *old_variable = parent->getLocalVariable(variableName, index);
    // and poke it into our new table.
    context->putLocalVariable(old_variable, index);
}


/**
 * Expose a simple object variable.
 *
 * @param context The current execution context.
 * @param object_dictionary
 *                The target variable dictionary from the method scope.
 */
void RexxSimpleVariable::expose(RexxActivation *context, VariableDictionary *object_dictionary)
{
    // get the old variable entry
    RexxVariable *old_variable = object_dictionary->getVariable(variableName);
    // set the entry in the new table
    context->putLocalVariable(old_variable, index);
}


/**
 * Return the name of this variable.
 *
 * @return The string value of the variable name.
 */
RexxString *RexxSimpleVariable::getName()
{
    return variableName;
}


/**
 * Alias a local variable name to a supplied variable reference
 * from another context.
 *
 * @param context  The variable context
 * @param variable The variable object.
 */
void RexxSimpleVariable::alias(RexxActivation *context, RexxVariable *variable)
{
    context->aliasLocalVariable(variableName, index, variable);
}

