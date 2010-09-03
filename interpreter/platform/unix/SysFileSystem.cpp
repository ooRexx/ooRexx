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
#include <unistd.h>
#include <utime.h>
#include <pwd.h>
#include <errno.h>
#include "SysFileSystem.hpp"
#include "Utilities.hpp"

const char SysFileSystem::EOF_Marker = 0x1A;
const char *SysFileSystem::EOL_Marker = "\n";
const char SysFileSystem::PathDelimiter = '/';

/*********************************************************************/
/*                                                                   */
/* FUNCTION    : SearchFileName                                      */
/*                                                                   */
/* DESCRIPTION : Search for a given filename, returning the fully    */
/*               resolved name if it is found.                       */
/*                                                                   */
/*********************************************************************/

bool SysFileSystem::searchFileName(
  const char* name,                    /* name of rexx proc to check        */
  char *     fullName )                /* fully resolved name               */
{
    size_t nameLength;                   /* length of name                    */
    char tempPath[MaximumFileNameBuffer];// temporary place to store the path/name
    char * currentpath;                  // current path
    char * sep;                          // next colon in the path

    nameLength = strlen(name);           /* get length of incoming name       */

    /* if name is too small or big */
    if (nameLength < 1 || nameLength > MaximumFileNameBuffer)
    {
        return false;
    }

    /* does the filename already have a path? */
    /* or does it start with "~" ? */
	/* (beware, don't test "." because files like ".hidden" alone are candidate for search in PATH */
    if (strstr(name, "/") != NULL || name[0] == '~' || name[0] == '.')
    {
        bool done = SysFileSystem::canonicalizeName(tempPath);
        if (done == false || fileExists(tempPath) == false)
        {
            fullName[0] = '\0';
            return false;
        }
        strcpy(fullName, tempPath);
        return true;
    }

    /* there was no leading path so try the current directory */
    getcwd(tempPath, MaximumFileNameBuffer);
    strcat(tempPath, "/");
    strcat(tempPath, name);
    if (fileExists(name) == true)
    {
        strcpy(fullName, name);
        return true;
    }

    /* it was not in the current directory so search the PATH */
    currentpath = getenv("PATH");
    if (currentpath == NULL)
    {
        fullName[0] = '\0';
        return false;
    }
    sep = strchr(currentpath, ':');
    while (sep != NULL)
    {
        /* try each entry in the PATH */
        int i = sep - currentpath;
        strncpy(tempPath, currentpath, i);
        tempPath[i] = '\0';
        strcat(tempPath, "/");
        strcat(tempPath, name);
        if (fileExists(tempPath) == true)
        {
            strcpy(fullName, tempPath);
            return true;
        }
        currentpath = sep + 1;
        sep = strchr(currentpath, ':');
    }
    /* the last entry in the PATH may not be terminated by a colon */
    if (*currentpath != '\0')
    {
        strcpy(tempPath, currentpath);
        strcat(tempPath, "/");
        strcat(tempPath, name);
        if (fileExists(tempPath) == true)
        {
            strcpy(fullName, tempPath);
            return true;
        }
    }

    /* file not found */
    fullName[0] = '\0';
    return false;
}

/*********************************************************************/
/*                                                                   */
/* FUNCTION    : getTempFileName                                     */
/*                                                                   */
/* DESCRIPTION : Returns a temp file name.                           */
/*                                                                   */
/*********************************************************************/
const char *SysFileSystem::getTempFileName()
{
    return tmpnam(NULL);
}


void SysFileSystem::qualifyStreamName(
  const char *name,                   // input name
  char *fullName,                     // the output name
  size_t bufferSize)                  // size of the output buffer
/*******************************************************************/
/* Function:  Qualify a stream name for this system                */
/*******************************************************************/
{
    char tempPath[MaximumFileNameBuffer];// temporary place to store the path/name

    /* already expanded? */
    if (*fullName != '\0')
    {
        return;                            /* nothing more to do                */
    }

    strcpy(tempPath, name);
    bool done = SysFileSystem::canonicalizeName(tempPath);
    if (done && strlen(tempPath) < bufferSize)
    {
        strcpy(fullName, tempPath);
    }
    else
    {
        fullName[0] = '\0'; // or leave it unchanged ?
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
bool SysFileSystem::fileExists(const char * fname)
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
RexxString *SysFileSystem::extractDirectory(RexxString *file)
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
RexxString *SysFileSystem::extractExtension(RexxString *file)
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
RexxString *SysFileSystem::extractFile(RexxString *file)
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
 * @param name      The name to search for.
 * @param directory A specific directory to look in first (can be NULL).
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
    char       tempName[PATH_MAX + 3];

    // construct the search name, potentially adding on an extension
    strncpy(tempName, name, sizeof(tempName));
    if (extension != NULL)
    {
        strncat(tempName, extension, sizeof(tempName));
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
            Utilities::strlower(tempName);
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
            Utilities::strlower(tempName);
        }
        return false;
    }
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
    // validate that this is a name that can even be located.
    size_t nameLength = strlen(name);

    if (nameLength < 1 || nameLength > PATH_MAX + 1)
    {
        return false;
    }

    // make a copy of the input name
    strcpy(resolvedName, name);
    // take care of any special conditions in the name structure
    // a failure here means an invalid name of some sort
    if (!canonicalizeName(resolvedName))
    {
        return false;
    }

    struct stat64 dummy;                 /* structure for stat system calls   */

    // ok, if this exists, life is good.  Return it.
    if (stat64(resolvedName, &dummy) == 0)           /* look for file              */
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
 * @param name      The name to search for.
 * @param path      The search path to use.
 * @param resolvedName
 *                  A buffer used for returning the resolved name.
 *
 * @return Returns true if the file was located.  If true, the resolvedName
 *         buffer will contain the returned name.
 */
bool SysFileSystem::searchPath(const char *name, const char *path, char *resolvedName)
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
        size_t sublength = q - p;

        memcpy(resolvedName, p, sublength);
        resolvedName[sublength] = '/';
        resolvedName[sublength + 1] = '\0';
        strncat(resolvedName, name, PATH_MAX + 1);

        // take care of any special conditions in the name structure
        // a failure here means an invalid name of some sort
        if (canonicalizeName(resolvedName))
        {
            struct stat64 dummy;
            if (stat64(resolvedName, &dummy) == 0)   /* If file is found,          */
            {
                // this needs to be a regular file
                if (S_ISREG(dummy.st_mode))
                {
                    return true;
                }
                return false;
            }
        }
    }
    return false;
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
bool SysFileSystem::canonicalizeName(char *name)
{
    // does it start with the user home marker?
    if (name[0] == '~')
    {
        // this is the typical case.  This is a directory based off of
        // the current users home directory.
        if (name[1] == '\0' || name[1] == '/')
        {

            char tempName[PATH_MAX + 3];
            // make a copy of the name
            strncpy(tempName, name, PATH_MAX + 1);
            strcpy(name, getenv("HOME"));
            // We don't need to add a slash : If we have "~" alone, then no final slash expected (same as for "~user"). If "~/..." then we have the slash already
            strncat(name, tempName + 1, PATH_MAX + 1);
        }
        else
        {
            // referencing a file in some other user's home directory.
            // we need to extract the username and resolve that home directory
            char tempName[PATH_MAX + 3];
            char userName[PATH_MAX + 3];

            // make a copy of the name
            strncpy(tempName, name, PATH_MAX + 1);
            // look for the start of a directory
            char *slash = strchr(tempName,'/');
            // if there is a directory after the username, we need
            // to copy just the name piece
            if (slash != NULL)
            {
                size_t nameLength = slash - tempName - 1;
                memcpy(userName, tempName + 1, nameLength);
                userName[nameLength] = '\0';
            }
            // all username, just copy
            else
            {
                strcpy(userName, tempName + 1);
            }

            // see if we can retrieve the information
            struct passwd *ppwd = getpwnam(userName);
            if (ppwd == NULL)
            {
                // this is not valid without user information, so just fail the operation
                // if we can't get this.
                return false;                    /* nothing happend            */
            }

            strncpy(name, ppwd->pw_dir, PATH_MAX + 1);
            // if we have a directory after the username, copy the whole thing
            if (slash != NULL)
            {
                strncat(name, slash, PATH_MAX + 1);
            }
        }
    }

    // if we're not starting with root, we need to add the
    // current working directory.  This will also handle the
    // "." and ".." cases, which will be removed by the canonicalization
    // process.
    else if (name[0] != '/')
    {
        char tempName[PATH_MAX + 2];
        // make a copy of the name
        strncpy(tempName, name, PATH_MAX + 1);
        getcwd(name, PATH_MAX + 1);
        strncat(name, "/", PATH_MAX + 1);
        strncat(name, tempName, PATH_MAX + 1);
    }

    // NOTE:  realpath() is more portable than canonicalize_file_name().
    // However, they both have problems in that they both fail if the file does
    // not exist.  There are a number of places where the interpreter needs to
    // canonicalize a path name whether the file exists or not.  So we use our
    // own function to normalize the name.
    char tempName[PATH_MAX + 2];
    if ( normalizePathName(name, tempName) )
    {
        strcpy(name, tempName);
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
bool SysFileSystem::normalizePathName(const char *name, char *resolved)
{
    // Path name has to be absolute.
    if ( *name != '/' )
    {
        return false;
    }

    char *dest = resolved;
    char *prevSl = dest;
    *dest = '/';

    // For each character in the path name, decide whether, and where, to copy.
    for ( const char *p = name; *p; p++ )
    {
        if ( *p == '/' )
        {
            // Only adjust prevSl if we don't have a "." coming up next.
            if ( *(p + 1) != '.' )
            {
                prevSl = dest;
            }
            if ( *dest == '/' )
            {
                // Remove double "/"
                continue;
            }
            *++dest = *p;
        }
        else if ( *p == '.' )
        {
            if ( *dest == '/' )
            {
                char next = *(p + 1);
                if ( next == '\0' || next == '/' )
                {
                    // Don't copy the ".", if at the end, the trailing "/" will
                    // be removed in the final step. If it is: "./", the double
                    // "//" will be removed in the next iteration.
                    continue;
                }
                else if ( next == '.' )
                {
                    // We have "..", but we don't do anything unless the next
                    // position is a "/" or the end.  (In case of a file like:
                    // ..my.file)
                    next = *(p + 2);
                    if ( next == '\0' || next == '/' )
                    {
                        p++;
                        dest = prevSl;

                        // Now we probably have to push prevSl back, unless we
                        // are at the root of the file system.
                        while ( prevSl > resolved )
                        {
                            if ( *--prevSl == '/' )
                            {
                                break;
                            }
                        }
                        continue;
                    }
                }
                *++dest = *p;
            }
            else
            {
                *++dest = *p;
            }
        }
        else
        {
            *++dest = *p;
        }
    }

    // Terminate. Where, depends on several things.
    (*dest == '/' && dest != resolved) ? *dest = '\0' : *++dest = '\0';

    return true;
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
    return unlink(name) == 0;
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
    return remove(name) == 0;
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
    return access(name, W_OK) != 0;
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
    return access(name, R_OK) != 0;
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
 * Test if a file exists using a fully qualified name.
 *
 * @param name   The target file name.
 *
 * @return True if the file exists, false if it is unknown.
 */
bool SysFileSystem::exists(const char *name)
{
    struct stat64 finfo;                 /* return buf for the finfo   */

    int rc = stat64(name, &finfo);       /* read the info about it     */
    return rc == 0;
}


/**
 * Get the last modified file date as a file time value.
 *
 * @param name   The target name.
 *
 * @return the file time value for the modified date, or -1 for any
 *         errors.  The time is returned in ticks units
 */
int64_t SysFileSystem::getLastModifiedDate(const char *name)
{
    struct stat64 st;
    tzset ();

    if (stat64(name, &st))
    {
        return -1;
    }
    return (int64_t)st.st_mtime;
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
 * Move (rename) a file.
 *
 * @param oldName The name of an existing file.
 * @param newName The new file name.
 *
 * @return A success/failure flag.
 */
bool SysFileSystem::moveFile(const char *oldName, const char *newName)
{
    return rename(oldName, newName) == 0;
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
    struct stat64 statbuf;
    struct utimbuf timebuf;
    if (stat64(name, &statbuf) != 0)
    {
        return false;
    }

    timebuf.actime = statbuf.st_atime;
    timebuf.modtime = (time_t)time;
    return utime(name, &timebuf) == 0;
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
    mode = mode & 07555;
    return chmod(name, mode) == 0;
}


/**
 * indicate whether the file system is case sensitive.
 *
 * @return For Unix systems, always returns true.
 */
bool SysFileSystem::isCaseSensitive()
{
    return true;
}


/**
 * Retrieve the file system root elements.  On Windows,
 * each of the drives is a root element.
 *
 * @return The number of roots located.
 */
int SysFileSystem::getRoots(char *roots)
{
    // just one root to return
    strcpy(roots, "/");
    return 1;
}


/**
 * Return the separator used for separating path names.
 *
 * @return The ASCII-Z version of the path separator.
 */
const char *SysFileSystem::getSeparator()
{
    return "/";
}


/**
 * Return the separator used for separating search path elements
 *
 * @return The ASCII-Z version of the path separator.
 */
const char *SysFileSystem::getPathSeparator()
{
    return ":";
}


/**
 * Create a new SysFileIterator instance.
 *
 * @param p      The directory we're iterating over.
 */
SysFileIterator::SysFileIterator(const char *p)
{
    // this assumes we'll fail...if we find something,
    // we'll flip this
    completed = true;
    handle = opendir(p);
    // if didn't open, this either doesn't exist or
    // isn't a directory
    if (handle == NULL)
    {
        return;
    }
    entry = readdir(handle);
    if (entry == NULL)
    {
        closedir(handle);
        return;
    }
    // we have a value
    completed = false;
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
        strcpy(buffer, entry->d_name);
    }
    entry = readdir(handle);
    if (entry == NULL)
    {
        // we're done once we hit a failure
        completed = true;
        close();
    }
}
