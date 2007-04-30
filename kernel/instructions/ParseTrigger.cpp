/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* REXX Translator                                              ParseTrigger.c    */
/*                                                                            */
/* Primitive Procedure Parse Trigger Class                                    */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"
#include "ParseTrigger.hpp"
#include "ParseTarget.hpp"
#include "ExpressionBaseVariable.hpp"

RexxTrigger::RexxTrigger(
    INT        type,                   /* type of trigger                   */
    RexxObject *value,                 /* value to evaluatate               */
    LONG        variableCount,         /* count of variables                */
    RexxQueue  *variables)             /* array of trigger variables        */
/******************************************************************************/
/* Function:  Initialize a parse trigger translator object                    */
/******************************************************************************/
{
  this->setType(type);                 /* set the type (and hashvalue)      */
  this->variableCount = variableCount; /* set the number of variables also  */
  OrefSet(this, this->value, value);   /* save the associated value         */
                                       /* loop through the variable list    */
  while (variableCount > 0)            /* copying each variable             */
    OrefSet(this, this->variables[--variableCount], (RexxVariableBase *)variables->pop());
}


long RexxTrigger::integerTrigger(
    RexxObject *value)                 /* value to be converted             */
/******************************************************************************/
/* Function:  Convert a trigger value to an integer, with appopriate error    */
/*            reporting.                                                      */
/******************************************************************************/
{
  LONG       result;                   /* converted result                  */
                                       /* convert the value                 */
  result = REQUEST_LONG(value, NO_LONG);
  if (result == NO_LONG || result < 0) /* bad value or negative?            */
                                       /* report an exception               */
    report_exception1(Error_Invalid_whole_number_parse, value);
  return result;                       /* finished                          */
}


RexxString *RexxTrigger::stringTrigger(
    RexxObject *value)                 /* value to be converted             */
/******************************************************************************/
/* Function:  Convert a trigger expression to a String, with appopriate error */
/*            reporting.                                                      */
/******************************************************************************/
{
                                       /* force to string form              */
  return REQUEST_STRING(value);
}


void RexxTrigger::parse(
    RexxActivation      *context,      /* current execution context         */
    RexxExpressionStack *stack,        /* current expression stack          */
    RexxTarget          *target )      /* current parsing target string     */
/******************************************************************************/
/* Function:  Apply a parsing trigger against a parsing target                */
/******************************************************************************/
{
  RexxObject       *value;             /* evaluated trigger part            */
  RexxString       *stringvalue;       /* new string value                  */
  LONG              integer;           /* target integer value              */
  INT               i;                 /* loop counter                      */
  INT               size;              /* size of variables array           */
  RexxVariableBase *variable;          /* current variable processing       */

  if (this->value != OREF_NULL) {      /* need a value processed?           */
                                       /* evaluate the expression part      */
    value = this->value->evaluate(context, stack);
    context->traceResult(value);       /* trace if necessary                */
    stack->pop();                      /* Get rid of the value off the stack*/
  }
  switch (this->getType()) {           /* perform the trigger operations    */

    case TRIGGER_END:                  /* just match to the end             */
      target->moveToEnd();             /* move the pointers                 */
      break;

    case TRIGGER_PLUS:                 /* positive relative target          */
      integer = this->integerTrigger(value);  /* get binary version of trigger     */
      target->forward(integer);        /* move the position                 */
      break;

    case TRIGGER_MINUS:                /* negative relative target          */
      integer = this->integerTrigger(value);  /* get binary version of trigger     */
      target->backward(integer);       /* move the position                 */
      break;

    case TRIGGER_PLUS_LENGTH:          /* positive length                   */
      integer = this->integerTrigger(value);  /* get binary version of trigger     */
      target->forwardLength(integer);  /* move the position                 */
      break;

    case TRIGGER_MINUS_LENGTH:         /* negative relative target          */
      integer = this->integerTrigger(value);  /* get binary version of trigger     */
      target->backwardLength(integer); /* move the position                 */
      break;

    case TRIGGER_ABSOLUTE:             /* absolute column position          */
      integer = this->integerTrigger(value);  /* get binary version of trigger     */
      target->absolute(integer);       /* move the position                 */
      break;

    case TRIGGER_STRING:               /* string search                     */
                                       /* force to string form              */
      stringvalue = this->stringTrigger(value);
      target->search(stringvalue);     /* perform the search                */
      break;

    case TRIGGER_MIXED:                /* string search                     */
                                       /* force to string form              */
      stringvalue = this->stringTrigger(value);
                                       /* and go search                     */
      target->caselessSearch(stringvalue);
      break;
  }
  if (context->tracingResults()) {     /* are we tracing?                   */
                                       /* loop through the entire list      */
    for (i = 0, size = this->variableCount - 1; i <= size; i++) {
      if (i == size)                   /* last variable?                    */
        value = target->remainder();   /* extract the remainder             */
      else
        value = target->getWord();     /* just get the next word            */
      variable = this->variables[i];   /* get the next variable retriever   */
      if (variable != OREF_NULL) {     /* not a place holder dummy?         */
                                       /* set the value                     */
        variable->assign(context, stack, value);
        context->traceResult(value);   /* trace if necessary                */
      }
      else                             /* dummy variable, just trace it     */
                                       /* trace if necessary                */
        context->traceIntermediate(value, TRACE_PREFIX_DUMMY);
    }
  }
  else {                               /* not tracing, can optimize         */
                                       /* loop through the entire list      */
    for (i = 0, size = this->variableCount - 1; i <= size; i++) {
      variable = this->variables[i];   /* get the next variable retriever   */
      if (variable != OREF_NULL) {     /* not a place holder dummy?         */
        if (i == size)                 /* last variable?                    */
          value = target->remainder(); /* extract the remainder             */
        else
          value = target->getWord();   /* just get the next word            */
                                       /* set the value                     */
        variable->assign(context, stack, value);
      }
      else {                           /* dummy variable, just skip it      */
        if (i == size)                 /* last variable?                    */
          target->skipRemainder();     /* skip the remainder                */
        else
          target->skipWord();          /* just skip the next word           */
      }
    }
  }
}

void RexxTrigger::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpMemoryMark
  for (i = 0, count = this->variableCount; i < count; i++)
    memory_mark(this->variables[i]);
  memory_mark(this->value);
  cleanUpMemoryMark
}

void RexxTrigger::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpMemoryMarkGeneral
  for (i = 0, count = this->variableCount; i < count; i++)
    memory_mark_general(this->variables[i]);
  memory_mark_general(this->value);
  cleanUpMemoryMarkGeneral
}

void RexxTrigger::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpFlatten(RexxTrigger)            /* set up for the flatten            */

    flatten_reference(newThis->value, envelope);
    for (i = 0, count = this->variableCount; i < count; i++)
      flatten_reference(newThis->variables[i], envelope);

  cleanUpFlatten
}

void  *RexxTrigger::operator new(size_t size,
    LONG  variableCount)               /* list of variables                 */
/******************************************************************************/
/* Function:  Create a new parsing trigger object                             */
/******************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */

                                       /* Get new object                    */
  newObject = new_object(size + (variableCount - 1) * sizeof(RexxObject *));
                                       /* Give new object its behaviour     */
  BehaviourSet(newObject, TheParseTriggerBehaviour);
  ClearObject(newObject);              /* initialize the object             */
  return newObject;                    /* return the new trigger            */
}

