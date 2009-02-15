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
/* REXX Translator                            IndirectVariableReference.cpp   */
/*                                                                            */
/* Primitive Translator Expression Parsing Variable Indirect Reference Class  */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "RexxVariableDictionary.hpp"
#include "IndirectVariableReference.hpp"

RexxVariableReference::RexxVariableReference(
     RexxVariableBase *variable)       /* variable retriever for reference  */
/******************************************************************************/
/* Function:  Complete initialization of a variable reference object          */
/******************************************************************************/
{
                                       /* set the name value                */
  OrefSet(this, this->variableObject, variable);
}

void RexxVariableReference::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->variableObject);
}

void RexxVariableReference::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->variableObject);
}

void RexxVariableReference::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxVariableReference)

  flatten_reference(newThis->variableObject, envelope);

  cleanUpFlatten
}

RexxList *RexxVariableReference::list(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/******************************************************************************/
/* Function:  Parse a variables contents into an array of "good" variable     */
/*            retrievers.                                                     */
/******************************************************************************/
{
                                         /* get the variable value            */
    RexxObject *value = this->variableObject->evaluate(context, stack);
    stack->toss();                       /* remove the stack item             */
    RexxString *name_string = REQUEST_STRING(value); /* force to string form              */
    stack->push(name_string);            /* protect this on the stack         */
    RexxList *name_list = new_list();    /* create a new list item            */
    stack->push(name_list);              /* protect this also                 */
    size_t i = 1;                        /* start with the first word         */
                                         /* get the next variable             */
    RexxString *variable_name = (RexxString *)name_string->word(new_integer(i));
    i++;                                 /* step the index                    */
    while (variable_name->getLength() != 0)
    {
        /* get the first character           */
        int character = variable_name->getChar(0);
        if (character == '.')              /* start with a period?              */
        {
                                           /* report that error                 */
            reportException(Error_Invalid_variable_period, variable_name);
        }
        /* how about a digit?                */
        else if (character >= '0' && character <= '9')
        {
            /* constant symbol                   */
            reportException(Error_Invalid_variable_number, variable_name);
        }
        /* convert into a variable reference */
        RexxVariableBase *retriever = RexxVariableDictionary::getVariableRetriever(variable_name);
        if (retriever == OREF_NULL)        /* not converted ok?                 */
        {
            reportException(Error_Symbol_expected_expose);
        }
        /* add to the processing list        */
        name_list->addLast((RexxObject *)retriever);
        /* get the next variable             */
        variable_name = (RexxString *)name_string->word(new_integer(i));
        i++;                               /* and step the index                */
    }
    return name_list;                    /* return the list directly          */
}

void RexxVariableReference::drop(
  RexxActivation *context)             /* the context we're dropping in     */
/******************************************************************************/
/* Function:  Drop a subsidiary list of variables                             */
/******************************************************************************/
{
    RexxExpressionStack *stack = context->getStack();         /* get the stack from the context    */
    /* evaluate into a variable list     */
    RexxList *name_list = this->list(context, stack);
    /* get the first list item           */
    RexxVariableBase *variable = (RexxVariableBase *)name_list->removeFirst();
    /* while more list items             */
    while (variable != (RexxVariableBase *)TheNilObject)
    {
        variable->drop(context);           /* drop the this variable            */
                                           /* get the next list item            */
        variable = (RexxVariableBase *)name_list->removeFirst();
    }
}


void RexxVariableReference::procedureExpose(
  RexxActivation      *context,        /* current activation context        */
  RexxActivation      *parent,         /* the parent activation context     */
  RexxExpressionStack *stack)          /* current evaluation stack          */
/******************************************************************************/
/* Function:  Expose a subsidiary list of variables                           */
/******************************************************************************/
{
    /* expose the variable first         */
    variableObject->procedureExpose(context, parent, stack);
    /* evaluate into a variable list     */
    RexxList *name_list = this->list(context, stack);
    /* get the first list item           */
    RexxVariableBase *variable = (RexxVariableBase *)name_list->removeFirst();
    /* while more list items             */
    while (variable != (RexxVariableBase *)TheNilObject)
    {
        /* expose this variable              */
        variable->procedureExpose(context, parent, stack);
        /* get the next list item            */
        variable = (RexxVariableBase *)name_list->removeFirst();
    }
}


void RexxVariableReference::expose(
  RexxActivation      *context,        /* current activation context        */
  RexxExpressionStack *stack,          /* current evaluation stack          */
                                       /* variable scope we're exposing from*/
  RexxVariableDictionary *object_dictionary)
/******************************************************************************/
/* Function:  Expose a subsidiary list of variables                           */
/******************************************************************************/
{
    /* expose the variable first         */
    variableObject->expose(context, stack, object_dictionary);
    /* evaluate into a variable list     */
    RexxList *name_list = this->list(context, stack);
    /* get the first list item           */
    RexxVariableBase *variable = (RexxVariableBase *)name_list->removeFirst();
    /* while more list items             */
    while (variable != (RexxVariableBase *)TheNilObject)
    {
        /* expose this variable              */
        variable->expose(context, stack, object_dictionary);
        /* get the next list item            */
        variable = (RexxVariableBase *)name_list->removeFirst();
    }
}


void *RexxVariableReference::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
                                       /* Get new object                        */
    return new_object(size, T_IndirectVariableTerm);
}

