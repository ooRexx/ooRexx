/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                              SysFileSystem.cpp */
/*                                                                            */
/* Windows implementation of the SysFileSystem class.                         */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "SysFileSystem.hpp"
#include "ActivityManager.hpp"
#include "FileNameBuffer.hpp"
#include <shlwapi.h>

int SysFileSystem::stdinHandle = 0;
int SysFileSystem::stdoutHandle = 1;
int SysFileSystem::stderrHandle = 2;

const char SysFileSystem::EOF_Marker = 0x1a;    // the end-of-file marker
const char *SysFileSystem::EOL_Marker = "\r\n"; // the end-of-line marker
const char SysFileSystem::PathDelimiter = '\\'; // directory path delimiter
const char SysFileSystem::NewLine = '\n';
const char SysFileSystem::CarriageReturn = '\r';

const int64_t FiletimeEpoch = 50491123200000000; // microseconds between 0001-01-01 and 1601-01-01
const int64_t NoTimeStamp = -999999999999999999; // invalid file time

/**
 * Convert a file time into microseconds since 0001-01-01 00:00:00
 *
 * Search MSDN for 'Converting a time_t Value to a File Time' for following implementation.
 *
 * @param timeStamp The source time stamp
 *
 * @return The corresponding microseconds value.
 */
int64_t FileTimeToMicroseconds(FILETIME &timeStamp)
{
    // FILETIME contains a 64-bit value representing the number of 100-nanosecond
    // intervals since January 1, 1601 (UTC).
    int64_t temp = ((int64_t) timeStamp.dwHighDateTime << (int64_t)32) | (int64_t)timeStamp.dwLowDateTime;
    return temp / 10 + FiletimeEpoch;
}

/**
 * Convert a microsecond value back into a FILETIME structure.
 *
 * @param usecs     The source microseconds value
 * @param timeStamp The target time stame structure.
 */
void MicrosecondsToFileTime(uint64_t usecs, FILETIME &timeStamp)
{
    // convert back to a file time
    // FILETIME needs a 64-bit value representing the number of 100-nanosecond
    // intervals since January 1, 1601 (UTC).
    int64_t temp = (usecs - FiletimeEpoch) * 10;

    timeStamp.dwHighDateTime = (DWORD)(temp >> 32);
    timeStamp.dwLowDateTime = (DWORD)temp;
}

/**
 * Search for a given filename, returning the fully
 * resolved name if it is found.
 *
 * @param name     The input search name.
 * @param fullName The returned fully resolved name.
 * @param fullNameLength
 *                 length of the full name return buffer
 *
 * @return True if the file was located, false otherwise.
 */
bool SysFileSystem::searchFileName(const char *name, FileNameBuffer &fullName)
{
    // try to resolve the path name
    if (getFullPathName(name, fullName))
    {
        // this must be a file.
        return isFile(fullName);
    }

    // search on the path...also must be a file
    if (searchOnPath(name, NULL, NULL, fullName))
    {
        return isFile(fullName);
    }

    return false;
}


/**
 * Get the full path name of a file.
 *
 * @param name   The source name
 * @param fullName a buffer for returning the resolved name
 *
 * @return true if this could be resolved, false otherwise.
 */
bool SysFileSystem::getFullPathName(const char *name, FileNameBuffer &fullName)
{
    // now try for original name
    if (name[0] == '\\' && name[1] == '\\' && name[2] == '?' && name[3] == '\\')
    {
       // if name starts with the Win32 namespace prefix \\?\ this may
       // be a request for a long (or otherwise subtle) path which
       // GetFullPathName() doesn't support, so we just pass through
       // see also
       // Naming Files, Paths, and Namespaces: https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file
       // File path formats on Windows systems: https://docs.microsoft.com/en-us/dotnet/standard/io/file-path-formats
       fullName = name;
       return true;
    }

    DWORD rc = GetFullPathName(name, (DWORD)fullName.capacity(), (LPTSTR)fullName, NULL);
    // it is possible that the filename is good, but the buffer was too small to hold the result
    if (rc > fullName.capacity())
    {
        // expand the buffer and try again.
        fullName.ensureCapacity(rc);
        rc = GetFullPathName(name, (DWORD)fullName.capacity(), (LPTSTR)fullName, NULL);
    }

    // sadly, this might differ from the real file name case, so we need to take one more step
    // to get a matching name
    if (rc > 0)
    {
        getLongName(fullName);
    }

    return rc > 0;
}


/**
 * Primitive search path with retry for buffer size problems.
 *
 * @param name   The name to search for
 * @param resolvedName
 *               The buffer for the return result
 *
 * @return true if there was a hit, false otherwise.
 */
bool SysFileSystem::searchOnPath(const char *name, const char *path, const char *extension, FileNameBuffer &fullName)
{
    // now try for original name
    LPSTR filePart;

    DWORD rc = SearchPath(path, name, extension, (DWORD)fullName.capacity(), fullName, &filePart);

    // it is possible that the filename is good, but the buffer was too small to hold the result
    if (rc > fullName.capacity())
    {
        fullName.ensureCapacity(rc);
        rc = SearchPath(path, name, extension, (DWORD)fullName.capacity(), fullName, &filePart);
    }

    // We want the name using the real case, so we have to do one extra step.
    if (rc > 0)
    {
        getLongName(fullName);
    }

    return rc > 0;
}


/**
 * Generate a fully qualified stream name.
 *
 * @param unqualifiedName
 *               The starting name.
 * @param qualifiedName
 *               The fully expanded and canonicalized file name.
 */
void SysFileSystem::qualifyStreamName(const char *unqualifiedName, FileNameBuffer &qualifiedName)
{
    // If already expanded, there is nothing more to do.
    if (qualifiedName.length() > 0)
    {
        return;
    }

    if (getFullPathName(unqualifiedName, qualifiedName))
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
            qualifiedName.shiftLeft(4);
        }
    }
}


/**
 * Get attribute flags for a file with the given name.
 *
 * @param name   The name to check.
 *
 * @return File attribute flags if the file exists,
 *         INVALID_FILE_ATTRIBUTES otherwise.
 */
DWORD SysFileSystem::getFileAttributes(const char *name)
{
    DWORD dwAttrib = GetFileAttributes(name);

    // For system files like hiberfil.sys GetFileAttributes fails with
    // ERROR_SHARING_VIOLATION (The process cannot access the file because it is
    // being used by another process).  In this case FindFirstFile is still able
    // to return attributes (we don't use it in the first place as it is slower).
    if (dwAttrib == INVALID_FILE_ATTRIBUTES &&
        GetLastError() == ERROR_SHARING_VIOLATION)
    {
        WIN32_FIND_DATA findData;
        HANDLE hFind = FindFirstFile(name, &findData);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            dwAttrib = findData.dwFileAttributes;
            FindClose(hFind);
        }
    }
    return dwAttrib;
}


/**
 * Get the FILETIME for a file with the given name.
 *
 * @param name   The name to check.
 * @param type   The type of FILETIME to return.
 *               Can be FiletimeAccess, FiletimeWrite, or FiletimeCreation
 * @param time   The returned FILETIME.
 *
 * @return true if the file exists, false otherwise.
 */
bool SysFileSystem::getFiletime(const char *name, FiletimeType type, FILETIME *time)
{
    WIN32_FILE_ATTRIBUTE_DATA stat;

    if (GetFileAttributesEx(name, GetFileExInfoStandard, &stat))
    {
        *time = type == FiletimeAccess ? stat.ftLastAccessTime :
               (type == FiletimeWrite ? stat.ftLastWriteTime : stat.ftCreationTime);
        return true;
    }

    // For system files like hiberfil.sys GetFileAttributesEx fails with
    // ERROR_SHARING_VIOLATION (The process cannot access the file because it is
    // being used by another process).  In this case FindFirstFile is still able
    // to return attributes (we don't use it in the first place as it is slower).
    else if (GetLastError() == ERROR_SHARING_VIOLATION)
    {
        WIN32_FIND_DATA statFind;
        HANDLE hFind = FindFirstFile(name, &statFind);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);
            *time = type == FiletimeAccess ? statFind.ftLastAccessTime :
                   (type == FiletimeWrite ? statFind.ftLastWriteTime : statFind.ftCreationTime);
            return true;
        }
    }
    // we failed to retrieve the timestamp
    return false;
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
    DWORD dwAttrib = getFileAttributes(name);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
          !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


/**
 * Determine if the given file is a symbolic link to another file.
 *
 * @param name   The file name.
 *
 * @return True if the file is considered a link, false if this is the real
 *         file object.
 */
bool SysFileSystem::isLink(const char *name)
{
    DWORD dwAttrs = GetFileAttributes(name);
    return (dwAttrs != INVALID_FILE_ATTRIBUTES) && (dwAttrs & FILE_ATTRIBUTE_REPARSE_POINT);
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
 * @param path      The path to search on (can be NULL)
 * @param extension A potential extension to add to the file name (can be NULL).
 * @param resolvedName
 *                  The buffer used to return the resolved file name.
 *
 * @return true if the file was located.  A true returns indicates the
 *         resolved file name has been placed in the provided buffer.
 */
bool SysFileSystem::searchName(const char *name, const char *path, const char *extension, FileNameBuffer &resolvedName)
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
 * @param path      The path to be searched on
 * @param extension A potential extension to add to the file name (can be NULL).
 * @param resolvedName
 *                  The buffer used to return the resolved file name.
 *
 * @return true if the file was located.  A true returns indicates the
 *         resolved file name has been placed in the provided buffer.
 */
bool SysFileSystem::primitiveSearchName(const char *name, const char *path, const char *extension, FileNameBuffer &resolvedName)
{
    // this is for building a temporary name
    AutoFileNameBuffer tempName(resolvedName);
    tempName = name;

    if (extension != NULL)
    {
        tempName += extension;
    }

    // if this appears to be a fully qualified name, then check it as-is and
    // quit.  The path searches might give incorrect results if performed with such
    // a name and this should only check on the raw name.
    if (hasDirectory(tempName))
    {
        // check the file as is first
        return checkCurrentFile(tempName, resolvedName);
    }

    if (searchOnPath(tempName, path, NULL, resolvedName))
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
    // (there are more ways to specify a drive than just d: but still ..)
    return name[0] == '\\' || name[1] == ':' ||
          (name[0] == '.' && name[1] == '\\') ||
          (name[0] == '.' && name[1] == '.' && name[2] == '\\');
}



/**
 * Try to locate a file using just the raw name passed in, as
 * opposed to searching along a path for the name.
 *
 * @param name   The name to use for the search.
 * @param resolvedName
 *               The returned resolved name.
 * @param resolvedNameLength
 *               The length of the resolved name buffer
 *
 * @return An RexxString version of the file name, iff the file was located. Returns
 *         OREF_NULL if the file did not exist.
 */
bool SysFileSystem::checkCurrentFile(const char *name, FileNameBuffer &resolvedName)
{
    if (getFullPathName(name, resolvedName))
    {
        // if this is a real file vs. a directory, make sure we return
        // the long name value in the correct casing
        if (isFile(resolvedName))
        {
            getLongName(resolvedName);
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
 * @param resolvedName
 *                  A buffer used for returning the resolved name.
 *
 * @return Returns true if the file was located.  If true, the resolvedName
 *         buffer will contain the returned name.
 */
bool SysFileSystem::searchPath(const char *name, const char *path, FileNameBuffer &resolvedName)
{
    if (searchOnPath(name, path, NULL, resolvedName))
    {
        // if this is a file, return the long name
        if (isFile(resolvedName))
        {
            getLongName(resolvedName);
            return true;
        }
    }
    resolvedName = "";
    return false;        // not found
}


/**
 * Get the actual name value of a located file, in the exact case
 * used on the harddrive.
 *
 * @param fullName The buffer used for the name.
 * @param size     The size of the buffer.
 */
void SysFileSystem::getLongName(FileNameBuffer &fullName)
{
    // GetLongPathName() and friends do not give a consistent case for the
    // drive letter, so uppercase it if this path does have one.
    if (fullName.length() >= 2 && fullName.at(1) == ':')
    {
        fullName.at(0) = toupper(fullName.at(0));
    }

    DWORD length = GetLongPathName(fullName, fullName, (DWORD)fullName.capacity());
    if (length > (DWORD)fullName.capacity())
    {
        fullName.ensureCapacity(length);
        length = GetLongPathName(fullName, fullName, length);

    }

    // if we got a result, then do a search for the file and
    // truncate this to just the file name.
    if (length != 0)
    {
        WIN32_FIND_DATA findData;
        HANDLE hFind = FindFirstFile(fullName, &findData);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            const char *p = strrchr((const char *)fullName, '\\');
            if (p != NULL)
            {
                fullName.truncate((p - fullName) + 1);
                fullName += findData.cFileName;
            }
            FindClose(hFind);
        }
    }
    // we're processing a path that does not exist. Try to resolve as much of this as we can
    else
    {
        getProperPathCase(fullName);
    }
    return;
}


/**
 * Resolve as much of the path as we can to obtain the path in
 * Unfortunately, GetLongPathName() will preserve original case
 * for both the file names and portions of the the path, so we
 * need to step back through all of the elements of the path and
 * resolve the actual case name of the element.
 *
 * @param fullName The source path name. This will be overwritten with the correct
 *                 path on return.
 */
void SysFileSystem::getProperPathCase(FileNameBuffer &fullName)
{
    AutoFileNameBuffer buffer(fullName);

    buffer = (const char *)fullName;

    // we are only called because GetLongPathName() has already failed on the full path,
    // so we need to back down one path element at a time until we find something that exists

    while (true)
    {
        // scan back for the last backslash
        const char *p = strrchr((const char *)buffer, '\\');
        // if we can't find a back slash, then there's nothing we can fix up
        if (p == NULL)
        {
            return;
        }
        size_t prefixLength = p - buffer;
        // chop off the last path section
        buffer.truncate(prefixLength);

        // now try to see if we can resolve the full name for this section. If
        // we can, then do the FindFirstFile trick to ensure the last bit is in the
        // correct case
        DWORD length = GetLongPathName(buffer, buffer, (DWORD)buffer.capacity());
        if (length > (DWORD)buffer.capacity())
        {
            buffer.ensureCapacity(length);
            length = GetLongPathName(buffer, buffer, length);
        }

        // if we got a result, then do a search for the file and
        // truncate this to just the file name.
        if (length != 0)
        {
            WIN32_FIND_DATA findData;
            HANDLE hFind = FindFirstFile(buffer, &findData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                p = strrchr((const char *)buffer, '\\');
                if (p != NULL)
                {
                    buffer.truncate((p - buffer) + 1);
                    buffer += findData.cFileName;
                }
                FindClose(hFind);
                // add the sections we skipped to the part we we able to resolve
                buffer += (const char *)fullName + prefixLength;
                // and return this full name
                fullName = buffer;
                return;
            }
        }
    }
}



/**
 * Delete a file from the file system.
 *
 * @param name   The fully qualified name of the file.
 *
 * @return The return code from the delete operation.
 */
int SysFileSystem::deleteFile(const char *name)
{
    return DeleteFile(name) != 0 ? 0 : GetLastError();
}

/**
 * Delete a directory from the file system.
 *
 * @param name   The name of the target directory.
 *
 * @return The return code from the delete operation.
 */
int SysFileSystem::deleteDirectory(const char *name)
{
    return RemoveDirectory(name) != 0 ? 0 : GetLastError();
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
    DWORD dwAttrs = getFileAttributes(name);
    return (dwAttrs != INVALID_FILE_ATTRIBUTES) && (dwAttrs & FILE_ATTRIBUTE_DIRECTORY);
}


/**
 * Test is a file is read only.
 *
 * @param name   The target file name.
 *
 * @return true if the file can be opened for reading
 */
bool SysFileSystem::isReadOnly(const char *name)
{
    DWORD dwAttrs = GetFileAttributes(name);
    return (dwAttrs != INVALID_FILE_ATTRIBUTES) && (dwAttrs & FILE_ATTRIBUTE_READONLY);
}


/**
 * Test is a file is read only.
 *
 * @param name   The target file name.
 *
 * @return true if the file is marked as read-only.
 */
bool SysFileSystem::canRead(const char *name)
{
    // attempt to open for read...if this fails, this is write only
    HANDLE handle = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    CloseHandle(handle);
    return true;
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
 * Test if a file is writable
 *
 * @param name   The target file name.
 *
 * @return true if the file exists and can be opened for write
 *         operations.
 */
bool SysFileSystem::canWrite(const char *name)
{
    // if this is a directory, we just check the read only status
    if (isDirectory(name))
    {
        return !isReadOnly(name);
    }

    // attempt to open for read...if this fails, this is write only
    HANDLE handle = CreateFile(name, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    CloseHandle(handle);
    return true;
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
    DWORD dwAttrs = getFileAttributes(name);
    return (dwAttrs != INVALID_FILE_ATTRIBUTES) && (dwAttrs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == 0;
}


/**
 * Test if a file or directory exists using a fully qualified name.
 *
 * @param name   The target file or directory name.
 *
 * @return True if the file or directory exists, false if it is unknown.
 */
bool SysFileSystem::exists(const char *name)
{
    DWORD dwAttrs = getFileAttributes(name);
    return (dwAttrs != INVALID_FILE_ATTRIBUTES);
}


/**
 * Get the last modified file date as a file time value.
 *
 * @param name   The target name.
 *
 * @return the file time value for the modified date, or -999999999999999999
 *         for any errors.
 */
int64_t SysFileSystem::getLastModifiedDate(const char *name)
{
    FILETIME time, localTime;
    if (getFiletime(name, FiletimeWrite, &time) &&
        FileTimeToLocalFileTime(&time, &localTime))
    {
        // convert to ticks
        return FileTimeToMicroseconds(localTime);
    }
    else
    {
        return NoTimeStamp;
    }
}


/**
 * Get the file last access data as a file time value.
 *
 * @param name   The target name.
 *
 * @return the file time value for the last access date, or -999999999999999999
 *         for any errors.
 */
int64_t SysFileSystem::getLastAccessDate(const char *name)
{
    FILETIME time, localTime;
    if (getFiletime(name, FiletimeAccess, &time) &&
        FileTimeToLocalFileTime(&time, &localTime))
    {
        // convert to ticks
        return FileTimeToMicroseconds(localTime);
    }
    else
    {
        return NoTimeStamp;
    }
}


/**
 * Retrieve the size of a file.
 *
 * @param name   The name of the target file.
 *
 * @return the 64-bit file size, or zero if not a valid file
 */
int64_t SysFileSystem::getFileLength(const char *name)
{
    WIN32_FILE_ATTRIBUTE_DATA stat;
    DWORD high = 0, low = 0;

    if (GetFileAttributesEx(name, GetFileExInfoStandard, &stat))
    {
        // if this is a directory we cannot retrieve a file size
        // these fields do not have a meaning for directories
        if ((stat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            high = stat.nFileSizeHigh;
            low = stat.nFileSizeLow;
        }
    }

    // For system files like hiberfil.sys GetFileAttributesEx fails with
    // ERROR_SHARING_VIOLATION (The process cannot access the file because it is
    // being used by another process).  In this case FindFirstFile is still able
    // to return attributes (we don't use it in the first place as it is slower).
    else if (GetLastError() == ERROR_SHARING_VIOLATION)
    {
        WIN32_FIND_DATA statFind;

        HANDLE hFind = FindFirstFile(name, &statFind);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);
            // if this is a directory we cannot retrieve a file size
            // for directories these fields have no meaning
            if ((statFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                high = statFind.nFileSizeHigh;
                low = statFind.nFileSizeLow;
            }
        }
    }
    // returns file size if a valid file, otherwise returns 0
    return (((int64_t)high) << 32) + ((int64_t)low);
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
 * Test if a given file or directory is hidden.
 *
 * @param name   The target name.
 *
 * @return true if the file or directory is hidden, false otherwise.
 */
bool SysFileSystem::isHidden(const char *name)
{
    DWORD dwAttrs = getFileAttributes(name);
    return (dwAttrs != INVALID_FILE_ATTRIBUTES) && (dwAttrs & FILE_ATTRIBUTE_HIDDEN);
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
    if (result == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    if ( result & FILE_ATTRIBUTE_DIRECTORY )
    {
        flags = FILE_FLAG_BACKUP_SEMANTICS;
    }

    // MSDN SetFileTime function: "The handle must have been created using
    // the CreateFile function with the FILE_WRITE_ATTRIBUTES"
    HANDLE hFile = CreateFile(name, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, OPEN_EXISTING, flags, NULL);
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        return false;
    }

    // convert back to a file time
    MicrosecondsToFileTime(time, localFileTime);

    if ( LocalFileTimeToFileTime(&localFileTime, &fileTime) &&
         SetFileTime(hFile, NULL, NULL, &fileTime) )
    {
        CloseHandle(hFile);
        return true;
    }
    CloseHandle(hFile);
    return false;
}


/**
 * Set the last access date for a file.
 *
 * @param name   The target name.
 * @param time   The new file time.
 *
 * @return true if the filedate was set correctly, false otherwise.
 */
bool SysFileSystem::setLastAccessDate(const char *name, int64_t time)
{
    FILETIME fileTime, localFileTime;

    /**
     * Open the path ensuring GENERIC_WRITE and FILE_FLAG_BACKUP_SEMANTICS if it's a directory.
     * The directory modification is only supported on some platforms (NT, Windows2000).
     */
    DWORD flags = FILE_ATTRIBUTE_NORMAL;
    int result = GetFileAttributes(name);
    if (result == INVALID_FILE_ATTRIBUTES)
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
    MicrosecondsToFileTime(time, localFileTime);

    if (LocalFileTimeToFileTime(&localFileTime, &fileTime) &&
        SetFileTime(hFile, NULL, &fileTime, NULL))
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
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    attrs = attrs | FILE_ATTRIBUTE_READONLY;
    return SetFileAttributes(name, attrs) != 0;
}


/**
 * Set the read-only attribute on a file or directory.
 *
 * @param name   The target name.
 *
 * @return true if the attribute was set, false otherwise.
 */
bool SysFileSystem::setFileWritable(const char *name)
{
    DWORD attrs = GetFileAttributes(name);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    attrs = attrs & ~FILE_ATTRIBUTE_READONLY;
    return SetFileAttributes(name, attrs) != 0;
}



/**
 * indicate whether the file system is case sensitive.
 *
 * @return For Windows, the case-sensitivity of the Windows system
 *         directory is returned, typically C:\Windows\system32
 */
bool SysFileSystem::isCaseSensitive()
{
    char system[SysFileSystem::MaximumPathLength];

    GetSystemDirectory(system, sizeof(system));
    return isCaseSensitive(system);
}


/**
 * Test if a file or directory is case-sensitive.
 *
 * On Windows, case-sensitivity is defined on a per-directory basis.
 * Once a directory has been set as case-sensitive, files in this
 * directory and any newly created subdirectories within this directory
 * become case-sensitive.  Existing subdirectories and files within
 * those existing subdirectories stay as they are.
 *
 * @return true if this is a case-sensitive directory, or a file within a
 *              case-sensitive directory, false otherwise.
 */
bool SysFileSystem::isCaseSensitive(const char *name)
{
    // Starting with Windows 10 build 17093, if the Windows Subsystem for Linux
    // is enabled, a per-directory case sensitivity can be set with the command
    // fsutil file setCaseSensitiveInfo <path> enable.
    // As of 10/2020 there's only scarce information available on the API
    // to query the case-sensitivity of a directory.  It seems that
    // GetFileInformationByHandleEx() will be enhanced as follows:
    // typedef enum _FILE_INFO_BY_HANDLE_CLASS will include a new
    //     FileCaseSensitiveInfo Int32 with value 23
    // plus the following structure is to return the value
    // typedef struct _FILE_CASE_SENSITIVE_INFO {
    //     ULONG Flags;
    // } FILE_CASE_SENSITIVE_INFO, *PFILE_CASE_SENSITIVE_INFO;
#define future_FileCaseSensitiveInfo (FILE_INFO_BY_HANDLE_CLASS)23
    typedef struct _future_FILE_CASE_SENSITIVE_INFO {
        ULONG Flags;
    } future_FILE_CASE_SENSITIVE_INFO;

    future_FILE_CASE_SENSITIVE_INFO stat;

    AutoFree tmp = strdup(name);

    while (!isDirectory(tmp))
    {
        size_t len = strlen(tmp);
        // scan backwards to find the previous directory delimiter
        for (; len > 1; len--)
        {
            // is this the directory delimiter?
            if (tmp[len] == '\\')
            {
                // don't go beyond root backslash
                tmp[tmp[len - 1] == ':' ? len + 1 : len] = '\0';
                break;
            }
        }
    }
    // at this point the tmp variable should name an existing directory

    HANDLE handle = CreateFile(tmp, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    bool success = GetFileInformationByHandleEx(handle, future_FileCaseSensitiveInfo,
        &stat, sizeof(future_FILE_CASE_SENSITIVE_INFO));
    CloseHandle(handle);
    return success ? stat.Flags != 0 : false;
}


/**
 * Retrieve the file system root elements.  On Windows,
 * each of the drives is a root element.
 *
 * @return The number of roots located.
 */
int SysFileSystem::getRoots(FileNameBuffer &roots)
{
    int length = GetLogicalDriveStrings((DWORD)roots.capacity(), roots);
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
 * Retrieve the current working directory into a FileNameBuffer.
 *
 * @param directory The return directory name.
 *
 * @return true if this was retrieved, false otherwise.
 */
bool SysFileSystem::getCurrentDirectory(FileNameBuffer &directory)
{
    // Get the current directory.  First check that the path buffer is large
    // enough.
    uint32_t ret = GetCurrentDirectory(0, NULL);
    if (ret == 0)
    {
        // make a null string and return a failure.
        directory = "";
        return false;
    }
    else
    {
        directory.ensureCapacity(ret);
        GetCurrentDirectory((DWORD)directory.capacity(), directory);
        return true;
    }
}


/**
 * Set the current working directory
 *
 * @param directory The new directory set.
 *
 * @return True if this worked, false for any failures
 */
bool SysFileSystem::setCurrentDirectory(const char *directory)
{
    return SetCurrentDirectory(directory) != 0;
}


/**
 * Implementation of a copy file operation.
 *
 * @param fromFile The from file of the copy operation
 * @param toFile   The to file of the copy operation.
 *
 * @return Any error codes.
 */
int SysFileSystem::copyFile(const char *fromFile, const char *toFile)
{
    return CopyFile(fromFile, toFile, 0) != 0 ? 0 : GetLastError();
}


/**
 * Move a file from one location to another. This is typically just a rename, but if necessary, the file will be copied and the original unlinked.
 *
 * @param fromFile The file we're copying from.
 * @param toFile   The target file.
 *
 * @return 0 if this was successful, otherwise the system error code.
 */
int SysFileSystem::moveFile(const char *fromFile, const char *toFile)
{
    return MoveFile(fromFile, toFile) != 0 ? 0 : GetLastError();
}


/**
 * Locate the last directory delimiter in a file spec name
 *
 * @param name   The give name.
 *
 * @return The location of the last directory delimiter, or NULL if one is
 *         not found.
 */
const char* SysFileSystem::getPathEnd(const char *name)
{
    const char *pathEnd = strrchr(name, '\\');       // find the last backslash in name
    const char *altPathEnd = strrchr(name, '/');     // 3.2.0 also looked for a forward slash, so handle that also
    if (altPathEnd > pathEnd)
    {
        pathEnd = altPathEnd;
    }
    return pathEnd;
}


/**
 * Locate the last directory delimiter in a file spec name
 *
 * @param name   The give name.
 *
 * @return The location of the last directory delimiter, or NULL if one is
 *         not found.
 */
const char* SysFileSystem::getPathStart(const char *name)
{
    // we look for the first colon, because this could also be a device specification
    const char *driveEnd = strchr(name, ':');        // and first colon

    return driveEnd == NULL ? name : driveEnd + 1;
}


/**
 * Retrieve the location where temporary files should be stored.
 * This will be the value of environment variable TMP, TEMP, or USERPROFILE,
 * if defined, otherwise the current working directory.
 *
 * @return true if successful, false otherwise.
 */
bool SysFileSystem::getTemporaryPath(FileNameBuffer &temporary)
{
    if (!SystemInterpreter::getEnvironmentVariable("TMP", temporary) &&
        !SystemInterpreter::getEnvironmentVariable("TEMP", temporary) &&
        !SystemInterpreter::getEnvironmentVariable("USERPROFILE", temporary) &&
        !SysFileSystem::getCurrentDirectory(temporary))
    {
        return false;
    }
    return true;
}



/**
 * Create a new SysFileIterator instance.
 *
 * @param path    The path we're going to be searching in
 * @param pattern The pattern to use. If NULL, then everything in the path will be returned.
 * @param buffer  A FileNameBuffer object used to construct the path name.
 * @param c       the caseless flag (ignored for Windows)
 */
SysFileIterator::SysFileIterator(const char *path, const char *pattern, FileNameBuffer &buffer, bool c)
{
    // we can bypass limits on length of the path if we prepend the file spec with
    // the special prefix \\?\ for local paths or \\?\UNC\ for UNC paths
    // (see https://docs.microsoft.com/en-us/dotnet/standard/io/file-path-formats)
    // Note: prepending this prefix will stop Windows converting slashes in the
    // path to backslashes.

    // don't double-prefix if we've already been given a prefix
    // if starts with ..    prefix with ..
    // \\.\xxx              (leave as-is)
    // \\?\xxx              (leave as-is)
    // \\xxx                \\?\UNC\xxx
    // xxx                  \\?\xxx
    if (path[0] == '\\' && path[1] == '\\')
    {
        if (path[2] != '.' && path[2] != '?')
        {
            // this is a UNC path .. prefix with "\\?\UNC\"
            buffer = "\\\\?\\UNC\\";
            buffer += path + 2;
        }
        else
        {
            // this is already prefixed, keep it as-is
            buffer = path;
        }
    }
    else
    {
        // this is a local path .. prefix with "\\?\"
        buffer = "\\\\?\\";
        buffer += path;
    }

    // save the pattern and convert into a wild card
    // search

    // make sure there is a trailing delimiter
    buffer.addFinalPathDelimiter();
    // if no pattern was given, then just use a wild card
    if (pattern == NULL)
    {
        // no extra filtering required
        fileSpec = NULL;
        buffer += "*";
    }
    // add the pattern section to the fully-resolved buffer
    else
    {
        buffer += pattern;
        // we need to save this for short name filtering
        fileSpec = pattern;
    }

    // this assumes we'll fail...if we find something,
    // we'll flip this
    completed = true;
    handle = FindFirstFile(buffer, &findFileData);
    if (handle != INVALID_HANDLE_VALUE)
    {
        // we can still return data
        completed = false;
        // find the next real entry, filtered for short name problems.
        filterShortNames();
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
 * @param buffer     The buffer used to return the value.
 * @param attributes The returned system-dependent attributes of the next file.
 */
void SysFileIterator::next(FileNameBuffer &buffer, SysFileIterator::FileAttributes &attributes)
{
    if (completed)
    {
        buffer = "";
    }
    else
    {
        // copy our current result over
        buffer = findFileData.cFileName;
        // and also copy the file attribute data
        attributes.findFileData = findFileData;
    }

    // find the next entry (with filtering)
    findNextEntry();
}


/**
 * Scan forward through the directory to find the next matching entry.
 *
 * @return true if a matching entry was found, false for any error or nothign found.
 */
void SysFileIterator::findNextEntry()
{
    // now locate the next one
    if (!FindNextFile(handle, &findFileData))
    {
        // we're done once we hit a failure
        completed = true;
        close();
    }

    // we might need to scan forward for a suitable name if we got a hit on
    // a short name
    filterShortNames();
}


/**
 * Perform filtering on false hits due to short file name matching problems.
 */
void SysFileIterator::filterShortNames()
{
    // no filtering for short names if we don't have a pattern specified.
    if (fileSpec == NULL)
    {
        return;
    }

    // PathMatchSpec seems to treat fileSpec as a semicolon-delimited list
    // We must use the Ex version
    while (S_FALSE == PathMatchSpecEx(findFileData.cFileName, fileSpec, PMSF_NORMAL))
    {
        // now locate the next one
        if (!FindNextFile(handle, &findFileData))
        {
            // we're done once we hit a failure
            completed = true;
            close();
            return;
        }
    }
}




