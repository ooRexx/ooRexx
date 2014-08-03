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
    for (size_t i = 1; i <= instance->searchExtensions->items(); i++)
    {
        RexxString *ext = (RexxString *)instance->searchExtensions->get(i);

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


/**
 * Load the base image into storage.
 *
 * @param imageBuffer  returned start of the image
 * @param imageSize    returned size of the image
 *
 * @remarks  We pass null to primitiveSearchName(), which has the effect of the
 *           Windows API SearchPath() being called with null for the path.  When
 *           SearchPath is called with null for the path, and there is no path
 *           informtion in the file name, the first directory searched is the
 *           directory in which the application (rexx.exe) image is located.
 *
 *           Since, on Windows, the base image (rexx.img) is always located in
 *           the same directory as rexx.exe, this results in the image being
 *           found faster.  For development, it prevents the wrong base image
 *           being picked up during a compile.
 *
 *           If the image is not found in the application image's directory, the
 *           regular executable search, which searches the path, is performed.
 */
void SystemInterpreter::loadImage(char *&imageBuffer, size_t &imageSize )
{
    char fullname[MAX_PATH + 1];    // finally resolved name

    // if we can't find the image on the search path, this is a logic error
    if (!SysFileSystem::primitiveSearchName(BASEIMAGE, NULL, NULL, fullname))
    {
        Interpreter::logicError("no startup image");
    }

    // try to open the file
    HANDLE fileHandle = CreateFile(fullname, GENERIC_READ, FILE_SHARE_READ,
                            NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);

    // we could resolve the file name, but can't open the file for some reason
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        Interpreter::logicError("no startup image");
    }

    DWORD     bytesRead;
    // the image is written out with a size before the buffer
    ReadFile(fileHandle, &imageSize, sizeof(size_t), &bytesRead, NULL);

    // now allocate the image buffer and read the entire file into the buffer
    imageBuffer = memoryObject.allocateImageBuffer(imageSize);
    /* read in the image                 */
    ReadFile(fileHandle, imageBuffer, (DWORD)imageSize, &bytesRead, NULL);
    // set this to the actual size read.
    imageSize = bytesRead;
    CloseHandle(fileHandle);
}


/**
 * Read in a program and return it in a buffer object.
 *
 * @param file_name the target file name.
 *
 * @return A buffer containing the program.
 */
BufferClass *SystemInterpreter::readProgram(const char *file_name)
{

    HANDLE        fileHandle;
    BY_HANDLE_FILE_INFORMATION   status;

    // NOTE: We read this in without holding the kernel locak
    {
        UnsafeBlock releaser;
        fileHandle = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ,
                                NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            return OREF_NULL;
        }
        // get the file size    retrieve the file size            */
        GetFileInformationByHandle(fileHandle, &status);
    }

    // back to locked status
    // get the file size and allocate a new buffer of the program size
    size_t buffersize = status.nFileSizeLow;
    BufferClass *buffer = new_buffer(buffersize);
    ProtectedObject p(buffer);
    {
        UnsafeBlock releaser;

        DWORD bytesRead;
        // read in the data, but return nothing if there is an error
        if (ReadFile(fileHandle, buffer->getData(), (DWORD)buffersize, &bytesRead, NULL) == 0)
        {
            return OREF_NULL;
        }
        CloseHandle(fileHandle);
        return buffer;
    }
}


/**
 * Fully qualify a program file name.
 *
 * @param name   The starting file name.
 *
 * @return A fully qualified string file name.
 */
RexxString *SystemInterpreter::qualifyFileSystemName(RexxString *name)
{
    char nameBuffer[SysFileSystem::MaximumFileNameBuffer];

    // expand the file name.
    memset(nameBuffer, 0, sizeof(nameBuffer));
    SysFileSystem::qualifyStreamName((char *)name->getStringData(), nameBuffer, sizeof(nameBuffer));
    // get the qualified file name.
    return new_string(nameBuffer);
}





