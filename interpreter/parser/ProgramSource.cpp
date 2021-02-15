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
/*                                                                            */
/* Abstractions for different types of program source entities we can parse   */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "ProgramSource.hpp"
#include "ProtectedObject.hpp"
#include "BufferClass.hpp"
#include "SmartBuffer.hpp"
#include "SystemInterpreter.hpp"
#include "GlobalNames.hpp"
#include "ActivityManager.hpp"
#include "SysFile.hpp"


/**
 * Create a new program source instance
 *
 * @param size   The size of the handler
 *
 * @return The allocated object.
 */
void *ProgramSource::operator new(size_t size)
{
    return new_object(size, T_ProgramSource);  // Get new object
}


/**
 * Retrieve a source line as a string object.
 *
 * @param position The line position.
 *
 * @return A string version of the source line.  Returns "" if the
 *         source line is not available.
 */
RexxString *ProgramSource::getStringLine(size_t position)
{
    const char *linePointer;
    size_t lineLength;

    getLine(position, linePointer, lineLength);

    // a length of 0 either means this IS a null line or
    // the line does not exist.  Just return the constant
    // null string regardless.
    if (lineLength == 0)
    {
        return GlobalNames::NULLSTRING;
    }
    // convert to a string object.
    return new_string(linePointer, lineLength);
}


/**
 * Retrieve a a portion of a source line
 *
 * @param position  The line position.
 * @param startOffset
 *                  The offset of the first character (zero based).
 * @param endOffset The offset of the last character (zero
 *                  based), but actually points one past the
 *                  end.
 *
 * @return A string version of the source line.  Returns "" if the
 *         source line is not available.
 */
RexxString *ProgramSource::getStringLine(size_t position, size_t startOffset, size_t endOffset)
{
    const char *linePointer;
    size_t lineLength;

    getLine(position, linePointer, lineLength);

    // a length of 0 either means this IS a null line or
    // the line does not exist.  Just return the constant
    // null string regardless.
    if (lineLength == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // protect from an overrun
    startOffset = std::min(startOffset, lineLength);
    // we frequently ask for the entire line by giving an offset of zero.
    if (endOffset == 0)
    {
        endOffset = lineLength;
    }
    else
    {
        endOffset = std::min(endOffset, lineLength);
    }

    // we can use this to extract from a position to the end by
    // specifying an end offset of 0
    if (endOffset < startOffset)
    {
        endOffset = lineLength;
    }

    // convert to a string object.
    return new_string(linePointer + startOffset, endOffset - startOffset);
}


/**
 * Extract a line from the source given a source location.
 *
 * @param location The location indicator.
 *
 * @return The source line, as a string object.
 */
RexxString *ProgramSource::extract(SourceLocation &location )
{
    // not traceable means no source, so just return a null string regardless
    if (!isTraceable())
    {
        return GlobalNames::NULLSTRING;
    }

    // make sure this is within range of the lines we have.
    if (location.getLineNumber() < getFirstLine() || location.getLineNumber() > lineCount)
    {
        return GlobalNames::NULLSTRING;
    }
    // easiest situation is all on one line.
    else if (location.getLineNumber() >= location.getEndLine())
    {
        return getStringLine(location.getLineNumber(), location.getOffset(), location.getEndOffset());
    }
    else
    {
        // multiple line case...sigh.  We need to build this up
        // start with the first line, which might be a partial
        ProtectedObject line = getStringLine(location.getLineNumber(), location.getOffset());
        // now concatentate all of the full lines onto this until we get to the final line, which is likely
        // a partial line again.
        for (size_t counter = location.getLineNumber() + 1; counter < location.getEndLine(); counter++)
        {
            line = ((RexxString *)line)->concat(getStringLine(counter));
        }
        // and finally add the partial last line
        return ((RexxString *)line)->concat(getStringLine(location.getEndLine(), 0, location.getEndOffset()));
    }
}


/**
 * Extract a section of source as an array of lines.
 *
 * @param location The location defining the start and end positions.
 *
 * @return An array of the source lines. The first and last lines
 *         might be partial lines from the source.
 */
ArrayClass *ProgramSource::extractSourceLines(SourceLocation &location )
{
    // not traceable means no source, so just return a null string regardless
    if (!isTraceable())
    {
        // return a zero length array
        return new_array((size_t)0);
    }

    // is the start location out of bounds?  Also a null array
    if (location.getLineNumber() == 0 || location.getLineNumber() > lineCount)
    {
        return new_array((size_t)0);
    }
    else
    {
        // we have something to extract.  If the end line is set to zero, that means
        // extract everything, so set the location information.
        if (location.getEndLine() == 0)
        {
            // retrieve the last line specifics and fudge the location data
            // with the information.
            const char *linePointer;
            size_t lineLength;
            getLine(lineCount, linePointer, lineLength);
            location.setEnd(lineCount, lineLength);
        }
        // is the last line marked at the start?  Don't
        // include anything from that line
        else if (location.getEndOffset() == 0)
        {
            // step back a line, and set the location to the line length
            location.setEndLine(location.getEndLine() - 1);

            const char *linePointer;
            size_t lineLength;
            getLine(location.getEndLine(), linePointer, lineLength);
            location.setEndOffset(lineLength);
        }

        // get the result array
        ArrayClass *source = new_array(location.getEndLine() - location.getLineNumber() + 1);
        ProtectedObject p(source);

        // is this just one one line?  This is easy
        if (location.getLineNumber() == location.getEndLine())
        {
            // get the single line and add to the source array.  Then we're done
            RexxString *line = extract(location);
            source->put(line, 1);
            return source;
        }

        // extract everything from the starting offset to the end. and add it to the array
        source->put(getStringLine(location.getLineNumber(), location.getOffset(), 0), 1);

        size_t i = 2;
        // now loop over all lines in between, adding entire lines
        for (size_t counter = location.getLineNumber() + 1; counter < location.getEndLine(); counter++, i++)
        {
            source->put(getStringLine(counter), i);
        }

        // now get the end portion
        source->put(getStringLine(location.getEndLine(), 0, location.getEndOffset()), i);
        return source;
    }
}


/**
 * Create a new program source instance
 *
 * @param size   The size of the handler
 *
 * @return The allocated object.
 */
void *BufferProgramSource::operator new(size_t size)
{
    return new_object(size, T_BufferProgramSource);  // Get new object
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void BufferProgramSource::live(size_t liveMark)
{
    memory_mark(descriptorArea);
    memory_mark(buffer);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void BufferProgramSource::liveGeneral(MarkReason reason)
{
    memory_mark_general(descriptorArea);
    memory_mark_general(buffer);
}


/**
 * Flatten a program source object.
 *
 * @param envelope The envelope used for the data.
 */
void BufferProgramSource::flatten(Envelope *envelope)
{
    setUpFlatten(BufferProgramSource)
      flattenRef(descriptorArea);
      flattenRef(buffer);
    cleanUpFlatten
}


/**
 * Get the buffer pointer and length information so that
 * the descriptors can be set up.
 *
 * @param data   The returned data pointer.
 * @param length The returned data length.
 */
void BufferProgramSource::getBuffer(const char *&data, size_t &length)
{
    // just copy the buffer specifics to the variables
    data = buffer->getData();
    length = buffer->getDataLength();
}

/**
 * Perform setup on the descriptors and the buffer
 * information.
 */
void BufferProgramSource::setup()
{
    // just construct the descriptor area
    buildDescriptors();
}


/**
 * Perform setup on the descriptors and the buffer
 * information.
 */
void BufferProgramSource::buildDescriptors()
{
    const char *bufferArea = NULL;
    size_t bufferLength = 0;

    // access the buffer data (provided by the subclasses
    getBuffer(bufferArea, bufferLength);

    // allocate a smart buffer to hold the line descriptors.  This will expand
    // as necessary as we add data to this
    SmartBuffer *indices = new SmartBuffer(1024);
    ProtectedObject p(indices);

    LineDescriptor descriptor;


    // copy a dummy 0-th descriptor to the list.
    // NOTE: SmartBuffer copyData appends to last copied position.
    indices->copyData(&descriptor, sizeof(descriptor));

    // we have zero lines initially.
    lineCount = 0;


    // scan for an EOF mark (rare), but we truncate the data if we find one.
    const char *scan = (const char *)memchr(bufferArea, ctrl_z, bufferLength);
    if (scan != NULL)
    {
        bufferLength = scan - bufferArea;
    }

    // now we start scanning for lines from the beginning.  NOTE:
    // we will handle any #! line after we scan.  If we find one, we'll
    // adjust the first descriptor to be a zero-length line so that
    // nothing will end up getting scanned.  This way we don't end up
    // modifying data we shouldn't be touching.
    const char *current = bufferArea;

    // now search for all of the line-end markers in the buffer
    while (bufferLength != 0)
    {
        // bump the line count, and set the descriptor offset based on
        // our current scan positions.
        lineCount++;
        descriptor.position = current - bufferArea;

        // scan for a line-end (note, we handle both /n and /r/n variants
        scan = Utilities::locateCharacter(current, line_delimiters, bufferLength);

        // not found, we're at the end with no line-end marker.  Consume
        // the remainder for this descriptor and set the length to 0 to
        // terminate the scan
        if (scan == NULL)
        {
            current = current + bufferLength;
            descriptor.length = bufferLength;
            bufferLength = 0;
        }
        else
        {
            // consume this segment, then figure out what sort
            // of line terminator we have
            descriptor.length = scan - current;
            // was this a CR character?  We need to check if there is also
            // a LF paired with it.
            if (*scan == line_delimiters[0])
            {
                scan++;
                // this could be at the end, don't overrun the buffer
                if (bufferLength > (size_t)(scan - current))
                {
                    // still have room, so check for the LF and step if
                    // found.
                    if (*scan == line_delimiters[1])
                    {
                        scan++;
                    }
                }
            }
            // by definition, this must be a LF, so we can just step over it.
            else
            {
                scan++;
            }

            // consume this section and go around
            bufferLength -= scan - current;
            current = scan;
        }
        // append this to the end of the descriptors
        indices->copyData(&descriptor, sizeof(descriptor));
    }

    // ok, get the embedded buffer and keep that portion
    OrefSet(this, this->descriptorArea, indices->getBuffer());

    // now we need to see if we've got a shebang line.  If we find
    // this, tell the language parser to start parsing on the second line.
    // This will cause the line to be ignored, but it will be left in the
    // lines returned by sourceline.
    if (bufferArea[0] == '#' && bufferArea[1] == '!')
    {
        // start parsing with the second line
        firstLine = 2;
    }
}


/**
 * Get a specific descriptor for a line.  NOTE:  this does
 * not do any validity checking on the line position.
 *
 * @param l      The target line position.
 *
 * @return The descriptor for that line.
 */
LineDescriptor &BufferProgramSource::getDescriptor(size_t l)
{
    return getDescriptors()[l];
}


/**
 * Get a pointer to the buffer descriptors.
 *
 * @return A pointer to the array of line descriptors.
 */
LineDescriptor *BufferProgramSource::getDescriptors()
{
    // just cast the pointer to the buffer area
    return (LineDescriptor *)descriptorArea->getData();
}


/**
 * Retrieve the line information for a specific line
 * position.
 *
 * @param lineNumber The target line number
 * @param linePointer
 *                   A returned line pointer for the line.
 *
 * @param lineLength The returned line length.
 */
void BufferProgramSource::getLine(size_t lineNumber, const char *&linePointer, size_t &lineLength)
{
    // check to see if  the requested number in bounds
    if (lineNumber > lineCount)
    {
        // null out the line information and quit
        linePointer = NULL;
        lineLength = 0;
        return;
    }

    // get the line descriptor and turn that into a pointer/length pair
    LineDescriptor &line = getDescriptor(lineNumber);
    line.getLine(getBufferPointer(), linePointer, lineLength);
}


/**
 * Get the pointer to the source buffer area.
 *
 * @return The character pointer to the buffer.
 */
const char *BufferProgramSource::getBufferPointer()
{
    return buffer->getData();
}


/**
 * Create a new program source instance
 *
 * @param size   The size of the handler
 *
 * @return The allocated object.
 */
void *ArrayProgramSource::operator new(size_t size)
{
    return new_object(size, T_ArrayProgramSource);  // Get new object
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void ArrayProgramSource::live(size_t liveMark)
{
    memory_mark(array);
}

/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void ArrayProgramSource::liveGeneral(MarkReason reason)
{
    memory_mark_general(array);
}


/**
 * Flatten a program source object.
 *
 * @param envelope The envelope used for the data.
 */
void ArrayProgramSource::flatten(Envelope *envelope)
{
    setUpFlatten(ArrayProgramSource)
      flattenRef(array);
    cleanUpFlatten
}

/**
 * Perform any needed setup for this program source.  For
 * arrays, this is just setting the linecount.
 */
void ArrayProgramSource::setup()
{
    size_t adjust = 0;

    // if we have an interpret adjustment, back it off one
    if (interpretAdjust > 0)
    {
        adjust = interpretAdjust - 1;
    }

    // set the line count to the number of items
    lineCount = array->lastIndex();

    // fake things out by the interpret adjustment amount
    lineCount += adjust;

    // also adjust the first line
    firstLine += adjust;

    // it is possible that the array version might include a shebang line (sigh).
    // if we detect that, replace it with a null line so it will be ignored.
    // BUT, we don't do this if we're an interpret.
    if (lineCount > 0 && interpretAdjust == 0)
    {
        RexxString *line = (RexxString *)array->get(1);
        // now we need to see if we've got a shebang line.  If we find
        // this, bump the start line by one so we start parsing after the shebang
        // we want to keep the line so we don't throw off the line counts.
        if (line->startsWith("#!"))
        {
            firstLine++;
        }
    }
}


/**
 * Retrieve the line information for a specific line
 * position.
 *
 * @param lineNumber The target line number (which will be adjusted with
 *                   the interpret adjustment).
 * @param linePointer
 *                   A returned line pointer for the line.
 *
 * @param lineLength The returned line length.
 */
void ArrayProgramSource::getLine(size_t lineNumber, const char *&linePointer, size_t &lineLength)
{
    if (lineNumber > lineCount || lineNumber < interpretAdjust)
    {
        // null out the line information and quit
        linePointer = NULL;
        lineLength = 0;
        return;
    }

    // adjust for the interpret offset
    size_t targetLine = lineNumber - (interpretAdjust > 0 ? interpretAdjust - 1 : 0);

    // get the line from the array, making sure we adjust for interpret line numbers.
    RexxString *line = (RexxString *)(array->get(targetLine));
    // A missing line?  We could have been handed a sparse array.  This is an error
    if (line == OREF_NULL)
    {
        reportException(Error_Translation_invalid_line);
    }

    //  now make sure we've been given a string item in each position
    if (!isString(line))
    {
        // convert any non-string to a string if we can
        line = line->stringValue();
        // .nil can't be converted, this is also an error
        if (line == TheNilObject)
        {
            reportException(Error_Translation_invalid_line);
        }
    }
    // we work with direct pointer/length information, so set the new current
    linePointer = line->getStringData();
    lineLength = line->getLength();
}


/**
 * Create a new program source instance
 *
 * @param size   The size of the handler
 *
 * @return The allocated object.
 */
void *FileProgramSource::operator new(size_t size)
{
    return new_object(size, T_FileProgramSource);  // Get new object
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void FileProgramSource::live(size_t liveMark)
{
    memory_mark(descriptorArea);
    memory_mark(buffer);
    memory_mark(fileName);
}

/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void FileProgramSource::liveGeneral(MarkReason reason)
{
    memory_mark_general(descriptorArea);
    memory_mark_general(buffer);
    memory_mark_general(fileName);
}


/**
 * Flatten a program source object.
 *
 * @param envelope The envelope used for the data.
 */
void FileProgramSource::flatten(Envelope *envelope)
{
    setUpFlatten(FileProgramSource)
      flattenRef(descriptorArea);
      flattenRef(buffer);
      flattenRef(fileName);
    cleanUpFlatten
}


/**
 * Initialize the program source object from file data.
 */
void FileProgramSource::setup()
{
    // read the file into a buffer object, reporting an error if this failed
    // for any reason
    buffer = readProgram(fileName->getStringData());
    if (buffer == OREF_NULL)
    {
        reportException(Error_Program_unreadable_name, fileName);
    }
    // go set up all of the line information so the parser can read this.
    BufferProgramSource::setup();
}


/**
 * Read a program, returning a buffer object.
 *
 * @param file_name The target file name.
 *
 * @return A buffer object holding the program data.
 */
BufferClass* FileProgramSource::readProgram(const char *file_name)
{
    SysFile programFile;          // the file we're reading

    // if unable to open this, return false
    if (!programFile.open(file_name, RX_O_RDONLY, RX_S_IREAD, RX_SH_DENYWR))
    {
        return OREF_NULL;
    }

    int64_t bufferSize = 0;

    // get the size of the file
    programFile.getSize(bufferSize);
    size_t readSize;

    // get a buffer object to return the image
    Protected<BufferClass> buffer = new_buffer((size_t)bufferSize);
    {
        UnsafeBlock releaser;

        // read the data
        programFile.read(buffer->getData(), (size_t)bufferSize, readSize);
        programFile.close();
    }
    // if there was a read error, return nothing
    if ((int64_t)readSize < bufferSize)
    {
        return OREF_NULL;
    }
    // ready to run
    return buffer;
}
