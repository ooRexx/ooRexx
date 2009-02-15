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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "SysLocalAPIManager.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>


void SysLocalAPIManager::startServerProcess()
{
    char apiExeName[] = "rxapi";

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		return;
	}

	pid_t pid = fork();
	if (pid < 0) {
        throw new ServiceException(API_FAILURE, "Unable to start API server");
	}
	if (pid != 0) {
        // we are the parent process
		return;
	}
    // if we get here we are the child process

    // become the session leader
	setsid();

    // housekeeping - chdir to the root subdir and close all open files
	chdir("/");
	umask(0);
	for(int i = 0; i < 1024; i++) {
		close(i);
	}

    // now start rxapi
    if (execvp(apiExeName, NULL) == -1) {
        throw new ServiceException(API_FAILURE, "Unable to start API server");
    }

	return;
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
        sscanf(envbuffer, "%p", (char **)&sessionQueue);
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
    sprintf(envbuffer, "%p", (void *)sessionQueue);
    setenv("RXQUEUESESSION", envbuffer, 1); // overwrite the old value
}

