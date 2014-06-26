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
/* Expose instruction class                                                   */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "QueueClass.hpp"
#include "ExposeInstruction.hpp"
#include "ExpressionBaseVariable.hpp"

/**
 * Complete construction of an EXPOSE instruction.
 *
 * @param varCount The count of variables to expose.
 * @param variable_list
 *                 The queue of the variable retrievers (in reverse order).
 */
RexxInstructionExpose::RexxInstructionExpose(size_t varCount, RexxQueue  *variable_list)
{
    // get the variable size
    variableCount = varCount;
    // now copy all of the retriever references from the queue, back to front.
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
void RexxInstructionExpose::live(size_t liveMark)
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
void RexxInstructionExpose::liveGeneral(int reason)
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
void RexxInstructionExpose::flatten(RexxEnvelope *envelope)
{
    setUpFlatten(RexxInstructionExpose)

    flattenRef(nextInstruction);

    for (size_t i = 0; i < variableCount; i++)
    {
        flattenRef(variables[i]);
    }

    cleanUpFlatten
}


/**
 * Execute an EXPOSE instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionExpose::execute(RexxActivation *context, RexxExpressionStack *stack)
{
    // standard trace on entry.
    context->traceInstruction(this);

    // not allowed in a method context.
    if (!context->inMethod())
    {
        reportException(Error_Translation_expose);
    }

    // the context processeses these
    context->expose(variables, variableCount);

    // and standare debug pause.
    context->pauseInstruction();
}

