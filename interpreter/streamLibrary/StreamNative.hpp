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
/* REXX Kernel                                                                */
/*                                                                            */
/* Stream declarations and includes                                           */
/*                                                                            */
/******************************************************************************/
#ifndef StreamNative_Included
#define StreamNative_Included

#include "SysFileSystem.hpp"
#include "SysFile.hpp"

#define nbt_line_end "\n"
#define nbt_line_end_size 1

#define std_line_end "\n"
#define std_line_end_size 1

#define ctrl_z 0x1a                    /* end-of-file marker                */
#define nl     '\n'                    /* new line character                */
#define cr     '\r'                    /* carriage return character         */


/*****************************************************************************/
/* Stream structure to hold information pertinent to all streams             */
/*****************************************************************************/

class StreamInfo
{
    friend class LineBuffer;
public:
    enum
    {
        DefaultBufferSize = 512        // default read buffer size
    };

    typedef enum
    {
        StreamUnknown       = 0,       // stream is in unknown state
        StreamReady         = 1,       // stream is in ready state
        StreamNotready      = 2,       // stream is in a notready condition
        StreamEof           = 3,       // stream is in an eof condition
        StreamError         = 4,       // stream has had an error condition
    } StreamState;

    StreamInfo(RexxObjectPtr s, const char *inputName);
    inline void setContext(RexxMethodContext *c, RexxObjectPtr d)
    {
        context = c;
        defaultResult = d;
    }
    char *allocateBuffer(size_t length);
    char *getDefaultBuffer(size_t &length);
    char *extendBuffer(size_t &length);
    void  freeBuffer();
    bool  open(int openFlags, int openMode, int sharedFlag);
    int64_t size();
    void  notreadyError();
    void  notreadyError(int error_code, RexxObjectPtr result = NULL);
    void  raiseException(int err);
    void  raiseException(int err, RexxObjectPtr sub1);
    void  raiseException(int err, RexxObjectPtr sub1, RexxObjectPtr sub2);
    void  eof();
    void  checkEof();
    void  checkStreamType();
    void  close();
    const char *openStd(const char *options);
    const char *handleOpen(const char *options);
    void  resetFields();
    void  implicitOpen(int type);
    void  readSetup();
    void  writeSetup();
    RexxStringObject readLine(char *buffer, size_t length, bool update_position);
    void resolveStreamName();
    void writeBuffer(const char *data, size_t length, size_t &bytesWritten);
    void writeLine(const char *data, size_t length, size_t &bytesWritten);
    void readBuffer(char *data, size_t length, size_t &bytesRead);
    void completeLine(size_t writeLength);
    void writeFixedLine(const char *data, size_t length);
    void setPosition(int64_t position, int64_t &newPosition);
    void setPosition(int64_t offset, int style, int64_t &newPosition);
    void setReadPosition(int64_t position);
    void setWritePosition(int64_t position);
    void setCharReadPosition(int64_t position);
    void setLineReadPosition(int64_t position);
    void setCharWritePosition(int64_t position);
    void setLineWritePosition(int64_t position);
    RexxStringObject readVariableLine();
    void appendVariableLine(RexxArrayObject r);
    void lineReadIncrement();
    void resetLinePositions();
    RexxStringObject charin(bool setPosition, int64_t position, size_t read_length);
    size_t charout(RexxStringObject data, bool setPosition, int64_t position);
    RexxStringObject linein(bool setPosition, int64_t position, size_t count);
    int arrayin(RexxArrayObject r);
    int64_t lines(bool quick);
    int64_t chars();
    int lineout(RexxStringObject data, bool setPosition, int64_t position);
    const char *streamClose();
    const char *streamFlush();
    const char *streamOpen(const char *options);
    void setHandle(int fh);
    int64_t streamPosition(const char *options);
    int64_t getLineSize();
    int64_t seekLinePosition(int64_t offset, int direction, int64_t &current_line, int64_t &current_position);
    int64_t setLinePosition(int64_t new_line, int64_t &current_line, int64_t &current_position);
    int64_t queryLinePosition(int64_t current_position);
    RexxObjectPtr queryStreamPosition(const char *options);
    int64_t getLineReadPosition();
    int64_t getLineWritePosition();
    int64_t readForwardByLine(int64_t offset, int64_t &current_line, int64_t &current_position);
    int64_t seekToVariableLine(int64_t offset, int64_t& current_line, int64_t &current_position);
    int64_t setLinePositions();
    const char *getQualifiedName();
    const char *streamExists();
    int64_t queryHandle();
    const char *getStreamType();
    int64_t getStreamSize();
    const char *getTimeStamp();
    const char *getState();
    RexxStringObject getDescription();
    int64_t countStreamLines(int64_t currentLinePosition, int64_t currentPosition);
    inline void setStandard() { stdstream = true; }


protected:
   RexxMethodContext *context;         // our current execution context
   RexxObjectPtr self;                 // our real Rexx object instance
   RexxObjectPtr defaultResult;        // default result used for notready conditions.
                                       // specified stream name
   char stream_name[SysFileSystem::MaximumFileNameBuffer];
                                       // fully resolved stream name
   char qualified_name[SysFileSystem::MaximumFileNameLength];
   int64_t charReadPosition;          // current character position
   int64_t charWritePosition;         // current write position
   int64_t lineReadPosition;          // current read line number
   int64_t lineWritePosition;         // current write line number
   int64_t lineReadCharPosition;      // current line read position
   int64_t lineWriteCharPosition;     // current line write position
   int64_t stream_line_size;          // emulated stream size (lines)
   StreamState state;                 // current stream state
   size_t  binaryRecordLength;        // binary file record length

   char *bufferAddress;                // current read buffer
   size_t bufferLength;                // current read buffer size

   SysFile  fileInfo;                  // system specific file implementation

   // the various flag state settings
   bool read_only;
   bool write_only;
   bool read_write;
   bool append;
   bool nobuffer;
   bool stdstream;                     // true if a standard I/O stream
   bool last_op_was_read;              // still needed?
   bool opened_as_handle;              // given a handle directly
   bool transient;                     // non-persistent stream
   bool record_based;                  // uses fixed-length, non-terminated records
   bool isopen;                        // the stream is open
};

#endif
