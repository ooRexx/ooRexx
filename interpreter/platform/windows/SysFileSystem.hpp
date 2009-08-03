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

    static const char *getTempFileName();
    static bool  searchFileName(const char * name, char *fullName);
    static void  qualifyStreamName(const char *unqualifiedName, char *qualifiedName, size_t bufferSize);
    static bool  fileExists(const char *name);
    static bool  hasExtension(const char *name);
    static RexxString *extractDirectory(RexxString *file);
    static RexxString *extractExtension(RexxString *file);
    static RexxString *extractFile(RexxString *file);
    static bool  searchName(const char *name, const char *path, const char *extension, char *resolvedName);
    static bool  primitiveSearchName(const char *name, const char *path, const char *extension, char *resolvedName);
    static bool  checkCurrentFile(const char *name, char *resolvedName);
    static bool  searchPath(const char *name, const char *path, const char *extension, char *resolvedName);
    static void  getLongName(char *fullName, size_t size);
    static bool  findFirstFile(const char *name);
    static bool  deleteFile(const char *name);
    static bool  deleteDirectory(const char *name);
    static bool  isDirectory(const char *name);
    static bool  isReadOnly(const char *name);
    static bool  isWriteOnly(const char *name);
    static bool  isFile(const char *name);
    static bool  exists(const char *name);
    static int64_t getLastModifiedDate(const char *name);
    static int64_t getFileLength(const char *name);
    static bool  makeDirectory(const char *name);
    static bool  moveFile(const char *oldName, const char *newName);
    static bool  isHidden(const char *name);
    static bool  setLastModifiedDate(const char *name, int64_t time);
    static bool  setFileReadOnly(const char *name);
    static bool  isCaseSensitive();
    static int   getRoots(char *roots);
    static const char *getSeparator();
    static const char *getPathSeparator();
};

class SysFileIterator
{
public:
    SysFileIterator(const char *pattern);
    ~SysFileIterator();
    void close();
    bool hasNext();
    void next(char *buffer);
protected:
    bool completed;       // the iteration completed flag
    HANDLE handle;        // The handle for the FindFirst operation
    WIN32_FIND_DATA findFileData;
};

#endif

