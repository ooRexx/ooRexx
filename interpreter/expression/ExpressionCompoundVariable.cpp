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
/* REXX Translator                                                            */
/*                                                                            */
/* Primitive Translator Expression Parsing Compound Variable Reference Class  */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "QueueClass.hpp"
#include "StemClass.hpp"
#include "VariableDictionary.hpp"
#include "RexxActivation.hpp"
#include "Activity.hpp"
#include "ExpressionCompoundVariable.hpp"
#include "ExpressionVariable.hpp"
#include "RexxVariable.hpp"
#include "ProtectedObject.hpp"
#include "CompoundVariableTail.hpp"
#include "CompoundTableElement.hpp"


/**
 * Allocate storage for a compound variable object.
 *
 * @param size      The base object size
 * @param tailCount The number of tail elements in the variable.
 *
 * @return storage for the object.
 */
void * RexxCompoundVariable::operator new(size_t size, size_t tailCount)
{
    // belt-and-braces...it would be a strange compound variable indeed with
    // no tail pieces.
    if (tailCount == 0)
    {
        // this object is normal sized, minus the dummy tail element
        return new_object(size - sizeof(RexxObject *), T_CompoundVariableTerm);
    }
    else
    {
        // adjust the size based on the number of tail elements.
        return new_object(size + ((tailCount - 1) * sizeof(RexxObject *)), T_CompoundVariableTerm);
    }
}


/**
 * Construct a retriever object for a compound variable
 * instance.
 *
 * @param _stemName The string stem name.
 * @param stemIndex The stem variable stack frame cache.
 * @param tailList  The queue for obtaining the tail elements (pushed on in reverse order)
 * @param _tailCount The count of the tail elements.
 */
RexxCompoundVariable::RexxCompoundVariable(RexxString *_stemName,
    size_t index, QueueClass *tailList, size_t _tailCount)
{
    tailCount = _tailCount;
    stemName = _stemName;
    stemIndex = index;

    // initialize the list of tails
    initializeObjectArray(_tailCount, tails, RexxInternalObject, tailList);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxCompoundVariable::live(size_t liveMark)
{
    memory_mark(stemName);
    memory_mark_array(tailCount, tails)
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxCompoundVariable::liveGeneral(MarkReason reason)
{
    memory_mark_general(stemName);
    memory_mark_general_array(tailCount, tails)
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxCompoundVariable::flatten(Envelope *envelope)
{
    setUpFlatten(RexxCompoundVariable)

    flattenRef(stemName);
    flattenArrayRefs(tailCount, tails);

    cleanUpFlatten
}


/**
 * Evaluate a compound variable in an expression.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 *
 * @return The compound variable value.
 */
RexxObject * RexxCompoundVariable::evaluate(RexxActivation *context, ExpressionStack *stack )
{
    // the context does the real evaluation work
    RexxObject *value = context->evaluateLocalCompoundVariable(stemName, stemIndex, &tails[0], tailCount);
    // expression values need to be pushed on to the stack before returning
    stack->push(value);
    return value;
}

/**
 * retrieve the value of a compound variable from a variable dictionary
 *
 * @param dictionary The variable dictionary context for the retrieval.
 *
 * @return The compound variable value (note, no stack pushing here).
 */
RexxObject *RexxCompoundVariable::getValue(VariableDictionary *dictionary)
{
    // the dictionary handles the details
    return dictionary->getCompoundVariableValue(stemName, &tails[0], tailCount);
}


/**
 * Direct value retrieval of a compound variable value.
 *
 * @param context The execution context to retrieve the value from.
 *
 * @return The compound variable value.
 */
RexxObject *RexxCompoundVariable::getValue(RexxActivation *context)
{
                                       /* resolve the tail element          */
    return context->getLocalCompoundVariableValue(stemName, stemIndex, &tails[0], tailCount);
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
RexxObject *RexxCompoundVariable::getRealValue(VariableDictionary *dictionary)
{
    return dictionary->getCompoundVariableRealValue(stemName, &tails[0], tailCount);
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
RexxObject *RexxCompoundVariable::getRealValue(RexxActivation *context)
{
    return context->getLocalCompoundVariableRealValue(stemName, stemIndex, &tails[0], tailCount);
}


/**
 * Set the value of a compound variable.
 *
 * @param context The variable context to operate in.
 * @param value   The new value to assign.
 */
void RexxCompoundVariable::set(RexxActivation *context, RexxObject *value)
{
                                       /* the dictionary manages all of these details */
    context->setLocalCompoundVariable(stemName, stemIndex, &tails[0], tailCount, value);
}


/**
 * Set the value of a compound variable.
 *
 * @param dictionary The variable dictionary context for the set operation.
 * @param value      The new variable value.
 */
void RexxCompoundVariable::set(VariableDictionary *dictionary, RexxObject *value)
{
    dictionary->setCompoundVariable(stemName, &tails[0], tailCount, value);
}


/**
 * Test if a compound variable exists.
 *
 * @param context The variable context.
 *
 * @return True if the variable has an assigned value, false
 *         otherwise.
 */
bool RexxCompoundVariable::exists(RexxActivation *context)
{
    return context->localCompoundVariableExists(stemName, stemIndex, &tails[0], tailCount);
}


/**
 * Perform a variable assignment operation (in the context of an assignment instruction)
 *
 * @param context The current execution context.
 * @param value   The new variable value.
 */
void RexxCompoundVariable::assign(RexxActivation *context, RexxObject *value )
{
    context->assignLocalCompoundVariable(stemName, stemIndex, &tails[0], tailCount, value);
}


/**
 * Drop a compound variable.
 *
 * @param context The current execution context.
 */
void RexxCompoundVariable::drop(RexxActivation *context)
{
    context->dropLocalCompoundVariable(stemName, stemIndex, &tails[0], tailCount);
}


/**
 * Drop a variable that's directly in a variable dictionary.
 *
 * @param dictionary The target dictionary
 */
void RexxCompoundVariable::drop(VariableDictionary *dictionary)
{
    dictionary->dropCompoundVariable(stemName, &tails[0], tailCount);
}


/**
 * Expose a compound variable as part of a PROCEDURE
 * instruction.
 *
 * @param context The current execution context.
 * @param parent  The parent (calling) context.
 * @param stack   The current evaluation stack.
 */
void RexxCompoundVariable::procedureExpose(RexxActivation *context, RexxActivation *parent)
{
    // first get (and possible create) the compound variable in the parent context.
    CompoundTableElement *variable = parent->exposeLocalCompoundVariable(stemName, stemIndex, &tails[0], tailCount);
    // get the stem index from the current level.  This may end up
    // creating the stem that holds the exposed value.
    StemClass *stem_table = context->getLocalStem(stemName, stemIndex);
    // have the stem expose this
    stem_table->expose(variable);
    // trace resolved compound name
    context->traceCompoundName(stemName, &tails[0], tailCount, variable->getName());
}


/**
 * Expose an object compound variable in a method.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 * @param object_dictionary
 */
void RexxCompoundVariable::expose(RexxActivation *context, VariableDictionary *object_dictionary)
{
    // get the stem in the source dictionary
    StemClass *source_stem = object_dictionary->getStem(stemName);
    // new tail for the compound variable
    CompoundVariableTail resolved_tail(context, &tails[0], tailCount);
    // first get (and possible create) the compound variable in the
    // object context.
    CompoundTableElement *variable = source_stem->exposeCompoundVariable(resolved_tail);
    // get the stem index from the current level.  This may end up
    // creating the stem that holds the exposed value.
    StemClass *stem_table = context->getLocalStem(stemName, stemIndex);
    // have the stem expose this
    stem_table->expose(variable);
    // trace resolved compound name
    context->traceCompoundName(stemName, &tails[0], tailCount, variable->getName());
}


/**
 * Set a guard wait on a compound variable.
 *
 * @param context The current execution context.
 */
void RexxCompoundVariable::setGuard(RexxActivation *context )
{
    // get the variable element and add our activity to the inform list.
    CompoundTableElement *variable = context->getLocalCompoundVariable(stemName, stemIndex, &tails[0], tailCount);
    variable->inform(ActivityManager::currentActivity);
}

/**
 * Clear the guard watch on a compound variable instance.
 *
 * @param context The current execution context.
 */
void RexxCompoundVariable::clearGuard(RexxActivation *context )
{
    // get the variable context and remove this activity from the watch list.
    CompoundTableElement *variable = context->getLocalCompoundVariable(stemName, stemIndex, &tails[0], tailCount);
    variable->uninform(ActivityManager::currentActivity);
}

