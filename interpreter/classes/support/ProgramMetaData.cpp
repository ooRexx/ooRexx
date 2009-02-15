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


#include "RexxCore.h"
#include "ProgramMetaData.hpp"
#include "BufferClass.hpp"
#include "Interpreter.hpp"
#include <stdio.h>
#include <fcntl.h>


const char * compiledHeader = "/**/@REXX";


/**
 * Allocate a combined metadata object with the flattened
 * program data after it.
 *
 * @param size   The size of the object
 * @param buff   The appended buffer.
 *
 * @return The storage allocated for the new instance.
 */
void *ProgramMetaData::operator new (size_t size, RexxBuffer *buff)
{
    // allocate a new buffer for this
    return SystemInterpreter::allocateResultMemory(buff->getDataLength() + size - sizeof(char[4]));
}


/**
 * Initialize the meta data directly from a buffer.
 *
 * @param image  The image buffer.
 */
ProgramMetaData::ProgramMetaData(RexxBuffer *image)
{
    // add the leading header
    strcpy(fileTag, compiledHeader);
    // fill in the version specifics and the hardward architecture type
    magicNumber = MAGICNUMBER;
    imageVersion = METAVERSION;
    // this is the number of bits in a word
    wordSize = Interpreter::getWordSize();
    bigEndian = Interpreter::isBigEndian();

    RexxString *versionNumber = Interpreter::getVersionNumber();
    strncpy(rexxVersion, versionNumber->getStringData(), sizeof(rexxVersion));

    // copy in the image information
    imageSize = image->getDataLength();
    memcpy(imageData, image->getData(), imageSize);
}


/**
 * Initialize program metadata for a specific size image.
 *
 * @param size   The size of the program data.
 */
ProgramMetaData::ProgramMetaData(size_t size)
{
    // add the leading header
    strcpy(fileTag, compiledHeader);
    // fill in the version specifics and the hardward architecture type
    magicNumber = MAGICNUMBER;
    imageVersion = METAVERSION;
    // this is the number of bits in a word
    wordSize = Interpreter::getWordSize();
    bigEndian = Interpreter::isBigEndian();

    RexxString *versionNumber = Interpreter::getVersionNumber();
    strncpy(rexxVersion, versionNumber->getStringData(), sizeof(rexxVersion));

    // copy in the image information
    imageSize = size;
}


/**
 * Initialized a default metadata descriptor.
 */
ProgramMetaData::ProgramMetaData()
{
    // this is for the purposes of reading in...force everything to zero.
    magicNumber = 0;
    imageVersion = 0;
    // this is the number of bits in a word
    wordSize = 0;
    bigEndian = 0;
    imageSize = 0;
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
 * Extract the following data as a RexxBuffer object.
 *
 * @return The extracted buffer object.
 */
RexxBuffer *ProgramMetaData::extractBufferData()
{
    return new_buffer(imageData, imageSize);
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
 * Validate that this saved program image is valid for this
 * interpreter.
 *
 * @param badVersion Indicates whether this is a version
 *                   failure.
 *
 * @return true if this is good data, false otherwise.
 */
bool ProgramMetaData::validate(bool &badVersion)
{
    badVersion = false;
    // we always add the compiled program tag to the front
    if (strcmp(fileTag, compiledHeader) != 0)
    {
        return false;
    }
    // check all of the version specifics
    if (magicNumber != MAGICNUMBER || imageVersion != METAVERSION || wordSize != Interpreter::getWordSize() ||
        (bigEndian != 0) != Interpreter::isBigEndian())
    {
        // this is a version failure, mark it as such
        badVersion = true;
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
void ProgramMetaData::write(FILE *handle, RexxBuffer *program)
{
    fwrite(this, 1, getHeaderSize(), handle);
    /* and finally the flattened method  */
    fwrite(program->getData(), 1, program->getDataLength(), handle);
}


/**
 * Read the program meta data and the image data from a file, with
 * image validation.
 *
 * @param handle The input file handle.
 *
 * @return A RexxBuffer instance containing the program data, or OREF_NULL
 *         if the file is not a valid image.
 */
RexxBuffer *ProgramMetaData::read(RexxString *fileName, FILE *handle)
{
    bool badVersion = false;

    // now read the control info
    fread((char *)this, 1, getHeaderSize(), handle);
    // validate all of the meta information
    if (!validate(badVersion))
    {
        // if this failed because of the version signature, we need to raise an error now.
        if (badVersion)
        {
            fclose(handle);                    /* close the file                    */
            reportException(Error_Program_unreadable_version, fileName);
        }

        // if it didn't validate, it might be because we have a unix-style "hash bang" line at the
        // beginning of the file.  The first read in bit has a "#!", then we need to read
        // beyond the first linend and try again.
        if (fileTag[0] == '#' && fileTag[1] == '!')
        {
            // back to the start (ok, just past the hash bang)
            fseek(handle, 2, SEEK_SET);

            while (true)
            {
                if (fread(fileTag, 1, 1, handle) <= 0)
                {
                    fclose(handle);
                    return OREF_NULL;
                }
                // if we hit a newline, this is our stopping point.
                // NB:  this works with both \n and \r\n sequences.
                if (fileTag[0] == '\n')
                {
                    break;
                }
                // ok, try to read the control information one more time.
                // if this doesn't work, no point in being pushy about it.
                fread((char *)this, 1, getHeaderSize(), handle);
                // validate all of the meta information
                if (!validate(badVersion))
                {
                    fclose(handle);                    /* close the file                    */
                    // if because of a bad version sig, we can close now
                    if (badVersion)
                    {
                        reportException(Error_Program_unreadable_version, fileName);
                    }
                    return OREF_NULL;
                }
            }
        }
    }
    RexxBuffer *buffer = new_buffer(imageSize);
    fread(buffer->getData(), 1, imageSize, handle);
    return buffer;
}
