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
/* Base class for an exported hash collection type.                           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_HashCollection
#define Included_HashCollection

#include "HashContents.hpp"


/**
 * Base class for all collection types based on hash tables.
 * The implements most of the base methods, and uses object
 * identity comparison semantics.
 */
class HashCollection : public RexxObject
{
 public:
    inline HashCollection() { ; }

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    RexxInternalObject *unflatten(Envelope *) override;
    RexxInternalObject *copy() override;
    ArrayClass *makeArray() override;

    virtual HashContents *allocateContents(size_t bucketSize, size_t capacity) = 0;
    virtual void validateIndex(RexxObject *&index);
    virtual void validateValueIndex(RexxObject *&value, RexxObject *&index);
    virtual bool requiresRehash() { return true; }

    void initialize(size_t capacity = DefaultTableSize);
    void copyValues();
    void expandContents();
    void expandContents(size_t capacity );
    void ensureCapacity(size_t delta);
    void checkFull();
    static size_t calculateBucketSize(size_t capacity);
    HashContents::TableIterator iterator();
    HashContents::ReverseTableIterator reverseIterator();
    HashContents::IndexIterator iterator(RexxInternalObject *index);

    // virtual methods that subclasses can override to provide special behaviour.
    virtual void mergeItem(RexxInternalObject *, RexxInternalObject *);
    virtual RexxInternalObject *remove(RexxInternalObject *key);
    virtual RexxInternalObject *get(RexxInternalObject *key);
    virtual void put(RexxInternalObject *, RexxInternalObject *);
    virtual RexxInternalObject *removeItem(RexxInternalObject *value);
    virtual bool hasIndex(RexxInternalObject *);
    virtual bool hasItem(RexxInternalObject *);
    virtual RexxInternalObject *getIndex(RexxInternalObject * value);
    virtual void empty();
    virtual SupplierClass *supplier();
    virtual ArrayClass    *allItems();
    virtual ArrayClass    *allIndexes();
    virtual size_t items() { return contents->items(); }

    // the Exported Rexx methods.  These methods cannot be virtual methods.
    RexxObject           *initRexx(RexxObject *);
    RexxObject           *removeRexx(RexxObject *);
    RexxObject           *getRexx(RexxObject *);
    RexxObject           *putRexx(RexxObject *, RexxObject *);
    RexxObject           *addRexx(RexxObject *, RexxObject *);
    RexxObject           *hasIndexRexx(RexxObject *);
    RexxObject           *hasItemRexx(RexxObject *);
    RexxObject           *removeItemRexx(RexxObject *value);
    ArrayClass           *allAtRexx(RexxObject *);
    RexxObject           *indexRexx(RexxObject * value);
    SupplierClass        *supplierRexx();
    ArrayClass           *allItemsRexx();
    ArrayClass           *allIndexesRexx();
    RexxObject           *emptyRexx();
    RexxObject           *isEmptyRexx();
    RexxObject           *itemsRexx();

    void           merge(HashCollection *);
    void           putAll(HashCollection *);
    void           reHash();
    ArrayClass    *uniqueIndexes();

    // do this based off of items(), which can be overridden
    inline bool   isEmpty() { return items() == 0; }

    // minimum bucket size we'll work with
    static const size_t MinimumBucketSize = 17;

    // Our default bucket size (currently the same as the minimum, but
    // it does not need to be)
    static const size_t DefaultTableSize = 17;

    HashContents *contents;           // the backing hash table collection.

protected:

    // These are protected because we want to enable this on a case-by-case basis.
    // in appropriate subclasses.
    virtual void add(RexxInternalObject *, RexxInternalObject *);
    virtual void addFront(RexxInternalObject *, RexxInternalObject *);
};


/**
 * A hash collection subclass for all classes where
 * equality is based on object identity
 */
class IdentityHashCollection : public HashCollection
{
public:
            IdentityHashCollection(size_t capacity);
    inline  IdentityHashCollection() { ; }

    HashContents *allocateContents(size_t bucketSize, size_t capacity) override;
};


/**
 * A hash collection subclass for all classes where
 * equality is based on object equality rather than identity.
 */
class EqualityHashCollection : public HashCollection
{
public:
            EqualityHashCollection(size_t capacity);
    inline  EqualityHashCollection() { ; }

    HashContents *allocateContents(size_t bucketSize, size_t capacity) override;
};


/**
 * A hash collection subclass for all classes where
 * equality index equality is based on string comparisons and
 * index values are restricted to being strings.
 */
class StringHashCollection : public HashCollection
{
public:
            StringHashCollection(size_t capacity);
    inline  StringHashCollection() { ; }

    HashContents *allocateContents(size_t bucketSize, size_t capacity) override;
    void validateIndex(RexxObject *&index) override;
    // string collections don't require a rehash
    bool requiresRehash() override { return false; }

    // additional string oriented lookup functions
    // base implementations of extra directory methods.
    virtual bool hasEntry(RexxString *entryName);
    virtual void setEntry(RexxString *entryname, RexxInternalObject *entryobj);
    virtual RexxInternalObject *entry(RexxString *index);
    virtual RexxInternalObject *removeEntry(RexxString *index);
    virtual RexxObject *unknown(RexxString *msgname, RexxObject **arguments, size_t count);
    void processUnknown(RexxErrorCodes error, RexxString *, RexxObject **, size_t, ProtectedObject &) override;

    // Rexx stubs for these additional functions.
    RexxObject *entryRexx(RexxObject *entryName);
    RexxObject *hasEntryRexx(RexxObject *entryName);
    RexxObject *setEntryRexx(RexxObject *entryname, RexxObject *entryobj);
    RexxObject *removeEntryRexx(RexxObject *entryName);
    RexxObject *unknownRexx(RexxString *message, ArrayClass  *arguments);
};


/**
 * A hash collection subclass for all classes where
 * only index values are stored in the collection, not
 * index/value pairs (i.e., Set and Bag)
 */
class IndexOnlyHashCollection : public EqualityHashCollection
{
public:
            IndexOnlyHashCollection(size_t capacity) : EqualityHashCollection(capacity) { }
    inline  IndexOnlyHashCollection() { ; }

    void validateValueIndex(RexxObject *&value, RexxObject *&index) override;
    bool hasItem(RexxInternalObject *) override;
    RexxInternalObject *getIndex(RexxInternalObject * value) override;
    void put(RexxInternalObject *v) { HashCollection::put(v, v); }

};
#endif
