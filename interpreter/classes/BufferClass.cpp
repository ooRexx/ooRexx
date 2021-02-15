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
/*                                                                            */
/* Primitive Buffer Class                                                     */
/*                                                                            */
/******************************************************************************/

#include <algorithm>
#include "RexxCore.h"
#include "Activity.hpp"
#include "ActivityManager.hpp"
#include "BufferClass.hpp"


RexxClass *BufferClass::classInstance = OREF_NULL;   // singleton class instance


/**
 * Initial bootstrap of the buffer class object.
 */
void BufferClass::createInstance()
{
    CLASS_CREATE(Buffer);
}


/**
 * Allocate a new buffer object.
 *
 * @param size    The size of the object.
 * @param _length The length of the buffer portion required.
 *
 * @return The new buffer object.
 */
void *BufferClass::operator new(size_t size, size_t length)
{
    return new_object(size + length, T_Buffer);
}


/**
 * New method for the buffer class.  This always raises
 * an error if called.
 *
 * @param args   The new arguments.
 * @param argc   The argument count.
 *
 * @return Always raises an error.
 */
RexxObject *BufferClass::newRexx(RexxObject **args, size_t argc)
{
    // we do not allow these to be allocated from Rexx code...
    reportException(Error_Unsupported_new_method, ((RexxClass *)this)->getId());
    return TheNilObject;
}


/**
 * Create a larger buffer and copy existing data into it
 *
 * @param l      The new buffer size.
 *
 * @return A new buffer object of the expanded size.  All existing
 *         data from the target buffer is copied into the new buffer.
 */
BufferClass *BufferClass::expand(size_t l)
{
    // we will either return a buffer twice the size of the current
    // buffer, or this size of current(this)buffer + requested
    // minimum length.

    l = std::max(l, getBufferSize());

    BufferClass *newBuffer = new_buffer(getBufferSize() + l);
    // have new buffer, so copy data from current buffer into new buffer.
    newBuffer->copyData(0, getData(), getDataLength());
    return newBuffer;
}
