/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                        MutableBufferClass.hpp  */
/*                                                                            */
/* Primitive MutableBuffer Class Definition                                   */
/*                                                                            */
/******************************************************************************/
#ifndef Included_MutableBuffer
#define Included_MutableBuffer

#include "StringClass.hpp"
#include "IntegerClass.hpp"
#include "BufferClass.hpp"


/**
 * A string-like object where in-place updating of the data is permitted.
 */
class MutableBuffer : public RexxObject
{
 public:
           void *operator new(size_t size);

           MutableBuffer();
           MutableBuffer(size_t, size_t);
    inline MutableBuffer(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *envelope) override;

    RexxInternalObject *copy() override;
    void        ensureCapacity(size_t addedLength);

    RexxObject *lengthRexx();

    MutableBuffer *appendRexx(RexxObject **args, size_t argc);
    MutableBuffer *setTextRexx(RexxObject *);
    void           setText(RexxString *);
    MutableBuffer *insert(RexxObject*, RexxObject*, RexxObject*, RexxObject*);
    MutableBuffer *overlay(RexxObject*, RexxObject*, RexxObject*, RexxObject*);
    MutableBuffer *bracketsEqual(RexxObject *str, RexxObject *pos, RexxObject *len);
    MutableBuffer *replaceAt(RexxObject *str, RexxObject *pos, RexxObject *len, RexxObject *pad);
    MutableBuffer *mydelete(RexxObject*, RexxObject*);
    RexxString    *substr(RexxInteger *startPosition, RexxInteger *len, RexxString *pad);
    RexxString    *brackets(RexxInteger *startPosition, RexxInteger *len);
    RexxInteger   *lastPos(RexxString *needle, RexxInteger *_start, RexxInteger *_range);
    RexxInteger   *posRexx(RexxString *needle, RexxInteger *_start, RexxInteger *_range);
    RexxObject    *containsRexx(RexxString *needle, RexxInteger *_start, RexxInteger *_range);
    RexxInteger   *caselessLastPos(RexxString *needle, RexxInteger *_start, RexxInteger *_range);
    RexxInteger   *caselessPos(RexxString *needle, RexxInteger *_start, RexxInteger *_range);
    RexxObject    *caselessContains(RexxString *needle, RexxInteger *_start, RexxInteger *_range);
    RexxString    *subchar(RexxInteger *startPosition);

    RexxInteger   *getBufferSize() { return new_integer(bufferLength); }
    RexxObject    *setBufferSize(RexxInteger*);
    ArrayClass    *makeArrayRexx(RexxString *div);

    ArrayClass *makeArray() override;
    RexxString *makeString() override;
    RexxString *primitiveMakeString() override;
    RexxString *stringValue()override;

    RexxInteger   *countStrRexx(RexxString *needle);
    RexxInteger   *caselessCountStrRexx(RexxString *needle);
    MutableBuffer *changeStr(RexxString *needle, RexxString *newNeedle, RexxInteger *countArg);
    MutableBuffer *caselessChangeStr(RexxString *needle, RexxString *newNeedle, RexxInteger *countArg);
    MutableBuffer *upper(RexxInteger *_start, RexxInteger *_length);
    MutableBuffer *lower(RexxInteger *_start, RexxInteger *_length);
    MutableBuffer *translate(RexxString *tableo, RexxString *tablei, RexxString *pad, RexxInteger *, RexxInteger *);
    RexxObject  *match(RexxInteger *start_, RexxString *other, RexxInteger *offset_, RexxInteger *len_);
    RexxObject  *caselessMatch(RexxInteger *start_, RexxString *other, RexxInteger *offset_, RexxInteger *len_);
    RexxObject  *startsWithRexx(RexxString *other);
    RexxObject  *endsWithRexx(RexxString *other);
    RexxObject  *caselessStartsWithRexx(RexxString *other);
    RexxObject  *caselessEndsWithRexx(RexxString *other);
    bool primitiveMatch(size_t start, RexxString *other, size_t offset, size_t len);
    bool primitiveCaselessMatch(size_t start, RexxString *other, size_t offset, size_t len);
    RexxObject  *matchChar(RexxInteger *position_, RexxString *matchSet);
    RexxObject  *caselessMatchChar(RexxInteger *position_, RexxString *matchSet);
    RexxInteger *verify(RexxString *, RexxString *, RexxInteger *, RexxInteger *);
    RexxString  *subWord(RexxInteger *, RexxInteger *);
    ArrayClass  *subWords(RexxInteger *, RexxInteger *);
    RexxString  *word(RexxInteger *);
    RexxInteger *wordIndex(RexxInteger *);
    RexxInteger *wordLength(RexxInteger *);
    RexxInteger *words();
    RexxInteger *wordPos(RexxString *, RexxInteger *);
    RexxInteger *caselessWordPos(RexxString *, RexxInteger *);
    RexxObject  *containsWord(RexxString *, RexxInteger *);
    RexxObject  *caselessContainsWord(RexxString *, RexxInteger *);
    MutableBuffer *delWord(RexxInteger *position, RexxInteger *plength);
    MutableBuffer *space(RexxInteger *space_count, RexxString  *pad);

    inline const char *getStringData() { return data->getData(); }
    inline size_t      getLength()     { return dataLength; }
    inline char *      getData()       { return data->getData(); }
    void append(char c);
    void append(const char *string, size_t l);
    void append(RexxString *s) { append(s->getStringData(), s->getLength()); };
    void append(const char *string) { append(string, strlen(string)); }
    inline void copyData(size_t offset, const char *string, size_t l) { data->copyData(offset, string, l); }
    inline void openGap(size_t offset, size_t _size, size_t tailSize) { data->openGap(offset, _size, tailSize); }
    inline void closeGap(size_t offset, size_t _size, size_t tailSize) { data->closeGap(offset, _size, tailSize); }
    inline void adjustGap(size_t offset, size_t _size, size_t _newSize) { data->adjustGap(offset, _size, _newSize, dataLength); }
    inline void setData(size_t offset, char character, size_t l) { data->setData(offset, character, l); }
           size_t setDataLength(size_t l);
    inline char getChar(size_t offset) { return getData()[offset]; }
    inline size_t getCapacity() { return bufferLength; }
           char *setCapacity(size_t newLength);

           MutableBuffer *newRexx(RexxObject**, size_t);

    static const size_t DEFAULT_BUFFER_LENGTH = 256;

    static void createInstance();
    static RexxClass *classInstance;

 protected:

     size_t             bufferLength;    /* buffer length                   */
     size_t             defaultSize;     /* default size when emptied       */
     size_t             dataLength;      // current length of data
     BufferClass        *data;            /* buffer used for the data        */
};
#endif
