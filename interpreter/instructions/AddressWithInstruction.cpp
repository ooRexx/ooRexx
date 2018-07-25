/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* REXX Translator                               AddressWithInstruction.cpp   */
/*                                                                            */
/* Implementation of the address with instruction                             */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "AddressWithInstruction.hpp"
#include "SystemInterpreter.hpp"
#include "MethodArguments.hpp"
#include "CommandIOConfiguration.hpp"
#include "CommandIOContext.hpp"

/**
 * Constructor for an Address With instruction object.
 *
 * @param _expression
 *                 An optional expression for ADDRESS VALUE forms.
 * @param _environment
 *                 A static environment name.
 * @param _command A command expression to be issued.
 * @param config   A potential I/O configuration for doing command redirects.
 */
RexxInstructionAddressWith::RexxInstructionAddressWith(RexxInternalObject *_expression,
    RexxString *_environment, RexxInternalObject *_command, CommandIOConfiguration *config) :
    RexxInstructionAddress(_expression, _environment, _command), ioConfig(config) { ; }


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionAddressWith::live(size_t liveMark)
{
    // must be first one marked
    memory_mark(nextInstruction);
    memory_mark(dynamicAddress);
    memory_mark(environment);
    memory_mark(command);
    memory_mark(ioConfig);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionAddressWith::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general(dynamicAddress);
    memory_mark_general(environment);
    memory_mark_general(command);
    memory_mark_general(ioConfig);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionAddressWith::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionAddressWith)

    flattenRef(nextInstruction);
    flattenRef(dynamicAddress);
    flattenRef(environment);
    flattenRef(command);
    flattenRef(ioConfig);

    cleanUpFlatten
}

