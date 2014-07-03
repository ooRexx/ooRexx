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
/* REXX Kernel                                                 BaseCode.hpp   */
/*                                                                            */
/* Class defintions for executable code objects.                              */
/*                                                                            */
/******************************************************************************/
#ifndef Included_BaseCode
#define Included_BaseCode

/**
 * Base class for a code object.  Code objects can be invoked as
 * methods, or called.
 */
class BaseCode : public RexxInternalObject
{
public:
    virtual void run(RexxActivity *, MethodClass *, RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &);
    virtual void call(RexxActivity *, RoutineClass *, RexxString *,  RexxObject **, size_t, RexxString *, RexxString *, int, ProtectedObject &);
    virtual void call(RexxActivity *, RoutineClass *, RexxString *,  RexxObject **, size_t, ProtectedObject &);
    virtual RexxArray *getSource();
    virtual RexxObject *setSecurityManager(RexxObject *manager);
    virtual RexxSource *getSourceObject();
    virtual RexxClass *findClass(RexxString *className);
    virtual BaseCode  *setSourceObject(RexxSource *s);
    virtual PackageClass *getPackage();
};

// pointer to native method function
typedef uint16_t *(RexxEntry *PNATIVEMETHOD)(RexxMethodContext *, ValueDescriptor *);
// pointer to native function function
typedef uint16_t *(RexxEntry *PNATIVEROUTINE)(RexxCallContext *, ValueDescriptor *);
// prototype for a registered function call.
typedef size_t (RexxEntry *PREGISTEREDROUTINE)(const char *, size_t, PCONSTRXSTRING, const char *, PRXSTRING);

/**
 * Base class for all executable objects.  Executable
 * objects and Methods and Routines.
 */
class BaseExecutable : public RexxObject
{
public:
    inline RexxSource *getSourceObject() { return code->getSourceObject(); };
    inline BaseCode   *getCode() { return code; }
    RexxArray  *getSource() { return code->getSource(); }
    PackageClass *getPackage();

    RexxArray *source();
    RexxClass *findClass(RexxString *className);
    BaseExecutable *setSourceObject(RexxSource *s);
    RexxString *getName() { return executableName; }

protected:

    RexxString *executableName;         // the created name of this routine
    BaseCode   *code;                   // the backing code object
};

#endif
