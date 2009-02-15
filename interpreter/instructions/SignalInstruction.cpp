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
/* Primitive Signal Parse Class                                               */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "DirectoryClass.hpp"
#include "SignalInstruction.hpp"

RexxInstructionSignal::RexxInstructionSignal(
    RexxObject *_expression,            /* expression for signal value       */
    RexxString *_condition,             /* signalled condition               */
    RexxString *_name,                  /* signal target name                */
    size_t flags )                      /* option flags                      */
/******************************************************************************/
/* Initialize a SIGNAL instruction                                            */
/******************************************************************************/
{
                                       /* save all appropriate info         */
  OrefSet(this, this->expression, _expression);
  OrefSet(this, this->condition, _condition);
  OrefSet(this, this->name, _name);
  instructionFlags = (uint16_t)flags;
}

void RexxInstructionSignal::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->target);
  memory_mark(this->name);
  memory_mark(this->condition);
  memory_mark(this->expression);
}

void RexxInstructionSignal::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->target);
  memory_mark_general(this->name);
  memory_mark_general(this->condition);
  memory_mark_general(this->expression);
}

void RexxInstructionSignal::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionSignal)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->target, envelope);
  flatten_reference(newThis->name, envelope);
  flatten_reference(newThis->condition, envelope);
  flatten_reference(newThis->expression, envelope);

  cleanUpFlatten
}

void RexxInstructionSignal::resolve(
    RexxDirectory *labels)             /* table of program labels           */
/******************************************************************************/
/* Function:  Resolve a SIGNAL instruction label target                       */
/******************************************************************************/
{
    if (this->name == OREF_NULL)         /* not a name target form?           */
    {
        return;                            /* just return                       */
    }
                                           /* have a labels table?              */
    if (labels != OREF_NULL && this->name != OREF_NULL)
    {
        /* just get this from the table      */
        OrefSet(this, this->target, (RexxInstruction *)labels->at(this->name));
    }
}

void RexxInstructionSignal::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/******************************************************************************/
/* Function:  Execute a REXX SIGNAL instruction                               */
/******************************************************************************/
{
    RexxObject *result;                  /* evaluated expression              */
    RexxString *stringResult;            /* string version of the result      */

    context->traceInstruction(this);     /* trace if necessary                */
    if (this->condition != OREF_NULL)  /* is this the ON/OFF form?          */
    {
        if (instructionFlags&signal_on)    /* ON form?                          */
        {
                                           /* turn on the trap                  */
            context->trapOn(this->condition, (RexxInstructionCallBase *)this);
        }
        else
        {
            /* turn off the trap                 */
            context->trapOff(this->condition);
        }
        context->pauseInstruction();       /* do debug pause if necessary       */
    }
    else                               /* a normal signal?                  */
    {
        if (this->expression == OREF_NULL)/* already have the target?          */
        {
            if (this->target == OREF_NULL)   /* unknown target?                   */
            {
                reportException(Error_Label_not_found_name, this->name);
            }
            /* tell the activation to perform    */
            context->signalTo(this->target); /* the signal                        */
        }
        else                             /* need to evaluate an expression    */
        {
            /* get the expression value          */
            result = this->expression->evaluate(context, stack);
            /* force to a string value           */
            stringResult = REQUEST_STRING(result);
            context->traceResult(result);    /* trace if necessary                */
                                             /* tell the activation to perform    */
                                             /* the signal                        */
            context->signalValue(stringResult);
        }
    }
}

void RexxInstructionSignal::trap(
    RexxActivation *context,           /* current execution context         */
    RexxDirectory  *conditionObj)      /* associated condition object       */
/******************************************************************************/
/* Function:  Process a SIGNAL ON trap                                        */
/******************************************************************************/
{
    context->trapOff(this->condition);   /* turn off the trap                 */
    if (this->target == OREF_NULL)       /* unknown target?                   */
    {
        reportException(Error_Label_not_found_name, this->name);
    }
    /* set the new condition object      */
    context->setConditionObj(conditionObj);
    /* tell the activation to perform    */
    context->signalTo(this->target);     /* the signal                        */
}

