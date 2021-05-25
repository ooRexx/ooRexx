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
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "InterpreterInstance.hpp"
#include "SystemInterpreter.hpp"


/**
 * Initialize the interpreter instance.
 *
 * @param i       Our interpreter instance container.
 * @param options The options used to initialize us.  We can add additional
 *                platform-specific options if we wish.
 */
void SysInterpreterInstance::initialize(InterpreterInstance *i, RexxOption *options)
{
    instance = i;

    externalTraceEnabled = false;    // off by default
    /* scan current environment,         */
    const char *rxTraceBuf = getenv("RXTRACE");
    if (rxTraceBuf != NULL)
    {
        if (!Utilities::strCaselessCompare(rxTraceBuf, "ON"))    /* request to turn on?               */
        {
            externalTraceEnabled = true;   // turn on tracing of top-level activations for this instance
        }
    }

    // add our default search extension as both upper and lower case
    addSearchExtension(".REX");
    addSearchExtension(".rex");
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
 * @param activation The activation for the top-level program.
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
 * Build a search path used for this resolution step.
 *
 * @param parentDir The location of the program calling us (can be null).
 * @param extensionPath
 *                  The system extension path (can be null).
 */
SysSearchPath::SysSearchPath(const char *parentDir, const char *extensionPath)
{
    const char *sysPath = getenv("PATH");
    const char *rexxPath = getenv("REXX_PATH");

#ifdef ORX_REXXPATH
    if (rexxPath == NULL)
    {
       rexxPath = ORX_REXXPATH;
    }
#endif

    addPath(parentDir);
    // add on the current directory after the parent dir
    addPath(".");
    // next comes the extension path defined on the instance
    addPath(extensionPath);
    // followed by the REXX_PATH
    addPath(rexxPath);
    // and finally the PATH
    addPath(sysPath);
}
