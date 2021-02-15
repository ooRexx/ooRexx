/*----------------------------------------------------------------------------*/
/*                                                                            */
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

#include "RexxCore.h"
#include "FileNameBuffer.hpp"
#include "ActivityManager.hpp"


/**
 * Constructor for a filename buffer. Performs initial setup
 *
 * @param size   The initial size. If zero, then the default size is used.
 */
FileNameBuffer::FileNameBuffer(size_t initial) : buffer(NULL), bufferSize(0)
{
    init(initial);
}


/**
 * Copy constructor for a FileNameBuffer object
 *
 * @param o      The source object.
 */
FileNameBuffer::FileNameBuffer(const FileNameBuffer &o)
{
    init(o.bufferSize);
    strncpy(buffer, o.buffer, bufferSize);
}


/**
 * Do the initial setup for a FileNameBuffer object.
 *
 * @param initial The initial capacity.
 */
void FileNameBuffer::init(size_t initial)
{
    if (initial == 0)
    {
        initial = SysFileSystem::MaximumPathLength;
    }

    bufferSize = initial;
    buffer = new char[bufferSize];

    // if we can't allocate, then raise the error
    if (buffer == NULL)
    {
        handleMemoryError();
    }
    // By default, we make this a null ascii-z string
    buffer[0] = '\0';
}


/**
 * Destructor to perform out of scope cleanup.
 */


void FileNameBuffer::ensureCapacity(size_t c)
{
    // we always leave a space for a null terminator here
    size_t newSize = c + 1;

    if (bufferSize < newSize)
    {
        char *newBuffer = new char[newSize];

        // if we can't allocate, then raise the error
        if (newBuffer == NULL)
        {
            handleMemoryError();
        }

        // copy old data over and release the old buffer
        memcpy(newBuffer, buffer, bufferSize);
        bufferSize = newSize;
        delete buffer;
        buffer = newBuffer;
    }
}


/**
 * Handle a memory error. Base implementation is for just raising a normal exception.
 * This can only be used within internal code when the kernel lock is held.
 */
void FileNameBuffer::handleMemoryError()
{
    reportException(Error_System_resources);
}
