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
/* REXX Kernel                                                  ListClass.c      */
/*                                                                            */
/* Primitive List Class                                                       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "ListClass.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"

void RexxList::init(void)
/******************************************************************************/
/* Function:  Initial set up of a list object instance                        */
/******************************************************************************/
{
  this->first = LIST_END;              /* no first element                  */
  this->last = LIST_END;               /* no last element                   */
  this->size = INITIAL_LIST_SIZE;      /* set the buffer upper bound        */
                                       /* chain up the free elements        */
  this->partitionBuffer(0, INITIAL_LIST_SIZE);
}

RexxObject *RexxList::copy(void)
/******************************************************************************/
/* Function:   create a copy of a list and the associated table               */
/******************************************************************************/
{
  RexxList *newlist;                   /* new list  object                  */

                                       /* make a copy of ourself (also      */
                                       /* copies the object variables)      */
  newlist = (RexxList *)this->RexxObject::copy();
                                       /* make a copy of the table          */
  OrefSet(newlist, newlist->table, (RexxListTable *)this->table->copy());
  return (RexxObject *)newlist;        /* return the new list               */
}

void RexxList::partitionBuffer(
     long  first,                      /* first entry location              */
     long  count )                     /* entries to partition              */
/******************************************************************************/
/* Function:  Partition a buffer up into a chain of free elements and set     */
/*            this up as the free element chain                               */
/******************************************************************************/
{
  long       i;                        /* loop counter                      */
  LISTENTRY *element;                  /* working list element              */

  this->free = first;                  /* set the new free chain head       */
  i = first;                           /* get a loop counter                */
  element = ENTRY_POINTER(i);          /* point to the first element        */
  while (count--) {
                                       /* zero the element value            */
    OrefSet(this->table, element->value, OREF_NULL);
    element->next = ++i;               /* set the next element pointer      */
    element->previous = NOT_ACTIVE;    /* mark as a free element            */
    element++;                         /* step to the next element          */
  }
  element--;                           /* step back to last element         */
  element->next = LIST_END;            /* set the terminator                */
}

long RexxList::getFree(void)
/******************************************************************************/
/* Function:  Check that we have at least one element on the free chain, and  */
/*            if not, expand our buffer size to add some space.               */
/******************************************************************************/
{
  RexxListTable *newLTable;            /* new allocated buffer              */
  LISTENTRY *element;                  /* current working element           */
  long    new_index;                   /* return index value                */
  long    i;                           /* loop counter                      */

  if (this->free == LIST_END) {        /* no free elements?                 */
                                       /* allocate a larger table           */
    newLTable = new (this->size * 2) RexxListTable;
                                       /* copy over to the new buffer       */
    memcpy(newLTable->address(), this->table->address(), TABLE_SIZE(this->size));
                                       /* make this the new buffer          */
    OrefSet(this, this->table, newLTable);
                                       /* If either of the objects are in   */
                                       /* OldSpace,  we need to OrefSet     */
                                       /* each copied element               */
    if (OldSpace(this) || OldSpace(newLTable)) {
      element = ENTRY_POINTER(0);      /* point at the first element        */
                                       /* copy each element into new buffer */
      for (i = 0; i < this->size; i++) {
                                       /* do an explicit set operator       */
        OrefSet(this->table, element->value, element->value);
        element++;                     /* step to the next element          */
      }
    }
                                       /* chain up the free elements        */
    this->partitionBuffer(this->size, this->size);
    this->size += this->size;          /* increase the size                 */
  }
  new_index = this->free;              /* get the free element index        */
                                       /* close up the free chain           */
  this->free = ENTRY_POINTER(new_index)->next;
  return new_index;                    /* return the first free element     */
}

void RexxList::live(void)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->table);
  memory_mark(this->objectVariables);
  cleanUpMemoryMark
}

void RexxList::liveGeneral(void)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->table);
  memory_mark_general(this->objectVariables);
  cleanUpMemoryMarkGeneral
}

void RexxList::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxList)

   flatten_reference(newThis->table, envelope);
   flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}

LISTENTRY * RexxList::getEntry(
     RexxObject *index,                /* list index item                   */
     RexxObject *position)             /* index argument error position     */
/******************************************************************************/
/* Function:  Resolve a list index argument to a list element position        */
/******************************************************************************/
{
  RexxInteger *integer_index;          /* requested integer index           */
  LONG     item_index;                 /* converted item number             */
  LISTENTRY *element;                  /* located list element              */

  if (index == OREF_NULL)              /* must have one here                */
                                       /* else an error                     */
    report_exception1(Error_Incorrect_method_noarg, position);
                                       /* force to integer form             */
  integer_index = (RexxInteger *)REQUEST_INTEGER(index);
  if (integer_index == TheNilObject)   /* doesn't exist?                    */
                                       /* raise an exception                */
    report_exception1(Error_Incorrect_method_index, index);
                                       /* get the binary value              */
  item_index = integer_index->value;
  if (item_index < 0)                  /* not a valid index?                */
                                       /* raise an exception                */
    report_exception1(Error_Incorrect_method_index, index);
  if (item_index >= this->size)        /* out of possible range?            */
    return NULL;                       /* not found                         */
  element = ENTRY_POINTER(item_index); /* point to the item                 */
  if (element->previous == NOT_ACTIVE) /* not a real item?                  */
    element = NULL;                    /* no element found                  */
  return element;                      /* return this                       */
}

RexxObject *RexxList::value(
     RexxObject *index)                /* list index item                   */
/******************************************************************************/
/* Function:  Retrieve the value for a given list index                       */
/******************************************************************************/
{
  LISTENTRY *element;                  /* list element                      */
  RexxObject *result;                  /* returned result                   */

                                       /* locate this entry                 */
  element = this->getEntry(index, (RexxObject *)IntegerOne);
  if (element == NULL)                 /* not there?                        */
    return TheNilObject;               /* doesn't exist, return .NIL        */
  result = element->value;             /* get the value                     */
  if (result == OREF_NULL)             /* not there?                        */
    result = TheNilObject;             /* just return NIL                   */
  return result;                       /* return this item                  */
}

RexxObject *RexxList::put(
     RexxObject *value,                /* new value for the item            */
     RexxObject *index )               /* index of item to replace          */
/******************************************************************************/
/* Function:  Replace the value of an item already in the list.               */
/******************************************************************************/
{
  LISTENTRY *element;                  /* list element                      */

                                       /* locate this entry                 */
  element = this->getEntry(index, (RexxObject *)IntegerTwo);
  required_arg(value, ONE);            /* must have a value also            */
  if (element == NULL)                 /* not a valid index?                */
                                       /* raise an error                    */
    report_exception1(Error_Incorrect_method_index, index);
                                       /* replace the value                 */
  OrefSet(this->table, element->value, value);
  return OREF_NULL;                    /* return nothing at all             */
}

RexxObject *RexxList::section(
     RexxObject *index,                /* index of starting item            */
     RexxObject *count )               /* count of items to return          */
/******************************************************************************/
/* Function:  Create a sublist of this list                                   */
/******************************************************************************/
{
  LISTENTRY *element;                  /* list element                      */
  LONG   counter;                      /* object counter                    */
  RexxList *result;                    /* result list                       */

                                       /* locate this entry                 */
  element = this->getEntry(index, (RexxObject *)IntegerOne);
  if (count != OREF_NULL) {            /* have a count?                     */
                                       /* Make sure it's a good integer     */
    counter = count->requiredNonNegative(ARG_TWO);
  }
  else
    counter = 999999999;               /* just use largest possible count   */
  if (element == NULL)                 /* index doesn't exist?              */
                                       /* raise an error                    */
    report_exception1(Error_Incorrect_method_index, index);
  if (!OTYPE(List, this))              /* actually a list subclass?         */
                                       /* need to do this the slow way      */
    return this->sectionSubclass(element, counter);
  result = new RexxList;               /* create a new list                 */
  save(result);                        /* protect this                      */
                                       /* while still more to go and not at */
                                       /* the end of the list               */
  while (counter--) {                  /* while still more items            */
                                       /* add the this item to new list     */
    result->addLast(element->value);
    if (element->next == LIST_END)     /* this the last one?                */
      break;                           /* done sectioning                   */
                                       /* step to the next item             */
    element = ENTRY_POINTER(element->next);
  }
  discard(hold(result));               /* release the save lock             */
  return result;                       /* return the sectioned list         */
}

RexxObject *RexxList::sectionSubclass(
    LISTENTRY *element,                /* starting element                  */
    LONG       counter)                /* count of elements                 */
/******************************************************************************/
/* Function:  Rexx level section method                                       */
/******************************************************************************/
{
  RexxList *newList;                   /* returned array                    */

                                       /* create a new list                 */
  newList = (RexxList *)this->behaviour->getCreateClass()->sendMessage(OREF_NEW);
  save(newList);                       /* protect this                      */
                                       /* while still more to go and not at */
                                       /* the end of the list               */
  while (counter--) {                  /* while still more items            */
                                       /* add the this item to new list     */
    newList->sendMessage(OREF_INSERT, element->value);
    if (element->next == LIST_END)     /* this the last one?                */
      break;                           /* done sectioning                   */
                                       /* step to the next item             */
    element = ENTRY_POINTER(element->next);
  }
  discard_hold(newList);               /* release the save lock             */
  return newList;                      /* return the sectioned list         */
}

RexxObject *RexxList::add(
     RexxObject *value,                /* new value to add                  */
     RexxObject *index)                /* addition insertion index          */
/******************************************************************************/
/* Function:  Add a new element to the list at the given insertion point.     */
/*            TheNilObject indicates it should be added to the list end       */
/******************************************************************************/
{
  LISTENTRY *new_element;              /* new insertion element             */
  LISTENTRY *element;                  /* list element                      */
  long       new_index;                /* index of new inserted item        */

                                       /* make sure we have room to insert  */
  new_index = this->getFree();
                                       /* point to the actual element       */
  new_element = ENTRY_POINTER(new_index);
  if (index == TheNilObject)           /* inserting at the end?             */
    element = NULL;                    /* flag this as new                  */
  else {
                                       /* locate this entry                 */
    element = this->getEntry(index, (RexxObject *)IntegerOne);
    if (element == NULL)               /* index doesn't exist?              */
                                       /* raise an error                    */
      report_exception1(Error_Incorrect_method_index, index);
  }
  this->count++;                       /* increase our count                */
                                       /* set the value                     */
  OrefSet(this->table, new_element->value, value);
  if (element == NULL) {               /* adding at the end                 */
    if (this->last == LIST_END) {      /* first element added?              */
      this->first = new_index;         /* set this as the first             */
      this->last = new_index;          /* and the last                      */
      new_element->next = LIST_END;    /* this is the last element          */
      new_element->previous = LIST_END;/* in both directions                */
    }
    else {                             /* adding at the end                 */
                                       /* previous is current last          */
      new_element->previous = this->last;
      new_element->next = LIST_END;    /* nothing after this                */
                                       /* point to the end element          */
      element = ENTRY_POINTER(this->last);
      element->next = new_index;       /* point it at the new entry         */
      this->last = new_index;          /* this is the new last element      */
    }
  }
  else {                               /* have a real insertion point       */
                                       /* set the next pointer              */
    new_element->next = ENTRY_INDEX(element);

    if (element->previous == LIST_END) /* inserting at the front?           */
      this->first = new_index;         /* new first element                 */
    else                               /* fudge the previous element        */
      ENTRY_POINTER(element->previous)->next = new_index;
                                       /* insert before this element        */
    new_element->previous = element->previous;
    element->previous = new_index;     /* new previous one                  */
                                       /* point at the insertion point      */
    new_element->next = ENTRY_INDEX(element);
  }
                                       /* return this index item            */
  return (RexxObject *)new_integer(new_index);
}

void RexxList::addLast(
     RexxObject *value )               /* new value to add                  */
/******************************************************************************/
/* Function:  Add a new element to the tail of a list                         */
/******************************************************************************/
{
  LISTENTRY *new_element;              /* new insertion element             */
  long       new_index;                /* index of new inserted item        */

                                       /* make sure we have room to insert  */
  new_index = this->getFree();
                                       /* point to the actual element       */
  new_element = ENTRY_POINTER(new_index);
  this->count++;                       /* increase our count                */
                                       /* set the value                     */
  OrefSet(this->table, new_element->value, value);
  if (this->last == LIST_END) {        /* first element added?              */
    this->first = new_index;           /* set this as the first             */
    this->last = new_index;            /* and the last                      */
    new_element->next = LIST_END;      /* this is the last element          */
    new_element->previous = LIST_END;  /* in both directions                */
  }
  else {                               /* adding at the end                 */
    new_element->previous = this->last;/* previous is current last          */
    new_element->next = LIST_END;      /* nothing after this                */
                                       /* point to the end element          */
    new_element = ENTRY_POINTER(this->last);
    new_element->next = new_index;     /* point it at the new entry         */
    this->last = new_index;            /* this is the new last element      */
  }
}

void RexxList::addFirst(
     RexxObject *value)                /* new value to add                  */
/******************************************************************************/
/* Function:  Insert an element at the front of the list                      */
/******************************************************************************/
{
  LISTENTRY *element;                  /* list element                      */
  LISTENTRY *new_element;              /* new insertion element             */
  long       new_index;                /* index of new inserted item        */


  new_index = this->getFree();         /* make sure we have room to insert  */
                                       /* point to the actual element       */
  new_element = ENTRY_POINTER(new_index);
  this->count++;                       /* increase our count                */
                                       /* set the value                     */
  OrefSet(this->table, new_element->value, value);
  if (this->last == LIST_END) {        /* first element added?              */
    this->first = new_index;           /* set this as the first             */
    this->last = new_index;            /* and the last                      */
    new_element->next = LIST_END;      /* this is the last element          */
    new_element->previous = LIST_END;  /* in both directions                */
  }
  else {                               /* adding at the front               */

    new_element->next = this->first;   /* previous is current first         */
    new_element->previous = LIST_END;  /* nothing before this               */
                                       /* point to the first element        */
    element = ENTRY_POINTER(this->first);
    element->previous = new_index;     /* point it at the new entry         */
    this->first = new_index;           /* this is the new first element     */
  }
}


RexxObject *RexxList::insertRexx(
     RexxObject *value,                /* new value to add                  */
     RexxObject *index)                /* addition insertion index          */
/******************************************************************************/
/* Function:  Publicly accessible version of the list insert function.        */
/******************************************************************************/
{
  required_arg(value, ONE);            /* must have a value to insert       */
                                       /* go do the real insert             */
  return this->insert(value, index);
}


/**
 * Append an item after the last item in the list.
 *
 * @param value  The value to append.
 *
 * @return The index of the appended item.
 */
RexxObject *RexxList::append(RexxObject *value)
{
    required_arg(value, ONE);
    // this is just an insertion operation with an ommitted index.
    return insert(value, OREF_NULL);
}


RexxObject *RexxList::insert(
     RexxObject *value,                /* new value to add                  */
     RexxObject *index)                /* addition insertion index          */
/******************************************************************************/
/* Function:  Add a new element to the list at the given insertion point.     */
/*            TheNilObject indicates it should be added to the list fron      */
/******************************************************************************/
{
  LISTENTRY *element;                  /* list element                      */
  LISTENTRY *new_element;              /* new insertion element             */
  long       new_index;                /* index of new inserted item        */

                                       /* make sure we have room to insert  */
  new_index = this->getFree();
                                       /* point to the actual element       */
  new_element = ENTRY_POINTER(new_index);
  if (index == TheNilObject)           /* inserting at the front?           */
    element = NULL;                    /* flag this as new                  */
  else if (index == OREF_NULL) {       /* inserting at the end?             */
    if (this->last == LIST_END)        /* currently empty?                  */
      element = NULL;                  /* just use the front insert code    */
    else                               /* insert after the last element     */
      element = ENTRY_POINTER(this->last);
  }
  else {
                                       /* locate this entry                 */
    element = this->getEntry(index, (RexxObject *)IntegerOne);
    if (element == NULL)               /* index doesn't exist?              */
                                       /* raise an error                    */
      report_exception1(Error_Incorrect_method_index, index);
  }
  this->count++;                       /* increase our count                */
                                       /* set the value                     */
  OrefSet(this->table, new_element->value, value);
  if (element == NULL) {               /* adding at the front               */
    if (this->last == LIST_END) {      /* first element added?              */
      this->first = new_index;         /* set this as the first             */
      this->last = new_index;          /* and the last                      */
      new_element->next = LIST_END;    /* this is the last element          */
      new_element->previous = LIST_END;/* in both directions                */
    }
    else {                             /* adding at the front               */

      new_element->next = this->first; /* previous is current first         */
      new_element->previous = LIST_END;/* nothing before this               */
                                       /* point to the first element        */
      element = ENTRY_POINTER(this->first);
      element->previous = new_index;   /* point it at the new entry         */
      this->first = new_index;         /* this is the new first element     */
    }
  }
  else {                               /* have a real insertion point       */
                                       /* set the next pointer              */
    new_element->previous = ENTRY_INDEX(element);

    if (element->next == LIST_END)     /* inserting at the end?             */
      this->last = new_index;          /* new first element                 */
    else                               /* fudge the next element            */
      ENTRY_POINTER(element->next)->previous = new_index;
    new_element->next = element->next; /* insert after this element         */
    element->next = new_index;         /* new following one                 */
                                       /* point at the insertion point      */
    new_element->previous = ENTRY_INDEX(element);
  }
                                       /* return this index item            */
  return (RexxObject *)new_integer(new_index);
}

RexxObject *RexxList::remove(
     RexxObject *index)                /* index of item to remove           */
/******************************************************************************/
/* Function:  Remove a list item from the list                                */
/******************************************************************************/
{
                                       /* just remove the given index       */
  return this->primitiveRemove(this->getEntry(index, (RexxObject *)IntegerOne));
}

RexxObject *RexxList::primitiveRemove(
     LISTENTRY *element)               /* element to remove                 */
/******************************************************************************/
/* Function:  Remove a list item from the list                                */
/******************************************************************************/
{
  RexxObject *value;                   /* value of the removed item         */
  LISTENTRY *previous;                 /* previous entry                    */
  LISTENTRY *next;                     /* next entry                        */

  if (element == NULL)                 /* not a valid index?                */
    return TheNilObject;               /* just return .nil                  */

  value = element->value;              /* copy the value                    */
  if (element->next != LIST_END) {     /* not end of the list?              */
                                       /* point to the next entry           */
    next = ENTRY_POINTER(element->next);
    next->previous = element->previous;/* update the previous pointer       */
  }
  else
    this->last = element->previous;    /* need to update the last pointer   */
  if (element->previous != LIST_END) { /* not end of the list?              */
                                       /* point to the next entry           */
    previous = ENTRY_POINTER(element->previous);
    previous->next = element->next;    /* remove this from the chain        */
  }
  else
    this->first = element->next;       /* need to update the last pointer   */

  this->count--;                       /* "forget" we had this              */
  element->previous = NOT_ACTIVE;      /* no longer a good element          */
  element->next = this->free;          /* new head of the free chain        */
  this->free = ENTRY_INDEX(element);   /* this is the first free element    */

  return value;                        /* return the old value              */
}

RexxObject *RexxList::firstItem(void)
/******************************************************************************/
/* Function:  Return first item (value part) in the list                      */
/******************************************************************************/
{
  if (this->first == LIST_END)         /* empty list?                       */
    return TheNilObject;               /* give the empty indicator          */
  else                                 /* return the first value            */
    return ENTRY_POINTER(this->first)->value;
}

RexxObject *RexxList::lastItem(void)
/******************************************************************************/
/* Function:  Return last item (value part) in the list                       */
/******************************************************************************/
{
  if (this->last == LIST_END)          /* empty list?                       */
    return TheNilObject;               /* give the empty indicator          */
  else                                 /* return the first value            */
    return ENTRY_POINTER(this->last)->value;
}

RexxObject *RexxList::firstRexx(void)
/******************************************************************************/
/* Function:  Return index of the first list item                             */
/******************************************************************************/
{
  if (this->first == LIST_END)         /* no first item?                    */
    return TheNilObject;               /* just return a .nil index          */
  else {
                                       /* return the index as an integer    */
    return (RexxObject *)new_integer(this->first);
  }
}

RexxObject *RexxList::lastRexx(void)
/******************************************************************************/
/* Function:  Return index of the last list item                              */
/******************************************************************************/
{
  if (this->last == LIST_END)          /* no first item?                    */
    return TheNilObject;               /* just return a .nil index          */
  else {
                                       /* return the index as an integer    */
    return (RexxObject *)new_integer(this->last);
  }
}

RexxObject *RexxList::next(
     RexxObject *index)                /* index of the target item          */
/******************************************************************************/
/* Function:  Return the next item after the given indexed item               */
/******************************************************************************/
{
  LISTENTRY *element;                  /* current working entry             */
                                       /* locate this entry                 */
  element = this->getEntry(index, (RexxObject *)IntegerOne);
  if (element == NULL)                 /* not a valid index?                */
                                       /* raise an error                    */
    report_exception1(Error_Incorrect_method_index, index);

  if (element->next == LIST_END)       /* no next item?                     */
    return TheNilObject;               /* just return .nil                  */
  else {
                                       /* return the next item              */
    return (RexxObject *)new_integer(element->next);
  }
}

RexxObject *RexxList::previous(
     RexxObject *index)                /* index of the target item          */
/******************************************************************************/
/* Function:  Return the item previous to the indexed item                    */
/******************************************************************************/
{
  LISTENTRY *element;                  /* current working entry             */

                                       /* locate this entry                 */
  element = this->getEntry(index, (RexxObject *)IntegerOne);
  if (element == NULL)                 /* not a valid index?                */
                                       /* raise an error                    */
    report_exception1(Error_Incorrect_method_index, index);

  if (element->previous == LIST_END)   /* no previous item?                 */
    return TheNilObject;               /* just return .nil                  */
  else {                               /* return the previous item index    */
    return (RexxObject *)new_integer(element->previous);
  }
}

RexxObject *RexxList::hasIndex(
     RexxObject *index)                /* index of the target item          */
/******************************************************************************/
/* Function:  Return an index existence flag                                  */
/******************************************************************************/
{
  LISTENTRY *element;                  /* current working entry             */

                                       /* locate this entry                 */
  element = this->getEntry(index, (RexxObject *)IntegerOne);
                                       /* return an existence flag          */
  return (element!= NULL) ? (RexxObject *)TheTrueObject : (RexxObject *)TheFalseObject;
}

RexxArray *RexxList::requestArray()
/******************************************************************************/
/* Function:  Primitive level request('ARRAY') fast path                      */
/******************************************************************************/
{
  if (OTYPE(List, this))               /* primitive level object?           */
    return this->makeArray();          /* just do the makearray             */
  else                                 /* need to so full request mechanism */
    return (RexxArray *)send_message1(this, OREF_REQUEST, OREF_ARRAYSYM);
}


RexxArray *RexxList::makeArray(void)
/******************************************************************************/
/* Function:  Return all of the list values in an array                       */
/******************************************************************************/
{
    return this->allItems();           // this is just all of the array items.
}


/**
 * Return an array containing all elements contained in the list,
 * in sorted order.
 *
 * @return An array with the list elements.
 */
RexxArray *RexxList::allItems(void)
{
    // just iterate through the list, copying the elements.
    RexxArray *array = (RexxArray *)new_array(this->count);
    long   next = this->first;
    for (long  i = 1; i <= this->count; i++)
    {
        LISTENTRY *element = ENTRY_POINTER(next);
        array->put(element->value, i);
        next = element->next;
    }
    return array;
}


/**
 * Empty all of the items from a list.
 *
 * @return No return value.
 */
RexxObject *RexxList::empty()
{
    // just iterate through the list, copying the elements.
    RexxArray *array = (RexxArray *)new_array(this->count);
    while (this->first != LIST_END)
    {
        // get the list entry and remove the value
        LISTENTRY *element = ENTRY_POINTER(this->first);
        primitiveRemove(element);
    }
    return OREF_NULL;
}



/**
 * Test if a list is empty.
 *
 * @return True if the list is empty, false otherwise
 */
RexxObject *RexxList::isEmpty()
{
    return (count == 0) ? TheTrueObject : TheFalseObject;
}


/**
 * Return an array containing all elements contained in the list,
 * in sorted order.
 *
 * @return An array with the list elements.
 */
RexxArray *RexxList::allIndexes(void)
{
    // just iterate through the list, copying the elements.
    RexxArray *array = (RexxArray *)new_array(this->count);
    // this requires protecting, since we're going to be creating new
    // integer objects.
    save(array);
    long   next = this->first;
    for (long   i = 1; i <= this->count; i++)
    {
        LISTENTRY *element = ENTRY_POINTER(next);
        array->put((RexxObject *)new_integer(next), i);
        next = element->next;
    }
    discard_hold(array);
    return array;
}


/**
 * Return the index of the first item with a matching value
 * in the list.  Returns .nil if the object is not found.
 *
 * @param target The target object.
 *
 * @return The index of the item, or .nil.
 */
RexxObject *RexxList::index(RexxObject *target)
{
    // we require the index to be there.
    required_arg(target, ONE);

    // ok, now run the list looking for the target item
    long next = this->first;
    for (long   i = 1; i <= this->count; i++)
    {
        LISTENTRY *element = ENTRY_POINTER(next);
        // if we got a match, return the item
        if (target->equalValue(element->value))
        {
            return new_integer(next);
        }
        next = element->next;
    }
    // no match
    return TheNilObject;
}


/**
 * Tests whether there is an object with the given value in the
 * list.
 *
 * @param target The target value.
 *
 * @return .true if there is a match, .false otherwise.
 */
RexxObject *RexxList::hasItem(RexxObject *target)
{
    // we require the index to be there.
    required_arg(target, ONE);

    // ok, now run the list looking for the target item
    long next = this->first;
    for (long   i = 1; i <= this->count; i++)
    {
        LISTENTRY *element = ENTRY_POINTER(next);
        // if we got a match, return the item
        if (target->equalValue(element->value))
        {
            return TheTrueObject;
        }
        next = element->next;
    }
    // no match
    return TheFalseObject;
}


/**
 * Removes an item from the collection.
 *
 * @param target The target value.
 *
 * @return The target item.
 */
RexxObject *RexxList::removeItem(RexxObject *target)
{
    // we require the index to be there.
    required_arg(target, ONE);

    // ok, now run the list looking for the target item
    long next = this->first;
    for (long   i = 1; i <= this->count; i++)
    {
        LISTENTRY *element = ENTRY_POINTER(next);
        // if we got a match, return the item
        if (target->equalValue(element->value))
        {
            // remove this item
            return primitiveRemove(element);
        }
        next = element->next;
    }
    // no match
    return TheNilObject;
}


RexxObject *RexxList::indexOfValue(
     RexxObject *value)
/*****************************************************************************/
/* Function:  Return the index for the value that was passed in              */
/*            or OREF_NULL                                                   */
/*****************************************************************************/
{
  RexxObject *index = OREF_NULL;
  RexxObject *last_index;
  RexxObject *this_value = OREF_NULL;
                                       /* get the last index in the list     */
  last_index = this->lastRexx();
                                       /* if there was any items in list     */
  if (last_index != TheNilObject)
                                       /* loop thru the list looking for the */
                                       /* specified value                    */
    for (index = this->firstRexx();
        ((this_value = this->value(index)) != value) && index != last_index;
         index = this->next(index));
                                       /* return the index if the value was  */
                                       /* in the list                        */
  if (this_value == value)
    return index;
                                       /* otherwise return nothing           */
  return OREF_NULL;
}

RexxArray  *RexxList::makeArrayIndices()
/******************************************************************************/
/* Function:  Return an array containing all of the list indices              */
/******************************************************************************/
{
  RexxArray *array;                    /* returned array value              */
  LISTENTRY *element;                  /* current working entry             */
  long        i;                       /* loop counter                      */
  long        next;                    /* next item to process              */

                                       /* allocate proper sized array       */
  array = (RexxArray *)new_array(this->count);
  save(array);                         /* lock the array                    */
  next = this->first;                  /* point to the first element        */
  for (i = 1; i <= this->count; i++) { /* step through the array elements   */
    element = ENTRY_POINTER(next);     /* get the next item                 */
                                       /* create an index item              */
    array->put((RexxObject *)new_integer(next), i);
    next = element->next;              /* get the next pointer              */
  }
  discard(hold(array));                /* release the GC lock               */
  return array;                        /* return the array element          */
}

RexxSupplier *RexxList::supplier(void)
/******************************************************************************/
/* Function:  Create a supplier object for this list                          */
/******************************************************************************/
{
  RexxArray *values;                   /* array of value items              */
  RexxArray *indices;                  /* array of index items              */

                                       /* and all of the indices            */
  indices = this->makeArrayIndices();
  values = this->makeArray();          /* get the list values               */
                                       /* return the supplier values        */
  return (RexxSupplier *)new_supplier(values, indices);
}

RexxObject *RexxList::itemsRexx(void)
/******************************************************************************/
/* Function:  Return the size of the list as an integer                       */
/******************************************************************************/
{
                                       /* return the item count             */
  return (RexxObject *)new_integer(this->count);
}

void *RexxList::operator new(size_t size)
/******************************************************************************/
/* Function:  Construct and initialized a new list item                       */
/******************************************************************************/
{
  RexxList   * newList;                /* newly created list                */

                                       /* Get new object                    */
  newList = (RexxList *)new (INITIAL_LIST_SIZE, size) RexxListTable;
                                       /* Give new object its behaviour     */
  BehaviourSet(newList, TheListBehaviour);
                                       /* set the default hash value        */
  newList->hashvalue = HASHOREF(newList);
  newList->init();                     /* finish initializing               */
  return newList;                      /* return the new list item          */
}

RexxList *RexxListClass::newRexx(
    RexxObject **init_args,
    size_t       argCount)
/******************************************************************************/
/* Function:  Construct and initialized a new list item                       */
/******************************************************************************/
{
  RexxList *newList;                   /* newly created list                */

                                       /* get a new directory               */
                                       /* NOTE:  this does not use the      */
                                       /* macro version because the class   */
                                       /* object might actually be for a    */
                                       /* subclass                          */
  newList = new RexxList;
                                       /* Give new object its behaviour     */
  BehaviourSet(newList, this->instanceBehaviour);
  if (this->uninitDefined()) {
    newList->hasUninit();
  }
                                       /* Initialize the new list instance  */
  newList->sendMessage(OREF_INIT, init_args, argCount);
  return newList;                      /* return the new list item          */
}

RexxList *RexxListClass::classOf(
     RexxObject **args,                /* array of list items               */
     size_t       argCount)            /* size of the argument array        */
/******************************************************************************/
/* Function:  Create a new list containing the given list items               */
/******************************************************************************/
{
  LONG     size;                       /* size of the array                 */
  LONG     i;                          /* loop counter                      */
  RexxList *newList;                   /* newly created list                */
  RexxObject *item;                    /* item to add                       */

  if (TheListClass == this ) {         /* creating an internel list item?   */
    size = argCount;                   /* get the array size                */
    newList  = new RexxList;           /* get a new list                    */
    save(newList);                     /* protect from garbage collection   */
    for (i = 0; i < size; i++) {       /* step through the array            */
      item = args[i];                  /* get the next item                 */
      if (item == OREF_NULL) {         /* omitted item?                     */
        discard(newList);              /* release the new list              */
                                       /* raise an error on this            */
        report_exception1(Error_Incorrect_method_noarg, new_integer(i + 1));
      }
                                       /* add this to the list end          */
      newList->addLast(item);
    }
  }
  else {
    size = argCount;                   /* get the array size                */
                                       /* get a new list                    */
    newList = (RexxList *)send_message0(this, OREF_NEW);
    save(newList);                     /* protect from garbage collection   */
    for (i = 0; i < size; i++) {       /* step through the array            */
      item = args[i];                  /* get the next item                 */
      if (item == OREF_NULL) {         /* omitted item?                     */
        discard(newList);              /* release the new list              */
                                       /* raise an error on this            */
        report_exception1(Error_Incorrect_method_noarg, new_integer(i + 1));
      }
                                       /* add this to the list end          */
      send_message1(newList, OREF_INSERT, item);
    }
  }
  discard(hold(newList));              /* release the collection lock       */
  return newList;                      /* give back the list                */
}

