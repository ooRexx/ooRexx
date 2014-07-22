/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "ElseInstruction.hpp"
#include "IfInstruction.hpp"
#include "Token.hpp"

/**
 * Initialize an ELSE clause.
 *
 * @param token  The token for the ELSE (used to set the location);
 */
RexxInstructionElse::RexxInstructionElse(RexxToken  *token)
{
    // override the location information from the full clause to
    // be just the ELSE keyword.
    setLocation(token->getLocation());
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionElse::live(size_t liveMark)
{
    // must be first object marked.
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
void RexxInstructionElse::liveGeneral(MarkReason reason)
{
    // must be first object marked.
    memory_mark_general(nextInstruction);
    memory_mark_general(parent);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionElse::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionElse)

    flattenRef(nextInstruction);
    flattenRef(parent);

    cleanUpFlatten
}


/**
 * Execute an ELSE instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionElse::execute(RexxActivation *context, ExpressionStack *stack )
{
    // executing an ELSE is not very complicated.  We indent a little, trace the instruction
    // then indent again for the instruction that follows the ELSE.
    context->indent();
    context->traceInstruction(this);
    context->indent();
}


/**
 * Hook the ELSE instruction back to is parent IF.
 *
 * @param ifthen The parent IF instruction.
 */
void  RexxInstructionElse::setParent(RexxInstructionEndIf *ifthen)
{
    parent = (RexxInstructionIf *)ifthen;
}

/**
 * complete links between the IF, THEN, and ELSE constructs now
 * that the full scoping has been resolved.
 *
 * @param end_clause The instruction that marks the end of the instruction
 *                   afer the instruction on the ELSE.  This is where the
 *                   THEN instruction will branch to when finished.
 */
void  RexxInstructionElse::setEndInstruction(RexxInstructionEndIf *end_clause)
{
    // the ELSE doesn't really need this, but we poke the THEN so it knows where
    // control goes after its instruction completes.
    parent->setEndInstruction(end_clause);
}


