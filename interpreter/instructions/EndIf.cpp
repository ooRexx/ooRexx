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
/* Primitive End-If Parse Class                                               */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "IfInstruction.hpp"
#include "Token.hpp"


RexxInstructionEndIf::RexxInstructionEndIf(
    RexxInstructionIf *_parent)         /* base parent instruction (IF/WHEN) */
/******************************************************************************/
/* Function:  Complete initialization of a PARSE ENDIF object                 */
/******************************************************************************/
{
    this->setType(KEYWORD_ENDTHEN);      /* set the default type              */
    OrefSet(this, this->parent, _parent);/* remember parent IF/WHEN/ELSE      */
    parent->setEndInstruction(this);     /* hook up with the parent object    */
                                         /* is this the ELSE end?             */
    if (parent->instructionType == KEYWORD_ELSE)
    {

        this->setType(KEYWORD_ENDELSE);    /* change this into an ELSE end      */
    }
                                           /* is this the ELSE end?             */
    else if (parent->instructionType == KEYWORD_WHENTHEN)
    {
        this->setType(KEYWORD_ENDWHEN);    /* change this into an WHEN end      */
    }
}

void RexxInstructionEndIf::setEndInstruction(
    RexxInstructionEndIf *end_clause)  /* target end point                  */
/******************************************************************************/
/* Function:  Set the END skip point for the instruction                      */
/******************************************************************************/
{
                                       /* set the new location              */
  OrefSet(this, this->else_end, end_clause);
}

void RexxInstructionEndIf::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->else_end);
  memory_mark(this->parent);
}

void RexxInstructionEndIf::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->else_end);
  memory_mark_general(this->parent);
}


void RexxInstructionEndIf::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionEndIf)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->else_end, envelope);
  flatten_reference(newThis->parent, envelope);

  cleanUpFlatten
}

void RexxInstructionEndIf::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/****************************************************************************/
/* Function:  Execute a REXX ENDIF instruction                              */
/****************************************************************************/
{
    context->unindent();                 /* unindent the context              */
    context->unindent();                 /* unindent for the total            */
                                         /* this the end of a WHEN block?     */
    if (this->instructionType == KEYWORD_ENDWHEN)
    {
        context->removeBlock();            /* remove item from block stack      */
        context->unindent();               /* unindent for the SELECT           */
                                           /* set the restart point             */
        context->setNext((this->else_end)->nextInstruction);
    }
    if (this->else_end != OREF_NULL)     /* have to jump around an else?      */
    {
                                         /* set the restart point             */
        context->setNext((this->else_end)->nextInstruction);
    }
}

