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
    // TODO:  simplify CLASS_CREATE() by defaulting RexxClass...and possibly generating
    // the both the create method and the static variable initializer.
    CLASS_CREATE(BagClass, "Bag", RexxClass);
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
    return new_object(size, T_Set);
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
 * Create a new table instance from Rexx code.
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

    RexxObject *initialSize;

    // parse the arguments
    RexxClass::processNewArgs(args, argCount, &args, &argCount, 1, (RexxObject **)&initialSize, NULL);

    // the capacity is optional, but must be a positive numeric value
    size_t capacity = optionalLengthArgument(initialSize, DefaultTableSize, ARG_ONE);

    // create the new identity table item
    BagClass *temp = new BagClass(capacity);
    ProtectedObject p(temp);
    // finish setting this up.
    classThis->completeNewObject(temp, args, argCount);
    return temp;
}

