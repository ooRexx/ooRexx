/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* All DO/LOOP count instruction variants                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"
#include "EndInstruction.hpp"


/**
 * Initialize a Do count block.
 *
 * @param l      The block label.
 * @param c      A variable name for setting a counter (optional)
 * @param f      The FOR loop control information.
 */
RexxInstructionDoCount::RexxInstructionDoCount(RexxString *l, RexxVariableBase *c, ForLoop &f)
{
    label = l;
    countVariable = c;
    forLoop = f;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionDoCount::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(end);
    memory_mark(label);
    memory_mark(countVariable);

    // helpers for additional types of loops handle marking here
    forLoop.live(liveMark);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionDoCount::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(end);
    memory_mark_general(label);
    memory_mark_general(countVariable);

    // helpers for additional types of loops handle marking here
    forLoop.liveGeneral(reason);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionDoCount::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionDoCount)

    flattenRef(nextInstruction);
    flattenRef(end);
    flattenRef(label);
    flattenRef(countVariable);

    // flatten is a bit of a pain with embedded objects because
    // everything depends on having correct pointers to object references in
    // the copied buffer.  We need to directly reference all of the elements here.

    flattenRef(forLoop.forCount);

    cleanUpFlatten
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
void RexxInstructionDoCount::setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock)
{
    // perform the DO OVER initialization
    forLoop.setup(context, stack, doblock, false);
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
bool RexxInstructionDoCount::iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    return doblock->checkFor();
}


/**
 * Initialize a Do count UNTIL block.
 *
 * @param l      The block label.
 * @param c      A variable name for setting a counter (optional)
 * @param f      The FOR loop control information.
 * @param w      The block conditional information.
 */
RexxInstructionDoCountUntil::RexxInstructionDoCountUntil(RexxString *l, RexxVariableBase *c, ForLoop &f, WhileUntilLoop &w)
{
    label = l;
    countVariable = c;
    forLoop = f;
    whileLoop = w;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionDoCountUntil::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(end);
    memory_mark(label);
    memory_mark(countVariable);

    // helpers for additional types of loops handle marking here
    forLoop.live(liveMark);
    whileLoop.live(liveMark);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionDoCountUntil::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(end);
    memory_mark_general(label);
    memory_mark_general(countVariable);

    // helpers for additional types of loops handle marking here
    forLoop.liveGeneral(reason);
    whileLoop.liveGeneral(reason);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionDoCountUntil::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionDoCountUntil)

    flattenRef(nextInstruction);
    flattenRef(end);
    flattenRef(label);
    flattenRef(countVariable);

    // flatten is a bit of a pain with embedded objects because
    // everything depends on having correct pointers to object references in
    // the copied buffer.  We need to directly reference all of the elements here.

    flattenRef(forLoop.forCount);
    flattenRef(whileLoop.conditional);

    cleanUpFlatten
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
bool RexxInstructionDoCountUntil::iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    // we don't check the UNTIL condition on the first iteration
    if (first)
    {
        return doblock->checkFor();
    }

    return !whileLoop.checkUntil(context, stack) && doblock->checkFor();
}


/**
 * Initialize a Do count WHILE block.
 *
 * @param l      The block label.
 * @param c      A variable name for setting a counter (optional)
 * @param f      The FOR loop control information.
 * @param w      The block conditional information.
 */
RexxInstructionDoCountWhile::RexxInstructionDoCountWhile(RexxString *l, RexxVariableBase *c, ForLoop &f, WhileUntilLoop &w)
{
    label = l;
    countVariable = c;
    forLoop = f;
    whileLoop = w;
}

// NOTE The WHILE variant does not any additional fields, so it can
// just use the live() methods inherited from the UNTIL version.


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
bool RexxInstructionDoCountWhile::iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    return doblock->checkFor() && whileLoop.checkWhile(context, stack);
}

