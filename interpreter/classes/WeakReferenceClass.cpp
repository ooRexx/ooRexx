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
/* Interpreter                                                                */
/*                                                                            */
/* Weak reference support for Rexx memory management                          */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "WeakReferenceClass.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"


RexxClass *WeakReference::classInstance = OREF_NULL;   // singleton class instance

/**
 * Create initial bootstrap objects
 */
void WeakReference::createInstance()
{
    CLASS_CREATE(WeakReference);
}


/**
 * Create a new pointer object
 *
 * @param size   The size of the object.
 *
 * @return The backing memory for an object.
 */
void *WeakReference::operator new(size_t size)
{
    // NB:  We can't just mark this as having no references.  There are operations
    // where we need to have the marking routines called.
    return new_object(size, T_WeakReference);
}


/**
 * Construct a non-notifying weak reference
 *
 * @param r      The referent object.
 */
WeakReference::WeakReference(RexxInternalObject *r)
{
    // NOTE:  We do not use OrefSet here, since we don't want the referenced
    // objects to be added to the old to new table.
    referentObject = r;
    // tell the memory manager that we exist
    memoryObject.addWeakReference(this);
}


/**
 * Construct a non-notifying weak reference
 */
WeakReference::WeakReference()
{
    // NOTE:  We do not use OrefSet here, since we don't want the referenced
    // objects to be added to the old to new table.
    referentObject = OREF_NULL;
    // tell the memory manager that we exist
    memoryObject.addWeakReference(this);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void WeakReference::live(size_t liveMark)
{
    // we need to get called, but we don't do any marking of the referent.
    // we do, however, need to mark the object variables in case this is a subclass.
    memory_mark(objectVariables);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void WeakReference::liveGeneral(MarkReason reason)
{
    // this might be a subclass, so we need to mark the object variables always
    memory_mark_general(objectVariables);
    // these references are only marked during a save or restore image process.
    // NOTE:  WeakReference objects saved in the Rexx image get removed from the
    // weak reference list and just become normal objects.  Since the weak references
    // can only contain references to other oldspace objects at that point, those
    // objects will never get collected.

    if (reason == SAVINGIMAGE || reason == RESTORINGIMAGE)
    {
        memory_mark_general(referentObject);
    }
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void WeakReference::flatten(Envelope *envelope)
{
    setUpFlatten(WeakReference)
    // not normally needed, but this might be a subclass
    flattenRef(objectVariables);
    flattenRef(referentObject);

    // make sure the new version has nulled out list pointers
    newThis->nextReferenceList = OREF_NULL;

    cleanUpFlatten
}


/**
 * Copy a WeakReference object.
 *
 * @return A new weakreference object.
 */
RexxInternalObject *WeakReference::copy()
{
    // this is potentially a subclass and might have some object variable
    // so we need to make an actual copy
    WeakReference *newRef = (WeakReference *)RexxObject::copy();

    // make sure the new version has nulled out list pointers
    newRef->nextReferenceList = OREF_NULL;
    // tell the memory manager to track this instance
    memoryObject.addWeakReference(newRef);

    return newRef;
}


/**
 * unflatten an object
 *
 * @param envelope The owning envelope.
 *
 * @return The replacement object (always just this)
 */
RexxInternalObject *WeakReference::unflatten(Envelope *envelope)
{
    // We add ourselves unconditionally as a weak object, even if the referenent
    // is null, since we could have a new one assigned.
    memoryObject.addWeakReference(this);
    return this;
}


/**
 * clear an object reference.
 */
void WeakReference::clear()
{
    // NOTE:  We do not use OrefSet here, since we don't want the referenced
    // objects to be added to the old to new table.
    referentObject = OREF_NULL;
}


/**
 * Get the value of the weak reference.  If this has been cleared,
 * it returns .nil.  Otherwise, it returns the referenced object.
 *
 * @return The referenced object, or .nil if the object has been garbage
 *         collected.
 */
RexxInternalObject *WeakReference::value()
{
    return resultOrNil(referentObject);
}


/**
 * Create a new weak reference value.
 *
 * @param init_args The new arguments.
 * @param argCount  The count of arguments
 *
 * @return A new WeakReference object.
 */
RexxObject *WeakReference::newRexx(RexxObject **init_args, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxObject *refObj;

    RexxClass::processNewArgs(init_args, argCount, init_args, argCount, 1, refObj, NULL);
    // must have a value
    requiredArgument(refObj, ARG_ONE);
    // create a new weakReference
    RexxObject *newObj = new WeakReference(refObj);
    ProtectedObject p(newObj);

    // handle Rexx class completion
    classThis->completeNewObject(newObj, init_args, argCount);

    return newObj;
}
