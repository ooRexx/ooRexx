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
/* Primitive Drop Parse Class                                                 */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"
#include "DropInstruction.hpp"
#include "ExpressionBaseVariable.hpp"
#include "Activity.hpp"
#include "BufferClass.hpp"


/**
 * Complete construction of a drop instruction.
 *
 * @param varCount The count of variables.
 * @param variable_list
 *                 The list of variables, a queue with the variables
 *                 stored in reverse order.
 */
RexxInstructionDrop::RexxInstructionDrop(size_t varCount, QueueClass *variable_list)
{
    // copy each of the variables from the queue into the object storage.
    // the copy is done back to front because the queue has them in LIFO order.
    variableCount = varCount;
    while (varCount > 0)
    {
        variables[--varCount] = (RexxVariableBase *)variable_list->pop();
    }
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionDrop::live(size_t liveMark)
{
    // must be first one marked
    memory_mark(nextInstruction);
    for (size_t i = 0; i < variableCount; i++)
    {
        memory_mark(variables[i]);
    }
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionDrop::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    for (size_t i = 0; i < variableCount; i++)
    {
        memory_mark_general(variables[i]);
    }
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionDrop::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionDrop)

    flattenRef(nextInstruction);

    for (size_t i = 0; i < variableCount; i++)
    {
        flattenRef(variables[i]);
    }

    cleanUpFlatten
}

/**
 * Execute a drop instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionDrop::execute(RexxActivation *context, ExpressionStack *stack)
{
    // trace if necessary
    context->traceInstruction(this);

    // loop through the list telling each variable to drop.
    for (size_t i = 0; i < variableCount; i++)
    {
        variables[i]->drop(context);
    }

    // standard debug pause.
    context->pauseInstruction();
}

