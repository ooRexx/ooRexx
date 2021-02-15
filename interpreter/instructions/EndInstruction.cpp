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
/* Primitive End Parse Class                                                  */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "EndInstruction.hpp"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"

/**
 * Construct an END instruction.
 *
 * @param _name  The optional END label, which will need to be matched
 *               up with the apprpriate block instruction.
 */
RexxInstructionEnd::RexxInstructionEnd(RexxString *_name)
{
    name = _name;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionEnd::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(name);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionEnd::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(name);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionEnd::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionEnd)

    flattenRef(nextInstruction);
    flattenRef(name);

    cleanUpFlatten
}


/**
 * Runtime execution of an END statement.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionEnd::execute(RexxActivation *context, ExpressionStack *stack )
{
    // SIGNAL will disable all active block instructions, so it is possible that
    // this END has been encountered without its corresponding block instruction actually
    // being active.
    if (!context->hasActiveBlockInstructions())
    {
        // trace anyway, then give an error
        context->traceInstruction(this);
        reportException(Error_Unexpected_end_nodo);
    }

    // we have different actions depending on the type of instruction
    // we're hooked to.  Probably not worth trying to create separate END
    // instruction types.
    switch (getStyle())
    {
        // All types of loops.  There will be a block managed by the
        // context that holds the state of an active loop.
        case LOOP_BLOCK:
        {
            // get the top block from the context.
            DoBlock *doBlock = context->topBlockInstruction();
            // For tracing the end, reset the indentation to the block beginning indent.
            // then trace.
            context->setIndent(doBlock->getIndent());
            context->traceInstruction(this);
            // tell the DO/LOOP instruction we're back around.
            ((RexxInstructionBaseLoop *)(doBlock->getParent()))->reExecute(context, stack, doBlock);
            break;
        }

        // END of a select block.  If we have fallen through to this, then we have an
        // error with a SELECT instruction that does not have an OTHERWISE section.
        // This is an error.
        case SELECT_BLOCK:
        {
            // back off the indentation and trace
            context->unindent();
            context->traceInstruction(this);

            // we've traced, now raise an error.
            reportException(Error_When_expected_nootherwise);
            break;
        }

        // for labeled BLOCK types, we need to remove the active marker.
        case OTHERWISE_BLOCK:
        case LABELED_OTHERWISE_BLOCK:
        case LABELED_DO_BLOCK:
        {
            // terminateBlock will also reset the indentation for tracing
            context->terminateBlockInstruction();
            context->traceInstruction(this);
            break;
        }

        // probably a simple unlabeled DO...back off and fall through.
        default:
        {
            context->removeBlockInstruction();
            context->traceInstruction(this);
            break;
        }
    }
}

