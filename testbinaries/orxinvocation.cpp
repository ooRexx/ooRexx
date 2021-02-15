/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008 Rexx Language Association. All rights reserved.         */
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

#include <string.h>
#include "oorexxapi.h"
#include "orxexits.hpp"

RexxMethod0(int, init)
{
    RexxBufferObject buffer = context->NewBuffer(sizeof(InstanceInfo));
    InstanceInfo *instanceInfo = new (context->BufferData(buffer)) InstanceInfo();
    instanceInfo->exitStyle = InstanceInfo::NONE;
    instanceInfo->programName = "";
    instanceInfo->initialAddress = NULL;
    context->SetObjectVariable("CSELF", buffer);
    return 0;
}


RexxMethod2(int, setExitType,
            CSELF, self,
            CSTRING, type)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;

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
    return 0;
}


RexxMethod2(int, setProgramName,
            CSELF, self,
            CSTRING, name)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->programName = name;
    return 0;
}


RexxMethod2(int, setInitialAddress,
            CSELF, self,
            CSTRING, name)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->initialAddress = name;
    return 0;
}


RexxMethod2(int, setSearchPath,
            CSELF, self,
            CSTRING, path)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->extensionPath = path;
    return 0;
}


RexxMethod2(int, setExtensions,
            CSELF, self,
            CSTRING, extensions)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->extensions = extensions;
    return 0;
}


RexxMethod2(int, setLoadLibrary,
            CSELF, self,
            CSTRING, lib)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->loadLibrary = lib;
    return 0;
}



RexxMethod1(CSTRING, getExitType,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    switch (instanceInfo->exitStyle)
    {
        case InstanceInfo::NONE:
            return "NONE";
        case InstanceInfo::CONTEXT:
            return "CONTEXT";
        case InstanceInfo::REGISTERED_DLL:
            return "DLL";
        case InstanceInfo::REGISTERED_EXE:
            return "EXE";

    }
    return "NONE";
}


RexxMethod2(int, setFunctionExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->fnc = setting;
    return 0;
}

RexxMethod2(int, setCommandExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->cmd = setting;
    return 0;
}

RexxMethod2(int, setQueueExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->msq = setting;
    return 0;
}

RexxMethod2(int, setIOExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->sio = setting;
    return 0;
}

RexxMethod2(int, setTraceExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->trc = setting;
    return 0;
}

RexxMethod2(int, setHaltExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->hlt = setting;
    return 0;
}

RexxMethod2(int, setInitExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->ini = setting;
    return 0;
}

RexxMethod2(int, setTermExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->ter = setting;
    return 0;
}

RexxMethod2(int, setScriptingExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->exf = setting;
    return 0;
}

RexxMethod2(int, setObjectFunctionExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->ofnc = setting;
    return 0;
}

RexxMethod2(int, setNovalueExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->var = setting;
    return 0;
}

RexxMethod2(int, setValueExit,
            CSELF, self,
            CSTRING, setting)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->val = setting;
    return 0;
}

RexxMethod1(CSTRING, getFunctionExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->fnc;
}

RexxMethod1(CSTRING, getCommandExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->cmd;
}

RexxMethod1(CSTRING, getQueueExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->msq;
}

RexxMethod1(CSTRING, getIOExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->sio;
}

RexxMethod1(CSTRING, getTraceExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->trc;
}

RexxMethod1(CSTRING, getHaltExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->hlt;
}

RexxMethod1(CSTRING, getInitExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->ini;
}

RexxMethod1(CSTRING, getTermExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->ter;
}

RexxMethod1(CSTRING, getScriptingExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->exf;
}

RexxMethod1(CSTRING, getObjectFunctionExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->ofnc;
}

RexxMethod1(CSTRING, getNovalueExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->var;
}

RexxMethod1(CSTRING, getValueExit,
            CSELF, self)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    return (const char *)instanceInfo->val;
}

RexxMethod1(wholenumber_t, getRC,
            CSELF, self)
{
     InstanceInfo *instanceInfo = (InstanceInfo *)self;
     return instanceInfo->rc;
}


RexxMethod1(wholenumber_t, getCode,
            CSELF, self)
{
     InstanceInfo *instanceInfo = (InstanceInfo *)self;
     return instanceInfo->code;
}

RexxMethod2(wholenumber_t, setRC,
            CSELF, self,
            int, rc)
{
     InstanceInfo *instanceInfo = (InstanceInfo *)self;
     instanceInfo->rc = rc;
     return instanceInfo->rc;
}


RexxMethod2(wholenumber_t, setCode,
            CSELF, self,
            int, code)
{
     InstanceInfo *instanceInfo = (InstanceInfo *)self;
     instanceInfo->code = code;
     return instanceInfo->code;
}


RexxMethod3(RexxObjectPtr, callInstanceProgram,
            CSELF, self,
            CSTRING, program,
            OPTIONAL_RexxArrayObject, args)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->programName = program;

    if (args == NULLOBJECT)
    {
        instanceInfo->argCount = 0;
    }
    else
    {
        instanceInfo->argCount = context->ArraySize(args);
        for (size_t i = 0; i < instanceInfo->argCount; i++)
        {
            RexxObjectPtr arg = context->ArrayAt(args, i + 1);
            if (arg != NULLOBJECT)
            {
                instanceInfo->arguments[i] = context->CString(arg);
            }
            else
            {
                instanceInfo->arguments[i] = NULL;
            }
        }
    }

    invokeProgram(instanceInfo);
    if (instanceInfo->rc != 0)
    {
        return context->Nil();
    }
    else
    {
        return context->String(instanceInfo->returnResult);
    }
}


RexxMethod3(RexxObjectPtr, callRexxStart,
            CSELF, self,
            CSTRING, program,
            RexxArrayObject, args)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)self;
    instanceInfo->programName = program;

    if (args == NULLOBJECT)
    {
        instanceInfo->argCount = 0;
    }
    else
    {
        instanceInfo->argCount = context->ArraySize(args);
        for (size_t i = 0; i < instanceInfo->argCount; i++)
        {
            RexxObjectPtr arg = context->ArrayAt(args, i + 1);
            if (arg != NULLOBJECT)
            {
                instanceInfo->arguments[i] = context->CString(arg);
            }
            else
            {
                instanceInfo->arguments[i] = NULL;
            }
        }
    }

    invokeRexxStart(instanceInfo);
    if (instanceInfo->rc != 0)
    {
        return context->Nil();
    }
    else
    {
        return context->String(instanceInfo->returnResult);
    }
}

/**
 * The package loader and unloader don't really have anything
 * sensible that they can test at this point, but having these
 * defined is useful for the purposes of being able to
 * verify that they are called.
 *
 * @param context The ThreadContext that allows us to do "stuff".
 *
 * @return void
 */
void RexxEntry packageLoader(RexxThreadContext *context)
{
    return;
}

void RexxEntry packageUnloader(RexxThreadContext *context)
{
    return;
}


RexxMethodEntry orxtest_methods[] = {
    REXX_METHOD(init,                    init),
    REXX_METHOD(setExitType,             setExitType),
    REXX_METHOD(getExitType,             getExitType),
    REXX_METHOD(setProgramName,          setProgramName),
    REXX_METHOD(setInitialAddress,       setInitialAddress),
    REXX_METHOD(setSearchPath,           setSearchPath),
    REXX_METHOD(setExtensions,           setExtensions),
    REXX_METHOD(setLoadLibrary,          setLoadLibrary),
    REXX_METHOD(setFunctionExit,         setFunctionExit),
    REXX_METHOD(setCommandExit,          setCommandExit),
    REXX_METHOD(setQueueExit,            setQueueExit),
    REXX_METHOD(setIOExit,               setIOExit),
    REXX_METHOD(setTraceExit,            setTraceExit),
    REXX_METHOD(setHaltExit,             setHaltExit),
    REXX_METHOD(setInitExit,             setInitExit),
    REXX_METHOD(setTermExit,             setTermExit),
    REXX_METHOD(setScriptingExit,        setScriptingExit),
    REXX_METHOD(setObjectFunctionExit,   setObjectFunctionExit),
    REXX_METHOD(setNovalueExit,          setNovalueExit),
    REXX_METHOD(setValueExit,            setValueExit),
    REXX_METHOD(getFunctionExit,         getFunctionExit),
    REXX_METHOD(getCommandExit,          getCommandExit),
    REXX_METHOD(getQueueExit,            getQueueExit),
    REXX_METHOD(getIOExit,               getIOExit),
    REXX_METHOD(getTraceExit,            getTraceExit),
    REXX_METHOD(getHaltExit,             getHaltExit),
    REXX_METHOD(getInitExit,             getInitExit),
    REXX_METHOD(getTermExit,             getTermExit),
    REXX_METHOD(getScriptingExit,        getScriptingExit),
    REXX_METHOD(getObjectFunctionExit,   getObjectFunctionExit),
    REXX_METHOD(getNovalueExit,          getNovalueExit),
    REXX_METHOD(getValueExit,            getValueExit),
    REXX_METHOD(getRC,                   getRC),
    REXX_METHOD(getCode,                 getCode),
    REXX_METHOD(setRC,                   setRC),
    REXX_METHOD(setCode,                 setCode),
    REXX_METHOD(callInstanceProgram,     callInstanceProgram),
    REXX_METHOD(callRexxStart,           callRexxStart),
    REXX_LAST_METHOD()
};


RexxPackageEntry UnitTest_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "InvocationTest",                    // name of the package
    "1.0.0",                             // package information
    packageLoader,                       // no load/unload functions
    packageUnloader,
    NULL,                                // the exported routines
    orxtest_methods                      // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(UnitTest);
