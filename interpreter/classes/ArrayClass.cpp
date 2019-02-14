/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* Primitive Array Class                                                      */
/*                                                                            */
/*   This Array class functions in two ways.  One as an auto-extending array  */
/* and as a static array.  The static methods are used inside the interpreter */
/* since we always know the exact size of the array we want and will place    */
/* those elements into the array, so the static sized methods are optimized.  */
/*                                                                            */
/*   The auto-extending methods are for the OREXX level behaviour.  They      */
/* may also be used inside the base code if that behaviour is required.       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"
#include "MutableBufferClass.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *ArrayClass::classInstance = OREF_NULL;
ArrayClass *ArrayClass::nullArray = OREF_NULL;
const size_t ArrayClass::MinimumArraySize = 8;      // the minimum size we allocate.

/**
 * Create initial class object at bootstrap time.
 */
void ArrayClass::createInstance()
{
    CLASS_CREATE(Array);
    // the null array is useful for some internal purposes.
    // this should never be used in a situation where it might get
    // handed out out, since user code might add items to it.
    nullArray = new_array((size_t)0);
}

// CLASS methods of the Array class

/**
 * Create a new array from Rexx code.
 *
 * @param arguments The pointer to the arguments (size items)
 * @param argCount  The number of arguments.
 *
 * @return An allocated Array item of the target class.
 */
RexxObject *ArrayClass::newRexx(RexxObject **arguments, size_t argCount)
{
    // this method is defined as an instance method, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // creating an array of the default size?
    if (argCount == 0)
    {
        Protected<ArrayClass> temp = new (0, DefaultArraySize) ArrayClass;
        // finish setting this up.
        classThis->completeNewObject(temp);
        return temp;
    }

    // Special case for 1-dimensional.  This could be a single integer size or
    // an array of sizes to create a multi-dimension array.
    if (argCount == 1)
    {
        RexxObject *currentDim = arguments[0];
        // specified as an array of dimensions?
        // this gets special handling
        if (currentDim != OREF_NULL && isArray(currentDim))
        {
            return createMultidimensional((ArrayClass *)currentDim, classThis);
        }

        // Make sure it's an integer
        size_t totalSize = validateSize(currentDim, ARG_ONE);

        // allocate a new item
        Protected<ArrayClass> temp = new (totalSize) ArrayClass();
        // if we're creating an explicit 0-sized array, create
        // a dimension array so dimension cannot be changed later.
        if (totalSize == 0)
        {
            temp->dimensions = new (1) NumberArray(1);
        }

        // finish the class initialization and init calls.
        classThis->completeNewObject(temp);
        return temp;
    }
    else
    {
        // more than one argument, so all arguments must be valid size values.
        return createMultidimensional(arguments, argCount, classThis);
    }
}


/**
 * Rexx accessible version of the OF method.
 *
 * @param args     The pointer to the OF arguments.
 * @param argCount The count of arguments.
 *
 * @return A new array of the target class, populated with the argument objects.
 */
RexxObject *ArrayClass::ofRexx(RexxObject **args, size_t argCount)
{
    // this method is defined as an instance method, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // create, and fill in the array item.  Make sure we create one with the specific
    // size for the added items.  The completeNewObject call will turn this into
    // the correct class if this is a subclass getting created.  The constructor
    // fills in the array before we call init (cheating a bit, but I don't care!)
    Protected<ArrayClass> newArray = new (argCount) ArrayClass((RexxInternalObject **)args, argCount);

    // finish the class initialization and init calls.
    classThis->completeNewObject(newArray);
    return newArray;
}


/**
 * Validate an array size item.  This converts to binary
 * and checks limits.
 *
 * @param size   The argument size object.
 *
 * @return The size, converted to binary.
 */
size_t ArrayClass::validateSize(RexxObject *size, size_t position)
{
    // Make sure it's an integer
    size_t totalSize = nonNegativeArgument(size, position);

    if (totalSize >= MaxFixedArraySize)
    {
        reportException(Error_Incorrect_method_array_too_big, MaxFixedArraySize - 1);
    }
    return totalSize;
}


/**
 * Helper routine for creating a multidimensional array.
 *
 * @param dims   Pointer to an array of pointers to dimension objects.
 * @param count  the number of dimensions to create
 *
 * @return The created array
 */
ArrayClass *ArrayClass::createMultidimensional(RexxObject **dims, size_t count, RexxClass *classThis)
{
    // Working with a multi-dimension array, so get a dimension array
    Protected<NumberArray> dim_array = new (count) NumberArray(count);

    // we need to calculate total size needed for this array
    // since we multiply each additional size in, start with a zeroth size of 1
    size_t totalSize = 1;
    for (size_t i = 0; i < count; i++)
    {
        // each dimension must be there and must be a non-negative integer value
        // ...a dimension of 0 really does not make sense, right, but does work.
        // this creates a multi-dimensional array of zero size which will get resized
        // on the first assignment.  Doesn't really make sense, but it's perfectly legal.
        RexxObject *currentDim = dims[i];
        size_t currentSize = nonNegativeArgument(currentDim, i + 1);
        // going to do an overflow?  By dividing, we can detect a
        // wrap situation.
        if (currentSize != 0 && ((MaxFixedArraySize / currentSize) < totalSize))
        {
            reportException(Error_Incorrect_method_array_too_big, MaxFixedArraySize - 1);
        }
        // keep running total size and put integer object into current position
        totalSize *= currentSize;

        // add this to our dimensions array, using a new integer object so
        // we know what is there.
        dim_array->put(currentSize, i + 1);
    }

    // a final sanity check for out of bounds
    if (totalSize >= MaxFixedArraySize)
    {
        reportException(Error_Incorrect_method_array_too_big, MaxFixedArraySize - 1);
    }

    // create a new array item

    Protected<ArrayClass> temp = new (totalSize) ArrayClass;

    // set the dimension array
    temp->dimensions = dim_array;

    // finish the class initialization and init calls.
    classThis->completeNewObject(temp);
    return temp;
}


// End of Class methods for the Array class


/**
 * Allocate a new array item.
 *
 * @param size    The base size of the object.
 * @param items   The logical size of the array (that reported
 *                by size())
 * @param maxSize The maximum size we wish to allocate for this.
 *
 * @return Allocated object space.
 */
void *ArrayClass::operator new(size_t size, size_t items, size_t maxSize)
{
    // use the common allocation routine.
    return allocateNewObject(size, items, maxSize, T_Array);
}


/**
 * Common routine for use by Array and subclass operator methods.
 *
 * @param size    The base object size.
 * @param items   The number of items we wish to hold.
 * @param maxSize The maximum size to allocate.
 * @param type    The Rexx class type code for this object.
 *
 * @return A newly allocated object.
 */
ArrayClass *ArrayClass::allocateNewObject(size_t size, size_t items, size_t maxSize, size_t type)
{
    size_t bytes = size;
    // we never create below a minimum size
    maxSize = std::max(maxSize, MinimumArraySize);
    // and use at least the size as the max
    maxSize = std::max(maxSize, items);
    // add in the max size value.  Note that we subtract one since
    // the first item is contained in the base object allocation.
    bytes += sizeof(RexxInternalObject *) * (maxSize - 1);
    // now allocate the new object with that size.  We also give a hint to
    // the language process about how many objects we can potentially mark during
    // a garbage collection.
    ArrayClass *newArray = (ArrayClass *)new_object(bytes, type, maxSize);

    // now fill in the various control bits.  Ideally, this
    // really should be done in the constructor, but that gets really too
    // complicated.
    newArray->arraySize = items;
    newArray->maximumSize = maxSize;
    newArray->lastItem = 0;
    newArray->itemCount = 0;
    // no expansion yet, use ourself as the expansion
    newArray->expansionArray = newArray;
    // and return the new array.
    return newArray;
}


/**
 * An initializer for an OF method.
 *
 * @param objs   Pointer to an array ob objects references.
 *
 * @param count  The count of objects in the array.
 */
ArrayClass::ArrayClass(RexxInternalObject **objs, size_t count)
{
    itemCount = 0;
    lastItem = 0;

    // if we are creating an empty array, then we need to add
    // a dimension array so that this is properly dimensioned.
    if (count == 0)
    {
        dimensions = new (1) NumberArray(1);
    }

    // if we have array items, do a quick copy of the object references
    if (count != 0)
    {
        for (size_t i = 1; i <= count; i++)
        {
            if (objs[i - 1] != OREF_NULL)
            {
                // this ensures the item count and the last item are updated properly.
                setArrayItem(i, objs[i - 1]);
            }
        }
    }
}


/**
 * Copy the array (and the expansion array, if necessary)
 *
 * @return A copy of the array object.
 */
RexxInternalObject *ArrayClass::copy()
{
    // make a copy of of the main array
    ArrayClass *newArray = (ArrayClass *)RexxObject::copy();
    // have we expanded in the past?  Then we need to
    // copy the expansion array too.  NOTE:  It would be tempting
    // to just copy the expansion array and return that, but
    // the possibilty that the array might have object variables
    // (e.g., because of an OBJECTNAME= call) means we really can't do
    // that directly.  It is easier to just copy both pieces when needed.
    if (hasExpanded())
    {
        newArray->expansionArray = (ArrayClass *)expansionArray->copy();
    }
    else
    {
        // make sure the new array is updated to point to itself
        newArray->expansionArray = newArray;
    }
    return newArray;
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void ArrayClass::live(size_t liveMark)
{
    memory_mark(dimensions);
    memory_mark(objectVariables);
    memory_mark(expansionArray);

    // if we expand, we adjust the expansion size down so we don't overrun.
    // but we need to mark our space too, since we could be the expansion array.
    // NOTE that we could be the original array or the extension.  The extension
    // array doesn't track last item, so it needs to mark the entire active
    // section of the array because it won't know everything.
    memory_mark_array(arraySize, objects);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void ArrayClass::liveGeneral(MarkReason reason)
{
    memory_mark_general(dimensions);
    memory_mark_general(objectVariables);
    memory_mark_general(expansionArray);

    // if we expand, we adjust the expansion size down so we don't overrun.
    // but we need to mark our space too.
    memory_mark_general_array(arraySize, objects);
}


void ArrayClass::flatten(Envelope *envelope)
{
    setUpFlatten(ArrayClass)

    flattenRef(dimensions);
    flattenRef(objectVariables);
    flattenRef(expansionArray);

    flattenArrayRefs(arraySize, objects);

    cleanUpFlatten
}


/**
 * Check and raise an error for a operation that is not
 * permitted on a multi-dimensioal array.
 *
 * @param methodName The name of the method doing the checking.
 */
void ArrayClass::checkMultiDimensional(const char *methodName)
{
    if (isMultiDimensional())
    {
        reportException(Error_Incorrect_method_array_dimension, methodName);
    }

}


/**
 * Simple inline method for setting a value in the array.
 *
 * @param value    The value to set.
 * @param position The index position.
 */
inline void ArrayClass::setItem(size_t position, RexxInternalObject *value)
{
    setOtherField(expansionArray, objects[position - 1], value);
}


/**
 * Simple inline method for clearing an array position
 *
 * @param value    The value to set.
 * @param position The index position.
 */
inline void ArrayClass::clearItem(size_t position)
{
    setOtherField(expansionArray, objects[position - 1], OREF_NULL);
}


/**
 * Inline method for setting an object into the array and
 * updating the item count and last item positions, if
 * necessary.
 *
 * @param value    The value to set.
 * @param position The index position.
 */
void ArrayClass::setOrClearArrayItem(size_t position, RexxInternalObject *value)
{
    // some operations will set a value to OREF_NULL, so we need to
    // distinguish between clearing the item or setting it to a value
    // so that the internal counters stay in sync.
    if (value == OREF_NULL)
    {
        clearArrayItem(position);
    }
    else
    {
        setArrayItem(position, value);
    }
}


/**
 * Inline method for copying an array item from one array to
 * another.  This only updates if we are getting a non-null
 * value.  This also updates the itemCount and lastItem values
 * on the set.
 *
 * @param value    The value to set.
 * @param position The index position.
 */
void ArrayClass::copyArrayItem(size_t position, RexxInternalObject *value)
{
    // only perform the set if this is non-null to avoid changing the count and last
    // positions for a null copy.
    if (value != OREF_NULL)
    {
        setArrayItem(position, value);
    }
}


/**
 * Inline method for setting an object into the array and
 * updating the item count and last item positions, if
 * necessary.
 *
 * @param value    The value to set.
 * @param position The index position.
 */
void ArrayClass::setArrayItem(size_t position, RexxInternalObject *value)
{
    // if not overwriting an existing item, update the item count
    checkSetItemCount(position);
    setItem(position, value);
    // check the last set element
    checkLastItem(position);
}


/**
 * Simple inline method for clearing an array position, while
 * maintaining the count and last item settings.
 *
 * @param value    The value to set.
 * @param position The index position.
 */
inline void ArrayClass::clearArrayItem(size_t position)
{
    // see if clearing this field alters the item count
    checkClearItemCount(position);
    clearItem(position);
    checkClearLastItem(position);
}


/**
 * We've cleared the current past item position...we need
 * to search for the new last item position.
 */
void ArrayClass::updateLastItem()
{
    // we know that the current last item is now too high,
    // so scan backwards from there until we find an occupied location.

    for (; lastItem > 0; lastItem--)
    {
        if (isOccupied(lastItem))
        {
            return;
        }
    }

    // if we fall through, lastItem is 0, which is correct for an empty
    // array.
}


/**
 * Place an object into the array.  Note:  This version does not
 * do any bounds checking.  The caller is responsible for
 * ensuring there is sufficient space.
 *
 * @param value  The object to add.
 * @param pos    The index of the added item (origin 1)
 */
void ArrayClass::put(RexxInternalObject *value, size_t pos)
{
    // make sure we don't overwrite
    ensureSpace(pos);
    // set the item...this also updates the count
    // and the last item, if needed.
    setArrayItem(pos, value);
}


/**
 * The Rexx stub for the Array PUT method.  This does full
 * checking for the array.
 *
 * @param arguments The array of all arguments sent to the
 *                  method (variable arguments allowed
 *                  here...the first argument is the item being
 *                  added, all other arguments are the index).
 * @param argCount  The number of arguments in the method call.
 *
 * @return Always return nothing.
 */
RexxObject *ArrayClass::putRexx(RexxObject **arguments, size_t argCount)
{
    if (argCount < 2)
    {
        reportException(Error_Incorrect_method_minarg, IntegerTwo);
    }

    // The first argument is a required value
    RexxObject *value = arguments[0];
    requiredArgument(value, ARG_ONE);

    // all the rest of the arguments are the index...go validate it, and expand if necessary
    size_t position;
    validateIndex(arguments + 1, argCount - 1, ARG_TWO, IndexUpdate, position);

    // set the new value and return nothing
    put(value, position);
    return OREF_NULL;
}


/**
 * Fill all locations of the array with the given object
 *
 * @param value  The object that will fill the array.
 *
 * @return No return value.
 */
RexxObject *ArrayClass::fillRexx(RexxObject *value)
{
    requiredArgument(value, ARG_ONE);

    // perform the fill
    fill(value);

    return this;          // return receiving Array
}


/**
 * Fill all locations of the array with the given object
 *
 * @param value  The object used to fill the array.
 */
void ArrayClass::fill(RexxInternalObject *value)
{
    for (size_t i = 1; i <= size(); i++)
    {
        // set the item using the fast version...we
        // can just set the itemCount and lastItem afterwards
        setItem(i, value);
    }

    // we are full...item count and last item are both maxed out.
    itemCount = size();
    lastItem = size();
}


/**
 * Empty all of the items from an array.
 *
 * @return No return value.
 */
RexxObject *ArrayClass::empty()
{
    // if not working with an oldspace object (VERY likely), we can just use memset to clear
    // everything.
    if (isNewSpace())
    {
        memset(data(), '\0', sizeof(RexxObject *) * size());
    }
    else
    {
        // sigh, we have to use OrefSet
        for (size_t i = 1; i <= size(); i++)
        {
            // use the simple form that doesn't track items and
            // last item
            clearItem(i);
        }
    }
    // no element set yet
    lastItem = 0;
    // no items either
    itemCount = 0;
    return this;          // return receiving Array
}


/**
 * Test if an array is empty.
 *
 * @return True if the array is empty, false otherwise
 */
bool ArrayClass::isEmpty()
{
    return items() == 0;
}


/**
 * Test if an array is empty.
 *
 * @return True if the array is empty, false otherwise
 */
RexxObject *ArrayClass::isEmptyRexx()
{
    return booleanObject(isEmpty());
}


/**
 * Append an item after the last item in the array.
 *
 * @param value  The value to append.
 *
 * @return The index of the appended item.
 */
RexxObject *ArrayClass::appendRexx(RexxObject *value)
{
    requiredArgument(value, ARG_ONE);
    checkMultiDimensional("APPEND");

    // let the low-level version handle the expansion.
    return new_integer(append(value));
}


/**
 * Low-level append of an object to the array.
 *
 * @param value  The value to append.
 *
 * @return The index of the appended object.
 */
size_t ArrayClass::append(RexxInternalObject *value)
{
    size_t newIndex = lastItem + 1;
    // the put method will expand the array, if necessary
    put(value, newIndex);
    return newIndex;
}


/**
 * Insert an element into the given index location.
 *
 * @param value  The value to insert.  This can be omitted,
 *               which will insert an empty slot at the
 *               indicated position.
 * @param index  The target index.  This can be .nil, which will insert
 *               at the beginning, omitted, which will insert at the end,
 *               or a single-dimensional index of the position where the
 *               value will be inserted after.
 *
 * @return The index of the inserted valued.
 */
RexxObject *ArrayClass::insertRexx(RexxObject *value, RexxObject *index)
{
    checkMultiDimensional("INSERT");

    size_t position;                     // array position

    if (index == TheNilObject)
    {
        position = 1;                   // the insertion point of the item is 1
    }
    else if (index == OREF_NULL)
    {
        position = lastItem + 1;        // inserting after the last item
    }
    else
    {
        // validate the index and expand if necessary.
        validateIndex(index, ARG_TWO, IndexUpdate, position);
        // check that the position is good for this collection type.
        // For arrays, all valid indexes are good.  For Queues,
        // only indexes within the current occupied range are good.
        checkInsertIndex(position);
        // we insert AFTER the given index, so bump this
        position = position + 1;
    }

    // do the actual insertion
    return new_integer(insert(value, position));
}


/**
 * Insert an element into the array, shifting all of the
 * existing elements at the inserted position and adjusting
 * the size accordingly.
 *
 * @param value  The value to insert (can be OREF_NULL to just open a new slot)
 * @param _index The insertion index position. NOTE:  Unlike the
 *               Rexx version, the index is the position where
 *               this value will be inserted, not the index of
 *               where it is inserted after.
 *
 * @return The index of the inserted item.
 */
size_t ArrayClass::insert(RexxInternalObject *value, size_t index)
{
    // open an appropriate sized gap in the array.  Note that any adjustments
    // to the lastItem are done in the open gap routine.  If we're inserting
    // at the end, then the put() with the value will update lastItem
    openGap(index, 1);                // open an appropriate sized gap in the array


    // only do the put() operation if we have a real value to put so that
    // the item count doesn't get updated incorrectly.
    if (value != OREF_NULL)
    {
        put(value, index);
    }
    return index;                     // return the index of the insertion
}


/**
 * Delete an element from the given index location, shifting the
 * item indexes as required.
 *
 * @param index  The target index.  This can be a single number or an array
 *               containing a single number.
 *
 * @return The object that has been deleted.  Returns .nil if the
 *         target index does not exist.
 */
RexxInternalObject *ArrayClass::deleteRexx(RexxObject *index)
{
    requiredArgument(index, ARG_ONE);
    checkMultiDimensional("DELETE");

    size_t position;                     // array position

    // validate the index and expand if necessary.
    validateIndex(index, ARG_ONE, IndexAccess, position);

    // check that the position is good for this collection type.
    // For arrays, all valid indexes are good.  For Queues,
    // only indexes within the current occupied range are good.
    checkInsertIndex(position);

    // do the actual insertion
    return resultOrNil(deleteItem(position));
}


/**
 * Open a gap in the array by shifting each element to the right
 * starting at the given index.
 *
 * @param index The index of the first item to shift.
 *
 * @param elements The number of elements to shift.
 */
void ArrayClass::openGap(size_t index, size_t elements)
{
    // is this larger than our current last element?  If so, we have nothing to move
    // but do need to expand the array size to accommodate the additional members
    if (index > lastItem)
    {
        ensureSpace(index + elements - 1);
    }
    else
    {
        // make sure we have space for the additional elements
        ensureSpace(lastItem + elements);

        // the last element to move.
        char *_end = (char *)slotAddress(lastItem + 1);

        // get the start and end of the gap
        char *_start = (char *)slotAddress(index);
        char *_target = (char *)slotAddress(index + elements);
        // shift the array section over to create a gap
        memmove(_target, _start, _end - _start);

        // now null out all of the slots in the gap, using an
        // explicit assignment rather than put to avoid old-to-new
        // tracking issues
        for (size_t i = index; i <= index + elements - 1; i++)
        {
            zeroItem(i);
        }
        // we only adjust the last item position if there is a current last item.
        if (lastItem != 0)
        {
            lastItem += elements;
        }
    }
}


/**
 * Close a gap in the array item.
 *
 * @param index    The gap to close.
 * @param elements The size of the gap to close.
 */
void ArrayClass::closeGap(size_t index, size_t elements)
{
    // if we're beyond the current last item, nothing to do here
    if (index > lastItem)
    {
        // if the index is within the size bounds of the
        // array, we need to adjust the size
        if (index <= size())
        {
            shrink(elements);
        }
        return;
    }

    // cap the number of elements we're shifting.
    elements = std::min(elements, lastItem - index + 1);

    // explicitly null out the slots of the gap we're closing to
    // ensure that any oldspace tracking issues are resolved.
    // use the clear method to ensure the item count is getting updated
    // appropriately.
    for (size_t i = index; i < index + elements; i++)
    {
        clearArrayItem(i);
    }

    // we could have cleared out the last item when
    // we cleared the gap, thus removing the need to
    // shift anything
    if (lastItem < index)
    {
        // if the index is within the size bounds of the
        // array, we need to adjust the size
        if (index <= size())
        {
            shrink(elements);
        }
        return;
    }

    // get the address of first element and the first item to move.
    char *_target = (char *)slotAddress(index);
    char *_start =  (char *)slotAddress(index + elements);
    // and the end location of the real data
    char *_end = (char *)slotAddress(lastItem + 1);
    // shift the array over
    memmove(_target, _start, _end - _start);
    // because we track the item count, we need to ensure that
    // the positions that we just copied out of are cleared out, otherwise
    // the item count will not get incremented when these slots get reused.
    _start = (char *)slotAddress(lastItem + 1 - elements);
    memset(_start, 0,  _end - _start);
    // adjust the last element position (NOTE:  because we needed to shift,
    // we know that lastItem is non-zero)
    lastItem -= elements;
    // reduce the size
    shrink(elements);
}


/**
 * Shrink an array without reallocating any elements
 * Single Dimension ONLY
 *
 * @param amount The number of elements to shrink by.
 */
void ArrayClass::shrink(size_t amount)
{
    size_t newSize = size() - amount;
    // array size is different in the main array item and the
    // expansion array.  The current total array size is always
    // the expansion array version.  This just shrinks the logical
    // size of the array.
    expansionArray->arraySize = newSize;
}


/**
 * The Rexx stub for the Array GET method.  This does full
 * checking for the array.
 *
 * @param arguments The array of all arguments sent to the method (variable arguments allowed here.)
 * @param argCount  The number of arguments in the method call.
 *
 * @return Value at the provided index or .nil if the item does
 *         not exist.
 */
RexxInternalObject *ArrayClass::getRexx(RexxObject **arguments, size_t argCount)
{
    size_t position;
    // validate the index
    if (!validateIndex(arguments, argCount, ARG_ONE, IndexAccess, position))
    {
        // return .nil for anything out of bounds
        return TheNilObject;
    }
    else
    {
        // return the object at that position, or .nil if it is empty.
        return resultOrNil(get(position));
    }
}


/**
 * Get an item from the array, with array size sensitivity.
 *
 * @param position The target position.
 *
 * @return The object at the array position.  Returns OREF_NULL if there
 *         is not item at that position.
 */
RexxInternalObject *ArrayClass::safeGet(size_t position)
{
    // do the bounds check               */
    if (position > size())
    {
        return OREF_NULL;
    }
    return get(position);
}


/**
 * Remove an object from the array.
 *
 * @param index  The target index position.
 *
 * @return The removed object, if any.  Returns OREF_NULL if there
 *         is no item at that index.
 */
RexxInternalObject *ArrayClass::remove(size_t index)
{
    // if this index is out of the allowed range or the
    // slot is not occupied, just return OREF_NULL;
    if (!isInbounds(index) || !isOccupied(index))
    {
        return OREF_NULL;
    }

    // get the removed item
    RexxInternalObject *result = get(index);
    // clear the slot, updating the item count and lastItem as appropriate.
    clearArrayItem(index);
    return result;
}


/**
 * Remove an item from the array and return the item.  .nil
 * is returned if the item does not exist
 *
 * @param arguments Pointer to the index arguments.
 * @param argCount  The count of index arguments.
 *
 * @return The removed object, if any.  Returns .nil if there
 *         is no item at the index position.
 */
RexxInternalObject *ArrayClass::removeRexx(RexxObject **arguments, size_t argCount)
{
    size_t position;
    // if this position doesn't validate, just return .nil
    if (!validateIndex(arguments, argCount, ARG_ONE, IndexAccess, position))
    {
        return TheNilObject;
    }

    // do the remove and return the value.
    return resultOrNil(remove(position));
}

/**
 * Rexx exported version of the items() method.
 *
 * @return An integer object holding the count.
 */
RexxObject  *ArrayClass::itemsRexx()
{
    return new_integer(items());
}


/**
 * Return an array of the dimensions of this array.
 *
 * @return An array item with one size item for each dimension of the
 *         array.
 */
ArrayClass *ArrayClass::getDimensionsRexx()
{
    // if it is a single dimension array, return an array with the size
    // as a single item
    if (isSingleDimensional())
    {
        return new_array(new_integer(size()));
    }
    else
    {
        // return a copy of the dimensions array
        return dimensions->toArray();
    }
}


/**
 * Request a specific dimension from an array object.
 *
 * @param target The target dimension.
 *
 * @return The size of the dimension...returns 0 for any unknown ones.
 */
RexxObject *ArrayClass::dimensionRexx(RexxObject *target)
{
    // if no target specified, then we return the number of dimensions.
    // it is possible that the number of dimensions have not been determined yet,
    // in which case, we return 0.
    if (target == OREF_NULL)
    {
        // if no dimensions specified and the size is still zero, then this
        // is still unknown.
        if (dimensions == OREF_NULL)
        {
            if (size() == 0)
            {
                return IntegerZero;
            }
            else
            {
                return IntegerOne;
            }
        }
        else
        {
            // the number of dimensions is determined by the size of the dimensions array.
            return new_integer(dimensions->size());
        }
    }
    else
    {
        // specific dimension request.
        size_t position = target->requiredPositive(ARG_ONE);
        // asking for dimension of single?
        if (isSingleDimensional())
        {
            // only return something if the requested position was 0
            if (position == 1)
            {

                return new_integer(size());
            }
            else
            {
                return IntegerZero;
            }
        }
        // out of range for the sizes we do have?  That's a zero also
        else if (position > dimensions->size())
        {
            return IntegerZero;
        }
        // return the specific dimension value
        else
        {
            return new_integer(dimensions->get(position));
        }
    }
}


/**
 * Create a supplier for this array.
 *
 * @return an appropriate supplier.
 */
SupplierClass *ArrayClass::supplier()
{
    size_t slotCount = size();            // get the array size
    size_t itemCount = items();           // and the actual count in the array

    // get arrays for both the values and the indexes
    Protected<ArrayClass> values = new_array(itemCount);
    Protected<ArrayClass> indexes = new_array(itemCount);

    size_t count = 1;
    for (size_t i = 1; i <= slotCount; i++)
    {
        RexxInternalObject *item = get(i);
        // if we have an item in this slot, copy the item into the array and
        // generate an index
        if (item != OREF_NULL)
        {
            values->put(item, count);
            indexes->put(convertIndex(i), count);
            count++;
        }
    }

    return new_supplier(values, indexes);
}


/**
 * Process and validate a potentially multi-dimensional array
 * index.  If the index is out of bounds in any dimension it will
 * either return false or raise an error, depending on the bounds
 * checking parameter.
 *
 * @param index      The array of objects representing the index.
 * @param indexCount The count of index items.
 * @param argPosition  The argument position (used for error
 *               reporting)
 * @param boundsError Flags indicating how bounds errors should
 *                   be handled.
 * @param position   The returned flattened index pointing to the actual
 *                   item in the array.
 *
 * @return true if this was a valid index (within bounds) based
 *         on the bounds checking flags.
 */
bool ArrayClass::validateIndex(RexxObject **index, size_t indexCount,
    size_t argPosition, size_t boundsError, size_t &position)
{
    // one possibility is a single array used as a single argument specifying the
    // index position.  If this is the case, dummy up the argument list to point
    // to the array's information.

    // do we really have a single index item given as an array?
    if (indexCount == 1 && index[0] != OREF_NULL && isArray(index[0]))
    {
        // we process this exactly the same way, but swap the count and
        // pointers around to be the array data.
        ArrayClass *indirect = (ArrayClass *)index[0];
        indexCount = indirect->items();
        index = (RexxObject **)indirect->data();
    }

    /* Is this array one-dimensional?    */
    if (isSingleDimensional())
    {
        return validateSingleDimensionIndex(index, indexCount, argPosition, boundsError, position);
    }
    else
    {
        return validateMultiDimensionIndex(index, indexCount, argPosition, boundsError, position);
    }
}


/**
 * Process and validate an index for a single dimension array
 * (The most common index).  This might end up reconfiguring
 * this array to a multi-dimension array if it is still zero
 * sized.
 *
 * @param index      The array of objects representing the index.
 * @param indexCount The count of index items.
 * @param argPosition The argument position (used for error
 *                   reporting)
 * @param boundsError
 *                   Flags indicating how bounds errors should be handled.
 * @param position   The returned flattened index pointing to the actual
 *                   item in the array.
 *
 * @return true if this was a valid index (within bounds) based
 *         on the bounds checking flags.
 */
bool ArrayClass::validateSingleDimensionIndex(RexxObject **index, size_t indexCount,
    size_t argPosition, size_t boundsError, size_t &position)
{
    // we have a single dimension array and one argument...we are very happy!
    if (indexCount == 1)
    {
        position = index[0]->requiredPositive(argPosition);
        // ok, this is out of bounds.  Figure out how to handle this
        if (!isInbounds(position))
        {
            // could be WAAAAAAY out of bounds.
            if ((boundsError & RaiseBoundsInvalid) && position >= MaxFixedArraySize)
            {
                reportException(Error_Incorrect_method_array_too_big, MaxFixedArraySize - 1);
            }
            // if we're doing a put operation, we need to extend the upper bounds
            // to include
            if (boundsError & ExtendUpper)
            {
                // yes, expand to at least that size.
                extend(position);
                return true;
            }
            // not asked to extend or raise an error, but indicate this is no good.
            else
            {
                return false;
            }
        }
        // good index...we're done.
        return true;
    }
    // a multi-dimension array for a single dimension item.  We might need to
    // reconfigure this into a multidimension array
    if (indexCount > 1)
    {
        // should the array be extended?  This can only extended if
        // the size is still 0 and this was not set explicitly (indicated by
        // having an dimensions array resulting from calling .array~new(0)

        // TODO: pretty sure there are no unit tests for this extension behaviour
        if (boundsError & ExtendUpper)
        {
            // both of these are error conditions
            if (isFixedDimension())
            {
                reportException(Error_Incorrect_method_maxsub, IntegerOne);
            }
            else
            {
                // ok, we can extend this into a multi-dimension item
                extendMulti(index, indexCount, argPosition);
                // we have successfully turned into a multi-dimension array, so
                // loop back around and try to validate the index.
                return validateMultiDimensionIndex(index, indexCount, argPosition , boundsError, position);
            }
        }
        // asked to raise an error if we have too many?  Need to sort out
        // which error to issue.
        else if (boundsError & RaiseBoundsTooMany)
        {
            // already have a fixed dimension?
            if (isFixedDimension())
            {
                // too many subscripts
                reportException(Error_Incorrect_method_maxsub, IntegerOne);
            }
            // We are not yet fixed in dimension, so we could be expanded later.
            else
            {
                return false;
            }
        }
        // asked to fail this silently
        else
        {
            return false;
        }
    }
    // zero subscripts is always an error
    else
    {
        reportException(Error_Incorrect_method_minarg, argPosition);
    }
    return false;
}


/**
 * Process and validate an index for a multi dimension array.
 *
 * @param index      The array of objects representing the index.
 * @param indexCount The count of index items.
 * @param argPosition The argument position (used for error
 *                   reporting)
 * @param boundsError
 *                   Flags indicating how bounds errors should be handled.
 * @param position   The returned flattened index pointing to the actual
 *                   item in the array.
 *
 * @return true if this was a valid index (within bounds) based
 *         on the bounds checking flags.
 */
bool ArrayClass::validateMultiDimensionIndex(RexxObject **index, size_t indexCount,
    size_t argPosition, size_t boundsError, size_t &position)
{
    size_t numSubscripts = indexCount;
    size_t numDimensions = dimensions->size();

    // do we have the right number of subscripts for this array?
    // this is worth processing.
    if (numSubscripts == numDimensions)
    {
        // a multiplier for translating into an absolute index position
        size_t multiplier = 1;
        size_t offset = 0;

        // now loop through all of the index values
        for (size_t i = numSubscripts; i > 0; i--)
        {
            // each subscript is required.
            RexxObject *value = index[i - 1];
            // this must be a positive whole number and is required.
            position = positionArgument(value, argPosition + i);

            // get the current dimension
            size_t dimension = dimensions->get(i);
            // is this position larger than the current dimension?  Check how
            // the out of bounds situation should be handled.
            if (position > dimension)
            {
                // allowed to extend...we resize this array to at least match the
                // upper bounds on all dimensions here.
                if (boundsError & ExtendUpper)
                {
                    // go extend this
                    extendMulti(index, indexCount, argPosition);
                    // Ok, now try validating again now that the array has expanded.
                    // Note that because all subscripts are handled here, we should not
                    // need to extend a second time.
                    return validateMultiDimensionIndex(index, indexCount, argPosition, boundsError, position);
                }
                // probably a get request, so just say this is out of bounds
                else
                {
                    return false;
                }
            }

            // calculate the next offset position
            offset += multiplier * (position - 1);
            // and get the total dimension base size.
            multiplier *= dimension;
        }
        // get the accumulated position value...we now have a good value within the bounds
        position = offset + 1;
        return true;
    }
    // we have a mismatch between the index and the number of dimensions...issue the appropriate error
    else if (numSubscripts < numDimensions)
    {
        reportException(Error_Incorrect_method_minsub, numDimensions);
    }
    // must be too many subscripts
    else
    {
        reportException(Error_Incorrect_method_maxsub, numDimensions);
    }
    return false;
}


/**
 * Return the array size as an integer object
 *
 * @return An integer object containing the array size.
 */
RexxInteger *ArrayClass::sizeRexx()
{
    return new_integer(size());
}


/**
 * Section an array to the given size.
 *
 * @param _start The starting point of the section.
 * @param _end   The end section index
 *
 * @return A new array representing the given section.
 */
ArrayClass *ArrayClass::section(size_t start, size_t end)
{
    // 0 means the starting position was omitted, so start
    // from the beginning
    if (start == 0)
    {
        start = 1;
    }

    // ending position omitted or, End pos past end of array?
    // either way, this goes to the end of the array
    if (end == 0 || end > size())
    {
        end = size();
    }

    // if the start is before the end, then we have something to section
    if (start <= end)
    {
        // get a new array of the required size
        size_t newSize = end + 1 - start;
        ArrayClass *newArray = (ArrayClass *)new_array(newSize);
        // while we could just copy the elements in one shot,
        // we need to process each element so that the item count and
        // last item fields are properly set up.
        for (size_t i = 1; i <= newSize; i++, start++)
        {
            RexxInternalObject *obj = get(start);
            if (obj != OREF_NULL)
            {
                newArray->put(obj, i);
            }
        }
        return newArray;
    }
    else
    {
        return new_array((size_t)0);
    }
}


/**
 * Extract a section of the array as another array
 *
 * @param _start The starting index position
 * @param _end   The number of items to section.
 *
 * @return A new array containing the section elements.
 */
ArrayClass *ArrayClass::sectionRexx(RexxObject *start, RexxObject *end)
{
    // not allowed for a multi dimensional array
    checkMultiDimensional("SECTION");

    // the index is required
    requiredArgument(start, ARG_ONE);

    size_t nstart;
    // validate the index
    validateIndex(start, ARG_ONE, IndexAccess, nstart);

    // the ending position is a length value and defaults to the size of the array
    size_t nend = optionalLengthArgument(end, size(), ARG_TWO);

    if (nstart > size())
    {
        nend = 0;
    }
    else
    {
        // now cap the length at the remaining size of the array
        nend = std::min(nend, size() - nstart + 1);
    }

    // get an array of the appropriate subclass
    ArrayClass *newArray = allocateArrayOfClass(nend);
    // copy all of the elements
    for (size_t i = 1; i <= nend; i++, nstart++)
    {
        RexxInternalObject *obj = get(nstart);
        if (obj != OREF_NULL)
        {
            newArray->put(obj, i);
        }
    }

    return newArray;
}


/**
 * Allocate an array of the same class as the target.  Used
 * for the section operation.
 *
 * @param size   The size of the array.
 *
 * @return An array object allocated from the same class as the
 *         receiver.
 */
ArrayClass *ArrayClass::allocateArrayOfClass(size_t size)
{
    // typical is a just a primitive array.
    if (isArray(this))
    {
        return new_array(size);
    }

    // need to do this by sending a message to the class object.
    ProtectedObject result;
    classObject()->sendMessage(GlobalNames::NEW, new_integer(size), result);
    return (ArrayClass *)(RexxObject *)result;
}


/**
 * Retrieve the first element index from the array as an integer
 * object
 *
 * @return The index of the first item.
 */
RexxObject *ArrayClass::firstRexx()
{
    // locate the first item
    size_t index = firstIndex();

    // if no element found, this is the nil object.  Otherwise, convert
    // the index into external form (we could be multidimensional)
    // this is all handled by convertIndex()
    return convertIndex(index);
}


/**
 * Retrieve the index of the first real element
 *
 * @return The index of the first item.
 */
size_t ArrayClass::firstIndex()
{
    // locate the next index starting from the 0th position (this will
    // start the search at index 1).
    return nextIndex(0);
}


/**
 * Return the index of the last array item as an integer object
 *
 * @return The index position, as an object.
 */
RexxObject *ArrayClass::lastRexx()
{
    return convertIndex(lastItem);
}


/**
 * Return the index of the last item in the array.  Returns
 * 0 if the array is empty.
 *
 * @return The index of the last item.
 */
size_t ArrayClass::lastIndex()
{
    return lastItem;
}


/**
 * Return the first item in the array, or .nil if the
 * array is empty.
 *
 * @return The first item in the array
 */
RexxInternalObject *ArrayClass::getFirstItem()
{
    // find the index of the first item
    size_t index = firstIndex();
    return index == 0 ? TheNilObject : get(index);
}


/**
 * Return the last item in the array.
 *
 * @return The last item, or .nil if the array is empty.
 */
RexxInternalObject *ArrayClass::getLastItem()
{
    return lastItem == 0 ? TheNilObject : get(lastItem);
}

/**
 * Find the next index position with an item.
 *
 * @param index  The index position to search from.
 *
 * @return The index position of the next item, or 0 if there is no
 *         next item.
 */
size_t ArrayClass::nextIndex(size_t index)
{
    // scan from the next slot up to the last item position.  Note that
    // we allow a starting value of 0...useful for locating the first index!
    for (size_t nextIndex = index + 1; nextIndex <= lastItem; nextIndex++)
    {
        if (isOccupied(nextIndex))
        {
            return nextIndex;
        }
    }

    // nothing found
    return 0;
}


/**
 * Return the next entry after a given array index
 *
 * @param arguments The pointer to the index arguments.
 * @param argCount  The count of index arguments.
 *
 * @return The index position of the next item, as an object.
 */
RexxObject *ArrayClass::nextRexx(RexxObject **arguments, size_t argCount)
{
    size_t position;
    // go validate the index
    if (!validateIndex(arguments, argCount, ARG_ONE, IndexAccess, position))
    {
        // out of bounds results in the .nil object
        return TheNilObject;
    }

    // go locate the next item
    return convertIndex(nextIndex(position));
}


/**
 * Return the index preceeding a given index
 *
 * @param index  The starting index position
 *
 * @return The previous index position with an item.  Returns 0 if
 *         none was found.
 */
size_t ArrayClass::previousIndex(size_t index)
{
    // no need to scan if past the end position...we know the answer.
    if (index > lastItem)
    {
        return lastItem;
    }

    // scan backwards from the starting index until we found an occupied slot.
    for (size_t previous = index - 1; previous > 0; previous--)
    {
        if (isOccupied(previous))
        {
            return previous;
        }
    }
    // nothing found
    return 0;
}


/**
 * Return the index preceeding a given index
 *
 * @param arguments The index arguments (might be multi dimensional)
 * @param argCount  The count of index elements.
 *
 * @return The index position, as an object.
 */
RexxObject *ArrayClass::previousRexx(RexxObject **arguments, size_t argCount)
{
    size_t position;
    // go validate the index...just ignore if out of bounds
    validateIndex(arguments, argCount, ARG_ONE, IndexAccess, position);

    // go locate the next item
    return convertIndex(previousIndex(position));
}


/**
 * True if array has an entry for the index, false otherwise
 *
 * Note:  This routine should not raise an error, regardless of the indices
 * being used.  The only error produced is if no parms were passed.
 *
 * @param index      Arguments for a potential multi-dimensional array index.
 * @param indexCount The count of indexes.
 *
 * @return .true if there is an item at that position, .false otherwise.
 */
RexxObject *ArrayClass::hasIndexRexx(RexxObject **index, size_t indexCount)
{
    size_t position;
    // validate the index position.  Out of bounds is always false.
    if (!validateIndex(index, indexCount, ARG_ONE, IndexAccess, position))
    {
        return TheFalseObject;
    }
    // now check the slot position
    return booleanObject(hasIndex(position));
}


/**
 * Return a single dimension array of self, with only items that
 * has values, so it will be of size items.
 *
 * @return A single-dimension array containing all of the items in the array (non-sparse).
 */
ArrayClass *ArrayClass::makeArray()
{
    // for an array, this is the all items value.
    return allItems();
}


/**
 * Return an array of all real items contained in the collection.
 *
 * @return An array with all of the array items (non-sparse).
 */
ArrayClass *ArrayClass::allItems()
{
    // get a result array of the appropriate size
    ArrayClass *newArray = (ArrayClass *)new_array(items());

    // now fill in that array
    size_t count = 1;
    for (size_t index = 1; index <= lastItem; index++)
    {
        if (isOccupied(index))
        {
            newArray->put(get(index), count++);
        }
    }

    return newArray;
}


/**
 * Return an array of all indices of real array items contained
 * in the collection.
 *
 * @return An array with all of the array indices (non-sparse).
 */
ArrayClass *ArrayClass::allIndexes()
{
    // get a result array of the appropriate size
    Protected<ArrayClass> newArray = (ArrayClass *)new_array(items());

    // loop through the array, copying all of the items.
    for (size_t iterator = 1; iterator <= lastItem; iterator++ )
    {
        // if this is a real array item, add an index item to the
        // result collection.
        if (isOccupied(iterator))
        {
            newArray->append(convertIndex(iterator));
        }
    }
    return newArray;
}


/**
 * Handle a REQUEST('STRING') request for a REXX string object
 *
 * @return A string value for the array.
 */
RexxString  *ArrayClass::primitiveMakeString()
{
    // forward to the real makestring method
    return toString(OREF_NULL, OREF_NULL);
}


/**
 * Handle a REQUEST('STRING') request for a REXX string object
 *
 * @return A string value for the array.
 */
RexxString  *ArrayClass::makeString()
{
    // forward to the real makestring method
    return toString(OREF_NULL, OREF_NULL);
}


/**
 * Convert an array into a string item
 *
 * @param format    The optional format ('C' or 'L')
 * @param separator The separator between elements for the 'L' format.
 *
 * @return A single string value for the array.
 */
RexxString *ArrayClass::toString(RexxString *format, RexxString *separator)
{
    // we build up in a mutable buffer
    Protected<MutableBuffer> mutbuffer = new MutableBuffer();
    // convert into a non-sparse single dimension array item.
    Protected<ArrayClass> newArray = makeArray();

    // get the count of items
    size_t itemCount = newArray->items();

    // get the option argument and apply the default
    char form = optionalOptionArgument(format, 'L', ARG_ONE);

    // must be one of L or C.
    if (form != 'L' && form != 'C')
    {
        reportException(Error_Incorrect_method_option, "CL", format);
    }

    // character oriented process.  The separator string is not allowed with that form
    if (form == 'C')
    {
        if (separator != OREF_NULL)
        {
            reportException(Error_Incorrect_method_maxarg, IntegerOne);

        }

        // loop through the array
        for (size_t i = 1; i <=itemCount ; i++)
        {
            RexxInternalObject *item = newArray->get(i);
            // if we have a real item (which we should since we used makearray() at the
            // beginning
            if (item != OREF_NULL)
            {
                // NOTE:  We use stringValue here rather than requestString().  You can
                // get into some nasty recursion problems with with trying to do a full
                // string resolution here.  Items like arrays stored in arrays will display
                // as "An Array".
                RexxString *value = item->stringValue();
                mutbuffer->append(value);
            }
        }
    }
    // LINE oriented processing
    else
    {
        Protected<RexxString> lineEndString;
        if (separator != OREF_NULL)
        {
            lineEndString = stringArgument(separator, ARG_TWO);
        }
        else
        {
            lineEndString = new_string(line_end);
        }

        bool first = true;

        for (size_t i = 1; i <= itemCount; i++)
        {
            RexxInternalObject *item = newArray->get(i);
            if (item != OREF_NULL)
            {
                // append a linend between the previous item and this one.
                if (!first)
                {
                    mutbuffer->append(lineEndString);
                }
                RexxString *value = item->stringValue();
                mutbuffer->append(value);
                first = false;
            }
        }
    }

    return mutbuffer->makeString();
}


/**
 * Join two arrays into one array.
 *
 * @param other  The other array of the join operation.
 *
 * @return A new array will all elements of the first.
 */
RexxObject *ArrayClass::join(ArrayClass *other)
{
    /* get new array, total size is size */
    /* of both arrays.                   */
    // get a new array with the combined size of the two arrays.
    ArrayClass *newArray = (ArrayClass*)new_array(size() + other->size());
    // it's safe to just copy the references because the newArray will be new space
    // copy first array into new one
    memcpy(newArray->data(), data(), dataSize());
    // copy 2nd array into the new one after the first one
    memcpy((void *)newArray->slotAddress(this->size() + 1), other->data(), other->dataSize());
    // we need to update the items and the last item pointer
    newArray->itemCount = items() + other->items();
    newArray->lastItem = size() + other->lastItem;

    return newArray;
}


/**
 * An Array is being expanded so chop off the data (array)
 * portion of this array.
 */
void ArrayClass::resize()
{
    // still working off of the original array allocation?
    // if not, we need to chop down the initial allocation.
    if (!hasExpanded())
    {
        // if this array is an oldspace one, there's no
        // point in resizing this.  However, we do need to
        // clear out any of the entries so that the old-to-new table is
        // updated correctly.
        if (isOldSpace())
        {
            for (size_t i = 0; i < arraySize; i++)
            {
                // Note, we do this directly rather than use
                // the other setting methods because we don't
                // want touch the item count or last item, and
                // we certainly don't want to hit the expansion array
                // by mistake.
                setField(objects[i], OREF_NULL);
            }
            // we don't get marked as an oldspace object, but adjust the
            // array size down anyway.
            arraySize = 0;
        }
        else
        {
            // resize the array object
            memoryObject.reSize(this, sizeof(ArrayClass));
            // the outer array has no elements
            arraySize = 0;
        }
    }
}


/**
 * Low-level search in an array.  Searches are performed
 * on object identity rather than equality.
 *
 * @param target The target object.
 *
 * @return The index of the located object or 0 if not found.
 */
size_t ArrayClass::indexOf(RexxInternalObject *target)
{
    size_t count = size();
    for (size_t i = 1; i <= count; i++)
    {
        if (get(i) == target)
        {
            return i;
        }
    }
    return 0;
}


/**
 * Extend an array to a given number of elements Single
 * Dimension ONLY
 *
 * @param toSize The target size we need to extend to.
 */
void ArrayClass::extend(size_t toSize)
{
    // do we have room for this in our additional space?
    // we don't need to reallocate anything here, just adjust the size values
    if (toSize <= maximumSize)
    {
        // during marking, the main array and the expanson array both mark.  If we've
        // extended, we need to keep the size at zero in the original.  If we've not
        // extended, then updating the expansion array size also updates the original.
        expansionArray->arraySize = toSize;
        return;
    }

    if (toSize >= MaxFixedArraySize)
    {
        reportException(Error_Incorrect_method_array_too_big, MaxFixedArraySize - 1);
    }

    // double the size for small Arrays
    // for Arrays above the limit, just add half of the actual size
    size_t newSize = size();
    newSize += newSize <= ExpansionDoubleLimit ? newSize : newSize / 2;

    // now allocate the extension array of the required size + some extra.
    ArrayClass *newArray = new (toSize, newSize) ArrayClass;
    // The extension array, by definition, will not be in old space, so
    // we can just copy everything in one shot.
    memcpy(newArray->data(), data(), dataSize());

    // resize the original array item to a small size, but only
    // if this is the first extension.
    resize();

    // tell the expansion array it is the extension version
    newArray->expansionArray = OREF_NULL;

    // and update our expansion array pointer.
    setField(expansionArray, newArray);
    // keep max Size value in synch
    maximumSize = newArray->maximumSize;
}


/**
 * Find the index of a single item in the array.
 *
 * @param item   The item to locate.
 *
 * @return The numeric index of the item.
 */
size_t ArrayClass::findSingleIndexItem(RexxInternalObject *item)
{
    for (size_t i = 1; i <= lastItem; i++)
    {
        RexxInternalObject *test = get(i);

        // if there's an object in the slot, compare it.
        if (test != OREF_NULL)
        {
            // if the items are equal, return the index
            if (item->equalValue(test))
            {
                return i;
            }
        }
    }
    return 0;
}


/**
 * Convert an internal index item into "external" form.  Handles
 * both single- and multi-dimensional arrays.
 *
 * @param idx    The internal index to convert.
 *
 * @return An index object proper for the array type.
 */
RexxObject *ArrayClass::convertIndex(size_t idx)
{
    // 0 indicates no valid index, which always gets returned as .nil.
    if (idx == 0)
    {
        return TheNilObject;
    }

    // single dimension array?  This is easy
    if (isSingleDimensional())
    {
        return new_integer(idx);
    }
    else
    {
        // compose a composite index
        return indexToArray(idx);
    }
}


/**
 * Convert a multi-dimensional array index into an array
 * of index values for the flattened dimension.
 *
 * @param idx     The index to covert.
 *
 * @return An array of the individual index items.
 */
RexxObject* ArrayClass::indexToArray(size_t idx)
{
    // work with an origin-origin zero version of the index, which is easier
    // do work with.
    idx--;
    // get the number of dimensions specified.
    size_t dims = dimensions->size();
    // get an array we fill in as we go
    Protected<ArrayClass> index = new_array(dims);

    for (size_t i = dims; i > 0; i--)
    {
        // get the next dimension size
        size_t _dimension = dimensionSize(i);
        // now get the remainder.  This tells us the position within this
        // dimension of the array.  Make an integer object and store in the
        // array.
        size_t digit = idx % _dimension;
        // the digit is origin-zero, but the Rexx index is origin-one.
        index->put(new_integer(digit + 1), i);
        // now strip out that portion of the index.
        idx = (idx - digit) / _dimension;
    }
    return index;
}


/**
 * Return the index for the first occurrence of the target in
 * the array.
 *
 * @param target The target object.
 *
 * @return The index for the array.  For a multi-dimensional array, this
 *         returns an array of indices.
 */
RexxObject *ArrayClass::indexRexx(RexxObject *target)
{
    // we require the target to be there.
    requiredArgument(target, ARG_ONE);
    // see if we have this item.  If not, then
    // we return .nil.
    size_t index = findSingleIndexItem(target);

    // convert (handles the not found case as well)
    return convertIndex(index);
}


/**
 * Remove the target object from the collection.
 *
 * @param target The target object.
 *
 * @return The removed object (same as target).
 */
RexxInternalObject *ArrayClass::removeItemRexx(RexxObject *target)
{
    // we require the index to be there.
    requiredArgument(target, ARG_ONE);
    // do the removal
    return removeItem(target);
}


/**
 * Remove the target object from the collection.
 *
 * @param target The target object.
 *
 * @return The removed object (same as target).
 */
RexxInternalObject *ArrayClass::removeItem(RexxInternalObject *target)
{
    // see if we have this item.  If not, then
    // we return .nil.
    size_t index = findSingleIndexItem(target);

    if (index == 0)
    {
        return TheNilObject;
    }
    // remove the item at the location
    return remove(index);
}


/**
 * Test if an item is within the array.
 *
 * @param target The target test item.
 *
 * @return .true if this item exists in the array. .false if it does not
 *         exist.
 */
RexxObject *ArrayClass::hasItemRexx(RexxObject *target)
{
    // this is pretty simple.  One argument, required, and just search to see
    // if we have it.
    requiredArgument(target, ARG_ONE);
    return booleanObject(findSingleIndexItem(target) != 0);
}


/**
 * Test if an item is within the array.
 *
 * @param target The target test item.
 *
 * @return .true if this item exists in the array. .false if it does not
 *         exist.
 */
bool ArrayClass::hasItem(RexxInternalObject *target)
{
    return findSingleIndexItem(target) != 0;
}


/**
 * Test if an item is within the array.
 *
 * @param target The target test item.
 *
 * @return .true if this item exists in the array. .false if it does not
 *         exist.
 */
bool ArrayClass::hasIdentityItem(RexxInternalObject *target)
{
    for (size_t i = 1; i <= lastItem; i++)
    {
        RexxInternalObject *test = get(i);

        if (test == target)
        {
            return true;
        }
    }
    return false;
}


/**
 * Recursive routine to copy element from one multiDim array
 * to another.  This works in reverse order, drilling down to the
 * highest level, copying, and then unwinding to get the
 * higher dimensions.
 *
 * @param parm   The copy parameters, which are updated through each
 *               recursion level.
 * @param newDimension
 */
void ArrayClass::ElementCopier::copyElements(size_t newDimension)
{
    // We don't do anything about different dimensions here or worry about which
    // dimensions have changed, we just to a complete recursive copy.

    // When all is said and done, everything in the array is just sections of the
    // if the last (highest numbered dimension)
    if (newDimension == highestDimension)
    {
        for (size_t i = 1; i <= elementsToCopy; i++, startNew++, startOld++ )
        {
            // this only updates the count and last item for real values.
            newArray->copyArrayItem(startNew, oldArray->get(startOld));
        }
        // bump for any delta caused by differences between the dimensions.
        startNew += elementsToSkip;
    }
    else
    {
        // get the relative sizes of for this dimension
        size_t newDimSize = newArray->dimensionSize(newDimension);
        size_t oldDimSize = oldArray->dimensionSize(newDimension);
        // For each subscript at this level, recurse on the copy call,
        // but only for the sections of the old array where we have elements
        for (size_t i= 1; i <= oldDimSize; i++)
        {
            copyElements(newDimension + 1);
        }

        // was this dimension expanded?  we need to compute the skip amounts
        // to be applied when we return back
        if (newDimSize > oldDimSize)
        {
            for (size_t i = newArray->getDimensions(), skipAmount = 1; i > newDimension; i--)
            {
                skipAmount *= newArray->dimensionSize(i);
            }
            // multiply by delta add at this dimension level (could be zero here)
            size_t skipAmount = (newDimSize - oldDimSize);
            // bump our output position past the added empty space for this dimension
            startNew += skipAmount;
        }
    }
    return;
}


/**
 * Extend an array by a given number of elements.  A Multiple
 * Dimension Index either has more dimensions that the current
 * array or is the same number.  So the expansion array will
 * have same number of dimensions as this index. Examine size of
 * each dimension and the new array will be max of two sizes.
 *
 * @param index      The pointer to the argument index items.
 * @param indexCount The count of indexes.
 * @param argPosition
 *                   The argument position (used for error reporting)
 */
void ArrayClass::extendMulti(RexxObject **index, size_t indexCount, size_t argPosition)
{
    // our new dimensions array will be the same as the number of indexes.
    Protected<NumberArray> newDimArray = new (indexCount) NumberArray(indexCount);

    // used for optimizing the copy operations
    size_t firstChangedDimension = getDimensions() + 1;
    // this is a multiplier to calculate the total required
    // array size.
    size_t totalSize = 1;

    // extending from a single dimension into multi-dimension?
    // the subscripts determine everything.
    if (isSingleDimensional())
    {
        // we never call this for a single dimension array when
        //.the size is anything other than zero, so the new
        // dimensions are totally determined by subscripts.

        for (size_t i = 0; i < indexCount; i++)
        {
            // NOTE:  We're processing this as a subscript really, so
            // parse them as position arguments.  Validate each one
            // and just copy into the dimensions array
            size_t dimensionSize = positionArgument(index[i], i + 1);
            totalSize = totalSize * dimensionSize;
            newDimArray->put(dimensionSize, i + 1);
        }
    }
    else
    {
        // we need to process each index subscript and compare each
        //.against the corresponding dimensions.  Our new size will
        // use the larger of the two values.

        for (size_t i = 0; i < indexCount; i++)
        {
            // NOTE:  We're processing this as a subscript really, so
            // parse them as position arguments.  Validate each one
            // and just copy into the dimensions array
            size_t newDimensionSize = positionArgument(index[i], i + 1);
            size_t oldDimensionSize = dimensionSize(i + 1);

            // keep track of where we find the first difference
            if (newDimensionSize > oldDimensionSize)
            {
                firstChangedDimension = std::min(firstChangedDimension, i + 1);
            }

            size_t newSize = std::max(newDimensionSize, oldDimensionSize);
            totalSize = totalSize * newSize;

            newDimArray->put(newSize, i + 1);
        }
    }


    // Now create the new array for this dimension.  This also
    // creates the dimensions array in the new array
    Protected<ArrayClass> newArray = new_array(totalSize, newDimArray);


    // anything in the original?
    if (items() > 0)
    {
        // yes, move values into the new array.  This only occurs if we've
        // expanded one or more dimensions of a multi-dimension array, so we're
        // copying between two multidimension arrays

        size_t accumSize = 1;
        // For all dimensions before the first to change, we can
        // move things in bulk....
        for (size_t i = getDimensions(); i > firstChangedDimension; i--)
        {
            accumSize *= dimensionSize(i);
        }

        ElementCopier copier;

        // Compute lowest largest contiguous chunk that can be copied
        copier.elementsToCopy = accumSize * dimensionSize(firstChangedDimension);
        // Compute amount need to skip to complete expanded dimension
        copier.elementsToSkip = accumSize * newArray->dimensionSize(firstChangedDimension) - dimensionSize(firstChangedDimension);

        // copying starts at the beginning of both
        copier.startNew = 1;
        copier.startOld = 1;

        // Setup parameter structure
        copier.highestDimension = firstChangedDimension;
        copier.newArray = newArray;
        copier.oldArray = this;
        // go copy the elements
        copier.copy();
    }

    // resize the original, if necessary
    resize();

    // currently, we have the old dimensions while the expansion array has
    // the new.  Set them here and make the expansion array into a real expansion array
    setField(dimensions, (NumberArray *)newDimArray);
    // mark this as not the main array.
    newArray->expansionArray = OREF_NULL;
    setField(expansionArray, newArray);

    // keep max Size value in sync with expansion
    maximumSize = newArray->maximumSize;
    // copy operation will have updated the items and last item
    // values in the expansion array.  Copy them into the base array
    itemCount = newArray->itemCount;
    lastItem = newArray->lastItem;
}


/**
 * Insert an element into the array, shifting all of the
 * existing elements at the inserted position and adjusting
 * the size accordingly.
 *
 * @param value  The value to insert (can be OREF_NULL to just open a new slot)
 * @param _index The insertion index position.
 *
 * @return The index of the inserted item.
 */
RexxInternalObject *ArrayClass::deleteItem(size_t index)
{
    // if this index is out of the allowed range,
    // just return OREF_NULL;
    if (!isInbounds(index))
    {
        return OREF_NULL;
    }

    RexxInternalObject *result = get(index);   // save the return value
    closeGap(index, 1);          // close up the gap for the deleted item
                                 // return .nil if there's nothing at that position
    return result;
}


/**
 * Append all elements of an array to this array.
 *
 * @param other  The source array.
 */
void ArrayClass::appendAll(ArrayClass *other)
{
    size_t count = other->size();
    for (size_t i = 1; i <= count; i++)
    {
        append(other->get(i));
    }
}


/**
 * The merge sort routine.  This will partition the data in to
 * two sections, mergesort each partition, then merge the two
 * partitions together.
 *
 * @param comparator The comparator object used for the compares.
 * @param working    The working array (same size as the sorted array).
 * @param left       The left bound of the partition.
 * @param right      The right bounds of the parition
 *                   (inclusive).
 */
void ArrayClass::mergeSort(BaseSortComparator &comparator, ArrayClass *working, size_t left, size_t right)
{
    // General note about sorting.  These are in place sorts where the same items will be in the
    // array after the sort operation.  The item count will be the same and the last item will be the
    // same.  There are no old-to-new considerations either because we will end up with the same
    // number of cross references once this completes.  Therefore, all movement between these arrays
    // occur using a special assignment method that is not to be used in other code.

    size_t len = right - left + 1;  // total size of the range
    // use insertion sort for small arrays
    if (len <= 10)
    {
        for (size_t i = left + 1; i <= right; i++)
        {
            // During the insertion sort operation, 'current' may be temporarily
            // out of both the source and working arrays, so it is vulnerable to
            // garbage collection during subsequent compare operations.
            // We need to give an extra layer of protection here.
            Protected<RexxInternalObject> current = get(i);
            RexxInternalObject *prev = get(i - 1);
            if (comparator.compare(current, prev) < 0)
            {
                size_t j = i;
                do
                {
                    setSortItem(j--, prev);
                } while (j > left && comparator.compare(current, prev = get(j - 1)) < 0);
                setSortItem(j, current);
            }
        }
        return;
    }

    size_t mid = (right + left) / 2;
    mergeSort(comparator, working, left, mid);
    mergeSort(comparator, working, mid + 1, right);
    merge(comparator, working, left, mid + 1, right);
}


/**
 * Perform the merge operation on two partitions.
 *
 * @param comparator The comparator used to produce the ordering.
 * @param working    The temporary working storage.
 * @param left       The left bound of the range.
 * @param mid        The midpoint of the range.  This merges the two partitions
 *                   (left, mid - 1) and (mid, right).
 * @param right      The right bound of the array.
 */
void ArrayClass::merge(BaseSortComparator &comparator, ArrayClass *working, size_t left, size_t mid, size_t right)
{
    size_t leftEnd = mid - 1;
    // merging

    // if arrays are already sorted - no merge
    if (comparator.compare(get(leftEnd), get(mid)) <= 0)
    {
        return;
    }

    size_t leftCursor = left;
    size_t rightCursor = mid;
    size_t workingPosition = left;

    // use merging with exponential search
    do
    {
        RexxInternalObject *fromVal = get(leftCursor);
        RexxInternalObject *rightVal = get(rightCursor);
        // if the left value is the smaller one, so we try to find the
        // insertion point of the right value into the left side of the
        // the array
        if (comparator.compare(fromVal, rightVal) <= 0)
        {
            // try to find an insertion point in the remaining left-hand elements
            size_t leftInsertion = find(comparator, rightVal, -1, leftCursor + 1, leftEnd);
            // we start copying with the left-hand bound up to the insertion point
            size_t toCopy = leftInsertion - leftCursor + 1;
            arraycopy(this, leftCursor, working, workingPosition, toCopy);
            workingPosition += toCopy;
            // add the inserted position
            working->setSortItem(workingPosition++, rightVal);
            // now we've added this
            rightCursor++;
            // step over the section we just copied...which might be
            // all of the remaining section
            leftCursor = leftInsertion + 1;
        }
        else
        {
            // find the insertion point of the left value into the remaining right
            // hand section
            size_t rightInsertion = find(comparator, fromVal, 0, rightCursor + 1, right);
            size_t toCopy = rightInsertion - rightCursor + 1;
            arraycopy(this, rightCursor, working, workingPosition, toCopy);
            workingPosition += toCopy;
            // insert the right-hand value
            working->setSortItem(workingPosition++, fromVal);
            leftCursor++;
            rightCursor = rightInsertion + 1;
        }
    } while (right >= rightCursor && mid > leftCursor);

    // copy rest of array.  If we've not used up the left hand side,
    // we copy that.  Otherwise, there are items on the right side
    if (leftCursor < mid)
    {
        arraycopy(this, leftCursor, working, workingPosition, mid - leftCursor);
    }
    else
    {
        arraycopy(this, rightCursor, working, workingPosition, right - rightCursor + 1);
    }

    // finally, copy everything back into the the target array.
    arraycopy(working, left, this, left, right - left + 1);
}


/**
 * copy segments of one array into another.
 *
 * @param source The source array
 * @param start  The starting index of the source copy
 * @param target The target array
 * @param index  The target copy index
 * @param count  The number of items to count.
 */
void ArrayClass::arraycopy(ArrayClass *source, size_t start, ArrayClass *target, size_t index, size_t count)
{
    for (size_t i = start; i < start + count; i++)
    {
        target->setSortItem(index++, source->get(i));
    }
}


/**
 * Finds the place in the given range of specified sorted array, where the
 * element should be inserted for getting sorted array. Uses exponential
 * search algorithm.
 *
 * @param comparator The comparator used to compare pairs of elements.
 * @param val        object to be inserted
 * @param limit      possible values 0,-1. "-1" - val is located
 *                   at index more then elements equals to val.
 *                   "0" - val is located at index less then
 *                   elements equals to val.
 * @param left       The left bound of the insert operation.
 * @param right      The right bound for the insert (inclusive)
 *
 * @return The insertion point.
 */
size_t ArrayClass::find(BaseSortComparator &comparator, RexxInternalObject *val, int limit, size_t left, size_t right)
{
    size_t checkPoint = left;
    size_t delta = 1;
    while (checkPoint <= right)
    {
        // if this is too big, then we're moving to the right
        if (comparator.compare(val, get(checkPoint)) > limit)
        {
            // the left bound is at least this
            left = checkPoint + 1;
        }
        else
        {
            // we've found a right limit.  We can stop scanning here
            right = checkPoint - 1;
            break;
        }
        // step the delta amount
        checkPoint += delta;
        // and double the movement amount
        delta = delta * 2;
    }
    // we should have now limited the bounds for the insertion point
    // now start in the middle and shrink the range with each comparison
    while (left <= right)
    {
        // start in the middle of the current range
        checkPoint = (left + right) / 2;
        if (comparator.compare(val, get(checkPoint)) > limit)
        {
            // pull in the left end of the range
            left = checkPoint + 1;
        }
        else
        {
            // chop the right range
            right = checkPoint - 1;
        }
    }

    // the left bound is the insertion point
    return left - 1;
}


/**
 * Sort elements of the array in place, using a stable merge
 * sort.
 *
 * @return Returns the same array, with the elements sorted.
 */
ArrayClass *ArrayClass::stableSortRexx()
{
    size_t count = items();
    if (count == 0)         // if the count is zero, sorting is easy!
    {
        return this;
    }

    // make sure this is a non-sparse array.  Checking up front means we don't
    // need to check on each compare operation.
    for (size_t i = 1; i <= count; i++)
    {
        if (get(i) == OREF_NULL)
        {
            reportException(Error_Execution_sparse_array, i);
        }
    }

    // the merge sort requires a temporary scratch area for the sort.
    Protected<ArrayClass> working = new_array(count);

    BaseSortComparator comparator;

    // go do the quick sort
    mergeSort(comparator, working, 1, count);
    return this;
}


/**
 * Sort elements of the array in place, using a stable merge
 * sort.
 *
 * @return Returns the same array, with the elements sorted.
 */
ArrayClass *ArrayClass::stableSortWithRexx(RexxObject *comparator)
{
    requiredArgument(comparator, ARG_ONE);

    size_t count = items();
    if (count <= 1)         // if the count is zero, sorting is easy!
    {
        return this;
    }

    // make sure this is a non-sparse array.  Checking up front means we don't
    // need to check on each compare operation.
    for (size_t i = 1; i <= count; i++)
    {
        if (get(i) == OREF_NULL)
        {
            reportException(Error_Execution_sparse_array, i);
        }
    }

    // the merge sort requires a temporary scratch area for the sort.
    Protected<ArrayClass> working = new_array(count);

    WithSortComparator c(comparator);

    // go do the quick sort
    mergeSort(c, working, 1, count);
    return this;
}


wholenumber_t ArrayClass::BaseSortComparator::compare(RexxInternalObject *first, RexxInternalObject *second)
{
    return first->compareTo(second);
}


wholenumber_t ArrayClass::WithSortComparator::compare(RexxInternalObject *first, RexxInternalObject *second)
{
    ProtectedObject result;
    comparator->sendMessage(GlobalNames::COMPARE, (RexxObject *)first, (RexxObject *)second, result);
    if (result.isNull())
    {
        reportException(Error_No_result_object_message, GlobalNames::COMPARE);
    }

    wholenumber_t comparison;
    if (!result->numberValue(comparison, Numerics::DEFAULT_DIGITS))
    {
        reportException(Error_Invalid_whole_number_compare, (RexxObject *)result);
    }
    return comparison;
}


