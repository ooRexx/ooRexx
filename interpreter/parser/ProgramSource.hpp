/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                             ProgramSource.hpp  */
/*                                                                            */
/* Classes for abstracting out different types of program source.             */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ProgramSource
#define Included_ProgramSource

#define line_delimiters "\r\n"         // stream file line end characters
#define ctrl_z 0x1a                    // the end of file marker

/**
 * A descriptor for a single source line.
 */
class LineDescriptor
{
 public:
    LineDescriptor() : position(0), length(0) { }

    // clear the descriptor
    inline void clear()
    {
        position = 0;
        length = 0;
    }

    // map the descriptor to a line position/length
    inline void getLine(const char *dataPointer, const char *&linePointer, size_t &lineLength)
    {
        linePointer = dataPointer + position;
        lineLength = length;
    }

    size_t position;                     // position with the buffer
    size_t length;                       // length of the line
};


/**
 * Base class for data holding program source.  This also acts
 * as a source provider for "sourceless" code units.
 */
class ProgramSource: public RexxInternalObject
{
public:
    void *operator new(size_t);
    inline void  operator delete(void *) { ; }
    ProgramSource() : lineCount(0) { };

    // each subclass will need to implement this.
    // default setup is to do nothing.
    virtual void setup() { }
    // a default implementation that returns nothing.
    virtual void getLine(size_t lineNumber, const char *&data, size_t &length)
    {
        data = NULL;
        length = 0;
    }

    // provides with real source need to override this
    virtual bool isTraceable() { return false; }
    inline size_t getLineCount() { return lineCount; }

    RexxString *getStringLine(size_t lineNumber);
    RexxString *extract(SourceLocation &location);

protected:

    size_t lineCount;               // count of lines in the source file.
};


/**
 * A program source where the data is stored in a RexxBuffer
 * object.
 */
class BufferProgramSource: public ProgramSource
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { ; }
    BufferProgramSource(RexxBuffer *b) : buffer(b), descriptorArea(OREF_NULL), ProgramSource() { }

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(RexxEnvelope *);

    // virtual definitions
    virtual void getBuffer(const char *&data, size_t &length);
    virtual void setup();
    virtual void getLine(size_t lineNumber, const char *&data, size_t &length);
    virtual const char *getBufferPointer();
    virtual bool isTraceable() { return true; }

    void            buildDescriptors();
    LineDescriptor *getDescriptors();
    LineDescriptor &getDescriptor(size_t l);

protected:

    RexxBuffer *descriptorArea;   // our table of line descriptors
    RexxBuffer *buffer;           // the buffer where the source data is installed
};


/**
 * A program source where the data is stored in a RexxBuffer
 * object read from a file.
 */
class FileProgramSource: public BufferProgramSource
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { ; }

    // we provide the buffer source after we've read the file information in from
    // the target file.
    FileProgramSource(RexxString *f) : fileName(f), BufferProgramSource(OREF_NULL) { }

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(RexxEnvelope *);

    // virtual definitions
    virtual void setup();
    virtual void getBuffer(const char *&data, size_t &length);

protected:

    RexxString *fileName;      // file for the program source
};


/**
 * A program source where the data is stored in an Array object.
 */
class ArrayProgramSource: public ProgramSource
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { ; };

    ArrayProgramSource(RexxArray *a, size_t adjust = 0) : interpretAdjust(adjust), array(a), ProgramSource() { };

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(RexxEnvelope *);

    virtual void getLine(size_t lineNumber, const char *&data, size_t &length);
    virtual bool isTraceable() { return true; }

 protected:

    size_t interpretAdjust;  // if this is an interpret, we fudge the line positions
    RexxArray  *array;       // the array where the source data is installed
};

#endif
