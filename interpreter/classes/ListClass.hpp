/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                               ListClass.hpp    */
/*                                                                            */
/* Primitive List Class Definitions                                           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ListClass
#define Included_ListClass

#include "ListContents.hpp"

class RexxObject;

class ListClass : public RexxObject
{
  friend class ListTable;
 public:
    void * operator new(size_t);

    inline ListClass(RESTORETYPE restoreType) { ; };
    ListClass(size_t capacity = DefaultListSize);
    ListClass(bool fromRexx) { }

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    RexxInternalObject *copy() override;
    ArrayClass *makeArray() override;
    ArrayClass *requestArray() override;

    // APIS for use from other parts of the interpreter
    void put(RexxInternalObject *value, size_t index);
    RexxInternalObject *get(size_t index);
    ListClass *section(size_t index, size_t count);
    size_t insert(RexxInternalObject *value, size_t insertionPoint);
    size_t addLast(RexxInternalObject *value);
    size_t addFirst(RexxInternalObject *value);
    size_t append(RexxInternalObject *value);
    RexxInternalObject *remove(size_t index);
    RexxInternalObject *firstItem();
    RexxInternalObject *lastItem();
    size_t firstIndex();
    size_t lastIndex();
    size_t nextIndex(size_t index);
    size_t previousIndex(size_t _index);
    bool hasIndex(size_t index);
    bool hasItem(RexxInternalObject *target);
    ArrayClass *allItems();
    void empty();
    bool isEmpty();
    ArrayClass *allIndexes();
    size_t getIndex(RexxInternalObject *target);

    RexxInternalObject *removeItem(RexxInternalObject *target);
    SupplierClass *supplier();
    size_t items();
    ArrayClass *weakReferenceArray();

    // The exported Rexx methods
    RexxObject *initRexx(RexxObject *initialSize);
    RexxInternalObject *putRexx(RexxObject *value, RexxObject *argIndex);
    RexxInternalObject *getRexx(RexxObject *argIndex);
    RexxObject *sectionRexx(RexxObject *argIndex, RexxObject *count);
    RexxObject *insertRexx(RexxObject *value, RexxObject *index);
    RexxObject *appendRexx(RexxObject *value);
    RexxObject *removeRexx(RexxObject *index);
    RexxObject *firstItemRexx();
    RexxObject *lastItemRexx();
    RexxObject *firstRexx();
    RexxObject *lastRexx();
    RexxObject *nextRexx(RexxObject *index);
    RexxObject *previousRexx(RexxObject *index);
    RexxObject *hasIndexRexx(RexxObject *index);
    RexxObject *emptyRexx();
    RexxObject *isEmptyRexx();
    RexxObject *indexRexx(RexxObject *target);
    RexxInternalObject *removeItemRexx(RexxObject *target);
    RexxObject *itemsRexx();
    RexxObject *hasItemRexx(RexxObject *target);

    // Class related methods
    ListClass     *newRexx(RexxObject **, size_t);
    ListClass     *ofRexx(RexxObject **, size_t);

    static void createInstance();
    static RexxClass *classInstance;

    // the default size of a list (and also the minimum size we'll create)
    static const size_t DefaultListSize;
    // for small Lists, we expand by doubling the current size, however
    // for Lists larger than this limit, we just extend by half the current size
    static const size_t ExpansionDoubleLimit = 2000;


 protected:

    // internal support methods
    void initialize(size_t capacity = DefaultListSize);
    ListContents::ItemLink validateIndex(RexxObject *index, size_t position);
    ListContents::ItemLink validateInsertionIndex(RexxObject *index, size_t position);
    ListContents::ItemLink requiredIndex(RexxObject *index, size_t position);
    void expandContents();
    void expandContents(size_t capacity );
    void ensureCapacity(size_t delta);
    void checkFull();
    RexxObject *indexObject(ListContents::ItemLink index);

    ListContents *contents;               // list table  item
};


inline ListClass *new_list(size_t capacity = ListClass::DefaultListSize) { return new ListClass(capacity); }

#endif
