/*----------------------------------------------------------------------------*/
/*                                                                            */
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
/*                                                        InputRedirector.hpp */
/*                                                                            */
/* Implementations of the different ADDRESS WITH Input redirectors            */
/*                                                                            */
/******************************************************************************/
#ifndef Included_InputRedirector
#define Included_InputRedirector

#include "RexxCore.h"
#include "CommandIOConfiguration.hpp"
#include "StemClass.hpp"

class NativeActivation;
class MutableBuffer;

/**
 * Base class for I/O redirectors.
 */
class InputRedirector : public RexxInternalObject
{
 public:
    // Note, we never directly create instances of this
    // class so we don't need new, live, etc. These must
    // be implemented by the concrete classes
    inline InputRedirector() : inputBuffer(OREF_NULL) { ; }
    inline InputRedirector(RESTORETYPE restoreType) { ; };

    virtual void init () { ; }
    virtual RexxString *read(NativeActivation *context) { return OREF_NULL; }
    virtual void readBuffered(NativeActivation *context, const char *&data, size_t &length);
    virtual void cleanup() { ; }
    virtual RedirectionType::Enum type() { return RedirectionType::NONE; }
    virtual RexxObject *target() { return OREF_NULL; }

protected:
    MutableBuffer *inputBuffer;      // buffered version of the input

    // a reasonable initial buffer size
    static const size_t defaultBufferSize = 4096;
};


/**
 */
class StemInputSource : public InputRedirector
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    StemInputSource(StemClass *stem);
    inline StemInputSource(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void init () override;
    RexxString *read(NativeActivation *context) override;
    RedirectionType::Enum type() override { return RedirectionType::STEM_VARIABLE; }
    RexxObject *target() override { return stem; }

protected:
    StemClass  *stem;      // the stem we're handling
    RexxString *lastValue; // last value returned for GC protection
    size_t      index;     // the current read index
    size_t      arraySize; // the limit to where we write
};


/**
 * Inpout handler for reading lines from an InputStream object
 *
 *
 */
class StreamObjectInputSource : public InputRedirector
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    inline StreamObjectInputSource() { ; }
    StreamObjectInputSource(RexxObject *s);
    inline StreamObjectInputSource(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void init() override { hitEnd = false; }
    RexxString *read(NativeActivation *context) override;
    RedirectionType::Enum type() override { return RedirectionType::STREAM_OBJECT; }
    RexxObject *target() override { return stream; }

protected:
    RexxObject *stream;   // the stream object
    RexxString *lastValue;// last value returned for GC protection
    bool        hitEnd;   // indicator that we're done reading
};


/**
 * Input handler for supplying lines from a named stream
 *
 */
class StreamInputSource : public StreamObjectInputSource
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    StreamInputSource(RexxString *n);
    inline StreamInputSource(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void init () override;
    void cleanup() override;
    RedirectionType::Enum type() override { return RedirectionType::STREAM_NAME; }
    RexxObject *target() override { return name; }

protected:
    RexxString *name;     // the stream name
};


/**
 * Output handler for collecting lines in an ordered collection
 *
 */
class ArrayInputSource : public InputRedirector
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    ArrayInputSource(ArrayClass *a);
    inline ArrayInputSource(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void init () override;
    RexxString *read(NativeActivation *context) override;
    RedirectionType::Enum type() override { return RedirectionType::ARRAY_OBJECT; }
    RexxObject *target() override { return array; }

protected:
    ArrayClass *array;          // the source array object
    RexxString *lastValue;      // string version of last value returned to protect from GC.
    size_t      index;          // current read index
    size_t      arraySize;      // read upper limit
};
#endif

