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
/* REXX Translator                                                            */
/*                                                                            */
/* Primitive Forward Translator Class                                         */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxVariableDictionary.hpp"
#include "RexxActivation.hpp"
#include "ForwardInstruction.hpp"

void RexxInstructionForward::live(size_t liveMark)
/******************************************************************************/
/* Function:  Process live calls                                              */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->target);
  memory_mark(this->message);
  memory_mark(this->superClass);
  memory_mark(this->arguments);
  memory_mark(this->array);
}

void RexxInstructionForward::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Process general live calls                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->target);
  memory_mark_general(this->message);
  memory_mark_general(this->superClass);
  memory_mark_general(this->arguments);
  memory_mark_general(this->array);
}

void RexxInstructionForward::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an instruction object                                   */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionForward)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->target, envelope);
  flatten_reference(newThis->message, envelope);
  flatten_reference(newThis->superClass, envelope);
  flatten_reference(newThis->arguments, envelope);
  flatten_reference(newThis->array, envelope);

  cleanUpFlatten
}

void RexxInstructionForward::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/******************************************************************************/
/* Function:  Execute a forward instruction                                   */
/******************************************************************************/
{
    RexxObject *_target;                  /* evaluated target                  */
    RexxString *_message;                 /* evaluated message                 */
    RexxObject *_superClass;              /* evaluated super class             */
    RexxObject *result;                  /* message result                    */
    RexxObject *temp;                    /* temporary object                  */
    size_t      count = 0;               /* count of array expressions        */
    size_t      i;                       /* loop counter                      */
    RexxObject **_arguments;

    context->traceInstruction(this);     /* trace if necessary                */
    if (!context->inMethod())            /* is this a method clause?          */
    {
                                         /* raise an error                    */
        reportException(Error_Execution_forward);
    }
    _target = OREF_NULL;                  /* no object yet                     */
    _message = OREF_NULL;                 /* no message over ride              */
    _superClass = OREF_NULL;              /* no super class over ride          */
    _arguments = OREF_NULL;               /* no argument over ride             */

    if (this->target != OREF_NULL)       /* sent to a different object?       */
    {
                                         /* get the expression value          */
        _target = this->target->evaluate(context, stack);
    }
    if (this->message != OREF_NULL)    /* sending a different message?      */
    {
        /* get the expression value          */
        temp = this->message->evaluate(context, stack);
        _message = REQUEST_STRING(temp);    /* get the string version            */
        _message = _message->upper();       /* and force to uppercase            */
    }
    if (this->superClass != OREF_NULL)   /* overriding the super class?       */
    {
                                         /* get the expression value          */
        _superClass = this->superClass->evaluate(context, stack);
    }
    if (this->arguments != OREF_NULL)  /* overriding the arguments?         */
    {
        /* get the expression value          */
        temp = this->arguments->evaluate(context, stack);
        /* get an array version              */
        RexxArray *argArray = REQUEST_ARRAY(temp);
        stack->push(argArray);           /* protect this on the stack         */
        /* not an array item or a multiple   */
        /* dimension one?                    */
        if (argArray == TheNilObject || argArray->getDimension() != 1)
        {
            /* this is an error                  */
            reportException(Error_Execution_forward_arguments);
        }
        count = argArray->size();          /* get the size                      */
                                           /* omitted trailing arguments?       */
        if (count != 0 && argArray->get(count) == OREF_NULL)
        {
            count--;                         /* decrement the count               */
            while (count > 0)              /* loop down to first full one       */
            {
                /* find a real argument              */
                if (argArray->get(count) != OREF_NULL)
                {
                    break;                       /* break out of here                 */
                }
                count--;                       /* step back the count               */
            }
        }
        _arguments = argArray->data();    /* point directly to the argument data */
    }
    if (this->array != OREF_NULL)      /* have an array of extra info?      */
    {
        count = this->array->size();       /* get the expression count          */
        for (i = 1; i <= count; i++)     /* loop through the expression list  */
        {
            RexxObject *argElement = this->array->get(i);
            /* real argument?                    */
            if (argElement != OREF_NULL)
            {
                /* evaluate the expression           */
                argElement->evaluate(context, stack);
            }
            else
            {
                /* just push a null reference for the missing ones */
                stack->push(OREF_NULL);
            }
        }
        /* now point at the stacked values */
        _arguments = stack->arguments(count);
    }
    /* go forward this                   */
    result = context->forward(_target, _message, _superClass, _arguments, count, instructionFlags&forward_continue);
    if (instructionFlags&forward_continue)  /* not exiting?                      */
    {
        if (result != OREF_NULL)         /* result returned?                  */
        {
            context->traceResult(result);    /* trace if necessary                */
                                             /* set the RESULT variable to the    */
                                             /* message return value              */
            context->setLocalVariable(OREF_RESULT, VARIABLE_RESULT, result);
        }
        else                               /* drop the variable RESULT          */
        {
            context->dropLocalVariable(OREF_RESULT, VARIABLE_RESULT);
        }
        context->pauseInstruction();       /* do debug pause if necessary       */
    }
}

