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
/* Base class for a hash-based collection object.                             */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "HashCollection.hpp"
#include "ArrayClass.hpp"
#include "MethodArguments.hpp"

/**
 * construct a HashCollection with a given size.
 *
 * @param capacity The required capacity.
 */
HashCollection::HashCollection(size_t capacity)
{
    // get a suggested bucket size for this capacity
    size_t bucketSize = calculateBucketSize(capacity);
    contents = allocateContents(bucketSize, bucketSize *2);
}


/**
 * Virtual method for allocating a new contents item for this
 * collection.  Collections with special requirements should
 * override this and return the appropriate subclass.
 *
 * @param bucketSize The bucket size of the collection.
 * @param totalSize  The total capacity of the collection.
 *
 * @return A new HashContents object appropriate for this collection type.
 */
HashContents *HashCollection::allocateContents(size_t bucketSize, size_t totalSize)
{
    return new (totalSize) HashContents(bucketSize, totalSize);
}


/**
 * Expand the contents of a collection.  We try for double the
 * size.
 */
void HashCollection::expandContents()
{
    // just double the bucket size...or there abouts
    expandContents(contents->capacity() * 2);
}


/**
 * Expand the contents of a collection to a given bucket
 * capacity.
 */
void HashCollection::expandContents(size_t capacity )
{
    size_t bucketSize = calculateBucketSize(capacity);
    Protected<HashContents> newContents = allocateContents(bucketSize, bucketSize * 2);
    // copy all of the items into the new table
    contents->reMerge(newContents);
    // if this is a contents item in the old space, we need to
    // empty this so any old-to-new table entries in the contents can get updated.
    if (contents->isOldSpace())
    {
        contents->empty();
    }
    // replace the contents
    setField(contents, newContents);
}


/**
 * Ensure that the collection has sufficient space for a
 * mass incoming addition.
 *
 * @param delta  The number of entries we wish to add.
 */
void HashCollection::ensureCapacity(size_t delta)
{
    // not enough space?  time to expand.  We'll go to
    // the current total capacity plus the delta.
    if (!contents->hasCapacity(delta))
    {
        expandContents(contents->capacity() + delta)
    }
}


/**
 * Calculate an optimal bucket size for a given capacity.  This
 * is derived from how Java calculates this.  This will generally
 * allocate more entries than asked for, but will generally
 * have a lower collision rate than using it directly.
 *
 * @param capacity The desired capacity.
 *
 * @return The calculated capacity.
 */
size_t HashCollection::calculateBucketSize(size_t capacity)
{
    // if this is a request for a very large table, cap the size.
    // it is likely we'll never be able to even allocate a bucket that large!
    if (capacity >= 1 << 30)
    {
        return 1 << 30;
    }

    if (capacity < MinimumBucketSize)
    {
        return MinimumBucketSize;
    }

    // now hash up the requested value a bit to get a capacity
    capacity = capacity - 1;
    capacity |= capacity >> 1;
    capacity |= capacity >> 2;
    capacity |= capacity >> 4;
    capacity |= capacity >> 8;
    capacity |= capacity >> 16;

    // we prefer to have a odd number for the bucket size, so add
    // 1 if we end up with an even number.
    if ((capacity | 1) == 0)
    {
        capacity += 1
    }

    return 1;
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void HashCollection::live(size_t liveMark)
{
    memory_mark(contents);
    memory_mark(objectVariables);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void HashCollection::liveGeneral(MarkReason reason)
{
    memory_mark_general(contents);
    memory_mark_general(objectVariables);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void HashCollection::flatten(RexxEnvelope *envelope)
{
    setUpFlatten(HashCollection)

    flattenRef(contents);
    flattenRef(objectVariables);

    cleanUpFlatten
}


/**
 * Handle an unflatten operation on a hash-based table.
 *
 * @param envelope The envelope that is handling the unflatten.
 */
RexxObject *HashCollection::unflatten(RexxEnvelope *envelope)
{
    // just add this as a table to the envelope.
    envelope->addTable(this);
    return this;
}

/**
 * TODO:  The makeProxy object needs to be in the directory class, since that
 * is where it happens.
 *
 * Copy a hash-based collection object.
 *
 * @return A new instance of this collection.
 */
RexxObject *HashCollection::copy()
{
    // make a copy of the base object
    HashCollection *newObj = (HashCollection *)RexxObject::copy();
    // and copy the contents as well
    newObj->contents = (HashContents *)contents->copy();
    return newObj;
}


/**
 * Implement the makearray function for the collection.
 *
 * @return An array of the collection indexes.
 */
RexxArray *HashCollection::makeArray(void)
{
    return contents->makeArray();
}


/**
 * Merge an item into the contents of this collection.
 *
 * @param value  The value to add
 * @param index  The index of the added value.
 */
void HashCollection::mergeItem(RexxInternalObject *value, RexxInternalObject *index)
{
    // try to add first.  If there is a failure,
    // we need to expand our contents and try again
    if (!contents->mergeItem(value, index))
    {
        expandContents();
        contents->mergeItem(value, index);
    }
}


/**
 * Validate an index for an operation.  Subclasses with
 * special index requirements should override.
 *
 * @param index    The method index value.
 * @param position The argument position for error reporting.
 */
void HashCollection::validateIndex(RexxInternalObject *index, size_t position)
{
    requiredArgument(index, position);    // make sure we have an index
}


/**
 * The exported remove() method for hash collection
 * classes.  This is the Rexx stub method.  The removal
 * operation is delegated to the virtual method defined
 * by the implementing class.
 *
 * @param _index The target removal index.
 *
 * @return The removed object, or .nil if the index was not found.
 */
RexxInternalObject *HashCollection::removeRexx(RexxInternalObject *index)
{
    // validate the argument for this collection type
    validateIndex(index, ARG_ONE);

    // remove the item...if it was not there, then return .NIL.
    return resultOrNil(remove(index));
}


/**
 * Base virtual function for a table remove operation.
 * This applies object equality semantics to the operation.
 *
 * @param _index The object index.
 *
 * @return The removed object.
 */
RexxInternalObject *HashCollection::remove(RexxInternalObject *index)
{
    return contents->remove(index);
}

/**
 * Retrieve all objects with the same index.  Only used
 * for bag and relation.
 *
 * @param index  The target index.
 *
 * @return An array of all matching index items.
 */
RexxArray *HashCollection::allAtRexx(RexxInternalObject *index)
{
    validateindex(index, ARG_ONE);
    return contents->getAll(index);
}


/**
 * Exported get() accessor for a hash table collection.
 * This delegates to a virtual method defined by the
 * target collection.
 *
 * @param _index The target index.
 *
 * @return The fetched object, or .nil if the index does not
 *         exist in the collection.
 */
RexxInternalObject *HashCollection::getRexx(RexxInternalObject *index)
{
    validateindex(index, ARG_ONE);
    return resultOrNil(get(index));
}


/**
 * Retrieve an item from a hash collection using a key.
 * This is the base virtual implementation, which uses
 * equality semantics for the retrieveal.  Other implementations
 * may override this.
 *
 * @param key    The target key.
 *
 * @return The retrieved object.  Returns OREF_NULL if the object
 *         was not found.
 */
RexxInternalObject *HashCollection::get(RexxInternalObject *key)
{
    return contents->get(key);
}


/**
 * Exported Rexx method for adding an item to a collection.
 * The put operation is delegated to the implementing
 * class virtual function.
 *
 * @param _value The value to add.
 * @param _index The index for the added item.
 *
 * @return Always returns OREF_NULL.
 */
RexxInternalObject *HashCollection::putRexx(RexxInternalObject *item, RexxInternalObject *index)
{
    // validate both the index and value
    requiredArgument(value, ARG_ONE);
    validateindex(index, ARG_TWO);
    put(value, index);
    // always returns nothing
    return OREF_NULL;
}


/**
 * Place an item into a hash collection using a key.
 * This is the base virtual implementation, which uses
 * equality semantics for the retrieveal.  Other implementations
 * may override this.
 *
 * @param _value The inserted value.
 * @param _index The insertion key.
 *
 * @return The retrieved object.  Returns OREF_NULL if the object
 *         was not found.
 */
void HashCollection::put(RexxInternalObject *value, RexxInternalObject *index)
{
    // try to add first.  If there is a failure,
    // we need to expand our contents and try again
    if (!contents->put(value, index))
    {
        expandContents();
        contents->put(value, index);
    }
}


/**
 * Merge a collection into another similar collection.  This
 * is just used internally.
 *
 * @param target The merge target.
 *
 * @return Nothing.
 */
void HashCollection::merge(HashCollection * target)
{
    // give the target collection a nudge about how many items we
    // wish to add.  This will speed up the merger by potentially reducing
    // the number of reallocations required.
    target->ensureCapacity(contents->items());
    // and do the merger.
    return contents->merge(target);
}


/**
 * Copy a set of values contained within a hash collection.  Usually
 * done after a copy operation (e.g., for a variable dictionary).
 */
void HashCollection::copyValues()
{
    contents->copyValues();
}


/**
 * Test for the existence of an index in the collection.
 * This uses the get() virtual function to determine if
 * the item exists.
 *
 * @param _index The target index.
 *
 * @return True if the index exists, false if the index does not
 *         exist.
 */
RexxInternalObject *HashCollection::hasIndexRexx(RexxInternalObject *index)
{
    validateIndex(index, ARG_ONE);
    return hasItem(index) ? TheTrueObject : TheFalseObject;
}


/**
 * Virtual method for checking an item index.
 *
 * @param index  The target index.
 *
 * @return true if the index exists, false otherwise.
 */
bool HashCollection::hasItem(RexxInternalObject *index)
{
    return contents->hasItem();
}


/**
 * Retrieve an index for a given item.  Which index is returned
 * is indeterminate.
 *
 * @param target The target object.
 *
 * @return The index for the target object, or .nil if no object was
 *         found.
 */
RexxInternalObject *HashCollection::indexRexx(RexxInternalObject *target)
{
    // required argument
    requiredArgument(target, ARG_ONE);
    // retrieve this from the hash table
    return resultOrNile(getIndex(target));
}


/**
 * Retrieve an index for a given item.  Which index is returned
 * is indeterminate.
 *
 * @param target The target object.
 *
 * @return The index for the target object, or .nil if no object was
 *         found.
 */
RexxInternalObject *HashCollection::getIndex(RexxInternalObject *target)
{
    // retrieve this from the hash table
    return contents->getIndex(target);
}


/**
 * Exported method to remove an item specified by value.
 *
 * @param target The target object.
 *
 * @return The target object again.
 */
RexxInternalObject *HashCollection::removeItemRexx(RexxInternalObject *target)
{
    // required argument
    requiredArgument(target, ARG_ONE);
    // the actual target class may use different semantics for this.
    return resultOrNil(removeItem(target));
}


/**
 * Remove an item specified by value.
 *
 * @param target The target object.
 *
 * @return The target object again.
 */
RexxInternalObject *HashCollection::removeItem(RexxInternalObject *target)
{
    // the contents handle all of this.
    return contents->removeItem(target);
}


/**
 * Test if a given item exists in the collection.
 *
 * @param target The target object.
 *
 * @return .true if the object exists, .false otherwise.
 */
RexxInternalObject *HashCollection::hasItemRexx(RexxInternalObject *target)
{
    requiredArgument(target, ARG_ONE);
    return this->hasItem(target);
}


/**
 * Test if a given item exists in the collection.
 *
 * @param target The target object.
 *
 * @return .true if the object exists, .false otherwise.
 */
RexxInternalObject *HashCollection::hasItem(RexxInternalObject *target)
{
    return this->contents->hasItem(target);
}


/**
 * Create a collecton supplier
 *
 * @return The supplier object.
 */
RexxSupplier *HashCollection::supplier()
{
    return contents->supplier();
}


/**
 * Retrieve all items of the collection, as an array.
 *
 * @return An array of the items.
 */
RexxArray *HashCollection::allItems()
{
    return contents->allItems();
}


/**
 * Retrieve all indexes of the collection, as an array.
 *
 * @return An array with all of the collection indexes.
 */
RexxArray *HashCollection::allIndexes()
{
    return contents->allIndexes();
}

/**
 * Return the unique indexes in a hash collection.
 *
 * @return The set of uniqueIndexes
 */
RexxArray *HashCollection::uniqueIndexes()
{
    return contents->uniqueIndexes();
}


/**
 * Empty a hash table collection.
 *
 * @return nothing
 */
RexxObject *HashCollection::emptyRexx()
{
    contents->empty();
    return OREF_NULL;
}


/**
 * Empty a hash table collection.
 *
 * @return nothing
 */
void HashCollection::empty()
{
    contents->empty();
}


/**
 * Test if a HashTableCollection is empty.
 *
 * @return
 */
RexxObject *HashCollection::isEmptyRexx()
{
    return contents->isEmpty() ? TheTrueObject : TheFalseObject;
}


/**
 * Test if a HashTableCollection is empty.
 *
 * @return
 */
bool HashCollection::isEmpty()
{
    return contents->isEmpty();
}
