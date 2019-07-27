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
/* Unix implementation of the SysFileSystem class.                            */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <pwd.h>
#include <errno.h>
#include <fnmatch.h>
#include "SysFileSystem.hpp"
#include "Utilities.hpp"
#include "ActivityManager.hpp"
#include "FileNameBuffer.hpp"


const char SysFileSystem::EOF_Marker = 0x1A;
const char *SysFileSystem::EOL_Marker = "\n";
const char SysFileSystem::PathDelimiter = '/';
const char SysFileSystem::NewLine = '\n';
const char SysFileSystem::CarriageReturn = '\r';

// .DateTime~new(1970, 1, 1) - .DateTime~new(1, 1, 1)~totalSeconds
const int64_t StatEpoch = 62135596800;           // seconds between 0001-01-01 and 1970-01-01
const int64_t NoTimeStamp = -999999999999999999; // invalid file time

/**
 * Search for a given filename, returning the fully
 * resolved name if it is found.
 *
 * @param name     The original name.
 * @param fullName A pointer to the buffer where the resolved name is returned.
 *
 * @return True if the file was found, false otherwise.
 */
bool SysFileSystem::searchFileName(const char *name, FileNameBuffer &fullName)
{
    size_t nameLength = strlen(name);

    /* does the filename already have a path? */
    /* or does it start with "~" ? */
    /* (beware, don't test "." because files like ".hidden" alone are candidate for search in PATH */
    if (strstr(name, "/") != NULL || name[0] == '~' || name[0] == '.')
    {
        bool done = canonicalizeName(fullName);
        if (done == false || fileExists(fullName) == false)
        {
            fullName.at(0) = '\0';
            return false;
        }
        return true;
    }

    // Get the current working directory
    if (!getCurrentDirectory(fullName))
    {
        return false;
    }
    // now add on to the front of the name
    fullName += "/";
    fullName += name;

    // if the file exists, then return it
    if (fileExists(fullName))
    {
        return true;
    }

    // it was not in the current directory so search the PATH
    const char *currentPath = getenv("PATH");
    if (currentPath == NULL)
    {
        fullName = "";
        return false;
    }

    const char *sep = strchr(currentPath, ':');
    while (sep != NULL)
    {
        /* try each entry in the PATH */
        int i = sep - currentPath;
        fullName.set(currentPath, i);
        fullName += "/";
        fullName += name;
        if (fileExists(fullName) == true)
        {
            return true;
        }
        currentPath = sep + 1;
        sep = strchr(currentPath, ':');
    }

    /* the last entry in the PATH might not be terminated by a colon */
    if (*currentPath != '\0')
    {
        fullName = currentPath;
        fullName += currentPath;
        fullName += name;
        if (fileExists(fullName) == true)
        {
            return true;
        }
    }

    // not found, return a null string
    fullName = "";
    return false;
}


/**
 * Get the fully qualified name for a file.
 *
 * @param name     The input file name.
 * @param fullName The return full file name
 */
void SysFileSystem::qualifyStreamName(const char *name, FileNameBuffer &fullName)
{
    // if already expanded, then do nothing
    if (fullName.length() != 0)
    {
        return;                           /* nothing more to do               */
    }

    // copy this over and canonicalize the name
    fullName = name;
    if (!SysFileSystem::canonicalizeName(fullName))
    {
        // reset to a null string for any failures
        fullName = "";
    }
    return;
}


/**
 * Test if a given file exists.
 *
 * @param fname  The target file name.
 *
 * @return true if the file exists, false otherwise.
 */
bool SysFileSystem::fileExists(const char *fname)
{
    struct stat64 filestat;              // file attributes
    int rc;                              // stat function return code

    rc = stat64(fname, &filestat);
    if (rc == 0)
    {
        if (S_ISREG(filestat.st_mode))
        {
            return true;
        }
    }
    return false;
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
RexxString* SysFileSystem::extractDirectory(RexxString *file)
{
    const char *pathName = file->getStringData();
    const char *endPtr = pathName + file->getLength() - 1;

    // scan backwards looking for a directory delimiter.  This name should
    // be fully qualified, so we don't have to deal with drive letters
    while (pathName < endPtr)
    {
        // find the first directory element?
        if (*endPtr == '/')
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
RexxString* SysFileSystem::extractExtension(RexxString *file)
{
    const char *pathName = file->getStringData();
    const char *endPtr = pathName + file->getLength() - 1;

    // scan backwards looking for a directory delimiter.  This name should
    // be fully qualified, so we don't have to deal with drive letters
    while (pathName < endPtr)
    {
        // find the first directory element?
        if (*endPtr == '/')
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
RexxString* SysFileSystem::extractFile(RexxString *file)
{
    const char *pathName = file->getStringData();
    const char *endPtr = pathName + file->getLength() - 1;

    // scan backwards looking for a directory delimiter.  This name should
    // be fully qualified, so we don't have to deal with drive letters
    while (pathName < endPtr)
    {
        // find the first directory element?
        if (*endPtr == '/')
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
    return name[0] == '~' || name[0] == '/' || name[0] == '.';
}


/**
 * Do a search for a single variation of a filename.
 *
 * @param name      The name to search for.
 * @param path      The path to search over (can be NULL)
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
 * @param name      The name to search for.
 * @param path      The path to search over
 * @param extension A potential extension to add to the file name (can be NULL).
 * @param resolvedName
 *                  The buffer used to return the resolved file name.
 *
 * @return true if the file was located.  A true returns indicates the
 *         resolved file name has been placed in the provided buffer.
 */
bool SysFileSystem::primitiveSearchName(const char *name, const char *path, const char *extension, FileNameBuffer &resolvedName)
{
    FileNameBuffer tempName;
    // construct the search name, potentially adding on an extension
    tempName = name;
    if (extension != NULL)
    {
        tempName += extension;
    }

    // only do the direct search if this is qualified enough that
    // it should not be located on the path
    if (hasDirectory(tempName))
    {
        for (int i = 0; i < 2; i++)
        {
            // check the file as is first
            if (checkCurrentFile(tempName, resolvedName))
            {
                return true;
            }
            // try again in lower case
            Utilities::strlower((char *)tempName);
        }
        return false;
    }
    else
    {
        // for each name, check in both the provided case and lower case.
        for (int i = 0; i < 2; i++)
        {
            // go search along the path
            if (searchPath(tempName, path, resolvedName))
            {
                return true;
            }
            // try again in lower case
            Utilities::strlower((char *)tempName);
        }
        return false;
    }
}


/**
 * Try to locate a file using just the raw name passed in, as
 * opposed to searching along a path for the name.
 *
 * @param name   The name to use for the search.
 * @param resolvedName
 *               The buffer for the returned name.
 *
 * @return true if the file was located, false otherwise.
 */
bool SysFileSystem::checkCurrentFile(const char *name, FileNameBuffer &resolvedName)
{
    resolvedName = name;
    // take care of any special conditions in the name structure
    // a failure here means an invalid name of some sort
    if (!canonicalizeName(resolvedName))
    {
        return false;
    }

    struct stat64 dummy;                 /* structure for stat system calls   */

    // ok, if this exists, life is good.  Return it.
    if (stat64(resolvedName, &dummy) == 0)           /* look for file         */
    {
        // this needs to be a regular file
        if (S_ISREG(dummy.st_mode))
        {
            return true;
        }
        return false;
    }
    // not found
    return false;
}


/**
 * Do a path search for a file.
 *
 * @param name   The name to search for.
 * @param path   The search path to use.
 * @param resolvedName
 *               A buffer used for returning the resolved name.
 *
 * @return Returns true if the file was located.  If true, the resolvedName
 *         buffer will contain the returned name.
 */
bool SysFileSystem::searchPath(const char *name, const char *path, FileNameBuffer &resolvedName)
{
    // get an end pointer
    const char *pathEnd = path + strlen(path);

    const char *p = path;
    const char *q = strchr(p, ':');
    /* For every dir in searchpath*/
    for (; p < pathEnd; p = q + 1, q = strchr(p, ':'))
    {
        // it's possible we've hit the end, in which case, point the delimiter marker past the end of the
        // string
        if (q == NULL)
        {
            q = pathEnd;
        }
        size_t subLength = q - p;

        resolvedName.set(p, subLength);
        resolvedName += "/";
        resolvedName += name;

        // take care of any special conditions in the name structure
        // a failure here means an invalid name of some sort
        if (canonicalizeName(resolvedName))
        {
            struct stat64 dummy;
            if (stat64(resolvedName, &dummy) == 0)   /* If file is found,     */
            {
                // this needs to be a regular file
                if (S_ISREG(dummy.st_mode))
                {
                    return true;
                }
                resolvedName = "";
                return false;
            }
        }
    }
    resolvedName = "";
    return false;
}


/**
 * resolve the user home directory portion of a file name
 *
 * @param name       The current working name. This is updated in place.
 * @param nameLength The length of the working buffer
 *
 * @return true if this was valid enough to normalize.
 */
bool SysFileSystem::resolveTilde(FileNameBuffer &name)
{
    // save a copy of the name
    AutoFileNameBuffer tempName(name);

    // does it start with the user home marker?
    // this is the typical case.  This is a directory based off of
    // the current users home directory.
    if (name.at(1) == '\0' || name.at(1) == '/')
    {
        // save everything after the first character
        tempName = ((const char *)name) + 1;
        // start with the home directory
        name = getenv("HOME");
        // We don't need to add a slash : If we have "~" alone, then no final slash expected (same as for "~user"). If "~/..." then we have the slash already
        name += (const char *)tempName;
    }
    else
    {
        // referencing a file in some other user's home directory.
        // we need to extract the username and resolve that home directory

        AutoFileNameBuffer userName(name);
        // copy the whole original name into the temporary
        tempName = (const char *)name;

        // look for the start of a directory
        const char *slash = strchr((const char *)tempName, '/');
        // if there is a directory after the username, we need
        // to copy just the name piece
        if (slash != NULL)
        {
            size_t nameLen = slash - ((const char *)tempName) - 1;
            userName.set(((const char *)tempName) + 1, nameLen);
        }
        // all username, just copy
        else
        {
            userName = ((const char *)tempName) + 1;
        }

        // see if we can retrieve the information
        struct passwd *ppwd = getpwnam(userName);
        if (ppwd == NULL)
        {
            // this is not valid without user information, so just fail the operation
            // if we can't get this.
            return false;                    /* nothing happend            */
        }
        // copy the home dir
        name = ppwd->pw_dir;
        // if we have a directory after the username, copy the whole thing
        // from that point
        if (slash != NULL)
        {
            name += slash;
        }
    }
    return true;
}



/**
 * Process a file name to add the current working directory
 * or the home directory, as needed, then remove all of the
 * . and .. elements.
 *
 * @param name   The current working name.
 *
 * @return true if this was valid enough to normalize.
 */
bool SysFileSystem::canonicalizeName(FileNameBuffer &name)
{
    // does it start with the user home marker?
    if (name.startsWith('~'))
    {
        resolveTilde(name);
    }
    // if we're not starting with root, we need to add the
    // current working directory.  This will also handle the
    // "." and ".." cases, which will be removed by the canonicalization
    // process.
    else if (!name.startsWith('/'))
    {
        // copy the name
        FileNameBuffer tempName = name;

        // get the current directory in the name buffer
        if (!getCurrentDirectory(name))
        {
            return false;
        }
        name += "/";
        name += tempName;
    }

    // NOTE:  realpath() is more portable than canonicalize_file_name().
    // However, they both have problems in that they both fail if the file does
    // not exist.  There are a number of places where the interpreter needs to
    // canonicalize a path name whether the file exists or not.  So we use our
    // own function to normalize the name.
    FileNameBuffer tempName;

    if (normalizePathName(name, tempName))
    {
        name = tempName;
        return true;
    }
    return false;
}


/**
 * Normalize an absolute Unix path name.  Removes duplicate and trailing
 * slashes, resolves and removes ./ or ../  This works for any path name,
 * whether the resovled name exists or not.
 *
 * @param name      The path name to normalize.
 * @param resolved  On success the normalized name is returned here.
 *
 * @return True on success, otherwise false.
 *
 * @assumes  Name is null-terminated and that resolved is an adequate buffer.
 */
bool SysFileSystem::normalizePathName(const char *name, FileNameBuffer &resolvedName)
{
    // Path name has to be absolute.
    if (*name != '/')
    {
        return false;
    }

    // we copy caracter-by-character
    size_t dest = 0;
    size_t previousSlash = dest;
    resolvedName.at(dest) = '/';

    // For each character in the path name, decide whether, and where, to copy.
    for (const char *p = name; *p != '\0'; p++)
    {
        if (*p == '/')
        {
            // Only adjust previousSlash if we don't have a "." coming up next.
            if (*(p + 1) != '.')
            {
                previousSlash = dest;
            }
            if (resolvedName.at(dest) == '/')
            {
                // Remove double "/"
                continue;
            }
            resolvedName.at(++dest) = *p;
        }
        else if (*p == '.')
        {
            if (resolvedName.at(dest) == '/')
            {
                char next = *(p + 1);
                if (next == '\0' || next == '/')
                {
                    // Don't copy the ".", if at the end, the trailing "/" will
                    // be removed in the final step. If it is: "./", the double
                    // "//" will be removed in the next iteration.
                    continue;
                }
                else if (next == '.')
                {
                    // We have "..", but we don't do anything unless the next
                    // position is a "/" or the end.  (In case of a file like:
                    // ..my.file)
                    next = *(p + 2);
                    if (next == '\0' || next == '/')
                    {
                        p++;
                        dest = previousSlash;

                        // Now we probably have to push prevSl back, unless we
                        // are at the root of the file system.
                        while (previousSlash > 0)
                        {
                            if (resolvedName.at(--previousSlash) == '/')
                            {
                                break;
                            }
                        }
                        continue;
                    }
                }
                resolvedName.at(++dest) = *p;
            }
            else
            {
                resolvedName.at(++dest) = *p;
            }
        }
        else
        {
            resolvedName.at(++dest) = *p;
        }
    }

    // Terminate. Where, depends on several things.
    if (resolvedName.at(dest) == '/' && dest != 0)
    {
        // overwrite a trailing slash
        resolvedName.at(dest) = '\0';
    }
    else
    {
        resolvedName.at(++dest) = '\0';
    }
    return true;
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
    // only delete files if we have write permision
    if (!canWrite(name))
    {
        return EACCES;
    }
    return unlink(name) == 0 ? 0 : errno;
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
    return remove(name) == 0 ? 0 : errno;
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
    struct stat64 finfo;                 /* return buf for the finfo   */

    int rc = stat64(name, &finfo);       /* read the info about it     */
    return rc == 0 && S_ISDIR(finfo.st_mode);
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
    return access(name, W_OK) != 0 && access(name, R_OK) == 0;
}


/**
 * Test is a file is readable .
 *
 * @param name   The target file name.
 *
 * @return true if the file can be read.
 */
bool SysFileSystem::canRead(const char *name)
{
    return access(name, R_OK) == 0;
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
    return access(name, R_OK) != 0 && access(name, W_OK) == 0;
}


/**
 * Test if the process can write to the file. This is not the
 * same as being read only
 *
 * @param name   The target file name.
 *
 * @return true if the file is only writeable.  false if read
 *         operations are permitted.
 */
bool SysFileSystem::canWrite(const char *name)
{
    return access(name, W_OK) == 0;
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
    struct stat64 finfo;                 /* return buf for the finfo   */

    int rc = stat64(name, &finfo);       /* read the info about it     */
    return rc == 0 && (S_ISREG(finfo.st_mode) || S_ISBLK(finfo.st_mode));
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
    struct stat64 finfo;                 /* return buf for the finfo   */

    int rc = stat64(name, &finfo);       /* read the info about it     */
    return rc == 0;
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
    struct stat64 finfo;                   /* return buf for the finfo   */

    int rc = lstat64(name, &finfo);        /* read the info about it     */
    return rc == 0 && S_ISLNK(finfo.st_mode);
}


/*
  getLastModifiedDate / setLastModifiedDate helper function

  loosely based on http://stackoverflow.com/questions/283166/easy-way-to-convert-a-struct-tm-expressed-in-utc-to-time-t-type
  returns local timezone offset from UTC *including* DST offest in seconds
*/
int get_utc_offset(time_t time)
{
    struct tm gmt, local;
    int one_day = 24 * 60 * 60;
    int offset;

    // there seems to be no easy way to find the local timezone offset from UTC,
    // including the DST offset, for a specific UTC time
    // to calculate the UTC/DST offset we subtract the localtime() timestamp from
    // the gmtime() timestamp
    // NOTE: how this does/should work during the (typically) one-hour
    //       DST transition period, remains to be investigated

    gmtime_r(&time, &gmt);               // use re-entrant gmtime() version
    localtime_r(&time, &local);          // use re-entrant localtime() version

    // if both local and UTC timestamps fall on the same day, this is the UTC/DST offset
    offset = ((local.tm_hour - gmt.tm_hour) * 60
              + (local.tm_min - gmt.tm_min)) * 60
        + local.tm_sec - gmt.tm_sec;

    // if either local year or local day_in_year is less than its UTC counterpart,
    // we're dealing with a negative UTC/DST offset which extends into the prior day
    // we'll have to subtract a full day from 'offset' to compensate
    if ((local.tm_year < gmt.tm_year) || (local.tm_yday < gmt.tm_yday))
    {
        offset -= one_day;
    }

    // similar to above, if we're dealing with a positive UTC/DST offset extending
    // into the next day, we'll have to add a full day to 'offset' to compensate
    if ((local.tm_year > gmt.tm_year) || (local.tm_yday > gmt.tm_yday))
    {
        offset += one_day;
    }

    return offset;
}


/**
 * Get the last modified file date as a file time value.
 *
 * @param name   The target name.
 *
 * @return the file time value for the modified date, or -999999999999999999
 *         for any errors.  The time is returned in microseconds.
 */
int64_t SysFileSystem::getLastModifiedDate(const char *name)
{
    struct stat64 st;

    if (stat64(name, &st))
    {
        return NoTimeStamp;
    }
    int64_t temp = (int64_t)st.st_mtime + get_utc_offset(st.st_mtime) + StatEpoch;
    temp *= 1000000;

    // add microseconds, if available
#ifdef HAVE_STAT_ST_MTIM
    temp += st.st_mtim.tv_nsec / 1000;
#elif defined HAVE_STAT_ST_MTIMESPEC
    temp += st.st_mtimespec.tv_nsec / 1000;
#endif

    return temp;
}


/**
 * Get the file last access date as a file time value.
 *
 * @param name   The target name.
 *
 * @return the file time value for the modified date, or -999999999999999999
 *         for any errors.  The time is returned in microseconds.
 */
int64_t SysFileSystem::getLastAccessDate(const char *name)
{
    struct stat64 st;

    if (stat64(name, &st))
    {
        return NoTimeStamp;
    }
    int64_t temp = (int64_t)st.st_atime + get_utc_offset(st.st_atime) + StatEpoch;
    temp *= 1000000;

    // add microseconds, if available
#ifdef HAVE_STAT_ST_MTIM
    temp += st.st_atim.tv_nsec / 1000;
#elif defined HAVE_STAT_ST_MTIMESPEC
    temp += st.st_atimespec.tv_nsec / 1000;
#endif

    return temp;
}


/**
 * Retrieve the size of a file.
 *
 * @param name   The name of the target file.
 *
 * @return the 64-bit file size.
 */
uint64_t SysFileSystem::getFileLength(const char *name)
{
    struct stat64 st;
    if (stat64(name, &st) != 0)
    {
        return 0;
    }
    return st.st_size;
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
    return mkdir(name, S_IRWXU | S_IRWXG | S_IRWXO) != -1;
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
    // it must exist
    if (!exists(name))
    {
        return false;
    }

    size_t length = strlen(name);
    for (size_t index = length - 1; index > 0; index--)
    {
        if (name[index] == '.' && (index > 0 && name[index - 1] == '/'))
        {
            return true;
        }
    }

    return false;
}


/**
 * Set the last modified date for a file.
 *
 * @param name   The target name.
 * @param time   The new file time (in ticks).
 *
 * @return true if the filedate was set correctly, false otherwise.
 */
bool SysFileSystem::setLastModifiedDate(const char *name, int64_t time)
{
    struct stat64 st;
    struct timeval times[2];
    if (stat64(name, &st) != 0)
    {
        return false;
    }

    // get microseconds, if available
    int64_t usec_a = 0;
#ifdef HAVE_STAT_ST_MTIM
    usec_a = st.st_atim.tv_nsec / 1000;
#elif defined HAVE_STAT_ST_MTIMESPEC
    usec_a = st.st_atimespec.tv_nsec / 1000;
#endif

    times[0].tv_sec = st.st_atime;
    times[0].tv_usec = usec_a;
    int64_t seconds = time / 1000000 - StatEpoch;
    times[1].tv_sec = seconds - get_utc_offset(seconds);
    times[1].tv_usec = time % 1000000;
    return utimes(name, times) == 0;
}


/**
 * Set the last access date for a file.
 *
 * @param name   The target name.
 * @param time   The new file time (in ticks).
 *
 * @return true if the filedate was set correctly, false otherwise.
 */
bool SysFileSystem::setLastAccessDate(const char *name, int64_t time)
{
    struct stat64 st;
    struct timeval times[2];
    if (stat64(name, &st) != 0)
    {
        return false;
    }

    // get microseconds, if available
    int64_t usec_m = 0;
#ifdef HAVE_STAT_ST_MTIM
    usec_m = st.st_mtim.tv_nsec / 1000;
#elif defined HAVE_STAT_ST_MTIMESPEC
    usec_m = st.st_mtimespec.tv_nsec / 1000;
#endif

    times[1].tv_sec = st.st_mtime;
    times[1].tv_usec = usec_m;
    int64_t seconds = time / 1000000 - StatEpoch;
    times[0].tv_sec = seconds - get_utc_offset(seconds);
    times[0].tv_usec = time % 1000000;
    return utimes(name, times) == 0;
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
    struct stat64 buffer;
    if (stat64(name, &buffer) != 0)
    {
        return false;
    }
    mode_t mode = buffer.st_mode;

    // this really turns off the write permissions
    mode = mode & ~(S_IWUSR | S_IWGRP | S_IWOTH);
    return chmod(name, mode) == 0;
}


/**
 * Set the write attribute on a file or directory.
 *
 * @param name   The target name.
 *
 * @return true if the attribute was set, false otherwise.
 */
bool SysFileSystem::setFileWritable(const char *name)
{
    struct stat64 buffer;
    if (stat64(name, &buffer) != 0)
    {
        return false;
    }
    mode_t mode = buffer.st_mode;
    // this really turns on the write permissions
    mode = mode | (S_IWUSR | S_IWGRP | S_IWOTH);

    return chmod(name, mode) == 0;
}



/**
 * indicate whether the file system is case sensitive.
 *
 * @return For Unix systems, always returns true. For MacOS,
 *         this needs to be determined on a case-by-case basis.
 *         This returns the information for the root file system
 */
bool SysFileSystem::isCaseSensitive()
{
#ifndef HAVE_PC_CASE_SENSITIVE
    return true;
#else
    long res = pathconf("/", _PC_CASE_SENSITIVE);
    if (res != -1)
    {
        return (res == 1);
    }
    // any error means this value is not supported for this file system
    // so the result is most likely true (unix standard)
    return true;
#endif
}


/**
 * test if an individual file is a case sensitive name
 *
 * @return For Unix systems, always returns true. For MacOS,
 *         this needs to be determined on a case-by-case basis.
 */
bool SysFileSystem::isCaseSensitive(const char *name)
{
#ifndef HAVE_PC_CASE_SENSITIVE
    return true;
#else
    AutoFree tmp = strdup(name);

    while (!SysFileSystem::exists(tmp))
    {
        size_t len = strlen(tmp);
        // scan backwards to find the previous directory delimiter
        for (; len > 0; len--)
        {
            // is this the directory delimiter?
            if (tmp[len] == '/')
            {
                tmp[len] = '\0';
                break;
            }
        }
        // ugly hack . . . to preserve the "/"
        if (len == 0)
        {
            tmp[len + 1] = '\0';
            break;
        }
    }

    // at this point the tmp variable contains something that exists
    long res = pathconf(tmp, _PC_CASE_SENSITIVE);
    if (res != -1)
    {
        return (res == 1);
    }

    // non-determined, just return true
    return true;
#endif
}


/**
 * Retrieve the file system root elements.  On Windows,
 * each of the drives is a root element.
 *
 * @return The number of roots located.
 */
int SysFileSystem::getRoots(FileNameBuffer &roots)
{
    // just one root to return
    roots = "/";
    return 1;
}


/**
 * Return the separator used for separating path names.
 *
 * @return The ASCII-Z version of the path separator.
 */
const char* SysFileSystem::getSeparator()
{
    return "/";
}


/**
 * Return the separator used for separating search path elements
 *
 * @return The ASCII-Z version of the path separator.
 */
const char* SysFileSystem::getPathSeparator()
{
    return ":";
}


/**
 * Return the string used as a line-end separator.
 *
 * @return The ASCII-Z version of the line end sequence.
 */
const char* SysFileSystem::getLineEnd()
{
    return "\n";
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
    while (true)
    {
        if (getcwd(directory, directory.capacity()) != NULL)
        {
            return true;
        }
        // erange indicates the buffer was too small. Anything else is a
        // full failure
        if (errno != ERANGE)
        {
            return false;
        }

        // expand by an arbitrary amount.
        directory.expandCapacity(256);
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
    return chdir(directory) == 0;
}



/**
 * Check to see if two paths are identical
 *
 * @param path1  The the first path to check
 * @param path2  The second path to check.
 *
 * @return true if these paths map to the same existing file, false if they are distinct.
 */
bool samePaths(const char *path1, const char *path2)
{
    AutoFree actualPath1 = realpath(path1, NULL);

    // realpath() gives a NULL return if the file does not exist. For this operation
    // at least one of the file must exist. If either does not exist, they cannot be bad.
    if (actualPath1 == NULL)
    {
        return false;
    }

    AutoFree actualPath2 = realpath(path2, NULL);

    if (actualPath2 == NULL)
    {
        return false;
    }

    // and compare them. Note that if we have a file in a case-insensitive file
    // system, we need to perform a caseless compare.
    if (!SysFileSystem::isCaseSensitive(actualPath1))
    {
        return strcasecmp((const char *)actualPath1, (const char *)actualPath2) == 0;
    }
    else
    {
        return strcmp((const char *)actualPath1, (const char *)actualPath2) == 0;
    }
}


/**
 * Copy a file to another file location with dereferencing of symbolic links.
 *  If the source file is a symbolic link, the actual file copied is the target of the symbolic link.
 *  If the destination file already exists and is a symbolic link, the target of the symbolic link is overwritten by the source file.
 * If the destination file does not exist then it is created with the access mode of the source file (if possible).
 *
 * @param fromFile The file we're copying from.
 * @param toFile   The file we're copying to
 * @param preserveTimestamps
 *                 if true, the copied file will have the same timestamps.
 * @param preserveMode
 *                 If true, the file modes are preserved in the new file.
 *
 * @return 0 if successful, or an error code for any failure.
 */
int copyFileDereferenceSymbolicLinks(const char *fromFile, const char *toFile, bool preserveTimestamps, bool preserveMode)
{
    // first verify we're not trying to copy a file onto itself.
    if (samePaths(fromFile, toFile))
    {
        return EEXIST;
    }

    // the from file must exist
    struct stat64 fromStat;
    if (stat64(fromFile, &fromStat) == -1)
    {
        return errno;
    }
    // and we must be abl to open it for reading
    AutoClose fromHandle = open64(fromFile, O_RDONLY);
    if (fromHandle == -1)
    {
        return errno;
    }

    // and we need to be able to
    struct stat64 toStat;
    bool toFileCreated = (stat64(toFile, &toStat) == -1);
    AutoClose toHandle = open64(toFile, O_WRONLY | O_CREAT | O_TRUNC, 0666); // default access mode for the moment (like fopen)
    if (toHandle == -1)
    {
        return errno;
    }


    char buffer[4096];          // a buffer of reasonable size for copying
    while (1)
    {
        int count = read(fromHandle, &buffer, sizeof(buffer));
        if (count == -1)
        {
            return errno;
        }
        if (count == 0)
        {
            break; // EOF
        }
        if (write(toHandle, buffer, count) == -1)
        {
            return errno;
        }
    }

    if (fromHandle.close() == -1)
    {
        return errno;
    }
    if (toHandle.close() == -1)
    {
        return errno;
    }
    // if asked to preserve the timestamps, then
    // copy the stat information from the from file.
    // NB, the access time is the last access before this copy operation.
    if (preserveTimestamps)
    {
        struct utimbuf timebuf;
        timebuf.actime = fromStat.st_atime;
        timebuf.modtime = fromStat.st_mtime;
        utime(toFile, &timebuf);
    }

    // if we created this file or preserveMode was specified,
    // set the mode to the same as the from file.
    if (toFileCreated || preserveMode)
    {
        chmod(toFile, fromStat.st_mode);
    }
    return 0;
}


/*
Returns a new filename whose directory path is the same as filename's directory
and that is not the same as the name of an existing file in this directory.
Used to temporarily rename a file in place (i.e. in its directory).
This new filename must be freed when no longer needed.
*/
char* temporaryFilename(const char *filename, int &errInfo)
{
    // allocate a buffer large enough to hold the file name plus the extra characters
    // we add to the end.
    size_t fullLength = strlen(filename) + 8;
    char *tempFileName = (char *)malloc(fullLength);
    if (tempFileName == NULL)
    {
        return NULL;
    }

    // generate a random starting point
    srand((int)time(NULL));
    size_t num = rand();
    // we only handle the lower six digits
    num = num % 6;
    size_t start = num;

    while (true)
    {
        char numstr[8];
        // append 6 random digits to the base file name
        snprintf(tempFileName, sizeof(numstr), "%s%06zu", filename, num);

        // if there's no matching file, we're finished.
        if (!SysFileSystem::fileExists(tempFileName))
        {
            return tempFileName;
        }

        // generate a new number for filling in the name
        num = (num + 1) % 6;

        // if we've wrapped around to where we started, time to give up
        if (num == start)
        {
            return NULL;
        }
    }
}


/**
 * Copy a file without dereferencing the symbolic links.
 *  If the source file is a symbolic link, the actual file copied is the symbolic link itself.
 *  If the destination file already exists and is a symbolic link, the target of the symbolic link is not overwritten by the source file.
 *
 * again, the source and the target cannot be the same
 *
 * @param fromFile The file to copy from.
 * @param toFile   The file to copy to
 * @param force    Force the rename if the target exists as a symbolic link
 * @param preserveTimestamps
 *                 preserve the time stamps (if possible)
 * @param preserveMode
 *                 Preserve the file mode (if possible)
 *
 * @return 0 if successful, or the appropriate error code.
 */
int copyFileDontDereferenceSymbolicLinks(const char *fromFile, const char *toFile, bool force, bool preserveTimestamps, bool preserveMode)
{
    // again, the source and the target cannot be the same
    if (samePaths(fromFile, toFile))
    {
        return EEXIST;
    }

    struct stat64 fromStat;
    if (lstat64(fromFile, &fromStat) == -1)
    {
        return errno;
    }

    // remember if we are copying from a symbolic link
    bool fromFileIsSymbolicLink = S_ISLNK(fromStat.st_mode);

    // and also get the link state for the copy target.
    struct stat64 toStat;
    bool toFileExists = (lstat64(toFile, &toStat) == 0);
    bool toFileIsSymbolicLink = (toFileExists && S_ISLNK(toStat.st_mode));

    AutoFree toFileNewname;

    // if we have an existing file and either is a symbolic link, we need to
    // create a temporary file.
    if (toFileExists && (fromFileIsSymbolicLink || toFileIsSymbolicLink))
    {
        // Must remove toFile when fromFile is a symbolic link, to let copy the link.
        // Must remove toFile when toFile is a symbolic link, to not follow the link.
        // But do that safely : first rename toFile, then do the copy and finally remove the renamed toFile.
        if (force == false)
        {
            return EEXIST; // Not allowed by caller to remove it
        }
        int errInfo;
        // create a temporary file name in the same path location as the toFile
        toFileNewname = temporaryFilename(toFile, errInfo);
        if (errInfo != 0)
        {
            return errInfo;
        }
    }

    // we are dealing with a symbolic link for the source. We are going to create the target as
    // a symbolic link back to the original file rather than copy the data
    if (fromFileIsSymbolicLink)
    {
        off_t pathSize = fromStat.st_size; // The length in bytes of the pathname contained in the symbolic link
        AutoFree pathBuffer = (char *)malloc(pathSize + 1);
        if (pathBuffer == NULL)
        {
            return errno;
        }
        if (readlink(fromFile, pathBuffer, pathSize) == -1)
        {
            return errno;
        }
        pathBuffer[pathSize] = '\0';

        // if the target file exists, rename to the temporary file name (if we can)
        if (toFileNewname != NULL && rename(toFile, toFileNewname) == -1)
        {
            return errno;
        }

        // Now create a symbolic link between the linked name and the target file.
        if (symlink(pathBuffer, toFile) == -1)
        {
            int errInfo = errno;
            // Undo the renaming
            if (toFileNewname != NULL)
            {
                rename(toFileNewname, toFile);
            }
            return errInfo;
        }
        // Note : there is no API to preserve the timestamps and mode of a symbolic link
    }
    else
    {
        // do we need to rename the target?
        if (toFileNewname != NULL && rename(toFile, toFileNewname) == -1)
        {
            return errno;
        }

        // this is a real copy operation
        int errInfo = copyFileDereferenceSymbolicLinks(fromFile, toFile, preserveTimestamps, preserveMode);
        if (errInfo != 0)
        {
            // Undo the renaming
            if (toFileNewname != NULL)
            {
                rename(toFileNewname, toFile);
            }
            return errInfo;
        }
    }

    // The copy has been done, now can remove the backup
    if (toFileNewname != NULL)
    {
        unlink(toFileNewname);
    }
    return 0;

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
    // this is a direct copy operation. We preserve the timestamps, but not
    // the mode.
    return copyFileDereferenceSymbolicLinks(fromFile, toFile, true, false);
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
    // rename does not return an error if both files resolve to the same file
    // but we want to return an error in this case, to follow the behavior of mv
    if (samePaths(fromFile, toFile))
    {
        return EEXIST; // did not find a better error code to return
    }
    // for consistency with other platforms, we don't do the move if the target is
    // and existing file.
    if (fileExists(toFile))
    {
        return EEXIST;
    }

    if (rename(fromFile, toFile) == 0)
    {
        return 0; // move done
    }

    if (errno != EXDEV)
    {
        return errno; // move ko, no fallback
    }

    // Before trying the fallback copy+unlink, ensure that we can unlink fromFile.
    // If we can rename it in place, then we can unlink it.
    int errInfo;
    AutoFree fromFileNewname = temporaryFilename(fromFile, errInfo);
    if (errInfo != 0)
    {
        return errInfo;
    }

    if (rename(fromFile, fromFileNewname) == -1)
    {
        return errno;
    }
    // Good, here we know we can unlink fromFile, undo the rename
    if (rename(fromFileNewname, fromFile) == -1)
    {
        return errno; // should not happen, but...
    }

    // The files are on different file systems and the implementation does not support that.
    // Try to copy then unlink
    int copyStatus = copyFileDontDereferenceSymbolicLinks(fromFile, toFile, false, true, true); // 3rd arg=false : dont't force (good choice ?)
    if (copyStatus != 0)
    {
        return copyStatus; // fallback ko
    }
    // Note : no error returned if timestamps or mode not preserved

    // copy to is ok, now unlink from
    // in case of error, don't remove toFile, it's a bad idea !
    return unlink(fromFile);
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
    return strrchr(name, PathDelimiter);             // find the last backslash in name
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
    // this always starts with the name
    return name;
}

/**
 * Retrieve the location where temporary files should be stored.
 * This will be the value of environment variable TMPDIR, if defined, or /tmp.
 *
 * @return true if successful, false otherwise.
 */
bool SysFileSystem::getTemporaryPath(FileNameBuffer &temporary)
{
    if (!SystemInterpreter::getEnvironmentVariable("TMPDIR", temporary))
    {
        temporary = "/tmp";
    }
    return true;
}



/**
 * Create a new SysFileIterator instance.
 *
 * @param path    The path we're going to be searching in
 * @param pattern The pattern to use. If NULL, then everything in the path will be returned.
 * @param buffer  A FileNameBuffer object used to construct the path name.
 */
SysFileIterator::SysFileIterator(const char *path, const char *pattern, FileNameBuffer &buffer, bool c)
{
    // save the pattern spec to check agains each path entry
    patternSpec = pattern;
    // and also save the directory we are searching in
    directory = path;

    // caseLess can be explicit or implicit, based on the characteristics of the path.
    // Mac file systems are generally case insensitive, but other unix variants are
    // usually case sensitive.
    caseLess = c || !SysFileSystem::isCaseSensitive(path);

#ifndef HAVE_FNM_CASEFOLD
    // if we're tasked with doing a caseless search but the option is
    // not available with fnmatch(), then we need to copy the pattern
    // spec and uppercase it.
    if (caseLess && patternSpec != NULL)
    {
        char *upperString = strdup(patternSpec);
        Utilities::strupper(upperString);
        patternSpec = upperString;
    }
#endif

    // this assumes we'll fail...if we find something,
    // we'll flip this
    completed = true;
    handle = opendir(path);
    // if didn't open, this either doesn't exist or
    // isn't a directory
    if (handle == NULL)
    {
        return;
    }
    // ok, we potentially have something to return
    completed = false;
    // go locate the first matching entry (if it can)
    findNextEntry();
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
    if (handle != 0)
    {
        closedir(handle);
        handle = 0;
    }
#ifndef HAVE_FNM_CASEFOLD
    // if we had to copy the patternSpec, make sure we free the copy up
    if (caseLess && patternSpec != NULL)
    {
        free((void *)patternSpec);
        patternSpec = NULL;
    }
#endif
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
        buffer = entry->d_name;
        attributes.findFileData = findFileData;
    }

    findNextEntry();
}


/**
 * Scan forward through the directory to find the next matching entry.
 *
 * @return true if a matching entry was found, false for any error or nothign found.
 */
void SysFileIterator::findNextEntry()
{
    entry = readdir(handle);
    if (entry == NULL)
    {
        // we're done once we hit a failure
        completed = true;
        close();
        return;
    }

    // requesting everything? we've got what we want
    if (patternSpec == NULL)
    {
        // we need to perform the stat64() using the fully resolved name.
        // if there is an allocation error here, we have limited ability to raise
        // an error here so we will just fail silently and return incorrect information.
        // There is very low probability anybody will ever see this error.
        size_t bufferLength = strlen(directory) + strlen(entry->d_name) + 8;

        AutoFree fullName = (char *)malloc(bufferLength);
        if (fullName == (char *)NULL)
        {
            return;
        }
        snprintf(fullName, bufferLength, "%s/%s", directory, entry->d_name);

        // save the attribute information for this file
        stat64(fullName, &findFileData);
        return;
    }

    int flags = FNM_NOESCAPE | FNM_PATHNAME;
    const char *testName = entry->d_name;

    // if this is a caseless search, there are two ways to
    // implement this. The easy way is to use the FNM_CASEFOLD
    // extension of fnmatch(). If this is not available, we need
    // to create an uppercase version of the name we're matching against
    // and compare against the already uppercased pattern.
    if (caseLess)
    {
#ifdef HAVE_FNM_CASEFOLD
        flags |= FNM_CASEFOLD;
#else
        char *upperName = strdup(testName);
        Utilities::strupper(upperName);
        testName = upperName;
#endif
    }

    // use fnmatch() to handle all of the globbing
    while (fnmatch(patternSpec, testName, flags) != 0)
    {
#ifndef HAVE_FNM_CASEFOLD
        // free the uppercase copy of the last test
        free((void *)testName);
#endif
        entry = readdir(handle);
        if (entry == NULL)
        {
            // we're done once we hit a failure
            completed = true;
            close();
            return;
        }
        testName = entry->d_name;
#ifndef HAVE_FNM_CASEFOLD
        char *upperName = strdup(testName);
        Utilities::strupper(upperName);
        testName = upperName;
#endif
    }
#ifndef HAVE_FNM_CASEFOLD
    // free the uppercase copy of the last test
    free((void *)testName);
#endif
    // we need to perform the stat64() using the fully resolved name.
    // if there is an allocation error here, we have limited ability to raise
    // an error here so we will just fail silently and return incorrect information.
    // There is very low probability anybody will ever see this error.
    size_t bufferLength = strlen(directory) + strlen(entry->d_name) + 8;

    AutoFree fullName = (char *)malloc(bufferLength);
    if (fullName == (char *)NULL)
    {
        return;
    }
    snprintf(fullName, bufferLength, "%s/%s", directory, entry->d_name);

    // save the attribute information for this file
    stat64(fullName, &findFileData);
    return;
}



