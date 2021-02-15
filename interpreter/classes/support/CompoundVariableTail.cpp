/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/* Support class for building a compound variable tail.                       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "CompoundVariableTail.hpp"
#include "BufferClass.hpp"
#include "Numerics.hpp"
#include "ExpressionVariable.hpp"
#include "GlobalNames.hpp"


/**
 * Build a tail value from a set of tail elements without doing
 * variable resolution.
 *
 * @param tails  An array of tail elements.
 * @param count  The count of tail elements.
 */
void CompoundVariableTail::buildUnresolvedTail(RexxInternalObject **tails, size_t count)
{
    bool first = true;

    for (size_t i = 0; i < count; i++)
    {
        // only add a dot after the first piece
        if (!first)
        {
            addDot();
        }

        first = false;
        RexxInternalObject *part = tails[i];
        // this could be ommitted in a [] list
        if (part != OREF_NULL)
        {
            // if this is a variable, just copy the name.  Otherwise, copy the value
            if (isOfClass(VariableTerm, part))
            {
                ((RexxSimpleVariable *)part)->getName()->copyIntoTail(this);
            }
            else
            {
                part->stringValue()->copyIntoTail(this);
            }
        }
    }
    length = current - tail;             // set the final, updated length
}


/**
 * Construct a tail piece from its elements, using a
 * variable dictionary as the context for resolving
 * variables.
 *
 * @param dictionary The source variable dictionary.
 * @param tails
 * @param tailCount
 */
void CompoundVariableTail::buildTail(VariableDictionary *dictionary, RexxInternalObject **tails, size_t tailCount)
{
    // a single element is easiest, and common enough that we optimize for it.
    if (tailCount == 1)
    {
        // get the tail value
        RexxInternalObject *_tail = tails[0]->getValue(dictionary);
        // if it is an integer type, we might be able to address the string representation directly.
        if (isInteger(_tail))
        {
            RexxString *rep = ((RexxInteger *)_tail)->getStringrep();
            if (rep != OREF_NULL)
            {
                useStringValue(rep);
                return;
            }
        }
        // if this is directly a string, we can use this directly
        if (isString(_tail))
        {
            useStringValue((RexxString *)_tail);
            return;
        }
        // some other type of object, or an integer without a string
        // rep.  We need to have it do the copy operation.
        _tail->copyIntoTail(this);
        length = current - tail;             // set the final, updated length
    }
    else
    {
        // copy the first element, then the remainder of the elements
        // with a dot in between.
        tails[0]->getValue(dictionary)->copyIntoTail(this);
        for (size_t i = 1; i < tailCount; i++)
        {
            addDot();
            tails[i]->getValue(dictionary)->copyIntoTail(this);
        }

        length = current - tail;             // set the final, updated length
    }
}


/**
 * Construct a tail piece from its elements, using a Rexx
 * context for resolving variables.
 *
 * @param context    The Rexx variable context
 * @param tails
 * @param tailCount
 */
void CompoundVariableTail::buildTail(RexxActivation *context, RexxInternalObject **tails, size_t tailCount)
{
    // a single element is easiest, and common enough that we optimize for it.
    if (tailCount == 1)
    {
        // get the tail value
        RexxInternalObject *_tail = tails[0]->getValue(context);
        // if it is an integer type, we might be able to address the string representation directly.
        if (isInteger(_tail))
        {
            RexxString *rep = ((RexxInteger *)_tail)->getStringrep();
            if (rep != OREF_NULL)
            {
                useStringValue(rep);
                return;
            }
        }
        // if this is directly a string, we can use this directly
        if (isString(_tail))
        {
            useStringValue((RexxString *)_tail);
            return;
        }
        // some other type of object, or an integer without a string
        // rep.  We need to have it do the copy operation.
        _tail->copyIntoTail(this);
        length = current - tail;             // set the final, updated length
    }
    else
    {
        // copy the first element, then the remainder of the elements
        // with a dot in between.
        tails[0]->getValue(context)->copyIntoTail(this);
        for (size_t i = 1; i < tailCount; i++)
        {
            addDot();
            tails[i]->getValue(context)->copyIntoTail(this);
        }

        length = current - tail;             // set the final, updated length
    }
}


/**
 * Resolve the "stem.[a,b,c]=" pieces to a fully qualified stem
 * name.
 *
 * @param tails  The array of tails.
 * @param count  The count of tails.
 */
void CompoundVariableTail::buildTail(RexxInternalObject **tails, size_t count)
{
    bool first = true;
    // copy in each piece, with dots added after the first.

    for (size_t i = 0; i < count; i++)
    {
        if (!first)
        {
            addDot();
        }
        first = false;

        RexxInternalObject *part = (RexxInternalObject *)tails[i];
        // omitted pices are null strings
        if (part == OREF_NULL)
        {
            part = GlobalNames::NULLSTRING;
        }
        part->copyIntoTail(this);
    }
    length = current - tail;
}


/**
 * Construct a tail from a single string index
 *
 * @param _tail  The single tail value.
 */
void CompoundVariableTail::buildTail(RexxString *_tail)
{
    tail = _tail->getWritableData();
    length = _tail->getLength();
    remainder = 0;
    value = _tail;
}


/**
 * Build a tail directly from a single character string value.
 *
 * @param t      The pointer to the tail name (as an asciiz string);
 */
void CompoundVariableTail::buildTail(const char *t)
{
    tail = const_cast<char *>(t);
    length = strlen(t);
    remainder = 0;
}


/**
 * Build a tail from a single string and a number index
 * value.  This is used by the stem sort, and the if the tail
 * piece does not end with a dot, one is provided.
 *
 * @param _tail  the tail prefix.
 * @param index  the index value.
 */
void CompoundVariableTail::buildTail(RexxString *_tail, size_t index)
{
    // the tail prefix can be optional,
    if (_tail != OREF_NULL)
    {
        _tail->copyIntoTail(this);
        length = length + _tail->getLength();
        // if we don't have a dot on the end of this tail piece, add one
        if (!_tail->endsWith('.'))
        {
            addDot();
        }
        length = current - tail;
    }

    size_t numberLength = Numerics::formatWholeNumber(index, current);
    current += numberLength;
    length += numberLength;
    remainder -= length;
}


/**
 * Build a tail from a single numeric value.
 *
 * @param index  The numeric index.
 */
void CompoundVariableTail::buildTail(size_t index)
{
    length = Numerics::formatWholeNumber(index, current);
    length = strlen((char *)current);
    current += length;
    remainder -= length;
}


/**
 * Ensure we have sufficient space for adding another
 * tail element to our buffer.
 *
 * @param needed The size of the required element.
 */
void CompoundVariableTail::expandCapacity(size_t needed)
{
    // update the accumulated length
    length = current - tail;
    // if we have previously allocated a buffer, expand the size (this must be
    // a BIG tail!)
    if (!temp.isNull())
    {
        temp->expand(needed + ALLOCATION_PAD);
        tail = temp->getData();
        current = tail + length;
        remainder += needed + ALLOCATION_PAD;
    }
    // allocate a completly new buffer, copying our
    // existing tail data into it.
    else
    {

        size_t newLength = length + needed + ALLOCATION_PAD;
        temp = (BufferClass *)new_buffer(newLength);
        tail = temp->getData();
        current = tail + length;
        memcpy(tail, buffer, length);
        remainder = newLength - length;
    }
}


/**
 * Create a fully resolved compound name from this tail.
 * This appends the stem name to the constructed tail.
 *
 * @param stem   The stem name.
 *
 * @return The fully resolved compound variable name as a string object.
 */
RexxString *CompoundVariableTail::createCompoundName(RexxString *stem)
{
    size_t len1 = stem->getLength();
    // create a new string of the required length (note, this assumes the
    // first period of the compound name is in the stem name.
    RexxString *result = (RexxString *)raw_string(len1 + length);
    // copy in the stem name and the tail piece.
    char *data = result->getWritableData();
    if (len1 != 0)
    {
        memcpy(data, stem->getStringData(), len1);
        data += len1;
    }
    if (length != 0)
    {
        memcpy(data, tail, length);
    }
    return result;
}


/**
 * Get the resolved tail value as a string object.
 *
 * @return A string object representation of the tail name.
 */
RexxString *CompoundVariableTail::makeString()
{
    if (value == NULL)
    {
        value = (RexxString *)new_string(tail, length);
    }
    return value;
}
