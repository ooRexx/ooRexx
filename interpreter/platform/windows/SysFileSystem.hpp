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
/* REXX Kernel                                              SysFileSystem.hpp */
/*                                                                            */
/* System support for FileSystem operations.                                  */
/*                                                                            */
/******************************************************************************/

#ifndef Included_SysFileSystem
#define Included_SysFileSystem

#include "windows.h"
#include <stdio.h>

class RexxString;
class FileNameBuffer;

class SysFileSystem
{
 public:
     enum
     {
         MaximumPathLength = MAX_PATH,
         MaximumFileNameLength = FILENAME_MAX,
         MaximumFileNameBuffer = MAX_PATH + FILENAME_MAX
     };

     static int stdinHandle;
     static int stdoutHandle;
     static int stderrHandle;

     static const char EOF_Marker;
     static const char *EOL_Marker;          // the end-of-line marker
     static const char PathDelimiter;        // directory path delimiter
     static const char NewLine;
     static const char CarriageReturn;

     static bool  searchFileName(const char *name, FileNameBuffer &fullName);
     static bool  searchOnPath(const char *name, const char *path, const char *extension, FileNameBuffer &resolvedName);
     static bool  searchPath(const char *name, const char *path, FileNameBuffer &resolvedName);
     static void  qualifyStreamName(const char *unqualifiedName, FileNameBuffer &qualifiedName);
     static bool  getFullPathName(const char *name, FileNameBuffer &resolvedName);
     static bool  fileExists(const char *name);
     static bool  hasExtension(const char *name);
     static bool  hasDirectory(const char *name);
     static RexxString* extractDirectory(RexxString *file);
     static RexxString* extractExtension(RexxString *file);
     static RexxString* extractFile(RexxString *file);
     static bool  searchName(const char *name, const char *path, const char *extension, FileNameBuffer &resolvedName);
     static bool  primitiveSearchName(const char *name, const char *path, const char *extension, FileNameBuffer &resolvedName);
     static bool  checkCurrentFile(const char *name, FileNameBuffer &resolvedName);
     static void  getLongName(FileNameBuffer &name);
     static bool  findFirstFile(const char *name);
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

     static int64_t getFileLength(const char *name);
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
     static const char* getPathStart(const char *name);
     static const char* getPathEnd(const char *name);
     static bool getTemporaryPath(FileNameBuffer &temporary);
};

class SysFileIterator
{
 public:
     /**
      * A platform-specific class for passing file attribute data around
      * for the file iterations.
      */
     class FileAttributes
     {
      public:
          FileAttributes() {}

          WIN32_FIND_DATA findFileData;

          bool isDirectory()
          {
              return (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
          }
     };

     SysFileIterator(const char *path, const char *pattern, FileNameBuffer &buffer, bool c = false);
     ~SysFileIterator();
     void close();
     bool hasNext();
     void next(FileNameBuffer &buffer, FileAttributes &attributes);

 protected:

     void findNextEntry();
     void filterShortNames();

     const char *fileSpec; // the spec we're searching against
     bool completed;       // the iteration completed flag
     HANDLE handle;        // The handle for the FindFirst operation
                           // the full file data available for the next file
     WIN32_FIND_DATA findFileData;
};


/**
 * A simple implementation to ensure the Windows error mode gets reset on a return.
 */
class AutoErrorMode
{
 public:
     AutoErrorMode(UINT m) { errorMode = SetErrorMode(SEM_FAILCRITICALERRORS); }
     ~AutoErrorMode() { SetErrorMode(SEM_FAILCRITICALERRORS); }
 private:
     UINT errorMode;   // the old error mode
};



#endif

