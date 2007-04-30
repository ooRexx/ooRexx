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
/* REXX AIX Support                                             aixmisc.c     */
/*                                                                            */
/* Miscellaneous AIX specific routines.                                       */
/*                                                                            */
/******************************************************************************/

/*********************************************************************/
/*                                                                   */
/*   Function:  Miscellaneous system specific routines               */
/*                                                                   */
/*********************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "RexxCore.h"
#include "StringClass.hpp"
// #include "RexxNativeAPI.h"                      /*  THUTHUREXX native interface*/
#include "DirectoryClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "ThreadSupport.hpp"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "ActivityTable.hpp"

#if defined( HAVE_SIGNAL_H )
# include <signal.h>
#endif

#if defined( HAVE_SYS_SIGNAL_H )
# include <sys/signal.h>
#endif

#if defined( HAVE_SYS_LDR_H )
# include <sys/ldr.h>
#endif

#if defined( HAVE_FILEHDR_H )
# include <filehdr.h>
#endif

#include <dlfcn.h>

#if defined( HAVE_SYS_UTSNAME_H )
# include <sys/utsname.h>               /* get the uname() function   */
#endif

void RxExitClear(int);
void RxExitClearNormal();


extern int  ProcessNumActs;
extern int  ProcessTerminating;
extern BOOL bProcessExitInitFlag;

extern ULONG mustCompleteNest;         /* Global variable for MustComplete  */
extern RexxActivity *CurrentActivity;  /* expose current activity object    */
                                       /* default active settings           */
extern ACTIVATION_SETTINGS *current_settings;

#ifdef HIGHTID
extern ActivityTable *ProcessLocalActs;
#else
extern RexxArray *ProcessLocalActs;
#endif

UINT iClauseCounter=0;                      // count of clauses
UINT iTransClauseCounter=0;                 // count of clauses in translator
#define LOADED_OBJECTS 100

RexxObject *SysProcessName( void )
/******************************************************************************/
/* Function:  Get a referenceable name for this process                       */
/******************************************************************************/
{
  return new_integer((long)getpid());
}

void SysTermination(void)
/******************************************************************************/
/* Function:   Perform system specific termination.                           */
/******************************************************************************/
{
//                                       /* remove the exit list routine      */
// DosExitList(EXLST_REMOVE,(PFNEXITLIST)exit_handler);
    if (ProcessLocalActs != OREF_NULL)
       delete ProcessLocalActs;
}

void SysInitialize(void)
/******************************************************************************/
/* Function:   Perform system specific initialization.                        */
/******************************************************************************/
{
//   DosExitList(EXLST_ADD,(PFNEXITLIST)exit_handler);
//SysThreadInit();             /* initialize the main thread of the main process*/


/* this is for normal process termination                                     */
  if (bProcessExitInitFlag == FALSE)
  {
     bProcessExitInitFlag = TRUE;
     atexit(RxExitClearNormal);
    /* Set the cleanup handler for unconditional process termination          */
    struct sigaction new_action;
    struct sigaction old_action;

    /* Set up the structure to specify the new action                         */
    new_action.sa_handler = RxExitClear;
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
}

RexxString *SysVersion(void)
/******************************************************************************/
/* Function:   Return the system specific version identifier that is stored   */
/*             in the image.                                                  */
/******************************************************************************/
{
  struct utsname info;                 /* info structur              */

  uname(&info);                        /* get the info               */

  return new_cstring(info.release);    /* return as a string                */
}

PFN SysLoadProcedure(
  RexxInteger * LibraryHandle,         /* library load handle               */
  RexxString  * Procedure)             /* required procedure name           */
/******************************************************************************/
/* Function:  Resolve a named procedure in a library                          */
/******************************************************************************/
{
 //  return (PFN) dld_get_func(Procedure->stringData);
   PFN load_address;
   load_address = dlsym((PVOID) LibraryHandle->value, Procedure->stringData);
   if (!load_address)
   {
      report_exception1(Error_External_name_not_found_method, Procedure);
   }
   else
   {
      return load_address;
   }
}

RexxInteger * SysLoadLibrary(
     RexxString * Library)             /* required library name             */
/******************************************************************************/
/* Function:  Load a named library, returning the library handle              */
/******************************************************************************/
{
  RexxString *result;
  RexxString *tempresult;
  LONG plib;

  result = (RexxString*) new_cstring("lib");
  result = result->concatWithCstring(Library->stringData);
  result = result->concatWithCstring(ORX_SHARED_LIBRARY_EXT);
  tempresult = (RexxString *)result->copy();

  if (!(plib = (LONG) dlopen(result->stringData, RTLD_LAZY )))
  {
//     result = result->concatToCstring("/usr/lib/");

     if (!(plib = (LONG) dlopen(result->stringData, RTLD_LAZY )))
     {
        fprintf(stderr, " *** Error dlopen: %s\n", dlerror());
        report_exception1(Error_Execution_library, tempresult);
     }
  }
  return new_integer((LONG)plib);
}

#define MAX_ADDRESS_NAME_LENGTH  250   /* maximum command environment name  */

void SysValidateAddressName(
  RexxString *Name )                   /* name to validate                  */
/******************************************************************************/
/* Function:  Validate an external address name                               */
/******************************************************************************/
{
                                       /* name too long?                    */
  if (Name->length > MAX_ADDRESS_NAME_LENGTH)
                                       /* go report an error                */
    report_exception2(Error_Environment_name_name, new_integer(MAX_ADDRESS_NAME_LENGTH), Name);
}

RexxString * SysGetCurrentQueue(void)
/******************************************************************************/
/* Function:  Return the current queue object name                            */
/******************************************************************************/
{
  RexxString * queue;                  /* current queue object              */
  RexxString * queue_name;             /* name of the queue object          */

                                       /* get the default queue             */
  queue = (RexxString *)CurrentActivity->local->at(OREF_REXXQUEUE);

  if (queue == OREF_NULL)              /* no queue?                         */
    queue_name = OREF_SESSION;         /* use the default name              */
  else
                                       /* get the actual queue name         */
    queue_name = (RexxString *)send_message0(queue, OREF_GET);
  return queue_name;                   /* return the name                   */
}

void SysSetupProgram(
  RexxActivation *activation)          /* current running program           */
/******************************************************************************/
/* Function:  Do system specific program setup                                */
/******************************************************************************/
{
#ifdef RXTRACE_SUPPORT
  PCHAR RxTraceBuf = NULL;
  LONG      Length;                    /* length of variable                */

                                       /* scan current environment,         */
/*if (GetEnvironmentVariable("RXTRACE", RxTraceBuf, 8)) {                   */
  RxTraceBuf = getenv("RXTRACE");
  if (RxTraceBuf)
  {
    if (!stricmp(RxTraceBuf, "ON"))    /* request to turn on?               */
                                       /* turn on tracing                   */
      activation->setTrace(TRACE_RESULTS, DEBUG_ON);
  }
#endif
}

RexxString * SysSourceString(
  RexxString * callType,               /* type of call token                */
  RexxString * programName )           /* program name token                */
/******************************************************************************/
/* Function:  Produce a system specific source string                         */
/******************************************************************************/
{
  RexxString * rsSysName;              // buffer for version

  RexxString * source_string;          /* final source string               */
  char       * outPtr;
  source_string = raw_string(1+sizeof(ORX_SYS_STR)+callType->length+programName->length);
  outPtr = source_string->stringData;  /* point to result Data.             */

  strcpy(outPtr, ORX_SYS_STR);          /* copy the system name              */
  outPtr +=sizeof(ORX_SYS_STR) - 1;     /* step past the name                */
  *outPtr++ = ' ';                     /* put a blank between               */
                                       /* copy the call type                */
  memcpy(outPtr, callType->stringData, callType->length);
  outPtr += callType->length;          /* step over the call type           */
  *outPtr++ = ' ';                     /* put a blank between               */
                                       /* copy the system name              */
  memcpy(outPtr, programName->stringData, programName->length);
  source_string->generateHash();       /* now create the hash value         */
  return source_string;                /* return the source string          */
}


void SysRegisterSignals(
  SYSEXCEPTIONBLOCK *exception_info)   /* system specific exception info    */
/******************************************************************************/
/* Function:   Establish exception handlers                                   */
/******************************************************************************/
{
#if !defined(AIX) && !defined(LINUX)
  PTIB   tibp;                         /* process information block         */
  PPIB   pibp;                         /* thread information block          */
  ULONG  NestingLevel;                 /* signal trap nesting level         */
                                       /* pointer to the exception record   */
  EXCEPTIONREGISTRATIONRECORD *exception_record;

                                       /* cast to the OS/2 block type       */
  exception_record = (EXCEPTIONREGISTRATIONRECORD *)exception_info;
  DosGetInfoBlocks(&tibp, &pibp);      /* get the process and thread blocks */
                                       /* establish the exception handler   */
  exception_record->ExceptionHandler = SysExceptionHandler;
                                       /* register the handler              */
  DosSetExceptionHandler(exception_info);
                                       /* running in a PM session?          */
  if (pibp->pib_ultype != SSF_TYPE_PM) {
                                       /* Nope setup Ctrl-C exception       */
    DosSetSignalExceptionFocus(SIG_SETFOCUS, &NestingLevel);
  }
#endif   // AIX and LINUX
}

void SysDeregisterSignals(
  SYSEXCEPTIONBLOCK *exception_info)   /* system specific exception info    */
/******************************************************************************/
/* Function:   Clear out registered exception handlers                        */
/******************************************************************************/
{
#if !defined(AIX) && !defined(LINUX)
  PTIB   tibp;                         /* process information block         */
  PPIB   pibp;                         /* thread information block          */
  ULONG  NestingLevel;                 /* signal trap nesting level         */

  DosGetInfoBlocks(&tibp, &pibp);      /* get the process and thread blocks */
                                       /* running in a PM session?          */
  if (pibp->pib_ultype != SSF_TYPE_PM)
                                       /* NOPE, reset Cntl-C trapping       */
    DosSetSignalExceptionFocus(SIG_UNSETFOCUS, &NestingLevel);
                                       /* remove the exception handler      */
  DosUnsetExceptionHandler(exception_info);
#endif  // AIX and LINUX
}

SYSWINDOWINFO *SysInitializeWindowEnv()
/******************************************************************************/
/* Function:  Initialize a PM thread for possible WIN message later.          */
/******************************************************************************/
{
  SYSWINDOWINFO *windowInfo = NULL;    /* PM specific thread info           */
#if !defined(AIX) && !defined(LINUX)
  PTIB   tibp;                         /* process information block         */
  PPIB   pibp;                         /* thread information block          */

  DosGetInfoBlocks(&tibp, &pibp);      /* get the process and thread blocks */
                                       /* running in a PM session?          */
  if (pibp->pib_ultype == SSF_TYPE_PM) {
                                       /* Yes, get a block for PM info      */
    windowInfo = (SYSWINDOWINFO *)malloc(sizeof(SYSWINDOWINFO));
                                       /* Initialize the thread for PM      */
    windowInfo->hAnchorBlock = WinInitialize(0);
                                       /* Did we get our HAB?               */
    if (windowInfo->hAnchorBlock) {
                                       /* Yes, make sure we do a WinTerminat*/
      windowInfo->mustTerminate = TRUE;
    }
    else {
      windowInfo->hAnchorBlock = WinQueryAnchorBlock(HWND_DESKTOP);
      windowInfo->mustTerminate = FALSE;
    }


                                       /* Create a message Queue for PM     */
    windowInfo->messageQueue = WinCreateMsgQueue(windowInfo->hAnchorBlock, 0);
                                       /* able to create a MessageQueue?    */
    if (windowInfo->messageQueue) {
                                       /* Make sure we cancel ShutdownMsg   */
                                       /* for this msgQueue. Avoid Shutudown*/
                                       /* hangs.                            */
      WinCancelShutdown(windowInfo->messageQueue, TRUE);
    }
#ifdef DEBUG
    else {                             /* Create failed, make see if error  */
                                       /* was that is already exists.       */
     if (PMERR_MSG_QUEUE_ALREADY_EXISTS != WinGetLastError(windowInfo->hAnchorBlock)) {
       printf("Error: could not create Message Queue for PM Thread id %u\n", tibp->tib_ptib2->tib2_ultid);
     }
    }
#endif
  }
#endif  // AIX and LINUX
return windowInfo;
}

void SysTerminateWindowEnv(SYSWINDOWINFO *windowInfo)
/******************************************************************************/
/* Function:  Initialize a PM thread for possible WIN message later.          */
/******************************************************************************/
{
#if !defined(AIX) && !defined(LINUX)
                                       /* Were we even passed an INfo Block?*/
  if (windowInfo) {
                                       /* Yes, Free up resources.           */
                                       /* Do we have our own MessageQueue?  */
    if (windowInfo->messageQueue)
                                       /* Yes, destory the messageQueue     */
      WinDestroyMsgQueue(windowInfo->messageQueue);
                                       /* Do we have a HAB to free?         */
    if (windowInfo->mustTerminate)
                                       /* Yes, Terminate our PM session     */
      WinTerminate(windowInfo->hAnchorBlock);
                                       /* now free the info block.          */
    free(windowInfo);
  }
#endif  // AIX and LINUX
}

void SysClauseBoundary(RexxActivation *stub)
{
}
