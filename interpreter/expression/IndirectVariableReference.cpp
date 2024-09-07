/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
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
/* REXX Translator                            IndirectVariableReference.cpp   */
/*                                                                            */
/* Primitive Translator Expression Parsing Variable Indirect Reference Class  */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "Activity.hpp"
#include "VariableDictionary.hpp"
#include "IndirectVariableReference.hpp"
#include "MethodArguments.hpp"


/**
 * Allocate memory for a RexxVariableReference item.
 *
 * @param size   The base object size.
 *
 * @return Storage for creating a new variable instance.
 */
void *RexxVariableReference::operator new(size_t size)
{
    return new_object(size, T_IndirectVariableTerm);
}


/**
 * Construct an indirect variable reference.
 *
 * @param variable The variable instance used for retrieval and
 *                 sub operations.
 */
RexxVariableReference::RexxVariableReference(RexxVariableBase *variable)
{
    variableObject = variable;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxVariableReference::live(size_t liveMark)
{
    memory_mark(variableObject);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxVariableReference::liveGeneral(MarkReason reason)
{
    memory_mark_general(variableObject);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxVariableReference::flatten(Envelope *envelope)
{
    setUpFlatten(RexxVariableReference)

    flattenRef(variableObject);

    cleanUpFlatten
}


/**
 * Parse a variable's string contents into an array of
 * "good" variable retrievers.
 *
 * @param context The current execution context.
 *
 * @return An array of variable retrievers.
 */
ArrayClass *RexxVariableReference::list(RexxActivation *context)
{
    // evaluate the variable.
    // this also traces and does the NOVALUE checks.
    RexxObject *value = variableObject->evaluate(context, context->getStack());
    // force to string form
    Protected<RexxString> nameString = value->requestString();

    // get this as a list of words
    Protected<ArrayClass> list = ((RexxString *)nameString)->subWords(OREF_NULL, OREF_NULL);

    size_t count = list->size();

    // now process all of the names, replacing them with retrievers
    for (size_t i = 1; i <= count; i++)
    {
        // get the next name
        RexxString *variable_name = (RexxString *)list->get(i);

        // get the first character
        unsigned int character = variable_name->getChar(0);
        // report obvious non-variable names
        // start with a period
        if (character == '.')
        {
            reportException(Error_Invalid_variable_period, variable_name);
        }
        // or a digit
        else if (character >= '0' && character <= '9')
        {
            reportException(Error_Invalid_variable_number, variable_name);
        }
        // now convert into a variable reference
        RexxVariableBase *retriever = VariableDictionary::getVariableRetriever(variable_name);
        // if it did not convert (invalid characters, for example), report this
        if (retriever == OREF_NULL)
        {
            reportException(Error_Symbol_expected_indirect, variable_name);
        }
        // replace the name with the retriever
        list->put(retriever, i);
    }
    return list;
}


/**
 * Perform a drop operation using an indirect name.
 *
 * @param context The current execution context.
 */
void RexxVariableReference::drop(RexxActivation *context)
{
    // evaluate into a variable list
    ArrayClass *variables = list(context);
    ProtectedObject p(variables);

    size_t count = variables->size();
    // perform the drop on all of the list variables
    for (size_t i = 1; i <= count; i++)
    {
        RexxVariableBase *variable = (RexxVariableBase *)variables->get(i);
        variable->drop(context);
    }
}


/**
 * Expose a subsidiary list of variables.
 *
 * @param context The current execution context.
 * @param parent  The parent execution context.
 */
void RexxVariableReference::procedureExpose(RexxActivation *context, RexxActivation *parent)
{
    // expose this variable first
    variableObject->procedureExpose(context, parent);

    // evaluate into a variable list
    ArrayClass *variables = list(context);
    ProtectedObject p(variables);

    size_t count = variables->size();
    // perform the expose on all of the list variables
    for (size_t i = 1; i <= count; i++)
    {
        RexxVariableBase *variable = (RexxVariableBase *)variables->get(i);
        variable->procedureExpose(context, parent);
    }
}


/**
 * Expose a subsidiary list of variables.
 *
 * @param context The current execution context.
 * @param object_dictionary
 *                The source object scope variable dictionary.
 */
void RexxVariableReference::expose(RexxActivation *context, VariableDictionary *object_dictionary)
{
    // expose the variable first
    variableObject->expose(context, object_dictionary);
    // evaluate into a variable list
    ArrayClass *variables = list(context);
    ProtectedObject p(variables);

    size_t count = variables->size();
    // perform the expose on all of the list variables
    for (size_t i = 1; i <= count; i++)
    {
        RexxVariableBase *variable = (RexxVariableBase *)variables->get(i);
        variable->expose(context, object_dictionary);
    }
}

