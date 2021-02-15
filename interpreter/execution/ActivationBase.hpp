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
/******************************************************************************/
/* REXX Kernel                                                                */
/*                                                                            */
/* Base class for the activation types                                        */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ActivationBase
#define Included_ActivationBase

#include "Numerics.hpp"

class RexxClass;
class Activity;
class BaseExecutable;
class DirectoryClass;
class MessageClass;
class SecurityManager;

/**
 * Base class for the different activation types.  This
 * defines the common interface that the activation subclasses
 * must implement.
 */
class ActivationBase : public RexxInternalObject
{
public:

    // guard scopy settings
    typedef enum
    {
        SCOPE_RELEASED = 0,
        SCOPE_RESERVED = 1,
    } GuardStatus;

    inline ActivationBase() {;};
    inline ActivationBase(RESTORETYPE restoreType) { ; };
    virtual RexxObject  *dispatch() {return NULL;};
    virtual wholenumber_t digits() {return Numerics::DEFAULT_DIGITS;};
    virtual wholenumber_t fuzz() {return Numerics::DEFAULT_FUZZ;};
    virtual bool form() {return Numerics::DEFAULT_FORM;};
    virtual const NumericSettings *getNumericSettings() { return Numerics::getDefaultSettings(); }
    virtual RexxActivation *getRexxContext() { return OREF_NULL; }
    virtual RexxActivation *findRexxContext() { return OREF_NULL; }
    virtual void setDigits(wholenumber_t) {;};
    virtual void setFuzz(wholenumber_t) {;};
    virtual void setForm(bool) {;}
    virtual bool trap(RexxString *, DirectoryClass *) {return false;};
    virtual bool willTrap(RexxString *) {return false;};
    virtual void setObjNotify(MessageClass *) {;};
    virtual void termination(){;};
    virtual SecurityManager *getSecurityManager() = 0;
    virtual bool isForwarded() { return false; }
    virtual bool isStackBase() { return false; }
    virtual bool isRexxContext() { return false; }
    virtual RexxObject *getReceiver() { return OREF_NULL; }
    virtual PackageClass *getPackage() { return OREF_NULL; }

    inline void setPreviousStackFrame(ActivationBase *p) { previous = p; }
    inline ActivationBase *getPreviousStackFrame() { return previous; }
    inline BaseExecutable *getExecutable() { return executable; }
    BaseExecutable *getExecutableObject() { return executable; }

protected:

    ActivationBase *previous;        // previous activation in the chain
    BaseExecutable *executable;      // the executable associated with this activation.
    GuardStatus     objectScope;     // reserve/release state of variables

};


/**
 * Block guard lock on an object instance.  This allows us to
 * grab a guard lock, while ensuring that the lock is released
 * during exception unwind.
 */
class GuardLock
{
public:
    inline GuardLock(Activity *a, RexxObject *o, RexxClass *s) : activity(a), target(o), scope(s)
    {
        // just acquire the scope
        target->guardOn(activity, scope);
    }

    inline ~GuardLock()
    {
        target->guardOff(activity, scope);
    }

private:

    Activity   *activity;    // the activity we're running on
    RexxObject *target;      // the target object for the lock
    RexxClass  *scope;       // the scope of the required guard lock
};


#endif

