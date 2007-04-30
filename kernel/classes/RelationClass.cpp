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
/* REXX Kernel                                                  RelationClass.c     */
/*                                                                            */
/* Primitive Table Class                                                      */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "ArrayClass.hpp"
#include "RelationClass.hpp"
#include "SupplierClass.hpp"

RexxObject *RexxRelation::removeItem(
  RexxObject *value,                   /* value part of the tuple           */
  RexxObject *index)                   /* item to remove                    */
/******************************************************************************/
/* Function:  Remove an item from a relation using an index                   */
/******************************************************************************/
{
  return this->contents->removeItem(value, index);
}

RexxSupplier *RexxRelation::supplier(
  RexxObject *index )                  /* optional index                    */
/******************************************************************************/
/* Function:  Create a supplier for a relation                                */
/******************************************************************************/
{
  RexxArray  *items;                   /* array of returned items           */
  RexxArray  *indexes;                 /* array of indexes                  */
  LONG i;                              /* loop counter                      */
  LONG size;                           /* size of the array                 */

  if (index == OREF_NULL)              /* no index given?                   */
                                       /* return the entire supplier        */
    return this->contents->supplier();
  else {                               /* partial supplier                  */
                                       /* get all of the items with index   */
    items = this->contents->getAll(index);
    size = items->size();              /* get the array size                */
    indexes = new_array(size);         /* get an index array                */
    for (i = 1; i <= size; i++)        /* loop through the index array      */
                                       /* insert the same index here        */
      indexes->put(index, i);
                                       /* turn this into a supplier         */
    return (RexxSupplier *)new_supplier(items, indexes);
  }
}

RexxObject *RexxRelation::itemsRexx(
     RexxObject *index )               /* optional target index             */
/******************************************************************************/
/* Function:  Return the count of items associated with the given index, or   */
/*            the entire collection if the index is omitted.                  */
/******************************************************************************/
{
  RexxArray *items;                    /* array of returned items           */
  LONG   numEntries;                   /* number of entries                 */
  LONG   tempSize;                     /* array size                        */

  if (index == OREF_NULL) {            /* no index given?                   */
                                       /* return count for everything       */
    numEntries = this->contents->totalEntries();
    return (RexxObject *)new_integer(numEntries);
  }
  else {
                                       /* get all of the items with index   */
    items = this->contents->getAll(index);
    tempSize = items->size();          /* return the array size             */
    return (RexxObject *)new_integer(tempSize);
  }
}

RexxObject *RexxRelation::removeItemRexx(
  RexxObject *value,                   /* value part of the tuple           */
  RexxObject *index)                   /* item to remove                    */
/******************************************************************************/
/* Function:  Remove an item from a relation using an index                   */
/******************************************************************************/
{
  RexxObject *item;                    /* removed item                      */

  required_arg(value, ONE);            /* make sure we have a value         */

  // standard remove form?
  if (index == OREF_NULL)
  {
      item = this->contents->removeItem(value);
  }
  else    // multi-item form
  {
      item = this->contents->removeItem(value, index);
  }
  if (item == OREF_NULL)               /* If nothing found, give back .nil  */
    item = TheNilObject;               /* (never return OREF_NULL to REXX)  */
  return item;                         /* return removed value              */
}

RexxObject *RexxRelation::hasItem(
  RexxObject *value,                   /* value part of the tuple           */
  RexxObject *index)                   /* item to remove                    */
/******************************************************************************/
/* Function:  Remove an item from a relation using an index                   */
/******************************************************************************/
{
    required_arg(value, ONE);            /* make sure we have a value         */
    if (index == OREF_NULL)              // just an item search
    {
        return this->contents->hasItem(value);
    }
    else   // tuple search
    {
        return this->contents->hasItem(value, index);
    }
}

RexxObject *RexxRelation::allIndex(
  RexxObject *value)                   /* value to get                      */
/******************************************************************************/
/* Function:  return all indices with the same value                          */
/******************************************************************************/
{
  required_arg(value, ONE);            /* make sure we have a value         */
                                       /* just get from the hash table      */
  return this->contents->allIndex(value);
}

RexxObject *RexxRelation::allAt(
  RexxObject *index)                   /* index to get                      */
/******************************************************************************/
/* Function:  return all values with the same index                           */
/******************************************************************************/
{
  required_arg(index, ONE);            /* make sure we have an index        */
                                       /* just get from the hash table      */
  return this->contents->allIndex(index);
}


RexxObject *RexxRelation::put(
  RexxObject *value,                   /* new value to add                  */
  RexxObject *index)                   /* index for insertion               */
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
  newHash = this->contents->add(value, index);
  if (newHash != OREF_NULL)            /* have a reallocation occur?        */
                                       /* hook on the new hash table        */
    OrefSet(this, this->contents, newHash);
  return OREF_NULL;                    /* never returns anything            */
}

RexxObject *RexxRelation::newRexx(
     RexxObject **init_args,           /* subclass init arguments           */
     size_t       argCount)            /* the number of arguments           */
/******************************************************************************/
/* Function:  Create an instance of a relation                                */
/******************************************************************************/
{
  RexxRelation *newObject;             /* newly created queue object        */

  newObject = new_relation();          /* get a new relation                */
                                       /* object parse_assignment behaviour */
  BehaviourSet(newObject, ((RexxClass *)this)->instanceBehaviour);
                                       /* Initialize the new list instance  */
  newObject->sendMessage(OREF_INIT, init_args, argCount);
  return newObject;                    /* return the new object             */
}

RexxRelation *RexxMemory::newRelation()
/******************************************************************************/
/* Function:  Create a new relation item                                      */
/******************************************************************************/
{
  RexxRelation *newTable;

                                       /* Get new object                    */
                                       /* get a new object and hash         */
  newTable = (RexxRelation *)new_hashCollection(DEFAULT_HASH_SIZE, sizeof(RexxRelation));
                                       /* Give new object its behaviour     */
  BehaviourSet(newTable, TheRelationBehaviour);
                                       /* set the virtual function table    */
  setVirtualFunctions(newTable, T_relation);
                                       /* use the default hash setting      */
  newTable->hashvalue = HASHOREF(newTable);
  return newTable;                     /* return the new object             */
}

