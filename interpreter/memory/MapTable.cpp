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
/* A collection that maps objects to integer values.  Used for internal       */
/* memory management.                                                         */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "MapTable.hpp"


/**
 * Allocate a new MapTable item.
 *
 * @param size    The base object size.
 *
 * @return The storage for creating a MapBucket.
 */
void *MapTable::operator new(size_t size)
{
   return new_object(size, T_MapTable);
}


/**
 * Initialize a MapTable object.
 *
 * @param entries The number of entries.
 */
MapTable::MapTable(size_t entries)
{
    // get a new bucket of the correct size
    contents = new (entries) MapBucket(entries);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void MapTable::live(size_t liveMark)
{
    memory_mark(contents);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void MapTable::liveGeneral(MarkReason reason)
{
    memory_mark_general(contents);
}


/**
 * Copy a map table.
 *
 * @return The new maptable object.
 */
RexxInternalObject *MapTable::copy()
{
    // copy this object first
    MapTable *newObj = (MapTable *)RexxInternalObject::copy();
    newObj->contents = (MapBucket *)contents->copy();
    return newObj;
}


/**
 * Place an item into a hash collection using a key.
 *
 * @param value The inserted value.
 * @param index The insertion key.
 *
 * @return The retrieved object.  Returns OREF_NULL if the object
 *         was not found.
 */
void MapTable::put(size_t value, RexxInternalObject *index)
{
    // try to insert in the existing hash tables...if this
    // fails, we're full and need to reallocate.
    if (!contents->put(value, index))
    {
        // reallocate and try again
        reallocateContents();
        contents->put(value, index);
    }
}


/**
 * Increment the value associated with a key.  If the key does
 * not exist, it is inserted into the table with a value of 1.
 *
 * @param key    The target key.
 */
void MapTable::increment(RexxInternalObject *index)
{
    // try to insert in the existing hash tables...if this
    // fails, we're full and need to reallocate.
    if (!contents->increment(index))
    {
        // reallocate and try again
        reallocateContents();
        contents->increment(index);
    }
}


/**
 * Reallocate the hash bucket to a larger version after
 * a failed put() operation.
 */
void MapTable::reallocateContents()
{
    // create a new bucket and merge the old bucket into it, then replace the contents
    // with the new ones.
    MapBucket *newContents = new (contents->totalSize * 2) MapBucket(contents->totalSize * 2);
    contents->merge(newContents);
    contents = newContents;
}
