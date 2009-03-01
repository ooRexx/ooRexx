/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive MutableBuffer Class                                              */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "RexxCore.h"
#include "StringClass.hpp"
#include "MutableBufferClass.hpp"
#include "ProtectedObject.hpp"
#include "StringUtil.hpp"


// singleton class instance
RexxClass *RexxMutableBuffer::classInstance = OREF_NULL;



/**
 * Create initial class object at bootstrap time.
 */
void RexxMutableBuffer::createInstance()
{
    CLASS_CREATE(MutableBuffer, "MutableBuffer", RexxClass);
}


#define DEFAULT_BUFFER_LENGTH 256

RexxMutableBuffer *RexxMutableBufferClass::newRexx(RexxObject **args, size_t argc)
/******************************************************************************/
/* Function:  Allocate (and initialize) a string object                       */
/******************************************************************************/
{
    RexxString        *string;
    RexxMutableBuffer *newBuffer;         /* new mutable buffer object         */
    size_t            bufferLength = DEFAULT_BUFFER_LENGTH;
    size_t            defaultSize;
    if (argc >= 1)
    {
        if (args[0] != NULL)
        {
            /* force argument to string value    */
            string = stringArgument(args[0], ARG_ONE);
        }
        else
        {
            string = OREF_NULLSTRING;           /* default to empty content          */
        }
    }
    else                                      /* minimum buffer size given?        */
    {
        string = OREF_NULLSTRING;
    }

    if (argc >= 2)
    {
        bufferLength = optionalLengthArgument(args[1], DEFAULT_BUFFER_LENGTH, ARG_TWO);
    }

    defaultSize = bufferLength;           /* remember initial default size     */

                                          /* input string longer than demanded */
                                          /* minimum size? expand accordingly  */
    if (string->getLength() > bufferLength)
    {
        bufferLength = string->getLength();
    }
    /* allocate the new object           */
    newBuffer = new ((RexxClass *)this) RexxMutableBuffer(bufferLength, defaultSize);
    newBuffer->dataLength = string->getLength();
    /* copy the content                  */
    newBuffer->data->copyData(0, string->getStringData(), string->getLength());

    ProtectedObject p(newBuffer);
    newBuffer->sendMessage(OREF_INIT, args, argc > 2 ? argc - 2 : 0);
    return newBuffer;
}


/**
 * Default constructor.
 */
RexxMutableBuffer::RexxMutableBuffer()
{
    bufferLength = DEFAULT_BUFFER_LENGTH;   /* save the length of the buffer    */
    defaultSize  = bufferLength;            /* store the default buffer size    */
    // NB:  we clear this before we allocate the new buffer because allocating the
    // new buffer might trigger a garbage collection, causing us to mark bogus
    // reference.
    data = OREF_NULL;
    data = new_buffer(bufferLength);

}


/**
 * Constructor with explicitly set size and default.
 *
 * @param l      Initial length.
 * @param d      The explicit default size.
 */
RexxMutableBuffer::RexxMutableBuffer(size_t l, size_t d)
{
    bufferLength = l;               /* save the length of the buffer    */
    defaultSize  = d;               /* store the default buffer size    */
    // NB: As in the default constructor, we clear this before we allocate the
    // new buffer in case garbage collection is triggered.
    data = OREF_NULL;
    data = new_buffer(bufferLength);
}


/**
 * Create a new mutable buffer object from a potential subclass.
 *
 * @param size   The size of the buffer object.
 *
 * @return A new instance of a mutable buffer, with the default class
 *         behaviour.
 */
void *RexxMutableBuffer::operator new(size_t size)
{
    return new_object(size, T_MutableBuffer);
}

/**
 * Create a new mutable buffer object from a potential subclass.
 *
 * @param size   The size of the buffer object.
 * @param bufferClass
 *               The class of the buffer object.
 *
 * @return A new instance of a mutable buffer, with the target class
 *         behaviour.
 */
void *RexxMutableBuffer::operator new(size_t size, RexxClass *bufferClass)
{
    RexxObject * newObj = new_object(size, T_MutableBuffer);
    newObj->setBehaviour(bufferClass->getInstanceBehaviour());
    return newObj;
}


void RexxMutableBuffer::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->objectVariables);
    memory_mark(this->data);
}

void RexxMutableBuffer::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->objectVariables);
    memory_mark_general(this->data);
}


void RexxMutableBuffer::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten a mutable buffer                                        */
/******************************************************************************/
{
  setUpFlatten(RexxMutableBuffer)

  flatten_reference(newThis->data, envelope);
  flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}

RexxObject *RexxMutableBuffer::copy()
/******************************************************************************/
/* Function:  copy an object                                                  */
/******************************************************************************/
{

    RexxMutableBuffer *newObj = (RexxMutableBuffer *)this->clone();

                                           /* see the comments in ::newRexx()!! */
    newObj->data = new_buffer(bufferLength);
    newObj->dataLength = this->dataLength;
    newObj->data->copyData(0, data->getData(), bufferLength);

    newObj->defaultSize = this->defaultSize;
    newObj->bufferLength = this->bufferLength;
    return newObj;
}

void RexxMutableBuffer::ensureCapacity(size_t addedLength)
/******************************************************************************/
/* Function:  append to the mutable buffer                                    */
/******************************************************************************/
{
    size_t resultLength = this->dataLength + addedLength;

    if (resultLength > bufferLength)
    {   /* need to enlarge?                  */
        bufferLength *= 2;                   /* double the buffer                 */
        if (bufferLength < resultLength)
        {   /* still too small? use new length   */
            bufferLength = resultLength;
        }

        RexxBuffer *newBuffer = new_buffer(bufferLength);
        // copy the data into the new buffer
        newBuffer->copyData(0, data->getData(), dataLength);
        // replace the old data buffer
        OrefSet(this, this->data, newBuffer);
    }
}


/**
 * Return the length of the data in the buffer currently.
 *
 * @return The current length, as an Integer object.
 */
RexxObject *RexxMutableBuffer::lengthRexx()
{
    return new_integer(dataLength);
}


RexxMutableBuffer *RexxMutableBuffer::append(RexxObject *obj)
/******************************************************************************/
/* Function:  append to the mutable buffer                                    */
/******************************************************************************/
{
    RexxString *string = stringArgument(obj, ARG_ONE);
    ProtectedObject p(string);
    // make sure we have enough room
    ensureCapacity(string->getLength());

    data->copyData(dataLength, string->getStringData(), string->getLength());
    this->dataLength += string->getLength();
    return this;
}


RexxMutableBuffer *RexxMutableBuffer::insert(RexxObject *str, RexxObject *pos, RexxObject *len, RexxObject *pad)
/******************************************************************************/
/* Function:  insert string at given position                                 */
/******************************************************************************/
{
    // force this into string form
    RexxString * string = stringArgument(str, ARG_ONE);

    // we're using optional length because 0 is valid for insert.
    size_t begin = optionalNonNegative(pos, 0, ARG_TWO);
    size_t insertLength = optionalLengthArgument(len, string->getLength(), ARG_THREE);

    char padChar = optionalPadArgument(pad, ' ', ARG_FOUR);

    size_t copyLength = Numerics::minVal(insertLength, string->getLength());
    size_t padLength = insertLength - copyLength;


    // if inserting within the current bounds, we only need to add the length
    // if inserting beyond the end, we need to make sure we add space for the gap too
    if (begin < dataLength)
    {
        // if inserting a zero length string, this is simple!
        if (insertLength == 0)
        {
            return this;                            /* do nothing                   */
        }
        ensureCapacity(insertLength);
    }
    else
    {
        ensureCapacity(insertLength + (begin - dataLength));
    }


    /* create space in the buffer   */
    if (begin < dataLength)
    {
        data->openGap(begin, insertLength, dataLength - begin);
    }
    else if (begin > this->dataLength)
    {
        /* pad before insertion         */
        data->setData(dataLength, padChar, begin - dataLength);
    }
    /* insert string contents       */
    data->copyData(begin, string->getStringData(), copyLength);
    // do we need data padding?
    if (padLength > 0)
    {
        data->setData(begin + string->getLength(), padChar, padLength);
    }
    // inserting after the end? the resulting length is measured from the insertion point
    if (begin > this->dataLength)
    {
        this->dataLength = begin + insertLength;
    }
    else
    {
        // just add in the inserted length
        this->dataLength += insertLength;
    }
    return this;
}


RexxMutableBuffer *RexxMutableBuffer::overlay(RexxObject *str, RexxObject *pos, RexxObject *len, RexxObject *pad)
/******************************************************************************/
/* Function:  replace characters in buffer contents                           */
/******************************************************************************/
{
    RexxString *string = stringArgument(str, ARG_ONE);
    size_t begin = optionalPositionArgument(pos, 1, ARG_TWO) - 1;
    size_t replaceLength = optionalLengthArgument(len, string->getLength(), ARG_THREE);

    char padChar = optionalPadArgument(pad, ' ', ARG_FOUR);

    // make sure we have room for this
    ensureCapacity(begin + replaceLength);

    // is our start position beyond the current data end?
    if (begin > dataLength)
    {
        // add padding to the gap
        data->setData(dataLength, padChar, begin - dataLength);
    }

    // now overlay the string data
    data->copyData(begin, string->getStringData(), Numerics::minVal(replaceLength, string->getLength()));
    // do we need additional padding?
    if (replaceLength > string->getLength())
    {
        // pad the section after the overlay
        data->setData(begin + string->getLength(), padChar, replaceLength - string->getLength());
    }

    // did this add to the size?
    if (begin + replaceLength > dataLength)
    {
        //adjust upward
        dataLength = begin + replaceLength;
    }
    return this;
}


/**
 * Replace a target substring within a string with
 * a new string value.  This is similar overlay, but
 * replacing might cause the characters following the
 * replacement position to be shifted to the left or
 * right.
 *
 * @param str    The replacement string.
 * @param pos    The target position (required).
 * @param len    The target length (optional).  If not specified, the
 *               length of the replacement string is used, and this
 *               is essentially an overlay operation.
 * @param pad    A padding character if padding is required.  The default
 *               pad is a ' '.  Padding only occurs if the replacement
 *               position is beyond the current data length.
 *
 * @return The target mutablebuffer object.
 */
RexxMutableBuffer *RexxMutableBuffer::replaceAt(RexxObject *str, RexxObject *pos, RexxObject *len, RexxObject *pad)
{
    RexxString *string = stringArgument(str, ARG_ONE);
    size_t begin = positionArgument(pos, ARG_TWO) - 1;
    size_t newLength = string->getLength();
    size_t replaceLength = optionalLengthArgument(len, newLength, ARG_THREE);

    char padChar = optionalPadArgument(pad, ' ', ARG_FOUR);
    size_t finalLength;

    // will this extend beyond the end of the string, we require
    // space for the position + the replacement string length
    if (begin + newLength > dataLength)
    {
        finalLength = begin + newLength;
    }
    else
    {
        // we need to add the delta between the excised string and the inserted
        // replacement string
        finalLength = dataLength - replaceLength + newLength;
    }

    // make sure we have room for this
    ensureCapacity(finalLength);

    // is our start position beyond the current data end?
    // NB: Even though we've adjusted the buffer size, the dataLength is still
    // the original entry length.
    if (begin > dataLength)
    {
        // add padding to the gap
        data->setData(dataLength, padChar, begin - dataLength);
        // now overlay the string data
        data->copyData(begin, string->getStringData(), newLength);
    }
    else
    {
        // if the strings are of different lengths, we need to adjust the size
        // of the gap we're copying into
        if (replaceLength != newLength)
        {
            // snip out the original string
            data->adjustGap(begin, replaceLength, newLength);
        }
        // now overlay the string data
        data->copyData(begin, string->getStringData(), newLength);
    }

    // and finally adjust the length
    dataLength = finalLength;
    // our return value is always the target mutable buffer
    return this;
}


RexxMutableBuffer *RexxMutableBuffer::mydelete(RexxObject *_start, RexxObject *len)
/******************************************************************************/
/* Function:  delete character range in buffer                                */
/******************************************************************************/
{
    size_t begin = positionArgument(_start, ARG_ONE) - 1;
    size_t range = optionalLengthArgument(len, this->data->getDataLength() - begin, ARG_TWO);

    // is the begin point actually within the string?
    if (begin < dataLength)
    {           /* got some work to do?         */
        // deleting from the middle?
        if (begin + range < dataLength)
        {
            // shift everything over
            data->closeGap(begin, range, dataLength - (begin + range));
            dataLength -= range;
        }
        else
        {
            // we're just truncating
            dataLength = begin;
        }
    }
    return this;
}


RexxObject *RexxMutableBuffer::setBufferSize(RexxInteger *size)
/******************************************************************************/
/* Function:  set the size of the buffer                                      */
/******************************************************************************/
{
    size_t newsize = lengthArgument(size, ARG_ONE);
    // has a reset to zero been requested?
    if (newsize == 0)
    {
        // have we increased the buffer size?
        if (bufferLength > defaultSize)
        {
            // reallocate the buffer
            OrefSet(this, this->data, new_buffer(defaultSize));
            // reset the size to the default
            bufferLength = defaultSize;
        }
        dataLength = 0;
    }
    // an actual resize?
    else if (newsize != bufferLength)
    {
        // reallocate the buffer
        RexxBuffer *newBuffer = new_buffer(newsize);
        // if we're shrinking this, it truncates.
        dataLength = Numerics::minVal(dataLength, newsize);
        newBuffer->copyData(0, data->getData(), dataLength);
        // replace the old buffer
        OrefSet(this, this->data, newBuffer);
        // and update the size....
        bufferLength = newsize;
    }
    return this;
}


RexxString *RexxMutableBuffer::makeString()
/******************************************************************************/
/* Function:  Handle a REQUEST('STRING') request for a mutablebuffer object   */
/******************************************************************************/
{
    return new_string(data->getData(), dataLength);
}


/******************************************************************************/
/* Arguments:  String position for substr                                     */
/*             requested length of new string                                 */
/*             pad character to use, if necessary                             */
/*                                                                            */
/*  Returned:  string, sub string of original.                                */
/******************************************************************************/
RexxString *RexxMutableBuffer::substr(RexxInteger *argposition,
                                      RexxInteger *arglength,
                                      RexxString  *pad)
{
    return StringUtil::substr(getStringData(), getLength(), argposition, arglength, pad);
}


/**
 * Perform a search for a string within the buffer.
 *
 * @param needle The search needle.
 * @param pstart the starting position.
 *
 * @return The index of the located string.  Returns 0 if no matches
 *         are found.
 */
RexxInteger *RexxMutableBuffer::posRexx(RexxString  *needle, RexxInteger *pstart, RexxInteger *range)
{
    return StringUtil::posRexx(getStringData(), getLength(), needle, pstart, range);
}


/**
 * Perform a search for the last position of a string within the
 * buffer.
 *
 * @param needle The search needle.
 * @param pstart the starting position.
 *
 * @return The index of the located string.  Returns 0 if no matches
 *         are found.
 */
RexxInteger *RexxMutableBuffer::lastPos(RexxString  *needle, RexxInteger *_start, RexxInteger *_range)
{
    return StringUtil::lastPosRexx(getStringData(), getLength(), needle, _start, _range);
}


/**
 * Perform a caseless search for a string within the buffer.
 *
 * @param needle The search needle.
 * @param pstart the starting position.
 *
 * @return The index of the located string.  Returns 0 if no matches
 *         are found.
 */
RexxInteger *RexxMutableBuffer::caselessPos(RexxString  *needle, RexxInteger *pstart, RexxInteger *range)
{
    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    /* get the starting position         */
    size_t _start = optionalPositionArgument(pstart, 1, ARG_TWO);
    size_t _range = optionalLengthArgument(range, getLength() - _start + 1, ARG_THREE);
    /* pass on to the primitive function */
    /* and return as an integer object   */
    return new_integer(StringUtil::caselessPos(getStringData(), getLength(), needle , _start - 1, _range));
}


/**
 * Perform a caseless search for the last position of a string
 * within the buffer.
 *
 * @param needle The search needle.
 * @param pstart the starting position.
 *
 * @return The index of the located string.  Returns 0 if no matches
 *         are found.
 */
RexxInteger *RexxMutableBuffer::caselessLastPos(RexxString  *needle, RexxInteger *pstart, RexxInteger *range)
{
    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    /* get the starting position         */
    size_t _start = optionalPositionArgument(pstart, getLength(), ARG_TWO);
    size_t _range = optionalLengthArgument(range, getLength(), ARG_THREE);
    /* pass on to the primitive function */
    /* and return as an integer object   */
    return new_integer(StringUtil::caselessLastPos(getStringData(), getLength(), needle , _start, _range));
}


/**
 * Extract a single character from a string object.
 * Returns a null string if the specified position is
 * beyond the bounds of the string.
 *
 * @param positionArg
 *               The position of the target  character.  Must be a positive
 *               whole number.
 *
 * @return Returns the single character at the target position.
 *         Returns a null string if the position is beyond the end
 *         of the string.
 */
RexxString *RexxMutableBuffer::subchar(RexxInteger *positionArg)
{
    return StringUtil::subchar(getStringData(), getLength(), positionArg);
}


RexxArray *RexxMutableBuffer::makearray(RexxString *div)
/******************************************************************************/
/* Function:  Split string into an array                                      */
/******************************************************************************/
{
    return StringUtil::makearray(getStringData(), getLength(), div);
}


RexxInteger *RexxMutableBuffer::countStrRexx(RexxString *needle)
/******************************************************************************/
/* Function:  Count occurrences of one string in another.                     */
/******************************************************************************/
{
    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    // delegate the counting to the string util
    return new_integer(StringUtil::countStr(getStringData(), getLength(), needle));
}

RexxInteger *RexxMutableBuffer::caselessCountStrRexx(RexxString *needle)
/******************************************************************************/
/* Function:  Count occurrences of one string in another.                     */
/******************************************************************************/
{
    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    // delegate the counting to the string util
    return new_integer(StringUtil::caselessCountStr(getStringData(), getLength(), needle));
}

/**
 * Do an inplace changeStr operation on a mutablebuffer.
 *
 * @param needle    The search needle.
 * @param newNeedle The replacement string.
 * @param countArg  The number of occurrences to replace.
 *
 * @return The target MutableBuffer
 */
RexxMutableBuffer *RexxMutableBuffer::changeStr(RexxString *needle, RexxString *newNeedle, RexxInteger *countArg)
{
    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    /* newneedle must be a string two    */
    newNeedle = stringArgument(newNeedle, ARG_TWO);

    // we'll only change up to a specified count.  If not there, we do everything.
    size_t count = optionalPositive(countArg, Numerics::MAX_WHOLENUMBER, ARG_THREE);
    // find the number of matches in the string
    size_t matches = StringUtil::countStr(getStringData(), getLength(), needle);
    if (matches > count)                 // the matches are bounded by the count
    {
        matches = count;
    }
    // no matches is easy!
    if (matches == 0)
    {
        return this;
    }
    size_t needleLength = needle->getLength();  /* get the length of the needle      */
    size_t newLength = newNeedle->getLength();  /* and the replacement length        */
    // calculate the final length and make sure we have enough space
    size_t resultLength = this->getLength() - (matches * needleLength) + (matches * newLength);
    ensureCapacity(resultLength);

    // an inplace update has complications, depending on whether the new string is shorter,
    // the same length, or longer

    // simplest case...same length strings.  We can just overlay the existing occurrences
    if (needleLength == newLength)
    {
        const char *source = getStringData();
        size_t sourceLength = getLength();
        size_t _start = 0;                          /* set a zero starting point         */
        for (size_t i = 0; i < matches; i++)
        {
            // search for the next occurrence...which should be there because we
            // already know the count
            size_t matchPos = StringUtil::pos(source, sourceLength, needle, _start, sourceLength);
            copyData(matchPos - 1, newNeedle->getStringData(), newLength);
            // step to the next search position
            _start = matchPos + newLength - 1;
        }
    }
    // this will be a shorter thing, so we can do things in place as if we were using two buffers
    else if (needleLength > newLength)
    {
        // we start building from the beginning
        size_t copyOffset = 0;
        size_t _start = 0;
        // get our string bounds
        const char *source = getStringData();
        size_t sourceLength = getLength();
        const char *newPtr = newNeedle->getStringData();
        // this is our scan offset
        for (size_t i = 0; i < matches; i++)
        {
            // look for each instance and replace
            size_t matchPos = StringUtil::pos(source, sourceLength, needle, _start, sourceLength);
            size_t copyLength = (matchPos - 1) - _start;  /* get the next length to copy       */
            // if this skipped over characters, we need to copy those
            if (copyLength != 0)
            {
                copyData(copyOffset, source + _start, copyLength);
                copyOffset += copyLength;
            }
            // replacing with a non-null string, copy the replacement string in
            if (newLength != 0)
            {
                copyData(copyOffset, newPtr, newLength);
                copyOffset += newLength;
            }
            _start = matchPos + needleLength - 1;  /* step to the next position         */
        }
        // we likely have some remainder that needs copying
        if (_start < sourceLength)
        {
            copyData(copyOffset, source + _start, sourceLength - _start);
        }
    }
    // hardest case...the string gets longer.  We need to shift all of the data
    // to the end and then pull the pieces back in as we go
    else
    {
        size_t growth = (newLength - needleLength) * matches;

        // we start building from the beginning
        size_t copyOffset = 0;
        size_t _start = 0;
        // get our string bounds
        const char *source = getStringData() + growth;
        size_t sourceLength = getLength();
        // this shifts everything to the end of the buffer.  From there,
        // we pull pieces back into place.
        openGap(0, growth, sourceLength);
        const char *newPtr = newNeedle->getStringData();
        // this is our scan offset
        for (size_t i = 0; i < matches; i++)
        {
            // look for each instance and replace
            size_t matchPos = StringUtil::pos(source, sourceLength, needle, _start, sourceLength);
            size_t copyLength = (matchPos - 1) - _start;  /* get the next length to copy       */
            // if this skipped over characters, we need to copy those
            if (copyLength != 0)
            {
                copyData(copyOffset, source + _start, copyLength);
                copyOffset += copyLength;
            }
            // replacing with a non-null string, copy the replacement string in
            if (newLength != 0)
            {
                copyData(copyOffset, newPtr, newLength);
                copyOffset += newLength;
            }
            _start = matchPos + needleLength - 1;  /* step to the next position         */
        }
        // we likely have some remainder that needs copying
        if (_start < sourceLength)
        {
            copyData(copyOffset, source + _start, sourceLength - _start);
        }
    }
    // update the result length, and return
    dataLength = resultLength;
    return this;
}

/**
 * Do an inplace caseless changeStr operation on a
 * mutablebuffer.
 *
 * @param needle    The search needle.
 * @param newNeedle The replacement string.
 * @param countArg  The number of occurrences to replace.
 *
 * @return The target MutableBuffer
 */
RexxMutableBuffer *RexxMutableBuffer::caselessChangeStr(RexxString *needle, RexxString *newNeedle, RexxInteger *countArg)
{
    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    /* newneedle must be a string two    */
    newNeedle = stringArgument(newNeedle, ARG_TWO);

    // we'll only change up to a specified count.  If not there, we do everything.
    size_t count = optionalPositive(countArg, Numerics::MAX_WHOLENUMBER, ARG_THREE);
    // find the number of matches in the string
    size_t matches = StringUtil::caselessCountStr(getStringData(), getLength(), needle);
    if (matches > count)                 // the matches are bounded by the count
    {
        matches = count;
    }
    // no matches is easy!
    if (matches == 0)
    {
        return this;
    }
    size_t needleLength = needle->getLength();  /* get the length of the needle      */
    size_t newLength = newNeedle->getLength();  /* and the replacement length        */
    // calculate the final length and make sure we have enough space
    size_t resultLength = this->getLength() - (matches * needleLength) + (matches * newLength);
    ensureCapacity(resultLength);

    // an inplace update has complications, depending on whether the new string is shorter,
    // the same length, or longer

    // simplest case...same length strings.  We can just overlay the existing occurrences
    if (needleLength == newLength)
    {
        const char *source = getStringData();
        size_t sourceLength = getLength();
        size_t _start = 0;                          /* set a zero starting point         */
        for (size_t i = 0; i < matches; i++)
        {
            // search for the next occurrence...which should be there because we
            // already know the count
            size_t matchPos = StringUtil::caselessPos(source, sourceLength, needle, _start, sourceLength);
            copyData(matchPos - 1, newNeedle->getStringData(), newLength);
            // step to the next search position
            _start = matchPos + newLength - 1;
        }
    }
    // this will be a shorter thing, so we can do things in place as if we were using two buffers
    else if (needleLength > newLength)
    {
        // we start building from the beginning
        size_t copyOffset = 0;
        size_t _start = 0;
        // get our string bounds
        const char *source = getStringData();
        size_t sourceLength = getLength();
        const char *newPtr = newNeedle->getStringData();
        // this is our scan offset
        for (size_t i = 0; i < matches; i++)
        {
            // look for each instance and replace
            size_t matchPos = StringUtil::caselessPos(source, sourceLength, needle, _start, sourceLength);
            size_t copyLength = (matchPos - 1) - _start;  /* get the next length to copy       */
            // if this skipped over characters, we need to copy those
            if (copyLength != 0)
            {
                copyData(copyOffset, source + _start, copyLength);
                copyOffset += copyLength;
            }
            // replacing with a non-null string, copy the replacement string in
            if (newLength != 0)
            {
                copyData(copyOffset, newPtr, newLength);
                copyOffset += newLength;
            }
            _start = matchPos + needleLength - 1;  /* step to the next position         */
        }
        // we likely have some remainder that needs copying
        if (_start < sourceLength)
        {
            copyData(copyOffset, source + _start, sourceLength - _start);
        }
    }
    // hardest case...the string gets longer.  We need to shift all of the data
    // to the end and then pull the pieces back in as we go
    else
    {
        size_t growth = (newLength - needleLength) * matches;

        // we start building from the beginning
        size_t copyOffset = 0;
        size_t _start = 0;
        // get our string bounds
        const char *source = getStringData() + growth;
        size_t sourceLength = getLength();
        // this shifts everything to the end of the buffer.  From there,
        // we pull pieces back into place.
        openGap(0, growth, sourceLength);
        const char *newPtr = newNeedle->getStringData();
        // this is our scan offset
        for (size_t i = 0; i < matches; i++)
        {
            // look for each instance and replace
            size_t matchPos = StringUtil::caselessPos(source, sourceLength, needle, _start, sourceLength);
            size_t copyLength = (matchPos - 1) - _start;  /* get the next length to copy       */
            // if this skipped over characters, we need to copy those
            if (copyLength != 0)
            {
                copyData(copyOffset, source + _start, copyLength);
                copyOffset += copyLength;
            }
            // replacing with a non-null string, copy the replacement string in
            if (newLength != 0)
            {
                copyData(copyOffset, newPtr, newLength);
                copyOffset += newLength;
            }
            _start = matchPos + needleLength - 1;  /* step to the next position         */
        }
        // we likely have some remainder that needs copying
        if (_start < sourceLength)
        {
            copyData(copyOffset, source + _start, sourceLength - _start);
        }
    }
    // update the result length, and return
    dataLength = resultLength;
    return this;
}


/**
 * Rexx exported method stub for the lower() method.
 *
 * @param start  The optional starting location.  Defaults to the first character
 *               if not specified.
 * @param length The length to convert.  Defaults to the segment from the start
 *               position to the end of the string.
 *
 * @return A new string object with the case conversion applied.
 */
RexxMutableBuffer *RexxMutableBuffer::lower(RexxInteger *_start, RexxInteger *_length)
{
    size_t startPos = optionalPositionArgument(_start, 1, ARG_ONE) - 1;
    size_t rangeLength = optionalLengthArgument(_length, getLength(), ARG_TWO);

    // if we're starting beyond the end bounds, return unchanged
    if (startPos >= getLength())
    {
        return this;
    }

    rangeLength = Numerics::minVal(rangeLength, getLength() - startPos);

    // a zero length value is also a non-change.
    if (rangeLength == 0)
    {
        return this;
    }

    char *bufferData = getData() + startPos;
    // now uppercase in place
    for (size_t i = 0; i < rangeLength; i++)
    {
        *bufferData = tolower(*bufferData);
        bufferData++;
    }
    return this;
}


/**
 * Rexx exported method stub for the upper() method.
 *
 * @param start  The optional starting location.  Defaults to the first character
 *               if not specified.
 * @param length The length to convert.  Defaults to the segment from the start
 *               position to the end of the string.
 *
 * @return A new string object with the case conversion applied.
 */
RexxMutableBuffer *RexxMutableBuffer::upper(RexxInteger *_start, RexxInteger *_length)
{
    size_t startPos = optionalPositionArgument(_start, 1, ARG_ONE) - 1;
    size_t rangeLength = optionalLengthArgument(_length, getLength(), ARG_TWO);

    // if we're starting beyond the end bounds, return unchanged
    if (startPos >= getLength())
    {
        return this;
    }

    rangeLength = Numerics::minVal(rangeLength, getLength() - startPos);

    // a zero length value is also a non-change.
    if (rangeLength == 0)
    {
        return this;
    }

    char *bufferData = getData() + startPos;
    // now uppercase in place
    for (size_t i = 0; i < rangeLength; i++)
    {
        *bufferData = toupper(*bufferData);
        bufferData++;
    }
    return this;
}


/**
 * translate characters in the buffer using a translation table.
 *
 * @param tableo The output table specification
 * @param tablei The input table specification
 * @param pad    An optional padding character (default is a space).
 * @param _start The starting position to translate.
 * @param _range The length to translate
 *
 * @return The target mutable buffer.
 */
RexxMutableBuffer *RexxMutableBuffer::translate(RexxString *tableo, RexxString *tablei, RexxString *pad, RexxInteger *_start, RexxInteger *_range)
{
    // just a simple uppercase?
    if (tableo == OREF_NULL && tablei == OREF_NULL && pad == OREF_NULL)
    {
        return this->upper(_start, _range);
    }
                                            /* validate the tables               */
    tableo = optionalStringArgument(tableo, OREF_NULLSTRING, ARG_ONE);
    size_t outTableLength = tableo->getLength();      /* get the table length              */
    /* input table too                   */
    tablei = optionalStringArgument(tablei, OREF_NULLSTRING, ARG_TWO);
    size_t inTableLength = tablei->getLength();       /* get the table length              */
    const char *inTable = tablei->getStringData();    /* point at the input table          */
    const char *outTable = tableo->getStringData();   /* and the output table              */
                                          /* get the pad character             */
    char padChar = optionalPadArgument(pad, ' ', ARG_THREE);
    size_t startPos = optionalPositionArgument(_start, 1, ARG_FOUR);
    size_t range = optionalLengthArgument(_range, getLength() - startPos + 1, ARG_FOUR);

    // if nothing to translate, we can return now
    if (startPos > getLength() || range == 0)
    {
        return this;
    }
    // cape the real range
    range = Numerics::minVal(range, getLength() - startPos + 1);
    char *scanPtr = getData() + startPos - 1;   /* point to data                     */
    size_t scanLength = range;                  /* get the length too                */

    while (scanLength--)
    {                /* spin thru input                   */
        char ch = *scanPtr;                      /* get a character                   */
        size_t position;

        if (tablei != OREF_NULLSTRING)      /* input table specified?            */
        {
            /* search for the character          */
            position = StringUtil::memPos(inTable, inTableLength, ch);
        }
        else
        {
            position = (size_t)ch;            /* position is the character value   */
        }
        if (position != (size_t)(-1))
        {     /* found in the table?               */
            if (position < outTableLength)    /* in the output table?              */
            {
                /* convert the character             */
                *scanPtr = *(outTable + position);
            }
            else
            {
                *scanPtr = padChar;             /* else use the pad character        */
            }
        }
        scanPtr++;                          /* step the pointer                  */
    }
    return this;
}


/**
 * Test if regions within two strings match.
 *
 * @param start_  The starting compare position within the target string.  This
 *                must be within the bounds of the string.
 * @param other   The other compare string.
 * @param offset_ The starting offset of the compare string.  This must be
 *                within the string bounds.  The default start postion is 1.
 * @param len_    The length of the compare substring.  The length and the
 *                offset must specify a valid substring of other.  If not
 *                specified, this defaults to the substring from the
 *                offset to the end of the string.
 *
 * @return True if the two regions match, false for any mismatch.
 */
RexxInteger *RexxMutableBuffer::match(RexxInteger *start_, RexxString *other, RexxInteger *offset_, RexxInteger *len_)
{
    stringsize_t _start = positionArgument(start_, ARG_ONE);
    // the start position must be within the string bounds
    if (_start > getLength())
    {
        reportException(Error_Incorrect_method_position, start_);
    }
    other = stringArgument(other, ARG_TWO);

    stringsize_t offset = optionalPositionArgument(offset_, 1, ARG_THREE);

    if (offset > other->getLength())
    {
        reportException(Error_Incorrect_method_position, offset);
    }

    stringsize_t len = optionalLengthArgument(len_, other->getLength() - offset + 1, ARG_FOUR);

    if ((offset + len - 1) > other->getLength())
    {
        reportException(Error_Incorrect_method_length, len);
    }

    return primitiveMatch(_start, other, offset, len) ? TheTrueObject : TheFalseObject;
}


/**
 * Test if regions within two strings match.
 *
 * @param start_  The starting compare position within the target string.  This
 *                must be within the bounds of the string.
 * @param other   The other compare string.
 * @param offset_ The starting offset of the compare string.  This must be
 *                within the string bounds.  The default start postion is 1.
 * @param len_    The length of the compare substring.  The length and the
 *                offset must specify a valid substring of other.  If not
 *                specified, this defaults to the substring from the
 *                offset to the end of the string.
 *
 * @return True if the two regions match, false for any mismatch.
 */
RexxInteger *RexxMutableBuffer::caselessMatch(RexxInteger *start_, RexxString *other, RexxInteger *offset_, RexxInteger *len_)
{
    stringsize_t _start = positionArgument(start_, ARG_ONE);
    // the start position must be within the string bounds
    if (_start > getLength())
    {
        reportException(Error_Incorrect_method_position, start_);
    }
    other = stringArgument(other, ARG_TWO);

    stringsize_t offset = optionalPositionArgument(offset_, 1, ARG_THREE);

    if (offset > other->getLength())
    {
        reportException(Error_Incorrect_method_position, offset);
    }

    stringsize_t len = optionalLengthArgument(len_, other->getLength() - offset + 1, ARG_FOUR);

    if ((offset + len - 1) > other->getLength())
    {
        reportException(Error_Incorrect_method_length, len);
    }

    return primitiveCaselessMatch(_start, other, offset, len) ? TheTrueObject : TheFalseObject;
}


/**
 * Perform a compare of regions of two string objects.  Returns
 * true if the two regions match, returns false for mismatches.
 *
 * @param start  The starting offset within the target string.
 * @param other  The source string for the compare.
 * @param offset The offset of the substring of the other string to use.
 * @param len    The length of the substring to compare.
 *
 * @return True if the regions match, false otherwise.
 */
bool RexxMutableBuffer::primitiveMatch(stringsize_t _start, RexxString *other, stringsize_t offset, stringsize_t len)
{
    _start--;      // make the starting point origin zero
    offset--;

    // if the match is not possible in the target string, just return false now.
    if ((_start + len) > getLength())
    {
        return false;
    }

    return memcmp(getStringData() + _start, other->getStringData() + offset, len) == 0;
}


/**
 * Perform a caselesee compare of regions of two string objects.
 * Returns true if the two regions match, returns false for
 * mismatches.
 *
 * @param start  The starting offset within the target string.
 * @param other  The source string for the compare.
 * @param offset The offset of the substring of the other string to use.
 * @param len    The length of the substring to compare.
 *
 * @return True if the regions match, false otherwise.
 */
bool RexxMutableBuffer::primitiveCaselessMatch(stringsize_t _start, RexxString *other, stringsize_t offset, stringsize_t len)
{
    _start--;      // make the starting point origin zero
    offset--;

    // if the match is not possible in the target string, just return false now.
    if ((_start + len) > getLength())
    {
        return false;
    }

    return StringUtil::caselessCompare(getStringData() + _start, other->getStringData() + offset, len) == 0;
}


/**
 * Compare a single character at a give position against
 * a set of characters to see if any of the characters is
 * a match.
 *
 * @param position_ The character position
 * @param matchSet  The set to compare against.
 *
 * @return true if the character at the give position is any of the characters,
 *         false if none of them match.
 */
RexxInteger *RexxMutableBuffer::matchChar(RexxInteger *position_, RexxString *matchSet)
{
    stringsize_t position = positionArgument(position_, ARG_ONE);
    // the start position must be within the string bounds
    if (position > getLength())
    {
        reportException(Error_Incorrect_method_position, position);
    }
    matchSet = stringArgument(matchSet, ARG_TWO);

    stringsize_t _setLength = matchSet->getLength();
    char         _matchChar = getChar(position - 1);

    // iterate through the match set looking for a match
    for (stringsize_t i = 0; i < _setLength; i++)
    {
        if (_matchChar == matchSet->getChar(i))
        {
            return TheTrueObject;
        }
    }
    return TheFalseObject;
}


/**
 * Compare a single character at a give position against
 * a set of characters to see if any of the characters is
 * a match.
 *
 * @param position_ The character position
 * @param matchSet  The set to compare against.
 *
 * @return true if the character at the give position is any of the characters,
 *         false if none of them match.
 */
RexxInteger *RexxMutableBuffer::caselessMatchChar(RexxInteger *position_, RexxString *matchSet)
{
    stringsize_t position = positionArgument(position_, ARG_ONE);
    // the start position must be within the string bounds
    if (position > getLength())
    {
        reportException(Error_Incorrect_method_position, position);
    }
    matchSet = stringArgument(matchSet, ARG_TWO);

    stringsize_t _setLength = matchSet->getLength();
    char         _matchChar = getChar(position - 1);
    _matchChar = toupper(_matchChar);

    // iterate through the match set looking for a match, using a
    // caseless compare
    for (stringsize_t i = 0; i < _setLength; i++)
    {
        if (_matchChar == toupper(matchSet->getChar(i)))
        {
            return TheTrueObject;
        }
    }
    return TheFalseObject;
}


/**
 * Perform a character verify operation on a mutable buffer.
 *
 * @param ref    The reference string.
 * @param option The match/nomatch option.
 * @param _start The start position for the verify.
 * @param range  The range to search
 *
 * @return The offset of the first match/mismatch within the buffer.
 */
RexxInteger *RexxMutableBuffer::verify(RexxString *ref, RexxString *option, RexxInteger *_start, RexxInteger *range)
{
    return StringUtil::verify(getStringData(), getLength(), ref, option, _start, range);
}


/**
 * Perform a subword extraction from a mutable buffer.
 *
 * @param position The first word to be extracted.
 * @param plength  The number of words to extract.
 *
 * @return The substring containing the extacted words.
 */
RexxString *RexxMutableBuffer::subWord(RexxInteger *position, RexxInteger *plength)
{
    return StringUtil::subWord(getStringData(), getLength(), position, plength);
}


/**
 * Extract a given word from a mutable buffer.
 *
 * @param position The target word position.
 *
 * @return The extracted word, as a string.
 */
RexxString *RexxMutableBuffer::word(RexxInteger *position)
{
    return StringUtil::word(getStringData(), getLength(), position);
}


/**
 * return the index of a given word position in a mutable buffer
 *
 *
 * @param position The target word position.
 *
 * @return The position of the target word.
 */
RexxInteger *RexxMutableBuffer::wordIndex(RexxInteger *position)
{
    return StringUtil::wordIndex(getStringData(), getLength(), position);
}


/**
 * return the length of a given word position in a mutable
 * buffer
 *
 *
 * @param position The target word position.
 *
 * @return The length of the target word.
 */
RexxInteger *RexxMutableBuffer::wordLength(RexxInteger *position)
{
    return StringUtil::wordLength(getStringData(), getLength(), position);
}

/**
 * Return the count of words in the buffer.
 *
 * @return The buffer word count.
 */
RexxInteger *RexxMutableBuffer::words()
{
    size_t tempCount = StringUtil::wordCount(this->getStringData(), this->getLength());
    return new_integer(tempCount);
}


/**
 * Perform a wordpos search on a mutablebuffer object.
 *
 * @param phrase The search phrase
 * @param pstart The starting search position.
 *
 * @return The index of the match location.
 */
RexxInteger *RexxMutableBuffer::wordPos(RexxString  *phrase, RexxInteger *pstart)
{
    return StringUtil::wordPos(getStringData(), getLength(), phrase, pstart);
}


/**
 * Perform a caseless wordpos search on a string object.
 *
 * @param phrase The search phrase
 * @param pstart The starting search position.
 *
 * @return The index of the match location.
 */
RexxInteger *RexxMutableBuffer::caselessWordPos(RexxString  *phrase, RexxInteger *pstart)
{
    return StringUtil::caselessWordPos(getStringData(), getLength(), phrase, pstart);
}


/**
 * Perform a delword operation on a mutable buffer
 *
 * @param position The position to delete.
 * @param plength  The number of words to delete
 *
 * @return Always returns the target mutable buffer.
 */
RexxMutableBuffer *RexxMutableBuffer::delWord(RexxInteger *position, RexxInteger *plength)
{
                                         /* convert position to binary        */
    size_t _wordPos = positionArgument(position, ARG_ONE);
    /* get num of words to delete, the   */
    /* default is "a very large number"  */
    size_t count = optionalLengthArgument(plength, Numerics::MAX_WHOLENUMBER, ARG_TWO);

    size_t length = getLength();         /* get string length                 */
    if (length == 0)                     /* null string?                      */
    {
        return this;                     /* nothing to delete                 */
    }
    if (count == 0)                      /* deleting zero words?              */
    {
        return this;                     /* also very easy                    */
    }
    const char *_word = getStringData();  /* point to the string               */
    const char *nextSite = NULL;
                                       /* get the first word                */
    size_t _wordLength = StringUtil::nextWord(&_word, &length, &nextSite);
    while (--_wordPos > 0 && _wordLength != 0)
    {  /* loop until we reach tArget        */
        _word = nextSite;                /* copy the start pointer            */
                                         /* get the next word                 */
        _wordLength = StringUtil::nextWord(&_word, &length, &nextSite);
    }
    if (_wordPos != 0)                   /* run out of words first            */
    {
        return this;                     /* return the buffer unaltered       */
    }
    // get the deletion point as an offset
    size_t deletePosition = _word - this->getStringData();
    while (--count > 0 && _wordLength != 0)
    {  /* loop until we reach tArget        */
        _word = nextSite;              /* copy the start pointer            */
                                       /* get the next word                 */
        _wordLength = StringUtil::nextWord(&_word, &length, &nextSite);
    }
    if (length != 0)                   /* didn't use up the string          */
    {
        StringUtil::skipBlanks(&nextSite, &length);/* skip over trailing blanks         */
    }

    size_t gapSize = dataLength - (deletePosition + length);
    // close up the delete part
    closeGap(deletePosition, gapSize, length);
    // adjust for the deleted data
    dataLength -= gapSize;
    return this;
}
