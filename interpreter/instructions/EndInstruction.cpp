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
/* Primitive End Parse Class                                                  */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "EndInstruction.hpp"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"

RexxInstructionEnd::RexxInstructionEnd(
    RexxString *_name)                  /* The END instruction name          */
/****************************************************************************/
/* Function:  Set the name of an END instruction                            */
/****************************************************************************/
{
  OrefSet(this, this->name, _name);
}

void RexxInstructionEnd::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->name);
}

void RexxInstructionEnd::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->name);
}

void RexxInstructionEnd::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionEnd)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->name, envelope);

  cleanUpFlatten
}

void RexxInstructionEnd::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/****************************************************************************/
/* Function:  Execute a REXX END instruction                                */
/****************************************************************************/
{
    RexxDoBlock       *doBlock;          /* target DO block                   */

    if (!context->hasActiveBlocks())     /* no possible blocks?               */
    {
        context->traceInstruction(this);     /* trace if necessary                */
                                         /* this is an error                  */
        reportException(Error_Unexpected_end_nodo);
    }

    switch (this->getStyle())
    {          /* process each loop type            */

        case LOOP_BLOCK:                   /* is this a loop?                   */
            doBlock = context->topBlock();   /* get the top DO block              */
                                             /* reset the indentation             */
            context->setIndent(doBlock->getIndent());
            context->traceInstruction(this);     /* trace if necessary                */
            /* pass on the reexecution           */
            ((RexxInstructionDo *)(doBlock->getParent()))->reExecute(context, stack, doBlock);
            break;

        case SELECT_BLOCK:                 /* END of a select block             */
            context->unindent();                 /* remove indentation                */
            context->traceInstruction(this);     /* trace if necessary                */
            /* looking for a WHEN match          */
            /* this is an error                  */
            reportException(Error_When_expected_nootherwise);
            break;

            // for labeled BLOCK types, we need to remove the active marker.
        case OTHERWISE_BLOCK:
        case LABELED_OTHERWISE_BLOCK:
        case LABELED_DO_BLOCK:
            context->terminateBlock();
            context->traceInstruction(this);     /* trace if necessary                */
            break;

        default:                                 /* all others                        */
            context->unindent();                 /* remove indentation                */
            context->removeBlock();              /* just step back next level         */
            context->traceInstruction(this);     /* trace if necessary                */
            break;
    }
}

