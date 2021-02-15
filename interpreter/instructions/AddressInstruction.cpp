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
/* REXX Translator                                   AddressInstruction.cpp   */
/*                                                                            */
/* Primitive Address Parse Class                                              */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "AddressInstruction.hpp"
#include "SystemInterpreter.hpp"
#include "MethodArguments.hpp"

/**
 * Constructor for an Address instruction object.
 *
 * @param _expression
 *                 An optional expression for ADDRESS VALUE forms.
 * @param _environment
 *                 A static environment name.
 * @param _command A command expression to be issued.
 */
RexxInstructionAddress::RexxInstructionAddress(RexxInternalObject *_expression,
    RexxString *_environment, RexxInternalObject *_command)
{

    dynamicAddress = _expression;
    environment = _environment;
    command = _command;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionAddress::live(size_t liveMark)
{
    // must be first one marked
    memory_mark(nextInstruction);
    memory_mark(dynamicAddress);
    memory_mark(environment);
    memory_mark(command);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionAddress::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general(dynamicAddress);
    memory_mark_general(environment);
    memory_mark_general(command);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionAddress::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionAddress)

    flattenRef(nextInstruction);
    flattenRef(dynamicAddress);
    flattenRef(environment);
    flattenRef(command);

    cleanUpFlatten
}

/**
 * Execute an addres instruction.
 *
 * @param context The current program execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionAddress::execute(RexxActivation *context, ExpressionStack *stack )
{
    // tracing is handled seperately in each section because the command form
    // needs to trace if "Trace Commands" is in effect.

    // Nothing specified is just an address toggle.. this is simple.
    if (environment == OREF_NULL && dynamicAddress == OREF_NULL)
    {
        // trace if necessary
        context->traceInstruction(this);
        context->toggleAddress();
        context->pauseInstruction();
    }
    // have a static address name?  We could also have a command to issue.
    else if (environment != OREF_NULL)
    {
        // Is this the command form?  Evaluate and issue to the target environment
        if (command != OREF_NULL)
        {
            // this also traces if TRACE COMMANDS is in effect.
            context->traceCommand(this);
            // evaluate the command expression
            RexxObject *result = command->evaluate(context, stack);
            // this must be a string
            RexxString *_command = result->requestString();
            // protect this
            stack->push(_command);
            // are we tracing commands?
            if (context->tracingCommands())
            {
                // trace the full command result
                context->traceResultValue(_command);
            }
            // validate the address name using system rules
            SystemInterpreter::validateAddressName(environment);
            // and execute the command
            context->command(environment, _command, getIOConfig());
        }
        // we're just changing the current address target
        else
        {
            context->traceInstruction(this);
            // validate this environment name
            SystemInterpreter::validateAddressName(environment);
            // and make that the current address
            context->setAddress(environment, getIOConfig());
            context->pauseInstruction();
        }
    }
    // ADDRESS VALUE form
    else
    {
        context->traceInstruction(this);
        // evaluate
        RexxObject *result = dynamicAddress->evaluate(context, stack);
        RexxString *_address = result->requestString();
        // protect this
        stack->push(_address);
        context->traceResult(_address);
        // validate this using system rules, then set the new address
        SystemInterpreter::validateAddressName(_address);
        context->setAddress(_address, getIOConfig());
        context->pauseInstruction();
    }
}

