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
/* Definition for the contents stored in a List class instance                */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ListContents
#define Included_ListContents

#include "ObjectClass.hpp"

class ListContents : public RexxInternalObject
{
  public:
    // The type for the reference links
    typedef size_t ItemLink;

    // link terminator
    static const ItemLink NoMore = SIZE_MAX;
    // indicates not linked
    static const ItemLink NoLink = SIZE_MAX;
    // insert at the end (overloaded value)
    static const ItemLink AtEnd = SIZE_MAX;
    // insert at the beginning
    static const ItemLink AtBeginning = SIZE_MAX - 1;

    class ListEntry
    {
     public:
        inline bool isAvailable() { return value == OREF_NULL; }
        inline bool isInUse() { return value != OREF_NULL; }
        RexxInternalObject *value;           // list element value
        size_t next;                         // next list element in chain
        size_t previous;                     // previous list element in chain
    };

           void *operator new(size_t, size_t);
    inline void  operator delete(void *) { }

    inline ListContents(RESTORETYPE restoreType) { ; };
    inline ListContents() {;};
           ListContents(size_t size);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    void initializeFreeChain();
    void prepareForMerge();
    void completeMerge();
    void mergeInto(ListContents *target);
    ItemLink allocateSlot(RexxInternalObject *value);

    void insertAtEnd(ItemLink newItem);
    void insertAtFront(ItemLink newItem);
    void insertAfter(ItemLink newItem, ItemLink insertItem);
    void insertBefore(ItemLink newItem, ItemLink insertItem);
    ItemLink insert(RexxInternalObject *value, ItemLink index);
    ItemLink insertAtBeginning(RexxInternalObject *value);
    ItemLink insertAtEnd(RexxInternalObject *value);
    void removeItem(ItemLink item);
    ItemLink append(RexxInternalObject *value);
    void append(ItemLink index, RexxInternalObject *value);

    RexxInternalObject *get(ItemLink index);
    RexxInternalObject *put(RexxInternalObject *value, ItemLink index);
    RexxInternalObject *remove(ItemLink index);
    RexxInternalObject *getFirstItem();
    RexxInternalObject *getLastItem();
    ItemLink firstIndex();
    ItemLink lastIndex();
    ItemLink nextIndex(ItemLink item);
    ItemLink previousIndex(ItemLink item);
    ArrayClass *allItems();
    ArrayClass *allIndexes();
    void empty();
    ItemLink getIndex(RexxInternalObject *target);
    bool hasItem(RexxInternalObject *target);
    RexxInternalObject *removeItem(RexxInternalObject *target);
    SupplierClass *supplier();
    ArrayClass *weakReferenceArray();

    // clear an entry in the chain
    void clearEntry(ItemLink position);

    // update a next entry
    void setNext(ItemLink position, ItemLink next) { entries[position].next = next; }
    void setPrevious(ItemLink position, ItemLink previous) { entries[position].previous = previous; }
    void clearLinks(ItemLink position)
    {
        setNext(position, NoMore);
        setPrevious(position, NoMore);
    }

    // copy an entry contents into another entry
    inline void copyEntry(ItemLink target, ItemLink source)
    {
        // copy all of the information (NOTE:  we need to use setField() for this)
        setValue(target, entryValue(source));
        entries[target].next = entries[source].next;
        entries[target].previous = entries[source].next;
    }


    // remove a non-anchor entry from a hash chain
    inline void closeChain(ItemLink position, ItemLink previous)
    {
        if (previous != NoMore)
        {
            entries[previous].next = entries[position].next;
        }
        if (entries[position].next != NoMore)
        {
            entries[entries[position].next].previous = previous;
        }

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
        // only use a single link for the free chain
        entries[position].next = freeChain;
        freeChain = position;
    }

    // set the value in an existing entry
    void setValue(ItemLink position, RexxInternalObject *value);

    // perform an item comparison for a position
    inline bool isItem(ItemLink position, RexxInternalObject *item)
    {
        // default comparison is object identity
        return item->isEqual(entryValue(position));
    }

    // check if an entry is availabe
    inline bool isAvailable(ItemLink postion)
    {
        return entries[postion].isAvailable();
    }

    // check if an entry is a real item
    inline bool isInUse(ItemLink postion)
    {
        return entries[postion].isInUse();
    }

    // step to the next position in the chain
    inline ItemLink nextEntry(ItemLink position)
    {
        return entries[position].next;
    }

    // step to the previous position in the chain
    inline ItemLink previousEntry(ItemLink position)
    {
        return entries[position].previous;
    }

    // get the value for an entry
    inline RexxInternalObject *entryValue(ItemLink position)
    {
        return entries[position].value;
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


    /**
     * Return the total current capacity.
     *
     * @return The total number of items this table can hold.
     */
    inline size_t capacity()
    {
        return totalSize;
    }

    // test if an index position is valid in this table.
    inline bool isIndexValid(ItemLink i)
    {
        return i < totalSize && isInUse(i);
    }

    inline size_t items() { return itemCount; }
    inline bool isEmpty() { return itemCount == 0; }

 protected:

    size_t   totalSize;                 // total size of the table, including the overflow area
    size_t   itemCount;                 // total number of items in the table
    ItemLink firstItem;                 // the head of the list chain
    ItemLink lastItem;                  // tail end of the list chain
    ItemLink freeChain;                 // first free element
    ListEntry entries[1];               // hash table entries
};
#endif

