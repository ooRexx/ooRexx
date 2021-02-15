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

#include "ObjectClass.hpp"
#include "SourceLocation.hpp"

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

    ProgramSource() : firstLine(1), lineCount(0) { };
    inline ProgramSource(RESTORETYPE restoreType) { ; };

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
    // provides the starting line location.  Can be non-zero
    // if this is an interpret
    virtual size_t getFirstLine() { return firstLine; }
    size_t getLineCount() { return lineCount; }

    RexxString *getStringLine(size_t lineNumber);
    RexxString *getStringLine(size_t position, size_t startOffset, size_t endOffset = SIZE_MAX);
    RexxString *extract(SourceLocation &location);
    ArrayClass *extractSourceLines(SourceLocation &location);

protected:

    size_t firstLine;               // the first line of the source for parsing
    size_t lineCount;               // count of lines in the source file.
};


/**
 * A program source where the data is stored in a BufferClass
 * object.
 */
class BufferProgramSource: public ProgramSource
{
 public:
    void *operator new(size_t);
    inline void  operator delete(void *) { ; }

    BufferProgramSource(BufferClass *b) : buffer(b), descriptorArea(OREF_NULL), ProgramSource() { }
    inline BufferProgramSource(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // virtual definitions
    void setup() override;
    void getLine(size_t lineNumber, const char *&data, size_t &length) override;
    bool isTraceable() override { return true; }

    const char *getBufferPointer();
    void getBuffer(const char *&data, size_t &length);

    void            buildDescriptors();
    LineDescriptor *getDescriptors();
    LineDescriptor &getDescriptor(size_t l);

protected:

    BufferClass *descriptorArea;   // our table of line descriptors
    BufferClass *buffer;           // the buffer where the source data is installed
};


/**
 * A program source where the data is stored in a BufferClass
 * object read from a file.
 */
class FileProgramSource: public BufferProgramSource
{
 public:
    void *operator new(size_t);
    inline void  operator delete(void *) { ; }

    // we provide the buffer source after we've read the file information in from
    // the target file.
    FileProgramSource(RexxString *f) : fileName(f), BufferProgramSource(OREF_NULL) { }
    inline FileProgramSource(RESTORETYPE restoreType) : BufferProgramSource(restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    // virtual definitions
    void setup() override;

    // also used directly by the language parser
    static BufferClass* readProgram(const char *file_name);

protected:

    RexxString *fileName;      // file for the program source
};


/**
 * A program source where the data is stored in an Array object.
 */
class ArrayProgramSource: public ProgramSource
{
 public:
    void *operator new(size_t);
    inline void  operator delete(void *) { ; }

    ArrayProgramSource(ArrayClass *a, size_t adjust = 0) : interpretAdjust(adjust), array(a), ProgramSource() { };
    inline ArrayProgramSource(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    void setup() override;
    void getLine(size_t lineNumber, const char *&data, size_t &length) override;
    bool isTraceable() override { return true; }

 protected:

    size_t interpretAdjust;  // if this is an interpret, we fudge the line positions
    ArrayClass  *array;       // the array where the source data is installed
};

#endif
