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
/* Primitive Do Parse Class                                                   */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"
#include "EndInstruction.hpp"
#include "Token.hpp"
#include "SourceFile.hpp"
#include "ExpressionBaseVariable.hpp"


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionDo::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(end);
    memory_mark(label);

    // helpers for additional types of loops handle marking here
    controlLoop.live(liveMark);
    whileLoop.live(liveMark);
    overLoop.live(liveMark);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionDo::liveGeneral(int reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(end);
    memory_mark_general(label);

    // helpers for additional types of loops handle marking here
    controlLoop.liveGeneral(reason);
    whileLoop.liveGeneral(reason);
    overLoop.liveGeneral(reason);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionDo::flatten(RexxEnvelope *envelope)
{
    setUpFlatten(RexxInstructionDo)

    flattenRef(nextInstruction);
    flattenRef(end);
    flattenRef(label);

    // flatten is a bit of a pain with embedded objects because
    // everything depends on having correct pointers to object references in
    // the copied buffer.  We need to directly reference all of the elements here.

    flattenRef(controlLoop.initial);
    flattenRef(controlLoop.to);
    flattenRef(controlLoop.by);
    flattenRef(controlLoop.forCount);
    flattenRef(controlLoop.control);
    flattenRef(forLoop.forCount);
    flattenRef(whileLoop.conditional);
    flattenRef(overLoop.control);
    flattenRef(overLoop.target);

    cleanUpFlatten
}


/**
 * Execute a complex DO/LOOP instruction.  NOTE:  Does not
 * handle the simple DO case.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionDo::execute(RexxActivation *context, RexxExpressionStack *stack)
{
    RexxDoBlock  *doblock = OREF_NULL;   /* active DO block                   */
    RexxObject   *result;                /* expression evaluation result      */
    RexxArray    *array;                 /* converted collection object       */
    wholenumber_t count;                 /* count for repetitive or FOR loops */
    RexxObject   *object;                /* result object (for error)*/

    // we all start with a trace....
    context->traceInstruction(this);

    // create an active DO block and add it to the context block stack.
    RexxDoBlock *doblock = new RexxDoBlock (this, context->getIndent());
    context->newDo(doblock);

    // each type of DO/LOOP is processed differently
    switch (type)
    {
        case DO_FOREVER:                 // Nothing set up initially.
        case DO_UNTIL:                   // DO UNTIL - no checks or setup on first iteration.
            break;

        case DO_OVER:                    // DO name OVER collection
        case DO_OVER_UNTIL:              // same as DO_OVER on first pass
            // perform the DO OVER initialization
            overLoop.setup(context, stack, doblock)

            // now perform the initial DO OVER checks for the first iteration.
            if (!doblock->checkOver(context, stack))
            {
                // nothing to process, terminate the loop now
                terminate(context, doblock);
            }
            break;

        case DO_OVER_WHILE:              // DO name OVER collection WHILE cond
            // perform the DO OVER initialization
            overLoop.setup(context, stack, doblock)

            // do initial termination checks
            if (!doblock->checkOver(context, stack) || !whileLoop.checkWhile(context, stack))
            {
                terminate(context, doblock);
            }
            break;

        case DO_COUNT:                   // DO expr
        case DO_COUNT_UNTIL:             // DO expr UNTIL foo ...no UNTIL check until end of the loop
            // do initial FOR loop setup
            forLoop.setup(context, stack, doblock);

            // if this is just DO 0, terminate now
            if (doblock->testFor() || !whileLoop.checkWhile(context, stack))
            {
                terminate(context, doblock);
            }
            break;

        case DO_COUNT_WHILE:             // DO expr WHILE foo
            // do initial FOR loop setup
            forLoop.setup(context, stack, doblock);

            // if this is just DO 0 or the while fails, terminate now
            if (doblock->testFor() || !whileLoop.checkWhile(context, stack))
            {
                terminate(context, doblock);
            }
            break;

        case DO_WHILE:                   // DO WHILE
            // no setup, just try the condition
            if (!whileLoop.checkWhile(context, stack))
            {
                this->terminate(context, doblock);
            }
            break;

        case CONTROLLED_DO:              // DO i=expr TO expr BY expr FOR expr
        case CONTROLLED_UNTIL:           // DO i=expr ... UNTIL condition ...no additional checks on first pass.
            // do initial control setup and validation
            controlLoop.setup(context, stack, doblock);
            // and perform the termination check for the first pass.
            if (!doblock->checkControl(context, stack, false))
            {
                terminate(context, doblock);
            }
            break;

        case CONTROLLED_WHILE:           // DO i=expr ... WHILE condition
            // do initial control setup and validation
            controlLoop.setup(context, stack, doblock);
            // and perform the termination check for the first pass.
            if (!doblock->checkControl(context, stack, false) || !whileLoop.checkWhile(context, stack))
            {
                terminate(context, doblock);
            }
            break;
    }

    // handle end-of-loop re-execution pause
    handleDebugPause(context, doblock);
}


/**
 * Handle a re-execution pass of a DO/LOOP instruction.
 * This is everything but the first pass.
 *
 * @param context The current execution context.
 * @param stack   The curren evaluation stack.
 * @param doblock The loop state information created on the first
 *                pass through the loop.
 */
void RexxInstructionDo::reExecute(RexxActivation *context, RexxExpressionStack *stack, RexxDoBlock *doblock )
{
    // change the next instruction to be the one at the top of the loop
    context->setNext(nextInstruction);
    // trace this instruction and reset the loop indentation level
    context->traceInstruction(this);
    context->indent();

    // handle each loop type
    switch (type)
    {

        case DO_FOREVER:                   // DO FOREVER loop...never anything to do
            return;

        case DO_OVER:                      // DO name OVER collection loop
            // return and continue if this passes.
            if (doblock->checkOver(context, stack))
            {
                return;
            }
            break;

        case DO_OVER_UNTIL:                // DO name OVER coll. UNTIL cond.
            // both checks need to be true to continue looping
            if (!whileLoop.checkUntil(context, stack) && doblock->checkOver(context, stack))
            {
                return;
            }
            break;

        case DO_OVER_WHILE:                // DO name OVER coll. WHILE cond.
            if (doblock->checkOver(context, stack, ) && whileLoop.checkWhile(context, stack))
            {
                return;
            }
            break;

        case DO_UNTIL:                     // DO UNTIL condition
            if (!whileLoop.checkUntil(context, stack))
            {
                return;
            }
            break;

        case DO_COUNT:                     // DO expr
            if (!doblock->testFor())
            {
                return;
            }
            break;

        case DO_COUNT_WHILE:               // DO expr WHILE expr
            if (!doblock->testFor() && whileLoop.checkWhile(context, stack))
            {
                return;
            }
            break;

        case DO_COUNT_UNTIL:               // DO expr UNTIL expr
            if (!whileLoop.checkUntil(context, stack) && !doblock->testFor())
            {
                return;
            }
            break;

        case DO_WHILE:                     // DO WHILE condition
            if (whileLoop.checkUntil(context, stack))
            {
                return;
            }
            break;

        case CONTROLLED_DO:                // DO i=expr TO expr BY expr FOR expr
            if (doblock->checkControl(context, stack, true))
            {
                return;
            }
            break;

        case CONTROLLED_UNTIL:             // DO i=expr ... UNTIL condition
            if (!whileLoop.checkUntil(context, stack) && doblock->checkControl(context, stack, true))
            {
                return;
            }
            break;

        case CONTROLLED_WHILE:             // DO i=expr ... WHILE condition
            if (doblock->checkControl(context, stack, true) && whileLoop.checkWhile(context, stack))
            {
                return;                        /* finish quickly                    */
            }
            break;
    }

    // shut the loop down.
    endLoop(context);
}
