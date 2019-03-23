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


#ifndef FileNameBuffer_Included
#define FileNameBuffer_Included

#include <cstring>
#include "SysFileSystem.hpp"

/**
 * A class for performing safer file name resolution that is expandable if needed. Replaces the
 * use of fixed-sized buffers for file operations.
 *
 * NOTE: This can only be used within code that runs
 * while the interpreter has control.
 */
class FileNameBuffer
{
 public:
     FileNameBuffer(size_t initial = 0);
     FileNameBuffer(const FileNameBuffer &o);
     inline ~FileNameBuffer()
     {
         if (buffer != NULL)
         {
             delete buffer;
             buffer = NULL;
         }
     }

     void init(size_t initial);

     virtual void handleMemoryError();

     size_t capacity() { return bufferSize; }
     size_t length() { return strlen(buffer); }

     // cast conversion operators
     inline operator char *()
     {
         return buffer;
     }

     // cast conversion operators
     inline operator const char *()
     {
         return (const char *)buffer;
     }

     void ensureCapacity(size_t size);
     inline void ensureCapacity(const char *add) { ensureCapacity(strlen(buffer) + strlen(add) + 1); }
     inline void expandCapacity(size_t c)  { ensureCapacity(bufferSize + c); }
     inline void shiftLeft(size_t l)
     {
         size_t len = length();
         // longer than the length? This becomes a null string
         if (l > len)
         {
             buffer[0] = '\0';
         }
         else
         {
             // move the remainder plus the null.
             memmove(buffer, buffer + l, len - l + 1);
         }
     }

     inline void truncate(size_t l)
     {
         // longer than the length? This is a nop
         if (l <= length())
         {
             // add the null terminator at the new position
             buffer[l] = '\0';
         }
     }

     inline void empty()
     {
         buffer[0] = '\0';
     }

     inline FileNameBuffer &operator=(const char *s)
     {
         ensureCapacity(strlen(s) + 1);
         strncpy(buffer, s, bufferSize);
         return *this;
     }

     inline FileNameBuffer &operator=(const FileNameBuffer &s)
     {
         // prevent self assignment
         if (this == &s)
         {
             return *this;
         }

         const char *str = (const char *)s.buffer;

         ensureCapacity(strlen(str) + 1);
         strncpy(buffer, str, bufferSize);
         return *this;
     }

     inline FileNameBuffer &operator=(char *s)
     {
         ensureCapacity(strlen(s) + 1);
         strncpy(buffer, s, bufferSize);
         return *this;
     }

     inline FileNameBuffer &operator+=(const char *s)
     {
         ensureCapacity(s);
         strncat(buffer, s, bufferSize);
         return *this;
     }

     inline FileNameBuffer &operator+=(char *s)
     {
         ensureCapacity(s);
         strncat(buffer, s, bufferSize);
         return *this;
     }


     inline FileNameBuffer &operator+=(char c)
     {
         size_t currentLen = length();
         ensureCapacity(currentLen + 2);

         buffer[currentLen] = c;
         buffer[currentLen + 1] = '\0';
         return *this;
     }


     inline FileNameBuffer &operator+=(FileNameBuffer &s)
     {
         return *this += (const char *)s;
     }



     inline FileNameBuffer& set(const char *s, size_t l)
     {
         ensureCapacity(l + 1);
         memcpy(buffer, s, l);
         buffer[l] = '\0';
         return *this;
     }

     inline FileNameBuffer& append(const char *s, size_t l)
     {
         size_t currentLength = length();
         ensureCapacity(currentLength + 1 + 1);
         memcpy(buffer + currentLength, s, l);
         buffer[currentLength + l] = '\0';
         return *this;
     }

     inline bool operator==(const char *s)
     {
         return strcmp(buffer, s) == 0;
     }

     inline bool operator!=(const char *s)
     {
         return strcmp(buffer, s) != 0;
     }


     // this is a mutable request, so we need to ensure the position is within the
     // current buffer size
     inline char & at(size_t pos)
     {
         ensureCapacity(pos + 1);
         return *(buffer + pos);
     }

     inline bool startsWith(char c)
     {
         return buffer[0] == c;
     }

     inline bool startsWith(const char *s)
     {
         size_t slen = strlen(s);
         size_t len = length();
         if (slen > len)
         {
             return false;
         }

         return strcmp(buffer, s) == 0;
     }

     inline bool endsWith(char c)
     {
         return length() > 0 && buffer[length() - 1] == c;
     }

     inline bool endsWith(const char *s)
     {
         size_t slen = strlen(s);
         size_t len = length();
         if (slen > len)
         {
             return false;
         }

         return strcmp(buffer + (len - slen), s) == 0;
     }

     inline void addFinalPathDelimiter()
     {
         if (!endsWith(SysFileSystem::PathDelimiter))
         {
             *this += SysFileSystem::PathDelimiter;
         }
     }

 protected:
     char *buffer;                 // the current buffer
     size_t bufferSize;            // the current buffer size

};


/**
 * Simple class for resolving qualified file names.
 *
 * NOTE: This can only be used within code that runs
 * while the interpreter has control.
 */
class QualifiedName
{
 public:
     QualifiedName(const char *name)
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
     FileNameBuffer qualifiedName;

};


/**
 * A version of a FileNameBuffer that uses a parent FileNameBuffer for handling
 * allocation error.
 */
class AutoFileNameBuffer : public FileNameBuffer
{
 public:
     AutoFileNameBuffer(FileNameBuffer &b) : parent(b), FileNameBuffer() { }

     // for some reason, the compiler is not recognizing the super-class operators, so
     // we reimplement them here using the class name.
     inline AutoFileNameBuffer &operator=(const char *s)
     {
         FileNameBuffer::operator=(s);
         return *this;
     }

     inline AutoFileNameBuffer &operator=(char *s)
     {
         FileNameBuffer::operator=(s);
         return *this;
     }

     inline AutoFileNameBuffer &operator=(char s)
     {
         FileNameBuffer::operator=(s);
         return *this;
     }

     inline AutoFileNameBuffer &operator+=(FileNameBuffer &s)
     {
         FileNameBuffer::operator+=(s);
         return *this;
     }

     inline AutoFileNameBuffer &operator+=(const char *s)
     {
         FileNameBuffer::operator+=(s);
         return *this;
     }

     inline AutoFileNameBuffer &operator+=(char *s)
     {
         FileNameBuffer::operator+=(s);
         return *this;
     }

     inline AutoFileNameBuffer &operator+=(char s)
     {
         FileNameBuffer::operator+=(s);
         return *this;
     }

     void handleMemoryError() override { parent.handleMemoryError(); };

 private:
     FileNameBuffer &parent;
};
#endif
