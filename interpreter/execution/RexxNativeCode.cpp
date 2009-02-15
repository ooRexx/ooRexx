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
/* REXX Kernel                                         RexxNativeMethod.cpp   */
/*                                                                            */
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxNativeCode.hpp"
#include "SourceFile.hpp"
#include "PackageManager.hpp"
#include <ctype.h>


RexxNativeCode::RexxNativeCode(RexxString *_package, RexxString *_name)
{
    // and this is the information needed to resolve this again after an
    // image restore
    OrefSet(this, this->package, _package);
    OrefSet(this, this->name, _name);
    // this will be set later, if available
    OrefSet(this, this->source, OREF_NULL);
}


void RexxNativeCode::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->package);
    memory_mark(this->name);
    memory_mark(this->source);
}


void RexxNativeCode::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark_general(this->package);
    memory_mark_general(this->name);
    memory_mark_general(this->source);
}


void RexxNativeCode::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxNativeMethod)
   flatten_reference(newThis->package, envelope);
   flatten_reference(newThis->name, envelope);
   flatten_reference(newThis->source, envelope);
  cleanUpFlatten
}


/**
 * Resolve a class in the context of this code object.
 *
 * @param className The name of the desired class.
 *
 * @return The returned class.
 */
RexxClass *RexxNativeCode::findClass(RexxString *className)
{
    // if there is a source object attached, have it resolve things.  Otherwise, go back to the default.
    if (source != OREF_NULL)
    {
        return source->findClass(className);
    }
    return BaseCode::findClass(className);
}


/**
 * Set a source object into a native code context.  If the
 * object is already set, then this returns a copy of the code object.
 *
 * @param s      The new source object.
 *
 * @return Either the same object, or a new copy of the code object.
 */
BaseCode *RexxNativeCode::setSourceObject(RexxSource *s)
{
    if (source == OREF_NULL)
    {
        OrefSet(this, this->source, s);
        return this;
    }
    else
    {
        RexxNativeCode *codeCopy = (RexxNativeCode *)this->copy();
        OrefSet(codeCopy, codeCopy->source, s);
        return codeCopy;
    }
}


/**
 * Get the source object backing this code instance.  If created from
 * a directive, this will be the source package containing the
 * directive.
 *
 * @return The parent source instance.
 */
RexxSource *RexxNativeCode::getSourceObject()
{
    return source;
}


/**
 * Get the security manager associated with native code.  Generally,
 * only native methods and routines defined with directives
 * will have an associated security manager.
 *
 * @return The source security manager.
 */
SecurityManager *RexxNativeCode::getSecurityManager()
{
    if (source != OREF_NULL)
    {
        return source->getSecurityManager();
    }
    return OREF_NULL;
}


void RexxNativeMethod::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    // zero the entry point if saving the image
    if (reason == SAVINGIMAGE)
    {
        entry = NULL;
    }
    RexxNativeCode::liveGeneral(reason);
}


void RexxNativeMethod::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxNativeMethod)
   newThis->entry = NULL;
   RexxNativeCode::flatten(envelope);
  cleanUpFlatten
}


void RexxNativeRoutine::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    // zero the entry point if saving the image
    if (reason == SAVINGIMAGE)
    {
        entry = NULL;
    }
    RexxNativeCode::liveGeneral(reason);
}


void RexxNativeRoutine::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxNativeRoutine)
   newThis->entry = NULL;
   RexxNativeCode::flatten(envelope);
  cleanUpFlatten
}


void RegisteredRoutine::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    // zero the entry point if saving the image
    if (reason == SAVINGIMAGE)
    {
        entry = NULL;
    }
    RexxNativeCode::liveGeneral(reason);
}


void RegisteredRoutine::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RegisteredRoutine)
   newThis->entry = NULL;
   RexxNativeCode::flatten(envelope);
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
void RexxNativeMethod::run(RexxActivity *activity, RexxMethod *method, RexxObject *receiver, RexxString *messageName,
    RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // if this is NULL currently, we need to lazy resolve this entry
    if (entry == NULL)
    {
        // have the package manager resolve this for us before we make a call
        entry = PackageManager::resolveMethodEntry(package, name);
    }

    // create a new native activation
    RexxNativeActivation *newNActa = ActivityManager::newNativeActivation(activity);
    activity->pushStackFrame(newNActa);   /* push it on the activity stack     */
                                       /* and go run it                     */
    newNActa->run(method, this, receiver, messageName, argPtr, count, result);
}


void * RexxNativeMethod::operator new(
     size_t      size)                 /* object size                       */
/****************************************************************************/
/* Function:  Create a new Native method                                    */
/****************************************************************************/
{
    return new_object(size, T_NativeMethod);  // Get new object
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
void RexxNativeRoutine::call(RexxActivity *activity, RoutineClass *routine, RexxString *functionName, RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // if this is NULL currently, we need to lazy resolve this entry
    if (entry == NULL)
    {
        // have the package manager resolve this for us before we make a call
        entry = PackageManager::resolveRoutineEntry(package, name);
    }

    // create a new native activation
    RexxNativeActivation *newNActa = ActivityManager::newNativeActivation(activity);
    activity->pushStackFrame(newNActa);   /* push it on the activity stack     */
                                       /* and go run it                     */
    newNActa->callNativeRoutine(routine, this, functionName, argPtr, count, result);
}


void * RexxNativeRoutine::operator new(
     size_t      size)                 /* object size                       */
/****************************************************************************/
/* Function:  Create a new Native method                                    */
/****************************************************************************/
{
    return new_object(size, T_NativeRoutine);  // Get new object
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
void RegisteredRoutine::call(RexxActivity *activity, RoutineClass *routine, RexxString *functionName, RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // if this is NULL currently, we need to lazy resolve this entry
    if (entry == NULL)
    {
        // have the package manager resolve this for us before we make a call
        entry = PackageManager::resolveRegisteredRoutineEntry(package, name);
    }

    // create a new native activation
    RexxNativeActivation *newNActa = ActivityManager::newNativeActivation(activity);
    activity->pushStackFrame(newNActa);   /* push it on the activity stack     */
                                       /* and go run it                     */
    newNActa->callRegisteredRoutine(routine, this, functionName, argPtr, count, result);
}


void * RegisteredRoutine::operator new(
     size_t      size)                 /* object size                       */
/****************************************************************************/
/* Function:  Create a new Native method                                    */
/****************************************************************************/
{
    return new_object(size, T_RegisteredRoutine);  // Get new object
}
