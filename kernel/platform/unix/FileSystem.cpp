/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* REXX AIX Support                                            aixfile.c      */
/*                                                                            */
/* AIX specific file related routines.                                        */
/*                                                                            */
/******************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "RexxCore.h"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "ProtectedObject.hpp"
#include "SystemInterpreter.hpp"
#include "SysInterpreterInstance.hpp"
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <limits.h>

#if defined( HAVE_SYS_FILIO_H )
# include <sys/filio.h>
#endif

#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_STROPTS_H
# include <stropts.h>
#endif

#define CCHMAXPATH PATH_MAX+1


/**
 * Extract directory information from a file name.
 *
 * @param file   The input file name.  If this represents a real source file,
 *               this will be fully resolved.
 *
 * @return The directory portion of the file name.  If the file name
 *         does not include a directory portion, then OREF_NULL is returned.
 */
RexxString *SystemInterpreter::extractDirectory(RexxString *file)
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
RexxString *SystemInterpreter::extractExtension(RexxString *file)
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
 * Resolve a program for intial loading or a subroutine call.
 *
 * @param _name      The target name.  This can be fully qualified, or a simple name
 *                   without an extension.
 * @param _parentDir The directory of the file of our calling program.  The first place
 *                   we'll look is in the same directory as the program.
 * @param _parentExtension
 *                   The extension our calling program has.  If there is an extension,
 *                   we'll use that version first before trying any of the default
 *                   extensions.
 *
 * @return A string version of the file name, if found.  Returns OREF_NULL if
 *         the program cannot be found.
 */
RexxString *SysInterpreterInstance::resolveProgram(RexxString *_name, RexxString *_parentDir, RexxString *_parentExtension)
{
    char resolvedName[CCHMAXPATH + 2];    // finally resolved name

    const char *name = _name->getStringData();
    const char *parentDir = _parentDir == OREF_NULL ? NULL : _parentDir->getStringData();
    const char *parentExtension = _parentExtension == OREF_NULL ? NULL : _parentExtension->getStringData();
    const char *pathExtension = instance->searchPath == OREF_NULL ? NULL : instance->searchPath->getStringData();

    SysSearchPath searchPath(parentDir, pathExtension);

    // if the file already has an extension, this dramatically reduces the number
    // of searches we need to make.
    if (hasExtension(name))
    {
        if (searchName(name, searchPath.path, NULL, resolvedName))
        {
            return new_string(resolvedName);
        }
        return OREF_NULL;
    }

    // if we have a parent extension provided, use that in preference to any default searches
    if (parentExtension != NULL)
    {
        if (searchName(name, searchPath.path, parentExtension, resolvedName))
        {
            return new_string(resolvedName);
        }

    }

    // ok, now time to try each of the individual extensions along the way.
    size_t count = searchExtensions->size();
    for (size_t i = 1; i <= count; i++)
    {
        RexxString *ext = (RexxString *)searchExtensions->get(i);

        if (searchName(name, searchPath.path, ext->getStringData(), resolvedName))
        {
            return new_string(resolvedName);
        }
    }

    // The file may purposefully have no extension.
    if (searchName(name, searchPath.path, NULL, resolvedName))
    {
        return new_string(resolvedName);
    }

    return OREF_NULL;
}


/**
 * Test if a filename has an extension.
 *
 * @param name   The name to check.
 *
 * @return true if an extension was found on the file, false if there
 *         is no extension.
 */
bool SysInterpreterInstance::hasExtension(const char *name)
{
    const char *endPtr = name + strlen(name) - 1

    // scan backwards looking for a directory delimiter.  This name should
    // be fully qualified, so we don't have to deal with drive letters
    while (name < endPtr)
    {
        // find the first directory element?
        if (*endPtr == '\\')
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
bool SysInterpreterInstance::searchName(const char *name, const char *path, const char *extension, char *resolvedName)
{
    UnsafeBlock releaser;
    // this is for building a temporary name
    char       tempName[CCHMAXPATH + 2];

    // construct the search name, potentially adding on an extension
    strncpy(tempName, name, sizeof(tempName));
    if (extension != OREF)
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
        strlower(tempName);
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
bool SysInterpreterInstance::checkCurrentFile(const char *name, char *resolvedName)
{
    // validate that this is a name that can even be located.
    size_t nameLength = strlen(name);

    if (nameLength < 1 || NameLength > CCHMAXPATH)
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
    if (stat(resolvedName, &dummy))             /* look for file              */
    {
        return true;
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
bool SysInterpreterInstance::searchPath(const char *name, const char *path, char *resolvedName)
{
    // get an end pointer
    const char *pathEnd = path + strlen(path);

    /* For every dir in searchpath*/
    for (const char *p = path, const char *q = strchr(p, ':'); p < pathEnd; p = q + 1, q = strchr(p, ':'))
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
        strncat(resolvedName, name, CCHMAXPATH)

        // take care of any special conditions in the name structure
        // a failure here means an invalid name of some sort
        if (canonicalizeName(resolvedName))
        {
            if (!stat(resolvedName, &dummy))     /* If file is found,          */
            {
                return true;
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
bool SysInterpreterInstance::canonicalizeName(char *name)
{
    // copy over the reduced form
    strncpy(name, tempName, CCHMAXPATH);

    // does it start with the user home marker?
    if (name[0] == '~')
    {
        // this is the typical case.  This is a directory based off of
        // the current users home directory.
        if (name[1] == '/')
        {

            char tempName[CCHMAXPATH + 2];
            // make a copy of the name
            strncpy(tempName, name, CCHMAXPATH);
            strcpy(name, getenv("HOME"));
            // if we need a separator, add one
            if (name[1] != '/')
            {
                strncat(name, "/", CCHMAXPATH);
            }
            strncat(name, tempName + 1, CCHMAXPATH);
        }
        else
        {
            // referencing a file in some other user's home directory.
            // we need to extract the username and resolve that home directory
            char tempName[CCHMAXPATH + 2];
            char userName[CCHMAXPATH + 2];

            // make a copy of the name
            strncpy(tempName, name, CCHMAXPATH);
            // look for the start of a directory
            char *slash = strchr(tempName,'/');
            // if there is a directory after the username, we need
            // to copy just the name piece
            if (!slash != NULL)
            {
                size_t nameLength = slash - tempName - 1;
                memcpy(userName, tempName + 1, nameLength;
                userName[nameLength] = '\0';
            }
            // all username, just copy
            else
            {
                strcpy(userName, tempName + 1);
            }

            // see if we can retrieve the information
            struct passwd *ppwd getpwnam(userName);
            if (ppwd == NULL)
            {
                // this is not valid without user information, so just fail the operation
                // if we can't get this.
                return false;                    /* nothing happend            */
            }

            strncpy(name, ppwd->pw_dir, CCHMAXPATH);
            // if we have a directory after the username, copy the whole thing
            if (slash != NULL)
            {
                strncat(name, slash, CCHMAXPATH);
            }
        }
    }

    // if we're not starting with root, we need to add the
    // current working directory.  This will also handle the
    // "." and ".." cases, which will be removed by the canonicalization
    // process.
    else if (name[0] != '/')
    {
        char tempName[CCHMAXPATH + 2];
        // make a copy of the name
        strncpy(tempName, name, CCHMAXPATH);
        getcwd(name, CCHMAXPATH);
        strncat(name, "/", CCHMAXPATH);
        strncat(name, tempName, CCHMAXPATH);
    }

    char *tempName = canonicalize_file_name(name);
    if (tempName == NULL)
    {
        return false;
    }
    return true;
}


/**
 * Portable implementation of an ascii-z string to uppercase (in place).
 *
 * @param str    String argument
 *
 * @return The address of the str unput argument.
 */
void strlower(char *str)
{
    while (*str)
    {
        *str = tolower(*str);
        str++;
    }

    return;
}


void SystemInterpreter::loadImage(char **imageBuffer, size_t *imageSize)
/*******************************************************************/
/* Function : Load the image into storage                          */
/*******************************************************************/
{
    FILE *image = NULL;
    const char *fullname;

    fullname = searchFileName(BASEIMAGE, 'P');  /* PATH search         */

#ifdef ORX_CATDIR
    if ( fullname == OREF_NULL )
    {
        fullname = ORX_CATDIR"/rexx.img";
    }
#endif

    if ( fullname != OREF_NULL )
    {
        image = fopen(fullname, "rb");/* try to open the file              */
    }
    else
    {
        logic_error("no startup image");   /* open failure                      */
    }

    if ( image == NULL )
    {
        logic_error("unable to open image file");
    }

    /* Read in the size of the image     */
    if (!fread(imageSize, 1, sizeof(size_t), image))
    {
        logic_error("could not check the size of the image");
    }
    /* Create new segment for image      */
    *imageBuffer = (char *)memoryObject.allocateImageBuffer(*imageSize);
    /* Create an object the size of the  */
    /* image. We will be overwriting the */
    /* object header.                    */
    /* read in the image, store the      */
    /* the size read                     */
    if (!(*imageSize = fread(*imageBuffer, 1, *imageSize, image)))
    {
        logic_error("could not read in the image");
    }
    fclose(image);                       /* and close the file                */
}


RexxBuffer *SystemInterpreter::readProgram(const char *file_name)
/*******************************************************************/
/* Function:  Read a program into a buffer                         */
/*******************************************************************/
{
    FILE    *handle;                     /* open file access handle           */
    size_t   buffersize;                 /* size of read buffer               */
    {
        handle = fopen(file_name, "rb");     /* open as a binary file             */
        if (handle == NULL)
        {                 /* open error?                       */
            return OREF_NULL;                  /* return nothing                    */
        }

        if (fileno(handle) == (FOPEN_MAX - 2))
        {      /* open error?                       */
            return OREF_NULL;                  /* return nothing                    */
        }

        fseek(handle, 0, SEEK_END);          /* seek to the file end              */
        buffersize = ftell(handle);          /* get the file size                 */
        fseek(handle, 0, SEEK_SET);          /* seek back to the file beginning   */
    }
    RexxBuffer *buffer = new_buffer(buffersize);     /* get a buffer object               */
    ProtectedObject p(buffer);
    {
        UnsafeBlock releaser;

        fread(buffer->getData(), 1, buffersize, handle);
        fclose(handle);                      /* close the file                    */
    }
    return buffer;                       /* return the program buffer         */
}

RexxString *SystemInterpreter::qualifyFileSystemName(
  RexxString * name)                   /* stream information block          */
/*******************************************************************/
/* Function:  Qualify a stream name for this system                */
/*******************************************************************/
{
   char nameBuffer[SysFileSystem::MaximumFileNameBuffer];

                       /* clear out the block               */
   memset(nameBuffer, 0, sizeof(nameBuffer));
   SysFileSystem::qualifyStreamName(name->getStringData(), nameBuffer, sizeof(nameBuffer)); /* expand the full name              */
                       /* uppercase this                    */
   SysUtil::strupr(nameBuffer);
                       /* get the qualified file name       */
   return new_string(nameBuffer);
}

bool SearchFirstFile(
  const char *Name)                     /* name of file with wildcards       */
{
    return(0);
}



#if defined( FIONREAD )
int SysPeekSTD(STREAM_INFO *stream_info)
{
  int c;

  /* ioctl returns number of fully received bytes from keyboard, after        */
  /* the Enter key has been hit. After the first byte has been read with      */
  /* charin, ioctl returns '0'. stream_file->_cnt returns '0' until a first   */
  /* charin has buffered input from STDIN. After that _cnt returns the num-   */
  /* of still buffered characters. If new input arrives through STDIN, the    */
  /* already buffered input is worked off, _cnt gets '0' and with that,       */
  /* ioctl returns a non zero value. With the first charin, the logic repeats */

  ioctl(stream_info->fh, FIONREAD, &c);
#if defined( HAVE_FILE__IO_READ_PTR )
  if ( (!c) && (!(stream_info->stream_file->_IO_read_ptr <
      stream_info->stream_file->_IO_read_end)) )
#elif defined( HAVE_FILE__CNT )
  if( (!c) && (!stream_info->stream_file->_cnt) )
#else
  if ( !c ) /* not sure what to do here ? */
#endif
   return(0);
  else
   return(1);
}
#endif




