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

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include "APIServer.hpp"
#include "stdio.h"

// Add signal handler for SIGTERM
#define ENABLE_SIGTERM

// For testing purposes comment out the following line to force RXAPI to
// run as a foreground process.
#define RUN_AS_DAEMON

#ifdef RUN_AS_DAEMON
#define OOREXX_PIDFILE "/var/run/ooRexx.pid"
bool run_as_daemon = true;
#else
#define OOREXX_PIDFILE "/tmp/ooRexx.pid"
bool run_as_daemon = false;
#endif

APIServer apiServer;             // the real server instance


/*==========================================================================*
 *  Function: Run
 *
 *  Purpose:
 *
 *  handles the original RXAPI functions.
 *    Perform the message loop
 *
 *
 *==========================================================================*/
void Run (bool asService)
{
    try
    {
        apiServer.initServer();               // start up the server
        apiServer.listenForConnections();     // go into the message loop
    }
    catch (ServiceException *)
    {
    }
    apiServer.terminateServer();     // shut everything down
}

#ifdef ENABLE_SIGTERM
/*==========================================================================*
 *  Function: Stop
 *
 *  Purpose:
 *
 *  handles the stop request.
 *
 *
 *==========================================================================*/
void Stop(int signo)
{
    apiServer.terminateServer();     // shut everything down

    exit(1);
}
#endif

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// Routines to run RXAPI as an daemon BEGIN


/*==========================================================================*
 *  Function: morph2daemon
 *
 *  Purpose:
 *
 *  Turn this process into a daemon.
 *
 * Returns TRUE if a daemon, FALSE if not or an error
 *
 *==========================================================================*/
static bool morph2daemon(void)
{
    char pid_buf[256];

    if (run_as_daemon == false) {
        return true; // go ahead and run in the foreground
    }

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		return false;
	}

	pid_t pid = fork();
	if (pid < 0) {
		return false;
	}
    // if we are the parent process then we are done
	if (pid != 0) {
		exit( 0 );
	}

    // become the session leader
	setsid();
    // second fork to become a real daemon
	pid = fork();
	if (pid < 0) {
		return false;
	}
    // if we are the parent process then we are done
	if (pid != 0) {
		exit(0);
	}

    // create the pid file (overwrite of old pid file is ok)
    unlink(OOREXX_PIDFILE);
    int pfile = open(OOREXX_PIDFILE, O_WRONLY | O_CREAT, 0640);
    snprintf(pid_buf, sizeof(pid_buf), "%d\n", (int)getpid());
    write(pfile, pid_buf, strlen(pid_buf));
    close(pfile);

    // housekeeping
	chdir("/");
	umask(0);
	for(int i = 0; i < 1024; i++) {
		close(i);
	}

    // We start out with root privileges. This is bad from a security perspective. So
    // switch to the nobody user so we do not have previleges we do not need.
    struct passwd *pw;
    pw = getpwnam("nobody");
    if (pw != NULL) {
        setuid(pw->pw_uid);
    }

	return true;
}


/*==========================================================================*
 *  Function: main
 *
 *  Purpose:
 *
 *  Main entry point.
 *
 *==========================================================================*/
int main(int argc, char *argv[])
{
    char pid_buf[256];
    int pfile, len;
    pid_t pid = 0;
#if defined(AIX)
    struct stat st;
#endif
#ifdef ENABLE_SIGTERM
    struct sigaction sa;
#endif
    // Get the command line args
    if (argc > 1) {
        printf("Error: Invalid command line option(s).\n");
        printf("       Aborting execution.\n\n");
        return -1;
    }

    // see if we are already running
    if ((pfile = open(OOREXX_PIDFILE, O_RDONLY)) > 0) {
            len = read(pfile, pid_buf, sizeof(pid_buf) - 1);
            close(pfile);
            pid_buf[len] = '\0';
            pid = (pid_t)atoi(pid_buf);
            if (pid && (pid == getpid() || kill(pid, 0) < 0)) {
                    unlink(OOREXX_PIDFILE);
            } else {
                    // there is already a server running
                    printf("Error: There is already a server running.\n");
                    printf("       Aborting execution.\n");
                    return -1;
            }
    }

    // write the pid file
    pfile = open(OOREXX_PIDFILE, O_WRONLY | O_CREAT,
                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (pfile == -1) {
            // cannot open pid file
            printf("Error: Cannot open PID file %s.\n", OOREXX_PIDFILE);
            printf("       Aborting execution.\n\n");
            return -1;
    }
    snprintf(pid_buf, sizeof(pid_buf), "%d\n", (int)getpid());
    write(pfile, pid_buf, strlen(pid_buf));
    close(pfile);

    // make ourselves a daemon
    // - if this is AIX we check if the rxapi daemon was sarted via SRC 
    //   - if the daemon was started via SRC we do not morph - the SRC handles this
    //
    // - add to AIX SRC without auto restart:
    //   mkssys -s rxapi -p /opt/ooRexx/bin/rxapi -i /dev/null -e /dev/console \
    //          -o /dev/console -u 0 -S -n 15 -f 9 -O -Q
    //
    // - add to AIX SRC with auto restart:
    //   mkssys -s rxapi -p /opt/ooRexx/bin/rxapi -i /dev/null -e /dev/console \
    //          -o /dev/console -u 0 -S -n 15 -f 9 -R -Q
#if defined(AIX)
    if (fstat(0, &st) <0) {
        if (morph2daemon() == false) {
            return -1;
        }
    } else {
        if ((st.st_mode & S_IFMT) == S_IFCHR) {
            if (isatty(0)) {
                if (morph2daemon() == false) {
                    return -1;
                }
            }
        } else {
            if (morph2daemon() == false) {
                return -1;
            }
        }
    }
#else
        if (morph2daemon() == false) {
            return -1;
        }
#endif

    // run the server
#if defined(AIX)
    if (run_as_daemon == false) {
        printf("Starting request processing loop.\n");
    } else {
        (void) setsid();

        // We start out with root privileges. This is bad from a security perspective. So
        // switch to the nobody user so we do not have previleges we do not need.
        struct passwd *pw;
        pw = getpwnam("nobody");
        if (pw != NULL) {
            setuid(pw->pw_uid);
        }
    }
#else
    if (run_as_daemon == false) {
        printf("Starting request processing loop.\n");
    }
#endif

#ifdef ENABLE_SIGTERM
    // handle kill -15
    (void) sigemptyset(&sa.sa_mask);
    (void) sigaddset(&sa.sa_mask, SIGTERM);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = Stop;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        exit(1);
    }
#endif

    Run(false);

    return 0;
}

