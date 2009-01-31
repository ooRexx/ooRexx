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
/* REXX Kernel                                            RelationClass.c     */
/*                                                                            */
/* Primitive Table Class                                                      */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "ArrayClass.hpp"
#include "RelationClass.hpp"
#include "SupplierClass.hpp"
#include "ProtectedObject.hpp"

// singleton class instance
RexxClass *RexxRelation::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxRelation::createInstance()
{
    CLASS_CREATE(Relation, "Relation", RexxClass);
}


RexxObject *RexxRelation::removeItem(
  RexxObject *_value,                  /* value part of the tuple           */
  RexxObject *_index)                  /* item to remove                    */
/******************************************************************************/
/* Function:  Remove an item from a relation using an index                   */
/******************************************************************************/
{
    return this->contents->removeItem(_value, _index);
}

RexxSupplier *RexxRelation::supplier(
  RexxObject *_index )                 /* optional index                    */
/******************************************************************************/
/* Function:  Create a supplier for a relation                                */
/******************************************************************************/
{
    RexxArray  *itemArray;               /* array of returned items           */
    RexxArray  *indexArray;              /* array of indexes                  */
    size_t   i;                          /* loop counter                      */
    size_t   size;                       /* size of the array                 */

    if (_index == OREF_NULL)              /* no index given?                   */
    {
        /* return the entire supplier        */
        return this->contents->supplier();
    }
    else
    {                               /* partial supplier                  */
                                    /* get all of the items with index   */
        itemArray = this->contents->getAll(_index);
        size = itemArray->size();          /* get the array size                */
        indexArray = new_array(size);      /* get an index array                */
        for (i = 1; i <= size; i++)        /* loop through the index array      */
        {
            /* insert the same index here        */
            indexArray->put(_index, i);
        }
        /* turn this into a supplier         */
        return(RexxSupplier *)new_supplier(itemArray, indexArray);
    }
}

RexxObject *RexxRelation::itemsRexx(
     RexxObject *_index )               /* optional target index             */
/******************************************************************************/
/* Function:  Return the count of items associated with the given index, or   */
/*            the entire collection if the index is omitted.                  */
/******************************************************************************/
{
    if (_index == OREF_NULL)
    {           /* no index given?                   */
                /* return count for everything       */
        return new_integer(contents->totalEntries());
    }
    else
    {
        // just search and count
        return new_integer(contents->countAll(_index));
    }
}

RexxObject *RexxRelation::removeItemRexx(
  RexxObject *_value,                   /* value part of the tuple           */
  RexxObject *_index)                   /* item to remove                    */
/******************************************************************************/
/* Function:  Remove an item from a relation using an index                   */
/******************************************************************************/
{
    RexxObject *item;                    /* removed item                      */

    requiredArgument(_value, ARG_ONE);            /* make sure we have a value         */

    // standard remove form?
    if (_index == OREF_NULL)
    {
        item = this->contents->removeItem(_value);
    }
    else    // multi-item form
    {
        item = this->contents->removeItem(_value, _index);
    }
    if (item == OREF_NULL)               /* If nothing found, give back .nil  */
    {
        item = TheNilObject;               /* (never return OREF_NULL to REXX)  */
    }
    return item;                         /* return removed value              */
}

RexxObject *RexxRelation::hasItem(
  RexxObject *_value,                   /* value part of the tuple           */
  RexxObject *_index)                   /* item to remove                    */
/******************************************************************************/
/* Function:  Remove an item from a relation using an index                   */
/******************************************************************************/
{
    requiredArgument(_value, ARG_ONE);            /* make sure we have a value         */
    if (_index == OREF_NULL)              // just an item search
    {
        return this->contents->hasItem(_value);
    }
    else   // tuple search
    {
        return this->contents->hasItem(_value, _index);
    }
}

RexxObject *RexxRelation::allIndex(
  RexxObject *_value)                   /* value to get                      */
/******************************************************************************/
/* Function:  return all indices with the same value                          */
/******************************************************************************/
{
    requiredArgument(_value, ARG_ONE);           /* make sure we have a value         */
                                         /* just get from the hash table      */
    return this->contents->allIndex(_value);
}

RexxObject *RexxRelation::allAt(
  RexxObject *_index)                   /* index to get                      */
/******************************************************************************/
/* Function:  return all values with the same index                           */
/******************************************************************************/
{
    requiredArgument(_index, ARG_ONE);           /* make sure we have an index        */
                                       /* just get from the hash table      */
    return this->contents->allIndex(_index);
}


RexxObject *RexxRelation::put(
  RexxObject *_value,                   /* new value to add                  */
  RexxObject *_index)                   /* index for insertion               */
/******************************************************************************/
/* Arguments:  Value, index                                                   */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
    requiredArgument(_value, ARG_ONE);            /* make sure we have an value        */
    requiredArgument(_index, ARG_TWO);            /* make sure we have an index        */
    /* try to place in existing hashtab  */
    RexxHashTable *newHash = this->contents->add(_value, _index);
    if (newHash != OREF_NULL)            /* have a reallocation occur?        */
    {
        /* hook on the new hash table        */
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;                    /* never returns anything            */
}

RexxObject *RexxRelation::newRexx(
     RexxObject **init_args,           /* subclass init arguments           */
     size_t       argCount)            /* the number of arguments           */
/******************************************************************************/
/* Function:  Create an instance of a relation                                */
/******************************************************************************/
{
    RexxRelation *newObj = new_relation();             /* get a new relation                */
    /* object parse_assignment behaviour */
    newObj->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
    /* Initialize the new list instance  */
    newObj->sendMessage(OREF_INIT, init_args, argCount);
    return newObj;                       /* return the new object             */
}

RexxRelation *RexxRelation::newInstance()
/******************************************************************************/
/* Function:  Create a new relation item                                      */
/******************************************************************************/
{
    /* Get new object                    */
    /* get a new object and hash         */
    return(RexxRelation *)new_hashCollection(RexxHashTable::DEFAULT_HASH_SIZE, sizeof(RexxRelation), T_Relation);
}

