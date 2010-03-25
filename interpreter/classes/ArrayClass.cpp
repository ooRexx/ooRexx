/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
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
/*   Object creation functions:                                               */
/*     new_array(s) - Create an array of size s, initial size of array is     */
/*                    set to s.  This array will be used as a static array    */
/*     new_array(a1) - Create array of size 1 and put a1 in it.               */
/*                      same as an array~of(a1)                               */
/*     new_array(a1,a2) - Create array of size 2 and put a1 and a2 in it      */
/*                      same as an array~of(a1,a2)                            */
/*     new_array(a1, a2, a3)      Same as new_array2 but 3 elements.          */
/*     new_array(a1, a2, a3, a4)    "   "    "        "  4   "                */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "RexxActivity.hpp"
#include "IntegerClass.hpp"
#include "SupplierClass.hpp"
#include "ArrayClass.hpp"
#include "MutableBufferClass.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"

#include <deque>


// singleton class instance
RexxClass *RexxArray::classInstance = OREF_NULL;
RexxArray *RexxArray::nullArray = OREF_NULL;

const size_t RexxArray::MAX_FIXEDARRAY_SIZE = (Numerics::MAX_WHOLENUMBER/10) + 1;
const size_t RexxArray::ARRAY_MIN_SIZE = 4;
const size_t RexxArray::ARRAY_DEFAULT_SIZE = 10;    // we use a larger default for ooRexx allocated arrays

/**
 * Create initial class object at bootstrap time.
 */
void RexxArray::createInstance()
{
    CLASS_CREATE(Array, "Array", RexxClass);
    nullArray = new_array((size_t)0); /* set up a null array               */
}


void RexxArray::init(size_t _size, size_t maxSize)
/******************************************************************************/
/* Function:  Initialize an array                                             */
/******************************************************************************/
{
  this->arraySize = _size;
  this->maximumSize = maxSize;
  this->lastElement = 0;               // no elements set yet
                                       /* no expansion yet, use ourself     */
  OrefSet(this, this->expansionArray, this);
}

RexxObject  *RexxArray::copy(void)
/******************************************************************************/
/* Function:   create a copy of array and expansion array.                    */
/******************************************************************************/
{
    /* make a copy of ourself            */
    RexxArray *newArray = (RexxArray *) this->RexxObject::copy();
    /* this array contain the data?      */
    if (this->expansionArray != OREF_NULL && this->expansionArray != this)
    {
        /* no, make sure we get a copy of    */
        /* array containing data.            */
        newArray->setExpansion(this->expansionArray->copy());
    }
    else
    {
        /* no, make sure we get a copy of    */
        /* array containing data.            */
        newArray->setExpansion(newArray);  /* make sure we point to ourself!    */
    }
    return(RexxObject *)newArray;       /* return the new array              */
}

void RexxArray::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  RexxObject **arrayPtr;
  RexxObject **endPtr;

  memory_mark(this->dimensions);
  memory_mark(this->objectVariables);
                                       /* mark expanded array               */
  memory_mark(this->expansionArray);
  for (arrayPtr = this->objects, endPtr = arrayPtr + this->arraySize; arrayPtr < endPtr; arrayPtr++)
  {
      memory_mark(*arrayPtr);
  }
}

void RexxArray::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->dimensions);
  memory_mark_general(this->objectVariables);
  memory_mark_general(this->expansionArray);

  for (RexxObject **arrayPtr = this->objects; arrayPtr < this->objects + this->arraySize; arrayPtr++)
  {
      memory_mark_general(*arrayPtr);
  }
}

void RexxArray::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxArray)

    flatten_reference(newThis->dimensions, envelope);
    flatten_reference(newThis->objectVariables, envelope);
    flatten_reference(newThis->expansionArray, envelope);
    for (size_t i = 0; i < this->arraySize; i++)
    {
        flatten_reference(newThis->objects[i], envelope);
    }

  cleanUpFlatten
}


void RexxArray::put(RexxObject * eref, size_t pos)
/******************************************************************************/
/* Function:  Place an object in the array                                    */
/******************************************************************************/
{
    OrefSet(this->expansionArray, (this->data())[pos -1], eref);
    // check the last set element
    if (pos > lastElement)
    {
        lastElement = pos;
    }
}

RexxObject  *RexxArray::putRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Performs REXX-level puts to arrays.                             */
/******************************************************************************/
{
    size_t position;                     /* array position                    */

    RexxObject *value = arguments[0];                /* get the value to assign           */
    /* no real value?                    */
    if (argCount == 0 || value == OREF_NULL)
    {
        /* this is an error                  */
        missingArgument(ARG_ONE);         /* this is an error                  */
    }
    /* go validate the index             */
    /* have array expanded if necessary  */
    this->validateIndex(arguments + 1, argCount - 1, 2, RaiseBoundsInvalid | ExtendUpper | RaiseBoundsTooMany, position);

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
    if (this->isNewSpace())
    {
        memset(this->data(), '\0', sizeof(RexxObject *) * this->size());
    }
    else
    {
        // sigh, we have to use OrefSet
        for (size_t i = 0; i < this->size(); i++)
        {

            OrefSet(this, this->objects[i], OREF_NULL);
        }
    }
    // no element set yet
    lastElement = 0;
    return OREF_NULL;     // no real return value
}


/**
 * Test if an array is empty.
 *
 * @return True if the array is empty, false otherwise
 */
RexxObject *RexxArray::isEmpty()
{
    return (items() == 0) ? TheTrueObject : TheFalseObject;
}


/**
 * Append an item after the last item in the array.
 *
 * @param value  The value to append.
 *
 * @return The index of the appended item.
 */
RexxObject  *RexxArray::appendRexx(RexxObject *value)
{
    requiredArgument(value, ARG_ONE);

    // this is not intended for multi-dimensional arrays since they can't expand
    if (this->dimensions != OREF_NULL && this->dimensions->size() != 1)
    {
        reportException(Error_Incorrect_method_array_dimension, CHAR_APPEND);
    }

    size_t newIndex = lastElement + 1;

    ensureSpace(newIndex);
    put(value, newIndex);
    return new_integer(newIndex);
}


/**
 * Append an item after the last item in the array.
 *
 * @param value  The value to append.
 *
 * @return The index of the appended item.
 */
size_t RexxArray::append(RexxObject *value)
{
    size_t newIndex = lastElement + 1;

    ensureSpace(newIndex);
    put(value, newIndex);
    return newIndex;
}


RexxObject  *RexxArray::getRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Performs REXX-level gets from arrays.                           */
/******************************************************************************/
{
    size_t position;                     /* array position                    */
    RexxObject * _result;                /* returned result                   */

                                         /* go validate the index             */
    if (!this->validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid, position))
    {
        _result = TheNilObject;            /* just return .nil                  */
    }
    else
    {                               /* return that element               */
        _result = *(this->data() + position - 1);
        if (_result == OREF_NULL)          /* no object there?                  */
        {
            _result = TheNilObject;          /* just return .nil                  */
        }
    }
    return _result;                      /* return the result                 */
}


/**
 * Get an item from the array, with array size sensitivity.
 *
 * @param position The target position.
 *
 * @return The object at the array position.  Returns OREF_NULL if there
 *         is not item at that position.
 */
RexxObject *RexxArray::getApi(size_t position)
{
    /* out of bounds?                    */
    if (position > this->size())
    {
        return OREF_NULL;
    }
    return get(position);
}


/**
 * Put an array item into an array on behalf of an API.  This will
 * extend the array, if necessary, to accomodate the put.
 *
 * @param o        The inserted object.
 * @param position The target position.
 */
void RexxArray::putApi(RexxObject *o, size_t position)
{
    /* out of bounds?                    */
    if (position > this->size())
    {
        if (position >= MAX_FIXEDARRAY_SIZE)
        {
            reportException(Error_Incorrect_method_array_too_big);
        }
        this->extend(position - this->size());
    }
    put(o, position);
}


bool RexxArray::hasIndexApi(size_t i)
/******************************************************************************/
/* Function:  Determine if an element exist for a position                    */
/******************************************************************************/
{
    /* in bounds and here?               */
    if (i > 0 && i <= this->size() && *(this->data()+i-1) != OREF_NULL)
    {
        return true;                            /* this is true                      */
    }
    else
    {
        return false;                           /* nope, don't have it               */
    }
}


RexxObject *RexxArray::remove(size_t _index)
/******************************************************************************/
/* Function:  Remove an item from the array                                   */
/******************************************************************************/
{
    RexxObject *result;                  /* removed object                    */

                                         /* within the bounds?                */
    if (_index > 0 && _index <= this->size() && *(this->data() + _index - 1) != OREF_NULL)
    {
        /* get the item                      */
        result = *(this->data() + _index - 1);
        /* clear it out                      */
        OrefSet(this->expansionArray, *(this->data() + _index - 1), OREF_NULL);
        // if we removed the last element, we need to scan backwards to find
        // the last one
        if (_index == lastElement)
        {
            // back off at least one position, then scan for the new last one
            lastElement--;
            while (lastElement != 0 && *(this->data() + lastElement - 1) == OREF_NULL)
            {
                lastElement--;
            }
        }
        return result;                     /* and return                        */
    }
    else
    {
        return OREF_NULL;                  /* return nothing                    */
    }
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
    if (!this->validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid, position))
    {
        result = TheNilObject;             /* yup, return .nil.                 */
    }
    else
    {
        /* get the current element           */
        result = *(this->data() + position - 1);
        /* remove the item from the array    */
        OrefSet(this->expansionArray, *(this->data() + position - 1), OREF_NULL);
        if (position == lastElement)
        {
            // back off at least one position, then scan for the new last one
            lastElement--;
            while (lastElement != 0 && *(this->data() + lastElement - 1) == OREF_NULL)
            {
                lastElement--;
            }
        }
        if (result == OREF_NULL)           /* no existing value?                */
        {
            result = TheNilObject;           /* return .nil instead               */
        }
    }
    return result;                       /* return this value                 */
}

size_t RexxArray::items()
/******************************************************************************/
/* Function:  Return count of actual items in an array                        */
/******************************************************************************/
{
    size_t count;                        /* actual count of items in array    */
    RexxArray *realArray;                /* array item pointer                */

    count = 0;                           /* no items yet                      */
    realArray = this->expansionArray;    /* Get array with data               */
                                         /* loop through all array items      */
    for (size_t i = 0; i < realArray->arraySize; i++)
    {
        /* have a real item here?            */
        if (realArray->objects[i] != OREF_NULL)
        {
            count++;                         /* bump the item counter             */
        }
    }
    return count;                        /* return the count object           */
}

RexxObject  *RexxArray::itemsRexx()
/******************************************************************************/
/* Function:  Return count of actual items in an array                        */
/******************************************************************************/
{
   return new_integer(items());
}

size_t RexxArray::getDimension()
/******************************************************************************/
/* Function:  Query array dimension information                               */
/******************************************************************************/
{
    if (this->dimensions == OREF_NULL)   /* no dimensions array?              */
    {
        return 1;                          /* one dimension array               */
    }
    else
    {
        return this->dimensions->size();   /* return size of dimensions array   */
    }
}

RexxObject *RexxArray::dimension(      /* query dimensions of an array      */
     RexxObject *target)               /* dimension to query                */
/******************************************************************************/
/* Function:  Query array dimension information                               */
/******************************************************************************/
{
    if (target == OREF_NULL)
    {           /* non-specific query?               */
        if (this->dimensions == OREF_NULL)
        {
            if (this->size() == 0)
            {
                /* unknown dimension                 */
                return IntegerZero;
            }
            else
            {
                /* one dimension array               */
                return IntegerOne;
            }
        }
        else
        {                             /* return size of dimensions array   */
            return new_integer(this->dimensions->size());
        }
    }
    else
    {
        /* convert to a number               */
        size_t position = target->requiredPositive(ARG_ONE);
        /* asking for dimension of single?   */
        if ((this->dimensions == OREF_NULL) || (this->dimensions->size() == 1))
        {
            if (position == 1)
            {             /* first dimension?                  */
                                               /* just give back the size           */
                return new_integer(this->size());
            }
            else
            {
                /* no size in this dimension         */
                return IntegerZero;
            }
        }
        /* out of range?                     */
        else if (position > this->dimensions->size())
        {
            /* no size in this dimension         */
            return IntegerZero;
        }
        else                               /* return the specific dimension     */
        {
            return this->dimensions->get(position);
        }
    }
}

RexxObject *RexxArray::supplier()
/******************************************************************************/
/* Function:  create a supplier for this array                                */
/******************************************************************************/
{
    size_t slotCount = this->size();            /* get the array size                */
    size_t itemCount = this->items();           /* and the actual count in the array */

    RexxArray *values = new_array(itemCount);       /* get the values array              */
    RexxArray *indexes = new_array(itemCount);      /* and an index array                */

    ProtectedObject v(values);
    ProtectedObject s(indexes);

    size_t count = 1;                           /* next place to add                 */
    for (size_t i = 1; i <= slotCount; i++)
    {   /* loop through the array            */
        RexxObject *item = this->get(i);     /* get the next item                 */
        if (item != OREF_NULL)
        {           /* got an item here                  */
            values->put(item, count);        /* copy over to the values array     */

                                             /* add the index location            */
            indexes->put((RexxObject*)convertIndex(i), count);
            count++;                         /* step the location                 */
        }
    }

    return new_supplier(values, indexes);
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


bool  RexxArray::validateIndex(        /* validate an array index           */
    RexxObject **_index,               /* array index (possibly multi-dim)  */
    size_t       indexCount,           /* size of the index array           */
    size_t       _start,               /* starting point on the array       */
    size_t       bounds_error,         /* raise errors on out-of-bounds     */
    stringsize_t &position)             // returned position
/******************************************************************************/
/* Function:  Process and validate a potentially multi-dimensional array      */
/*            index.  If the index is out of bounds in any dimension it will  */
/*            either return false or raise an error, depending on the bounds  */
/*            checking parameter.                                             */
/******************************************************************************/
{
    RexxObject *value;                   /* individual index value            */
    size_t  numsubs;                     /* number of subscripts              */
    size_t  i;                           /* loop counter                      */
    size_t  multiplier;                  /* accumlation factor                */
    size_t  offset;                      /* accumulated offset                */
    size_t  _dimension;                  /* current working dimension         */
    size_t  numSize;                     /* temporary long variable           */


    // do we really have a single index item given as an array?
    if (indexCount == 1 && _index[0] != OREF_NULL && isOfClass(Array, _index[0]))
    {
        // we process this exactly the same way, but swap the count and
        // pointers around to be the array data.
        RexxArray *indirect = (RexxArray *)_index[0];
        indexCount = indirect->items();
        _index = indirect->data();
    }

    /* Is this array one-dimensional?    */
    if (this->dimensions == OREF_NULL || this->dimensions->size() == 1)
    {
        /* Too many subscripts?  Say so.     */
        if (indexCount > 1)
        {
            /* should the array be extended?     */
            if ((bounds_error & ExtendUpper) && this->dimensions == OREF_NULL)
            {
                /* anytyhing in the array?           */
                /* or explicitly created array with 0*/
                /* elements (.array~new(0))          */
                if (this->size() != 0)
                {
                    /* Yes, number of dims can't change  */
                    /* report apropriate bounds          */
                    reportException(Error_Incorrect_method_maxsub, IntegerOne);
                }
                else
                {
                    /* its empty, entendarray for new siz*/
                    /* extend the array.                 */
                    this->extendMulti(_index, indexCount, _start);
                    /* Call us again to get position, now*/
                    /* That the array is extended.       */
                    return this->validateIndex(_index, indexCount, _start, bounds_error, position);
                }
            }

            else if (bounds_error & RaiseBoundsTooMany)
            {
                /* anytyhing in the array?           */
                /* or explicitly created array with 0*/
                /* elements (.array~new(0))          */
                if (this->dimensions != OREF_NULL || this->size() != 0)
                {
                    /* report apropriate bounds          */
                    reportException(Error_Incorrect_method_maxsub, IntegerOne);
                }
                else
                {
                    return false;                /* just report not here              */
                }
            }
            else
            {
                return false;                  /* not fixed yet, but don't complain */
            }
        }
        /* Too few? subscripts?  Say so.     */
        else if (indexCount == 0)
        {
            /* report apropriate bounds          */
            reportException(Error_Incorrect_method_minarg, _start);
        }
        /* validate integer index            */
        position = _index[0]->requiredPositive((int)_start);
        /* out of bounds?                    */
        if (position > this->size() )
        {
            if (position >= MAX_FIXEDARRAY_SIZE)
            {
                reportException(Error_Incorrect_method_array_too_big);
            }
            /* are we to expand the array?       */
            if (bounds_error & ExtendUpper)
            {
                /* yes, compute amount to expand     */
                this->extend(position - this->size());

            }
            /* need to raise an error?           */
            else if (bounds_error & RaiseBoundsUpper)
            {
                reportException(Error_Incorrect_method_array, position);
            }
            else
            {
                return false;                  /* just return indicator             */
            }
        }
    }
    else
    {                               /* multidimensional array            */
                                    /* get the number of subscripts      */
        numsubs = indexCount;
        numSize = this->dimensions->size();/* Get the size of dimension */
                                           /* right number of subscripts?       */
        if (numsubs == numSize)
        {
            multiplier = 1;                  /* multiply by 1 for first dimension */
            offset = 0;                      /* no accumulated offset             */

            for (i = numsubs; i > 0; i--)
            {  /* loop through the dimensions       */

                value = _index[i - 1];

                if (value == OREF_NULL)        /* not given?                        */
                {
                    /* this is an error too              */
                    reportException(Error_Incorrect_method_noarg, i + _start);
                }
                /* validate integer index            */
                position = value->requiredPositive((int)i);
                /* get the current dimension         */
                _dimension = ((RexxInteger *)this->dimensions->get(i))->getValue();
                if (position > _dimension)
                {   /* too large?                        */
                    /* should the array be extended?     */
                    if (bounds_error & ExtendUpper)
                    {
                        /* go extend it.                     */
                        this->extendMulti(_index, indexCount, _start);
                        /* Call us again to get position, now*/
                        /* That the array is extended.       */
                        return this->validateIndex(_index, indexCount, _start, bounds_error, position);
                    }
                    /* need to raise an error?           */
                    else if (bounds_error & RaiseBoundsUpper)
                    {
                        reportException(Error_Incorrect_method_array, position);
                    }
                    else
                    {
                        return false;              /* just return indicator             */
                    }
                }
                /* calculate next offset             */
                offset += multiplier * (position - 1);
                multiplier *= _dimension;      /* step the multiplier               */
            }
            position = offset + 1;           /* get accumulated position          */
        }
        /* Not enough subscripts?            */
        else if (numsubs < numSize)
        {
            reportException(Error_Incorrect_method_minsub, numSize);
        }
        else                               /* Must be too many subscripts       */
                                           /* should the array be extended?     */
#ifdef EXTEND_DIMENSIONS
            if (bounds_error & ExtendUpper)
        {
            /* anytyhing in the array?           */
            if (this->size() != 0)
            {
                /* Yes, number of dims can't change  */
                /* report apropriate bounds          */
                reportException(Error_Incorrect_method_maxarg, numSize);
            }
            else
            {
                /* array empty, extend the array     */
                this->extendMuti(_index, indexCount, _start);
                /* Call us again to get position, now*/
                /* That the array is extended.       */
                return this->validateIndex(_index, indexCount, _start, bounds_error, position);
            }
        }
        else
        {
            reportException(Error_Incorrect_method_maxsub, numSize);
        }
#else
            reportException(Error_Incorrect_method_maxsub, numSize);
#endif
    }
    return true;                         /* return the position               */
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
            reportException(Error_Incorrect_method_array_too_big);
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
    return new_integer(this->size());
}

RexxArray *  RexxArray::section(size_t _start, size_t _end)
/******************************************************************************/
/* Function:  Section an array for a given size                               */
/******************************************************************************/
{
    size_t newSize;                      /* Size for new array.               */
    RexxArray *newArray;                 /* The new array.                    */

    if (_start == 0)                      /* starting position specified?      */
    {
        _start = 1;                         /* nope, start at begining           */
    }

    /* ending position omitted           */
    /* or, End pos past end of array?    */
    if (_end == 0 || _end > this->size())
    {
        _end = this->size();                /* yes,  use end of array            */
    }
    if (_start <= _end)
    {                  /* is start before end?              */
        newSize = _end + 1 - _start;         /* yes, compute new size             */
        /* Get new array, of needed size     */
        newArray = (RexxArray *)new_array(newSize);
        // a new array cannot be oldspace, by definition.  It's safe to use
        // memcpy to copy the data.
        /* yes, we can do a memcpy           */
        memcpy(newArray->data(), &(this->expansionArray->objects[_start-1]), sizeof(RexxObject *) * newSize);
    }
    else
    {
        /* return 0 element array            */
        newArray = (RexxArray *)new_array((size_t)0);
    }
    return newArray;                     /* return the newly created array    */
}

RexxObject *RexxArray::sectionRexx(
            RexxObject * _start,
            RexxObject * _end)
/******************************************************************************/
/* Function:  Rexx level section method                                       */
/******************************************************************************/
{
    size_t nstart, nend, i;
    RexxArray *rref;


    requiredArgument(_start, ARG_ONE);   /* need a start position             */
                                         /* Start specified - check it out    */
    nstart = _start->requiredPositive(ARG_ONE);
    if (_end == OREF_NULL)                /* If no end position specified,     */
    {
        nend = this->size();               /* Defaults to last element          */
    }
    else
    {                               /* End specified - check it out      */
        nend = _end->requiredNonNegative(ARG_TWO);
    }
    /* multidimensional array?           */
    if (this->dimensions != OREF_NULL && this->dimensions->size() != 1)
    {
        /* this is an error                  */
        reportException(Error_Incorrect_method_section);
    }
    if (!isOfClass(Array, this))             /* actually an array subclass?       */
    {
        /* need to do this the slow way      */
        return this->sectionSubclass(nstart, nend);
    }
    if (nstart > this->size())           /* too big?                          */
    {

        rref = (RexxArray *)(((RexxArray *)TheNullArray)->copy());
    }
    /* return zero elements              */
    else
    {
        /* go past the bounds?               */
        if (nend > this->size() - nstart + 1)
        {
            nend = this->size() - nstart + 1;/* truncate to the end               */
        }
        if (nend == 0)                     /* requesting zero?                  */
        {
            /* return zero elements              */
            rref = (RexxArray *)(((RexxArray *)TheNullArray)->copy());
        }
        else
        {                             /* real sectioning to do             */
                                      /* create a new array                */
            rref = (RexxArray *)new_array(nend);
            for (i = 1; i <= nend; i++)
            {    /* loop through the elements         */
                 /* copy an element                   */
                rref->put(this->get(nstart + i - 1), i);
            }
        }
    }
    return rref;                         /* return the new array              */
}

RexxObject *RexxArray::sectionSubclass(
    size_t _start,                      /* starting element                  */
    size_t _end )                       /* ending element                    */
/******************************************************************************/
/* Function:  Rexx level section method                                       */
/******************************************************************************/
{
    size_t i;                            /* loop counter                      */
    RexxArray *newArray;                 /* returned array                    */
    ProtectedObject result;

    if (_start > this->size())           /* too big?                          */
    {
        this->behaviour->getOwningClass()->sendMessage(OREF_NEW, IntegerZero, result);
        newArray = (RexxArray *)(RexxObject *)result;
    }
    /* return a zero element one         */
    else
    {
        if (_end > this->size() - _start + 1)
        {
            /* go past the bounds?               */
            _end = this->size() - _start + 1;/* truncate to the end               */
        }
        if (_end == 0)                     /* requesting zero?                  */
        {
            this->behaviour->getOwningClass()->sendMessage(OREF_NEW, IntegerZero, result);
            newArray = (RexxArray *)(RexxObject *)result;
        }
        /* return a zero element one         */
        else
        {                             /* real sectioning to do             */
                                      /* create a new array                */
            this->behaviour->getOwningClass()->sendMessage(OREF_NEW, new_integer(_end), result);
            newArray = (RexxArray *)(RexxObject *)result;
            for (i = 1; i <= _end; i++)       /* loop through the elements         */
            {
                /* copy an element                   */
                newArray->sendMessage(OREF_PUT, this->get(_start + i - 1), new_integer(i));
            }
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
    RexxObject **thisObject;
    size_t  _arraySize;                   /* size of the array                 */

    /* get the address of the first      */
    /*element in the array               */
    thisObject = this->expansionArray->objects;
    _arraySize = this->size();            /* get the size of the array         */
    /* find first position in the        */
    /*array with data                    */
    for (i = 0; i < _arraySize && thisObject[i] == OREF_NULL; i++);

    if (i == _arraySize)                  /* is array empty                    */
    {
        return TheNilObject;              /* return nil object                 */
    }
    else
    {
        /* return index of the first entry   */
        return convertIndex(i + 1);
    }
}

RexxObject  *RexxArray::lastRexx(void)
/******************************************************************************/
/* Function:  Return the index of the last array item as an integer object    */
/******************************************************************************/
{
    // for an empy array, the index is .nil
    if (lastElement == 0)
    {
        return TheNilObject;
    }

    // return as an object
    return (RexxObject *)convertIndex(lastElement);
}

size_t RexxArray::lastIndex()
/******************************************************************************/
/* Function:  Return the index of the last array item as an integer object    */
/******************************************************************************/
{
    return lastElement;       // we've kept track of this
}

RexxObject  *RexxArray::nextRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Return the next entry after a given array index                 */
/******************************************************************************/
{
    size_t i;
    stringsize_t position;
    /* go validate the index             */
    if (!this->validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid, position))
    {
        // out of bounds results in the .nil object
        return TheNilObject;
    }
    /* get the address of the first      */
    /*element in the array               */
    RexxObject **thisObject = this->data();
    size_t _arraySize = this->size();            /* get the size of the array         */
    /* find next entry in the array with */
    /*data                               */
    for (i = position; i < _arraySize && thisObject[i] == OREF_NULL; i++);

    if (i >= this->size())
    {
        return TheNilObject;             /* return nil object                 */
    }
    else
    {
        /* return index of the next entry    */
        return convertIndex(i + 1);
    }
}

RexxObject  *RexxArray::previousRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Return the index preceeding a given index                       */
/******************************************************************************/
{
    size_t i;                            /* counter for the loop              */
    RexxObject **thisObject;
    size_t  _arraySize;                   /* size of the array                 */
    stringsize_t position;

    this->validateIndex(arguments, argCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid, position);
    /* get the index object into an      */
    /*integer object                     */
    i = position;

    _arraySize = this->size();            /* get the size of the array         */

    if (i > _arraySize)                   /* beyond the size of the array?     */
    {
        /* set i to one more than the last   */
        /*entry                              */
        i = _arraySize;
    }
    else
    {
        i = i-1;                           /* Account for 0 based 'C' arrays    */
    }

                                           /* get the address of the first      */
                                           /*element in the array               */
    thisObject = this->expansionArray->objects;
    /* find previous entry in the        */
    /*array with data                    */
    for (; i > 0 && thisObject[i-1] == OREF_NULL; i--);

    if (i == 0)
    {
        return TheNilObject;                /* return nil object                 */
    }
    else
    {
        /* return the index to the           */
        /*previous entry                     */
        return convertIndex(i);
    }
}


RexxInteger *RexxArray::hasIndex(RexxObject * _index)
/******************************************************************************/
/* Function:  Determine if an individual array element exists                 */
/******************************************************************************/
{
    size_t i;

    if (isOfClass(Integer,_index) && (i = ((RexxInteger *)_index)->getValue()) > 0 && i <= this->size() &&
        *(this->data()+i-1) != OREF_NULL)
    {
        return TheTrueObject;
    }
    else
    {
        return TheFalseObject;
    }
}

RexxObject  *RexxArray::hasIndexRexx(RexxObject ** _index, size_t _indexCount)
/******************************************************************************/
/*  Function:  True if array has an entry for the index, false otherwise      */
/*  Note:  This routine should not raise an error, regardless of the indices  */
/*         being used.  The only error produced is if no parms were passed.   */
/******************************************************************************/
{
    stringsize_t position;               /* array position                    */

                                         /* go validate the index             */
    if (!this->validateIndex(_index, _indexCount, 1, RaiseBoundsTooMany | RaiseBoundsInvalid, position))
    {
        /* this is false                     */
        return TheFalseObject;

    }
    else                                 /* check the position                */
    {
        /* have a real entry?                */
        if (*(this->data() + position - 1) != OREF_NULL)
        {
            /* got a true                        */
            return TheTrueObject;
        }
        else
        {
            /* no index here                     */
            return TheFalseObject;
        }
    }
}

bool RexxArray::hasIndexNative(size_t _index)
/******************************************************************************/
/* Function:  Determine if an element exist for a position                    */
/******************************************************************************/
{
    /* in bounds and here?               */
    if (_index > 0 && _index <= this->size() && *(this->data() + _index - 1) != OREF_NULL)
    {
        return true;                       /* this is true                      */
    }
    else
    {
        return false;                      /* nope, don't have it               */
    }
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
    RexxArray *newArray = (RexxArray *)new_array(this->items());

    // we need to fill in based on actual items, not the index.
    size_t count = 0;
    RexxObject **item = this->data();
    // loop through the array, copying all of the items.
    for (size_t iterator = 0; iterator < this->size(); iterator++ )
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
RexxArray *RexxArray::allIndexes()
{
    // get a result array of the appropriate size
    RexxArray *newArray = (RexxArray *)new_array(this->items());
    ProtectedObject p(newArray);

    // we need to fill in based on actual items, not the index.
    size_t count = 0;
    RexxObject **item = this->data();
    // loop through the array, copying all of the items.
    for (size_t iterator = 0; iterator < this->size(); iterator++ )
    {
        // if this is a real array item, add an integer index item to the
        // result collection.
        if (item[iterator] != OREF_NULL)
        {
            newArray->put(convertIndex(iterator+1), ++count);
        }
    }
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
    size_t _items;
    size_t i;
    RexxArray *newArray;                  /* New array                         */
    RexxString *newString;
    RexxString *line_end_string;          /* converted substitution value      */
    RexxMutableBuffer *mutbuffer;
    RexxObject *item;                     /* inserted value item               */
    int i_form = 0;                       /* 1 == line, 2 == char              */

    mutbuffer = ((RexxMutableBufferClass*) TheMutableBufferClass)->newRexx(NULL, 0);
    ProtectedObject p1(mutbuffer);

    newArray = this->makeArray();          /* maybe multidimensional, make onedimensional  */
    ProtectedObject p2(newArray);

    _items = newArray->items();            /* and the actual count in the array */

    if (format != OREF_NULL)
    {
        // a string value is required here
        format = stringArgument(format, ARG_ONE);
    }

    if (format == OREF_NULL)
    {
        i_form = 2;                         /* treat item as LINE by default   */
    }
    else if (toupper((format->getStringData()[0])) == 'C')
    {
        i_form = 1;
    }
    else if (toupper((format->getStringData()[0])) == 'L')
    {
        i_form = 2;
    }
    else
    {
        reportException(Error_Incorrect_method_option, "CL", format);
    }

    if (i_form == 1)                       /* character oriented processing    */
    {
        if (separator != OREF_NULL)
        {
            reportException(Error_Incorrect_method_maxarg, IntegerOne);

        }

        for (i = 1; i <=_items ; i++)         /* loop through the array           */
        {
            item = newArray->get(i);           /* get the next item                */
            if (item != OREF_NULL)
            {
                RexxObject * _stringValue = item->requiredString();
                if (_stringValue != TheNilObject)
                {
                    mutbuffer->append(_stringValue);
                }
            }
        }
    }
    else if (i_form == 2)                 /* line oriented processing          */
    {
        if (separator != OREF_NULL)
        {
            line_end_string = stringArgument(separator, ARG_TWO);
        }
        else
        {
            line_end_string = new_string(line_end);
        }

        ProtectedObject p3(line_end_string);
        bool first = true;

        for (i = 1; i <= _items; i++)      /* loop through the array            */
        {
            item = newArray->get(i);       /* get the next item                 */
            if (item != OREF_NULL)
            {
                // append a linend between the previous item and this one.
                if (!first)
                {
                    mutbuffer->append((RexxObject *) line_end_string);
                }
                RexxObject *_stringValue = item->requiredString();
                if (_stringValue != TheNilObject)
                {
                    mutbuffer->append(_stringValue);
                }
                first = false;
            }
        }
    }

    newString = mutbuffer->makeString();
    return newString;
}

RexxObject *RexxArray::join(           /* join two arrays into one          */
     RexxArray *other)                 /* array to be appended to self      */
/******************************************************************************/
/* Function:  Join two arrays into one array                                  */
/******************************************************************************/
{
    /* get new array, total size is size */
    /* of both arrays.                   */
    RexxArray *newArray = (RexxArray*)new_array(this->size() + other->size());
    // it's safe to just copy the references because the newArray will be new space
    /* copy first array into new one     */
    memcpy(newArray->data(), this->data(), ((char *)&(this->data()[this->size()])) - ((char *)this->data()));
    /* copy 2nd array into the new one   */
    /* after the first one.              */
    memcpy((void *)&(newArray->data()[this->size()]), other->data(), ((char *)&(other->data()[other->size()])) - ((char *)other->data()));
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
    if (this->expansionArray == this)
    {
        /* no, then we resize the array      */
        /* is this array in OldSpace ?       */
        if (this->isOldSpace())
        {
            /* Old Space, remove any reference   */
            /* to new space from memory tables   */
            for (i = 0; i < this->arraySize; i++)
            {
                OrefSet(this, this->objects[i], OREF_NULL);
            }
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
    size_t _size = this->size();                 /* get the size                      */
    size_t newSize = _size - amount;             /* get the new size                  */

    size_t i = newSize + 1;                     /* address first removed element     */
    for (i = newSize + 1; i <= _size; i++)/* for all removed elements          */
    {
        this->put(OREF_NULL, i);           /* clear out the element             */
    }
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
    size_t _size = this->size();                /* get the array size                */
    for (size_t i = 1; i <= _size; i++)
    {       /* spin through the array            */
        if (this->get(i) == target)        /* is this the one?                  */
        {
            return i;                        /* return the index                  */
        }
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
    size_t _size = this->size();                /* get the array size                */
    /* spin through the array            */
    for (size_t i = targetIndex; i < _size; i++)
    {
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
    this->extend(1);                     /* extend the array size             */
                                         /* spin through the array            */
    for (size_t i = this->size(); i > targetIndex; i--)
    {
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
    /* do we really need to extend array */
    /* or just adjust size.              */
    if (this->size() + extension <= this->maximumSize)
    {
        /* adjust the size .                 */
        this->expansionArray->arraySize += extension;
        return this;
    }

    size_t newSize = this->size() + extension;
    size_t extendSize = this->size() / 2;

    /* get a new array, total size is    */
    /* size of both arrays.              */
    RexxArray *newArray = (RexxArray *)new_array(newSize + extendSize);
    /* If none of the objects are in     */
    /* OldSpace,  we can skip the        */
    /* OrefSets and just copy            */
    /* copy ourselves into the new array */
    memcpy(newArray->data(), this->data(), ((char *)&(this->data()[this->size()])) - ((char *)this->data()));
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
size_t RexxArray::findSingleIndexItem(RexxObject *item)
{
    for (size_t i = 1; i <= this->size(); i++)
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
    RexxArray * _index = new_array(dims);

    ProtectedObject p(_index);

    for (size_t i = dims; i > 0; i--)
    {
        // get the next dimension size
        size_t _dimension = ((RexxInteger *)this->dimensions->get(i))->getValue();
        // now get the remainder.  This tells us the position within this
        // dimension of the array.  Make an integer object and store in the
        // array.
        size_t digit = idx % _dimension;
        // the digit is origin-zero, but the Rexx index is origin-one.
        _index->put(new_integer(digit + 1), i);
        // now strip out that portion of the index.
        idx = (idx - digit) / _dimension;
    }
    return _index;
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
    requiredArgument(target, ARG_ONE);
    // see if we have this item.  If not, then
    // we return .nil.
    size_t _index = findSingleIndexItem(target);

    if (_index == 0)
    {
        return TheNilObject;
    }

    return convertIndex(_index);
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
    requiredArgument(target, ARG_ONE);
    // see if we have this item.  If not, then
    // we return .nil.
    size_t _index = findSingleIndexItem(target);

    if (_index == 0)
    {
        return TheNilObject;
    }
    // remove the item at the location
    put(OREF_NULL, _index);
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
    requiredArgument(target, ARG_ONE);
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
    if (newDimension == parm->firstChangedDimension)
    {
        /* is new array in OldSpace?         */
        if (parm->newArray->isOldSpace())
        {
            /* Yes,need to do OrefSets           */
            /* For each element to copy          */
            for (i = 1; i <= parm->copyElements; i++, parm->startNew++, parm->startOld++ )
            {
                /* set the newvalue.                 */
                OrefSet(parm->newArray, *parm->startNew, *parm->startOld);
            }
        }
        else
        {
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
    else
    {
        /* Compute the old dimension num     */
        oldDimension = newDimension - parm->deltaDimSize;
        /* Get size for new Dimension        */
        newDimSize = ((RexxInteger *)parm->newDimArray->get(newDimension))->getValue();
        /* Get size for old Dimension        */
        oldDimSize = ((RexxInteger *)parm->oldDimArray->get(oldDimension))->getValue();
        /* For each subscript at this        */
        for (i= 1; i <= oldDimSize; i++)
        {/* dimension, (of old size)          */
         /* copy elelments.                   */
            copyElements(parm, newDimension + 1);
        }
        if (newDimSize > oldDimSize)
        {    /* Was this dimension expanded?      */
             /* compute total space need for      */
             /* block of all lower dimensions     */
            for (i = parm->newDimArray->size(), skipAmount = 1;
                i > newDimension;
                skipAmount *= ((RexxInteger *)parm->newDimArray->get(i))->getValue(), i--);
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
     RexxObject ** _index,             /* Dimension array is to be          */
     size_t _indexCount,               /* number of indices in the index    */
     size_t _start)                    /* Starting position of dimensions   */
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
    newDimArraySize = _indexCount;
    newDimArray = new_array(newDimArraySize);
    ProtectedObject p(newDimArray);
    /* extending from single Dimensio    */
    /*  to a multi Dimensionsal array    */
    if (this->dimensions == OREF_NULL)
    {
        /* Get value for 1st dimension       */
        /*  its the last element             */
        i = newDimArraySize - 1;
        newDimSize = _index[i]->requiredPositive((int)i);
        /* Yes, is 1st Dimension going to    */
        /* be bigger than current size?      */
        if (newDimSize > this->size())
            /* Yes, use new size + buffer for    */
            /*  1st Dimension                    */
            newDimArray->put(new_integer(newDimSize), newDimArraySize);
        else
        {
            /* nope, use same size for Dim       */
            tempSize = this->size();
            newDimArray->put(new_integer(tempSize), newDimArraySize);
        }
    }
    else
    {
        for (oldDimension = this->dimensions->size(), newDimension = newDimArraySize;
            oldDimension > 0 ;
            oldDimension--, newDimension--)
        {
            /* Get current size of this dimension*/
            currDimSize = ((RexxInteger *)this->dimensions->get(oldDimension))->getValue();
            /* Get indexd  size of this dimension*/

            newDimSize = _index[newDimension - 1]->requiredPositive((int)newDimension);
            /* does this dimension need to be    */
            /*  expanded.                        */
            if (newDimSize > currDimSize)
            {
                newDimArray->put((RexxObject *)new_integer(newDimSize), newDimension);
                /* has a dimension already been chang*/
                if (!firstDimChanged)
                {
                    /* remember the first dimenion chenge*/
                    firstDimChanged = newDimension;
                }
            }
            else
            {
                newDimArray->put(this->dimensions->get(oldDimension), newDimension);
            }
        }
    }
    /* Was original array single dim     */
    if (this->dimensions == OREF_NULL)
    {
        /* additional Dimensions is 1        */
        /*  minus size, (1st Dimension)      */
        additionalDim = newDimArraySize - 1;
    }
    else
    {
        /* compute number of dimensions added*/
        additionalDim = newDimArraySize - this->dimensions->size();
    }
    /* is index greater than current     */
    /*  dimensions of this array.        */
    if (additionalDim > 0)
    {
        /* yes, for remainder of dimensions  */
        for (newDimension = additionalDim;
            newDimension > 0 ;
            newDimension--)
        {
            /* Get indexd  size of this dimension*/
            newDimSize = ((RexxInteger *)_index[newDimension - 1])->getValue();
            /* set up value for this dimension   */
            newDimArray->put(new_integer(newDimSize), newDimension);
        }
    }
    /* Now create the new array for this */
    /*  dimension.                       */
    newArray = new (newDimArray->data(), newDimArraySize, TheArrayClass) RexxArray;
    ProtectedObject p1(newArray);
    /* Anything in original?             */
    if (this->size())
    {
        /* Yes, move values into new arra    */
        /* Extending from single Dimension   */
        /* or original array was empty       */
        /* or adding dimensions or increas   */
        /* last original dimension?          */
        if (this->dimensions == OREF_NULL || this->dimensions->size() == 1 ||
            this->size() == 0 ||
            !firstDimChanged || firstDimChanged <= additionalDim + 1)
        {
            /* If none of the objects are in     */
            /* OldSpace,  we can 'cheat' and do  */
            /* a fast copy (memcpy)              */
            /* copy ourselves into the new array */
            memcpy(newArray->data(), this->data(), sizeof(RexxObject *) * this->size());
        }
        /* Working with 2 multi-dimensional  */
        /* Array's.                          */
        else
        {
            /* Now we need to move all elements  */

            /* for all Dimensions before first   */
            /* to increase.                      */
            for (i = newDimArraySize, accumSize = 1;
                i > firstDimChanged ;
                accumSize *= ((RexxInteger *)this->dimensions->get(i))->getValue(), i--);
            /* Compute lowest largest contig     */
            /* chunk that can be copied.         */
            copyParm.copyElements = accumSize * ((RexxInteger *)this->dimensions->get(firstDimChanged))->getValue();
            /* Compute amount need to ship       */
            /* to compete expanded dimension     */
            copyParm.skipElements = accumSize * (((RexxInteger *)newDimArray->get(firstDimChanged))->getValue() -
                                                 ((RexxInteger *)this->dimensions->get(firstDimChanged))->getValue());

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
    return this;                         /* All done, return array            */
}


RexxObject *RexxArray::insert(         /* insert an element into an array   */
     RexxObject *value,                /* value to be inserted              */
     size_t  _index )                  /* index of the insertion point      */
/******************************************************************************/
/* Function:  Insert an element into an array                                 */
/******************************************************************************/
{
    RexxObject ** _start;                /* move starting point               */
    RexxObject ** _end;                  /* move ending point                 */

                                         /* get the address of first element  */
    _start = &(this->data()[_index - 1 ]);
    /* and the last element              */
    _end = &(this->data()[this->size() - 1]);
    /* shift the array over              */
    memmove(_start + sizeof(RexxObject *), _start, _end - _start);
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
    this->data()[_index - 1] = OREF_NULL;
    this->put(value, _index);             /* add the new element in            */
    return value;                        /* return the inserted value         */
}

void *   RexxArray::operator new(size_t size,
    size_t items,                      /* items in the array                */
    RexxObject **first )               /* array of items to fill array      */
/******************************************************************************/
/* Quick creation of an argument array                                        */
/******************************************************************************/
{
    RexxArray *newArray = new_array(items);         /* get a new array                   */
    if (items != 0)                      /* not a null array?                 */
    {
        /* copy the references in            */
        memcpy(newArray->data(), first, (sizeof(RexxObject *) * items));
        // we need to make sure the lastElement field is set to the
        // new arg position.
        for (;items > 0; items--)
        {
            if (newArray->get(items) != OREF_NULL)
            {
                newArray->lastElement = items;
                break;
            }
        }
    }
    return newArray;                     /* return the new array              */
}

void *RexxArray::operator new(size_t size, RexxObject **args, size_t argCount, RexxClass *arrayClass)
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



    if (argCount == 0)
    {
        /* If no argument is passed          */
        /*  create an empty array.           */
        temp = new ((size_t)0, ARRAY_DEFAULT_SIZE, arrayClass) RexxArray;
        ProtectedObject p(temp);
        temp->sendMessage(OREF_INIT);      /* call any rexx init's              */
        return temp;
    }

    /* Special case for 1-dimensional    */
    if (argCount == 1)
    {
        current_dim = (RexxInteger *)args[0];
        /* Make sure it's an integer         */
        total_size = current_dim->requiredNonNegative(ARG_ONE, number_digits());
        if (total_size < 0)
        {
            reportException(Error_Incorrect_method_array, total_size);
        }

        if (total_size >= MAX_FIXEDARRAY_SIZE)
        {
            reportException(Error_Incorrect_method_array_too_big);
        }

        /* Note: The following will leave the dimension field set to OREF_NULL, */
        /* which is what we want (it will be done by array_init above).         */
        /* Create new array of approp. size. */

        temp = (RexxArray *)new_externalArray(total_size, arrayClass);
        ProtectedObject p(temp);
        if (total_size == 0)
        {             /* Creating 0 sized array?           */
                      /* Yup, setup a Dimension array      */
                      /* single Dimension, Mark so         */
                      /* can't change Dimensions.          */
            OrefSet(temp, temp->dimensions, new_array(IntegerZero));
        }
        temp->sendMessage(OREF_INIT);      /* call any rexx init's              */
        return temp;                       /* Return the new array.             */
    }
    /* Working with a multi-dimension    */
    /*  array, so get a dimension arr    */
    dim_array = (RexxArray *)new_array(argCount);
    total_size = 1;                      /* Set up for multiplication         */
    for (i=0; i < argCount; i++)
    {
        /* make sure current parm is inte    */
        current_dim = (RexxInteger *)args[i];
        if (current_dim == OREF_NULL)      /* was this one omitted?             */
            missingArgument(i+1);           /* must have this value              */
                                             /* get the long value                */
        cur_size = current_dim->requiredNonNegative((int)(i+1));
        /* going to do an overflow?          */
        if (cur_size != 0 && ((MAX_FIXEDARRAY_SIZE / cur_size) < total_size))
        {
            /* this is an error                  */
            reportException(Error_Incorrect_method_array_too_big);
        }
        total_size *= cur_size;            /* keep running total size           */
                                           /* Put integer object into curren    */
                                           /* dimension array position          */
        dim_array->put(new_integer(cur_size), i+1);
    }
    /* Make sure flattened array isn't   */
    /*  too big.                         */
    if (total_size >= MAX_FIXEDARRAY_SIZE)
    {
        reportException(Error_Incorrect_method_array_too_big);
    }
    /* Create the new array              */
    temp = (RexxArray *)new_externalArray(total_size, arrayClass);
    /* put dimension array in new arr    */
    OrefSet(temp, temp->dimensions, dim_array);
    ProtectedObject p(temp);
    temp->sendMessage(OREF_INIT);        /* call any rexx init's              */
    return temp;
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
void RexxArray::mergeSort(BaseSortComparator &comparator, RexxArray *working, size_t left, size_t right)
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
void RexxArray::merge(BaseSortComparator &comparator, RexxArray *working, size_t left, size_t mid, size_t right)
{
    size_t leftEnd = mid - 1;
    size_t elements = right - left + 1;
    size_t mergePosition = left;

    while ((left <= leftEnd) && (mid <= right))
    {
        RexxObject *leftItem = get(left);
        RexxObject *midItem = get(mid);
        if (comparator.compare(leftItem, midItem) <= 0)
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
 * Interchange two items in an array.
 *
 * @param i1     The first index to swap.
 * @param i2     The second index to swap.
 */
void RexxArray::interchange(size_t i1, size_t i2)
{
    RexxObject *temp = get(i1);
    put(get(i2), i1);
    put(temp, i2);
}


/**
 * Perform an insertion sort one a section of an array.
 *
 * @param comparator The comparator used for comparing two array items.
 * @param bounds     The bounds of the partition to be sorted.
 */
void RexxArray::insertionSort(BaseSortComparator &comparator, PartitionBounds &bounds)
{
    for (size_t i = bounds.left; i <= bounds.right; i++) {
        // get an item to insert
        RexxObject *current = get(i);
        // and a new position to scan through
        size_t scan = i;
        while ((scan > bounds.left) && comparator.compare(get(scan - 1), current) > 0) {
            put(get(scan - 1), scan);
            scan--;
        }
        put(current, scan);
    }
}


/**
 * Split a partition into sections.
 *
 * @param comparator The comparator used to comparing items.
 * @param bounds     The current partition bounds for the spli.
 *
 * @return The split position for the partition.
 */
size_t RexxArray::split(BaseSortComparator &comparator, PartitionBounds &bounds)
{
    size_t s;
    size_t g;
    size_t x;

    PartitionBounds mid;

    // here, atleast three elements are needed
    if (comparator.compare(get(bounds.left), get(bounds.midPoint())) < 0)
    {
        s = bounds.left;
        g = bounds.midPoint();
    }
    else
    {
        g = bounds.left;
        s = bounds.midPoint();
    }

    if (comparator.compare(get(bounds.right), get(s)) <= 0)
    {
        x = s;
    }
    else if (comparator.compare(get(bounds.right), get(g)) <= 0)
    {
        x = bounds.right;
    }
    else {
        x = g;
    }

    // swap the split-point element
    // with the first
    interchange(x, bounds.left);

    // this is our pivot element
    RexxObject *pivot = get(bounds.left);
    // set the scanning points
    size_t i = bounds.left;
    size_t j = bounds.right + 1;
    // now find the pivot point
    while (i < j)
    {
        do {                                 // find j
            j--;
        }
        while (i < j && comparator.compare(get(j), pivot) > 0);
        do {
            i++;                             // find i
        } while (i < j && comparator.compare(get(i), pivot) < 0);
        interchange(i, j);
    }
    interchange(i, j);                       // undo the extra swap
    interchange(bounds.left, j);             // bring the split-point
                                             // element to the first
    return j;                                // this is our new pivot point
}


/**
 * Recursive quick sort routine for sorting a partition.
 *
 * @param left   The left bound of the partition.
 * @param right  The right bound of the partition.
 */
void RexxArray::quickSort(BaseSortComparator &comparator, size_t left, size_t right)
{
    std::deque<PartitionBounds>stack;  // stack for non-recursive quick sort
    PartitionBounds bounds(left, right);    // initial bounds

    stack.push_front(bounds);          // push the initial bounds

    // keep processing until the stack dries up
    while (!stack.empty())
    {
        // get the top stack frame
        bounds = stack.front();
        stack.pop_front();
        for (;;)
        {
            // if this is a larger range, then we need to partition
            if (!bounds.isSmall())
            {
                // go find the new pivot point
                size_t splitpoint = split(comparator, bounds);
                // push the smaller list
                if (bounds.right - splitpoint < bounds.left - splitpoint)
                {
                    stack.push_front(PartitionBounds(bounds.left, splitpoint - 1));
                    bounds.left = splitpoint + 1;
                }
                else
                {
                    stack.push_front(PartitionBounds(splitpoint + 1, bounds.right));
                    bounds.right = splitpoint - 1;
                }
            }
            else {
                // sort the smaller partitions via insertion sort
                insertionSort(comparator, bounds);
                // we break out of this once we find a small section we can sort
                break;
            }
        }
    }
}


/**
 * Sort elements of the array in place, using a quicksort.
 *
 * @return Returns the same array, with the elements sorted.
 */
RexxArray *RexxArray::sortRexx()
{
    size_t count = items();
    if (count == 0)         // if the count is zero, sorting is easy!
    {
        return this;
    }

    // make sure this is a non-sparse array.  Checking up front means we don't
    // need to check on each compare operation.
    for (size_t i = 1; i <= count; i++)
    {
        if (get(i) == OREF_NULL)
        {
            reportException(Error_Execution_sparse_array, i);
        }
    }

    BaseSortComparator comparator;

    // go do the quick sort
    quickSort(comparator, 1, count);
    return this;
}


/**
 * Sort elements of the array in place, using a quicksort.
 *
 * @return Returns the same array, with the elements sorted.
 */
RexxArray *RexxArray::sortWithRexx(RexxObject *comparator)
{
    requiredArgument(comparator, ARG_ONE);

    size_t count = items();
    if (count <= 1)         // if the count is zero, sorting is easy!
    {
        return this;
    }

    // make sure this is a non-sparse array.  Checking up front means we don't
    // need to check on each compare operation.
    for (size_t i = 1; i <= count; i++)
    {
        if (get(i) == OREF_NULL)
        {
            reportException(Error_Execution_sparse_array, i);
        }
    }

    WithSortComparator c(comparator);

    // go do the quick sort
    quickSort(c, 1, count);
    return this;
}


/**
 * Sort elements of the array in place, using a quicksort.
 *
 * @return Returns the same array, with the elements sorted.
 */
RexxArray *RexxArray::stableSortRexx()
{
    size_t count = items();
    if (count == 0)         // if the count is zero, sorting is easy!
    {
        return this;
    }

    // make sure this is a non-sparse array.  Checking up front means we don't
    // need to check on each compare operation.
    for (size_t i = 1; i <= count; i++)
    {
        if (get(i) == OREF_NULL)
        {
            reportException(Error_Execution_sparse_array, i);
        }
    }

    // the merge sort requires a temporary scratch area for the sort.
    RexxArray *working = new_array(count);
    ProtectedObject p(working);

    BaseSortComparator comparator;

    // go do the quick sort
    mergeSort(comparator, working, 1, count);
    return this;
}


/**
 * Sort elements of the array in place, using a quicksort.
 *
 * @return Returns the same array, with the elements sorted.
 */
RexxArray *RexxArray::stableSortWithRexx(RexxObject *comparator)
{
    requiredArgument(comparator, ARG_ONE);

    size_t count = items();
    if (count <= 1)         // if the count is zero, sorting is easy!
    {
        return this;
    }

    // make sure this is a non-sparse array.  Checking up front means we don't
    // need to check on each compare operation.
    for (size_t i = 1; i <= count; i++)
    {
        if (get(i) == OREF_NULL)
        {
            reportException(Error_Execution_sparse_array, i);
        }
    }

    // the merge sort requires a temporary scratch area for the sort.
    RexxArray *working = new_array(count);
    ProtectedObject p(working);

    WithSortComparator c(comparator);

    // go do the quick sort
    mergeSort(c, working, 1, count);
    return this;
}


void *  RexxArray::operator new(size_t size, RexxObject *first)
/******************************************************************************/
/* Function:  Create an array with 1 element (new_array1)                     */
/******************************************************************************/
{
  RexxArray *aref;

  aref = (RexxArray *)new_array(1);
  aref->put(first, 1);

  return aref;
}

void *   RexxArray::operator new(size_t size, RexxObject *first, RexxObject *second)
{
  RexxArray *aref;

  aref = new_array(2);
  aref->put(first, 1);
  aref->put(second, 2);
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
  aref->put(first,  1);
  aref->put(second, 2);
  aref->put(third,  3);

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
  aref->put(first,  1);
  aref->put(second, 2);
  aref->put(third,  3);
  aref->put(fourth, 4);

  return aref;
}

void *RexxArray::operator new(size_t newSize, size_t size, size_t maxSize, RexxClass *arrayClass)
/******************************************************************************/
/* Function:  Low level array creation                                        */
/******************************************************************************/
{
    size_t bytes;
    RexxArray *newArray;
    /* is hintsize lower than minimal    */
    if (maxSize <= ARRAY_MIN_SIZE)
    {        /*  allocation array size?           */
        maxSize = ARRAY_MIN_SIZE;          /* yes, we will actually min size    */
    }
    // if the max is smaller than the size, just use the max size.
    if (maxSize < size)
    {
        maxSize = size;
    }
    /* compute size of new array obj     */
    bytes = newSize + (sizeof(RexxObject *) * (maxSize - 1));
    /* Create the new array              */
    newArray = (RexxArray *)new_object(bytes);
    /* Give it array behaviour.          */
    newArray->setBehaviour(arrayClass->getInstanceBehaviour());

    newArray->arraySize = size;
    newArray->maximumSize = maxSize;
    /* no expansion yet, use ourself     */
    newArray->expansionArray = newArray;
    /* moved _after_ setting hashvalue, otherwise the uninit table will not be*/
    /* able to find the new array object again later!                         */
    if (arrayClass->hasUninitDefined())
    {/* does object have an UNINT method  */
        ProtectedObject p(newArray);
        /* require new REXX objects          */
        newArray->hasUninit();            /* Make sure everyone is notified.   */
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
    if (TheArrayClass != (RexxClass *)this)
    {
        /* nope, better create properly      */
        ProtectedObject result;
        /* send new to actual class.         */
        this->sendMessage(OREF_NEW, new_integer(argCount), result);
        newArray = (RexxArray *)result;
        /* For each argument to of, send a   */
        /* put message                       */
        for (i = 0; i < argCount; i++)
        {
            item = args[i];                 /* get the item                      */
            if (item != OREF_NULL)          /* have a real item here?            */
            {
                /* place it in the target array      */
                newArray->sendMessage(OREF_PUT, item, new_integer(i+1));
            }
        }
        return newArray;
    }
    else
    {
        newArray = new (argCount, args) RexxArray;
        if (argCount == 0)
        {             /* creating 0 sized array?           */
                      /* Yup, setup a Dimension array      */
                      /* single Dimension, Mark so         */
                      /* can't change Dimensions.          */
            OrefSet(newArray, newArray->dimensions, new_array(IntegerZero));
        }
        /* argument array is exactly what    */
        /* we want, so just return it        */
        return newArray;
    }

}


wholenumber_t BaseSortComparator::compare(RexxObject *first, RexxObject *second)
{
    return first->compareTo(second);
}


wholenumber_t WithSortComparator::compare(RexxObject *first, RexxObject *second)
{
    ProtectedObject result;
    comparator->sendMessage(OREF_COMPARE, first, second, result);
    if ((RexxObject *)result == OREF_NULL)
    {
        reportException(Error_No_result_object_message, OREF_COMPARE);
    }

    wholenumber_t comparison;
    if (!((RexxObject *)result)->numberValue(comparison, Numerics::DEFAULT_DIGITS))
    {
        reportException(Error_Invalid_whole_number_compare, (RexxObject *)result);
    }
    return comparison;
}
