/*----------------------------------------------------------------------------*/
/*                                                                            */
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


#ifndef ExternalFileNameBuffer_Included
#define ExternalFileNameBuffer_Included

#include "FileNameBuffer.hpp"
#include "oorexxapi.h"

/**
 * A class for performing safer file name resolution that is expandable if needed. Replaces the
 * use of fixed-sized buffers for file operations.
 */
class RoutineFileNameBuffer : public FileNameBuffer
{
 public:
     RoutineFileNameBuffer(RexxCallContext *c, size_t initial = 0) : context(c), FileNameBuffer(initial) { }

     void handleMemoryError()override { context->ThrowException0(Rexx_Error_System_resources); }

     // for some reason, the compiler is not recognizing the super-class operators, so
     // we reimplement them here using the class name.
     inline RoutineFileNameBuffer &operator=(const char *s)
     {
         FileNameBuffer::operator=(s);
         return *this;
     }

     inline RoutineFileNameBuffer &operator=(char *s)
     {
         FileNameBuffer::operator=(s);
         return *this;
     }

     inline RoutineFileNameBuffer &operator=(FileNameBuffer &s)
     {
         FileNameBuffer::operator=(s);
         return *this;
     }

     inline bool operator==(const char *s)
     {
         return strcmp(buffer, s) == 0;
     }


 protected:
     RexxCallContext *context;
};


/**
 * A class for performing safer file name resolution that is expandable if needed. Replaces the
 * use of fixed-sized buffers for file operations.
 */
class MethodFileNameBuffer : public FileNameBuffer
{
 public:
     MethodFileNameBuffer(RexxMethodContext *c, size_t initial = 0) : context(c), FileNameBuffer(initial) { }

     void handleMemoryError()override { context->ThrowException0(Rexx_Error_System_resources); }

     // for some reason, the compiler is not recognizing the super-class operators, so
     // we reimplement them here using the class name.
     inline MethodFileNameBuffer &operator=(const char *s)
     {
         FileNameBuffer::operator=(s);
         return *this;
     }

     inline MethodFileNameBuffer &operator=(char *s)
     {
         FileNameBuffer::operator=(s);
         return *this;
     }

     inline MethodFileNameBuffer &operator=(FileNameBuffer &s)
     {
         FileNameBuffer::operator=(s);
         return *this;
     }

     inline bool operator==(const char *s)
     {
         return strcmp(buffer, s) == 0;
     }



 protected:
     RexxMethodContext *context;
};


/**
 * Simple class for resolving qualified file names.
 */
class RoutineQualifiedName
{
 public:
     RoutineQualifiedName(RexxCallContext *c, const char *name) : qualifiedName(c)
     {
         SysFileSystem::qualifyStreamName(name, qualifiedName);
     }

     // cast conversion operators for some very common uses of protected object.
     inline operator const char *()
     {
         return (const char *)qualifiedName;
     }


 protected:
     // buffer for holding the qualified name
     RoutineFileNameBuffer qualifiedName;
};



/**
 * Simple class for resolving qualified file names.
 */
class MethodQualifiedName
{
 public:
     MethodQualifiedName(RexxMethodContext *c, const char *name) : qualifiedName(c)
     {
         SysFileSystem::qualifyStreamName(name, qualifiedName);
     }

     // cast conversion operators for some very common uses of protected object.
     inline operator const char *()
     {
         return (const char *)qualifiedName;
     }


 protected:
     // buffer for holding the qualified name
     MethodFileNameBuffer qualifiedName;
};
#endif

