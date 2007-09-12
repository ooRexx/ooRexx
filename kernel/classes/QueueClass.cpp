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
/* REXX Kernel                                                  QueueClass.c     */
/*                                                                            */
/* Primitive Queue Class                                                      */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "QueueClass.hpp"
#include "IntegerClass.hpp"
#include "SupplierClass.hpp"

RexxObject *RexxQueue::pullRexx()
/******************************************************************************/
/* Function:  Pull an item off of the stack, returning .nil if no items       */
/*            are available                                                   */
/******************************************************************************/
{
  RexxObject *item;                    /* removed item                      */

  item = this->pop();                  /* remove the first item             */
  if (item == OREF_NULL)               /* nothing there?                    */
    item = TheNilObject;               /* use .nil instead                  */
  return item;                         /* return the pulled item            */
}

RexxObject *RexxQueue::pushRexx(RexxObject *item)
                                                                                                           /* item to push onto the queue       */
/******************************************************************************/
/* Function:  Push an item onto the queue                                     */
/******************************************************************************/
{

  required_arg(item, ONE);             /* make sure we have an argument     */
  this->push(item);                    /* push onto the queue               */
  return OREF_NULL;                    /* return nothing                    */
}



/**
 * Append an item after the last item in the list.
 *
 * @param value  The value to append.
 *
 * @return The index of the appended item.
 */
RexxObject *RexxQueue::append(RexxObject *item)
{

  required_arg(item, ONE);             /* make sure we have an argument     */
  this->queue(item);                   /* push onto the queue               */
  // the insertion index is the position.
  return new_integer(this->count);
}


RexxObject *RexxQueue::queueRexx(RexxObject *item)
/******************************************************************************/
/* Function:  Push an item onto the queue                                     */
/******************************************************************************/
{
  required_arg(item, ONE);             /* make sure we have an argument     */
                                       /* add to the end of the queue       */
  this->queue(item);
  return OREF_NULL;                    /* return nothing                    */
}


LISTENTRY *RexxQueue::locateEntry(RexxObject *index, RexxObject *position)
/******************************************************************************/
/* Function:  Resolve a queue index argument to a list index                  */
/******************************************************************************/
{
    // we must have an index
    if (index == OREF_NULL)
    {
        report_exception1(Error_Incorrect_method_noarg, position);
    }

    // and it must be a valid whole number
    RexxInteger *integerIndex = (RexxInteger *)REQUEST_INTEGER(index);
    if (integerIndex == TheNilObject)
    {
        report_exception1(Error_Incorrect_method_queue_index, index);
    }
    // and positive
    wholenumber_t item_index = integerIndex->value;
    if (item_index < 1)
    {
        report_exception1(Error_Incorrect_method_queue_index, index);
    }

    // we need to iterate through the entries to locate this
    long listIndex = this->first;
    while (listIndex != LIST_END)
    {
        // have we reached the entry?  return the item
        item_index--;
        if (item_index == 0)
        {
            return ENTRY_POINTER(listIndex);
        }
        // step to the next entry
        listIndex = ENTRY_POINTER(listIndex)->next;
    }
    return NULL;          // this queue item not found
}


RexxObject *RexxQueue::put(
    RexxObject *value,                 /* value to add                      */
    RexxObject *index)                 /* target index                      */
/******************************************************************************/
/* Function:  Replace the value of an item already in the queue.              */
/******************************************************************************/
{
  required_arg(value, ONE);            /* must have a value also            */
                                       /* locate this entry                 */
  LISTENTRY *list_index = this->locateEntry(index, IntegerTwo);
  if (list_index == NULL)              /* not a valid index?                */
                                       /* raise an error                    */
    report_exception1(Error_Incorrect_method_queue_index, index);
  OrefSet(this->table, list_index->value, value);
  return OREF_NULL;                    /* return nothing at all             */
}

RexxObject *RexxQueue::at(RexxObject *index)
                                                                                                                    /* queue index item                  */
/******************************************************************************/
/* Function:  Retrieve the value for a given queue index                      */
/******************************************************************************/
{
                                       /* locate this entry                 */
  LISTENTRY *list_index = this->locateEntry(index, IntegerOne);
  if (list_index == NULL)              /* not a valid index?                */
    return TheNilObject;               /* doesn't exist, return .NIL        */
  RexxObject *result = list_index->value;  /* get the list entry                */
  if (result == OREF_NULL)             /* not there?                        */
    result = TheNilObject;             /* just return NIL                   */
  return (RexxObject *)result;         /* return this item                  */
}


/**
 * Insert an item into the queue at a specific index position.
 *
 * @param value  The value to insert.
 * @param index  The index.  This can be omitted, which inserts at the end, .nil
 *               which inserts at the beginning, or a valid existing index.
 *
 * @return The inserted object's index.
 */
RexxObject *RexxQueue::insert(RexxObject *value, RexxObject *index)
{
  LISTENTRY *element;                  /* list element                      */
  LISTENTRY *new_element;              /* new insertion element             */
  long       new_index;                /* index of new inserted item        */

  required_arg(value, ONE);            /* must have a value to insert       */

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
    element = this->locateEntry(index, IntegerTwo);
    if (element == NULL)               /* index doesn't exist?              */
                                       /* raise an error                    */
      report_exception1(Error_Incorrect_method_queue_index, index);
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
  return (RexxObject *)new_integer(entryToIndex(new_index));
}


RexxObject *RexxQueue::remove(RexxObject *index)
/******************************************************************************/
/* Function:  Remove a given queue item                                       */
/******************************************************************************/
{
                                       /* locate this entry                 */
  LISTENTRY *list_index = this->locateEntry(index, IntegerOne);
                                       /* remove from the list              */
  return this->primitiveRemove(list_index);
}

RexxObject *RexxQueue::hasindex(RexxObject *index)
                                                                                                                    /* queue index item                  */
/******************************************************************************/
/* Function:  Return an index existence flag                                  */
/******************************************************************************/
{
                                       /* locate this entry                 */
  LISTENTRY *list_index = this->locateEntry(index, IntegerOne);
                                       /* return an existence flag          */
  return (RexxObject *) (( list_index != NULL) ? TheTrueObject : TheFalseObject);
}

RexxObject *RexxQueue::peek()
/******************************************************************************/
/* Function:  Return the first element of the queue without removing it       */
/******************************************************************************/
{
    return firstItem();
}


RexxObject *RexxQueue::supplier()
/******************************************************************************/
/* Function:  Create a supplier for a queue object                            */
/******************************************************************************/
{
  RexxArray *values;                   /* queue values array                */

  values = this->makeArray();          /* convert into an array             */
                                       /* turn this into a supplier         */
  return (RexxObject *)new_supplier(values, OREF_NULL);
}


/**
 * Retrieve an array containing all index values for the queue.
 * For queue classes, the indices are the integers 1 - items(), so
 * this is generally a pretty silly way to access this.
 *
 * @return An array containing all of the queue indices.
 */
RexxArray *RexxQueue::allIndexes()
{
    // create an array and protect it.
    arraysize_t size = this->items();

    RexxArray *result = new_array(size);
    save(result);

    // now just make an array containing each index value.
    for (arraysize_t i = 1; i <= size; i++)
    {
        result->put(new_integer(i), i);
    }

    discard_hold(result);
    return result;
}


/**
 * Return the index of the first item with a matching value
 * in the list.  Returns .nil if the object is not found.
 *
 * @param target The target object.
 *
 * @return The index of the item, or .nil.
 */
RexxObject *RexxQueue::index(RexxObject *target)
{
    // we require the index to be there.
    required_arg(target, ONE);

    // ok, now run the list looking for the target item
    long next = this->first;
    for (long i = 1; i <= this->count; i++)
    {
        LISTENTRY *element = ENTRY_POINTER(next);
        // if we got a match, return the item
        if (target->equalValue(element->value))
        {
            // queue indices are positional.
            return new_integer(i);
        }
        next = element->next;
    }
    // no match
    return TheNilObject;
}


RexxObject *RexxQueue::firstRexx(void)
/******************************************************************************/
/* Function:  Return index of the first list item                             */
/******************************************************************************/
{
    if (this->first == LIST_END)
    {
        return TheNilObject;     // empty queue is the .nil object
    }

    else
    {
        return (RexxObject *)IntegerOne;   // first index is always one
    }
}


RexxObject *RexxQueue::lastRexx(void)
/******************************************************************************/
/* Function:  Return index of the last list item                              */
/******************************************************************************/
{
    if (this->last == LIST_END)
    {
        return TheNilObject;        // no last item is an empty queue...return .nil

    }
    else
    {
                                    // return the item count as the final index
        return (RexxObject *)new_integer(this->items());
    }
}

RexxObject *RexxQueue::next(
     RexxObject *index)                /* index of the target item          */
/******************************************************************************/
/* Function:  Return the next item after the given indexed item               */
/******************************************************************************/
{
    LISTENTRY *element;                /* current working entry             */
                                       /* locate this entry                 */
    element = this->locateEntry(index, (RexxObject *)IntegerOne);
    if (element == NULL)                 /* not a valid index?                */
    {
        report_exception1(Error_Incorrect_method_queue_index, index);
    }

    if (element->next == LIST_END)     /* no next item?                     */
    {
        return TheNilObject;           /* just return .nil                  */
    }
    else
    {
                                       /* return the next item              */
        return (RexxObject *)new_integer(entryToIndex(element->next));
    }
}


RexxObject *RexxQueue::previous(
     RexxObject *index)                /* index of the target item          */
/******************************************************************************/
/* Function:  Return the item previous to the indexed item                    */
/******************************************************************************/
{
  LISTENTRY *element;                  /* current working entry             */

                                       /* locate this entry                 */
  element = this->locateEntry(index, (RexxObject *)IntegerOne);
  if (element == NULL)                 /* not a valid index?                */
                                       /* raise an error                    */
    report_exception1(Error_Incorrect_method_queue_index, index);

  if (element->previous == LIST_END)   /* no previous item?                 */
    return TheNilObject;               /* just return .nil                  */
  else {                               /* return the previous item index    */
    return (RexxObject *)new_integer(entryToIndex(element->previous));
  }
}


/**
 * Convert an entry index into a queue index relative to the
 * beginning.
 *
 * @param target The target index position.
 *
 * @return The queue index value.
 */
long RexxQueue::entryToIndex(long target)
{
    long current = this->first;
    long counter = 0;
    while (current != LIST_END)
    {
        counter++;
        if (current == target)
        {
            return counter;
        }

        current = ENTRY_POINTER(current)->next;
    }

    return 0;
}


RexxObject *RexxQueue::newRexx(RexxObject **init_args, size_t argCount)
/******************************************************************************/
/* Function:  Create an instance of a queue                                   */
/******************************************************************************/
{
  RexxObject *newObject;               /* newly created queue object        */

  newObject =  new RexxQueue;          /* get a new queue                   */
                                       /* Initialize the new list instance  */
  BehaviourSet(newObject, ((RexxClass *)this)->instanceBehaviour);
  if (((RexxClass *)this)->uninitDefined()) {
    newObject->hasUninit();
  }

  newObject->sendMessage(OREF_INIT, init_args, argCount);
  return (RexxObject *)newObject;      /* return the new object             */
}

RexxQueue *RexxQueue::ofRexx(
     RexxObject **args,                /* array of list items               */
     size_t       argCount)            /* size of the argument array        */
/******************************************************************************/
/* Function:  Create a new queue containing the given items                   */
/******************************************************************************/
{
  LONG     size;                       /* size of the array                 */
  LONG     i;                          /* loop counter                      */
  RexxQueue *newQueue;                 /* newly created list                */
  RexxObject *item;                    /* item to add                       */

  if (TheQueueClass == ((RexxClass *)this)) {        /* creating an internel list item?   */
    size = argCount;                   /* get the array size                */
    newQueue = new RexxQueue;          /* get a new list                    */
    save(newQueue);                    /* protect from garbage collection   */
    for (i = 0; i < size; i++) {       /* step through the array            */
      item = args[i];                  /* get the next item                 */
      if (item == OREF_NULL) {         /* omitted item?                     */
        discard(newQueue);             /* release the new list              */
                                       /* raise an error on this            */
        report_exception1(Error_Incorrect_method_noarg, new_integer(i + 1));
      }
                                       /* add this to the list end          */
      newQueue->addLast(item);
    }
  }
  else {
    size = argCount;                   /* get the array size                */
                                       /* get a new list                    */
    newQueue = (RexxQueue *)send_message0(this, OREF_NEW);
    save(newQueue);                    /* protect from garbage collection   */
    for (i = 0; i < size; i++) {       /* step through the array            */
      item = args[i];                  /* get the next item                 */
      if (item == OREF_NULL) {         /* omitted item?                     */
        discard(newQueue);             /* release the new list              */
                                       /* raise an error on this            */
        report_exception1(Error_Incorrect_method_noarg, new_integer(i + 1));
      }
                                       /* add this to the list end          */
      send_message1(newQueue, OREF_QUEUENAME, item);
    }
  }
  discard(hold(newQueue));             /* release the collection lock       */
  return newQueue;                     /* give back the list                */
}



void *RexxQueue::operator new(size_t size)
/******************************************************************************/
/* Function:  Create an instance of a queue                                   */
/******************************************************************************/
{
  RexxQueue  * newQueue;               /* newly created list                */

                                       /* Get new object                    */
  newQueue = (RexxQueue *)new (INITIAL_LIST_SIZE, size) RexxListTable;
                                       /* Give new object its behaviour     */
  BehaviourSet(newQueue, TheQueueBehaviour);
                                       /* set the default hash value        */
  newQueue->hashvalue = HASHOREF(newQueue);
  newQueue->init();                    /* finish initializing               */
  return newQueue;                     /* return the new list item          */
}

