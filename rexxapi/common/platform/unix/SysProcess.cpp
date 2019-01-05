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
/*                                                                           */
/* Process support for Unix based systems.                                   */
/*                                                                           */
/*****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "SysProcess.hpp"
#include "rexx.h"
#include <stdio.h>

// location of our executables
const char *SysProcess::executableLocation = NULL;

/**
 * Get the current user name information.
 *
 * @param buffer The buffer (of at least MAX_USERID_LENGTH characters) into which the userid is copied.
 */
void SysProcess::getUserID(char *buffer)
{
#if defined( HAVE_GETPWUID )
    struct passwd * pstUsrDat;
#endif

#if defined( HAVE_GETPWUID )
    pstUsrDat = getpwuid(geteuid());
    strncpy( buffer,  pstUsrDat->pw_name, MAX_USERID_LENGTH-1);
#elif defined( HAVE_IDTOUSER )
    strncpy( buffer, IDtouser(geteuid()), MAX_USERID_LENGTH-1);
#else
    strcpy( buffer, "unknown" );
#endif
}


/**
 * Determine the location of the running program. This returns the path
 * of the current executable.
 *
 * @return A character string of the location (does not need to be freed by the caller)
 */
const char* SysProcess::getExecutableLocation()
{
    if (executableLocation != NULL)
    {
        return executableLocation;
    }

#ifdef HAVE_DLADDR
    Dl_info dlInfo;
    if (dladdr((void *)RexxCreateQueue, &dlInfo) == 0)
    {
        // a zero return means this could not be resolved. Should
        // not be possible, but we'll just return NULL.
        return NULL;
    }

    // this is the file location
    char *moduleName = strdup(dlInfo.dli_fname);

    size_t nameLength = strlen(moduleName);
    // scan backwards to find the last directory delimiter

    for (; nameLength > 0; nameLength--)
    {
        // is this the directory delimiter?
        if (moduleName[nameLength - 1] == '/')
        {
            // terminate the string after the first encountered backslash and quit
            moduleName[nameLength] = '\0';
            break;
        }
    }

    // belt-and-braces, make sure we found a directory
    if (nameLength == 0)
    {
        free(moduleName);
        return NULL;
    }

    // save this for future use
    executableLocation = moduleName;
    return executableLocation;
#else
    // no means to determine this, so we always return NULL
    return NULL;
#endif
}


