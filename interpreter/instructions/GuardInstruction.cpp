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
/* Primitive Guard Parse Class                                                */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "ArrayClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "GuardInstruction.hpp"
#include "ExpressionBaseVariable.hpp"

RexxInstructionGuard::RexxInstructionGuard(
    RexxObject *_expression,            /* guard expression                  */
    RexxArray  *variable_list,         /* list of variables to trigger on   */
    bool        on_off)                /* ON or OFF form                    */
/******************************************************************************/
/* Function:  Complete initialization of a GUARD instruction object           */
/******************************************************************************/
{
    size_t i;                            /* loop counter                      */

                                         /* save the guard expression         */
    OrefSet(this, this->expression, _expression);
    if (on_off)                          /* this the ON form?                 */
    {
        instructionFlags |= guard_on_form; /* turn on the flag                  */
    }
    if (variable_list != OREF_NULL)    /* got a guard expression?           */
    {
        /* get the variable size             */
        variableCount = variable_list->size();
        /* loop through the variable list    */
        for (i = 1; i <= variableCount; i++)
        {
            /* copying each variable             */
            OrefSet(this, this->variables[i-1], (RexxVariableBase *)(variable_list->get(i)));
        }
    }
    else
    {
        variableCount = 0;                 /* no extra variables                */
    }
}

void RexxInstructionGuard::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/****************************************************************************/
/* Function:  Execute a REXX GUARD instruction                              */
/****************************************************************************/
{
    size_t      size;                    /* size of guard variables list      */
    size_t      i;                       /* loop counter                      */
    RexxObject *result;                  /* guard expression result           */

    context->traceInstruction(this);     /* trace if necessary                */
    if (!context->inMethod())            /* is this a method clause?          */
    {
                                         /* raise an error                    */
        reportException(Error_Translation_guard_guard);
    }
    /* non-expression form?              */
    else if (this->expression == OREF_NULL)
    {

        if (!(instructionFlags&guard_on_form))      /* is this the OFF form?             */
        {
            context->guardOff();             /* set unguarded status in activation*/
        }
        else
        {
            context->guardOn();              /* set guarded status in activation  */
        }
    }
    else
    {
        size = variableCount;              /* get variable list count           */
        for (i = 0; i < size; i++)       /* loop through the variable list    */
        {
            /* set a guard on each variable,     */
            /* counting the guards on each       */
            /* variable that is actually exposed */
            this->variables[i]->setGuard(context);
        }

        if (!(instructionFlags&guard_on_form)) /* is this the OFF form?             */
        {
            context->guardOff();             /* set unguarded status in activation*/
        }
        else
        {
            context->guardOn();              /* set guarded status in activation  */
        }

        ActivityManager::currentActivity->guardSet();       /* initialize the guard sem          */
        /* get the expression value          */
        result = this->expression->evaluate(context, stack);
        context->traceResult(result);      /* trace if necessary                */
                                           /* do first evaluation without       */
                                           /* establishing any variable guards  */
                                           /* false on first attempt?           */
        if (!result->truthValue(Error_Logical_value_guard))
        {
            do                             /* need to loop until true           */
            {
                stack->clear();                /* clear the expression stack        */
                context->guardWait();       /* establish guards and wait         */
                ActivityManager::currentActivity->guardSet();   /* initialize the guard sem          */
                result = this->expression->evaluate(context, stack);
                context->traceResult(result);  /* trace if necessary                */
                                               /* while this is still false         */
            } while (!result->truthValue(Error_Logical_value_guard));
        }
        for (i = 0; i < size; i++)       /* loop through the variable list    */
        {
            /* set a guard on each variable,     */
            /* counting the guards on each       */
            /* variable that is actually exposed */
            this->variables[i]->clearGuard(context);
        }
    }
}

void RexxInstructionGuard::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    size_t i;                            /* loop counter                      */
    size_t count;                        /* argument count                    */

    memory_mark(this->nextInstruction);  /* must be first one marked          */
    for (i = 0, count = variableCount; i < count; i++)
    {
        memory_mark(this->variables[i]);
    }
    memory_mark(this->expression);
}



void RexxInstructionGuard::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    size_t i;                            /* loop counter                      */
    size_t count;                        /* argument count                    */

                                         /* must be first one marked          */
    memory_mark_general(this->nextInstruction);
    memory_mark_general(this->expression);
    for (i = 0, count = variableCount; i < count; i++)
    {
        memory_mark_general(this->variables[i]);
    }
}

void RexxInstructionGuard::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
    size_t i;                            /* loop counter                      */
    size_t count;                        /* argument count                    */

    setUpFlatten(RexxInstructionGuard)

    flatten_reference(newThis->nextInstruction, envelope);
    flatten_reference(newThis->expression, envelope);
    for (i = 0, count = variableCount; i < count; i++)
    {
        flatten_reference(newThis->variables[i], envelope);
    }

    cleanUpFlatten
}

