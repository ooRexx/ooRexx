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
/* Primitive Supplier Class                                                   */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *SupplierClass::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void SupplierClass::createInstance()
{
    CLASS_CREATE(Supplier, "Supplier", RexxClass);
}


/**
 * Allocate a new supplier object.
 *
 * @param size   The object size
 *
 * @return Storage for creating a supplier object.
 */
void *SupplierClass::operator new(size_t size)
{
    return new_object(size, T_Supplier);
}


/**
 * Constructor for a supplier.
 *
 * @param _values  The array of values.
 * @param _indexes The array of indexes
 */
SupplierClass::SupplierClass(ArrayClass  *_values, ArrayClass  *_indexes )
{
    values = _values;
    indexes = _indexes;
    position = 1;
}


/**
 * No argument constructor for a supplier.
 */
SupplierClass::SupplierClass()
{
    values = OREF_NULL;
    indexes = OREF_NULL;
}


void SupplierClass::live(size_t liveMark)
{
    memory_mark(values);
    memory_mark(indexes);
    memory_mark(objectVariables);
}


void SupplierClass::liveGeneral(MarkReason reason)
{
    memory_mark_general(values);
    memory_mark_general(indexes);
    memory_mark_general(objectVariables);
}

void SupplierClass::flatten(Envelope *envelope)
{
   setUpFlatten(SupplierClass)

   flattenRef(values);
   flattenRef(indexes);
   flattenRef(objectVariables);

   cleanUpFlatten
}


/**
 * Test if the supplier has a next item available.
 *
 * @return True if there are still objects to supply, false otherwise.
 */
RexxObject *SupplierClass::available()
{
    // test if we have an available next item
    return booleanObject(isAvailable());
}


/**
 * Test if the supplier has a next item available.
 *
 * @return True if there are still objects to supply, false otherwise.
 */
bool SupplierClass::isAvailable()
{
    // test if we have an available next item
    return (position <= values->size());
}


/**
 * Step to the next available item
 *
 * @return Returns NOTHING
 */
RexxObject  *SupplierClass::next()
{
    // it is an error to ask for next after we've hit the end.
    if (position > values->size())
    {
        reportException(Error_Incorrect_method_supplier);
    }
    // step the position
    position++;
    return OREF_NULL;
}


/**
 * Retrieve the value portion of the pair.
 *
 * @return The associated value.
 */
RexxInternalObject *SupplierClass::value()
{
    // already gone past the end the end is an error
    if (position > values->size())
    {
        reportException(Error_Incorrect_method_supplier);
    }

    // get the value, but make sure we at least return .nil
    return resultOrNil(values->get(position));
}


/**
 * Retrieve the index of the current position.
 *
 * @return The position index.
 */
RexxInternalObject *SupplierClass::index()
{
    // past the end if an error
    if (position > values->size())
    {
        reportException(Error_Incorrect_method_supplier);
    }
    // the index array is optional...if we don't have it, just give
    // the numeric position
    if (indexes == OREF_NULL)
    {
        return new_integer(position);
    }

    // already gone past the end of the index array?
    if (position > indexes->size())
    {
        return TheNilObject;
    }
    else
    {
        // get the current value and return .nil if nothing is there.
        return resultOrNil(indexes->get(position));
    }
}


/**
 * Supplier initializer for suppliers created via
 * .supplier~new(values, indexes).
 *
 * @param values  The values array object
 * @param indexes The indexes array object
 *
 * @return Nothing
 */
RexxObject *SupplierClass::initRexx(ArrayClass *_values, ArrayClass *_indexes)
{
    ArrayClass *new_values = arrayArgument(_values, ARG_ONE);           // both values are required
    ArrayClass *new_indexes = arrayArgument(_indexes, ARG_TWO);

    // technically, we could probably directly assign these since this really is a constructor,
    // but it doesn't hurt to use these here.
    setField(values, new_values);
    setField(indexes, new_indexes);
    position = 1;
    return OREF_NULL;
}


/**
 * Append a additional items to this supplier object.
 *
 * @param _values  The additional values to append.
 * @param _indexes The additional indexes to append.
 */
void SupplierClass::append(ArrayClass  *_values, ArrayClass  *_indexes )
{
    values->appendAll(_values);
    indexes->appendAll(_indexes);
}


/**
 * Append the contents of another supplier to this one.
 *
 * @param s      The source supplier.
 */
void SupplierClass::append(SupplierClass *s)
{
    append(s->getValues(), s->getIndexes());
}


/**
 * Create a new supplier from Rexx code.
 *
 * @param init_args The arguments passed to the new method.
 * @param argCount  The count of arguments.
 *
 * @return A new supplier object.
 */
RexxObject  *SupplierClass::newRexx(RexxObject **init_args, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxObject *newObj = new SupplierClass();
    ProtectedObject p(newObj);

    // handle Rexx class completion
    classThis->completeNewObject(newObj, init_args, argCount);

    return newObj;
}
