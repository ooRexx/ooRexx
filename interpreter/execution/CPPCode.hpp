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
#ifndef Included_CPPCode
#define Included_CPPCode


#include "MethodClass.hpp"


/**
 * Class for a method-wrappered CPP internal method.
 */
class CPPCode : public BaseCode
{
public:
    void *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) { return ptr; };
    inline void operator delete(void *) { }
    inline void operator delete(void *, void *) { }
    CPPCode(size_t, PCPPM, size_t);
    inline CPPCode(RESTORETYPE restoreType) { ; };
    void liveGeneral(int reason);
    RexxObject *unflatten(RexxEnvelope *envelope);

    void run(RexxActivity *, RexxMethod *, RexxObject *, RexxString *, RexxObject **, size_t, ProtectedObject &);

    static CPPCode *resolveExportedMethod(const char *, PCPPM targetMethod, size_t argcount);
    // The table of exported methods.
    static PCPPM exportedMethods[];

protected:
    uint16_t   methodIndex;           // kernel method number
    uint16_t   argumentCount;         // argument count
    PCPPM      cppEntry;              // C++ Method entry point.
};


/**
 * Class for an attribute getter method
 */
class AttributeGetterCode : public BaseCode
{
public:
    void *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) { return ptr; };
    inline void operator delete(void *) { }
    inline void operator delete(void *, void *) { }
    inline AttributeGetterCode(RexxVariableBase *a) { attribute = a; }
    inline AttributeGetterCode(RESTORETYPE restoreType) { ; };
    void live(size_t);
    void liveGeneral(int reason);
    void flatten(RexxEnvelope*);

    void run(RexxActivity *, RexxMethod *, RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &);

protected:
    RexxVariableBase *attribute;      /* method attribute info             */
};


/**
 * Class for an attribute setter method.
 */
class AttributeSetterCode : public AttributeGetterCode
{
public:
    void *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) { return ptr; };
    inline void operator delete(void *) { }
    inline void operator delete(void *, void *) { }
    inline AttributeSetterCode(RexxVariableBase *a) : AttributeGetterCode(a) { }
    inline AttributeSetterCode(RESTORETYPE restoreType) : AttributeGetterCode(restoreType) { }

    void run(RexxActivity *, RexxMethod *, RexxObject *, RexxString *,  RexxObject **, size_t,  ProtectedObject &);
};


/**
 * Class for a constant retriever method
 */
class ConstantGetterCode : public BaseCode
{
public:
    void *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) { return ptr; };
    inline void operator delete(void *) { }
    inline void operator delete(void *, void *) { }
    inline ConstantGetterCode(RexxObject * v) { constantValue = v; }
    inline ConstantGetterCode(RESTORETYPE restoreType) { }
    void live(size_t);
    void liveGeneral(int reason);
    void flatten(RexxEnvelope*);

    void run(RexxActivity *, RexxMethod *, RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &);

protected:
    RexxObject *constantValue;        // the returned constant value
};


/**
 * Class for a constant retriever method
 */
class AbstractCode : public BaseCode
{
public:
    void *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) { return ptr; };
    inline void operator delete(void *) { }
    inline void operator delete(void *, void *) { }
    inline AbstractCode() { }
    inline AbstractCode(RESTORETYPE restoreType) { }

    void run(RexxActivity *, RexxMethod *, RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &);
};

#endif
