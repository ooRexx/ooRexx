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
#include "SourceFile.hpp"
#include "ActivityManager.hpp"
#include "RexxActivation.hpp"


/**
 * Allocate storage for a new RexxCode object.
 *
 * @param size   The size of the object.
 *
 * @return Memory for creating this object.
 */
void * RexxCode::operator new(size_t size)
{
    return new_object(size, T_RexxCode);
}


/**
 * Initialize a RexxCode unit that can be attached to
 * a method or routine object and execute Rexx code.
 *
 * @param _source  The source object this was compiled from.
 * @param _start   The first instruction object in the execution chain.
 * @param _labels  The directory of labels in this code unit (can be null)
 * @param maxstack The maximum expression stack required to execute this code.
 * @param variable_index The number of pre-assigned variable
 *                       slots.
 */
RexxCode::RexxCode(RexxSource *_source, RexxInstruction *_start, RexxDirectory *_labels,
     size_t maxstack, size_t  variable_index)
{
    source = _source;
    start = _start;
    labels = _labels;
    maxStack = maxstack;
    vdictSize = variable_index;
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
void RexxCode::call(RexxActivity *activity, RoutineClass *routine, RexxString *msgname, RexxObject**argPtr, size_t argcount, ProtectedObject &result)
{
    // just forward to the more general method
    call(activity, routine, msgname, argPtr, argcount, OREF_SUBROUTINE, OREF_NULL, EXTERNALCALL, result);
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
void RexxCode::call(RexxActivity *activity, RoutineClass *routine, RexxString *routineName,
    RexxObject**argPtr, size_t argcount, RexxString *calltype, RexxString *environment,
    int context, ProtectedObject &result)
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
void RexxCode::run(RexxActivity *activity, MethodClass *method, RexxObject *receiver,
    RexxString *msgname, RexxObject**argPtr, size_t argcount, ProtectedObject &result)
{
    // create a new activation object and push it on the top of the stack.
    RexxActivation *newacta = ActivityManager::newActivation(activity, method, this);
    activity->pushStackFrame(newacta);
    // run the method.  The result is returned via the ProtectedObject reference.
    newacta->run(receiver, msgname, argPtr, argcount, OREF_NULL, result);
    // yield control now.
    activity->relinquish();
}


/**
 * Extract from the source code for the span represented
 * by this code block.
 *
 * @return An array of the code source lines.
 */
RexxArray *RexxCode::getSource()
{
    // the source package handles this.
    return source->extractSourceLines(location);
}


/**
 * Get the code program name.
 *
 * @return The string name of the program.
 */
RexxString * RexxCode::getProgramName()
{
    return source->getProgramName();
}


/**
 * Set a security manager on this code object (actually sets it on the package.)
 *
 * @param manager The security manager object.
 *
 * @return always returns true.
 */
RexxObject *RexxCode::setSecurityManager(RexxObject *manager)
/******************************************************************************/
/* Function:  Associate a security manager with a method's source             */
/******************************************************************************/
{
    source->setSecurityManager(manager);
    return TheTrueObject;
}

