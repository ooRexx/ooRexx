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
/* A class for implementing a method dictionary                               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_MethodDictionary
#define Included_MethodDictionary

#include "HashCollection.hpp"

class MethodClass;

/**
 * Exported table class where indexing is done using object
 * identity
 */
class MethodDictionary: public StringHashCollection
{
 public:
     void        *operator new(size_t);
     inline void  operator delete(void *) { ; }

    inline MethodDictionary(RESTORETYPE restoreType) { ; }
           MethodDictionary(size_t capacity = DefaultTableSize);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    RexxInternalObject *copy() override;
    MethodClass *getMethod(RexxString *methodName) { return (MethodClass *)get(methodName); }
    void addMethod(RexxString *methodName, MethodClass *method);
    void replaceMethod(RexxString *methodName, MethodClass *method);
    void replaceMethods(MethodDictionary *source, RexxClass *scope);
    void replaceMethods(MethodDictionary *source, RexxClass *filterScope, RexxClass *scope);
    void replaceMethods(StringTable *source, RexxClass *scope);
    void addMethods(StringTable *source, RexxClass *scope);
    bool removeMethod(RexxString *methodName);
    void hideMethod(RexxString *methodName);
    void removeInstanceMethod(RexxString *name);
    void addInstanceMethod(RexxString *name, MethodClass *method);
    void addInstanceMethods(MethodDictionary *source);
    MethodClass *findSuperMethod(RexxString *name, RexxClass *startScope);
    void setMethodScope(RexxClass *scope);
    SupplierClass *getMethods(RexxClass *scope);
    RexxClass  *resolveSuperScope(RexxClass *);
    void addScope(RexxClass *scope);
    void mergeMethods(MethodDictionary *target);
    void mergeScopes(MethodDictionary *target);
    void merge(MethodDictionary *target);
    bool hasScope(RexxClass *scope);
    ArrayClass *allScopes() { return scopeList; }

    inline bool hasInstanceMethods() { return instanceMethods != OREF_NULL; }

 protected:

    static const size_t DefaultScopeListSize = 10;

    StringTable *instanceMethods;     // any methods defined on this instance
    ArrayClass  *scopeList;           // the list of scope value order use for lookups
    IdentityTable *scopeOrders;       // the scope orders for each class in the hierarchy.
};

#endif
