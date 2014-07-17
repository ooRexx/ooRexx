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
/* Code for the table backing the List class                                  */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ListClass.hpp"
#include "Memory.hpp"


/**
 * Allocate memory for a new ListContents entry.
 *
 * @param size   The base object size.
 * @param initialSize
 *               The initial number of entries
 *
 * @return The storage for a new table object.
 */
void *ListContents::operator new(size_t size, size_t initialSize)
{
    return new_object(size + sizeof(ListEntry) * (initialSize - 1), T_ListContents);
}


/**
 * Construct a HashContent item of the given size.
 *
 * @param entries The total number of entries in the object.
 */
ListContents::ListContents(size_t entries, size_t total)
{
    // clear the entire object for safety
    clearObject();

    // this is the total number of slots in the table.  The
    // optimal bucket size should already have been calculated
    bucketSize = entries;
    // this is the total size of the bucket
    totalSize = total;

    firstItem = NoMore;
    lastItem = NoMore

    // initialize the free chains
    initializeFreeChain();
}


/**
 * Initialize the free chains, either at construction time or
 * after an empty() operation.
 */
void ListContents::initializeFreeChain()
{
    // this is an empty bucket
    itemCount = 0;

    // we keep the available items on a chain, so
    // chain up the items into a free chain
    freeChain = 0;

    for (ItemLink i = freeChain; i < totalSize; i++)
    {
        entries[i].next = i + 1;
    }
    // make sure the last item ends the chain.  Note, we
    // don't bother double linking the free chain since we
    // only remove from the front.
    entries[totalSize - 1].next = NoMore;
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void ListContents::live(size_t liveMark)
{
    for (size_t index = 0; index < totalSize; index++)
    {
        memory_mark(entries[index].value);
    }
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void ListContents::liveGeneral(MarkReason reason)
{
    for (size_t index = 0; index < totalSize; index++)
    {
        memory_mark_general(elements[index].value);
    }
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void ListContents::flatten(RexxEnvelope *envelope)
{
    setUpFlatten(ListContents)

    for (size_t i = totalSize; i > 0 ; i--)
    {
       flattenRef(elements[i - 1].value);
    }

    cleanUpFlatten
}


/**
 * Merge the list maintained in this contents object into
 * a target one after an expansion has occurred.
 *
 * @param target The target contents.
 */
void ListContents::mergeInto(ListContents *target)
{
    // NOTE:  This assumes the target contents item is at least
    // as large as this one.

    // run the chain appending each item on to the target
    ItemLink position = firstItem;
    while (position != NoMore)
    {
        target->append(entryValue(i));
    }
}


/**
 * Allocate a slot for a new value and fill it in.
 *
 * @param value  The value we're inserting.
 *
 * @return The index position for the inserted item.
 */
ItemLink ListContents::allocateSlot(RexxInternalOBject *value)
{
    // this is a nice central place to handle bumping the count
    itemCount++;

    // get an item off of the chain and set the value
    ItemLink newItem = freeChain;
    freeChain = nextEntry(newItem);

    setEntryValue(newItem, value);
    return newItem;
}


/**
 * Insert an item at the end of the existing list.
 *
 * @param newItem The new item we're inserting.
 */
void ListContents::insertAtEnd(ItemLink newItem)
{
    // first insertion into this list?
    if (lastItem = NoMore)
    {
        // make this the first and last, and leave all of its links
        // as terminators
        firstItem = newItem;
        lastItem = newItem;
    }
    // insert after this item
    else
    {
        insertAfter(newItem, lastItem);
    }
}


/**
 * Insert an item at the front of the existing list.
 *
 * @param newItem The new item we're inserting.
 */
void ListContents::insertAtFront(ItemLink newItem)
{
    // first insertion into this list?
    if (firstItem = NoMore)
    {
        // make this the first and last, and leave all of its links
        // as terminators
        firstItem = newItem;
        lastItem = newItem;
    }
    // insert before the first item
    else
    {
        insertAfter(newItem, firstItem);
    }
}


/**
 * Insert an item after a given list item.
 *
 * @param newItem    The new item we're inserting.
 * @param insertItem The item this is inserted after.
 */
void ListContents::insertAfter(ItemLink newItem, ItemLink insertItem)
{
    // the new item gets the next item of our predecessor
    setNext(newItem, nextEntry(insertItem));
    // set the next and previous of these two to point at each other
    setNext(insertItem, newItem);
    setPrevious(newItem, insertItem);

    // do we have a following item?...if not, we're the new last item
    if (nextEntry(newItem) == NoMore)
    {
        lastItem = newItem
    }
    // need to update the item after us to point back to us
    else
    {
        setPrevious(nextEntry(newItem), newItem)
    }
}


/**
 * Insert an item before a given list item.
 *
 * @param newItem    The new item we're inserting.
 * @param insertItem The item this is inserted before.
 */
void ListContents::insertBefore(ItemLink newItem, ItemLink insertItem)
{
    // the new item gets the prevous item of our predecessor
    setPrevious(newItem, previousEntry(insertItem));
    // set the next and previous of these two to point at each other
    setPrevious(insertItem, newItem);
    setNext(newItem, insertItem);

    // do we have a previous item?...if not, we're the new first item
    if (previousEntry(insertItem) == NoMore)
    {
        firstItem = newItem
    }
    // need to update the item after us to point back to us
    else
    {
        setNext(previousEntry(newItem), newItem)
    }
}


/**
 * Add a value to the list at a given index position.
 *
 * @param value  The value to add.
 * @param index  The target index postion.  NoMore indicates add to the end.
 *
 * @return The index position of the new item.
 */
ItemLink ListContents::insert(RexxInternalObject *value, ItemLink index)
{
    newItem = allocateSlot(value);
    // if we got a .nil index for the insertion, this is at the beginning.
    if (index == AtEnd)
    {
        insertAtEnd(newItem);
    }
    if (index == AtBeginning)
    {
        insertAtFront(newItem);
    }
    else
    {
        // this is inserted before the index position
        insertBefore(newItem, index);
    }

    // return the new index
    return newItem;
}


/**
 * Add a value to the beginning of the list
 *
 * @param value  The value to add.
 *
 * @return The index position of the new item.
 */
ItemLink ListContents::insertAtBeginning(RexxInternalObject *value)
{
    newItem = allocateSlot(value);
    insertAtFront(newItem);
    // return the new index
    return newItem;
}


/**
 * Add a value to the beginning of the list
 *
 * @param value  The value to add.
 *
 * @return The index position of the new item.
 */
ItemLink ListContents::insertAtEnd(RexxInternalObject *value)
{
    newItem = allocateSlot(value);
    insertAtEnd(newItem);
    // return the new index
    return newItem;
}


void ListContents::removeItem(ItemLink item)
{
    // we have one fewer item now.
    itemCount--;

    // handle being just the only item first, since it
    // simplifies some things by removing that possibility first
    if (item == firstItem)
    {
        // first and last, this is easy.
        if (item == firstItem)
        {
            firstItem = NoMore;
            lastItem = NoMore;
        }
        // removing the first item, promote our
        // successor to the front
        else
        {
            firstItem = nextEntry(item);
            setPrevious(firstItem, NoMore);
        }

    }
    // not the first, could be the last
    else if (item == lastItem)
    {
        lastItem = previousEntry(item);
        setNext(lastItem, NoMore);
    }
    // have both a previous and next,
    else
    {
        // update the next item first
        setPrevious(nextEntry(item), previousEntry(item))
        // and the reverse for our previous item
        setNext(previousEntry(item), nextEntry(item))
    }

    // put this back on the free chain
    returnToFreeChain(item);
}


/**
 * Remove an item from the list at a given index.
 *
 * @param index  The target index.
 *
 * @return The removed item, if any.
 */
RexxInternalObject *ListContents::remove(ItemLink index)
{
    // this might have been out of bounds or obsolete...just return
    // NULL because there's nothing to remove.
    if (index == NoLink)
    {
        return OREF_NULL;
    }

    // get the current item before removing this from the chain.  That
    // is our return value.
    RexxInternalObject *removed = entryValue(index);
    removeItem(index);
}


/**
 * Return the first item in the list.
 *
 * @return The first item, or OREF_NULL if the list is empty.
 */
RexxInternalObject *ListContents::firstItem()
{
    if (firstItem == NoMore)
    {
        return OREF_NULL;
    }

    return entryValue(firstItem);
}


/**
 * Return the last item in the list.
 *
 * @return The last item, or OREF_NULL if the list is empty.
 */
RexxInternalObject *ListContents::lastItem()
{
    if (lastItem == NoMore)
    {
        return OREF_NULL;
    }

    return entryValue(lastItem);
}


/**
 * Return the index of the first item in the list.
 *
 * @return The first item index, or NoMore
 */
ItemLink ListContents::firstIndex()
{
    return firstItem;
}


/**
 * Return the index of the last item in the list.
 *
 * @return The last item index, or NoMore if the list is empty
 */
ItemLink ListContents::lastIndex()
{
    return lastIndex;
}


/**
 * Get the index of the next item after a target item.
 *
 * @param item   The item index.
 *
 * @return The next item.  NoMore indicates there is no next item.
 */
ItemLink ListContents::nextIndex(ItemLink item)
{
    return item == NoMore ? NoMore : nextEntry(item);
}


/**
 * Get the index of the item before a given index.
 *
 * @param item   The target item.
 *
 * @return The item index, or NoMore if there is not valid previous item.
 */
ItemLink ListContents::previousIndex(ItemLink item)
{
    return item == NoMore ? NoMore : previousEntry(item);
}


/**
 * Get all of the items in the list as an array item.
 *
 * @return An array containing all of the items.
 */
RexxArray *ListContents::allItems()
{
    RexxArray *items = new_array(items());

    size_t position = firstItem;
    while (position != NoMore)
    {
        items->append(entryValue(position));
        position = nextEntry(position);
    }

    return items
}


/**
 * Get all of the indexes in the list as an array item.
 *
 * @return An array containing all of the indexes.
 */
RexxArray *ListContents::allIndexes()
{
    RexxArray *items = new_array(items());

    size_t position = firstItem;
    while (position != NoMore)
    {
        items->append(new_integer(position)));
        position = nextEntry(position);
    }

    return items
}


void ListContents::empty()
{
    // clear all of the entries so we handle old-to-new properly.
    size_t position = firstItem;
    while (position != NoMore)
    {
        // get the next link before clearing
        ItemLink next = nextEntry(position);
        clearEntry(position);
        position = next;
    }

    // reset the free chains
    initializeFreeChain();
}


ItemLink ListContents::getIndex(RexxInternalObject *target)
{
    size_t position = firstItem;
    while (position != NoMore)
    {
        if (target->equalValue(entryValue(position)))
        {
            return position;
        }
        position = nextEntry(position);
    }
    return NoMore;
}


RexxInternalObject *ListContents::removeItem(RexxInternalObject *target)
{
    size_t position = firstItem;
    while (position != NoMore)
    {
        if (target->equalValue(entryValue(position)))
        {
            RexxInternalObject *removed = entryValue(position);
            removeItem(position);
            return removed;

        }
        position = nextEntry(position);
    }
    return OREF_NULL;

}


SupplierClass *ListContents::supplier()
{
    RexxArray *indexes = allIndexes();
    RexxArray *values = allItems();
    return new_supplier(values, indices);
}
