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
/* REXX Kernel                                               QueueClass.cpp   */
/*                                                                            */
/* Primitive Queue Class                                                      */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "QueueClass.hpp"
#include "IntegerClass.hpp"
#include "SupplierClass.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *RexxQueue::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxQueue::createInstance()
{
    CLASS_CREATE(Queue, "Queue", RexxClass);
}


/**
 * Create an instance of the Queue class.
 *
 * @param size   The base size of the object.
 *
 * @return An initialized queue object.
 */
void *RexxQueue::operator new(size_t size)
{
    RexxQueue *newQueue = (RexxQueue *)new (INITIAL_LIST_SIZE, size) ListTable;
    // these are handled a little differently, so we have to explicitly set our behavior (for now)
    newQueue->setBehaviour(TheQueueBehaviour);
    newQueue->init();
    return newQueue;
}


RexxObject *RexxQueue::pullRexx()
/******************************************************************************/
/* Function:  Pull an item off of the stack, returning .nil if no items       */
/*            are available                                                   */
/******************************************************************************/
{
    return resultOrNil(pop());
}

RexxObject *RexxQueue::pushRexx(RexxObject *item)

/******************************************************************************/
/* Function:  Push an item onto the queue                                     */
/******************************************************************************/
{

  requiredArgument(item, ARG_ONE);             /* make sure we have an argument     */
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

  requiredArgument(item, ARG_ONE);             /* make sure we have an argument     */
  this->queue(item);                   /* push onto the queue               */
  // the insertion index is the position.
  return new_integer(this->count);
}


RexxObject *RexxQueue::queueRexx(RexxObject *item)
/******************************************************************************/
/* Function:  Push an item onto the queue                                     */
/******************************************************************************/
{
  requiredArgument(item, ARG_ONE);             /* make sure we have an argument     */
                                       /* add to the end of the queue       */
  this->queue(item);
  return OREF_NULL;                    /* return nothing                    */
}


LISTENTRY *RexxQueue::locateEntry(RexxObject *_index, RexxObject *position)
/******************************************************************************/
/* Function:  Resolve a queue index argument to a list index                  */
/******************************************************************************/
{
    // we must have an index
    if (_index == OREF_NULL)
    {
        reportException(Error_Incorrect_method_noarg, position);
    }

    // and it must be a valid whole number
    RexxInteger *integerIndex = (RexxInteger *)REQUEST_INTEGER(_index);
    if (integerIndex == TheNilObject)
    {
        reportException(Error_Incorrect_method_queue_index, _index);
    }
    // and positive
    wholenumber_t item_index = integerIndex->wholeNumber();
    if (item_index < 1)
    {
        reportException(Error_Incorrect_method_queue_index, _index);
    }

    // we need to iterate through the entries to locate this
    size_t listIndex = this->first;
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
    RexxObject *_value,                 /* value to add                      */
    RexxObject *_index)                 /* target index                      */
/******************************************************************************/
/* Function:  Replace the value of an item already in the queue.              */
/******************************************************************************/
{
    requiredArgument(_value, ARG_ONE);           /* must have a value also            */
                                         /* locate this entry                 */
    LISTENTRY *list_index = this->locateEntry(_index, IntegerTwo);
    if (list_index == NULL)              /* not a valid index?                */
    {
        /* raise an error                    */
        reportException(Error_Incorrect_method_queue_index, _index);
    }
    OrefSet(this->table, list_index->value, _value);
    return OREF_NULL;                    /* return nothing at all             */
}

RexxObject *RexxQueue::at(RexxObject *_index)
/******************************************************************************/
/* Function:  Retrieve the value for a given queue index                      */
/******************************************************************************/
{
    /* locate this entry                 */
    LISTENTRY *list_index = this->locateEntry(_index, IntegerOne);
    if (list_index == NULL)              /* not a valid index?                */
    {
        return TheNilObject;               /* doesn't exist, return .NIL        */
    }
    RexxObject *result = list_index->value;  /* get the list entry                */
    if (result == OREF_NULL)             /* not there?                        */
    {
        result = TheNilObject;             /* just return NIL                   */
    }
    return(RexxObject *)result;         /* return this item                  */
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
RexxObject *RexxQueue::insert(RexxObject *_value, RexxObject *_index)
{
    LISTENTRY *element;                  /* list element                      */
    LISTENTRY *new_element;              /* new insertion element             */
    size_t     new_index;                /* index of new inserted item        */

    requiredArgument(_value, ARG_ONE);           /* must have a value to insert       */

                                         /* make sure we have room to insert  */
    new_index = this->getFree();
    /* point to the actual element       */
    new_element = ENTRY_POINTER(new_index);
    if (_index == TheNilObject)          /* inserting at the front?           */
    {
        element = NULL;                    /* flag this as new                  */
    }
    else if (_index == OREF_NULL)
    {      /* inserting at the end?             */
        if (this->last == LIST_END)        /* currently empty?                  */
        {
            element = NULL;                  /* just use the front insert code    */
        }
        else                               /* insert after the last element     */
        {
            element = ENTRY_POINTER(this->last);
        }
    }
    else
    {
        /* locate this entry                 */
        element = this->locateEntry(_index, IntegerTwo);
        if (element == NULL)               /* index doesn't exist?              */
        {
            /* raise an error                    */
            reportException(Error_Incorrect_method_queue_index, _index);
        }
    }
    this->count++;                       /* increase our count                */
                                         /* set the value                     */
    OrefSet(this->table, new_element->value, _value);
    if (element == NULL)
    {               /* adding at the front               */
        if (this->last == LIST_END)
        {      /* first element added?              */
            this->first = new_index;         /* set this as the first             */
            this->last = new_index;          /* and the last                      */
            new_element->next = LIST_END;    /* this is the last element          */
            new_element->previous = LIST_END;/* in both directions                */
        }
        else
        {                             /* adding at the front               */

            new_element->next = this->first; /* previous is current first         */
            new_element->previous = LIST_END;/* nothing before this               */
                                             /* point to the first element        */
            element = ENTRY_POINTER(this->first);
            element->previous = new_index;   /* point it at the new entry         */
            this->first = new_index;         /* this is the new first element     */
        }
    }
    else
    {                               /* have a real insertion point       */
                                    /* set the next pointer              */
        new_element->previous = ENTRY_INDEX(element);

        if (element->next == LIST_END)     /* inserting at the end?             */
        {
            this->last = new_index;          /* new first element                 */
        }
        else                               /* fudge the next element            */
        {
            ENTRY_POINTER(element->next)->previous = new_index;
        }
        new_element->next = element->next; /* insert after this element         */
        element->next = new_index;         /* new following one                 */
                                           /* point at the insertion point      */
        new_element->previous = ENTRY_INDEX(element);
    }
    /* return this index item            */
    return new_integer(entryToIndex(new_index));
}


RexxObject *RexxQueue::remove(RexxObject *_index)
/******************************************************************************/
/* Function:  Remove a given queue item                                       */
/******************************************************************************/
{
                                       /* locate this entry                 */
  LISTENTRY *list_index = this->locateEntry(_index, IntegerOne);
                                       /* remove from the list              */
  return this->primitiveRemove(list_index);
}

RexxObject *RexxQueue::hasindex(RexxObject *_index)
/******************************************************************************/
/* Function:  Return an index existence flag                                  */
/******************************************************************************/
{
                                       /* locate this entry                 */
  LISTENTRY *list_index = this->locateEntry(_index, IntegerOne);
                                       /* return an existence flag          */
  return booleanObject(list_index != NULL);
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
    ArrayClass *values = this->makeArray();          /* convert into an array             */
    /* turn this into a supplier         */
    return new_supplier(values, OREF_NULL);
}


/**
 * Retrieve an array containing all index values for the queue.
 * For queue classes, the indices are the integers 1 - items(), so
 * this is generally a pretty silly way to access this.
 *
 * @return An array containing all of the queue indices.
 */
ArrayClass *RexxQueue::allIndexes()
{
    // create an array and protect it.
    size_t arraysize = this->items();

    ArrayClass *result = new_array(arraysize);
    ProtectedObject p(result);

    // now just make an array containing each index value.
    for (size_t i = 1; i <= arraysize; i++)
    {
        result->put(new_integer(i), i);
    }
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
    requiredArgument(target, ARG_ONE);

    // ok, now run the list looking for the target item
    size_t nextEntry = this->first;
    for (size_t i = 1; i <= this->count; i++)
    {
        LISTENTRY *element = ENTRY_POINTER(nextEntry);
        // if we got a match, return the item
        if (target->equalValue(element->value))
        {
            // queue indices are positional.
            return new_integer(i);
        }
        nextEntry = element->next;
    }
    // no match
    return TheNilObject;
}


RexxObject *RexxQueue::firstRexx()
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


RexxObject *RexxQueue::lastRexx()
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
     RexxObject *_index)               /* index of the target item          */
/******************************************************************************/
/* Function:  Return the next item after the given indexed item               */
/******************************************************************************/
{
    LISTENTRY *element;                /* current working entry             */
                                       /* locate this entry                 */
    element = this->locateEntry(_index, (RexxObject *)IntegerOne);
    if (element == NULL)                 /* not a valid index?                */
    {
        reportException(Error_Incorrect_method_queue_index, _index);
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
     RexxObject *_index)               /* index of the target item          */
/******************************************************************************/
/* Function:  Return the item previous to the indexed item                    */
/******************************************************************************/
{
    /* locate this entry                 */
    LISTENTRY *element = this->locateEntry(_index, (RexxObject *)IntegerOne);
    if (element == NULL)                 /* not a valid index?                */
    {
        /* raise an error                    */
        reportException(Error_Incorrect_method_queue_index, _index);
    }

    if (element->previous == LIST_END)   /* no previous item?                 */
    {
        return TheNilObject;               /* just return .nil                  */
    }
    else
    {                               /* return the previous item index    */
        return(RexxObject *)new_integer(entryToIndex(element->previous));
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
size_t RexxQueue::entryToIndex(size_t target)
{
    size_t current = this->first;
    size_t counter = 0;
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

/**
 * Create a sublist of this queue.
 *
 * @param _index The starting index
 * @param _count The size of the subset to extract.
 *
 * @return A new instance of this class containing the subsection items.
 */
RexxObject *RexxQueue::section(RexxObject *_index, RexxObject *_count )
{
    size_t counter;                      /* object counter                    */
                                         /* locate this entry                 */
    LISTENTRY *element = this->locateEntry(_index, (RexxObject *)IntegerOne);
    if (_count != OREF_NULL)
    {           /* have a count?                     */
                /* Make sure it's a good integer     */
        counter = _count->requiredNonNegative(ARG_TWO);
    }
    else
    {
        counter = 999999999;               /* just use largest possible count   */
    }
    if (element == NULL)                 /* index doesn't exist?              */
                                         /* raise an error                    */
        reportException(Error_Incorrect_method_index, _index);
    if (!isOfClass(Queue, this))              /* actually a queue subclass?         */
    {
        /* need to do this the slow way      */
        return this->sectionSubclass(element, counter);
    }
    RexxQueue *result = new RexxQueue;     /* create a new queue instance       */
    ProtectedObject p(result);
    /* while still more to go and not at */
    /* the end of the list               */
    while (counter--> 0)
    {               /* while still more items            */
                    /* add the this item to new list     */
        result->addLast(element->value);
        if (element->next == LIST_END)     /* this the last one?                */
        {
            break;                           /* done sectioning                   */
        }
                                             /* step to the next item             */
        element = ENTRY_POINTER(element->next);
    }
    return result;                       /* return the sectioned list         */
}



/**
 * Section method used when dealing with a subclass of the Queue class.
 *
 * @param element The starting element.
 * @param counter the number of items to extract.
 *
 * @return A new instance of the target class containing the
 *         subsection elements.
 */
RexxObject *RexxQueue::sectionSubclass(LISTENTRY *element, size_t counter)
{
    ProtectedObject r;
    /* create a new list                 */
    this->behaviour->getOwningClass()->sendMessage(OREF_NEW, r);
    RexxQueue *newQueue = (RexxQueue *)(RexxObject *)r;
    /* while still more to go and not at */
    /* the end of the list               */
    while (counter-- > 0)                /* while still more items            */
    {
        /* add the this item to new list     */
        newQueue->sendMessage(OREF_INSERT, element->value);
        if (element->next == LIST_END)     /* this the last one?                */
        {
            break;                           /* done sectioning                   */
        }
                                             /* step to the next item             */
        element = ENTRY_POINTER(element->next);
    }
    return newQueue;                      /* return the sectioned list         */
}


/**
 * Create an instance of a queue object from Rexx code.
 *
 * @param init_args The pointer to the arguments.
 * @param argCount  The count of arguments.
 *
 * @return A new instance of the queue class.
 */
RexxObject *RexxQueue::newRexx(RexxObject **init_args, size_t argCount)
/******************************************************************************/
/* Function:  Create an instance of a queue                                   */
/******************************************************************************/
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxObject *newObj =  new RexxQueue;
    ProtectedObject p(newObj);

    // handle Rexx class completion
    classThis->completeNewObject(newObj, init_args, argCount);
    return newObj;
}


/**
 * Create an instance of the queue class and fill it with objects.
 *
 * @param args     The argument pointer.
 * @param argCount The count of arguments.
 *
 * @return An instance of the queue class populated with objects.
 */
RexxQueue *RexxQueue::ofRexx(RexxObject **args, size_t argCount)
{
    // create a queue object.
    RexxQueue *newQueue = (RexxQueue *)RexxQueue::newRexx(NULL, 0);
    ProtectedObject p(newQueue);

    for (size_t i = 0; i < argCount; i++)
    {
        // add each item, but ommitted arguments are an error.
        RexxObject *item = args[i];
        if (item == OREF_NULL)
        {
            reportException(Error_Incorrect_method_noarg, i + 1);
        }
        newQueue->addLast(item);
    }
    return newQueue;
}




