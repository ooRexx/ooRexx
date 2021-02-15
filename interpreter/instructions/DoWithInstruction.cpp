/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* All DO/LOOP WITH instruction variants                                      */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"
#include "EndInstruction.hpp"


/**
 * Initialize a Do Over block.
 *
 * @param l      The optional label.
 * @param c      A variable name for setting a counter (optional)
 * @param o      The loop control information.
 */
RexxInstructionDoWith::RexxInstructionDoWith(RexxString *l, RexxVariableBase *c, WithLoop &o)
{
    label = l;
    countVariable = c;
    withLoop = o;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionDoWith::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(end);
    memory_mark(label);
    memory_mark(countVariable);

    // helpers for additional types of loops handle marking here
    withLoop.live(liveMark);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionDoWith::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(end);
    memory_mark_general(label);
    memory_mark_general(countVariable);

    // helpers for additional types of loops handle marking here
    withLoop.liveGeneral(reason);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionDoWith::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionDoWith)

    flattenRef(nextInstruction);
    flattenRef(end);
    flattenRef(label);
    flattenRef(countVariable);

    // flatten is a bit of a pain with embedded objects because
    // everything depends on having correct pointers to object references in
    // the copied buffer.  We need to directly reference all of the elements here.

    flattenRef(withLoop.itemVar);
    flattenRef(withLoop.indexVar);
    flattenRef(withLoop.supplierSource);

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
void RexxInstructionDoWith::setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock)
{
    // perform the DO OVER initialization
    withLoop.setup(context, stack, doblock);
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
bool RexxInstructionDoWith::iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    return withLoop.checkIteration(context, stack, doblock, first);
}


/**
 * Initialize a Do Over For block.
 *
 * @param l      The optional label.
 * @param c      A variable name for setting a counter (optional)
 * @param o      The loop control information.
 * @param f      The loop For control
 */
RexxInstructionDoWithFor::RexxInstructionDoWithFor(RexxString *l, RexxVariableBase *c, WithLoop &o, ForLoop &f)
{
    label = l;
    countVariable = c;
    withLoop = o;
    forLoop = f;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionDoWithFor::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(end);
    memory_mark(label);
    memory_mark(countVariable);

    // helpers for additional types of loops handle marking here
    withLoop.live(liveMark);
    forLoop.live(liveMark);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionDoWithFor::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(end);
    memory_mark_general(label);
    memory_mark_general(countVariable);

    // helpers for additional types of loops handle marking here
    withLoop.liveGeneral(reason);
    forLoop.liveGeneral(reason);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionDoWithFor::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionDoWithFor)

    flattenRef(nextInstruction);
    flattenRef(end);
    flattenRef(label);
    flattenRef(countVariable);

    // flatten is a bit of a pain with embedded objects because
    // everything depends on having correct pointers to object references in
    // the copied buffer.  We need to directly reference all of the elements here.

    flattenRef(withLoop.itemVar);
    flattenRef(withLoop.indexVar);
    flattenRef(withLoop.supplierSource);
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
void RexxInstructionDoWithFor::setup(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock)
{
    // perform the DO OVER initialization
    withLoop.setup(context, stack, doblock);
    // perform the DO COUNT initialization
    forLoop.setup(context, stack, doblock, true);
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
bool RexxInstructionDoWithFor::iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    return withLoop.checkIteration(context, stack, doblock, first) && doblock->checkFor();
}


/**
 * Initialize a Do Over Until block
 *
 * @param l      The loop label.
 * @param c      A variable name for setting a counter (optional)
 * @param o      The loop control information.
 * @param w      The loop conditional information.
 */
RexxInstructionDoWithUntil::RexxInstructionDoWithUntil(RexxString *l, RexxVariableBase *c, WithLoop &o, WhileUntilLoop &w)
{
    label = l;
    countVariable = c;
    withLoop = o;
    whileLoop = w;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionDoWithUntil::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(end);
    memory_mark(label);
    memory_mark(countVariable);

    // helpers for additional types of loops handle marking here
    withLoop.live(liveMark);
    whileLoop.live(liveMark);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionDoWithUntil::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(end);
    memory_mark_general(label);
    memory_mark_general(countVariable);

    // helpers for additional types of loops handle marking here
    withLoop.liveGeneral(reason);
    whileLoop.liveGeneral(reason);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionDoWithUntil::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionDoWithUntil)

    flattenRef(nextInstruction);
    flattenRef(end);
    flattenRef(label);
    flattenRef(countVariable);

    // flatten is a bit of a pain with embedded objects because
    // everything depends on having correct pointers to object references in
    // the copied buffer.  We need to directly reference all of the elements here.

    flattenRef(withLoop.itemVar);
    flattenRef(withLoop.indexVar);
    flattenRef(withLoop.supplierSource);
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
bool RexxInstructionDoWithUntil::iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    // we don't check the UNTIL condition on the first iteration
    if (first)
    {
        return withLoop.checkIteration(context, stack, doblock, first);
    }

    return !whileLoop.checkUntil(context, stack) && withLoop.checkIteration(context, stack, doblock, first);
}


/**
 * Initialize a Do Over While block
 *
 * @param l      The loop label.
 * @param c      A variable name for setting a counter (optional)
 * @param o      The loop control information.
 * @param w      The loop conditional information.
 */
RexxInstructionDoWithWhile::RexxInstructionDoWithWhile(RexxString *l, RexxVariableBase *c, WithLoop &o, WhileUntilLoop &w)
{
    label = l;
    countVariable = c;
    withLoop = o;
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
bool RexxInstructionDoWithWhile::iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    return withLoop.checkIteration(context, stack, doblock, first) && whileLoop.checkWhile(context, stack);
}


/**
 * Initialize a Do Over For Until block
 *
 * @param l      The loop label.
 * @param c      A variable name for setting a counter (optional)
 * @param o      The loop control information.
 * @param w      The loop conditional information.
 */
RexxInstructionDoWithForUntil::RexxInstructionDoWithForUntil(RexxString *l, RexxVariableBase *c, WithLoop &o, ForLoop &f, WhileUntilLoop &w)
{
    label = l;
    countVariable = c;
    withLoop = o;
    forLoop = f;
    whileLoop = w;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionDoWithForUntil::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(end);
    memory_mark(label);
    memory_mark(countVariable);

    // helpers for additional types of loops handle marking here
    withLoop.live(liveMark);
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
void RexxInstructionDoWithForUntil::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(end);
    memory_mark_general(label);
    memory_mark_general(countVariable);

    // helpers for additional types of loops handle marking here
    withLoop.liveGeneral(reason);
    forLoop.liveGeneral(reason);
    whileLoop.liveGeneral(reason);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionDoWithForUntil::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionDoWithForUntil)

    flattenRef(nextInstruction);
    flattenRef(end);
    flattenRef(label);
    flattenRef(countVariable);

    // flatten is a bit of a pain with embedded objects because
    // everything depends on having correct pointers to object references in
    // the copied buffer.  We need to directly reference all of the elements here.

    flattenRef(withLoop.itemVar);
    flattenRef(withLoop.indexVar);
    flattenRef(withLoop.supplierSource);
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
bool RexxInstructionDoWithForUntil::iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    // we don't check the UNTIL condition on the first iteration
    if (first)
    {
        return withLoop.checkIteration(context, stack, doblock, first) && doblock->checkFor();
    }

    return !whileLoop.checkUntil(context, stack) && withLoop.checkIteration(context, stack, doblock, first) && doblock->checkFor();
}


/**
 * Initialize a Do Over For While block
 *
 * @param l      The loop label.
 * @param c      A variable name for setting a counter (optional)
 * @param o      The loop control information.
 * @param w      The loop conditional information.
 */
RexxInstructionDoWithForWhile::RexxInstructionDoWithForWhile(RexxString *l, RexxVariableBase *c, WithLoop &o, ForLoop &f, WhileUntilLoop &w)
{
    label = l;
    countVariable = c;
    withLoop = o;
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
bool RexxInstructionDoWithForWhile::iterate(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    return withLoop.checkIteration(context, stack, doblock, first) && doblock->checkFor() && whileLoop.checkWhile(context, stack);
}
