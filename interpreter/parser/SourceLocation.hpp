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
/* REXX Kernel                                             SourceLocation.hpp */
/*                                                                            */
/* Primitive Translator Token Class Definitions                               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_SourceLocation
#define Included_SourceLocation


class SourceLocation                   /* token/clause location information */
{
public:
    inline size_t getLineNumber() { return startLine; }
    inline size_t getOffset()     { return startOffset; }
    inline size_t getEndLine()    { return endLine; }
    inline size_t getEndOffset()  { return endOffset; }

    inline void setLineNumber(size_t l) { startLine = l; }
    inline void setOffset(size_t l)     { startOffset = l; }
    inline void setEndLine(size_t l)    { endLine = l; }
    inline void setEndOffset(size_t l)  { endOffset = l; }

    inline void setStart(SourceLocation &l)
    {
        startLine = l.getLineNumber();
        startOffset = l.getOffset();
    }

    inline void setEnd(SourceLocation &l)
    {
        setEnd(l.getEndLine(), l.getEndOffset());
    }

    inline void   setStart(size_t line, size_t offset)
    {
        startLine = line;
        startOffset = offset;
    }

    inline void   setEnd(size_t line, size_t offset)
    {
        // only set if this makes sense
        if (line > startLine || (line == startLine && offset > startOffset))
        {
            endLine = line;
            endOffset = offset;
        }

    }

    inline void setLocation(size_t line, size_t offset, size_t end, size_t end_offset)
    {
        startLine = line;
        startOffset = offset;
        endLine = end;
        endOffset = end_offset;
    }

    inline void setLocation(SourceLocation &l)
    {
        startLine = l.startLine;
        startOffset = l.startOffset;
        endLine = l.endLine;
        endOffset = l.endOffset;
    }

    inline const SourceLocation & operator= (const SourceLocation &l)
    {
        startLine = l.startLine;
        startOffset = l.startOffset;
        endLine = l.endLine;
        endOffset = l.endOffset;
        return *this;
    }

protected:
    size_t startLine;                    // file line start location
    size_t startOffset;                  // offset within the file line
    size_t endLine;                      // file line end location
    size_t endOffset;                    // file end offset
};


#endif
