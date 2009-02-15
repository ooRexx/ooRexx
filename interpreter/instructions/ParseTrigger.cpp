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
/* REXX Translator                                          ParseTrigger.cpp  */
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
    int        type,                    /* type of trigger                   */
    RexxObject *_value,                 /* value to evaluatate               */
    size_t      _variableCount,         /* count of variables                */
    RexxQueue  *_variables)             /* array of trigger variables        */
/******************************************************************************/
/* Function:  Initialize a parse trigger translator object                    */
/******************************************************************************/
{
    this->setType(type);                 /* set the type (and hashvalue)      */
    this->variableCount = _variableCount; /* set the number of variables also  */
    OrefSet(this, this->value, _value);   /* save the associated value         */
    /* loop through the variable list    */
    while (_variableCount > 0)            /* copying each variable             */
    {
        OrefSet(this, this->variables[--_variableCount], (RexxVariableBase *)_variables->pop());
    }
}


stringsize_t RexxTrigger::integerTrigger(
    RexxObject *trigger)               /* value to be converted             */
/******************************************************************************/
/* Function:  Convert a trigger value to an integer, with appopriate error    */
/*            reporting.                                                      */
/******************************************************************************/
{
    stringsize_t result;                 /* converted result                  */
                                         /* convert the value                 */
    if (!trigger->requestUnsignedNumber(result, number_digits()))
    {
        /* report an exception               */
        reportException(Error_Invalid_whole_number_parse, trigger);
    }
    return result;                       /* finished                          */
}


RexxString *RexxTrigger::stringTrigger(
    RexxObject *trigger)               /* value to be converted             */
/******************************************************************************/
/* Function:  Convert a trigger expression to a String, with appopriate error */
/*            reporting.                                                      */
/******************************************************************************/
{
                                       /* force to string form              */
  return REQUEST_STRING(trigger);
}


void RexxTrigger::parse(
    RexxActivation      *context,      /* current execution context         */
    RexxExpressionStack *stack,        /* current expression stack          */
    RexxTarget          *target )      /* current parsing target string     */
/******************************************************************************/
/* Function:  Apply a parsing trigger against a parsing target                */
/******************************************************************************/
{
    RexxObject       *_value = OREF_NULL;/* evaluated trigger part            */
    RexxString       *stringvalue;       /* new string value                  */
    stringsize_t      integer;           /* target integer value              */
    size_t            i;                 /* loop counter                      */
    size_t            size;              /* size of variables array           */
    RexxVariableBase *variable;          /* current variable processing       */

    if (this->value != OREF_NULL)
    {      /* need a value processed?           */
           /* evaluate the expression part      */
        _value = this->value->evaluate(context, stack);
        context->traceResult(_value);      /* trace if necessary                */
        stack->pop();                      /* Get rid of the value off the stack*/
    }
    switch (this->getType())
    {           /* perform the trigger operations    */

        case TRIGGER_END:                  /* just match to the end             */
            target->moveToEnd();             /* move the pointers                 */
            break;

        case TRIGGER_PLUS:                 /* positive relative target          */
            integer = this->integerTrigger(_value);  /* get binary version of trigger     */
            target->forward(integer);        /* move the position                 */
            break;

        case TRIGGER_MINUS:                /* negative relative target          */
            integer = this->integerTrigger(_value);  /* get binary version of trigger     */
            target->backward(integer);       /* move the position                 */
            break;

        case TRIGGER_PLUS_LENGTH:          /* positive length                   */
            integer = this->integerTrigger(_value);  /* get binary version of trigger     */
            target->forwardLength(integer);  /* move the position                 */
            break;

        case TRIGGER_MINUS_LENGTH:         /* negative relative target          */
            integer = this->integerTrigger(_value);  /* get binary version of trigger     */
            target->backwardLength(integer); /* move the position                 */
            break;

        case TRIGGER_ABSOLUTE:             /* absolute column position          */
            integer = this->integerTrigger(_value);  /* get binary version of trigger     */
            target->absolute(integer);       /* move the position                 */
            break;

        case TRIGGER_STRING:               /* string search                     */
            /* force to string form              */
            stringvalue = this->stringTrigger(_value);
            target->search(stringvalue);     /* perform the search                */
            break;

        case TRIGGER_MIXED:                /* string search                     */
            /* force to string form              */
            stringvalue = this->stringTrigger(_value);
            /* and go search                     */
            target->caselessSearch(stringvalue);
            break;
    }
    if (context->tracingResults())
    {     /* are we tracing?                   */
          /* loop through the entire list      */
        for (i = 0, size = this->variableCount; i < size; i++)
        {
            if (i + 1 == size)               /* last variable?                    */
            {
                _value = target->remainder();  /* extract the remainder             */
            }
            else
            {
                _value = target->getWord();    /* just get the next word            */
            }
            variable = this->variables[i];   /* get the next variable retriever   */
            if (variable != OREF_NULL)
            {     /* not a place holder dummy?         */
                  /* set the value                     */
                // NOTE:  The different variable tpes handle their own assignment tracing
                variable->assign(context, stack, _value);
            }
            else                             /* dummy variable, just trace it     */
            {
                /* trace if necessary                */
                context->traceIntermediate(_value, TRACE_PREFIX_DUMMY);
            }
        }
    }
    else
    {                               /* not tracing, can optimize         */
                                    /* loop through the entire list      */
        for (i = 0, size = this->variableCount; i < size; i++)
        {
            variable = this->variables[i];   /* get the next variable retriever   */
            if (variable != OREF_NULL)
            {     /* not a place holder dummy?         */
                if (i + 1 == size)             /* last variable?                    */
                {
                    _value = target->remainder(); /* extract the remainder             */
                }
                else
                {
                    _value = target->getWord();   /* just get the next word            */
                }
                /* set the value                     */
                variable->assign(context, stack, _value);
            }
            else
            {                           /* dummy variable, just skip it      */
                if (i + 1 == size)              /* last variable?                    */
                {
                    target->skipRemainder();     /* skip the remainder                */
                }
                else
                {
                    target->skipWord();          /* just skip the next word           */
                }
            }
        }
    }
}

void RexxTrigger::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    size_t  i;                           /* loop counter                      */
    size_t  count;                       /* argument count                    */

    for (i = 0, count = this->variableCount; i < count; i++)
    {
        memory_mark(this->variables[i]);
    }
    memory_mark(this->value);
}

void RexxTrigger::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    size_t  i;                           /* loop counter                      */
    size_t  count;                       /* argument count                    */

    for (i = 0, count = this->variableCount; i < count; i++)
    {
        memory_mark_general(this->variables[i]);
    }
    memory_mark_general(this->value);
}

void RexxTrigger::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
    size_t  i;                           /* loop counter                      */
    size_t  count;                       /* argument count                    */

    setUpFlatten(RexxTrigger)            /* set up for the flatten            */

    flatten_reference(newThis->value, envelope);
    for (i = 0, count = this->variableCount; i < count; i++)
    {
        flatten_reference(newThis->variables[i], envelope);
    }

    cleanUpFlatten
}

void  *RexxTrigger::operator new(size_t size,
    int    variableCount)              /* list of variables                 */
/******************************************************************************/
/* Function:  Create a new parsing trigger object                             */
/******************************************************************************/
{
    /* Get new object                    */
    return new_object(size + (variableCount - 1) * sizeof(RexxObject *), T_ParseTrigger);
}

