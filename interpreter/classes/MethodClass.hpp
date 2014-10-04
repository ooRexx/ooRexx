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
/* REXX Kernel                                             MethodClass.hpp    */
/*                                                                            */
/* Primitive Kernel Method Class Definitions                                  */
/*                                                                            */
/******************************************************************************/
#ifndef Included_MethodClass
#define Included_MethodClass

#include "RexxCore.h"
#include "BaseExecutable.hpp"
#include "FlagSet.hpp"

class Activity;
class MethodClass;
class ProtectedObject;
class ArrayClass;
class RexxClass;
class PackageClass;


/**
 * Base class for method object.  This is the frontend for
 * The different types of executable code objects.
 */
class MethodClass : public BaseExecutable
{
 public:
    void *operator new(size_t);

    MethodClass(RexxString *name, BaseCode *_code);
    inline MethodClass(RESTORETYPE restoreType) { ; }

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope*);

    void         run(Activity *,  RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &);
    MethodClass *newScope(RexxClass  *);
    void         setScope(RexxClass  *);
    SmartBuffer  *saveMethod();
    RexxObject  *setUnguardedRexx();
    RexxObject  *setGuardedRexx();
    RexxObject  *setPrivateRexx();
    RexxObject  *setProtectedRexx();
    RexxObject  *setSecurityManager(RexxObject *);

    RexxObject  *isGuardedRexx();
    RexxObject  *isPrivateRexx();
    RexxObject  *isProtectedRexx();

    inline bool  isGuarded()      { return !methodFlags[UNGUARDED_FLAG]; }
    inline bool  isPrivate()      { return methodFlags[PRIVATE_FLAG]; }
    inline bool  isProtected()    { return methodFlags[PROTECTED_FLAG]; }
    inline bool  isSpecial()      { return methodFlags.any(PROTECTED_FLAG, PRIVATE_FLAG); }
    inline bool  isConstant()     { return methodFlags[CONSTANT_METHOD]; }
    inline bool  isAttribute()    { return methodFlags[ATTRIBUTE_METHOD]; }

    inline void  setUnguarded()    { methodFlags.set(UNGUARDED_FLAG); }
    inline void  setGuarded()      { methodFlags.reset(UNGUARDED_FLAG); }
    inline void  setPrivate()      { methodFlags.set(PRIVATE_FLAG); }
    inline void  setProtected()    { methodFlags.set(PROTECTED_FLAG); }
    inline void  setUnprotected()  { methodFlags.reset(PROTECTED_FLAG); }
    inline void  setPublic()       { methodFlags.reset(PRIVATE_FLAG); }
    inline void  setConstant()     { methodFlags.set(CONSTANT_METHOD); }
    inline void  setAttribute()    { methodFlags.set(ATTRIBUTE_METHOD); }
           void  setAttributes(bool _private, bool _protected, bool _guarded);
    inline RexxClass *getScope() { return scope; }
    inline bool  isScope(RexxClass *s) {return scope == s;}

    inline BaseCode  *getCode()     { return code; }
    MethodClass  *newRexx(RexxObject **, size_t);
    MethodClass  *newFileRexx(RexxString *);
    MethodClass  *loadExternalMethod(RexxString *name, RexxString *descriptor);

    static MethodClass *newMethodObject(RexxString *, RexxObject *, RexxClass *, RexxObject *);

    static void createInstance();
    static RexxClass *classInstance;

 protected:

    typedef enum
    {
        PRIVATE_FLAG,                    // private method
        UNGUARDED_FLAG,                  // Method can run with GUARD OFF
        PROTECTED_FLAG,                  // method is protected
        ATTRIBUTE_METHOD,                // defined as an attribute method
        CONSTANT_METHOD,                 // defined as a constant method
    } MethodFlags;

    FlagSet<MethodFlags, 32>  methodFlags;  // method status flags
    RexxClass  *scope;                      // pointer to the method scope
};

#endif
