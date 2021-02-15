/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* Primitive Parse Parse Class                                                */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "QueueClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxActivation.hpp"
#include "Activity.hpp"
#include "ParseInstruction.hpp"
#include "ParseTrigger.hpp"
#include "ParseTarget.hpp"
#include "Token.hpp"
#include "Interpreter.hpp"


/**
 * Initialize a parse instruction.
 *
 * @param sourceExpression
 *               An expression for parse value/parse var.
 * @param string_source
 *               A marker for the string source.
 * @param flags  option flags
 * @param templateCount
 *               The number of templates in the parse.
 * @param parse_template
 *               The source of the parsing templates.
 */
RexxInstructionParse::RexxInstructionParse(RexxInternalObject *sourceExpression, InstructionSubKeyword string_source,
    FlagSet<ParseFlags, 32> flags, size_t templateCount, QueueClass *parse_template )
{
    expression = sourceExpression;
    parseFlags = flags;
    stringSource = string_source;
    triggerCount = templateCount;
    // now copy any triggerss from the sub term stack
    // NOTE:  The triggerss are in last-to-first order on the stack.
    initializeObjectArray(templateCount, triggers, ParseTrigger, parse_template);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionParse::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(expression);
    memory_mark_array(triggerCount, triggers);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionParse::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(expression);
    memory_mark_general_array(triggerCount, triggers);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionParse::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionParse)

    flattenRef(nextInstruction);
    flattenRef(expression);
    flattenArrayRefs(triggerCount, triggers);

    cleanUpFlatten
}


/**
 * Execute a PARSE instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionParse::execute(RexxActivation *context, ExpressionStack *stack )
{
    // trace if necessary
    context->traceInstruction(this);

    // this is our parsing target, which holds the current parsing information.
    RexxTarget target;
    // we only have multiple argument lists if this is PARSE ARG
    bool multiple = false;

    // default different pieces of source data information
    RexxObject *value = GlobalNames::NULLSTRING;
    RexxObject **argList = OREF_NULL;
    size_t argCount = 0;

    switch (stringSource)
    {
        // PULL or PARSE PULL instruction
        case SUBKEY_PULL:
            // pull a line from the queue and push on the stack for safekeeping
            value = context->pullInput();
            stack->push(value);
            context->traceKeywordResult(GlobalNames::PULL, value);
            break;

        // PARSE LINEIN
        case SUBKEY_LINEIN:
            // read a line from the console stream
            value = context->lineIn();
            stack->push(value);
            context->traceKeywordResult(GlobalNames::LINEIN, value);
            break;

        // ARG OR PARSE ARG
        case SUBKEY_ARG:
            // we have multiple values
            multiple = true;
            // get the argument list information
            argList = context->getMethodArgumentList();
            argCount = context->getMethodArgumentCount();
            break;

        // PARSE SOURCE
        case SUBKEY_SOURCE:
            // we're using the source string
            value = context->sourceString();
            stack->push(value);
            context->traceKeywordResult(GlobalNames::SOURCE, value);
            break;

        // PARSE VERSION
        case SUBKEY_VERSION:
            // get the version string
            value = Interpreter::getVersionString();
            stack->push(value);
            context->traceKeywordResult(GlobalNames::VERSION_STRING, value);
            break;

        // PARSE VAR
        case SUBKEY_VAR:
            // our expression is a variable retriever which we can
            // evaluate to get the value.
            value = expression->evaluate(context, stack);
            context->traceKeywordResult(GlobalNames::VAR, value);
            // Evaluate always pushes the object on to the evaluation stack.
            // We don't need to do anthing extra to protect this.
            break;

        // PARSE VALUE expr WITH
        case SUBKEY_VALUE:
            // we should have an expression, but if not, use a NULLSTRING
            if (expression != OREF_NULL)
            {
                value = expression->evaluate(context, stack);
            }
            // PARSE VALUE WITH template actually is legal.  Not useful, but legal :-)
            else
            {
                value = GlobalNames::NULLSTRING;
            }
            context->traceKeywordResult(GlobalNames::VALUE, value);
            // the expression version is still on the evalation stack.
            break;

        // invalid PARSE subkey (should really never happen)
        default:
            reportException(Error_Interpretation_switch, "PULL/PARSE subkey", stringSource);
            break;
    }

    // create the parse target
    target.init(value, argList, argCount, parseFlags, multiple, context, stack);

    // now loop through the triggers, have each perform its configured operation.
    for (size_t i = 0; i < triggerCount; i++)
    {
        // a NULL trigger marks the boundary between comman delimited template
        // sections.  For PARSE ARG, this will advance to the next argument.  For
        // all other sources, this will just use "" as the string value.
        ParseTrigger *trigger = triggers[i];
        if (trigger == OREF_NULL)
        {
            target.next(context);
        }
        else
        {
            // each trigger handles both movement and variable assignment.
            trigger->parse(context, stack, &target);
        }
    }

    // and the ubiquitous debug pause.
    context->pauseInstruction();
}

