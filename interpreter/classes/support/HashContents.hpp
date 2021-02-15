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
/* Backing contents for all hash-based collections                            */
/*                                                                            */
/******************************************************************************/
#ifndef Included_HashContents
#define Included_HashContents

#include "StringClass.hpp"

class HashCollection;

/**
 * Base class for storing the contents of a hash-based
 * collection.  This version bases all index and item
 * searches on object equality.  Subclasses can override these
 * default rules.  This is the backing contents only,
 * there is a separate collection class that implements
 * the collection interface and allocates a backing
 * context.
 */
class HashContents : public RexxInternalObject
{
public:
    // The type for the reference links
    typedef size_t ItemLink;

    // link terminator
    static const ItemLink NoMore = SIZE_MAX;
    // indicates not linked
    static const ItemLink NoLink = SIZE_MAX;


    /**
     * an iterator for iterating over all entries that have a given index.
     */
	class IndexIterator
	{
		friend class HashContents;

	public:
		inline ~IndexIterator() {}

        inline bool isAvailable()  { return position != NoMore; }
        inline RexxInternalObject *value() { return contents->entryValue(position); }
        inline RexxInternalObject *index() { return contents->entryIndex(position); }
        inline void replace(RexxInternalObject *v) { contents->setValue(position, v); }
        inline void next() { contents->nextMatch(indexValue, position); }

	private:
        // constructor for an index iterator
		IndexIterator(HashContents *c, RexxInternalObject *i, ItemLink p)
            : contents(c), indexValue(i), position(p) { }

        HashContents *contents;
        RexxInternalObject *indexValue;
        ItemLink position;
	};


    /**
     * an iterator for iterating over all entries of the table
     */
	class TableIterator
	{
		friend class HashContents;

	public:
        inline TableIterator() : contents(OREF_NULL), position(0), nextBucket(0) { }
		inline ~TableIterator() {}

        inline bool isAvailable()  { return position != NoMore; }
        inline RexxInternalObject *value() { return contents->entryValue(position); }
        inline RexxInternalObject *index() { return contents->entryIndex(position); }
        inline void replace(RexxInternalObject *v) { contents->setValue(position, v); }
        inline void next() { contents->iterateNext(position, nextBucket); }
        inline void removeAndAdvance() { contents->iterateNextAndRemove(position, nextBucket); }

	private:
        // constructor for an index iterator
		TableIterator(HashContents *c, ItemLink p, ItemLink n)
            : contents(c), position(p), nextBucket(n) { }

        HashContents *contents;
        ItemLink position;
        ItemLink nextBucket;
	};


    /**
     * an iterator for iterating over all entries of the table in
     * reverse lookup order.  Used for merging dictionaries where
     * preserving relative order is critical.
     */
	class ReverseTableIterator
	{
		friend class HashContents;

	public:
        inline ReverseTableIterator() : contents(OREF_NULL), position(0), currentBucket(0) { }
		inline ~ReverseTableIterator() {}

        inline bool isAvailable()  { return position != NoMore; }
        inline RexxInternalObject *value() { return contents->entryValue(position); }
        inline RexxInternalObject *index() { return contents->entryIndex(position); }
        inline void replace(RexxInternalObject *v) { contents->setValue(position, v); }
        inline void next() { contents->iterateNextReverse(position, currentBucket); }

	private:
        // constructor for an index iterator
		ReverseTableIterator(HashContents *c, ItemLink p, ItemLink n)
            : contents(c), position(p), currentBucket(n) { }

        HashContents *contents;
        ItemLink position;
        ItemLink currentBucket;
	};

    /**
     * Small helper class for an entry stored in the contents.
     */
    class ContentEntry
    {
    public:
        inline bool isAvailable() { return index == OREF_NULL; }
        // these can only be used when object identity matches are called for...generally
        // just some special purpose things.
        inline bool matches(RexxInternalObject *i, RexxInternalObject *v) { return index == i && value == v; }
        inline bool matches(RexxInternalObject *i) { return index == i; }

        RexxInternalObject *index;           // item index object
        RexxInternalObject *value;           // item value object
        ItemLink next;                       // next item in overflow bucket
    };

    inline HashContents() { ; };
           HashContents(size_t entries, size_t total);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;


    // default index comparison method
    virtual bool isIndexEqual(RexxInternalObject *target, RexxInternalObject *entryIndex)
    {
        // default comparison is object identity
        return target == entryIndex;
    }

    // default item comparison method
    virtual bool isItemEqual(RexxInternalObject *target, RexxInternalObject *entryItem)
    {
        // default comparison is object identity
        return target == entryItem;
    }

    // default index hashing method.  bypass the hash() method and directly use the hash value
    virtual ItemLink hashIndex(RexxInternalObject *index)
    {
        // Note: even though we are using object reference identity to find a match,
        // we use the hash value returned from getHashValue() rather than the identityHash
        // because identityTables stored in the saved image or compiled programs will have
        // a different reference value (and thus a different identifyHash) on restore, meaning
        // that lookups against these tables may fail.
        return (ItemLink)(index->getHashValue() % bucketSize);
    }

    void initializeFreeChain();

    // set the entry values for a position
    void setEntry(ItemLink position, RexxInternalObject *value, RexxInternalObject *index);

    // clear and entry in the chain
    void clearEntry(ItemLink position);

    // update a next entry
    void setNext(ItemLink position, ItemLink next) { entries[position].next = next; }

    // copy an entry contents into another entry
    inline void copyEntry(ItemLink target, ItemLink source)
    {
        // copy all of the information
        setEntry(target, entryValue(source), entryIndex(source));
        entries[target].next = entries[source].next;
    }

    // remove a non-anchor entry from a hash chain
    inline void closeChain(ItemLink position, ItemLink previous)
    {
        entries[previous].next = entries[position].next;
        // move the removed item back to the free chain and
        // clear it out
        returnToFreeChain(position);
    }

    // return an entry to the free chain
    inline void  returnToFreeChain(ItemLink position)
    {
        // clear the entry, then place at the head of the
        // free chain.
        clearEntry(position);
        entries[position].next = freeChain;
        freeChain = position;
    }

    // set the value in an existing entry
    void setValue(ItemLink position, RexxInternalObject *value);

    // perform an index comparison for a position
    inline bool isIndex(ItemLink position, RexxInternalObject *index)
    {
        return isIndexEqual(index, entries[position].index);
    }

    // perform an item comparison for a position
    inline bool isItem(ItemLink position, RexxInternalObject *item)
    {
        return isItemEqual(item, entries[position].value);
    }

    // perform an entry comparison for a position using both index and item value
    inline bool isItem(ItemLink position, RexxInternalObject *index, RexxInternalObject *item)
    {
        return isIndexEqual(index, entries[position].index) && isItemEqual(item, entries[position].value);
    }

    // check if an entry is availabe
    inline bool isAvailable(ItemLink postion)
    {
        return entries[postion].isAvailable();
    }

    // check if an entry is availabe
    inline bool isInUse(ItemLink postion)
    {
        return !entries[postion].isAvailable();
    }

    // step to the next position in the chain
    inline ItemLink nextEntry(ItemLink position)
    {
        return entries[position].next;
    }

    // get the value for an entry
    inline RexxInternalObject *entryValue(ItemLink position)
    {
        return entries[position].value;
    }

    // get the index for an entry
    inline RexxInternalObject *entryIndex(ItemLink position)
    {
        return entries[position].index;
    }

    // test if the table is full
    inline bool isFull()
    {
        return freeChain == NoMore;
    }

    // check if this table can hold an additional number of items (usually used on merge operations)
    inline bool hasCapacity(size_t count)
    {
        return totalSize - itemCount > count;
    }

    // test if a position is a bucket anchor position.
    inline bool isBucketPosition(ItemLink position) { return position < bucketSize; }

    /**
     * Return the total current capacity.
     *
     * @return The total number of items this table can hold.
     */
    inline size_t capacity()
    {
        return totalSize;
    }


    /**
     * Return the bucket size of the contents.
     *
     * @return The hash bucket size
     */
    inline size_t hashBucket()
    {
        return bucketSize;
    }

    // NOTE:  put() is virtual so that specialized hash tables can
    // override put() and change the replace vs. add semantics.
    virtual void put(RexxInternalObject *value, RexxInternalObject *index);

    inline size_t items() { return itemCount; }
    inline bool isEmpty() { return itemCount == 0; }
    void append(RexxInternalObject *value, RexxInternalObject * index, ItemLink position);
    void insert(RexxInternalObject *value, RexxInternalObject * index, ItemLink position);
    RexxInternalObject *remove(RexxInternalObject *index);
    void removeChainLink(ItemLink &position, ItemLink previous);
    bool locateEntry(RexxInternalObject *index, ItemLink &position, ItemLink &previous);
    bool locateEntry(RexxInternalObject *index, RexxInternalObject *item, ItemLink &position, ItemLink &previous);
    bool locateItem(RexxInternalObject *item, ItemLink &position, ItemLink &previous);
    void nextMatch(RexxInternalObject *index, ItemLink &position);
    void iterateNext(ItemLink &position, ItemLink &nextBucket);
    void iterateNextAndRemove(ItemLink &position, ItemLink &nextBucket);
    void iterateNextReverse(ItemLink &position, ItemLink &nextBucket);
    void locateNextBucketEnd(ItemLink &position, ItemLink &currentBucket);
    void locatePreviousEntry(ItemLink &position, ItemLink currentBucket);
    ArrayClass *removeAll(RexxInternalObject *index);
    RexxInternalObject *removeItem(RexxInternalObject *value, RexxInternalObject *index);
    bool hasItem(RexxInternalObject *value, RexxInternalObject *index );
    bool hasItem(RexxInternalObject *item);
    bool hasIndex(RexxInternalObject *index);
    RexxInternalObject *removeItem(RexxInternalObject *item);
    RexxInternalObject *nextItem(RexxInternalObject *value, RexxInternalObject *index);
    RexxInternalObject *get(RexxInternalObject *index);
    ArrayClass  *getAll(RexxInternalObject *index);
    size_t countAllIndex(RexxInternalObject *index, ItemLink &anchorPosition);
    size_t countAllItem(RexxInternalObject *item);
    ArrayClass  *allIndex(RexxInternalObject *item);
    RexxInternalObject *getIndex(RexxInternalObject *item);
    void merge(HashCollection *target);
    void putAll(HashCollection *target);
    void reMerge(HashContents *newHash);
    void mergeItem(RexxInternalObject *, RexxInternalObject *index);
    void mergePut(RexxInternalObject *item, RexxInternalObject *index);
    ArrayClass  *allItems();
    void empty();
    ArrayClass *allIndexes();
    ArrayClass *uniqueIndexes();
    SupplierClass *supplier();
    SupplierClass *supplier(RexxInternalObject *index);
    void reHash(HashContents *newHash);
    void add(RexxInternalObject *item, RexxInternalObject *index);
    void addFront(RexxInternalObject *item, RexxInternalObject *index);
    void copyValues();
    size_t items(RexxInternalObject *item);

    IndexIterator iterator(RexxInternalObject *index);
    TableIterator iterator();
    ReverseTableIterator reverseIterator();

protected:

    size_t   bucketSize;                // size of the hash table
    size_t   totalSize;                 // total size of the table, including the overflow area
    size_t   itemCount;                 // total number of items in the table
    ItemLink freeChain;                 // first free element
    ContentEntry entries[1];            // hash table entries
};


/**
 * A contents class that applies object identity look up
 * for key\item equality.  Used for the identity table
 * class.
 */
class IdentityHashContents : public HashContents
{
public:
           void * operator new(size_t size, size_t capacity);
    inline void  operator delete(void *) { ; }

    inline IdentityHashContents() { ; };
    inline IdentityHashContents(RESTORETYPE restoreType) { ; };
           IdentityHashContents(size_t entries, size_t total) : HashContents(entries, total) { }
};


/**
 * A contents class that applies object equality lookup for
 * key\item equality.  Used for the table and set classes.
 */
class EqualityHashContents : public HashContents
{
public:
           void *operator new(size_t size, size_t capacity);
    inline void  operator delete(void *) { ; }

    inline EqualityHashContents() { ; };
    inline EqualityHashContents(RESTORETYPE restoreType) { ; };
           EqualityHashContents(size_t entries, size_t total) : HashContents(entries, total) { }

    // default index comparison method
    bool isIndexEqual(RexxInternalObject *target, RexxInternalObject *entryIndex) override
    {
        // compare using object equality
        return target->equalValue(entryIndex);
    }

    // default item comparison method
    bool isItemEqual(RexxInternalObject *target, RexxInternalObject *entryItem) override
    {
        // compare using object equality
        return target->equalValue(entryItem);
    }

    // Use the full hash() method processing to determine this.
    ItemLink hashIndex(RexxInternalObject *index) override
    {
        return (ItemLink)(index->hash() % bucketSize);
    }
};


/**
 * A contents class that maps the put semantics to add
 * additional entries under the same index.  Use for the
 * Relation and Bag classes.
 */
class MultiValueContents : public EqualityHashContents
{
public:
           void *operator new(size_t size, size_t capacity);
    inline void  operator delete(void *) { ; }

    inline MultiValueContents() { ; };
    inline MultiValueContents(RESTORETYPE restoreType) { ; };
           MultiValueContents(size_t entries, size_t total) : EqualityHashContents(entries, total) { }

    // remap the put method to the multi-value type
    void put(RexxInternalObject *value, RexxInternalObject *index) override
    {
        addFront(value, index);
    }
};


/**
 * A contents class that is optimized for String lookups key
 * equality.  Used for the Directory class.
 */
class StringHashContents : public EqualityHashContents
{
public:
           void *operator new(size_t size, size_t capacity);
    inline void  operator delete(void *) { ; }

    inline StringHashContents() { ; };
    inline StringHashContents(RESTORETYPE restoreType) { ; };
           StringHashContents(size_t entries, size_t total) : EqualityHashContents(entries, total) { }

    // default index comparison method
    bool isIndexEqual(RexxInternalObject *target, RexxInternalObject *entryIndex) override
    {
        // compare using fast string comparisons
        return ((RexxString *)target)->memCompare((RexxString *)entryIndex);
    }

    // Take advantage of the knowledge that indexes are all strings and
    // do directly to the string hash method, which might be inlined.
    ItemLink hashIndex(RexxInternalObject *index) override
    {
        return (ItemLink)(((RexxString *)index)->getStringHash() % bucketSize);
    }
};

#endif
