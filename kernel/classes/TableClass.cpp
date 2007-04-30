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
/****************************************************************************/
/* REXX Kernel                                                  TableClass.c   */
/*                                                                          */
/* Primitive Table Class                                                    */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "TableClass.hpp"
#include "RexxActivity.hpp"
#include "RexxNativeAPI.h"

RexxObject *RexxTable::addOffset(
  RexxObject *value,                   /* object to add                     */
  RexxObject *index)                   /* added index                       */
/******************************************************************************/
/* Function:  This add method is used by the envelope packing/copybuffer      */
/*  processing.  it is used to maintain the nodupTable.  The value argument   */
/*  is not really an OREF but rather a offSet into the smartBuffer.           */
/*  This routine will do the same function as Add except that we won't verify */
/*    any arguments and f we need to grow the hashtab we mark it as           */
/*    having to references.  This is needed to so that we don't try and Mark  */
/*    (Collect) the offset values.                                            */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* newly created hash table          */

  memoryObject.disableOrefChecks();    /* Turn off OrefSet Checking.        */
                                       /* try to place in existing hashtab  */
  newHash = this->contents->primitiveAdd(value, index);
  if (newHash  != OREF_NULL) {         /* have a reallocation occur?        */
                                       /* mark the hash as not having refere*/
                                       /* even though the indices are objs  */
                                       /* we don't need to mark this Hash.  */
                                       /* Trust me !!!                      */
    SetObjectHasNoReferences(newHash);
                                       /* hook on the new hash table        */
    OrefSet(this, this->contents, newHash);
  }
  memoryObject.enableOrefChecks();     /* Turn OrefSet Checking.            */
  return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxTable::stringAdd(
  RexxObject *value,                   /* object to add                     */
  RexxString *index)                   /* added index                       */
/******************************************************************************/
/* Function:  Add an object to a table using a string index.                  */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* newly created hash table          */

  required_arg(value, ONE);            /* make sure we have an value        */
  required_arg(index, TWO);            /* make sure we have an index        */
                                       /* try to place in existing hashtab  */
  newHash = this->contents->stringAdd(value, index);
  if (newHash != OREF_NULL)            /* have a reallocation occur?        */
                                       /* hook on the new hash table        */
    OrefSet(this, this->contents, newHash);
  return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxTable::stringPut(
  RexxObject *value,                   /* value to insert                   */
  RexxString *index)                   /* item index                        */
/******************************************************************************/
/* Function:  Put an object into the table using a string index               */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* newly created hash table          */

  required_arg(value, ONE);            /* make sure we have an value        */
  required_arg(index, TWO);            /* make sure we have an index        */
                                       /* try to place in existing hashtab  */
  newHash = this->contents->stringPut(value, index);
  if (newHash  != OREF_NULL)           /* have a reallocation occur?        */
                                       /* hook on the new hash table        */
    OrefSet(this, this->contents, newHash);
  return OREF_NULL;                    /* always return nothing             */
}

RexxArray  *RexxTable::requestArray()
/******************************************************************************/
/* Function:  Primitive level request('ARRAY') fast path                      */
/******************************************************************************/
{
  if (OTYPE(Table, this))              /* primitive level object?           */
    return this->makeArray();          /* just do the makearray             */
  else                                 /* need to so full request mechanism */
    return (RexxArray *)send_message1(this, OREF_REQUEST, OREF_ARRAYSYM);
}

RexxObject *RexxTable::itemsRexx(void)
/******************************************************************************/
/* Function:  Return the count of items in the table                          */
/******************************************************************************/
{
  LONG  numEntries;

  numEntries = this->contents->totalEntries();
  return (RexxObject *)new_integer(numEntries);
}

void RexxTable::reset()
/******************************************************************************/
/* Function:  Reset a table by clearing out the old contents table            */
/******************************************************************************/
{
  OrefSet(this, this->contents, (RexxHashTable *)new_hashtab(DEFAULT_HASH_SIZE));
}

RexxObject *RexxTable::putNodupe(RexxObject *value, RexxObject *index)
/******************************************************************************/
/* Function:  Put an object into a table, ensuring no duplicates.             */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* newly created hash table          */

                                       /* try to place in existing hashtab  */
  newHash = this->contents->putNodupe(value, index);
  if (newHash  != OREF_NULL)           /* have a reallocation occur?        */
                                       /* hook on the new hash table        */
    OrefSet(this, this->contents, newHash);
  return OREF_NULL;
}

void RexxTable::reHash(void)
/******************************************************************************/
/* Function:  Create an instance of a table                                   */
/******************************************************************************/
{
   OrefSet(this, this->contents, this->contents->reHash());
}

RexxObject *RexxTable::newRexx(
    RexxObject **args,                 /* subclass init arguments           */
    size_t argCount)                   /* count of arguments                */
/******************************************************************************/
/* Function:  Create an instance of a table                                   */
/******************************************************************************/
{
  RexxTable * newObject;               /* newly created table object        */

  newObject = new_table();             /* get a new table                   */
  BehaviourSet(newObject, ((RexxClass *)this)->instanceBehaviour);
                                       /* does object have an UNINT method  */
  if (((RexxClass *)this)->uninitDefined()) {
     newObject->hasUninit();           /* Make sure everyone is notified.   */
  }
                                       /* call any rexx level init's        */
  newObject->sendMessage(OREF_INIT, args, argCount);
  return newObject;                    /* return the new object             */
}

RexxTable *RexxMemory::newTable()
/******************************************************************************/
/* Function:  Create an instance of a table                                   */
/******************************************************************************/
{
  RexxTable *newTable;                 /* new object created                */

                                       /* get a new object                  */
  newTable = (RexxTable *)new_hashCollection(DEFAULT_HASH_SIZE, sizeof(RexxTable));
                                       /* set the new behaviour             */
  BehaviourSet(newTable, TheTableBehaviour);
                                       /* set the virtual function table    */
  setVirtualFunctions(newTable, T_table);
                                       /* fill in the hash value            */
  newTable->hashvalue = HASHOREF(newTable);
                                       /* create the initial hash table     */
  return newTable;                     /* return the new table              */
}

RexxObjectTable *RexxMemory::newObjectTable(size_t size)
/******************************************************************************/
/* Function:  Create an instance of an object table                           */
/******************************************************************************/
{
  RexxObjectTable *newTable;           /* new object created                */

                                       /* get a new object                  */
  newTable = (RexxObjectTable *)new_hashCollection(size, sizeof(RexxObjectTable));
                                       /* set the new behaviour             */
  BehaviourSet(newTable, TheTableBehaviour);
                                       /* set the virtual function table    */
  setVirtualFunctions(newTable, T_table);
                                       /* fill in the hash value            */
  newTable->hashvalue = HASHOREF(newTable);
                                       /* create the initial hash table     */
  return newTable;                     /* return the new table              */
}

RexxObject *RexxObjectTable::put(
  RexxObject *value,                   /* value to insert                   */
  RexxObject *index)                   /* item index                        */
/******************************************************************************/
/* Function:  Do a put operation into a primitive object table                */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* newly created hash table          */

                                       /* try to place in existing hashtab  */
  newHash = this->contents->primitivePut(value, index);
  if (newHash != OREF_NULL)            /* have a reallocation occur?        */
                                       /* hook on the new hash table        */
    OrefSet(this, this->contents, newHash);
  return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxObjectTable::add(
  RexxObject *value,                   /* object to add                     */
  RexxObject *index)                   /* added index                       */
/******************************************************************************/
/* Function:  do an add operation into a primitive object table               */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* newly created hash table          */

                                       /* try to place in existing hashtab  */
  newHash  = this->contents->primitiveAdd(value, index);
  if (newHash  != OREF_NULL)           /* have a reallocation occur?        */
                                       /* hook on the new hash table        */
    OrefSet(this, this->contents, newHash);
  return OREF_NULL;                    /* always return nothing             */
}

#define this ((RexxTable *)self)

/* ========================================================================== */
/* ===                                                                    === */
/* ========================================================================== */


native2 (REXXOBJECT, TABLE_ADD, REXXOBJECT, object, REXXOBJECT, index)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(this->add((RexxObject *)object, (RexxObject *)index));
}

native1 (REXXOBJECT, TABLE_REMOVE, REXXOBJECT, index)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(this->remove((RexxObject *)index));
}

native1 (REXXOBJECT, TABLE_GET, REXXOBJECT, index)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(this->get((RexxObject *)index));
}

