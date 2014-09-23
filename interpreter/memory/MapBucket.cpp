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
/* Primitive Hash Table Class                                                 */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "MapBucket.hpp"


/**
 * Allocate a new MapBucket item.
 *
 * @param size    The base object size.
 * @param entries The number of entries we need space for.
 *
 * @return The storage for creating a MapBucket.
 */
void *MapBucket::operator new(size_t size, size_t entries)
{
   size_t bytes = size + (sizeof(MapEntry) * (entries - 1));
   return new_object(bytes, T_MapBucket);
}


/**
 * Construct a MapBucket item of the given size.
 *
 * @param entries The total number of entries in the object.
 */
MapBucket::MapBucket(size_t entries)
{
    totalSize = entries;

    // the bucket size is half the entries size, but we don't want
    // an even value here, so we steal a slot from the overflow area
    bucketSize = entries / 2;
    if (bucketSize % 2 == 0)
    {
        bucketSize++;
    }
    freeItem = entries - 1;         // this is our first free overflow item
    itemCount = 0;                  // nothing in this
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void MapBucket::live(size_t liveMark)
{
    // only the index portions have object references
    for (size_t i = 0; i < totalSize; i++)
    {
        memory_mark(entries[i].index);
    }
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void MapBucket::liveGeneral(MarkReason reason)
{
    // only the index portions have object references
    for (size_t i = 0; i < totalSize; i++)
    {
        memory_mark_general(entries[i].index);
    }
}


/**
 * Remove an element from the table.
 *
 * @param index  The index of the item to remove.
 *
 * @return The value stored at that table, or 0 if nothing was
 *         found.
 */
size_t MapBucket::remove(RexxInternalObject *index)
{
    // get the slot position to start searching.
    MapLink position = hashIndex(index);

    // if there is nothing in the slot postion, we do not
    // have this entry.
    if (entries[position].isAvailable())
    {
        return 0;
    }


    MapLink previous = NoLink;
    // no run the elements chained off of this slot position looking for
    // the target one.
    do
    {
        // got a hit?  We need to fix up the chains to remove this entry
        if (entries[position].isIndex(index))
        {
            // we found our item, so decrement our count.
            itemCount--;
            // save the return value
            size_t removed = entries[position].value;
            // get the next pointer and see what we have
            MapLink next = entries[position].next;

            // this is the last entry in the slot chain.
            if (next == NoMore)
            {
                entries[position].clear();
                // is this the first one in the chain.  Nothing left
                // to do but return
                if (previous == NoLink)
                {
                    return removed;
                }

                // The free position is the highest available free slot.  If
                // this has been removed for a higher position (likely), make this
                // the new free position.
                if (position > freeItem)
                {
                    freeItem = position;
                }
                // chop off the tail position
                entries[previous].next = NoMore;
            }
            // our item has trailing items.  We need to close up the chain.
            else
            {
                // close up the link
                entries[position].copyElement(entries[next]);
                // clean the removed link
                entries[next].clear();
                // fix up the search position for a new item.
                if (freeItem < next)
                {
                    freeItem = next;
                }
            }
            return removed;
        }
        // remember the previous and step
        previous = position;
        position = entries[position].next;

    } while (position != NoMore);

    // the target item not found
    return 0;
}


/**
 * Get the value associated with a given index.
 *
 * @param index The index object.
 *
 * @return The associated value, or 0 for an item that is not there.
 */
size_t MapBucket::get(RexxInternalObject *index)
{
    // get the slot position
    MapLink position = locate(index);

    // if not in the table, return zero
    if (position == NoLink)
    {
        return 0;
    }

    // found an item, return the value.
    return entries[position].value;
}


/**
 * Test if an index exists in the table.
 *
 * @param index The index object.
 *
 * @return The associated value, or 0 for an item that is not there.
 */
bool MapBucket::hasIndex(RexxInternalObject *index)
{
    return locate(index) != NoLink;
}


/**
 * Add an entry to the table, replacing any existing
 * entries.
 *
 * @param value  The value to put into the table.
 * @param index  The index object for the value.
 *
 * @return An indicator that we were unable to add because we're
 *         full.
 */
bool MapBucket::put(size_t value, RexxInternalObject *index)
{
    // indicate an addition failure if we're out of room
    if (isFull())
    {
        return false;
    }

    // calculate the hash slot where we add this.
    MapLink position = hashIndex(index);

    // if this is available, then set the value and return
    if (entries[position].isAvailable())
    {
        // one more item in the chain.
        entries[position].set(index, value);
        itemCount++;
        return true;
    }

    MapLink previous = NoLink;

    // this slot is in use.  We either A) have this entry already, in
    // which case we replace the value, or B) need to add another entry to the end
    do
    {
        // if we got a hit, just change the slot position
        if (entries[position].isIndex(index))
        {
            // everything worked, we have room.
            entries[position].setValue(value);
            return true;
        }
        previous = position;
        position = entries[position].next;
    } while (position != NoMore);

    // append a new value.
    return append(value, index, previous);
}


/**
 * Add a link to a slot position chain.
 *
 * @param value    The value to add.
 * @param index    The index value.
 * @param position The position of the last link in the chain.
 *
 * @return true if we added this successfully, false if we had
 *         an overflow condition.
 */
bool MapBucket::append(size_t value, RexxInternalObject * index, MapLink position)
{
    // the free position is "generally" an empty slot, at least until
    // we start removing things.  We'll need to scan for a real empty one
    for (MapLink over = freeItem; over >= bucketSize; over--)
    {
        // found an empty slot, insert the value here
        if (entries[over].isAvailable())
        {
            // set the value and set the next link to nothing
            entries[over].set(index, value);
            entries[over].next = NoMore;
            // append this to the bucket position chain.
            entries[position].next = over;
            // we start searching for a new slot here the next time.
            freeItem = over - 1;
            // we have another item here.
            itemCount++;
            // this worked fine
            return true;
        }
    }
    // no free slots available...which should not really happen because
    // we checked to see if we were full first.
    return false;
}


/**
 * Empty a MapBucket.
 */
void MapBucket::empty()
{
    // just clear this out
    memset((void *)&entries[0], 0, sizeof(MapEntry) * totalSize);
    // reset the pointes and counters
    itemCount = 0;
    freeItem = totalSize - 1;
}


/**
 * Merge the contents of this bucket into the other bucket.
 *
 * @param other  The target bucket.
 */
void MapBucket::merge(MapBucket *other)
{
    for (size_t i = 0; i < totalSize; i++)
    {
        // if we have a real entry, add these values to the target bucket.
        // this will rehash and redistrubute into the larger table.
        if (!entries[i].isAvailable())
        {
            other->put(entries[i].value, entries[i].index);
        }
    }
}


/**
 * Locate the slot position for an index.
 *
 * @param index The index object.
 *
 * @return The position of the item, or NoLink if this is not
 *         found.
 */
MapBucket::MapLink MapBucket::locate(RexxInternalObject *index)
{
    // get the slot position
    MapLink position = hashIndex(index);

    // nothing in that slot, we don't have this
    if (entries[position].isAvailable())
    {
        return NoLink;
    }

    // loop looking for a match
    do
    {
        // got a match, we can return this now.
        if (entries[position].isIndex(index))
        {
            return position;
        }
        position = entries[position].next;
    } while (position != NoMore);
    // no entry found
    return NoLink;
}


/**
 * Increment the value associated with a key.  If the key does
 * not exist, it is inserted into the table with a value of 1.
 *
 * @param key    The target key.
 */
bool MapBucket::increment(RexxInternalObject *key)
{
    // locate the item
    MapLink position = locate(key);

    // bail if not in the table, add with a value of 1
    if (position == NoLink)
    {
        return put(1, key);
    }
    // increment the value
    entries[position].value++;
    return true;
}


/**
 * Decrement the value associated with a key.  If the value goes
 * to zero, the key is removed.
 *
 * @param key    The target key.
 */
void MapBucket::decrement(RexxInternalObject *key)
{
    // locate the item
    MapLink position = locate(key);

    // bail if
    if (position == NoLink)
    {
        return;
    }

    // if this is already zero, remove the item
    if (entries[position].value == 0)
    {
        remove(key);
        return;
    }

    // decrement the value
    entries[position].value--;

    // and test again to see if we went to zero
    if (entries[position].value == 0)
    {
        remove(key);
    }
}
