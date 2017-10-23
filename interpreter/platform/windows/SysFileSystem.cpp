/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                              SysFileSystem.cpp */
/*                                                                            */
/* Windows implementation of the SysFileSystem class.                         */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "SysFileSystem.hpp"
#include "ActivityManager.hpp"

int SysFileSystem::stdinHandle = 0;
int SysFileSystem::stdoutHandle = 1;
int SysFileSystem::stderrHandle = 2;

const char SysFileSystem::EOF_Marker = 0x1a;    // the end-of-file marker
const char *SysFileSystem::EOL_Marker = "\r\n"; // the end-of-line marker
const char SysFileSystem::PathDelimiter = '\\'; // directory path delimiter

/**
 * Search for a given filename, returning the fully
 * resolved name if it is found.
 *
 * @param name     The input search name.
 * @param fullName The returned fully resolved name.
 *
 * @return True if the file was located, false otherwise.
 */
bool SysFileSystem::searchFileName(const char *name, char *fullName)
{
    DWORD dwFileAttrib;                  // file attributes
    LPTSTR ppszFilePart=NULL;            // file name only in buffer
    UINT   errorMode;

    size_t nameLength = strlen(name);

    // if name is tool small or big, this cannot be a valid name.
    if (nameLength < 1 || nameLength > MAX_PATH)
    {
        return false;
    }
    // now try for original name
    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    if (GetFullPathName(name, MAX_PATH, (LPTSTR)fullName, &ppszFilePart))
    {
        // if this is a good name, then make sure this really exists and it's not a directory.
        // make sure it's not a directory
        if (-1 != (dwFileAttrib=GetFileAttributes((LPTSTR)fullName)) && (dwFileAttrib != FILE_ATTRIBUTE_DIRECTORY))
        {
            SetErrorMode(errorMode);
            return true;
        }
    }
    // search on the path
    if ( SearchPath(NULL, (LPCTSTR)name, NULL, MAX_PATH, (LPTSTR)fullName, &ppszFilePart) )
    {
        // and again, we need this to be a real file.
        if (-1 != (dwFileAttrib=GetFileAttributes((LPTSTR)fullName)) && (dwFileAttrib != FILE_ATTRIBUTE_DIRECTORY))
        {
            SetErrorMode(errorMode);
            return true;
        }
    }

    SetErrorMode(errorMode);
    return false;
}


/**
 * Generate a temporary file name.
 *
 * @return The string name of the temporary file.
 */
const char *SysFileSystem::getTempFileName()
{
    return tmpnam(NULL);
}


/**
 * Generate a fully qualified stream name.
 *
 * @param unqualifiedName
 *                   The starting name.
 * @param qualifiedName
 *                   The fully expanded and canonicalized file name.
 * @param bufferSize
 */
void SysFileSystem::qualifyStreamName(const char *unqualifiedName, char *qualifiedName, size_t bufferSize)
{
    LPTSTR  lpszLastNamePart;
    UINT errorMode;

    // If already expanded, there is nothing more to do.
    if (qualifiedName[0] != '\0')
    {
        return;
    }

    // get the fully expanded name
    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    // similar to the Unix behaviour, we return the current directory
    // if 'unqualifiedName' is the null string
    DWORD length = GetFullPathName((unqualifiedName[0] == '\0') ? "." : unqualifiedName,
                               (DWORD)bufferSize, qualifiedName, &lpszLastNamePart);
    if ((length == 0) || (length >= bufferSize))
    {
      // if GetFullPathName() failed or would need a larger buffer, return null string 
      qualifiedName[0] = '\0';
    }
    else
    {
      // qualifying a Windows device like "aux", "con" or "nul" (even when written in
      // the form "xxx:" or /dir/xxx" etc.) will return the device name with a leading
      // Win32 device namespace prefix "\\.\", e. g. QUALIFY("nul") --> "\\.\nul"
      // (see https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247%28v=vs.85%29.aspx#win32_device_namespaces)
      // the downside of a path with such a prefix is, that .File methods like exists(),
      // isFile(), canRead(), or canWrite() will unconditionally return .false for it.
      // To make things simple, we just remove such a prefix, if it hasn't already
      // been there in 'unqualifiedName'
      if ((strncmp(unqualifiedName, "\\\\.\\", 4) != 0) &&  // didn't start with "\\.\"
          (strncmp(qualifiedName, "\\\\.\\", 4) == 0))      // starts with "\\.\"
      {
        memmove(qualifiedName, &qualifiedName[4], length - 4 + 1); // remove leading "\\.\"
      }
    }
    SetErrorMode(errorMode);
}


/**
 * Perform a "find first" operation for a file.
 *
 * @param name   the target name (may include wildcard characters)
 *
 * @return true if a file was located, false if not.
 */
bool SysFileSystem::findFirstFile(const char *name)
{
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    UINT errorMode;

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    FindHandle = FindFirstFile(name, &FindData);
    SetErrorMode(errorMode);

    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(FindHandle);
        if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
            || (FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
            || (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

/**
 * Check to see if a file with a given name exists.
 *
 * @param name   The name to check.
 *
 * @return True if the file exists, false otherwise.
 */
bool SysFileSystem::fileExists(const char *name)
{
    DWORD dwAttrib = GetFileAttributes(name);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
         !((dwAttrib & FILE_ATTRIBUTE_SYSTEM)
          || (dwAttrib & FILE_ATTRIBUTE_HIDDEN)
          || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)));
}



/**
 * Extract directory information from a file name.
 *
 * @param file   The input file name.  If this represents a real source file,
 *               this will be fully resolved.
 *
 * @return The directory portion of the file name.  If the file name
 *         does not include a directory portion, then OREF_NULL is returned.
 */
RexxString *SysFileSystem::extractDirectory(RexxString *file)
{
    const char *pathName = file->getStringData();
    const char *endPtr = pathName + file->getLength() - 1;

    // scan backwards looking for a directory delimiter.  This name should
    // be fully qualified, so we don't have to deal with drive letters
    while (pathName < endPtr)
    {
        // find the first directory element?
        if (*endPtr == '\\')
        {
            // extract the directory information, including the final delimiter
            // and return as a string object.
            return new_string(pathName, endPtr - pathName + 1);
        }
        endPtr--;
    }
    return OREF_NULL;      // not available
}


/**
 * Extract extension information from a file name.
 *
 * @param file   The input file name.  If this represents a real source file,
 *               this will be fully resolved.
 *
 * @return The extension portion of the file name.  If the file
 *         name does not include an extension portion, then
 *         OREF_NULL is returned.
 */
RexxString *SysFileSystem::extractExtension(RexxString *file)
{
    const char *pathName = file->getStringData();
    const char *endPtr = pathName + file->getLength() - 1;

    // scan backwards looking for a directory delimiter.  This name should
    // be fully qualified, so we don't have to deal with drive letters
    while (pathName < endPtr)
    {
        // find the first directory element?
        if (*endPtr == '\\')
        {
            return OREF_NULL;    // found a directory portion before an extension...we're extensionless
        }
        // is this the extension dot?
        else if (*endPtr == '.')
        {
            // return everything from the period on.  Keeping the period on is a convenience.
            return new_string(endPtr);
        }
        endPtr--;
    }
    return OREF_NULL;      // not available
}


/**
 * Extract file information from a file name.
 *
 * @param file   The input file name.  If this represents a real source file,
 *               this will be fully resolved.
 *
 * @return The file portion of the file name.  If the file name
 *         does not include a directory portion, then the entire
 *         string is returned
 */
RexxString *SysFileSystem::extractFile(RexxString *file)
{
    const char *pathName = file->getStringData();
    const char *endPtr = pathName + file->getLength() - 1;

    // scan backwards looking for a directory delimiter.  This name should
    // be fully qualified, so we don't have to deal with drive letters
    while (pathName < endPtr)
    {
        // find the first directory element?
        if (*endPtr == '\\')
        {
            // extract the directory information, including the final delimiter
            // and return as a string object.
            return new_string(endPtr);
        }
        endPtr--;
    }
    return file;     // this is all filename
}


/**
 * Test if a filename has an extension.
 *
 * @param name   The name to check.
 *
 * @return true if an extension was found on the file, false if there
 *         is no extension.
 */
bool SysFileSystem::hasExtension(const char *name)
{
    const char *endPtr = name + strlen(name) - 1;

    // scan backwards looking for a directory delimiter.  This name should
    // be fully qualified, so we don't have to deal with drive letters
    while (name < endPtr)
    {
        // find the first directory element?
        if (*endPtr == '/')
        {
            return false;        // found a directory portion before an extension...we're extensionless
        }
        // is this the extension dot?
        else if (*endPtr == '.')
        {
            // return everything from the period on.  Keeping the period on is a convenience.
            return true;
        }
        endPtr--;
    }
    return false;          // not available
}


/**
 * Do a search for a single variation of a filename.
 *
 * @param name      The name to search for.
 * @param directory A specific directory to look in first (can be NULL).
 * @param extension A potential extension to add to the file name (can be NULL).
 * @param resolvedName
 *                  The buffer used to return the resolved file name.
 *
 * @return true if the file was located.  A true returns indicates the
 *         resolved file name has been placed in the provided buffer.
 */
bool SysFileSystem::searchName(const char *name, const char *path, const char *extension, char *resolvedName)
{
    UnsafeBlock releaser;
    return primitiveSearchName(name, path, extension, resolvedName);
}


/**
 * Do a search for a single variation of a filename.
 *
 * NOTE:  This version does not do anything with the
 * kernel lock, so it is callable before the first activity
 * is set up.
 *
 * @param name      The name to search for.
 * @param path
 * @param extension A potential extension to add to the file name (can be NULL).
 * @param resolvedName
 *                  The buffer used to return the resolved file name.
 *
 * @return true if the file was located.  A true returns indicates the
 *         resolved file name has been placed in the provided buffer.
 */
bool SysFileSystem::primitiveSearchName(const char *name, const char *path, const char *extension, char *resolvedName)
{
    // this is for building a temporary name
    char       tempName[MAX_PATH + 2];

    // construct the search name, potentially adding on an extension
    strncpy(tempName, name, sizeof(tempName));
    if (extension != NULL)
    {
        strncat(tempName, extension, sizeof(tempName) - strlen(tempName) - 1);
    }

    *resolvedName = '\0';
    // if this appears to be a fully qualified name, then check it as-is and
    // quit.  The path searches might give incorrect results if performed with such
    // a name and this should only check on the raw name.
    if (hasDirectory(tempName))
    {
        // check the file as is first
        return checkCurrentFile(tempName, resolvedName);
    }

    if (searchPath(name, path, extension, resolvedName))
    {
        return true;
    }
    return false;
}


/**
 * Test if a filename has a directory portion
 *
 * @param name   The name to check.
 *
 * @return true if a directory was found on the file, false if
 *         there is no directory.
 */
bool SysFileSystem::hasDirectory(const char *name)
{
    // hasDirectory() means we have enough absolute directory
    // information at the beginning to bypass performing path searches.
    // We really only need to look at the first character.
    return name[0] == '\\' || name[0] == '.' || name[2] == ':';
}



/**
 * Try to locate a file using just the raw name passed in, as
 * opposed to searching along a path for the name.
 *
 * @param name   The name to use for the search.
 *
 * @return An RexxString version of the file name, iff the file was located. Returns
 *         OREF_NULL if the file did not exist.
 */
bool SysFileSystem::checkCurrentFile(const char *name, char *resolvedName)
{
    size_t nameLength = strlen(name); /* get length of incoming name       */

    // make sure we have a valid length for even searching
    if (nameLength < 1 || nameLength > MAX_PATH)
    {
        return false;
    }

    // check the name in the current directory
    unsigned int errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    LPTSTR ppszFilePart=NULL;            // file name only in buffer

    if (GetFullPathName(name, MAX_PATH, (LPTSTR)resolvedName, &ppszFilePart))
    {
        DWORD fileAttrib = GetFileAttributes((LPTSTR)resolvedName);

        // if this is a real file vs. a directory, make sure we return
        // the long name value in the correct casing
        if (fileAttrib != INVALID_FILE_ATTRIBUTES && fileAttrib != FILE_ATTRIBUTE_DIRECTORY)
        {
            getLongName(resolvedName, MAX_PATH);
            SetErrorMode(errorMode);
            return true;
        }
    }
    return false;        // not found
}


/**
 * Do a path search for a file.
 *
 * @param name      The name to search for.
 * @param path      The search path to use.
 * @param extension Any extension that should be added to the search (can be NULL).
 * @param resolvedName
 *                  A buffer used for returning the resolved name.
 *
 * @return Returns true if the file was located.  If true, the resolvedName
 *         buffer will contain the returned name.
 */
bool SysFileSystem::searchPath(const char *name, const char *path, const char *extension, char *resolvedName)
{
    unsigned int errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    LPTSTR ppszFilePart=NULL;            // file name only in buffer
    if (SearchPath((LPCTSTR)path, (LPCTSTR)name, (LPCTSTR)extension, MAX_PATH, (LPTSTR)resolvedName, &ppszFilePart))
    {
        DWORD fileAttrib = GetFileAttributes((LPTSTR)resolvedName);

        // if this is a real file vs. a directory, make sure we return
        // the long name value in the correct casing
        if (fileAttrib != INVALID_FILE_ATTRIBUTES && fileAttrib != FILE_ATTRIBUTE_DIRECTORY)
        {
            getLongName(resolvedName, MAX_PATH);
            SetErrorMode(errorMode);
            return true;
        }
    }
    return false;        // not found
}


/**
 * Get the actual name value of a located file, in the exact case
 * used on the harddrive.
 *
 * @param fullName The buffer used for the name.
 * @param size     The size of the buffer.
 */
void SysFileSystem::getLongName(char *fullName, size_t size)
{
    char           *p;

    if (size >= MAX_PATH)
    {
        DWORD length = GetLongPathName(fullName, fullName, (DWORD)size);

        if ( 0 < length && length <= size )
        {
            WIN32_FIND_DATA findData;
            HANDLE hFind = FindFirstFile(fullName, &findData);
            if ( hFind != INVALID_HANDLE_VALUE )
            {
                p = strrchr(fullName, '\\');
                if ( p )
                {
                    strcpy(++p, findData . cFileName);
                }
                FindClose(hFind);
            }
        }
    }
    return;
}


/**
 * Delete a file from the file system.
 *
 * @param name   The fully qualified name of the file.
 *
 * @return The return code from the delete operation.
 */
bool SysFileSystem::deleteFile(const char *name)
{
    return DeleteFile(name) != 0;
}

/**
 * Delete a directory from the file system.
 *
 * @param name   The name of the target directory.
 *
 * @return The return code from the delete operation.
 */
bool SysFileSystem::deleteDirectory(const char *name)
{
    return RemoveDirectory(name) != 0;
}


/**
 * Test if a given file name is for a directory.
 *
 * @param name   The target name.
 *
 * @return true if the file is a directory, false for any other
 *         type of entity.
 */
bool SysFileSystem::isDirectory(const char *name)
{
    DWORD dwAttrs = GetFileAttributes(name);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_DIRECTORY);
}


/**
 * Test is a file is read only.
 *
 * @param name   The target file name.
 *
 * @return true if the file is marked as read-only.
 */
bool SysFileSystem::isReadOnly(const char *name)
{
    DWORD dwAttrs = GetFileAttributes(name);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_READONLY);
}


/**
 * Test if a file is marked as write-only.
 *
 * @param name   The target file name.
 *
 * @return true if the file is only writeable.  false if read
 *         operations are permitted.
 */
bool SysFileSystem::isWriteOnly(const char *name)
{
    // attempt to open for read...if this fails, this is write only
    HANDLE handle = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return true;
    }

    CloseHandle(handle);
    return false;
}


/**
 * Test if a give file name is for a real file (not
 * a directory).
 *
 * @param name   The target file name.
 *
 * @return true if the file is a real file, false if some other
 *         filesystem entity.
 */
bool SysFileSystem::isFile(const char *name)
{
    DWORD dwAttrs = GetFileAttributes(name);
    return (dwAttrs != 0xffffffff) && (dwAttrs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == 0;
}


/**
 * Test if a file exists using a fully qualified name.
 *
 * @param name   The target file name.
 *
 * @return True if the file exists, false if it is unknown.
 */
bool SysFileSystem::exists(const char *name)
{
    DWORD dwAttrs = GetFileAttributes(name);
    return (dwAttrs != 0xffffffff);
}


/**
 * Get the last modified file date as a file time value.
 *
 * @param name   The target name.
 *
 * @return the file time value for the modified date, or -1 for any
 *         errors.
 */
int64_t SysFileSystem::getLastModifiedDate(const char *name)
{
    HANDLE newHandle = CreateFile(name, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (newHandle == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    FILETIME lastWriteGetTime, lastWriteTime;
    if (!(GetFileTime(newHandle, NULL, NULL, &lastWriteGetTime) &&
          FileTimeToLocalFileTime(&lastWriteGetTime, &lastWriteTime)))
    {
        CloseHandle(newHandle);
        return -1;
    }

    /*
    * Search MSDN for 'Converting a time_t Value to a File Time' for following implementation.
    */
    int64_t tempResult = ((int64_t) lastWriteTime.dwHighDateTime << (int64_t)32) | (int64_t) lastWriteTime.dwLowDateTime;
    int64_t result = (tempResult - 116444736000000000) / 10000000;

    CloseHandle(newHandle);
    return result;
}


/**
 * Retrieve the size of a file.
 *
 * @param name   The name of the target file.
 *
 * @return the 64-bit file size.
 */
int64_t SysFileSystem::getFileLength(const char *name)
{
    WIN32_FILE_ATTRIBUTE_DATA stat;

    if (GetFileAttributesEx(name, GetFileExInfoStandard, &stat) == 0)
    {
        return 0;
    }

    int64_t result = ((int64_t)stat.nFileSizeHigh) << 32;
    result += (int64_t)stat.nFileSizeLow;
    return result;
}


/**
 * Create a directory in the file system.
 *
 * @param name   The target name.
 *
 * @return The success/failure flag.
 */
bool SysFileSystem::makeDirectory(const char *name)
{
    return CreateDirectory(name, 0) != 0;
}


/**
 * Move (rename) a file.
 *
 * @param oldName The name of an existing file.
 * @param newName The new file name.
 *
 * @return A success/failure flag.
 */
bool SysFileSystem::moveFile(const char *oldName, const char *newName)
{
    return MoveFile(oldName, newName) != 0;
}


/**
 * Test if a given file or directory is hidden.
 *
 * @param name   The target name.
 *
 * @return true if the file or directory is hidden, false otherwise.
 */
bool SysFileSystem::isHidden(const char *name)
{
    DWORD dwAttrs = GetFileAttributes(name);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_HIDDEN);
}


/**
 * Set the last modified date for a file.
 *
 * @param name   The target name.
 * @param time   The new file time.
 *
 * @return true if the filedate was set correctly, false otherwise.
 */
bool SysFileSystem::setLastModifiedDate(const char *name, int64_t time)
{
    FILETIME fileTime, localFileTime;

    /**
     * Open the path ensuring GENERIC_WRITE and FILE_FLAG_BACKUP_SEMANTICS if it's a directory.
     * The directory modification is only supported on some platforms (NT, Windows2000).
     */
    DWORD flags = FILE_ATTRIBUTE_NORMAL;
    int result = GetFileAttributes(name);
    if (result == 0xFFFFFFFF)
    {
        return false;
    }

    if (result & FILE_ATTRIBUTE_DIRECTORY)
    {
        flags = FILE_FLAG_BACKUP_SEMANTICS;
    }

    // MSDN SetFileTime function: "The handle must have been created using
    // the CreateFile function with the FILE_WRITE_ATTRIBUTES"
    HANDLE hFile = CreateFile(name, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
       NULL, OPEN_EXISTING, flags, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // convert back to a file time
    int64_t temp = (time * (int64_t)10000000) + 116444736000000000;

    localFileTime.dwHighDateTime = (DWORD)(temp >> 32);
    localFileTime.dwLowDateTime = (DWORD)temp;
    if (LocalFileTimeToFileTime(&localFileTime, &fileTime) &&
        SetFileTime(hFile, NULL, NULL, &fileTime))
    {
        CloseHandle(hFile);
        return true;
    }
    CloseHandle(hFile);
    return false;
}


/**
 * Set the read-only attribute on a file or directory.
 *
 * @param name   The target name.
 *
 * @return true if the attribute was set, false otherwise.
 */
bool SysFileSystem::setFileReadOnly(const char *name)
{
    DWORD attrs = GetFileAttributes(name);
    if (attrs == 0xFFFFFFFF)
    {
        return false;
    }

    attrs = attrs | FILE_ATTRIBUTE_READONLY;
    return SetFileAttributes(name, attrs) != 0;
}


/**
 * indicate whether the file system is case sensitive.
 *
 * @return For Windows, always returns false.
 */
bool SysFileSystem::isCaseSensitive()
{
    return false;
}


/**
 * Retrieve the file system root elements.  On Windows,
 * each of the drives is a root element.
 *
 * @return The number of roots located.
 */
int SysFileSystem::getRoots(char *roots)
{
    int length = GetLogicalDriveStrings(MAX_PATH, roots);
    // elements are returned in the form "d:\", with a null
    // separator.  Each root thus takes up 4 characters
    return length / 4;
}


/**
 * Return the separator used for separating path names.
 *
 * @return The ASCII-Z version of the path separator.
 */
const char *SysFileSystem::getSeparator()
{
    return "\\";
}


/**
 * Return the separator used for separating search path elements
 *
 * @return The ASCII-Z version of the path separator.
 */
const char *SysFileSystem::getPathSeparator()
{
    return ";";
}


/**
 * Return the string used as a line-end separator.
 *
 * @return The ASCII-Z version of the line end sequence.
 */
const char *SysFileSystem::getLineEnd()
{
    return "\r\n";
}


/**
 * Create a new SysFileIterator instance.
 *
 * @param p      The directory we're iterating over.
 */
SysFileIterator::SysFileIterator(const char *p)
{
    char pattern[MAX_PATH + 1];

    // save the pattern and convert into a wild card
    // search
    strncpy(pattern, p, sizeof(pattern));
    strncat(pattern, "\\*", sizeof(pattern) - strlen(pattern) - 1);

    // this assumes we'll fail...if we find something,
    // we'll flip this
    completed = true;
    handle = FindFirstFile (pattern, &findFileData);
    if (handle != INVALID_HANDLE_VALUE)
    {
        // we can still return data
        completed = false;
    }
}

/**
 * Destructor for the iteration operation.
 */
SysFileIterator::~SysFileIterator()
{
    close();
}


/**
 * close the iterator.
 */
void SysFileIterator::close()
{
    if (handle != INVALID_HANDLE_VALUE)
    {
        FindClose(handle);
        handle = INVALID_HANDLE_VALUE;
    }
}


/**
 * Check if the iterator has new results it can return.
 *
 * @return true if the iterator has another value to return, false if
 *         the iteration is complete.
 */
bool SysFileIterator::hasNext()
{
    return !completed;
}


/**
 * Retrieve the next iteration value.
 *
 * @param buffer The buffer used to return the value.
 */
void SysFileIterator::next(char *buffer)
{
    if (completed)
    {
        strcpy(buffer, "");
    }
    else
    {
        // copy our current result over
        strcpy(buffer, findFileData.cFileName);
    }
    // now locate the next one
    if (!FindNextFile (handle, &findFileData))
    {
        // we're done once we hit a failure
        completed = true;
        close();
    }
}



