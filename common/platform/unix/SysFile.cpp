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
/* REXX Kernel                                              SysFile.cpp       */
/*                                                                            */
/* Unix implementation of the SysFile class.                                  */
/*                                                                            */
/******************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#if defined( HAVE_SYS_FILIO_H )
# include <sys/filio.h>
#endif
#include "SysFile.hpp"

// This is all the static stuff
const int SysFile::stdinHandle = 0;
const int SysFile::stdoutHandle = 1;
const int SysFile::stderrHandle = 2;

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
}

/**
 * Opens a file.  This opens the file for both lowlevel I/O
 * and also for higher level I/O.
 *
 * @param name       Name of the stream.
 * @param openFlags  The open flags.  This are the same flags used on the open()
 *                   function.
 *
 * @return true if the file was opened successfully, false otherwise.
 */
bool SysFile::open(const char *name, int openFlags, int openMode, int shareMode)
{
    flags = openFlags;           // save the initial flag values

    // we must open this with the NOINHERIT flag added
    fileHandle = ::open64(name, openFlags, (mode_t)openMode);
    if ( fileHandle == -1 )
    {
        errInfo = errno;
        return false;
    }

    // mark that we opened this handle
    openedHandle = true;

    // save a copy of the name
    filename = strdup(name);
    ungetchar = -1;              // 0xFF indicates no char

    // is this append mode?
    if ((flags & RX_O_APPEND) != 0)
    {
        // mark this true, and position at the end
        append = true;
        lseek64(fileHandle, 0, SEEK_END);
    }

    // set eof flag
    fileeof = false;

    // set the default buffer size (and allocate the buffer)
    setBuffering(true, 0);
    getStreamTypeInfo();
    return true;
}


/**
 * Open a stream using a provided handle value.
 *
 * @param handle     The source stream handle.
 * @param fdopenMode The fdopen() mode flags for the stream.
 *
 * @return true if the file opened ok, false otherwise.
 */
bool SysFile::open(int handle)
{
    // we didn't open this.
    openedHandle = false;
    fileHandle = handle;
    ungetchar = -1;              // 0xFF indicates no char
    // set the default buffer size (and allocate the buffer)
    setBuffering(true, 0);
    getStreamTypeInfo();
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
        free(const_cast<char *>(filename));
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
            int written = ::write(fileHandle, buffer, (unsigned int)bufferPosition);
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
                int blockRead = ::read(fileHandle, buffer, (unsigned int)bufferSize);
                if (blockRead <= 0)
                {
                    // not get anything?
                    if (blockRead == 0)
                    {
                        fileeof = true;
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
            int blockRead = ::read(fileHandle, buf + bytesRead, (unsigned int)len);
            if (blockRead <= 0)
            {
                // not get anything?
                if (blockRead == 0)
                {
                    fileeof = true;
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
            lseek64(fileHandle, offset, SEEK_SET);
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
            int written = ::write(fileHandle, data, (unsigned int)len);
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
            if ((flags & O_APPEND) != 0)
            {
                // seek to the end of the file, return if there is an error
                if (lseek64(fileHandle, 0, SEEK_END) < 0)
                {
                    errInfo = errno;
                    return false;
                }
            }
            // write the data
            int written = ::write(fileHandle, data, (unsigned int)len);
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
            int written = ::write(fileHandle, data, (unsigned int)len);
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
    ungetchar = ((int)ch) & 0xff;
    return true;
}

bool SysFile::getChar(char &ch)
{
    size_t len;

    return read(&ch, 1, len);
}

bool SysFile::puts(const char *data, size_t &len)
{
    return write(data, strlen(data), len);
}

/**
 * Write a line to the stream, adding the platform specific
 * line terminator.
 *
 * @param mybuffer Start of the line to write.
 * @param len    The length to write.
 * @param bytesWritten
 *               The actual number of bytes written, including the line
 *               terminator.
 *
 * @return A success/failure indicator.
 */
bool SysFile::putLine(const char *mybuffer, size_t len, size_t &bytesWritten)
{
    // this could be a null line...don't try to write zero bytes
    if (len > 0)
    {
        if (!write(mybuffer, len, bytesWritten))
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


bool SysFile::gets(char *mybuffer, size_t bufferLen, size_t &bytesRead)
{
    size_t i;
    for (i = 0; i < bufferLen - 1; i++)
    {
        size_t len;

        // if we don't get a character break out of here.
        if (!this->read(mybuffer + i, 1, len))
        {
            break;
        }

        // we only look for a newline character.  On return, this
        // line will have the terminator characters at the end, or
        // if the buffer fills up before we find the terminator,
        // this will just be null terminated.  If this us a multi
        // character line terminator, both characters will appear
        // at the end of the line.
        if (mybuffer[i] == '\n')
        {
            // once we hit a new line character, back up and see if the
            // previous character is a carriage return.  If it is, collapse
            // it to the single line delimiter.
            if (i >= 1 && mybuffer[i - 1] == '\r')
            {
                i--;
                mybuffer[i] = '\n';
            }
            i++;   // we need to step the position so that the null terminator doesn't overwrite
            break;
        }
    }

    // if there is no data read at all, this is an eof failure;
    if (i == 0)
    {
        return false;
    }

    // null terminate, set the length, and return
    mybuffer[i] = '\0';
    // this is the length minus the terminating null
    bytesRead = i;
    // return an error state, but not EOF status.
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
    char *mybuffer = (char *)malloc(LINE_POSITIONING_BUFFER);
    if (mybuffer == NULL)
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
            free(mybuffer);
            // set the return position and get outta here
            endPosition = startPosition;
            return true;
        }

        size_t bytesRead;
        if (!read(mybuffer, readLength, bytesRead))
        {
            free(mybuffer);
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
            free(mybuffer);
            // set the return position and get outta here
            endPosition = startPosition;
            return true;
        }


        size_t offset = 0;
        while (offset < bytesRead)
        {
            // we're only interested in \n character, since this will
            // mark the transition point between lines.
            if (mybuffer[offset] == '\n')
            {
                // reduce the line count by one.
                lineCount--;
                // reached the requested point?
                if (lineCount == 0)
                {
                    // set the return position and get outta here
                    endPosition = startPosition + offset + 1;
                    free(mybuffer);
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
    if (location >= (filePointer - bufferedInput) && location < filePointer)
    {
        // just shift the buffer position;
        bufferPosition = (size_t)(location - (filePointer - (int64_t)bufferedInput));
        // just return the same value
        position = location;
    }
    else
    {
        // go to the absolute position
        position = lseek64(fileHandle, location, SEEK_SET);
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
                position = lseek64(fileHandle, offset, SEEK_SET);
                break;

            case SEEK_CUR:
                position = lseek64(fileHandle, offset, SEEK_CUR);
                break;

            case SEEK_END:
                position = lseek64(fileHandle, offset, SEEK_END);
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
        position = lseek64(fileHandle, 0, SEEK_CUR);
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
        // have a handle, use fstat() to get the info
        struct stat64 fileInfo;
        if (fstat64(fileHandle, &fileInfo) == 0)
        {
            // regular file?  return the defined size
            if ((fileInfo.st_mode & S_IFREG) != 0)
            {
                size = fileInfo.st_size;
            }
            else
            {
                size = 0;
            }
            return true;
        }
    }
    return false;
}

/**
 * Retrieve the size of a file from the file name.  If the
 * name is a device, it zero is returned.
 *
 * @param size   The returned size value.
 *
 * @return True if the size was retrievable, false otherwise.
 */
bool SysFile::getSize(const char *name, int64_t &size)
{
    // the handle is not active, use the name
    struct stat64 fileInfo;
    if (stat64(name, &fileInfo) == 0)
    {
        // regular file?  return the defined size
        if ((fileInfo.st_mode & S_IFREG) != 0)
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
 * Retrieve the size of the stream.  If the stream is open,
 * it returns the stream size.  Zero is returned if this
 * stream is not a regular file.
 *
 * @param size   The returned size value.
 *
 * @return True if the size was retrievable, false otherwise.
 */
bool SysFile::getTimeStamp(const char *&time)
{
    time = "";     // default return value
    // are we open?
    if (fileHandle >= 0)
    {
        // have a handle, use fstat() to get the info
        struct stat64 fileInfo;
        if (fstat64(fileHandle, &fileInfo) == 0)
        {
            // regular file?  return the defined size
            if ((fileInfo.st_mode & S_IFREG) != 0)
            {
                time = ctime(&fileInfo.st_mtime);
            }
        }
    }
    return false;
}

/**
 * Retrieve the size of a file from the file name.  If the
 * name is a device, it zero is returned.
 *
 * @param size   The returned size value.
 *
 * @return True if the size was retrievable, false otherwise.
 */
bool SysFile::getTimeStamp(const char *name, const char *&time)
{
    time = "";         // default return value
    // the handle is not active, use the name
    struct stat64 fileInfo;
    if (stat64(name, &fileInfo) == 0)
    {
        // regular file?  return the defined size
        if ((fileInfo.st_mode & (S_IFREG | S_IFDIR)) != 0)
        {
            time = ctime(&fileInfo.st_mtime);
        }
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

    if (isatty(fileHandle))
    {
        transient = true;
        device = true;
        isTTY = true;
    }
    // have a handle, use fstat() to get the info
    struct stat64 fileInfo;
    if (fstat64(fileHandle, &fileInfo) == 0)
    {
        //  character device?  set those characteristics
        if ((fileInfo.st_mode & S_IFCHR) != 0)
        {
            device = true;
            transient = true;
        }

        if ((fileInfo.st_mode & S_IWRITE) != 0)
        {
            writeable = true;
        }

        if ((fileInfo.st_mode & S_IREAD) != 0)
        {
            readable = true;
        }
    }
}

/**
 * Set a SysFile object to be the standard output stream.
 */
void SysFile::setStdIn()
{
    // set the file handle
    fileHandle = stdinHandle;
    // we didn't open this.
    openedHandle = false;
    ungetchar = -1;              // -1 indicates no char
    getStreamTypeInfo();
    setBuffering(false, 0);
    readable = true;             // force this to readable
    // STDIN is buffered by default so make it unbuffered
    setbuf(stdin, NULL);
}

/**
 * Set a SysFile object to the standard output stream.
 */
void SysFile::setStdOut()
{
    // set the file handle
    fileHandle = stdoutHandle;
    // we didn't open this.
    openedHandle = false;
    ungetchar = -1;              // -1 indicates no char
    getStreamTypeInfo();
    setBuffering(false, 0);
    writeable = true;             // force this to writeable

    // STDOUT is buffered by default so make it unbuffered
    setbuf(stdout, NULL);
}

/**
 * Set a SysFile object to the stderr stream.
 */
void SysFile::setStdErr()
{
    // set the file handle
    fileHandle = stderrHandle;
    // we didn't open this.
    openedHandle = false;
    ungetchar = -1;              // -1 indicates no char
    getStreamTypeInfo();
    setBuffering(false, 0);
    writeable = true;             // force this to writeable

    // STDERR is not buffered by default so no need to do anything
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

    // tty devices require special handling
    if (isTTY)
    {
        int bytesWaiting;
        ioctl(fileHandle, FIONREAD, &bytesWaiting);
        if (bytesWaiting)
        {
            return true;
        }
        else {
            return false;
        }
    }

    // we might have something buffered, but also check the
    // actual stream.
    return !atEof();
}
