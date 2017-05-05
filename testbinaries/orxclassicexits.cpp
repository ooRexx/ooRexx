/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2017 Rexx Language Association. All rights reserved.    */
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

#include "rexx.h"
#include "orxexits.hpp"
#include <string.h>

InstanceInfo *getApplicationData()
{
    void *userData[2];
    unsigned short flag;

    RexxQueryExit("TestFunctionExit", NULL, &flag, (char *)userData);
    return (InstanceInfo *)userData[1];
}


void setContextVariable(const char *name, const char *value)
{
    SHVBLOCK shvb;

    memset(&shvb, 0, sizeof(shvb));
    shvb.shvnext = NULL;
    shvb.shvname.strptr = name;
    shvb.shvname.strlength = strlen(name);
    shvb.shvvalue.strptr = const_cast<char *>(value);
    shvb.shvvalue.strlength = strlen(value);
    shvb.shvnamelen = shvb.shvname.strlength;
    shvb.shvvaluelen = shvb.shvvalue.strlength;
    shvb.shvcode = RXSHV_SET;
    shvb.shvret = 0;
    RexxVariablePool(&shvb);
}


void getContextVariable(const char *name, RXSTRING *value)
{
    SHVBLOCK shvb;

    memset(&shvb, 0, sizeof(shvb));
    shvb.shvnext = NULL;
    shvb.shvname.strptr = name;
    shvb.shvname.strlength = strlen(name);
    shvb.shvvalue.strptr = value->strptr;
    shvb.shvvalue.strlength = value->strlength;
    shvb.shvnamelen = shvb.shvname.strlength;
    shvb.shvvaluelen = shvb.shvvalue.strlength;
    shvb.shvcode = RXSHV_FETCH;
    shvb.shvret = 0;
    RexxVariablePool(&shvb);
}


void dropContextVariable(const char *name)
{
    SHVBLOCK shvb;

    memset(&shvb, 0, sizeof(shvb));
    shvb.shvnext = NULL;
    shvb.shvname.strptr = name;
    shvb.shvname.strlength = strlen(name);
    shvb.shvnamelen = shvb.shvname.strlength;
    shvb.shvcode = RXSHV_DROPV;
    shvb.shvret = 0;
    RexxVariablePool(&shvb);
}

extern "C" {

int RexxEntry TestFunctionExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->fnc.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        default: break; // avoid warning: enumeration values not handled in switch
    }

    RXFNCCAL_PARM *parms = (RXFNCCAL_PARM *)exitInfo;
    const char *functionName = parms->rxfnc_name;
    if (strcmp(functionName, "TESTSUBCALL") == 0)
    {
        // return the status of the flag
        if (parms->rxfnc_flags.rxffsub)
        {
            strcpy(parms->rxfnc_retc.strptr, "SUBROUTINE");
        }
        else
        {
            strcpy(parms->rxfnc_retc.strptr, "FUNCTION");
        }
        parms->rxfnc_retc.strlength = strlen(parms->rxfnc_retc.strptr);
        return RXEXIT_HANDLED;
    }
    else if (strcmp(functionName, "TESTERROR") == 0)
    {
        // this should raise an error
        parms->rxfnc_flags.rxfferr = 1;
        return RXEXIT_HANDLED;
    }
    else if (strcmp(functionName, "TESTNOTOUND") == 0)
    {
        // this should raise an error
        parms->rxfnc_flags.rxffnfnd = 1;
        return RXEXIT_HANDLED;
    }
    else if (strcmp(functionName, "TESTGETCONTEXTVARIABLE") == 0)
    {
        getContextVariable(parms->rxfnc_argv[0].strptr, &parms->rxfnc_retc);
        return RXEXIT_HANDLED;
    }
    else if (strcmp(functionName, "TESTSETCONTEXTVARIABLE") == 0)
    {
        setContextVariable(parms->rxfnc_argv[0].strptr, parms->rxfnc_argv[1].strptr);
        parms->rxfnc_retc.strlength = 0;
        return RXEXIT_HANDLED;
    }
    else if (strcmp(functionName, "TESTDROPCONTEXTVARIABLE") == 0)
    {
        dropContextVariable(parms->rxfnc_argv[0].strptr);
        return RXEXIT_HANDLED;
    }
    else
    {
        // pass on this
        return RXEXIT_NOT_HANDLED;
    }
}

int RexxEntry TestCommandExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->cmd.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        default: break; // avoid warning: enumeration values not handled in switch
    }

    RXCMDHST_PARM *parms = (RXCMDHST_PARM *)exitInfo;
    // handle commands here...we need to process both the address environment and the commands
    if (strcmp(parms->rxcmd_address, "FOOBAR") == 0)
    {
        strcpy(parms->rxcmd_retc.strptr, "-1");
        parms->rxcmd_retc.strlength = 2;
        return RXEXIT_HANDLED;
    }

    // ok, a good address...now do the different commands
    if (strcmp(parms->rxcmd_command.strptr, "GOOD") == 0)
    {
        strcpy(parms->rxcmd_retc.strptr, "0");
        parms->rxcmd_retc.strlength = 1;
        return RXEXIT_HANDLED;
    }
    else if (strcmp(parms->rxcmd_command.strptr, "ERROR") == 0)
    {
        strcpy(parms->rxcmd_retc.strptr, "1");
        parms->rxcmd_retc.strlength = 1;
        parms->rxcmd_flags.rxfcerr = 1;
        return RXEXIT_HANDLED;
    }
    // ok, a good address...now do the different commands
    else if (strcmp(parms->rxcmd_command.strptr, "TRACEON") == 0)
    {
        instanceInfo->trc = InstanceInfo::TRACEON;
        strcpy(parms->rxcmd_retc.strptr, "0");
        parms->rxcmd_retc.strlength = 1;
        return RXEXIT_HANDLED;
    }
    // ok, a good address...now do the different commands
    else if (strcmp(parms->rxcmd_command.strptr, "TRACEOFF") == 0)
    {
        instanceInfo->trc = InstanceInfo::TRACEOFF;
        strcpy(parms->rxcmd_retc.strptr, "0");
        parms->rxcmd_retc.strlength = 1;
        return RXEXIT_HANDLED;
    }
    // ok, a good address...now do the different commands
    else if (strcmp(parms->rxcmd_command.strptr, "HALT") == 0)
    {
        instanceInfo->hlt = InstanceInfo::HALT;
        strcpy(parms->rxcmd_retc.strptr, "0");
        parms->rxcmd_retc.strlength = 1;
        return RXEXIT_HANDLED;
    }
    else
    {
        // unknown command
        strcpy(parms->rxcmd_retc.strptr, "-2");
        parms->rxcmd_retc.strlength = 2;
        parms->rxcmd_flags.rxfcfail = 1;
        return RXEXIT_HANDLED;
    }
}

int RexxEntry TestQueueExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->msq.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        default: break; // avoid warning: enumeration values not handled in switch
    }

    switch (subcode)
    {
        case RXMSQPLL:
        {
            RXMSQPLL_PARM *parms = (RXMSQPLL_PARM *)exitInfo;
            strcpy(parms->rxmsq_retc.strptr, "Hello World");
            parms->rxmsq_retc.strlength = strlen("Hello World");
            return RXEXIT_HANDLED;
        }
        case RXMSQPSH:
        {
            RXMSQPSH_PARM *parms = (RXMSQPSH_PARM *)exitInfo;
            if (parms->rxmsq_flags.rxfmlifo)
            {
                if (strcmp(parms->rxmsq_value.strptr, "LIFO") == 0)
                {
                    return RXEXIT_HANDLED;
                }
                else
                {
                    return RXEXIT_RAISE_ERROR;
                }
            }
            else
            {
                if (strcmp(parms->rxmsq_value.strptr, "FIFO") == 0)
                {
                    return RXEXIT_HANDLED;
                }
                else
                {
                    return RXEXIT_RAISE_ERROR;
                }
            }
        }
        case RXMSQSIZ:
        {
            RXMSQSIZ_PARM *parms = (RXMSQSIZ_PARM *)exitInfo;
            // this always returns a large, distinctive number
            parms->rxmsq_size = 999999;
            return RXEXIT_HANDLED;
        }
        case RXMSQNAM:
        {
            RXMSQPLL_PARM *parms = (RXMSQPLL_PARM *)exitInfo;
            strcpy(parms->rxmsq_retc.strptr, "FOOBAR");
            parms->rxmsq_retc.strlength = strlen("FOOBAR");
            return RXEXIT_HANDLED;
        }
    }
    return RXEXIT_RAISE_ERROR;
}

int RexxEntry TestSessionIOExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->sio.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        default: break; // avoid warning: enumeration values not handled in switch
    }
    switch (subcode)
    {
        case RXSIOTRD:
        {
            if (instanceInfo->sio == InstanceInfo::ALL || instanceInfo->sio == InstanceInfo::CONSOLE)
            {
                RXSIOTRD_PARM *parms = (RXSIOTRD_PARM *)exitInfo;
                strcpy(parms->rxsiotrd_retc.strptr, "Hello World");
                parms->rxsiotrd_retc.strlength = strlen("Hello World");
                return RXEXIT_HANDLED;
            }
            return RXEXIT_NOT_HANDLED;
        }
        case RXSIODTR:
        {
            if (instanceInfo->sio == InstanceInfo::ALL || instanceInfo->sio == InstanceInfo::CONSOLE_DEBUG)
            {
                RXSIODTR_PARM *parms = (RXSIODTR_PARM *)exitInfo;
                strcpy(parms->rxsiodtr_retc.strptr, "trace off");
                parms->rxsiodtr_retc.strlength = strlen("trace off");
                return RXEXIT_HANDLED;
            }
            return RXEXIT_NOT_HANDLED;
        }
        case RXSIOSAY:
        {
            if (instanceInfo->sio == InstanceInfo::ALL || instanceInfo->sio == InstanceInfo::CONSOLE)
            {
                RXSIOSAY_PARM *parms = (RXSIOSAY_PARM *)exitInfo;
                if (strcmp(parms->rxsio_string.strptr, "HELLO") == 0)
                {
                    return RXEXIT_HANDLED;
                }
                else
                {
                    return RXEXIT_RAISE_ERROR;
                }
            }
            return RXEXIT_NOT_HANDLED;
        }
        case RXSIOTRC:
        {
            if (instanceInfo->sio == InstanceInfo::ALL || instanceInfo->sio == InstanceInfo::CONSOLE_DEBUG)
            {
                RXMSQSIZ_PARM *parms = (RXMSQSIZ_PARM *)exitInfo;
                // this one is really hard to test, so it's sufficient that we got here.
                return RXEXIT_HANDLED;
            }
            return RXEXIT_NOT_HANDLED;
        }
    }
    return RXEXIT_RAISE_ERROR;
}

int RexxEntry TestHaltExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->hlt.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::HALT:
        {
            RXHLTTST_PARM *parms = (RXHLTTST_PARM *)exitInfo;
            parms->rxhlt_flags.rxfhhalt = 1;
            // the next call is a no
            instanceInfo->hlt = InstanceInfo::NOHALT;
            return RXEXIT_HANDLED;
        }
        case InstanceInfo::NOHALT:
        {
            RXHLTTST_PARM *parms = (RXHLTTST_PARM *)exitInfo;
            parms->rxhlt_flags.rxfhhalt = 0;
            return RXEXIT_HANDLED;
        }
        default: break; // avoid warning: enumeration values not handled in switch
    }
    return RXEXIT_NOT_HANDLED;
}

int RexxEntry TestTraceExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->trc.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::TRACEON:
        {
            RXTRCTST_PARM *parms = (RXTRCTST_PARM *)exitInfo;
            parms->rxtrc_flags.rxftrace = 1;
            // just one shot at this, otherwise the test rig goes into
            // a loop in the io intercepter.
            instanceInfo->trc = InstanceInfo::TRACEOFF;
            return RXEXIT_HANDLED;
        }
        case InstanceInfo::TRACEOFF:
        {
            RXTRCTST_PARM *parms = (RXTRCTST_PARM *)exitInfo;
            parms->rxtrc_flags.rxftrace = 0;
            return RXEXIT_HANDLED;
        }
        default: break; // avoid warning: enumeration values not handled in switch
    }
    return RXEXIT_NOT_HANDLED;
}

int RexxEntry TestInitExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->ini.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        default: break; // avoid warning: enumeration values not handled in switch
    }
    setContextVariable("TEST1", "Hello World");
    return RXEXIT_HANDLED;
}

int RexxEntry TestTerminationExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->ter.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        default: break; // avoid warning: enumeration values not handled in switch
    }

    char buffer[256];
    RXSTRING value;
    MAKERXSTRING(value, buffer, sizeof(buffer));

    getContextVariable("TEST1", &value);
    if (strcmp(value.strptr, "Hello World") != 0)
    {
        return RXEXIT_RAISE_ERROR;
    }
    return RXEXIT_NOT_HANDLED;
}

int RexxEntry TestScriptFunctionExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->exf.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        default: break; // avoid warning: enumeration values not handled in switch
    }
    // nothing else really testable in a classic fashion
    return RXEXIT_NOT_HANDLED;
}

int RexxEntry TestObjectFunctionExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->ofnc.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        default: break; // avoid warning: enumeration values not handled in switch
    }
    // nothing else really testable in a classic fashion
    return RXEXIT_NOT_HANDLED;
}

int RexxEntry TestNovalueExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->var.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        default: break; // avoid warning: enumeration values not handled in switch
    }

    // nothing else really testable in a classic fashion
    return RXEXIT_NOT_HANDLED;
}

int RexxEntry TestValueExit(int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = getApplicationData();

    switch (instanceInfo->val.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            return RXEXIT_RAISE_ERROR;
        default: break; // avoid warning: enumeration values not handled in switch
    }

    // nothing else really testable in a classic fashion
    return RXEXIT_NOT_HANDLED;
}


RexxReturnCode RexxEntry TestSubcomHandler(CONSTRXSTRING *cmd, unsigned short *flags, PRXSTRING retstr)
{
    *flags = RXSUBCOM_OK;
    // ok, a good address...now do the different commands
    if (strcmp(cmd->strptr, "GOOD") == 0)
    {
        strcpy(retstr->strptr, "0");
        retstr->strlength = 1;
        return 0;
    }
    else if (strcmp(cmd->strptr, "ERROR") == 0)
    {
        strcpy(retstr->strptr, "1");
        retstr->strlength = 1;
        *flags = RXSUBCOM_ERROR;
        return 0;
    }
    else if (strcmp(cmd->strptr, "SETVAR") == 0)
    {
        setContextVariable("TEST1", "Hello World");
        strcpy(retstr->strptr, "0");
        retstr->strlength = 1;
        return 0;
    }
    else if (strcmp(cmd->strptr, "GETVAR") == 0)
    {
        getContextVariable("TEST1", retstr);
        return 0;
    }
    else
    {
        strcpy(retstr->strptr, "-1");
        retstr->strlength = 2;
        *flags = RXSUBCOM_FAILURE;
        return 0;
    }
}
}

void REXXENTRY deregisterSubcomHandler()
{
    RexxDeregisterSubcom("TestSubcomHandler", NULL);
}


void REXXENTRY registerSubcomHandler(void *data)
{
    void *userData[2];
    userData[1] = data;
    // make sure this is deregistered first
    deregisterSubcomHandler();
    RexxRegisterSubcomExe("TestSubcomHandler", (REXXPFN)TestSubcomHandler, (char *)userData);
}


void REXXENTRY deregisterExits()
{
    RexxDeregisterExit("TestFunctionExit", NULL);
    RexxDeregisterExit("TestObjectFunctionExit", NULL);
    RexxDeregisterExit("TestScriptFunctionExit", NULL);
    RexxDeregisterExit("TestCommandExit", NULL);
    RexxDeregisterExit("TestQueueExit", NULL);
    RexxDeregisterExit("TestSessionIOExit", NULL);
    RexxDeregisterExit("TestHaltExit", NULL);
    RexxDeregisterExit("TestInitExit", NULL);
    RexxDeregisterExit("TestNovalueExit", NULL);
    RexxDeregisterExit("TestTerminationExit", NULL);
    RexxDeregisterExit("TestValueExit", NULL);
    RexxDeregisterExit("TestTraceExit", NULL);
}

void REXXENTRY registerExeExits(void *data)
{
    deregisterExits();

    void *userData[2];
    userData[1] = data;

    RexxRegisterExitExe("TestFunctionExit", (REXXPFN)TestFunctionExit, (char *)userData);
    RexxRegisterExitExe("TestObjectFunctionExit", (REXXPFN)TestObjectFunctionExit, (char *)userData);
    RexxRegisterExitExe("TestScriptFunctionExit", (REXXPFN)TestScriptFunctionExit, (char *)userData);
    RexxRegisterExitExe("TestCommandExit", (REXXPFN)TestCommandExit, (char *)userData);
    RexxRegisterExitExe("TestQueueExit", (REXXPFN)TestQueueExit, (char *)userData);
    RexxRegisterExitExe("TestSessionIOExit", (REXXPFN)TestSessionIOExit, (char *)userData);
    RexxRegisterExitExe("TestHaltExit", (REXXPFN)TestHaltExit, (char *)userData);
    RexxRegisterExitExe("TestInitExit", (REXXPFN)TestInitExit, (char *)userData);
    RexxRegisterExitExe("TestNovalueExit", (REXXPFN)TestNovalueExit, (char *)userData);
    RexxRegisterExitExe("TestTerminationExit", (REXXPFN)TestTerminationExit, (char *)userData);
    RexxRegisterExitExe("TestValueExit", (REXXPFN)TestValueExit, (char *)userData);
    RexxRegisterExitExe("TestTraceExit", (REXXPFN)TestTraceExit, (char *)userData);
}

void REXXENTRY registerDllExits(void *data)
{
    deregisterExits();

    void *userData[2];
    userData[1] = data;

    RexxRegisterExitDll("TestFunctionExit", "orxexits",        "TestFunctionExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestObjectFunctionExit", "orxexits",  "TestObjectFunctionExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestScriptFunctionExit", "orxexits",  "TestScriptFunctionExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestCommandExit", "orxexits",         "TestCommandExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestQueueExit", "orxexits",           "TestQueueExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestSessionIOExit", "orxexits",       "TestSessionIOExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestHaltExit", "orxexits",            "TestHaltExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestInitExit", "orxexits",            "TestInitExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestNovalueExit", "orxexits",         "TestNovalueExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestTerminationExit", "orxexits",     "TestTerminationExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestValueExit", "orxexits",           "TestValueExit", (char *)userData, RXEXIT_DROPPABLE);
    RexxRegisterExitDll("TestTraceExit", "orxexits",           "TestTraceExit", (char *)userData, RXEXIT_DROPPABLE);
}


bool REXXENTRY buildRegisteredExitList(InstanceInfo *instanceInfo, RXSYSEXIT *exitList)
{
    int counter = 0;
    if (instanceInfo->fnc.isEnabled())
    {
        exitList->sysexit_name = "TestFunctionExit";
        exitList->sysexit_code = RXFNC;
        exitList++;
        counter++;
    }
    if (instanceInfo->cmd.isEnabled())
    {
        exitList->sysexit_name = "TestCommandExit";
        exitList->sysexit_code = RXCMD;
        exitList++;
        counter++;
    }
    if (instanceInfo->msq.isEnabled())
    {
        exitList->sysexit_name = "TestQueueExit";
        exitList->sysexit_code = RXMSQ;
        exitList++;
        counter++;
    }
    if (instanceInfo->sio.isEnabled())
    {
        exitList->sysexit_name = "TestSessionIOExit";
        exitList->sysexit_code = RXSIO;
        exitList++;
        counter++;
    }
    if (instanceInfo->hlt.isEnabled())
    {
        exitList->sysexit_name = "TestHaltExit";
        exitList->sysexit_code = RXHLT;
        exitList++;
        counter++;
    }
    if (instanceInfo->trc.isEnabled())
    {
        exitList->sysexit_name = "TestTraceExit";
        exitList->sysexit_code = RXTRC;
        exitList++;
        counter++;
    }
    if (instanceInfo->ini.isEnabled())
    {
        exitList->sysexit_name = "TestInitExit";
        exitList->sysexit_code = RXINI;
        exitList++;
        counter++;
    }
    if (instanceInfo->ter.isEnabled())
    {
        exitList->sysexit_name = "TestTerminationExit";
        exitList->sysexit_code = RXTER;
        exitList++;
        counter++;
    }
    if (instanceInfo->exf.isEnabled())
    {
        exitList->sysexit_name = "TestScriptFunctionExit";
        exitList->sysexit_code = RXEXF;
        exitList++;
        counter++;
    }
    if (instanceInfo->var.isEnabled())
    {
        exitList->sysexit_name = "TestNovalueExit";
        exitList->sysexit_code = RXNOVAL;
        exitList++;
        counter++;
    }
    if (instanceInfo->val.isEnabled())
    {
        exitList->sysexit_name = "TestValueExit";
        exitList->sysexit_code = RXVALUE;
        exitList++;
        counter++;
    }
    if (instanceInfo->ofnc.isEnabled())
    {
        exitList->sysexit_name = "TestObjectFunctionExit";
        exitList->sysexit_code = RXOFNC;
        exitList++;
        counter++;
    }

    if (counter > 0)
    {
        exitList->sysexit_name = NULL;
        exitList->sysexit_code = 0;
        return true;
    }
    return false;
}


void REXXENTRY invokeRexxStart(InstanceInfo *instanceInfo)
{
    CONSTRXSTRING args[10];
    RXSYSEXIT       registeredExits[RXNOOFEXITS];
    short           callRC = 0;
    RXSTRING        returnValue;

    instanceInfo->code = 0;
    instanceInfo->rc = 0;
    strcpy(instanceInfo->returnResult, "");

    for (size_t i = 0; i < instanceInfo->argCount; i++)
    {
        if (instanceInfo->arguments[i] != NULL)
        {
            MAKERXSTRING(args[i], instanceInfo->arguments[i], strlen(instanceInfo->arguments[i]));
        }
        else
        {
            MAKERXSTRING(args[i], NULL, 0);
        }
    }

    RXSYSEXIT *exits = NULL;

    if (instanceInfo->exitStyle == InstanceInfo::REGISTERED_DLL)
    {
        registerDllExits((void *)instanceInfo);
        buildRegisteredExitList(instanceInfo, registeredExits);
        exits = registeredExits;
    }

    registerSubcomHandler(instanceInfo);
    MAKERXSTRING(returnValue, NULL, 0);

    int rc = RexxStart(instanceInfo->argCount, args, instanceInfo->programName, NULL, instanceInfo->initialAddress, RXCOMMAND, exits, &callRC, &returnValue);

    if (rc < 0)
    {
        instanceInfo->rc = -rc;
    }
    else
    {
        if (returnValue.strptr != NULL)
        {
            strncpy(instanceInfo->returnResult, returnValue.strptr, sizeof(instanceInfo->returnResult));
            RexxFreeMemory(returnValue.strptr);
        }
    }
    deregisterExits();
    deregisterSubcomHandler();
}
