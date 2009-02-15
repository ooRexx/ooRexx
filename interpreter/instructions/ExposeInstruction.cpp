/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* Primitive Expose Parse Class                                               */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "QueueClass.hpp"
#include "ExposeInstruction.hpp"
#include "ExpressionBaseVariable.hpp"

RexxInstructionExpose::RexxInstructionExpose(
    size_t      varCount,              /* count of variables                */
    RexxQueue  *variable_list)         /* list of exposed variables         */
/******************************************************************************/
/* Function:  Complete initialization of an EXPOSE instruction object         */
/******************************************************************************/
 {
    /* get the variable size             */
    variableCount = varCount;
    while (varCount > 0)                 /* loop through the variable list    */
    {
                                         /* copying each variable             */
        OrefSet(this, this->variables[--varCount], (RexxVariableBase *)(variable_list->pop()));
    }
}

void RexxInstructionExpose::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    size_t i;                            /* loop counter                      */
    size_t count;                        /* argument count                    */

    memory_mark(this->nextInstruction);  /* must be first one marked          */
    for (i = 0, count = variableCount; i < count; i++)
    {
        memory_mark(this->variables[i]);
    }
}

void RexxInstructionExpose::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    size_t i;                            /* loop counter                      */
    size_t count;                        /* argument count                    */

                                         /* must be first one marked          */
    memory_mark_general(this->nextInstruction);
    for (i = 0, count = variableCount; i < count; i++)
    {
        memory_mark_general(this->variables[i]);
    }
}

void RexxInstructionExpose::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
    size_t i;                            /* loop counter                      */
    size_t count;                        /* argument count                    */

    setUpFlatten(RexxInstructionExpose)

    flatten_reference(newThis->nextInstruction, envelope);
    for (i = 0, count = variableCount; i < count; i++)
    {
        flatten_reference(newThis->variables[i], envelope);
    }

    cleanUpFlatten
}

void RexxInstructionExpose::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/******************************************************************************/
/* Function:  Execute a REXX EXPOSE instruction                               */
/******************************************************************************/
{
    context->traceInstruction(this);     /* trace if necessary                */
    if (!context->inMethod())            /* is this a method clause?          */
    {
                                         /* raise an error                    */
        reportException(Error_Translation_expose);
    }

    /* have the context expose these */
    context->expose(variables, variableCount);
    context->pauseInstruction();         /* do debug pause if necessary       */
}

