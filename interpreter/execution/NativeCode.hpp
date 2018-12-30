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
/* REXX Kernel                                             NativeMethod.hpp   */
/*                                                                            */
/* Primitive Native Code Class Definitions                                    */
/*                                                                            */
/******************************************************************************/
#ifndef Included_NativeCode
#define Included_NativeCode

#include "MethodClass.hpp"


/**
 * Base class for external methods and routines written in C++
 */
class NativeCode : public BaseCode
{
  public:

    inline NativeCode() { }
    NativeCode(RexxString *p, RexxString *n);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *envelope) override;

    BaseCode  *setPackageObject(PackageClass *s) override;

    SecurityManager *getSecurityManager();

protected:

    RexxString   *packageName;         // the package name
    RexxString   *name;                // the mapped method name
};


/**
 * An external method written in C++
 */
class NativeMethod : public NativeCode
{
  public:
    void        *operator new(size_t size);
    inline void  operator delete(void *) { ; }

    inline NativeMethod(RESTORETYPE restoreType) { ; };
    inline NativeMethod(RexxString *p, RexxString *n, PNATIVEMETHOD e) : NativeCode(p, n), entry(e) { }

    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *envelope) override;

    inline PNATIVEMETHOD getEntry() { return entry; }

    void run(Activity *activity, MethodClass *method, RexxObject *receiver, RexxString *messageName,
        RexxObject **argPtr, size_t count, ProtectedObject &result) override;

protected:

    PNATIVEMETHOD entry;               // method entry point.
};


/**
 * Base class for a native code routine.  The different
 * routine types subclass this.
 */
class BaseNativeRoutine : public NativeCode
{
  public:

    inline BaseNativeRoutine() { }
    inline BaseNativeRoutine(RexxString *p, RexxString *n) : NativeCode(p, n) { }
};


/**
 * An external routine written in C++
 */
class NativeRoutine : public BaseNativeRoutine
{
  public:
    void        *operator new(size_t size);
    inline void  operator delete(void *) { ; }

    inline NativeRoutine(RESTORETYPE restoreType) { ; };
    inline NativeRoutine(RexxString *p, RexxString *n, PNATIVEROUTINE e) : BaseNativeRoutine(p, n), entry(e) { }

    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *envelope) override;

    inline PNATIVEROUTINE getEntry() { return entry; }

    void call(Activity *, RoutineClass *, RexxString *, RexxObject **, size_t, ProtectedObject &) override;

protected:

    PNATIVEROUTINE entry;               // method entry point.
};


/**
 * A legacy-style external routine.
 */
class RegisteredRoutine : public BaseNativeRoutine
{
  public:
    void        *operator new(size_t size);
    inline void  operator delete(void *) { ; }

    inline RegisteredRoutine(RESTORETYPE restoreType) { ; };
    RegisteredRoutine(RexxString *n, RexxRoutineHandler *e)  : BaseNativeRoutine(OREF_NULL, n), entry(e) { }
    RegisteredRoutine(RexxString *p, RexxString *n, RexxRoutineHandler *e)  : BaseNativeRoutine(p, n), entry(e) { }

    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *envelope) override;

    void call(Activity *, RoutineClass *, RexxString *, RexxObject **, size_t, ProtectedObject &) override;

    inline RexxRoutineHandler *getEntry() { return entry; }

protected:

    RexxRoutineHandler *entry;          // method entry point.
};

#endif
