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
#ifndef Included_CPPCode
#define Included_CPPCode


#include "MethodClass.hpp"

class RexxVariableBase;


// pass arguments as pointer/count pair
const size_t A_COUNT   = 127;

/**
 * Class for a method-wrappered CPP internal method.
 */
class CPPCode : public BaseCode
{
public:
    void *operator new(size_t);
    inline void operator delete(void *) { }

    CPPCode(size_t, PCPPM, size_t);
    inline CPPCode(RESTORETYPE restoreType) { ; };

    void live(size_t mark) override;
    void liveGeneral(MarkReason reason) override;

    void run(Activity *, MethodClass *, RexxObject *, RexxString *, RexxObject **, size_t, ProtectedObject &) override;

    static CPPCode *resolveExportedMethod(const char *name, PCPPM targetMethod, size_t argcount, const char* entryPointName);
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
    inline void operator delete(void *) { }

    inline AttributeGetterCode(RexxVariableBase *a) { attribute = a; }
    inline AttributeGetterCode(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope*) override;

    void run(Activity *, MethodClass *, RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &) override;

protected:
    RexxVariableBase *attribute;      // method attribute info
};


/**
 * Class for an attribute setter method.
 */
class AttributeSetterCode : public AttributeGetterCode
{
public:
    void *operator new(size_t);
    inline void operator delete(void *) { }

    inline AttributeSetterCode(RexxVariableBase *a) : AttributeGetterCode(a) { }
    inline AttributeSetterCode(RESTORETYPE restoreType) : AttributeGetterCode(restoreType) { }

    void run(Activity *, MethodClass *, RexxObject *, RexxString *,  RexxObject **, size_t,  ProtectedObject &) override;
};


/**
 * Class for a constant retriever method
 */
class ConstantGetterCode : public BaseCode
{
public:
    void *operator new(size_t);
    inline void operator delete(void *) { }

    inline ConstantGetterCode(RexxString *n, RexxObject * v) { constantName = n; constantValue = v; }
    inline ConstantGetterCode(RESTORETYPE restoreType) { }

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope*) override;

    void run(Activity *, MethodClass *, RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &) override;

    void setValue(RexxObject *v) { constantValue = v; }

protected:
    RexxString *constantName;         // the name of the constant
    RexxObject *constantValue;        // the returned constant value
};


/**
 * Class for a constant retriever method
 */
class AbstractCode : public BaseCode
{
public:
    void *operator new(size_t);
    inline void operator delete(void *) { }

    void live(size_t)override;
    void liveGeneral(MarkReason)override;
    void flatten(Envelope *)override;

    inline AbstractCode() { }
    inline AbstractCode(RESTORETYPE restoreType) { }

    void run(Activity *, MethodClass *, RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &) override;
};


/**
 * Class for a delegate method
 */
class DelegateCode : public BaseCode
{
public:
    void *operator new(size_t);
    inline void operator delete(void *) { }

    inline DelegateCode(RexxVariableBase *a) { attribute = a; }
    inline DelegateCode(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope*) override;

    void run(Activity *, MethodClass *, RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &) override;

protected:
    RexxVariableBase *attribute;      // method attribute info
};

#endif
