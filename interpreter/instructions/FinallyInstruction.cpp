/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026 Rexx Language Association. All rights reserved.         */
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
/******************************************************************************/
/* REXX Translator                                                            */
/*                                                                            */
/* FINALLY clause of a DO, LOOP or SELECT block (RFE #31)                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "FinallyInstruction.hpp"
#include "Token.hpp"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"
#include "EndInstruction.hpp"


/**
 * Construct a FINALLY clause.
 */
RexxInstructionFinally::RexxInstructionFinally(RexxToken *token) : RexxInstructionCatch((ArrayClass *)OREF_NULL)
{
    // an instruction may follow FINALLY in the same clause, as after
    // OTHERWISE: the clause is the keyword alone
    setLocation(token->getLocation());
}


/**
 * Execute a FINALLY clause.  Control reaches the clause itself when
 * the body of the block, or the code of its last CATCH clause, runs
 * into it.  Every other way out of the block goes straight to the
 * code of the clause (RexxActivation::leaveBlocks()).
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionFinally::execute(RexxActivation *context, ExpressionStack *stack)
{
    // no active block: a SIGNAL into the middle of a block
    if (!context->hasActiveBlockInstructions())
    {
        context->traceInstruction(this);
        reportException(Error_Unexpected_end_finally);
    }

    DoBlock *doblock = context->topBlockInstruction();

    // the body has finished (not the code of a CATCH clause): do what
    // the END would have done
    if (!doblock->isInCatch())
    {
        // a loop goes round again (the END is traced, as without clauses,
        // and as a CATCH clause does); when it ends, the code of this clause runs
        if (ownerStyle == LOOP_BLOCK)
        {
            context->setIndent(doblock->getIndent());
            context->traceInstruction(owner->end);
            ((RexxInstructionBaseLoop *)owner)->reExecute(context, stack, doblock);
            return;
        }
        // a SELECT without OTHERWISE where no WHEN was true: the code of
        // this clause runs before the error goes on, as for any condition
        if (ownerStyle == SELECT_BLOCK)
        {
            context->unindent();
            context->traceInstruction(owner->end);
            reportException(Error_When_expected_nootherwise);
        }
    }

    // the block ends: this starts the code of the clause (and traces it)
    owner->terminate(context, doblock);
}
