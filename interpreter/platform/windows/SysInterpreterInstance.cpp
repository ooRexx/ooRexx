/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
#include "RexxActivation.hpp"


/**
 * Initialize the interpreter instance.
 *
 * @param i       Our interpreter instance container.
 * @param options The options used to initialize us.  We can add additional
 *                platform-specific options if we wish.
 */
void SysInterpreterInstance::initialize(InterpreterInstance *i, RexxOption *options)
{
    externalTraceEnabled = false;    // off by default
    TCHAR rxTraceBuf[8];

    // check the current environment for RXTRACE
    if (GetEnvironmentVariable("RXTRACE", rxTraceBuf, 8))
    {
        // if this is set to on, start with tracing enabled
        if (!Utilities::strCaselessCompare(rxTraceBuf, "ON"))
        {
            externalTraceEnabled = true;   // turn on tracing of top-level activations for this instance
        }
    }

    instance = i;

    // add our default search extension
    addSearchExtension(".REX");
}


/**
 * Terminate the interpreter instance.
 */
void SysInterpreterInstance::terminate()
{
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
    if (!instance->searchExtensions->hasItem(ext))
    {
        instance->searchExtensions->append(ext);
    }
}


/**
 * Do system specific program setup
 *
 * @param activation the activation starting up.
 */
void SysInterpreterInstance::setupProgram(RexxActivation *activation)
{
    // trace this activation if turned on externally when the instance was started
    if (externalTraceEnabled)
    {
        activation->enableExternalTrace();
    }
}


/**
 * Initialize a path searcher.
 *
 * @param parentDir The parent dirctory for the search.
 * @param extensionPath
 *                  The system extension path.
 */
SysSearchPath::SysSearchPath(const char *parentDir, const char *extensionPath)
{
    char temp[4];             // this is just a temp buffer to check component sizes

    // this is a little bit of a pain because this gets returned in a buffer. We need to
    // find the size so we can ensure our buffer is large enough before we start to
    // assemble it
    size_t pathSize = GetEnvironmentVariable("PATH", temp, sizeof(temp));
    size_t rexxPathSize = GetEnvironmentVariable("REXX_PATH", temp, sizeof(temp));
    size_t parentSize = parentDir == NULL ? 0 : strlen(parentDir);
    size_t extensionSize = extensionPath == NULL ? 0 : strlen(extensionPath);

    // enough room for separators and a terminating null
    path.ensureCapacity(pathSize + rexxPathSize + parentSize + extensionSize + 16);

    // parent directory
    addPath(parentDir);
    // add on the current directory
    addPath(".");
    // next comes the extension path defined on the instance
    addPath(extensionPath);

    // followed by the REXX_PATH
    if (!path.endsWith(';'))
    {
        path += ";";
    }
    GetEnvironmentVariable("REXX_PATH", (char *)path + path.length(), (DWORD)rexxPathSize + 1);

    // and finally the PATH
    if (!path.endsWith(';'))
    {
        path += ";";
    }
    GetEnvironmentVariable("PATH", (char *)path + path.length(), (DWORD)pathSize + 1);
}
