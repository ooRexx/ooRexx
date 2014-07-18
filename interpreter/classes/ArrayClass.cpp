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
#include "RexxActivity.hpp"
#include "IntegerClass.hpp"
#include "SupplierClass.hpp"
#include "ArrayClass.hpp"
#include "MutableBufferClass.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *ArrayClass::classInstance = OREF_NULL;
ArrayClass *ArrayClass::nullArray = OREF_NULL;

const size_t ArrayClass::MAX_FIXEDARRAY_SIZE = (Numerics::MAX_WHOLENUMBER/10) + 1;
const size_t ArrayClass::ARRAY_MIN_SIZE = 4;
const size_t ArrayClass::ARRAY_DEFAULT_SIZE = 10;    // we use a larger default for ooRexx allocated arrays

/**
 * Create initial class object at bootstrap time.
 */
void ArrayClass::createInstance()
{
    CLASS_CREATE(Array, "Array", RexxClass);
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
RexxObject * ArrayClass::newRexx(RexxObject **arguments, size_t argCount)
{
    // this method is defined as an instance method, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // creating an array of the default size?
    if (argCount == 0)
    {
        Protected<ArrayClass> temp = new (0, ARRAY_DEFAULT_SIZE) ArrayClass;
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
        if (currentDim != OREF_NULL && isOfClass(Array, currentDim))
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
            temp->dimensions = new_array(IntegerZero);
        }

        // finish the class initialization and init calls.
        classThis->completeNewObject(temp);
    }

    // more than one argument, so all arguments must be valid size values.
    return createMultidimensional(arguments, argCount, classThis);
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
    size_t totalSize = size->requiredNonNegative(position, number_digits());

    if (totalSize >= MAX_FIXEDARRAY_SIZE)
    {
        reportException(Error_Incorrect_method_array_too_big);
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
    Protected<ArrayClass> dim_array = new_array(count);

    // we need to calculate total size needed for this array
    // since we multiply each additional size in, start with a zeroth size of 1
    size_t totalSize = 1;
    for (size_t i = 0; i < count; i++)
    {
        // each dimension must be there and must be a non-negative integer value
        // ...a dimension of 0 really does not make sense, right?
        // TODO: Figure out the impact of specifying 0 in a multi-dimension array...does this
        // make sense at all?
        RexxObject *currentDim = dims[i];
        size_t currentSize = currentDim->requiredNonNegative(i + 1);
        // going to do an overflow?  By dividing, we can detect a
        // wrap situation.
        if (curSize != 0 && ((MaxFixedArraySize / currentSize) < totalSize))
        {
            reportException(Error_Incorrect_method_array_too_big);
        }
        // keep running total size and put integer object into current position
        totalSize *= currentSize;

        // add this to our dimensions array, using a new integer object so
        // we know what is there.
        dim_array->put(new_integer(currentSize), i + 1);
    }

    // a final sanity check for out of bounds
    if (totalSize >= MaxFixxedArraySize)
    {
        reportException(Error_Incorrect_method_array_too_big);
    }

    // create a new array item

    // TODO:  make sure the constructor sets the itemCount to 0.
    Protected<ArrayClass> temp = new (totalSize) ArrayClass;

    // set the dimension array
    temp->dimensions = dim_array;

    // finish the class initialization and init calls.
    classThis->completeNewObject(temp);
    return temp;
}


/**
 * Rexx accessible version of the OF method.
 *
 * @param args     The pointer to the OF arguments.
 * @param argCount The count of arguments.
 *
 * @return A new array of the target class, populated with the argument objects.
 */
RexxObject *ArrayClass::of(RexxObject **args, size_t argCount)
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
    Protected<ArrayClass> newArray = new (argCount) ArrayClass(args, argCount);

    // finish the class initialization and init calls.
    classThis->completeNewObject(newArray);
    return newArray;
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
    size_t bytes = size;
    // we never create below a minimum size
    maxSize = Numerics::maxVal(maxSize, MinimumArraySize);
    // and use at least the size as the max
    maxSize = Numerics::maxVal(maxSize, items);
    // add in the max size value.  Note that we subtract one since
    // the first item is contained in the base object allocation.
    bytes += sizeof(RexxInternalObject *) * (maxSize - 1);
    // now allocate the new object with that size.
    ArrayClass *newArray = (ArrayClass *)new_object(bytes, T_Array);

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
ArrayClass::ArrayClass(RexxObject **objs, size_t count)
{
    // if we have array items, do a quick copy of the object references
    if (count != 0)
    {
        for (size_t i = 1; i <= count; i++)
        {
            if (objs[i - 1] != OREF_NULL)
            {
                // this ensures the item count and the last item are updated properly.
                setArrayItem(objs[i - 1], i);
            }
        }
    }
}


/**
 * Copy the array (and the expansion array, if necessary)
 *
 * @return A copy of the array object.
 */
RexxObject *ArrayClass::copy()
{
    // TODO:  if we have an expansion array, then allocate
    // just a single item of the correct size and copy that.
    // could be a significant performance improvement.

    // if we've expanded in the past, then just copy the
    // expansion array and update it with the appropriate
    // values from this array.
    if (hasExpanded())
    {
        // make a copy of ourself
        ArrayClass *newArray = (ArrayClass *)expansionArray->copy();
        newArray->dimensions = dimensions;
        // TODO:  Double check how arraySize is handled with the expansion array.
        // I don't think that needs to be copied, but not sure.  maximumSize might also
        // be suspect.
        newArray->maximumSize = maximumSize;
        newArray->lastItem = lastItem;
        newArray->itemCount = itemCount;
        // the copy is non-expanded now.
        newArray->expansionArray = newArray;
    }
    else
    {
        // make a copy of ourself
        ArrayClass *newArray = (ArrayClass *)RexxObject::copy();
        // The expansion array needs to point to itself in the new object,
        // not back to us.
        newArray->expansionArray = newArray;
    }

    // return the new array
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


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void ArrayClass::flatten(RexxEnvelope *envelope)
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
void ArrayClass::checkMultiDimentional(char *methodName)
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
inline void ArrayClass::setItem(RexxInternalObject *value, size_t position)
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
 * Simple inline method for clearing an array position
 *
 * @param value    The value to set.
 * @param position The index position.
 */
inline void ArrayClass::zeroItem(size_t position)
{
    data()[position - 1] = OREF_NULL;
}


/**
 * Inline method for setting an object into the array and
 * updating the item count and last item positions, if
 * necessary.
 *
 * @param value    The value to set.
 * @param position The index position.
 */
inline void ArrayClass::setOrClearArrayItem(RexxInternalObject *value, size_t position)
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
        setArrayItem(value, position);
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
inline void ArrayClass::setArrayItem(RexxInternalObject *value, size_t position)
{
    // if not overwriting an existing item, update the item count
    checkItemCount(position);
    setItem(expansionArray, objects[position - 1], value);
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
inline void ArrayClass::clearArrayItem(RexxInternalObject *value, size_t position)
{
    // see if clearing this field alters the item count
    checkClearItemCount();
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
    setArrayItem(value, pos);
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
    // TODO:  consider some helper routines for sorting out the
    // variable arguments here.
    if (argcount < 2)
    {
        reportException(Error_Incorrect_method_minarg, IntegerTwo);
    }
    // The first argument is a required value
    RexxObject *value = arguments[0];
    requiredArgument(value, ARG_ONE);

    // all the rest of the arguments are the index...go valicate it, and expand if necessary
    size_t position;
    validateIndex(arguments + 1, argCount - 1, 2, RaiseBoundsInvalid | ExtendUpper | RaiseBoundsTooMany, position);

    // set the new value and return nothing
    put(value, position);
    return OREF_NULL;
}


/**
 * Fill all locations of the array with the given object
 *
 * @return No return value.
 */
RexxObject *ArrayClass::fill(RexxInternalObject *value)
{
    requiredArgument(value, ARG_ONE);


    for (size_t i = 1; i <= size(); i++)
    {
        // set the item using the fast version...we
        // can just set the itemCount and lastItem afterwards
        setItem(value, i);
    }

    // we are full...item count and last item are both maxed out.
    itemCount = size();
    lastItem = size();

    return OREF_NULL;     // no real return value
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
        for (size_t i = i; i <= size(); i++)
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
    return OREF_NULL;     // no real return value
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
RexxObject *ArrayClass::appendRexx(RexxInternalObject *value)
{
    requiredArgument(value, ARG_ONE);
    checkMultiDimensional(CHAR_APPEND);

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
    size_t newIndex = lastIndex + 1;
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
    checkMultiDimensional(CHAR_INSERT);

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
        validateIndex(&index, 1, 2, RaiseBoundsInvalid | RaiseBoundsTooMany, position);
        position = position + 1;          // we insert AFTER the given index, so bump this
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
    openGap(index, 1);                // open an appropriate sized gap in the array
    lastItem++;                       // even if inserting at the end, the last item will move.
    // only do the put() operation if we have a real value to put so that
    // the item count doesn't get updated incorrectly.
    if (value != OREF_NULL)
    {
        put(value, index)
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
RexxObject *ArrayClass::deleteRexx(RexxObject *index)
{
    checkMultiDimensional("DELETE");

    size_t position;                     // array position

    // validate the index and expand if necessary.
    validateIndex(&index, 1, 1, RaiseBoundsInvalid | RaiseBoundsTooMany, position);

    // do the actual insertion
    return deleteItem(position);
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
    // is this larger than our current size?  If so, we have nothing to move
    // but do need to expand the array size to accommodate the additional members
    if (index > size())
    {
        ensureSpace(index + elements - 1);
    }
    else
    {
        // the last element to move.  NOTE:  we check this BEFORE
        // expanding the size, otherwise we move too many elements.
        char *_end = (char *)slotAddress(this->size() + 1);

        // make sure we have space for the additional elements
        ensureSpace(size() + elements);
                                             /* get the address of first element  */
        char *_start = (char *)slotAddress(index);
        char *_target = (char *)slotAddress(index + elements);
        /* shift the array over              */
        memmove(_target, _start, _end - _start);

        // now null out all of the slots in the gap, using an
        // explicit assignment rather than put to avoid old-to-new
        // tracking issues
        for (size_t i = index; i <= index + elements - 1; i++)
        {
            zeroItem(i);
        }
        lastElement += elements;     // the position of the last element has now moved.
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
    // if we're beyond the current size, nothing to do
    if (index > size())
    {
        return;
    }

    // cap the number of elements we're shifting.
    elements = Numerics::minVal(elements, lastElement - index + 1);

    // explicitly null out the slots of the gap we're closing to
    // ensure that any oldspace tracking issues are resolved.
    // use the clear method to ensure the item count is getting updated
    // appropriate.
    for (size_t i = index; i < index + elements; i++)
    {
        clearArrayItem(i);
    }
    // get the address of first element and the first item to move.
    char *_target = (char *)slotAddress(index);
    char *_start =  (char *)slotAddress(index + elements);
    // and the end location of the real data
    char *_end = (char *)slotAddress(lastElement + 1);
    // shift the array over
    memmove(_target, _start, _end - _start);
    // adjust the last element position
    lastElement -= elements;
    // adjust the size downward
    shrink(elements);
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
RexxObject  *ArrayClass::getRexx(RexxObject **arguments, size_t argCount)
{
    size_t position;
    // validate the index
    if (validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid, position))
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
    if (!inBounds(index) || !isOccupied(index))
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
RexxObject *ArrayClass::removeRexx(RexxObject **arguments, size_t argCount)
{
    size_t position;
    // if this position doesn't validate, just return .nil
    if (!validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid, position))
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
 * Query the number of dimensions of an array object.
 *
 * @return The count of dimensions.
 */
size_t ArrayClass::getDimension()
{
    return isSingleDimensional() ? 1 : dimensions->size();
}


/**
 * Return an array of the dimensions of this array.
 *
 * @return An array item with one size item for each dimension of the
 *         array.
 */
ArrayClass *ArrayClass::getDimensions()
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
        return dimensions->copy();
    }
}


/**
 * Request a specific dimension from an array object.
 *
 * @param target The target dimension.
 *
 * @return The size of the dimension...returns 0 for any unknown ones.
 */
RexxObject *ArrayClass::dimension(RexxObject *target)
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
            only return something if the requested position was 0
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
            return dimensions->get(position);
        }
    }
}


/**
 * Create a supplier for this array.
 *
 * @return an appropriate supplier.
 */
RexxObject *ArrayClass::supplier()
{
    size_t slotCount = size();            // get the array size
    size_t itemCount = items();           // and the actual count in the array

    ArrayClass *values = new_array(itemCount);       /* get the values array              */
    ArrayClass *indexes = new_array(itemCount);      /* and an index array                */

    ProtectedObject v(values);
    ProtectedObject s(indexes);

    size_t count = 1;                           /* next place to add                 */
    for (size_t i = 1; i <= slotCount; i++)
    {   /* loop through the array            */
        RexxObject *item = this->get(i);     /* get the next item                 */
        if (item != OREF_NULL)
        {           /* got an item here                  */
            values->put(item, count);        /* copy over to the values array     */

                                             /* add the index location            */
            indexes->put((RexxObject*)convertIndex(i), count);
            count++;                         /* step the location                 */
        }
    }

    return new_supplier(values, indexes);
}


void  ArrayClass::setExpansion(RexxObject * expansion)
/******************************************************************************/
/* Function:  Set a new expansion array item                                  */
/******************************************************************************/
{
    OrefSet(this, this->expansionArray, (ArrayClass *)expansion);
}


bool  ArrayClass::validateIndex(        /* validate an array index           */
    RexxObject **_index,               /* array index (possibly multi-dim)  */
    size_t       indexCount,           /* size of the index array           */
    size_t       _start,               /* starting point on the array       */
    size_t       bounds_error,         /* raise errors on out-of-bounds     */
    stringsize_t &position)             // returned position
/******************************************************************************/
/* Function:  Process and validate a potentially multi-dimensional array      */
/*            index.  If the index is out of bounds in any dimension it will  */
/*            either return false or raise an error, depending on the bounds  */
/*            checking parameter.                                             */
/******************************************************************************/
{
    RexxObject *value;                   /* individual index value            */
    size_t  numsubs;                     /* number of subscripts              */
    size_t  i;                           /* loop counter                      */
    size_t  multiplier;                  /* accumlation factor                */
    size_t  offset;                      /* accumulated offset                */
    size_t  _dimension;                  /* current working dimension         */
    size_t  numSize;                     /* temporary long variable           */


    // do we really have a single index item given as an array?
    if (indexCount == 1 && _index[0] != OREF_NULL && isOfClass(Array, _index[0]))
    {
        // we process this exactly the same way, but swap the count and
        // pointers around to be the array data.
        ArrayClass *indirect = (ArrayClass *)_index[0];
        indexCount = indirect->items();
        _index = indirect->data();
    }

    /* Is this array one-dimensional?    */
    if (isSingleDimensional())
    {
        /* Too many subscripts?  Say so.     */
        if (indexCount > 1)
        {
            /* should the array be extended?     */
            if ((bounds_error & ExtendUpper) && this->dimensions == OREF_NULL)
            {
                /* anytyhing in the array?           */
                /* or explicitly created array with 0*/
                /* elements (.array~new(0))          */
                if (this->size() != 0)
                {
                    /* Yes, number of dims can't change  */
                    /* report apropriate bounds          */
                    reportException(Error_Incorrect_method_maxsub, IntegerOne);
                }
                else
                {
                    /* its empty, entendarray for new siz*/
                    /* extend the array.                 */
                    this->extendMulti(_index, indexCount, _start);
                    /* Call us again to get position, now*/
                    /* That the array is extended.       */
                    return this->validateIndex(_index, indexCount, _start, bounds_error, position);
                }
            }

            else if (bounds_error & RaiseBoundsTooMany)
            {
                /* anytyhing in the array?           */
                /* or explicitly created array with 0*/
                /* elements (.array~new(0))          */
                if (this->dimensions != OREF_NULL || this->size() != 0)
                {
                    /* report apropriate bounds          */
                    reportException(Error_Incorrect_method_maxsub, IntegerOne);
                }
                else
                {
                    return false;                /* just report not here              */
                }
            }
            else
            {
                return false;                  /* not fixed yet, but don't complain */
            }
        }
        /* Too few? subscripts?  Say so.     */
        else if (indexCount == 0)
        {
            /* report apropriate bounds          */
            reportException(Error_Incorrect_method_minarg, _start);
        }
        /* validate integer index            */
        position = _index[0]->requiredPositive((int)_start);
        /* out of bounds?                    */
        if (position > this->size() )
        {
            if (position >= MAX_FIXEDARRAY_SIZE)
            {
                reportException(Error_Incorrect_method_array_too_big);
            }
            /* are we to expand the array?       */
            if (bounds_error & ExtendUpper)
            {
                /* yes, compute amount to expand     */
                this->extend(position - this->size());

            }
            /* need to raise an error?           */
            else if (bounds_error & RaiseBoundsUpper)
            {
                reportException(Error_Incorrect_method_array, position);
            }
            else
            {
                return false;                  /* just return indicator             */
            }
        }
    }
    else
    {                               /* multidimensional array            */
                                    /* get the number of subscripts      */
        numsubs = indexCount;
        numSize = this->dimensions->size();/* Get the size of dimension */
                                           /* right number of subscripts?       */
        if (numsubs == numSize)
        {
            multiplier = 1;                  /* multiply by 1 for first dimension */
            offset = 0;                      /* no accumulated offset             */

            for (i = numsubs; i > 0; i--)
            {  /* loop through the dimensions       */

                value = _index[i - 1];

                if (value == OREF_NULL)        /* not given?                        */
                {
                    /* this is an error too              */
                    reportException(Error_Incorrect_method_noarg, i + _start);
                }
                /* validate integer index            */
                position = value->requiredPositive((int)i);
                /* get the current dimension         */
                _dimension = ((RexxInteger *)this->dimensions->get(i))->getValue();
                if (position > _dimension)
                {   /* too large?                        */
                    /* should the array be extended?     */
                    if (bounds_error & ExtendUpper)
                    {
                        /* go extend it.                     */
                        this->extendMulti(_index, indexCount, _start);
                        /* Call us again to get position, now*/
                        /* That the array is extended.       */
                        return this->validateIndex(_index, indexCount, _start, bounds_error, position);
                    }
                    /* need to raise an error?           */
                    else if (bounds_error & RaiseBoundsUpper)
                    {
                        reportException(Error_Incorrect_method_array, position);
                    }
                    else
                    {
                        return false;              /* just return indicator             */
                    }
                }
                /* calculate next offset             */
                offset += multiplier * (position - 1);
                multiplier *= _dimension;      /* step the multiplier               */
            }
            position = offset + 1;           /* get accumulated position          */
        }
        /* Not enough subscripts?            */
        else if (numsubs < numSize)
        {
            reportException(Error_Incorrect_method_minsub, numSize);
        }
        // must be too many subscripts
        else
        {
            reportException(Error_Incorrect_method_maxsub, numSize);
        }
    }
    return true;                         /* return the position               */
}


/**
 * Make sure that the array has been expanded to sufficient
 * size for a primitive put operation.
 *
 * @param newSize The new required size.
 */
void ArrayClass::ensureSpace(size_t newSize)
{
    /* out of bounds?                    */
    if (newSize > this->size())
    {
        if (newSize >= MAX_FIXEDARRAY_SIZE)
        {
            reportException(Error_Incorrect_method_array_too_big);
        }
        /* yes, compute amount to expand     */
        this->extend(newSize - this->size());

    }
}



RexxInteger *ArrayClass::sizeRexx()
/******************************************************************************/
/* Function:  Return the array size as an integer object                      */
/******************************************************************************/
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
ArrayClass *ArrayClass::section(size_t _start, size_t _end)
{
    size_t newSize;                      /* Size for new array.               */
    ArrayClass *newArray;                 /* The new array.                    */

    if (_start == 0)                      /* starting position specified?      */
    {
        _start = 1;                         /* nope, start at begining           */
    }

    /* ending position omitted           */
    /* or, End pos past end of array?    */
    if (_end == 0 || _end > this->size())
    {
        _end = this->size();                /* yes,  use end of array            */
    }
    if (_start <= _end)
    {                  /* is start before end?              */
        newSize = _end + 1 - _start;         /* yes, compute new size             */
        /* Get new array, of needed size     */
        newArray = (ArrayClass *)new_array(newSize);
        // a new array cannot be oldspace, by definition.  It's safe to use
        // memcpy to copy the data.
        /* yes, we can do a memcpy           */
        memcpy(newArray->data(), slotAddress(_start), sizeof(RexxObject *) * newSize);
    }
    else
    {
        /* return 0 element array            */
        newArray = (ArrayClass *)new_array((size_t)0);
    }
    return newArray;                     /* return the newly created array    */
}

/**
 * Extract a section of the array as another array
 *
 * @param _start The starting index position
 * @param _end   The number of items to section.
 *
 * @return A new array containing the section elements.
 */
RexxObject *ArrayClass::sectionRexx(RexxObject * _start, RexxObject * _end)
{
    /* multidimensional array?           */
    if (isMultiDimensional())
    {
        /* this is an error                  */
        reportException(Error_Incorrect_method_section);
    }

    // the index is required
    requiredArgument(_start, ARG_ONE);
    size_t nstart;                    // array position

    // validate the index and expand if necessary.
    this->validateIndex(&_start, 1, 1, RaiseBoundsInvalid | RaiseBoundsTooMany, nstart);
    size_t nend = 0; ;
    if (_end == OREF_NULL)                /* If no end position specified,     */
    {
        nend = this->size();               /* Defaults to last element          */
    }
    else
    {                               /* End specified - check it out      */
        nend = _end->requiredNonNegative(ARG_TWO);
    }

    if (!isOfClass(Array, this))             /* actually an array subclass?       */
    {
        /* need to do this the slow way      */
        return this->sectionSubclass(nstart, nend);
    }

    if (nstart > this->size())           /* too big?                          */
    {
        // return a zero-size array
        return (ArrayClass *)(((ArrayClass *)TheNullArray)->copy());
    }
    else
    {
        /* go past the bounds?               */
        if (nend > this->size() - nstart + 1)
        {
            nend = this->size() - nstart + 1;/* truncate to the end               */
        }
        if (nend == 0)                     /* requesting zero?                  */
        {
            /* return zero elements              */
            return (ArrayClass *)(((ArrayClass *)TheNullArray)->copy());
        }
        else
        {                             /* real sectioning to do             */
                                      /* create a new array                */
            ArrayClass *rref = (ArrayClass *)new_array(nend);
            for (size_t i = 1; i <= nend; i++)
            {    /* loop through the elements         */
                 /* copy an element                   */
                rref->put(this->get(nstart + i - 1), i);
            }
            return rref;                         /* return the new array              */
        }
    }
}

RexxObject *ArrayClass::sectionSubclass(
    size_t _start,                      /* starting element                  */
    size_t _end )                       /* ending element                    */
/******************************************************************************/
/* Function:  Rexx level section method                                       */
/******************************************************************************/
{
    size_t i;                            /* loop counter                      */
    ArrayClass *newArray;                 /* returned array                    */
    ProtectedObject result;

    if (_start > this->size())           /* too big?                          */
    {
        this->behaviour->getOwningClass()->sendMessage(OREF_NEW, IntegerZero, result);
        newArray = (ArrayClass *)(RexxObject *)result;
    }
    /* return a zero element one         */
    else
    {
        if (_end > this->size() - _start + 1)
        {
            /* go past the bounds?               */
            _end = this->size() - _start + 1;/* truncate to the end               */
        }
        if (_end == 0)                     /* requesting zero?                  */
        {
            this->behaviour->getOwningClass()->sendMessage(OREF_NEW, IntegerZero, result);
            newArray = (ArrayClass *)(RexxObject *)result;
        }
        /* return a zero element one         */
        else
        {                             /* real sectioning to do             */
                                      /* create a new array                */
            this->behaviour->getOwningClass()->sendMessage(OREF_NEW, new_integer(_end), result);
            newArray = (ArrayClass *)(RexxObject *)result;
            for (i = 1; i <= _end; i++)       /* loop through the elements         */
            {
                /* copy an element                   */
                newArray->sendMessage(OREF_PUT, this->get(_start + i - 1), new_integer(i));
            }
        }
    }
    return newArray;                     /* return the new array              */
}

RexxObject  *ArrayClass::firstRexx()
/******************************************************************************/
/* Function:  Retrieve the first element index from the array as an integer   */
/*            object                                                          */
/******************************************************************************/
{
    /* get the address of the first      */
    /*element in the array               */
    RexxObject **thisObject = this->expansionArray->objects;
    size_t _arraySize = this->size();            /* get the size of the array         */
    /* find first position in the        */
    /*array with data                    */

    size_t i;
    for (i = 0; i < _arraySize && thisObject[i] == OREF_NULL; i++);

    if (i == _arraySize)                  /* is array empty                    */
    {
        return TheNilObject;              /* return nil object                 */
    }
    else
    {
        /* return index of the first entry   */
        return convertIndex(i + 1);
    }
}

RexxObject  *ArrayClass::lastRexx()
/******************************************************************************/
/* Function:  Return the index of the last array item as an integer object    */
/******************************************************************************/
{
    // for an empy array, the index is .nil
    if (lastElement == 0)
    {
        return TheNilObject;
    }

    // return as an object
    return (RexxObject *)convertIndex(lastElement);
}

/**
 * Return the first item in the array, or .nil if the
 * array is empty.
 *
 * @return The first item in the array
 */
RexxObject  *ArrayClass::firstItem()
{
    /* get the address of the first      */
    /*element in the array               */
    RexxObject **thisObject = this->expansionArray->objects;
    size_t _arraySize = this->size();            /* get the size of the array         */
    /* find first position in the        */
    /*array with data                    */

    for (size_t i = 0; i < _arraySize; i++)
    {
        // if we have a non-nil object, return it
        if (thisObject[i] != OREF_NULL)
        {
            return thisObject[i];
        }
    }
    // not found, return .nil
    return TheNilObject;
}

/**
 * Return the last item in the array.
 *
 * @return The last item, or .nil if the array is empty.
 */
RexxObject  *ArrayClass::lastItem()
{
    // for an empy array, the item is .nil
    if (lastElement == 0)
    {
        return TheNilObject;
    }

    // return the last item
    return (RexxObject *)get(lastElement);
}

size_t ArrayClass::lastIndex()
/******************************************************************************/
/* Function:  Return the index of the last array item as an integer object    */
/******************************************************************************/
{
    return lastElement;       // we've kept track of this
}

RexxObject  *ArrayClass::nextRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Return the next entry after a given array index                 */
/******************************************************************************/
{
    size_t position;
    /* go validate the index             */
    if (!this->validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid, position))
    {
        // out of bounds results in the .nil object
        return TheNilObject;
    }
    /* get the address of the first      */
    /*element in the array               */
    RexxObject **thisObject = this->data();
    size_t _arraySize = this->size();            /* get the size of the array         */
    /* find next entry in the array with */
    /*data                               */

    size_t i;
    for (i = position; i < _arraySize && thisObject[i] == OREF_NULL; i++);

    if (i >= this->size())
    {
        return TheNilObject;             /* return nil object                 */
    }
    else
    {
        /* return index of the next entry    */
        return convertIndex(i + 1);
    }
}

RexxObject  *ArrayClass::previousRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Return the index preceeding a given index                       */
/******************************************************************************/
{
    size_t position;

    this->validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid, position);
    /* get the index object into an      */
    /*integer object                     */
    size_t i = position;

    size_t _arraySize = this->size();     /* get the size of the array         */

    if (i > _arraySize)                   /* beyond the size of the array?     */
    {
        /* set i to one more than the last   */
        /*entry                              */
        i = _arraySize;
    }
    else
    {
        i = i-1;                           /* Account for 0 based 'C' arrays    */
    }

                                           /* get the address of the first      */
                                           /*element in the array               */
    RexxObject **thisObject = this->expansionArray->objects;
    /* find previous entry in the        */
    /*array with data                    */
    for (; i > 0 && thisObject[i-1] == OREF_NULL; i--);

    if (i == 0)
    {
        return TheNilObject;                /* return nil object                 */
    }
    else
    {
        /* return the index to the           */
        /*previous entry                     */
        return convertIndex(i);
    }
}

RexxObject  *ArrayClass::hasIndexRexx(RexxObject ** _index, size_t _indexCount)
/******************************************************************************/
/*  Function:  True if array has an entry for the index, false otherwise      */
/*  Note:  This routine should not raise an error, regardless of the indices  */
/*         being used.  The only error produced is if no parms were passed.   */
/******************************************************************************/
{
    stringsize_t position;               /* array position                    */

                                         /* go validate the index             */
    if (!this->validateIndex(_index, _indexCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid, position))
    {
        /* this is false                     */
        return TheFalseObject;

    }
    else                                 /* check the position                */
    {
        /* have a real entry?                */
        if (*(this->data() + position - 1) != OREF_NULL)
        {
            /* got a true                        */
            return TheTrueObject;
        }
        else
        {
            /* no index here                     */
            return TheFalseObject;
        }
    }
}

bool ArrayClass::hasIndexNative(size_t _index)
/******************************************************************************/
/* Function:  Determine if an element exist for a position                    */
/******************************************************************************/
{
    /* in bounds and here?               */
    if (_index > 0 && _index <= this->size() && *(this->data() + _index - 1) != OREF_NULL)
    {
        return true;                       /* this is true                      */
    }
    else
    {
        return false;                      /* nope, don't have it               */
    }
}

ArrayClass *ArrayClass::makeArray(void)
/******************************************************************************/
/* Function: Return a single dimension array of self, with only items that    */
/*           has values, so it will be of size items.                         */
/******************************************************************************/
{
    // for an array, this is the all items value.
    return this->allItems();
}


/**
 * Return an array of all real items contained in the collection.
 *
 * @return An array with all of the array items (non-sparse).
 */
ArrayClass *ArrayClass::allItems()
{
    // get a result array of the appropriate size
    ArrayClass *newArray = (ArrayClass *)new_array(this->items());

    // we need to fill in based on actual items, not the index.
    size_t count = 0;
    RexxObject **item = this->data();
    // loop through the array, copying all of the items.
    for (size_t iterator = 0; iterator < this->size(); iterator++ )
    {
        // if this is a real array item, copy over to the result
        if (item[iterator] != OREF_NULL)
        {
            newArray->put(item[iterator], ++count);
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
    ArrayClass *newArray = (ArrayClass *)new_array(this->items());
    ProtectedObject p(newArray);

    // we need to fill in based on actual items, not the index.
    size_t count = 0;
    RexxObject **item = this->data();
    // loop through the array, copying all of the items.
    for (size_t iterator = 0; iterator < this->size(); iterator++ )
    {
        // if this is a real array item, add an integer index item to the
        // result collection.
        if (item[iterator] != OREF_NULL)
        {
            newArray->put(convertIndex(iterator+1), ++count);
        }
    }
    return newArray;
}

// Temporary bypass for BUG #1700606
#if 0
RexxString  *ArrayClass::primitiveMakeString()
/******************************************************************************/
/* Function:  Handle a REQUEST('STRING') request for a REXX string object     */
/******************************************************************************/
{
    return this->makeString((RexxString *)OREF_NULL);    /* forward to the real makestring method */
}
#endif

RexxString *ArrayClass::makeString(RexxString *format, RexxString *separator)
{
    return toString(format, separator);
}

RexxString *ArrayClass::toString(       /* concatenate array elements to create string object */
     RexxString *format,               /* format of concatenation (one of: "C", "L")         */
     RexxString *separator)            /* separator to use if "L" is specified for format    */
/******************************************************************************/
/* Function:  Make a string out of an array                                   */
/******************************************************************************/
{
    size_t _items;
    size_t i;
    ArrayClass *newArray;                  /* New array                         */
    RexxString *newString;
    RexxString *line_end_string;          /* converted substitution value      */
    RexxMutableBuffer *mutbuffer;
    RexxObject *item;                     /* inserted value item               */
    int i_form = 0;                       /* 1 == line, 2 == char              */

    mutbuffer = new RexxMutableBuffer();
    ProtectedObject p1(mutbuffer);

    newArray = this->makeArray();          /* maybe multidimensional, make onedimensional  */
    ProtectedObject p2(newArray);

    _items = newArray->items();            /* and the actual count in the array */

    if (format != OREF_NULL)
    {
        // a string value is required here
        format = stringArgument(format, ARG_ONE);
    }

    if (format == OREF_NULL)
    {
        i_form = 2;                         /* treat item as LINE by default   */
    }
    else if (toupper((format->getStringData()[0])) == 'C')
    {
        i_form = 1;
    }
    else if (toupper((format->getStringData()[0])) == 'L')
    {
        i_form = 2;
    }
    else
    {
        reportException(Error_Incorrect_method_option, "CL", format);
    }

    if (i_form == 1)                       /* character oriented processing    */
    {
        if (separator != OREF_NULL)
        {
            reportException(Error_Incorrect_method_maxarg, IntegerOne);

        }

        for (i = 1; i <=_items ; i++)         /* loop through the array           */
        {
            item = newArray->get(i);           /* get the next item                */
            if (item != OREF_NULL)
            {
                RexxObject * _stringValue = item->requiredString();
                if (_stringValue != TheNilObject)
                {
                    mutbuffer->append(_stringValue);
                }
            }
        }
    }
    else if (i_form == 2)                 /* line oriented processing          */
    {
        if (separator != OREF_NULL)
        {
            line_end_string = stringArgument(separator, ARG_TWO);
        }
        else
        {
            line_end_string = new_string(line_end);
        }

        ProtectedObject p3(line_end_string);
        bool first = true;

        for (i = 1; i <= _items; i++)      /* loop through the array            */
        {
            item = newArray->get(i);       /* get the next item                 */
            if (item != OREF_NULL)
            {
                // append a linend between the previous item and this one.
                if (!first)
                {
                    mutbuffer->append((RexxObject *) line_end_string);
                }
                RexxObject *_stringValue = item->requiredString();
                if (_stringValue != TheNilObject)
                {
                    mutbuffer->append(_stringValue);
                }
                first = false;
            }
        }
    }

    newString = mutbuffer->makeString();
    return newString;
}

RexxObject *ArrayClass::join(           /* join two arrays into one          */
     ArrayClass *other)                 /* array to be appended to self      */
/******************************************************************************/
/* Function:  Join two arrays into one array                                  */
/******************************************************************************/
{
    /* get new array, total size is size */
    /* of both arrays.                   */
    ArrayClass *newArray = (ArrayClass*)new_array(this->size() + other->size());
    // it's safe to just copy the references because the newArray will be new space
    /* copy first array into new one     */
    memcpy(newArray->data(), this->data(), this->dataSize());
    /* copy 2nd array into the new one   */
    /* after the first one.              */
    memcpy((void *)newArray->slotAddress(this->size() + 1), other->data(), other->dataSize());
    return newArray;                     /* All done, return joined array     */

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
                setField(objects[i], OREF_NULL);
            }
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

void ArrayClass::shrink(
     size_t amount)                    /* amount to shrink an array         */
/******************************************************************************/
/* Function:  Shrink an array without reallocating any elements               */
/*            Single Dimension ONLY                                           */
/******************************************************************************/
{
    size_t _size = this->size();                 /* get the size                      */
    size_t newSize = _size - amount;             /* get the new size                  */

    this->expansionArray->arraySize = newSize;
}


size_t ArrayClass::indexOf(
    RexxObject *target)                /* target object to locate           */
/*****************************************************************************/
/* Function:  To search a list in the form of an array for an item, returning*/
/*            the index                                                      */
/*****************************************************************************/
{
    size_t _size = this->size();                /* get the array size                */
    for (size_t i = 1; i <= _size; i++)
    {       /* spin through the array            */
        if (this->get(i) == target)        /* is this the one?                  */
        {
            return i;                        /* return the index                  */
        }
    }
    return 0;                            /* not found here                    */
}


ArrayClass *ArrayClass::extend(          /* join two arrays into one          */
    size_t extension)                  /* number of elements to extend      */
/******************************************************************************/
/* Function:  Extend an array by a given number of elements                   */
/*            Single Dimension ONLY                                           */
/******************************************************************************/
{
    /* do we really need to extend array */
    /* or just adjust size.              */
    if (this->size() + extension <= this->maximumSize)
    {
        /* adjust the size .                 */
        this->expansionArray->arraySize += extension;
        return this;
    }

    size_t newSize = this->size() + extension;
    size_t extendSize = this->size() / 2;

    /* get a new array, total size is    */
    /* size of both arrays.              */
    ArrayClass *newArray = (ArrayClass *)new_array(newSize + extendSize);
    /* If none of the objects are in     */
    /* OldSpace,  we can skip the        */
    /* OrefSets and just copy            */
    /* copy ourselves into the new array */
    memcpy(newArray->data(), this->data(), this->dataSize());
    this->resize();                      /* adjust ourself to be null arrayobj*/

    newArray->setExpansion(OREF_NULL);   /* clear the new expansion array     */
                                         /* set new expansion array           */
    OrefSet(this, this->expansionArray, newArray);
    /* keep max Size value in synch      */
    /* with expansion.                   */
    this->maximumSize = newArray->maximumSize;
    /* make sure size is correct.        */
    newArray->arraySize = newSize;
    return this;                         /* All done, return array            */
}


/**
 * Find the index of a single item in the array.
 *
 * @param item   The item to locate.
 *
 * @return The numeric index of the item.
 */
size_t ArrayClass::findSingleIndexItem(RexxObject *item)
{
    for (size_t i = 1; i <= this->size(); i++)
    {
        RexxObject *test = get(i);

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
    size_t dims = this->dimensions->size();
    // get an array we fill in as we go
    ArrayClass * _index = new_array(dims);

    ProtectedObject p(_index);

    for (size_t i = dims; i > 0; i--)
    {
        // get the next dimension size
        size_t _dimension = ((RexxInteger *)this->dimensions->get(i))->getValue();
        // now get the remainder.  This tells us the position within this
        // dimension of the array.  Make an integer object and store in the
        // array.
        size_t digit = idx % _dimension;
        // the digit is origin-zero, but the Rexx index is origin-one.
        _index->put(new_integer(digit + 1), i);
        // now strip out that portion of the index.
        idx = (idx - digit) / _dimension;
    }
    return _index;
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
RexxObject *ArrayClass::index(RexxObject *target)
{
    // we require the index to be there.
    requiredArgument(target, ARG_ONE);
    // see if we have this item.  If not, then
    // we return .nil.
    size_t _index = findSingleIndexItem(target);

    if (_index == 0)
    {
        return TheNilObject;
    }

    return convertIndex(_index);
}


/**
 * Remove the target object from the collection.
 *
 * @param target The target object.
 *
 * @return The removed object (same as target).
 */
RexxObject *ArrayClass::removeItem(RexxObject *target)
{
    // we require the index to be there.
    requiredArgument(target, ARG_ONE);
    // see if we have this item.  If not, then
    // we return .nil.
    size_t _index = findSingleIndexItem(target);

    if (_index == 0)
    {
        return TheNilObject;
    }
    // remove the item at the location
    remove(_index);
    return target;
}


/**
 * Test if an item is within the array.
 *
 * @param target The target test item.
 *
 * @return .true if this item exists in the array. .false if it does not
 *         exist.
 */
RexxObject *ArrayClass::hasItem(RexxObject *target)
{
    // this is pretty simple.  One argument, required, and just search to see
    // if we have it.
    requiredArgument(target, ARG_ONE);
    return booleanObject(findSingleIndexItem(target) == 0);
}


void copyElements(
    COPYELEMENTPARM *parm,
    size_t newDimension)
/******************************************************************************/
/* Function:  Recursive routine to copy element from one multiDim array       */
/*            to another.                                                     */
/******************************************************************************/
{
    size_t skipAmount;                   /* amount to skip for increased      */
                                         /* dimension.                        */
    size_t newDimSize;
    size_t oldDimSize;
    size_t oldDimension;
    size_t i;                            /* count for each subscript at       */
                                         /* this dimension                    */

                                         /* At the point where we can         */
                                         /* copy elements?                    */
                                         /* ending condition for recusion     */
    if (newDimension == parm->firstChangedDimension)
    {
        /* is new array in OldSpace?         */
        if (parm->newArray->isOldSpace())
        {
            /* Yes,need to do OrefSets           */
            /* For each element to copy          */
            for (i = 1; i <= parm->copyElements; i++, parm->startNew++, parm->startOld++ )
            {
                /* set the newvalue.                 */
                OrefSet(parm->newArray, *parm->startNew, *parm->startOld);
            }
        }
        else
        {
            /* not old Spcae we can do memcpy    */
            memcpy(parm->startNew, parm->startOld, sizeof(RexxObject *) * parm->copyElements);
            /* update pointers                   */
            parm->startNew += parm->copyElements;
            parm->startOld += parm->copyElements;
        }
        /* now bump past space for           */
        /* additional size of dimension      */
        parm->startNew += parm->skipElements;
    }
    else
    {
        /* Compute the old dimension num     */
        oldDimension = newDimension - parm->deltaDimSize;
        /* Get size for new Dimension        */
        newDimSize = ((RexxInteger *)parm->newDimArray->get(newDimension))->getValue();
        /* Get size for old Dimension        */
        oldDimSize = ((RexxInteger *)parm->oldDimArray->get(oldDimension))->getValue();
        /* For each subscript at this        */
        for (i= 1; i <= oldDimSize; i++)
        {/* dimension, (of old size)          */
         /* copy elelments.                   */
            copyElements(parm, newDimension + 1);
        }
        if (newDimSize > oldDimSize)
        {    /* Was this dimension expanded?      */
             /* compute total space need for      */
             /* block of all lower dimensions     */
            for (i = parm->newDimArray->size(), skipAmount = 1;
                i > newDimension;
                skipAmount *= ((RexxInteger *)parm->newDimArray->get(i))->getValue(), i--);
            /* multiple by delta add at this     */
            /* dimension.                        */
            skipAmount *= (newDimSize - oldDimSize);
            /* Bump our start pointer past       */
            /* empty space added for this        */
            /* dimension.                        */
            parm->startNew += skipAmount;
        }
    }
    /* all done at this level return     */
    /* to caller.                        */
    return;
}


ArrayClass *ArrayClass::extendMulti(     /* Extend multi array                */
     RexxObject ** _index,             /* Dimension array is to be          */
     size_t _indexCount,               /* number of indices in the index    */
     size_t _start)                    /* Starting position of dimensions   */
                                       /*  in index.                        */
/******************************************************************************/
/* Function:  Extend an array by a given number of elements                   */
/*            Multiple Dimensions                                             */
/*            Index either has more dimensions that the current array or      */
/*             is the same number.  So expansion array will have same number  */
/*             of dimensions as index.                                        */
/*            So examine size of each dimension and new array will be max     */
/*            of two sizes.                                                   */
/*                                                                            */
/*   NOTE: Even though we don't currently allow for changing the number       */
/*     of dimensions in an array, the support to do this, is included         */
/*     in this method.  We currently won't be called for this condition       */
/*     but if we ever are, this method should not need to be changed.         */
/*                                                                            */
/******************************************************************************/
{
    size_t currDimSize;                  /* Current size of Dimension         */
    size_t additionalDim;                /* Number of additional DImension    */
    size_t newDimSize;                   /* New size for this Dimension       */
    size_t newDimension;                 /* Current dimension                 */
    size_t oldDimension;                 /* Current dimension                 */
    size_t i;
    ArrayClass *newArray;
    ArrayClass *newDimArray;              /* Array containing new dimension    */
    size_t newDimArraySize;              /* Size of Dimension Array           */
    size_t accumSize;
    size_t firstDimChanged = 0;          /* First Dimension to grow           */
    COPYELEMENTPARM copyParm;            /* Structure for copyElement         */
    size_t  tempSize;

    /* New dimension array size of       */
    /* index array.                      */
    /* index is actually 1 bigger tha    */
    /* dimension size, since it          */
    /* contains the new value at 1st     */
    /* position                          */

    /* Compute new Size for DimArray     */
    newDimArraySize = _indexCount;
    newDimArray = new_array(newDimArraySize);
    ProtectedObject p(newDimArray);
    /* extending from single Dimensio    */
    /*  to a multi Dimensionsal array    */
    if (this->dimensions == OREF_NULL)
    {
        /* Get value for 1st dimension       */
        /*  its the last element             */
        i = newDimArraySize - 1;
        newDimSize = _index[i]->requiredPositive((int)i);
        /* Yes, is 1st Dimension going to    */
        /* be bigger than current size?      */
        if (newDimSize > this->size())
            /* Yes, use new size + buffer for    */
            /*  1st Dimension                    */
            newDimArray->put(new_integer(newDimSize), newDimArraySize);
        else
        {
            /* nope, use same size for Dim       */
            tempSize = this->size();
            newDimArray->put(new_integer(tempSize), newDimArraySize);
        }
    }
    else
    {
        for (oldDimension = this->dimensions->size(), newDimension = newDimArraySize;
            oldDimension > 0 ;
            oldDimension--, newDimension--)
        {
            /* Get current size of this dimension*/
            currDimSize = ((RexxInteger *)this->dimensions->get(oldDimension))->getValue();
            /* Get indexd  size of this dimension*/

            newDimSize = _index[newDimension - 1]->requiredPositive((int)newDimension);
            /* does this dimension need to be    */
            /*  expanded.                        */
            if (newDimSize > currDimSize)
            {
                newDimArray->put((RexxObject *)new_integer(newDimSize), newDimension);
                /* has a dimension already been chang*/
                if (!firstDimChanged)
                {
                    /* remember the first dimenion chenge*/
                    firstDimChanged = newDimension;
                }
            }
            else
            {
                newDimArray->put(this->dimensions->get(oldDimension), newDimension);
            }
        }
    }
    /* Was original array single dim     */
    if (this->dimensions == OREF_NULL)
    {
        /* additional Dimensions is 1        */
        /*  minus size, (1st Dimension)      */
        additionalDim = newDimArraySize - 1;
    }
    else
    {
        /* compute number of dimensions added*/
        additionalDim = newDimArraySize - this->dimensions->size();
    }
    /* is index greater than current     */
    /*  dimensions of this array.        */
    if (additionalDim > 0)
    {
        /* yes, for remainder of dimensions  */
        for (newDimension = additionalDim;
            newDimension > 0 ;
            newDimension--)
        {
            /* Get indexd  size of this dimension*/
            newDimSize = ((RexxInteger *)_index[newDimension - 1])->getValue();
            /* set up value for this dimension   */
            newDimArray->put(new_integer(newDimSize), newDimension);
        }
    }
    /* Now create the new array for this */
    /*  dimension.                       */
    newArray = new_array(newDimArraySize, newDimArray->data());
    ProtectedObject p1(newArray);
    /* Anything in original?             */
    if (this->size())
    {
        /* Yes, move values into new arra    */
        /* Extending from single Dimension   */
        /* or original array was empty       */
        /* or adding dimensions or increas   */
        /* last original dimension?          */
        if (isSingleDimensional() ||
            this->size() == 0 ||
            !firstDimChanged || firstDimChanged <= additionalDim + 1)
        {
            /* If none of the objects are in     */
            /* OldSpace,  we can 'cheat' and do  */
            /* a fast copy (memcpy)              */
            /* copy ourselves into the new array */
            memcpy(newArray->data(), this->data(), sizeof(RexxObject *) * this->size());
        }
        /* Working with 2 multi-dimensional  */
        /* Array's.                          */
        else
        {
            /* Now we need to move all elements  */

            /* for all Dimensions before first   */
            /* to increase.                      */
            for (i = newDimArraySize, accumSize = 1;
                i > firstDimChanged ;
                accumSize *= ((RexxInteger *)this->dimensions->get(i))->getValue(), i--);
            /* Compute lowest largest contig     */
            /* chunk that can be copied.         */
            copyParm.copyElements = accumSize * ((RexxInteger *)this->dimensions->get(firstDimChanged))->getValue();
            /* Compute amount need to ship       */
            /* to compete expanded dimension     */
            copyParm.skipElements = accumSize * (((RexxInteger *)newDimArray->get(firstDimChanged))->getValue() -
                                                 ((RexxInteger *)this->dimensions->get(firstDimChanged))->getValue());

            copyParm.startNew = newArray->data();
            copyParm.startOld = this->data();
            /* Setup parameter structure         */
            copyParm.firstChangedDimension = firstDimChanged;
            copyParm.newArray = newArray;
            copyParm.newDimArray = newDimArray;
            copyParm.oldDimArray = this->dimensions;
            /* Compute delta dimensions size     */
            copyParm.deltaDimSize = newDimArraySize - this->dimensions->size();
            /* do the copy. starting w/          */
            /* Highest Dimension                 */
            copyElements(&copyParm, newDimArraySize - this->dimensions->size() + 1);
        }
    }                                    /* end, if anything in original      */


    this->resize();
    /* Set dimensions to be new dimension*/
    OrefSet(this, this->dimensions, newDimArray);
    newArray->setExpansion(OREF_NULL);
    /* update expansion array.           */
    OrefSet(this, this->expansionArray, newArray);
    /* keep max Size value in synch      */
    /* with expansion.                   */
    this->maximumSize = newArray->maximumSize;
    return this;                         /* All done, return array            */
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
RexxObject *ArrayClass::deleteItem(size_t  _index)
{
    RexxObject *result = get(_index);   // save the return value
    closeGap(_index, 1);         // close up the gap for the deleted item
                                 // return .nil if there's nothing at that position
    return resultOrNil(result);
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
    size_t len = right - left + 1;  // total size of the range
    // use insertion sort for small arrays
    if (len <= 7) {
        for (size_t i = left + 1; i <= right; i++) {
            RexxObject *current = get(i);
            RexxObject *prev = get(i - 1);
            if (comparator.compare(current, prev) < 0) {
                size_t j = i;
                do {
                    put(prev, j--);
                } while (j > left && comparator.compare(current, prev = get(j - 1)) < 0);
                put(current, j);
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
    if (comparator.compare(get(leftEnd), get(mid)) <= 0) {
        return;
    }

    size_t leftCursor = left;
    size_t rightCursor = mid;
    size_t workingPosition = left;

    // use merging with exponential search
    do
    {
        RexxObject *fromVal = get(leftCursor);
        RexxObject *rightVal = get(rightCursor);
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
            working->put(rightVal, workingPosition++);
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
            working->put(fromVal, workingPosition++);
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
        target->put(source->get(i), index++);
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
size_t ArrayClass::find(BaseSortComparator &comparator, RexxObject *val, int limit, size_t left, size_t right)
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
    ArrayClass *working = new_array(count);
    ProtectedObject p(working);

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
    ArrayClass *working = new_array(count);
    ProtectedObject p(working);

    WithSortComparator c(comparator);

    // go do the quick sort
    mergeSort(c, working, 1, count);
    return this;
}


wholenumber_t BaseSortComparator::compare(RexxObject *first, RexxObject *second)
{
    return first->compareTo(second);
}


wholenumber_t WithSortComparator::compare(RexxObject *first, RexxObject *second)
{
    ProtectedObject result;
    comparator->sendMessage(OREF_COMPARE, first, second, result);
    if ((RexxObject *)result == OREF_NULL)
    {
        reportException(Error_No_result_object_message, OREF_COMPARE);
    }

    wholenumber_t comparison;
    if (!((RexxObject *)result)->numberValue(comparison, Numerics::DEFAULT_DIGITS))
    {
        reportException(Error_Invalid_whole_number_compare, (RexxObject *)result);
    }
    return comparison;
}
