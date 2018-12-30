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
/******************************************************************************/
/*                                                                            */
/* Mapping table for non-object values indexed by object identity.  These     */
/* are just intended for internal uses such as memory tables, and are much    */
/* simpler classes.                                                           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_MapBucket
#define Included_MapBucket

#include "ObjectClass.hpp"


/**
 * A mapping class for keeping non-object values that
 * are indexed by object identify.  This is ideal for
 * purposes such as managing the old space-to-new space
 * table or the flattening object offset tables.
 */
class MapBucket : public RexxInternalObject
{
 friend class MapTable;
 public:
    typedef size_t MapLink;                  // a link to another map item

           void *operator new(size_t base, size_t entries);
    inline void  operator delete(void *) {;}

    MapBucket(size_t entries);
    inline MapBucket(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void         empty();
    bool         isEmpty() { return itemCount == 0; }
    bool         isFull()  { return itemCount >= totalSize; }
    size_t       items() { return itemCount; };

    size_t       remove(RexxInternalObject *key);
    size_t       get(RexxInternalObject *key);
    bool         put(size_t value, RexxInternalObject *key);
    bool         hasIndex(RexxInternalObject *key);
    bool         append(size_t value, RexxInternalObject * index, MapLink position);
    void         merge(MapBucket *other);
    MapLink      locate(RexxInternalObject *key);
    bool         increment(RexxInternalObject *key);
    void         decrement(RexxInternalObject *key);

    // We never get saved in the image or flattened with other objects, so we can just use the
    // identity hash to generate the index
    inline MapLink hashIndex(RexxInternalObject *obj) { return (MapLink)(obj->identityHash() % bucketSize); }


    // link terminator
    static const MapLink NoMore = 0;
    // indicates not linked
    static const MapLink NoLink = ~((MapLink)0);

protected:

   class MapEntry
   {
   public:
       RexxInternalObject *index;           // the value we index from
       size_t value;                        // the stored value (always an integer type
       MapLink next;                        // next item in overflow bucket

       inline bool isAvailable() { return index == OREF_NULL; }
       inline bool isIndex(RexxInternalObject *i) { return i == index; }
       inline void clear() { set(OREF_NULL, 0); next = NoMore; }
       inline void copyElement(MapEntry &other)
       {
           value = other.value;
           index = other.index;
           next = other.next;
       }
       // NOTE:  This is a transient object, no setField() used.
       inline void set(RexxInternalObject *i, size_t v) { index = i; value = v; }
       inline void setValue(size_t v) { value = v; }
   };

    size_t   bucketSize;                // size of the hash table
    size_t   totalSize;                 // total size of the table, including the overflow area
    size_t   itemCount;                 // total number of items in the table
    MapLink  freeItem;                  // first free element
    MapEntry entries[1];                // hash table entries
};

#endif

