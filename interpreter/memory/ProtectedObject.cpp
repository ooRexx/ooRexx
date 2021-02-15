/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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

#include "RexxCore.h"
#include "ProtectedObject.hpp"
#include "Activity.hpp"
#include "ArrayClass.hpp"
#include "ActivityManager.hpp"


/**
 * Default contstructor for a ProtectedBase object.
 */
ProtectedBase::ProtectedBase()
{
    // save the activity
    activity = ActivityManager::currentActivity;

    // it would be better to have the activity class do this, but because
    // we're doing this with inline methods, we run into a bit of a
    // circular reference problem

    // NOTE:  ProtectedObject gets used in a few places during image
    // restore before we have a valid activity.  If we don't have
    // one, then just assume this will be safe.
    if (activity != OREF_NULL)
    {
        next = activity->protectedObjects;
        activity->protectedObjects = this;
    }
}


/**
 * Create a ProtectedBase object with an explicitly specified activity.
 *
 * @param a      The current activity.
 */
ProtectedBase::ProtectedBase(Activity *a) : activity(a)
{
    // it would be better to have the activity class do this, but because
    // we're doing this with inline methods, we run into a bit of a
    // circular reference problem

    // NOTE:  ProtectedObject gets used in a few places during image
    // restore before we have a valid activity.  If we don't have
    // one, then just assume this will be safe.
    if (activity != OREF_NULL)
    {
        next = activity->protectedObjects;
        activity->protectedObjects = this;
    }
}


/**
 * Destructor for a ProtectedBase object.
 */
ProtectedBase::~ProtectedBase()
{
    // remove ourselves from the list.

    // NOTE:  ProtectedObject gets used in a few places during image
    // restore before we have a valid activity.  If we don't have
    // one, then just assume this will be safe.
    if (activity != OREF_NULL)
    {
        activity->protectedObjects = next;
    }
}


/**
 * Add an object to a protected set.
 *
 * @param o      The new object.
 */
void ProtectedSet::add(RexxInternalObject *o)
{
    // first one we've added?
    if (protectedObject == OREF_NULL)
    {
        protectedObject = new_array();
    }
    ArrayClass *saveTable = (ArrayClass *)protectedObject;
    saveTable->append(o);
}

