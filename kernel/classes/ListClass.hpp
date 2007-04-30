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
/* REXX Kernel                                                  ListClass.hpp    */
/*                                                                            */
/* Primitive List Class Definitions                                           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxList
#define Included_RexxList

#include "RexxListTable.hpp"

#define INITIAL_LIST_SIZE     5        /* initial list allocation           */
#define EXTEND_LIST_SIZE      5        /* amount to extend by each time     */
                                       /* size of buffer for a given number */
                                       /* of list entries                   */
#define TABLE_SIZE(n)         ((n)*sizeof(LISTENTRY))
                                       /* number of list entries in a given */
                                       /* buffer size                       */
#define ENTRY_COUNT(n)        ((n)/sizeof(LISTENTRY))
                                       /* address of a given buffer entry   */
#define ENTRY_POINTER(n)      (this->table->address() + n)
#define ENTRY_INDEX(p)        (p - this->table->address())
#define LIST_END              -1       /* end of list marker                */
#define NOT_ACTIVE            -2       /* free element marker               */

 class RexxList : public RexxObject {
  public:
   void * operator new(size_t);
   inline void * operator new(size_t size, void *objectPtr) { return objectPtr; };
   inline RexxList(RESTORETYPE restoreType) { ; };
   inline RexxList() { ; }

   void          init();
   void          live();
   void          liveGeneral();
   void          flatten(RexxEnvelope *);
   RexxObject   *copy();
   RexxArray    *makeArray();
   RexxArray    *allItems();
   RexxArray    *allIndexes();
   RexxArray    *requestArray();

   RexxObject   *value(RexxObject *);
   RexxObject   *remove(RexxObject *);
   RexxObject   *primitiveRemove(LISTENTRY *);
   RexxObject   *firstRexx();
   RexxObject   *lastRexx();
   RexxObject   *next(RexxObject *);
   RexxObject   *previous(RexxObject *);
   RexxObject   *hasIndex(RexxObject *);
   RexxSupplier *supplier();
   RexxObject   *itemsRexx();
   inline size_t items() { return (size_t)count; };
   RexxObject   *insert(RexxObject *, RexxObject *);
   RexxObject   *put(RexxObject *, RexxObject *);
   RexxObject   *section(RexxObject *, RexxObject *);
   RexxObject   *sectionSubclass(LISTENTRY *, LONG);
   RexxObject   *firstItem();
   RexxObject   *lastItem();
   RexxObject   *insertRexx(RexxObject *, RexxObject *);
   void          partitionBuffer(long, long);
   RexxArray    *makeArrayIndices();
   long          getFree();
   RexxObject   *add(RexxObject *, RexxObject *);
   RexxObject   *removeFirst() { return (this->first != LIST_END) ? this->primitiveRemove(ENTRY_POINTER(this->first)) : TheNilObject; }
   RexxObject   *removeLast() { return (this->last != LIST_END) ? this->primitiveRemove(ENTRY_POINTER(this->last)) : TheNilObject; }
   LISTENTRY    *getEntry(RexxObject *, RexxObject *);
   RexxObject   *indexOfValue(RexxObject *);
   RexxObject   *empty();
   RexxObject   *isEmpty();
   RexxObject  *index(RexxObject *);
   RexxObject  *hasItem(RexxObject *);
   RexxObject  *removeItem(RexxObject *);

   void          addLast(RexxObject *value);
   void          addFirst(RexxObject *value);
   inline long   getSize() {return this->count;}
   RexxObject   *append(RexxObject *);

   RexxListTable *table;               /* list table  item                  */
   long first;                         /* first real element index          */
   long last;                          /* last real element index           */
   long count;                         /* count of items in the list        */
   long size;                          /* element slots in the buffer       */
   long free;                          /* start of free element chain       */
 };

class RexxListClass : public RexxClass {
 public:
   RexxList     *newRexx(RexxObject **, size_t);
   RexxList     *classOf(RexxObject **, size_t);
};

#endif
