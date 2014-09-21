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
/* Primitive Options Parse Class                                              */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "OptionsInstruction.hpp"
#include "ArrayClass.hpp"

/**
 * Constructor for an OPTIONS instruction
 *
 * @param _expression
 *               The expression to evaluate for the options string.
 */
RexxInstructionOptions::RexxInstructionOptions(RexxInternalObject *_expression)
{
    expression = _expression;
}

/**
 * Execute an OPTIONS instruction.  NOTE:  Currently, there
 * are no OPTIONS that we support on this.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionOptions::execute(RexxActivation *context, ExpressionStack *stack)
{
    // even though we don't support anything, we still trace the evaluation and
    // raise any errors if they occure.
    context->traceInstruction(this);

    // trace, and get as a string.
    RexxString *stringValue = evaluateStringExpression(context, stack);

// NOTE:  processing is currently disabled, but if options are reenabled, the
// processing framework is here.
#if 0
    // break up into an array of words
    ArrayClass *words = stringValue->subWords(OREF_NULL, OREF_NULL)l
    size_t wordCount = words->size();

    for (size_t i = 1; i <= wordCount ;i++)
    {
        RexxString *word = (RexxString *)words->get(i);

#ifdef _DEBUG
        if (word->strCaselessCompare("DUMPMEMORY"))
        {
            memoryObject.dumpEnable = true;
            memoryObject.dump();
        }
#endif
    }
#endif
    context->pauseInstruction();
}

