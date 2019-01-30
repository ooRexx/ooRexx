/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2019 Rexx Language Association. All rights reserved.    */
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

#include "oorexxapi.h"
#include "orxexits.hpp"

RexxObjectPtr TestGetContextVariable(RexxExitContext *context, size_t argc, RexxObjectPtr *argv)
{
    return context->GetContextVariable(context->ObjectToStringValue(argv[0]));
}

RexxObjectPtr TestSetContextVariable(RexxExitContext *context, size_t argc, RexxObjectPtr *argv)
{
    context->SetContextVariable(context->ObjectToStringValue(argv[0]), argv[1]);
    return context->True();
}

RexxObjectPtr TestDropContextVariable(RexxExitContext *context, size_t argc, RexxObjectPtr *argv)
{
    context->DropContextVariable(context->ObjectToStringValue(argv[0]));
    return context->True();
}

RexxObjectPtr TestGetAllContextVariables(RexxExitContext *context, size_t argc, RexxObjectPtr *argv)
{
    return (RexxObjectPtr)context->GetAllContextVariables();
}

RexxObjectPtr TestGetCallerContext(RexxExitContext *context, size_t argc, RexxObjectPtr *argv)
{
    return context->GetCallerContext();
}


int invokeExitFunction(RexxExitContext *context, const char *name, PEXIT exitInfo)
{
    RXOFNCCAL_PARM *parms = (RXOFNCCAL_PARM *)exitInfo;
    const char *functionName = parms->rxfnc_name.strptr;

    if (strcmp(functionName, "TESTGETCONTEXTVARIABLE") == 0)
    {
        parms->rxfnc_retc = TestGetContextVariable(context, parms->rxfnc_argc, parms->rxfnc_argv);
        return RXEXIT_HANDLED;
    }
    else if (strcmp(functionName, "TESTSETCONTEXTVARIABLE") == 0)
    {
        parms->rxfnc_retc = TestSetContextVariable(context, parms->rxfnc_argc, parms->rxfnc_argv);
        return RXEXIT_HANDLED;
    }
    else if (strcmp(functionName, "TESTDROPCONTEXTVARIABLE") == 0)
    {
        parms->rxfnc_retc = TestDropContextVariable(context, parms->rxfnc_argc, parms->rxfnc_argv);
        return RXEXIT_HANDLED;
    }
    else if (strcmp(functionName, "TESTGETALLCONTEXTVARIABLES") == 0)
    {
        parms->rxfnc_retc = TestGetAllContextVariables(context, parms->rxfnc_argc, parms->rxfnc_argv);
        return RXEXIT_HANDLED;
    }
    else if (strcmp(functionName, "TESTGETCALLERCONTEXT") == 0)
    {
        parms->rxfnc_retc = TestGetCallerContext(context, parms->rxfnc_argc, parms->rxfnc_argv);
        return RXEXIT_HANDLED;
    }
    else if (strcmp(functionName, "TESTSUBCALL") == 0)
    {
        // return the status of the flag
        parms->rxfnc_retc = parms->rxfnc_flags.rxffsub ? context->NewStringFromAsciiz("SUBROUTINE") : context->NewStringFromAsciiz("FUNCTION");
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
    else if (strcmp(functionName, "TESTARGUMENTS") == 0 || strcmp(functionName, "TESTARGUMENTS2") == 0)
    {
        RexxArrayObject args = context->NewArray(parms->rxfnc_argc);
        for (int i = 0; i < parms->rxfnc_argc; i++)
        {
            if (parms->rxfnc_argv[i] != NULLOBJECT)
            {
                context->ArrayPut(args, parms->rxfnc_argv[i], i + 1);
            }
        }
        parms->rxfnc_retc = (RexxObjectPtr)context->SendMessage2(args, "TOSTRING", context->String("L"), context->String(","));
        return RXEXIT_HANDLED;
    }
    else
    {
        // pass on this
        return RXEXIT_NOT_HANDLED;
    }
}

int RexxEntry TestContextFunctionExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->fnc.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Function Exit"));
            return RXEXIT_NOT_HANDLED;
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
    else
    {
        // pass on this
        return RXEXIT_NOT_HANDLED;
    }
}

int RexxEntry TestContextCommandExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->cmd.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Command Exit"));
            return RXEXIT_NOT_HANDLED;
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

int RexxEntry TestContextQueueExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->msq.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Queue Exit"));
            return RXEXIT_NOT_HANDLED;
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

int RexxEntry TestContextSessionIOExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->sio.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("I/O Exit"));
            return RXEXIT_NOT_HANDLED;
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

int RexxEntry TestContextHaltExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->hlt.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Halt Exit"));
            return RXEXIT_NOT_HANDLED;
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

int RexxEntry TestContextTraceExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->trc.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Trace Exit"));
            return RXEXIT_NOT_HANDLED;
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

int RexxEntry TestContextInitExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->ini.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Init Exit"));
            return RXEXIT_NOT_HANDLED;
        default: break; // avoid warning: enumeration values not handled in switch
    }

    context->SetContextVariable("test1", context->NewStringFromAsciiz("Hello World"));
    // retrieve this to ensure this was set
    RexxObjectPtr value = context->GetContextVariable("test1");
    if (value == NULL)
    {
        context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Initialization Exit context variable set failure"));
    }
    else if (strcmp(context->ObjectToStringValue(value), "Hello World") != 0)
    {
        context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Initialization Exit context variable set failure"));
    }
    return RXEXIT_HANDLED;
}

int RexxEntry TestContextTerminationExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->ter.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Termination Exit"));
            return RXEXIT_NOT_HANDLED;
        default: break; // avoid warning: enumeration values not handled in switch
    }

    RexxObjectPtr value = context->GetContextVariable("test1");
    if (strcmp(context->ObjectToStringValue(value), "Hello World") != 0)
    {
        context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Termination Exit"));
    }
    return RXEXIT_HANDLED;
}

int RexxEntry TestContextScriptFunctionExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->exf.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Script Function Exit"));
            return RXEXIT_NOT_HANDLED;
        default: break; // avoid warning: enumeration values not handled in switch
    }
    return invokeExitFunction(context, "Script", exitInfo);
}

int RexxEntry TestContextObjectFunctionExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->ofnc.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Object Function Exit"));
            return RXEXIT_NOT_HANDLED;
        default: break; // avoid warning: enumeration values not handled in switch
    }
    return invokeExitFunction(context, "Object", exitInfo);
}

int RexxEntry TestContextNovalueExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->var.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Novalue Exit"));
            return RXEXIT_NOT_HANDLED;
        default: break; // avoid warning: enumeration values not handled in switch
    }

    RXVARNOVALUE_PARM *parms = (RXVARNOVALUE_PARM *)exitInfo;

    const char *variableName = context->CString(parms->variable_name);

    if (strcmp(variableName, "FOO") == 0)
    {
        // need to just return a string value for proper validation
        parms->value = context->String("BAR");
        return RXEXIT_HANDLED;
    }
    return RXEXIT_NOT_HANDLED;
}

int RexxEntry TestContextValueExit(RexxExitContext *context, int code, int subcode, PEXIT exitInfo)
{
    InstanceInfo *instanceInfo = (InstanceInfo *)context->GetApplicationData();

    switch (instanceInfo->val.action)
    {
        case InstanceInfo::SKIP:
            return RXEXIT_NOT_HANDLED;
        case InstanceInfo::EXIT_ERROR:
            return RXEXIT_RAISE_ERROR;
        case InstanceInfo::RAISE:
            context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz("Value() Exit"));
            return RXEXIT_NOT_HANDLED;
        default: break; // avoid warning: enumeration values not handled in switch
    }

    RXVALCALL_PARM *parms = (RXVALCALL_PARM *)exitInfo;
    const char *selector = context->CString(parms->selector);

    if (strcmp(selector, "TEST") == 0)
    {
        const char *variableName = context->CString(parms->variable_name);
        if (strcmp(variableName, "FOO") == 0)
        {
            // need to just return a string value for proper validation
            parms->value = context->String("BAR");
            return RXEXIT_HANDLED;
        }
    }
    return RXEXIT_NOT_HANDLED;
}

// direct command handler
RexxObjectPtr RexxEntry TestDirectCommandHandler(RexxExitContext *context, RexxStringObject address, RexxStringObject command)
{
    const char *c = context->CString(command);

    // ok, a good address...now do the different commands
    if (strcmp(c, "GOOD") == 0)
    {
        return context->False();
    }
    else if (strcmp(c, "ERROR") == 0)
    {
        context->RaiseCondition("ERROR", command, NULLOBJECT, context->True());
        return NULLOBJECT;
    }
    else if (strcmp(c, "RAISE") == 0)
    {
        context->RaiseException1(Rexx_Error_System_service_user_defined, context->String("Command Handler"));
        return NULLOBJECT;
    }
    else if (strcmp(c, "GETVAR") == 0)
    {
        return context->GetContextVariable("TEST1");
    }
    else if (strcmp(c, "SETVAR") == 0)
    {
        context->SetContextVariable("TEST1", context->String("Hello World"));
        return context->False();
    }
    else
    {
        context->RaiseCondition("FAILURE", command, NULLOBJECT, context->WholeNumber(-1));
        return NULLOBJECT;
    }
}

// redirecting command handler with additional ioContext argument
RexxObjectPtr RexxEntry TestRedirectingCommandHandler(RexxExitContext *context, RexxStringObject address, RexxStringObject command, RexxIORedirectorContext *ioContext)
{
    const char *c = context->CString(command);

    // ok, a good address...now do the different commands
    if (strcmp(c, "GOOD") == 0)
    {
        return context->False();
    }
    else if (strcmp(c, "ERROR") == 0)
    {
        context->RaiseCondition("ERROR", command, NULLOBJECT, context->True());
        return NULLOBJECT;
    }
    else if (strcmp(c, "RAISE") == 0)
    {
        context->RaiseException1(Rexx_Error_System_service_user_defined, context->String("Command Handler"));
        return NULLOBJECT;
    }
    else if (strcmp(c, "GETVAR") == 0)
    {
        return context->GetContextVariable("TEST1");
    }
    else if (strcmp(c, "SETVAR") == 0)
    {
        context->SetContextVariable("TEST1", context->String("Hello World"));
        return context->False();
    }
    else if (strcmp(c, "REDIRECTION") == 0)
    {
        size_t rc = 0;
        // we return a five-digit number with each digit in sequence
        // representing the status of:
        // - IsRedirectionRequested()
        // - IsInputRedirected()
        // - IsOutputRedirected()
        // - IsErrorRedirected()
        // - AreOutputAndErrorSameTarget()
        rc = rc * 10 + ioContext->IsRedirectionRequested();
        rc = rc * 10 + ioContext->IsInputRedirected();
        rc = rc * 10 + ioContext->IsOutputRedirected();
        rc = rc * 10 + ioContext->IsErrorRedirected();
        rc = rc * 10 + ioContext->AreOutputAndErrorSameTarget();
        return context->WholeNumberToObject(rc);
    }
    else
    {
        context->RaiseCondition("FAILURE", command, NULLOBJECT, context->WholeNumber(-1));
        return NULLOBJECT;
    }
}


bool REXXENTRY buildContextExitList(InstanceInfo *instanceInfo, RexxContextExit *exitList)
{
    int counter = 0;
    if (instanceInfo->fnc.isEnabled())
    {
        exitList->handler = TestContextFunctionExit;
        exitList->sysexit_code = RXFNC;
        exitList++;
        counter++;
    }
    if (instanceInfo->cmd.isEnabled())
    {
        exitList->handler = TestContextCommandExit;
        exitList->sysexit_code = RXCMD;
        exitList++;
        counter++;
    }
    if (instanceInfo->msq.isEnabled())
    {
        exitList->handler = TestContextQueueExit;
        exitList->sysexit_code = RXMSQ;
        exitList++;
        counter++;
    }
    if (instanceInfo->sio.isEnabled())
    {
        exitList->handler = TestContextSessionIOExit;
        exitList->sysexit_code = RXSIO;
        exitList++;
        counter++;
    }
    if (instanceInfo->hlt.isEnabled())
    {
        exitList->handler = TestContextHaltExit;
        exitList->sysexit_code = RXHLT;
        exitList++;
        counter++;
    }
    if (instanceInfo->trc.isEnabled())
    {
        exitList->handler = TestContextTraceExit;
        exitList->sysexit_code = RXTRC;
        exitList++;
        counter++;
    }
    if (instanceInfo->ini.isEnabled())
    {
        exitList->handler = TestContextInitExit;
        exitList->sysexit_code = RXINI;
        exitList++;
        counter++;
    }
    if (instanceInfo->ter.isEnabled())
    {
        exitList->handler = TestContextTerminationExit;
        exitList->sysexit_code = RXTER;
        exitList++;
        counter++;
    }
    if (instanceInfo->exf.isEnabled())
    {
        exitList->handler = TestContextScriptFunctionExit;
        exitList->sysexit_code = RXEXF;
        exitList++;
        counter++;
    }
    if (instanceInfo->var.isEnabled())
    {
        exitList->handler = TestContextNovalueExit;
        exitList->sysexit_code = RXNOVAL;
        exitList++;
        counter++;
    }
    if (instanceInfo->val.isEnabled())
    {
        exitList->handler = TestContextValueExit;
        exitList->sysexit_code = RXVALUE;
        exitList++;
        counter++;
    }
    if (instanceInfo->ofnc.isEnabled())
    {
        exitList->handler = TestContextObjectFunctionExit;
        exitList->sysexit_code = RXOFNC;
        exitList++;
        counter++;
    }

    if (counter > 0)
    {
        exitList->handler = NULL;
        exitList->sysexit_code = 0;
        return true;
    }
    return false;
}


RexxReturnCode REXXENTRY createInstance(InstanceInfo *instanceInfo, RexxInstance *&instance, RexxThreadContext *&threadContext)
{
    RexxOption options[25];      // space for a boatload of options
                                 // space for building exit lists
    RexxContextExit contextExits[RXNOOFEXITS];
    RXSYSEXIT       registeredExits[RXNOOFEXITS];
    RexxContextEnvironment directCommandHandlers[2];
    RexxRedirectingEnvironment redirectingCommandHandlers[2];
    RexxRegisteredEnvironment subcomHandlers[2];
    int optionCount = 0;

    switch (instanceInfo->exitStyle)
    {
        case InstanceInfo::CONTEXT:
        {
            options[optionCount].optionName = DIRECT_EXITS;
            buildContextExitList(instanceInfo, contextExits);
            options[optionCount].option = (void *)contextExits;
            optionCount++;
            break;
        }
        case InstanceInfo::REGISTERED_DLL:
        {
            options[optionCount].optionName = REGISTERED_EXITS;
            registerDllExits((void *)instanceInfo);
            buildRegisteredExitList(instanceInfo, registeredExits);
            options[optionCount].option = (void *)registeredExits;
            optionCount++;
            break;
        }
        case InstanceInfo::REGISTERED_EXE:
        {
            options[optionCount].optionName = REGISTERED_EXITS;
            registerExeExits((void *)instanceInfo);
            buildRegisteredExitList(instanceInfo, registeredExits);
            options[optionCount].option = (void *)registeredExits;
            optionCount++;
            break;
        }
        default:
            // no options added
            break;

    }

    if (instanceInfo->extensionPath != NULL)
    {
        options[optionCount].optionName = EXTERNAL_CALL_PATH;
        options[optionCount].option = instanceInfo->extensionPath;
        optionCount++;
    }

    if (instanceInfo->extensions != NULL)
    {
        options[optionCount].optionName = EXTERNAL_CALL_EXTENSIONS;
        options[optionCount].option = instanceInfo->extensions;
        optionCount++;
    }

    if (instanceInfo->loadLibrary != NULL)
    {
        options[optionCount].optionName = LOAD_REQUIRED_LIBRARY;
        options[optionCount].option = instanceInfo->loadLibrary;
        optionCount++;
    }

    if (instanceInfo->initialAddress != NULL)
    {
        options[optionCount].optionName = INITIAL_ADDRESS_ENVIRONMENT;
        options[optionCount].option = instanceInfo->initialAddress;
        optionCount++;
    }

    directCommandHandlers[0].name = "TESTDIRECT";
    directCommandHandlers[0].handler = TestDirectCommandHandler;
    directCommandHandlers[1].name = NULL;
    directCommandHandlers[1].handler = NULL;
    options[optionCount].optionName = DIRECT_ENVIRONMENTS;
    options[optionCount].option = directCommandHandlers;
    optionCount++;

    redirectingCommandHandlers[0].name = "TESTREDIRECTING";
    redirectingCommandHandlers[0].handler = TestRedirectingCommandHandler;
    redirectingCommandHandlers[1].name = NULL;
    redirectingCommandHandlers[1].handler = NULL;
    options[optionCount].optionName = REDIRECTING_ENVIRONMENTS;
    options[optionCount].option = redirectingCommandHandlers;
    optionCount++;

    subcomHandlers[0].name = "TESTSUBCOM";
    subcomHandlers[0].registeredName = "TestSubcomHandler";
    subcomHandlers[1].name = NULL;
    subcomHandlers[1].registeredName = NULL;
    options[optionCount].optionName = REGISTERED_ENVIRONMENTS;
    options[optionCount].option = subcomHandlers;
    optionCount++;

    registerSubcomHandler(instanceInfo);

    options[optionCount].optionName = APPLICATION_DATA;
    options[optionCount].option = (void *)instanceInfo;
    optionCount++;

    options[optionCount].optionName = NULL;
    options[optionCount].option = (void *)NULL;

    return RexxCreateInterpreter(&instance, &threadContext, options);
}


void REXXENTRY invokeProgram(InstanceInfo *instanceInfo)
{
    RexxInstance *instance;
    RexxThreadContext *context;

    instanceInfo->code = 0;
    instanceInfo->rc = 0;
    strcpy(instanceInfo->returnResult, "");

    createInstance(instanceInfo, instance, context);

    RexxArrayObject args = context->NewArray(instanceInfo->argCount);
    for (size_t i = 0; i < instanceInfo->argCount; i++)
    {
        if (instanceInfo->arguments[i] != NULL)
        {
            context->ArrayPut(args, context->String(instanceInfo->arguments[i]), i + 1);
        }
    }

    RexxObjectPtr result = context->CallProgram(instanceInfo->programName, args);
    // if an exception occurred, get the decoded exception information
    if (context->CheckCondition())
    {
        RexxCondition condition;

        RexxDirectoryObject cond = context->GetConditionInfo();
        context->DecodeConditionInfo(cond, &condition);
        instanceInfo->code = condition.code;
        instanceInfo->rc = condition.rc;
    }
    else
    {
        if (result != NULLOBJECT)
        {
            CSTRING resultString = context->CString(result);
            strncpy(instanceInfo->returnResult, resultString, sizeof(instanceInfo->returnResult));
        }
    }
    // make sure we terminate this first
    instance->Terminate();
    deregisterExits();
    deregisterSubcomHandler();
}
