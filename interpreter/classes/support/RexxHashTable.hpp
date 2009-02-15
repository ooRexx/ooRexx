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
/******************************************************************************/
/* REXX Kernel                                           RexxHashTable.hpp    */
/*                                                                            */
/* Primitive Hash Table Class Definitions                                     */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxHash
#define Included_RexxHash


/* The type for the reference links */
typedef size_t HashLink;

 typedef struct tabentry {
  RexxObject *value;                   /* item value object                 */
  RexxObject *index;                   /* item index object                 */
  HashLink next;                       /* next item in overflow bucket      */
 } TABENTRY;

 class RexxHashTableCollection;
 class RexxTable;

 class RexxHashTable : public RexxInternalObject {
  public:
   enum
   {
       DEFAULT_HASH_SIZE = 22,
       STRING_TABLE      = 1,
       PRIMITIVE_TABLE   = 2,
       FULL_TABLE        = 3,
   };

   inline void * operator new(size_t size, void *objectPtr) { return objectPtr; };
   inline void  operator delete(void *) { ; }
   inline void  operator delete(void *, void *) { ; }

   inline RexxHashTable(RESTORETYPE restoreType) { ; };
   inline RexxHashTable() { ; }

   void         live(size_t);
   void         liveGeneral(int reason);
   void         flatten(RexxEnvelope *);
   RexxArray  * makeArray();
   void         empty();
   bool         isEmpty();
   size_t       items();
   void         emptySlot(HashLink);

   HashLink       next(HashLink position);
   RexxObject    *value(HashLink position);
   RexxObject    *index(HashLink position);
   RexxObject    *merge(RexxHashTableCollection *target);
   RexxObject    *mergeItem(RexxObject *value, RexxObject *index);
   RexxHashTable *add(RexxObject *value, RexxObject *key);
   RexxObject    *remove(RexxObject *key);
   RexxArray     *getAll(RexxObject *key);
   size_t         countAll(RexxObject *key);
   RexxObject    *get(RexxObject *key);
   RexxHashTable *put(RexxObject *value, RexxObject *key);
   RexxHashTable *primitiveAdd(RexxObject *value, RexxObject *key);
   RexxObject    *primitiveRemove(RexxObject *key);
   RexxArray     *primitiveGetAll(RexxObject *key);
   RexxObject    *primitiveGet(RexxObject *key);
   RexxHashTable *primitivePut(RexxObject *value, RexxObject *key);
   RexxObject    *primitiveRemoveItem(RexxObject *value, RexxObject *key);
   RexxObject    *primitiveRemoveItem(RexxObject *value);
   RexxObject    *primitiveHasItem(RexxObject *, RexxObject *);
   RexxObject    *primitiveHasItem(RexxObject *);
   RexxObject    *primitiveGetIndex(RexxObject *value);
   size_t         totalEntries();
   HashLink       first();
   RexxObject    *replace(RexxObject *value, HashLink position);
   RexxArray     *allIndex(RexxObject *key);
   RexxObject    *getIndex(RexxObject *value);
   RexxHashTable *reHash();
   RexxHashTable *putNodupe(RexxObject *value, RexxObject *key);
   RexxSupplier  *supplier();
   RexxArray     *allItems();
   RexxArray     *allIndexes();
   RexxObject    *removeItem(RexxObject *value, RexxObject *key);
   RexxObject    *removeItem(RexxObject *value);
   RexxObject    *stringGet(RexxString *key);
   RexxHashTable *stringPut(RexxObject *value, RexxString *key);
   RexxHashTable *stringAdd(RexxObject *value, RexxString *key);
   RexxArray     *stringGetAll(RexxString *key);
   RexxObject    *stringMerge(RexxHashTable *target);
   RexxObject    *hasItem(RexxObject * value, RexxObject *key);
   RexxObject    *hasItem(RexxObject * value);
   void           reMerge(RexxHashTable *target);
   void           primitiveMerge(RexxHashTable *target);
   RexxHashTable *insert(RexxObject *value, RexxObject *index, HashLink position, int type);
   RexxObject    *nextItem(RexxObject *, RexxObject *);
   RexxObject    *primitiveNextItem(RexxObject *, RexxObject *);
   inline size_t  mainSlotsSize()  { return this->size; };
   inline size_t  totalSlotsSize() { return this->size * 2; };
   inline bool    available(HashLink position) { return (size_t)position < this->totalSlotsSize(); };
   inline HashLink hashIndex(RexxObject *obj) { return (HashLink)(obj->hash() % this->mainSlotsSize()); }
   // NB:  Ideally, hashPrimitiveIndex() would be best served by using the identityHash().  Unfortunately,
   // the identity hash value is derived directly from the object reference.  This means that objects that
   // are in the saved image (or restored as part of saved programs) will have different identity hashes before
   // and after the store, which will cause hash table lookup failures.  We'll use whatever value is stored
   // in the hashvalue field.
   inline HashLink hashPrimitiveIndex(RexxObject *obj) { return (HashLink)(obj->getHashValue() % this->mainSlotsSize()); }
   inline HashLink hashStringIndex(RexxObject *obj) { return (HashLink)(obj->hash() % this->mainSlotsSize()); }

   static RexxTable *newInstance(size_t, size_t, size_t);
   static RexxHashTable *newInstance(size_t);

 protected:

   size_t   size;                      // size of the hash table
   HashLink free;                      /* first free element                */
   TABENTRY entries[1];                /* hash table entries                */
 };


inline RexxTable *new_hashCollection(size_t s, size_t s2, size_t t) { return RexxHashTable::newInstance(s, s2, t); }
inline RexxHashTable *new_hashtab(size_t s) { return RexxHashTable::newInstance(s); }

 #endif
