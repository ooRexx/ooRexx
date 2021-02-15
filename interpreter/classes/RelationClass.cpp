/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* Code for the Rexx relation class.                                          */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RelationClass.hpp"
#include "MethodArguments.hpp"
#include "ProtectedObject.hpp"

// singleton class instance
RexxClass *RelationClass::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RelationClass::createInstance()
{
    CLASS_CREATE(Relation);
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
 * construct a RelationClass with a given size.
 *
 * @param capacity The required capacity.
 */
RelationClass::RelationClass(size_t capacity)
{
    // get a suggested bucket size for this capacity
    // NOTE:  all of this needs to be done at the top-level constructor
    // because of the way C++ constructors work.  As each
    // previous contructor level gets called, the virtual function
    // pointer gets changed to match the class of the contructor getting
    // called.  We don't have access to our allocateContents() override
    // until the final constructor is run.
    size_t bucketSize = calculateBucketSize(capacity);
    contents = allocateContents(bucketSize, bucketSize *2);
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

    // create the new identity table item (this version does not have a backing contents yet).
    Protected<RelationClass> temp = new RelationClass(true);
    // finish setting this up.
    classThis->completeNewObject(temp, args, argCount);

    // make sure this has been completely initialized
    temp->initialize();
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
    return contents->removeItem(value, index);
}


/**
 * Get a supplier for either the whole collection or a subset based
 * on the index.
 *
 * @param index  The optional index.
 *
 * @return A supplier object for iterating over the collection items.
 */
SupplierClass *RelationClass::supplierRexx(RexxObject *index)
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
RexxObject *RelationClass::itemsRexx(RexxObject *index)
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
RexxObject *RelationClass::removeItemRexx(RexxObject *value, RexxObject *index)
{
    requiredArgument(value,ARG_ONE);
    return resultOrNil(contents->removeItem(value, index));
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
RexxObject *RelationClass::hasItemRexx(RexxObject *value, RexxObject *index)
{
    requiredArgument(value, ARG_ONE);
    return booleanObject(contents->hasItem(value, index));
}


/**
 * Return all indexes associated with a single value.
 *
 * @param value  The target value.
 *
 * @return The associated indexes.
 */
ArrayClass *RelationClass::allIndexRexx(RexxObject *value)
{
    requiredArgument(value, ARG_ONE);
    return contents->allIndex(value);
}


/**
 * Return all values associated with the same index.
 *
 * @param index The target index.
 *
 * @return An array holding all of the indexes.
 */
ArrayClass *RelationClass::allAt(RexxObject *index)
{
    requiredArgument(index, ARG_ONE);
    return contents->getAll(index);
}


/**
 * Remove all items with a given index.
 *
 * @param index The index to remove.
 *
 * @return An array of all removed items.  Returns an empty array
 *         if the index is not found.
 */
ArrayClass *RelationClass::removeAll(RexxObject *index)
{
    requiredArgument(index, ARG_ONE);
    return contents->removeAll(index);
}

