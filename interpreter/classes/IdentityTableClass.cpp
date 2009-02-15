/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* REXX Kernel                                       IdentityTableClass.cpp */
/*                                                                          */
/* Primitive IdentityTable Class                                            */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "TableClass.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"

// singleton class instance
RexxClass *RexxIdentityTable::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxIdentityTable::createInstance()
{
    CLASS_CREATE(IdentityTable, "IdentityTable", RexxClass);
}


/**
 * Create a new identity table instance.
 *
 * @param args     The new arguments.
 * @param argCount The count of new arguments.
 *
 * @return The constructed instance.
 */
RexxObject *RexxIdentityTable::newRexx(RexxObject **args, size_t argCount)
{
    RexxIdentityTable *newObj = new_identity_table();
    newObj->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
    /* does object have an UNINT method  */
    if (((RexxClass *)this)->hasUninitDefined())
    {
        newObj->hasUninit();              /* Make sure everyone is notified.   */
    }
    /* call any rexx level init's        */
    newObj->sendMessage(OREF_INIT, args, argCount);
    return newObj;                       /* return the new object             */
}


/**
 * Remove an object from an IdentityTable
 *
 * @param key    The key of the object to remove
 *
 * @return The removed object (if any).
 */
RexxObject *RexxIdentityTable::remove(RexxObject *key)
{
    return this->contents->primitiveRemove(key);
}


/**
 * Retrieve an object from the table using identity
 * semantics.  This is an override for the base
 * collection get method.
 *
 * @param key    The target index.
 *
 * @return The retrieved object.  Returns OREF_NULL if the key is
 *         not found.
 */
RexxObject *RexxIdentityTable::get(RexxObject *key)
{
    return this->contents->primitiveGet(key);
}


/**
 * Create a new instance of an identity table.
 *
 * @param size   The initial table capacity.
 *
 * @return The newly created table.
 */
RexxIdentityTable *RexxIdentityTable::newInstance(size_t size)
{
    return (RexxIdentityTable *)new_hashCollection(size, sizeof(RexxIdentityTable), T_IdentityTable);
}


/**
 * Virtual override for the default hash collection put()
 * operation.
 *
 * @param _value The value to insert.
 * @param _index The target index.
 *
 * @return Always returns OREF_NULL
 */
RexxObject *RexxIdentityTable::put(RexxObject *_value, RexxObject *_index)
{
    /* try to place in existing hashtab  */
    RexxHashTable *newHash = this->contents->primitivePut(_value, _index);
    if (newHash != OREF_NULL)            /* have a reallocation occur?        */
    {
        /* hook on the new hash table        */
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;                    /* always return nothing             */
}


/**
 * Override for the default hash collection add()
 * method.
 *
 * @param _value The value to insert.
 * @param _index The index this will be stored under.
 *
 * @return Always returns OREF_NULL.
 */
RexxObject *RexxIdentityTable::add(RexxObject *_value, RexxObject *_index)
{
    /* try to place in existing hashtab  */
    RexxHashTable *newHash  = this->contents->primitiveAdd(_value, _index);
    if (newHash  != OREF_NULL)           /* have a reallocation occur?        */
    {
        /* hook on the new hash table        */
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;                    /* always return nothing             */
}


/**
 * Remove an item specified by value.
 *
 * @param target The target object.
 *
 * @return The target object again.
 */
RexxObject *RexxIdentityTable::removeItem(RexxObject *target)
{
    // the contents handle all of this.
    return this->contents->primitiveRemoveItem(target);
}


/**
 * Test if a given item exists in the collection.
 *
 * @param target The target object.
 *
 * @return .true if the object exists, .false otherwise.
 */
RexxObject *RexxIdentityTable::hasItem(RexxObject *target)
{
    return this->contents->primitiveHasItem(target);
}


/**
 * Retrieve an index for a given item.  Which index is returned
 * is indeterminate.
 *
 * @param target The target object.
 *
 * @return The index for the target object, or .nil if no object was
 *         found.
 */
RexxObject *RexxIdentityTable::getIndex(RexxObject *target)
{
    // retrieve this from the hash table
    return contents->primitiveGetIndex(target);
}
