/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/******************************************************************************/
/* REXX Kernel                                                  okstart.c     */
/*                                                                            */
/* Startup                                                                    */
/*                                                                            */
/******************************************************************************/
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include "RexxCore.h"
#include "RexxMemory.hpp"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "MethodClass.hpp"
#include "RexxNativeAPI.h"
#include "StackClass.hpp"
#if defined(AIX) || defined(LINUX)
#include <limits.h>
#include <unistd.h>
#include "APIDefinitions.h"

#define CCHMAXPATH PATH_MAX+1
#endif

extern BOOL  ProcessDoneInit;          /* initialization is done            */
extern BOOL  ProcessColdStart;         /* we're coldstarting this           */
extern BOOL  ProcessDoneTerm;          /* termination is done               */
extern BOOL  ProcessFirstThread;       /* first (and primary thread)        */

extern SEV   RexxTerminated;           /* Termination complete semaphore.   */
extern RexxInteger *ProcessName;

#if defined(AIX) || defined(LINUX)
char achRexxCurDir[ CCHMAXPATH+2 ];          /* Save current working direct */
extern int thread_counter;
extern int  SecureFlag;
#endif

void kernelShutdown (void)
/******************************************************************************/
/* Shutdown OREXX System for this process                                     */
/******************************************************************************/
{
  MTXRQ(start_semaphore);              /* serialize startup/shutdown        */
  SysTermination();                    /* cleanup                           */
  EVPOST(RexxTerminated);              /* let anyone who cares know we're done*/
//  memoryObject.dumpMemoryProfile();    /* optionally dump memory stats      */
  if (!ProcessDoneTerm) {              /* if first time through             */
    ProcessDoneTerm = TRUE;            /* don't allow a "reterm"            */
    ProcessDoneInit = FALSE;           /* no longer initialized.            */
    ProcessColdStart = TRUE;           /* next one is a cold start          */
    ProcessFirstThread = TRUE;         /* first thread needs to be created  */
    memoryObject.freePools();          /* release access to memoryPools     */
  }
#ifdef SHARED
#if defined(AIX) || defined(LINUX)
    MTXRL(initialize_sem);
#else
    MTXRL(start_semaphore);              /* serialize startup/shutdown        */
#endif
#endif
}


extern BOOL ProcessSaveImage;


int REXXENTRY RexxTerminate (void)
/******************************************************************************/
/* Function:  Terminate the REXX interpreter...will only terminate if the     */
/*            call nesting level has reached zero.                            */
/******************************************************************************/
{
    // terminate one instance and shutdown, if necessary
    ActivityManager::terminateInterpreter();
    return 0;
}

BOOL REXXENTRY RexxInitialize (void)
/******************************************************************************/
/* Function:  Perform main kernel initializations                             */
/******************************************************************************/
{
  BOOL result;                         /* initialization result             */

#if defined(AIX) || defined(LINUX)
  LONG lRC;                            /* Return Code                       */
  if (!getcwd(achRexxCurDir, CCHMAXPATH))    /* Save current working direct */
  {
    strncpy( achRexxCurDir, getenv("PWD"), CCHMAXPATH);
    achRexxCurDir[CCHMAXPATH - 1] = '\0';
    if (achRexxCurDir[0] != '/' )
    {
      fprintf(stderr," *** ERROR: No current working directory for REXX!\n");
      exit(-1);                              /* all done ERROR end          */
    }
    else
      lRC = RxAPIHOMEset();            /* Set the REXX HOME                 */
  }
  lRC = RxAPIHOMEset();                /* Set the REXX HOME                 */

  if ( lRC )
  {
    fprintf(stderr," *** ERROR: No HOME or RXHOME directory for REXX!\n");
    exit(-1);                                /* all done ERROR end          */
  }
#endif

  setbuf(stdout,NULL);                 /* No buffering                      */
  setbuf(stderr,NULL);

#if defined(AIX) || defined(LINUX)
#ifdef THREADS
  if (!thread_counter)                 /*thread_counter is global defined    */
  {                                    /* and reset in RexxWaitForTermination */
    thread_counter++;
    SysThreadInit();                   /* do thread initialization          */
  }
#else
  OryxThreadInit();                    /* do thread initialization          */
#endif
#else
#ifdef THREADS
  SysThreadInit();                     /* do thread initialization          */
#else
  OryxThreadInit();                    /* do thread initialization          */
#endif
#endif

  SysEnterCriticalSection();

  result = ProcessFirstThread;         /* check on the first thread         */

  // perform activity manager startup for another instance
  ActivityManager::createInterpreter();
                                       /* make sure we can't loose control  */
                                       /* durecing creation/opening of THIS */
                                       /* MUTEX.                            */
  MTXCROPEN(start_semaphore, "OBJREXXSTARTSEM");  /* get the start semaphore           */
  SysExitCriticalSection();
  MTXRQ(start_semaphore);              /* lock the startup                  */

  if (ProcessFirstThread) {            /* if the first time                 */
    ProcessFirstThread = FALSE;        /* this is the first thread          */
    MTXCROPEN(resource_semaphore, "OBJREXXRESSEM");         /* create or open the other          */
    ActivityManager::createKernelLock();
#ifdef FIXEDTIMERS
    EVCROPEN(rexxTimeSliceSemaphore, "OBJREXXTSSEM");      // originally EVOPEN
#endif
    EVCR(RexxTerminated);              /* Create the terminated semaphore   */
    EVSET(RexxTerminated);             /* make sure Semaphore is UnPosted   */
#if defined(AIX) || defined(LINUX)
    SecureFlag = 1;
#endif
    ProcessDoneInit = FALSE;           /* allow for restart :               */
    ProcessDoneTerm = FALSE;           /* allow for restart :               */
    memoryObject.accessPools();        /* Gain access to memory Pools       */
    /* now that we have the shared memory, we can create and */
    /* use semaphores (prereq for AIX, though not for OS/2)  */
    /* (with one exception: startsem, which seems to be      */
    /* needed to serialize the shared memory setup)          */
    SysInitialize();                   /* perform other system init work    */

    if (ProcessSaveImage)              /* need to create the image?         */
    {
        RexxMemory::createImage();     /* go create the image               */
    }
    else {
      RexxMemory::restore();           // go restore the state of the memory object
      ActivityManager::startup();      // go create the local enviroment.
    }
    ProcessDoneInit = TRUE;            /* we're now initialized             */
  }                                    /* end of serialized block of code   */
  return result;                       /* all done                          */
}

BOOL REXXENTRY RexxQuery (void)
/******************************************************************************/
/* Function:  Determine if the REXX interpreter is initialized and active     */
/******************************************************************************/
{
  return ProcessDoneInit;              /* just check the doneinit flag      */
}
