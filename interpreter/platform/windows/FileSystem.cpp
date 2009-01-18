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
/* REXX Kernel                                                                */
/*                                                                            */
/* Windows specific file related routines.                                    */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "Interpreter.hpp"
#include "SystemInterpreter.hpp"
#include "InterpreterInstance.hpp"
#include "SysFileSystem.hpp"
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <conio.h>
#define MAX_STDOUT_LENGTH     32767    /* max. amount of data to push to STDOUT @THU007A */ /* @HOL007M */

#if (WINVER < 0x0600)
#define COMPILE_NEWAPIS_STUBS          /* Allows GetLongPathName to run on  */
#define WANT_GETLONGPATHNAME_WRAPPER   /* NT versions less than SP 3 and on */
#define NO_SHOBJIDL_SORTDIRECTION      /* Windows 95                        */
#include <NewAPIs.h>
#endif


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
RexxString *SysInterpreterInstance::resolveProgramName(RexxString *_name, RexxString *_parentDir, RexxString *_parentExtension)
{
    char resolvedName[MAX_PATH + 2];    // finally resolved name

    const char *name = _name->getStringData();
    const char *parentDir = _parentDir == OREF_NULL ? NULL : _parentDir->getStringData();
    const char *parentExtension = _parentExtension == OREF_NULL ? NULL : _parentExtension->getStringData();
    const char *pathExtension = instance->searchPath == OREF_NULL ? NULL : instance->searchPath->getStringData();

    SysSearchPath searchPath(parentDir, pathExtension);


    // if the file already has an extension, this dramatically reduces the number
    // of searches we need to make.
    if (SysFileSystem::hasExtension(name))
    {
        if (SysFileSystem::searchName(name, searchPath.path, NULL, resolvedName))
        {
            return new_string(resolvedName);
        }
        return OREF_NULL;
    }

    // if we have a parent extension provided, use that in preference to any default searches
    if (parentExtension != NULL)
    {
        if (SysFileSystem::searchName(name, searchPath.path, parentExtension, resolvedName))
        {
            return new_string(resolvedName);
        }

    }

    // ok, now time to try each of the individual extensions along the way.
    for (size_t i = instance->searchExtensions->firstIndex(); i != LIST_END; i = instance->searchExtensions->nextIndex(i))
    {
        RexxString *ext = (RexxString *)instance->searchExtensions->getValue(i);

        if (SysFileSystem::searchName(name, searchPath.path, ext->getStringData(), resolvedName))
        {
            return new_string(resolvedName);
        }
    }

    // The file may purposefully have no extension.
    if (SysFileSystem::searchName(name, searchPath.path, NULL, resolvedName))
    {
        return new_string(resolvedName);
    }

    return OREF_NULL;
}

void SystemInterpreter::loadImage(
  char **imageBuffer,                  /* returned start of the image       */
  size_t *imageSize )                  /* size of the image                 */
/*******************************************************************/
/* Function:  Load the image into storage                          */
/*******************************************************************/
{
    char fullname[MAX_PATH + 1];    // finally resolved name
    // The file may purposefully have no extension.
    if (!SysFileSystem::primitiveSearchName(BASEIMAGE, getenv("PATH"), NULL, fullname))
    {
        Interpreter::logicError("no startup image");   /* can't find it                     */
    }

    /* try to open the file              */
    HANDLE fileHandle = CreateFile(fullname, GENERIC_READ, FILE_SHARE_READ,
                            NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        Interpreter::logicError("no startup image");   /* can't find it                     */
    }
    DWORD     bytesRead;                 /* number of bytes read              */
    /* Read in the size of the image     */
    ReadFile(fileHandle, imageSize, sizeof(size_t), &bytesRead, NULL);
    *imageBuffer = memoryObject.allocateImageBuffer(*imageSize);
    /* read in the image                 */
    ReadFile(fileHandle, *imageBuffer, (DWORD)*imageSize, &bytesRead, NULL);
    // set this to the actual size read.
    *imageSize = bytesRead;
    CloseHandle(fileHandle);                /* and close the file                */
}


RexxBuffer *SystemInterpreter::readProgram(
  const char *file_name)               /* program file name                 */
/*******************************************************************/
/* Function:  Read a program into a buffer                         */
/*******************************************************************/
{
  HANDLE        fileHandle;             /* open file access handle           */
  size_t   buffersize;                 /* size of read buffer               */
  RexxBuffer * buffer;                 /* buffer object to read file into   */
  BY_HANDLE_FILE_INFORMATION   status; /* file status information           */
  DWORD        bytesRead;              /* number of bytes read              */

  {
      UnsafeBlock releaser;
                           /* try to open the file              */
      fileHandle = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
      if (fileHandle == INVALID_HANDLE_VALUE)
      {
        return OREF_NULL;                  /* return nothing                    */
      }
                           /* retrieve the file size            */
      GetFileInformationByHandle(fileHandle, &status);
  }
  buffersize = status.nFileSizeLow;    /* get the file size                 */
  buffer = new_buffer(buffersize);     /* get a buffer object               */
  ProtectedObject p(buffer);
  {
      UnsafeBlock releaser;

                           /* read in a buffer of data   */
      if (ReadFile(fileHandle, buffer->getData(), (DWORD)buffersize, &bytesRead, NULL) == 0) {
        return OREF_NULL;                  /* return nothing                    */
      }
      CloseHandle(fileHandle);                /* close the file now         */
      return buffer;                       /* return the program buffer         */
  }
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
   SysFileSystem::qualifyStreamName((char *)name->getStringData(), nameBuffer, sizeof(nameBuffer)); /* expand the full name              */
                       /* uppercase this                    */
   strupr(nameBuffer);
                       /* get the qualified file name       */
   return new_string(nameBuffer);
}





