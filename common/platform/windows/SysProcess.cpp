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
/*****************************************************************************/
/* REXX Windows Support                                                      */
/*                                                                           */
/* Process support for Windows                                               */
/*                                                                           */
/*****************************************************************************/

#include "windows.h"
#include "SysProcess.hpp"


// full path of the currently running executable
const char *SysProcess::executableFullPath = NULL;
// directory of our Rexx shared libraries
const char *SysProcess::libraryLocation = NULL;

/**
 * Get the current user name information.
 *
 * @param buffer The buffer (of at least MAX_USERID_LENGTH characters) into which the userid is copied.
 */
void SysProcess::getUserID(char *buffer)
{
    DWORD account_size = MAX_USERID_LENGTH;
    GetUserName(buffer, &account_size);
}


/**
 * Determine the location of the running program. This returns the full path
 * of the currently running executable.
 *
 * @return A character string of the path (does not need to be freed by the caller)
 */
const char *SysProcess::getExecutableFullPath()
{
    if (executableFullPath != NULL)
    {
        return executableFullPath;
    }

    char modulePath[MAX_PATH];

    // NULL means the current executable
    GetModuleFileName(NULL, modulePath, sizeof(modulePath));

    // save a copy of this
    executableFullPath = strdup(modulePath);
    return executableFullPath;
}


/**
 * Determine the location of the rexxapi.dll library. This returns the
 * directory portion of the library's path with a trailing backslash.
 *
 * @return A character string of the location (does not need to be freed by the caller)
 */
const char *SysProcess::getLibraryLocation()
{
    if (libraryLocation != NULL)
    {
        return libraryLocation;
    }

     // look up rexxapi.dll
    HMODULE module = GetModuleHandle("rexxapi");

    char modulePath[MAX_PATH];
    GetModuleFileName(module, modulePath, sizeof(modulePath));

    size_t pathLength = strlen(modulePath);

    // scan backwards to find the last directory delimiter
    for (; pathLength > 0; pathLength--)
    {
        // is this the directory delimiter?
        if (modulePath[pathLength - 1] == '\\')
        {
            // terminate the string after the first encountered backslash and quit
            modulePath[pathLength] = '\0';
            break;
        }
    }

    // belt-and-braces, make sure we found a directory
    if (pathLength == 0)
    {
        return NULL;
    }

    // save a copy of this
    libraryLocation = strdup(modulePath);
    return libraryLocation;
}


/**
 * do a beep tone
 *
 * @param frequency The frequency to beep at
 * @param duration  The duration to beep (in milliseconds)
 */
void SysProcess::beep(int frequency, int duration)
{
    Beep((DWORD)frequency, (DWORD)duration);
}
