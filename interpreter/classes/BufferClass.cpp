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
/* Primitive Buffer Class                                                     */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "RexxCore.h"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "BufferClass.hpp"


RexxClass *RexxBuffer::classInstance = OREF_NULL;   // singleton class instance

void RexxBuffer::createInstance()
/******************************************************************************/
/* Function:  Create initial bootstrap objects                                */
/******************************************************************************/
{
    CLASS_CREATE(Buffer, "Buffer", RexxClass);
}


RexxBuffer *RexxBuffer::expand(
    size_t l)                            /* minimum space needed              */
/******************************************************************************/
/* Function:  Create a larger buffer and copy existing data into it           */
/******************************************************************************/
{
    RexxBuffer * newBuffer;              /* returned new buffer               */

                                         /* we will either return a buffer    */
                                         /* twice the size of the current     */
                                         /* buffer, or this size of           */
                                         /* current(this)buffer + requested   */
                                         /* minimum length.                   */
    if (l > this->getBufferSize())       /* need more than double?            */
    {
        /* increase by the requested amount  */
        newBuffer = new_buffer(this->getBufferSize() + l);
    }
    else                                 /* just double the existing length   */
    {
        newBuffer = new_buffer(this->getBufferSize() * 2);
    }
    /* have new buffer, so copy data from*/
    /* current buffer into new buffer.   */
    memcpy(newBuffer->getData(), this->getData(), this->getDataLength());
    return newBuffer;                    /* all done, return new buffer       */

}

void *RexxBuffer::operator new(size_t size, size_t _length)
/******************************************************************************/
/* Function:  Create a new buffer object                                      */
/******************************************************************************/
{
                                         /* Get new object                    */
    RexxBuffer *newBuffer = (RexxBuffer *) new_object(size + _length, T_Buffer);
    /* Initialize this new buffer        */
    newBuffer->bufferSize = _length;     /* set the length of the buffer      */
    newBuffer->dataLength = _length;     // by default, the data length and size are the same
    newBuffer->setHasNoReferences();     /* this has no references            */
    return(void *)newBuffer;            /* return the new buffer             */
}


RexxObject *RexxBuffer::newRexx(RexxObject **args, size_t argc)
/******************************************************************************/
/* Function:  Allocate a buffer  object from Rexx code.                       */
/******************************************************************************/
{
    // we do not allow these to be allocated from Rexx code...
    reportException(Error_Unsupported_new_method, ((RexxClass *)this)->getId());
    return TheNilObject;
}

