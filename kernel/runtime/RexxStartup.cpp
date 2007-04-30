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
#include "MethodClass.hpp"
#include "RexxNativeAPI.h"
#include "ActivityTable.hpp"
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

#ifdef HIGHTID
extern ActivityTable *ProcessLocalActs;    /* list of process activities    */
#else
extern RexxArray *ProcessLocalActs;    /* list of process activities        */
#endif

extern BOOL  ProcessRestoreImage;      /* we're restoring the image         */
extern RexxDirectory *ProcessLocalEnv; /* process local environment (.local)*/
extern RexxObject * ProcessLocalServer;
extern int   ProcessInitializations;   /* number of nested initializations  */
extern BOOL  ProcessTerminating;       /* termination has started           */
extern int   ProcessNumActs;           /* number of activities              */
extern SEV   RexxTerminated;           /* Termination complete semaphore.   */
extern SEV   RexxServerWait;           /* Termination complete semaphore.   */
extern RexxInteger *ProcessName;
                                       /* Local and global memory Pools     */
                                       /*  last one accessed.               */
extern MemorySegmentPool *ProcessCurrentPool;
extern MemorySegmentPool *GlobalCurrentPool;

#if defined(AIX) || defined(LINUX)
char achRexxCurDir[ CCHMAXPATH+2 ];          /* Save current working direct */
extern int thread_counter;
extern int  SecureFlag;
#endif

void   activity_startup(void);
void   createImage(void);

#ifdef SOM
  #include <som.xh>
#endif

void kernelShutdown (void)
/******************************************************************************/
/* Shutdown OREXX System for this process                                     */
/******************************************************************************/
{
#ifdef SHARED
#if defined(AIX) || defined(LINUX)
    MTXRQ(initialize_sem);
#else
  MTXRQ(start_semaphore);              /* serialize startup/shutdown        */
#endif
#endif
  SysTermination();                    /* cleanup                           */
  EVPOST(RexxTerminated);              /* let anyone who cares know we're done*/
//  memoryObject.dumpMemoryProfile();    /* optionally dump memory stats      */
  if (!ProcessDoneTerm) {              /* if first time through             */
    ProcessDoneTerm = TRUE;            /* don't allow a "reterm"            */
    ProcessDoneInit = FALSE;           /* no longer initialized.            */
    ProcessTerminating = FALSE;        /* no longer terminating             */
    ProcessInitializations = 0;        /* no initializations done           */
    ProcessColdStart = TRUE;           /* next one is a cold start          */
    ProcessFirstThread = TRUE;         /* first thread needs to be created  */
    ProcessLocalActs = OREF_NULL;      /* no local activities               */
    ProcessRestoreImage = TRUE;        /* and we have to restore the image  */
    if (ProcessLocalEnv != OREF_NULL)  /* have a local environment directory*/
      discard_hold(ProcessLocalEnv);   /* we can now release this           */
    ProcessLocalEnv = OREF_NULL;       /* no local environment              */
#ifndef OS2REXX                        /* This is a severe problem on OS2.  REXX is started from cmd.exe as a */
                                       /* subprocess, which means that memory is not released at program end */
                                       /* only on shutdown. So if there is (for huge files) a linked list of */
                                       /* memory pools allocated, the newly created pools would be freed here*/
                                       /* exept the base pool, BUT the linked lists of f.e Dead Objects swept*/
                                       /* segments etc, are still alive. I have tried to relinke those Object*/
                                       /* s but without success. So i decided not to free the pools on OS/2. */
                                       /* Be aware of possible resource problems especially when the interpre*/
                                       /* is started in a loop with REXXSTART                                */
    memoryObject.freePools();          /* release access to memoryPools     */
#endif
    ProcessName = OREF_NULL;           /* no process name now               */
#if defined(AIX) || defined(LINUX)
    EVCLOSE(RexxServerWait);           /* and clear the semaphore           */
#else
    EVCL(RexxServerWait);              /* and clear the semaphore           */
#endif
  }
#ifdef SHARED
#if defined(AIX) || defined(LINUX)
    MTXRL(initialize_sem);
#else
    MTXRL(start_semaphore);              /* serialize startup/shutdown        */
#endif
#endif
}

void kernelTerminate (int terminateType)
/******************************************************************************/
/* Terminate thread and possibly shutdown system for process                  */
/******************************************************************************/
{
                                       /* Let any pending threads from      */
                                       /* a start or reply, know to         */
                                       /* Terminate when finished           */
  ProcessTerminating = TRUE;
  if (ProcessNumActs == 0)             /* any active activities?            */
    kernelShutdown();                  /* shut everything down              */
                                       /* any active activities?            */
  else {
    MTXRQ(kernel_semaphore);           /* get the kernel semophore          */
                                       /* Make sure we wake up server       */
                                       /* Make sure all free Activities     */
                                       /*  get the terminate message        */
                                       /* done after uninit calls. incas    */
                                       /*  uninits needed some.             */
    TheActivityClass->terminateFreeActs();
    EVPOST(RexxServerWait);            /*message wait thread.               */
    MTXRL(kernel_semaphore);           /* Done with Kernel stuff            */
  }
}

void kernelRestore (void);
void kernelNewProcess (void);
RexxString * kernel_name (char* value);

extern BOOL ProcessSaveImage;

void restoreImage(void)
/******************************************************************************/
/* Main startup routine for OREXX                                             */
/******************************************************************************/
{
  activity_lock_kernel();              /* lock the kernel                   */
  kernelRestore();                     /* initialize the kernel             */
  activity_unlock_kernel();            /* lock the kernel                   */
}

void start_rexx_environment(void)
/******************************************************************************/
/* Main startup routine for OREXX                                             */
/******************************************************************************/
{
  RexxObject *server_class;            /* server class object               */

  activity_lock_kernel();              /* lock the kernel                   */
  kernelNewProcess();                  /* do new process initialization     */
  if (ProcessLocalEnv == OREF_NULL)    /* need a new local environment?     */
                                       /* get the local environment         */
    ProcessLocalEnv = (RexxDirectory *)save(new_directory());
                                       /* get the server class              */
  server_class = env_find(new_cstring("!SERVER"));
  activity_unlock_kernel();            /* now unlock the kernel             */

  TheActivityClass->getActivity();     /* get an activity set up            */
                                       /* create a new server object        */
  CurrentActivity->messageSend(server_class, OREF_NEW, 0, OREF_NULL, &ProcessLocalServer);
                                       /* now release this activity         */
  TheActivityClass->returnActivity(CurrentActivity);
  ProcessRestoreImage = FALSE;         /* Turn off restore image flag.      */
}

int REXXENTRY RexxTerminate (void)
/******************************************************************************/
/* Function:  Terminate the REXX interpreter...will only terminate if the     */
/*            call nesting level has reached zero.                            */
/******************************************************************************/
{
  SysEnterCriticalSection();
  ProcessInitializations--;            /* reduce the active count           */
  if (ProcessInitializations == 0)     /* down to nothing?                  */
  {
                                       /* force termination                 */
    kernelTerminate(NORMAL_TERMINATION);
    /* semaphores are closed when process detaches and not when  */
    /* RexxStart finishes. This allows asynchronous threads to continue in  */
    /* the background. Handles will not increase because DllMain will be    */
    /* called when the process finishes.                                    */
  }
  SysExitCriticalSection();
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

#ifndef SHARED
/* Be sure if REXX threads run in parallel, that initialization semaphore is active ==> SysThreadInit must be done */
   while (!initialize_sem)
   {
     SysThreadYield();
   }
#endif

  SysEnterCriticalSection();

  result = ProcessFirstThread;         /* check on the first thread         */
  /* active count needs to be protected */
  ProcessInitializations++;            /* bump the active count             */
                                       /* make sure we can't loose control  */
                                       /* durecing creation/opening of THIS */
                                       /* MUTEX.                            */
  MTXCROPEN(start_semaphore, "OBJREXXSTARTSEM");              /* get the start semaphore           */
  SysExitCriticalSection();
  MTXRQ(start_semaphore);              /* lock the startup                  */

  if (ProcessFirstThread) {            /* if the first time                 */
    ProcessFirstThread = FALSE;        /* this is the first thread          */
    MTXCROPEN(resource_semaphore, "OBJREXXRESSEM");         /* create or open the other          */
    MTXCROPEN(kernel_semaphore, "OBJREXXKERNELSEM");           /* semaphores                        */
#ifdef FIXEDTIMERS
    EVCROPEN(rexxTimeSliceSemaphore, "OBJREXXTSSEM");      // originally EVOPEN
#endif
    EVCR(RexxTerminated);              /* Create the terminated semaphore   */
    EVSET(RexxTerminated);             /* make sure Semaphore is UnPosted   */
#if defined(AIX) || defined(LINUX)
    SecureFlag = 1;
#endif
    EVCR(RexxServerWait);              /* Create the ServerWait semaphore   */
    EVSET(RexxServerWait);             /* make sure ServerWait is UnPosted  */
    ProcessDoneInit = FALSE;           /* allow for restart :               */
    ProcessDoneTerm = FALSE;           /* allow for restart :               */
    memoryObject.accessPools();        /* Gain access to memory Pools       */
    /* now that we have the shared memory, we can create and */
    /* use semaphores (prereq for AIX, though not for OS/2)  */
    /* (with one exception: startsem, which seems to be      */
    /* needed to serialize the shared memory setup)          */
    SysInitialize();                   /* perform other system init work    */

    if (ProcessSaveImage)              /* need to create the image?         */
      createImage();                   /* go create the image               */
    else {
      if (ProcessRestoreImage) {       /* Does image need to be restored    */
        restoreImage();                /*Yes, restore the image             */
      }
      start_rexx_environment();        /* call main initialization routine. */
    }
    ProcessDoneInit = TRUE;            /* we're now initialized             */
  }                                    /* end of serialized block of code   */
  /* we need to extend the savestack because of concurrency */
  else
  {
      /* no memory section needed here */
      memoryObject.extendSaveStack(ProcessInitializations-1);
  }
#ifdef SHARED
  MTXRL(start_semaphore);              /* release the startup semaphore     */
#endif


  return result;                       /* all done                          */
}

BOOL REXXENTRY RexxQuery (void)
/******************************************************************************/
/* Function:  Determine if the REXX interpreter is initialized and active     */
/******************************************************************************/
{
  return ProcessDoneInit;              /* just check the doneinit flag      */
}
