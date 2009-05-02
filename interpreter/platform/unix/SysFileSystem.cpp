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
#include <pwd.h>
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
    if (strstr(name, "/") != NULL)
    {
        switch (*name)
        {
            case '~':
                strcpy(tempPath, getenv("HOME"));
                strcat(tempPath, name + 1);
                break;
            case '.':
                getcwd(tempPath, MaximumFileNameBuffer);
                strcat(tempPath, name + 1);
                break;
            case '/':
                strcpy(tempPath, name);
                break;
            default:
                getcwd(tempPath, MaximumFileNameBuffer);
                strcat(tempPath, "/");
                strcat(tempPath, name);
                break;
        }
        if (fileExists(tempPath) == false)
        {
            strcpy(fullName, tempPath);
            return false;
        }
        return true;
    }

    /* there was no leading path so try the current directory */
    getcwd(tempPath, MaximumFileNameBuffer);
    strcat(tempPath, "/");
    strcat(tempPath, name);
    if (fileExists(name) == false)
    {
        strcpy(fullName, name);
        return false;
    }

    /* it was not in the current directory so search the PATH */
    currentpath = getenv("PATH");
    if (currentpath == NULL)
    {
        return true;
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
        if (fileExists(tempPath) == false)
        {
            strcpy(fullName, tempPath);
            return false;
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
        if (fileExists(tempPath) == false)
        {
            strcpy(fullName, tempPath);
            return false;
        }
    }

    /* file not found */
    return false;
}

/*********************************************************************/
/*                                                                   */
/* FUNCTION    : getTempFileName                                     */
/*                                                                   */
/* DESCRIPTION : Returns a temp file name.                           */
/*                                                                   */
/*********************************************************************/
char *SysFileSystem::getTempFileName()
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

    /* does the filename already have a path? */
    if (strstr(name, "/") != NULL)
    {
        switch (*name)
        {
            case '~':
                strcpy(tempPath, getenv("HOME"));
                strcat(tempPath, name + 1);
                break;
            case '.':
                getcwd(tempPath, MaximumFileNameBuffer);
                strcat(tempPath, name + 1);
                break;
            case '/':
                strcpy(tempPath, name);
                break;
            default:
                getcwd(tempPath, MaximumFileNameBuffer);
                strcat(tempPath, "/");
                strcat(tempPath, name);
                break;
        }
        if (strlen(tempPath) < bufferSize)
        {
            strcpy(fullName, tempPath);
        }
        else
        {
            *fullName = '\0';
        }
        return;
    }

    /* there was no leading path so try the current directory */
    getcwd(tempPath, MaximumFileNameBuffer);
    strcat(tempPath, "/");
    strcat(tempPath, name);
    if (strlen(tempPath) < bufferSize)
    {
        strcpy(fullName, tempPath);
    }
    else
    {
        *fullName = '\0';
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
    struct stat filestat;                // file attributes
    int rc;                              // stat function return code

    rc = stat(fname, &filestat);
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
    const char *endPtr = name + strlen(name) - 1;

    // scan backwards looking for a directory delimiter.  This name should
    // be fully qualified, so we don't have to deal with drive letters
    while (name < endPtr)
    {
        // find the first directory element?
        if (*endPtr == '/')
        {
            return true;         // found a directory delimiter
        }
        endPtr--;
    }
    return false;          // no directory
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

    // for each name, check in both the provided case and lower case.
    for (int i = 0; i < 2; i++)
    {
        // check the file as is first
        if (checkCurrentFile(tempName, resolvedName))
        {
            return true;
        }

        // we don't do path searches if there's directory information in the name
        if (!hasDirectory(tempName))
        {
            // go search along the path
            if (searchPath(tempName, path, resolvedName))
            {
                return true;
            }
        }
        // try again in lower case
        Utilities::strlower(tempName);
    }
    return false;
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

    struct stat dummy;                   /* structure for stat system calls   */

    // ok, if this exists, life is good.  Return it.
    if (stat(resolvedName, &dummy) == 0)             /* look for file              */
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
            struct stat dummy;
            if (stat(resolvedName, &dummy) == 0)     /* If file is found,          */
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
        if (name[1] == '/')
        {

            char tempName[PATH_MAX + 3];
            // make a copy of the name
            strncpy(tempName, name, PATH_MAX + 1);
            strcpy(name, getenv("HOME"));
            // if we need a separator, add one
            if (name[1] != '/')
            {
                strncat(name, "/", PATH_MAX + 1);
            }
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

    // NOTE:  realpath() is more portable than canonicalize_file_name().  The biggest difference between them
    // is support for really long file names (longer than PATH_MAX).  Since everything we've done up to this point
    // has assumed that PATH_MAX is the limit, it probably doesn't make much sense to start worrying about this
    // at the very last stage of the process.
    char tempName[PATH_MAX + 2];
    char *temp = realpath(name, tempName);
    if (temp == NULL)
    {
        return false;
    }
    strcpy(name, tempName);
    return true;
}


