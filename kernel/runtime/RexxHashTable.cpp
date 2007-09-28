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
/* REXX Kernel                                                  RexxHashTable.c      */
/*                                                                            */
/* Primitive Hash Table Class                                                 */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxHashTable.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"

/******************************************************************************/
/*                                                                            */
/* NOTE:  The following value for NO_MORE is based on the assumption that all */
/* hash table entry chaining proceeds upward from the lower section of the    */
/* table to the upper table.  That is, offset zero is never used for a next   */
/* element location.  This fact allows us to just issue a ClearObject call    */
/* to initialize the hash table rather than having to explicitly set each     */
/* index pointer to a value such as -1.                                       */
/*                                                                            */
/******************************************************************************/
#define NO_MORE 0
#define NO_LINK 0xffffffff             /* used for traversals               */

                                       /* compare a value item to the value */
                                       /* at the specified position         */
BOOL inline EQUAL_VALUE(RexxObject *value, RexxObject *other)
{
                                       /* true for either direct identity or*/
                                       /* real equality ("==")              */
  return (value == other) || value->isEqual(other);
}

RexxHashTable *RexxMemory::newHashTable(
  size_t entries )                     /* number of entries in the table    */
/******************************************************************************/
/* Function:  Create a new hash table                                         */
/******************************************************************************/
{
  RexxHashTable * newHash;             /* new hash table object             */
  size_t bytes;                        /* size of the allocated object      */
  size_t bucketSize;                   /* size of the bucket                */

  bucketSize = entries / 2;            /* get the size of the bucket from this capacity */
  if (bucketSize % 2 == 0) {           /* if this is even, increase it      */
      bucketSize++;
  }

  entries = bucketSize * 2;            /* double the bucket size for the overflow */

  bytes = sizeof(RexxHashTable) + (sizeof(TABENTRY) * (entries - 1));
                                       /* Get new object                    */
  newHash = (RexxHashTable *)new_object(bytes);
                                       /* Give new object its behaviour     */
  BehaviourSet(newHash, TheHashTableBehaviour);
                                       /* set the virtual function table    */
  setVirtualFunctions(newHash, T_hashtab);
  ClearObject(newHash);                /* clear things out                  */
  newHash->u_size = bucketSize;        /* record the size                   */
  newHash->free = entries - 1;         /* and the first free slot           */
  return newHash;                      /* and return it                     */
}

RexxTable *RexxMemory::newHashCollection(
  size_t entries,                      /* number of entries in the table    */
  size_t companionSize )               /* size of companion "table" object  */
/******************************************************************************/
/* Function:  Create a hash table object and a "companion" table, vdict,      */
/*            directory, or relation object all in one shot.                  */
/*                                                                            */
/* Note:      It is necessary to make several assumptions about the           */
/*            "companion" object.  1)  The size specified is already a        */
/*            multiple of 4.  2)  The companion has an OREF pointing to the   */
/*            hashtab object in the same location as the table class.         */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* new hash table object             */
  RexxTable     *newTable;             /* associated table object           */
  size_t bytes;                        /* size of the allocated object      */
  size_t bucketSize;                   /* size of the bucket                */

  bucketSize = entries / 2;            /* get the size of the bucket from this capacity */
  if (bucketSize % 2 == 0) {           /* if this is even, increase it      */
      bucketSize++;
  }

  entries = bucketSize * 2;            /* double the bucket size for the overflow */
                                       /* Compute size of hash tab object   */
  bytes = roundObjectBoundary(sizeof(RexxHashTable) + (sizeof(TABENTRY) * (entries - 1)));
  /* make sure we've got proper sizes for each of the object parts. */
  companionSize = roundObjectBoundary(companionSize);
                                       /* Get space for two objects         */
  newTable = (RexxTable *)new_object(bytes + companionSize);
  ClearObject(newTable);               /* clear the entire lot              */
                                       /* address the hash table            */
  newHash = (RexxHashTable *)(((PCHAR)newTable) + companionSize);
                                       /* compute total size of the hash    */
                                       /* table (allowing for possible      */
                                       /* over allocation by the memory     */
                                       /* manager                           */
  bytes = ObjectSize(newTable) - companionSize;
  SetUpNewObject((RexxObject *)newHash, bytes); /* make this an object               */
                                       /* reduce the companion size         */
  SetObjectSize(newTable, companionSize);
                                       /* do a dummy new against the hash   */
                                       /* table to get the correct virtual  */
                                       /* function table set up             */
  newHash = new ((void *)newHash) RexxHashTable;
                                       /* Give new object its behaviour     */
  BehaviourSet(newHash, TheHashTableBehaviour);
                                       /* set the virtual function table    */
  setVirtualFunctions(newHash, T_hashtab);
  newHash->u_size = bucketSize;        /* record the size                   */
  newHash->free = entries - 1;         /* and the first free slot           */
                                       /* hook the hash into the companion  */
                                       /* OrefSet is not used, because the  */
                                       /* companion object is not fully set */
  newTable->contents = newHash;        /* up yet (no behaviour)             */
  return newTable;                     /* return the object                 */
}

void RexxHashTable::live(void)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  TABENTRY *ep;                        /* table element pointer             */
  TABENTRY *endp;
                                       /* hash table size                   */
  size_t size = this->totalSlotsSize();

  setUpMemoryMark
                                       /* loop through all of the entries   */
  for (ep = this->entries, endp = ep+size; ep < endp; ep++) {
    if (ep->index != OREF_NULL) {      /* have a value here?                */
      memory_mark(ep->index);          /* mark both the index and the       */
      memory_mark(ep->value);          /* value                             */
    }
  }
  cleanUpMemoryMark
}

void RexxHashTable::liveGeneral(void)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  TABENTRY *ep;                        /* table element pointer             */
                                       /* hash table size                   */
  size_t size = this->totalSlotsSize();

  setUpMemoryMarkGeneral
                                       /* loop through all of the entries   */
  for (ep = this->entries; ep < this->entries+size; ep++) {
    if (ep->index != OREF_NULL) {      /* have a value here?                */
      memory_mark_general(ep->index);  /* mark both the index and the       */
      memory_mark_general(ep->value);  /* value                             */
    }
  }
  cleanUpMemoryMarkGeneral
}

void RexxHashTable::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxHashTable)
  size_t size = this->totalSlotsSize();  /* hash table size                   */

  size_t i;

   for (i=0; i < size ; i++) {
     if (this->entries[i].index != OREF_NULL) {
       flatten_reference(newThis->entries[i].index, envelope);
       flatten_reference(newThis->entries[i].value, envelope);
     }
   }
  cleanUpFlatten
}

RexxHashTable * RexxHashTable::add(
    RexxObject *value,                 /* added element value               */
    RexxObject *index)                 /* added index value                 */
/******************************************************************************/
/* Function:  Add an element to a hash table.  If the table needs to expand,  */
/*            the new table is returned.                                      */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */

  position = hashIndex(index);         /* calculate the hash slot           */
                                       /* if the hash slot is empty         */
  if (this->entries[position].index == OREF_NULL) {
                                       /* fill in both the value and index  */
    OrefSet(this,this->entries[position].value,value);
    OrefSet(this,this->entries[position].index,index);
    return OREF_NULL;                  /* successful  addition              */
  }
                                       /* go insert                         */
  return this->insert(value, index, position, FULL_TABLE);
}

RexxHashTable * RexxHashTable::primitiveAdd(
    RexxObject *value,                 /* added element value               */
    RexxObject *index)                 /* added index value                 */
/******************************************************************************/
/* Function:  Add an element to a hash table.  If the table needs to expand,  */
/*            the new table is returned.                                      */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */

  position = hashPrimitiveIndex(index);   /* calculate the hash slot           */
                                       /* if the hash slot is empty         */
  if (this->entries[position].index == OREF_NULL) {
                                       /* fill in both the value and index  */
    OrefSet(this,this->entries[position].value,value);
    OrefSet(this,this->entries[position].index,index);
    return OREF_NULL;                  /* successful  addition              */
  }
                                       /* go insert                         */
  return this->insert(value, index, position, PRIMITIVE_TABLE);
}


RexxObject *RexxHashTable::remove(
            RexxObject *index)         /* index to remove                   */
/******************************************************************************/
/* Function:  Remove an element from a hash table                             */
/******************************************************************************/
{
  HashLink position;                   /* target hash position              */
  HashLink previous;                   /* previous hash position            */
  HashLink next;                       /* next hash position                */
  RexxObject *removed;                 /* removed item value                */

  position = hashIndex(index);         /* calculate the hash slot           */
  previous = NO_LINK;                  /* no previous slot yet              */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* get a match?                      */
      if (EQUAL_VALUE(index, this->entries[position].index)) {
                                       /* save the current value            */
        removed = this->entries[position].value;
                                       /* get the next pointer              */
        next = this->entries[position].next;
        if (next == NO_MORE) {         /* end of the chain?                 */
                                       /* clear this slot entry             */
          OrefSet(this,this->entries[position].index,OREF_NULL);
          OrefSet(this,this->entries[position].value,OREF_NULL);
          if (previous != NO_LINK)     /* if not the first of the chain     */
          {
              /* IH: In this special case we delete an item from the overhead and
                     therefore might have to increase the free counter, otherwise
                     hash table will be extended unnecessarily !!! */
            if (position > this->free)
                this->free = position;
                                       /* break the link                    */
            this->entries[previous].next = NO_MORE;
          }
        }                              /* non-terminal chain element        */
        else {
                                       /* close up the link                 */
          this->entries[position].next = this->entries[next].next;
                                       /* copy value and index to current   */
          OrefSet(this,this->entries[position].index,this->entries[next].index);
          OrefSet(this,this->entries[position].value,this->entries[next].value);
                                       /* clear the next entry              */
          OrefSet(this,this->entries[next].index,OREF_NULL);
          OrefSet(this,this->entries[next].value,OREF_NULL);
                                       /* set to "pristine" condition       */
          this->entries[next].next = NO_MORE;
          if (this->free < next)       /* new-low water mark?               */
            this->free = next;         /* reset to this point               */
        }
        return removed;                /* return removed element value      */
      }
      else
        previous = position;           /* remember previous member          */
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  return OREF_NULL;                    /* removed item not found            */
}

RexxObject *RexxHashTable::primitiveRemove(
            RexxObject *index)         /* index to remove                   */
/******************************************************************************/
/* Function:  Remove an element from a hash table                             */
/******************************************************************************/
{
  HashLink position;                   /* target hash position              */
  HashLink previous;                   /* previous hash position            */
  HashLink next;                       /* next hash position                */
  RexxObject *removed;                 /* removed item value                */

  position = hashPrimitiveIndex(index);  /* calculate the hash slot           */

  previous = NO_LINK;                  /* no previous slot yet              */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* get a match?                      */
      if (index == this->entries[position].index) {
                                       /* save the current value            */
        removed = this->entries[position].value;
                                       /* get the next pointer              */
        next = this->entries[position].next;
        if (next == NO_MORE) {         /* end of the chain?                 */
                                       /* clear this slot entry             */
          OrefSet(this,this->entries[position].index,OREF_NULL);
          OrefSet(this,this->entries[position].value,OREF_NULL);
          if (previous != NO_LINK)     /* if not the first of the chain     */
          {
              /* IH: In this special case we delete an item from the overhead and
                     therefore might have to increase the free counter, otherwise
                     hash table will be extended unnecessarily !!! */
            if (position > this->free)
                this->free = position;
                                       /* break the link                    */
            this->entries[previous].next = NO_MORE;
          }
        }                              /* non-terminal chain element        */
        else {
                                       /* close up the link                 */
          this->entries[position].next = this->entries[next].next;
                                       /* copy value and index to current   */
          OrefSet(this,this->entries[position].index,this->entries[next].index);
          OrefSet(this,this->entries[position].value,this->entries[next].value);
                                       /* clear the next entry              */
          OrefSet(this,this->entries[next].index,OREF_NULL);
          OrefSet(this,this->entries[next].value,OREF_NULL);
                                       /* set to "pristine" condition       */
          this->entries[next].next = NO_MORE;
          if (this->free < next)       /* new-low water mark?               */
            this->free = next;         /* reset to this point               */
        }
        return removed;                /* return removed element value      */
      }
      else
        previous = position;           /* remember previous member          */
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  return OREF_NULL;                    /* removed item not found            */
}

RexxObject *RexxHashTable::removeItem(
    RexxObject *value,                 /* item to remove                    */
    RexxObject *index )                /* index to remove                   */
/******************************************************************************/
/* Function:  Remove the tuple (value, item) from the hash table.  If         */
/*            the item is not found, .nil is returned.                        */
/******************************************************************************/
{
  HashLink position;                   /* target hash position              */
  HashLink previous;                   /* previous hash position            */
  HashLink next;                       /* next hash position                */
  RexxObject * removed;                /* removed item value                */

  position = hashIndex(index);         /* calculate the hash slot           */
  previous = NO_LINK;                  /* no previous slot yet              */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* get a match?                      */
      if (EQUAL_VALUE(index, this->entries[position].index) &&
          EQUAL_VALUE(value, this->entries[position].value)) {
                                       /* save the current value            */
        removed = this->entries[position].value;
                                       /* get the next pointer              */
        next = this->entries[position].next;
        if (next == NO_MORE) {         /* end of the chain?                 */
                                       /* clear this slot entry             */
          OrefSet(this,this->entries[position].index,OREF_NULL);
          OrefSet(this,this->entries[position].value,OREF_NULL);
          if (previous != NO_LINK)     /* if not the first of the chain     */
                                       /* break the link                    */
            this->entries[previous].next = NO_MORE;
        }                              /* non-terminal chain element        */
        else {
                                       /* close up the link                 */
          this->entries[position].next = this->entries[next].next;
                                       /* copy value and index to current   */
          OrefSet(this,this->entries[position].index,this->entries[next].index);
          OrefSet(this,this->entries[position].value,this->entries[next].value);
                                       /* clear the next entry              */
          OrefSet(this,this->entries[next].index,OREF_NULL);
          OrefSet(this,this->entries[next].value,OREF_NULL);
                                       /* set to "pristine" condition       */
          this->entries[next].next = NO_MORE;
          if (this->free < next)       /* new-low water mark?               */
            this->free = next;         /* reset to this point               */
        }
        return removed;                /* return removed element value      */
      }
      else
        previous = position;           /* remember previous member          */
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  return OREF_NULL;                    /* removed item not found            */
}


RexxObject *RexxHashTable::primitiveRemoveItem(
    RexxObject *value,                 /* item to remove                    */
    RexxObject *index )                /* index to remove                   */
/******************************************************************************/
/* Function:  Remove the tuple (value, item) from the hash table.  If         */
/*            the item is not found, .nil is returned.                        */
/******************************************************************************/
{
  HashLink position;                   /* target hash position              */
  HashLink previous;                   /* previous hash position            */
  HashLink next;                       /* next hash position                */
  RexxObject * removed;                /* removed item value                */

  position = hashPrimitiveIndex(index);  /* calculate the hash slot           */
  previous = NO_LINK;                  /* no previous slot yet              */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* get a match?                      */
      if (index == this->entries[position].index && value == this->entries[position].value) {
                                       /* save the current value            */
        removed = this->entries[position].value;
                                       /* get the next pointer              */
        next = this->entries[position].next;
        if (next == NO_MORE) {         /* end of the chain?                 */
                                       /* clear this slot entry             */
          OrefSet(this,this->entries[position].index,OREF_NULL);
          OrefSet(this,this->entries[position].value,OREF_NULL);
          if (previous != NO_LINK)     /* if not the first of the chain     */
                                       /* break the link                    */
            this->entries[previous].next = NO_MORE;
        }                              /* non-terminal chain element        */
        else {
                                       /* close up the link                 */
          this->entries[position].next = this->entries[next].next;
                                       /* copy value and index to current   */
          OrefSet(this,this->entries[position].index,this->entries[next].index);
          OrefSet(this,this->entries[position].value,this->entries[next].value);
                                       /* clear the next entry              */
          OrefSet(this,this->entries[next].index,OREF_NULL);
          OrefSet(this,this->entries[next].value,OREF_NULL);
                                       /* set to "pristine" condition       */
          this->entries[next].next = NO_MORE;
          if (this->free < next)       /* new-low water mark?               */
            this->free = next;         /* reset to this point               */
        }
        return removed;                /* return removed element value      */
      }
      else
        previous = position;           /* remember previous member          */
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  return OREF_NULL;                    /* removed item not found            */
}

RexxObject *RexxHashTable::primitiveHasItem(
    RexxObject *value,                 /* item to locate                    */
    RexxObject *index )                /* index to locate                   */
/******************************************************************************/
/* Function:  Determine if a tuple (value, item) pair exists in the hash      */
/*            table.  Return TRUE if found, FALSE other wise.                 */
/******************************************************************************/
{
  HashLink position;                   /* target hash position              */

  position = hashPrimitiveIndex(index);    /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* get a match?                      */
      if (index == this->entries[position].index && value == this->entries[position].value)
                                       /* got the one we want               */
        return (RexxObject *)TheTrueObject;
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  return (RexxObject *)TheFalseObject; /* item was not found                */
}

RexxObject *RexxHashTable::hasItem(
    RexxObject *value,                 /* item to locate                    */
    RexxObject *index )                /* index to locate                   */
/******************************************************************************/
/* Function:  Determine if a tuple (value, item) pair exists in the hash      */
/*            table.  Return TRUE if found, FALSE other wise.                 */
/******************************************************************************/
{
  HashLink position;                   /* target hash position              */

  position = hashIndex(index);         /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* get a match?                      */
      if (EQUAL_VALUE(index, this->entries[position].index) &&
          EQUAL_VALUE(value, this->entries[position].value))
                                       /* got the one we want               */
        return (RexxObject *)TheTrueObject;
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  return (RexxObject *)TheFalseObject; /* item was not found                */
}


/**
 * Test if an item exists in the hash collection.
 *
 * @param value  The test value.
 *
 * @return .true if it exists, .false otherwise.
 */
RexxObject *RexxHashTable::hasItem(RexxObject *value)
{
    // our size
    size_t size = this->totalSlotsSize();

    TABENTRY *ep = this->entries;
    TABENTRY *endp = ep + size;
                                         /* loop through all of the entries   */
    for (; ep < endp; ep++)
    {
        // if we have an item, see if it's the one we're looking for.
        if (ep->index != OREF_NULL)
        {
            if (EQUAL_VALUE(value, ep->value))
            {
                return TheTrueObject;    // return the index value

            }
        }
    }
    return TheFalseObject;
}


/**
 * Removes an item from the hash table.
 *
 * @param value  The test value.
 *
 * @return .true if it exists, .false otherwise.
 */
RexxObject *RexxHashTable::removeItem(RexxObject *value)
{
    // our size
    size_t size = this->totalSlotsSize();

    TABENTRY *ep = this->entries;
    TABENTRY *endp = ep + size;
                                         /* loop through all of the entries   */
    for (; ep < endp; ep++)
    {
        // if we have an item, see if it's the one we're looking for.
        if (ep->index != OREF_NULL)
        {
            if (EQUAL_VALUE(value, ep->value))
            {
                // this is complicated, so it's easier to just remove
                // this using the fully qualified tuple.
                return removeItem(value, ep->index);
            }
        }
    }
    return TheNilObject;
}


RexxObject *RexxHashTable::nextItem(
  RexxObject *value,                   /* item to locate                    */
  RexxObject *index )                  /* index to locate                   */
/******************************************************************************/
/* Function:  Return the next item with the key index that follows the        */
/*            given (value index) pair.  Used only for superscope lookup.     */
/*            Note:  This routine does the comparisons in as fast as way      */
/*            as possible, relying solely on object identify for a match.     */
/******************************************************************************/
{
  HashLink position;                   /* target hash position              */
  RexxObject *scope;                   /* returned scope                    */

  position = hashIndex(index);         /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* get a match?                      */
      if (this->entries[position].index == index && this->entries[position].value == value) {
        while ((position = this->entries[position].next) != NO_MORE) {
                                       /* same index value?                 */
          if (this->entries[position].index == index)
                                       /* this is the value we want         */
            return this->entries[position].value;
        }
        return TheNilObject;           /* didn't find what we wanted        */
      }
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
                                       /* must be added via setmethod, so   */
    scope = this->primitiveGet(index); /* return first one for this index   */
                                       /* truely not there?                 */
    if (scope == (RexxObject *)OREF_NULL)
      return TheNilObject;             /* return a failure                  */
    return scope;                      /* return the first scope            */
  }
  return TheNilObject;                 /* item was not found                */
}


RexxObject *RexxHashTable::primitiveNextItem(
  RexxObject *value,                   /* item to locate                    */
  RexxObject *index )                  /* index to locate                   */
/******************************************************************************/
/* Function:  Return the next item with the key index that follows the        */
/*            given (value index) pair.  Used only for superscope lookup.     */
/*            Note:  This routine does the comparisons in as fast as way      */
/*            as possible, relying solely on object identify for a match.     */
/******************************************************************************/
{
  HashLink position;                   /* target hash position              */
  RexxObject *scope;                   /* returned scope                    */

  position = hashPrimitiveIndex(index); /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* get a match?                      */
      if (this->entries[position].index == index && this->entries[position].value == value) {
        while ((position = this->entries[position].next) != NO_MORE) {
                                       /* same index value?                 */
          if (this->entries[position].index == index)
                                       /* this is the value we want         */
            return this->entries[position].value;
        }
        return TheNilObject;           /* didn't find what we wanted        */
      }
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
                                       /* must be added via setmethod, so   */
    scope = this->primitiveGet(index); /* return first one for this index   */
                                       /* truely not there?                 */
    if (scope == (RexxObject *)OREF_NULL)
      return TheNilObject;             /* return a failure                  */
    return scope;                      /* return the first scope            */
  }
  return TheNilObject;                 /* item was not found                */
}

RexxArray  *RexxHashTable::getAll(
    RexxObject *index)                 /* target index                      */
/******************************************************************************/
/* Function:  Get all elements with a specified index as an array             */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */
  size_t count;                        /* count of items to return          */
  RexxArray  *result;                  /* returned result                   */
  size_t i;                            /* loop counter                      */

  count = 0L;                          /* no items found yet                */
  position = hashIndex(index);         /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* if got a match                    */
      if (EQUAL_VALUE(index, this->entries[position].index))
        count++;                       /* bump our counter                  */
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  else
                                       /* no elements found                 */
    return (RexxArray *)TheNullArray->copy();

  result = new_array(count);           /* get proper size result array      */
  i = 1L;                              /* start at the first element        */
  position = hashIndex(index);         /* calculate the hash slot           */
  do {                                 /* while more items in chain         */
                                       /* if got a match                    */
    if (EQUAL_VALUE(index, this->entries[position].index))
                                       /* copy the value into our array     */
      result->put(this->entries[position].value,i++);
                                       /* step to the next link             */
  } while ((position = this->entries[position].next) != NO_MORE);
  return result;                       /* return the result array           */
}

RexxArray  *RexxHashTable::primitiveGetAll(
    RexxObject *index)                 /* target index                      */
/******************************************************************************/
/* Function:  Get all elements with a specified index as an array             */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */
  size_t count;                        /* count of items to return          */
  RexxArray  *result;                  /* returned result                   */
  size_t i;                            /* loop counter                      */

  count = 0L;                          /* no items found yet                */
  position = hashPrimitiveIndex(index);  /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* if got a match                    */
      if (index == this->entries[position].index)
        count++;                       /* bump our counter                  */
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  else
                                       /* no elements found                 */
    return (RexxArray *)TheNullArray->copy();

  result = new_array(count);           /* get proper size result array      */
  i = 1;                               /* start at the first element        */
  position = hashPrimitiveIndex(index);  /* calculate the hash slot           */
  do {                                 /* while more items in chain         */
                                       /* if got a match                    */
    if (index == this->entries[position].index)
                                       /* copy the value into our array     */
      result->put(this->entries[position].value,i++);
                                       /* step to the next link             */
  } while ((position = this->entries[position].next) != NO_MORE);
  return result;                       /* return the result array           */
}

RexxArray *RexxHashTable::stringGetAll(
    RexxString *index)                 /* target index                      */
/******************************************************************************/
/* Function:  Return an array of all value with the same index (using string  */
/*            lookup semantics)                                               */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */
  size_t count;                        /* count of items to return          */
  RexxArray *result;                   /* returned result                   */
  size_t i;                            /* loop counter                      */
  PCHAR data;                          /* string data                       */
  size_t length;                       /* string length                     */
  RexxString *entry;                   /* current compare entry             */

  data = index->stringData;            /* get the string data               */
  length = index->length;              /* and the length also               */
  count = 0L;                          /* no items found yet                */
  position = hashStringIndex(index);   /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* get the entry                     */
      entry = (RexxString *)this->entries[position].index;
                                       /* if got a match                    */
      if (index == entry || entry->memCompare(data, length))
        count++;                       /* bump our counter                  */
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  else
                                       /* no elements found                 */
    return (RexxArray *)TheNullArray->copy();

  result = new_array(count);           /* get proper size result array      */
  i = 1L;                              /* start at the first element        */
  position = hashIndex(index);         /* calculate the hash slot           */
  do {                                 /* while more items in chain         */
                                       /* get the entry                     */
    entry = (RexxString *)this->entries[position].index;
                                       /* if got a match                    */
    if (index == entry || entry->memCompare(data, length))
                                       /* copy the value into our array     */
      result->put(this->entries[position].value,i++);
                                       /* step to the next link             */
  } while ((position = this->entries[position].next) != NO_MORE);
  return result;                       /* return the result array           */
}

RexxArray  *RexxHashTable::allIndex(
    RexxObject *value)                 /* target value item                 */
/******************************************************************************/
/* Function:  Return all index items that match the associated value          */
/*            item.                                                           */
/******************************************************************************/
{
  size_t count;                        /* count of items to return          */
  RexxArray * result;                  /* returned result                   */
  long   i;                            /* loop counter                      */
  size_t j;                            /* array index                       */

  count = 0L;                          /* no items found yet                */
                                       /* loop through them all             */
  for (i = (long)this->totalSlotsSize() - 1; i >= 0; i--) {
                                       /* real entry?                       */
    if (this->entries[i].index != OREF_NULL) {
                                       /* is this the item we want?         */
      if (EQUAL_VALUE(value, this->entries[i].value))
        count++;                       /* bump our counter                  */
    }
  }

  result = new_array(count);           /* get proper size result array      */
  j = 1;                               /* start at the first element        */
                                       /* loop through them all             */
  for (i = (long)this->totalSlotsSize() - 1; i >= 0; i--) {
                                       /* real entry?                       */
    if (this->entries[i].index != OREF_NULL) {
                                       /* is this the item we want?         */
      if (EQUAL_VALUE(value, this->entries[i].value))
                                       /* copy the value into our array     */
        result->put(this->entries[i].index, j++);
    }
  }
  return result;                       /* return the result array           */
}


RexxObject *RexxHashTable::getIndex(
    RexxObject *value)                 /* target object                     */
/******************************************************************************/
/* Function:  Return an index associated with the supplied item.  The result  */
/*            is undefined for items that are referenced by multiple          */
/*            indices.                                                        */
/******************************************************************************/
{
  RexxObject *result;                  /* result index item                 */
  long   i;                            /* loop counter                      */

  result = OREF_NULL;                  /* no item yet                       */
                                       /* loop through them all             */
  for (i = (long)this->totalSlotsSize() - 1; i >= 0; i--) {
                                       /* real entry?                       */
    if (this->entries[i].index != OREF_NULL) {
                                       /* is this the item we want?         */
      if (EQUAL_VALUE(value, this->entries[i].value)) {
                                       /* get the index                     */
        result = this->entries[i].index;
        break;                         /* finished                          */
      }
    }
  }
  return result;                       /* return the count                  */
}

RexxObject *RexxHashTable::get(
    RexxObject *index)                 /* index of target item              */
/******************************************************************************/
/* Function:  Get an item from a hash table                                   */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */

  position = hashIndex(index);         /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* if got a match                    */
      if (EQUAL_VALUE(index, this->entries[position].index))
                                       /* return this item's value          */
        return this->entries[position].value;
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  return OREF_NULL;                    /* no value found                    */
}

RexxObject *RexxHashTable::primitiveGet(
    RexxObject *index)                 /* index of target item              */
/******************************************************************************/
/* Function:  Get an item from a hash table                                   */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */

  position = hashPrimitiveIndex(index);  /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* if got a match                    */
      if (index == this->entries[position].index)
                                       /* return this item's value          */
        return this->entries[position].value;
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  return OREF_NULL;                    /* no value found                    */
}

RexxHashTable *RexxHashTable::insert(
    RexxObject *value,                 /* value to insert                   */
    RexxObject * index,                /* index to insert                   */
    HashLink position,                 /* insertion position                */
    LONG   type )                      /* string type insertion             */
/******************************************************************************/
/* Function:  Insert an element into an overflow location of a hash table     */
/******************************************************************************/
{
  HashLink over;                       /* overflow slot location            */
  HashLink low;                        /* low check bound                   */
  RexxHashTable *newHash;              /* newly created hash table          */
  TABENTRY *primeEntry = &(this->entries[position]);

  low = this->mainSlotsSize();         /* get low free bound                */
  for (over = this->free;              /* look for overflow slot            */
       over >= low;
       over--) {
    TABENTRY *entry = &(this->entries[over]);
                                       /* find an empty slot?               */
    if (entry->index == OREF_NULL) {
                                       /* insert after hash slot            */
      entry->next = primeEntry->next;
                                       /* copy current index and value      */
                                       /* to the overflow slot              */
      OrefSet(this, entry->value, primeEntry->value);
      OrefSet(this, entry->index, primeEntry->index);
                                       /* set the new values in the first   */
                                       /* chain entry                       */
      OrefSet(this, primeEntry->value, value);
      OrefSet(this, primeEntry->index, index);
                                       /* set the overflow location         */
      primeEntry->next = (long)over;
      this->free = (long)(over-1);     /* update free pointer               */
      return OREF_NULL;                /* this was a successful addition    */
    }
  }
                                       /* allocate a larger hash table      */
  newHash = new_hashtab(this->totalSlotsSize() * 2);
  switch (type) {                      /* remerge based on the type         */

    case STRING_TABLE:                 /* string based table                */
      this->stringMerge(newHash);      /* do a string merge                 */
      break;

    case PRIMITIVE_TABLE:              /* primitive object table            */
      this->primitiveMerge(newHash);   /* do a primitive merge              */
      break;

    case FULL_TABLE:                   /* full table look ups               */
      save(newHash);                   /* secure to prevent garb.col.       */
      this->reMerge(newHash);          /* do a normal merge                 */
      discard_hold(newHash);           /* release  again                    */
      break;
  }

  // primitive tables require a primitive index.
  if (type == PRIMITIVE_TABLE)
  {
      position = newHash->hashPrimitiveIndex(index);/* calculate the hash slot           */
  }
  else
  {

      position = newHash->hashIndex(index);/* calculate the hash slot           */
  }
                                       /* if the hash slot is empty         */
  if (newHash->entries[position].index == OREF_NULL) {
                                       /* fill in both the value and index  */
    OrefSet(newHash, newHash->entries[position].value, value);
    OrefSet(newHash, newHash->entries[position].index, index);
  }
  else
                                       /* try to insert again               */
    newHash->insert(value, index, position, type);
  return newHash;                     /* return the new hash               */
}

RexxHashTable *RexxHashTable::stringAdd(
    RexxObject *value,                 /* added element value               */
    RexxString *index)                 /* added index value                 */
/******************************************************************************/
/* Function:  Add an element to a hash table, using only string searches      */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */

  position = hashStringIndex(index);   /* calculate the hash slot           */
                                       /* if the hash slot is empty         */
  if (this->entries[position].index == OREF_NULL) {
                                       /* fill in both the value and index  */
    OrefSet(this,this->entries[position].value,value);
    OrefSet(this,this->entries[position].index,index);
    return OREF_NULL;                  /* successful  addition              */
  }
  else                                 /* hash slot is full                 */
                                       /* go insert                         */
    return this->insert(value, index, position, STRING_TABLE);
}

RexxObject *RexxHashTable::stringGet(
    RexxString *index)                 /* index of target item              */
/******************************************************************************/
/* Function:  Search a hash table, restricting the search to string items.    */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */
  PCHAR data;                          /* string data                       */
  size_t length;                       /* string length                     */
  RexxString *entry;                   /* current table entry               */
  int depth = 0;

  data = index->stringData;            /* get the string data               */
  length = index->length;              /* and the length also               */

  position = hashStringIndex(index);   /* calculate the hash slot           */
  ULONG indexHash = HASHVALUE(index);  /* save the index                    */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    do {                               /* while more items in chain         */
                                       /* get the entry                     */
      entry = (RexxString *)this->entries[position].index;
                                       /* if got a match                    */
      if (index == entry || (HASHVALUE(entry) == indexHash && entry->memCompare(data, length))) {
                                       /* return this item's value          */
        return this->entries[position].value;
      }
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
  }
  return OREF_NULL;                    /* no value found                    */
}

RexxHashTable *RexxHashTable::stringPut(
    RexxObject *value,                 /* value of the item                 */
    RexxString *index)                 /* target index                      */
/******************************************************************************/
/* Function:  Add a value to a table, searching only for character string     */
/*            matches.                                                        */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */
  PCHAR data;                          /* string data                       */
  size_t length;                       /* string length                     */
  HashLink front;                      /* starting location                 */
  RexxString *entry;                   /* current table entry               */

  data = index->stringData;            /* get the string data               */
  length = index->length;              /* and the length also               */
  ULONG indexHash = HASHVALUE(index);  /* save the index                    */

  position = hashStringIndex(index);   /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    front = position;                  /* save the starting location        */
    do {                               /* while more items in chain         */
                                       /* get the entry                     */
      entry = (RexxString *)this->entries[position].index;
                                       /* if got a match                    */
      if (index == entry || (HASHVALUE(entry) == indexHash && entry->memCompare(data, length))) {
                                       /* set a new value                   */
        OrefSet(this, this->entries[position].value, value);
        return OREF_NULL;              /* indicate success                  */
      }
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
                                       /* go insert                         */
    return this->insert(value, index, front, STRING_TABLE);
  }
  else {                               /* empty at this slot                */
                                       /* fill in both the value and index  */
    OrefSet(this,this->entries[position].value,value);
    OrefSet(this,this->entries[position].index,index);
    return OREF_NULL;                  /* successful  addition              */
  }
}

RexxHashTable *RexxHashTable::put(
    RexxObject *value,                 /* value of the item                 */
    RexxObject *index)                 /* target index                      */
/******************************************************************************/
/* Function:  Add or replace an entry in the hash table.  This will return    */
/*            a new hash table if there is an overflow situation.             */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */
  HashLink front;                      /* starting hash position            */

  position = hashIndex(index);         /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    front = position;                  /* save the starting position        */
    do {                               /* while more items in chain         */
                                       /* if got a match                    */
      if (EQUAL_VALUE(index, this->entries[position].index)) {
                                       /* set a new value                   */
        OrefSet(this,this->entries[position].value,value);
        return OREF_NULL;              /* indicate success                  */
      }
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
                                       /* go insert                         */
    return this->insert(value, index, front, FULL_TABLE);
  }
  else {                               /* empty at this slot                */
                                       /* fill in both the value and index  */
    OrefSet(this,this->entries[position].value,value);
    OrefSet(this,this->entries[position].index,index);
    return OREF_NULL;                  /* successful  addition              */
  }
}

RexxHashTable *RexxHashTable::primitivePut(
    RexxObject *value,                 /* value of the item                 */
    RexxObject *index)                 /* target index                      */
/******************************************************************************/
/* Function:  Add or replace an entry in the hash table.  This will return    */
/*            a new hash table if there is an overflow situation.             */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */
  HashLink front;                      /* starting hash position            */

  position = hashPrimitiveIndex(index);  /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    front = position;                  /* save the starting position        */
    do {                               /* while more items in chain         */
                                       /* if got a match                    */
      if (index == this->entries[position].index) {
                                       /* set a new value                   */
        OrefSet(this,this->entries[position].value,value);
        return OREF_NULL;              /* indicate success                  */
      }
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
                                       /* go insert                         */
    return this->insert(value, index, front, PRIMITIVE_TABLE);
  }
  else {                               /* empty at this slot                */
                                       /* fill in both the value and index  */
    OrefSet(this,this->entries[position].value,value);
    OrefSet(this,this->entries[position].index,index);
    return OREF_NULL;                  /* successful  addition              */
  }
}

RexxHashTable *RexxHashTable::putNodupe(
    RexxObject *value,                 /* value of the item                 */
    RexxObject *index)                 /* target index                      */
/******************************************************************************/
/* Function:  Add an entry to a hash table, making sure there are no          */
/*            duplicate entries.                                              */
/******************************************************************************/
{
  HashLink position;                   /* current position                  */
  HashLink front;                      /* starting hash position            */

  position = hashIndex(index);         /* calculate the hash slot           */
                                       /* have an entry at this slot        */
  if (this->entries[position].index != OREF_NULL) {
    front = position;                  /* save the starting point           */
    do {                               /* while more items in chain         */
                                       /* if got match on index and value   */
      if (EQUAL_VALUE(index, this->entries[position].index) && this->entries[position].value == value) {
        return OREF_NULL;              /* indicate success                  */
      }
                                       /* step to the next link             */
    } while ((position = this->entries[position].next) != NO_MORE);
                                       /* go insert                         */
    return this->insert(value, index, front, FULL_TABLE);
  }
  else {                               /* empty at this slot                */
                                       /* fill in both the value and index  */
    OrefSet(this,this->entries[position].value,value);
    OrefSet(this,this->entries[position].index,index);
    return OREF_NULL;                  /* successful  addition              */
  }
}

RexxObject *RexxHashTable::value(
  HashLink position)                   /* target position                   */
/******************************************************************************/
/* Function:  Return the value associated with a numeric index.               */
/******************************************************************************/
{
                                       /* within the bounds?                */
  if (position < this->totalSlotsSize())
                                       /* return the entry value            */
    return this->entries[position].value;
  else
    return OREF_NULL;                  /* return nothing                    */
}

size_t RexxHashTable::totalEntries(void)
/******************************************************************************/
/* Function:  Return the count of entries in a hash table.                    */
/******************************************************************************/
{
  size_t result;                       /* count of items                    */
  long   i;                            /* loop counter                      */

  result = 0;                          /* no items yet                      */
                                       /* loop through them all             */
  for (i = (long)this->totalSlotsSize() - 1; i >= 0; i--) {
                                       /* real entry?                       */
    if (this->entries[i].index != OREF_NULL)
      result++;                        /* count it!                         */
  }
  return result;                       /* return the count                  */
}


RexxObject *RexxHashTable::merge(
    RexxHashTableCollection *target)   /* target other collection           */
/******************************************************************************/
/* Function:  Merge a hash table into another collection target.              */
/******************************************************************************/
{
  long   i;                            /* loop counter                      */

                                       /* loop through them all             */
  for (i = (long)this->totalSlotsSize() - 1; i >= 0; i--) {
                                       /* is this a real entry?             */
    if (this->entries[i].index != OREF_NULL)
                                       /* merge into the target collection  */
      target->mergeItem(this->entries[i].value,this->entries[i].index);
  }
  return OREF_NULL;                    /* always returns nothing            */
}


void RexxHashTable::reMerge(
  RexxHashTable *newHash)              /* target other collection           */
/******************************************************************************/
/* Function:   Merge a hash table into another hash table after a table       */
/*             expansion.                                                     */
/******************************************************************************/
{
  long   i;                            /* loop counter                      */

                                       /* loop through them all             */
  for (i = (long)this->totalSlotsSize() - 1; i >= 0; i--) {
                                       /* is this a real entry?             */
    if (this->entries[i].index != OREF_NULL)
                                       /* add this item to the new hash     */
      newHash->add(this->entries[i].value, this->entries[i].index);
  }
}

void RexxHashTable::primitiveMerge(
  RexxHashTable *newHash)              /* target other collection           */
/******************************************************************************/
/* Function:   Merge a hash table into another hash table after a table       */
/*             expansion.                                                     */
/******************************************************************************/
{
  long   i;                            /* loop counter                      */

                                       /* loop through them all             */
  for (i = (long)this->totalSlotsSize() - 1; i >= 0; i--) {
                                       /* is this a real entry?             */
    if (this->entries[i].index != OREF_NULL)
                                       /* add this item to the new hash     */
      newHash->primitiveAdd(this->entries[i].value, this->entries[i].index);
  }
}

RexxObject *RexxHashTable::stringMerge(
  RexxHashTable *target)               /* target other collection           */
/******************************************************************************/
/* Function:  Merge a string based hash table into another                    */
/******************************************************************************/
{
  long   i;                            /* loop counter                      */

                                       /* loop through them all             */
  for (i = (long)this->totalSlotsSize() - 1; i >= 0; i--) {
                                       /* is this a real entry?             */
    if (this->entries[i].index != OREF_NULL)
                                       /* merge into the target collection  */
      target->stringAdd(this->entries[i].value, (RexxString *)this->entries[i].index);
  }
  return OREF_NULL;                    /* always returns nothing            */
}

HashLink RexxHashTable::first(void)
/******************************************************************************/
/* Function:  Return the index of the first item in the hash table            */
/******************************************************************************/
{
  size_t i;                            /* loop founter                      */

                                       /* loop until first item is found or */
                                       /* we reach the end of the table     */
  for (i = 0; i < this->totalSlotsSize() && this->entries[i].index == OREF_NULL; i++) ;
  return i;                            /* return the position               */
}

HashLink RexxHashTable::next(
  HashLink position)                   /* current index position            */
/******************************************************************************/
/* Function:  Return the index of the "next" item after an index              */
/******************************************************************************/
{
  size_t i;                            /* loop counter                      */

                                       /* loop until first item is found or */
                                       /* we reach the end of the table     */
  for (i = position+1; i < this->totalSlotsSize() && this->entries[i].index == OREF_NULL; i++) ;
  return i;                            /* return the position               */
}

RexxObject *RexxHashTable::mergeItem(
    RexxObject *value,                 /* value to add                      */
    RexxObject *index)                 /* index location                    */
/******************************************************************************/
/* Function:  Merge an individual item into this hash table                   */
/******************************************************************************/
{
  this->putNodupe(value, index);       /* add without duplicating           */
  return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxHashTable::replace(
    RexxObject *value,                 /* value to replace                  */
    HashLink  position)                /* position to replace               */
/******************************************************************************/
/* Function:  Replace an item with a new value                                */
/******************************************************************************/
{
                                       /* just set the new value            */
  OrefSet(this,this->entries[position].value,value);
  return OREF_NULL;                    /* always return nothing             */
}

RexxArray  *RexxHashTable::allItems()
/******************************************************************************/
/* Function:  Create an array containing the hash table values                */
/******************************************************************************/
{
  size_t i;                            /* loop counter                      */
  size_t j;                            /* loop counter                      */
  RexxArray *result;                   /* result array                      */

  result = new_array(items());         /* get a new array                   */
  j = 0;                               /* set the insertion point           */
                                       /* loop through all of the items     */
  for (i = 0; i < this->totalSlotsSize(); i++) {
                                       /* real entry?                       */
    if (this->entries[i].index != OREF_NULL)
                                       /* copy the value into the array     */
      result->put(this->entries[i].value, ++j);
  }
  return result;                       /* return the result array           */
}


/**
 * count the number of items in the hash table.
 *
 * @return The item count.
 */
size_t RexxHashTable::items()
{
    size_t count = 0;

    for (size_t i = 0; i < this->totalSlotsSize(); i++)
    {

        if (this->entries[i].index != OREF_NULL)
        {
            count++;
        }
    }
    return count;
}


/**
 * Empty an individual hashtable bucket.  This will clear
 * the entire chain.
 *
 * @param position The hash table bucket to clear.
 */
void RexxHashTable::emptySlot(HashLink position)
{
    if (this->entries[position].index != OREF_NULL)
    {
        // we have an initial link, so clear those entries out
        OrefSet(this,this->entries[position].index,OREF_NULL);
        OrefSet(this,this->entries[position].value,OREF_NULL);
        // we have at least a head element, so run the chain
        // clearing everything out

        // step to the next link.  The remainder are cleared out and
        // returned to the free pool.
        HashLink next = entries[position].next;
        // and make sure the link is severed.
        entries[position].next = NO_MORE;
        while (next != NO_MORE)
        {
            position = next;
            // clear the entries out
            OrefSet(this,this->entries[position].index,OREF_NULL);
            OrefSet(this,this->entries[position].value,OREF_NULL);

            // get the next link, and clear the link info in the current
            next = entries[position].next;
            entries[position].next = NO_MORE;
            // if this creates a new highwater mark, move the free pointer.
            if (position > this->free)
            {
                this->free = position;
            }

        }
    }
}


/**
 * Empty a HashTable.
 */
void RexxHashTable::empty()
{
    // run the main hash bucket clearing the links
    for (HashLink i = 0; i < mainSlotsSize(); i++)
    {
        emptySlot(i);
    }
}


/**
 * Test if the hash table is empty.
 *
 * @return
 */
bool RexxHashTable::isEmpty()
{
    return items() == 0;
}




RexxArray *RexxHashTable::makeArray(void)
/******************************************************************************/
/* Function:  Create an array containing the hash table indexes.              */
/******************************************************************************/
{
    // this just returns the index values
    return this->allIndexes();
}


RexxArray *RexxHashTable::allIndexes()
/******************************************************************************/
/* Function:  Create an array containing the hash table indexes.              */
/******************************************************************************/
{
  size_t i;                            /* loop counter                      */
  size_t j;                            /* loop counter                      */
  RexxArray *result;                   /* result array                      */

  result = new_array(items());         /* get a new array                   */
  j = 0;                               /* set the insertion point           */
                                       /* loop through all of the items     */
  for (i = 0; i < this->totalSlotsSize(); i++) {
                                       /* real entry?                       */
    if (this->entries[i].index != OREF_NULL)
                                       /* copy the index into the array     */
      result->put(this->entries[i].index, ++j);
  }
  return result;                       /* return the result array           */
}


RexxSupplier *RexxHashTable::supplier(void)
/******************************************************************************/
/* Function:  create a supplier from a hash table                             */
/******************************************************************************/
{
  size_t count;                        /* count of values                   */
  size_t i;                            /* loop counter                      */
  size_t j;                            /* loop counter                      */
  RexxArray *values;                   /* value array                       */
  RexxArray *indexes;                  /* index array                       */

  count = items();                     /* no items yet                      */

  values = new_array(count);           /* get a new array                   */
  indexes = new_array(count);          /* and an index array                */
  j = 1;                               /* set the insertion point           */
                                       /* loop through all of the items     */
  for (i = 0; i < this->totalSlotsSize(); i++) {
                                       /* real entry?                       */
    if (this->entries[i].index != OREF_NULL) {
                                       /* copy the index into the array     */
      indexes->put(this->entries[i].index, j);
                                       /* and the value                     */
      values->put(this->entries[i].value, j);
      j++;                             /* step the array location           */
    }
  }
                                       /* return the supplier               */
  return (RexxSupplier *)new_supplier(values, indexes);
}

RexxObject *RexxHashTable::index(
  HashLink position)                   /* return an index item for position */
/******************************************************************************/
/* Function:  Return the item associated with a numeric hash table index      */
/******************************************************************************/
{
                                       /* if within bounds                  */
  if (position < this->totalSlotsSize())
                                       /* return the index item             */
    return this->entries[position].index;
  else
    return OREF_NULL;                  /* otherwise, no item for this       */
}

RexxHashTable *RexxHashTable::reHash(void)
/******************************************************************************/
/* Function:  Rehash the elements of a hash table because of a restore        */
/**REXX************************************************************************/
{
  long   i;
  RexxHashTable *newHash;
  RexxHashTable *expandHash;

                                       /* Assume the same size will work,   */
  newHash = new_hashtab(this->totalSlotsSize());
                                       /* should in MOST cases....          */
  for (i = (long)this->totalSlotsSize() - 1; i >= 0; i--) {
    if (this->entries[i].index != OREF_NULL) {
      expandHash = newHash->add(this->entries[i].value, this->entries[i].index);
      if (expandHash != OREF_NULL)     /* have a reallocation occur?        */
        newHash = expandHash;          /* finish up with the new table      */
    }
  }
  return newHash;                      /* Return the updated(rehashed) hashtable*/
}

