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
/* Base methods shared amoung all of the DO/LOOP instruction subclasses       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxInstruction.hpp"
#include "DoInstruction.hpp"
#include "StringClass.hpp"
#include "EndInstruction.hpp"
#include "Token.hpp"
#include "LanguageParser.hpp"
#include "RexxActivation.hpp"



/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxBlockInstruction::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(end);
    memory_mark(label);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxBlockInstruction::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(end);
    memory_mark_general(label);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxBlockInstruction::flatten(Envelope *envelope)
{
    setUpFlatten(RexxBlockInstruction)

    flattenRef(nextInstruction);
    flattenRef(end);
    flattenRef(label);

    cleanUpFlatten
}


/**
 * Common method for handling debug pauses at block boundaries.
 *
 * @param context The current activation context.
 * @param doblock The active deblock (if any).
 */
void RexxBlockInstruction::handleDebugPause(RexxActivation *context, DoBlock *doblock)
{
    // do blocks will only do the debug pause on first execution, so
    // the context needs to determine if we're at a good point.  If this is a
    // re-execute option, then we need to redo everything from the start.
    if (context->conditionalPauseInstruction())
    {
        // if we have a doblock for this, do termination cleanup
        if (doblock != OREF_NULL)
        {
            this->terminate(context, doblock);
        }
        // no block required, just remove the nesting
        else
        {
            context->removeBlockInstruction();
        }
        // this makes us the next instruction to be executed
        context->setNext(this);
    }
}


/**
 * Perform parse-time match ups between a DO block and
 * an END instruction.  This performs all appropriate label name
 * matching and
 *
 * @param partner The matched up END instruction.
 * @param parser  The current parser context.
 */
void RexxBaseBlockInstruction::matchEnd(RexxInstructionEnd *partner, LanguageParser *parser)
{
    // make sure we have a good name match
    matchLabel(partner, parser);
    // hook up the END as our partner in crime.
    end = partner;
    // let the END instruction know what action it needs to perform at instruction
    // end.
    partner->setStyle(getEndStyle());
}


/**
 * Terminate a DO or LOOP.  This is really the same for all loop
 * types, so is implemented in the base class
 *
 * @param context The current execution context.
 * @param doblock Our doblock, provided by the context.
 */
void RexxBaseBlockInstruction::terminate(RexxActivation *context, DoBlock *doblock)
{
    // reset the DO block
    context->terminateBlockInstruction(doblock->getIndent());
    // The next instruction is the one after the END
    context->setNext(end->nextInstruction);
}


/**
 * Verify that the name on an END instructon and
 * the instruction label match.
 *
 * @param _end   The candidate end statement.
 * @param parser The LanguageParser context (used for error reporting).
 */
void RexxBaseBlockInstruction::matchLabel(RexxInstructionEnd *_end, LanguageParser *parser)
{
    // get the END name.
    RexxString *name = _end->endName();

    // if there a name on the END?  If no name, we
    // will always match up ok.
    if (name != OREF_NULL)
    {

        // get the location for error reporting
        SourceLocation location = _end->getLocation();
        // and we include our line number so they know which two entities don't match.
        size_t lineNum = getLineNumber();
        // now get my label for comparisons.
        RexxString *myLabel = getLabel();

        // if we don't have a label, then the END cannot
        if (myLabel == OREF_NULL)
        {
            // report the error
            parser->error(Error_Unexpected_end_nocontrol, location, new_array(name, new_integer(lineNum)));
        }
        // we both have names, but they mismatch.
        // NOTE:  We deal with interned names here, so a pointer comparison will suffice for identity.
        else if (name != myLabel)
        {
            parser->error(Error_Unexpected_end_control, location, new_array(name, myLabel, new_integer(lineNum)));
        }
    }
}



// NOTE:  some of the DO instructions don't add additional references, so they can
// just inherit these base marking methods.  Subclasses that do add additional
// references will need to also mark these objects.

/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionBaseLoop::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(end);
    memory_mark(label);
    memory_mark(countVariable);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionBaseLoop::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(end);
    memory_mark_general(label);
    memory_mark_general(countVariable);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionBaseLoop::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionBaseLoop)

    flattenRef(nextInstruction);
    flattenRef(end);
    flattenRef(label);
    flattenRef(countVariable);

    cleanUpFlatten
}


/**
 * Base execute method for a DO instruction.  This performs all
 * common execution steps and delegates the continue tests to
 * the subclasses
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionBaseLoop::execute(RexxActivation *context, ExpressionStack *stack)
{
    // trace on entry
    context->traceInstruction(this);

    // all we do here is create a new doblock and make it active
    Protected<DoBlock> doblock = new DoBlock(context, this);

    // perform loop specific initialization before pushing the doblock
    setup(context, stack, doblock);

    // set the block to the top of the context stack.
    context->newBlockInstruction(doblock);

    // update the iteration counters
    doblock->newIteration();
    // now perform the initial iteration checks
    if (!iterate(context, stack, doblock, true))
    {
        // nothing to process, terminate the loop now
        terminate(context, doblock);
    }
    else
    {
        // we only set the counter variable if we are entering the loop body
        doblock->setCounter(context);
    }

    // handle a debug pause that might cause re-execution
    handleDebugPause(context, OREF_NULL);
}


/**
 * Base re-execute method for do loops.  This does all common
 * logic, then delegates the continue steps to the subclasses.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 * @param doblock The doblock associated with this loop instance.
 */
void RexxInstructionBaseLoop::reExecute(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock)
{
    // change control to the top of the loop
    context->setNext(nextInstruction);
    context->traceInstruction(this);
    context->indent();

    // update the iteration counters
    doblock->newIteration();

    // now perform the loop iteration checkes now...if we're good, we just return
    if (iterate(context, stack, doblock, false))
    {
        doblock->setCounter(context);
        // we're all good.
        return;
    }

    // we need to terminate this loop
    endLoop(context);
}


/**
 * Base setup routine for a loop.  This performs any initial
 * setup instructions for a loop subclass and is called
 * before the first iteration test is performed.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 * @param doblock The doblock associated with this loop instance.
 */
void RexxInstructionBaseLoop::setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock)
{
    // default is no setup
    return;
}


/**
 * Perform a test on whether this loop iteration should
 * be executed or not.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 * @param doblock The doblock for this instruction instance.
 * @param first   True if this is the first iteration, false for
 *                reExecute iterations.
 *
 * @return true if we should execute the loop block, false if
 *         we should terminate the loop.
 */
bool RexxInstructionBaseLoop::iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    // the default is basically a DO FOREVER loop.
    return true;
}


/**
 * Perform normal loop end processing.  This occurs
 * after the loop termination conditions fail.
 *
 * @param context The current execution context.
 */
void RexxInstructionBaseLoop::endLoop(RexxActivation *context)
{
    // pop the block instruction and remove the execution nest.
    context->popBlockInstruction();
    // jump to the loop end
    context->setNext(end->nextInstruction);
    context->unindent();
}
