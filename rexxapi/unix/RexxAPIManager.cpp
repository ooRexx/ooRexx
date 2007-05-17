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
/**********************************************************************
**
** Module:      AIXRXAPI.C
**
** Description: Utility functions used by the REXXAPI
**              Dynalink Module
**
**********************************************************************/
/******************************************************************************/
/* Concerns about REXXAPI:                                                    */
/*                                                                            */
/* Problem: Memory leaks through reload (dlopen) of the same API library.     */
/*                                                                            */
/* After shared memory creation the first process registers all the functions */
/* from a library it wants to use.                                            */
/* This is the main registration of that library, which is kept as long as    */
/* the shared memory exists.                                                  */
/* All following processes reuse the first registraton of the functions from  */
/* the same library to load and store the handles of the library's functions. */
/* All the >cblocks< of the main registration get the  apiFunRegFlag  set to  */
/* >0< and all others get it set to >1< and are copied  >cblocks<  from  the  */
/* main registration.                                                         */
/* All the >cblocks< with  apiFunRegFlag  set to >1< will be deleted shortly  */
/* before the process ends which owns (via PID) the >cblocks<.                */
/* The first process which has registered the main function entries (cblocks) */
/* sets all handles and the PID to NULL shortly before it ends.               */
/* Now the main cblocks can be reused by other processes, if the handles and  */
/* PID are NULL.                                                              */
/* The RegDrop function must only set the apiFunRegFlag to >1< of the main    */
/* function entries (cblocks). This will prevent running REXX scripts from    */
/* abnormal termination, because the cblocks are deleted at the end of the    */
/* running REXX script. If a >running< script tries to register functions     */
/* again, it must not be possible. Only this way you can get rid of libraries */
/* and their registered functions. This is necessary for to be able of        */
/* loading new libraies with the same name ( for libraries development ).     */
/******************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define INCL_RXSUBCOM

#include "RexxCore.h"
#include <stdio.h>
#include <stdlib.h>
#include "RexxAPIManager.h"
#include "APIUtilities.h"
#include "SystemSemaphores.h"

#include "ActivityTable.hpp"
#include "RexxActivation.hpp"

#ifdef HIGHTID
extern ActivityTable *ProcessLocalActs;
#else
extern RexxArray *ProcessLocalActs;
#endif

#include "SharedMemorySupport.h"                  /* system shared memory       */
#include <string.h>                    /* to have the memset() func  */
#include "RexxNativeAPI.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>

#if defined( HAVE_SIGNAL_H )
# include <signal.h>
#endif

#if defined( HAVE_LIMITS_H )
# include <limits.h>
#endif

#if defined( HAVE_USERSEC_H )
# include <usersec.h>
#endif

#if defined( PATH_MAX )
# define CCHMAXPATH PATH_MAX+1
#elif defined( _POSIX_PATH_MAX )
# define CCHMAXPATH _POSIX_PATH_MAX+1
#else
# define CCHMAXPATH 512+1
#endif
INT  iSemShmMode = 384;              /* Set MODE 600 for sem and shm         */
char achRexxHomeDir[ CCHMAXPATH+2 ]; /* Save home dir at startup             */

extern int errno;
extern HEV  RexxTerminated;          /* Termination complete semaphore.      */
REXXAPIDATA  *apidata = NULL;

/* Save the caller for signal blocking to filter pairs for unblocking */
/* Pairs are : 1 - 50, 5 - 51/52, 6 - 53/54, 1 - 60                   */
INT      iCallSigSet = 0;

//BOOL WAITANDRESET = FALSE;            /* to add SysCreateEventSem Flag  */
BOOL CALL_BY_RXQUEUE = FALSE;           /* THU021A */

INT opencnt[MAXUTILSEM][2] = {0};      /* array for remembering the  */
                                       /* open calls to the rexxutil */
                                       /* semaphores.And TIDs        */

void attachall(int);                   /* attach shared memory blocks*/
                                       /* deregister proc specific   */
void RxSubcomExitList();               /*  subcomands or exits       */

void RxExitClear(int);
void RxExitClearNormal();

#define ALREADY_INIT      1            /* queue manager status       */

#ifdef HAVE_SIGPROCMASK
static sigset_t oldmask,newmask;
#endif
//#define  lazy_block apidata->lazy_block
//#define  lazy_size apidata->lazy_size


/*********************************************************************/
/* Function:           Serialize REXX API function execution and     */
/*                     perform memory initialization.                */
/*                                                                   */
/* Description:        Obtain ownership of the rexxapi semaphore that*/
/*                     serializes REXX API Function execution.       */
/*                     Establish an exit list routine to clear the   */
/*                     semaphore if we abort before we clear it.     */
/*                                                                   */
/*                     Also obtains access to all of the memory      */
/*                     blocks currently allocated for API control    */
/*                     blocks.                                       */
/*                                                                   */
/* Input:              Info which API memory to attach               */
/*                                                                   */
/* Output:             Zero if everything went well, error code      */
/*                     otherwise.                                    */
/*                                                                   */
/* Side effects:       Semaphore set.  Process, Thread and Session   */
/*                     ID variables initialized.  All memory segments*/
/*                     obtained and reconnected.                     */
/*                                                                   */
/*********************************************************************/

ULONG  RxAPIStartUp(INT chain)
{
KMTX        SemId;
INT         ShmemId = 0;
INT         semrc, value;
ULONG       current;                   /* session queue              */
key_t       ipckey;
shmid_ds    buf;                       /* buf to hold memory info    */
INT         used;                      /* semaphore used flag        */
LONG        lRC;

  if (iCallSigSet == 0 )                   /* No signal hold set             */
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
     iCallSigSet = 1;
//   EnterMustComplete(1);             /* signals need to be blocked through */
  }
                                       /* critical section                  */
 if ( achRexxHomeDir[0] != '/' )
 {
   lRC = RxAPIHOMEset();                /* Set the REXX HOME                */
   if ( lRC )
      exit(-1);                         /* Exit anyway                      */
   /* For API functions fist time cleanup of shared memory to drop functions */
#ifndef OPSYS_AIX41
   if(!CALL_BY_RXQUEUE)
      atexit(RxExitClearNormal);
#endif

    /* Set the cleanup handler for unconditional process termination  */
    struct sigaction new_action;
    struct sigaction old_action;        /* test if signal set already */

    /* Set up the structure to specify the new action                           */
    new_action.sa_handler = RxExitClear;
    old_action.sa_handler = NULL;
    sigfillset(&new_action.sa_mask);
    new_action.sa_flags = SA_RESTART;

/* Termination signals are set by Object REXX whenever the signals were not set */
/* from outside (calling C-routine). The SIGSEGV signal is not set any more, so */
/* that we now get a coredump instead of a hang up                              */

   sigaction(SIGINT, NULL, &old_action);
   if (old_action.sa_handler == NULL)           /* not set by ext. exit handler */
   {
     sigaction(SIGINT, &new_action, NULL);  /* exitClear on SIGTERM signal      */
   }
 }
 if(!apidata)                           /* if memory anchor not attached  */
 {                                      /* create the semaphore set       */
   ipckey = ftok(achRexxHomeDir, 'r');  /* generate a unique key          */
   if ( ipckey == -1)                   /* No key error                   */
   {
      perror(" *E*  No key generated for shared memory.\n");
      send_exception(Error_System_service);
      exit(-1);                         /* Stop anyway                    */
   }
/* ipckey = ftok(getenv("HOME"),'r');    * generate a unique key          */
/* semrc = createsem(&SemId,ipckey, MAXSEM+1);                      */
   semrc = createsem(&SemId,ipckey, MAXSEM);
   if(semrc == -1)                      /* already exists             */
   {
     if(opensem(&SemId,ipckey))         /* open the API semaphore     */
     {
        perror(" *E* Open of API semaphore failed.\n");
        send_exception(Error_System_service);
        exit(-1);                       /* Stop anyway                    */
     }
   }
   else
     if(semrc > 0 )                     /* error while create             */
     {
       fprintf(stderr," *E* No further API user possible!\n");
       send_exception(Error_System_service);
       exit(-1);                       /* Stop anyway                     */
     }
 }
 else
  SemId = apidata->rexxapisemaphore;
/* Check if the semaphore owner exists                               */
  if(!(getval(SemId, 0)) &&                   /* semaphore is set    */
                                              /* and owner died      */
     ((kill(semgetpid(SemId,0), 0)) == -1 ) )
//   ((getpgid(semgetpid(SemId,0)) == -1) && (errno == ESRCH)))
       unlocksem(SemId,0);                    /* unlock the sem      */

/*Check if the semaphore is in an undefined state, clear if necessary*/
  if(((value=getval((SemId), 0)) > 1)
     || (getval((SemId), 0) < 0 )) {
    if(value > 1){
      while(value != 1){                /* unlock the sem            */
        locksem(SemId, 0);              /* NULL */
        --value;
      }
    }
    else if(value < 0 ){
      while(value != 1){                /* unlock the sem            */
        unlocksem(SemId, 0);            /* NULL */
        value++;
      }
    }
  }
                                       /* serialize API function     */
                                       /* execution.  Wait forever   */
                                       /* for the API semaphore.     */
                                       /* Execution is single-       */
                                       /* threaded past this point.  */
  locksem(SemId, 0);

  if (!apidata){                     /* if memory anchor not attached*/
                                     /* create or open the shared    */
                                     /* API memory anchor            */
    ShmemId = getshmem(ipckey, sizeof(REXXAPIDATA));
    if(ShmemId == -1)                /* already exists               */
      ShmemId = openshmem(ipckey, sizeof(REXXAPIDATA));
    else
     if(ShmemId == -2)               /* system limit reached         */
     {
        fprintf(stderr," *E*  No further API user possible !\n");
        send_exception(Error_System_service);
        exit(-1);                    /* Stop anyway                  */
     }
    if(ShmemId == -1)                /* open failed                  */
    {
       perror(" *E*  Open of the shared memory failed!\n");
       send_exception(Error_System_service);
    }
                                     /* attach the anchor            */
    apidata =(REXXAPIDATA *)attachshmem(ShmemId);
  }

  apidata->rexxapisemaphore = SemId;

  attachall(chain);                      /*get all shared memory blocks*/

  apidata->ThreadId=(TID)SysQueryThreadID();
  if(apidata->ThreadId!=(TID)-1){/* if Rexx is up*/
                                         /*if no session queue exitsts */
                                         /*greate one                  */
    if(chain != QUEUECHAIN)
      attachall(QUEUECHAIN);             /* get the queue memory       */
    current = search_session();          /* greate the session queue   */
    if(chain != QUEUECHAIN)              /* release the queue memeory  */
      detachall(QUEUECHAIN);
//  ExitMustComplete();                  /* exit critical section      */
  }
//else{                                  /* only API call              */
//                                       /* establish cleanup handlers */
//  /* Set the cleanup handler for unconditional process termination              */
//  struct sigaction new_action;
//  struct sigaction old_action;         /* test if signal set already */
//
//  /* Set up the structure to specify the new action                             */
//  new_action.sa_handler = RxExitClear;
//  old_action.sa_handler = NULL;
//  sigfillset(&new_action.sa_mask);
//  new_action.sa_flags = SA_RESTART;

/* Termination signals are set by Object REXX whenever the signals were not set */
/* from outside (calling C-routine). The SIGSEGV signal is not set any more, so */
/* that we now get a coredump instead of a hang up                              */
//
//sigaction(SIGINT, NULL, &old_action);
//if (old_action.sa_handler == NULL)                       /* not set by ext. exit handler*/
//{
//  sigaction(SIGINT, &new_action, NULL);  /* exitClear on SIGTERM signal         */
//}
/* THU012 end                                                                   */

    /* this is for normal process termination                                   */

//  atexit(RxExitClearNormal);
//}
  apidata->ProcessId = getpid();       /* set the process id                    */

  apidata->init = ALREADY_INIT;

  /* Check for rexxutil semaphores an their usecounts. The usecount  */
  /* can be to high if one process died without cleanup.             */

  if(apidata->rexxutilsems){           /* if we have util semaphores */
    if(!ShmemId){                       /* if we have no mem ID       */
      ipckey = ftok(achRexxHomeDir, 'r'); /* generate a unique key          */
      if ( ipckey == -1)                 /* No key error                    */
      {
        perror(" *E*  No key generated for the shared memory");
        send_exception(Error_System_service);
        exit(-1);                      /* Stop anyway                       */
      }
/*ipckey = ftok(getenv("HOME"),'r');    * generate a unique key             */
   /* ipckey = ftok(getenv("HOME"),'r'); * generate the key and      */
      ShmemId = openshmem(ipckey, sizeof(REXXAPIDATA));/* get the ID */
    }
    shmctl(ShmemId,IPC_STAT,&buf);     /* get the memory info        */
    if(buf.shm_nattch == 1){           /*if only I can work with sems*/
      for(int i=0;i<MAXUTILSEM;i++){/* for all possible semaphores   */
        if(((apidata->utilsemfree[i]).usecount))/* if it is used     */
          if(!(opencnt[i][0])){        /* but not from me            */
                                       /* clear the name array       */
              memset((apidata->utilsemfree[i]).name, 0, MAXNAME);
              (apidata->utilsemfree[i]).usecount=0;/* free it        */
              /* Possibly this was the last used sem. So we can remove the       */
              /* semaphore set. Check this possibility.                          */
          }
      }
      used = 0;
      for(int j=0;j<MAXUTILSEM;j++)
      {                                       /* for all semaphores    */
        if((apidata->utilsemfree[j]).usecount != 0 ) /* a used one ?   */
        {
          used = 1;                      /* remember it                */
          break;
        }
      }
      if(!used)
      {                                     /* if all sems are unused  */
         removesem(apidata->rexxutilsems);  /* remove the semaphore set*/
         apidata->rexxutilsems = 0;         /* delete the old ID       */
      }
    }
  }

  return (0);                            /* it all worked              */
}

/*********************************************************************/
/* Function:           End single-threaded execution of the REXX     */
/*                     API library, releasing shared access to the   */
/*                     global memory segments.                       */
/*                                                                   */
/* Description:        Release ownership of the REXX API semaphore   */
/*                     and global memory segments.                   */
/*                                                                   */
/* Inputs:             Info which API memory to detach               */
/*                                                                   */
/* Outputs:            Nothing.  Called for side effects only.       */
/*                                                                   */
/* Side effects:       REXX API Semaphore cleared and shared access  */
/*                     to global memory segments released.           */
/*                                                                   */
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/

void RxAPICleanUp(INT chain, INT iSigCntl)
{


  detachall(chain);                    /* detach shared memory       */

   /******************************************************************/
   /* End of single-threaded API code.  Release the                  */
   /* semaphore so other threads can enter the API system.           */
   /******************************************************************/

                                       /* Release the semaphore.     */
   unlocksem(apidata->rexxapisemaphore, 0);
// if (iSigCntl)                       /* If SIGCNTL_RELEASE         */
   if (iCallSigSet == 1 )              /* Signal set                 */
   {
#if defined( HAVE_SIGPROCMASK )
      sigprocmask( SIG_SETMASK, &oldmask , NULL );
#elif defined( HAVE_SIGHOLD )
      sigrelse(SIGINT);
      sigrelse(SIGTERM);
      sigrelse(SIGILL);
      sigrelse(SIGSEGV);
#endif
      iCallSigSet = 0;
//    ExitMustComplete(50);
   }
}
/* Function added                                                    */
/*********************************************************************/
/* Function:           Set the global variable for REXX HOME         */
/*                     to get a key for semaphore and shared         */
/*                     memory segments access.                       */
/*                                                                   */
/* Description:        Get ownership of the REXX API semaphore       */
/*                     and global memory segments.                   */
/*                                                                   */
/* Inputs:             Environment variable RXHOME or HOME           */
/*                                                                   */
/* Outputs:            Return code  0 for O.K.                       */
/*                                 -1 for NOT O.K.                   */
/*                                                                   */
/* Notes:              RXHOME is used first.                         */
/*                                                                   */
/*********************************************************************/

LONG RxAPIHOMEset( void )
{
  char * pcharHome;                       /* Pointer to environmet var HOME */
//char   charCodeHome = '/';              /* RXHOME or HOME var is used     */
  int    iHandleHome;                     /* File handle for IPC anchor     */
  struct stat statbuf;                    /* structure for stat system calls*/
#if defined( HAVE_GETPWUID )
  struct passwd * pstUsrDat;
#endif
  char *pcharUsername;

  if (!(pcharHome = getenv("RXHOME")))    /* Get pointer to group home var  */
  {
    // get the username once
#if defined( HAVE_GETPWUID )
    pstUsrDat = getpwuid(geteuid());
    pcharUsername = pstUsrDat->pw_name;
    pcharHome = pstUsrDat->pw_dir;        /* Get pointer to own home var    */
#elif defined( HAVE_IDTOUSER )
    pcharUsername = IDtouser(geteuid()));
    #if defined( HAVE_GETUSERATTR )
        getuserattr(pcharUsername, S_HOME, &pcharHome, SEC_CHAR);
    #else
        /* this is not the best method to obtain the user's home dir as it      */
        /* could fail on LDAP enabled systems.                                  */
        pcharHome = getenv("HOME");           /* Get pointer to own home var    */
    #endif
#else
    pcharUsername = "unknown";
    /* this is not the best method to obtain the user's home dir as it      */
    /* could fail on LDAP enabled systems.                                  */
    pcharHome = getenv("HOME");           /* Get pointer to own home var    */
#endif
    sprintf(achRexxHomeDir,"%s/..OOREXX%d.%d.%d.%d_%s",
              pcharHome, ORX_VER, ORX_REL, ORX_MOD, ORX_FIX, pcharUsername );
    iHandleHome = open( achRexxHomeDir, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
    if ( iHandleHome < 0 )
    {
       sprintf(achRexxHomeDir,"/tmp/..OOREXX%d.%d.%d.%d_%s",
                        ORX_VER, ORX_REL, ORX_MOD, ORX_FIX, pcharUsername );
       iHandleHome = open( achRexxHomeDir, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
       if ( iHandleHome < 0 )
       {
          fprintf(stderr," *E* No HOME directory and file anchor for REXX!\n");
          return(-1);                        /* all done ERROR end          */
       }
    }
    close( iHandleHome );
  }
  else
  {
    strcpy(achRexxHomeDir, pcharHome);       /* Save HOME directory          */
    if ( achRexxHomeDir[0] != '/' )
    {
      fprintf(stderr," *E* The directory and file > %s < is not fully qualified!\n",
              achRexxHomeDir );
      return(-1);                            /* all done ERROR end           */
    }
    if ( stat(achRexxHomeDir, &statbuf) < 0 ) /* If file is found,           */
    {
      fprintf(stderr," *E* The directory or file > %s < does not exist!\n",
              achRexxHomeDir );
      return(-1);                            /* all done ERROR end           */
    }
    else
    {                                        /* Check group and user permission */
      if ( ((statbuf.st_gid != getegid()) ||
           ((statbuf.st_mode & (S_IRGRP | S_IWGRP)) != (S_IRGRP | S_IWGRP))) &&
           ((statbuf.st_uid != geteuid()) ||
           ((statbuf.st_mode & (S_IRUSR | S_IWUSR)) != (S_IRUSR | S_IWUSR))) )
      {
        fprintf(stderr," *E* No read and write permission for REXX to use > %s < !\n",
                achRexxHomeDir );
        return(-1);                          /* all done ERROR end            */
      }
      else
        iSemShmMode = 432;                   /* Set MODE 660 for sem and shm  */
    }
  }
  return(0);
}

/*********************************************************************/
/* Function:           Allocate a control block for one of the apis. */
/*                                                                   */
/*                                                                   */
/* Description:        Allocates a control block for an api          */
/*                                                                   */
/*                                                                   */
/* Inputs:             The size of the control block. Info from      */
/*                     which shared memory segment to allocate.      */
/*                                                                   */
/* Outputs:            Offset to the allocated block                 */
/*                                                                   */
/*********************************************************************/

/*THU017C begin */
LONG  RxAllocMem(
  PULONG block,                      /* offset to reach the block    */
  ULONG size,                        /* size to allocate             */
  INT flag)                          /* from which memory to allocate*/
{
  key_t key;
  PCHAR  newmem;                     /* new memory                   */
  PCHAR  runptr;                     /* runs through shared memory   */
  INT   newmemId;                    /* ID of the new memory         */
  ULONG mbase, baseblock, next, movesize, base, newbase, session_base, newsession_base, first, last, inext, temptop, tempapidatasize;
  PQUEUEHEADER queueheader;
  PQUEUEITEM item;
  ULONG addsize;                      /*THU025A */
  PCHAR tempptr;                      /*THU025A */


  if(flag == MACROMEM){         /* allocate in the macro memory pool?*/
    if(apidata->macrobase == NULL){  /*if no macros until now        */
      /* Allocate a shared memory segment of the standard size       */
      if(((apidata->mbasememId) = getshmem(IPC_PRIVATE, MSTDSIZE)) == -2)
        return (1);                  /* system limit reached         */
                                     /* attach the memory            */
      apidata->macrobase = attachshmem(apidata->mbasememId);
                                     /* and clear it out             */
      memset((void*)apidata->macrobase, 0, MSTDSIZE);
      apidata->macrosize = MSTDSIZE; /* remember the size            */
      apidata->mmemtop = SHM_OFFSET; /* the memory is empty          */
      apidata->mbase = 0;            /* set end of chain             */
      apidata->macrocount = 0;       /* make sure, no garbage        */
    }
    /* Do we need a larger segment to provide the memory ?           */
    if(size > ((apidata->macrosize) - (apidata->mmemtop))
                                  -10){/* sub 10 to be sure          */
                                     /* while segment to small       */
      while(size > (((apidata->macrosize) - (apidata->mmemtop)-10))){
                                     /* alloc a larger segment       */
        if((newmemId = getshmem(IPC_PRIVATE,((apidata->macrosize)+MSTDSIZE))) == -2)
          return (1);                /* no more memory avialble      */
                                     /* attach the new memory        */
        newmem = attachshmem(newmemId);
                                     /* and clear it out             */
        memset((void*)newmem, 0, ((apidata->macrosize)+MSTDSIZE));
                                     /* copy the data                */
        memcpy((void*)newmem, (void*)apidata->macrobase,
                              (size_t)apidata->mmemtop);
        removeshmem(apidata->mbasememId); /* remove the old memory   */
        detachshmem(apidata->macrobase);  /* force the deletion      */
        apidata->macrobase = newmem; /* remember the new pointer     */
        apidata->mbasememId = newmemId; /* and the new memory ID     */
                                     /* don't forget the new size    */
        apidata->macrosize = ((apidata->macrosize)+MSTDSIZE);
      }
      *block = apidata->mmemtop;     /* return the space             */
      apidata->mmemtop += size;      /* set the new top              */
    }
    else {                           /* there is enough space        */
      *block = apidata->mmemtop;     /* return the space             */
      apidata->mmemtop += size;      /* set the new top              */

    }
    return (0);                      /* worked well                  */
  }
  /* allocate in the se (subcom,exit,func) memory pool ?             */
  else if(flag ==SEMEM){
    if(apidata->sebase == NULL){     /*if no registations until now  */
      /* Allocate a shared memory segment of the standard size       */
      if(((apidata->sebasememId) = getshmem(IPC_PRIVATE, SESTDSIZE)) == -2)
        return (1);                  /* system limit reached         */
                                     /* attach the memory            */
      apidata->sebase = attachshmem(apidata->sebasememId);
                                     /* and clear it out             */
      memset((void*)apidata->sebase, 0, SESTDSIZE);
      apidata->sememsize = SESTDSIZE;/* remember the size            */
      apidata->sememtop = SHM_OFFSET;/* the memory is empty          */
                                     /* set all chains to empty      */
      apidata->baseblock[REGSUBCOMM] = 0;
      apidata->baseblock[REGSYSEXIT] = 0;
      apidata->baseblock[REGFUNCTION] = 0;
    }
    /* Do we need a larger segment to provide the memory ?           */
    if(size > ((apidata->sememsize) - (apidata->sememtop))
                                  -10){/* sub 10 to be sure          */
                                     /* while segment to small       */
      while(size > (((apidata->sememsize) - (apidata->sememtop)-10))){
                                     /* allocate a greater segment   */
        if((newmemId = getshmem(IPC_PRIVATE,((apidata->sememsize)+SESTDSIZE))) == -2)
          return (1);                /* no more memory avialble       */
        newmem = attachshmem(newmemId);/* attach the new memory       */
                                     /* and clear it out              */
        memset((void*)newmem, 0, ((apidata->sememsize)+SESTDSIZE));
        memcpy((void*)newmem, (void*)apidata->sebase,/* copy the data */
                                            (size_t)apidata->sememtop);
        removeshmem(apidata->sebasememId);/* remove the old memory    */
        detachshmem(apidata->sebase);/* force the deletion            */
        apidata->sebase = newmem;    /* remember the new pointer      */
        apidata->sebasememId = newmemId;/* and the new memory ID      */
                                     /* don't forget the new size     */
        apidata->sememsize = ((apidata->sememsize)+SESTDSIZE);
      }
      *block = apidata->sememtop;    /* return the space              */
      apidata->sememtop += size;     /* set the new top               */
    }
    else {                           /* there is enough space         */
      *block = apidata->sememtop;    /* return the space              */
      apidata->sememtop += size;     /* set the new top               */

    }
    return (0);                      /* worked well                   */
  }
  /* allocate in the queue  memory pool ?                             */
  else if(flag == QMEM)
  {
    if((apidata->base == 0 )         /*if no queues until now         */
          && (apidata->session_base == 0 ))
    {
      /* Allocate a shared memory segment of the standard size       */
      if(((apidata->qbasememId) = getshmem(IPC_PRIVATE, QSTDSIZE)) == -2)
        return (1);                  /* system limit reached          */
                                     /* attach the memory             */
      apidata->qbase = attachshmem(apidata->qbasememId);
                                     /* and clear it out              */
      memset((void*)apidata->qbase, 0, QSTDSIZE);
      apidata->qmemsize = QSTDSIZE;  /* remember the size             */
      apidata->qmemtop = SHM_OFFSET; /* the memory is empty           */
                                     /* set all chains to empty       */
      apidata->qmemsizeused = SHM_OFFSET; /* THU as for new implementa*/
                                     /* tion i need to know the total */
                                     /* size of all still alive elem  */
                                     /* ents in shared memory.The rea */
                                     /* son is, that qmemtop does not */
                                     /* give me the right value any   */
                                     /* more, as we have gaps in betw */
                                     /* een the elements. And this    */
                                     /* value is essential if i decide*/
                                     /* to decrease shared memory.    */
      apidata->trialcounter = 0;     /* The trialcounter is used to   */
                                     /* check, how many elements have */
                                     /* been successfully pulled from */
                                     /* the shared memory, until the  */
                                     /* last time of rearranging the  */
                                     /* queue. If not at least three  */
                                     /* elements have been pulled from*/
                                     /* queue since last time of re   */
                                     /* arrangement, i will increase  */
                                     /* shared memory in any way      */
    }

      base = apidata->base;            /* get the anchor of the chain*/
                                       /* apidata->base points to the*/
                                       /* queue,that was created last*/
                                       /* apidata->qbase points to th*/
                                       /* e beginning of shared mem  */

      session_base = apidata->session_base;
      ULONG previous = 0;
      previous = base;

/* Do we need a larger segment to provide the memory ?                             */
/* qmemtop does not show how much memory is used at whole (only if no item is being*/
/* erased), because there could have been elements deleted already in between. This*/
/* is the task of the qmemsizeuse parameter.                                       */
/* now let me check whether the new element fits into the buffer without having to */
/* reorganize the buffer. (size is the size of the element, qmemsize is the size   */
/* of the shared memory, qmemtop is the place where the next element should be plac*/
/* ed), qmemsizeused is the amount of data alive in shared memory                  */

    if(size > ((apidata->qmemsize) - (apidata->qmemtop)) - 10) /* sub 10 to be sure */
    {
#ifdef QUEUE_DBG
       printf("now i know the item goes behind the actual shared memory, i need to have special processing \n");
       printf("Element size : %d , qmemsize: %d \n ", size, apidata->qmemsize);
#endif

       /* memory would be rearranged, but only if at least 5 items have been pulled  */
       /* from the queue. This is to prevent rearrangement, if queue-processing is on its limit */

       if (size < (apidata->qmemsize - (apidata->qmemsizeused + 10)) && (apidata->trialcounter > 5))
       {
          /* we first start with a shared memory segement that is equal the memory       */
          /* of the existing shared memory.                                              */

#ifdef QUEUE_DBG
          printf("now i know the item would fit into the buffer, if i would rearrange, size: %d  \n", size);
          printf("Size of element: %d, apidata->qmemsize: %d, apidata->qmemsizeused: %d, apidata->trialcounter: %d\n", size, apidata->qmemsize, apidata->qmemsizeused, apidata->trialcounter);
#endif
          if(((newmemId) = getshmem(IPC_PRIVATE, apidata->qmemsize)) == -2)
            return (1);
          newmem = attachshmem(newmemId);
          memset((void*)newmem, 0, ((apidata->qmemsize)));
          runptr = newmem;

//          runptr++;                         /* qmemtop always starts with '1'          */
          runptr = runptr + SHM_OFFSET;       /* qmemtop starts at position 4            */
          /* now i need to copy: for every queue (session queue as well as named queue   */
          /* in the shared memory), let us first copy the queuehaeder to the new desti   */
          /* nation. After that, the elements (itemheaders and items) of that queue.     */

          newsession_base = 0;
          newbase = 0;
          if (session_base)
          {
             newsession_base = runptr - newmem;
          }
          while(session_base!=0)                   /* while there are session queues  */
          {
            queueheader = (PQUEUEHEADER) runptr; /* for creating new next values  */
            next = QHDATA(session_base)->next;     /* remember the next session queue in old shared mem*/

            /* first copy the header of the queue                           */
            memcpy(runptr, QHDATA(session_base), sizeof(QUEUEHEADER));
            /*  printf("queueheader->next is equal %d\n", (queueheader)->next);                 */
            /*  printf("QHDATA(session_base)->next is equal %d\n", QHDATA(session_base)->next); */
            runptr = runptr + sizeof(QUEUEHEADER);
            first = QHDATA(session_base)->queue_first; /*this is the first element  */
                                               /* of this queue             */
            last = QHDATA(session_base)->queue_last;   /*this is the last element   */
                                               /* of this queue             */
                                               /* move through item chain   */
            if (first)
            {
#ifdef QUEUE_DBG
               printf("There is at least one element in the session queue it is located at: %d element: %s\n", first,QDATA(QIDATA(first)->queue_element));
               printf("There is at least one element in the session queue it is located at: %p \n", first);
#endif
               queueheader->queue_first = runptr - newmem;  /* proceed to the place where i can put my first element, also be  */
            }                                               /* necessary if the first element == last element                  */
            else
            {
               queueheader->queue_first = 0;
               queueheader->queue_last = 0;
               /*  printf("There was nothing in this session queue \n"); */
            }
            while((first) && (first!=last))
            {
              inext = QIDATA(first)->next;     /* remember the next item    */
              /* now let me start with the itemheader                       */
              item = (PQUEUEITEM) runptr;
              memcpy(runptr, QIDATA(first), sizeof(QUEUEITEM));
              addsize = RXROUNDUP(sizeof(QUEUEITEM) + QIDATA(first)->size, SHM_OFFSET );
              tempptr= runptr;
              runptr = runptr + sizeof(QUEUEITEM);
              /* and the element                                            */
              item->queue_element = runptr - newmem;      /* here is the element located */
              memcpy(runptr, QDATA(QIDATA(first)->queue_element), QIDATA(first)->size);
//            runptr = runptr + QIDATA(first)->size;
              runptr = tempptr + addsize;
              if(inext)
              {
                 (item)->next = runptr - newmem;      /* remember the next item*/
              }
              else
              {
                 (item)->next = 0;                    /* remember the next item*/
              }
              first = inext;
            }
            if(last)
            {
#ifdef QUEUE_DBG
              printf("now i copy the last item to the new shared memory\n");
#endif
              queueheader->queue_last = runptr - newmem;
              item = (PQUEUEITEM) runptr;
              memcpy(runptr, QIDATA(last), sizeof(QUEUEITEM));
              addsize = RXROUNDUP(sizeof(QUEUEITEM) + QIDATA(last)->size, SHM_OFFSET );
              tempptr= runptr;
              runptr = runptr + sizeof(QUEUEITEM);
              /* and the last in line                                          */
              item->queue_element = runptr - newmem;
              memcpy(runptr, QDATA(QIDATA(last)->queue_element), QIDATA(last)->size);
//            runptr = runptr + QIDATA(last)->size; /* now the runptr is behind the last element */
              runptr = tempptr + addsize;
              item->next = 0;                    /* there is nothing left to do*/
            }
            /* if last = 0 there is no action, because the correct value is taken */
            /* from the copy of the QUEUEHEADER                                   */

            /* the 'next' value of the new queueheader is still wrong, correct it           */
            if(next) /* this is the place in old shared memory, where next queuehaeder is   */
            {
               (queueheader)->next = runptr - newmem;/* here is the place where we can start with the next session queue */
#ifdef QUEUE_DBG
               printf("There is another session queue in shared memory, (queueheader)->next: %d\n",(queueheader)->next);
#endif
            }
            else
            {
#ifdef QUEUE_DBG
               printf("There is no other session queue in shared memory, nothing left do for session queues, queueheader->next = 0 \n");
#endif
               (queueheader)->next = 0;             /* there is no next queue */
            }
            session_base = next;                   /* take the next queue        */
          }

          /* now the named queues. it is important that the session queue(s) are copied BEFORE */
          /* the named queues. This is because otherwhile the anchor-points (apidata->base and */
          /* apidata->session_base wont fit)                                                   */

          if (base)
          {
             newbase = runptr - newmem;
          }

          while(base!=0)                   /* while there are named queues  */
          {
            next = QHDATA(base)->next;     /* remember the next named queue in old shared mem*/
            queueheader = (PQUEUEHEADER) runptr; /* for creating new next values  */

            /* first copy the header of the queue                           */
            memcpy(runptr, QHDATA(base), sizeof(QUEUEHEADER));
            runptr = runptr + sizeof(QUEUEHEADER);
            first = QHDATA(base)->queue_first; /*this is the first element  */
                                               /* of this queue             */
            last = QHDATA(base)->queue_last;   /*this is the last element   */
                                               /* of this queue             */
                                               /* move through item chain   */
            if (first)
            {
               queueheader->queue_first = runptr - newmem;
            }
            else
            {
               queueheader->queue_first = 0;
               queueheader->queue_last = 0;
            }
            while((first) && (first!=last))
            {
              inext = QIDATA(first)->next;     /* remember the next item    */
              /* now let me start with the itemheader                       */
              item = (PQUEUEITEM) runptr;
              memcpy(runptr, QIDATA(first), sizeof(QUEUEITEM));
              addsize = RXROUNDUP(sizeof(QUEUEITEM) + QIDATA(first)->size, SHM_OFFSET );
              tempptr= runptr;
              runptr = runptr + sizeof(QUEUEITEM);
              /* and the element                                            */
              item->queue_element = runptr - newmem;      /* here is the element located */
              memcpy(runptr, QDATA(QIDATA(first)->queue_element), QIDATA(first)->size);
//            runptr = runptr + QIDATA(first)->size;
              runptr = tempptr + addsize;
              if(inext)
              {
                 (item)->next = runptr - newmem;      /* remember the next item*/
              }
              else
              {
                 (item)->next = 0;                    /* remember the next item*/
              }
              first = inext;
            }
            if(last)
            {
#ifdef QUEUE_DBG
              /*   printf("now i copy the last item to the new shared memory\n"); */
#endif
              queueheader->queue_last = runptr - newmem;
              item = (PQUEUEITEM) runptr;
              memcpy(runptr, QIDATA(last), sizeof(QUEUEITEM));
              addsize = RXROUNDUP(sizeof(QUEUEITEM) + QIDATA(last)->size, SHM_OFFSET );
              tempptr= runptr;
              runptr = runptr + sizeof(QUEUEITEM);
              /* and the last in line                                          */
              item->queue_element = runptr - newmem;
              memcpy(runptr, QDATA(QIDATA(last)->queue_element), QIDATA(last)->size);
//            runptr = runptr + QIDATA(last)->size; /* now the runptr is behind the last element */
              runptr = tempptr + addsize;
            }
            /* if last = 0 there is no action, because the correct value is taken */
            /* from the copy of the QUEUEHEADER                                   */

            /* the 'next' value of the new queueheader is still wrong, correct it           */
            if(next) /* this is the place in old shared memory, where next queuehaeder is   */
            {
               (queueheader)->next = runptr - newmem;/* remember the next named queue */
            }
            else
            {
               (queueheader)->next = 0;             /* there is no next queue */
            }
            base = next;                   /* take the next queue        */
          }
          /* now we have rearranged the new shared memory, let us forget*/
          /* about the old one. Make the new one the one and only       */

          removeshmem(apidata->qbasememId);
          detachshmem(apidata->qbase);
          apidata->qbase = newmem;
          apidata->qbasememId = newmemId;
          apidata->session_base = newsession_base;
          apidata->base = newbase;
          apidata->qmemtop = runptr - newmem; /*rearrange qmemtop         */
          *block = apidata->qmemtop;     /* return the space              */
          apidata->qmemtop += size;      /* set the new top               */

          /* set back trialcounter,to see the next time whether at least*/
          /* 5 items have been pulled from the queue. If not it makes no*/
          /* sense to rearrange, because we are permanently on the limit*/
          /* of the actual shared memory                                */

          apidata->trialcounter = 0;
       }
       /* either the item would not fit although i rearrange the shared memory*/
       /* or there are not at least 5 elements pulled from the queue, so let  */
       /* us increase the memory so far, that the new item would fit in. Be-  */
       /* for chaining it into apidata, we need to copy the old shared memory */
       else
       {
#ifdef QUEUE_DBG
          printf("the item would not fit into shared memory although i rearrange, let me increase and copy over\n");
          printf("The size of the item: %d, the size of the buffer(qmemsize): %d, end of itemchain (qmemtop): %d \n", size, apidata->qmemsize, apidata->qmemtop);
#endif
          while(size > ((apidata->qmemsize) - (apidata->qmemtop)-10) )
          {
//           if((newmemId = getshmem(IPC_PRIVATE,((apidata->qmemsize)+QSTDSIZE))) == -2)
             if((newmemId = getshmem(IPC_PRIVATE,((apidata->qmemsize) * 2))) == -2)
               return (1);                /* no more memory avialble       */
             newmem = attachshmem(newmemId);/* attach the new memory       */
                                          /* and clear it out              */
             memset((void*)newmem, 0, ((apidata->qmemsize)+QSTDSIZE));
             memcpy((void*)newmem, (void*)apidata->qbase,/* copy the data  */
                                               (size_t)apidata->qmemtop);
             temptop = apidata->qmemtop;
             tempapidatasize = apidata->qmemsize;
             removeshmem(apidata->qbasememId);/* remove the old memory     */
             detachshmem(apidata->qbase); /* force the deletion            */
             apidata->qbase = newmem;     /* remember the new pointer      */
             apidata->qbasememId = newmemId;/* and the new memory ID       */
                                        /* don't forget the new size     */
//           apidata->qmemsize = tempapidatasize + QSTDSIZE;
             apidata->qmemsize = tempapidatasize * 2;
#ifdef QUEUE_DBG
             printf("New buffer size: %d \n", apidata->qmemsize);
#endif
             apidata->qmemtop = temptop;
          }/* end while */
          *block = apidata->qmemtop;     /* return the space              */
          apidata->qmemtop += size;      /* set the new top               */
          apidata->trialcounter = 0;     /* reset for next time           */
       } /* end else */
    }
    else                             /* there is enough space         */
    {
      *block = apidata->qmemtop;     /* return the space              */
      apidata->qmemtop += size;      /* set the new top               */
    }
#ifdef QUEUE_DBG
    printf("now increasing qmemsizeused. actual: %d, size: %d, new: %d \n", apidata->qmemsizeused, size, apidata->qmemsizeused +size);
#endif
    apidata->qmemsizeused = apidata->qmemsizeused + size;
    return (0);                      /* worked well                   */
  }
  else return (1);                   /* return unknown memory pool    */
}
/*THU017C end */

/*********************************************************************/
/* Function:           Release a control block for one of the apis   */
/*                                                                   */
/*                                                                   */
/* Description:        Free a control block for one of the apis      */
/*                                                                   */
/* Inputs:             The size of the control block. Info from      */
/*                     which shared memory segment to free.          */
/*                                                                   */
/* Outputs:            Return code                                   */
/*                                                                   */
/*********************************************************************/
LONG  RxFreeMem(
  ULONG      pblock,                 /* returned block               */
  ULONG      size,                   /* size of the block            */
  INT        flag)                   /* form which memory to drop    */
{
                                     /* offsets                      */
  ULONG mbase, baseblock, next, movesize, base, newbase, session_base, newsession_base, first, last, inext, temptop, tempapidatasize;
  key_t key;
  INT i;
  PVOID movearea;
  INT newmemId;                    /* ID of the new memory         */
  PCHAR newmem;                      /* new memory                   */
  LONG temp_cblock;
  LONG temp_nblock;
  ULONG previousitem = 0;
  ULONG previousbase  = 0;
  BOOL found = FALSE;

  if(flag == MACROMEM){              /* free in the macro memory pool*/
    mbase = apidata->mbase;          /* get the anchor               */
                                     /* need to update the offset    */
    if((mbase!=0) && (mbase > pblock))
      apidata->mbase -= size;        /* redurce the offset           */
    while(mbase!=0){                 /* while there are macros       */
                                     /* remember the next one        */
      next = ((PMACRO)((apidata->macrobase)+mbase))->next;
      if(next && (next > pblock))    /* if not the last in chain     */
                                     /* reduce the 'next' offset     */
        ((PMACRO)((apidata->macrobase)+mbase))->next -= size;
                                /* need to update the 'image' offset?*/
      if((((PMACRO)((apidata->macrobase)+mbase))->image) > pblock)
       ((PMACRO)((apidata->macrobase)+mbase))->image -= size;/* do it*/
      mbase = next;                  /* take the next one in chain   */
    }/* move the data to close the gap (the 'memmove' function does not work well !) */
    movesize = apidata->mmemtop-(pblock+size);/* size to move        */
    if(movesize > 0 ){             /* if there is something to move  */
      movearea = malloc((size_t)movesize); /* allocate a temp area   */
                                   /* move the data to the temp area */
      memcpy(movearea, apidata->macrobase+(pblock+size), movesize);
                                /* copy it back to the shared memory */
      memcpy(apidata->macrobase+pblock, movearea, movesize);
      free(movearea);                /* free the temp memory         */
    }
    apidata->mmemtop -= size;        /* update the top 'pointer'     */
                                     /* clear out the unused memory  */
    memset(apidata->macrobase+(apidata->mmemtop), 0, size);

                              /* while we can reduce the memory size */
    while((apidata->mmemtop < (((apidata->macrosize)/2)-10))
                                   && (apidata->macrosize > MSTDSIZE)){
        /* Allocate a smaller memory segment                         */
        if((newmemId = getshmem(IPC_PRIVATE,((apidata->macrosize)/2))) == -2)
          return (-1);                 /* system limit reached       */
        newmem = attachshmem(newmemId);/* attach the new memory      */
                                       /* and clear it out           */
        memset((void*)newmem, 0, ((apidata->macrosize)/2));
        memcpy((void*)newmem,          /* copy the data              */
                  (void*)apidata->macrobase, (size_t)apidata->mmemtop);
        removeshmem(apidata->mbasememId);/* remove the old memory    */
        detachshmem(apidata->macrobase); /* force the deletion       */
        apidata->macrobase = newmem;     /* remember the new pointer */
        apidata->mbasememId = newmemId;  /* and the new memory ID    */
                                         /* don't forget the new size*/
        apidata->macrosize = ((apidata->macrosize)/2);
    }
    return (0);                              /* Yep !             */
  }
  else if(flag == SEMEM){              /* free in the se memory pool */
//  for(i=0;i<REGNOOFTYPES;i++){       /* for all chains             */
//                                     /* get the anchor of the chain*/
//    baseblock = apidata->baseblock[i];
//                                 /* if there are registation blocks*/
//    if((baseblock!=0) && (baseblock > pblock))
//      apidata->baseblock[i] -= size; /* redurce the offset         */
//    while(baseblock!=0){             /* while there are macros     */
//                                     /* remember the next one      */
//      next = ((PMACRO)((apidata->sebase)+baseblock))->next;
//      if(next && (next > pblock))    /* if not the last in chain   */
//                                     /* reduce the 'next' offset   */
//        ((PMACRO)((apidata->sebase)+baseblock))->next -= size;
//      baseblock = next;              /* take the next one in chain */
//    }
//  }
    /* move the data to close the gap (the 'memmove' function does not work well !) */
    movesize = apidata->sememtop-(pblock+size);/* size to move       */
    if(movesize > 0)
    {
      movearea = malloc((size_t)movesize);  /* allocate a temp area  */
                                   /* move the data to the temp area */
      memcpy(movearea, apidata->sebase+(pblock+size), movesize);
                                /* copy it back to the shared memory */
      memcpy(apidata->sebase+pblock, movearea, movesize);
      free(movearea);                  /* free the temp memory       */
    }
    apidata->sememtop -= size;         /* update the top 'pointer'   */
                                       /* clear out the unused memory*/
    memset(apidata->sebase+(apidata->sememtop), 0, size);

    /* while we can reduce the memory size                           */
    while((apidata->sememtop < (((apidata->sememsize)/2)-10)) && (apidata->sememsize > SESTDSIZE)){
        /* Allocate a smaller memory segment                         */
        if((newmemId = getshmem(IPC_PRIVATE,((apidata->sememsize)/2))) == -2)
          return (-1);                 /* system limit reached       */
        newmem = attachshmem(newmemId);/* attach the new memory      */
                                       /* and clear it out           */
        memset((void*)newmem, 0, ((apidata->sememsize)/2));
        memcpy((void*)newmem,          /* copy the data              */
                    (void*)apidata->sebase, (size_t)apidata->sememtop);
        removeshmem(apidata->sebasememId);/* remove the old memory   */
        detachshmem(apidata->sebase);  /* force the deletion         */
        apidata->sebase = newmem;      /* remember the new pointer   */
        apidata->sebasememId = newmemId;/* and the new memory ID     */
                                        /* don't forget the new size */
        apidata->sememsize = ((apidata->sememsize)/2);
    }
    /* Repair chain of registration blocks, get order back               */
    /* Remember to set apidata->baseblock[i];                            */
    temp_cblock = apidata->sememtop;    /* get the end pointer           */
    temp_nblock = temp_cblock - size;   /* get ptr to last block         */
    while ( temp_cblock > SHM_OFFSET )
    {
      temp_cblock  -= size;       /* Set the current block offset        */

      if ( temp_cblock > size )
        temp_nblock  -= size;     /* Set the next    block offset        */
      else
        temp_nblock  = 0;         /* Set the next to beginning           */
                                  /* Update the entry to top             */
      ((PMACRO)((apidata->sebase)+temp_cblock))->next = temp_nblock;
    }
    if ( apidata->sememtop > SHM_OFFSET )
    {
        apidata->baseblock[REGSUBCOMM]  = apidata->sememtop - size;
        apidata->baseblock[REGSYSEXIT]  = apidata->sememtop - size;
        apidata->baseblock[REGFUNCTION] = apidata->sememtop - size;
    }
    else
    {
        apidata->baseblock[REGSUBCOMM]  = 0;     /* if all chains empty   */
        apidata->baseblock[REGSYSEXIT]  = 0;
        apidata->baseblock[REGFUNCTION] = 0;
    }
    return (0);                        /* Yep !                      */
  }
  else return (1);                     /* unknown memory pool        */
}

/*********************************************************************/
/* Function:           Release a control block for one of the apis   */
/*                                                                   */
/*                                                                   */
/* Description:        Free a control block for one of the apis      */
/*                                                                   */
/* Inputs:             The size of the control block. Info from      */
/*                     which shared memory segment to free.          */
/*                                                                   */
/* Outputs:            Return code                                   */
/*                                                                   */
/*********************************************************************/
/*THU017C begin */
LONG  RxFreeMemQue(
  ULONG      pblock,                 /* returned block               */
  ULONG      size,                   /* size of the block            */
  INT        flag,                   /* named or unnamed queue       */
  ULONG      current)                /* base block of the queue      */
{
                                     /* offsets                      */
  ULONG mbase, baseblock, next, movesize, base, newbase, session_base, newsession_base, first, last, inext, temptop, tempapidatasize;
  key_t key;
  INT i;
  PVOID movearea;
  INT newmemId;                      /* ID of the new memory         */
  PCHAR newmem;                      /* new memory                   */
  LONG temp_cblock;
  LONG temp_nblock;
  ULONG previousitem = 0;
  ULONG previousbase  = 0;
  BOOL found = FALSE;

  if(flag == QMEMNAMEDQUE)               /* free named queue element in the queue pool     */
  {
    base = apidata->base;                               /* get the anchor of the chain                      */
    if (current == pblock)                              /* so i should chain out a queueheader. All elements*/
    {                                                   /* are chained out already                          */
       while(( base != current) && (base != 0) )
       {
          previousbase = base;
          base = QHDATA(base)->next;
       } /* endwhile */
       if (base)
       {
          found = TRUE;
          if( (previousbase == 0) && (QHDATA(base)->next == 0) )
          {        /*This was the only named queue in shared memory          */
             apidata->base = 0;        /* chain out the base                 */
          }
          else if( (!previousbase) && (QHDATA(base)->next) )         /* The named queue is the first but not las queue in shared memory */
          {
             apidata->base = QHDATA(base)->next;           /* there are more named queues. Make the next the base */
          }
          else if( (previousbase) && (QHDATA(base)->next == 0) )      /* The named queue is not the first but last queue in shared memory */
          {
             QHDATA(previousbase)->next = 0;                          /* there are more named queues. Make the next the base */
          }
          else
          {
             QHDATA(previousbase)->next = QHDATA(base)->next;                          /* there are more named queues. Make the next the base */
          }
       }
       else /* could not find named queue, this should normaly be handled already */
       {
          return (1);
       }
    }  /* endif for current == pblock */

    /* that is, i have to chain out an item in a named queue */

    first = QHDATA(current)->queue_first;               /* lets go through item chain                     */
    last  = QHDATA(current)->queue_last;
    previousitem = 0;
    while( (first != 0)  && (!found) )
    {
       if(first == pblock)                           /* that is a queueitem needs to be chained out    */
       {
          found = TRUE;
          if( (!previousitem) && (QIDATA(first)->next == 0) ) /* the item to be chained out is the only element in queue  */
          {
             QHDATA(current)->queue_first = 0;
             QHDATA(current)->queue_last  = 0;
          }
          else if( (!previousitem) && (QIDATA(first)->next != 0) ) /* the item is the first but not last element in queue*/
          {
             QHDATA(current)->queue_first = QIDATA(first)->next;      /* queue_first could now be queue_last, thats ok      */
          }
          else if ( (previousitem) && (QIDATA(first)->next == 0) ) /* the item  is not the first but the last element in queue */
          {
             QIDATA(previousitem)->next = 0;                       /* should not be possible, let me think about it */
             QHDATA(current)->queue_last   = previousitem;
          }
          else                                     /* first is not the first and not the last element in queue         */
          {
             QIDATA(previousitem)->next = QIDATA(first)->next;
          }
       }
       else
       {
          previousitem = first;
          first = QIDATA(first)->next;
       }
    } /* end while for items */
//       if (!found)
//       {
//          previousbase = base;
//          base = next;
//          if ( base == pblock )
//          {
//             found = TRUE;
//             if (QHDATA(base)->next != NULL)
//             {
//                QHDATA(previousbase)->next = QHDATA(base)->next;
//             }
//             else
//             {
//                QHDATA(previousbase)->next = NULL;                         /* chain out the base                               */
//             }
//          }
//       }
//    } /* end while for named queues */
    apidata->trialcounter += 1;        /* increase for every element */
                                       /* pulled from the queue      */
    apidata->qmemsizeused = apidata->qmemsizeused - size;
    return (0);                        /* Yep !                      */
  } /* endwhile */
  else if(flag == QMEMSESSION)
  {                                                     /* free session queue element in the queue pool     */
    session_base = apidata->session_base;              /* get the anchor of the chain*/
    if (current == pblock)                              /* so i should chain out a queueheader of a sessionqueue. All elements*/
    {                                                   /* are chained out already                          */
       while(( session_base != current) && (session_base != 0) )
       {
          previousbase = session_base;
          session_base = QHDATA(session_base)->next;
       } /* endwhile */
       if (session_base)
       {
          found = TRUE;
          if( (previousbase == 0) && (QHDATA(session_base)->next == 0) ) /*This was the only named queue in shared memory */
          {
#ifdef QUEUE_DBG
             printf("RxFreeMemQue: HAVING ONLY ONE SESSION QUEUE in SHARED MEMORY\n");
#endif
             apidata->session_base = 0;   /* chain out the base     */
          }
          else if( (!previousbase) && (QHDATA(session_base)->next) )         /* The named queue is the first but not las queue in shared memory */
          {
#ifdef QUEUE_DBG
             printf("RxFreeMemQue: HAVING MORE THAN ONE SESSION QUEUE in SHARED MEMORY\n");
#endif
             apidata->session_base = QHDATA(session_base)->next;           /* there are more named queues. Make the next the base */
          }
          else if( (previousbase) && (QHDATA(session_base)->next == 0) )      /* The named queue is not the first but last queue in shared memory */
          {
             QHDATA(previousbase)->next = 0;                          /* there are more named queues. Make the next the base */
          }
          else
          {
             QHDATA(previousbase)->next = QHDATA(session_base)->next;         /* there are more named queues. Make the next the base */
          }
       }
       else /* could not find named queue, this should normaly be handled already */
       {
          return (1);
       }
    }  /* endif for current == pblock */


    /* that is, i have to chain out an item in a session queue */

    first = QHDATA(current)->queue_first;               /* lets go through item chain                     */
    last  = QHDATA(current)->queue_last;
    previousitem = 0;
    while( (first != 0)  && (!found) )
    {
       if(first == pblock)                           /* that is a queueitem needs to be chained out    */
       {
          found = TRUE;
          if( (!previousitem) && (QIDATA(first)->next == 0) ) /* the item to be chained out is the only element in queue  */
          {
             QHDATA(current)->queue_first = 0;
             QHDATA(current)->queue_last  = 0;
          }
          else if( (!previousitem) && (QIDATA(first)->next != 0) ) /* the item is the first but not last element in queue*/
          {
             QHDATA(current)->queue_first = QIDATA(first)->next;      /* queue_first could now be queue_last, thats ok      */
          }
          else if ( (previousitem) && (QIDATA(first)->next == 0) ) /* the item  is not the first but the last element in queue */
          {
             QIDATA(previousitem)->next = 0;                       /* should not be possible, let me think about it */
             QHDATA(current)->queue_last   = previousitem;
          }
          else                                     /* first is not the first and not the last element in queue         */
          {
             QIDATA(previousitem)->next = QIDATA(first)->next;
          }
       }
       else
       {
          previousitem = first;
          first = QIDATA(first)->next;
       }
    } /* end while for items */
    apidata->trialcounter += 1;        /* increase for every element */
                                       /* pulled from the queue      */
    apidata->qmemsizeused = apidata->qmemsizeused - size;
#ifdef QUEUE_DEBUG
    printf("NOW at END of RXFREEMEMQUE, apidata->trialcounter: %d, apidata->qmemsizeused: %d \n", apidata->trialcounter, apidata->qmemsizeused);
#endif
    return (0);                        /* Yep !                      */
  }
  else return (1);                     /* unknown memory pool        */
}

/*THU017C end */

/*********************************************************************/
/* Function:           Rearrange the shared memory if possible.      */
/*                                                                   */
/*                                                                   */
/* Description:        Rearrange shared memory as long as possible   */
/*                                                                   */
/* Inputs:             The size of the control block. Info from      */
/*                     which shared memory segment to free.          */
/*                                                                   */
/* Outputs:            Return code                                   */
/*                                                                   */
/*********************************************************************/

/*THU017A begin */
LONG CheckForMemory()
{
   INT newmemId,i;                    /* ID of the new memory         */
   PCHAR newmem;                      /* new memory                   */
   PCHAR  runptr;                     /* runs through shared memory   */
   PCHAR tempptr;
   PQUEUEHEADER queueheader;
   PQUEUEITEM item;
   ULONG addsize;
   ULONG mbase, baseblock, next, movesize, base, newbase, session_base, newsession_base, first, last, inext, temptop, tempapidatasize;

   while( (apidata->qmemsizeused < ((apidata->qmemsize)/4)) && (apidata->qmemsize > QSTDSIZE) )
   {
#ifdef QUEUE_DBG
      printf("CheckForMemory: I can reduce the size of the shared memory, qmemsizeused: %d, qmemsize: %d \n",apidata->qmemsizeused, apidata->qmemsize);
#endif
      if((newmemId = getshmem(IPC_PRIVATE,((apidata->qmemsize)/2))) == -2)
        return (1);
      newmem = attachshmem(newmemId);
      memset((void*)newmem, 0, ((apidata->qmemsize)/2));
      tempapidatasize = apidata->qmemsize / 2;
      runptr = newmem;

//      runptr++;                         /* qmemtop always starts with SHM_OFFSET   */
      runptr = runptr + SHM_OFFSET;       /*                                         */
      session_base = apidata->session_base;
      base = apidata->base;              /* get the anchor of the chain*/

      /* now i need to copy: for every queue (session queue as well as named queue   */
      /* in the shared memory), let us first copy the queuehaeder to the new desti   */
      /* nation. After that, the elements (itemheaders and items) of that queue.     */

      newsession_base = 0;
      newbase = 0;
      if (session_base)
      {
         newsession_base = runptr - newmem;
      }
      while(session_base!=0)                   /* while there are session queues  */
      {
         queueheader = (PQUEUEHEADER) runptr; /* for creating new next values  */
         next = QHDATA(session_base)->next;     /* remember the next session queue in old shared mem*/

           /* first copy the header of the queue                           */
         memcpy(runptr, QHDATA(session_base), sizeof(QUEUEHEADER));
#ifdef QUEUE_DBG
         printf("queueheader->next is equal %d\n", (queueheader)->next);
         printf("QHDATA(session_base)->next is equal %d\n", QHDATA(session_base)->next);
#endif
         runptr = runptr + sizeof(QUEUEHEADER);
         first = QHDATA(session_base)->queue_first; /*this is the first element  */
                                              /* of this queue             */
         last = QHDATA(session_base)->queue_last;   /*this is the last element   */
                                              /* of this queue             */
                                              /* move through item chain   */
         if (first)
         {
#ifdef QUEUE_DBG
            printf("(RxFree)There is at least one element in the session queue it is located at: %d element: \n", first);
#endif
            queueheader->queue_first = runptr - newmem;  /* proceed to the place where i can put my first element, also be  */
         }                                               /* necessary if the first element == last element                  */
         else
         {
            queueheader->queue_first = 0;
            queueheader->queue_last = 0;
#ifdef QUEUE_DBG
           printf("(RxFree)There was nothing in this session queue \n");
#endif
         }
         while((first) && (first!=last))
         {
           inext = QIDATA(first)->next;     /* remember the next item    */
           /* now let me start with the itemheader                       */
           item = (PQUEUEITEM) runptr;
           memcpy(runptr, QIDATA(first), sizeof(QUEUEITEM));
           addsize = RXROUNDUP(sizeof(QUEUEITEM) + QIDATA(first)->size, SHM_OFFSET );
           tempptr= runptr;
           runptr = runptr + sizeof(QUEUEITEM);
           item->queue_element = runptr - newmem;
           memcpy(runptr, QDATA(QIDATA(first)->queue_element), QIDATA(first)->size);
//         runptr = runptr + QIDATA(first)->size; /* now the runptr is behind the last element */
           runptr = tempptr + addsize;

           /* and the element                                            */
           if(inext)
           {
              (item)->next = runptr - newmem;      /* remember the next item*/
           }
           else
           {
              (item)->next = 0;                    /* remember the next item*/
           }
           first = inext;
         }
         if(last)
         {
#ifdef QUEUE_DBG
           printf("(RxFree)now i copy the last item to the new shared memory\n");
#endif
           queueheader->queue_last = runptr - newmem;
           item = (PQUEUEITEM) runptr;
           memcpy(runptr, QIDATA(last), sizeof(QUEUEITEM));
           addsize = RXROUNDUP(sizeof(QUEUEITEM) + QIDATA(last)->size, SHM_OFFSET );
           tempptr= runptr;
           runptr = runptr + sizeof(QUEUEITEM);
           item->queue_element = runptr - newmem;
           memcpy(runptr, QDATA(QIDATA(last)->queue_element), QIDATA(last)->size);
//         runptr = runptr + QIDATA(last)->size; /* now the runptr is behind the last element */
           runptr = tempptr + addsize;
           item->next = 0;                    /* there is nothing left to do*/
         }
           /* if last = 0 there is no action, because the correct value is taken */
           /* from the copy of the QUEUEHEADER                                   */

           /* the 'next' value of the new queueheader is still wrong, correct it           */
         if(next) /* this is the place in old shared memory, where next queuehaeder is   */
         {
            (queueheader)->next = runptr - newmem;/* here is the place where we can start with the next session queue */
#ifdef QUEUE_DBG
             printf("(RxFree)There is another session queue in shared memory, (queueheader)->next: %d\n",(queueheader)->next);
#endif
         }
         else
         {
#ifdef QUEUE_DBG
            printf("(RxFree)There is no other session queue in shared memory, nothing left to do for session queues, queueheader->next = 0 \n");
#endif
            (queueheader)->next = 0;             /* there is no next queue */
         }
         session_base = next;                   /* take the next queue        */
      }


         /* now the named queues. it is important that the session queue(s) are copied BEFORE */
         /* the named queues. This is because otherwhile the anchor-points (apidata->base and */
         /* apidata->session_base wont fit)                                                   */

      if (base)
      {
          newbase = runptr - newmem;
      }

      while(base!=0)                   /* while there are named queues  */
      {
         next = QHDATA(base)->next;     /* remember the next named queue in old shared mem*/
         queueheader = (PQUEUEHEADER) runptr; /* for creating new next values  */
#ifdef QUEUE_DBG
         printf("(RxFree)now i copy the header of a named queue to the new shared memory. Queuename: %s, item_count: %d \n", QHDATA(base)->queue_name, QHDATA(base)->item_count);
#endif
         memcpy(runptr, QHDATA(base), sizeof(QUEUEHEADER));
         runptr = runptr + sizeof(QUEUEHEADER);
         first = QHDATA(base)->queue_first; /* this is the first element  */
                                            /* of this queue              */
         last = QHDATA(base)->queue_last;   /* this is the last element   */
                                            /* of this queue              */
                                            /* move through item chain    */
         if (first)
         {
           queueheader->queue_first = runptr - newmem;
         }
         else
         {
            queueheader->queue_first = 0;
            queueheader->queue_last = 0;
         }
         while((first) && (first!=last))
         {
            inext = QIDATA(first)->next;     /* remember the next item    */
            /* now let me start with the itemheader                       */
            item = (PQUEUEITEM) runptr;
            memcpy(runptr, QIDATA(first), sizeof(QUEUEITEM));
           addsize = RXROUNDUP(sizeof(QUEUEITEM) + QIDATA(first)->size, SHM_OFFSET );
           tempptr= runptr;
           runptr = runptr + sizeof(QUEUEITEM);
           item->queue_element = runptr - newmem;
           memcpy(runptr, QDATA(QIDATA(first)->queue_element), QIDATA(first)->size);
//         runptr = runptr + QIDATA(first)->size; /* now the runptr is behind the last element */
           runptr = tempptr + addsize;
            if(inext)
            {
               (item)->next = runptr - newmem;      /* remember the next item*/
            }
            else
            {
               (item)->next = 0;                    /* remember the next item*/
            }
            first = inext;
         }
         if(last)
         {
#ifdef QUEUE_DBG
            printf("(RxFree)now i copy the last item to the new shared memory\n");
#endif
            queueheader->queue_last = runptr - newmem;
            item = (PQUEUEITEM) runptr;
            memcpy(runptr, QIDATA(last), sizeof(QUEUEITEM));
           addsize = RXROUNDUP(sizeof(QUEUEITEM) + QIDATA(last)->size, SHM_OFFSET );
           tempptr= runptr;
           runptr = runptr + sizeof(QUEUEITEM);
           item->queue_element = runptr - newmem;
           memcpy(runptr, QDATA(QIDATA(last)->queue_element), QIDATA(last)->size);
//         runptr = runptr + QIDATA(last)->size; /* now the runptr is behind the last element */
           runptr = tempptr + addsize;
         }
         /* if last = 0 there is no action, because the correct value is taken */
         /* from the copy of the QUEUEHEADER                                   */

         /* the 'next' value of the new queueheader is still wrong, correct it           */
         if(next) /* this is the place in old shared memory, where next queuehaeder is   */
         {
            (queueheader)->next = runptr - newmem;/* remember the next named queue */
         }
         else
         {
            (queueheader)->next = 0;             /* there is no next queue */
         }
         base = next;                   /* take the next queue        */
      }
      /* now we have rearranged the new shared memory, let us forget*/
      /* about the old one. Make the new one the one and only       */

      removeshmem(apidata->qbasememId);
      detachshmem(apidata->qbase);
      apidata->qbase = newmem;
      apidata->qbasememId = newmemId;
      apidata->session_base = newsession_base;
      apidata->base = newbase;
      apidata->qmemtop = runptr - newmem; /*rearrange qmemtop         */
      apidata->qmemsize = tempapidatasize;
#ifdef QUEUE_DBG
      printf("I have rearranged memory, apidata->base: %d. apidata->qmemsize: %d, apidata->qmemtop: %d \n", apidata->base, apidata->qmemsize, apidata->qmemtop);
#endif
   }
   return (0);
}
/*THU017A end */
/*********************************************************************/
/* Function:           Release a control block for one of the apis.  */
/*                                                                   */
/*                                                                   */
/* Inputs:             The offset to the block and it's size         */
/*                                                                   */
/* Outputs:            None.                                         */
/*                                                                   */
/*********************************************************************/
void RxFreeAPIBlock(
  ULONG offset, ULONG size )
{
  RxFreeMem(offset,size,SEMEM);
}

/*********************************************************************/
/* Function:           Allocate and fill in an API block.            */
/*                                                                   */
/* Inputs:             Pointers the 3 possible ASCII-Z strings that  */
/*                     may be included in the block and the storage  */
/*                     base.                                         */
/*                                                                   */
/* Outputs:            The allocated APIBLOCK.                       */
/*                                                                   */
/*********************************************************************/
LONG  RxAllocAPIBlock(
  PAPIBLOCK *block,                    /* allocated block            */
  PSZ        name,                     /* api name                   */
  PSZ        dll_name,                 /* name of dll                */
  PSZ        dll_proc)                 /* dll procedure name         */
{
LONG    size;                          /* total allocation size      */
PUCHAR  temp;                          /* used to fill in APIBLOCK   */
ULONG   offset;

 /* Until now the arrays of the names have a constant length of 64   */
 /* characters.                                                      */

  /* Check the sizes!             Simplified the size checks MAE004M */
  if((name && strlen(name) >= MAXNAME) ||
     (dll_name && strlen(dll_name) >= MAXNAME) ||
     (dll_proc && strlen(dll_proc) >= MAXNAME)) {
    printf("\n*E*  API, DLL or procedure name is larger than 63 characters!\n");
    return (1);
  }
  size = APISIZE;                      /* get minimum size           */
  if (RxAllocMem(&offset, size,        /* Request the control block  */
      SEMEM))
    return (1);                        /* Get failed, return error   */
  *block = ((PAPIBLOCK)(apidata->sebase+offset));/* get the ptr      */
  strcpy(((*block)->apiname),name);    /* copy the name into block   */
  if (dll_name) {                      /* if we have a dll_name      */
    strcpy(((*block)->apidll_name),dll_name);/* copy the dll_name too*/
  }
  else
    *((*block)->apidll_name) = 0;      /* otherwise say no dll used  */

  if (dll_proc) {                      /* if we have a dll_proc      */
    strcpy(((*block)->apidll_proc),dll_proc);/* copy the dll_proc too*/
  }
  else
    *((*block)->apidll_proc) = 0;      /* otherwise say no dll used  */
  return (0);                          /* no errors, return nicely   */
}



/*********************************************************************/
/* Function:           Attach the shared memory segment you want to  */
/*                     work with.                                    */
/*                                                                   */
/* Inputs:             Info which chain to attach.                   */
/*                                                                   */
/* Outputs:            none                                          */
/*                                                                   */
/*********************************************************************/
void attachall(int chain){
  PAPIBLOCK current;
//EnterMustComplete(2);           /* enter critical section          */
  PMACRO mcurrent;

  switch (chain) {
    case ALLCHAINS: {
      /* Attach Subcom,Exits and Functions                           */
                                    /* if there is any registration  */
      if((apidata->baseblock[REGSUBCOMM] != 0 )
          || (apidata->baseblock[REGSYSEXIT] != 0 )
          || (apidata->baseblock[REGFUNCTION] != 0 ))
        apidata->sebase = attachshmem(apidata->sebasememId);

      /* Attach the macro space                                        */
      if(apidata->macrocount > 0)    /* if there are macros            */
      {                              /* attach the space               */
        apidata->macrobase = attachshmem(apidata->mbasememId);
      /* Attach the queue space                                        */
                                     /* if there are queues            */
      if(apidata->base != 0 || apidata->session_base != 0)
                                     /* attach the space               */
        apidata->qbase = attachshmem(apidata->qbasememId);
      }
      break;
    }
    case QUEUECHAIN: {
                                        /* if there are queues           */
      if(apidata->base != 0 || apidata->session_base != 0)
                                        /* attach the space              */
        apidata->qbase = attachshmem(apidata->qbasememId);
      break;
    }
    case SECHAIN: {
                                        /* if there is any registration  */
      if((apidata->baseblock[REGSUBCOMM] != 0)
          || (apidata->baseblock[REGSYSEXIT] != 0)
          || (apidata->baseblock[REGFUNCTION] != 0))
        apidata->sebase = attachshmem(apidata->sebasememId);
      break;
    }
    case MACROCHAIN: {
      if(apidata->macrocount > 0)    /* if there are macros              */
      {                                 /* attach the space              */
        apidata->macrobase = attachshmem(apidata->mbasememId);
      }
      break;
    }

  }
//ExitMustComplete();                /* exit critical section        */
}

/*********************************************************************/
/* Function:           Detach all API blocks and macros              */
/*                                                                   */
/* Inputs:             none                                          */
/*                                                                   */
/* Outputs:            none                                          */
/*                                                                   */
/*********************************************************************/
void detachall(int chain){
  PAPIBLOCK current, pdetach;
  PMACRO    mcurrent, mpdetach;
//EnterMustComplete(3);          /* enter critical section.          */


  switch (chain) {
    case ALLCHAINS: {
      /* Detach Subcom,Exits and Fuctions                                */
      if(apidata->sebase != NULL)
        detachshmem(apidata->sebase);
      /* Detach the macro space                                          */
      if(apidata->macrobase != NULL){/* if there are macros              */
        detachshmem(apidata->macrobase);  /* detach the space            */
      }
      /* Detach the queue space                                          */
      if(apidata->qbase != NULL)     /* if there are queues              */
        detachshmem(apidata->qbase); /* detach the space                 */
      break;
    }
    case QUEUECHAIN: {
      if(apidata->qbase != NULL)     /* if there are queues              */
        detachshmem(apidata->qbase); /* detach the space                 */
      break;
    }
    case SECHAIN: {
      if(apidata->sebase != NULL)
        detachshmem(apidata->sebase);
      break;
    }

    case MACROCHAIN: {
      if(apidata->macrobase != NULL){/* if there are macros              */
        detachshmem(apidata->macrobase);  /* detach the space            */
      }
      break;
    }
  }
//ExitMustComplete();                /* exit critical section  */
}

/*********************************************************************/
/* Function:           Exception handler to ensure that the global   */
/*                     semaphore is released if the process          */
/*                     terminates while we are holding the semaphore.*/
/*                     Deregister Process specific registrations     */
/*                     (Subcom/Exe).                                 */
/*                                                                   */
/* Description:        Unconditionally releases and closes the Rexx  */
/*                     API global semaphore.                         */
/*                                                                   */
/* Inputs:             None                                          */
/*                                                                   */
/* Outputs:            None                                          */
/*                                                                   */
/*********************************************************************/

VOID  RxExitClear(INT sig) {

  INT value, i;
  ULONG  ulRC;
  INT used;
  RexxActivity * activity;
  RexxActivationBase * currentActivation;

  if (iCallSigSet == 0 )              /* If siganls nor set          */
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
     iCallSigSet = 5;
//   EnterMustComplete(5);
  }
  /* THU006M begin                                                   */
  if(apidata != NULL){       /* if we have access to the semaphore   */
  /* Check if the semaphore owner exists                             */
    if( (getval((apidata->rexxapisemaphore), 0) == 0) /* if locked   */
       && (kill(semgetpid(apidata->rexxapisemaphore,0), 0) == -1 ) )
//     && ((getpgid(semgetpid(apidata->rexxapisemaphore,0)) == -1)
//     && (errno == ESRCH)))
          unlocksem(apidata->rexxapisemaphore, 0);/* unlock API sem  */
// @MIC Preprocessor error
//#ifdef 0
//   if(((value=getval((apidata->rexxapisemaphore), 0)) > 1)
//      || (getval((apidata->rexxapisemaphore), 0) < NULL)) {
//     if(value > 1){
//       while(value != 1){                    /* unlock the sem      */
//         locksem(apidata->rexxapisemaphore, NULL);
//         --value;
//       }
//     }
//     else if(value < NULL){
//       while(value != 1){                   /* unlock the sem       */
//         unlocksem(apidata->rexxapisemaphore, NULL);
//         value++;
//       }
//     }
//   }
//#endif
// Do not drop own functions for SIGINT because of call on halt .....
//  ulRC = RegDeregFunc( NULL, REGFUNCTION); /* Drop the reg function or */
                                             /* enable for reuse         */
    RxSubcomExitList();        /* remove process specific registrations*/
    locksem(apidata->rexxapisemaphore, 0);
    attachall(QUEUECHAIN);     /* get the queue memeory pool           */
    if(SysQueryThreadID()!=-1) /* if Rexx is up                       */
      Queue_Detach(getpid());  /* remove the session queue             */
    detachall(QUEUECHAIN);     /* release the queue memory pool        */

    if(apidata->rexxutilsems){ /* if we have rexxutil semaphores       */
      for(int i=0;i<MAXUTILSEM;i++){/* for all possible semaphores     */
        if((opencnt[i][0])){   /* if I have it open                    */
          for(int k=0;k<(opencnt[i][0]);k++){/* for every open I've made */
            --((apidata->utilsemfree[i]).usecount);/*decrement usecount*/
            if(!(apidata->utilsemfree[i]).usecount){/*sem now unused ? */
                               /* clear the name array                 */
              memset((apidata->utilsemfree[i]).name, 0, MAXNAME);
              /* make sure the sem  is in a clear state                */
              init_sema(apidata->rexxutilsems, i);
            }
          }
        }
      }
      /* Possibly this was the last used sem. So we can remove the       */
      /* semaphore set. Check this possibility.                          */
      used = 0;
      for(int j=0;j<MAXUTILSEM;j++)
      {                                           /* for all semaphores  */
         if((apidata->utilsemfree[j]).usecount != 0 )/* a used one ?     */
           used = 1;                /* remember it                */
      }
      if(!used)
      {                                    /* if all sems are unused     */
        removesem(apidata->rexxutilsems);  /* remove the semaphore set   */
        apidata->rexxutilsems = 0;         /* delete the old ID          */
      }
    }
    unlocksem(apidata->rexxapisemaphore, 0 );

  } /* if apidata not equal NULL */

#ifdef ORXAP_DEBUG
              switch(sig){
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


              /* start signal handling when semaphore handling is done       */
/*
 *            if (sig == SIGINT)
 *            {
 *               for (i=ProcessLocalActs->first();ProcessLocalActs->available(i);i=ProcessLocalActs->next(i))
 *               {
 *                  activity = (RexxActivity *)(ProcessLocalActs->value(i));
 *
 *                  currentActivation = (RexxActivationBase *) activity->currentAct();
 *
 *                  if (currentActivation != (RexxActivationBase *)TheNilObject)
 *                  {
 *                       ((RexxActivation *)currentActivation)->halt(OREF_NULL);
 *                  }
 *               }
 *               return;
 *            }
 *            else
 *               exit(0);
*/

#ifdef ORXAP_DEBUG
  switch(sig){
    case (SIGINT):
      printf("\n*** Rexx interrupted.\n");
      break;
    case (SIGTERM):
      printf("\n*** Rexx terminated.\n*** Closing Rexx !\n");
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

  if (sig == SIGINT)                    /* special for interrupt             */
  {
     for (i=ProcessLocalActs->first();ProcessLocalActs->available(i);i=ProcessLocalActs->next(i))
     {
        activity = (RexxActivity *)(ProcessLocalActs->value(i));

        currentActivation = (RexxActivationBase *) activity->currentAct();

        if (currentActivation != (RexxActivationBase *)TheNilObject)
        {
                                        /* Yes, issue the halt to it.        */
             ((RexxActivation *)currentActivation)->halt(OREF_NULL);
        }
     }
    if (iCallSigSet == 5 )                     /* Signal set                 */
    {
#if defined( HAVE_SIGPROCMASK )
       sigprocmask( SIG_SETMASK, &oldmask , NULL );
#elif defined( HAVE_SIGHOLD )
       sigrelse(SIGINT);
       sigrelse(SIGTERM);
       sigrelse(SIGILL);
       sigrelse(SIGSEGV);
#endif
      iCallSigSet = 0;
//   ExitMustComplete(51);
     }
     return;
  }
  else
  {
    if (iCallSigSet == 5 )                     /* Signal set                 */
    {
#if defined( HAVE_SIGPROCMASK )
       sigprocmask( SIG_SETMASK, &oldmask , NULL );
#elif defined( HAVE_SIGHOLD )
       sigrelse(SIGINT);
       sigrelse(SIGTERM);
       sigrelse(SIGILL);
       sigrelse(SIGSEGV);
#endif
       iCallSigSet = 0;
//     ExitMustComplete(52);
     }
     exit(0);
  }
}
    /* THU006M end                                                           */

/*********************************************************************/
/* Function:           Exception handler to ensure that the global   */
/*                     semaphore is released if the process          */
/*                     terminates while we are holding the semaphore.*/
/*                     Deregister Process specific registrations     */
/*                     (Subcom/Exe).                                 */
/*                                                                   */
/* Description:        The same as the RxExitClear fuction but       */
/*                     without parameter                             */
/*                                                                   */
/* Inputs:             None                                          */
/*                                                                   */
/* Outputs:            None                                          */
/*                                                                   */
/*********************************************************************/

VOID  RxExitClearNormal() {

  INT value;
  ULONG  ulRC;
  INT used;

  if (iCallSigSet == 0 )                     /* Set signals          */
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
     iCallSigSet = 6;
//   EnterMustComplete();
  }
  if(apidata != NULL){       /* if we have access to the semaphore   */
/* Check if the semaphore owner exists                               */
    if((getval((apidata->rexxapisemaphore), 0) == 0 ) /* if locked   */
       && (kill(semgetpid(apidata->rexxapisemaphore,0), 0) == -1 ) )
//     && ((getpgid(semgetpid(apidata->rexxapisemaphore,0)) == -1)
//     && (errno == ESRCH)))
        unlocksem(apidata->rexxapisemaphore, 0);/* unlock API sem    */
// @MIC Preprocessor error
//#ifdef 0
//   if(((value=getval((apidata->rexxapisemaphore), 0)) > 1)
//      || (getval((apidata->rexxapisemaphore), 0) < NULL)) {
//     if(value > 1){
//       while(value != 1){                    /* unlock the sem      */
//         locksem(apidata->rexxapisemaphore, NULL);
//         --value;
//       }
//     }
//     else if(value < NULL){
//       while(value != 1){                   /* unlock the sem       */
//         unlocksem(apidata->rexxapisemaphore, NULL);
//         value++;
//       }
//     }
//   }
//#endif

    ulRC = RegDeregFunc( NULL, REGFUNCTION); /* Drop the reg function or */
                                             /* enable for reuse         */
    RxSubcomExitList();        /* remove process specific registrations*/
    locksem(apidata->rexxapisemaphore, 0 );
    attachall(QUEUECHAIN);     /* get the queue memeory pool           */
    if(SysQueryThreadID()!=-1) /* if Rexx is up                        */
      Queue_Detach(getpid());  /* remove the session queue             */
    detachall(QUEUECHAIN);     /* release the queue memory pool        */

    if(apidata->rexxutilsems){ /* if we have rexxutil semaphores       */
      for(int i=0;i<MAXUTILSEM;i++){/* for all possible semaphores     */
        if((opencnt[i][0])){   /* if I have it open                    */
          for(int k=0;k<(opencnt[i][0]);k++){/* for every open I've made */
            --((apidata->utilsemfree[i]).usecount);/*decrement usecount*/
            if(!(apidata->utilsemfree[i]).usecount){/*sem now unused ? */
                               /* clear the name array                 */
              memset((apidata->utilsemfree[i]).name, 0, MAXNAME);
              /* make sure the sem  is in a clear state                */
              init_sema(apidata->rexxutilsems, i);
            }
          }
        }
      }
      /* Possibly this was the last used sem. So we can remove the       */
      /* semaphore set. Check this possibility.                          */
      used = 0;
      for(int j=0;j<MAXUTILSEM;j++)
      {                                           /* for all semaphores        */
         if((apidata->utilsemfree[j]).usecount != 0 ) /* a used one ?   */
           used = 1;                /* remember it                */
      }
      if(!used)
      {                                    /* if all sems are unused     */
        removesem(apidata->rexxutilsems);  /* remove the semaphore set   */
        apidata->rexxutilsems = 0;         /* delete the old ID          */
        unlocksem(apidata->rexxapisemaphore, 0 );
        if(RexxTerminated)
          EVPOST(RexxTerminated);
        if (iCallSigSet == 6 )     /* If MICMIC lin22                      */
        {
#if defined( HAVE_SIGPROCMASK )
          sigprocmask( SIG_SETMASK, &oldmask , NULL );
#elif defined( HAVE_SIGHOLD )
          sigrelse(SIGINT);
          sigrelse(SIGTERM);
          sigrelse(SIGILL);
          sigrelse(SIGSEGV);
#endif
          iCallSigSet = 0;
//      ExitMustComplete();
        }
        return;
      }
    }
    unlocksem(apidata->rexxapisemaphore, 0 );
    if(RexxTerminated)
      EVPOST(RexxTerminated);
  }
   if (iCallSigSet == 6 )        /* If MICMIC lin22                        */
   {
#if defined( HAVE_SIGPROCMASK )
      sigprocmask( SIG_SETMASK, &oldmask , NULL );
#elif defined( HAVE_SIGHOLD )
      sigrelse(SIGINT);
      sigrelse(SIGTERM);
      sigrelse(SIGILL);
      sigrelse(SIGSEGV);
#endif
      iCallSigSet = 0;
//  ExitMustComplete();
  }
}

/*********************************************************************/
/* Function:          EnterMustComplete                              */
/*                                                                   */
/* Description:       Set the signal mask of the process to block    */
/*                    all signals (not the errors)                   */
/*                                                                   */
/* Inputs:             None                                          */
/*                                                                   */
/* Outputs:            None                                          */
/*                                                                   */
/*********************************************************************/
void EnterMustComplete()
{
#ifndef ORXNO_MUSTDEB
  sigset_t sigSet;
  sigfillset(&sigSet);                /* fill the set for all signals*/
  sigdelset(&sigSet, SIGSEGV);        /* but not for error signals   */
  sigdelset(&sigSet, SIGFPE);
  sigdelset(&sigSet, SIGILL);
  sigdelset(&sigSet, SIGBUS);

  sigprocmask(SIG_SETMASK, &sigSet, NULL); /* set the mask           */
#endif
}

/*********************************************************************/
/* Function:          ExitMustComplete                               */
/*                                                                   */
/* Description:       Set the signal mask of the process to get      */
/*                    all signals                                    */
/*                                                                   */
/* Inputs:             None                                          */
/*                                                                   */
/* Outputs:            None                                          */
/*                                                                   */
/*********************************************************************/

void ExitMustComplete()
{
#ifndef ORXNO_MUSTDEB
  sigset_t sigSet;
  sigemptyset(&sigSet);               /* fill the set for all signals*/
  sigprocmask(SIG_SETMASK, &sigSet, NULL); /* set the mask           */
#endif
}

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxShutDownAPI                                 */
/*                                                                   */
/*  Description:     Deletes all shared memory blocks if there is    */
/*                   no process which has currently attached the     */
/*                   API anchor memory. The API semaphore will not   */
/*                   be deleted. It stays until Linux goes down.     */
/*                                                                   */
/*                                                                   */
/*  Entry Point:     RexxShutDownAPI                                 */
/*                                                                   */
/*  Parameter(s):    none                                            */
/*                                                                   */
/*  Return Value:    0 - shutdown worked well                        */
/*                   1 - shutdown not possible now (memory in use)   */
/*                                                                   */
/*********************************************************************/
APIRET  APIENTRY RexxShutDownAPI(void)
{
  int semId;                           /* API semaphore ID            */
  PAPIBLOCK current;
  PMACRO mcurrent;
  shmid_ds buf;                        /* to hold the shmem info      */
  int      ShmemId;                    /* ID of the API anchor block  */
  key_t    ipckey;
  LONG     lRC = 0;
  LONG temp_cblock;
  LONG temp_nblock;
  ULONG ulBlockSize;
  ULONG  ulQcurrent  = 0;             /* Current queue element        */
  ULONG  ulQprevious = 0;             /* Previous queue element       */
  ULONG  ulQnext     = 0;             /* Next queue element           */

  if ( achRexxHomeDir[0] != '/' )
  {
    lRC = RxAPIHOMEset();              /* Set the REXX HOME           */
    if ( lRC )
       return(1);                      /* Exit anyway                 */
  }
  ipckey = ftok(achRexxHomeDir, 'r');  /* generate a unique key       */
  if ( ipckey == -1)                   /* No key error                */
  {
     perror(" *E*  No key generated for shared memory");
     return(1);                        /* Stop anyway                 */
  }
                                       /* Get the anchor block ID     */
  ShmemId = openshmem(ipckey, sizeof(REXXAPIDATA));
  if(ShmemId == -1)
    return (1);                        /* Shutdown not possible       */
                                       /* Get the info about the block*/
  if(shmctl(ShmemId, IPC_STAT, &buf))
    return (1);                        /* Shutdown not possible       */
                                       /* If there is no other process*/
                                       /* which has attached the API  */
  if(buf.shm_nattch <= 1)              /* data, start the shutdown    */
  {
//  EnterMustComplete(16);
    APISTARTUP(ALLCHAINS);
    /* Check whether a PID still exists for rexx queues -------------------- */

    if ( apidata->session_base )
    {
      ulQcurrent = apidata->session_base;  /* get current base pointer */
      attachall(QUEUECHAIN);               /* get the queue memory     */

      /* look for active queue                                         */
      while (ulQcurrent && !lRC )    /* while more queues              */
      {
        ulQnext = QHDATA(ulQcurrent)->next;
        if( (QHDATA(ulQcurrent)->queue_session != 0 ) &&
            (kill(QHDATA(ulQcurrent)->queue_session, 0 ) == 0 ) &&
            (QHDATA(ulQcurrent)->queue_session != getpid() ) )
        {
           fprintf(stderr," *E*  A REXX procedure (PID: %d) is still running!\n",
                   QHDATA(ulQcurrent)->queue_session);
           lRC = -1;
        }
        ulQcurrent = ulQnext;        /* step to the next block         */
      }
    }

    if (!lRC)
    {
      /* Check whether a PID still exists for rexx registered functions ------ */
      ulBlockSize = sizeof(APIBLOCK);
      temp_cblock = apidata->sememtop;
      temp_nblock = temp_cblock - ulBlockSize;
      while ( (temp_cblock > SHM_OFFSET) && !lRC )
      {
        temp_cblock  -= ulBlockSize;   /* Set the current block offset */
        if ( temp_cblock > ulBlockSize )
          temp_nblock  -= ulBlockSize; /* Set the next    block offset */
        else
          temp_nblock  = 0;            /* Set the next    block offset */

        if ( (SEDATA(temp_cblock)->apipid != 0)  &&
             (kill(SEDATA(temp_cblock)->apipid, 0) == 0) )
        {
           fprintf(stderr," *E*  A REXX procedure (PID: %d) is still running!\n",
                   SEDATA(temp_cblock)->apipid);
           lRC = -1;
        }
      }
    }
    if (!lRC)
    {
      /*Mark the API chains as removalble, they will be removed as soon  */
      /* as no process has attached them                                 */
      if(apidata->sebase != NULL)     /* if there are registartions      */
        removeshmem(apidata->sebasememId);
      /*Mark the Macro chain as removalble, they will be removed as soon */
      /* as no process has attached them                                 */
                                      /* if there are registrated macros */
      if(apidata->macrobase != NULL){
        removeshmem(apidata->mbasememId);
      }
      /*Mark the Queue chain as removalble, they will be removed as soon */
      /* as no process has attached them                                 */
                                      /* if there are registrated queues */
      if(apidata->qbase != NULL){
        removeshmem(apidata->qbasememId);
      }
      detachall(ALLCHAINS);              /* release the chains           */
      if(apidata->rexxutilsems)          /* if there are util semaphores */
        removesem(apidata->rexxutilsems);/* remove them                  */
      semId = apidata->rexxapisemaphore; /* remember the semaphore ID    */
      /* Now mark the anchor block as removable                          */
      removeshmem(ShmemId);
      /* detach the anchor to force the system to delete it              */
      detachshmem((char*)apidata);
      apidata = NULL;

//    unlocksem(semId, NULL);
      removesem(semId);

#if defined( HAVE_SIGPROCMASK )
      sigprocmask( SIG_SETMASK, &oldmask , NULL );
#elif defined( HAVE_SIGHOLD )
      sigrelse(SIGINT);
      sigrelse(SIGTERM);
      sigrelse(SIGILL);
      sigrelse(SIGSEGV);
#endif
      iCallSigSet = 0;
//    ExitMustComplete(60);
      return (0);                        /* shutdown worked well        */
    }
  }
  if (lRC < 0 )
  {
    semId = apidata->rexxapisemaphore;  /* Get the semaphoreID          */
    unlocksem(semId, 0);
  }
#if defined( HAVE_SIGPROCMASK )
  sigprocmask( SIG_SETMASK, &oldmask , NULL );
#elif defined( HAVE_SIGHOLD )
  sigrelse(SIGINT);
  sigrelse(SIGTERM);
  sigrelse(SIGILL);
  sigrelse(SIGSEGV);
#endif
  iCallSigSet = 0;
//ExitMustComplete(61);
  return (1);                            /* shutdown not possible       */

}
/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxAllocateMemory                              */
/*                                                                   */
/*  Description:     Operating system independant method to          */
/*                   allocate memory. The function is a wrapper      */
/*                   for appropriate compiler or operating system    */
/*                   memory function.                                */
/*                                                                   */
/*                                                                   */
/*  Entry Point:     RexxAllocateMemory                              */
/*                                                                   */
/*  Parameter(s):    size of memory to allocate (ULONG)              */
/*                                                                   */
/*  Return Value:    The allocated Block of memory (PVOID)           */
/*                                                                   */
/*********************************************************************/
PVOID APIENTRY RexxAllocateMemory(ULONG size)
{
   PVOID tmpPtr;
   tmpPtr = (void *) malloc(size);
   return tmpPtr;
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxFreeMemory                                  */
/*                                                                   */
/*  Description:     Operating system independant method to          */
/*                   free memory. The function is a wrapper          */
/*                   for appropriate compiler or operating system    */
/*                   memory function.                                */
/*                                                                   */
/*                                                                   */
/*  Entry Point:     RexxFreeMemory                                  */
/*                                                                   */
/*  Parameter(s):    size of memory to allocate (ULONG)              */
/*                                                                   */
/*  Return Value:    The allocated Block of memory (PVOID)           */
/*                                                                   */
/*********************************************************************/
ULONG RexxFreeMemory(PVOID ptr)
{
   free(ptr);
   return 0;
}

#ifdef __cplusplus
}
#endif
