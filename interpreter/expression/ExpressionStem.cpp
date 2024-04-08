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
/* REXX Translator                                       ExpressionStem.cpp   */
/*                                                                            */
/* Primitive Translator Expression Parsing Stem Reference Class               */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "RexxVariable.hpp"
#include "VariableDictionary.hpp"
#include "StemClass.hpp"
#include "ExpressionStem.hpp"
#include "VariableReference.hpp"


/**
 * Allocate a new stem variable object.
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *RexxStemVariable::operator new(size_t size)
{
    return new_object(size, T_StemVariableTerm);
}


/**
 * Construct a Stem variable expression object.
 *
 * @param name      The name of the stem (including the period)
 * @param var_index The index of the variable slot in the current stack frame.
 */
RexxStemVariable::RexxStemVariable(RexxString *name, size_t var_index)
{
    stemName = name;
    stemIndex = var_index;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxStemVariable::live(size_t liveMark)
{
    memory_mark(stemName);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxStemVariable::liveGeneral(MarkReason reason)
{
    memory_mark_general(stemName);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxStemVariable::flatten(Envelope *envelope)
{
    setUpFlatten(RexxStemVariable)

    flattenRef(stemName);

    cleanUpFlatten
}


/**
 * Evaluate a Rexx Stem variable.
 *
 * @param context The current execution context
 * @param stack   The current evaluation stack.
 *
 * @return The variable value (which will be a Stem object instance.)
 */
RexxObject  *RexxStemVariable::evaluate(RexxActivation *context, ExpressionStack *stack )
{
    // look up the name
    RexxObject *value = context->getLocalStem(stemName, stemIndex);
    // NOTE:  stem accesses do NOT report NOVALUE so that DO OVER,
    // call-by-reference with a stem and return with a stem does not
    // trigger a novalue trap unexpectedly

    // evaluate always pushes on the stack.
    stack->push(value);
    context->traceVariable(stemName, value);
    return value;
}


/**
 * Retrieve a stem variable value from a variable dictionary.
 *
 * @param dictionary The source dictionary.
 *
 * @return The current stem variable contents.
 */
RexxObject  *RexxStemVariable::getValue(VariableDictionary *dictionary)
{
    // just look up the stem
    return dictionary->getStem(stemName);
}


/**
 * Retrieve a stem variable value from a given program context.
 *
 * @param context The source context.
 *
 * @return The variable value (a stem object).
 */
RexxObject  *RexxStemVariable::getValue(RexxActivation *context)
{
    return context->getLocalStem(stemName, stemIndex);
}

/**
 * Retrieve the real value of a stem variable.  Stem variables
 * will always be created on first reference, so there is no
 * difference between getValue() and getRealValue().
 *
 * @param dictionary The source variable dictionary.
 *
 * @return The stem object representing this variable.
 */
RexxObject  *RexxStemVariable::getRealValue(VariableDictionary *dictionary)
{
    return dictionary->getStem(stemName);
}

/**
 * Retrieve the real value of a stem variable.  Stem variables
 * will always be created on first reference, so there is no
 * difference between getValue() and getRealValue().
 *
 * @param context The current execution context.
 *
 * @return The stem object representing this variable.
 */
RexxObject  *RexxStemVariable::getRealValue(RexxActivation *context)
{
    return context->getLocalStem(stemName, stemIndex);
}


/**
 * Set a value in a stem variable...this is more direct
 * than an assign operation.
 *
 * @param context The variable context.
 * @param value   The new value.
 */
void RexxStemVariable::set(RexxActivation *context, RexxObject *value )
{
    // Look up the stem
    RexxVariable *variable = context->getLocalStemVariable(stemName, stemIndex);
    // the variable object handles the setting details
    variable->setStem(value);
}


/**
 * Set a variable using a variable dictionary context.
 *
 * @param dictionary The target variable dictionary.
 * @param value      The new variable value.
 */
void RexxStemVariable::set(VariableDictionary  *dictionary, RexxObject *value )
{
    // look up the stem variable in the dictionary
    RexxVariable *variable = dictionary->getStemVariable(stemName);
    // the variable object handles the setting details
    variable->setStem(value);
}


/**
 * Test if a stem variable exists in the current context.
 *
 * @param context The current execution context.
 *
 * @return true if the variable exists, false otherwise.
 */
bool RexxStemVariable::exists(RexxActivation *context)
{
    return context->localStemVariableExists(stemName, stemIndex);
}


/**
 * Assign a new value to a stem object (used by assignment
 * instructions).
 *
 * @param context The current execution context.
 * @param value   The new value.
 */
void RexxStemVariable::assign(RexxActivation *context, RexxObject *value )
{
    // get the stem variable
    RexxVariable *variable = context->getLocalStemVariable(stemName, stemIndex);
    // the variable object handles the setting details
    variable->setStem(value);
    // trace the assignment
    context->traceAssignment(stemName, value);
}


/**
 * Drop a stem variable from the current context.
 *
 * @param context The current execution context.
 */
void RexxStemVariable::drop(RexxActivation *context)
{
    context->dropLocalStem(stemName, stemIndex);
}


/**
 * Drop a variable that's directly in a variable dictionary.
 *
 * @param dictionary The target dictionary
 */
void RexxStemVariable::drop(VariableDictionary *dictionary)
{
    // dropping the stem name is sufficient
    dictionary->dropStemVariable(stemName);
}


/**
 * Handle procedure expose for a stem variable.
 *
 * @param context The current execution context.
 * @param parent  The parent execution context (source of the variable.)
 */
void RexxStemVariable::procedureExpose(RexxActivation *context, RexxActivation *parent)
{
    // get the old variable entry
    RexxVariable *old_variable = parent->getLocalStemVariable(stemName, stemIndex);

    // if we have an index (and generally we should because procedure cannot
    // be interpreted), we just put this variable into the local context.
    // otherwise, we need to do a dynamic search to update this.
    if (stemIndex == 0)
    {
        context->updateLocalVariable(old_variable);
    }
    else
    {
        context->putLocalVariable(old_variable, stemIndex);
    }
}


/**
 * Expose a variable in a specific object variable scope.
 *
 * @param context The current execution context.
 * @param object_dictionary
 *                The source object scope variable dictionary.
 */
void RexxStemVariable::expose(RexxActivation *context, VariableDictionary *object_dictionary)
{
    // get the old variable entry (will create if first request)
    RexxVariable *old_stem = object_dictionary->getStemVariable(stemName);
    // and make this a local variable
    context->putLocalVariable(old_stem, stemIndex);
}


/**
 * Set a GUARD WHEN watch on a stem variable.
 *
 * @param context The current execution context.
 */
void RexxStemVariable::setGuard(RexxActivation *context)
{
    // get the variable and ask for our activity to be notified.
    RexxVariable *variable = context->getLocalStemVariable(stemName, stemIndex);
    variable->inform(context->getActivity());
}


/**
 * Remove a guard notification from a variable.
 *
 * @param context The current execution context.
 */
void RexxStemVariable::clearGuard(RexxActivation *context )
{
    // look up the variable and remove the inform status for this activity.
    RexxVariable *variable = context->getLocalStemVariable(stemName, stemIndex);
    variable->uninform(context->getActivity());
}


/**
 * Set a GUARD WHEN watch on a stem variable.
 *
 * @param dictionary The target dictionary
 */
void RexxStemVariable::setGuard(VariableDictionary *dictionary)
{
    // get the variable and ask for our activity to be notified.
    RexxVariable *variable = dictionary->getStemVariable(stemName);
    variable->inform(ActivityManager::currentActivity);
}


/**
 * Remove a guard notification from a variable.
 *
 * @param dictionary The target dictionary
 */
void RexxStemVariable::clearGuard(VariableDictionary *dictionary)
{
    // look up the variable and remove the inform status for this activity.
    RexxVariable *variable = dictionary->getStemVariable(stemName);
    variable->uninform(ActivityManager::currentActivity);
}


/**
 * Alias a local variable name to a supplied variable reference
 * from another context.
 *
 * @param context  The variable context
 * @param variable The variable object.
 */
void RexxStemVariable::alias(RexxActivation *context, RexxVariable *variable)
{
    // since we don't create the stem object here, we can treat the stem
    // variables and the simple variables the same.
    context->aliasLocalVariable(stemName, stemIndex, variable);
}


/**
 * Retrieve a VariableReference object for this variable from
 * the given source dictionary.
 *
 * @param dictionary The source variable dictionary.
 *
 * @return The variable reference for this variable.
 */
VariableReference *RexxStemVariable::getVariableReference(VariableDictionary *dictionary)
{
    // look up the stem variable in the dictionary
    RexxVariable *variable = dictionary->getStemVariable(stemName);
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
VariableReference *RexxStemVariable::getVariableReference(RexxActivation *context)
{
    RexxVariable *variable = context->getLocalStemVariable(stemName, stemIndex);
    return variable->createReference();
}


/**
 * Sort the elements of a stem object using the old
 * stem array convention.
 *
 * @param context  The current execution context.
 * @param prefix   The stem name prefix for sub variable sorts.
 * @param order    The sort order.
 * @param type     The type of sort (ascending vs. descending).
 * @param start    The starting position.
 * @param end      The end position.
 * @param firstcol The first column to sort on.
 * @param lastcol  the last column to sort on.
 *
 * @return The success/failure indicator.
 */
bool RexxStemVariable::sort(RexxActivation *context, RexxString *prefix,
    int order, int type, size_t start, size_t end, size_t firstcol, size_t lastcol)
{
    // get the stem variable and ask it's value to perform the sort
    StemClass *stem_table = context->getLocalStem(stemName, stemIndex);
    // the stem object handles the sorting.
    return stem_table->sort(prefix, order, type, start, end, firstcol, lastcol);
}

