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
#ifndef ProgramMetaData_Included
#define ProgramMetaData_Included

#include "SystemInterpreter.hpp"
#include "LanguageLevel.hpp"

class BufferClass;
class SysFile;
class RoutineClass;

class ProgramMetaData
{
public:
    void *operator new (size_t size, BufferClass *buff);
    void operator delete (void *p) { SystemInterpreter::releaseResultMemory(p); }

    ProgramMetaData(LanguageLevel, BufferClass *);
    ProgramMetaData(LanguageLevel, size_t);

    size_t getDataSize();
    size_t getHeaderSize();
    char *getImageData();
    size_t getImageSize() { return imageSize; }

    bool validate(RexxString *fileName);
    void write(SysFile &target, BufferClass *program, bool encode);

    static RoutineClass* restore(RexxString *fileName, BufferClass *buffer);
    static bool processRestoreData(RexxString *fileName, BufferClass *data, ProgramMetaData *&metaData);

    static const size_t encodingChunkLength = 72;    // the chunking to use with encoded data

protected:
    enum
    {
        MAGICNUMBER = 11111,           // remains constant from release-to-release
        METAVERSION = 42               // gets updated when internal form changes
    };


    char fileTag[16];                  // special header for file tagging
    uint16_t       magicNumber;        // special tag to indicate good meta data
    uint16_t       imageVersion;       // version identifier for validity
    uint16_t       wordSize;           // size of a word
    uint16_t       bigEndian;          // true if this is a big-endian platform
    LanguageLevel  requiredLevel;      // required language level for execution
    uint32_t       reserved;           // padding to bring imageSize to a 64-bit boundary
    size_t         imageSize;          // size of the image
    char           imageData[4];       // the copied image data
};

#endif
