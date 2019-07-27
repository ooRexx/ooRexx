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
/******************************************************************************/
/* REXX Kernel                                                                */
/*                                                                            */
/* System support for FileSystem operations.                                  */
/*                                                                            */
/******************************************************************************/

#ifndef Included_SysFileSystem
#define Included_SysFileSystem

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>


#if defined __APPLE__
// avoid warning: '(f)stat64' is deprecated: first deprecated in macOS 10.6
#define stat64 stat
#define fstat64 fstat
#define open64 open
#define lstat64 lstat
#endif

#if defined(PATH_MAX)
#define MAXIMUM_PATH_LENGTH PATH_MAX + 1
#elif defined(_POSIX_PATH_MAX)
#define MAXIMUM_PATH_LENGTH _POSIX_PATH_MAX + 1
#else
#define MAXIMUM_PATH_LENGTH
#endif

#if defined(FILENAME_MAX)
#define MAXIMUM_FILENAME_LENGTH FILENAME_MAX + 1
#elif defined(_MAX_FNAME)
#define MAXIMUM_FILENAME_LENGTH _MAX_FNAME + 1
#elif defined(_POSIX_NAME_MAX)
#define MAXIMUM_FILENAME_LENGTH _POSIX_NAME_MAX + 1
#else
#define MAXIMUM_FILENAME_LENGTH 256
#endif

#define NAME_BUFFER_LENGTH (MAXIMUM_PATH_LENGTH + MAXIMUM_FILENAME_LENGTH)

class RexxString;
class FileNameBuffer;

class SysFileSystem
{
 public:
     enum
     {
         MaximumPathLength = MAXIMUM_PATH_LENGTH,
         MaximumFileNameLength = MAXIMUM_FILENAME_LENGTH,
         MaximumFileNameBuffer = MAXIMUM_PATH_LENGTH + MAXIMUM_FILENAME_LENGTH
     };

     static const char EOF_Marker;
     static const char *EOL_Marker;    // the end-of-line marker
     static const char PathDelimiter;  // directory path delimiter
     static const char NewLine;
     static const char CarriageReturn;

     static bool  searchFileName(const char *name, FileNameBuffer &fileName);
     static void  qualifyStreamName(const char *unqualifiedName, FileNameBuffer &qualifiedName);
     static bool  fileExists(const char *name);
     static bool  searchName(const char *name, const char *path, const char *extension, FileNameBuffer &resolvedName);
     static bool  searchPath(const char *name, const char *path, FileNameBuffer &resolvedName);
     static bool  primitiveSearchName(const char *name, const char *path, const char *extension, FileNameBuffer &resolvedName);
     static bool  checkCurrentFile(const char *name, FileNameBuffer &resolvedName);
     static bool  hasExtension(const char *name);
     static bool  hasDirectory(const char *name);
     static bool  canonicalizeName(FileNameBuffer &resolvedName);
     static bool  normalizePathName(const char *name, FileNameBuffer &resolvedName);
     static RexxString* extractDirectory(RexxString *file);
     static RexxString* extractExtension(RexxString *file);
     static RexxString* extractFile(RexxString *file);

     static int   deleteFile(const char *name);
     static int   deleteDirectory(const char *name);
     static bool  isDirectory(const char *name);
     static bool  isReadOnly(const char *name);
     static bool  isWriteOnly(const char *name);
     static bool  canWrite(const char *name);
     static bool  canRead(const char *name);
     static bool  isFile(const char *name);
     static bool  isLink(const char *name);
     static bool  exists(const char *name);

     static int64_t getLastModifiedDate(const char *name);
     static int64_t getLastAccessDate(const char *name);
     static bool  setLastModifiedDate(const char *name, int64_t time);
     static bool  setLastAccessDate(const char *name, int64_t time);

     static uint64_t getFileLength(const char *name);

     static bool  makeDirectory(const char *name);
     static bool  isHidden(const char *name);
     static bool  setFileReadOnly(const char *name);
     static bool  setFileWritable(const char *name);
     static bool  isCaseSensitive();
     static bool  isCaseSensitive(const char *name);
     static int   getRoots(FileNameBuffer &roots);
     static const char* getSeparator();
     static const char* getPathSeparator();
     static const char* getLineEnd();
     static bool  getCurrentDirectory(FileNameBuffer &directory);
     static bool  setCurrentDirectory(const char *directory);
     static int   copyFile(const char *fromFile, const char *toFile);
     static int   moveFile(const char *fromFile, const char *toFile);
     static bool  resolveTilde(FileNameBuffer &name);
     static const char* getPathStart(const char *name);
     static const char* getPathEnd(const char *name);
     static bool getTemporaryPath(FileNameBuffer &temporary);
};

class SysFileIterator
{
public:
    /*
     * A platform-specific class for passing file attribute data around
     * for the file iterations.
     */
     class FileAttributes
     {
      public:
         FileAttributes()
         {}

         struct stat64 findFileData;

         bool isDirectory()
         {
             return S_ISDIR(findFileData.st_mode);
         }
     };

    SysFileIterator(const char *path, const char *pattern, FileNameBuffer &buffer, bool c = false);
    ~SysFileIterator();
    void close();
    bool hasNext();
    void next(FileNameBuffer &buffer, FileAttributes &attributes);

protected:
    void findNextEntry();

    bool completed;              // the iteration completed flag
    const char *directory;       // the directory we're searching through
    struct dirent *entry;        // contains the name of the file
    struct stat64 findFileData;  // contains the file attributes
    DIR    *handle;              // the directory handle we're iterating over
    bool    caseLess;            // indicates we do caseless searches
    const char *patternSpec;     // the spec we test against
};


/**
 * A simple class to ensure an open file is closed in error situations.
 */
class AutoClose
{
 public:
     AutoClose() : value(-1)
     { };
     AutoClose(int fd) : value(fd)
     { }
     ~AutoClose()
     {
         close(false);
     }
     AutoClose & operator=(int fd)
     {
         close(false);
         value = fd;
         return *this;
     }
     operator int() const
     {
         return value;
     }
     int operator==(int fd)
     {
         return value == fd;
     }
     int close(bool returnError = true)
     {
         int closeStatus = 0;
         if (returnError)
         {
             if (value >= 0)
             {
                 closeStatus = ::close(value);
             }
         }
         else
         {
             if (value >= 0)
             {
                 int backup = errno;
                 ::close(value);
                 errno = backup;
             }
         }
         value = -1;
         return closeStatus;
     }
 private:
     int value; // >= 0 if opened
};

#endif

