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
/* REXX Translator                                  ExpressionVariable.cpp    */
/*                                                                            */
/* Primitive Translator Expression Parsing Variable Reference Class           */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "RexxVariableDictionary.hpp"
#include "RexxVariable.hpp"
#include "ExpressionVariable.hpp"

RexxParseVariable::RexxParseVariable(
  RexxString *variable_name,           /* variable name to access           */
  size_t var_index)                    /* dictionary lookaside index        */
/******************************************************************************/
/* Complete initialization of a variable object                               */
/******************************************************************************/
{
                                       /* set the name value                */
  OrefSet(this, this->variableName, variable_name);
  this->index = var_index;             /* save the index                    */
}

void RexxParseVariable::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->variableName);
}

void RexxParseVariable::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->variableName);
}

void RexxParseVariable::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxParseVariable)

  flatten_reference(newThis->variableName, envelope);

  cleanUpFlatten
}

RexxObject  *RexxParseVariable::evaluate(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/******************************************************************************/
/* Function:  Evaluate a REXX simple variable                                 */
/******************************************************************************/
{
    /* look up the name                  */
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    RexxObject *value = variable->getVariableValue();/* get the value                     */
    if (value == OREF_NULL)              /* no value yet?                     */
    {
        // try the various novalue mechanisms
        value = context->handleNovalueEvent(variableName, variable);
    }
    stack->push(value);                  /* place on the evaluation stack     */
                                         /* trace if necessary                */
    context->traceVariable(variableName, value);
    return value;                        /* return the located variable       */
}

RexxObject  *RexxParseVariable::getValue(
    RexxVariableDictionary *dictionary)/* current activation dictionary     */
/******************************************************************************/
/* Function:  retrieve a simple variable's value (notready condition will     */
/*            not be raised)                                                  */
/******************************************************************************/
{
    /* look up the name                  */
    RexxVariable *variable = dictionary->getVariable(variableName);
    RexxObject *value = variable->getVariableValue();/* get the value                     */
    if (value == OREF_NULL)              /* no value yet?                     */
    {
        value = this->variableName;        /* just use the name                 */
    }
    return value;                        /* return the located variable       */
}

RexxObject  *RexxParseVariable::getValue(
    RexxActivation *context)           /* current activation context        */
/******************************************************************************/
/* Function:  retrieve a simple variable's value (notready condition will     */
/*            not be raised)                                                  */
/******************************************************************************/
{
    /* look up the name                  */
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    RexxObject *value = variable->getVariableValue();/* get the value                     */
    if (value == OREF_NULL)              /* no value yet?                     */
    {
        value = this->variableName;        /* just use the name                 */
    }
    return value;                        /* return the located variable       */
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
RexxObject  *RexxParseVariable::getRealValue(RexxVariableDictionary *dictionary)
{
                                       /* look up the name                  */
    RexxVariable *variable = dictionary->getVariable(variableName);
    return variable->getVariableValue();/* get the value                     */
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
RexxObject  *RexxParseVariable::getRealValue(RexxActivation *context)
{
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    return variable->getVariableValue();/* get the value                     */
}

void RexxParseVariable::set(
  RexxVariableDictionary  *dictionary, /* current activation dictionary     */
  RexxObject *value )
/******************************************************************************/
/* Function:  Fast set of a variable value                                    */
/******************************************************************************/
{
                                       /* look up the name                  */
    RexxVariable *variable = dictionary->getVariable(variableName);
    variable->set(value);                /* and perform the set               */
}

void RexxParseVariable::set(
  RexxActivation *context,             /* current activation context        */
  RexxObject *value )
/******************************************************************************/
/* Function:  Fast set of a variable value                                    */
/******************************************************************************/
{
    /* The context handles the details of this */
    context->setLocalVariable(variableName, index, value);
}

bool RexxParseVariable::exists(
  RexxActivation *context)             /* current activation context        */
/******************************************************************************/
/*  Function:  Check the existance of a REXX variable                         */
/******************************************************************************/
{
    return context->localVariableExists(variableName, index);
}

void RexxParseVariable::assign(
    RexxActivation *context,           /* current activation context        */
    RexxExpressionStack *stack,        /* current evaluation stack          */
    RexxObject     *value )            /* new value to assign               */
/******************************************************************************/
/* Function:  Assign a value to a simple variable                             */
/******************************************************************************/
{
    /* The context handles the details of this */
    context->setLocalVariable(variableName, index, value);
    context->traceAssignment(variableName, value);
}

void RexxParseVariable::drop(
  RexxActivation *context)             /* target variable dictionary        */
/******************************************************************************/
/* Function:  Drop a variable object                                          */
/******************************************************************************/
{
                                       /* drop the variable value           */
  context->dropLocalVariable(variableName, index);
}

/**
 * Drop a variable that's directly in a variable dictionary.
 *
 * @param dictionary The target dictionary
 */
void RexxParseVariable::drop(RexxVariableDictionary *dictionary)
{
                                       /* look up the name                  */
    RexxVariable *variable = dictionary->getVariable(variableName);
    variable->drop();                    /* and perform the set               */
}

void RexxParseVariable::setGuard(
  RexxActivation *context )            /* current activation context        */
/******************************************************************************/
/* Set a guard variable notification on an object variable                    */
/******************************************************************************/
{
                                       /* look up the name                  */
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    variable->inform(ActivityManager::currentActivity);   /* mark the variable entry           */
}

void RexxParseVariable::clearGuard(
  RexxActivation *context )            /* current activation context        */
/******************************************************************************/
/* Remove a guard variable notification on an object variable                 */
/******************************************************************************/
{
                                       /* look up the name                  */
    RexxVariable *variable = context->getLocalVariable(variableName, index);
    variable->uninform(ActivityManager::currentActivity); /* remove the notification           */
}

void RexxParseVariable::procedureExpose(
  RexxActivation      *context,        /* current activation context        */
  RexxActivation      *parent,         /* the parent activation context     */
  RexxExpressionStack *stack)          /* current evaluation stack          */
/******************************************************************************/
/* Function:  Expose a variable                                               */
/******************************************************************************/
{
                                         /* get the old variable entry        */
    RexxVariable *old_variable = parent->getLocalVariable(variableName, index);
                                         /* set the entry in the new table    */
    context->putLocalVariable(old_variable, index);
}


void RexxParseVariable::expose(
  RexxActivation      *context,        /* current activation context        */
  RexxExpressionStack *stack,          /* current evaluation stack          */
                                       /* variable scope we're exposing from*/
  RexxVariableDictionary *object_dictionary)
/******************************************************************************/
/* Function:  Expose a variable                                               */
/******************************************************************************/
{
    /* get the old variable entry        */
    RexxVariable *old_variable = object_dictionary->getVariable(variableName);
    /* set the entry in the new table    */
    context->putLocalVariable(old_variable, index);
}

/**
 * Return the name of this variable.
 *
 * @return The string value of the variable name.
 */
RexxString *RexxParseVariable::getName()
{
    return variableName;
}


void *RexxParseVariable::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a REXX variable translator object                        */
/******************************************************************************/
{
    return new_object(size, T_VariableTerm);        /* Get new object                    */
}

