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
/* Use Local instruction class                                                */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "QueueClass.hpp"
#include "UseLocalInstruction.hpp"
#include "ExpressionBaseVariable.hpp"

/**
 * Complete construction of a USE LOCAL instruction.
 *
 * @param varCount The count of variables to expose.
 * @param variable_list
 *                 The queue of the variable retrievers (in reverse order).
 */
RexxInstructionUseLocal::RexxInstructionUseLocal(size_t varCount, QueueClass  *variable_list)
{
    // get the variable size
    variableCount = varCount;
    // now copy all of the retriever references from the queue, back to front.
    initializeObjectArray(varCount, variables, RexxVariableBase, variable_list);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionUseLocal::live(size_t liveMark)
{
    // must be first one marked
    memory_mark(nextInstruction);
    memory_mark_array(variableCount, variables);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionUseLocal::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general_array(variableCount, variables);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionUseLocal::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionUseLocal)

    flattenRef(nextInstruction);
    flattenArrayRefs(variableCount, variables);

    cleanUpFlatten
}


/**
 * Execute a USE LOCAL instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionUseLocal::execute(RexxActivation *context, ExpressionStack *stack)
{
    // standard trace on entry.
    context->traceInstruction(this);

    // only allowed in a method context.
    if (!context->inMethod())
    {
        reportException(Error_Execution_use_local_method);
    }

    // the context processes these
    context->autoExpose(variables, variableCount);

    // and standard debug pause.
    context->pauseInstruction();
}


