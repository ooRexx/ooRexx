/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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


RexxClass *WeakReference::classInstance = OREF_NULL;   // singleton class instance

void WeakReference::createInstance()
/******************************************************************************/
/* Function:  Create initial bootstrap objects                                */
/******************************************************************************/
{
    CLASS_CREATE(WeakReference, "WeakReference", RexxClass);
}

void *WeakReference::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new pointer object                                     */
/******************************************************************************/
{
                                       /* Get new object                    */
    // NB:  We can't just mark this as having no references.  There are operations
    // where we need to have the marking routines called.
    return new_object(size, T_WeakReference);
}


WeakReference::WeakReference(RexxObject *r)
/******************************************************************************/
/* Function:  Construct a non-notifying weak reference                        */
/******************************************************************************/
{
    // NOTE:  We do not use OrefSet here, since we don't want the referenced
    // objects to be added to the old to new table.
    referentObject = r;
    // tell the memory manager that we exist
    memoryObject.addWeakReference(this);
}


WeakReference::WeakReference()
/******************************************************************************/
/* Function:  Construct a non-notifying weak reference                        */
/******************************************************************************/
{
    // NOTE:  We do not use OrefSet here, since we don't want the referenced
    // objects to be added to the old to new table.
    referentObject = OREF_NULL;
    // tell the memory manager that we exist
    memoryObject.addWeakReference(this);
}


void WeakReference::live(size_t liveMark)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    // we need to get called, but we don't do any marking
}


void WeakReference::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
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

void WeakReference::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(WeakReference)

   flatten_reference(newThis->referentObject, envelope);

   // make sure the new version has nulled out list pointers
   newThis->nextReferenceList = OREF_NULL;

  cleanUpFlatten
}

RexxObject *WeakReference::unflatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  unflatten an object                                             */
/******************************************************************************/
{
    // if we still have a reference to handle, then add this to the
    // tracking list
    if (referentObject != OREF_NULL)
    {
        memoryObject.addWeakReference(this);
    }
    return (RexxObject *)this;           /* return ourself.                   */
}

void WeakReference::clear()
/******************************************************************************/
/* Function:  clear an object reference, and potentially move to notification */
/* queue so the notification object can be "tapped"                           */
/******************************************************************************/
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
RexxObject *WeakReference::value()
{
    if (referentObject == OREF_NULL)
    {
        return TheNilObject;
    }
    return referentObject;
}


RexxObject *WeakReference::newRexx(RexxObject **init_args, size_t argCount)
/******************************************************************************/
/* Arguments: Subclass init arguments                                         */
/* Function:  Create a new string value (used primarily for subclasses)       */
/******************************************************************************/
{
  RexxObject *refObj;                  /* string value                      */

                                       /* break up the arguments            */
  RexxClass::processNewArgs(init_args, argCount, &init_args, &argCount, 1, &refObj, NULL);
  // must have a value
  requiredArgument(refObj, ARG_ONE);
  // create a new weakReference
  RexxObject *newObj = new WeakReference(refObj);
  // override the behaviour in case this is a subclass
  newObj->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
  if (((RexxClass *)this)->hasUninitDefined())
  {
      newObj->hasUninit();
  }

                                       /* Initialize the new instance       */
  newObj->sendMessage(OREF_INIT, init_args, argCount);
  return newObj;                       /* return the new instance           */
}
