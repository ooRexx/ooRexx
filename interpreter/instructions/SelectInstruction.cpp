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
/* Primitive Select Parse Class                                               */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "QueueClass.hpp"
#include "IntegerClass.hpp"
#include "SelectInstruction.hpp"
#include "EndInstruction.hpp"
#include "IfInstruction.hpp"
#include "OtherwiseInstruction.hpp"
#include "DoBlock.hpp"


RexxInstructionSelect::RexxInstructionSelect(RexxString *name)
/******************************************************************************/
/* Function:  Initialize a SELECT instruction object                          */
/******************************************************************************/
{
                                       /* create a list of WHEN targets     */
    OrefSet(this, this->when_list, new_queue());
    // set the label name
    OrefSet(this, this->label, name);
}


void RexxInstructionSelect::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->when_list);
  memory_mark(this->end);
  memory_mark(this->otherwise);
  memory_mark(this->label);
}

void RexxInstructionSelect::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->when_list);
  memory_mark_general(this->end);
  memory_mark_general(this->otherwise);
  memory_mark_general(this->label);
}

void RexxInstructionSelect::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionSelect)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->when_list, envelope);
  flatten_reference(newThis->end, envelope);
  flatten_reference(newThis->otherwise, envelope);
  flatten_reference(newThis->label, envelope);

  cleanUpFlatten
}



/**
 * Return the associated label.
 *
 * @return The select label (which might be OREF_NULL)
 */
RexxString *RexxInstructionSelect::getLabel()
{
    return label;
}


/**
 * Tests to see if this is a loop instruction.
 *
 * @return True if this is a repetitive loop, false otherwise.
 */
bool RexxInstructionSelect::isLoop()
{
    return false;
}


/**
 * Check for a label match on a block instruction.
 *
 * @param name   The target block name.
 *
 * @return True if this is a name match, false otherwise.
 */
bool RexxInstructionSelect::isLabel(RexxString *name)
{
    return label == name;
}


void RexxInstructionSelect::terminate(
     RexxActivation *context,          /* current execution context         */
     RexxDoBlock    *doblock )         /* active do block                   */
/******************************************************************************/
/* Function:  Terminate an active do loop                                     */
/******************************************************************************/
{
                                       /* perform cleanup                   */
  context->terminateBlock(doblock->getIndent());
                                       /* jump to the loop end              */
  context->setNext(this->end->nextInstruction);
}


void RexxInstructionSelect::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/****************************************************************************/
/* Function:  Execute a REXX SELECT instruction                             */
/****************************************************************************/
{
    RexxDoBlock *doblock = OREF_NULL;

    context->traceInstruction(this);     /* trace if necessary                */
    if (getLabel() != OREF_NULL)
    {
                                           /* create an active DO block         */
        doblock = new RexxDoBlock (this, context->getIndent());
        context->newDo(doblock);           /* set the new block                 */
    }
    else
    {
        context->indent();                   /* indent on tracing                 */
        context->addBlock();               /* step the nesting level            */
    }
                                       /* do debug pause if necessary       */

                                       /* have to re-execute?               */
    if (context->conditionalPauseInstruction())
    {
        if (doblock != OREF_NULL)
        {
            this->terminate(context, doblock); /* cause termination cleanup         */
        }
        else
        {
            context->removeBlock();        /* cause termination cleanup         */
            context->unindent();               /* step back trace indentation       */
        }
    }
}


void RexxInstructionSelect::matchEnd(
     RexxInstructionEnd *partner,      /* end to match up                   */
     RexxSource         *source )      /* parsed source file (for errors)   */
/******************************************************************************/
/* Function:  Match an END instruction up with a SELECT                       */
/******************************************************************************/
{
    RexxInstructionIf    *when;          /* target WHEN clause                */
    SourceLocation        location;      /* location of the end               */
    size_t                lineNum;       /* Instruction line number           */

    location = partner->getLocation();   /* get location of END instruction   */
    lineNum = this->getLineNumber();     /* get the instruction line number   */

    RexxString *name = partner->name;    /* get then END name                 */
    if (name != OREF_NULL)             /* was a name given?                 */
    {
        RexxString *myLabel = getLabel();
        if (myLabel == OREF_NULL)          /* name given on non-control form?   */
        {
            ActivityManager::currentActivity->raiseException(Error_Unexpected_end_select_nolabel, &location, source, OREF_NULL, new_array(partner->name, new_integer(lineNum)), OREF_NULL);
        }
        else if (name != myLabel)          /* not the same name?                */
        {
            ActivityManager::currentActivity->raiseException(Error_Unexpected_end_select, &location, source, OREF_NULL, new_array(name, myLabel, new_integer(lineNum)), OREF_NULL);
        }
    }
    /* misplaced END instruction         */
    OrefSet(this, this->end, partner);   /* match up with the END instruction */
                                         /* get first item off of WHEN list   */
    when = (RexxInstructionIf *)(this->when_list->pullRexx());
    /* nothing there?                    */
    if (when == (RexxInstructionIf *)TheNilObject)
    {
        location = this->getLocation();    /* get the location info             */
                                           /* need at least one WHEN here       */
        ActivityManager::currentActivity->raiseException(Error_When_expected_when, &location, source, OREF_NULL, new_array(new_integer(lineNum)), OREF_NULL);
    }
    /* link up each WHEN with the END    */
    while (when != (RexxInstructionIf *)TheNilObject)
    {
        /* hook up with the partner END      */
        when->fixWhen((RexxInstructionEndIf *)partner);
        /* get the next list item            */
        when = (RexxInstructionIf *)(this->when_list->pullRexx());
    }
    /* get rid of the lists              */
    OrefSet(this, this->when_list, OREF_NULL);
    if (this->otherwise != OREF_NULL)    /* an other wise block?              */
    {
        // for the END terminator on an OTHERWISE, we need to see if this
        // select has a label.  If it does, this needs special handling.
        if (getLabel() == OREF_NULL)
        {
            partner->setStyle(OTHERWISE_BLOCK);
        }
        else
        {
            partner->setStyle(LABELED_OTHERWISE_BLOCK);
        }
    }
    else
    {
        // the SELECT style will raise an error if hit, since it means
        // there is not OTHERWISE clause.  This doesn't matter if there
        // is a label or not.
        partner->setStyle(SELECT_BLOCK);
    }
}


void RexxInstructionSelect::addWhen(
    RexxInstructionIf *when)           /* associated WHEN instruction       */
/******************************************************************************/
/* Function:  Associate a WHEN instruction with its surrounding SELECT        */
/******************************************************************************/
{
                                       /* add to the WHEN list queue        */
  this->when_list->pushRexx((RexxObject *)when);
}

void RexxInstructionSelect::setOtherwise(
    RexxInstructionOtherwise *_otherwise) /* partner OTHERWISE for SELECT      */
/******************************************************************************/
/* Function:  Associate an OTHERSISE instruction with its surrounding SELECT  */
/******************************************************************************/
{
                                       /* save the otherwise partner        */
  OrefSet(this, this->otherwise, _otherwise);
}

