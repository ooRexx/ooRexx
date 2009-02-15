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
/* REXX Translator                                       ExpressionStem.cpp   */
/*                                                                            */
/* Primitive Translator Expression Parsing Stem Reference Class               */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/

#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "RexxVariable.hpp"
#include "RexxVariableDictionary.hpp"
#include "StemClass.hpp"
#include "ExpressionStem.hpp"

RexxStemVariable::RexxStemVariable(
     RexxString * stemName,            /* stem name to access               */
     size_t       var_index)           /* lookaside index for stem          */
/******************************************************************************/
/* Function:  Initialize a translator STEM object                             */
/******************************************************************************/
{
                                       /* set the name                      */
  OrefSet(this, this->stem, stemName); /* set the name                      */
  this->index = var_index;             /* and the index                     */
}

void RexxStemVariable::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->stem);
}

void RexxStemVariable::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->stem);
}

void RexxStemVariable::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxStemVariable)

  flatten_reference(newThis->stem, envelope);

  cleanUpFlatten
}

RexxObject  *RexxStemVariable::evaluate(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/******************************************************************************/
/* Function:  Evaluate a REXX stem variable                                   */
/******************************************************************************/
{
    /* look up the name                  */
    RexxObject *value = context->getLocalStem(this->stem, this->index);
    /* NOTE:  stem accesses do NOT       */
    /* report NOVALUE so that DO OVER,   */
    /* call-by-reference with a stem and */
    /* return with a stem does not       */
    /* trigger a novalue trap            */
    /* unexpectedly                      */
    stack->push(value);                  /* place on the evaluation stack     */
                                         /* trace if necessary                */
    context->traceVariable(stem, value);
    return value;                        /* return the located variable       */
}

RexxObject  *RexxStemVariable::getValue(
  RexxVariableDictionary *dictionary)  /* current activation dictionary     */
/******************************************************************************/
/* Function:  retrieve a stem variable's value (notready condition will       */
/*            not be raised)                                                  */
/******************************************************************************/
{
                                       /* look up the name                  */
    return dictionary->getStem(this->stem);
}

RexxObject  *RexxStemVariable::getValue(
  RexxActivation *context)             /* current activation context        */
/******************************************************************************/
/* Function:  retrieve a stem variable's value (notready condition will       */
/*            not be raised)                                                  */
/******************************************************************************/
{
                                       /* look up the name                  */
    return context->getLocalStem(stem, index);
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
RexxObject  *RexxStemVariable::getRealValue(RexxVariableDictionary *dictionary)
{
    return dictionary->getStem(this->stem);
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
    return context->getLocalStem(stem, index);
}

void RexxStemVariable::set(
  RexxActivation *context,             /* current activation context        */
  RexxObject *value )                  /* new value to be assigned          */
/******************************************************************************/
/* Function:  Fast set of a stem variable value                               */
/******************************************************************************/
{
    /* look up the name                  */
    RexxVariable *variable = context->getLocalStemVariable(stem, index);
    if (isOfClass(Stem, value))
    {            /* stem to stem assignment           */
        variable->set(value);              /* overlay the reference stem object */
    }
    else
    {
        /* create a new stem object as value */
        RexxStem *stem_table = new RexxStem (this->stem);
        variable->set(stem_table);         /* overlay the reference stem object */
        stem_table->setValue(value);       /* set the default value             */
    }
}


void RexxStemVariable::set(
  RexxVariableDictionary  *dictionary, /* current activation dictionary     */
  RexxObject *value )                  /* new value to be assigned          */
/******************************************************************************/
/* Function:  Fast set of a stem variable value                               */
/******************************************************************************/
{
    /* look up the name                  */
    RexxVariable *variable = dictionary->getStemVariable(this->stem);
    if (isOfClass(Stem, value))
    {            /* stem to stem assignment           */
        variable->set(value);              /* overlay the reference stem object */
    }
    else
    {
        /* create a new stem object as value */
        RexxStem *stem_table = new RexxStem (this->stem);
        variable->set(stem_table);         /* overlay the reference stem object */
        stem_table->setValue(value);       /* set the default value             */
    }
}


bool RexxStemVariable::exists(
  RexxActivation *context)             /* current activation context        */
/******************************************************************************/
/*  Function:  Check the existance of a REXX stem variable                    */
/******************************************************************************/
{
                                       /* retrieve the variable value       */
    return context->localStemVariableExists(stem, index);
}

void RexxStemVariable::assign(
    RexxActivation *context,           /* current activation context        */
    RexxExpressionStack *stack,        /* current evaluation stack          */
    RexxObject     *value )            /* new value to assign               */
/******************************************************************************/
/* Function:  Assign a value to a stem variable                               */
/******************************************************************************/
{
    RexxVariable *variable = context->getLocalStemVariable(stem, index);
    if (isOfClass(Stem, value))
    {            /* stem to stem assignment           */
        variable->set(value);              /* overlay the reference stem object */
    }
    else
    {
        /* create a new stem object as value */
        RexxStem *stem_table = new RexxStem (this->stem);
        variable->set(stem_table);         /* overlay the reference stem object */
        stem_table->setValue(value);       /* set the default value             */
    }
    // trace the assignment
    context->traceAssignment(stem, value);
}

void RexxStemVariable::drop(
  RexxActivation *context)             /* target variable dictionary        */
/******************************************************************************/
/* Function:  Drop a variable object                                          */
/******************************************************************************/
{
    /* drop the stem value */
    context->dropLocalStem(stem, index);
}

/**
 * Drop a variable that's directly in a variable dictionary.
 *
 * @param dictionary The target dictionary
 */
void RexxStemVariable::drop(RexxVariableDictionary *dictionary)
{
    // dropping the stem name is sufficient
    dictionary->dropStemVariable(stem);
}

void RexxStemVariable::procedureExpose(
  RexxActivation      *context,        /* current activation context        */
  RexxActivation      *parent,         /* the parent activation context     */
  RexxExpressionStack *stack)          /* current evaluation stack          */
/******************************************************************************/
/* Function:  Expose a stem variable                                          */
/******************************************************************************/
{
    /* get the old variable entry        */
    RexxVariable *old_variable = parent->getLocalStemVariable(stem, index);

    /* set the entry in the new table    */
    if (index == 0)
    {
        context->updateLocalVariable(old_variable);
    }
    else
    {
        context->putLocalVariable(old_variable, index);
    }
}


void RexxStemVariable::expose(
  RexxActivation      *context,        /* current activation context        */
  RexxExpressionStack *stack,          /* current evaluation stack          */
                                       /* variable scope we're exposing from*/
  RexxVariableDictionary *object_dictionary)
/******************************************************************************/
/* Function:  Expose a stem variable                                          */
/******************************************************************************/
{
                                         /* get the old variable entry        */
    RexxVariable *old_stem = object_dictionary->getStemVariable(stem);
                                         /* set the entry in the new table    */
    context->putLocalVariable(old_stem, index);
}


void RexxStemVariable::setGuard(
  RexxActivation *context )            /* current activation context        */
/******************************************************************************/
/* Set a guard variable notification on a stem variable                       */
/******************************************************************************/
{
                                       /* look up the name                  */
    RexxVariable *variable = context->getLocalStemVariable(this->stem, this->index);
    variable->inform(ActivityManager::currentActivity);   /* mark the variable entry           */
}

void RexxStemVariable::clearGuard(
  RexxActivation *context )            /* current activation context        */
/******************************************************************************/
/* Remove a guard variable notification on an object variable                 */
/******************************************************************************/
{
                                       /* look up the name                  */
    RexxVariable *variable = context->getLocalStemVariable(this->stem, this->index);
    variable->uninform(ActivityManager::currentActivity); /* mark the variable entry           */
}


bool RexxStemVariable::sort(
    RexxActivation *context, RexxString *prefix, int order, int type, size_t start,
    size_t end, size_t firstcol, size_t lastcol)
/******************************************************************************/
/* Sort the elements of a stem variable as if they were an array.             */
/******************************************************************************/
{
    /* get the stem object */
    RexxStem *stem_table = context->getLocalStem(stem, index);
    /* the stem object handles the sorting. */
    return stem_table->sort(prefix, order, type, start, end, firstcol, lastcol);
}

void *RexxStemVariable::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
                                         /* Get new object                    */
    return new_object(size, T_StemVariableTerm);
}

