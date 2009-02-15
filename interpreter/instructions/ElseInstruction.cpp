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
/* Primitive Else Parse Class                                                 */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "ElseInstruction.hpp"
#include "IfInstruction.hpp"
#include "Token.hpp"

RexxInstructionElse::RexxInstructionElse(
    RexxToken  *token)                 /* actual ELSE token                 */
/******************************************************************************/
/* Function:  Complete ELSE instruction initialization                        */
/******************************************************************************/
{
  SourceLocation location;             /* clause token location             */

  location = token->getLocation();     /* get the token location info       */
  this->setLocation(location);         /* set the clause location also      */
}

void  RexxInstructionElse::setParent(
    RexxInstructionEndIf *ifthen)      /* link back to associated then      */
/******************************************************************************/
/* Function:  Hook an ELSE instruction back to its parent IF                  */
/******************************************************************************/
{
                                       /* link up with the THEN             */
  OrefSet(this, this->parent, (RexxInstructionIf *)ifthen);
}

void  RexxInstructionElse::setEndInstruction(
    RexxInstructionEndIf *end_clause)
/******************************************************************************/
/* Function:  complete links between the IF, THEN, and ELSE constructs now    */
/*            that the full scoping has been resolved.                        */
/******************************************************************************/
{
                                       /* tell the THEN where to finish up  */
  this->parent->setEndInstruction(end_clause);
}

void RexxInstructionElse::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->parent);
}

void RexxInstructionElse::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->parent);
}

void RexxInstructionElse::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionElse)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->parent, envelope);

  cleanUpFlatten
}

void RexxInstructionElse::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/****************************************************************************/
/* Function:  Execute a REXX ELSE instruction                               */
/****************************************************************************/
{
  context->indent();                   /* indent a bit                      */
  context->traceInstruction(this);     /* trace if necessary                */
  context->indent();                   /* indent for the next one too       */
  return;                              /* just fall through to following    */
}

