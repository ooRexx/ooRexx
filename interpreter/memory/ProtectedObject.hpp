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
#ifndef ProtectedObject_Included
#define ProtectedObject_Included

#include "RexxActivity.hpp"
#include "ActivityManager.hpp"

class RexxInstruction;

class ProtectedObject
{
friend class RexxActivity;
public:
    inline ProtectedObject() : protectedObject(OREF_NULL)
    {
        // save the activity
        activity = ActivityManager::currentActivity;

        // it would be better to have the activity class do this, but because
        // we're doing this with inline methods, we run into a bit of a
        // circular reference problem
        next = activity->protectedObjects;
        activity->protectedObjects = this;
    }

    inline ProtectedObject(RexxActivity *a) : protectedObject(OREF_NULL), activity(a)
    {
        // it would be better to have the activity class do this, but because
        // we're doing this with inline methods, we run into a bit of a
        // circular reference problem
        next = activity->protectedObjects;
        activity->protectedObjects = this;
    }

    inline ProtectedObject(RexxObject *o) : protectedObject(o), next(NULL)
    {
        // save the activity
        activity = ActivityManager::currentActivity;
        next = activity->protectedObjects;
        activity->protectedObjects = this;
    }

    inline ProtectedObject(RexxObject *o, RexxActivity *a) : protectedObject(o), next(NULL), activity(a)
    {
        next = activity->protectedObjects;
        activity->protectedObjects = this;
    }

    inline ProtectedObject(RexxInternalObject *o) : protectedObject((RexxObject *)o), next(NULL)
    {
        // save the activity
        activity = ActivityManager::currentActivity;
        next = activity->protectedObjects;
        activity->protectedObjects = this;
    }

    inline ProtectedObject(RexxInternalObject *o, RexxActivity *a) : protectedObject((RexxObject *)o), next(NULL), activity(a)
    {
        next = activity->protectedObjects;
        activity->protectedObjects = this;
    }

    inline ~ProtectedObject()
    {
        // remove ourselves from the list and give this object a
        // little hold protection.
        activity->protectedObjects = next;
        if (protectedObject != OREF_NULL)
        {
            holdObject(protectedObject);
        }
    }

    inline ProtectedObject & operator=(RexxObject *o)
    {
        protectedObject = o;
        return *this;
    }

    inline bool operator == (RexxObject *o)
    {
        return protectedObject == o;
    }

    inline bool operator != (RexxObject *o)
    {
        return protectedObject != o;
    }

    // cast conversion operators for some very common uses of protected object.
    inline operator RexxObject *()
    {
        return protectedObject;
    }

    inline operator RexxObjectPtr ()
    {
        return (RexxObjectPtr)protectedObject;
    }

    inline operator RexxString *()
    {
        return (RexxString *)protectedObject;
    }

    inline operator RexxMethod *()
    {
        return (RexxMethod *)protectedObject;
    }

    inline operator RexxArray *()
    {
        return (RexxArray *)protectedObject;
    }

    // this conversion helps the parsing process protect objects
    inline operator RexxInstruction *()
    {
        return (RexxInstruction *)protectedObject;
    }

    inline operator void *()
    {
        return (void *)protectedObject;
    }

protected:
    RexxObject *protectedObject;       // next in the chain of protected object
    ProtectedObject *next;             // the pointer protected by the object
    RexxActivity *activity;            // the activity we're running on
};


class ProtectedSet : public ProtectedObject
{
public:
    inline ProtectedSet() : ProtectedObject() { }
    inline ProtectedSet(RexxActivity *a) : ProtectedObject(a) { }
    inline ~ProtectedSet() { }

    void add(RexxObject *);
};


#endif
