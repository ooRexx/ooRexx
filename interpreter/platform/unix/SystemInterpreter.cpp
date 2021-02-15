/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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

#include <termios.h>
#include <stdio.h>
#include "RexxCore.h"
#include "SystemInterpreter.hpp"
#include "Interpreter.hpp"
#include "GlobalNames.hpp"
#include "FileNameBuffer.hpp"

sigset_t SystemInterpreter::oldmask;
sigset_t SystemInterpreter::newmask;

class InterpreterInstance;

void SystemInterpreter::processStartup()
{
    // now do the platform independent startup
    Interpreter::processStartup();
}


void SystemInterpreter::processShutdown()
{
    // now do the platform independent shutdown
    Interpreter::processStartup();
}

void signalHandler(int sig)
{

#ifdef ORXAP_DEBUG
    switch (sig)
    {
        case (SIGINT):
            printf("\n*** Rexx interrupted.\n");
            break;
        case (SIGTERM):
            printf("\n*** Rexx terminated.\n*** Closing Rexx !\n");  /* exit(0); */
            break;
        case (SIGSEGV):
            printf("\n*** Segmentation fault.\n*** Closing Rexx !\n");
            break;
        case (SIGFPE):
            printf("\n*** Floating point error.\n*** Closing Rexx\n");
            break;
        case (SIGBUS):
            printf("\n*** Bus error.\n*** Closing Rexx\n");
            break;
        case (SIGPIPE):
            printf("\n*** Broken pipe.\n*** Closing Rexx\n");
            break;
        default:
            printf("\n*** Error,closing REXX !\n");
            break;
    }
#endif

    // if the signal is a ctrl-C, we perform a halt operation
    if (sig == SIGINT)
    {
        Interpreter::haltAllActivities(GlobalNames::SIGINT_STRING);
        return;
    }
    else if (sig == SIGTERM)
    {
        Interpreter::haltAllActivities(GlobalNames::SIGTERM_STRING);
        return;
    }
    else if (sig == SIGHUP)
    {
        Interpreter::haltAllActivities(GlobalNames::SIGHUP_STRING);
        return;
    }
    else
    {
        exit(0);
    }
}


void SystemInterpreter::startInterpreter()
{

    /* Set the cleanup handler for unconditional process termination          */
    struct sigaction new_action;
    struct sigaction old_action;

    /* Set up the structure to specify the new action                         */
    new_action.sa_handler = signalHandler;
    old_action.sa_handler = NULL;
    sigfillset(&new_action.sa_mask);
//    new_action.sa_flags = SA_RESTART;
    new_action.sa_flags = 0; // do not use SA_RESTART or ctrl-c will not work as expected!

/* Termination signals are set by Object REXX whenever the signals were not set */
/* from outside (calling C-routine). The SIGSEGV signal is not set any more, so */
/* that we now get a coredump instead of a hang up                              */

    sigaction(SIGINT, NULL, &old_action);
    sigaction(SIGTERM, NULL, &old_action);
    sigaction(SIGHUP, NULL, &old_action);
    if (old_action.sa_handler == NULL)           /* not set by ext. exit handler*/
    {
        sigaction(SIGINT, &new_action, NULL);  /* exitClear on SIGINT signal    */
        sigaction(SIGTERM, &new_action, NULL); /* exitClear on SIGTERM signal   */
        sigaction(SIGHUP, &new_action, NULL);  /* exitClear on SIGHUP signal    */
    }

    // Set SIGPIPE to ignore so that ADDRESS WITH pipes or rxsock sockets
    // may return EPIPE error codes instead of the interpreter being killed
    signal(SIGPIPE, SIG_IGN);
}


void SystemInterpreter::terminateInterpreter()
{
// revert stdin and stdout back to their original states
    setvbuf(stdin, (char *)NULL, _IOLBF, 0);
    setvbuf(stdout, (char *)NULL, _IOLBF, 0);
}



void SystemInterpreter::live(size_t liveMark)
{
}

void SystemInterpreter::liveGeneral(MarkReason reason)
{
}


/**
 * Retrieve the value of an envinment variable into a smart buffer.
 *
 * @param variable The name of the environment variable.
 * @param buffer   The buffer used for the return.
 *
 * @return true if the variable exists, false otherwise.
 */
bool SystemInterpreter::getEnvironmentVariable(const char *variable, FileNameBuffer &buffer)
{
    const char *value = getenv(variable);
    if (value != NULL)
    {
        buffer = value;
        return true;
    }
    // make sure this is a null string
    buffer = "";
    return false;
}


/**
 * Set an environment variable to a new value.
 *
 * @param variableName
 *               The name of the environment variable.
 * @param value  The variable value.
 */
void SystemInterpreter::setEnvironmentVariable(const char *variableName, const char *value)
{
    // A NULL value is an unset operation
    if (value == NULL)
    {
        unsetenv(variableName);
    }
    // we need a string value for the set.
    else
    {
        setenv(variableName, value, true);
    }
}
