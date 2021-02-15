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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "LocalAPIManager.hpp"
#include "SysLocalAPIManager.hpp"
#include "SysCSStream.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "SysProcess.hpp"

#ifdef AIX
extern "C"
{
#endif

int _rexxapi_fini()__attribute__((destructor));

int _rexxapi_fini()
{
    // this shuts down the entire environment
    LocalAPIManager::shutdownInstance();
    return 0;
}

#ifdef AIX
}
#endif

/**
 * Start the rxapi daemon process.
 */
void SysLocalAPIManager::startServerProcess()
{
#define RXAPI "rxapi"
#define DOTDOT_BIN_RXAPI "../bin/" RXAPI
#define DOT_RXAPI "./" RXAPI
    char apiExeName[] = RXAPI;
    char *apiExeArg[2];
    apiExeArg[0] = apiExeName;
    apiExeArg[1] = NULL;

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        return;
    }


    pid_t pid = fork();
    if (pid < 0)
    {
        throw new ServiceException(API_FAILURE, "Unable to start API server");
    }

    if (pid != 0)
    {
        // we are the parent process
        return;
    }
    // if we get here we are the child process

    // become the session leader
    setsid();

    // housekeeping - chdir to the root subdir and close all open files
    int ignore = chdir("/");
    umask(0);
    for(int i = 0; i < 1024; i++)
    {
        close(i);
    }

    // we will make multiple attempts at locating rxapi, first with a full path.
    // while on Windows the rxapi executable is located in the same directory
    // as the rexxapi library, on Unix these two are in different paths.
    // typically the rexxapi library is in install-path/lib/ or in
    // install-path/lib64/ and the rxapi executable is in install-path/bin/
    // so we try to locate rxapi with ../bin/rxapi relative to the library
    // location.
    AutoFree fullExeName = NULL;
    const char *installLocation =  SysProcess::getLibraryLocation();
    if (installLocation != NULL)
    {
        // the library location includes the trailing "/" character
        size_t commandSize = strlen(installLocation) + strlen(DOTDOT_BIN_RXAPI) + 1;

        fullExeName = (char *)malloc(commandSize);
        // the path might contain blanks, so we'll need to enclose the
        // command name in quotes
        snprintf(fullExeName, commandSize, "%s%s", installLocation, DOTDOT_BIN_RXAPI);
        execvp(fullExeName, apiExeArg);
    }

    // next we use the unqualified rxapi name and try to locate it on $PATH
    execvp(RXAPI, apiExeArg);

    // did this still fail? Last attempt, try to load this from the current directory
    execvp(DOT_RXAPI, apiExeArg);

    // still no luck? This is a launch failure. Because we are the forked process,
    // we need to exit immediately.
    exit(1);
}


/**
 * Check to see if we've inherited a session queue from a calling process.  This shows
 * up as an environment variable value.
 *
 * @param sessionQueue
 *               The returned session queue handle, if it exists.
 *
 * @return true if the session queue is inherited, false if a new once needs to be created.
 */
bool SysLocalAPIManager::getActiveSessionQueue(QueueHandle &sessionQueue)
{
    // check to see if we have an env variable set...if we do we
    // inherit from our parent session
    char *envbuffer = getenv("RXQUEUESESSION");
    if (envbuffer != NULL)
    {
        sscanf(envbuffer, "%p", (void **)&sessionQueue);
        return true;
    }
    return false;
}

/**
 * Set the active session queue as an environment variable.
 *
 * @param sessionQueue
 *               The session queue handle.
 */
void SysLocalAPIManager::setActiveSessionQueue(QueueHandle sessionQueue)
{
    char envbuffer[MAX_QUEUE_NAME_LENGTH+1];
    // set this as an environment variable for programs we call
    snprintf(envbuffer, sizeof(envbuffer), "%p", (void *)sessionQueue);
    setenv("RXQUEUESESSION", envbuffer, 1); // overwrite the old value
}


/**
 * Create a new connection instance of the appropiate type for
 * connection to the daemon process.
 *
 * @return A connection instance.
 */
ApiConnection *SysLocalAPIManager::newClientConnection()
{
    SysLocalSocketConnection *connection = new SysLocalSocketConnection();

    // open the pipe to the server
    if (!connection->connect(SysServerLocalSocketConnectionManager::generateServiceName()))
    {
        // don't leak memory!
        delete connection;
        throw new ServiceException(CONNECTION_FAILURE, "Failure connecting to rxapi server");
    }
    return connection;
}

