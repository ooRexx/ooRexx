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
/* REXX Kernel                                               TableClass.cpp */
/*                                                                          */
/* Primitive Table Class                                                    */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "TableClass.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *RexxTable::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxTable::createInstance()
{
    CLASS_CREATE(Table, "Table", RexxClass);
}

RexxObject *RexxTable::stringAdd(
  RexxObject *_value,                   /* object to add                     */
  RexxString *_index)                   /* added index                       */
/******************************************************************************/
/* Function:  Add an object to a table using a string index.                  */
/******************************************************************************/
{
    requiredArgument(_value, ARG_ONE);            /* make sure we have an value        */
    requiredArgument(_index, ARG_TWO);            /* make sure we have an index        */
    /* try to place in existing hashtab  */
    RexxHashTable *newHash = this->contents->stringAdd(_value, _index);
    if (newHash != OREF_NULL)            /* have a reallocation occur?        */
    {
        /* hook on the new hash table        */
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxTable::stringPut(
  RexxObject *_value,                   /* value to insert                   */
  RexxString *_index)                   /* item index                        */
/******************************************************************************/
/* Function:  Put an object into the table using a string index               */
/******************************************************************************/
{
    requiredArgument(_value, ARG_ONE);            /* make sure we have an value        */
    requiredArgument(_index, ARG_TWO);            /* make sure we have an index        */
    /* try to place in existing hashtab  */
    RexxHashTable *newHash = this->contents->stringPut(_value, _index);
    if (newHash  != OREF_NULL)           /* have a reallocation occur?        */
    {
        /* hook on the new hash table        */
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;                    /* always return nothing             */
}

RexxArray  *RexxTable::requestArray()
/******************************************************************************/
/* Function:  Primitive level request('ARRAY') fast path                      */
/******************************************************************************/
{
    if (isOfClass(Table, this))              /* primitive level object?           */
    {
        return this->makeArray();          /* just do the makearray             */
    }
    else                                 /* need to so full request mechanism */
    {
        return(RexxArray *)this->sendMessage(OREF_REQUEST, OREF_ARRAYSYM);
    }
}

RexxObject *RexxTable::itemsRexx()
/******************************************************************************/
/* Function:  Return the count of items in the table                          */
/******************************************************************************/
{
    size_t numEntries = this->contents->totalEntries();
    return(RexxObject *)new_integer(numEntries);
}

void RexxTable::reset()
/******************************************************************************/
/* Function:  Reset a table by clearing out the old contents table            */
/******************************************************************************/
{
    OrefSet(this, this->contents, new_hashtab(RexxHashTable::DEFAULT_HASH_SIZE));
}

RexxObject *RexxTable::putNodupe(RexxObject *_value, RexxObject *_index)
/******************************************************************************/
/* Function:  Put an object into a table, ensuring no duplicates.             */
/******************************************************************************/
{
    /* try to place in existing hashtab  */
    RexxHashTable *newHash = this->contents->putNodupe(_value, _index);
    if (newHash  != OREF_NULL)           /* have a reallocation occur?        */
    {
        /* hook on the new hash table        */
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;
}

void RexxTable::reHash()
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
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxTable *newObj = new_table();
    ProtectedObject p(newObj);

    // handle Rexx class completion
    classThis->completeNewObject(newObj, args, argCount);

    return newObj;
}

RexxTable *RexxTable::newInstance()
/******************************************************************************/
/* Function:  Create an instance of a table                                   */
/******************************************************************************/
{
    return (RexxTable *)new_hashCollection(RexxHashTable::DEFAULT_HASH_SIZE, sizeof(RexxTable), T_Table);
}


