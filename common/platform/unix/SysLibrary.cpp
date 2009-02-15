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
/* REXX Shared runtime                                                        */
/*                                                                            */
/* Unix implementation of the SysLibrary                                      */
/*                                                                            */
/******************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "SysLibrary.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>


#define MAX_LIBRARY_NAME_LENGTH 250
//TODO:  Add conditional compilation for Apple OSX
#define SYSTEM_LIBRARY_EXTENSION "so"
#define LIBARY_NAME_BUFFER_LENGTH (MAX_LIBRARY_NAME_LENGTH + sizeof("/usr/lib/lib.") + sizeof(SYSTEM_LIBRARY_EXTENSION))

SysLibrary::SysLibrary()
{
    libraryHandle = NULL;
}


void * SysLibrary::getProcedure(
  const char *name)                      /* required procedure name           */
/******************************************************************************/
/* Function:  Resolve a named procedure in a library                          */
/******************************************************************************/
{
    return dlsym(libraryHandle, name);
}


bool SysLibrary::load(
    const char *name)                    /* required library name             */
/******************************************************************************/
/* Function:  Load a named library, returning success/failure flag            */
/******************************************************************************/
{
    char nameBuffer[LIBARY_NAME_BUFFER_LENGTH];

    if (strlen(name) > MAX_LIBRARY_NAME_LENGTH)
    {
        return false;
    }

    sprintf(nameBuffer, "lib%s.%s", name, SYSTEM_LIBRARY_EXTENSION);
    // try loading directly
    libraryHandle = dlopen(nameBuffer, RTLD_LAZY);
    // if not found, then try from /usr/lib
    if (libraryHandle == NULL)
    {
        sprintf(nameBuffer, "/usr/lib/lib%s.%s", name, SYSTEM_LIBRARY_EXTENSION);
        libraryHandle = dlopen(nameBuffer, RTLD_LAZY);
        // still can't find it?
        if (libraryHandle == NULL)
        {
            return false;
        }
    }
    return true;     // loaded successfully
}


/**
 * Free a loaded library if the library is still loaded.
 *
 * @return True if we unloaded ok, false otherwise.
 */
bool SysLibrary::unload()
{
    if (libraryHandle != NULL)
    {
        dlclose(libraryHandle);
        return true;
    }
    return false;
}
