/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2023 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
#include "RexxCore.h"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "QueueClass.hpp"
#include "IntegerClass.hpp"
#include "SelectInstruction.hpp"
#include "EndInstruction.hpp"
#include "IfInstruction.hpp"
#include "WhenCaseInstruction.hpp"
#include "OtherwiseInstruction.hpp"
#include "DoBlock.hpp"


/**
 * Construct a SELECT instruction.
 *
 * @param name   The optional label name.
 */
RexxInstructionSelect::RexxInstructionSelect(RexxString *name)
{
    // we keep track of each WHEN that is added to the select.
    // once we get the END instruction, we can update each of the WHENs
    // so they know where to branch.
    whenList = new_queue();
    label = name;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionSelect::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(whenList);
    memory_mark(end);
    memory_mark(otherwise);
    memory_mark(label);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionSelect::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(whenList);
    memory_mark_general(end);
    memory_mark_general(otherwise);
    memory_mark_general(label);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionSelect::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionSelect)

    flattenRef(nextInstruction);
    flattenRef(whenList);
    flattenRef(end);
    flattenRef(otherwise);
    flattenRef(label);

    cleanUpFlatten
}


/**
 * Execute a SELECT instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionSelect::execute(RexxActivation *context, ExpressionStack *stack)
{
    context->traceInstruction(this);

    // create an active DO block, which marks that we have an active SELECT
    // in case someone tries do SIGNAL into the middle of the instruction.
    DoBlock *doblock = new DoBlock (context, this);
    // set the block to the top of the context stack.
    context->newBlockInstruction(doblock);
    // Debug pause requires a conditional pause that terminates the block construct
    // if we've been asked to re-execute.
    if (context->conditionalPauseInstruction())
    {
        this->terminate(context, doblock);
    }
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
 * Terminate the SELECT instruction (usually because of a LEAVE).
 *
 * @param context The current execution context.
 * @param doblock The current doblock context.
 */
void RexxInstructionSelect::terminate(RexxActivation *context, DoBlock *doblock )
{
    // we terminate this doblock
    context->terminateBlockInstruction(doblock->getIndent());
    // and we jump to the instruction after our END
    context->setNext(end->nextInstruction);
}


/**
 * Match an END instruction up with this SELECT.
 *
 * @param partner The partner END
 * @param parser  The language parser environment (used for error reporting)
 */
void RexxInstructionSelect::matchEnd(RexxInstructionEnd *partner, LanguageParser *parser)
{
    // get some location information for error reporting
    SourceLocation endLocation = partner->getLocation();
    size_t lineNum = getLineNumber();

    // ok, we need to match up the names. If the END has a label, then it must
    // match a label on the SELECT.
    RexxString *name = partner->endName();
    if (name != OREF_NULL)
    {
        // One error if we don't had a label, a different error if the
        // labels don't match
        RexxString *myLabel = getLabel();
        if (myLabel == OREF_NULL)
        {
            parser->error(Error_Unexpected_end_select_nolabel, endLocation, new_array(name, new_integer(lineNum)));
        }
        else if (name != myLabel)
        {
            parser->error(Error_Unexpected_end_select, endLocation, new_array(name, myLabel, new_integer(lineNum)));
        }
    }

    // record the END instruction, then do some additional validity checks
    end = partner;

    // if we don't have any WHEN clauses, this is an error
    size_t whenCount = whenList->items();
    if (whenCount == 0)
    {
        parser->error(Error_When_expected_when, getLocation(), new_array(new_integer(lineNum)));
    }

    // now link up each of the WHEN clauses with the END
    while (whenCount--)
    {
        // pull the next item from the queue
        RexxInstruction *when = (RexxInstruction *)whenList->pull();
        if (when->isType(KEYWORD_WHEN))
        {
            // hook up with the partner END instruction
            ((RexxInstructionIf *)when)->fixWhen((RexxInstructionEndIf *)partner);
        }
        // this is a select case version
        else
        {
            // hook up with the partner END instruction
            ((RexxInstructionCaseWhen *)when)->fixWhen((RexxInstructionEndIf *)partner);
        }
    }

    // the when list is empty, we can scrap it now
    whenList = OREF_NULL;

    // do we have an OTHERWISE block?
    if (otherwise != OREF_NULL)
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
        // there is no OTHERWISE clause.  This doesn't matter if there
        // is a label or not.
        partner->setStyle(SELECT_BLOCK);
    }
}


/**
 * Add a WHEN instruction to a SELECT.
 *
 * @param when   The added When instruction.
 */
void RexxInstructionSelect::addWhen(RexxInstructionIf *when)
{
    // just add to the queue
    whenList->push(when);
}


/**
 * Add an OTHERWISE to a SELECT instruction.
 *
 * @param _otherwise
 */
void RexxInstructionSelect::setOtherwise(RexxInstructionOtherwise *_otherwise)
{
    otherwise = _otherwise;
}


/**
 * Construct a SELECT CASE instruction.
 *
 * @param name   The optional label name.
 */
RexxInstructionSelectCase::RexxInstructionSelectCase(RexxString *name, RexxInternalObject *expr)
{
    // we keep track of each WHEN that is added to the select.
    // once we get the END instruction, we can update each of the WHENs
    // so they know where to branch.
    whenList = new_queue();
    label = name;
    caseExpr = expr;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionSelectCase::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(caseExpr);
    memory_mark(whenList);
    memory_mark(end);
    memory_mark(otherwise);
    memory_mark(label);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionSelectCase::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(caseExpr);
    memory_mark_general(whenList);
    memory_mark_general(end);
    memory_mark_general(otherwise);
    memory_mark_general(label);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionSelectCase::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionSelectCase)

    flattenRef(nextInstruction);
    flattenRef(whenList);
    flattenRef(caseExpr);
    flattenRef(end);
    flattenRef(otherwise);
    flattenRef(label);

    cleanUpFlatten
}


/**
 * Execute a SELECT CASE instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionSelectCase::execute(RexxActivation *context, ExpressionStack *stack)
{
    context->traceInstruction(this);

    // create an active DO block, which marks that we have an active SELECT
    // in case someone tries do SIGNAL into the middle of the instruction.
    DoBlock *doblock = new DoBlock (context, this);

    // evaluate the CASE instruction and store in the doblock so the WHEN
    // instructions can retrieve it.
    RexxObject *caseResult = caseExpr->evaluate(context, stack);
    context->traceKeywordResult(GlobalNames::CASE, caseResult);

    // set the block to the top of the context stack.
    context->newBlockInstruction(doblock);

    doblock->setCase(caseResult);

    // Debug pause requires a conditional pause that terminates the block construct
    // if we've been asked to re-execute.
    if (context->conditionalPauseInstruction())
    {
        terminate(context, doblock);
    }
}

