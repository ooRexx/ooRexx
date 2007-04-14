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
  this->push(item);                    /* push onto the queue               */
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

RexxObject *RexxQueue::getEntry(RexxObject *index, RexxObject *position)
/******************************************************************************/
/* Function:  Resolve a queue index argument to a list index                  */
/******************************************************************************/
{
  RexxInteger *integerIndex;           /* requested integer index           */
  LONG     item_index;                 /* converted item number             */
  RexxObject *listIndex;               /* located list index                */

  if (index == OREF_NULL)              /* must have one here                */
                                       /* else an error                     */
    report_exception1(Error_Incorrect_method_noarg, position);
                                       /* force to integer form             */
  integerIndex = (RexxInteger *)REQUEST_INTEGER(index);
  if (integerIndex == TheNilObject)    /* doesn't exist?                    */
                                       /* raise an exception                */
    report_exception1(Error_Incorrect_method_index, index);
                                       /* get the binary value              */
  item_index = integerIndex->value;
  if (item_index < 1)                  /* not a valid index?                */
                                       /* raise an exception                */
    report_exception1(Error_Incorrect_method_index, index);
                                       /* get the first index               */
  listIndex = this->firstRexx();
                                       /* locate the item                   */
  while (listIndex != TheNilObject) {  /* loop while still items            */
    item_index--;                      /* count this one                    */
    if (item_index == 0)               /* count run out?                    */
      return (RexxObject *)listIndex;  /* bingo, got what we need           */
                                       /* step to the next item             */
    listIndex = this->next(listIndex);
  }
  return OREF_NULL;                    /* list item not found               */
}

RexxObject *RexxQueue::put(
    RexxObject *value,                 /* value to add                      */
    RexxObject *index)                 /* target index                      */
/******************************************************************************/
/* Function:  Replace the value of an item already in the queue.              */
/******************************************************************************/
{
  RexxObject *list_index;              /* list index                        */

                                       /* locate this entry                 */
  list_index = this->getEntry(index, IntegerTwo);
  required_arg(value, ONE);            /* must have a value also            */
  if (list_index == NULL)              /* not a valid index?                */
                                       /* raise an error                    */
    report_exception1(Error_Incorrect_method_index, index);
                                       /* just do a put into the list       */
  this->RexxList::put(value, list_index);
  return OREF_NULL;                    /* return nothing at all             */
}

RexxObject *RexxQueue::at(RexxObject *index)
                                                                                                                    /* queue index item                  */
/******************************************************************************/
/* Function:  Retrieve the value for a given queue index                      */
/******************************************************************************/
{
  RexxObject *result;                  /* returned result                   */
  RexxObject *list_index;              /* list index                        */

                                       /* locate this entry                 */
  list_index = this->getEntry(index, IntegerOne);
  if (list_index == NULL)              /* not a valid index?                */
    return TheNilObject;               /* doesn't exist, return .NIL        */
  result = this->value(list_index);    /* get the list entry                */
  if (result == OREF_NULL)             /* not there?                        */
    result = TheNilObject;             /* just return NIL                   */
  return (RexxObject *)result;         /* return this item                  */
}

RexxObject *RexxQueue::remove(RexxObject *index)
/******************************************************************************/
/* Function:  Remove a given queue item                                       */
/******************************************************************************/
{
  RexxObject *list_index;              /* list index                        */

                                       /* locate this entry                 */
  list_index = this->getEntry(index, IntegerOne);
  if (list_index == NULL)              /* not a valid index?                */
    return TheNilObject;               /* doesn't exist, return .NIL        */
                                       /* remove from the list              */
  return this->RexxList::remove(list_index);
}

RexxObject *RexxQueue::hasindex(RexxObject *index)
                                                                                                                    /* queue index item                  */
/******************************************************************************/
/* Function:  Return an index existence flag                                  */
/******************************************************************************/
{
  RexxObject *list_index;               /* list index                        */

                                       /* locate this entry                 */
  list_index = this->getEntry(index, IntegerOne);
                                       /* return an existence flag          */
  return (RexxObject *) (( list_index != NULL) ? TheTrueObject : TheFalseObject);
}

RexxObject *RexxQueue::peek()
/******************************************************************************/
/* Function:  Return the first element of the queue without removing it       */
/******************************************************************************/
{
  RexxObject *first;                   /* index of first item               */
  RexxObject *item;                    /* returned item                     */

                                       /* get index of first item           */
  first = this->firstRexx();
  item = TheNilObject;                 /* default no first item             */
  if (first != TheNilObject)           /* have an item in the list?         */
                                       /* retrieve the item without removal */
    item = this->value(first);
  return (RexxObject *)item;           /* return the first item             */
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

