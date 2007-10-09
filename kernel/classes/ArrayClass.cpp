/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                               ArrayClass.c     */
/*                                                                            */
/* Primitive Array Class                                                      */
/*                                                                            */
/*                                                                            */
/*   This Array class functions in two ways.  One as an auto-extending array  */
/* and as a static array.  The static methods are used inside the kernel      */
/* since we always know the exact size of the array we want and will place    */
/* those elements into the array, so the static sized methods are optimized.  */
/*                                                                            */
/*   The auto-extending methods are for the OREXX level behaviour.  They      */
/* may also be used inside the kernel if that behaviour is required.          */
/*                                                                            */
/*   Any of the methods can be used on an array.  The behaviour of an array   */
/* depends on the methods used on the array, since there is only one type     */
/* of array object, and its data is the same irreguardless of the methods     */
/* used on it.                                                                */
/*                                                                            */
/*   Object creation macros:                                                  */
/*     new_array(s) - Create an array of size s, initial size of array is     */
/*                    set to s.  This array will be used as a static array    */
/*     new_array1(a1) - Create array of size 1 and put a1 in it.              */
/*                      same as an array~of(a1)                               */
/*     new_array2(a1,a2) - Create array of size 2 and put a1 and a2 in it     */
/*                      same as an array~of(a1,a2)                            */
/*     new_array3(a1, a2, a3)      Same as new_array2 but 3 elements.         */
/*     new_array4(a1, a2, a3, a4)    "   "    "        "  4   "               */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
#define INCL_REXX_STREAM

#include <stdlib.h>
#include "RexxCore.h"
#include "RexxActivity.hpp"
#include "IntegerClass.hpp"
#include "SupplierClass.hpp"
#include "ArrayClass.hpp"
#include "MutableBufferClass.hpp"

extern ACTIVATION_SETTINGS *current_settings;

void RexxArray::init(size_t size, size_t maxSize)
/******************************************************************************/
/* Function:  Initialize an array                                             */
/******************************************************************************/
{
  ClearObject(this);                   /* initialize the object             */
  this->hashvalue = HASHOREF(this);
  this->arraySize = size;
  this->maximumSize = maxSize;
                                       /* no expansion yet, use ourself     */
  OrefSet(this, this->expansionArray, this);
}

RexxObject  *RexxArray::copy(void)
/******************************************************************************/
/* Function:   create a copy of array and expansion array.                    */
/******************************************************************************/
{
  RexxArray *newArray;                 /* new array object                  */

                                       /* make a copy of ourself            */
  newArray = (RexxArray *) this->RexxObject::copy();
                                       /* this array contain the data?      */
  if (this->expansionArray != OREF_NULL && this->expansionArray != this) {
                                       /* no, make sure we get a copy of    */
                                       /* array containing data.            */
    newArray->setExpansion(this->expansionArray->copy());
  }
  else
                                       /* no, make sure we get a copy of    */
                                       /* array containing data.            */
    newArray->setExpansion(newArray);  /* make sure we point to ourself!    */
  return (RexxObject *)newArray;       /* return the new array              */
}

void RexxArray::live(void)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  RexxObject **arrayPtr;
  RexxObject **endPtr;
  setUpMemoryMark
  memory_mark(this->dimensions);
  memory_mark(this->objectVariables);
                                       /* mark expanded array               */
  memory_mark(this->expansionArray);
  for (arrayPtr = this->objects, endPtr = arrayPtr + this->arraySize; arrayPtr < endPtr; arrayPtr++)
    memory_mark(*arrayPtr);
  cleanUpMemoryMark
}

void RexxArray::liveGeneral(void)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  RexxObject **arrayPtr;

  setUpMemoryMarkGeneral
  memory_mark_general(this->dimensions);
  memory_mark_general(this->objectVariables);
  memory_mark_general(this->expansionArray);

  for (arrayPtr = this->objects; arrayPtr < this->objects + this->arraySize; arrayPtr++)
    memory_mark_general(*arrayPtr);
  cleanUpMemoryMarkGeneral
}

void RexxArray::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxArray)

   size_t i;

   flatten_reference(newThis->dimensions, envelope);
   flatten_reference(newThis->objectVariables, envelope);
   flatten_reference(newThis->expansionArray, envelope);
   for (i = 0; i < this->arraySize; i++)
       flatten_reference(newThis->objects[i], envelope);

  cleanUpFlatten
}


void RexxArray::put(RexxObject * eref, size_t pos)
/******************************************************************************/
/* Function:  Place an object in the array                                    */
/******************************************************************************/
{
  OrefSet(this->expansionArray, (this->data())[pos -1], eref);
}

RexxObject  *RexxArray::putRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Performs REXX-level puts to arrays.                             */
/******************************************************************************/
{
  RexxObject *value;                   /* assigned value                    */
  size_t position;                     /* array position                    */

  value = arguments[0];                /* get the value to assign           */
                                       /* no real value?                    */
  if (argCount == 0 || value == OREF_NULL)
                                       /* this is an error                  */
    missing_argument(ARG_ONE);         /* this is an error                  */
                                       /* go validate the index             */
                                       /* have array expanded if necessary  */
  position = this->validateIndex(arguments + 1, argCount - 1, 2, RaiseBoundsInvalid | ExtendUpper | RaiseBoundsTooMany);

  this->put(value, position);          /* set the new value                 */
  return OREF_NULL;                    /* Make sure RESULT gets dropped     */
}


/**
 * Empty all of the items from an array.
 *
 * @return No return value.
 */
RexxObject *RexxArray::empty()
{

    // if not working with an oldspace object (VERY likely), we can just use memset to clear
    // everything.
    if (!OldSpace(this))
    {
        memset(this->data(), '\0', sizeof(RexxObject *) * this->arraySize);
    }
    else
    {
        // sigh, we have to use OrefSet
        for (size_t i = 0; i < this->arraySize; i++)
        {

            OrefSet(this, this->objects[i], OREF_NULL);
        }
    }
    return OREF_NULL;     // no real return value
}


/**
 * Test if an array is empty.
 *
 * @return True if the array is empty, false otherwise
 */
RexxObject *RexxArray::isEmpty()
{
    return (numItems() == 0) ? TheTrueObject : TheFalseObject;
}



/**
 * Append an item after the last item in the array.
 *
 * @param value  The value to append.
 *
 * @return The index of the appended item.
 */
RexxObject  *RexxArray::append(RexxObject *value)
{
    required_arg(value, ONE);

    // this is not intended for multi-dimensional arrays since they can't expand
    if (this->dimensions != OREF_NULL && this->dimensions->size() != 1)
    {
        CurrentActivity->reportException(Error_Incorrect_method_array_dimension, CHAR_APPEND);
    }

    RexxObject *lastIndex = lastRexx();

    size_t newIndex;

    // empty array, so just insert at the first element
    if (lastIndex == TheNilObject)
    {
        newIndex = 1;
    }
    else
    {
        // the index requires validation
        newIndex = ((RexxInteger *)lastIndex)->value + 1;
    }

    ensureSpace(newIndex);
    put(value, newIndex);
    return new_integer(newIndex);
}


RexxObject  *RexxArray::getRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Performs REXX-level gets from arrays.                           */
/******************************************************************************/
{
  size_t position;                     /* array position                    */
  RexxObject *result;                  /* returned result                   */

                                       /* go validate the index             */
  position = this->validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid);
  if (position == NO_LONG)             /* not found?                        */
    result = TheNilObject;             /* just return .nil                  */
  else {                               /* return that element               */
    result = *(this->data() + position - 1);
    if (result == OREF_NULL)           /* no object there?                  */
      result = TheNilObject;           /* just return .nil                  */
  }
  return result;                       /* return the result                 */
}

RexxObject *RexxArray::remove(size_t index)
/******************************************************************************/
/* Function:  Remove an item from the array                                   */
/******************************************************************************/
{
  RexxObject *result;                  /* removed object                    */

                                       /* within the bounds?                */
  if (index > 0 && index <= this->size() && *(this->data() + index - 1) != OREF_NULL) {
                                       /* get the item                      */
    result = *(this->data() + index - 1);
                                       /* clear it out                      */
    OrefSet(this->expansionArray, *(this->data() + index - 1), OREF_NULL);
    return result;                     /* and return                        */
  }
  else
    return OREF_NULL;                  /* return nothing                    */
}

RexxObject  *RexxArray::removeRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Remove an item from the array and return the item.  .nil        */
/*            is returned if the item does not exist                          */
/******************************************************************************/
{
  RexxObject *result;                  /* returned result                   */
  size_t position;                     /* array position                    */

                                       /* go validate the index             */
  position = this->validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid);
  if (position == NO_LONG) {           /* position out of bound?            */
    result = TheNilObject;             /* yup, return .nil.                 */
  }
  else {
                                       /* get the current element           */
    result = *(this->data() + position - 1);
                                       /* remove the item from the array    */
    OrefSet(this->expansionArray, *(this->data() + position - 1), OREF_NULL);
    if (result == OREF_NULL)           /* no existing value?                */
      result = TheNilObject;           /* return .nil instead               */
  }
  return result;                       /* return this value                 */
}

size_t RexxArray::numItems(void)
/******************************************************************************/
/* Function:  Return count of actual items in an array                        */
/******************************************************************************/
{
  size_t count;                        /* actual count of items in array    */
  size_t index;
  RexxArray *realArray;                /* array item pointer                */

  count = 0;                           /* no items yet                      */
  realArray = this->expansionArray;    /* Get array with data               */
                                       /* loop through all array items      */
  for (index = 0; index < realArray->arraySize; index++) {
                                       /* have a real item here?            */
    if (realArray->objects[index] != OREF_NULL) {
      count++;                         /* bump the item counter             */
    }
  }
  return count;                        /* return the count object           */
}

RexxObject  *RexxArray::items()
/******************************************************************************/
/* Function:  Return count of actual items in an array                        */
/******************************************************************************/
{
   size_t tempNumItems;

   tempNumItems = this->numItems();    /* place holder to invoke new_integer */
   return (RexxObject *)(new_integer(tempNumItems));
}

size_t RexxArray::getDimension(void)
/******************************************************************************/
/* Function:  Query array dimension information                               */
/******************************************************************************/
{
  if (this->dimensions == OREF_NULL)   /* no dimensions array?              */
    return 1;                          /* one dimension array               */
  else
    return this->dimensions->size();   /* return size of dimensions array   */
}

RexxObject *RexxArray::dimension(      /* query dimensions of an array      */
     RexxObject *dimension)            /* dimension to query                */
/******************************************************************************/
/* Function:  Query array dimension information                               */
/******************************************************************************/
{
  RexxObject *result;                  /* query result                      */
  size_t  position;                    /* requested position                */
  size_t  tempSize;

  if (dimension == OREF_NULL) {        /* non-specific query?               */
    if (this->dimensions == OREF_NULL) {
      if (this->size() == 0)
                                       /* unknown dimension                 */
        result = (RexxObject*)IntegerZero;
      else
                                       /* one dimension array               */
        result = (RexxObject *)IntegerOne;
    }
    else {                             /* return size of dimensions array   */
      tempSize = this->dimensions->size();
      result = (RexxObject *)new_integer(tempSize);
    }
  }
  else {
                                       /* convert to a number               */
    position = dimension->requiredPositive(ARG_ONE);
                                       /* asking for dimension of single?   */
    if (this->dimensions == OREF_NULL) {
      if (position == 1) {             /* first dimension?                  */
        tempSize = this->size();       /* get the size */
                                       /* just give back the size           */
        result = (RexxObject *)new_integer(tempSize);
      }
      else
                                       /* no size in this dimension         */
        result = (RexxObject *)IntegerZero;
    }
                                       /* out of range?                     */
    else if (position > this->dimensions->size())
                                       /* no size in this dimension         */
      result = (RexxObject *)IntegerZero;
    else                               /* return the specific dimension     */
      result = this->dimensions->get(position);
  }
  return result;                       /* return the final result           */
}

RexxObject *RexxArray::supplier()
/******************************************************************************/
/* Function:  create a supplier for this array                                */
/******************************************************************************/
{
  size_t      items;                   /* items in the array                */
  RexxArray  *values;                  /* returned values                   */
  RexxArray  *indexes;                 /* returned index values             */
  size_t      size;                    /* array size                        */
  RexxObject *item;                    /* inserted value item               */
  size_t      i;                       /* loop counter                      */
  size_t      count;                   /* count added to the array          */

  size = this->size();                 /* get the array size                */
  items = this->numItems();            /* and the actual count in the array */

  values = new_array(items);           /* get the values array              */
  indexes = new_array(items);          /* and an index array                */

  save(values);                        /* save the values array             */
  save(indexes);                       /* save the indexes also             */

  count = 1;                           /* next place to add                 */
  for (i = 1; i <= size; i++) {        /* loop through the array            */
    item = this->get(i);               /* get the next item                 */
    if (item != OREF_NULL) {           /* got an item here                  */
      values->put(item, count);        /* copy over to the values array     */

                                       /* add the index location            */
      indexes->put((RexxObject*)convertIndex(i), count);
      count++;                         /* step the location                 */
    }
  }
  discard(hold(values));               /* release the lock                  */
  discard(hold(indexes));              /* on both items                     */
                                       /* create a supplier object          */
  return (RexxObject *)new_supplier(values, indexes);
}


void  RexxArray::setExpansion(RexxObject * expansion)
/******************************************************************************/
/* Function:  Set a new expansion array item                                  */
/******************************************************************************/
{
  OrefSet(this, this->expansionArray, (RexxArray *)expansion);
}

RexxInteger *RexxArray::available(size_t position)
/******************************************************************************/
/* Function:  Determine if a position element is "out-of-bounds"              */
/******************************************************************************/
{
  return (RexxInteger *) ((position < this->size())  ? TheTrueObject : TheFalseObject);
}

size_t  RexxArray::validateIndex(      /* validate an array index           */
    RexxObject **index,                /* array index (possibly multi-dim)  */
    size_t       indexCount,           /* size of the index array           */
    size_t       start,                /* starting point on the array       */
    size_t       bounds_error)         /* raise errors on out-of-bounds     */
/******************************************************************************/
/* Function:  Process and validate a potentially multi-dimensional array      */
/*            index.  If the index is out of bounds in any dimension it will  */
/*            either return NO_LONG or raise an error, depending on the bounds*/
/*            checking parameter.                                             */
/******************************************************************************/
{
  RexxObject *value;                   /* individual index value            */
  size_t  position;                    /* converted binary value            */
  size_t  numsubs;                     /* number of subscripts              */
  size_t  i;                           /* loop counter                      */
  size_t  multiplier;                  /* accumlation factor                */
  size_t  offset;                      /* accumulated offset                */
  size_t  dimension;                   /* current working dimension         */
  size_t  numSize;                     /* temporary long variable           */


  // do we really have a single index item given as an array?
  if (indexCount == 1 && index[0] != OREF_NULL && OTYPE(Array, index[0]))
  {
      // we process this exactly the same way, but swap the count and
      // pointers around to be the array data.
      RexxArray *indirect = (RexxArray *)index[0];
      indexCount = indirect->numItems();
      index = indirect->data();
  }

                                       /* Is this array one-dimensional?    */
  if (this->dimensions == OREF_NULL || this->dimensions->size() == 1) {
                                       /* Too many subscripts?  Say so.     */
    if (indexCount > 1) {
                                       /* should the array be extended?     */
      if ((bounds_error & ExtendUpper) && this->dimensions == OREF_NULL) {
                                       /* anytyhing in the array?           */
                                       /* or explicitly created array with 0*/
                                       /* elements (.array~new(0))          */
        if (this->size() != 0) {
                                       /* Yes, number of dims can't change  */
                                       /* report apropriate bounds          */
          report_exception1(Error_Incorrect_method_maxsub, IntegerOne);
        }
        else {
                                       /* its empty, entendarray for new siz*/
                                       /* extend the array.                 */
          this->extendMulti(index, indexCount, start);
                                       /* Call us again to get position, now*/
                                       /* That the array is extended.       */
          return this->validateIndex(index, indexCount, start, bounds_error);
        }
      }

      else if (bounds_error & RaiseBoundsTooMany) {
                                       /* anytyhing in the array?           */
                                       /* or explicitly created array with 0*/
                                       /* elements (.array~new(0))          */
        if (this->dimensions != OREF_NULL || this->size() != 0)
                                       /* report apropriate bounds          */
          report_exception1(Error_Incorrect_method_maxsub, IntegerOne);
        else
          return NO_LONG;              /* just report not here              */
      }
      else
        return NO_LONG;                /* not fixed yet, but don't complain */
    }
                                       /* Too few? subscripts?  Say so.     */
    else if (indexCount == 0)
                                       /* report apropriate bounds          */
      report_exception1(Error_Incorrect_method_minarg, new_integer(start));
                                       /* validate integer index            */
    position = index[0]->requiredPositive(start);
                                       /* out of bounds?                    */
    if (position > this->size() ) {
      if (position >= MAX_FIXEDARRAY_SIZE)
        report_exception(Error_Incorrect_method_array_too_big);
                                       /* are we to expand the array?       */
      if (bounds_error & ExtendUpper) {
                                       /* yes, compute amount to expand     */
        this->extend(position - this->size());

      }
                                       /* need to raise an error?           */
      else if (bounds_error & RaiseBoundsUpper)
        report_exception1(Error_Incorrect_method_array, new_integer(position));
      else
        position = NO_LONG;            /* just return indicator             */
    }
  }
  else {                               /* multidimensional array            */
                                       /* get the number of subscripts      */
    numsubs = indexCount;
    numSize = this->dimensions->size();/* Get the size of dimension */
                                       /* right number of subscripts?       */
    if (numsubs == numSize) {
      multiplier = 1;                  /* multiply by 1 for first dimension */
      offset = 0;                      /* no accumulated offset             */

      for (i = numsubs; i > 0; i--) {  /* loop through the dimensions       */

        value = index[i - 1];

        if (value == OREF_NULL)        /* not given?                        */
                                       /* this is an error too              */
          report_exception1(Error_Incorrect_method_noarg, new_integer(i + start));
                                       /* validate integer index            */
        position = value->requiredPositive(i);
                                       /* get the current dimension         */
        dimension = ((RexxInteger *)this->dimensions->get(i))->value;
        if (position > dimension) {    /* too large?                        */
                                       /* should the array be extended?     */
          if (bounds_error & ExtendUpper) {
                                       /* go extend it.                     */
            this->extendMulti(index, indexCount, start);
                                       /* Call us again to get position, now*/
                                       /* That the array is extended.       */
            return this->validateIndex(index, indexCount, start, bounds_error);
          }
                                       /* need to raise an error?           */
          else if (bounds_error & RaiseBoundsUpper)
            report_exception1(Error_Incorrect_method_array, new_integer(position));
          else
            return NO_LONG;            /* just return indicator             */
        }
                                       /* calculate next offset             */
        offset += multiplier * (position - 1);
        multiplier *= dimension;       /* step the multiplier               */
      }
      position = offset + 1;           /* get accumulated position          */
    }
                                       /* Not enough subscripts?            */
    else if (numsubs < numSize)
      report_exception1(Error_Incorrect_method_minsub, new_integer(numSize));
    else                               /* Must be too many subscripts       */
                                       /* should the array be extended?     */
#ifdef EXTEND_DIMENSIONS
     if (bounds_error & ExtendUpper) {
                                       /* anytyhing in the array?           */
       if (this->size() != 0) {
                                       /* Yes, number of dims can't change  */
                                       /* report apropriate bounds          */
         report_exception1(Error_Incorrect_method_maxarg, new_integer(numSize));
       }
       else {
                                       /* array empty, extend the array     */
         this->extendMuti(index, indexCount, start);
                                       /* Call us again to get position, now*/
                                       /* That the array is extended.       */
         return this->validateIndex(index, indexCount, start, bounds_error);
       }
     }
     else {
      report_exception1(Error_Incorrect_method_maxsub, new_integer(numSize));
     }
#else
      report_exception1(Error_Incorrect_method_maxsub, new_integer(numSize));
#endif
  }
  return position;                     /* return the position               */
}


/**
 * Make sure that the array has been expanded to sufficient
 * size for a primitive put operation.
 *
 * @param newSize The new required size.
 */
void RexxArray::ensureSpace(size_t newSize)
{
    /* out of bounds?                    */
    if (newSize > this->size())
    {
        if (newSize >= MAX_FIXEDARRAY_SIZE)
        {
            report_exception(Error_Incorrect_method_array_too_big);
        }
        /* yes, compute amount to expand     */
        this->extend(newSize - this->size());

    }
}



RexxInteger  *RexxArray::sizeRexx()
/******************************************************************************/
/* Function:  Return the array size as an integer object                      */
/******************************************************************************/
{
  return (RexxInteger *)new_integer(this->size());
}

RexxArray *  RexxArray::section(size_t start, size_t end)
/******************************************************************************/
/* Function:  Section an array for a given size                               */
/******************************************************************************/
{
  size_t newSize;                      /* Size for new array.               */
  size_t i;                            /* counter for loop.                 */
  RexxArray *newArray;                 /* The new array.                    */
  RexxArray *oldArray;                 /* The old array.                    */

  if (start == 0)                      /* starting position specified?      */
    start = 1;                         /* nope, start at begining           */

                                       /* ending position omitted           */
                                       /* or, End pos past end of array?    */
  if (end == 0 || end > this->size())
    end = this->size();                /* yes,  use end of array            */
  if (start <= end) {                  /* is start before end?              */
    newSize = end + 1 - start;         /* yes, compute new size             */
                                       /* Get new array, of needed size     */
    newArray = (RexxArray *)new_array(newSize);
    if (!OldSpace(newArray)) {         /* working with a new space obj      */
                                       /* yes, we can do a memcpy           */
     memcpy(newArray->data(), &(this->expansionArray->objects[start-1]), sizeof(RexxObject *) * newSize);
    }
    else {
      oldArray = this->expansionArray;
                                       /* its Old Space, need to do real put*/
      for (i = 1; i <= newSize; i++)
          newArray->put(oldArray[start+i-2],i);
    }
  } else {
                                       /* return 0 element array            */
    newArray = (RexxArray *)new_array(0);
  }
  return newArray;                     /* return the newly created array    */
}

RexxObject *RexxArray::sectionRexx(
            RexxObject * start,
            RexxObject * end)
/******************************************************************************/
/* Function:  Rexx level section method                                       */
/******************************************************************************/
{
  size_t nstart, nend, i;
  RexxArray *rref;


  required_arg(start, ONE);            /* need a start position             */
                                       /* Start specified - check it out    */
  nstart = start->requiredPositive(ARG_ONE);
  if (end == OREF_NULL)                /* If no end position specified,     */
    nend = this->size();               /* Defaults to last element          */
  else {                               /* End specified - check it out      */
    nend = end->requiredNonNegative(ARG_TWO);
  }
                                       /* multidimensional array?           */
  if (this->dimensions != OREF_NULL && this->dimensions->size() != 1)
                                       /* this is an error                  */
    report_exception(Error_Incorrect_method_section);
  if (!OTYPE(Array, this))             /* actually an array subclass?       */
                                       /* need to do this the slow way      */
    return this->sectionSubclass(nstart, nend);
  if (nstart > this->size())           /* too big?                          */

    rref = (RexxArray *)(((RexxArray *)TheNullArray)->copy());
                                       /* return zero elements              */
  else {
                                       /* go past the bounds?               */
    if (nend > this->size() - nstart + 1)
      nend = this->size() - nstart + 1;/* truncate to the end               */
    if (nend == 0)                     /* requesting zero?                  */
                                       /* return zero elements              */
      rref = (RexxArray *)(((RexxArray *)TheNullArray)->copy());
    else {                             /* real sectioning to do             */
                                       /* create a new array                */
      rref = (RexxArray *)new_array(nend);
      for (i = 1; i <= nend; i++) {    /* loop through the elements         */
                                       /* copy an element                   */
        rref->put(this->get(nstart + i - 1), i);
      }
    }
  }
  return rref;                         /* return the new array              */
}

RexxObject *RexxArray::sectionSubclass(
    size_t start,                      /* starting element                  */
    size_t end )                       /* ending element                    */
/******************************************************************************/
/* Function:  Rexx level section method                                       */
/******************************************************************************/
{
  size_t i;                            /* loop counter                      */
  RexxArray *newArray;                 /* returned array                    */

  if (start > this->size())            /* too big?                          */
                                       /* return a zero element one         */
    newArray = (RexxArray *)this->behaviour->getCreateClass()->sendMessage(OREF_NEW, IntegerZero);
  else {
    if (end > this->size() - start + 1)/* go past the bounds?               */
      end = this->size() - start + 1;  /* truncate to the end               */
    if (end == 0)                      /* requesting zero?                  */
                                       /* return a zero element one         */
      newArray = (RexxArray *)this->behaviour->getCreateClass()->sendMessage(OREF_NEW, IntegerZero);
    else {                             /* real sectioning to do             */
                                       /* create a new array                */
      newArray = (RexxArray *)this->behaviour->getCreateClass()->sendMessage(OREF_NEW, new_integer(end));
      save(newArray);                  /* protect the new one               */
      for (i = 1; i <= end; i++) {     /* loop through the elements         */
                                       /* copy an element                   */
        newArray->sendMessage(OREF_PUT, this->get(start + i - 1), new_integer(i));
      }
      discard_hold(newArray);          /* and release the lock on this      */
    }
  }
  return newArray;                     /* return the new array              */
}

RexxObject  *RexxArray::firstRexx(void)
/******************************************************************************/
/* Function:  Retrieve the first element index from the array as an integer   */
/*            object                                                          */
/******************************************************************************/
{
  size_t i;                            /* counter for the loop              */
  RexxObject *result;
  RexxObject **thisObject;
  size_t  arraySize;                   /* size of the array                 */

                                       /* get the address of the first      */
                                       /*element in the array               */
  thisObject = this->expansionArray->objects;
  arraySize = this->size();            /* get the size of the array         */
                                       /* find first position in the        */
                                       /*array with data                    */
  for (i = 0; i < arraySize && thisObject[i] == OREF_NULL; i++);

  if (i == arraySize)                  /* is array empty                    */
    result = TheNilObject;             /* return nil object                 */
  else
                                       /* return index of the first entry   */
    result = (RexxObject *)convertIndex(i + 1);

  return result;
}

RexxObject  *RexxArray::lastRexx(void)
/******************************************************************************/
/* Function:  Return the index of the last array item as an integer object    */
/******************************************************************************/
{
  size_t i;                            /* counter for the loop              */
  RexxObject *result;
  RexxObject **thisObject;

                                       /* get the address of the first      */
                                       /*element in the array               */
  thisObject = this->data();
                                       /* find last position in the array   */
                                       /*with data                          */
  for (i = this->size() ; i > 0 && thisObject[i-1] == OREF_NULL; i--);

  if (i == 0)                           /* if array is empty?                */
    result = TheNilObject;             /* return nil object                 */
  else
                                       /* return index to the last entry    */
    result = (RexxObject *)convertIndex(i);
  return result;
}

RexxObject  *RexxArray::nextRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Return the next entry after a given array index                 */
/******************************************************************************/
{
  size_t i;
  RexxObject *result;
  RexxObject **thisObject;
  size_t arraySize;                    /* size of the array                 */
                                       /* go validate the index             */
  size_t position = this->validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid);
  // out of bounds results in the .nil object
  if (position == NO_LONG)
  {
      return TheNilObject;
  }
                                       /* get the address of the first      */
                                       /*element in the array               */
  thisObject = this->data();
  arraySize = this->size();            /* get the size of the array         */
                                       /* find next entry in the array with */
                                       /*data                               */
  for (i = position; i < arraySize && thisObject[i] == OREF_NULL; i++);

  if (i >= this->size())
    result = TheNilObject;             /* return nil object                 */
  else
                                       /* return index of the next entry    */
    result = (RexxObject *)convertIndex(i + 1);

  return result;
}

RexxObject  *RexxArray::previousRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Return the index preceeding a given index                       */
/******************************************************************************/
{
  size_t i;                            /* counter for the loop              */
  RexxObject *result;
  RexxObject **thisObject;
  size_t  arraySize;                   /* size of the array                 */

  size_t position = this->validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid);
                                       /* get the index object into an      */
                                       /*integer object                     */
  i = position;

  arraySize = this->size();            /* get the size of the array         */

  if (i > arraySize)                   /* beyond the size of the array?     */
                                       /* set i to one more than the last   */
                                       /*entry                              */
    i = arraySize;
  else
    i = i-1;                           /* Account for 0 based 'C' arrays    */

                                       /* get the address of the first      */
                                       /*element in the array               */
  thisObject = this->expansionArray->objects;
                                       /* find previous entry in the        */
                                       /*array with data                    */
  for (; i > 0 && thisObject[i-1] == OREF_NULL;
       i--);

  if (i == 0)
    result = TheNilObject;             /* return nil object                 */
  else
                                       /* return the index to the           */
                                       /*previous entry                     */
    result = (RexxObject *)convertIndex(i);

  return result;
}


RexxInteger *RexxArray::hasIndex(RexxObject *index)
/******************************************************************************/
/* Function:  Determine if an individual array element exists                 */
/******************************************************************************/
{
  size_t i;

  if (OTYPE(Integer,index) && (i = ((RexxInteger *)index)->value) > 0 && i <= this->size() &&
      *(this->data()+i-1) != OREF_NULL)
    return (RexxInteger *)TheTrueObject;
  else
    return (RexxInteger *)TheFalseObject;
}

RexxObject  *RexxArray::hasIndexRexx(RexxObject **index, size_t indexCount)
/******************************************************************************/
/*  Function:  True if array has an entry for the index, false otherwise      */
/*  Note:  This routine should not raise an error, regardless of the indices  */
/*         being used.  The only error produced is if no parms were passed.   */
/******************************************************************************/
{
  size_t position;                     /* array position                    */

                                       /* go validate the index             */
  position = this->validateIndex(index, indexCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid);
  if (position == NO_LONG)             /* not found?                        */
                                       /* this is false                     */
    return (RexxObject *)TheFalseObject;
  else {                               /* check the position                */
                                       /* have a real entry?                */
    if (*(this->data() + position - 1) != OREF_NULL)
                                       /* got a true                        */
      return (RexxObject *)TheTrueObject;
    else
                                       /* no index here                     */
      return (RexxObject *)TheFalseObject;
  }
}

size_t RexxArray::hasIndexNative(size_t index)
/******************************************************************************/
/* Function:  Determine if an element exist for a position                    */
/******************************************************************************/
{
                                       /* in bounds and here?               */
  if (index > 0 && index <= this->size() && *(this->data()+index-1) != OREF_NULL)
    return (size_t)TRUE;               /* this is true                      */
  else
    return (size_t)FALSE;              /* nope, don't have it               */
}

RexxArray *RexxArray::makeArray(void)
/******************************************************************************/
/* Function: Return a single dimension array of self, with only items that    */
/*           has values, so it will be of size items.                         */
/******************************************************************************/
{
    // for an array, this is the all items value.
    return this->allItems();
}


/**
 * Return an array of all real items contained in the collection.
 *
 * @return An array with all of the array items (non-sparse).
 */
RexxArray *RexxArray::allItems(void)
{
    // get a result array of the appropriate size
    RexxArray *newArray = (RexxArray *)new_array(this->numItems());

    // we need to fill in based on actual items, not the index.
    arraysize_t count = 0;
    RexxObject **item = this->data();
    // loop through the array, copying all of the items.
    for (arraysize_t iterator = 0; iterator < this->size(); iterator++ )
    {
        // if this is a real array item, copy over to the result
        if (item[iterator] != OREF_NULL)
        {
            newArray->put(item[iterator], ++count);
        }
    }
    return newArray;
}


/**
 * Return an array of all indices of real array items contained
 * in the collection.
 *
 * @return An array with all of the array indices (non-sparse).
 */
RexxArray *RexxArray::allIndexes(void)
{
    // get a result array of the appropriate size
    RexxArray *newArray = (RexxArray *)new_array(this->numItems());
    save(newArray);

    // we need to fill in based on actual items, not the index.
    arraysize_t count = 0;
    RexxObject **item = this->data();
    // loop through the array, copying all of the items.
    for (arraysize_t iterator = 0; iterator < this->size(); iterator++ )
    {
        // if this is a real array item, add an integer index item to the
        // result collection.
        if (item[iterator] != OREF_NULL)
        {
            newArray->put(convertIndex(iterator+1), ++count);
        }
    }
    discard_hold(newArray);
    return newArray;
}

// Temporary bypass for BUG #1700606
#if 0
RexxString  *RexxArray::primitiveMakeString()
/******************************************************************************/
/* Function:  Handle a REQUEST('STRING') request for a REXX string object     */
/******************************************************************************/
{
  return this->makeString((RexxString *)OREF_NULL);    /* forward to the real makestring method */
}
#endif

RexxString *RexxArray::makeString(RexxString *format, RexxString *separator)
{
    return toString(format, separator);
}

RexxString *RexxArray::toString(       /* concatenate array elements to create string object */
     RexxString *format,               /* format of concatenation (one of: "C", "L")         */
     RexxString *separator)            /* separator to use if "L" is specified for format    */
/******************************************************************************/
/* Function:  Make a string out of an array                                   */
/******************************************************************************/
{
  size_t items;
  size_t i;
  RexxArray *newArray;                  /* New array                         */
  RexxString *newString;
  RexxString *line_end_string;          /* converted substitution value      */
  RexxMutableBuffer *mutbuffer;
  RexxObject *item;                     /* inserted value item               */
  int i_form;                           /* 1 == line, 2 == char              */

  mutbuffer = ((RexxMutableBufferClass*) TheMutableBufferClass)->newRexx(NULL, 0);
  save(mutbuffer);

  newArray = this->makeArray();          /* maybe multidimensional, make onedimensional  */
  save(newArray);

  items = newArray->numItems();          /* and the actual count in the array */

  if (format != OREF_NULL)
  {
     // a string value is required here
     format = REQUIRED_STRING(format, ARG_ONE);
  }

  if (format == OREF_NULL)
     i_form = 2;                         /* treat item as LINE by default   */
  else if (toupper((format->stringData[0])) == 'C')
     i_form = 1;
  else if (toupper((format->stringData[0])) == 'L')
     i_form = 2;
  else
     report_exception2(Error_Incorrect_method_option, new_cstring("CL"), format);

  if (i_form == 1)                       /* character oriented processing    */
  {
    if (separator != OREF_NULL)
    {
        report_exception1(Error_Incorrect_method_maxarg, IntegerOne);

    }

    for (i = 1; i <=items ; i++)         /* loop through the array           */
    {
      item = newArray->get(i);           /* get the next item                */
      if (item != OREF_NULL)
      {
          RexxObject *stringValue = item->requiredString();
          if (stringValue != TheNilObject)
          {
              mutbuffer->append(stringValue);
          }
      }
    }
  }
  else if (i_form == 2)                 /* line oriented processing          */
  {
      if (separator != OREF_NULL)
      {
         line_end_string = REQUIRED_STRING(separator, ARG_TWO);
      }
      else
         line_end_string = new_cstring(line_end);

      save(line_end_string);
      bool first = true;

      for (i = 1; i <= items; i++)         /* loop through the array            */
      {
          item = newArray->get(i);          /* get the next item                 */
          if (item != OREF_NULL)
          {
              // append a linend between the previous item and this one.
              if (!first)
              {
                  mutbuffer->append((RexxObject *) line_end_string);
              }
              RexxObject *stringValue = item->requiredString();
              if (stringValue != TheNilObject)
              {
                  mutbuffer->append(stringValue);
              }
              first = false;
          }
      }

      discard(line_end_string);
  }

  newString = mutbuffer->requestString();
  discard(newArray);
  discard(mutbuffer);
  return newString;
}

RexxObject *RexxArray::join(           /* join two arrays into one          */
     RexxArray *other)                 /* array to be appended to self      */
/******************************************************************************/
/* Function:  Join two arrays into one array                                  */
/******************************************************************************/
{
  RexxArray *newArray;                 /* result array of the join          */
  size_t  i;                           /* counter for spinning down array   */
                                       /* get new array, total size is size */
                                       /* of both arrays.                   */
  newArray = (RexxArray*)new_array(this->size() + other->size());
                                       /* If none of objects are in oldSpace*/
                                       /*  we can skip the OrefSets and do  */
                                       /*  path copy                        */
  if (!OldSpace(this) && !OldSpace(other) && !OldSpace(newArray)) {

                                       /* copy first array into new one     */
    memcpy(newArray->data(), this->data(), ((char *)&(this->data()[this->size()])) - ((char *)this->data()));
                                       /* copy 2nd array into the new one   */
                                       /* after the first one.              */
    memcpy((void *)&(newArray->data()[this->size()]), other->data(), ((char *)&(other->data()[other->size()])) - ((char *)other->data()));
  }
  else {
                                       /* first place self into new array   */
    for (i = 0; i < this->size(); i++)
      newArray->put(this->data()[i], i+1);
                                       /* now add the other array into new  */
                                       /*one                                */
    for (i = 1; i <= other->size(); i++)
      newArray->put(other->get(i), i + this->size());

  }
  return newArray;                     /* All done, return joined array     */

}

void RexxArray::resize(void)           /* resize this array to be a NULLARRA*/
/******************************************************************************/
/* Function:  An Array is being expanded so chop off the data (array)         */
/*            portion of this array.                                          */
/******************************************************************************/
{
  size_t i;
                                       /* Has the array already been        */
                                       /* expanded ?                        */
  if (this->expansionArray == this) {
                                       /* no, then we resize the array      */
                                       /* is this array in OldSpace ?       */
    if (OldSpace(this)) {
                                       /* Old Space, remove any reference   */
                                       /* to new space from memory tables   */
      for (i = 0; i < this->arraySize; i++)
        OrefSet(this, this->objects[i], OREF_NULL);
    }
                                       /* resize the array object           */
    memoryObject.reSize(this, sizeof(RexxArray));
    this->arraySize = 0;               /* outer array has no elements       */
  }
}

void RexxArray::shrink(
     size_t amount)                    /* amount to shrink an array         */
/******************************************************************************/
/* Function:  Shrink an array without reallocating any elements               */
/*            Single Dimension ONLY                                           */
/******************************************************************************/
{
  size_t newSize;                      /* new array size                    */
  size_t i;                            /* loop counter                      */
  size_t size;                         /* existing size                     */

  size = this->size();                 /* get the size                      */
  newSize = size - amount;             /* get the new size                  */

  i = newSize + 1;                     /* address first removed element     */
  for (i = newSize + 1; i <= size; i++)/* for all removed elements          */
    this->put(OREF_NULL, i);           /* clear out the element             */
                                       /* adjust the size .                 */
  this->expansionArray->arraySize = newSize;
}

size_t RexxArray::indexOf(
    RexxObject *target)                /* target object to locate           */
/*****************************************************************************/
/* Function:  To search a list in the form of an array for an item, returning*/
/*            the index                                                      */
/*****************************************************************************/
{
  size_t i;                            /* current array index               */
  size_t size;                         /* array size                        */

  size = this->size();                 /* get the array size                */
  for (i = 1; i <= size; i++) {        /* spin through the array            */
    if (this->get(i) == target)        /* is this the one?                  */
      return i;                        /* return the index                  */
  }
  return 0;                            /* not found here                    */
}

void RexxArray::deleteItem(
    size_t     targetIndex)            /* target to remove                  */
/*****************************************************************************/
/* Function:  Remove an item from a list array, shrinking the array item     */
/*            and moving the relevent items                                  */
/*****************************************************************************/
{
  size_t i;                            /* current array index               */
  size_t size;                         /* array size                        */

  size = this->size();                 /* get the array size                */
                                       /* spin through the array            */
  for (i = targetIndex; i < size; i++) {
    this->put(this->get(i +1), i);     /* move down each item               */
  }
  this->shrink(1);                     /* now shrink the array              */
}

void RexxArray::insertItem(
    RexxObject *newItem,               /* new item to add                   */
    size_t     targetIndex)            /* insertion point                   */
/*****************************************************************************/
/* Function:  Remove an item from a list array, shrinking the array item     */
/*            and moving the relevent items                                  */
/*****************************************************************************/
{
  size_t     i;                        /* current array index               */

  this->extend(1);                     /* extend the array size             */
                                       /* spin through the array            */
  for (i = this->size(); i > targetIndex; i--) {
    this->put(this->get(i - 1), i);    /* move up each item                 */
  }
  this->put(newItem, targetIndex);     /* insert the new item               */
}

RexxArray *RexxArray::extend(          /* join two arrays into one          */
    size_t extension)                  /* number of elements to extend      */
/******************************************************************************/
/* Function:  Extend an array by a given number of elements                   */
/*            Single Dimension ONLY                                           */
/******************************************************************************/
{
  RexxArray *newArray;                 /* result array of the extend        */
  size_t  i;                           /* counter for spinning down array   */

                                       /* do we really need to extend array */
                                       /* or just adjust size.              */
  if (this->size() + extension <= this->maximumSize) {
                                       /* adjust the size .                 */
    this->expansionArray->arraySize += extension;
    return this;
  }

  size_t newSize = this->size() + extension;
  size_t extendSize;
  /* are we still a relative small array? */
  if (newSize < ARRAY_EXTEND_EXTRA_LARGE_SIZE) {
      /* just bump this the small extra amount */
      extendSize = ARRAY_EXTEND_EXTRA_SIZE;
  }
  else {
      /* we're getting large.  We'll start bumping by a larger amount. */
/*    extendSize = ARRAY_EXTEND_EXTRA_LARGE_SIZE;           */
      extendSize = this->size() / 2;   /*               add 50% to the size */
  }

                                       /* get a new array, total size is    */
                                       /* size of both arrays.              */
  newArray = (RexxArray *)new_array(newSize + extendSize);
                                       /* If none of the objects are in     */
                                       /* OldSpace,  we can skip the        */
                                       /* OrefSets and just copy            */
  if (!OldSpace(newArray)) {
                                       /* copy ourselves into the new array */
    memcpy(newArray->data(), this->data(), ((char *)&(this->data()[this->size()])) - ((char *)this->data()));
  } else {
                                       /* copy each element into new array  */
    for (i = 0; i < this->size(); i++)
      newArray->put(this->data()[i], i+1);
  }
  this->resize();                      /* adjust ourself to be null arrayobj*/

  newArray->setExpansion(OREF_NULL);   /* clear the new expansion array     */
                                       /* set new expansion array           */
  OrefSet(this, this->expansionArray, newArray);
                                       /* keep max Size value in synch      */
                                       /* with expansion.                   */
  this->maximumSize = newArray->maximumSize;
                                       /* make sure size is correct.        */
  newArray->arraySize = newSize;
  return this;                         /* All done, return array            */
}


/**
 * Find the index of a single item in the array.
 *
 * @param item   The item to locate.
 *
 * @return The numeric index of the item.
 */
arraysize_t RexxArray::findSingleIndexItem(RexxObject *item)
{
    for (arraysize_t i = 1; i <= this->size(); i++)
    {
        RexxObject *test = get(i);

        // if there's an object in the slot, compare it.
        if (test != OREF_NULL)
        {
            // if the items are equal, return the index
            if (item->equalValue(test))
            {
                return i;
            }
        }
    }
    return 0;
}


/**
 * Convert an internal index item into "external" form.  Handles
 * both single- and multi-dimensional arrays.
 *
 * @param idx    The internal index to convert.
 *
 * @return An index object proper for the array type.
 */
RexxObject *RexxArray::convertIndex(size_t idx)
{
    // single dimension array?  This is easy
    if (this->dimensions == OREF_NULL || this->dimensions->size() == 1)
    {
        return new_integer(idx);
    }
    else
    {
        // compose a composite index
        return indexToArray(idx);
    }
}


/**
 * Convert a multi-dimensional array index into an array
 * of index values for the flattened dimension.
 *
 * @param idx     The index to covert.
 *
 * @return An array of the individual index items.
 */
RexxObject* RexxArray::indexToArray(size_t idx)
{
  // work with an origin-origin zero version of the index, which is easier
  // do work with.
  idx--;
  // get the number of dimensions specified.
  size_t dims = this->dimensions->size();
  // get an array we fill in as we go
  RexxArray *index = new_array(dims);

  for (size_t i = dims; i > 0; i--)
  {
      // get the next dimension size
      size_t dimension = ((RexxInteger *)this->dimensions->get(i))->value;
      // now get the remainder.  This tells us the position within this
      // dimension of the array.  Make an integer object and store in the
      // array.
      size_t digit = idx % dimension;
      // the digit is origin-zero, but the Rexx index is origin-one.
      index->put(new_integer(digit + 1), i);
      // now strip out that portion of the index.
      idx = (idx - digit) / dimension;
  }
  // return the array object
  discard_hold(index);
  return index;
}


/**
 * Return the index for the first occurrence of the target in
 * the array.
 *
 * @param target The target object.
 *
 * @return The index for the array.  For a multi-dimensional array, this
 *         returns an array of indices.
 */
RexxObject *RexxArray::index(RexxObject *target)
{
    // we require the index to be there.
    required_arg(target, ONE);
    // see if we have this item.  If not, then
    // we return .nil.
    arraysize_t index = findSingleIndexItem(target);

    if (index == 0)
    {
        return TheNilObject;
    }

    return convertIndex(index);
}


/**
 * Remove the target object from the collection.
 *
 * @param target The target object.
 *
 * @return The removed object (same as target).
 */
RexxObject *RexxArray::removeItem(RexxObject *target)
{
    // we require the index to be there.
    required_arg(target, ONE);
    // see if we have this item.  If not, then
    // we return .nil.
    arraysize_t index = findSingleIndexItem(target);

    if (index == 0)
    {
        return TheNilObject;
    }
    // remove the item at the location
    put(OREF_NULL, index);
    return target;
}


/**
 * Test if an item is within the array.
 *
 * @param target The target test item.
 *
 * @return .true if this item exists in the array. .false if it does not
 *         exist.
 */
RexxObject *RexxArray::hasItem(RexxObject *target)
{
    // this is pretty simple.  One argument, required, and just search to see
    // if we have it.
    required_arg(target, ONE);
    return findSingleIndexItem(target) == 0 ? TheFalseObject : TheTrueObject;
}



void copyElements(
    COPYELEMENTPARM *parm,
    size_t newDimension)
/******************************************************************************/
/* Function:  Recursive routine to copy element from one multiDim array       */
/*            to another.                                                     */
/******************************************************************************/
{
  size_t skipAmount;                   /* amount to skip for increased      */
                                       /* dimension.                        */
  size_t newDimSize;
  size_t oldDimSize;
  size_t oldDimension;
  size_t i;                            /* count for each subscript at       */
                                       /* this dimension                    */

                                       /* At the point where we can         */
                                       /* copy elements?                    */
                                       /* ending condition for recusion     */
   if (newDimension == parm->firstChangedDimension) {
                                       /* is new array in OldSpace?         */
     if (OldSpace(parm->newArray)) {
                                       /* Yes,need to do OrefSets           */
                                       /* For each element to copy          */
       for (i = 1; i <= parm->copyElements; i++, parm->startNew++, parm->startOld++ ) {
                                       /* set the newvalue.                 */
          OrefSet(parm->newArray, *parm->startNew, *parm->startOld);
       }
     }
     else {
                                       /* not old Spcae we can do memcpy    */
       memcpy(parm->startNew, parm->startOld, sizeof(RexxObject *) * parm->copyElements);
                                       /* update pointers                   */
       parm->startNew += parm->copyElements;
       parm->startOld += parm->copyElements;
     }
                                       /* now bump past space for           */
                                       /* additional size of dimension      */
     parm->startNew += parm->skipElements;
   }
   else {
                                       /* Compute the old dimension num     */
     oldDimension = newDimension - parm->deltaDimSize;
                                       /* Get size for new Dimension        */
     newDimSize = ((RexxInteger *)parm->newDimArray->get(newDimension))->value;
                                       /* Get size for old Dimension        */
     oldDimSize = ((RexxInteger *)parm->oldDimArray->get(oldDimension))->value;
                                       /* For each subscript at this        */
     for (i= 1; i <= oldDimSize; i++) {/* dimension, (of old size)          */
                                       /* copy elelments.                   */
       copyElements(parm, newDimension + 1);
     }
     if (newDimSize > oldDimSize) {    /* Was this dimension expanded?      */
                                       /* compute total space need for      */
                                       /* block of all lower dimensions     */
      for (i = parm->newDimArray->size(), skipAmount = 1;
           i > newDimension;
           skipAmount *= ((RexxInteger *)parm->newDimArray->get(i))->value, i--);
                                       /* multiple by delta add at this     */
                                       /* dimension.                        */
      skipAmount *= (newDimSize - oldDimSize);
                                       /* Bump our start pointer past       */
                                       /* empty space added for this        */
                                       /* dimension.                        */
      parm->startNew += skipAmount;
     }
   }
                                       /* all done at this level return     */
                                       /* to caller.                        */
   return;
}


RexxArray *RexxArray::extendMulti(     /* Extend multi array                */
     RexxObject **index,               /* Dimension array is to be          */
     size_t indexCount,                /* number of indices in the index    */
     size_t start)                     /* Starting position of dimensions   */
                                       /*  in index.                        */
/******************************************************************************/
/* Function:  Extend an array by a given number of elements                   */
/*            Multiple Dimensions                                             */
/*            Index either has more dimensions that the current array or      */
/*             is the same number.  So expansion array will have same number  */
/*             of dimensions as index.                                        */
/*            So examine size of each dimension and new array will be max     */
/*            of two sizes.                                                   */
/*                                                                            */
/*   NOTE: Even though we don't currently allow for changing the number       */
/*     of dimensions in an array, the support to do this, is included         */
/*     in this method.  We currently won't be called for this condition       */
/*     but if we ever are, this method should not need to be changed.         */
/*                                                                            */
/******************************************************************************/
{
  size_t currDimSize;                  /* Current size of Dimension         */
  size_t additionalDim;                /* Number of additional DImension    */
  size_t newDimSize;                   /* New size for this Dimension       */
  size_t newDimension;                 /* Current dimension                 */
  size_t oldDimension;                 /* Current dimension                 */
  size_t i;
  RexxArray *newArray;
  RexxArray *newDimArray;              /* Array containing new dimension    */
  size_t newDimArraySize;              /* Size of Dimension Array           */
  size_t accumSize;
  size_t firstDimChanged = 0;          /* First Dimension to grow           */
  COPYELEMENTPARM copyParm;            /* Structure for copyElement         */
  size_t  tempSize;

                                       /* New dimension array size of       */
                                       /* index array.                      */
                                       /* index is actually 1 bigger tha    */
                                       /* dimension size, since it          */
                                       /* contains the new value at 1st     */
                                       /* position                          */

                                       /* Compute new Size for DimArray     */
  newDimArraySize = indexCount;
  newDimArray = (RexxArray *)save(new_array(newDimArraySize));
                                       /* extending from single Dimensio    */
                                       /*  to a multi Dimensionsal array    */
  if (this->dimensions == OREF_NULL) {
                                       /* Get value for 1st dimension       */
                                       /*  its the last element             */
    i = newDimArraySize - 1;
    newDimSize = index[i]->requiredPositive(i);
                                       /* Yes, is 1st Dimension going to    */
                                       /* be bigger than current size?      */
    if (newDimSize > this->size())
                                       /* Yes, use new size + buffer for    */
                                       /*  1st Dimension                    */
      newDimArray->put(new_integer(newDimSize), newDimArraySize);
    else {
                                       /* nope, use same size for Dim       */
      tempSize = this->size();
      newDimArray->put(new_integer(tempSize), newDimArraySize);
    }
  }
  else {
    for (oldDimension = this->dimensions->size(), newDimension = newDimArraySize;
         oldDimension > 0 ;
         oldDimension--, newDimension--) {
                                       /* Get current size of this dimension*/
      currDimSize = ((RexxInteger *)this->dimensions->get(oldDimension))->value;
                                       /* Get indexd  size of this dimension*/

      newDimSize = index[newDimension - 1]->requiredPositive(newDimension);
                                       /* does this dimension need to be    */
                                       /*  expanded.                        */
      if (newDimSize > currDimSize) {
        newDimArray->put((RexxObject *)new_integer(newDimSize), newDimension);
                                       /* has a dimension already been chang*/
        if (!firstDimChanged) {
                                       /* remember the first dimenion chenge*/
          firstDimChanged = newDimension;
        }
      }
      else {
        newDimArray->put(this->dimensions->get(oldDimension), newDimension);
      }
    }
  }
                                       /* Was original array single dim     */
  if (this->dimensions == OREF_NULL) {
                                       /* additional Dimensions is 1        */
                                       /*  minus size, (1st Dimension)      */
    additionalDim = newDimArraySize - 1;
  }
  else {
                                       /* compute number of dimensions added*/
    additionalDim = newDimArraySize - this->dimensions->size();
  }
                                       /* is index greater than current     */
                                       /*  dimensions of this array.        */
  if (additionalDim > 0) {
                                       /* yes, for remainder of dimensions  */
    for (newDimension = additionalDim;
         newDimension > 0 ;
         newDimension--) {
                                       /* Get indexd  size of this dimension*/
      newDimSize = ((RexxInteger *)index[newDimension - 1])->value;
                                       /* set up value for this dimension   */
      newDimArray->put(new_integer(newDimSize), newDimension);
    }
  }
                                       /* Now create the new array for this */
                                       /*  dimension.                       */
  newArray = (RexxArray *)save(new (newDimArray->data(), newDimArraySize, TheArrayClass) RexxArray);
                                       /* Anything in original?             */
  if (this->size()) {
                                       /* Yes, move values into new arra    */
                                       /* Extending from single Dimension   */
                                       /* or original array was empty       */
                                       /* or adding dimensions or increas   */
                                       /* last original dimension?          */
    if (this->dimensions == OREF_NULL || this->dimensions->size() == 1 ||
        this->size() == 0 ||
        !firstDimChanged || firstDimChanged <= additionalDim + 1) {
                                       /* If none of the objects are in     */
                                       /* OldSpace,  we can 'cheat' and do  */
                                       /* a fast copy (memcpy)              */
      if (!OldSpace(newArray)) {
                                       /* copy ourselves into the new array */
        memcpy(newArray->data(), this->data(), sizeof(RexxObject *) * this->size());
      } else {
                                       /* copy each element into new array  */
        for (i = 0; i < this->size(); i++)
          newArray->put(this->data()[i], i+1);
      }

    }
                                       /* Working with 2 multi-dimensional  */
                                       /* Array's.                          */
    else {
                                       /* Now we need to move all elements  */

                                       /* for all Dimensions before first   */
                                       /* to increase.                      */
      for (i = newDimArraySize, accumSize = 1;
           i > firstDimChanged ;
           accumSize *= ((RexxInteger *)this->dimensions->get(i))->value, i--);
                                       /* Compute lowest largest contig     */
                                       /* chunk that can be copied.         */
      copyParm.copyElements = accumSize * ((RexxInteger *)this->dimensions->get(firstDimChanged))->value;
                                       /* Compute amount need to ship       */
                                       /* to compete expanded dimension     */
      copyParm.skipElements = accumSize * (((RexxInteger *)newDimArray->get(firstDimChanged))->value -
                 ((RexxInteger *)this->dimensions->get(firstDimChanged))->value);

      copyParm.startNew = newArray->data();
      copyParm.startOld = this->data();
                                       /* Setup parameter structure         */
      copyParm.firstChangedDimension = firstDimChanged;
      copyParm.newArray = newArray;
      copyParm.newDimArray = newDimArray;
      copyParm.oldDimArray = this->dimensions;
                                       /* Compute delta dimensions size     */
      copyParm.deltaDimSize = newDimArraySize - this->dimensions->size();
                                       /* do the copy. starting w/          */
                                       /* Highest Dimension                 */
      copyElements(&copyParm, newDimArraySize - this->dimensions->size() + 1);
    }
  }                                    /* end, if anything in original      */


  this->resize();
                                       /* Set dimensions to be new dimension*/
  OrefSet(this, this->dimensions, newDimArray);
  newArray->setExpansion(OREF_NULL);
                                       /* update expansion array.           */
  OrefSet(this, this->expansionArray, newArray);
                                       /* keep max Size value in synch      */
                                       /* with expansion.                   */
  this->maximumSize = newArray->maximumSize;
                                       /* no longer need to save created    */
                                       /* arrays, they are anchored         */
  discard(newDimArray);
  discard(newArray);
  return this;                         /* All done, return array            */
}

RexxObject *RexxArray::insert(         /* insert an element into an array   */
     RexxObject *value,                /* value to be inserted              */
     size_t  index )                   /* index of the insertion point      */
/******************************************************************************/
/* Function:  Insert an element into an array                                 */
/******************************************************************************/
{
  RexxObject **start;                  /* move starting point               */
  RexxObject **end;                    /* move ending point                 */

                                       /* get the address of first element  */
  start = &(this->data()[index - 1 ]);
                                       /* and the last element              */
  end = &(this->data()[this->size() - 1]);
                                       /* shift the array over              */
  memmove(start + sizeof(OREF), start, end - start);
                                       /* NOTE:  The following assignment   */
                                       /* is done directly to avoid problems*/
                                       /* with oldspace to newspace object  */
                                       /* references.  Just doing the put   */
                                       /* without nulling out the item first*/
                                       /* can cause the reference count of  */
                                       /* the object that previously        */
                                       /* occupied that spot to go to zero, */
                                       /* making the object subject to a    */
                                       /* garbage collection.  We zero it   */
                                       /* out first to make it an empty     */
                                       /* slot first                        */
  this->data()[index - 1] = OREF_NULL;
  this->put(value, index);             /* add the new element in            */
  return value;                        /* return the inserted value         */
}

void *   RexxArray::operator new(size_t size,
    size_t items,                      /* items in the array                */
    RexxObject **first )               /* array of items to fill array      */
/******************************************************************************/
/* Quick creation of an argument array                                        */
/******************************************************************************/
{
  RexxArray *newArray;                 /* newly created array               */

  newArray = new_array(items);         /* get a new array                   */
  if (items != 0)                      /* not a null array?                 */
                                       /* copy the references in            */
    memcpy(newArray->data(), first, (sizeof(RexxObject *) * items));
  return newArray;                     /* return the new array              */
}

void *   RexxArray::operator new(size_t size, RexxObject **args, size_t argCount, RexxClass *arrayClass)
/******************************************************************************/
/* Function:  Rexx level creation of an ARRAY object                          */
/******************************************************************************/
{
  RexxArray   *dim_array;
  RexxInteger *current_dim;
  RexxArray   *temp;
  size_t i;                            /* control variable                    */
  size_t cur_size;                     /* current dimentions size             */
  size_t total_size;                   /* total dimensional size of the array */



  if (argCount == 0) {
                                       /* If no argument is passed          */
                                       /*  create an empty array.           */
    temp = new ((size_t)0, arrayClass) RexxArray;
    save(temp);                        /* protect new object from GC        */
    send_message0(temp, OREF_INIT);    /* call any rexx init's              */
    discard(temp);                     /* protect new object from GC        */
    return temp;
  }

                                       /* Special case for 1-dimensional    */
  if (argCount == 1) {
    current_dim = (RexxInteger *)args[0];
                                       /* Make sure it's an integer         */
    total_size = current_dim->requiredNonNegative(ARG_ONE, number_digits());
    if (total_size < 0)
      report_exception1(Error_Incorrect_method_array, new_integer(total_size));

    if (total_size >= MAX_FIXEDARRAY_SIZE)
      report_exception(Error_Incorrect_method_array_too_big);

    /* Note: The following will leave the dimension field set to OREF_NULL, */
    /* which is what we want (it will be done by array_init above).         */
                                       /* Create new array of approp. size. */

    temp = (RexxArray *)new_externalArray(total_size, arrayClass);
    save(temp);                        /* protect new object from GC        */
    if (total_size == 0) {             /* Creating 0 sized array?           */
                                       /* Yup, setup a Dimension array      */
                                       /* single Dimension, Mark so         */
                                       /* can't change Dimensions.          */
      OrefSet(temp, temp->dimensions, new_array1(IntegerZero));
    }
    send_message0(temp, OREF_INIT);    /* call any rexx init's              */
    discard(temp);                     /* protect new object from GC        */
    return temp;                       /* Return the new array.             */
  }
                                       /* Working with a multi-dimension    */
                                       /*  array, so get a dimension arr    */
  dim_array = (RexxArray *)new_array(argCount);
  total_size = 1;                      /* Set up for multiplication         */
  for (i=0; i < argCount; i++) {
                                       /* make sure current parm is inte    */
    current_dim = (RexxInteger *)args[i];
    if (current_dim == OREF_NULL)      /* was this one omitted?             */
      missing_argument(i+1);           /* must have this value              */
                                       /* get the long value                */
    cur_size = current_dim->requiredNonNegative(i+1);
                                       /* going to do an overflow?          */
    if (cur_size != 0 && ((MAX_FIXEDARRAY_SIZE / cur_size) < total_size))
                                       /* this is an error                  */
      report_exception(Error_Incorrect_method_array_too_big);
    total_size *= cur_size;            /* keep running total size           */
                                       /* Put integer object into curren    */
                                       /* dimension array position          */
    dim_array->put(new_integer(cur_size), i+1);
  }
                                       /* Make sure flattened array isn't   */
                                       /*  too big.                         */
  if (total_size >= MAX_FIXEDARRAY_SIZE)
    report_exception(Error_Incorrect_method_array_too_big);
                                       /* Create the new array              */
  temp = (RexxArray *)new_externalArray(total_size, arrayClass);
                                       /* put dimension array in new arr    */
  OrefSet(temp, temp->dimensions, dim_array);
  save(temp);                          /* protect new object from GC        */
  send_message0(temp, OREF_INIT);      /* call any rexx init's              */
  discard(temp);
  return temp;
}


/**
 * The merge sort routine.  This will partition the data in to
 * two sections, mergesort each partition, then merge the two
 * partitions together.
 *
 * @param working The working array (same size as the sorted array).
 * @param left    The left bound of the partition.
 * @param right   The right bounds of the parition.
 */
void RexxArray::mergeSort(RexxArray *working, size_t left, size_t right)
{
    if (right > left)
    {
        size_t mid = (right + left) / 2;
        mergeSort(working, left, mid);
        mergeSort(working, mid + 1, right);
        merge(working, left, mid + 1, right);
    }
}


/**
 * Perform the merge operation on two partitions.
 *
 * @param working The temporary working storage.
 * @param left    The left bound of the range.
 * @param mid     The midpoint of the range.  This merges the two partitions
 *                (left, mid - 1) and (mid, right).
 * @param right   The right bound of the array.
 */
void RexxArray::merge(RexxArray *working, size_t left, size_t mid, size_t right)
{
    size_t leftEnd = mid - 1;
    size_t elements = right - left + 1;
    size_t mergePosition = left;

    while ((left <= leftEnd) && (mid <= right))
    {
        RexxObject *leftItem = get(left);
        RexxObject *midItem = get(mid);
        if (leftItem->compareTo(midItem) <= 0)
        {
            working->put(leftItem, mergePosition);
            mergePosition++;
            left++;
        }
        else
        {
            working->put(midItem, mergePosition);
            mergePosition++;
            mid++;
        }
    }

    // now we have to copy any remainders in the segments
    while (left <= leftEnd)
    {
        RexxObject *item = get(left);
        working->put(item, mergePosition);
        left++;
        mergePosition++;
    }

    while (mid <= right)
    {
        RexxObject *item = get(mid);
        working->put(item, mergePosition);
        mid++;
        mergePosition++;
    }

    // we've not modified the right position, so we can use that now to copy the
    // merged elements back into the original array in reverse order
    for (size_t i = 1; i <= elements; i++)
    {
        RexxObject *item = working->get(right);
        put(item, right);
        right--;
    }
}


/**
 * The merge sort routine.  This will partition the data in to
 * two sections, mergesort each partition, then merge the two
 * partitions together.
 *
 * @param comparator The comparator object used for the compares.
 * @param working    The working array (same size as the sorted array).
 * @param left       The left bound of the partition.
 * @param right      The right bounds of the parition.
 */
void RexxArray::mergeSort(RexxObject *comparator, RexxArray *working, size_t left, size_t right)
{
    if (right > left)
    {
        size_t mid = (right + left) / 2;
        mergeSort(comparator, working, left, mid);
        mergeSort(comparator, working, mid + 1, right);
        merge(comparator, working, left, mid + 1, right);
    }
}


/**
 * Perform the merge operation on two partitions.
 *
 * @param comparator The comparator used to produce the ordering.
 * @param working    The temporary working storage.
 * @param left       The left bound of the range.
 * @param mid        The midpoint of the range.  This merges the two partitions
 *                   (left, mid - 1) and (mid, right).
 * @param right      The right bound of the array.
 */
void RexxArray::merge(RexxObject *comparator, RexxArray *working, size_t left, size_t mid, size_t right)
{
    size_t leftEnd = mid - 1;
    size_t elements = right - left + 1;
    size_t mergePosition = left;

    while ((left <= leftEnd) && (mid <= right))
    {
        RexxObject *leftItem = get(left);
        RexxObject *midItem = get(mid);
        if (sortCompare(comparator, leftItem, midItem) <= 0)
        {
            working->put(leftItem, mergePosition);
            mergePosition++;
            left++;
        }
        else
        {
            working->put(midItem, mergePosition);
            mergePosition++;
            mid++;
        }
    }

    // now we have to copy any remainders in the segments
    while (left <= leftEnd)
    {
        RexxObject *item = get(left);
        working->put(item, mergePosition);
        left++;
        mergePosition++;
    }

    while (mid <= right)
    {
        RexxObject *item = get(mid);
        working->put(item, mergePosition);
        mid++;
        mergePosition++;
    }

    // we've not modified the right position, so we can use that now to copy the
    // merged elements back into the original array in reverse order
    for (size_t i = 1; i <= elements; i++)
    {
        RexxObject *item = working->get(right);
        put(item, right);
        right--;
    }
}


/**
 * Recursive quick sort routine for sorting a partition.
 *
 * @param left   The left bound of the partition.
 * @param right  The right bound of the partition.
 */
void RexxArray::quickSort(size_t left, size_t right)
{
    size_t old_left = left;
    size_t old_right = right;

    RexxObject *pivot = get(left);     // get the pivot value

    // now find the new partitioning
    while (left < right)
    {
        // fix the right end
        while (get(right)->compareTo(pivot) >= 0 && (left < right))
        {
            right--;
        }
        // did we find a mismatch while testing?  then pull things in from the left too
        if (left != right)
        {
            // swap these and pull the left in
            put(get(right), left);
            left++;
        }
        // now compare from the left
        while (get(left)->compareTo(pivot) <= 0 && (left < right))
        {
            left++;
        }
        // still not done?
        if (left != right)
        {
            // swap these two and continue
            put(get(left), right);
            right--;
        }
    }

    // store the pivot value in the current left position
    put(pivot, left);
    // this is the new pivot point
    size_t pivotPoint = left;
    // restore the old end points
    left = old_left;
    right = old_right;
    // something to the left of the pivot?
    if (left < pivotPoint)
    {
        // sort the left partition
        quickSort(left, pivotPoint - 1);
    }
    // and also the right partition if we have one
    if (right > pivotPoint)
    {
        quickSort(pivotPoint + 1, right);
    }
}


/**
 * Recursive quick sort routine for sorting a partition.
 *
 * @param left   The left bound of the partition.
 * @param right  The right bound of the partition.
 */
void RexxArray::quickSort(RexxObject *comparator, size_t left, size_t right)
{
    size_t old_left = left;
    size_t old_right = right;

    RexxObject *pivot = get(left);     // get the pivot value

    // now find the new partitioning
    while (left < right)
    {
        // fix the right end
        while (sortCompare(comparator, get(right), pivot) >= 0 && (left < right))
        {
            right--;
        }
        // did we find a mismatch while testing?  then pull things in from the left too
        if (left != right)
        {
            // swap these and pull the left in
            put(get(right), left);
            left++;
        }
        // now compare from the left
        while (sortCompare(comparator, get(left), pivot) <= 0 && (left < right))
        {
            left++;
        }
        // still not done?
        if (left != right)
        {
            // swap these two and continue
            put(get(left), right);
            right--;
        }
    }

    // store the pivot value in the current left position
    put(pivot, left);
    // this is the new pivot point
    size_t pivotPoint = left;
    // restore the old end points
    left = old_left;
    right = old_right;
    // something to the left of the pivot?
    if (left < pivotPoint)
    {
        // sort the left partition
        quickSort(comparator, left, pivotPoint - 1);
    }
    // and also the right partition if we have one
    if (right > pivotPoint)
    {
        quickSort(comparator, pivotPoint + 1, right);
    }
}


/**
 * Utility method for calling the sort comparators during a sort
 * operation.
 *
 * @param comparator The comparator object.
 * @param left       The left object to compare.
 * @param right      The right object to compare.
 *
 * @return -1, 0, 1 depending on the compare results.
 */
wholenumber_t RexxArray::sortCompare(RexxObject *comparator, RexxObject *left, RexxObject *right)
{
    RexxObject *result = comparator->sendMessage(OREF_COMPARE, left, right);
    if (result == OREF_NULL)
    {
        reportException(Error_No_result_object_message, OREF_COMPARE);
    }

    wholenumber_t comparison = result->longValue(DEFAULT_DIGITS);
    if (comparison == NO_LONG)
    {
        reportException(Error_Invalid_whole_number_compare, result);
    }
    return comparison;
}


/**
 * Sort elements of the array in place, using a quicksort.
 *
 * @return Returns the same array, with the elements sorted.
 */
RexxArray *RexxArray::sortRexx()
{
    arraysize_t count = numItems();
    if (count == 0)         // if the count is zero, sorting is easy!
    {
        return this;
    }

    // make sure this is a non-sparse array.  Checking up front means we don't
    // need to check on each compare operation.
    for (arraysize_t i = 1; i <= count; i++)
    {
        if (get(i) == OREF_NULL)
        {
            reportException(Error_Execution_sparse_array, new_integer(i));
        }
    }

    // go do the quick sort
    quickSort(1, count);
    return this;
}


/**
 * Sort elements of the array in place, using a quicksort.
 *
 * @return Returns the same array, with the elements sorted.
 */
RexxArray *RexxArray::sortWithRexx(RexxObject *comparator)
{
    required_arg(comparator, ONE);

    arraysize_t count = numItems();
    if (count <= 1)         // if the count is zero, sorting is easy!
    {
        return this;
    }

    // make sure this is a non-sparse array.  Checking up front means we don't
    // need to check on each compare operation.
    for (arraysize_t i = 1; i <= count; i++)
    {
        if (get(i) == OREF_NULL)
        {
            reportException(Error_Execution_sparse_array, new_integer(i));
        }
    }

    // go do the quick sort
    quickSort(comparator, 1, count);
    return this;
}


/**
 * Sort elements of the array in place, using a quicksort.
 *
 * @return Returns the same array, with the elements sorted.
 */
RexxArray *RexxArray::stableSortRexx()
{
    arraysize_t count = numItems();
    if (count == 0)         // if the count is zero, sorting is easy!
    {
        return this;
    }

    // make sure this is a non-sparse array.  Checking up front means we don't
    // need to check on each compare operation.
    for (arraysize_t i = 1; i <= count; i++)
    {
        if (get(i) == OREF_NULL)
        {
            reportException(Error_Execution_sparse_array, new_integer(i));
        }
    }

    // the merge sort requires a temporary scratch area for the sort.
    RexxArray *working = new_array(count);
    save(working);

    // go do the quick sort
    mergeSort(working, 1, count);
    discard_hold(working);
    return this;
}


/**
 * Sort elements of the array in place, using a quicksort.
 *
 * @return Returns the same array, with the elements sorted.
 */
RexxArray *RexxArray::stableSortWithRexx(RexxObject *comparator)
{
    required_arg(comparator, ONE);

    arraysize_t count = numItems();
    if (count <= 1)         // if the count is zero, sorting is easy!
    {
        return this;
    }

    // make sure this is a non-sparse array.  Checking up front means we don't
    // need to check on each compare operation.
    for (arraysize_t i = 1; i <= count; i++)
    {
        if (get(i) == OREF_NULL)
        {
            reportException(Error_Execution_sparse_array, new_integer(i));
        }
    }

    // the merge sort requires a temporary scratch area for the sort.
    RexxArray *working = new_array(count);
    save(working);

    // go do the quick sort
    mergeSort(comparator, working, 1, count);
    discard_hold(working);
    return this;
}


void *  RexxArray::operator new(size_t size, RexxObject *first)
/******************************************************************************/
/* Function:  Create an array with 1 element (new_array1)                     */
/******************************************************************************/
{
  RexxArray *aref;

  aref = (RexxArray *)new_array(1);
  aref->put(first, 1L);

  return aref;
}

void *   RexxArray::operator new(size_t size, RexxObject *first, RexxObject *second)
{
  RexxArray *aref;

  aref = new_array(2);
  aref->put(first, 1L);
  aref->put(second, 2L);
  return aref;
}

void *   RexxArray::operator new(size_t size,
    RexxObject *first,
    RexxObject *second,
    RexxObject *third)
/******************************************************************************/
/* Function:  Create an array with 3 elements (new_array3)                    */
/******************************************************************************/
{
  RexxArray *aref;

  aref = new_array(3);
  aref->put(first,  1L);
  aref->put(second, 2L);
  aref->put(third,  3L);

  return aref;
}

void *   RexxArray::operator new(size_t size,
    RexxObject *first,
    RexxObject *second,
    RexxObject *third,
    RexxObject *fourth)
/******************************************************************************/
/* Function:  Create an array with 4 elements (new_array4)                    */
/******************************************************************************/
{
  RexxArray *aref;

  aref = new_array(4);
  aref->put(first,  1L);
  aref->put(second, 2L);
  aref->put(third,  3L);
  aref->put(fourth, 4L);

  return aref;
}

void *   RexxArray::operator new(size_t newSize,
                                 size_t size, RexxClass *arrayClass)
/******************************************************************************/
/* Function:  Low level array creation                                        */
/******************************************************************************/
{
  size_t bytes;
  RexxArray *newArray;
  size_t maxSize;
                                       /* is hintsize lower than minimal    */
  if (size <= ARRAY_MIN_SIZE) {        /*  allocation array size?           */
    maxSize = ARRAY_MIN_SIZE;          /* yes, we will actually min size    */
  }
  else {
    maxSize = size;                    /* nope, hintSize is allocat size    */
  }
                                       /* compute size of new array obj     */
  bytes = newSize + (sizeof(RexxObject *) * (maxSize - 1));
                                       /* Create the new array              */
  newArray = (RexxArray *)new_object(bytes);
                                       /* Give it array behaviour.          */
  BehaviourSet(newArray, arrayClass->instanceBehaviour);

                                       /* set the hashvalue                 */
  newArray->hashvalue =  HASHOREF(newArray);
  ClearObject(newArray);               /* Clear the state data              */
  newArray->arraySize = size;
  newArray->maximumSize = maxSize;
                                       /* no expansion yet, use ourself     */
  newArray->expansionArray = newArray;
  /* moved _after_ setting hashvalue, otherwise the uninit table will not be*/
  /* able to find the new array object again later!                         */
  if (arrayClass->uninitDefined()) {   /* does object have an UNINT method  */
     save(newArray);                   /* protect from GC - uninit table may*/
                                       /* require new REXX objects          */
     newArray->hasUninit();            /* Make sure everyone is notified.   */
     discard(newArray);
  }
  return newArray;                     /* return the new array to caller    */
}

RexxObject * RexxArray::newRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Exported ARRAY NEW method                                       */
/******************************************************************************/
{
  return new (arguments, argCount, (RexxClass *) this) RexxArray;
}

RexxObject  *RexxArray::of(RexxObject **args, size_t argCount)
/******************************************************************************/
/* Function:  Exported REXX OF method                                         */
/******************************************************************************/
{
  RexxArray *newArray;                 /* new array item                    */
  size_t i;                            /* loop index                        */
  RexxObject*item;                     /* individual array item             */

                                       /* is array obj to be from internal  */
  if (TheArrayClass != (RexxClass *)this) {
                                       /* nope, better create properly      */
                                       /* send new to actual class.         */
    newArray = (RexxArray *)send_message1(this, OREF_NEW, new_integer(argCount));
                                       /* For each argument to of, send a   */
                                       /* put message                       */
    for (i = 0; i < argCount; i++) {
       item = args[i];                 /* get the item                      */
       if (item != OREF_NULL)          /* have a real item here?            */
                                       /* place it in the target array      */
         send_message2(newArray, OREF_PUT, item, new_integer(i+1));
    }
    return newArray;
  }
  else {
      newArray = new (argCount, args) RexxArray;
      if (argCount == 0) {             /* creating 0 sized array?           */
                                       /* Yup, setup a Dimension array      */
                                       /* single Dimension, Mark so         */
                                       /* can't change Dimensions.          */
          OrefSet(newArray, newArray->dimensions, new_array1(IntegerZero));
      }
                                       /* argument array is exactly what    */
                                       /* we want, so just return it        */
      return newArray;
  }

}
#include "RexxNativeAPI.h"

#define this ((RexxArray *)self)

native1 (size_t, ARRAY_HASINDEX, size_t, index)
/******************************************************************************/
/* Function:  External interface to the object method                         */
/******************************************************************************/
{
  size_t    result;                    /* method result                     */

  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
                                       /* forward the method                */
  result = this->hasIndexNative(index);
  return_value(result);                /* return this                       */
}


native0 (size_t, ARRAY_SIZE)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
/******************************************************************************/
/* NOTE:  This method does not reaquire kernel access                         */
/******************************************************************************/
                                       /* just forward and return           */
  return this->size();                 /* forward the method                */
}

native1 (REXXOBJECT, ARRAY_AT, size_t, pos)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(this->get(pos));
}


native2 (void, ARRAY_PUT, REXXOBJECT, object, size_t, pos)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
  this->put((RexxObject *)object, pos);/* just forward and return           */
  return_void;                         /* and return nothing                */
}

nativei1 (REXXOBJECT, ARRAY_NEW,
         size_t, size)                 /* size to allocate                  */
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(new_array(size));
}

nativei1 (REXXOBJECT, ARRAY_NEW1,
         REXXOBJECT, object1)          /* object to include                 */
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(new_array1((OREF)object1));
}

nativei2 (REXXOBJECT, ARRAY_NEW2,
         REXXOBJECT, object1,          /* object to include                 */
         REXXOBJECT, object2)          /* second object to include          */
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  return_oref(new_array2((OREF)object1, (OREF)object2));
}
