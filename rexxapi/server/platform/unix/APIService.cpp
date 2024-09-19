/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2023 Rexx Language Association. All rights reserved.    */
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>              // lockf()
#include <sys/types.h>
#include <sys/stat.h>            // umask()
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <errno.h>
#include "APIServer.hpp"
#include "SysCSStream.hpp"

APIServer apiServer;             // the real server instance



/**
 * Handle a server stop request
 *
 * @param signo The signal number (not used)
 */
void Stop(int signo)
{
    apiServer.terminateServer();     // shut everything down

    exit(1);
}


/**
 * Acquire a lock on a special file to ensure we are the only running
 * rxapi process for this user.
 *
 * @return The file descriptor for the lock file or -1 for any failure obtaining
 *         the lock.
 */
int acquireLock (const char *lockFileName)
{
    int lockFd;

    if ((lockFd = open(lockFileName, O_CREAT | O_RDWR, 0600))  < 0)
    {
        return -1;
    }

    // locks for exclusive use if another process has not already locked
    // if already locked by another process, returns -1
    if (lockf(lockFd, F_TLOCK, 0) < 0)
    {
        close(lockFd);
        return -1;
    }

    return lockFd;
}

/**
 * Release the lock on the lock file.
 *
 * @param lockFileName
 *               The name of the lock file.
 * @param lockFd The file descriptor of the file.
 */
void releaseLock (const char *lockFileName, int lockFd)
{
    int ignore; // avoid warning: ignoring return value of 'int lockf(int, int, __off_t)

    ignore = lockf(lockFd, F_ULOCK, 0);
    close(lockFd);
    unlink(lockFileName);
}


/**
 * The rxapi main entry point.
 *
 * @param argc   The command line arguments
 * @param argv   The arguments
 *
 * @return The completion return code.
 */
int main(int argc, char *argv[])
{
    bool dryrun = false; // handy (undocumented) test option
    if (argc > 1)
    {
        dryrun = strcmp(argv[1], "--dryrun") == 0;
        printf(dryrun ? "rxapi: dryrun\n" : "rxapi: no args allowed\n");
    }

    // neither our lock file nor our socket should give access to group or world
    umask(077);

    // capture SIGTERM (kill -15) and gracefully terminate when received
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = Stop;
    if (sigaction(SIGTERM, &sa, NULL) == -1)
    {
        printf("rxapi: sigaction(SIGTERM) failed with errno %d %s\n", errno, strerror(errno));
    }

    // a buffer for generating the name
    char lockFileName[PATH_MAX + 100];

    // the location of the lock file
    char servicePath[PATH_MAX];
    // determine the best place to put this
    SysServerLocalSocketConnectionManager::getServiceLocation(servicePath, sizeof(servicePath));
    snprintf(lockFileName, sizeof(lockFileName), "%s.lock", servicePath);
    printf("rxapi: lockfile path is %s\n", lockFileName);

    // see if we can get the lock file before proceeding. This is one
    // file per user.
    int fd;
    if ((fd = acquireLock(lockFileName)) == -1)
    {
        printf("rxapi: lockfile is locked by another rxapi instance; exiting\n");
        return EACCES;
    }
    printf("rxapi: lockfile lock acquired\n");

    try
    {
        // create a connection object that will be the server target
        SysServerLocalSocketConnectionManager *c = new SysServerLocalSocketConnectionManager();
        // try to create the Unix socket used for this server. If this fails, we
        // likely have an instance of the daemon already running, so just fail quietly.
        const char *service = SysServerLocalSocketConnectionManager::generateServiceName();
        printf("rxapi: service path is %s\n", service);
        if (!c->bind(service))
        {
            printf("rxapi: service is locked by another rxapi instance; exiting\n");
            delete c;
            return EACCES;
        }
        apiServer.initServer(c);              // start up the server
        printf("rxapi: service successfully started; listening\n");
        if (!dryrun)
        {
            apiServer.listenForConnections(); // go into the message loop
        }
    }
    catch (ServiceException *e)
    {
        delete e;  // just ignore errors
    }
    apiServer.terminateServer();     // shut everything down
    releaseLock(lockFileName, fd);   // release the exclusive lock
    printf("rxapi: service stopped and lockfile released; exiting\n");

    return 0;
}

