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
/* Code for the Rexx relation class.                                          */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RelationClass.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *RelationClass::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RelationClass::createInstance()
{
    CLASS_CREATE(Relation, "Relation", RexxClass);
}


/**
 * Create a new table object.
 *
 * @param size   the size of the object.
 *
 * @return Storage for creating a table object.
 */
void *RelationClass::operator new (size_t size)
{
    return new_object(size, T_Table);
}


/**
 * Create a new table instance from Rexx code.
 *
 * @param args     The new arguments.
 * @param argCount The count of new arguments.
 *
 * @return The constructed instance.
 */
RexxObject *RelationClass::newRexx(RexxObject **args, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxObject *initialSize;

    // parse the arguments
    RexxClass::processNewArgs(args, argCount, &args, &argCount, 1, (RexxObject **)&initialSize, NULL);

    // the capacity is optional, but must be a positive numeric value
    size_t capacity = optionalLengthArgument(initialSize, DefaultTableSize, ARG_ONE);

    // create the new identity table item
    RelationClass *temp = new RelationClass(capacity);
    ProtectedObject p(temp);
    // finish setting this up.
    classThis->completeNewObject(temp, args, argCount);
    return temp;
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
HashContents *RelationClass::allocateContents(size_t bucketSize, size_t totalSize)
{
    return new (totalSize) MultiValueContents(bucketSize, totalSize);
}


/**
 * Remove an item from the collection using a value and
 * an optional index qualifier.
 *
 * @param value  The item to remove.
 * @param index  The index qualifier.
 *
 * @return The removed item.
 */
RexxInternalObject *RelationClass::removeItem(RexxInternalObject *value, RexxInternalObject *index)
{
    return contents->removeItem(_value, _index);
}


/**
 * Get a supplier for either the whole collection or a subset based
 * on the index.
 *
 * @param index  The optional index.
 *
 * @return A supplier object for iterating over the collection items.
 */
SupplierClass *RelationClass::supplierRexx(RexxInternalObject *index)
{
    return contents->supplier(index);
}


/**
 * Returns the count of items in the collection, or if
 * an index is specified, the number of items associated with that index.
 *
 * @param index The target index.
 *
 * @return The appropriate count of items.
 */
RexxInternalObject *RelationClass::itemsRexx(RexxInternalObject *index)
{
    return new_integer(contents->items(index));
}


/**
 * Remove an item from a relation, optionally qualified with an index value.
 *
 * @param value The target value.
 * @param index The optional index.
 *
 * @return The removed item, if any.
 */
RexxInternalObject *RelationClass::removeItemRexx(RexxInternalObject *value, RexxInternalObject *index)
{
    requiredArgument(_value, ARG_ONE);            /* make sure we have a value         */
    RexxInternalObject item = contents->removeItem(value, index);

    // if nothing was removed, return .nil
    if (item == OREF_NULL)
    {
        item = TheNilObject;
    }
    return item;
}


/**
 * Test for an existance of an item in the collection, optionally
 * qualified by an index value.
 *
 * @param value  The target value.
 * @param index  The optional index.
 *
 * @return .true if this was found, .false otherwise.
 */
RexxInternalObject *RelationClass::hasItemRexx(RexxInternalObject *value, RexxInternalObject *index)
{
    requiredArgument(_value, ARG_ONE);
    return contents->hasItem(value, index) ? TheTrueObject : TheFalseObject;
}


/**
 * Return all indexes associated with a single value.
 *
 * @param value  The target value.
 *
 * @return The associated indexes.
 */
RexxInternalObject *RelationClass::allIndexRexx(RexxInternalObject *value)
{
    requiredArgument(_value, ARG_ONE);
    return this->contents->allIndex(value);
}


/**
 * Return all values associated with the same index.
 *
 * @param index The target index.
 *
 * @return An array holding all of the indexes.
 */
RexxInternalObject *RelationClass::allAt(RexxInternalObject *index)
{
    requiredArgument(index, ARG_ONE);
    return allIndex(index);
}


/**
 * Remove all items with a given index.
 *
 * @param index The index to remove.
 *
 * @return An array of all removed items.  Returns an empty array
 *         if the index is not found.
 */
RexxInternalObject *RelationClass::removeAll(RexxInternalObject *index)
{
    requiredArgument(index, ARG_ONE);
    return contents->removeAll(_index);
}

