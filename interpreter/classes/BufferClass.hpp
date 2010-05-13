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
/* REXX Kernel                                               BufferClass.hpp  */
/*                                                                            */
/* Primitive Buffer Class Definitions                                         */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxBuffer
#define Included_RexxBuffer

class RexxBufferBase : public RexxObject
{
public:

    inline RexxBufferBase() {;};

    inline size_t getDataLength() { return this->dataLength; }
    inline size_t getBufferSize() { return this->bufferSize; }
    inline void   setDataLength(size_t l) { this->dataLength = l; }
    virtual char *getData() = 0;
    inline void copyData(size_t offset, const char *string, size_t l) { memcpy(this->getData() + offset, string, l); }
    inline void copyData(CONSTRXSTRING &r) { copyData(0, r.strptr, r.strlength); }
    inline void copyData(RXSTRING &r) { copyData(0, r.strptr, r.strlength); }
    inline void openGap(size_t offset, size_t _size, size_t tailSize)
    {
        memmove(getData() + offset + _size, getData() + offset, tailSize);
    }
    inline void closeGap(size_t offset, size_t _size, size_t tailSize)
    {
        memmove(getData() + offset, getData() + offset + _size, tailSize);
    }

    inline void adjustGap(size_t offset, size_t _size, size_t _newSize)
    {
        memmove(getData() + offset + _newSize, getData() + offset + _size, getDataLength() - (offset + _size));
    }
    inline void setData(size_t offset, char character, size_t l)
    {
        memset(getData() + offset, character, l);
    }
protected:

    // the following field is padding required to get the start of the of the
    // buffer data aligned on an object grain boundary.  Since we unflatten saved programs
    // by reducing the size of the surrounding buffer to reveal the exposed data, we need
    // to ensure appropriate data alignment.  Fortunately, because the sizes of all of the
    // fields doubles when going to 64-bit, this single padding item is sufficient to
    // get everything lined up on all platforms.
    size_t reserved;
    size_t bufferSize;                  // size of the buffer
    size_t dataLength;                  // length of the buffer data (freqently the same)
};


class RexxBuffer : public RexxBufferBase
{
public:
    void *operator new(size_t, size_t);
    inline void *operator new(size_t size, void *ptr) {return ptr;};
    inline void  operator delete(void *) { ; }
    inline void  operator delete(void *, size_t) { ; }
    inline void  operator delete(void *, void *) { ; }

    inline RexxBuffer() {;}
    inline RexxBuffer(RESTORETYPE restoreType) { ; }

    RexxBuffer *expand(size_t);
    RexxObject *newRexx(RexxObject **args, size_t argc);
    virtual char *getData() { return data; }

    static void createInstance();

    static RexxClass *classInstance;   // singleton class instance

protected:
    char data[4];                       /* actual data length                */
};


 inline RexxBuffer *new_buffer(size_t s) { return new (s) RexxBuffer; }
 inline RexxBuffer *new_buffer(CONSTRXSTRING &r)
 {
     RexxBuffer *b = new_buffer(r.strlength);
     b->copyData(r);
     return b;
 }

 inline RexxBuffer *new_buffer(RXSTRING &r)
 {
     RexxBuffer *b = new_buffer(r.strlength);
     b->copyData(r);
     return b;
 }

 inline RexxBuffer *new_buffer(const char *data, size_t length)
 {
     RexxBuffer *b = new_buffer(length);
     b->copyData(0, data, length);
     return b;
 }
#endif
