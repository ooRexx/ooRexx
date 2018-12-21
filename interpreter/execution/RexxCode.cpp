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
/* REXX Kernel                                                 RexxCode.cpp   */
/*                                                                            */
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxCode.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxInstruction.hpp"
#include "ActivityManager.hpp"
#include "RexxActivation.hpp"
#include "LanguageParser.hpp"


/**
 * Allocate storage for a new RexxCode object.
 *
 * @param size   The size of the object.
 *
 * @return Memory for creating this object.
 */
void *RexxCode::operator new(size_t size)
{
    return new_object(size, T_RexxCode);
}


/**
 * Initialize a RexxCode unit that can be attached to
 * a method or routine object and execute Rexx code.
 *
 * @param _package
 * @param loc      The location of the code block.
 * @param _start   The first instruction object in the execution chain.
 * @param _labels  The directory of labels in this code unit (can be null)
 * @param maxstack The maximum expression stack required to execute this code.
 * @param variable_index
 *                 The number of pre-assigned variable
 *                 slots.
 */
RexxCode::RexxCode(PackageClass *_package, SourceLocation &loc, RexxInstruction *_start, StringTable *_labels,
     size_t maxstack, size_t  variable_index)
{
    package = _package;
    location = loc;
    start = _start;
    labels = _labels;
    // we add in a reasonable extra just in case the calculation got a little off.
    maxStack = maxstack + MINIMUM_STACK_FRAME;
    vdictSize = variable_index;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxCode::live(size_t liveMark)
{
    memory_mark(package);
    memory_mark(start);
    memory_mark(labels);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxCode::liveGeneral(MarkReason reason)
{
    // if we're getting ready to save the image, replace the source
    // package with the global REXX package
    if (reason == PREPARINGIMAGE)
    {
        package = TheRexxPackage;
    }

    memory_mark_general(package);
    memory_mark_general(start);
    memory_mark_general(labels);
}


/**
 * Flatten a Rexx code object
 *
 * @param envelope The envelope used for the data.
 */
void RexxCode::flatten(Envelope * envelope)
{
    setUpFlatten(RexxCode)

    flattenRef(package);
    flattenRef(start);
    flattenRef(labels);

    cleanUpFlatten
}


/**
 * Process a detached ::requires type call.
 *
 * @param activity The current activity,
 * @param routine  The routine object we're executing.
 * @param msgname  The name this was invoked under.
 * @param argPtr   The pointer to the call arguments,
 * @param argcount The count of arguments,
 * @param result   The returned result.
 */
void RexxCode::call(Activity *activity, RoutineClass *routine, RexxString *msgname, RexxObject**argPtr, size_t argcount, ProtectedObject &result)
{
    // just forward to the more general method
    call(activity, routine, msgname, argPtr, argcount, GlobalNames::SUBROUTINE, OREF_NULL, EXTERNALCALL, result);
}


/**
 * Call a unit of Rexx code as a routine or top-level call.
 *
 * @param activity The activity this is running on.
 * @param routine  The routine object this code is attached to.
 * @param routineName  The name of the call.
 * @param argPtr   Pointer to the call arguments.
 * @param argcount The number of arguments being passed.
 * @param calltype The type of call (used in parse source)
 * @param environment
 *                 The initial address environment.
 * @param context  The call context indicator (program, routine, internalcall, etc.)
 * @param result   A protected object for passing the return
 *                 value back.
 */
void RexxCode::call(Activity *activity, RoutineClass *routine, RexxString *routineName,
    RexxObject**argPtr, size_t argcount, RexxString *calltype, RexxString *environment,
    ActivationContext context, ProtectedObject &result)
{
    // check the stack space before proceeding
    activity->checkStackSpace();
    // create a new activity to run this and add to the stack.
    RexxActivation *newacta = ActivityManager::newActivation(activity, routine, this, calltype, environment, context);
    activity->pushStackFrame(newacta);
    // have the activation run this code.  The return result is passed back through result.
    newacta->run(OREF_NULL, routineName, argPtr, argcount, OREF_NULL, result);
}


/**
 * Call a unit of Rexx code as a method invocation.
 *
 * @param activity The activity this is running on.
 * @param method   The method object this is attached to.
 * @param receiver The target object for the message.
 * @param msgname  The name of the message.
 * @param argPtr   Pointer to the call arguments.
 * @param argcount The number of arguments being passed.
 * @param result   A protected object for passing the return
 *                 value back.
 */
void RexxCode::run(Activity *activity, MethodClass *method, RexxObject *receiver,
    RexxString *msgname, RexxObject**argPtr, size_t argcount, ProtectedObject &result)
{
    // create a new activation object and push it on the top of the stack.
    RexxActivation *newacta = ActivityManager::newActivation(activity, method, this);
    activity->pushStackFrame(newacta);
    // run the method.  The result is returned via the ProtectedObject reference.
    newacta->run(receiver, msgname, argPtr, argcount, OREF_NULL, result);
}


/**
 * Extract from the source code for the span represented
 * by this code block.
 *
 * @return An array of the code source lines.
 */
ArrayClass *RexxCode::getSource()
{
    // the source package handles this.
    return package->extractSource(location);
}


/**
 * Get the code program name.
 *
 * @return The string name of the program.
 */
RexxString * RexxCode::getProgramName()
{
    return package->getProgramName();
}


/**
 * Set a security manager on this code object (actually sets it on the package.)
 *
 * @param manager The security manager object.
 *
 * @return always returns true.
 */
RexxObject *RexxCode::setSecurityManager(RexxObject *manager)
{
    package->setSecurityManager(manager);
    return TheTrueObject;
}


/**
 * Translate a string into code that will be interpreted
 * in the current code context.
 *
 * @param source     The source string.
 * @param lineNumber The line number of the context.
 *
 * @return A translated code object.
 */
RexxCode *RexxCode::interpret(RexxString *source, size_t lineNumber)
{
    return LanguageParser::translateInterpret(source, package, labels, lineNumber);
}


/**
 * Add an instruction to the end of this code object.
 *
 * @param i      The instruction to add
 * @param m      The maximum stack size required by this instruction.
 * @param v      The size of the variables frame.
 */
void RexxCode::addInstruction(RexxInstruction *i, size_t m, size_t v)
{
    // first instruction? just set the chain head
    if (start == OREF_NULL)
    {
        start = i;
        // we have a single instruction, so that is the bounds of the source
        location.setLocation(i->getLocation());
    }
    // run the chain and add this to the end
    else
    {
        RexxInstruction *current = start;
        while (!current->isLast())
        {
            current = current->next();
        }

        current->setNext(i);
        // update the bounds of the source location.
        location.setLocation(i->getLocation());
    }

    // the max stack needs to be the largest value required by any of the
    // instructions. This is managed by the parser as the constants are
    // processed, so just take the latest value.
    maxStack = m + MINIMUM_STACK_FRAME;
    // we also need to variable dictionary frame size.
    vdictSize = v;
}

