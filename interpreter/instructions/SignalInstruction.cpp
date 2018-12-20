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
/* REXX Translator                                                            */
/*                                                                            */
/* Primitive Signal Parse Class                                               */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "DirectoryClass.hpp"
#include "SignalInstruction.hpp"
#include "MethodArguments.hpp"

/**
 * Constructor for a SIGNAL instruction.
 *
 * @param labelName The name of the target label.
 */
RexxInstructionSignal::RexxInstructionSignal(RexxString *labelName)
{
    targetName = labelName;
}

/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionSignal::live(size_t liveMark)
{
    memory_mark(nextInstruction);  /* must be first one marked          */
    memory_mark(targetInstruction);
    memory_mark(targetName);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionSignal::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general(targetInstruction);
    memory_mark_general(targetName);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionSignal::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionSignal)

    flattenRef(nextInstruction);
    flattenRef(targetInstruction);
    flattenRef(targetName);

    cleanUpFlatten
}

/**
 * Resolve a label target at the end of block parsing.
 *
 * @param labels The directory of label objects for this code section.
 */
void RexxInstructionSignal::resolve(StringTable *labels)
{
    // The section might not have any labels, but if it does, get our
    // name from the directory.  Note, we don't raise an error now, we
    // wait until the instruction is actually executed.
    if (labels != OREF_NULL)
    {
        // this links the signal instruction directly to the label
        // instruction that is the target.
        targetInstruction = (RexxInstruction *)labels->get(targetName);
    }
}


/**
 * Execute this instruction
 *
 * @param context The current program context.
 * @param stack   The expression stack
 */
void RexxInstructionSignal::execute(RexxActivation *context, ExpressionStack *stack)
{
    // trace the instruction if necessary
    context->traceInstruction(this);

    // normal signal instruction

    // the target should have been resolved during the resolve phase.
    // if we don't have anything, raise the error now
    if (targetInstruction == OREF_NULL)
    {
        reportException(Error_Label_not_found_name, targetName);
    }
    // tell the activation we're changing course.  Debug pauses
    // are handled after the signal
    context->signalTo(targetInstruction);
}


/**
 * Constructor for a dynamic SIGNAL instruction.
 *
 * @param labelName The name of the target label.
 */
RexxInstructionDynamicSignal::RexxInstructionDynamicSignal(RexxInternalObject *expr)
{
    dynamicName = expr;
}

/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionDynamicSignal::live(size_t liveMark)
{
    memory_mark(nextInstruction);  /* must be first one marked          */
    memory_mark(dynamicName);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionDynamicSignal::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general(dynamicName);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionDynamicSignal::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionDynamicSignal)

    flattenRef(nextInstruction);
    flattenRef(dynamicName);

    cleanUpFlatten
}


/**
 * Execute this instruction
 *
 * @param context The current program context.
 * @param stack   The expression stack
 */
void RexxInstructionDynamicSignal::execute(RexxActivation *context, ExpressionStack *stack)
{
    // trace the instruction if necessary
    context->traceInstruction(this);

    // evaluate the expression in the current context.
    RexxObject *result = dynamicName->evaluate(context, stack);
    // expression results require tracing
    context->traceKeywordResult(GlobalNames::VALUE, result);
    // force to a string value
    RexxString *stringResult = result->requestString();
    stack->push(stringResult);
    // the context handles locating the target label
    context->signalValue(stringResult);
}


/**
 * Construct a Signal ON instruction object.
 *
 * @param condition The name of the condition trap
 * @param name      The name of the signal target (NULL if this
 *                  is SIGNAL OFF)
 * @param builtin_index
 *                  An index for a potential builtin function call.
 */
RexxInstructionSignalOn::RexxInstructionSignalOn(RexxString *condition, RexxString *name)
{
    conditionName = condition;
    targetName = name;
    targetInstruction = OREF_NULL;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionSignalOn::live(size_t liveMark)
{
    memory_mark(nextInstruction);  // must be first one marked
    memory_mark(targetInstruction);
    memory_mark(conditionName);
    memory_mark(targetName);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionSignalOn::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general(targetInstruction);
    memory_mark_general(targetName);
    memory_mark_general(conditionName);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionSignalOn::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionSignalOn)

    flattenRef(nextInstruction);
    flattenRef(targetInstruction);
    flattenRef(targetName);
    flattenRef(conditionName);

    cleanUpFlatten
}


/**
 * Execute a SIGNAL ON/OFF instruction.  This either activates
 * or deactivates the signal trap.  Calling of a target only
 * happens when a trap is activated.
 *
 * @param context The current program context.
 * @param stack   The current context evaluation stack.
 */
void RexxInstructionSignalOn::execute(RexxActivation *context, ExpressionStack *stack)
{
    // do trace stuff.
    context->traceInstruction(this);

    // if we do not have a target name set, this is a SIGNAL OFF instruction.  Just
    // disable the trap.
    if (targetName != OREF_NULL)
    {
        // wax on...
        context->trapOn(conditionName, this, true);
    }
    else
    {
        // wax off...
        context->trapOff(conditionName, true);
    }
}


/**
 * Resolve a call target at the end of block processing.
 *
 * @param labels The table of label instructions in the current context.
 */
void RexxInstructionSignalOn::resolve(StringTable *labels)
{
    // if there is a labels table, see if we can find a label object from the context.
    // SIGNALS only go to labels, but we don't report an error until the trap is triggered.
    if (labels != OREF_NULL)
    {
        // see if there is a matching label.  If we get something,
        // we're finished.
        targetInstruction = (RexxInstruction *)labels->get((RexxString *)targetName);
    }
}


/**
 * Process a trapped condition.
 *
 * @param context The current program context.
 * @param conditionObj
 *                The condition object for the trap
 */
void RexxInstructionSignalOn::trap(RexxActivation *context, DirectoryClass  *conditionObj)
{
    // trapping a condition turns off the tracp
    context->trapOff(conditionName, true);
    // this should have been resolved already...raise the error now if
    // we don't have a resolved label.
    if (targetInstruction == OREF_NULL)
    {
        reportException(Error_Label_not_found_name, targetName);
    }

    // set the new condition object
    context->setConditionObj(conditionObj);
    // and jump to the condition target
    context->signalTo(targetInstruction);
}

