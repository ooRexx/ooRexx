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


/**
 * Create initial class object at bootstrap time.
 */
void ListClass::createInstance()
{
    CLASS_CREATE(List, "List", RexxClass);
}


/**
 * Create a copy of a list table item.
 *
 * @return The object copy.
 */
RexxObject *ListClass::copy()
{
    // make a copy of ourself (also copies the object variables)
    ListClass *newlist = (ListClass *)this->RexxObject::copy();
    // copy the backing contents
    newlist->table = (ListContents *)contents->copy();
    return newlist;
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
    memory_mark_general(table);
    memory_mark_general(objectVariables);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void ListClass::flatten(RexxEnvelope *envelope)
{
    setUpFlatten(ListClass)

    flattenRef(table);
    flattenRef(objectVariables);

    cleanUpFlatten
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
ItemLink ListClass::validateIndex(RexxObject *index, size_t position)
{
    // this is required
    requiredArgument(index, position);
    stringsize_t    item_index;
    // converted using the ARGUMENT_DIGITS value
    if (!argument->unsignedNumberValue(value, Numerics::ARGUMENT_DIGITS))
    {
        reportException(Error_Incorrect_method_index, index);
    }
    // if not valid, return the no link marker
    if (!contents->validateIndex(item_index))
    {
        return NoLink;
    }
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
ItemLink ListClass::validateInsertionIndex(RexxObject *index, size_t position)
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
ItemLink ListClass::requiredIndex(RexxObject *index, size_t position)
{
    // validate
    ItemLink convertedIndex = validateIndex(index, position);
    // if does not map to an item, this is an error.
    if (convertedIndex == NoLink)
    {
        reportException(Error_Incorrect_method_index, index);
    }
    return index
}


/**
 * Expand the contents of a collection.  We try for double the
 * size.
 */
void ListClass::expandContents()
{
    // just double the bucket size...or there abouts

    // TODO:  need to cap out the increment value...
    expandContents(contents->capacity() * 2);
}


/**
 * Expand the contents of a collection to a given bucket
 * capacity.
 */
void ListClass::expandContents(size_t capacity )
{
    // allocate a new table with the requested capacity, then merge the
    // contents back into the the new one before replacing.
    Protected<HashContents> newContents = new ListContents(capacity);
    contents->mergeInto(newContents);

    setField(contents, newContents;)
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
    // the current total capacity plus the delta...or the standard
    // doubling if the delta is a small value.
    if (!contents->hasCapacity(delta))
    {
        expandContents(contents->capacity() + Numerics::maxVal(delta, contents->capacity());
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
 * Put a value into the list, replacing the existing value.
 *
 * @param value  The value to insert.
 * @param index  The target index.
 *
 * @return returns nothing.
 */
RexxInternalObject *ListClass::putRexx(RexxInternalObject *value, RexxObject *argIndex)
{
    ItemLink index = requiredIndex(value, ARG_TWO);

    // do the actual replacement.
    put(value, index);
    /* replace the value                 */
    OrefSet(this->table, element->value, _value);
    return OREF_NULL;                    /* return nothing at all             */
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
    contents->put(value, index);
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
    ItemLink index = requiredIndex(argIndex, ARG_ONE);
    size_t counter = optionalLengthArgument(count, SIZE_MAX, ARG_TWO);

    // pass off to the lower level method
    return section(index, count);
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
    while (index != NoMore && count-- > 0)
    {
        // append to the list
        result->append(entryValue(index));
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
RexxObject *ListClass::insertRexx(RexxInternalObject *value, RexxObject *index)
{
    // figure out where to insert.
    ItemLink insertionPoint = validateInsertionIndex(index, ARG_TWO);
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
    return contents->insertAtEnd(value, ListContents::AtEnd);
}


/**
 * Add a value to the front of the list.
 *
 * @param value  The value to add.
 *
 * @return The index of the new item.
 */
size_t ListClass::addFirst(RexxObject *value)
{
    return contents->insertAtBeginning(value, ListContents::AtEnd);
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
    return contents->insertAtEnd(value);
}


/**
 * The rexx version of the append method.
 *
 * @param value  The value to append.
 *
 * @return An object version of the index.
 */
RexxObject *ListClass::appendRexx(RexxInternalObject *value)
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
RexxInternalObject *ListClass::removeRexx(RexxObject *index)
{
    // out of bounds is not an issue here...
    ItemLink index = validateIndex(argIndex, ARG_ONE);
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
RexxInternalObject *ListClass::firstItemRexx()
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
    return contents->firstItem();
}


/**
 * Rexx access to the first item of the list.
 *
 * @return The first item, or .nil if the list is empty.
 */
RexxInternalObject *ListClass::lastItemRexx()
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
    return contents->lastItem();
}


/**
 * Get the index of the first item in the list.
 *
 * @return The index value, or the .nil if the list is empty.
 */
RexxInternalObject *ListClass::firstRexx()
{
    return isEmpty() ? TheNilObject : new_integer(firstIndex());
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
    return isEmpty() ? TheNilObject : new_integer(lastIndex());
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


RexxObject *ListClass::nextRexx(RexxObject *index)
{
    ItemLink argIndex = validateIndex(index, ARG_ONE);
    ItemLink next = nextIndex(argIndex);
    return next == NoMore ? TheNilObject : new_integer(next);
}


RexxObject *ListClass::previousRexx(RexxObject *index)
{
    ItemLink argIndex = validateIndex(index, ARG_ONE);
    ItemLink previous = previousIndex(argIndex);
    return previous == NoMore ? TheNilObject : new_integer(previous);
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
size_t ListClass::previousIndex(size_t _index)
{
    return contents->nextIndex(index);
}


RexxObject *ListClass::hasIndexRexx(RexxObject *index)
{
    ItemLink argIndex = validateIndex(index, ARG_ONE);
    return booleanObject(argIndex != NoLink);
}


RexxArray *ListClass::requestArray()
{
    return allItems();
}


RexxArray *ListClass::makeArray()
{
    return allItems();           // this is just all of the array items.
}


/**
 * Return an array containing all elements contained in the list,
 * in sorted order.
 *
 * @return An array with the list elements.
 */
RexxArray *ListClass::allItems()
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
}


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
RexxArray *ListClass::allIndexes()
{
    return contents->allIndexes()
}


/**
 * Return the index of the first item with a matching value
 * in the list.  Returns .nil if the object is not found.
 *
 * @param target The target object.
 *
 * @return The index of the item, or .nil.
 */
RexxObject *ListClass::indexRexx(RexxInternalObject *target)
{
    // we require the index to be there.
    requiredArgument(target, ARG_ONE);
    ItemLink itemIndex = getIndex(target);
    return itemIndex == NoMore ? TheNilObject : new_integer(itemIndex);
}


size_t ListClass::getIndex(RexxInternalObject *target)

/**
 * Tests whether there is an object with the given value in the
 * list.
 *
 * @param target The target value.
 *
 * @return .true if there is a match, .false otherwise.
 */
RexxObject *ListClass::hasItem(RexxObject *target)
{
    // we require the index to be there.
    requiredArgument(target, ARG_ONE);

    // ok, now run the list looking for the target item
    size_t nextEntry = this->first;
    for (size_t i = 1; i <= this->count; i++)
    {
        LISTENTRY *element = ENTRY_POINTER(nextEntry);
        // if we got a match, return the item
        if (target->equalValue(element->value))
        {
            return TheTrueObject;
        }
        nextEntry = element->next;
    }
    // no match
    return TheFalseObject;
}
{
    return contents->getindex(target);
}


/**
 * Removes an item from the collection.
 *
 * @param target The target value.
 *
 * @return The target item.
 */
RexxInternalObject *ListClass::removeItemRexx(RexxInternalObject *target)
{
    // we require the index to be there.
    requiredArgument(target, ARG_ONE);

    return resultOrNil(removeItem(target));
}


RexxInternalObject *ListClass::removeItem(RexxInternalObject *target)
{
    return contents->removeItem(target);
}


SupplierClass *ListClass::supplier()
{
    return contents->supplier();

    RexxArray *values;                   /* array of value items              */
    RexxArray *indices;                  /* array of index items              */

                                         /* and all of the indices            */
    indices = this->makeArrayIndices();
    values = this->makeArray();          /* get the list values               */
                                         /* return the supplier values        */
    return(SupplierClass *)new_supplier(values, indices);
}


RexxObject *ListClass::itemsRexx()
{
    return new_integer(items());
}


size_t ListClass::items()
{
    return contents->items();
}


RexxArray *ListClass::weakReferenceArray()
/******************************************************************************/
/* Function:  Scan the list removing all cleared weak reference objects,      */
/*            and return an array of the dereferenced week array objects.     */
/******************************************************************************/
{
    LISTENTRY *element;                  /* current working entry             */

    size_t i = this->firstIndex();              /* point to the first element        */
    size_t itemCount = this->count;
    while (itemCount--)                  /* step through the array elements   */
    {
        element = ENTRY_POINTER(i);      /* get the next item                 */
        // step to the next element now, so we can remove this one
        i = element->next;

        // get the reference value
        WeakReference *ref = (WeakReference *)element->value;
        // has the referenced object gone out of scope?
        if (ref->get() == OREF_NULL)
        {
            // remove this element from the list...note that this also
            // decrements the count, which will effect the number of
            // loop iterations.
            primitiveRemove(element);
        }
    }

    // we've removed the dead references, so make a second pass copying
    // the real values into the returned array
    RexxArray *array = (RexxArray *)new_array(this->count);
    i = this->firstIndex();              /* point to the first element        */
    for (size_t j = 1; j <= this->count; j++) /* step through the array elements   */
    {
        element = ENTRY_POINTER(i);      /* get the next item                 */
                                         /* copy over to the array            */
        // get the reference value
        WeakReference *ref = (WeakReference *)element->value;
        array->put(ref->get(), j);
        i = element->next;               /* get the next pointer              */
    }
    return array;                        /* return the array element          */
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


    // TODO: add intial size stuff here
    ListClass *newList = new ListClass;
    ProtectedObject p(newList);

    // handle Rexx class completion
    classThis->completeNewObject(newList, init_args, argCount);

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
ListClass *ListClass::classOf(RexxObject **args, size_t  argCount)
{
    // create a list item of the appopriate type.
    // TODO: work out how to specify the initial size
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

