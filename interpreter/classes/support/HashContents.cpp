/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* Primitive Hash Table Class                                                 */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "HashContents.hpp"
#include "HashCollection.hpp"
#include "ProtectedObject.hpp"
#include "SupplierClass.hpp"
#include "MethodArguments.hpp"


/**
 * Allocate a new IdentityHashContent item.
 *
 * @param size     The base size of the object.
 * @param capacity The capacity in entries (must be greater than zero)
 *
 * @return The backing storage for a content instance.
 */
void *IdentityHashContents::operator new(size_t size, size_t capacity)
{
    size_t bytes = size + (sizeof(ContentEntry) * (capacity - 1));

    // now allocate the suggested bucket size
    return new_object(bytes, T_IdentityHashContents, capacity * 2);
}


/**
 * Allocate a new EqualityHashContent item.
 *
 * @param size     The base size of the object.
 * @param capacity The capacity in entries (must be greater than zero)
 *
 * @return The backing storage for a content instance.
 */
void *EqualityHashContents::operator new(size_t size, size_t capacity)
{
    size_t bytes = size + (sizeof(ContentEntry) * (capacity - 1));

    // now allocate the suggested bucket size
    return new_object(bytes, T_EqualityHashContents, capacity * 2);
}


/**
 * Allocate a new MultiValueContents item.
 *
 * @param size     The base size of the object.
 * @param capacity The capacity in entries (must be greater than zero)
 *
 * @return The backing storage for a content instance.
 */
void *MultiValueContents::operator new(size_t size, size_t capacity)
{
    size_t bytes = size + (sizeof(ContentEntry) * (capacity - 1));

    // now allocate the suggested bucket size
    return new_object(bytes, T_MultiValueContents, capacity * 2);
}


/**
 * Allocate a new StringHashContents item.
 *
 * @param size     The base size of the object.
 * @param capacity The capacity in entries (must be greater than zero)
 *
 * @return The backing storage for a content instance.
 */
void *StringHashContents::operator new(size_t size, size_t capacity)
{
    size_t bytes = size + (sizeof(ContentEntry) * (capacity - 1));

    // now allocate the suggested bucket size
    return new_object(bytes, T_StringHashContents, capacity * 2);
}


/**
 * Construct a HashContent item of the given size.
 *
 * @param entries The total number of entries in the object.
 */
HashContents::HashContents(size_t entries, size_t total)
{
    // clear the entire object for safety
    clearObject();

    // this is the total number of slots in the table.  The
    // optimal bucket size should already have been calculated
    bucketSize = entries;
    // this is the total size of the bucket
    totalSize = total;

    // initialize the free chains
    initializeFreeChain();
}


/**
 * Initialize the free chains, either at construction time or
 * after an empty() operation.
 */
void HashContents::initializeFreeChain()
{
    // initialize the next link in all bucket slots to NoMore.
    for (ItemLink i = 0; i < bucketSize; i++)
    {
        entries[i].next = NoMore;
    }

    // this is an empty bucket
    itemCount = 0;

    // we keep the available items on a chain, so
    // chain up the items into a free chain
    freeChain = bucketSize;

    for (ItemLink i = freeChain; i < totalSize; i++)
    {
        entries[i].next = i + 1;
    }
    // make sure the last item ends the chain
    entries[totalSize - 1].next = NoMore;
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void HashContents::live(size_t liveMark)
{
    for (size_t i = 0; i < totalSize; i++)
    {
        memory_mark(entries[i].index);
        memory_mark(entries[i].value);
    }
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void HashContents::liveGeneral(MarkReason reason)
{
    for (size_t i = 0; i < totalSize; i++)
    {
        memory_mark_general(entries[i].index);
        memory_mark_general(entries[i].value);
    }
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void HashContents::flatten(Envelope *envelope)
{
    setUpFlatten(HashContents)
    for (size_t i = 0; i < totalSize; i++)
    {
        flattenRef(entries[i].index);
        flattenRef(entries[i].value);
    }
    cleanUpFlatten
}


/**
 * Put an item into the hashtable.  Default behavior is
 * to replace any existing items.
 *
 * @param value  The value to insert.
 * @param _index The index to insert under.
 *
 * @return true if was inserted ok, false if we need to expand the
 *         table.
 */
void HashContents::put(RexxInternalObject *value, RexxInternalObject *index)
{
    // NOTE:  This depends on the caller to make sure there is
    // room in the table for this item!

    // calculate the bucket position
    ItemLink position = hashIndex(index);

    // if the hash slot is empty, we can just fill this in right here
    if (isAvailable(position))
    {
        setEntry(position, value, index);
        // new item, so bump the count
        itemCount++;
        return;
    }

    ItemLink previous = NoLink;

    // this slot is in use.  We either A) have this entry already, in
    // which case we replace the value, or B) need to add another entry to the end
    do
    {
        // if we got a hit, just change the slot position
        if (isIndex(position, index))
        {
            // everything worked, we have room.  No additional items added to
            // this table.
            setValue(position, value);
            return;
        }

        // continue running the chain to the end of the list.
        previous = position;
        position = entries[position].next;
    } while (position != NoMore);

    // append a new value.
    append(value, index, previous);
}


/**
 * Add a link to a slot position chain.
 *
 * @param value    The value to add.
 * @param index    The index value.
 * @param position The position of the last link in the chain.
 */
void HashContents::append(RexxInternalObject *value, RexxInternalObject * index, ItemLink position)
{
    // we keep all of the free items in a chain, so we can just pull
    // the first entry off of the chain.  In theory, we've already checked
    // that we're not full, so we should find one there.

    ItemLink newEntry = freeChain;

    // belt-and-braces...this should not occur...but give a logic
    // error if it does occur, since that indicates something bad has
    // occurred.
    if (newEntry == NoMore)
    {
        Interpreter::logicError("Attempt to add an object to a full Hash table");
    }

    // close up the chain
    freeChain = entries[newEntry].next;

    // set the entry
    setEntry(newEntry, value, index);
    // add this to the end of the chain
    setNext(position, newEntry);
    setNext(newEntry, NoMore);

    // we have a new item in the list.
    itemCount++;
}


/**
 * Add a link to a slot position chain  This adds it at the
 * FRONT of the chain.
 *
 * @param value    The value to add.
 * @param index    The index value.
 * @param position The position of the chain anchor.
 */
void HashContents::insert(RexxInternalObject *value, RexxInternalObject * index, ItemLink position)
{
    // we keep all of the free items in a chain, so we can just pull
    // the first entry off of the chain.  In theory, we've already checked
    // that we're not full, so we should find one there.

    ItemLink newEntry = freeChain;

    // belt-and-braces...this should not occur
    if (newEntry == NoMore)
    {
        Interpreter::logicError("Attempt to add an object to a full Hash table");
    }

    // close up the chain
    freeChain = entries[newEntry].next;

    // copy the anchor position to this new entry
    copyEntry(newEntry, position);

    // set the anchor item to the new values
    setEntry(position, value, index);
    // and chain the new entry off of this.
    setNext(position, newEntry);

    // we have a new item in the list.
    itemCount++;
}


/**
 * Remove an item from the collection.
 *
 * @param index The index value
 *
 * @return The removed object, or OREF_NULL if this was not found.
 */
RexxInternalObject *HashContents::remove(RexxInternalObject *index)
{
    ItemLink position;
    ItemLink previous;

    // go find the matching entry.  We have nothing to do if not there.
    if (!locateEntry(index, position, previous))
    {
        return OREF_NULL;
    }

    // save the original value
    RexxInternalObject *removed = entryValue(position);
    // remove the chain link and return the value
    removeChainLink(position, previous);
    return removed;
}


/**
 * Remove a link from the chain, taking into consideration
 * where the link occurs.  This will add a new link to
 * the free chain.
 *
 * @param position The position we're removing.
 * @param previous The previous chain entry (NoLink value if this is the head of the chain).
 */
void HashContents::removeChainLink(ItemLink &position, ItemLink previous)
{
    // reduce the item count for this removal.
    itemCount--;

    // are we removing the first item in the chain?  We can't move this
    // to the free list because this is the anchor for the hash postion.
    // we need to either promote the next item to the front of the chain
    // or clear this item out if this is also the end of the chain
    if (previous == NoLink)
    {
        ItemLink next = nextEntry(position);
        // just the single item on the chain?  reset to empty state and we're done.
        if (next == NoMore)
        {
            clearEntry(position);
            return;
        }
        // copy the next item to the head of the list
        copyEntry(position, next);
        // return the next entry to the free chain
        returnToFreeChain(next);
    }
    else
    {
        // close up the chain, and and also update the
        // previous position if we still need to run the chain
        closeChain(position, previous);
        position = nextEntry(previous);
    }
}


/**
 * Locate an entry in the table.
 *
 * @param index    The target entry index.
 * @param position The return position if located.
 * @param previous The previous link if located.  Has the value NoLink if
 *                 the located item is the chain anchor.
 *
 * @return true if the item is located, false for a failure
 */
bool HashContents::locateEntry(RexxInternalObject *index, ItemLink &position, ItemLink &previous)
{
    // get the bucket position to start searching
    position = hashIndex(index);
    previous = NoLink;

    // ok, run the chain searching for an index match.
    while (position != NoMore && isInUse(position))
    {
        // have a match? return to the caller.  All of the position
        // stuff should be set now
        if (isIndex(position, index))
        {
            return true;
        }
        // remember the previous position (needed for chain updates)
        previous = position;
        // step to the next link in the chain
        position = nextEntry(position);
    }

    // hit the end of the chain, we don't have this item
    position = NoMore;
    return false;
}


/**
 * Locate the next entry in the table for a given index
 *
 * @param index    The target entry index.
 * @param position The starting position for the search (the
 *                 last match)
 */
void HashContents::nextMatch(RexxInternalObject *index, ItemLink &position)
{
    // got a bad call here.
    if (position == NoMore)
    {
        return;
    }

    // step to the next position
    position = nextEntry(position);

    // keep looping until the end of this chain
    while (position != NoMore)
    {
        // if this is a match, we're done
        if (isIndex(position, index))
        {
            return;
        }
        // step to the next position
        position = nextEntry(position);
    }

    // hit the end of the chain, we don't have this item
    // (position also marks this as the end)
    return;
}


/**
 * Iterate to the next occupied entry of the list.
 *
 * @param position   The starting position.
 * @param nextBucket The next bucket to step to once this chain is used up.
 */
void HashContents::iterateNext(ItemLink &position, ItemLink &nextBucket)
{
    // have a good current position...step to the next chain item.
    if (position != NoMore)
    {
        // if we still have a chain, then we can just return this position as the next one
        position = nextEntry(position);
        if (position != NoMore)
        {
            return;
        }
    }

    // look for another bucket with an active chain.
    while (nextBucket < bucketSize)
    {
        position = nextBucket++;
        // if this bucket position is active, we've found our match
        if (isInUse(position))
        {
            return;
        }
    }
    // make sure this is marked as a failure
    position = NoMore;
}


/**
 * Safely remove an item from the table and advance the
 * iterator position to the next location.
 *
 * @param position   The current position.
 * @param nextBucket The nextBucket we advance to.
 */
void HashContents::iterateNextAndRemove(ItemLink &position, ItemLink &nextBucket)
{
    // a remove operation is special if the we're at the head of the chain,
    // since a remove operation will move another item.  into the slot, if
    // there is one.
    if (isBucketPosition(position))
    {
        // if this is the only item in this bucket, we can just
        // advance normally, then remove the item.
        if (nextEntry(position) == NoMore)
        {
            // save this for the deletion
            ItemLink current = position;
            iterateNext(position, nextBucket);
            // perform the removal now that we've moved
            removeChainLink(current, NoLink);
        }
        // we're at the head of the chain and we need to remove this
        // item.  This will move the next item into the bucket slot, which
        // means if we just remove the item, our position information will already
        // be correct.
        else
        {
            removeChainLink(position, NoLink);
        }
    }
    // we're past the bucket position in the chain.  To delete this,
    // we need to know the previous item.  However, if we advance to the
    // next position first, that information will still be correct after deletion.
    else
    {
        // save the bucket chain anchor and current position for the removal.
        ItemLink current = position;
        ItemLink currentBucket = nextBucket - 1;
        // we need this also
        ItemLink previous = position;
        // advance to the next position.
        iterateNext(position, nextBucket);
        // now find the previous entry so we can remove the item
        locatePreviousEntry(previous, currentBucket);
        // do the removal.
        removeChainLink(current, previous);
    }
}


/**
 * Iterate to the next occupied entry of the list in reverse
 * bucket order.
 *
 * @param position   The starting position.
 * @param currentBucket The current bucket we're iterating on.
 *
 */
void HashContents::iterateNextReverse(ItemLink &position, ItemLink &currentBucket)
{
    // The position will always be valid because we're backing up the chain to the
    // bucket anchor.  If we're at the bucket anchor, we then locate the last position
    // in the next occupied bucket
    if (position == currentBucket)
    {
        // step to the next bucket and see if we have a next entry
        currentBucket++;
        locateNextBucketEnd(position, currentBucket);
    }
    // we're traversing
    else
    {
        // we don't maintain back links, so we need to scan.  Not
        // terribly efficient, but the buckets don't tend to get very long
        // and we don't do this all that often.
        locatePreviousEntry(position, currentBucket);
    }
}


/**
 * Locate the end of the next occupied hash bucket in this
 * collection.
 *
 * @param position The returned position (NoMore indicates we've hit the end)
 * @param currentBucket
 *                 The starting bucket to search.  Updated to the new bucket
 *                 if something is found.
 */
void HashContents::locateNextBucketEnd(ItemLink &position, ItemLink &currentBucket)
{
    // search each of the buckets in turn looking for an occupied one.
    while (currentBucket < bucketSize)
    {
        // if we have an entry in the first position, this is the one we want
        if (isInUse(currentBucket))
        {
            // set the pointer at the beginning
            position = currentBucket;
            // locate the last entry in the bucket
            while(nextEntry(position) != NoMore)
            {
                position = nextEntry(position);
            }
            return;
        }
        // try the next bucket
        currentBucket++;
    }

    // indicate we've hit the iteration end
    position = NoMore;
}


/**
 * Locate the entry that precedes the current bucket item.
 *
 * @param position The current position in the chain.  Note:  we've already determined
 *                 that we are not at the anchor position.
 * @param currentBucket
 *                 The bucket anchor item.
 */
void HashContents::locatePreviousEntry(ItemLink &position, ItemLink currentBucket)
{
    // just run the chain until we find the next item
    while (nextEntry(currentBucket) != position)
    {
        currentBucket = nextEntry(currentBucket);
    }
    position = currentBucket;
}


/**
 * Locate an entry in the table by index/item pair
 *
 * @param index    The target entry index.
 * @param position The return position if located.
 * @param previous The previous link if located.  Has the value NoLink if
 *                 the located item is the chain anchor.
 *
 * @return true if the item is located, false for a failure
 */
bool HashContents::locateEntry(RexxInternalObject *index, RexxInternalObject *item, ItemLink &position, ItemLink &previous)
{
    // get the bucket position to start searching
    position = hashIndex(index);
    previous = NoLink;

    // ok, run the chain searching for an index match.
    while (position != NoMore && isInUse(position))
    {
        // have a match? return to the caller.  All of the position
        // stuff should be set now
        if (isItem(position, index, item))
        {
            return true;
        }
        // remember the previous position (needed for chain updates)
        previous = position;
        // step to the next link in the chain
        position = nextEntry(position);
    }

    // hit the end of the chain, we don't have this item
    return false;
}


/**
 * Locate an item in the table.
 *
 * @param item     The target item.
 * @param position The return position if located.
 * @param previous The previous link if located.  Has the value NoLink if
 *                 the located item is the chain anchor.
 *
 * @return true if the item is located, false for a failure
 */
bool HashContents::locateItem(RexxInternalObject *item, ItemLink &position, ItemLink &previous)
{
    // locating an item requires a table search.  Since we also want to locate what
    // chain this is on, we perform the search by running the chains in each bucket position.
    // since this does not scan any of the entries on the free chain, this method
    // could be faster than a brute force table search.

    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        position = i;
        previous = NoLink;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // have a match? return to the caller.  All of the position
            // stuff should be set now
            if (isItem(position, item))
            {
                return true;
            }
            // remember the previous position (needed for chain updates)
            previous = position;
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }

    // hit the end of the bucket list, we don't have this item
    return false;
}


/**
 * Remove all elements with a given index from a hashtable,
 * returning an array of all items.
 *
 * @param _index The target index.
 *
 * @return An array of all matching items.
 */
ArrayClass *HashContents::removeAll(RexxInternalObject *index)
{
    ItemLink position;
    ItemLink previous = NoLink;

    // get a count of matching items
    size_t count = countAllIndex(index, position);
    // get a result array
    ArrayClass *result = new_array(count);

    // if we have items to copy, run the chain and copy the index matches
    for (size_t i = 1; i <= count; i++)
    {
        // find an index match
        while (!isIndex(position, index))
        {
            previous = position;
            position = nextEntry(position);
        }
        // add this to the array and step again
        result->put(entryValue(position), i);
        // remove the link (which will also adjust the position
        removeChainLink(position, previous);
    }

    // return our result array
    return result;
}


/**
 * Remove the index/item tuple from the collection.
 *
 * @param value  The value to remove.
 * @param index  The index to remove.
 *
 * @return The removed value or OREF_NULL if the pair was not located.
 */
RexxInternalObject *HashContents::removeItem(RexxInternalObject *value, RexxInternalObject *index)
{
    // if no index was given, just return the first item.
    if (index == OREF_NULL)
    {
        return removeItem(value);
    }

    ItemLink position;
    ItemLink previous;

    // go find the matching entry.  We have nothing to do if not there.
    if (!locateEntry(index, value, position, previous))
    {
        return OREF_NULL;
    }

    // save the original value
    RexxInternalObject *removed = entryValue(position);
    // remove the chain link and return the value
    removeChainLink(position, previous);
    return removed;
}


/**
 * Determine if an item is in the table using the value/index pair.
 *
 * @param value  The target value.
 * @param index  The target index.
 *
 * @return true if the pair is in the table, false otherwise.
 */
bool HashContents::hasItem(RexxInternalObject *value, RexxInternalObject *index )
{
    // if the index was not specified, just locate any item.
    if (index == OREF_NULL)
    {
        return hasItem(value);
    }

    ItemLink position;
    ItemLink previous;

    // just run the locate operation
    return locateEntry(index, value, position, previous);
}


/**
 * Determine if an item is in the table.
 *
 * @param index  The target item.
 *
 * @return true if the index is in the table, false otherwise.
 */
bool HashContents::hasItem(RexxInternalObject *item)
{
    ItemLink position;
    ItemLink previous;

    // just run the locate operation
    return locateItem(item, position, previous);
}


/**
 * Removes an item from the hash table.
 *
 * @param item   The target value.
 *
 * @return The removed item or NULL if not located.
 */
RexxInternalObject *HashContents::removeItem(RexxInternalObject *item)
{
    ItemLink position;
    ItemLink previous;

    // go find the matching entry.  We have nothing to do if not there.
    if (!locateItem(item, position, previous))
    {
        return OREF_NULL;
    }

    // save the original value
    RexxInternalObject *removed = entryValue(position);
    // remove the chain link and return the value
    removeChainLink(position, previous);
    return removed;
}


/**
 * Return the next item with the key index that follows the
 * given (value index) pair.  Used only for superscope lookup.
 * Note:  This routine does the comparisons in as fast as way
 * as possible, relying solely on object identify for a match.
 * NOTE:  this is a somewhat special purpose method used only
 * for internal checks, so a lot of stuff is just done inline
 * rather than in methods because those would be somewhat single
 * purpose.
 *
 * @param value  The target value.
 * @param index  The target index
 *
 * @return The located next value, or .nil if not found.
 */
RexxInternalObject *HashContents::nextItem(RexxInternalObject *value, RexxInternalObject *index)
{
    ItemLink position = hashIndex(index);

    // loop the chain until we
    while (position != NoMore && isInUse(position))
    {
        // if we've found the pair, search for the next entry with the same index value.
        if (entries[position].matches(index, value))
        {
            position = nextEntry(position);
            while (position != NoMore && isInUse(position))
            {
                // we're only looking for index matches from this point
                if (entries[position].matches(index))
                {
                    return entryValue(position);
                }
                position = nextEntry(position);
            }
            // no next item found
            return TheNilObject;
        }
    }

    // the pair was not found...so see if we can find any value with this index
    // for a method table, this generally means it was added via setMethod().
    RexxInternalObject *scope = get(index);
    return resultOrNil(scope);
}


/**
 * Get an item from the hash table.
 *
 * @param index  The target index.
 *
 * @return The associated value item, or OREF_NULL if not located.
 */
RexxInternalObject *HashContents::get(RexxInternalObject *index)
{
    ItemLink position;
    ItemLink previous;

    // go find the matching entry.  We have nothing to do if not there.
    if (!locateEntry(index, position, previous))
    {
        return OREF_NULL;
    }
    // return the value
    return entryValue(position);
}


/**
 * Test if an index exists in the hash collection.
 *
 * @param index  The target index.
 *
 * @return True if we have an item at this index, false otherwise.
 */
bool HashContents::hasIndex(RexxInternalObject *index)
{
    ItemLink position;
    ItemLink previous;

    // go find the matching entry... return the success indicator
    return locateEntry(index, position, previous);
}


/**
 * Return a count of either all items, or all items with a given
 * index.
 *
 * @param index  The optional index.
 *
 * @return The appropriate count.
 */
size_t HashContents::items(RexxInternalObject *index)
{
    // if no index, return the total count
    if (index == OREF_NULL)
    {
        return items();
    }

    ItemLink position;

    // get a count of matching items
    return countAllIndex(index, position);
}


/**
 * Get all items with an associated index from the table.
 *
 * @param index  The target index value.
 *
 * @return An array containing all matching entries (zero length if none)
 */
ArrayClass  *HashContents::getAll(RexxInternalObject *index)
{
    ItemLink position;
    ItemLink previous = NoLink;

    // get a count of matching items
    size_t count = countAllIndex(index, position);
    // get a result array
    ArrayClass *result = new_array(count);

    // if we have items to copy, run the chain and copy the index matches
    for (size_t i = 1; i <= count; i++)
    {
        // find an index match
        while (!isIndex(position, index))
        {
            previous = position;
            position = nextEntry(position);
        }
        // add this to the array and step again
        result->put(entryValue(position), i);
        // step to the next chain item
        position = nextEntry(position);
    }

    // return our result array
    return result;
}


/**
 * Return a count of all entries with a given index.
 *
 * @param index
 * @param anchorPosition The returned hash bucket position for
 *                the head of the chain.
 *
 * @return The count of matching items.
 */
size_t HashContents::countAllIndex(RexxInternalObject *index, ItemLink &anchorPosition)
{
    size_t count = 0;
    // make sure we pass the anchor position back
    anchorPosition = hashIndex(index);
    ItemLink position = anchorPosition;

    while (position != NoMore && isInUse(position))
    {
        // compare the index values
        if (isIndex(position, index))
        {
            count++;
        }

        position = nextEntry(position);
    }
    return count;
}


/**
 * Return a count of all entries with a given item.
 *
 * @param index
 *
 * @return The count of matching items.
 */
size_t HashContents::countAllItem(RexxInternalObject *item)
{
    // locating an item requires a table search.  Since we also want to locate what
    // chain this is on, we perform the search by running the chains in each bucket position.
    // since this does not scan any of the entries on the free chain, this method
    // could be faster than a brute force table search.

    size_t count = 0;

    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // have a match? return to the caller.  All of the position
            // stuff should be set now
            if (isItem(position, item))
            {
                count++;
            }
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }

    return count;
}


/**
 * Return an array of all index values that match a
 * given item value.
 *
 * @param item   The target item.
 *
 * @return An array of all matching indexes.
 */
ArrayClass  *HashContents::allIndex(RexxInternalObject *item)
{
    // count all of the items first so we know how large the array
    // needs to be.
    size_t count = countAllItem(item);
    ArrayClass *result = new_array(count);

    // no need to search again if we don't have any matches
    if (count == 0)
    {
        return result;
    }

    size_t nextIndex = 1;

    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // have a match? return to the caller.  All of the position
            // stuff should be set now
            if (isItem(position, item))
            {
                // add to the result array, and if we've found the last match,
                // time to return.
                result->put(entryIndex(position), nextIndex++);
                if (nextIndex > count)
                {
                    return result;
                }
            }
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }
    return result;
}


/**
 * Return an index associated with the supplied item.  The result
 * is undefined for items that are referenced by multiple
 * indices.
 *
 * @param item   The target item.
 *
 * @return The index value, or OREF_NULL if not found.
 */
RexxInternalObject *HashContents::getIndex(RexxInternalObject *item)
{
    ItemLink position;
    ItemLink previous;

    // locate the item and return NULL if not found.
    if (!locateItem(item, position, previous))
    {
        return OREF_NULL;
    }

    // return the associated index.
    return entryIndex(position);
}


/**
 * Merge the contents of this collection into another table.
 *
 * @param target The target collection contents.
 */
void HashContents::merge(HashCollection *target)
{

    // since adding any item to the target collection might cause a size
    // expansion, let's give the target some notice about how many items
    // we're going to add so that the expansion can be handled up front.
    target->ensureCapacity(itemCount);

    // loop through all of the bucket items
    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // poke this item into the other table
            target->mergeItem(entryValue(position), entryIndex(position));
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }
}


/**
 * Put the contents of this collection into another collection.
 * This is different than merge, which does not replace existing
 * items.
 *
 * @param target The target collection contents.
 */
void HashContents::putAll(HashCollection *target)
{

    // since adding any item to the target collection might cause a size
    // expansion, let's give the target some notice about how many items
    // we're going to add so that the expansion can be handled up front.
    target->ensureCapacity(itemCount);

    // loop through all of the bucket items
    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // poke this item into the other table
            target->put(entryValue(position), entryIndex(position));
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }
}


/**
 * Merge a hash table into another hash table after a table
 * expansion.
 *
 * @param newHash The target new table.
 */
void HashContents::reMerge(HashContents *newHash)
{
    // one important aspect of a merge operation is keeping
    // values with identical indexes in the same relative order
    // after the merge (very important when merging MethodDirectories,
    // for example.  We process the merge by running each bucket chain,
    // and it is important that the target collection's mergeItem
    // method adds this to the end of the chain rather than to the beginning.

    // NOTE:  this merger is being doing under the assumption that the
    // hew hash contents is larger than the current.

    // loop through all of the bucket items
    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // poke this item into the other table.  We don't know
            // the characteristics of the target, so we might have
            // multiple hits on any given index.  We need to perform
            // this merge A) without losing any items in the collection and
            // B) keeping the relative order of duplicate indexes (very
            // important for MethodDictionaries, for example, which are
            // dependent on ordering of items in the table).  Therefore,
            // we do the merge by running each bucket chain and
            // using the add() method to add each additional item to the
            // end of the chain.
            newHash->add(entryValue(position), entryIndex(position));
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }
}


/**
 * Merge an item into this table without overwriting
 * values that might have the same index.  Used for building
 * the method dictionaries.
 *
 * @param item   The item value to merge.
 * @param index  The associated index value.
 */
void HashContents::mergeItem(RexxInternalObject *item, RexxInternalObject *index)
{
    // this is a special put method...not an add and not a put with overwrite.
    mergePut(item, index);
}


/**
 * Add an entry to the hash table, making sure we are
 * not overwriting an existing item.  This is used mostly
 * for merging various internal tables.
 *
 * @param item   The item value to add.
 * @param index  The index to add.
 *
 * @return true if we were able to add this (not full), false otherwise.
 */
void HashContents::mergePut(RexxInternalObject *item, RexxInternalObject *index)
{
    // NOTE:  This depends on the caller having checked that there is space.

    // calculate the bucket position
    ItemLink position = hashIndex(index);

    // if the hash slot is empty, we can just fill this in right here
    if (isAvailable(position))
    {
        setEntry(position, item, index);
        // new item, so bump the count
        itemCount++;
        return;
    }

    ItemLink previous = NoLink;

    // this slot is in use.  We either A) have this entry already, in
    // which case we just return, or B) need to add another entry to the end
    do
    {
        // if we got a hit, just change the slot position
        if (isIndex(position, index))
        {
            // nothing added, but this "worked"
            return;
        }

        // continue running the chain to the end of the list.
        previous = position;
        position = entries[position].next;
    } while (position != NoMore);

    // This was not already in the table, so add a new value to the chain
    append(item, index, previous);
}


/**
 * Return an array containing all of the table item objects.
 *
 * @return An array containing all of the items.
 */
ArrayClass  *HashContents::allItems()
{
    // get an array to hold the result
    ArrayClass *result = new_array(itemCount);

    // no need to search again if we don't have any matches
    if (itemCount == 0)
    {
        return result;
    }

    size_t nextIndex = 1;

    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // add to the result array, and if we've found the last match,
            // time to return.
            result->put(entryValue(position), nextIndex++);
            if (nextIndex > itemCount)
            {
                return result;
            }
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }

    return result;
}


/**
 * Empty a HashTable.
 */
void HashContents::empty()
{
    for (size_t i = 0; i < bucketSize; i++)
    {
        // if the first item of this bucket is in use
        if (isInUse(i))
        {
            // run the chain, clearing each item.  Note that because of
            // setField() considerations, we can't just use a memset.
            ItemLink position = i;
            do
            {
                // get the next link before clearing
                ItemLink next = nextEntry(position);
                clearEntry(position);
                position = next;
            } while (position != NoMore);
        }
    }
    // reset the free chains for the overflow area.
    initializeFreeChain();
}


/**
 * Create an array of all index items.
 *
 * @return An array of all indexes.
 */
ArrayClass *HashContents::allIndexes()
{
    // get an array to hold the result
    ArrayClass *result = new_array(itemCount);

    // no need to search again if we don't have any matches
    if (itemCount == 0)
    {
        return result;
    }

    size_t nextIndex = 1;

    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // add to the result array, and if we've found the last match,
            // time to return.
            result->put(entryIndex(position), nextIndex++);
            if (nextIndex > itemCount)
            {
                return result;
            }
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }

    return result;
}


/**
 * Return an array containing the unique indexes of a
 * Relation object.
 *
 * @return An array with the set of unique index values
 */
ArrayClass *HashContents::uniqueIndexes()
{
    // for tables with no duplicates, this is the same as allIndexes.
    // however, this method is only exposed for relations/bags, so we'll
    // leave the implementation in the base
    Protected<TableClass> indexSet = new_table(items());

    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // add to the result table.
            indexSet->put(TheNilObject, entryIndex(position));
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }
    // now return the reduced index set
    return indexSet->allIndexes();
}


/**
 * Create a supplier from the hash table
 *
 * @return A supplier instance.
 */
SupplierClass *HashContents::supplier()
{
    // get out target count and get arrays for both the values and indexes
    size_t count = itemCount;

    ArrayClass *values = new_array(count);
    ArrayClass *indexes = new_array(count);

    // if this is empty, no need to scan the table
    if (count == 0)
    {
        return new_supplier(values, indexes);
    }

    size_t nextIndex = 1;

    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // add to the result table.
            indexes->put(entryIndex(position), nextIndex);
            values->put(entryValue(position), nextIndex++);
            if (nextIndex > count)
            {
                // return the new supplier
                return new_supplier(values, indexes);
            }
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }

    // should never get here.
    return OREF_NULL;
}


/**
 * Create a supplier for the collection.  If the index
 * is specified, only the values associated with that index
 * are included.
 *
 * @param index  The optional index value.
 *
 * @return An appropriate supplier object.
 */
SupplierClass *HashContents::supplier(RexxInternalObject *index)
{
    // if no index was given, return a supplier for the entire collection
    if (index == OREF_NULL)
    {
        return supplier();
    }

    // get all of the items with that index
    Protected<ArrayClass> itemArray = getAll(index);
    size_t size = itemArray->items();
    Protected<ArrayClass> indexArray = new_array(size);
    // for the index array, we just fill in the same index value at every position
    for (size_t i = 1; i <= size; i++)
    {
        indexArray->put(index, i);
    }
    // and finally make a supplier from the two arrays
    return (SupplierClass *)new_supplier(itemArray, indexArray);
}


/**
 * Rehash a hash collection because of a restore.  This
 * pushes everything into the new collection object.
 *
 * @param newHash The target collection object.
 */
void HashContents::reHash(HashContents *newHash)
{
    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // poke into the other table
            newHash->put(entryValue(position), entryIndex(position));
            // step to the next link in the chain
            position = nextEntry(position);
        }
    }
}


/**
 * Add an element to a hash table without accounting
 * for duplicates.  This is not to be used directly, but
 * collections that allow multiple occurences of an index
 * (e.g relation and bag) should override the put() virtual
 * method and redirect it to this method.
 *
 * @param item   The value to add.
 * @param index  The index this will be added under.
 *
 * @return True if this was successfully added, false if the table
 *         is full.
 */
void HashContents::add(RexxInternalObject *item, RexxInternalObject *index)
{
    // NOTE:  This depends on the caller having checked that there is space.

    // calculate the bucket position
    ItemLink position = hashIndex(index);

    // if the hash slot is empty, we can just fill this in right here
    if (isAvailable(position))
    {
        setEntry(position, item, index);
        // new item, so bump the count
        itemCount++;
        return;
    }

    ItemLink previous = NoLink;

    // this slot is in use.  ok, just run to the end of the chain to find the insertion position.
    do
    {
        // continue running the chain to the end of the list.
        previous = position;
        position = entries[position].next;
    } while (position != NoMore);

    // append a new value.
    append(item, index, previous);
}


/**
 * Add an element to a hash table without accounting
 * for duplicates.  This operation adds the item to the front of
 * the bucket chain so that a search on this index will return
 * this item, essentially overriding any other item already in
 * the collection.
 *
 * @param item   The value to add.
 * @param index  The index this will be added under.
 *
 * @return True if this was successfully added, false if the table
 *         is full.
 */
void HashContents::addFront(RexxInternalObject *item, RexxInternalObject *index)
{
    // NOTE:  This depends on the caller having checked that there is space.

    // calculate the bucket position
    ItemLink position = hashIndex(index);

    // if the hash slot is empty, we can just fill this in right here
    if (isAvailable(position))
    {
        setEntry(position, item, index);
        // new item, so bump the count
        itemCount++;
        return;
    }

    // insert at the front of the bucket.
    insert(item, index, position);
}


/**
 * copy all of the values contained in this table.
 */
void HashContents::copyValues()
{
    for (size_t i = 0; i < bucketSize; i++)
    {
        // the current bucket is the search start, and we always
        // clear out the previous value for each chain start
        ItemLink position = i;

        // ok, run the chain searching for an index match.
        while (position != NoMore && isInUse(position))
        {
            // copy the value at every position
            setValue(position, entryValue(position)->copy());
            position = nextEntry(position);
        }
    }
}


/**
 * set the entry values for a position
 *
 * @param position The table position.
 * @param value    The value to set.
 * @param index    The index to set.
 */
void HashContents::setEntry(ItemLink position, RexxInternalObject *value, RexxInternalObject *index)
{
    setField(entries[position].value, value);
    setField(entries[position].index, index);
}


/**
 * clear an entry in the chain
 *
 * @param position The entry position.
 */
void HashContents::clearEntry(ItemLink position)
{
    // clear out the value/index fields
    setField(entries[position].value, OREF_NULL);
    setField(entries[position].index, OREF_NULL);
    // clear the link also.
    entries[position].next = NoMore;
}


/**
 * set the value in an existing entry
 *
 * @param position The position of the entry.
 * @param value    The value to set.
 */
void HashContents::setValue(ItemLink position, RexxInternalObject *value)
{
    setField(entries[position].value, value);
}


/**
 * Create an iterator for traversing through all of the
 * entries with a given index.
 *
 * @param index  The target index.
 *
 * @return An IndexIterator item.
 */
HashContents::IndexIterator HashContents::iterator(RexxInternalObject *index)
{
    ItemLink position;
    ItemLink previous;

    // try to find the first matching item.  At this point,
    // we don't really care if this succeeds or fails
    locateEntry(index, position, previous);

    return IndexIterator(this, index, position);
}


/**
 * Create an iterator for traversing through all of the
 * entries of the table
 *
 * @return A TableIterator item.
 */
HashContents::TableIterator HashContents::iterator()
{
    ItemLink position = NoMore;
    ItemLink nextBucket = 0;

    // try to find the first real item.
    // we don't really care if this succeeds or fails
    iterateNext(position, nextBucket);

    return TableIterator(this, position, nextBucket);
}


/**
 * Create an iterator for traversing through all of the
 * entries of the table in reverse order
 *
 * @return A ReverseTableIterator item.
 */
HashContents::ReverseTableIterator HashContents::reverseIterator()
{
    ItemLink position = NoMore;
    ItemLink currentBucket = 0;

    // Ue use thnd the first real item.
    // we don't really care if this succeeds or fails
    locateNextBucketEnd(position, currentBucket);

    return ReverseTableIterator(this, position, currentBucket);
}
