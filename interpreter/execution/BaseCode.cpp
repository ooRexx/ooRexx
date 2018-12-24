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
/* REXX Kernel                                                                */
/*                                                                            */
/* Base classes for executable objects                                        */
/*                                                                            */
/******************************************************************************/

#include "BaseCode.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"
#include "MethodClass.hpp"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "RexxActivation.hpp"




/**
 * Run this code as a method invocation.
 *
 * @param activity  The current activity.
 * @param method    The method we're invoking.
 * @param receiver  The method target object.
 * @param msgname   The name the method was invoked under.
 * @param argCount  The count of arguments.
 * @param arguments The argument pointer.
 * @param result    The returned result.
 */
void BaseCode::run(Activity *activity, MethodClass *method, RexxObject *receiver, RexxString *msgname, RexxObject **arguments, size_t argCount, ProtectedObject &result)
{
    // The subcasses decide which of run and call are allowed
    reportException(Error_Interpretation);
}


/**
 * Invoke a code element as a call target.  This form is generally
 * only used for calls from Rexx code to Rexx code or for top level
 * program invocation.
 *
 * @param activity  The activity we're running under.
 * @param msgname   The name of the program or name used to invoke the routine.
 * @param arguments The arguments to the method.
 * @param argcount  The count of arguments.
 * @param ct        The call context.
 * @param env       The current address environment.
 * @param context   The type of call being made (program call, internal call, interpret,
 *                  etc.)
 * @param result    The returned result.
 */
void BaseCode::call(Activity *activity, RoutineClass *routine, RexxString *msgname, RexxObject **arguments, size_t argcount, RexxString *ct, RexxString *env, ActivationContext context, ProtectedObject &result)
{
    // the default for this is the simplified call.   This is used by Rexx code to make calls to
    // both Rexx programs and native routines, so the polymorphism simplifies the processing.
    call(activity, routine, msgname, arguments, argcount, result);
}


/**
 * Simplified call form used for calling from Rexx code to native code.
 *
 * @param activity  The current activity.
 * @param msgname   The name of the call.
 * @param arguments the call arguments.
 * @param argcount  The count of arguments.
 * @param result    The returned result.
 */
void BaseCode::call(Activity *activity, RoutineClass *routine, RexxString *msgname, RexxObject **arguments, size_t argcount, ProtectedObject &result)
{
    // The subcasses decide which of run and call are allowed
    reportException(Error_Interpretation);
}


/**
 * Return source informaton for a BaseCode object.  If not
 * representing an element in a source file, this returns
 * an empty array.
 *
 * @return A null array.
 */
ArrayClass *BaseCode::getSource()
{
                                       /* this is always a null array       */
    return (ArrayClass *)TheNullArray->copy();
}


/**
 * Set the security manager in the code source context.
 *
 * @param manager The new security manager.
 *
 * @return Returns true if the manager could be set.  Non-Rexx code objects
 *         just return false unconditionally.
 */
RexxObject *BaseCode::setSecurityManager(RexxObject *manager)
{
    // the default is just to return a failure
    return TheFalseObject;
}


/**
 * Retrieve the source object associated with a code object.
 *
 * @return
 */
PackageClass *BaseCode::getPackageObject()
{
    return package;
}


/**
 * Default class resolution...which only looks in the environment
 * or .local.
 *
 * @param className The target class name.
 *
 * @return The resolved class object, or OREF_NULL if this is not known.
 */
RexxClass *BaseCode::findClass(RexxString *className)
{
    RexxClass *classObject;
    RexxObject *t = OREF_NULL;   // required for the findClass call

    // if we have a package set, search the packge first
    if (package != OREF_NULL)
    {
        classObject = package->findClass(className, t);
        if (classObject != OREF_NULL)
        {
            return classObject;
        }
    }

    // the interpreter class handles the default lookups
    return Interpreter::findClass(className);
}



/**
 * Set a source object into a code context.  The default
 * implementation is just to return the same object without
 * setting a source.  This is used mostly for attaching a source
 * context to native code methods and routines defined on
 * directives.
 *
 * @param s      The new source object.
 *
 * @return Either the same object, or a new copy of the code object.
 */
BaseCode *BaseCode::setPackageObject(PackageClass *s)
{
    // set in the base field
    setField(package, s);
    return this;
}


/**
 * Retrieve the package associated with a code object.  Returns
 * OREF_NULL if this code object doesn't have a package source.
 *
 * @return The associated package, or OREF_NULL.
 */
PackageClass *BaseCode::getPackage()
{
    return getPackageObject();
}


/**
 * Detach the source from a code object.
 */
ProgramSource *BaseCode::detachSource()
{
    if (package != OREF_NULL)
    {
        return package->detachSource();
    }
    return OREF_NULL;
}


/**
 * Attach the code to a program source object.
 *
 * @param s      The source program source object.
 */
void BaseCode::attachSource(ProgramSource *s)
{
    if (package != OREF_NULL)
    {
        package->attachSource(s);
    }
}
