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
/* REXX Windows Support                                                      */
/*                                                                           */
/* Main Windows interpreter control.  This is the preferred location for     */
/* all platform dependent global variables.                                  */
/* The interpreter does not instantiate an instance of this                  */
/* class, so most variables and methods should be static.                    */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#include "RexxCore.h"
#include "SystemInterpreter.hpp"
#include "Interpreter.hpp"
#include "FileNameBuffer.hpp"

ULONG SystemInterpreter::exceptionHostProcessId = 0;
HANDLE SystemInterpreter::exceptionHostProcess = NULL;
bool SystemInterpreter::exceptionConsole = false;
bool SystemInterpreter::explicitConsole = false;
int SystemInterpreter::signalCount = 0;

class InterpreterInstance;


HINSTANCE SystemInterpreter::moduleHandle = 0;      // handle to the interpeter DLL

/**
 * Handle system-specific once-per-process startup tasks.
 *
 * @param mod    The module handle.
 */
void SystemInterpreter::processStartup(HINSTANCE mod)
{
    moduleHandle = mod;
    // now do the platform independent startup
    Interpreter::processStartup();
}


/**
 * Handle once-per-process shutdown tasks.
 */
void SystemInterpreter::processShutdown()
{
    // now do the platform independent shutdown
    Interpreter::processShutdown();
}


/**
 * Handle any platform-specific tasks associated with starting
 * an interpreter instance.
 */
void SystemInterpreter::startInterpreter()
{
}


/**
 * Handle any platform-specific tasks associated with
 * terminating an interpreter instance.
 */
void SystemInterpreter::terminateInterpreter()
{
}


/**
 * Perform any additional garbage collection marking that might
 * be required for platform-specific interpreter instance objects.
 *
 * @param liveMark
 */
void SystemInterpreter::live(size_t liveMark)
{
}


/**
 * Perform any additional garbage collection marking that might
 * be required for platform-specific interpreter instance objects.
 *
 * @param liveMark
 */
void SystemInterpreter::liveGeneral(MarkReason reason)
{
}


/**
 * Load a message from the system message resources.
 *
 * @param code   The error message identifier.
 * @param buffer The buffer for the returned message.
 * @param bufferLength
 *               The length of th message buffer.
 *
 * @return The success/failure indicator.
 */
bool SystemInterpreter::loadMessage(wholenumber_t code, char *buffer, size_t bufferLength)
{
    return LoadString(moduleHandle, (UINT)code, buffer, (int)bufferLength) != 0;
}

/**
 * Process a signal based on current command handler settings.
 *
 * @param dwCtrlType The type of exception.
 *
 * @return true if we handled the signal, false if it should be
 *         passed up the handler chain.
 */
bool SystemInterpreter::processSignal(DWORD dwCtrlType)
{
    /* Ignore Ctrl+C if console is running in console */
    if (exceptionConsole)
    {
        GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, exceptionHostProcessId);
        return true;   /* ignore signal */
    }

    if (exceptionHostProcess)
    {
        GenerateConsoleCtrlEvent(CTRL_C_EVENT, exceptionHostProcessId);
        TerminateProcess(exceptionHostProcess, -1);
    }
    // if this is a ctrl_C, try to halt all of the activities.  If we hit this
    // in a situation where we still have one pending, then we'll allow the system
    // to kill the process.
    if (dwCtrlType == CTRL_C_EVENT)
    {
        return Interpreter::haltAllActivities(OREF_NULL);
    }
    return true;      /* ignore signal */
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
    // do the request first with no buffer...this will return the size of the variable, which
    // will allow us to ensure the buffer is large enough
    DWORD dwSize = GetEnvironmentVariable(variable, NULL, 0);
    if (dwSize > 0)
    {
        // now request the variable again
        buffer.ensureCapacity(dwSize);
        GetEnvironmentVariable(variable, (char *)buffer, dwSize);
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
int SystemInterpreter::setEnvironmentVariable(const char *variableName, const char *value)
{
    return SetEnvironmentVariable(variableName, value) == 0 ? 0 : GetLastError();
}
