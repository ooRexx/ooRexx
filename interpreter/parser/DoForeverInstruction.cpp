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
/* Simple (non-looping) Do instruction                                        */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"
#include "EndInstruction.hpp"

/**
 * Initialize a DoForever block from a fuller DO instruction.
 *
 * @param parent The parent DO instruction.
 */
RexxInstructionDoForever::RexxInstructionDoForever(RexxInstructionDo *parent)
{
    // the label is the only part of the big block that is interesting here.
    label = parent->label;
}

/**
 * Execute a simple block DO instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionDoForever::execute(RexxActivation *context, RexxExpressionStack *stack)
{
    // trace on entry
    context->traceInstruction(this);

    // all we do here is create a new doblock and make it active
    RexxDoBlock *doblock = new RexxDoBlock (this, context->getIndent());
    context->newDo(doblock);

    // handle a debug pause that might cause re-execution
    handleDebugPause(context, OREF_NULL);
}


/**
 * Base re-execute method for a DO FOREVER instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 * @param doblock The doblock associated with this loop instance.
 */
void RexxInstructionDoForever::reExecute(RexxActivation *context, RexxExpressionStack *stack, RexxDoBlock *doblock)
{
    // change control to the top of the loop
    context->setNext(nextInstruction);
    context->traceInstruction(this);
    context->indent();
}


/**
 * Terminate a simple do.
 *
 * @param context The current execution context.
 * @param doblock Our doblock, provided by the context.
 */
void RexxInstructionDoForever::terminate(RexxActivation *context, RexxDoBlock *doblock )
{
    // reset the DO block
    context->terminateBlock(doblock->getIndent());
    // The next instruction is the one after the END
    context->setNext(end->nextInstruction);
}

