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
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "NativeActivation.hpp"
#include "NativeCode.hpp"
#include "PackageManager.hpp"
#include "PackageClass.hpp"
#include "ActivityManager.hpp"


/**
 * Construct a NativeCode object.
 *
 * @param _package The name of the external library package this is created
 *                 from.
 * @param _name    The name of the package entry.
 */
NativeCode::NativeCode(RexxString *_package, RexxString *_name)
{
    // and this is the information needed to resolve this again after an
    // image restore
    packageName = _package;
    name = _name;
    // this will be set later, if available
    package = OREF_NULL;
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void NativeCode::live(size_t liveMark)
{
    memory_mark(packageName);
    memory_mark(name);
    memory_mark(package);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void NativeCode::liveGeneral(MarkReason reason)
{
    // if we're getting ready to save the image, replace the source
    // package with the global REXX package
    if (reason == PREPARINGIMAGE)
    {
        package = TheRexxPackage;
    }

    memory_mark_general(packageName);
    memory_mark_general(name);
    memory_mark_general(package);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void NativeCode::flatten(Envelope *envelope)
{
    setUpFlatten(NativeMethod)

    flattenRef(packageName);
    flattenRef(name);
    flattenRef(package);

    cleanUpFlatten
}


/**
 * Set a source object into a native code context.  If the
 * object is already set, then this returns a copy of the code object.
 *
 * @param s      The new source object.
 *
 * @return Either the same object, or a new copy of the code object.
 */
BaseCode *NativeCode::setPackageObject(PackageClass *s)
{
    if (package == OREF_NULL)
    {
        setField(package, s);
        return this;
    }
    else
    {
        // we're attaching a code object to a new package...make a copy
        // and return that instead.
        NativeCode *codeCopy = (NativeCode *)copy();
        codeCopy->package = s;
        return codeCopy;
    }
}


/**
 * Get the security manager associated with native code.  Generally,
 * only native methods and routines defined with directives
 * will have an associated security manager.
 *
 * @return The source security manager.
 */
SecurityManager *NativeCode::getSecurityManager()
{
    if (package != OREF_NULL)
    {
        return package->getSecurityManager();
    }
    return OREF_NULL;
}


/**
 * Allocate storage for a new NativeMethod object.
 *
 * @param size   The size of the object.
 *
 * @return A block of storage for creating this object.
 */
void * NativeMethod::operator new(size_t size)
{
    return new_object(size, T_NativeMethod);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void NativeMethod::liveGeneral(MarkReason reason)
{
    // zero the entry point if saving the image
    if (reason == SAVINGIMAGE)
    {
        entry = NULL;
    }
    NativeCode::liveGeneral(reason);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void NativeMethod::flatten(Envelope *envelope)
{
    setUpFlatten(NativeMethod)

    // zero the entry point address when being flattened.
    newThis->entry = NULL;
    NativeCode::flatten(envelope);

    cleanUpFlatten
}


/**
 * Allocate storage for creating a new NativeRoutine
 * object.
 *
 * @param size   The size of the object.
 *
 * @return Initialized storage for a new NativeRoutine.
 */
void *NativeRoutine::operator new(size_t size)
{
    return new_object(size, T_NativeRoutine);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void NativeRoutine::liveGeneral(MarkReason reason)
{
    // zero the entry point if saving the image
    if (reason == SAVINGIMAGE)
    {
        entry = NULL;
    }
    NativeCode::liveGeneral(reason);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void NativeRoutine::flatten(Envelope *envelope)
{
    setUpFlatten(NativeRoutine)

    newThis->entry = NULL;
    NativeCode::flatten(envelope);

    cleanUpFlatten
}


/**
 * Allocate memory for a new native routine.
 *
 * @param size   The size of the object.
 *
 * @return Storage for the new object.
 */
void *RegisteredRoutine::operator new(size_t size)
{
    return new_object(size, T_RegisteredRoutine);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void RegisteredRoutine::liveGeneral(MarkReason reason)
{
    // zero the entry point if saving the image
    if (reason == SAVINGIMAGE)
    {
        entry = NULL;
    }
    NativeCode::liveGeneral(reason);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void RegisteredRoutine::flatten(Envelope *envelope)
{
    setUpFlatten(RegisteredRoutine)

    newThis->entry = NULL;
    NativeCode::flatten(envelope);

    cleanUpFlatten
}


/**
 * Run a method call (vs a straight program call).
 *
 * @param activity The current activity.
 * @param method   The method we're attached to.
 * @param receiver The method receiver object (the "self" object).
 * @param messageName
 *                 The name of the message used to invoke the method.
 * @param count    The count of arguments.
 * @param argPtr   The pointer to the arguments.
 * @param result   The protected object used to return the result.
 */
void NativeMethod::run(Activity *activity, MethodClass *method, RexxObject *receiver, RexxString *messageName,
    RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // if this is NULL currently, we need to lazy resolve this entry
    if (entry == NULL)
    {
        // have the package manager resolve this for us before we make a call
        entry = PackageManager::resolveMethodEntry(packageName, name);
    }

    // create a new native activation
    NativeActivation *newNActa = ActivityManager::newNativeActivation(activity);
    activity->pushStackFrame(newNActa);   /* push it on the activity stack     */
                                       /* and go run it                     */
    newNActa->run(method, this, receiver, messageName, argPtr, count, result);
}


/**
 * Run a method call (vs a straight program call).
 *
 * @param activity The current activity.
 * @param functionName
 *                 The name of the message used to invoke the method.
 * @param count    The count of arguments.
 * @param argPtr   The pointer to the arguments.
 * @param result   The protected object used to return the result.
 */
void NativeRoutine::call(Activity *activity, RoutineClass *routine, RexxString *functionName, RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // if this is NULL currently, we need to lazy resolve this entry
    if (entry == NULL)
    {
        // have the package manager resolve this for us before we make a call
        entry = PackageManager::resolveRoutineEntry(packageName, name);
    }

    // create a new native activation
    NativeActivation *newNActa = ActivityManager::newNativeActivation(activity);
    activity->pushStackFrame(newNActa);   /* push it on the activity stack     */
                                       /* and go run it                     */
    newNActa->callNativeRoutine(routine, this, functionName, argPtr, count, result);
}


/**
 * Run a method call (vs a straight program call).
 *
 * @param activity The current activity.
 * @param functionName
 *                 The name of the message used to invoke the method.
 * @param count    The count of arguments.
 * @param argPtr   The pointer to the arguments.
 * @param result   The protected object used to return the result.
 */
void RegisteredRoutine::call(Activity *activity, RoutineClass *routine, RexxString *functionName, RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // if this is NULL currently, we need to lazy resolve this entry
    if (entry == NULL)
    {
        // have the package manager resolve this for us before we make a call
        entry = PackageManager::resolveRegisteredRoutineEntry(packageName, name);
    }

    // create a new native activation
    NativeActivation *newNActa = ActivityManager::newNativeActivation(activity);
    activity->pushStackFrame(newNActa);   /* push it on the activity stack     */
                                       /* and go run it                     */
    newNActa->callRegisteredRoutine(routine, this, functionName, argPtr, count, result);
}
