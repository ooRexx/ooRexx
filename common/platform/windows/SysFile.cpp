/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                              SysFile.cpp       */
/*                                                                            */
/* Windows implementation of the SysFile class.                               */
/*                                                                            */
/******************************************************************************/

#include "SysFile.hpp"
#include <errno.h>
#include <time.h>
#include <conio.h>
#include <stdio.h>

/**
 * Default constructor for a SysFile object.
 */
SysFile::SysFile()
{
    fileHandle = -1;
    errInfo = 0;
    openedHandle = false;
    flags = 0;
    mode = 0;
    share = 0;
    filename = NULL;
    buffered = true;
    transient = false;
    device = false;
    writeable = false;
    readable = false;
    isTTY = false;
    buffer = NULL;
    bufferSize = DEFAULT_BUFFER_SIZE;
    bufferPosition = 0;
    bufferedInput = 0;
    append = true;
    filePointer = 0;
    ungetchar = -1;
    writeBuffered = false;     // no pending write operations
    fileSize = -1;             // no retrieved file size yet
}

/**
 * Opens a file.  This opens the file for both lowlevel I/O
 * and also for higher level I/O.
 *
 * @param name       Name of the stream.
 * @param openFlags  The open flags.  This are the same flags used on the _sopen()
 *                   function.
 * @param openMode   Open mode.  Same flag values as _sopen().
 * @param shareMode  The sharing mode.  Same as used by the _sopen() library
 *                   function.
 *
 * @return true if the file was opened successfully, false otherwise.
 */
bool SysFile::open(const char *name, int openFlags, int openMode, int shareMode)
{
    flags = openFlags;           // save the initial flag values
    mode = openMode;
    share = shareMode;

    // we must open this with the NOINHERIT and BINARY flags added
    fileHandle = _sopen(name, openFlags|_O_NOINHERIT|_O_BINARY, shareMode, openMode);
    if ( fileHandle == -1 )
    {
        errInfo = errno;
        return false;
    }

    // we did open this handle
    openedHandle = true;

    // save a copy of the name
    filename = strdup(name);
    ungetchar = -1;            // -1 indicates no char
    fileSize = -1;               // make sure we don't have a file size from a previous open

    // is this append mode?
    if ((flags & RX_O_APPEND) != 0)
    {
        // mark this true, and position at the end
        append = true;
        _lseeki64(fileHandle, 0, SEEK_END);
    }

    getStreamTypeInfo();
    // set the default buffer size (and allocate the buffer)
    setBuffering(true, 0);
    return true;
}


/**
 * Open a stream using a provided handle value.
 *
 * @param handle     The source stream handle.
 *
 * @return true if the file opened ok, false otherwise.
 */
bool SysFile::open(int handle)
{
    // we didn't open this.
    openedHandle = false;
    fileHandle = handle;
    ungetchar = -1;            // -1 indicates no char
    getStreamTypeInfo();

    // set the default buffer size (and allocate the buffer)
    setBuffering(true, 0);
    return true;
}

/**
 * Reset this to an unopened state.
 */
void SysFile::reset()
{
    // make sure we flush anything pending.
    flush();
    if (buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }
    fileHandle = -1;
}

/**
 * Controls buffering for this stream.
 *
 * @param buffer True or false depending on the desired buffering mode.
 */
void SysFile::setBuffering(bool buffering, size_t length)
{
    if (buffering)
    {
        buffered = true;
        if (length == 0)
        {
            length = DEFAULT_BUFFER_SIZE;
        }
        buffer = (char *)malloc(length);
        if (buffer == NULL)
        {
            buffered = false;
        }
    }
    else
    {
        buffered = false;
        if (buffer != NULL)
        {
            free(buffer);
            buffer = NULL;
        }
    }
    // reset all of the buffering controls to the defaults
    bufferPosition = 0;
    bufferedInput = 0;
    writeBuffered = false;
}


/**
 * Close the stream, and free all associated resources.
 *
 * @return true if this closed successfully, false otherwise.
 */
bool SysFile::close()
{
    // don't do anything if not opened
    if (fileHandle == -1)
    {
        return true;
    }

    // if we're buffering, make sure the buffers are flushed
    if (buffered)
    {
        flush();
    }
    // free out storage areas first
    if (filename != NULL)
    {
        free(filename);
        filename = NULL;
    }
    if (buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }
    errInfo = 0;
    // if we opened this handle, we need to close it too.
    if (openedHandle)
    {
        if (::close(fileHandle) == EOF)
        {
            // we've got an error, but this needs to be cleared
            fileHandle = -1;
            errInfo = errno;
            return false;
        }
    }

    // always clear this on a close
    fileHandle = -1;

    return true;
}

/**
 * Flush the stream buffers.
 *
 * @return True if this worked without error, false otherwise.
 */
bool SysFile::flush()
{
    if (buffered)
    {
        // do we have data in a write buffer?
        if (writeBuffered && bufferPosition > 0)
        {
            // write this out...but if it fails, we need to bail
            ssize_t written = writeData(buffer, bufferPosition);
            // did we have an error?
            if (written <= 0)
            {
                errInfo = errno;
                return false;
            }
            // update the real output position
            filePointer += written;
            // and invalidate the buffer
            bufferPosition = 0;
            bufferedInput = 0;
        }
    }
    return true;
}

/**
 * Read bytes from the stream.
 *
 * @param buf        The buffer to read into.
 * @param len        The requested number of bytes to read.
 * @param bytesRead  The actual number of bytes read.
 *
 * @return True if one or more bytes are read into buf, otherwise false.
 */
bool SysFile::read(char *buf, size_t len, size_t &bytesRead)
{
    // set bytesRead to 0 to be sure we can tell if we are returning any bytes.
    bytesRead = 0;

    // asking for nothing?  this is pretty easy
    if (len == 0)
    {
        return true;
    }

    // if we have an ungetchar, we need to grab that first
    if (ungetchar != -1)
    {
        // add this to our count
        bytesRead = 1;
        // copy the character over
        buf[0] = (char)ungetchar;
        buf++;
        len--;
        ungetchar = -1;
        // were we only looking for one character (very common in cases where
        // we've had a char pushed back)
        if (len == 0)
        {
            return true;
        }
    }

    // are we doing buffering?
    if (buffered)
    {
        // do we have pending output in the buffer?
        if (writeBuffered)
        {
            flush();
            writeBuffered = false;
            bufferPosition = 0;
            bufferedInput = 0;
        }

        while (len > 0)
        {
            // have we exhausted the buffer data?
            if (bufferPosition >= bufferedInput)
            {
                // read another chunk of data.
                int blockRead = _read(fileHandle, buffer, (unsigned int)bufferSize);
                if (blockRead <= 0)
                {
                    // not get anything?
                    if (_eof(fileHandle))
                    {
                        return bytesRead > 0 ? true : false;
                    }
                    else
                    {
                        // had an error, so raise it
                        errInfo = errno;
                        return false;
                    }
                }
                else
                {
                    // update the positions
                    filePointer += blockRead;
                    bufferedInput = blockRead;
                    bufferPosition = 0;
                }
            }

            // see how much we can copy
            size_t blocksize = (size_t)(len > bufferedInput - bufferPosition ? bufferedInput - bufferPosition : len);
            memcpy(buf, buffer + bufferPosition, blocksize);
            // and adjust all of the positions
            bufferPosition += blocksize;
            buf += blocksize;
            len -= blocksize;
            bytesRead += blocksize;
        }
    }
    else
    {
        while (len > 0)
        {
            int blockRead = _read(fileHandle, buf + bytesRead, (unsigned int)len);
            if (blockRead <= 0)
            {
                // not get anything?
                if (_eof(fileHandle))
                {
                    // could have had an ungetchar
                    return bytesRead > 0 ? true : false;
                }
                else
                {
                    // had an error, so raise it
                    errInfo = errno;
                    return false;
                }
            }
            // update the length
            len -= blockRead;
            bytesRead += blockRead;
        }
    }
    return true;
}

/**
 * Wrapper around _write to handle block size issues with
 * device streams and _write itself.
 *
 * @param data   The data to write.
 * @param length The data length.
 *
 * @return The number of bytes written, or -1 on error
 */
ssize_t SysFile::writeData(const char *data, size_t length)
{
    // anytime we write data to the stream, we invalidate our cached
    // file size because it's likely no longer valid
    fileSize = -1;

    // _write can handle chunks of INT_MAX at most because of its
    // int return (on Windows int is 32-bit even on 64-bit systems)
    // for devices, we write this in blocks of BLOCK_THRESHOLD size
    size_t blocksize = device ? BLOCK_THRESHOLD : INT_MAX;

    // Windows _write isn't expected to successfully return with less
    // bytes written than requested, but we always loop until all data
    // has been written or _write fails.
    size_t bytesWritten = 0;
    while (length > 0)
    {
        size_t segmentSize = length > blocksize ? blocksize : length;
        int justWritten = _write(fileHandle, data, (unsigned int)segmentSize);
        // write error?  bail out
        if (justWritten <= 0)
        {
            return -1;
        }
        length -= justWritten;
        bytesWritten += justWritten;
        data += justWritten;
    }
    return bytesWritten;
}


/**
 * write data to the stream
 *
 * @param data   The data buffer to write.
 * @param len    The length to write
 * @param bytesWritten
 *               The number bytes actually written (return value).
 *
 * @return true if the write succeeded, false for any errors.
 */
bool SysFile::write(const char *data, size_t len, size_t &bytesWritten)
{
    // writing zero bytes is a NOP
    if (len == 0)
    {
        return true;
    }

    // anytime we write data to the stream, we invalidate our cached
    // file size because it's likely no longer valid
    fileSize = -1;

    // are we buffering?
    if (buffered)
    {
        // using the buffer for input at the moment?
        if (!writeBuffered)
        {
            // We need to position the file write pointer to the postion of our
            // last virtual read.
            int64_t offset = filePointer - bufferedInput + bufferPosition;
            // set the absolute position
            _lseeki64(fileHandle, offset, SEEK_SET);
            bufferedInput = 0;
            bufferPosition = 0;
            // we're switching modes.
            writeBuffered = true;
        }

        // is this too large to bother copying into the buffer?
        if (len > bufferSize)
        {
            // flush an existing data from the buffer
            flush();
            // write this out directly
            ssize_t written = writeData(data, len);
            // oh, oh...got a problem
            if (written <= 0)
            {
                // save the error status and bail
                errInfo = errno;
                return false;
            }
            bytesWritten = written;
            // update the real output position
            filePointer += written;
            return true;
        }

        bytesWritten = len;
        // ok, we have can fit in the buffer, but we might need to do this
        // in chunks
        while (len > 0)
        {
            // is the buffer full?
            if (bufferPosition == bufferSize)
            {
                // flush the buffer now
                flush();
            }

            // append to the buffer
            size_t blocksize = (size_t)(len > bufferSize - bufferPosition ? bufferSize - bufferPosition : len);
            memcpy(buffer + bufferPosition, data, blocksize);
            // and adjust all of the position pointers
            bufferPosition += blocksize;
            data += blocksize;
            len -= blocksize;
        }
        return true;
    }
    else
    {
        // not a transient stream?
        if (!transient)
        {
            // opened in append mode?
            if ((flags & _O_APPEND) != 0)
            {
                // seek to the end of the file, return if there is an error
                if (_lseeki64(fileHandle, 0, SEEK_END) < 0)
                {
                    errInfo = errno;
                    return false;
                }
            }
            // write the data
            ssize_t written = writeData(data, len);
            if (written <= 0)
            {
                // return error status if there was a problem
                errInfo = errno;
                return false;
            }

            bytesWritten = written;
        }
        else
        {
            // write the data
            ssize_t written = writeData(data, len);
            if (written <= 0)
            {
                // return error status if there was a problem
                errInfo = errno;
                return false;
            }

            bytesWritten = written;
        }
    }
    return true;
}

bool SysFile::putChar(char ch)
{
    size_t len;
    return write(&ch, 1, len);
}

bool SysFile::ungetc(char ch)
{
    // make sure there's no sign extension
    ungetchar = ((int)ch) & 0xff;
    return true;
}

bool SysFile::puts(const char *data, size_t &len)
{
    return write(data, strlen(data), len);
}

/**
 * Write a line to the stream, adding the platform specific
 * line terminator.
 *
 * @param buffer Start of the line to write.
 * @param len    The length to write.
 * @param bytesWritten
 *               The actual number of bytes written, including the line
 *               terminator.
 *
 * @return A success/failure indicator.
 */
bool SysFile::putLine(const char *buffer, size_t len, size_t &bytesWritten)
{
    // this could be a null line...don't try to write zero bytes
    if (len > 0)
    {
        if (!write(buffer, len, bytesWritten))
        {
            return false;
        }
    }
    size_t termlen;
    if (puts(LINE_TERMINATOR, termlen))
    {
        bytesWritten += termlen;
        return true;
    }
    return false;
}


/**
 * Read characters from the stream until the next newline, or until the
 * buffer fills up.  If the read characters end in a carriage return +
 * newline sequence, collapse it into a single newline character.
 * Return the line including the trailing newline character (if any).
 *
 * @param buffer Start of the line to write.
 * @param bufferLen
 *               The maximum length to read.
 * @param bytesRead
 *               The actual number of bytes read, including the line
 *               terminator.
 *
 * @return A success/failure indicator.
 */
bool SysFile::gets(char *buffer, size_t bufferLen, size_t &bytesRead)
{
    size_t i;

    for (i = 0; i < bufferLen - 1; i++)
    {
        size_t len;

        // if we don't get a character break out of here.
        if (!read(buffer + i, 1, len))
        {
            break;
        }

        // special handling for carriage return characters. we need to read the
        // next character and if it is a newline, then we convert the carriage return
        // into a newline and skip ahead in the file. Otherwise, we put the character back
        // and leave the \r as data within the line.
        if (buffer[i] == '\r')
        {
            char ch;

            // we need to be able to read the character for this to work.
            if (getChar(ch))
            {
                // if this is the second character of the sequence, just replace the '\r'
                if (ch == '\n')
                {
                    buffer[i] = '\n';
                }
                // not paired with a \n, so leave the \r as part of the line.
                // and return the read character to the stream
                else
                {
                    ungetc(ch);
                }
            }
        }

        // we only look for a newline character to terminate our line (if we have a
        // paired \r\n, the code above has already collapsed this to a single \n).
        if (buffer[i] == '\n')
        {
            i++;   // step the position to give the actual number of bytes read
            break;
        }
    }

    // if there is no data read at all, this is an eof failure;
    if (i == 0)
    {
        return false;
    }

    // this is the length of the read data (including the newline, if any)
    bytesRead = i;
    // return any error state, but not EOF status.
    return !error();
}

/**
 * Count the number of lines in the stream, starting from the
 * current file pointer position.
 *
 * @param count  The returned line count.
 *
 * @return The success/failure indicator.
 */
bool SysFile::countLines(int64_t &count)
{
    int64_t counter = 0;
    size_t bytesRead;

    while (nextLine(bytesRead))
    {
        if (bytesRead == 0)
        {
            count = counter;
            return true;
        }
        counter++;
    }

    return false;
}

/**
 * Count the number of lines between two character positions.
 *
 * @param start    The starting offset.
 * @param end      The ending offset
 * @param lastLine The starting offset of the last line before the end position.
 * @param count    The returned offset
 *
 * @return The success/failure indicator
 */
bool SysFile::countLines(int64_t start, int64_t end, int64_t &lastLine, int64_t &count)
{
    // go to the target location, if possible
    if (!seek(start, SEEK_SET, start))
    {
        return false;
    }

    int64_t counter = 0;
    size_t bytesRead;

    while (nextLine(bytesRead))
    {
        lastLine = start;
        // hit an eof?  we're done counting, return
        if (bytesRead == 0)
        {
            count = counter;
            return true;
        }
        counter++;
        start += bytesRead;
        // have we reached our end point?
        if (start > end)
        {
            count = counter;
            return true;
        }
    }

    return false;
}

/**
 * Move to the beginning of the next line in the stream, returning
 * a count of the bytes moved forward.
 *
 * @param bytesRead The returned byte count for the line (including the terminators).
 *
 * @return True if this was processed ok, false for any errors.
 */
bool SysFile::nextLine(size_t &bytesRead)
{
    size_t len = 0;

    for (;;)
    {
        char ch;
        // if we don't get a character break out of here.
        if (!getChar(ch))
        {
            break;
        }
        len++;       // count this
        // found our newline character?
        if (ch == '\n')
        {
            break;
        }
    }

    // this is the length including the line terminators
    bytesRead = len;
    // return an error state, but not EOF status.
    return !error();
}

bool SysFile::seekForwardLines(int64_t startPosition, int64_t &lineCount, int64_t &endPosition)
{
    // make sure we flush any output data
    flush();

    // get a buffer for searching
    char *buffer = (char *)malloc(LINE_POSITIONING_BUFFER);
    if (buffer == NULL)
    {
        errInfo = ENOMEM;
        return false;
    }

    for (;;)
    {
        int readLength = LINE_POSITIONING_BUFFER;

        // This is likely due to hitting the end-of-file.  We'll just
        // return our current count and indicate this worked.
        if (!setPosition(startPosition, startPosition))
        {
            free(buffer);
            // set the return position and get outta here
            endPosition = startPosition;
            return true;
        }

        size_t bytesRead;
        if (!read(buffer, readLength, bytesRead))
        {
            free(buffer);
            // if we've hit an eof condition, this is the end
            if (atEof())
            {
                // set the return position and get outta here
                endPosition = startPosition;
                return true;
            }
            // read error,
            return false;
        }
        // have we hit the eof?
        if (bytesRead == 0)
        {
            free(buffer);
            // set the return position and get outta here
            endPosition = startPosition;
            return true;
        }


        size_t offset = 0;
        while (offset < bytesRead)
        {
            // we're only interested in \n character, since this will
            // mark the transition point between lines.
            if (buffer[offset] == '\n')
            {
                // reduce the line count by one.
                lineCount--;
                // reached the requested point?
                if (lineCount == 0)
                {
                    // set the return position and get outta here
                    endPosition = startPosition + offset + 1;
                    free(buffer);
                    return true;
                }
            }
            // step to the next character;
            offset++;
        }
        // move the start position...if at the end, we might not
        // get a full buffer
        startPosition += bytesRead;
    }
}


bool SysFile::setPosition(int64_t location, int64_t &position)
{
    // have a pending write?
    if (writeBuffered)
    {
        // flush any pending data
        flush();
        // reset all buffer pointers
        writeBuffered = false;
        bufferPosition = 0;
        bufferedInput = 0;
    }


    // is this location within the buffer bounds?
    if (location >= (int64_t)(filePointer - bufferedInput) && location < filePointer)
    {
        // just shift the buffer position;
        bufferPosition = (size_t)(location - (filePointer - (int64_t)bufferedInput));
        // just return the same value
        position = location;
    }
    else
    {
        // go to the absolute position
        position = _lseeki64(fileHandle, location, SEEK_SET);
        // this return the error indicator?
        if (position == -1)
        {
            errInfo = errno;
            return false;
        }

        // reset all of the buffer information and the current file pointer.
        bufferPosition = 0;
        bufferedInput = 0;
        filePointer = position;
    }
    return true;
}

bool SysFile::seek(int64_t offset, int direction, int64_t &position)
{
    // we need special processing if buffered
    if (buffered)
    {
        switch (direction)
        {
            case SEEK_SET:
                return setPosition(offset, position);

            case SEEK_CUR:
                return setPosition(filePointer - bufferedInput + bufferPosition + offset, position);

            case SEEK_END:
                int64_t fileSize;
                if (getSize(fileSize))
                {
                    return setPosition(fileSize - offset, position);
                }
                return false;

            default:
                return false;
        }
    }
    else
    {
        switch (direction)
        {
            case SEEK_SET:
                position = _lseeki64(fileHandle, offset, SEEK_SET);
                break;

            case SEEK_CUR:
                position = _lseeki64(fileHandle, offset, SEEK_CUR);
                break;

            case SEEK_END:
                position = _lseeki64(fileHandle, offset, SEEK_END);
                break;

            default:
                return false;
        }

        // this return the error indicator?
        if (position == -1)
        {
            errInfo = errno;
            return false;
        }
    }
    return true;
}

bool SysFile::getPosition(int64_t &position)
{
    // we need special processing if we have anything in the
    // buffer right now
    if (buffered && !(writeBuffered && bufferPosition == 0))
    {
        // just return the current buffer position
        position = filePointer - bufferedInput + bufferPosition;
    }
    else
    {
        // get the stream postion
        position = _telli64(fileHandle);
        if (position == -1)
        {
            return false;
        }
    }
    return true;
}

/**
 * Retrieve the size of the stream.  If the stream is open,
 * it returns the stream size.  Zero is returned if this
 * stream is not a regular file.
 *
 * @param size   The returned size value.
 *
 * @return True if the size was retrievable, false otherwise.
 */
bool SysFile::getSize(int64_t &size)
{
    // are we open?
    if (fileHandle >= 0)
    {
        // we might have pending output that might change the size
        flush();

        // do we have a current file size? If not currently good, we need to
        // get it again
        if (fileSize == -1)
        {
            // have a handle, use fstat() to get the info
            struct _stati64 fileInfo;
            if (_fstati64(fileHandle, &fileInfo) == 0)
            {
                // regular file?  return the defined size
                if (fileInfo.st_dev == 0)
                {
                    fileSize = fileInfo.st_size;
                }
                else
                {
                    fileSize = 0;
                }
            }
        }
        size = fileSize;      // use our cached value
        return true;
    }
    return false;
}

/**
 * Retrieve the size of a file from the file name.  If the
 * name is not a regular file, zero is returned.
 *
 * @param size   The returned size value.
 *
 * @return True if the size was retrievable, false otherwise.
 */
bool SysFile::getSize(const char *name, int64_t &size)
{
    // the handle is not active, use the name
    struct _stati64 fileInfo;
    if (_stati64(name, &fileInfo) == 0)
    {
        // regular file?  return the defined size
        if ((fileInfo.st_mode & _S_IFREG) != 0)
        {
            size = fileInfo.st_size;
        }
        else
        {
            size = 0;
        }
        return true;
    }
    return false;
}

/**
 * Retrieve the time stamp of the stream.  If the stream is open,
 * it returns the stream time stamp.  The null string is returned if this
 * stream is not a regular file.
 *
 * @param time   The returned time stamp, e. g. Wed Jan 02 02:03:55 1980\n\0
 *
 * @return True if the time stamp was retrievable, false otherwise.
 */
bool SysFile::getTimeStamp(const char *&time)
{
    time = "";     // default return value
    // are we open?
    if (fileHandle >= 0)
    {
        // our existing 'fileHandle' doesn't seem to help; we still use 'filename'
        return getTimeStamp(filename, time);
    }
    return false;
}

/**
 * Retrieve the time stamp of a file from the file name.  If the
 * name is a device, the null string is returned.
 *
 * @param name   file path and name
 * @param time   The returned time stamp, e. g. Wed Jan 02 02:03:55 1980\n\0
 *
 * @return True if the time stamp was retrievable, false otherwise.
 */
bool SysFile::getTimeStamp(const char *name, const char *&time)
{
    time = "";     // default return value
    // the handle is not active, use the name
    FILETIME lastWriteGetTime, lastWriteTime;
    HANDLE h = CreateFile(name, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (h != INVALID_HANDLE_VALUE &&
        GetFileTime(h, NULL, NULL, &lastWriteGetTime) &&
        FileTimeToLocalFileTime(&lastWriteGetTime, &lastWriteTime))
    {
        time_t mtime;
        ULARGE_INTEGER ull;
        ull.LowPart = lastWriteTime.dwLowDateTime;
        ull.HighPart = lastWriteTime.dwHighDateTime;
        mtime = ull.QuadPart / 10000000ULL - 11644473600ULL;

        time = asctime(gmtime(&mtime));

        CloseHandle(h);
        return true;
    }
    return false;
}


/**
 * Determine the stream characteristics after an open.
 */
void SysFile::getStreamTypeInfo()
{
    transient = false;
    device = false;
    isTTY = false;
    writeable = false;
    readable = false;

    if (_isatty(fileHandle))
    {
        transient = true;
        device = true;
        isTTY = true;
    }
    // have a handle, use fstat() to get the info
    struct _stati64 fileInfo;
    if (_fstati64(fileHandle, &fileInfo) == 0)
    {
        //  character device?  set those characteristics
        if ((fileInfo.st_mode & _S_IFCHR) != 0)
        {
            device = true;
            transient = true;
        }

        if ((fileInfo.st_mode & _S_IWRITE) != 0)
        {
            writeable = true;
        }

        if ((fileInfo.st_mode & _S_IREAD) != 0)
        {
            readable = true;
        }
        // tagged as FIFO, then this is also a transient
        if ((fileInfo.st_mode & _S_IFIFO) != 0)
        {
            transient = true;
        }
    }
}

/**
 * Set a SysFile object to be the standard output stream.
 */
void SysFile::setStdIn()
{
    // set the file handle using the standard handles, but force binary mode
    fileHandle = _fileno(stdin);
    _setmode(fileHandle, _O_BINARY);
    ungetchar = -1;            // -1 indicates no char
    getStreamTypeInfo();
    // NB:  On Windows, we get a strange overlay when reading one character at a time from
    // stdin, so allow this to work buffered.
    readable = true;             // force this to readable
}

/**
 * Set a SysFile object to the standard output stream.
 */
void SysFile::setStdOut()
{
    // set the file handle using the standard handles, but force binary mode
    fileHandle = _fileno(stdout);
    _setmode(fileHandle, _O_BINARY);
    ungetchar = -1;            // -1 indicates no char
    getStreamTypeInfo();
    setBuffering(false, 0);
    writeable = true;             // force this to writeable
    // make this unbuffered
    setbuf(stdout, NULL);
}

/**
 * Set a SysFile object to the stderr stream.
 */
void SysFile::setStdErr()
{
    // set the file handle using the standard handles, but force binary mode
    fileHandle = _fileno(stderr);
    _setmode(fileHandle, _O_BINARY);
    ungetchar = -1;            // -1 indicates no char
    getStreamTypeInfo();
    setBuffering(false, 0);
    writeable = true;             // force this to writeable
    // make this unbuffered
    setbuf(stderr, NULL);
}


/**
 * Check to see if a stream still has data.
 *
 * @return True if data can be read from the stream, false otherwise.
 */
bool SysFile::hasData()
{
    // not available for reads?  Can't have data
    if (!readable)
    {
        return false;
    }

    // if there is buffered input, we can always return true
    if (ungetchar != -1 || hasBufferedInput())
    {
        return true;
    }

    // tty devices require special handling
    if (isTTY)
    {
        return (_kbhit() != 0) ? 1 : 0;
    }

    // we've already checked for buffered input, now check to see if the .
    // stream is readable.
    return !atEof();
}
