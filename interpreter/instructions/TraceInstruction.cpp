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
/* Primitive Trace Parse Class                                                */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "TraceInstruction.hpp"
#include "SourceFile.hpp"


/**
 * Initialize a Trace instruction.
 *
 * @param _expression
 *                   A potential expression to evaluate.
 * @param trace      The new trace setting (can be zero if numeric or dynamic form).
 * @param flags      The translated trace flags for setting-based forms.
 * @param debug_skip A potential debug_skip value.
 */
RexxInstructionTrace::RexxInstructionTrace(RexxObject *_expression, size_t trace, size_t flags, wholenumber_t debug_skip )
{
                                       /* process the expression            */
   OrefSet(this, this->expression, _expression);
   this->debugskip = debug_skip;       /* copy the skip value               */
   traceSetting = trace;               /* and the trace setting             */
   traceFlags = flags;
}

void RexxInstructionTrace::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->expression);
}

void RexxInstructionTrace::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->expression);
}

void RexxInstructionTrace::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionTrace)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->expression, envelope);

  cleanUpFlatten
}

void RexxInstructionTrace::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/******************************************************************************/
/* Function:  Execute a REXX TRACE instruction                                */
/******************************************************************************/
{
    RexxObject  *result;                 /* expression result                 */
    RexxString  *value;                  /* string version of expression      */

    context->traceInstruction(this);     /* trace if necessary                */
    // is this a debug skip request (the setting value is zero in that case)
    if ((traceSetting&TRACE_SETTING_MASK) == 0)
    {
                                         /* turn on the skip mode             */
        context->debugSkip(this->debugskip, (traceSetting&DEBUG_NOTRACE) != 0);
    }
    /* non-dynamic form?                 */
    else if (this->expression == OREF_NULL)
    {
        if (!context->inDebug())           /* not in debug mode?                */
        {
                                           /* just change the setting           */
            context->setTrace(traceSetting, traceFlags);
        }
        else
        {
            context->pauseInstruction();     /* do debug pause if necessary       */
        }
    }
    else                               /* need to evaluate an expression    */
    {
        /* get the expression value          */
        result = this->expression->evaluate(context, stack);
        value = REQUEST_STRING(result);    /* force to string form              */
        context->traceResult(result);      /* trace if necessary                */
        if (!context->inDebug())           /* not in debug mode?                */
        {
                                           /* now change the setting            */
            context->setTrace(value);
        }
        else
        {
            context->pauseInstruction();     /* do debug pause if necessary       */
        }
    }
}

