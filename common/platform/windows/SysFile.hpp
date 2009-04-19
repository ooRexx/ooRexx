/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/* REXX Kernel                                              SysFile.hpp       */
/*                                                                            */
/* System support for File operations.                                        */
/*                                                                            */
/******************************************************************************/

#ifndef Included_SysFile
#define Included_SysFile

#include "rexxapitypes.h"
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>

// The following define the platform independent open method flags
// openFlags argument flags
#define RX_O_RDONLY       _O_RDONLY
#define RX_O_WRONLY       _O_WRONLY
#define RX_O_RDWR         _O_RDWR
#define RX_O_CREAT        _O_CREAT
#define RX_O_EXCL         _O_EXCL
#define RX_O_TRUNC        _O_TRUNC
#define RX_O_APPEND       _O_APPEND
// openMode flags
#define RX_SH_DENYWR      _SH_DENYWR
#define RX_SH_DENYRD      _SH_DENYRD
#define RX_SH_DENYRW      _SH_DENYRW
#define RX_SH_DENYNO      _SH_DENYNO
// shareMode flags
#define RX_S_IWRITE       _S_IWRITE
#define RX_S_IREAD        _S_IREAD


class SysFile
{
public:
    SysFile();

    enum
    {
        DEFAULT_BUFFER_SIZE = 4096,   // default size for buffering
        LINE_POSITIONING_BUFFER = 512 // buffer size for line movement
    };

#define LINE_TERMINATOR "\r\n"

    bool open(const char *name, int openFlags, int openMode, int shareMode);
    bool open(int handle);
    void reset();
    void setStdIn();
    void setStdOut();
    void setStdErr();
    void setBuffering(bool buffer, size_t length);
    bool close();
    bool flush();
    bool read(char *buf, size_t len, size_t &bytesRead);
    bool write(const char *data, size_t len, size_t &bytesWritten);
    bool putChar(char ch);
    bool ungetc(char ch);
    bool getChar(char &ch);
    bool puts(const char *data, size_t &bytesWritten);
    bool gets(char *buffer, size_t len, size_t &bytesRead);
    bool setPosition(int64_t location, int64_t &position);
    bool seek(int64_t offset, int direction, int64_t &position);
    bool getPosition(int64_t &position);
    bool getSize(int64_t &size);
    bool getSize(const char *name, int64_t &size);
    bool getTimeStamp(char *&time);
    bool getTimeStamp(const char *name, char *&time);
    bool putLine(const char *buffer, size_t len, size_t &bytesWritten);
    bool hasData();
    bool countLines(int64_t &count);
    bool countLines(int64_t start, int64_t end, int64_t &lastLine, int64_t &count);
    bool nextLine(size_t &bytesRead);
    bool seekForwardLines(int64_t startPosition, int64_t &lineCount, int64_t &endPosition);
    inline bool isTransient() { return transient; }
    inline bool isDevice() { return device; }
    inline bool isReadable() { return readable; }
    inline bool isWriteable() { return writeable; }
    inline bool isOpen() { return fileHandle != -1; }

    inline bool error() { return errInfo != 0; }
    inline int  errorInfo() { return errInfo; }
    inline void clearErrors() { errInfo = 0; }
    inline bool atEof() { return !hasBufferedInput() && eof(fileHandle) == 1; }
    inline bool hasBufferedInput() { return buffered && (bufferedInput > bufferPosition); }
    inline int  getHandle() { return fileHandle; }

protected:
    void   getStreamTypeInfo();

    int    fileHandle;      // separate file handle
    int    errInfo;         // last error info
    bool   openedHandle;    // true if we opened the handle.
    int    flags;           // open flag information
    int    mode;            // mode flags
    int    share;           // sharing mode flags
    char  *filename;        // the input file name
    bool   buffered;        // the buffering indicator
    bool   transient;       // this is a transient stream
    bool   device;          // this stream is a device.
    bool   writeable;       // stream is capable of output
    bool   readable;        // stream is capable in input
    bool   isTTY;           // a keyboard based stream.
    char  *buffer;          // our read/write buffer.
    size_t bufferSize;      // the size of the buffer
    int64_t bufferPosition; // current read/write position in buffer
    size_t bufferedInput;   // amount of data in the buffer
    bool   writeBuffered;   // false == read, true == write
    bool   append;          // opened in append mode
    int64_t filePointer;    // current file pointer location
    int    ungetchar;       // a pushed back character value
};

#endif


