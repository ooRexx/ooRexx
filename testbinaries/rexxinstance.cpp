/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2024 Rexx Language Association. All rights reserved.    */
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
/* THIS SOFTWARE IS PROVIDED BY THE COPYright HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYright   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include "oorexxapi.h"
#include "orxexits.hpp"
#include <stdio.h>

void REXXENTRY buildInfo(InstanceInfo *instanceInfo, int argc, char **argv)
{
    int i = 1;

    // the type of call is the first character
    const char *callType = argv[i++];
    if (strcmp(callType, "INSTANCE") == 0)
    {
        instanceInfo->callType = InstanceInfo::INSTANCE;
    }
    else
    {
        instanceInfo->callType = InstanceInfo::REXXSTART;
    }

    // fill in the
    while (i < argc)
    {
        const char *arg = argv[i++];
        // process each of the options...once we find a non-option string,
        // we go into the next phase
        if (*arg != '-')
        {
            // this string is the program name.  Anything else beyond this
            // point are arguments to the target program.
            instanceInfo->programName = arg;
            break;
        }
        else if (strcmp(arg, "-FNC") == 0)
        {
            instanceInfo->fnc = argv[i++];
        }
        else if (strcmp(arg, "-CMD") == 0)
        {
            instanceInfo->cmd = argv[i++];
        }
        else if (strcmp(arg, "-MSQ") == 0)
        {
            instanceInfo->msq = argv[i++];
        }
        else if (strcmp(arg, "-SIO") == 0)
        {
            instanceInfo->sio = argv[i++];
        }
        else if (strcmp(arg, "-HLT") == 0)
        {
            instanceInfo->hlt = argv[i++];
        }
        else if (strcmp(arg, "-TRC") == 0)
        {
            instanceInfo->trc = argv[i++];
        }
        else if (strcmp(arg, "-INI") == 0)
        {
            instanceInfo->ini = argv[i++];
        }
        else if (strcmp(arg, "-TER") == 0)
        {
            instanceInfo->ter = argv[i++];
        }
        else if (strcmp(arg, "-EXF") == 0)
        {
            instanceInfo->exf = argv[i++];
        }
        else if (strcmp(arg, "-OFNC") == 0)
        {
            instanceInfo->ofnc = argv[i++];
        }
        else if (strcmp(arg, "-VAR") == 0)
        {
            instanceInfo->var = argv[i++];
        }
        else if (strcmp(arg, "-VAL") == 0)
        {
            instanceInfo->val = argv[i++];
        }
        else if (strcmp(arg, "-PATH") == 0)
        {
            instanceInfo->extensionPath = argv[i++];
        }
        else if (strcmp(arg, "-EXT") == 0)
        {
            instanceInfo->extensions = argv[i++];
        }
        else if (strcmp(arg, "-LIB") == 0)
        {
            instanceInfo->loadLibrary = argv[i++];
        }
        else if (strcmp(arg, "-ADDRESS") == 0)
        {
            instanceInfo->initialAddress = argv[i++];
        }
        else if (strcmp(arg, "-EXIT") == 0)
        {
            const char *type = argv[i++];
            if (strcmp(type, "NONE") == 0)
            {
                instanceInfo->exitStyle = InstanceInfo::NONE;
            }
            else if (strcmp(type, "CONTEXT") == 0)
            {
                instanceInfo->exitStyle = InstanceInfo::CONTEXT;
            }
            else if (strcmp(type, "DLL") == 0)
            {
                instanceInfo->exitStyle = InstanceInfo::REGISTERED_DLL;
            }
            else if (strcmp(type, "EXE") == 0)
            {
                instanceInfo->exitStyle = InstanceInfo::REGISTERED_EXE;
            }
            else
            {
                instanceInfo->exitStyle = InstanceInfo::NONE;
            }
        }
    }

    // fill in the argument count and the argument pointers
    instanceInfo->argCount = argc - i;
    int arg = 0;
    // fill in the
    while (i < argc)
    {
        instanceInfo->arguments[arg++] = argv[i++];
    }
}

int main (int argc, char **argv)
{
    // our invocation information
    InstanceInfo instanceInfo;

    // fill in the structure for the invocation
    buildInfo(&instanceInfo, argc, argv);
    // this can be done using either RexxCreateInterpreter or RexxStart
    if (instanceInfo.callType == InstanceInfo::INSTANCE)
    {
        // go invoke this
        invokeProgram(&instanceInfo);
    }
    else
    {
        invokeRexxStart(&instanceInfo);
    }
    // if no error, then push the return result on to the queue
    if (instanceInfo.rc == 0)
    {
        CONSTRXSTRING result;
        MAKERXSTRING(result, instanceInfo.returnResult, strlen(instanceInfo.returnResult));
        RexxAddQueue("TESTQUEUE", &result, RXQUEUE_LIFO);
    }
    else
    {
        CONSTRXSTRING result;
        char errorResult[50];

        snprintf(errorResult, sizeof(errorResult), "%zd %zd", instanceInfo.rc, instanceInfo.code);
        MAKERXSTRING(result, errorResult, strlen(errorResult));
        RexxAddQueue("TESTQUEUE", &result, RXQUEUE_LIFO);

    }
    return (int)instanceInfo.rc;   // return the error indicator
}
