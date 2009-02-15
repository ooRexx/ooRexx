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
/* Primitive If Parse Class                                                   */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "IfInstruction.hpp"
#include "Token.hpp"


RexxInstructionIf::RexxInstructionIf(
    RexxObject *_condition,            /* conditional expression            */
    RexxToken  *token)                 /* terminating THEN token            */
/******************************************************************************/
/* Function:  Complete IF instruction initialization                          */
/******************************************************************************/
{
  SourceLocation location;             /* clause token location             */

                                       /* save the condition                */
  OrefSet(this, this->condition, _condition);
  location = token->getLocation();     /* get the token location info       */
                                       /* update the end location           */
  this->setEnd(location.getLineNumber(), location.getOffset());
}

void RexxInstructionIf::setEndInstruction(
    RexxInstructionEndIf *end_target)  /* targer for the false branch       */
/******************************************************************************/
/* Function:  Set the location for the false branch of the IF instruction     */
/******************************************************************************/
{
                                       /* save the end location             */
  OrefSet(this, this->else_location, end_target);
}

void RexxInstructionIf::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->condition);
  memory_mark(this->else_location);
}


void RexxInstructionIf::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->condition);
  memory_mark_general(this->else_location);
}

void RexxInstructionIf::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionIf)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->condition, envelope);
  flatten_reference(newThis->else_location, envelope);

  cleanUpFlatten
}

void RexxInstructionIf::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/******************************************************************************/
/* Function:  Execute a REXX IF instruction                                   */
/******************************************************************************/
{
    context->traceInstruction(this);     /* trace if necessary                */
                                         /* get the expression value          */
    RexxObject *result = this->condition->evaluate(context, stack);
    context->traceResult(result);        /* trace if necessary                */

    /* the comparison methods return either .true or .false, so we */
    /* can to a quick test against those. */
    if (result == TheFalseObject)
    {
        /* we execute the ELSE branch        */
        context->setNext(this->else_location->nextInstruction);
    }
    /* if it is not the True object, we need to perform a fuller */
    /* evaluation of the result. */
    else if (result != TheTrueObject)
    {
        /* is the condition false?           */
        if (!result->truthValue(Error_Logical_value_if))
        {
            /* we execute the ELSE branch        */
            context->setNext(this->else_location->nextInstruction);
        }
    }
    context->pauseInstruction();         /* do debug pause if necessary       */
}

