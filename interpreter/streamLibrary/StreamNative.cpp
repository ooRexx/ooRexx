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
/* Stream processing (stream oriented file systems)                           */
/*                                                                            */
/******************************************************************************/

#include "oorexxapi.h"
#include "StreamNative.hpp"
#include "StreamCommandParser.h"
#include "Utilities.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

/********************************************************************************/
/*                                                                              */
/* Data area's for open routines                                                */
/*                                                                              */
/********************************************************************************/

// name of our stream object structure (automatically retrieved for us)
const char *StreamInfoProperty = "CSELF";

/*****************************************************************/
/* declares needed for command seek/position                     */
/*****************************************************************/

const int  operation_read = 0x01;
const int  operation_write = 0x02;
const int  operation_nocreate = 0x04;
const int  position_by_char = 0x04;
const int  position_by_line = 0x08;
const int  position_offset_specified = 0x10;

/*****************************************************************/
/* declares needed for command query seek/position               */
/*****************************************************************/

const int  query_read_position = 0x01;
const int  query_write_position = 0x02;
const int  query_char_position = 0x04;
const int  query_line_position = 0x08;
const int  query_system_position = 0x10;


// short hand defines for some file states
#define RDWR_CREAT  (O_RDWR | O_CREAT)
#define WR_CREAT    (O_WRONLY | O_CREAT)
#define IREAD_IWRITE (S_IREAD | S_IWRITE)


/**
 * Token parsing routine for record length parsing.
 *
 * @param ttsp      The token action definition associated with this parse.
 * @param token     The current token.
 * @param userparms An opaque user parameter info.
 *
 * @return 0 for success, 1 for any failure.
 */
int reclength_token(TokenDefinition* ttsp, StreamToken &tokenizer, void *userparms)
{
                                        /* get the next token in TokenString */
    if (tokenizer.nextToken())
    {
        int offset = 0;

        // must be convertable
        if (!tokenizer.toNumber(offset))
        {
            return 1;   // non numeric token, error
        }

        *((size_t *)userparms) = offset;
        return 0;
    }

    // we default the record length, so push this back into the stream
    tokenizer.previousToken();
    return 0;
}

/**
 * Token parsing routine for position offsets.
 *
 * @param ttsp      The token action definition associated with this parse.
 * @param token     The current token.
 * @param userparms An opaque user parameter info.
 *
 * @return 0 for success, 1 for any failure.
 */
int position_offset(TokenDefinition* ttsp, StreamToken &tokenizer, void *userparms)
{
                                       /* get the next token in TokenString */
   if (tokenizer.nextToken())
   {
       int64_t offset = 0;

       // must be convertable
       if (!tokenizer.toNumber(offset))
       {
           return 1;   // non numeric token, error
       }

       *((int64_t *)userparms) = offset;
       return 0;
   }
                                       /* no next token - position will     */
                                       /* raise syntax                      */
   return 1;
}

/**
 * Token parsing routine for unknown tokens.  This just forces a parsing error.
 *
 * @param ttsp      The token action definition associated with this parse.
 * @param token     The current token.
 * @param userparms An opaque user parameter info.
 *
 * @return 0 for success, 1 for any failure.
 */
int unknown_tr(TokenDefinition* ttsp, StreamToken &tokenizer, void *userparms)
{
   return 1;
}

/**
 * Get the buffer attached to this stream.  If the existing
 * buffer is large enough for the requested usage, then use
 * it.  Otherwise, allocate a larger one.
 *
 * @param length Required buffer length
 *
 * @return Pointer to the buffer area.
 */
char *StreamInfo::allocateBuffer(size_t length)
{
    // if we already have a sufficiently large buffer, just use it.
    if (bufferAddress != NULL)
    {
        if (bufferLength >= length)
        {
            return bufferAddress; /* return the existing one           */
        }
        bufferAddress = (char *)realloc(bufferAddress, length);
    }
    else
    {
        // make sure we get at least the minimum
        if (length < DefaultBufferSize)
        {
            length = DefaultBufferSize;
        }

        bufferAddress = (char *)malloc(length);
    }
    bufferLength = length;
    if (bufferAddress == NULL)
    {
        raiseException(Rexx_Error_System_service);
    }
    // and return the new buffer address
    return bufferAddress;
}

/**
 * Get a buffer of at least the default size, returning a
 * to the buffer and the current size.
 *
 * @param length The returned buffer length.
 *
 * @return The pointer to the allocated buffer.
 */
char *StreamInfo::getDefaultBuffer(size_t &length)
{
    // make sure we have at least a minimum size buffer allocated,
    // then return the pointer and length
    allocateBuffer(DefaultBufferSize);
    length = bufferLength;
    return bufferAddress;
}


/**
 * Extend the current I/O buffer, keeping any data in the
 * buffer intact.
 *
 * @param length The length of the extended buffer.
 *
 * @return Pointer to the reallocated buffer.
 */
char *StreamInfo::extendBuffer(size_t &length)
{
    // We need more room for reading...extend this by another allocation unit,
    // keeping the data in the new buffer.
    allocateBuffer(bufferLength + DefaultBufferSize);
    length = bufferLength;
    return bufferAddress;
}


/**
 * Release the buffer, if any, attached to this stream object.
 */
void StreamInfo::freeBuffer()
{
    // Free file buffer so it can be collected next time garbage collection
    // is invoked.
    if (bufferAddress != NULL)
    {
        free(bufferAddress);
        bufferAddress = NULL;
        bufferLength = 0;
    }
}

/**
 * Open the stream in the specified mode.
 *
 * @param openFlags  Open flags as defined by the _sopen() library function
 * @param openMode   Mode flags as defined by the _sopen() function.
 * @param sharedFlag Sharing flags as defined by _sopen()
 *
 * @return true if the file is opened successfully, false for any failures.
 */
bool StreamInfo::open(int openFlags, int openMode, int sharedFlag)
{
     return fileInfo.open(qualified_name, openFlags, openMode, sharedFlag);
}

/**
 * Retrieve the size of the stream, if available.
 *
 * @return The 64-bit stream size of the target stream.
 */
int64_t StreamInfo::size()
{
    int64_t streamSize;
    fileInfo.getSize(streamSize);
    return streamSize;
}

/**
 * Raise a stream error condition, including raising a NOTREADY
 * condition.
 */
void StreamInfo::notreadyError()
{
    notreadyError(fileInfo.errorInfo(), defaultResult);
}

/**
 * Raise a stream error condition, including raising a NOTREADY
 * condition.
 *
 * @param error_code The Rexx error condition to raise.
 * @param result     A NOTREADY condition result object.
 */
void StreamInfo::notreadyError(int error_code, RexxObjectPtr result)
{
    // if we don't have a result specified, use the default one.
    if (result == NULLOBJECT)
    {
        result = defaultResult;
    }
    state = StreamError;
    fileInfo.clearErrors();              // clear any errors if the stream is open
    // raise this as a notready condition
    context->RaiseCondition("NOTREADY", context->String(stream_name), context->ArrayOfOne(self), result);
    // throw the stream object as an exception to unwind
    throw this;
}

/**
 * Raise an exception for the stream code.
 *
 * @param err     The raised error code.
 */
void StreamInfo::raiseException(int err)
{
    context->RaiseException0(err);
    // and throw a C++ exception to go back to base camp.
    throw err;
}

/**
 * Raise an exception for the stream code.
 *
 * @param err     The raised error code.
 * @param sub1    First error substitution value.
 */
void StreamInfo::raiseException(int err, RexxObjectPtr sub1)
{
    context->RaiseException1(err, sub1);
    // and throw a C++ exception to go back to base camp.
    throw err;
}

/**
 * Raise an exception for the stream code.
 *
 * @param err     The raised error code.
 * @param sub1    First error substitution value.
 * @param sub2    Second error substitution value.
 */
void StreamInfo::raiseException(int err, RexxObjectPtr sub1, RexxObjectPtr sub2)
{
    context->RaiseException2(err, sub1, sub2);
    // and throw a C++ exception to go back to base camp.
    throw err;
}

/**
 * Process an EOF condition for a stream.
 *
 * @param result  A result object returned with the NotReady condition.
 */
void StreamInfo::eof()
{
    /* place this in an eof state        */
    state = StreamEof;
    /* raise this as a notready condition*/
    context->RaiseCondition("NOTREADY", context->String(stream_name), context->ArrayOfOne(self), defaultResult);

    // if a result object was given, the caller's not expecting control back, so
    // throw an exception to unwind.
    throw this;
}

/**
 * Raise the appropriate not ready condition, checking first for an eof
 * condition.
 *
 * @param result     A result object to be passed with the Notready condition.
 */
void StreamInfo::checkEof()
{
      // if this is an eof condition, raise the eof not ready
    if (fileInfo.atEof())
    {
        eof();
    }
    else
    {
        // must be an error, so raise the error not ready using the file error
        // information
        notreadyError();
    }
}


/**
 * Get the type of the stream in question.
 *
 * @param binary  Indicates whether we believe this to be a binary stream,
 *                or not.
 */
void StreamInfo::checkStreamType()
{
    // reset the current type flags
    transient = false;
    // see if the system believes this is transient.
    if (!fileInfo.isTransient())
    {
        // non-transient, now process this as binary or text based.
        if (record_based)
        {
            // not given as a binary record length?
            if (!binaryRecordLength)
            {
                // one stream, one record, up to the record size restriction
                binaryRecordLength = (size_t)size();
                if (binaryRecordLength == 0)
                {
                    // raise an exception for this
                    raiseException(Rexx_Error_Incorrect_method);
                }
            }
        }
    }
    else   // a transient stream
    {
        transient = true;
        // for transient binary streams, if a record length was not provided,
        // we default to 1.
        if (record_based)
        {
            if (binaryRecordLength == 0)
            {
                binaryRecordLength = 1;
            }
        }
    }
}

/**
 * Close stream, performing any data flushes that might be
 * required.
 *
 * This raises a NOTREADY condition if any errors occur on the
 * close.
 */
void StreamInfo::close()
{
    // do the stream close
    bool closed = fileInfo.close();
    // free our data buffer
    freeBuffer();
    // and raise a NOTREADY condition if anything went amiss
    if (!closed)
    {
        defaultResult = context->WholeNumberToObject(fileInfo.errorInfo());
        notreadyError();
    }
    // no longer open for business
    isopen = false;
    state = StreamUnknown;
}

/**
 * Open a standard stream, using the provided options string.
 *
 * @param options Open parameters, in character string form.
 *
 * @return The open condition string.
 */
const char *StreamInfo::openStd(const char *options)
{
    // first check for the standard io streams
   if (!Utilities::strCaselessCompare(stream_name, "STDIN") ||
       !Utilities::strCaselessCompare(stream_name,"STDIN:"))
   {
       // indicate this is stdin
       fileInfo.setStdIn();
       // this is a read only file
       read_only = 1;
   }

   else if (!Utilities::strCaselessCompare(stream_name,"STDOUT") ||
            !Utilities::strCaselessCompare(stream_name,"STDOUT:"))
   {
       // indicate this is stdout
       fileInfo.setStdOut();
       // stdout can only be appended to.
       append = 1;
   }
   else                                /* must be standard error            */
   {
       // indicate this is stderr
       fileInfo.setStdErr();
       // stderr can only be appended to.
       append = 1;
   }

   // check to see if buffering is allowed.
   if (options != NULL && !Utilities::strCaselessCompare(options, "NOBUFFER"))
   {
       nobuffer = 1;
   }
   else
   {
       nobuffer = 0;  /* buffering is used                 */
   }

   // the resolved name is the same as the input name.
   strcpy(qualified_name, stream_name);
   // we're open, and ready
   isopen = true;

   state = StreamReady;

   // and also record the transient nature of this
   transient = fileInfo.isTransient();

   // don't buffer if we've explicitly requested no buffering.
   if (nobuffer)
   {
       // we do not buffer buffer file
       fileInfo.setBuffering(false, 0);
   }
   else
   {
       // we buffer file
       fileInfo.setBuffering(true, 0);
   }
   // this was successful.
   return "READY:";
}

/**
 * Do a stream open using a supplied file handle.  This
 * open process parses all of the open parameters, setting
 * the appropriate state
 *
 * @param options The character string open optins.
 *
 * @return The stream state (READY/NOTREADY)
 */
const char *StreamInfo::handleOpen(const char *options)
{
    int oflag = 0;                        // no default options

    // reset the standard fields
    resetFields();

    /* copy into the full name from the name */
    strcpy(qualified_name,stream_name);

    // do we have options?
    if (options != NULL)
    {
    /* Action table for open parameters */
        ParseAction  OpenActionread[] = {
            ParseAction(MEB, write_only),
            ParseAction(MEB, read_write),
            ParseAction(BitOr, oflag, O_RDONLY),
            ParseAction(SetBool, read_only, true),
            ParseAction()
        };
        ParseAction OpenActionwrite[] = {
            ParseAction(MEB, read_only),
            ParseAction(MEB, read_write),
            ParseAction(BitOr, oflag, WR_CREAT),
            ParseAction(SetBool, write_only, true),
            ParseAction()
        };
        ParseAction OpenActionboth[] = {
            ParseAction(MEB, read_only),
            ParseAction(MEB, write_only),
            ParseAction(BitOr, oflag, RDWR_CREAT),
            ParseAction(SetBool, read_write, true),
            ParseAction()
        };
        ParseAction OpenActionnobuffer[] = {
            ParseAction(SetBool, nobuffer, true),
            ParseAction()
        };
        ParseAction OpenActionbinary[] = {
            ParseAction(MEB, record_based),
            ParseAction(SetBool, record_based, true),
            ParseAction()
        };
        ParseAction OpenActionreclength[] = {
            ParseAction(MIB, record_based, true),
            ParseAction(CallItem, reclength_token, &binaryRecordLength),
            ParseAction()
        };

    /* Token table for open parameters */
        TokenDefinition tts[] = {
            TokenDefinition("READ",3,     OpenActionread),
            TokenDefinition("WRITE",1,    OpenActionwrite),
            TokenDefinition("BOTH",2,     OpenActionboth),
            TokenDefinition("NOBUFFER",3, OpenActionnobuffer),
            TokenDefinition("BINARY",2,   OpenActionbinary),
            TokenDefinition("RECLENGTH",3,OpenActionreclength),
            TokenDefinition(unknown_tr)
        };
        /* call the parser to setup the input information */
        /* the input string should be upper cased         */
        if (parser(tts, options, NULL) != 0)
        {
            raiseException(Rexx_Error_Incorrect_method);
        }
    }

/********************************************************************************************/
/*          if it is a persistant stream put the write character pointer at the end         */
/*   need to check if the last character is end of file and if so write over it             */
/*   if the stream was created it will have a size of 0 but this will mess up the logic     */
/*          so set it to one                                                                */
/********************************************************************************************/

    if (!fileInfo.isTransient() && (write_only | read_write))
    {
        if (size() > 0)
        {
            // if the stream is persistent, check the end position to see
            // if there is an eof marker.  If there is, position to overwrite the
            // eof character.
            // position at the end, and set the write position
            setPosition(size(), charWritePosition);

            char   char_buffer;
            size_t bytesRead;
            // read the last character of the buffer.
            // we don't call readBuffer() for this because it
            // moves the read pointer
            if (!fileInfo.read(&char_buffer, 1, bytesRead))
            {
                notreadyError();
            }

            // if the last character is not a ctrl_z, we need to
            // step past it.
            if (ctrl_z != char_buffer)
            {
                charWritePosition++;
                /* error on Windows so we had to put in that */
                /* explicitly set the position       */
                setPosition(charWritePosition, charWritePosition);
            }
        }
        lineWritePosition = 0;
        lineWriteCharPosition = 0;
    }
    // ready to go here
    isopen = true;
    state = StreamReady;
    /* go process the stream type        */
    checkStreamType();
    return "READY:";                    /* return success                    */
}


/**
 * Reinitialize the stream fields to default values.
 */
void StreamInfo::resetFields()
{
    // initialize the stream info
    //  in case this is not the first
    // open for this stream
    strcpy(qualified_name, "\0");
    fileInfo.reset();
    stream_line_size = 0;
    binaryRecordLength = 0;
    read_only = false;
    write_only = false;
    read_write = false;
    stdstream = false;
    append = false;
    opened_as_handle = false;
    charReadPosition = 1;
    charWritePosition = 1;
    lineReadPosition = 1;
    lineWritePosition = 1;
    lineReadCharPosition = 1;
    lineWriteCharPosition = 1;
    nobuffer = false;
    last_op_was_read = true;
    transient = false;
    record_based = false;
    isopen = false;
    // NB:  defaultResult is NOT automatically cleared.  Individual stream methods might
    // set this to a default return value before an implicit open operation takes place,
    // so clearing this will munge the expected return value.
//    defaultResult = NULL;
}


/**
 * Do an implicit open of the stream...this fully parses
 * the information to sort out how the open needs to proceed.
 *
 * @param type    The type of open operation.
 */
void StreamInfo::implicitOpen(int type)
{
    // is this one of the standard streams?
    // those have their own special open process.
    if (stdstream)
    {
        openStd(NULL);
        return;
    }
    // this could be a directly provided handle
    else if (opened_as_handle)
    {
        handleOpen(NULL);
        return;
    }

    // reset everything to the default.
    resetFields();

    // get the fully qualified name
    resolveStreamName();

    // first try for read/write and open file without create if specified
    // If this is an implicit open, try to open for shared readwrite, otherwise
    // we'll break the stream BIFs with nested calls.
    read_write = true;
    if (type == operation_nocreate)
    {
        open(O_RDWR, IREAD_IWRITE, RX_SH_DENYWR);
    }
    else
    {
        open(RDWR_CREAT, IREAD_IWRITE, RX_SH_DENYWR);
    }

    // if there was an open error and we have the info to try again - doit
    if (!fileInfo.isOpen())
    {
        // turn off the read/write flag and try opening as write only or read
        // only, depending on the type specified.
        read_write = false;
        if (type == operation_write)
        {
            // In Windows, all files are readable. Therefore S_IWRITE is
            // equivalent to S_IREAD | S_IWRITE.
            open(O_WRONLY, IREAD_IWRITE, RX_SH_DENYWR);
            write_only = true;
        }
        else
        {
            open(O_RDONLY, S_IREAD, RX_SH_DENYWR);
            read_only = true;
        }

        // if there still was an error, raise notready condition
        if (!fileInfo.isOpen())
        {
            // if no result given, format the error return
            if (defaultResult == NULLOBJECT)
            {
                char work[30];
                sprintf(work, "ERROR:%d", errno);
                defaultResult = context->NewStringFromAsciiz(work);
            }
            notreadyError();
            return;
        }
    }

    // persistent writeable stream?
    if (!fileInfo.isTransient() && !read_only)
    {
        // if the stream already exists, so we need to
        // see if there is a terminationg eof marker.
        if (size() > 0)
        {
            // position at the end, and set the write position
            setPosition(size(), charWritePosition);

            char   char_buffer;
            size_t bytesRead;
            // read the last character of the buffer.
            // we don't call readBuffer() for this because it
            // moves the read pointer
            if (!fileInfo.read(&char_buffer, 1, bytesRead))
            {
                notreadyError();
            }

            // if the last character is not a ctrl_z, we need to
            // step past it.
            if (ctrl_z != char_buffer)
            {
                charWritePosition++;
                /* error on Windows so we had to put in that */
                /* explicitly set the position       */
                setPosition(charWritePosition, charWritePosition);
            }
        }
        // set default line positioning
        lineWritePosition = 0;
        lineWriteCharPosition = 0;
    }
    // ready to go
    isopen = true;
    state = StreamReady;

    // go process the stream type
    checkStreamType();
}

/**
 * Set up the stream for reading.
 */
void StreamInfo::readSetup()
{
    // make sure we're open
    if (!isopen)
    {
        implicitOpen(operation_nocreate);
    }

    // reset to ready state until something goes bad.
    state = StreamReady;

    if (!fileInfo.isTransient())
    {
        // get the current stream position
        int64_t tell_position;
        fileInfo.getPosition(tell_position);
                                            /* at the correct position?          */
        if (tell_position != -1 && (charReadPosition - 1) != tell_position)
        {
                                            /* do a seek to charReadPosition   */
            setPosition(charReadPosition, charReadPosition);
        }
    }
}

/**
 * Set up the stream for a write operation.
 */
void StreamInfo::writeSetup()
{
    // make sure we are properly opened
    if (!isopen)
    {
        implicitOpen(operation_write);
    }
    /* do the open                       */
    /* reset to a ready state            */
    state = StreamReady;
    /* get the current stream position   */
    int64_t tell_position;
    fileInfo.getPosition(tell_position);
    /* at the correct position?          */
    if (tell_position != -1 && (charWritePosition - 1) != tell_position)
    {
        // not opened for append?
        if (!append)
        {
            /* set stream back to write position */
            setPosition(charWritePosition, charWritePosition);
        }
    }
}


/**
 * Read a line from the stream
 *
 * @param buffer  The data to write to the stream
 * @param length  The length of the data buffer.
 * @param update_position
 *                determines whether the read will also update the write position.
 *
 * @return A string object representing the line.
 */
RexxStringObject StreamInfo::readLine(char *buffer, size_t length, bool update_position)
{
    size_t bytesRead;

    if (!fileInfo.read(buffer, length, bytesRead))
    {
        checkEof();
    }

    if (bytesRead == 0)                 /* work ok?                          */
    {
        // must be an eof condition
        eof();
    }
    else
    {
        /* create a result string            */
        RexxStringObject string = context->NewString(buffer, bytesRead);
        if (update_position)              /* need to move read position?       */
        {
            /* update the read position          */
            charReadPosition += bytesRead;
        }
        if (bytesRead != length)             /* not get it all?                   */
        {
            defaultResult = string;
            eof();
        }
        return string;                      /* return the string                 */
        /* go raise a notready condition     */
    }
    return context->NullString();       /* return the string                 */
}


/**
 * Convert a specified stream name into it's fully qualified
 * name.
 */
void StreamInfo::resolveStreamName()
{
    if (strlen(qualified_name) == 0)
    {
        SysFileSystem::qualifyStreamName(stream_name, qualified_name, sizeof(qualified_name));
    }
}

/**
 * Write a buffer of data to the stream, raising an notready
 * condition if it fails.
 *
 * @param data   Pointer to the first byte of data
 * @param length length of the data buffer
 * @param bytesWritten
 *               Actual number of bytes written to the stream.
 */
void StreamInfo::writeBuffer(const char *data, size_t length, size_t &bytesWritten)
{
    if (!fileInfo.write(data, length, bytesWritten))
    {
        notreadyError();
    }
    // make sure the current write position is updated after the write.
    if (!fileInfo.getPosition(charWritePosition))
    {
        notreadyError();
    }
    // make sure we keep this origin 1
    charWritePosition++;
}

/**
 * Write a terminated line of data to the stream, raising an notready
 * condition if it fails.
 *
 * @param data   Pointer to the first byte of data
 * @param length length of the data buffer
 * @param bytesWritten
 *               Actual number of bytes written to the stream.
 */
void StreamInfo::writeLine(const char *data, size_t length, size_t &bytesWritten)
{
    if (!fileInfo.putLine(data, length, bytesWritten))
    {
        notreadyError();
    }

    // for non-transient streams, update the output position
    if (!transient)
    {
        // make sure the current write position is updated after the write.
        if (!fileInfo.getPosition(charWritePosition))
        {
            notreadyError();
        }
        // make sure we keep this origin 1
        charWritePosition++;
    }
}

/**
 * Read a buffer of data from the current position for the
 * given length.  This also updates our character input
 * position information.
 *
 * @param data      The location to place the data.
 * @param length    The length to read.
 * @param bytesRead The number of bytes actually read.
 */
void StreamInfo::readBuffer(char *data, size_t length, size_t &bytesRead)
{
    if (!fileInfo.read(data, length, bytesRead))
    {
        notreadyError();
    }
    // we track the character read position whenever we do a read, so
    // update the position for the actual number we've advanced.
    charReadPosition += bytesRead;
}


/**
 * Write out the remainder of an output line for a record oriented I/O operation.
 */
void StreamInfo::completeLine(size_t writeLength)
{
    // write this out in chunks
    char buffer[256];
    memset(buffer, ' ', sizeof(buffer)); /* fill buffer with blanks           */

    while (writeLength > 0)
    {
        size_t bytesWritten;
        writeBuffer(buffer, writeLength < sizeof(buffer) ? writeLength : sizeof(buffer), bytesWritten);
        writeLength -= bytesWritten;
    }
}

/**
 * Write out a fixed record line, padding with blanks if the
 * line is not of the correct size.
 *
 * @param data    The data to write.
 * @param length  length of the buffered data.
 *
 * @return The line residual count.
 */
void StreamInfo::writeFixedLine(const char *data, size_t length)
{
    /* calculate the length needed       */
    size_t write_length = binaryRecordLength - (size_t)((charWritePosition % binaryRecordLength) - 1);
    // make sure we don't go over the length of the record.
    if (length > write_length)
    {
        length = write_length;
    }
    // get the padding amount
    size_t padding = write_length - length;

    // write the line, then complete with blanks up to the padding length.
    writeBuffer(data, length, length);
    completeLine(padding);
}

/**
 * Move the stream position, with error checking.
 *
 * @param position The target position.
 * @param newPosition
 *                 The updated final position of the move.
 */
void StreamInfo::setPosition(int64_t position, int64_t &newPosition)
{
    // Seek to the target position, if possible.  The request position
    // is a 1-based character number.  We need to convert this into
    // a zero-based one before moving.
    if (!fileInfo.seek(position - 1, SEEK_SET, newPosition))
    {
        // Failed, raise a not ready condition.
        checkEof();
    }
    // convert the target position back to 1-based.
    newPosition++;
}

/**
 * Move the stream position, with error checking.
 *
 * @param position The target position.
 * @param newPosition
 *                 The updated final position of the move.
 */
void StreamInfo::setPosition(int64_t offset, int style, int64_t &newPosition)
{
    // if doing absolute positioning, then we need to fudge the offset
    if (style == SEEK_SET)
    {
        offset--;
    }
    // Seek to the target position, if possible.  The request position
    // is a 1-based character number.  We need to convert this into
    // a zero-based one before moving.
    if (!fileInfo.seek(offset, style, newPosition))
    {
        // Failed, raise a not ready condition.
        checkEof();
    }
    // convert the target position back to 1-based.
    newPosition++;
}

/**
 * Sets the current read position for the stream.  This
 * updates charReadPosition to point to the target location.
 *
 * @param position The target character position (Rexx coordinates, which
 *                 means 1 based rather than the native zero-based).
 */
void StreamInfo::setReadPosition(int64_t position)
{
    setPosition(position, charReadPosition);
}

/**
 * Sets the current write position for the stream.  This
 * updates charWritePosition to point to the target location.
 *
 * @param position The target character position (Rexx coordinates, which
 *                 means 1 based rather than the native zero-based).
 */
void StreamInfo::setWritePosition(int64_t position)
{
    setPosition(position, charWritePosition);
}


/**
 * Set the current character read position, raising a NOTREADY
 * condition of there is a problem.
 *
 * @param position The target stream position.
 * @param result   A result object returned to the caller after raising the
 *                 condition.
 */
void StreamInfo::setCharReadPosition(int64_t position)
{
    if (transient)  /* trying to move a transient stream?*/
    {
        raiseException(Rexx_Error_Incorrect_method_stream_type);
    }

    if (position < 1)                  /* too small?                        */
    {
        raiseException(Rexx_Error_Incorrect_method_positive, context->WholeNumberToObject(1), context->Int64ToObject(position));
    }
                                       /* make sure we're within the bounds */
    if (size() >= position)
    {
        // try to move to the new position, raising the appropriate NOTREADY
        // if this is a failure.
        setReadPosition(position);
    }
    else
    {
        // I can't do that Dave...raise an eof NOTREADY.
        eof();
    }
}

/**
 * Set the line read position.
 *
 * @param position The target position.
 */
void StreamInfo::setLineReadPosition(int64_t position)
{
    if (transient)  /* trying to move a transient stream?*/
    {
        raiseException(Rexx_Error_Incorrect_method_stream_type);
    }

    if (position < 1)                  /* too small?                        */
    {
        raiseException(Rexx_Error_Incorrect_method_positive, context->WholeNumberToObject(1), context->Int64ToObject(position));
    }

    // go set the new locations information.
    setLinePosition(position, lineReadPosition, lineReadCharPosition);
    // and go set our read position appropriately
    setReadPosition(lineReadCharPosition);
}

/**
 * Set the char write position.
 *
 * @param position The target position.
 */
void StreamInfo::setCharWritePosition(int64_t position)
{
    if (transient)  /* trying to move a transient stream?*/
    {
        raiseException(Rexx_Error_Incorrect_method_stream_type);
    }
    if (position < 1)                  /* too small?                        */
    {
        raiseException(Rexx_Error_Incorrect_method_positive, context->WholeNumberToObject(1), context->Int64ToObject(position));
    }
    // go move to this position
    setWritePosition(position);
}

/**
 * Set the line write position.
 *
 * @param position The target position.
 * @param result   A result value to be used when raising a condition.
 */
void StreamInfo::setLineWritePosition(int64_t position)
{
    if (transient)  /* trying to move a transient stream?*/
    {
        /* this is an error                  */
        raiseException(Rexx_Error_Incorrect_method_stream_type);

    }
    if (position < 1)                  /* too small?                        */
    {
        /* report an error also              */
        raiseException(Rexx_Error_Incorrect_method_positive, context->WholeNumberToObject(1), context->Int64ToObject(position));

    }

    // go set the new locations information.
    setLinePosition(position, lineWritePosition, lineWriteCharPosition);
    // and go set our read position appropriately
    setWritePosition(lineWriteCharPosition);
}


/**
 * Read in a variable length line, searching for the eol marker
 * for the line.
 *
 * @return The read line.
 */
RexxStringObject StreamInfo::readVariableLine()
{
    // allocate a buffer for this line.  We get a pretty good size one, which will
    // most likely be sufficient for most file lines.
    size_t bufferSize;
    char  *buffer = getDefaultBuffer(bufferSize);
    size_t currentLength = 0;

    // now loop until get an entire line read in.
    for (;;)
    {
        char *readPosition = buffer + currentLength;
        size_t bytesRead = 0;
        if (!fileInfo.gets(readPosition, bufferSize - currentLength, bytesRead))
        {
            checkEof();
        }
        // update the size of the line now
        currentLength += bytesRead;

        // Check for new line character first.  If we are at eof and the last
        // line ended in a new line, we don't want the \n in the returned
        // string.

        // If we have a new line character in the last position, we have
        // a line.  The gets() function has translated crlf sequences into
        // single lf characters.
        if (buffer[currentLength - 1] == '\n')
        {
            lineReadIncrement();
            return context->NewString(buffer, currentLength - 1);
        }

        // No new line but we hit end of file reading this?  This will be the
        // entire line then.
        if (fileInfo.atEof())
        {
            lineReadIncrement();
            return context->NewString(buffer, currentLength);
        }
        buffer = extendBuffer(bufferSize);
    }
}


/**
 * Read in a variable length line, searching for the eol marker
 * for the line.
 *
 * @return The read line.
 */
void StreamInfo::appendVariableLine(RexxArrayObject result)
{
    // allocate a buffer for this line.  We get a pretty good size one, which will
    // most likely be sufficient for most file lines.
    size_t bufferSize;
    char  *buffer = getDefaultBuffer(bufferSize);
    size_t currentLength = 0;

    // now loop until get an entire line read in.
    for (;;)
    {
        char *readPosition = buffer + currentLength;
        size_t bytesRead = 0;
        if (!fileInfo.gets(readPosition, bufferSize - currentLength, bytesRead))
        {
            checkEof();
        }

        // Check for new line character first.  If we are at eof and the last
        // line ended in a new line, we don't want the \n in the returned
        // string.

        // If we have a new line character in the last position, we have
        // a line.  The gets() function has translated crlf sequences into
        // single lf characters.
        if (buffer[bytesRead - 1] == '\n')
        {
            lineReadIncrement();
            context->ArrayAppendString(result, buffer, currentLength + bytesRead - 1);
            return;
        }

        // No new line but we hit end of file reading this?  This will be the
        // entire line then.
        if (fileInfo.atEof())
        {
            lineReadIncrement();
            context->ArrayAppendString(result, buffer, currentLength + bytesRead);
        }
        currentLength += bytesRead;
        buffer = extendBuffer(bufferSize);
    }
}

/**
 * Increments the read positions, including the line-orientated positions, after
 * a single line has been read. Assumes one line has actually been read.
 */
void StreamInfo::lineReadIncrement()
{
    // transient streams don't have moveable positions
    if (transient)
    {
        return;
    }

    if ( !fileInfo.getPosition(charReadPosition) )
    {
        notreadyError();
    }
    // Keep this 1-based.
    charReadPosition++;

    lineReadPosition++;
    lineReadCharPosition = charReadPosition;
    last_op_was_read = true;
}

/**
 * Reset all line-oriented position information after an
 * operation that will invalidate the values (for example, a
 * charin() or charout() operation).
 */
void StreamInfo::resetLinePositions()
{
    // reset all cached line information after an invalidating operation.
    lineReadCharPosition = 0;
    lineReadPosition = 0;
    stream_line_size = 0;
}

/**
 * Perform a charin() operation on the stream.
 *
 * @param setPosition
 *                 Indicates whether it is necessary to move the read pointer
 *                 before reading.
 * @param position New target position.
 * @param read_length
 *                 Length to read.
 *
 * @return A string object containing the read characters.
 */
RexxStringObject StreamInfo::charin(bool _setPosition, int64_t position, size_t read_length)
{
    readSetup();                        /* do needed setup                   */
    // given a position?...go set it.
    if (_setPosition)
    {
       setCharReadPosition(position);
    }
    // reading nothing (silly, but allowed)
    if (read_length == 0)
    {
        return context->NullString();
    }

    // a buffer string allows us to read the data into an actual string object
    // without having to first read it into a separate buffer.  Since charin()
    // is frequently used to read in entire files at one shot, this can be a
    // fairly significant savings.
    RexxBufferStringObject result = context->NewBufferString(read_length);
    char *buffer = (char *)context->BufferStringData(result);

    // do the actual read
    size_t bytesRead;
    readBuffer(buffer, read_length, bytesRead);

    // invalidate all of the line positioning info
    resetLinePositions();

    // now convert our buffered string into a real string object and return it.
    return context->FinishBufferString(result, bytesRead);
}

/********************************************************************************************/
/* stream_charin                                                                            */
/********************************************************************************************/
RexxMethod3(RexxStringObject, stream_charin, CSELF, streamPtr, OPTIONAL_int64_t, position, OPTIONAL_size_t, read_length)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    try
    {
        return stream_info->charin(argumentExists(2), position, argumentOmitted(3) ? 1 : read_length);
    }
    catch (StreamInfo *)
    {
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }

    // give default return in case there is an error
    return context->NullString();
}

/**
 * Write character data to the stream.
 *
 * @param data     The string object data we're writing.
 * @param setPosition
 *                 An order to reset the position information.
 * @param position The new write position, if specified.
 *
 * @return The residual count on the write.
 */
size_t StreamInfo::charout(RexxStringObject data, bool _setPosition, int64_t position)
{
    // no data given?  This is really a close operation.
    if (data == NULLOBJECT)
    {
        // do the setup operations
        writeSetup();
        // if no position was specified, close this out
        if (!_setPosition)
        {
            close();
        }
        else
        {
            setCharWritePosition(position);
        }
        // no data, no residual!
        return 0;
    }

    // get the string pointer and length info
    size_t length = context->StringLength(data);
    const char *stringData = context->StringData(data);
    // errors from here return the residual count, so set up the default
    // result based on the string size.
    defaultResult = context->WholeNumberToObject(length);
    // and prepare for the write
    writeSetup();
    // set the output position to the new location, if given.
    if (_setPosition)
    {
        setCharWritePosition(position);
    }
    // only write if this is not a null string
    if (length > 0)
    {
        // now write everything out
        size_t bytesWritten;
        writeBuffer(stringData, length, bytesWritten);
        // unable to write for some reason?
        if (bytesWritten != length)
        {
            defaultResult = context->WholeNumberToObject(length - bytesWritten);
            notreadyError();
        }
    }
    // reset any line positioning information.
    resetLinePositions();
    // all written...life is good.
    return 0;
}


RexxMethod3(size_t, stream_charout, CSELF, streamPtr, OPTIONAL_RexxStringObject, data, OPTIONAL_int64_t, position)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->False());

    try
    {
        return stream_info->charout(data, argumentExists(3), position);
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }
    catch (StreamInfo *)
    {
    }
    return 0;     // return 0 for all exceptions...the result value has already been set.
}

/**
 * Perform a linein() operation on the stream.
 *
 * @param setPosition
 *                 Indicates whether it is necessary to move the read pointer
 *                 before reading.
 * @param position New target position.
 * @param count    count of lines to read.
 *
 * @return A string object containing the read characters.
 */
RexxStringObject StreamInfo::linein(bool _setPosition, int64_t position, size_t count)
{
    if (count != 1 && count != 0)       /* count out of range?               */
    {
        raiseException(Rexx_Error_Incorrect_method);
    }

    // do read setup
    readSetup();
    // set a position if we have one
    if (_setPosition)
    {
                                        /* set the proper position           */
        setLineReadPosition(position);
    }

    if (count == 0)                     /* nothing to read?                  */
    {
        return context->NullString();   /* just return a null string         */
    }

    // reading fixed length records?
    if (record_based)
    {
        // we need to adjust for any charin operations that might have
        // occurred within this record
        size_t read_length = binaryRecordLength -
         ((charReadPosition % (int64_t)binaryRecordLength) == 0 ? 0 :
         (size_t)(charReadPosition % (int64_t)binaryRecordLength) - 1);
        // a buffer string allows us to read the data into an actual string object
        // without having to first read it into a separate buffer.  Since charin()
        // is frequently used to read in entire files at one shot, this can be a
        // fairly significant savings.
        RexxBufferStringObject temp = context->NewBufferString(read_length);
        char *buffer = (char *)context->BufferStringData(temp);

        // do the actual read
        size_t bytesRead;
        readBuffer(buffer, count, bytesRead);

        // now convert our buffered string into a real string object and return it.
        return context->FinishBufferString(temp, bytesRead);
    }
    else
    {
        // we need to read a variable length line
        return readVariableLine();
    }
}


/********************************************************************************************/
/* stream_linein                                                                            */
/********************************************************************************************/
RexxMethod3(RexxStringObject, stream_linein, CSELF, streamPtr, OPTIONAL_int64_t, position, OPTIONAL_size_t, count)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    try
    {
        return stream_info->linein(argumentExists(2), position, argumentOmitted(3) ? 1 : count);
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }
    catch (StreamInfo *)
    {
    }

    return context->NullString();
}

/**
 * Perform a line-oriented arrayin operation on the stream
 *
 * @param setPosition
 *                 Indicates whether it is necessary to move the read pointer
 *                 before reading.
 * @param position New target position.
 * @param count    count of lines to read.
 *
 * @return A string object containing the read characters.
 */
int StreamInfo::arrayin(RexxArrayObject result)
{
    // do read setup
    readSetup();
    // reading fixed length records?
    if (record_based)
    {
        while (true)
        {
            // we need to adjust for any charin operations that might have
            // occurred within this record
            size_t read_length = binaryRecordLength -
             ((charReadPosition % (int64_t)binaryRecordLength) == 0 ? 0 :
             (size_t)(charReadPosition % (int64_t)binaryRecordLength) - 1);
            // a buffer string allows us to read the data into an actual string object
            // without having to first read it into a separate buffer.  Since charin()
            // is frequently used to read in entire files at one shot, this can be a
            // fairly significant savings.
            RexxBufferStringObject temp = context->NewBufferString(read_length);
            char *buffer = (char *)context->BufferStringData(temp);

            // do the actual read
            size_t bytesRead;
            readBuffer(buffer, read_length, bytesRead);

            // now convert our buffered string into a real string object and return it.
            context->FinishBufferString(temp, bytesRead);
            context->ArrayAppend(result, temp);
        }
    }
    else
    {
        while (true)
        {
            // we need to read a variable length line
            appendVariableLine(result);
        }
    }
    return 0;
}


/********************************************************************************************/
/* native method for doing an arrayin line operation                                        */
/********************************************************************************************/
RexxMethod2(int, stream_arrayin, CSELF, streamPtr, RexxArrayObject, result)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    try
    {
        return stream_info->arrayin(result);
    }
    // this is thrown for any exceptions
    catch (int)
    {
    }
    catch (StreamInfo *)
    {
    }

    // this will generally terminate because of a NOTREADY condition.  We've been filling
    // the array in as we go along, so the caller has the reference already, and will
    // return whatever we've managed to fill in before the notready occurred;
    return 0;
}

/**
 * Count the number lines in the stream.
 *
 * @param quick  Controls whether we just return a 1/0 indicicator that
 *               there is more data, or do an actualy count of the lines.
 *
 * @return Either a 1/0 indicator of more or the actual count of lines.
 */
int64_t StreamInfo::lines(bool quick)
{
    // if not open yet, open now, but don't create this if doesn't
    // already exist.
    if (!isopen)
    {
        implicitOpen(operation_nocreate);
    }

    // is this a non-persisent stream?
    if (fileInfo.isTransient())
    {
        // just return a success/failure indicator
        return fileInfo.hasData() ? 1 : 0;
    }
    // non-input stream?  Never have lines for those
    if (!read_only && !read_write)
    {
        return 0;
    }

    // if opened with fixed length records, we just check against the character position.
    if (record_based)      /* opened as a binary stream?        */
    {
        // get the current size
        int64_t currentSize = size();
        // already read past that point?
        if (charReadPosition > currentSize)
        {
            return 0;
        }

        // now calculate the number of lines in the stream from the size,
        // making sure we count any partial lines hanging off the end.
        int64_t lineCount = currentSize / binaryRecordLength;
        if ((currentSize % binaryRecordLength) > 0)
        {
            lineCount++;
        }

        // get the current line position.  We don't need to fudge this...since
        // this still gives us the line we're currently within.
        int64_t currentLine = (charReadPosition - 1) / binaryRecordLength;

        // and return the delta count.
        return lineCount - currentLine;
    }
    // non-binary persistent stream...these are a pain
    else
    {
        int64_t currentSize = size();

        // if our read position is in no-man's land, this is zero
        if (charReadPosition > currentSize)
        {
            return 0;
        }
        // if we're doing a quick check, we can return 1 now.
        else if (quick)
        {
            return 1;
        }

        // do we have good line size information?
        // This is pretty easy to calculate now.
        if (stream_line_size > 0 && lineReadPosition > 0)
        {
            return(stream_line_size - lineReadPosition) + 1;
        }

        // need to do an actual scan (bummer)
        readSetup();

        // Now go count.  If our position is at the beginning,
        // this reads everything.  Else, it counts from our current
        // know line read point and returns the full count based on
        // that.
        return countStreamLines(lineReadPosition, charReadPosition);
    }
}

/********************************************************************************************/
/* stream_lines                                                                             */
/********************************************************************************************/
RexxMethod2(int64_t, stream_lines, CSELF, streamPtr, OPTIONAL_CSTRING, option)
{
    bool quick = false;
    if (option != NULL)
    {
        if (toupper(*option) == 'N')
        {
            quick = true;
        }
        else if (toupper(*option) != 'C')
        {
            context->RaiseException0(Rexx_Error_Incorrect_method);
            return 0;
        }
    }

    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->WholeNumberToObject(0));

    try
    {
        return stream_info->lines(quick);
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }
    catch (StreamInfo *)
    {
    }
    return 0;
}

/**
 * Return the remaining character indicator for a stream.
 * For transient streams, this is a 1/0 value.  For persistent
 * streams, this is the remaining data left in the stream.
 *
 * @return A count of characters in the stream.
 */
int64_t StreamInfo::chars()
{
    // if not open, we go ahead and open, but do not create implicitly.
    if (!isopen)
    {
        implicitOpen(operation_nocreate);
    }

    // is this a non-persisent stream?
    if (fileInfo.isTransient())
    {
        // just return a success/failure indicator
        return fileInfo.hasData() ? 1 : 0;
    }
    // non-input stream?  Never have lines for those
    if (!read_only && !read_write)
    {
        return 0;
    }

    /* check for a negative return value and set it to 0 if neces.*/
    int64_t remainder = size() - (charReadPosition - 1);
    return remainder > 0 ? remainder : 0;
}

/********************************************************************************************/
/* stream_chars                                                                             */
/********************************************************************************************/
RexxMethod1(int64_t, stream_chars, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->WholeNumberToObject(0));

    try
    {
        return stream_info->chars();
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }
    catch (StreamInfo *)
    {
    }
    return 0;
}

/**
 * Write a line out to the stream.
 *
 * @param data     The string object to write.
 * @param setPosition
 *                 Indicates whether we've been given a line position to use
 *                 for the write.
 * @param position The provided line position.
 *
 * @return 0 if everything worked.  All failures result in notready
 *         conditions, which throw an exception.
 */
int StreamInfo::lineout(RexxStringObject data, bool _setPosition, int64_t position)
{
    // nothing to process?
    if (data == NULLOBJECT)
    {
        writeSetup();
        // if this is a binary stream, we may have a line to complete
        if (record_based)
        {
            // calculate length to write out
            size_t padding = binaryRecordLength - (size_t)((charWritePosition % binaryRecordLength) - 1);
            completeLine(padding);
        }
        // not a line repositioning?  we need to close
        if (!_setPosition)
        {
            close();
        }
        else  // setting the line position
        {
            setLineWritePosition(position);
        }
        /* set the proper position           */
        return 0;                       /* no residual                       */
    }

    // get the specifics
    const char *stringData = context->StringData(data);
    size_t length = context->StringLength(data);

    writeSetup();
    // set the position if needed
    if (_setPosition)
    {
        setLineWritePosition(position);
    }


    // binary mode write?
    if (record_based)
    {
        /* if the line_out is longer than    */
        /* reclength plus any char out data  */
        /*  raise a syntax error - invalid   */
        /* call to routine                   */
        if (binaryRecordLength < length + ((charWritePosition % binaryRecordLength) - 1))
        {
            raiseException(Rexx_Error_Incorrect_method);
        }

        // write the line out, padding if necessary
        writeFixedLine(stringData, length);
        // not ready conditions won't return here, so this is successful.
        return 0;
    }
    else
    {
        // are we keeping count of the lines?
        if (stream_line_size > 0)
        {
            // appending?  Then we know we can increase the size
            if (append || charWritePosition == size())
            {
                stream_line_size++;
            }
            else  // the counted number of lines can no longer be relied on.
            {
                stream_line_size = 0;
            }
        }
        // write the data and line terminator.
        writeLine(stringData, length, length);
        /* need to adjust line positions?    */
        if (lineWritePosition > 0)
        {
            ++lineWritePosition;
            lineWriteCharPosition = charWritePosition;
        }
        return 0;                         /* line written correctly            */
    }
}

/********************************************************************************************/
/* stream_lineout                                                                           */
/********************************************************************************************/
RexxMethod3(int, stream_lineout, CSELF, streamPtr, OPTIONAL_RexxStringObject, string, OPTIONAL_int64_t, position)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    // we give a 1 residual count for all errors
    stream_info->setContext(context, context->True());

    try
    {
        return stream_info->lineout(string, argumentExists(3), position);
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }
    catch (StreamInfo *)
    {
    }
    return 0;     // return 0 for all exceptions...the result value has already been set.
}

/**
 * Close the stream.
 *
 * @return The character string success/failure indicator.
 */
const char *StreamInfo::streamClose()
{
    // not open, just return a "" value
    if (!isopen)
    {
        state = StreamUnknown;
        return "";                        /* return empty string               */
    }

    close();                            /* go close the stream               */
    return "READY:";                    /* return the success indicator      */
}

/********************************************************************************************/
/* stream_close                                                                             */
/********************************************************************************************/
RexxMethod1(CSTRING, stream_close, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    try
    {
        return stream_info->streamClose();
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }
    catch (StreamInfo *)
    {
    }
    return 0;     // return 0 for all exceptions...the result value has already been set.
}

/**
 * Try to flush the stream, returning the appropriate error
 * state if there is a problem.
 *
 * @return "READY" if everything works.  If this fails, a notready
 *         condition is raised and an exception is thrown.
 */
const char *StreamInfo::streamFlush()
{
    // try to flush
    if (!fileInfo.flush())
    {
        char         work[30];              /* error information buffer          */
        sprintf(work, "ERROR:%d", fileInfo.errorInfo());   /* format the error return           */
                                        /* go raise a notready condition     */
        notreadyError(fileInfo.errorInfo(), context->NewStringFromAsciiz(work));
    }
    return "READY:";                    /* return success indicator          */

}

/********************************************************************************************/
/* stream_flush                                                                             */
/********************************************************************************************/
RexxMethod1(CSTRING, stream_flush, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    try
    {
        return stream_info->streamFlush();
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }
    catch (StreamInfo *)
    {
    }
    return 0;     // return 0 for all exceptions...the result value has already been set.
}

/**
 * Process explicit stream open requests, handling all of the
 * open option variations.
 *
 * @param options The specified option strings.
 *
 * @return The READY or NOTREADY strings.
 */
const char *StreamInfo::streamOpen(const char *options)
{
    int oflag = 0;                      // no default open options
    int pmode = 0;                      /* and the protection mode           */
    int shared = RX_SH_DENYRW;             /* def. open is non shared           */

    // if already open, make sure we close this
    if (isopen)
    {
        close();
    }

    // we sorted out the characteristics of this during the init.  Make
    // sure we open this the appropriate way.
    if (stdstream)        /* standard stream?                  */
    {
        return openStd(options); /* handle as a standard stream       */
    }
    else if (opened_as_handle)
    {
        /* do a handle open                  */
        return handleOpen(options);
    }


    // reset the standard fields
    resetFields();

    // if we have parameters, parse them
    if (options != NULL)
    {
    /* Action table for open parameters */
        ParseAction  OpenActionread[] = {
            ParseAction(MEB, read_write),
            ParseAction(MEB, write_only),
            ParseAction(SetBool, read_only, true),
            ParseAction(BitOr, oflag, O_RDONLY),
            ParseAction(BitOr, pmode, S_IREAD),
            ParseAction()
        };

        ParseAction OpenActionwrite[] = {
            ParseAction(MEB, read_write),
            ParseAction(MEB, read_only),
            ParseAction(SetBool, write_only, true),
            ParseAction(BitOr, oflag, WR_CREAT),
            ParseAction(BitOr, pmode, S_IWRITE),
            ParseAction()
        };
        ParseAction OpenActionboth[] = {
            ParseAction(MEB, write_only),
            ParseAction(MEB, read_only),
            ParseAction(SetBool, read_write, true),
            ParseAction(BitOr, oflag, RDWR_CREAT),
            ParseAction(BitOr, pmode, IREAD_IWRITE),
            ParseAction()
        };
        ParseAction OpenActionappend[] = {
            ParseAction(MEB, read_only),
            ParseAction(ME, oflag, O_TRUNC),
            ParseAction(SetBool, append, true),
            ParseAction(BitOr, oflag, O_APPEND),
            ParseAction()
        };
        ParseAction OpenActionreplace[] = {
            ParseAction(ME, oflag, O_APPEND),
            ParseAction(BitOr, oflag, O_TRUNC),
            ParseAction()
        };
        ParseAction OpenActionnobuffer[] = {
            ParseAction(SetBool, nobuffer, true),
            ParseAction()
        };
        ParseAction OpenActionbinary[] = {
            ParseAction(MEB, record_based, true),
            ParseAction(SetBool, record_based, true),
            ParseAction()
        };
        ParseAction OpenActionreclength[] = {
            ParseAction(MIB, record_based),
            ParseAction(CallItem, reclength_token, &binaryRecordLength),
            ParseAction()
        };

        ParseAction OpenActionshared[] = {
            ParseAction(SetItem, shared, RX_SH_DENYNO),
            ParseAction()
        };

        ParseAction OpenActionsharedread[] = {
            ParseAction(SetItem, shared, RX_SH_DENYWR),
            ParseAction()
        };

        ParseAction OpenActionsharedwrite[] = {
            ParseAction(SetItem, shared, RX_SH_DENYRD),
            ParseAction()
        };

    #ifdef STREAM_AUTOSYNC
        ParseAction OpenActionautosync[] = {
            ParseAction(BitOr, oflag, O_SYNC),
            ParseAction()
        };
    #endif

    #ifdef STREAM_SHAREDOPEN
        ParseAction OpenActionshareread[] = {
            ParseAction(MI, oflag, O_DELAY),
            ParseAction(BitOr, oflag, O_RSHARE),
            ParseAction()
        };
        ParseAction OpenActionnoshare[] = {
            ParseAction(MI, oflag, O_DELAY),
            ParseAction(BitOr, oflag, O_NSHARE),
            ParseAction()
        };
        ParseAction OpenActiondelay[] = {
            ParseAction(MI, oflag, O_RSHARE),
            ParseAction(MI, oflag, O_NSHARE),
            ParseAction(BitOr, oflag, O_DELAY),
            ParseAction()
        };
    #endif

    /* Token table for open parameters */
        TokenDefinition  tts[] = {
            TokenDefinition("READ",3,      OpenActionread),
            TokenDefinition("WRITE",1,     OpenActionwrite),
            TokenDefinition("BOTH",2,      OpenActionboth),
            TokenDefinition("APPEND",2,    OpenActionappend),
            TokenDefinition("REPLACE",3,   OpenActionreplace),
            TokenDefinition("NOBUFFER",3,  OpenActionnobuffer),
            TokenDefinition("BINARY",2,    OpenActionbinary),
            TokenDefinition("RECLENGTH",3, OpenActionreclength),
            TokenDefinition("SHARED",6,    OpenActionshared),
            TokenDefinition("SHAREREAD",6, OpenActionsharedread),
            TokenDefinition("SHAREWRITE",6,OpenActionsharedwrite),

    #ifdef STREAM_AUTOSYNC
            TokenDefinition("AUTOSYNC",2, OpenActionautosync),
    #endif

    #ifdef STREAM_SHAREDOPEN
            TokenDefinition("SHAREREAD",1,OpenActionshareread),
            TokenDefinition("NOSHARE",3,  OpenActionnoshare),
            TokenDefinition("DELAY",1,    OpenActiondelay),
    #endif
            TokenDefinition(unknown_tr)
        };

        if (parser(tts, options, NULL) != 0)
        {
            raiseException(Rexx_Error_Incorrect_method);
        }
    }
    else
    {
        // No options, set the defaults.
        read_write = true;
        append = false;
        oflag |= RDWR_CREAT;
        pmode |= IREAD_IWRITE;

        // TODO: note that the docs say the default shared mode is SHARED.  But,
        // the code on entry sets the default to not shared.  Need to either fix
        // the docs or the code.
    }


    resolveStreamName();                /* get the fully qualified name      */

                                        /* if replace and binary specified,  */
                                        /* but not reclength, give back a    */
                                        /* syntax error - don't know what to */
                                        /* do                                */
    if (record_based && (oflag & O_TRUNC) && !binaryRecordLength)
    {
        raiseException(Rexx_Error_Incorrect_method);
    }

    // If read/write/both/append not specified, the default is BOTH APPEND.
    // (According to the current doc.)
    if (!(oflag & (O_WRONLY | RDWR_CREAT )) && !read_only)
    {
        oflag |= O_RDWR | RDWR_CREAT;    /* set this up for read/write mode   */
        pmode = IREAD_IWRITE;            /* save the pmode info               */
        read_write = true;

        if (!(oflag & (O_TRUNC | O_APPEND)))
        {
            oflag |= O_APPEND;
            append = true;
        }
    }

    if (read_only)
    {                       /* read-only stream?                 */
                            /* check if the stream exists        */
        if (!SysFileSystem::fileExists(qualified_name))
        {
            char work[32];

            /* format the error return           */
            sprintf(work, "ERROR:%d", errno);
            /* go raise a notready condition     */
            notreadyError(errno, context->NewStringFromAsciiz(work));
        }
        /* and clear all of the write        */
        /* information                       */
        charWritePosition = 0;
        lineWritePosition = 0;
        lineWriteCharPosition = 0;
    }
    /* if write only specified           */
    /*      - try both first             */
    if (oflag & O_WRONLY)
    {
        /* set both flags                    */
        read_write = true;
        write_only = true;

        oflag &= ~O_WRONLY;              /* turn off the write only flag      */
        oflag |= RDWR_CREAT;             /* and turn on the read/write        */
        pmode = IREAD_IWRITE;            /* set the new pmode                 */
    }

    /* now open the stream               */
    if (!open(oflag, pmode, shared))
    {
        // if this is some sort of device, it might be output only (i.e., a
        // printer).  Try opening again write only
        if (fileInfo.isDevice())
        {
            if (!open(WR_CREAT, S_IWRITE, shared))
            {
                char work[32];

                sprintf(work, "ERROR:%d", fileInfo.errorInfo()); /* format the error return           */
                /* go raise a notready condition     */
                notreadyError(fileInfo.errorInfo(), context->NewStringFromAsciiz(work));
            }
            read_write = 0;/* turn off the read/write flag      */
            write_only = 1;/* turn on the write only flag       */
        }
        else
        {
            char work[32];
            sprintf(work, "ERROR:%d", fileInfo.errorInfo()); /* format the error return           */
            /* go raise a notready condition     */
            notreadyError(fileInfo.errorInfo(), context->NewStringFromAsciiz(work));
        }
    }

    // turn off buffering if requested.
    if (nobuffer)
    {
        fileInfo.setBuffering(false, 0);
    }

/********************************************************************************************/
/*          if it is a persistant stream put the write character pointer at the end         */
/*   need to check if the last character is end of file and if so write over it             */
/*   if the stream was created it will have a size of 0 but this will mess up the logic     */
/*          so set it to one                                                                */
/********************************************************************************************/
                                        /* persistent writeable stream?      */
    if (!fileInfo.isTransient() && (oflag & (O_WRONLY | RDWR_CREAT)))
    {
        if (size() > 0)
        {   /* existing stream?                  */
            // position at the end, and set the write position
            setPosition(size(), charWritePosition);

            char   char_buffer;
            size_t bytesRead;
            // read the last character of the buffer.
            // we don't call readBuffer() for this because it
            // moves the read pointer
            if (!fileInfo.read(&char_buffer, 1, bytesRead))
            {
                notreadyError();
            }

            // if the last character is not a ctrl_z, we need to
            // step past it.
            if (ctrl_z != char_buffer)
            {
                charWritePosition++;
                /* error on Windows so we had to put in that */
                /* explicitly set the position       */
                setPosition(charWritePosition, charWritePosition);
            }
        }
        /* set default line positioning      */
        lineWritePosition = 0;
        lineWriteCharPosition = 0;
    }
    isopen = true;     /* this is now open                  */
    /* this is now ready                 */
    state = StreamReady;
    /* go process the stream type        */
    checkStreamType();
    return "READY:";                    /* return success                    */
}


/********************************************************************************************/
/* stream_open - open a stream                                                              */
/********************************************************************************************/
RexxMethod2(CSTRING, stream_open, CSELF, streamPtr, OPTIONAL_CSTRING, options)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    try
    {
        return stream_info->streamOpen(options);
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }
    catch (StreamInfo *)
    {
    }
    return 0;     // return 0 for all exceptions...the result value has already been set.
}

/**
 * Set a stream to be opened with a specified handle.
 *
 * @param fh     The input file handle.
 */
void StreamInfo::setHandle(int fh)
{
    fileInfo.open(fh);
    opened_as_handle = 1;
}

/********************************************************************************************/
/* handle_set                                                                               */
/*           sets the handle into the stream info block                                     */
/********************************************************************************************/
RexxMethod2(int, handle_set, CSELF, streamPtr, int, fh)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    try
    {
        stream_info->setHandle(fh);
    }
    // this is thrown for any exceptions
    catch (int)
    {
    }
    catch (StreamInfo *)
    {
    }
    return 0;     // return 0 for all exceptions...the result value has already been set.
}

/********************************************************************************************/
/* std_set                                                                                  */
/*           tags this as a standard I/O stream                                             */
/********************************************************************************************/
RexxMethod1(int, std_set, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setStandard();
    return 0;
}

/**
 * Set the stream position as determined by the options.
 *
 * @param options The stream command string for positioning.
 *
 * @return The updated position.
 */
int64_t StreamInfo::streamPosition(const char *options)
{
    int style = SEEK_SET;    // default style is forward.
    bool styleSet = false;
    bool seekBack = false;
    int position_flags = 0;

    int64_t offset = -1;

    if (options != NULL)
    {             /* have parameters?                  */
    /* Action table for position parameters */
        ParseAction  Direction_From_Start[] = {
            ParseAction(MEB, styleSet),         // anything set is bad
            ParseAction(SetItem, style, SEEK_SET),
            ParseAction(SetBool, styleSet, true),
            ParseAction()
        };
        ParseAction Direction_From_End[] = {
            ParseAction(MEB, styleSet),         // anything set is bad
            ParseAction(SetItem, style, SEEK_END),
            ParseAction(SetBool, styleSet, true),
            ParseAction()
        };
        ParseAction Direction_Forward[] = {
            ParseAction(MEB, styleSet),         // anything set is bad
            ParseAction(SetItem, style, SEEK_CUR),
            ParseAction(SetBool, styleSet, true),
            ParseAction()
        };
        ParseAction Direction_Backward[] = {
            ParseAction(MEB, styleSet),         // anything set is bad
            ParseAction(SetItem, style, SEEK_CUR),
            ParseAction(SetBool, seekBack, true),
            ParseAction(SetBool, styleSet, true),
            ParseAction()
        };
        ParseAction Operation_Read[] = {
            ParseAction(ME, position_flags, operation_write),
            ParseAction(BitOr, position_flags, operation_read),
            ParseAction()
        };
        ParseAction Operation_Write[] = {
            ParseAction(ME, position_flags, operation_read),
            ParseAction(BitOr, position_flags, operation_write),
            ParseAction()
        };
        ParseAction Position_By_Char[] = {
            ParseAction(ME, position_flags, position_by_line),
            ParseAction(BitOr, position_flags, position_by_char),
            ParseAction()
        };
        ParseAction Position_By_Line[] = {
            ParseAction(ME, position_flags, position_by_char),
            ParseAction(BitOr, position_flags, position_by_line),
            ParseAction()
        };

    /* Token table for position parameters */
        TokenDefinition  tts[] = {
            TokenDefinition("=",1,     Direction_From_Start),
            TokenDefinition("<",1,     Direction_From_End),
            TokenDefinition("+",1,     Direction_Forward),
            TokenDefinition("-",1,     Direction_Backward),
            TokenDefinition("READ",1,  Operation_Read),
            TokenDefinition("WRITE",1, Operation_Write),
            TokenDefinition("CHAR",1,  Position_By_Char),
            TokenDefinition("LINE",1,  Position_By_Line),
            TokenDefinition(position_offset)
        };

                  /* call the parser to fix up         */
        if (parser(tts, options, (void *)(&offset)) != 0)
        {
            raiseException(Rexx_Error_Incorrect_method);
        }
    }

    // trying to move a transient stream?
    if (transient)
    {
        /* this is an error                  */
        raiseException(Rexx_Error_Incorrect_method_stream_type);
    }

    /* position offset must be specified */
    if (offset == -1)
    {
        raiseException(Rexx_Error_Incorrect_method_noarg, context->NewStringFromAsciiz("SEEK"), context->NewStringFromAsciiz("offset"));
    }
    /* if read or write was not specified*/
    /* check the open flags for read and */
    /* set read. check for write and set */
    /* write. if open both then set both */
    /* flags                             */
    if (!(position_flags & operation_read) && !(position_flags & operation_write))
    {
        // if this is a read only stream, only one thing we can read
        if (read_only)
        {
            position_flags |= operation_read;
        }
        /* opened write only?                */
        else if (write_only)
        {
            position_flags |= operation_write;
        }
        else
        {
            position_flags |= operation_read | operation_write;

            //TODO:  make sure the last op was recorded.
            /* set both stream pointers to last active position          */
            if (last_op_was_read)
            {
                charWritePosition = charReadPosition;
                lineWritePosition = lineReadPosition;
            }
            else
            {
                charReadPosition = charWritePosition;
                lineReadPosition = lineWritePosition;
            }
        }
    }
    /* if the write stream is being      */
    /* repositioned                      */
    if (position_flags & operation_write)
    {
        if (append)    /* opened append?                    */
        {

            notreadyError(0);             // cause a notready condition
            return 0;
        }
    }
                                          /* if moving the read position -     */
    if (position_flags & operation_read)
    {
        stream_line_size = 0;    /* reset the pseudo lines            */
    }                                   /* if char or line not specified -   */

    /* default to char                   */
    if (!(position_flags & (position_by_char | position_by_line)))
    {
        position_flags |= position_by_char;
    }

    // if this is a backward seek from the current position, we need to negate
    // the offset.
    if (seekBack)
    {
        offset = -offset;
    }

    // character positioning
    if (position_flags & position_by_char)
    {
        resetLinePositions();             /* reset all line positioning        */
        // moving the read pointer?
        if (position_flags & operation_read)
        {
            // make sure the file pointer is positioned appropriately.
            setPosition(offset, style, charReadPosition);

            // record the one-based position, and if moving both, update the
            // write position too.
            if (position_flags & operation_write)
            {
                charWritePosition = charReadPosition;
            }
            return charReadPosition;
        }
        else
        {
            // make sure the file pointer is positioned appropriately.
            setPosition(offset, style, charWritePosition);

            // We don't need to handle the
            // read position here, since the case above catches both.
            return charWritePosition;
        }
    }
    else   // line positioning
    {
        /* if positioning by line and write  */
        /* only stream, raise notready       */
        /* because we can't do reads         */
        if (!(read_write || read_only))
        {
            return 0;
        }

        // moving the read pointer?
        if (position_flags & operation_read)
        {
            // make sure the file pointer is positioned appropriately.
            setPosition(charReadPosition, charReadPosition);
            seekLinePosition(offset, style, lineReadPosition, lineReadCharPosition);

            // update the charReadPosition
            charReadPosition = lineReadCharPosition;

            if (position_flags & operation_write)
            {
                charWritePosition = charReadPosition;
                lineWriteCharPosition = lineReadCharPosition;
                lineWritePosition = lineReadPosition;
            }
            return lineReadPosition;
        }
        else
        {
            // make sure the file pointer is positioned appropriately.
            setPosition(charWritePosition, charWritePosition);
            seekLinePosition(offset, style, lineWritePosition, lineWriteCharPosition);

            return lineWritePosition;
        }
    }
}

/**
 * Get the line size for this stream.  If we're using fixed
 * length records, we can calculate this directly from the
 * size.  If it is a variable-line stream, we might have
 * already calculated the size and kept the information.  If
 * not, we're going to have go and count every line.
 *
 * @return The number of lines in the stream.
 */
int64_t StreamInfo::getLineSize()
{
    //  using fixed length records?
    if (record_based)
    {
        // this one is fairly simple.  Just get the size, and we can calculate
        // the end position from that.
        int64_t currentSize = size();
        int64_t lastLine = currentSize / binaryRecordLength;
        // if the current size doesn't have an exact fit, bump the line
        // count up one.
        if ((currentSize % binaryRecordLength) > 0)
        {
            lastLine++;
        }
        return lastLine;
    }
    else
    {
        // get a count from the beginning.  If we have full information
        // already, this will just return that.
        return countStreamLines(1, 1);
    }
}


/**
 * Perform a seek operation by line position instead of
 * character.  The seek can be relative to either the front,
 * end, or current positions.
 *
 * @param offset    The offset to move.  This can be negative for SEEK_CUR.
 * @param direction The position to seek from.  This can be SEEK_SET, SEEK_CUR,
 *                  or SEEK_END (using the stdio.h constants directly).
 * @param current_line
 *                  The current line position we're seeking from.  This can
 *                  be either the stream read or write position.  This value
 *                  is updated on completion of the seek.
 * @param current_position
 *                  The current character position associated with this
 *                  operation.  This is also updated with the seek.
 *
 * @return The new line position.
 */
int64_t StreamInfo::seekLinePosition(int64_t offset, int direction, int64_t &current_line, int64_t &current_position)
{
    int64_t newLinePosition = 0;

    switch (direction)
    {
        case SEEK_END:
        {
            // end is a little more complicated.  We need to find the record #
            // of the end position, and go from there.
            int64_t lastLine = getLineSize();
            newLinePosition = lastLine - offset;
            break;
        }

        case SEEK_SET:
        {
            // set is easy, the offset is the new line position.  Handle
            // everthing from there.
            newLinePosition = offset;
            break;
        }

        case SEEK_CUR:
        {
            // just add the offset to the current position.
            // The offset can be either positive or negative.
            newLinePosition = offset + current_line;
            break;
        }
    }

    // we can't seek past the first line.
    if (newLinePosition < 1)
    {
        newLinePosition = 1;
    }

    // now go directly and set this.
    return setLinePosition(newLinePosition, current_line, current_position);
}


/**
 * Set a line position to an explicit line number.  This takes
 * into account differences between record and variable line
 * streams.
 *
 * @param new_line The target new_line position.
 * @param current_line
 *                 The current line position we're seeking from.
 * @param current_position
 *                 The current character position we're seeking from.
 *
 * @return The new line position.
 */
int64_t StreamInfo::setLinePosition(int64_t new_line, int64_t &current_line, int64_t &current_position)
{
    if (new_line <= 1)
    {
        /* set the position to the beginning */
        current_position = 1;
        current_line = 1;
        return current_line;
    }

    if (record_based)
    {
        // calculate the character position using the record length.  +1 is
        // added because we're not origin zero for Rexx streams.
        current_position = binaryRecordLength * (new_line - 1) + 1;
        current_line = new_line;
        return current_line;
    }
    else
    {
        return seekToVariableLine(new_line, current_line, current_position);
    }
}


/********************************************************************************************/
/* stream_position                                                                          */
/********************************************************************************************/
RexxMethod2(int64_t, stream_position, CSELF, streamPtr, CSTRING, options)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->WholeNumberToObject(0));

    try
    {
        return stream_info->streamPosition(options);
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }
    catch (StreamInfo *)
    {
    }
    return 0;     // return 0 for all exceptions...the result value has already been set.
}

/**
 * Determine the count of lines from the file beginning to
 * specified stream location.
 *
 * @param current_position
 *               The target position we're trying to convert for a char
 *               position into a line position.
 *
 * @return A count indicating which logical line current_position lies
 *         within.
 */
int64_t StreamInfo::queryLinePosition(int64_t current_position)
{
    if (current_position == 0)            /* at the beginning?                 */
    {
        current_position = 1;             /* this is actually the start        */
    }

    int64_t lastLine;
    int64_t count;
    // try to count...raise notready if unable to
    if (!fileInfo.countLines(0, current_position - 1, lastLine, count))
    {
        notreadyError();
    }
    return count;   // return the line count
}

/**
 * process a method-level stream query call.
 *
 * @param options The string options that determine the requested options.
 *
 * @return The numeric position value as either a line or char position.
 */
RexxObjectPtr StreamInfo::queryStreamPosition(const char *options)
{
    int position_flags = 0;           /* clear the parseParms.position_flags          */

    // if we have options, parse them into the flag values.
    if (options != NULL)
    {
    /* Action table for query position parameters */

        ParseAction Query_System_Position[] = {
            ParseAction(ME, position_flags, query_write_position),
            ParseAction(ME, position_flags, query_read_position),
            ParseAction(BitOr, position_flags, query_system_position),
            ParseAction()
        };
        ParseAction Query_Read_Position[] = {
            ParseAction(ME, position_flags, query_write_position),
            ParseAction(ME, position_flags, query_system_position),
            ParseAction(BitOr, position_flags, query_read_position),
            ParseAction()
        };
        ParseAction Query_Write_Position[] = {
            ParseAction(ME, position_flags, query_read_position),
            ParseAction(ME, position_flags, query_system_position),
            ParseAction(BitOr, position_flags, query_write_position),
            ParseAction()
        };
        ParseAction Query_Char_Position[] = {
            ParseAction(ME, position_flags, query_line_position),
            ParseAction(BitOr, position_flags, query_char_position),
            ParseAction()
        };
        ParseAction Query_Line_Position[] = {
            ParseAction(ME, position_flags, query_char_position),
            ParseAction(BitOr, position_flags, query_line_position),
            ParseAction()
        };

    /* Token table for open parameters */
        TokenDefinition  tts[] = {
            TokenDefinition("SYS",1,   Query_System_Position),
            TokenDefinition("READ",1,  Query_Read_Position),
            TokenDefinition("WRITE",1, Query_Write_Position),
            TokenDefinition("CHAR",1,  Query_Char_Position),
            TokenDefinition("LINE",1,  Query_Line_Position),
            TokenDefinition(unknown_tr)
        };
                  /* parse the command string          */
        if (parser(tts, options, NULL) != 0)
        {
            raiseException(Rexx_Error_Incorrect_method);
        }
    }

    // the position of an unopened stream is a null string.
    if (!isopen)
    {
        return context->NullString();
    }

    // transient streams alwasy return 1
    if (transient)
    {
        return context->WholeNumberToObject(1);   // always a position one
    }
    // querying the system position?
    if (position_flags & query_system_position)
    {
        int64_t position;
        // just get the stream position, raising not ready if it fails.
        if (!fileInfo.getPosition(position))
        {
            notreadyError();
        }
        return context->Int64ToObject(position);
    }
    // no method specified?
    if (!(position_flags && (query_read_position | query_write_position)))
    {
        // is this a write-only stream?  Return that
        if (write_only)
        {
            position_flags |= query_write_position;
        }
        else  // query the read position by default
        {
            position_flags |= query_read_position;
        }
    }
        /* asking for the write position?    */
    if (position_flags & query_write_position)
    {
        /* check if line or char             */
        if (position_flags & query_line_position)
        {
            return context->Int64ToObject(getLineWritePosition());
        }
        else
        {
            // just return the character write pointer
            return context->Int64ToObject(charWritePosition);
        }
    }
    else  // read position
    {
        if (position_flags & query_line_position)
        {
            return context->Int64ToObject(getLineReadPosition());
        }
        else
        {
            // just return the character write pointer
            return context->Int64ToObject(charReadPosition);
        }
    }
}


/**
 * Calculate the line position of the stream based on the
 * position and the type.
 *
 * @return The 64-bit line position of the stream.
 */
int64_t StreamInfo::getLineReadPosition()
{
    // record-based I/O?
    if (record_based)
    {
        // calculate this using the record length
        return (charReadPosition / binaryRecordLength) + (charReadPosition % binaryRecordLength ? 1 : 0);
    }
    else
    {
        // no up-to-date line position?  Recalc this based on the character position.
        if (lineReadPosition == 0)
        {
            lineReadPosition = queryLinePosition(charReadPosition);
        }
        // set the character position
        lineReadCharPosition = charReadPosition;
        // return the position.
        return lineReadPosition;
    }
}



/**
 * Calculate the line position of the stream based on the
 * defined I/O type.
 *
 * @return Calculated 64-bit stream line position.
 */
int64_t StreamInfo::getLineWritePosition()
{
    // using fixed length records?
    if (record_based)
    {
        // the position is based on the size of the records.
        return (charWritePosition / binaryRecordLength) +
            (charWritePosition % binaryRecordLength ? 1 : 0);
    }
    // standard line-oriented text stream
    else
    {
        // if our line write position has been invalidated, we need to
        // recalculate (a real pain!).
        if (lineWritePosition == 0)
        {
            // update the position based on the current character position
            lineWritePosition = queryLinePosition(charWritePosition);
        }
        // synch up the character positioning
        lineWriteCharPosition = charWritePosition;
        // and return the position.
        return lineWritePosition;
    }
}

/********************************************************************************************/
/* stream_query_position                                                                    */
/********************************************************************************************/
RexxMethod2(RexxObjectPtr, stream_query_position, CSELF, streamPtr, OPTIONAL_CSTRING, options)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    try
    {
        return stream_info->queryStreamPosition(options);
    }
    // this is thrown for any exceptions
    catch (int)
    {
        return 0;
    }
    catch (StreamInfo *)
    {
    }
    return 0;     // return 0 for all exceptions...the result value has already been set.
}

/**
 * Move forward the specified number of lines in the file.
 *
 * @param offset Number of lines to move.
 * @param current_line
 *               Current line position in the file.
 * @param current_position
 *               The current character position to read from.
 *
 * @return The new current line position.
 */
int64_t StreamInfo::readForwardByLine(int64_t offset, int64_t &current_line, int64_t &current_position)
{
    readSetup();                      /* do additional setup               */

    // make sure we're reading from the correct position
    setPosition(current_position, current_position);

    // track how many lines are actually moved (move is decremented by seekForwardLines())
    int64_t move = offset;

    // remember to do the 1-based / 0-based conversions
    if (!fileInfo.seekForwardLines(current_position - 1, move, current_position))
    {
        // no good, raise an error
        notreadyError();
    }

    // back to 1-based
    current_position++;

    // set this according to the number of lines actually moved
    current_line += offset - move;
    // unable to read everything?  Then the current line is also the line size
    if (move != 0)
    {
        stream_line_size = current_line;
    }
    return current_line;                // return current line
}


/**
 * Seek to a specific line position
 *
 * @param offset The new target position
 * @param current_line
 *               The starting line position
 * @param current_position
 *               The starting character position.
 *
 * @return
 */
int64_t StreamInfo::seekToVariableLine(int64_t offset, int64_t &current_line, int64_t &current_position)
{
    if (current_line == offset)
    {
        return current_line;
    }
    // not possible to reach there by going forward?
    if (current_line > offset)
    {
        // then read forward from the beginning
        current_line = 1;
        current_position = 1;
    }
    return readForwardByLine(offset - current_line, current_line, current_position);
}


/**
 * Make sure we have valid line positions set.  If we've just
 * been doing character operations up to this point, our
 * line positions will not be set.  This may require reading
 * the file and counting up to the positions.
 *
 * @return
 */
int64_t StreamInfo::setLinePositions()
{
   // already have information?
   if (lineReadPosition != 0 && lineWritePosition != 0)
   {
       // just return the
       return lineReadPosition;
   }
   readSetup();                        // prepare to do reading
   // still at the beginning?
   if (charReadPosition == 1)
   {
      // set the line counts to match.
      lineReadPosition = 1;
      lineReadCharPosition = 1;
   }
   else
   {
       if (!fileInfo.countLines(0, charReadPosition - 1, lineReadCharPosition, lineReadPosition))
       {
           notreadyError();
       }
       // system positions are origin 0...Rexx ones are origin 1.
       lineReadCharPosition++;
   }
   // now try the write position
   if (charWritePosition == 1)
   {
     lineWritePosition = 1;
     lineWriteCharPosition = 1;
   }
   else
   {
       if (!fileInfo.countLines(0, charWritePosition - 1, lineWriteCharPosition, lineWritePosition))
       {
           notreadyError();
       }
       // system positions are origin 0...Rexx ones are origin 1.
       lineWriteCharPosition++;
   }
   return lineReadPosition;
}

/**
 * Return the qualified name of a stream.
 *
 * @return The character string qualified name.
 */
const char *StreamInfo::getQualifiedName()
{
    // resolve the stream name, if necesary, and return the fully qualified
    // name.
    resolveStreamName();
    return qualified_name;
}

/********************************************************************************************/
/* qualify                                                                                  */
/********************************************************************************************/
RexxMethod1(CSTRING, qualify, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    return stream_info->getQualifiedName();
}


/**
 * Check if a stream exists.  If it does, the qualified name
 * is returned.  This does not need to be an open stream for
 * this to succeed.
 *
 * @return The resolved stream name, or "" if not resolvable.
 */
const char *StreamInfo::streamExists()
{
    // opened with a provided handle?  We have no name.
    if (opened_as_handle)
    {
        return "";
    }

    // are we successfully open?  We've already resolved this then
    if (isopen)
    {
        // is this a device of some sort?  result is the original name
        if (fileInfo.isDevice())
        {
            return stream_name;
        }
        else  // return the fully qualified stream name.
        {
            return qualified_name;
        }
    }

    // make sure we have the fully resolved stream name.
    resolveStreamName();
    if (SysFileSystem::fileExists(qualified_name))
    {
        return qualified_name;
    }
    // can't find this, sorry.
    return "";
}

/********************************************************************************************/
/* query_exists                                                                             */
/********************************************************************************************/
RexxMethod1(CSTRING, query_exists, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    return stream_info->streamExists();
}

/**
 * Return the handle value for the stream.
 *
 * @return The binary handle for the stream.
 */
RexxObjectPtr StreamInfo::queryHandle()
{
    if (!isopen)       /* unopened stream?                  */
    {
        return context->NullString();
    }
    return context->Uintptr(fileInfo.getHandle());
}

/********************************************************************************************/
/* query_handle                                                                             */
/********************************************************************************************/
RexxMethod1(RexxObjectPtr, query_handle, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());
    return stream_info->queryHandle();
}


/**
 * Return the type of stream (persistent or transient).
 *
 * @return String value of the type.  Returns either "PERSISTENT",
 *         "TRANSIENT", or "UNKNOWN", if the stream is not open.
 */
const char *StreamInfo::getStreamType()
{
    if (!isopen)       /* not open?                         */
    {
        return "UNKNOWN";                 /* don't know the type               */
    }
    else if (transient)
    {
        return "TRANSIENT";               /* return the transient type         */
    }
    else
    {
        return "PERSISTENT";              /* this is a persistent stream       */
    }
}

/********************************************************************************************/
/* query_streamtype                                                                         */
/********************************************************************************************/
RexxMethod1(CSTRING, query_streamtype, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;

    return stream_info->getStreamType();
}

/**
 * Get the size of the stream, regardless of whether it is
 * open or not.
 *
 * @return The int64_t size of the stream.  Devices return a 0 size.
 */
RexxObjectPtr StreamInfo::getStreamSize()
{
    // if we're open, return the current fstat() information,
    // otherwise, we do this without a handle
    if (isopen)
    {
        int64_t streamsize;
        fileInfo.getSize(streamsize);
        return context->Int64ToObject(streamsize);
    }
    else
    {
        resolveStreamName();
        int64_t streamsize;
        if (fileInfo.getSize(qualified_name, streamsize))
        {
            return context->Int64ToObject(streamsize);
        }
        // return a null string if this doesn't exist
        return context->NullString();
    }
}

/**
 * Get the time stamp of the stream (in character form),
 * regardless of whether it is open or not.
 *
 * @return A character string time stamp for the stream.  Returns ""
 *         for streams where a time stamp is meaningless.
 */
const char *StreamInfo::getTimeStamp()
{
    char *time;
    // if we're open, return the current fstat() information,
    // otherwise, we do this without a handle
    if (isopen)
    {
        fileInfo.getTimeStamp(time);
        return time;
    }
    else
    {
        resolveStreamName();
        fileInfo.getTimeStamp(qualified_name, time);
        return time;
    }
}


/********************************************************************************************/
/* query_size                                                                               */
/********************************************************************************************/
RexxMethod1(RexxObjectPtr, query_size, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    return stream_info->getStreamSize();
}


/********************************************************************************************/
/* query_time                                                                               */
/********************************************************************************************/
RexxMethod1(CSTRING, query_time, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    return stream_info->getTimeStamp();
}

/**
 * Get the stream state as a string.  This value is a constant,
 * and does not contain any addtional information.
 *
 * @return A string token with the stream state.
 */
const char *StreamInfo::getState()
{
    switch (state)
    {        /* process the different states      */
        case StreamUnknown:                /* unknown stream status             */
            return "UNKNOWN";

        case StreamNotready:               /* both notready and an eof condition*/
        case StreamEof:                    /* return the same thing             */
            return "NOTREADY";

        case StreamError:                  /* had a stream error                */
            return "ERROR";

        case StreamReady:                  /* stream is ready to roll           */
            return "READY";
    }
    return "";
}

/********************************************************************************************/
/* stream_state -- return state of the stream                                               */
/********************************************************************************************/
RexxMethod1(CSTRING, stream_state, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    return stream_info->getState();
}


/**
 * Retrieve a description string for the stream.
 *
 * @return A string describing the stream state and error information.
 */
RexxStringObject StreamInfo::getDescription()
{
    char work[100];

    switch (state) {        /* process the different states      */

        case StreamUnknown:                /* unknown stream status             */
        return context->NewStringFromAsciiz("UNKNOWN:");
        break;

        case StreamEof:                    /* had an end-of-file condition      */
        return context->NewStringFromAsciiz("NOTREADY:EOF");
        break;

        case StreamNotready:               /* had some sort of notready         */
        {
            const char *errorString = NULL;

            int errorInfo = fileInfo.errorInfo();

            if (errorInfo != 0)
            {
                errorString = strerror(errorInfo);
            }

            if (errorString != NULL)
            {
                                                 /* format the result string          */
                sprintf(work, "NOTREADY:%d %s", errorInfo, errorString);
            }
            else
            {
                                                 /* format the result string          */
                sprintf(work, "NOTREADY:%d", errorInfo);

            }
            return context->NewStringFromAsciiz(work);
        }

        case StreamError:                  /* had a stream error                */
        {
            const char *errorString = NULL;
            int errorInfo = fileInfo.errorInfo();

            if (errorInfo != 0)
            {
                errorString = strerror(errorInfo);
            }

            if (errorString != NULL)
            {
                                                 /* format the result string          */
                sprintf(work, "ERROR:%d %s", errorInfo, errorString);
            }
            else
            {
                                                 /* format the result string          */
                sprintf(work, "ERROR:%d", errorInfo);

            }
            return context->NewStringFromAsciiz(work);
        }

        case StreamReady:                  /* stream is ready to roll           */
        return context->NewStringFromAsciiz("READY:");
        break;
    }
    return NULLOBJECT;
}

/********************************************************************************************/
/* stream_description -- return description of the stream                                   */
/********************************************************************************************/

RexxMethod1(RexxStringObject, stream_description, CSELF, streamPtr)
{
    StreamInfo *stream_info = (StreamInfo *)streamPtr;
    stream_info->setContext(context, context->NullString());

    return stream_info->getDescription();
}

/**
 * Constructor for a StreamInfo object.
 *
 * @param s         The Stream object this is attached to.
 * @param inputName The initial input name specified on the new call.
 */
StreamInfo::StreamInfo(RexxObjectPtr s, const char *inputName)
{
    self = s;

    // buffer needs to be allocated
    bufferAddress = NULL;
    bufferLength = 0;

    // initialize the default values
    resetFields();
    strncpy(stream_name, inputName, SysFileSystem::MaximumPathLength);
    // this stream is in an unknown state now.
    state = StreamUnknown;
}

/********************************************************************************************/
/* stream_init                                                                              */
/********************************************************************************************/

RexxMethod2(RexxObjectPtr, stream_init, OSELF, self, CSTRING, name)
{
    // create a new stream info member
    StreamInfo *stream_info = new StreamInfo(self, name);
    RexxPointerObject streamPtr = context->NewPointer(stream_info);
    context->SetObjectVariable("CSELF", streamPtr);

    return NULLOBJECT;
}

/**
 * Count the number of lines in the file.  If we already have
 * a pseudo line count, we can just return that.
 *
 * @param currentLinePosition
 *               The current line position we're scanning from.
 * @param currentPosition
 *               The position to start counting from.
 *
 * @return The number of lines in the file.
 */
int64_t StreamInfo::countStreamLines(int64_t currentLinePosition, int64_t currentPosition)
{
    // we already had to calculate this...just return the previous value.
    if (stream_line_size > 0)
    {
        return stream_line_size;
    }
    // go to the current position
    setPosition(currentPosition, currentPosition);

    int64_t count;
    // ask for the count from this point.
    if (!fileInfo.countLines(count))
    {
        notreadyError();
    }

    // update the position and also set the pseudo line count, since we know this now.
    stream_line_size = count + currentLinePosition - 1;
    return count;
}
