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
/* Implementation of the SysInterpreterInstance class                         */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "InterpreterInstance.hpp"
#include "ListClass.hpp"
#include "SystemInterpreter.hpp"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>


BOOL __stdcall WinConsoleCtrlHandler(DWORD dwCtrlType)
/******************************************************************************/
/* Arguments:  Report record, registration record, context record,            */
/*             dispatcher context                                             */
/*                                                                            */
/* DESCRIPTION : For Control Break conditions issue a halt to activation      */
/*               Control-C or control-Break is pressed.                       */
/*                                                                            */
/*  Returned:  Action code                                                    */
/******************************************************************************/
{
    // check to condition for all threads of this process */

    if ((dwCtrlType == CTRL_CLOSE_EVENT) || (dwCtrlType == CTRL_SHUTDOWN_EVENT))
    {
        return false;  /* send to system */
    }

    /* if RXCTRLBREAK=NO then ignore SIGBREAK exception */
    if (dwCtrlType == CTRL_BREAK_EVENT || dwCtrlType == CTRL_LOGOFF_EVENT)
    {
        char envp[65];
        if (GetEnvironmentVariable("RXCTRLBREAK", envp, sizeof(envp)) > 0 && strcmp("NO",envp) == 0)
        {
            return true;    /* ignore signal */
        }
    }

    if (dwCtrlType == CTRL_LOGOFF_EVENT)
    {
        return false;    /* send to system */
    }

    // we need to do something about this one, let the system interpreter handle
    return SystemInterpreter::processSignal(dwCtrlType);
}

/**
 * Initialize the interpreter instance.
 *
 * @param i       Our interpreter instance container.
 * @param options The options used to initialize us.  We can add additional
 *                platform-specific options if we wish.
 */
void SysInterpreterInstance::initialize(InterpreterInstance *i, RexxOption *options)
{
    /* Because of using the stand-alone runtime library or when using different compilers,
       the std-streams of the calling program and the REXX.DLL might be located at different
       addresses and therefore _file might be -1. If so, std-streams are reassigned to the
       file standard handles returned by the system */
    if ((stdin->_file == -1) && (GetFileType(GetStdHandle(STD_INPUT_HANDLE)) != FILE_TYPE_UNKNOWN))
    {
        *stdin = *_fdopen(_open_osfhandle((intptr_t)GetStdHandle(STD_INPUT_HANDLE),_O_RDONLY), "r");
    }
    if ((stdout->_file == -1) && (GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) != FILE_TYPE_UNKNOWN))
    {
        *stdout = *_fdopen(_open_osfhandle((intptr_t)GetStdHandle(STD_OUTPUT_HANDLE),_O_APPEND), "a");
    }
    if ((stderr->_file == -1) && (GetFileType(GetStdHandle(STD_ERROR_HANDLE)) != FILE_TYPE_UNKNOWN))
    {
        *stderr = *_fdopen(_open_osfhandle((intptr_t)GetStdHandle(STD_ERROR_HANDLE),_O_APPEND), "a");
    }
    // enable trapping for CTRL_C exceptions
    SetConsoleCtrlHandler(&WinConsoleCtrlHandler, true);
    instance = i;

    // add our default search extension
    addSearchExtension(".REX");
}


/**
 * Terminate the interpreter instance.
 */
void SysInterpreterInstance::terminate()
{
    SetConsoleCtrlHandler(&WinConsoleCtrlHandler, false);
}


/**
 * Append a system default extension to the extension search order.
 *
 * @param name   The name to add.
 */
void SysInterpreterInstance::addSearchExtension(const char *name)
{
    // if the extension is not already in the extension list, add it
    RexxString *ext = new_string(name);
    if (instance->searchExtensions->hasItem(ext) == TheFalseObject)
    {
        instance->searchExtensions->append(ext);
    }
}


SysSearchPath::SysSearchPath(const char *parentDir, const char *extensionPath)
{
    char temp[4];             // this is just a temp buffer to check component sizes

    size_t pathSize = GetEnvironmentVariable("PATH", temp, sizeof(temp));
    size_t rexxPathSize = GetEnvironmentVariable("REXX_PATH", temp, sizeof(temp));
    size_t parentSize = parentDir == NULL ? 0 : strlen(parentDir);
    size_t extensionSize = extensionPath == NULL ? 0 : strlen(extensionPath);


    // enough room for separators and a terminating null
    path = (char *)SystemInterpreter::allocateResultMemory(pathSize + rexxPathSize + parentSize + extensionSize + 16);
    *path = '\0';     // add a null character so strcat can work
    if (parentDir != NULL)
    {
        strcpy(path, parentDir);
        strcat(path, ";");
    }

    // add on the current directory
    strcat(path, ".;");

    if (extensionPath != NULL)
    {
        strcat(path, extensionPath);
        if (path[strlen(path) - 1] != ';')
        {
            strcat(path, ";");
        }
    }

    // add on the Rexx path, then the normal path
    GetEnvironmentVariable("REXX_PATH", path + strlen(path), (DWORD)pathSize + 1);
    if (path[strlen(path) - 1] != ';')
    {
        strcat(path, ";");
    }

    GetEnvironmentVariable("PATH", path + strlen(path), (DWORD)pathSize + 1);
    if (path[strlen(path) - 1] != ';')
    {
        strcat(path, ";");
    }
}


/**
 * Deconstructor for releasing storage used by the constructed path.
 */
SysSearchPath::~SysSearchPath()
{
    SystemInterpreter::releaseResultMemory((void *)path);
}
