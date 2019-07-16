/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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


#include "RexxCore.h"
#include "ProgramMetaData.hpp"
#include "BufferClass.hpp"
#include "Interpreter.hpp"
#include "ActivityManager.hpp"
#include "LanguageParser.hpp"
#include "ProtectedObject.hpp"
#include "SysFile.hpp"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "StringUtil.hpp"
#include "MutableBufferClass.hpp"

#include <stdio.h>


const char *compiledHeader = "/**/@REXX";
const char *encodedHeader = "/**/@REXX@\n";
const char *standardShebang = "#!/usr/bin/env rexx\n";


/**
 * Allocate a combined metadata object with the flattened
 * program data after it.
 *
 * @param size   The size of the object
 * @param buff   The appended buffer.
 *
 * @return The storage allocated for the new instance.
 */
void *ProgramMetaData::operator new (size_t size, BufferClass *buff)
{
    // allocate a new buffer for this
    return SystemInterpreter::allocateResultMemory(buff->getDataLength() + size - sizeof(char[4]));
}


/**
 * Initialize the meta data directly from a buffer.
 *
 * @param level  The language level required for the code stored in the buffer.
 * @param image  The image buffer.
 */
ProgramMetaData::ProgramMetaData(LanguageLevel level, BufferClass *image)
{
    // add the leading header, zero out remainder
    memset(&fileTag, '\0', sizeof(fileTag));
    strcpy(fileTag, compiledHeader);
    // fill in the version specifics and the hardware architecture type
    magicNumber = MAGICNUMBER;
    imageVersion = METAVERSION;
    // this is the number of bits in a word
    wordSize = Interpreter::getWordSize();
    bigEndian = Interpreter::isBigEndian();
    // this is the language level required to execute this program
    requiredLevel = level;
    memset(&reserved, '\0', sizeof(reserved));

    // copy in the image information
    imageSize = image->getDataLength();
    memcpy(imageData, image->getData(), imageSize);
}


/**
 * Initialize the meta data for a given data size.
 *
 * @param level      The language level required for the saved program.
 * @param bufferSize The size of the program data.
 */
ProgramMetaData::ProgramMetaData(LanguageLevel level, size_t bufferSize)
{
    // add the leading header, zero out remainder
    memset(&fileTag, '\0', sizeof(fileTag));
    strcpy(fileTag, compiledHeader);
    // fill in the version specifics and the hardward architecture type
    magicNumber = MAGICNUMBER;
    imageVersion = METAVERSION;
    // this is the number of bits in a word
    wordSize = Interpreter::getWordSize();
    bigEndian = Interpreter::isBigEndian();
    // this is the language level required to execute this program
    requiredLevel = level;
    memset(&reserved, '\0', sizeof(reserved));

    // copy in the image information
    imageSize = bufferSize;
}


/**
 * Get the final size of a copied buffer
 *
 * @return The data image size.
 */
size_t ProgramMetaData::getDataSize()
{
   return imageSize + sizeof(*this) - sizeof(imageData);
}


/**
 * Get the final size of a copied buffer
 *
 * @return The data image size.
 */
size_t ProgramMetaData::getHeaderSize()
{
   return (char *)&imageData - (char *)this;
}


/**
 * Return a pointer to the inline image data.
 *
 * @return The pointer to the image data following the metadata header.
 */
char *ProgramMetaData::getImageData()
{
    return imageData;
}


/**
 * Restore a saved program from the metadata.
 *
 * @param name   The program name being restored
 * @param buffer A buffer object that contains the image data
 *
 * @return The restored Routine object, or NULL if this was not a saved program image.
 */
RoutineClass *ProgramMetaData::restore(RexxString *name, BufferClass *buffer)
{
    ProgramMetaData *metaData;

    // check to see if this is already translated
    if (!processRestoreData(name, buffer, metaData))
    {
        // nope, can't restore this
        return OREF_NULL;
    }

    // make sure this is valid for interpreter
    if (!metaData->validate(name))
    {
        return OREF_NULL;
    }

    // this should be valid...try to restore.
    Protected<RoutineClass> routine = RoutineClass::restore(buffer, metaData->getImageData(), metaData->getImageSize());
    // change the program name to match the file this was restored from
    routine->getPackageObject()->setProgramName(name);
    return routine;
}


/**
 * Validate that this saved program image is valid for this
 * interpreter.
 *
 * @param fileName The filename this metadata belongs to. Used for errors.
 *
 * @return true if this is good data, false otherwise.
 */
bool ProgramMetaData::validate(RexxString *fileName)
{
    // we always add the compiled program tag to the front
    if (strcmp(fileTag, compiledHeader) != 0)
    {
        return false;
    }

    // check all of the version specifics
    if (magicNumber != MAGICNUMBER || imageVersion != METAVERSION || wordSize != Interpreter::getWordSize() ||
        (bigEndian != 0) != Interpreter::isBigEndian() || !LanguageParser::canExecute(requiredLevel))
    {
        // this is a version failure, mark it as such
        reportException(Error_Program_unreadable_version, fileName);
        return false;
    }
    // good to go.
    return true;
}


/**
 * Write the metadata to a file.
 *
 * @param handle  The handle of the output file.
 * @param program The program buffer data (also written out).
 */
void ProgramMetaData::write(SysFile &target, BufferClass *program, bool encode)
{
    size_t written = 0;

    // non-encoded is the easy way. We can just write the data directly to the file
    if (!encode)
    {
        UnsafeBlock releaser;

        // add a standard shebang line as a courtesy for the unix-based systems.
        target.write(standardShebang, strlen(standardShebang), written);
        target.write((const char *)this, getHeaderSize(), written);
        /* and finally the flattened method  */
        target.write(program->getData(), program->getDataLength(), written);
    }
    // encoding is a pain, we need to copy everything into a single string, base64-encode
    // the whole lot, then write it out with a special header that says it is encoded.
    else
    {
        Protected<RexxString> fullBuffer = raw_string(getHeaderSize() + program->getDataLength());

        // copy in the stem name and the tail piece.
        char *bufferData = fullBuffer->getWritableData();
        // copy the metadata structure to the string
        memcpy(bufferData, (const char *)this, getHeaderSize());
        // append the program data
        memcpy(bufferData + getHeaderSize(), program->getData(), program->getDataLength());

        size_t estimatedSize = ((program->getDataLength() / 3) * 4 + 1) + (program->getDataLength() / encodingChunkLength) + 1;

        Protected<MutableBuffer> encodedBuffer = new MutableBuffer(estimatedSize, estimatedSize);

        StringUtil::encodeBase64(bufferData, fullBuffer->getLength(), encodedBuffer, encodingChunkLength);

        {
            UnsafeBlock releaser;

            // add a standard shebang line as a courtesy for the unix-based systems.
            target.write(standardShebang, strlen(standardShebang), written);
            target.write(encodedHeader, strlen(encodedHeader), written);
            // and finally the encoded metadata and method
            target.write(encodedBuffer->getData(), encodedBuffer->getLength(), written);
        }
    }
}


/**
 * If restore data was base64 encoded, decode and copy the decoded data to the appropriate place in the received buffer
 *
 * @param fileName The name of the file being checked
 * @param buffer   The buffer object containing the program data. If this is a compiled program that has been string encoded, the data will be decoded in place.
 * @param metaData The pointer to the start of the saved metadata.
 *
 * @return true if this is a compiled program, false if this needs to be translated.
 */
bool ProgramMetaData::processRestoreData(RexxString *fileName, BufferClass *buffer, ProgramMetaData *&metaData)
{
    char *data = buffer->getData();
    size_t length = buffer->getDataLength();

    metaData = NULL;

    // does this start with a hash-bang?  Need to scan forward to the first
    // newline character
    if (data[0] == '#' && data[1] == '!')
    {
        data = (char *)Utilities::strnchr(data, length, '\n');
        if (data == OREF_NULL)
        {
            return false;
        }
        // step over the linend
        data++;
    }

    // pass back the pointer to the metadata location, assuming for now that
    // here is metadata.
    metaData = (ProgramMetaData *)data;

    // adjust the length for everything after the shebang
    length -= data - buffer->getData();

    // Now check in binary form of the compiled version
    if (length > strlen(compiledHeader) && strcmp(data, compiledHeader) == 0)
    {
        return true;
    }

    size_t encodedHeaderLength = strlen(encodedHeader);

    // if this is an encoded header, we need to decode this first
    if (length > encodedHeaderLength && memcmp(data, encodedHeader, encodedHeaderLength) == 0)
    {
        char *beginEncodedData = data + encodedHeaderLength;
        size_t encodedLength   = length - encodedHeaderLength;

        size_t decodedLength;

        // we now have the encoded data, decode it in place, overwriting the compiled header
        if (!StringUtil::decodeBase64(beginEncodedData, encodedLength, data, decodedLength))
        {
            reportException(Error_Program_unreadable_invalid_encoding, fileName);
        }
        return true;
    }
    return false;
}
