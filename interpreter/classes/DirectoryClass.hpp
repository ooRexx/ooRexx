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
/* REXX Kernel                                            DirectoryClass.hpp  */
/*                                                                            */
/* Primitive Directory Class Definitions                                      */
/*                                                                            */
/******************************************************************************/
#ifndef Included_DirectoryClass
#define Included_DirectoryClass

#include "HashCollection.hpp"

class DirectoryClass : public StringHashCollection
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { ; }

    inline DirectoryClass(RESTORETYPE restoreType) { ; }
           DirectoryClass(size_t capacity = HashCollection::DefaultTableSize) : methodTable(OREF_NULL), unknownMethod(OREF_NULL), StringHashCollection(capacity) { }

    RexxObject *newRexx(RexxObject **, size_t);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    virtual RexxInternalObject *copy();

    // virtual method overrides of the base collection class.
    virtual size_t items();
    virtual SupplierClass *supplier();
    virtual ArrayClass *allIndexes();
    virtual ArrayClass *allItems();
    virtual bool hasIndex(RexxInternalObject *indexName);
    virtual RexxInternalObject *remove(RexxInternalObject *entryname);
    virtual RexxInternalObject *get(RexxInternalObject *index);
    virtual void put(RexxInternalObject *value, RexxInternalObject *index);
    virtual void empty();
    virtual RexxInternalObject *getIndex(RexxInternalObject *target);
    virtual bool hasItem(RexxInternalObject *target);
    virtual RexxInternalObject *removeItem(RexxInternalObject *target);

    // stubs for additional exported directory methods.
    RexxInternalObject *setMethodRexx(RexxString *entryname, MethodClass *methodobj);
    RexxInternalObject *unsetMethodRexx(RexxString *entryname);

    // some private helper methods.
    RexxInternalObject *methodTableValue(RexxInternalObject *index);
    RexxInternalObject *unknownValue(RexxInternalObject *index);

    StringTable *methodTable;            // table of added methods
    MethodClass *unknownMethod;          // unknown method entry

    static void createInstance();
    // singleton class instance;
    static RexxClass *classInstance;
};

inline DirectoryClass *new_directory(size_t capacity = HashCollection::DefaultTableSize) { return new DirectoryClass(capacity); }

#endif
