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
/* Primitive End-If Parse Class                                               */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "IfInstruction.hpp"
#include "Token.hpp"


/**
 * Initialize an END IF instruction.
 *
 * @param _parent The parent IF instruction.
 */
RexxInstructionEndIf::RexxInstructionEndIf(RexxInstructionIf *_parent)
{
    // set the default type...this can change based on additional events.
    setType(KEYWORD_ENDTHEN);
    // tell the parent IF that we exist
    parent = _parent;
    parent->setEndInstruction(this);

    // now check to see what our parent instruction is and change
    // our type based on the parent type.

    // actually following an ELSE?  We're really an END ELSE.
    if (parent->instructionType == KEYWORD_ELSE)
    {
        setType(KEYWORD_ENDELSE);
    }
    // is our IF really a WHEN?  Again, change to match.
    else if (parent->instructionType == KEYWORD_WHENTHEN)
    {
        setType(KEYWORD_ENDWHEN);
    }
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionEndIf::live(size_t liveMark)
{
    // must be first object marked.
    memory_mark(nextInstruction);
    memory_mark(else_end);
    memory_mark(parent);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionEndIf::liveGeneral(MarkReason reason)
{
    // must be first object marked.
    memory_mark_general(nextInstruction);
    memory_mark_general(else_end);
    memory_mark_general(parent);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionEndIf::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionEndIf)

    flattenRef(nextInstruction);
    flattenRef(else_end);
    flattenRef(parent);

    cleanUpFlatten
}


/**
 * Execute an END IF instuction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionEndIf::execute(RexxActivation *context, ExpressionStack *stack)
{
    // end of a WHEN block?  This means we've executed a WHEN clause of a SELECT,
    // which terminates the SELECT.  The next instruction is the one after the closing
    // END.
    if (this->instructionType == KEYWORD_ENDWHEN)
    {
        // remove the select block and reset the indent
        context->terminateBlockInstruction();
        // branch to the END of the SELECT.
        context->setNext(else_end->nextInstruction);
    }
    else
    {
        // We indent twice at the THEN, so we need to remove both of these.
        context->unindentTwice();

        // There are three situation here 1)  We're following a THEN clause
        // with no ELSE, 2) we're following an ELSE, or 3) we're following
        // an THEN with an ELSE.  For 1&2, we automatically fall through
        // to the next instruction.  For 3, we need to jump to the instruction
        // following the else.
        if (else_end != OREF_NULL)
        {
            context->setNext(else_end->nextInstruction);
        }
    }
}


/**
 * Set the end location this instruction needs to
 * advance to.
 *
 * @param end_clause The target END IF instruction.  This is normally
 *                   only set if there is an ELSE or we're part of a SELECT.
 */
void RexxInstructionEndIf::setEndInstruction(RexxInstructionEndIf *end_clause)
{
    // we have somewhere to branch to when we complete.
    else_end = end_clause;
}
