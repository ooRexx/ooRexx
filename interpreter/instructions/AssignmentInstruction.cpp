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
/* Primitive Assignment Parse Class                                           */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "ExpressionBaseVariable.hpp"
#include "RexxActivation.hpp"
#include "AssignmentInstruction.hpp"

RexxInstructionAssignment::RexxInstructionAssignment(
    RexxVariableBase *target,          /* target variable instruction       */
    RexxObject       *_expression)      /* assigned expression value         */
/******************************************************************************/
/* Function:  complete ASSIGNMENT instruction initialization                  */
/******************************************************************************/
{
                                       /* get the variable target           */
  OrefSet(this, this->variable, target);
                                       /* process the expression            */
  OrefSet(this, this->expression, _expression);
}

void RexxInstructionAssignment::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->variable);
  memory_mark(this->expression);
}

void RexxInstructionAssignment::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->variable);
  memory_mark_general(this->expression);
}

void RexxInstructionAssignment::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionAssignment)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->variable, envelope);
  flatten_reference(newThis->expression, envelope);

  cleanUpFlatten
}

void RexxInstructionAssignment::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/******************************************************************************/
/* Function:  Execute a REXX assignment instruction                           */
/*            NOTE:  This instruction is implemented using two seperate paths */
/*            for traced vs. non-traced execution.  This reduces the checks   */
/*            for non-traced execution to a single check in this very         */
/*            heavily executed instruction.                                   */
/******************************************************************************/
{
    if (context->tracingInstructions())/* tracing?                          */
    {
        context->traceInstruction(this);   /* trace if necessary                */
                                           /* get the expression value          */
        RexxObject *result = this->expression->evaluate(context, stack);
        context->traceResult(result);      /* trace if necessary                */
                                           /* do the assignment                 */
        this->variable->assign(context, stack, result);
        context->pauseInstruction();       /* do debug pause if necessary       */
    }
    else                                 /* non-traced execution              */
    {
                                         /* do the assignment                 */
        this->variable->assign(context, stack, this->expression->evaluate(context, stack));
    }
}

