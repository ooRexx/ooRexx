/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* Primitive List Class                                                       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "ListClass.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "WeakReferenceClass.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *ListClass::classInstance = OREF_NULL;
const size_t ListClass::DefaultListSize = 8;


/**
 * Create initial class object at bootstrap time.
 */
void ListClass::createInstance()
{
    CLASS_CREATE(List);
}


/**
 * Create a new list object
 *
 * @param size   the size of the object.
 *
 * @return Storage for creating the object.
 */
void *ListClass::operator new (size_t size)
{
    return new_object(size, T_List);
}


/**
 * construct a list object with the given size.
 *
 * @param capacity The required capacity.
 */
ListClass::ListClass(size_t capacity)
{
    initialize(capacity);
}


/**
 * The init method for this class.  This does delayed
 * initialization of this object until a INIT message is
 * received during initialization.
 *
 * @param initialSize
 *               The initial list size (optional)
 *
 * @return Always returns nothing
 */
RexxObject *ListClass::initRexx(RexxObject *initialSize)
{
    // the capacity is optional, but must be a positive numeric value
    size_t capacity = optionalLengthArgument(initialSize, DefaultListSize, ARG_ONE);
    initialize(capacity);
    return OREF_NULL;
}


/**
 * Initialize the list contents, either directly from the
 * low level constructor or from the INIT method.
 *
 * @param capacity The requested capacity.
 */
void ListClass::initialize(size_t capacity)
{
    // only do this if we have no contents already
    if (contents == OREF_NULL)
    {
        // we don't want to create a zero sized contents, so cap the
        // minimum size at the default
        capacity = std::max(capacity, DefaultListSize);
        // and allocate the initial contents item.
        contents = new (capacity) ListContents(capacity);
    }
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void ListClass::live(size_t liveMark)
{
    memory_mark(contents);
    memory_mark(objectVariables);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void ListClass::liveGeneral(MarkReason reason)
{
    memory_mark_general(contents);
    memory_mark_general(objectVariables);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void ListClass::flatten(Envelope *envelope)
{
    setUpFlatten(ListClass)

    flattenRef(contents);
    flattenRef(objectVariables);

    cleanUpFlatten
}


/**
 * Create a copy of a list table item.
 *
 * @return The object copy.
 */
RexxInternalObject *ListClass::copy()
{
    // make a copy of ourself (also copies the object variables)
    ListClass *newlist = (ListClass *)this->RexxObject::copy();
    // copy the backing contents
    newlist->contents = (ListContents *)contents->copy();
    return newlist;
}


/**
 * Convert a provided index value into the
 * target item in the contents.
 *
 * @param index    The provided index.
 * @param position The argument position
 *
 * @return The validated location of the targetted entry.
 */
ListContents::ItemLink ListClass::validateIndex(RexxObject *index, size_t position)
{
    // this is required
    requiredArgument(index, position);

    size_t item_index;
    // converted using the ARGUMENT_DIGITS value
    if (!index->unsignedNumberValue(item_index, Numerics::ARGUMENT_DIGITS))
    {
        reportException(Error_Incorrect_method_index, index);
    }
    // if not valid, return the no link marker
    if (!contents->isIndexValid(item_index))
    {
        return ListContents::NoLink;
    }
    return item_index;
}


/**
 * Convert a provided index value into the
 * target item in the contents.  For this version, .nil is a
 * valid index and is indicated by returning NoLink.  Otherwise,
 * this must point to a real value
 *
 * @param index    The provided index.
 * @param position The argument position
 *
 * @return The validated location of the targetted entry.
 */
ListContents::ItemLink ListClass::validateInsertionIndex(RexxObject *index, size_t position)
{
    // this is a special index for insertion operations
    if (index == TheNilObject)
    {
        return ListContents::AtBeginning;
    }
    else if (index == OREF_NULL)
    {
        return ListContents::AtEnd;
    }
    // map this to a valid index position
    return requiredIndex(index, position);
}


/**
 * Convert an argument value into an index position that
 * MUST be in the table.
 *
 * @param index    The argument index value.
 * @param position The argument position for the index.
 *
 * @return The converted index position.
 */
ListContents::ItemLink ListClass::requiredIndex(RexxObject *index, size_t position)
{
    // validate
    ListContents::ItemLink convertedIndex = validateIndex(index, position);
    // if does not map to an item, this is an error.
    if (convertedIndex == ListContents::NoLink)
    {
        reportException(Error_Incorrect_method_index, index);
    }
    return convertedIndex;
}


/**
 * Expand the contents of a collection.  We try for double the
 * size.
 */
void ListClass::expandContents()
{
    // double the bucket size for small Lists
    // for Lists above the limit, just add half of the actual bucket size
    size_t size = contents->capacity();
    size += size <= ExpansionDoubleLimit ? size : size / 2;
    expandContents(size);
}


/**
 * Expand the contents of a collection to a given bucket
 * capacity.
 */
void ListClass::expandContents(size_t capacity )
{
    // allocate a new table with the requested capacity, then merge the
    // contents back into the the new one before replacing.
    Protected<ListContents> newContents = new (capacity) ListContents(capacity);
    contents->mergeInto(newContents);

    setField(contents, (ListContents *)newContents);
}


/**
 * Ensure that the collection has sufficient space for a
 * mass incoming addition.
 *
 * @param delta  The number of entries we wish to add.
 */
void ListClass::ensureCapacity(size_t delta)
{
    // not enough space?  time to expand.  We'll go to
    // double the bucket size for small Lists
    // for Lists above the limit, just add half of the actual bucket size
    if (!contents->hasCapacity(delta))
    {
        size_t size = contents->capacity();
        size += size <= ExpansionDoubleLimit ? size : size / 2;
        expandContents(size);
    }
}


/**
 * If the contents think it is time to expand, then increase the
 * contents size.
 */
void ListClass::checkFull()
{
    if (contents->isFull())
    {
        expandContents();
    }
}


/**
 * Utility method for formatting an index value into an object.
 *
 * @param index  The index to return.
 *
 * @return Either an integer version of the index or TheNilObject if the index is out of bounds.
 */
RexxObject *ListClass::indexObject(ListContents::ItemLink index)
{
    return index == ListContents::NoMore ? TheNilObject : new_integer(index);
}


/**
 * Put a value into the list, replacing the existing value.
 *
 * @param value  The value to insert.
 * @param index  The target index.
 *
 * @return returns nothing.
 */
RexxInternalObject *ListClass::putRexx(RexxObject *value, RexxObject *argIndex)
{
    ListContents::ItemLink index = requiredIndex(argIndex, ARG_TWO);

    // do the actual replacement.
    put(value, index);
    return OREF_NULL;
}


/**
 * Low level API for putting a value into the list.
 *
 * @param value  The value to put.
 * @param index  The index position (non-validated).
 */
void ListClass::put(RexxInternalObject *value, size_t index)
{
    // make sure we have enough space to add and then
    // have the contents add this.
    checkFull();
    contents->put(value, index);
}


/**
 * Get a value from the list at the provided index.
 *
 * @param index  The target index.
 *
 * @return returns nothing.
 */
RexxInternalObject *ListClass::getRexx(RexxObject *argIndex)
{
    ListContents::ItemLink index = validateIndex(argIndex, ARG_TWO);

    return resultOrNil(get(index));
}


/**
 * Low level API for getting a value from the list
 *
 * @param index  The index position (non-validated).
 */
RexxInternalObject *ListClass::get(size_t index)
{
    return contents->get(index);
}


/**
 * Snip out a section of the list.
 *
 * @param argIndex The starting list index.
 * @param count    The count of items to include in the section.
 *
 * @return A new list object filled with the subset items.
 */
RexxObject *ListClass::sectionRexx(RexxObject *argIndex, RexxObject *count)
{
    ListContents::ItemLink index = requiredIndex(argIndex, ARG_ONE);
    size_t counter = optionalLengthArgument(count, SIZE_MAX, ARG_TWO);

    // pass off to the lower level method
    return section(index, counter);
}


/**
 * Low-level section method for a list.
 *
 * @param index  The starting index (non-validated)
 * @param count  The count of items to include.
 *
 * @return A new list item containing the requested items.
 */
ListClass *ListClass::section(size_t index, size_t count)
{
    // create a new list to add things in.  It would be nice to allocate
    // this at the required size, but we don't really know how many
    // items we'll put in the list, so just use a default value.
    Protected<ListClass> result = new ListClass;

    // grab as many items as we can.
    while (index != ListContents::NoMore && count-- > 0)
    {
        // append to the list
        result->append(contents->get(index));
        index = contents->nextIndex(index);
    }
    return result;
}


/**
 * Add a new element to the list at the given insertion point.
 * TheNilObject indicates it should be added to the list end
 *
 * @param value  The value to insert
 * @param index  The index value for the insertion point.
 *
 * @return The index for the inserted item.
 */
RexxObject *ListClass::insertRexx(RexxObject *value, RexxObject *index)
{
    // figure out where to insert.
    ListContents::ItemLink insertionPoint = validateInsertionIndex(index, ARG_TWO);

    // insert, and return the insertion index as an object.
    return new_integer(insert(value, insertionPoint));
}


/**
 * Primitive insert operation to a List.
 *
 * @param insertionPoint
 *               The non-validated insertion index.  NoLink means add
 *               to the list end.
 *
 * @return The index of the added item.
 */
size_t ListClass::insert(RexxInternalObject *value, size_t insertionPoint)
{
    // make sure we have enough space to add and then
    // have the contents add this.
    checkFull();
    return contents->insert(value, insertionPoint);
}


/**
 * Add an object to the end of the list.
 *
 * @param value  The value to add.
 *
 * @return The insertion index.
 */
size_t ListClass::addLast(RexxInternalObject *value)
{
    // make sure we have enough space to add and then
    // have the contents add this.
    checkFull();
    return contents->insert(value, ListContents::AtEnd);
}


/**
 * Add a value to the front of the list.
 *
 * @param value  The value to add.
 *
 * @return The index of the new item.
 */
size_t ListClass::addFirst(RexxInternalObject *value)
{
    // make sure we have enough space to add and then
    // have the contents add this.
    checkFull();
    return contents->insert(value, ListContents::AtBeginning);
}


/**
 * Append an item after the last item in the list.
 *
 * @param value  The value to append.
 *
 * @return The index of the appended item.
 */
size_t ListClass::append(RexxInternalObject *value)
{
    // make sure we have enough space to add and then
    // have the contents add this.
    checkFull();
    return contents->insertAtEnd(value);
}


/**
 * The rexx version of the append method.
 *
 * @param value  The value to append.
 *
 * @return An object version of the index.
 */
RexxObject *ListClass::appendRexx(RexxObject *value)
{
    requiredArgument(value, ARG_ONE);
    return new_integer(append(value));
}

/**
 * Remove an item at the given index from the list.
 *
 * @param index  The index to remove.
 *
 * @return The removed item.
 */
RexxObject *ListClass::removeRexx(RexxObject *argIndex)
{
    // out of bounds is not an issue here...
    ListContents::ItemLink index = validateIndex(argIndex, ARG_ONE);
    return resultOrNil(remove(index));
}


/**
 * The low-level remove API
 *
 * @param index  The target index to remove.
 *
 * @return The removed object, or OREF_NULL if this index does not exist.
 */
RexxInternalObject *ListClass::remove(size_t index)
{
    return contents->remove(index);
}


/**
 * Return the first item in the list.
 *
 * @return The item, or .nil if the list is empty.
 */
RexxObject *ListClass::firstItemRexx()
{
    return resultOrNil(firstItem());
}


/**
 * Low level access to the firstItem function.
 *
 * @return The first item in the list or OREF_NULL if the list is empty.
 */
RexxInternalObject *ListClass::firstItem()
{
    return contents->getFirstItem();
}


/**
 * Rexx access to the first item of the list.
 *
 * @return The first item, or .nil if the list is empty.
 */
RexxObject *ListClass::lastItemRexx()
{
    return resultOrNil(lastItem());
}


/**
 * Low level access to the lastItem function.
 *
 * @return The last item in the list or OREF_NULL if the list is
 *         empty.
 */
RexxInternalObject *ListClass::lastItem()
{
    return contents->getLastItem();
}


/**
 * Get the index of the first item in the list.
 *
 * @return The index value, or the .nil if the list is empty.
 */
RexxObject *ListClass::firstRexx()
{
    return indexObject(firstIndex());
}


/**
 * Low level access to the firstItem function.
 *
 * @return The first index in the list or NoMore if the list is
 *         empty.
 */
size_t ListClass::firstIndex()
{
    return contents->firstIndex();
}


/**
 * Get the index of the last item in the list.
 *
 * @return The index value, or the .nil if the list is empty.
 */
RexxObject *ListClass::lastRexx()
{
    return indexObject(lastIndex());
}


/**
 * Low level access to the firstItem function.
 *
 * @return The last index in the list or NoMore if the list is
 *         empty.
 */
size_t ListClass::lastIndex()
{
    return contents->lastIndex();
}


/**
 * Step from a given index to the next element in the list.
 *
 * @param index  The starting index
 *
 * @return The indext of the next item, or .nil if their is no next item.
 */
RexxObject *ListClass::nextRexx(RexxObject *argIndex)
{
    ListContents::ItemLink index = validateIndex(argIndex, ARG_ONE);
    return indexObject(nextIndex(index));
}


/**
 * Step from a given index to the next element in the list.
 *
 * @param index  The starting index
 *
 * @return The indext of the next item, or .nil if their is no next item.
 */
RexxObject *ListClass::previousRexx(RexxObject *argIndex)
{
    ListContents::ItemLink index = validateIndex(argIndex, ARG_ONE);
    return indexObject(previousIndex(index));
}


/**
 * A low-level next() method for internal usage.  This works
 * directly off the index values without needing to create
 * object instances.  This is critical for some of the internal
 * data structures implemented as lists.
 *
 * @param index The target item index.
 *
 * @return The index of the next item, or NoMore if there is no
 *         next item.
 */
size_t ListClass::nextIndex(size_t index)
{
    return contents->nextIndex(index);
}


/**
 * A low-level previous() method for internal usage.  This works
 * directly off the index values without needing to create
 * object instances.  This is critical for some of the internal
 * data structures implemented as lists.
 *
 * @param _index The target item index.
 *
 * @return The index of the previous item, or NoMore if there is
 *         no previous item.
 */
size_t ListClass::previousIndex(size_t index)
{
    return contents->previousIndex(index);
}


/**
 * Test if the list has a given index value.
 *
 * @param index  The target index.
 *
 * @return True if this is a valid index in this list, false
 *         otherwise.
 */
RexxObject *ListClass::hasIndexRexx(RexxObject *argIndex)
{
    ListContents::ItemLink index = validateIndex(argIndex, ARG_ONE);
    return booleanObject(hasIndex(index));
}


/**
 * Low-level test of index existance.
 *
 * @param index  The target index.
 *
 * @return true if the index exists, false otherwise.
 */
bool ListClass::hasIndex(size_t index)
{
    return contents->isIndexValid(index);
}


/**
 * Process a request to convert this list to an array.
 *
 * @return An array containing all of the list items, in the current order.
 */
ArrayClass *ListClass::requestArray()
{
    return allItems();
}


/**
 * Process a request to convert this list to an array.
 *
 * @return An array containing all of the list items, in the current order.
 */
ArrayClass *ListClass::makeArray()
{
    return allItems();           // this is just all of the array items.
}


/**
 * Return an array containing all elements contained in the list,
 * in sorted order.
 *
 * @return An array with the list elements.
 */
ArrayClass *ListClass::allItems()
{
    return contents->allItems();
}


/**
 * Empty all of the items from a list.
 *
 * @return No return value.
 */
RexxObject *ListClass::emptyRexx()
{
    empty();
    return this;          // return receiving Array
}


/**
 * Low-level API for emptying a list.
 */
void ListClass::empty()
{
    contents->empty();
}


/**
 * Test if a list is empty.
 *
 * @return True if the list is empty, false otherwise
 */
RexxObject *ListClass::isEmptyRexx()
{
    return booleanObject(isEmpty());
}


/**
 * Test if a list is empty.
 *
 * @return True if the list is empty, false otherwise
 */
bool ListClass::isEmpty()
{
    return contents->isEmpty();
}


/**
 * Return an array containing all elements contained in the list,
 * in sorted order.
 *
 * @return An array with the list elements.
 */
ArrayClass *ListClass::allIndexes()
{
    return contents->allIndexes();
}


/**
 * Return the index of the first item with a matching value
 * in the list.  Returns .nil if the object is not found.
 *
 * @param target The target object.
 *
 * @return The index of the item, or .nil.
 */
RexxObject *ListClass::indexRexx(RexxObject *target)
{
    // we require the index to be there.
    requiredArgument(target, ARG_ONE);

    ListContents::ItemLink itemIndex = getIndex(target);
    return indexObject(itemIndex);
}


/**
 * Get the index associated with a value stored in the list.
 *
 * @param target The search item.
 *
 * @return The index, or NoMore if no matching item is found.
 */
size_t ListClass::getIndex(RexxInternalObject *target)
{
    return contents->getIndex(target);
}


/**
 * Tests whether there is an object with the given value in the
 * list.
 *
 * @param target The target value.
 *
 * @return .true if there is a match, .false otherwise.
 */
RexxObject *ListClass::hasItemRexx(RexxObject *target)
{
    // we require the index to be there.
    requiredArgument(target, ARG_ONE);
    return booleanObject(contents->hasItem(target));
}


/**
 * Test of the list contains a given target object.
 *
 * @param target The target object.
 *
 * @return true if the object is found, false otherwise.
 */
bool ListClass::hasItem(RexxInternalObject *target)
{
    return contents->hasItem(target);
}


/**
 * Removes an item from the collection.
 *
 * @param target The target value.
 *
 * @return The target item.
 */
RexxInternalObject *ListClass::removeItemRexx(RexxObject *target)
{
    // we require the index to be there.
    requiredArgument(target, ARG_ONE);

    return resultOrNil(removeItem(target));
}


/**
 * Remove an Item from the list.
 *
 * @param target The search item.
 *
 * @return The existing item.
 */
RexxInternalObject *ListClass::removeItem(RexxInternalObject *target)
{
    return contents->removeItem(target);
}


/**
 * Return a supplier for iterating over the list contents.
 *
 * @return A supplier for the current list.
 */
SupplierClass *ListClass::supplier()
{
    return contents->supplier();
}


/**
 * The Rexx version of the list items() method.
 *
 * @return The count of items in the list.
 */
RexxObject *ListClass::itemsRexx()
{
    return new_integer(items());
}


/**
 * Low-level items function.
 *
 * @return The count of items in the list.
 */
size_t ListClass::items()
{
    return contents->items();
}


/**
 * Scan the list and dereference the weak references,
 * returning an array of all of the active objects.
 *
 * @return An array with the active references.
 */
ArrayClass *ListClass::weakReferenceArray()
{
    return contents->weakReferenceArray();
}


/**
 * Rexx method for allocating a List item.
 *
 * @param init_args The new args.
 * @param argCount  The count of arguments.
 *
 * @return A new List instance.
 */
ListClass *ListClass::newRexx(RexxObject **init_args, size_t  argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // this version does not create the contents
    Protected<ListClass> newList = new ListClass(true);

    // handle Rexx class completion
    classThis->completeNewObject(newList, init_args, argCount);

    // this is a hedge against someone not forwarding the INIT method
    // to the base list class in their subclass INIT method.  This can
    // prevent some crashes.
    newList->initialize();
    return newList;
}


/**
 * Create a list item and populate with the argument items.
 *
 * @param args     Pointer to the arguments.
 * @param argCount The number of arguments.
 *
 * @return The populated list object.
 */
ListClass *ListClass::ofRexx(RexxObject **args, size_t  argCount)
{
    // it would be nice to create an object of the size we need, but
    // that is a little complicated from this code.  It's probably not
    // worth the effort because normally an OF() call has a relatively
    // small number of arguments.

    // create a list item of the appopriate type.
    Protected<ListClass> newList = newRexx(NULL, 0);

    // add all of the arguments
    for (size_t i = 0; i < argCount; i++)
    {
        RexxObject *item = args[i];
        // omitted arguments not allowed here.
        if (item == OREF_NULL)
        {
            reportException(Error_Incorrect_method_noarg, i + 1);
        }
        newList->append(item);
    }
    return newList;
}

