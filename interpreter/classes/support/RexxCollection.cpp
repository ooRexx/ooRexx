/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX Kernel                                          RexxCollection.cpp    */
/*                                                                            */
/* Primitive HashTableCollection Class                                        */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "RexxCollection.hpp"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "Interpreter.hpp"

void RexxHashTableCollection::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->contents);
  memory_mark(this->objectVariables);
}

void RexxHashTableCollection::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->contents);
  memory_mark_general(this->objectVariables);
}

void RexxHashTableCollection::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxHashTableCollection)

      flatten_reference(newThis->contents, envelope);
      flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}

RexxObject *RexxHashTableCollection::unflatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  unflatten an object                                             */
/******************************************************************************/
{
  envelope->addTable(this);
  return this;
}

RexxObject *RexxHashTableCollection::makeProxy(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Create a proxy object for a standard collection                 */
/******************************************************************************/
{
    /* Determine which special object    */
    /* this is and compute code for it.  */
    if (this == TheEnvironment)          /* the environment directory         */
    {
        return new_proxy(CHAR_ENVIRONMENT);
    }
    else if (this == TheKernel)          /* the kernel directory              */
    {
        return new_proxy(CHAR_KERNEL);
    }
    else if (this == TheSystem)          /* the system directory              */
    {
        return new_proxy(CHAR_SYSTEM);
    }
    else
    {
        Interpreter::logicError("Don't know how to generate a proxy object for an object");
    }
    return OREF_NULL;
}


RexxObject *RexxHashTableCollection::copy(void)
/******************************************************************************/
/* Function:  Copy a hash based collection object                             */
/******************************************************************************/
{
    /* make a copy of ourself (this also */
    /* copies the object variables       */
    RexxHashTableCollection *newObj = (RexxHashTableCollection *)this->RexxObject::copy();
    /* We copy the Hash table as well    */
    OrefSet(newObj, newObj->contents, (RexxHashTable *)this->contents->copy());
    return newObj;                       /* return the new object             */
}

RexxArray *RexxHashTableCollection::makeArray(void)
/******************************************************************************/
/* Function:  Return all of the collection indices in an array                */
/******************************************************************************/
{
    return this->contents->makeArray();
}

RexxObject *RexxHashTableCollection::mergeItem(RexxObject *_value, RexxObject *_index)
/******************************************************************************/
/* Arguments:  Value, index                                                   */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
    // put this in with duplicate protection
    RexxHashTable *newHash = this->contents->putNodupe(_value, _index);
    // the put can expand, so protect against that
    if (newHash != OREF_NULL)
    {
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;
}


/**
 * The exported remove() method for hash collection
 * classes.  This is the Rexx stub method.  The removal
 * operation is delegated to the virtual method defined
 * by the implementing class.
 *
 * @param _index The target removal index.
 *
 * @return The removed object, or .nil if the index was not found.
 */
RexxObject *RexxHashTableCollection::removeRexx(RexxObject *_index)
{
    requiredArgument(_index, ARG_ONE);            /* make sure we have an index        */

    RexxObject *removedItem = this->remove(_index);   /* remove the item                   */
    if (removedItem == OREF_NULL)        /* If nothing found, give back .nil  */
    {
        /* (never return OREF_NULL to REXX)  */
        return TheNilObject;
    }
    return removedItem;                  /* return removed value              */
}


/**
 * Base virtual function for a table remove operation.
 * This applies object equality semantics to the operation.
 *
 * @param _index The object index.
 *
 * @return The removed object.
 */
RexxObject *RexxHashTableCollection::remove(RexxObject *_index)
{
    return this->contents->remove(_index);
}


RexxObject *RexxHashTableCollection::allAt(
  RexxObject *_index)                   /* target index                      */
/******************************************************************************/
/* Arguments:  Index                                                          */
/*                                                                            */
/*  Returned:  Array of all values with the same index                        */
/******************************************************************************/
{
    requiredArgument(_index, ARG_ONE);            /* make sure we have an index        */
                                       /* do the get                        */
    return this->contents->getAll(_index);
}

/**
 * Exported get() accessor for a hash table collection.
 * This delegates to a virtual method defined by the
 * target collection.
 *
 * @param _index The target index.
 *
 * @return The fetched object, or .nil if the index does not
 *         exist in the collection.
 */
RexxObject *RexxHashTableCollection::getRexx(RexxObject *_index)
{
    requiredArgument(_index, ARG_ONE);            /* make sure we have an index        */
    RexxObject *object = this->get(_index);           /* get the item                      */
    if (object == OREF_NULL)             /* If nothing found, give back .nil  */
    {
        return TheNilObject;             /* (never return OREF_NULL to REXX)  */
    }
    return object;                       /* return the item                   */
}


/**
 * Retrieve an item from a hash collection using a key.
 * This is the base virtual implementation, which uses
 * equality semantics for the retrieveal.  Other implementations
 * may override this.
 *
 * @param key    The target key.
 *
 * @return The retrieved object.  Returns OREF_NULL if the object
 *         was not found.
 */
RexxObject *RexxHashTableCollection::get(RexxObject *key)
{
    return this->contents->get(key);
}


/**
 * Exported Rexx method for adding an item to a collection.
 * The put operation is delegated to the implementing
 * class virtual function.
 *
 * @param _value The value to add.
 * @param _index The index for the added item.
 *
 * @return Always returns OREF_NULL.
 */
RexxObject *RexxHashTableCollection::putRexx(RexxObject *_value, RexxObject *_index)
{
    requiredArgument(_value, ARG_ONE);            /* make sure we have an value        */
    requiredArgument(_index, ARG_TWO);            /* make sure we have an index        */
    /* try to place in existing hashtab  */
    return this->put(_value, _index);
}


/**
 * Place an item into a hash collection using a key.
 * This is the base virtual implementation, which uses
 * equality semantics for the retrieveal.  Other implementations
 * may override this.
 *
 * @param _value The inserted value.
 * @param _index The insertion key.
 *
 * @return The retrieved object.  Returns OREF_NULL if the object
 *         was not found.
 */
RexxObject *RexxHashTableCollection::put(RexxObject *_value, RexxObject *_index)
{
    /* try to place in existing hashtab  */
    RexxHashTable *newHash = this->contents->put(_value, _index);
    if (newHash != OREF_NULL)            /* have a reallocation occur?        */
    {
        /* hook on the new hash table        */
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxHashTableCollection::addRexx(
  RexxObject *_value,                   /* object to add                     */
  RexxObject *_index)                   /* added index                       */
/******************************************************************************/
/* Arguments:  Value, index                                                   */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
    requiredArgument(_value, ARG_ONE);            /* make sure we have an value        */
    requiredArgument(_index, ARG_TWO);            /* make sure we have an index        */
    return add(_value, _index);
}

RexxObject *RexxHashTableCollection::add(
  RexxObject *_value,                   /* object to add                     */
  RexxObject *_index)                   /* added index                       */
/******************************************************************************/
/* Arguments:  Value, index                                                   */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
    /* try to place in existing hashtab  */
    RexxHashTable *newHash  = this->contents->add(_value, _index);
    if (newHash  != OREF_NULL)           /* have a reallocation occur?        */
    {
        /* hook on the new hash table        */
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxHashTableCollection::merge(
  RexxHashTableCollection * target)    /* merge "partner" collection        */
/******************************************************************************/
/* Function:  Merge a hash table collection into another similar collection.  */
/******************************************************************************/
{
    return this->contents->merge(target);
}


RexxObject *RexxHashTableCollection::copyValues(
   int  depth)                         /* depth to propagate the copy to    */
/******************************************************************************/
/* Arguments:  Recursion depth                                                */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
    /* Get hash table.                   */
    RexxHashTable *hashTable = this->contents;
    /* For all indices.                  */
    for (HashLink i = hashTable->first(); i < hashTable->totalSlotsSize(); i = hashTable->next(i))
    {
        RexxObject *_value  = hashTable->value(i);     /* Get this value                    */
        RexxObject *valueCopy = _value->copy();        /* make a copy.                      */
        hashTable->replace(valueCopy, i);  /* Replace original w/ copy          */
        if (depth > 1)                     /* gone depth requested.             */
                                           /* nope, copy these values           */
            ((RexxHashTableCollection *)valueCopy)->copyValues(depth-1);
    }

    return OREF_NULL;
}


/**
 * Test for the existence of an index in the collection.
 * This uses the get() virtual function to determine if
 * the item exists.
 *
 * @param _index The target index.
 *
 * @return True if the index exists, false if the index does not
 *         exist.
 */
RexxObject *RexxHashTableCollection::hasIndexRexx(RexxObject *_index)
{
    requiredArgument(_index, ARG_ONE);           /* make sure we have an index        */
                                         /* try to get the item               */
    RexxObject *_value = this->get(_index);
    /* tell caller if we succeeded       */
    return(_value != OREF_NULL) ? (RexxObject *)TheTrueObject : (RexxObject *)TheFalseObject;
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
RexxObject *RexxHashTableCollection::indexRexx(RexxObject *target)
{
    // required argument
    requiredArgument(target, ARG_ONE);
    // retrieve this from the hash table
    RexxObject *result = this->getIndex(target);
    // not found, return .nil
    if (result == OREF_NULL)
    {
        return TheNilObject;
    }
    return result;
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
RexxObject *RexxHashTableCollection::getIndex(RexxObject *target)
{
    // retrieve this from the hash table
    return contents->getIndex(target);
}


/**
 * Exported method to remove an item specified by value.
 *
 * @param target The target object.
 *
 * @return The target object again.
 */
RexxObject *RexxHashTableCollection::removeItemRexx(RexxObject *target)
{
    // required argument
    requiredArgument(target, ARG_ONE);
    // the actual target class may use different semantics for this.
    return this->removeItem(target);
}


/**
 * Remove an item specified by value.
 *
 * @param target The target object.
 *
 * @return The target object again.
 */
RexxObject *RexxHashTableCollection::removeItem(RexxObject *target)
{
    // the contents handle all of this.
    return this->contents->removeItem(target);
}


/**
 * Test if a given item exists in the collection.
 *
 * @param target The target object.
 *
 * @return .true if the object exists, .false otherwise.
 */
RexxObject *RexxHashTableCollection::hasItemRexx(RexxObject *target)
{
    requiredArgument(target, ARG_ONE);
    return this->hasItem(target);
}


/**
 * Test if a given item exists in the collection.
 *
 * @param target The target object.
 *
 * @return .true if the object exists, .false otherwise.
 */
RexxObject *RexxHashTableCollection::hasItem(RexxObject *target)
{
    return this->contents->hasItem(target);
}


RexxSupplier *RexxHashTableCollection::supplier()
/******************************************************************************/
/* Function:  create a table supplier                                         */
/******************************************************************************/
{
                                       /* get the hashtab supplier          */
    return this->contents->supplier();
}

RexxArray *RexxHashTableCollection::allItems()
/******************************************************************************/
/* Function:  retrieve all items of the collection.                           */
/******************************************************************************/
{
    return this->contents->allItems();
}

RexxArray *RexxHashTableCollection::allIndexes()
/******************************************************************************/
/* Function:  retrieve all indexes of the collection.                         */
/******************************************************************************/
{
    return this->contents->allIndexes();
}


/**
 * Empty a hash table collection.
 *
 * @return nothing
 */
RexxObject *RexxHashTableCollection::empty()
{
    contents->empty();
    return OREF_NULL;
}


/**
 * Test if a HashTableCollection is empty.
 *
 * @return
 */
RexxObject *RexxHashTableCollection::isEmpty()
{
    return contents->isEmpty() ? TheTrueObject : TheFalseObject;
}
