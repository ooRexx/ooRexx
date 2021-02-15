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
/* Simple (non-looping) Do instruction                                        */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"
#include "EndInstruction.hpp"
#include "RexxActivation.hpp"


/**
 * Initialize a SimpleDo block.
 *
 * @param l      The block label (if any).
 */
RexxInstructionSimpleDo::RexxInstructionSimpleDo(RexxString *l)
{
    label = l;
}


/**
 * Execute a simple block DO instruction.  We have some
 * slightly different setup requirements, so we override
 * the execute() method rather than relying on the
 * default one from the base class.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionSimpleDo::execute(RexxActivation *context, ExpressionStack *stack)
{
    // trace on entry
    context->traceInstruction(this);

    DoBlock *doblock = OREF_NULL;

    // if we have a label then we need to add a block to the control stack
    // so LEAVE or ITERATE can determine the block bounds.
    if (getLabel() != OREF_NULL)
    {
        // create an active block, and make this the top of the
        // control stack
        doblock = new DoBlock (context, this);
        context->newBlockInstruction(doblock);
    }
    else
    {
        // this just tells the context to step the nesting level.
        context->addBlockInstruction();
    }

    // handle a debug pause that might cause re-execution
    handleDebugPause(context, doblock);
}
