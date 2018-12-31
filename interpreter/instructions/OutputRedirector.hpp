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
/*                                                       OutputRedirector.hpp */
/*                                                                            */
/* Implementations of the different ADDRESS WITH Output redirectors           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_OutputRedirector
#define Included_OutputRedirector

#include "CommandIOConfiguration.hpp"
#include "StemClass.hpp"

class InputRedirector;

/**
 * Base class for I/O redirectors.
 */
class OutputRedirector : public RexxInternalObject
{
 public:

    // Note, we never directly create instances of this
    // class so we don't need new, live, etc. These must
    // be implemented by the concrete classes
    inline OutputRedirector() { ; }
    inline OutputRedirector(RESTORETYPE restoreType) { ; };

    virtual void init () { ; }
    virtual void writeLine(RexxString *v)  { ; }
    virtual void cleanup() { flushBuffer(); }
    virtual RedirectionType::Enum type() { return RedirectionType::NONE; }
    virtual RexxObject *target() { return OREF_NULL; }

    virtual bool needsBuffering(InputRedirector *d);
    virtual bool isSameTarget(OutputRedirector *e);

    void write(RexxString *l);
    void writeBuffer(const char *data, size_t len);
    static void scanLine(const char *data, const char *bufferEnd, const char *&lineEnd, const char *&nextLine);

 protected:
    void flushBuffer();
    void resolveBuffer(const char *&data, const char *bufferEnd);
    void consumeBuffer(const char *data, const char *bufferEnd);

    OutputOption::Enum option;    // REPLACE/APPEND option
    logical_t    initialized;     // indicates this redirector has been initialized already
    RexxString  *bufferedData;    // if we're doing writes using buffered data, this is the tail of the last write.
};


/**
 * Output handler for collecting lines in a stem object.
 */
class StemOutputTarget : public OutputRedirector
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    StemOutputTarget(StemClass *stem, OutputOption::Enum o);
    inline StemOutputTarget(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void init () override;
    void writeLine(RexxString *v) override;
    RedirectionType::Enum type() override { return RedirectionType::STEM_VARIABLE; }
    RexxObject *target() override { return stem; }

protected:
    StemClass *stem;      // the stem we're handling
    size_t     index;     // current index we're writing to
};


/**
 * Output handler for collecting lines in an OutputStream object
 *
 */
class StreamObjectOutputTarget : public OutputRedirector
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    inline StreamObjectOutputTarget() { ; };
    inline StreamObjectOutputTarget(RESTORETYPE restoreType) { ; };
    StreamObjectOutputTarget(RexxObject *s, OutputOption::Enum o);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void writeLine(RexxString *v) override;
    RedirectionType::Enum type() override { return RedirectionType::STREAM_OBJECT; }
    RexxObject *target() override { return stream; }

protected:
    RexxObject *stream;   // the stream object
};


/**
 * Output handler for collecting lines in a RexxQueue object
 *
 */
class RexxQueueOutputTarget : public OutputRedirector
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    inline RexxQueueOutputTarget() { ; };
    inline RexxQueueOutputTarget(RESTORETYPE restoreType) { ; };
    RexxQueueOutputTarget(RexxObject *s);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void writeLine(RexxString *v) override;
    RedirectionType::Enum type() override { return RedirectionType::REXXQUEUE_OBJECT; }
    RexxObject *target() override { return queue; }

protected:
    RexxObject *queue;   // the queue object
};


/**
 * Output handler for collecting lines in a named stream
 *
 */
class StreamOutputTarget : public StreamObjectOutputTarget
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    StreamOutputTarget(RexxString *n, OutputOption::Enum o);
    inline StreamOutputTarget(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    bool needsBuffering(InputRedirector *d) override;
    void init() override;
    void cleanup() override;
    RedirectionType::Enum type() override { return RedirectionType::STREAM_NAME; }
    RexxObject *target() override { return name; }
    bool isSameTarget(OutputRedirector *e) override;

protected:
    RexxString *name;     // the stream name
};


/**
 * Output handler for collecting lines in an ordered collection
 *
 */
class CollectionOutputTarget : public OutputRedirector
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    CollectionOutputTarget(RexxObject *c, OutputOption::Enum o);
    inline CollectionOutputTarget(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void init() override;
    void writeLine(RexxString *v) override;
    RedirectionType::Enum type() override { return RedirectionType::COLLECTION_OBJECT; }
    RexxObject *target() override { return collection; }

protected:
    RexxObject *collection;  // the target collection object
};


/**
 * Output handler for buffering lines for delayed writing when
 * the same object is used for both input and output
 *
 */
class BufferingOutputTarget : public OutputRedirector
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    BufferingOutputTarget(OutputRedirector *t);
    inline BufferingOutputTarget(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void init() override;
    void writeLine(RexxString *v) override;
    void cleanup() override;

protected:
    ArrayClass *collector;       // the buffer used for collection
    OutputRedirector *target;    // the final output target
};
#endif

