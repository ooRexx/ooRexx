/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2020 Rexx Language Association. All rights reserved.    */
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
/* Code for the table backing the List class                                  */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ListClass.hpp"
#include "Memory.hpp"
#include "WeakReferenceClass.hpp"
#include "SupplierClass.hpp"
#include "ProtectedObject.hpp"


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
    return new_object(size + sizeof(ListEntry) * (initialSize - 1), T_ListContents, initialSize);
}


/**
 * Construct a ListContent item of the given size.
 *
 * @param size   The total number of entries in the object.
 */
ListContents::ListContents(size_t size)
{
    // clear the entire object for safety
    clearObject();

    // this is the total size of the bucket
    totalSize = size;

    // no first or last items.
    firstItem = NoMore;
    lastItem = NoMore;

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
 * We're merging a list contents into this collection after an
 * expansion.  We need to take care to keep the same index
 * values for each of the inserted items.  We cleare out all
 * slot positions, then rebuild the chains using the same
 * positions as the original.  Once the merge is complete, then
 * we rescan the table for free entries and rebuild the table.
 */
void ListContents::prepareForMerge()
{
    // no first or last items.
    firstItem = NoMore;
    lastItem = NoMore;

    // this is an empty bucket
    itemCount = 0;

    // we keep the available items on a chain, so
    // chain up the items into a free chain
    freeChain = NoMore;

    // now clear all of the entries out
    for (ItemLink i = 0; i < totalSize; i++)
    {
        clearEntry(i);
    }
}


/**
 * We've completed the merge operation.  Now we need to up the
 * unused slots and build a free chain.
 */
void ListContents::completeMerge()
{
    // we keep the available items on a chain, so
    // chain up the items into a free chain
    freeChain = NoMore;

    // ok, scan from the end of the table to the beginning, so we
    // build the free chain with the smallest indexes in the front
    for (ItemLink i = totalSize; i > 0; i--)
    {
        ItemLink index = i - 1;

        // if this slot is empty
        if (isAvailable(index))
        {
            // put this back on the free chain
            returnToFreeChain(index);
        }
    }
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void ListContents::live(size_t liveMark)
{
    // we only mark the active items rather than scanning the entire content area.
    for (size_t position = firstItem; position != NoMore; position = nextEntry(position))
    {
        memory_mark(entries[position].value);
    }
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void ListContents::liveGeneral(MarkReason reason)
{
    // we only mark the active items rather than scanning the entire content area.
    for (size_t position = firstItem; position != NoMore; position = nextEntry(position))
    {
        memory_mark_general(entries[position].value);
    }
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void ListContents::flatten(Envelope *envelope)
{
    setUpFlatten(ListContents)

    // we only mark the active items rather than scanning the entire content area.
    for (size_t position = firstItem; position != NoMore; position = nextEntry(position))
    {
        flattenRef(entries[position].value);
    }

    cleanUpFlatten
}


/**
 * Merge the list maintained in this contents object into
 * a target one after an expansion has occurred.  Note that this
 * is a little complicated, because we need to maintain the
 * relationship between items are their current indexes.
 *
 * @param target The target contents.
 */
void ListContents::mergeInto(ListContents *target)
{
    // NOTE:  This assumes the target contents item is at least
    // as large as this one.

    // tell the target we're going to do a merge
    target->prepareForMerge();

    // run the chain appending each item on to the target
    for (ItemLink position = firstItem; position != NoMore; position = nextEntry(position))
    {
        // this version of append will use the same index value for the append.
        target->append(position, entryValue(position));
    }

    // now the target needs to rebuild the free chains from the unused
    // slots.
    target->completeMerge();
}


/**
 * Allocate a slot for a new value and fill it in.
 *
 * @param value  The value we're inserting.
 *
 * @return The index position for the inserted item.
 */
ListContents::ItemLink ListContents::allocateSlot(RexxInternalObject *value)
{
    // this is a nice central place to handle bumping the count
    itemCount++;

    // get an item off of the chain and set the value
    ItemLink newItem = freeChain;

    // belt-and-braces...this should not occur...but give a logic
    // error if it does occur, since that indicates something bad has
    // occurred.
    if (newItem == NoMore)
    {
        Interpreter::logicError("Attempt to add an object to a full List contents");
    }

    freeChain = nextEntry(newItem);

    setValue(newItem, value);
    clearLinks(newItem);
    return newItem;
}


/**
 * Insert an item at the end of the existing list.
 *
 * @param newItem The new item we're inserting.
 */
void ListContents::insertAtEnd(ListContents::ItemLink newItem)
{
    // first insertion into this list?
    if (lastItem == NoMore)
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
void ListContents::insertAtFront(ListContents::ItemLink newItem)
{
    // first insertion into this list?
    if (firstItem == NoMore)
    {
        // make this the first and last, and leave all of its links
        // as terminators
        firstItem = newItem;
        lastItem = newItem;
    }
    // insert before the first item
    else
    {
        insertBefore(newItem, firstItem);
    }
}


/**
 * Insert an item after a given list item.
 *
 * @param newItem    The new item we're inserting.
 * @param insertItem The item this is inserted after.
 */
void ListContents::insertAfter(ListContents::ItemLink newItem, ListContents::ItemLink insertItem)
{
    // the new item gets the next item of our predecessor
    setNext(newItem, nextEntry(insertItem));
    // set the next and previous of these two to point at each other
    setNext(insertItem, newItem);
    setPrevious(newItem, insertItem);

    // do we have a following item?...if not, we're the new last item
    if (nextEntry(newItem) == NoMore)
    {
        lastItem = newItem;
    }
    // need to update the item after us to point back to us
    else
    {
        setPrevious(nextEntry(newItem), newItem);
    }
}


/**
 * Insert an item before a given list item.
 *
 * @param newItem    The new item we're inserting.
 * @param insertItem The item this is inserted before.
 */
void ListContents::insertBefore(ListContents::ItemLink newItem, ListContents::ItemLink insertItem)
{
    // the new item gets the prevous item of our predecessor
    setPrevious(newItem, previousEntry(insertItem));
    // set the next and previous of these two to point at each other
    setPrevious(insertItem, newItem);
    setNext(newItem, insertItem);

    // do we have a previous item?...if not, we're the new first item
    if (previousEntry(newItem) == NoMore)
    {
        firstItem = newItem;
    }
    // need to update the item after us to point back to us
    else
    {
        setNext(previousEntry(newItem), newItem);
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
ListContents::ItemLink ListContents::insert(RexxInternalObject *value, ListContents::ItemLink index)
{
    ItemLink newItem = allocateSlot(value);
    // if we got a .nil index for the insertion, this is at the beginning.
    if (index == AtEnd)
    {
        insertAtEnd(newItem);
    }
    else if (index == AtBeginning)
    {
        insertAtFront(newItem);
    }
    else
    {
        // this is inserted after the index position
        insertAfter(newItem, index);
    }

    // return the new index
    return newItem;
}


/**
 * Append an item to the list
 *
 * @param value  The value to add.
 *
 * @return The index position of the new item.
 */
ListContents::ItemLink ListContents::append(RexxInternalObject *value)
{
    ItemLink newItem = allocateSlot(value);
    // insert this at the end and return the index.
    insertAtEnd(newItem);
    return newItem;
}


/**
 * Append an item to the list, using a provided index.  This is
 * only used during a merge operation.
 *
 * @param value  The value to add.
 *
 * @return The index position of the new item.
 */
void ListContents::append(ItemLink newItem, RexxInternalObject *value)
{
    // we need to update this here because we're not allocating directly
    itemCount++;

    // this is also normally done during slot allocation, set the value now.
    setValue(newItem, value);

    // insert this at the end and return the index.
    insertAtEnd(newItem);
}


/**
 * Add a value to the beginning of the list
 *
 * @param value  The value to add.
 *
 * @return The index position of the new item.
 */
ListContents::ItemLink ListContents::insertAtBeginning(RexxInternalObject *value)
{
    ItemLink newItem = allocateSlot(value);
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
ListContents::ItemLink ListContents::insertAtEnd(RexxInternalObject *value)
{
    ItemLink newItem = allocateSlot(value);
    insertAtEnd(newItem);
    // return the new index
    return newItem;
}


/**
 * Remove an item postion from the chain and return
 * the location to the free chain.
 *
 * @param item   The item position to remove.
 */
void ListContents::removeItem(ListContents::ItemLink item)
{
    // we have one fewer item now.
    itemCount--;

    // handle being just the only item first, since it
    // simplifies some things by removing that possibility first
    if (item == firstItem)
    {
        // first and last, this is easy.
        if (item == lastItem)
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
        setPrevious(nextEntry(item), previousEntry(item));
        // and the reverse for our previous item
        setNext(previousEntry(item), nextEntry(item));
    }

    // put this back on the free chain
    returnToFreeChain(item);
}


/**
 * Retrieve an index from the contents.
 *
 * @param index  The index position.
 *
 * @return The associated value.
 */
RexxInternalObject *ListContents::get(ListContents::ItemLink index)
{
    return isIndexValid(index) ? entryValue(index) : OREF_NULL;
}


/**
 * Perform a PUT() operation on an existing index, replacing
 * the existing value.
 *
 * @param value  The new value
 * @param index  The index position.
 *
 * @return The old value or OREF_NULL if this is not a valid index.
 */
RexxInternalObject *ListContents::put(RexxInternalObject *value, ListContents::ItemLink index)
{
    if (!isIndexValid(index))
    {
        return OREF_NULL;
    }

    RexxInternalObject *oldValue = entryValue(index);
    setValue(index, value);
    return oldValue;
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
    return removed;
}


/**
 * Return the first item in the list.
 *
 * @return The first item, or OREF_NULL if the list is empty.
 */
RexxInternalObject *ListContents::getFirstItem()
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
RexxInternalObject *ListContents::getLastItem()
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
ListContents::ItemLink ListContents::firstIndex()
{
    return firstItem;
}


/**
 * Return the index of the last item in the list.
 *
 * @return The last item index, or NoMore if the list is empty
 */
ListContents::ItemLink ListContents::lastIndex()
{
    return lastItem;
}


/**
 * Get the index of the next item after a target item.
 *
 * @param item   The item index.
 *
 * @return The next item.  NoMore indicates there is no next item.
 */
ListContents::ItemLink ListContents::nextIndex(ListContents::ItemLink item)
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
ListContents::ItemLink ListContents::previousIndex(ListContents::ItemLink item)
{
    return item == NoMore ? NoMore : previousEntry(item);
}


/**
 * Get all of the items in the list as an array item.
 *
 * @return An array containing all of the items.
 */
ArrayClass *ListContents::allItems()
{
    ArrayClass *itemArray = new_array(itemCount);

    for (ItemLink position = firstItem; position != NoMore; position = nextEntry(position))
    {
        itemArray->append(entryValue(position));
    }

    return itemArray;
}


/**
 * Get all of the indexes in the list as an array item.
 *
 * @return An array containing all of the indexes.
 */
ArrayClass *ListContents::allIndexes()
{
    ArrayClass *itemArray = new_array(itemCount);

    for (ItemLink position = firstItem; position != NoMore; position = nextEntry(position))
    {
        itemArray->append(new_integer(position));
    }

    return itemArray;
}


/**
 * Empty the list of all contents.
 */
void ListContents::empty()
{
    // clear all of the entries so we handle old-to-new properly.
    for (ItemLink position = firstItem; position != NoMore;)
    {
        // get the next link before clearing
        ItemLink next = nextEntry(position);
        clearEntry(position);
        position = next;
    }

    itemCount = 0;
    firstItem = NoMore;
    lastItem = NoMore;

    // reset the free chains
    initializeFreeChain();
}


/**
 * Clear the value from a given entry.
 *
 * @param position The table position to clear.
 */
void ListContents::clearEntry(ListContents::ItemLink position)
{
    clearField(entries[position].value);
    // belt and braces...clear the next and previous values as well
    entries[position].next = NoMore;
    entries[position].previous = NoMore;
}


void ListContents::setValue(ListContents::ItemLink position, RexxInternalObject *v)
{
    setField(entries[position].value, v);
}


/**
 * Find the index for a target item.
 *
 * @param target The target item.
 *
 * @return The index of the located item or NoMore if this cannot be found.
 */
ListContents::ItemLink ListContents::getIndex(RexxInternalObject *target)
{
    // scan until we get a hit.
    for (ItemLink position = firstItem; position != NoMore; position = nextEntry(position))
    {
        if (target->equalValue(entryValue(position)))
        {
            return position;
        }
    }
    return NoMore;
}


/**
 * Test if the collection contains an instance of the given
 * item.
 *
 * @param target The target item.
 *
 * @return The index of the located item or NoMore if this cannot be found.
 */
bool ListContents::hasItem(RexxInternalObject *target)
{
    return getIndex(target) != NoMore;
}


/**
 * Remove an item from the collection.
 *
 * @param target The target item.
 *
 * @return The actual stored object value.
 */
RexxInternalObject *ListContents::removeItem(RexxInternalObject *target)
{
    // scan for the item until we get a hit.  Return the object that is actually there.
    for (ItemLink position = firstItem; position != NoMore; position = nextEntry(position))
    {
        if (target->equalValue(entryValue(position)))
        {
            RexxInternalObject *removed = entryValue(position);
            removeItem(position);
            return removed;
        }
    }
    return OREF_NULL;
}


/**
 * Create a supplier for the collection.
 *
 * @return A supplier for iterating over the collection.
 */
SupplierClass *ListContents::supplier()
{
    ArrayClass *indexes = allIndexes();
    ArrayClass *values = allItems();
    return new_supplier(values, indexes);
}


/**
 * Not really part of normal list operation, but classes
 * maintain their subclasses as a list of weak references
 * so that subclasses can get garbage collected when no
 * longer needed.  When the class needs to iterate over
 * the subclasses, it needs to resolve which weak references
 * are still valid.  This method will check the weak
 * references and remove any stale entries.  Then it
 * will return an array of the dereferenced objects.
 *
 * @return An array of the dereferenced objects.
 */
ArrayClass *ListContents::weakReferenceArray()
{
    // this is a little tricky.  We allocate the result array
    // before we process the weak references.  If we prune
    // the stale references first and then allocate an array,
    // we might hit a GC window that could create more stale
    // references.

    // make this large enough to hold what is currently there.
    // this could be more than we need, but that's fine. The initial size is zero.
    Protected<ArrayClass> result = new (0, items()) ArrayClass;

    ItemLink position = firstItem;
    while (position != NoMore)
    {
        // get the next position before processing the current link
        ItemLink next = nextEntry(position);

        // get the reference value and see if it is still valid
        // if not, just delink this position and allow the weak reference
        // to be garbage collected.
        WeakReference *ref = (WeakReference *)entryValue(position);
        if (!ref->hasReferent())
        {
            removeItem(position);
        }
        // we have a good value, so add this to the result array.
        else
        {
            result->append(ref->get());
        }
        // now step to the next position
        position = next;
    }
    return result;
}
