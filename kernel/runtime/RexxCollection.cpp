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
/* REXX Kernel                                                  RexxCollection.c      */
/*                                                                            */
/* Primitive HashTableCollection Class                                        */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "RexxCollection.hpp"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"

void RexxHashTableCollection::live(void)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->contents);
  memory_mark(this->objectVariables);
  cleanUpMemoryMark
}

void RexxHashTableCollection::liveGeneral(void)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->contents);
  memory_mark_general(this->objectVariables);
  cleanUpMemoryMarkGeneral
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

  RexxObject *newProxy;
  char *proxyCode;

                                       /* Determine which special object    */
                                       /* this is and compute code for it.  */
  if (this == TheEnvironment)          /* the environment directory         */
     proxyCode = CHAR_ENVIRONMENT;
  else if (this == TheKernel)          /* the kernel directory              */
     proxyCode = CHAR_KERNEL;
  else if (this == TheSystem)          /* the system directory              */
     proxyCode = CHAR_SYSTEM;
  else
     logic_error("Don't know how to generate a proxy object for an object");

                                       /* Generate a new proxy object to    */
                                       /*represent this object, and then    */
                                       /*send flatten to this proxy.        */
  newProxy = (RexxObject *)new_proxy(proxyCode);
                                       /*  We then return the result        */
                                       /*  from the proxt flatten method.   */
  return newProxy;
}


RexxObject *RexxHashTableCollection::copy(void)
/******************************************************************************/
/* Function:  Copy a hash based collection object                             */
/******************************************************************************/
{
  RexxHashTableCollection *newObject;
                                       /* make a copy of ourself (this also */
                                       /* copies the object variables       */
  newObject = (RexxHashTableCollection *)this->RexxObject::copy();
                                       /* fill in the hash value            */
  newObject->hashvalue = HASHOREF(newObject);
                                       /* We copy the Hash table as well    */
  OrefSet(newObject, newObject->contents, (RexxHashTable *)this->contents->copy());
  return newObject;                    /* return the new object             */
}

RexxArray *RexxHashTableCollection::makeArray(void)
/******************************************************************************/
/* Function:  Return all of the collection indices in an array                */
/******************************************************************************/
{
  return this->contents->makeArray();
}

RexxObject *RexxHashTableCollection::mergeItem(RexxObject *value, RexxObject *index)
/******************************************************************************/
/* Arguments:  Value, index                                                   */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
    // put this in with duplicate protection
    RexxHashTable *newHash = this->contents->putNodupe(value, index);
    // the put can expand, so protect against that
    if (newHash != OREF_NULL)
    {
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;
}

RexxObject *RexxHashTableCollection::removeRexx(
  RexxObject *index)                   /* removed item index                */
/******************************************************************************/
/* Arguments:  Index                                                          */
/*                                                                            */
/*  Returned:  Value, or null if no entry found                               */
/******************************************************************************/
{
  RexxObject *RemovedItem;

  required_arg(index, ONE);            /* make sure we have an index        */

  RemovedItem = this->remove(index);   /* remove the item                   */
  if (RemovedItem == OREF_NULL)        /* If nothing found, give back .nil  */
                                       /* (never return OREF_NULL to REXX)  */
    RemovedItem = (RexxObject *)TheNilObject;
  return RemovedItem;                  /* return removed value              */
}

RexxObject *RexxHashTableCollection::allAt(
  RexxObject *index)                   /* target index                      */
/******************************************************************************/
/* Arguments:  Index                                                          */
/*                                                                            */
/*  Returned:  Array of all values with the same index                        */
/******************************************************************************/
{
  required_arg(index, ONE);            /* make sure we have an index        */
                                       /* do the get                        */
  return this->contents->getAll(index);
}

RexxObject *RexxHashTableCollection::getRexx(
  RexxObject *index)                   /* target index                      */
/******************************************************************************/
/* Arguments:  Index                                                          */
/*                                                                            */
/*  Returned:  Associated value, or nil if index not found                    */
/******************************************************************************/
{
  RexxObject *object;

  required_arg(index, ONE);            /* make sure we have an index        */
  object = this->get(index);           /* get the item                      */
  if (object == OREF_NULL)             /* If nothing found, give back .nil  */
    object = TheNilObject;             /* (never return OREF_NULL to REXX)  */
  return object;                       /* return the item                   */
}

RexxObject *RexxHashTableCollection::put(
  RexxObject *value,                   /* value to insert                   */
  RexxObject *index)                   /* item index                        */
/******************************************************************************/
/* Arguments:  Value, index                                                   */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* newly created hash table          */

  required_arg(value, ONE);            /* make sure we have an value        */
  required_arg(index, TWO);            /* make sure we have an index        */
                                       /* try to place in existing hashtab  */
  newHash = this->contents->put(value, index);
  if (newHash != OREF_NULL)            /* have a reallocation occur?        */
                                       /* hook on the new hash table        */
    OrefSet(this, this->contents, newHash);
  return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxHashTableCollection::add(
  RexxObject *value,                   /* object to add                     */
  RexxObject *index)                   /* added index                       */
/******************************************************************************/
/* Arguments:  Value, index                                                   */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* newly created hash table          */

  required_arg(value, ONE);            /* make sure we have an value        */
  required_arg(index, TWO);            /* make sure we have an index        */
                                       /* try to place in existing hashtab  */
  newHash  = this->contents->add(value, index);
  if (newHash  != OREF_NULL)           /* have a reallocation occur?        */
                                       /* hook on the new hash table        */
    OrefSet(this, this->contents, newHash);
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
   long depth)                         /* depth to propagate the copy to    */
/******************************************************************************/
/* Arguments:  Recursion depth                                                */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  HashLink i;
  RexxObject *value;
  RexxObject *valueCopy;
  RexxHashTable *hashTable;
                                       /* Get hash table.                   */
  hashTable = this->contents;
                                       /* For all indices.                  */
  for (i = hashTable->first(); i < hashTable->totalSlotsSize(); i = hashTable->next(i)) {
    value  = hashTable->value(i);      /* Get this value                    */
    valueCopy = value->copy();         /* make a copy.                      */
    hashTable->replace(valueCopy, i);  /* Replace original w/ copy          */
    if (depth > 1)                     /* gone depth requested.             */
                                       /* nope, copy these values           */
      ((RexxHashTableCollection *)valueCopy)->copyValues(depth-1);
  }

  return OREF_NULL;
}

RexxObject *RexxHashTableCollection::hasIndex(
  RexxObject *index)                   /* requested index                   */
/******************************************************************************/
/* Arguments:  Index                                                          */
/*                                                                            */
/*  Returned:  True if table has an entry for the index, false otherwise      */
/******************************************************************************/
{
  RexxObject *value;

  required_arg(index, ONE);            /* make sure we have an index        */
                                       /* try to get the item               */
  value = this->contents->get(index);
                                       /* tell caller if we succeeded       */
  return (value != OREF_NULL) ? (RexxObject *)TheTrueObject : (RexxObject *)TheFalseObject;
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
    required_arg(target, ONE);
    // retrieve this from the hash table
    RexxObject *result = this->contents->getIndex(target);
    // not found, return .nil
    if (result == OREF_NULL)
    {
        return TheNilObject;
    }
    return result;
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
    // required argument
    required_arg(target, ONE);
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
RexxObject *RexxHashTableCollection::hasItem(RexxObject *target)
{
    required_arg(target, ONE);
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
