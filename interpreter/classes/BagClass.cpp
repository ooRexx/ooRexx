/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2017 Rexx Language Association. All rights reserved.    */
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
/****************************************************************************/
/* REXX Kernel                                                              */
/*                                                                          */
/* The StringTable class code                                               */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "BagClass.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *BagClass::classInstance = OREF_NULL;

/**
 * Create initial class object at bootstrap time.
 */
void BagClass::createInstance()
{
    CLASS_CREATE(Bag);
}


/**
 * Create a new table object.
 *
 * @param size   the size of the object.
 *
 * @return Storage for creating a table object.
 */
void *BagClass::operator new (size_t size)
{
    return new_object(size, T_Bag);
}


/**
 * construct a BagClass with a given size.
 *
 * @param capacity The required capacity.
 */
BagClass::BagClass(size_t capacity)
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
 * Virtual method for allocating a new contents item for this
 * collection.  Collections with special requirements should
 * override this and return the appropriate subclass.
 *
 * @param bucketSize The bucket size of the collection.
 * @param totalSize  The total capacity of the collection.
 *
 * @return A new HashContents object appropriate for this collection type.
 */
HashContents *BagClass::allocateContents(size_t bucketSize, size_t totalSize)
{
    return new (totalSize) MultiValueContents(bucketSize, totalSize);
}


/**
 * Create a new set instance from Rexx code.
 *
 * @param args     The new arguments.
 * @param argCount The count of new arguments.
 *
 * @return The constructed instance.
 */
RexxObject *BagClass::newRexx(RexxObject **args, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // create the new identity table item (this version does not have a backing contents yet).
    Protected<BagClass> temp = new BagClass(true);
    // finish setting this up.
    classThis->completeNewObject(temp, args, argCount);

    // make sure this has been completely initialized
    temp->initialize();
    return temp;
}


/**
 * Create a list bag and populate with the argument items.
 *
 * @param args     Pointer to the arguments.
 * @param argCount The number of arguments.
 *
 * @return The populated list object.
 */
BagClass *BagClass::ofRexx(RexxObject **args, size_t  argCount)
{
    // create a list item of the appopriate type.
    Protected<BagClass> newBag = (BagClass *)newRexx(NULL, 0);

    // add all of the arguments
    for (size_t i = 0; i < argCount; i++)
    {
        RexxObject *item = args[i];
        // omitted arguments not allowed here.
        if (item == OREF_NULL)
        {
            reportException(Error_Incorrect_method_noarg, i + 1);
        }
        newBag->put(item);
    }
    return newBag;
}


/**
 * Test for the existence of an item in the Bag, optionally
 * qualified by an index value.
 *
 * @param value  The target value.
 * @param index  The optional index.
 *
 * @return .true if this was found, .false otherwise.
 */
RexxObject *BagClass::hasItemRexx(RexxObject *value, RexxObject *index)
{
    requiredArgument(value, ARG_ONE);

    // index is optional, but if specified, it must be equal to value
    if (index != OREF_NULL && !contents->isIndexEqual(value, index))
    {
        return TheNilObject;
    }

    // hasItem() is the same as hasIndex() for the Bag class
    return booleanObject(hasIndex(value));
}


/**
 * Remove an item from a Bag, optionally qualified with an index value.
 *
 * @param value The target value.
 * @param index The optional index.
 *
 * @return The removed item, if any.
 */
RexxObject *BagClass::removeItemRexx(RexxObject *value, RexxObject *index)
{
    requiredArgument(value, ARG_ONE);

    // index is optional, but if specified, it must be equal to value
    if (index != OREF_NULL && !contents->isIndexEqual(value, index))
    {
        return TheNilObject;
    }

    // removeItem() is the same as remove() for the Bag class
    return resultOrNil(remove(value));
}
