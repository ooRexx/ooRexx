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
/* REXX Kernel                                                                */
/*                                                                            */
/* OutputRedirector implementations for the ADDRESS WITH instruction          */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "InputRedirector.hpp"
#include "OutputRedirector.hpp"
#include "Activity.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "PackageClass.hpp"
#include "StemClass.hpp"


/**
 * Test if an output redirector is using the same source
 * as the input redirector, and thus needs buffering.
 *
 * @param in     The input redirector
 *
 * @return True if the input redirector and the output redirector
 *         are using the same target object.
 */
bool OutputRedirector::needsBuffering(InputRedirector *in)
{
    return type() == in->type() && target() == in->target();
}


/**
 * Test if an output redirector is using the same target as the
 * error redirector.
 *
 * @param e      The error redirector
 *
 * @return True if the error redirector and the output
 *         redirector are using the same target object.
 */
bool OutputRedirector::isSameTarget(OutputRedirector *e)
{
    return type() == e->type() && target() == e->target();
}


/**
 * Process a write to this output target
 *
 * @param line   The line to write.
 */
void OutputRedirector::write(RexxString *line)
{
    // if we've had a previous writeBuffer, we
    // could have the tail end of the buffer still
    // pending. Consider that a complete line, then
    // write this new line
    flushBuffer();
    writeLine(line);
}


/**
 * Write a buffer of data potentially consisting of multiple
 * lines to the collector. We will parse out as many lines as
 * we can, then will hold the remainder until we receive another
 * buffer.
 *
 * @param data   The data buffer to process
 * @param len    The length of the buffer.
 */
void OutputRedirector::writeBuffer(const char *data, size_t len)
{
    // seems silly, but this is easy
    if (len == 0)
    {
        return;
    }

    const char *bufferEnd = data + len;
    // we might have a buffer here. Need to merge this with the data, then we can
    // chop up the rest of this buffer
    resolveBuffer(data, bufferEnd);
    // the process of resolving the buffer could have consumed all of this new
    // data, so
    if (data < bufferEnd)
    {
        consumeBuffer(data, bufferEnd);
    }
}


/**
 * Flush any remaining buffered data to the accumulator.
 */
void OutputRedirector::flushBuffer()
{
    if (bufferedData != OREF_NULL)
    {
        // On Windows, there seems to be a boundary condition that causes
        // a dangling '\r' on the last read buffer. If we have this at the end
        // of the buffer, we need to chop that off.
        if (bufferedData->endsWith('\r'))
        {
            // yep, all but the last part of the tail is a complete line,
            // and we need to step over the first buffer character
            Protected<RexxString> newLine = new_string(bufferedData->getStringData(), bufferedData->getLength() - 1);
            writeLine(newLine);
        }
        // the dangling buffer is all data
        else
        {
            writeLine(bufferedData);
        }
    }
    bufferedData = OREF_NULL;
}


/**
 * Check to see if we have the tail end of a previous buffer
 * still left. We need to combine this with the front of the new
 * buffer to create a complete line and add that to the
 * accumulator.
 *
 * @param data      The start of the buffer.
 * @param bufferEnd The end of the buffer.
 */
void OutputRedirector::resolveBuffer(const char *&data, const char *bufferEnd)
{
    // best case is we don't need to do anything.
    if (bufferedData == OREF_NULL)
    {
        return;
    }
    // one complication of the scan is the possibility that our
    // buffer ended with the first half of a linend pair. We need to check
    // that possibility first
    if (bufferedData->endsWith('\r'))
    {
        // rats, see if this was a true boundary condition
        if (*data == '\n')
        {
            // change the buffer start
            data++;
            // yep, all but the last part of the tail is a complete line,
            // and we need to step over the first buffer character
            Protected<RexxString> newLine = new_string(bufferedData->getStringData(), bufferedData->getLength() - 1);
            writeLine(newLine);

            // we no longer have a tail piece
            bufferedData = OREF_NULL;
            return;
        }

        // not a split, the '\r' is considered real data
    }

    const char *lineEnd = NULL;
    const char *nextLine = NULL;

    // scan for a line in the new buffer
    scanLine(data, bufferEnd, lineEnd, nextLine);

    // did we find a lineend? if not, we need to merge our old buffer with the new
    // data and hold it again.
    if (lineEnd == NULL)
    {
        // save this and cast off the previous buffer
        bufferedData = new_string(bufferedData->getStringData(), bufferedData->getLength(), data, bufferEnd - data);
        // mark that we've used up all of the new data
        data = bufferEnd;
        // and we're finished here
        return;
    }

    // we have a complete line now split in two pieces
    Protected<RexxString> newLine = new_string(bufferedData->getStringData(), bufferedData->getLength(), data, lineEnd - data);
    writeLine(newLine);

    // we're done with this
    bufferedData = OREF_NULL;

    // now update the data buffer position
    if (nextLine == NULL)
    {
        // end on a linend, we're done with this new buffer
        data = bufferEnd;
    }
    else
    {
        // start scanning at the next position
        data = nextLine;
    }
}


/**
 * Consume as much of a received buffer as individual
 * lines as we can, saving the rest.
 *
 * @param data      The start of the buffer.
 * @param bufferEnd The end of the buffer.
 */
void OutputRedirector::consumeBuffer(const char *data, const char *bufferEnd)
{
    const char *lineEnd = NULL;
    const char *nextLine = NULL;

    for (;;)
    {
        // scan for a line
        scanLine(data, bufferEnd, lineEnd, nextLine);
        // NULL lineEnd means we did not find a line terminator. We save the remainder
        // of the buffer and quit
        if (lineEnd == NULL)
        {
            // we just save this as a string object.
            bufferedData = new_string(data, bufferEnd - data);
            return;
        }
        // convert to a string object and add to the accumulator
        Protected<RexxString> newLine = new_string(data, lineEnd - data);
        writeLine(newLine);

        // the buffer could have ended with a linend
        // we've magically used up the entire buffer and can leave now
        if (nextLine == NULL)
        {
            return;
        }
        // step forward to that position and scan again
        data = nextLine;
    }
}


/**
 * Useful method for scanning a buffer looking for logical
 * lines with the buffer. This uses the same line termination
 * rules as the platform.
 *
 * @param lineStart The start location for scanning.
 * @param bufferLen The length of buffer we have to work with.
 * @param lineEnd   The return pointer to the end of the scanned line. Returns
 *                  NULL if no line end is found.
 * @param nextLine  Pointer to the start of the next line (past the linend)
 *                  returns NULL if this would be past the end of the buffer.
 */
void OutputRedirector::scanLine(const char *lineStart, const char *bufferEnd, const char *&lineEnd, const char *&nextLine)
{
    lineEnd = NULL;
    nextLine = NULL;

    // now scan the buffer looking for a line end.
    for (; lineStart < bufferEnd; lineStart++)
    {
        // special handling for carriage return characters. we check the next character
        // next character and if it is a newline, then we step over both characters and
        // consider this a new line. Otherwise, we skip the character
        // and leave the \r as data within the line.
        if (*lineStart == '\r')
        {

            // this is a little bit of a quandry. We found a \r as the last character
            // of the buffer. We can't make a decision here, so punt and say we don't have
            // a line
            if (lineStart == bufferEnd - 1)
            {
                return;
            }

            // is the next character a newline?
            if (*(lineStart + 1) == '\n')
            {
                lineEnd = lineStart;
                nextLine = lineStart + 2;
                // did this happen at the end of the buffer?
                // no next Line to note
                if (nextLine == bufferEnd)
                {
                    nextLine = NULL;
                }
                return;
            }
        }

        // Now check for a newline.
        else if (*lineStart == '\n')
        {
            lineEnd = lineStart;
            nextLine = lineStart + 1;
            // did this happen at the end of the buffer?
            // no next Line to note
            if (nextLine == bufferEnd)
            {
                nextLine = NULL;
            }
            return;
        }
    }

    // if we scan all of the buffer without finding a line,
    // the returns are already set to NULL;
}


/**
 * Allocate a new stem output target
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *StemOutputTarget::operator new(size_t size)
{
    return new_object(size, T_StemOutputTarget);
}


/**
 * Construct a output target ogject
 *
 * @param stem      The stem oubect being used
 * @param o         The append/replace option for the redirect.
 */
StemOutputTarget::StemOutputTarget(StemClass *s, OutputOption::Enum o)
{
    stem = s;
    option = o;
    initialized = false;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void StemOutputTarget::live(size_t liveMark)
{
    memory_mark(bufferedData);
    memory_mark(stem);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void StemOutputTarget::liveGeneral(MarkReason reason)
{
    memory_mark_general(bufferedData);
    memory_mark_general(stem);
}


/**
 * Initialize and validate the stem condition for updates.
 */
void StemOutputTarget::init()
{
    // only one per customer!
    if (initialized)
    {
        return;
    }
    initialized = true;
    // replace is the easy way to handle we just reset everything
    if (option == OutputOption::REPLACE || option == OutputOption::DEFAULT)
    {
        // empty everything out (could already be empty
        stem->empty();
        // set the stem.0 value to zero
        stem->setElement((size_t)0, IntegerZero);
        // our next element to write is 1
        index = 1;
    }
    // asked to append...this is a little more work.
    else
    {
        // get the stem.0 value
        RexxObject *count = stem->getElement(size_t(0));
        // if completely not there, then we'll just treat this like a replace
        if (count == OREF_NULL)
        {
            // empty everything out (could already be empty
            stem->empty();
            // set the stem.0 value to zero
            stem->setElement((size_t)0, IntegerZero);
            // our next element to write is 1
            index = 1;
        }
        // the stem.0 value must be an integer size. We'll start writing at the next position
        else
        {
            // a valid whole number index
            if (!count->unsignedNumberValue(index, Numerics::ARGUMENT_DIGITS))
            {
                reportException(Error_Invalid_whole_number_stem_array_index, stem->getName(), count);
            }
            // we write to the next position
            index ++;
        }
    }
}


/**
 * Write a value to the output redirector.
 *
 * @param value  The string value to write
 */
void StemOutputTarget::writeLine(RexxString *value)
{
    stem->setElement(index, value);
    // update stem.0
    Protected<RexxInteger> newIndex = new_integer(index);
    stem->setElement((size_t)0, newIndex);
    // and step to the next position
    index++;
}


/**
 * Allocate a new stem output target
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *StreamObjectOutputTarget::operator new(size_t size)
{
    return new_object(size, T_StreamObjectOutputTarget);
}


/**
 * Construct a output target ogject
 *
 * @param stem      The stem oubect being used
 * @param o         The append/replace option for the redirect.
 */
StreamObjectOutputTarget::StreamObjectOutputTarget(RexxObject *s, OutputOption::Enum o)
{
    stream = s;
    option = o;
    initialized = false;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void StreamObjectOutputTarget::live(size_t liveMark)
{
    memory_mark(bufferedData);
    memory_mark(stream);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void StreamObjectOutputTarget::liveGeneral(MarkReason reason)
{
    memory_mark_general(bufferedData);
    memory_mark_general(stream);
}


/**
 * Test if an output redirector is using the same source
 * as the input redirector, and thus needs buffering.
 *
 * @param in     The input redirector
 *
 * @return True if the input redirector and the output redirector
 *         are using the same target object.
 */
bool StreamOutputTarget::needsBuffering(InputRedirector *in)
{
    // we need to do a string comparison here, not identity comparison
    return type() == in->type() && name->strCompare((RexxString *)in->target());
}


/**
 * Test if an output redirector is using the same target as the
 * error redirector.
 *
 * @param e      The error redirector
 *
 * @return True if the error redirector and the output
 *         redirector are using the same target object.
 */
bool StreamOutputTarget::isSameTarget(OutputRedirector *e)
{
    // we need to override this one because we need to use a string comparison
    // to verify we're the same target.
    return type() == e->type() && name->strCompare((RexxString *)e->target());
}


/**
 * Write a value to the output redirector.
 *
 * @param value  The string value to write
 */
void StreamObjectOutputTarget::writeLine(RexxString *value)
{
    // add the string to the next position
    ProtectedObject result;
    // this just uses lineout
    stream->sendMessage(GlobalNames::LINEOUT, value, result);
}


/**
 * Allocate a new output target
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *StreamOutputTarget::operator new(size_t size)
{
    return new_object(size, T_StreamOutputTarget);
}


/**
 * Construct a output target ogject
 *
 * @param n         The stream name
 * @param o         The append/replace option for the redirect.
 */
StreamOutputTarget::StreamOutputTarget(RexxString *n, OutputOption::Enum o)
{
    name = n;
    option = o;
    initialized = false;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void StreamOutputTarget::live(size_t liveMark)
{
    memory_mark(bufferedData);
    memory_mark(stream);
    memory_mark(name);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void StreamOutputTarget::liveGeneral(MarkReason reason)
{
    memory_mark_general(bufferedData);
    memory_mark_general(stream);
    memory_mark_general(name);
}


/**
 * Initialize and validate the stream condition for updates.
 *
 * @param option The append/replace option for the redirect.
 */
void StreamOutputTarget::init()
{
    // only one per customer!
    if (initialized)
    {
        return;
    }
    initialized = true;

    // create an instance of the stream class for the given name
    RexxClass *streamClass = TheRexxPackage->findClass(GlobalNames::STREAM);
    ProtectedObject result;
    stream = streamClass->sendMessage(GlobalNames::NEW, name, result);

    RexxString *openResult = OREF_NULL;

    // If replace is specified, we open this WRITE REPLACE
    if (option == OutputOption::REPLACE || option == OutputOption::DEFAULT)
    {
        openResult = (RexxString *)stream->sendMessage(GlobalNames::OPEN, GlobalNames::WRITE_REPLACE, result);
    }
    // asked to append...just a different open option
    else
    {
        openResult = (RexxString *)stream->sendMessage(GlobalNames::OPEN, GlobalNames::WRITE_APPEND, result);
    }
    // the open should return ready
    if (!openResult->strCompare(GlobalNames::OPENREADY))
    {
        reportException(Error_Execution_file_not_writeable, name, openResult);
    }
}


/**
 * Perform post-command housekeeping on this operation.
 */
void StreamOutputTarget::cleanup()
{
    // perform the base class cleanup first
    flushBuffer();

    ProtectedObject result;
    stream->sendMessage(GlobalNames::CLOSE, result);
}


/**
 * Allocate a new output target
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *CollectionOutputTarget::operator new(size_t size)
{
    return new_object(size, T_CollectionOutputTarget);
}


/**
 * Construct a output target ogject
 *
 * @param n         The stream name
 * @param o         The append/replace option for the redirect.
 */
CollectionOutputTarget::CollectionOutputTarget(RexxObject *c, OutputOption::Enum o)
{
    collection = c;
    option = o;
    initialized = false;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void CollectionOutputTarget::live(size_t liveMark)
{
    memory_mark(bufferedData);
    memory_mark(collection);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void CollectionOutputTarget::liveGeneral(MarkReason reason)
{
    memory_mark_general(bufferedData);
    memory_mark_general(collection);
}


/**
 * Write a value to the output redirector.
 *
 * @param value  The string value to write
 */
void CollectionOutputTarget::writeLine(RexxString *value)
{
    ProtectedObject result;
    // this just uses lineout
    collection->sendMessage(GlobalNames::APPEND, value, result);
}


/**
 * Initialize and validate the stream condition for updates.
 *
 * @param option The append/replace option for the redirect.
 */
void CollectionOutputTarget::init()
{
    // only one per customer!
    if (initialized)
    {
        return;
    }
    initialized = true;

    // If replace is specified, then empty the collection
    if (option == OutputOption::REPLACE || option == OutputOption::DEFAULT)
    {
        ProtectedObject result;
        // this just uses lineout
        collection->sendMessage(GlobalNames::EMPTY, result);
    }
}


/**
 * Allocate a new output target
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *BufferingOutputTarget::operator new(size_t size)
{
    return new_object(size, T_BufferingOutputTarget);
}


/**
 * Construct a output target ogject
 *
 * @param n         The stream name
 * @param o         The append/replace option for the redirect.
 */
BufferingOutputTarget::BufferingOutputTarget(OutputRedirector *t)
{
    target = t;
    initialized = false;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void BufferingOutputTarget::live(size_t liveMark)
{
    memory_mark(bufferedData);
    memory_mark(collector);
    memory_mark(target);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void BufferingOutputTarget::liveGeneral(MarkReason reason)
{
    memory_mark_general(bufferedData);
    memory_mark_general(collector);
    memory_mark_general(target);
}


/**
 * Initialize and validate the stream condition for updates.
 *
 * @param option The append/replace option for the redirect.
 */
void BufferingOutputTarget::init()
{
    // only one per customer!
    if (initialized)
    {
        return;
    }
    initialized = true;
    collector = new_array();
}


/**
 * Add the output item to our buffer
 *
 * @param value  The string value to add.
 */
void BufferingOutputTarget::writeLine(RexxString *value)
{
    collector->append(value);
}


/**
 * Perform post-command housekeeping on this operation.
 */
void BufferingOutputTarget::cleanup()
{
    // perform the base class cleanup first
    flushBuffer();

    // we've hit the end of things, so now the target redirector
    // gets activated and pumped full of data

    target->init();
    size_t count = collector->items();

    for (size_t i = 1; i <= count; i++)
    {
        target->writeLine((RexxString *)collector->get(i));
    }

    // and finally the cleanup
    target->cleanup();
}


/**
 * Allocate a new RexxQueue output target
 *
 * @param size   The size of the object.
 *
 * @return Storage for creating the object.
 */
void *RexxQueueOutputTarget::operator new(size_t size)
{
    return new_object(size, T_RexxQueueOutputTarget);
}


/**
 * Construct a output target ogject
 *
 * @param stem      The RexxQueue objbect being used
 */
RexxQueueOutputTarget::RexxQueueOutputTarget(RexxObject *q)
{
    queue = q;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxQueueOutputTarget::live(size_t liveMark)
{
    memory_mark(bufferedData);
    memory_mark(queue);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxQueueOutputTarget::liveGeneral(MarkReason reason)
{
    memory_mark_general(bufferedData);
    memory_mark_general(queue);
}


/**
 * Write a value to the output redirector.
 * @param value  The string value to write
 */
void RexxQueueOutputTarget::writeLine(RexxString *value)
{
    // add the string to the next position
    ProtectedObject result;
    // this just uses lineout
    queue->sendMessage(GlobalNames::QUEUE, value, result);
}
