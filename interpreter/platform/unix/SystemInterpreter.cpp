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

#if defined( HAVE_SIGNAL_H )
# include <signal.h>
#endif

#if defined( HAVE_SYS_SIGNAL_H )
# include <sys/signal.h>
#endif

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
#if defined( HAVE_SIGPROCMASK )
        sigemptyset( &newmask );
        sigaddset( &newmask, SIGINT );
        sigaddset( &newmask, SIGTERM );
        sigaddset( &newmask, SIGILL );
        sigaddset( &newmask, SIGSEGV );
        sigprocmask( SIG_BLOCK, &newmask , &oldmask );
#elif defined( HAVE_SIGHOLD )
        sighold(SIGINT);
        sighold(SIGTERM);
        sighold(SIGILL);
        sighold(SIGSEGV);
#endif

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
        Interpreter::haltAllActivities();
#if defined( HAVE_SIGPROCMASK )
        sigprocmask( SIG_SETMASK, &oldmask , NULL );
#elif defined( HAVE_SIGHOLD )
        sigrelse(SIGINT);
        sigrelse(SIGTERM);
        sigrelse(SIGILL);
        sigrelse(SIGSEGV);
#endif
        return;
    }
    else
    {
#if defined( HAVE_SIGPROCMASK )
        sigprocmask( SIG_SETMASK, &oldmask , NULL );
#elif defined( HAVE_SIGHOLD )
        sigrelse(SIGINT);
        sigrelse(SIGTERM);
        sigrelse(SIGILL);
        sigrelse(SIGSEGV);
#endif
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
    new_action.sa_flags = SA_RESTART;

/* Termination signals are set by Object REXX whenever the signals were not set */
/* from outside (calling C-routine). The SIGSEGV signal is not set any more, so */
/* that we now get a coredump instead of a hang up                              */

    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler == NULL)           /* not set by ext. exit handler*/
    {
        sigaction(SIGINT, &new_action, NULL);  /* exitClear on SIGTERM signal     */
    }
}


void SystemInterpreter::terminateInterpreter()
{
}



void SystemInterpreter::live(size_t liveMark)
{
}

void SystemInterpreter::liveGeneral(int reason)
{
  if (!memoryObject.savingImage())
  {
  }
}


/**
 * Get the current working directory for the process.
 *
 * @return The current working directory as a Rexx string.
 */
void SystemInterpreter::getCurrentWorkingDirectory(char *buf)
{
    if (!getcwd(buf, PATH_MAX)) /* Get current working direct */
    {
       strncpy(buf, getenv("PWD"), PATH_MAX);
       // if we don't result in a real directory here, make it a null string.
       if (buf[0] != '/' )
       {
           buf[0] = '\0';
       }
    }
}
