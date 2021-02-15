/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* Primitive Then Parse Class                                                 */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "ThenInstruction.hpp"
#include "ElseInstruction.hpp"
#include "IfInstruction.hpp"
#include "Token.hpp"


/**
 * Initialize a THEN instruction.
 *
 * @param token   The token for the keyword (used for location information)
 * @param _parent The parent IF instruction.
 */
RexxInstructionThen::RexxInstructionThen(RexxToken *token, RexxInstructionIf *_parent)
{
    parent = _parent;
    // can be attached to either an IF or WHEN instruction, so make sure
    // we know which type we really are.

    // IF ... THEN?
    if (parent->isType(KEYWORD_IF))
    {
        setType(KEYWORD_IFTHEN);
    }
    // other option is WHEN ... THEN
    else
    {
        setType(KEYWORD_WHENTHEN);
    }
    // When first created, the location was set to the full clause containing the THEN,
    // but we want just the location of the THEN keyword for this.
    setLocation(token->getLocation());
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionThen::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(parent);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionThen::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(parent);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionThen::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionThen)

    flattenRef(nextInstruction);
    flattenRef(parent);

    cleanUpFlatten
}


/**
 * Execute a THEN instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionThen::execute(RexxActivation *context, ExpressionStack *stack )
{
    // Like an ELSE, not much going on here.  We indent, trace the instruction,
    // then bump the indent one more level for the instruction that follows the THEN.
    context->indent();
    context->traceInstruction(this);
    context->indent();
}


/**
 * Set the end location for this THEN clause.  This tells
 * us where to go after our following instruction completes.
 *
 * @param end_clause The end of control instruction.
 */
void  RexxInstructionThen::setEndInstruction(RexxInstructionEndIf *end_clause)
{
  // we really just tell the parent IF/WHEN where this is.
  ((RexxInstructionIf *)parent)->setEndInstruction(end_clause);
}


/**
 * Tell the THEN that there exists an ELSE clause.
 *
 * @param else_clause
 *               The partner ELSE clause.
 */
void  RexxInstructionThen::setElse(RexxInstructionElse *else_clause)
{
    // Tell the parent that we have an ELSE.
    ((RexxInstructionIf *)parent)->setEndInstruction((RexxInstructionEndIf *)else_clause);
    // and tell the ELSE who we're attached to.
    else_clause->setParent((RexxInstructionEndIf *)parent);
}

