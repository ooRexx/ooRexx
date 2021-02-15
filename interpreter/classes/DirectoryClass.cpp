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
/* Primitive Directory Class                                                  */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "MethodClass.hpp"
#include "RexxActivation.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"
#include "SupplierClass.hpp"

// singleton class instance
RexxClass *DirectoryClass::classInstance = OREF_NULL;

/**
 * Create initial class object at bootstrap time.
 */
void DirectoryClass::createInstance()
{
    CLASS_CREATE(Directory);
}


/**
 * Create a new directory object.
 *
 * @param size   the size of the object.
 *
 * @return Storage for creating a the object.
 */
void *DirectoryClass::operator new (size_t size)
{
    return new_object(size, T_Directory);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void DirectoryClass::live(size_t liveMark)
{
    memory_mark(contents);
    memory_mark(methodTable);
    memory_mark(unknownMethod);
    memory_mark(objectVariables);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void DirectoryClass::liveGeneral(MarkReason reason)
{
    memory_mark_general(contents);
    memory_mark_general(methodTable);
    memory_mark_general(unknownMethod);
    memory_mark_general(objectVariables);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void DirectoryClass::flatten(Envelope *envelope)
{
    setUpFlatten(DirectoryClass)

      flattenRef(contents);
      flattenRef(methodTable);
      flattenRef(unknownMethod);
      flattenRef(objectVariables);

    cleanUpFlatten
}


/**
 * Make a proxy object for one of the special directories (e.g.,
 * the environment directory)
 *
 * @param envelope The envelope we're flattening into.
 *
 * @return A string proxy name for this object.
 */
RexxObject *DirectoryClass::makeProxy(Envelope *envelope)
{
    // the environment directory is the only one that needs this treatment
    if (this == TheEnvironment)
    {
        return new_proxy("ENVIRONMENT");
    }
    else
    {
        Interpreter::logicError("Don't know how to generate a proxy object for an object");
    }
    return OREF_NULL;
}


/**
 * Copy a directory item.
 *
 * @return A copy of the item.
 */
RexxInternalObject *DirectoryClass::copy()
{
    // first copy via the superclass copy method
    Protected<DirectoryClass> newObj = (DirectoryClass *)HashCollection::copy();
    // if we have a method table, make a copy of that too.
    if (methodTable != OREF_NULL)
    {
        newObj->methodTable = (StringTable *)methodTable->copy();
    }
    return newObj;
}


/**
 * Return the count of items in the directory, including the
 * number of methods added via set method calls
 *
 * @return the count of items.
 */
size_t DirectoryClass::items()
{
    // get the direct table size
    size_t count = contents->items();
    // if we have a method table, add in its count as well.
    if (methodTable != OREF_NULL)
    {
        count += methodTable->items();
    }
    return count;
}

/**
 *
 * Create a supplier for a directory, including the results of all
 * of the SETMETHOD methods as values
 *
 * @return An appropriate supplier object.
 */
SupplierClass *DirectoryClass::supplier()
{
    // get the supplier for the base collection.
    Protected<SupplierClass> supplier = contents->supplier();

    // do we have a method table?  We need to include these also, which
    // requires running each of the methods to obtain the value.
    if (methodTable != OREF_NULL)
    {
        Protected<ArrayClass> indexes = new_array(methodTable->items());
        Protected<ArrayClass> values = new_array(methodTable->items());

        HashContents::TableIterator iterator = methodTable->iterator();

        for (; iterator.isAvailable(); iterator.next())
        {
            RexxString *name = (RexxString *)iterator.index();
            MethodClass *method = (MethodClass *)iterator.value();

            ProtectedObject v;
            // run the method, using the directory as the receiver
            method->run(ActivityManager::currentActivity, this, name, NULL, 0, v);

            indexes->append(name);
            values->append(v);
        }
        // append the method table part to the existing supplier
        supplier->append(values, indexes);
    }
    return supplier;
}


/**
 * Create an array of all of the directory indices, including those
 * of all the SETMETHOD methods.
 *
 * @return An array containing all of the directory indices.
 */
ArrayClass *DirectoryClass::allIndexes()
{
    // get the base set
    Protected<ArrayClass> indexes = contents->allIndexes();

    // if we have a method table, we need to append those indices also
    if (methodTable != OREF_NULL)
    {
        indexes->appendAll(methodTable->allIndexes());
    }
    return indexes;
}


/**
 * Create an array of all of the directory values, including the
 * values of all the SETMETHOD methods
 *
 * @return An array of all item values.
 */
ArrayClass *DirectoryClass::allItems()
{
    // get the base set
    Protected<ArrayClass> itemArray = contents->allItems();
    // have a method table? we need to run the methods an append to the result
    if (methodTable != OREF_NULL)
    {
        HashContents::TableIterator iterator = methodTable->iterator();

        for (; iterator.isAvailable(); iterator.next())
        {
            MethodClass *method = (MethodClass *)iterator.value();
            RexxString *name = (RexxString *)iterator.index();

            ProtectedObject v;
            // run the method, using the directory as the receiver
            method->run(ActivityManager::currentActivity, this, name, NULL, 0, v);

            itemArray->append(v);
        }
    }
    return itemArray;
}


/**
 * Determine if the directory has an entry with this name (used
 * without uppercasing).  This is an override of the base
 * virtual method.
 *
 * @param indexName The index name.
 *
 * @return true if the index exists, false otherwise.
 */
bool DirectoryClass::hasIndex(RexxInternalObject *indexName)
{
    // got a value in the main contents?
    if (contents->hasIndex(indexName))
    {
        return true;
    }

    // if we have a method table, check that also
    if (methodTable != OREF_NULL)
    {
        return methodTable->hasIndex(indexName);
    }
    return false;
}


/**
 * Remove an object from a directory.
 *
 * @param entryname The directory index.
 *
 * @return The removed object, if any.
 */
RexxInternalObject *DirectoryClass::remove(RexxInternalObject *entryName)
{
    // the removed value might come from running a method,
    RexxInternalObject *oldVal = get(entryName);

    // remove from the contents (unconditionally)
    contents->remove(entryName);

    // if there is a method table as well, remove this entry from that.
    if (methodTable != OREF_NULL)
    {
        methodTable->remove(entryName);
    }
    return oldVal;
}


/**
 * Retrieve an item from a directory.
 *
 * @param _index The target index.
 *
 * @return The result value, if available.
 */
RexxInternalObject *DirectoryClass::get(RexxInternalObject *index)
{
    // first from the contents
    RexxInternalObject *result = contents->get(index);
    if (result == OREF_NULL)
    {
        // then try the method table
        result = methodTableValue(index);
        // now try the unknown method if we still don't have anything.
        if (result == OREF_NULL)
        {
            result = unknownValue(index);
        }
    }
    return result;
}


/**
 * Override of the PUT method.
 *
 * @param value  The value to put
 * @param index  The index of the target value
 */
void DirectoryClass::put(RexxInternalObject *value, RexxInternalObject *index)
{
    // a PUT replaces any existing value, including methods that may have been defined.
    if (methodTable != OREF_NULL)
    {
        methodTable->remove(index);
    }

    // Just forward to the existing put method.
    return HashCollection::put(value, index);
}


/**
 * Empty a hash table collection.
 */
void DirectoryClass::empty()
{
    // empty the hashtables without reallocating.
    contents->empty();
    if (methodTable != OREF_NULL)
    {
        methodTable->empty();
    }
    // clear out the unknown method.
    setField(unknownMethod, OREF_NULL);
}


/**
 * Retrieve an index for a given item.  Which index is returned
 * is indeterminate.
 *
 * @param target The target object.
 *
 * @return The index for the target object NULL
 */
RexxInternalObject *DirectoryClass::getIndex(RexxInternalObject *target)
{
    // retrieve this from the hash table
    RexxInternalObject *result = contents->getIndex(target);
    // not found, check the other tables
    if (result == OREF_NULL)
    {
        // rats, we might need to do this the hard way
        if (methodTable != OREF_NULL)
        {
            // This is a serious pain.  We need to run the method table
            // calling all of the methods to see if we get a return value
            // that matches our target object.

            HashContents::TableIterator iterator = methodTable->iterator();

            for (; iterator.isAvailable(); iterator.next())
            {
                // we need to run each method, looking for a value that matches
                RexxString *name = (RexxString *)iterator.index();
                MethodClass *method = (MethodClass *)iterator.value();
                ProtectedObject v;
                method->run(ActivityManager::currentActivity, this, name, NULL, 0, v);
                // got a match?
                if (target->equalValue(v))
                {
                    return name;      // the name is the index
                }
            }
        }
    }
    return result;
}


/**
 * Test if a given item exists in the collection.
 *
 * @param target The target object.
 *
 * @return .true if the object exists, .false otherwise.
 */
bool DirectoryClass::hasItem(RexxInternalObject *target)
{
    // the lookup is more complicated, so just delegate to the index lookup code.

    return getIndex(target) != OREF_NULL;
}


/**
 * Remove a given item from the collection.
 *
 * @param target The target object.
 *
 * @return .true if the object exists, .false otherwise.
 */
RexxInternalObject *DirectoryClass::removeItem(RexxInternalObject *target)
{
    // the lookup is more complicated, so just delegate to the index lookup code.
    RexxInternalObject *i = getIndex(target);
    // just use the retrieved index to remove.
    if (i != OREF_NULL)
    {
        return remove(i);
    }
    // nothing removed
    return OREF_NULL;
}


// End of base internal API methods.  Following are the method stubs that
// are used for building the Rexx programmer visible behaviour.  This are
// largely stubs that call the API methods with argument validation and transformation
// of return values added.




/**
 * Set a method of executable code into a directory object.
 *
 * @param entryname The name this is set under
 * @param methodobj The associated method object.
 *
 * @return Nothing.
 */
RexxInternalObject *DirectoryClass::setMethodRexx(RexxString *name, MethodClass *methodobj)
{
    Protected<RexxString> entryname = stringArgument(name, "index")->upper();
    if (methodobj != OREF_NULL)
    {
        // make sure we have a method object for this.  The scope is .nil to indicate object scope.
        Protected<MethodClass> method = MethodClass::newMethodObject(entryname, methodobj, (RexxClass *)TheNilObject, "method");

        // the unknown method?  We keep that in a special place
        if (entryname->strCompare(GlobalNames::UNKNOWN))
        {
            setField(unknownMethod, method);
        }
        else
        {
            // create the table if this is the first addition
            if (methodTable == OREF_NULL)
            {
                setField(methodTable, new_string_table());
            }
            // and add the method to the table
            methodTable->put(method, entryname);
        }
    }
    // no method object given, this is a removal
    else
    {
        // if unknown, remove this from the special place.
        if (entryname->strCompare(GlobalNames::UNKNOWN))
        {
            clearField(unknownMethod);
        }
        else
        {
            // remove from the method table if we have one
            if (methodTable != OREF_NULL)
            {
                methodTable->remove(entryname);
            }
        }
    }

    // since setting a method will also override any contents, we need to
    // ensure the contents don't have this either.
    contents->remove(entryname);
    return OREF_NULL;
}


/**
 * unset a method of executable code into a directory object.
 *
 * @param entryname The name this is set under
 *
 * @return Nothing.
 */
RexxInternalObject *DirectoryClass::unsetMethodRexx(RexxString *entryname)
{
    // the entry name is always upper case
    entryname = stringArgument(entryname, "index")->upper();

    // if unknown, remove this from the special place.
    if (entryname->strCompare(GlobalNames::UNKNOWN))
    {
        clearField(unknownMethod);
    }
    else
    {
        // remove from the method table if we have one
        if (methodTable != OREF_NULL)
        {
            methodTable->remove(entryname);
        }
    }

    return OREF_NULL;
}


/**
 * Retrieve a value from the method table, if there is one.
 *
 * @param index  The target index.
 *
 * @return The method table method result, or OREF_NULL if none.
 */
RexxInternalObject *DirectoryClass::methodTableValue(RexxInternalObject *index)
{
    // if we have a method table, look for a method and run it if there is one.
    if (methodTable != OREF_NULL)
    {
        MethodClass *method = (MethodClass *)methodTable->get(index);
        if (method != OREF_NULL)
        {
            ProtectedObject v;
            method->run(ActivityManager::currentActivity, this, (RexxString *)index, NULL, 0, v);
            return v;

        }
    }
    return OREF_NULL;
}


/**
 * Retrieve a value via a set UNKNOWN method, if there is one.
 *
 * @param index  The index value.
 *
 * @return The value returned from an unknown method, if there is one.
 */
RexxInternalObject *DirectoryClass::unknownValue(RexxInternalObject *index)
{
    // if we have an UNKNOWN method, run it and return the result value.
    if (unknownMethod != OREF_NULL)
    {
        ProtectedObject v;
        unknownMethod->run(ActivityManager::currentActivity, this, GlobalNames::UNKNOWN, (RexxObject **)&index, 1, v);
        return v;
    }
    return OREF_NULL;
}


/**
 * Create a directory object from Rexx code.
 *
 * @param init_args The arguments to the NEW method.
 * @param argCount  The argument count.
 *
 * @return A new directory object.
 */
RexxObject *DirectoryClass::newRexx(RexxObject **init_args, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // create the new identity table item (this version does not have a backing contents yet).
    Protected<DirectoryClass> temp = new DirectoryClass(true);
    // finish setting this up.
    classThis->completeNewObject(temp, init_args, argCount);

    // make sure this has been completely initialized
    temp->initialize();
    return temp;
}

