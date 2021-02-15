/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* Primitive Smart Buffer Class                                               */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "BufferClass.hpp"
#include "SmartBuffer.hpp"


/**
 * Allocate storage for a SmartBuffer object.
 *
 * @param size   The size of the SmartBuffer data.
 *
 * @return Storage for creating a buffer instance.
 */
void   *SmartBuffer::operator new(size_t size)
{
    return new_object(size, T_SmartBuffer);
}


/**
 * Initialize a new SmartBuffer object.
 *
 * @param startSize The initial buffer size.
 */
SmartBuffer::SmartBuffer(size_t startSize)
{
    buffer = new_buffer(startSize);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void SmartBuffer::live(size_t liveMark)
{
    memory_mark(buffer);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void SmartBuffer::liveGeneral(MarkReason reason)
{
    memory_mark_general(buffer);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void SmartBuffer::flatten(Envelope *envelope)
{
    setUpFlatten(SmartBuffer)

    flattenRef(buffer);

    cleanUpFlatten
}


/**
 * Copy data into the buffer at the very end.
 *
 * @param start  The data to be copied.
 * @param length The length of data to copy.
 *
 * @return The offset of the copied data.
 */
size_t SmartBuffer::copyData(void *start, size_t length)
{
    // not enough room to copy this?  Get a bigger backing buffer
    if (space() < length)
    {
        setField(buffer, buffer->expand(length));
    }

    // copy the data into the buffer
    buffer->copyData(current, (char *)start, length);

    // bump the copy location, but return the offset of of where
    // this data was copied.
    size_t dataLoc = current;
    current = current + length;
    return dataLoc;
}

/**
 * Calculate the available space for the buffer.
 *
 * @return The size remaining in our backing buffer.
 */
size_t SmartBuffer::space()
{
    return buffer->getBufferSize() - current;
}

