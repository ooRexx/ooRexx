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
/* REXX Kernel                                                                */
/*                                                                            */
/* Base class for an exported hash collection type.                           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_HashCollection
#define Included_HashCollection

#include "HashContents.hpp"

/**
 * Base class for all collection types based on hash tables.
 * The implements most of the base methods.
 */
class HashCollection : public RexxObject
{
 public:
    HashCollection(size_t capacity);

    virtual void        live(size_t);
    virtual void        liveGeneral(MarkReason reason);
    virtual void        flatten(RexxEnvelope *);

    virtual RexxObject *unflatten(RexxEnvelope *);
    virtual RexxObject *copy();
    virtual RexxArray  *makeArray();

    virtual HashContent *allocateContents(size_t bucketSize, size_t capacity);
    virtual void validateIndex(RexxInternalObject *index, size_t position);

    void expandContents();
    void expandContents(size_t capacity );
    void ensureCapacity(size_t delta);
    size_t calculateBucketSize(size_t capacity);

    inline RexxInternalObject *resultOrNil(RexxInternalObject *o) { return o != OREF_NULL ? o : TheNilObject; }

    virtual RexxInternalObject *mergeItem(RexxInternalObject *, RexxInternalObject *);
    virtual RexxInternalObject *remove(RexxInternalObject *key);
    virtual RexxInternalObject *get(RexxInternalObject *key);
    virtual RexxInternalObject *put(RexxInternalObject *, RexxInternalObject *);
    virtual RexxInternalObject *add(RexxInternalObject *, RexxInternalObject *);
    virtual RexxInternalObject *removeItem(RexxInternalObject *value);
    virtual RexxInternalObject *hasItem(RexxInternalObject *);
    virtual RexxInternalObject *getIndex(RexxInternalObject * value);

    void          copyValues();
    RexxInternalObject   *removeRexx(RexxInternalObject *);
    RexxInternalObject   *getRexx(RexxInternalObject *);
    RexxInternalObject   *putRexx(RexxInternalObject *, RexxInternalObject *);
    RexxInternalObject   *addRexx(RexxInternalObject *, RexxInternalObject *);
    RexxInternalObject   *hasIndexRexx(RexxInternalObject *);
    RexxInternalObject   *hasItemRexx(RexxInternalObject *);
    RexxInternalObject   *removeItemRexx(RexxInternalObject *value);
    RexxInternalObject   *allAt(RexxInternalObject *);
    RexxInternalObject   *indexRexx(RexxInternalObject * value);
    RexxSupplier *supplier();
    void          merge(HashCollection *);
    RexxArray    *allItems();
    RexxArray    *allIndexes();
    RexxArray    *uniqueIndexes();
    RexxObject   *emptyRexx();
    void          empty();
    RexxObject   *isEmptyRexx();

    inline size_t items() { return contents->items(); }
    inline bool   isEmpty() { return contents->isEmpty(); }

    HashContents *contents;           // the backing hash table collection.
};
#endif
