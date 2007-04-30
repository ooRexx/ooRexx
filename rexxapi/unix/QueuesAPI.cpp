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
/* REXX for UNIX                                                    aixqapi.c */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/*  Program Name:       AIXQAPI.C                                             */
/*                                                                            */
/*  Description:        Library to contain API functions for                  */
/*                      REXX-SAA/PL queueing services.                        */
/*                                                                            */
/*  Entry Points:   LONG  APIENTRY RexxCreateQueue()-create a queue           */
/*                  LONG  APIENTRY RexxDeleteQueue()-destroy a                */
/*                      queue                                                 */
/*                  LONG  APIENTRY RexxQueryQueue() -query a queue            */
/*                  LONG  APIENTRY RexxAddQueue()   -add data                 */
/*                  LONG  APIENTRY RexxPullQueue()  -retrieve data            */
/*                  LONG  APIENTRY RexxInitDataQueueInit()-start              */
/*                       queuing system                                       */
/*                                                                            */
/******************************************************************************/
/* Please note the following:                                                 */
/*                                                                            */
/* Functions in this module manipulate data that must be available            */
/* to all processes that call REXX API functions.  These processes            */
/* may invoke the REXX interpreter, or make direct calls to the               */
/* API routines.                                                              */
/*                                                                            */
/* In addition, functions in this module may establish data that              */
/* must persist after the calling process terminates.                         */
/*                                                                            */
/* To satisfy these requirements, the system must maintain a process          */
/* that serves as a data repository.  Functions in this module then           */
/* give critical data to the repository, and the data persists as             */
/* long as the repository process continues running.                          */
/*                                                                            */
/******************************************************************************/

#define INCL_RXQUEUE
#define INCL_RXSUBCOM
#include "PlatformDefinitions.h"
#include SYSREXXSAA
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/sem.h>
#include <errno.h>
#include "RexxCore.h"
#include "SharedMemorySupport.h"
#include "APIDefinitions.h"
#include "RexxAPIManager.h"
#include "APIUtilities.h"
#include "ASCIISymbols.h"
#if defined(OPSYS_SUN)
#include <signal.h>
#endif
#define ALREADY_INIT      1    /* indicator of queue manager status  */
#define MAXQNUM         999    /* allow queues numbered 1 thru 999   */
#define MAXDATASIZE  0xffc0    /* maximum size of data elements      */
#define MAXSIZE       65536    /* maximum memory size.               */
#define RESET             0    /* used for DosSubSet() call          */
#define INITIALIZE        1    /* used for DosSubSet() call          */
#define SPARSE            4    /* used for DosSubSet() call          */
#define WAIT             -1L   /* used for DosSemxxx() calls         */
#define YES               1
#define NO                0
#define MAX_NAME_LENGTH   63   /* Longest permissable queue name.    */
#define QUEUE_BUF_SIZE    251  /* Longest queue name + 1.            */
#define MAXTRYS           50   /* Max retries on error.              */
#define SLEEPTIME         100L /* # MS to sleep between retries.     */
#define INTERNAL_NAME_SIZE  14         /* size of internal queue name*/

#define LOCAL             1

extern int errno;
extern REXXAPIDATA  *apidata;  /* Global state data                  */

/*********************************************************************/
/*                Function prototypes for local routines.            */
/*********************************************************************/

static LONG   val_queue_name(PSZ);
static ULONG  search(PSZ);
static LONG   queue_allocate(PSZ, PULONG);
static INT    create_queue_sem(ULONG);
static VOID delete_queue_sem(ULONG);
static VOID   release_queue_item(ULONG, INT, ULONG);
static LONG   allocate_queue_entry(ULONG,PULONG,PCHAR);
VOID GetDateTime(PDATETIME);
ULONG CreateMutexSem(INT *);
ULONG CreateEventSem(INT *);
VOID CloseMutexSem(INT);
VOID CloseEventSem(INT);
VOID PostEventSem(INT);
ULONG RequestMutexSem(INT,INT);
VOID ResetEventSem(INT);
ULONG WaitEventSem(INT,INT);
VOID ReleaseMutexSem(INT);

CHAR rxqueue_name_mask[]="S%02dQ%010u";

/*********************************************************************/
/*                                                                   */
/*  Function:        val_queue_name()                                */
/*                                                                   */
/*  Description:     Validate queue name.  Translate the name into   */
/*                   upper case.   Return translated queue name.     */
/*                                                                   */
/*                   If the input name is NULL, return NULL.         */
/*                                                                   */
/*  Function:        If the name is NULL, it is valid.  Otherwise,   */
/*                   it must be non-empty, and less than or equal    */
/*                   to 250 characters long.  The only characters    */
/*                   permitted in a queue name are '0' ... '9',      */
/*                   'A' ... 'Z', '.', '!', '?', and '_'.            */
/*                                                                   */
/*  Input:           queue name, buffer for validated name.          */
/*                                                                   */
/*  Output:          return code indicating validity                 */
/*                                                                   */
/*********************************************************************/
static LONG   val_queue_name(
  PSZ   usrname)                       /* User's name.               */
{
   LONG        rc;                     /* Return code.               */
   PSZ         valptr;                 /* Used to validate name.     */
   char        ch;
   ULONG       namelen;                /* Length of the user's name. */

   if (!usrname)                       /* NULL is OK.                */
      return (1);                      /* return code indication     */

   namelen = strlen(usrname);
   if (rc = ((0 < namelen) &&
       (namelen <= MAX_NAME_LENGTH) ) ) {

     valptr = usrname;                 /* point to name              */
     while(rc && (ch = *(valptr++))) { /* While have not reached end */
       ch = toupper(ch);               /* convert to upper case      */
       rc = ((ch >= 'A' && ch <= 'Z')||/* make sure the name is valid*/
            (isdigit(ch)) ||
            (ch == ch_PERIOD) ||
            (ch == ch_QUESTION_MARK) ||
            (ch == ch_EXCLAMATION) ||
            (ch == ch_UNDERSCORE));
     }
   }
   return(rc);
}


/*$PE*/
/*********************************************************************/
/*                                                                   */
/*  Function:        search()                                        */
/*                                                                   */
/*  Description:     Return a pointer to a named queue in the        */
/*                   current list.                                   */
/*                                                                   */
/*                                                                   */
/*  Function:        Search the list of currently created queues     */
/*                   for a queue with a given name.  Returns null    */
/*                   if the queue is not found.                      */
/*                                                                   */
/*  Input:           Name of queue to locate                         */
/*                                                                   */
/*  Output:          Address of the queue header.  Null if not found */
/*                                                                   */
/*********************************************************************/
static ULONG  search(
  PSZ  name)
{
ULONG current;                         /* Current queue element      */
ULONG previous;                        /* Previous queue element     */

  previous = NULL;                     /* no previous yet            */
  current = apidata->base;             /* get current base pointer   */

  while (current) {                    /* while more queues          */
                                       /* if we have a match         */
    if (!rxstricmp(name,QHDATA(current)->queue_name)) {
      if (previous) {                  /* if we have a predecessor   */
//      EnterMustComplete();           /* make sure we can complete  */
        QHDATA(previous)->next =       /* rearrange the chain to     */
          QHDATA(current)->next;       /* move this to the front     */
        QHDATA(current)->next = apidata->base;
        apidata->base = current;
//      ExitMustComplete();            /* end of must complete part  */
      }
      return (current);                /* we are finished            */
    }
    previous = current;                /* remember this block        */
    current = QHDATA(current)->next;   /* step to the next block     */
  }
  return (NULL);                       /* not found, tell caller so  */
}


/*$PE*/
/*********************************************************************/
/*                                                                   */
/*  Function:       queue_allocate()                                 */
/*                                                                   */
/*  Description:    Allocate storage for a new queue.                */
/*                                                                   */
/*  Function:       Allocates storage for the queue.  Makes up       */
/*                  the internal name for the queue.  Allocates      */
/*                  the equate record that links the internal name   */
/*                  to the external name.                            */
/*                                                                   */
/*  Notes:          Cleans up after itself if the allocation for     */
/*                  the equate record fails.                         */
/*                                                                   */
/*  Input:          Name of queue (which will be null if a new name  */
/*                  needs to be created)                             */
/*                                                                   */
/*  Output:         Pointer to new queue header.                     */
/*                                                                   */
/*  Effects:        Storage allocated.                               */
/*                                                                   */
/*********************************************************************/
static LONG   queue_allocate(
  PSZ                 name,            /* External queue name.       */
  PULONG       pnew)                   /* New queue header (returned)*/
{
   ULONG  size;                        /* size to allocate           */
   ULONG tag;                          /* unique queue identifier    */

  size = sizeof(QUEUEHEADER);
  /* Allocate the header record.  If it fails, terminate processing. */

   apidata->SessionId = getpgid(0);   /* get the session id          */
   if (!RxAllocMem(pnew,size,QMEM)) {

      if (create_queue_sem(*pnew)) {  /* create queue semaphores     */
                                      /* release the header          */
        RxFreeMemQue(*pnew, size, QMEMNAMEDQUE, *pnew);
        return 1;                     /* have an error               */
      }

      if (!name) {                    /* If no name                  */
        tag = (ULONG)QDATA(*pnew);    /* get value of pointer        */
        name = QHDATA(*pnew)->queue_name;/* new name will be buffer  */
        for (;;) {                    /* now create a unique name    */
          sprintf(name,               /* create a new queue name     */
                  rxqueue_name_mask,  /* from the session id and the */
                  apidata->SessionId, /* address of the control block*/
                  (ULONG)tag);
                                      /* if unique, we're done       */
          if (!search(name))
            break;                    /* get out                     */
          tag++;                      /* try a new number            */
        }
      }
      else {
                                      /* make it uppercase           */
                                      /* Do not uppercase the given space */
//        for(int j=0;j<(strlen(name));j++)
//          *(name+j) = toupper(*(name+j));
                                      /* copy the name over          */
        strcpy(QHDATA(*pnew)->queue_name,name);
                                      /* make it uppercase           */
        for(int j=0;j<(strlen(name));j++)
          *((QHDATA(*pnew)->queue_name)+j) = toupper(*((QHDATA(*pnew)->queue_name)+j));
      }
      return (0);                     /* worked fine                 */
   }

   return(1);                         /* have an error               */
}


/*$PE*/
/*********************************************************************/
/*                                                                   */
/*  Function:        create_queue_sem                                */
/*                                                                   */
/*  Description:     Create synchroniztion semaphores for a Rexx     */
/*                   queue.                                          */
/*                                                                   */
/*                                                                   */
/*  Function:        Create a mutex semaphore and an event semaphore */
/*                   for a newly created queue.                      */
/*                                                                   */
/*  Input:           Queue header block                              */
/*                                                                   */
/*  Output:          Success/failure return code.                    */
/*                                                                   */
/*********************************************************************/
static INT create_queue_sem(
    ULONG   queue)                     /* new queue                  */
{
                                       /* create a mutex sem         */
  if (CreateMutexSem(&(QHDATA(queue)->enqsem)))
    return (RXSUBCOM_NOEMEM);          /* call this a memory error   */

                                       /* and an event sem           */
  if (CreateEventSem(&(QHDATA(queue)->waitsem))) {
    CloseMutexSem(QHDATA(queue)->enqsem);
    return (RXSUBCOM_NOEMEM);          /* call this a memory error   */
  }
  ResetEventSem(QHDATA(queue)->waitsem);        /*THU008A */
  return (0);
}

/*$PE*/
/*********************************************************************/
/*                                                                   */
/*  Function:        release_queue_item()                            */
/*                                                                   */
/*  Description:     releases an allocated queue item, returning     */
/*                   the storage to the appropriate place.           */
/*                                                                   */
/*  Function:        The queue header is released to the control     */
/*                   block pool.  The queue element is also released,*/
/*                   with RxFreeMem doing all the hard work.         */
/*                                                                   */
/*  Input:           Pointer to the queue entry control              */
/*                   block.                                          */
/*                                                                   */
/*  Output:          None.                                           */
/*                                                                   */
/*  Effects:         Memory deleted.                                 */
/*                                                                   */
/*********************************************************************/
/*THU009C begin */
static VOID   release_queue_item(
  ULONG   item, INT flag, ULONG current)                        /* queue item to release      */
{
  /* Release header and data together to increase performance of the */
  /* memory manager                                                  */


  if (QIDATA(item)->size)              /* empty entry ?              */
  {                              /* delete header and data            */
//   RxFreeMemQue(item,sizeof(QUEUEITEM)+QIDATA(item)->size,flag, current);
     RxFreeMemQue(item,RXROUNDUP(sizeof(QUEUEITEM)+QIDATA(item)->size, SHM_OFFSET),flag, current);
  }
  else                                       /* no data !            */
  {                                    /* just delete header          */
//   RxFreeMemQue(item, sizeof(QUEUEITEM), flag, current);
     RxFreeMemQue(item, RXROUNDUP(sizeof(QUEUEITEM),SHM_OFFSET), flag, current); /*THU012A */
  }
}

/*********************************************************************/
/*                                                                   */
/*  Function:        delete_queue_sem                                */
/*                                                                   */
/*  Description:     Delete synchroniztion semaphores for a Rexx     */
/*                   queue.                                          */
/*                                                                   */
/*                                                                   */
/*  Function:        Send a message to REXXINIT.DLL to close the two */
/*                   semaphores for this queue, then close the queues*/
/*                   on this side also.                              */
/*                                                                   */
/*  Input:           Queue header block                              */
/*                                                                   */
/*  Output:          None                                            */
/*                                                                   */
/*********************************************************************/
static VOID delete_queue_sem(
    ULONG   queue)                     /* deleted queue              */
{
                                       /* delete the mutex sem       */
  CloseMutexSem(QHDATA(queue)->enqsem);
                                       /* and the event sem          */
  CloseEventSem(QHDATA(queue)->waitsem);
  return;
}


/*********************************************************************/
/*                                                                   */
/*  Function:         queue_detach()                                 */
/*                                                                   */
/*  Description:      Delete a session queue.                        */
/*                                                                   */
/*  Function:         Delete all entries in a queue, then delete     */
/*                    the queue header.                              */
/*                                                                   */
/*                                                                   */
/*  Input:            current - queue ptr.                           */
/*                                                                   */
/*  Effects:          Queue and all its entries deleted.             */
/*                                                                   */
/*********************************************************************/
void  queue_detach(
  ULONG current)                       /* queue to delete            */
{
  ULONG previous;
  ULONG curr_item;
  ULONG next_item;
  ULONG temp;

  if(apidata == NULL)                 /* nothing happend at all      */
    return;

/* let me free the items of the session queue of the died process          */

  curr_item = QHDATA(current)->queue_first;
  while(curr_item != 0)
  {
     next_item = QIDATA(curr_item)->next;
     release_queue_item(curr_item, QMEMSESSION, current);
     curr_item = next_item;
  }

/* and now the queue header                                            */

  delete_queue_sem(current);       /* get rid of semaphores      */
  RxFreeMemQue(current,               /* get rid of queue header    *//* always session queue */
            sizeof(QUEUEHEADER),
            QMEMSESSION, current);

  if(!(apidata->base) && !(apidata->session_base))
  {
    delete_queue_sem(current);      /* free the semaphores         */
    removeshmem(apidata->qbasememId);/* remove the queue mem pool  */
    detachshmem(apidata->qbase);    /* force the deletion          */
    apidata->qbase == NULL;         /* reset the memory pointer    */
    apidata->qmemsizeused = 1;
  }
  else                     /* we have still Named or/and Sessionqueues in shared memory */
  {
    CheckForMemory(); /* give memory decreasing algorithm a chance.      */
  }                   /* Be aware CheckForMemory rearrangement algorithm */
  /* It's possible that the process terminates until a RexxPullQueue */
  /* command is waiting on a queue. So let's clean up the semaphores */
  /* and the wait count of the appropriate queue.                    */
  /* don't forget to reinitialize the local copies of apidata values */

                                       /* first for the named queues */
  temp = apidata->base;             /* get the anchor             */
  while(temp)
  {
    if((QHDATA(temp)->waiting > 0 ) /* if somebody is waiting        */
       &&(QHDATA(temp)->waitprocess == (QHDATA(current)->waitprocess)))/* and I'm the one*/
    {
      /* clear out the mutex sem of the queue                        */
                                       /* do the initialisation      */
      init_sema(apidata->rexxapisemaphore, QHDATA(temp)->enqsem);
      QHDATA(temp)->waiting--;      /* decrement the counter      */
    }
    temp = QHDATA(temp)->next;   /* test the next queue        */
  }
                                       /* now for the session queues */
  temp = apidata->session_base;     /* get the anchor             */
  while(temp)
  {
    if((QHDATA(temp)->waiting > 0 ) /* if somebody is waiting         */
       &&(QHDATA(temp)->waitprocess == (QHDATA(current)->waitprocess)))/* and I'm the one*/
    {
      /* clear out the mutex sem of the queue                        */
                                       /* do the initialisation      */
      init_sema(apidata->rexxapisemaphore, QHDATA(temp)->enqsem);
      QHDATA(temp)->waiting--;      /* decrement the counter      */
    }
    temp = QHDATA(temp)->next;   /* test the next queue        */
  }
  return;
}

/*********************************************************************/
/*                                                                   */
/*  Function:        search_session;                                 */
/*                                                                   */
/*  Description:     Return a pointer to the default session queue   */
/*                   for the current session.                        */
/*                                                                   */
/*                                                                   */
/*  Function:        Search the list of session queues for the       */
/*                   correct queue.                                  */
/*                                                                   */
/*  Input:           None.                                           */
/*                                                                   */
/*  Output:          Address of the queue header.                    */
/*                                                                   */
/*********************************************************************/
ULONG  search_session(VOID)
{
ULONG        current;                  /* Current queue element      */
ULONG        previous;                 /* Previous queue element     */
ULONG        next;

  previous = NULL;                     /* no previous yet            */
  current = apidata->session_base;     /* get current base pointer   */
  apidata->SessionId = getpgid(0);     /* get the session id         */

  /* look for queue garbage                                          */
  while (current) {                    /* while more queues          */
    next = QHDATA(current)->next;
//  if((getpgid(QHDATA(current)->queue_session) == -1) && (errno == ESRCH))
    if( kill(QHDATA(current)->queue_session, 0 ) == -1 )
    {
       queue_detach(current);
    }
    current = next;                    /* step to the next block     */
  }

  current = apidata->session_base;     /* get current base pointer   */

  while (current) {                    /* while more queues          */
                                       /* if we have a match         */
    if (QHDATA(current)->queue_session == apidata->SessionId) {
//      if (previous) {                  /* if we have a predecessor   */
//        QHDATA(previous)->next =       /* rearrange the chain to     */
//          QHDATA(current)->next;       /* move this to the front     */
//        QHDATA(current)->next = apidata->session_base;
//        apidata->session_base = current;
//    }
//      printf("now ending function SEARCH_SESSION \n");
      return (current);                /* we are finished            */
    }
    previous = current;                /* remember this block        */
    current = QHDATA(current)->next;   /* step to the next block     */
  }
                                       /* not found, create one      */
  if (!RxAllocMem(&current,sizeof(QUEUEHEADER),
      QMEM)) {
    if (create_queue_sem(current)) {   /* create queue semaphores    */
                                       /* release the header         */
      RxFreeMemQue(current, sizeof(QUEUEHEADER), QMEMSESSION, current);  /* this is always session queue */
      return NULL;                     /* can't find this            */
    }
    QHDATA(current)->next = apidata->session_base;   /* add to front */
    apidata->session_base = current;
                                       /* set the session id         */
    QHDATA(current)->queue_session = apidata->SessionId;
  }
  return (current);                    /* return new block to caller */
}

/*********************************************************************/
/*                                                                   */
/*  Function:       alloc_queue_entry()                              */
/*                                                                   */
/*  Description:    Allocate a queue entry()                         */
/*                                                                   */
/*  Function:       Allocate a queue entry control block and a       */
/*                  queue element block.                             */
/*                                                                   */
/*  Input:          data size and pointer to queue data.             */
/*                                                                   */
/*  Output:         address of allocated memory                      */
/*                                                                   */
/*  Effects:        memory allocated and given to the detached       */
/*                  queue manager.                                   */
/*                                                                   */
/*********************************************************************/

static LONG   alloc_queue_entry(
  ULONG       size,                   /* size of queue entry.        */
  PULONG      element,                /* Address of queue element    */
  PCHAR       data)                   /* actual queue data           */
{
   LONG     rc = RXQUEUE_OK;          /* Function result.            */
   ULONG    entry;                    /* temp variable               */
   ULONG    addsize;
                                      /* first allocate header block */


   /* no chance, QUEUEITEM and element must be allocated together    */
   /* because otherwhile the rearrangement fails. Conceptually this  */
   /* should not be a problem THU                                    */

   /*addsize = size + sizeof(QUEUEITEM);  RXROUNDUP added to align queue data */
   addsize = RXROUNDUP( size + sizeof(QUEUEITEM), SHM_OFFSET );

   if (RxAllocMem(element, addsize, QMEM))
   {
      return (1);
   }

//   if (RxAllocMem(element,sizeof(QUEUEITEM),QMEM))
//     return (1);                      /* can't get it, error         */

                                      /* next get the queue element  */
//   if (size) {                        /* but only if not a null str. */
//     if (RxAllocMem(&entry,size,QMEM)) {
                                      /* didn't work, so return head */
//     apidata->qmemsizeused = apidata->qmemsizeused - sizeof(QUEUEITEM);
//       RxFreeMem(*element,sizeof(QUEUEITEM),QMEM);
//       return (1);                    /* and quit                    */
//   }
// apidata->qmemsizeused = apidata->qmemsizeused + addsize;

   QIDATA(*element)->size = size;     /* fill in the size            */
                                      /* fill in the data and time   */
   GetDateTime((PDATETIME)&(QIDATA(*element)->addtime));

   if (size == 0)
   {
      QIDATA(*element)->queue_element = NULL;
   }
   else
   {
     QIDATA(*element)->queue_element = (*element) + sizeof(QUEUEITEM);
     memcpy(QDATA(QIDATA(*element)->queue_element),data,size);
   }

//   QIDATA(*element)->queue_element = entry;
//                                      /* copy the queue data in      */
//                                      /* if we actually have any     */
//     memcpy(QDATA(QIDATA(*element)->queue_element),data,size);
//   }
//   else                               /* make a null pointer         */
//     QIDATA(*element)->queue_element = NULL;

   return(0);
}

/*********************************************************************/
/*                                                                   */
/*  Function:        GetDateTime                                     */
/*                                                                   */
/*  Description:     Sets the DATETIME struct for a queue item       */
/*                                                                   */
/*  Input:           Pointer to the struct                           */
/*                                                                   */
/*  Output:          none                                            */
/*                                                                   */
/*********************************************************************/
VOID GetDateTime(PDATETIME datetime){
  tm *dt;                             /* struct to hold info          */
  time_t t;

  t = time(NULL);
  dt = localtime(&t);                 /* get the info                 */

  datetime->hours        = dt->tm_hour;
  datetime->minutes      = dt->tm_min;
  datetime->seconds      = dt->tm_sec;
  datetime->hundredths   = NULL;      /* not possible                 */
  datetime->day          = dt->tm_mday;
  datetime->month        = dt->tm_mon;
  datetime->year         = 1900 + (dt->tm_year);
  datetime->weekday      = dt->tm_wday;
  datetime->microseconds = NULL;      /* not possible                 */
  datetime->yearday      = dt->tm_yday;
  datetime->valid        = 1;
}

/*$PE*/
/*********************************************************************/
/*                                                                   */
/*  Function:        RexxCreateQueue()                               */
/*                                                                   */
/*  Description:     API Entry to create a queue.                    */
/*                                                                   */
/*  Function:        Create a new (empty) queue.                     */
/*                                                                   */
/*  Notes:           Queues are assigned an internal name            */
/*                   derived from the session that created them      */
/*                   and the serial number of the queue.  When a     */
/*                   queue is deleted, its serial number becomes     */
/*                   reuseable.                                      */
/*                                                                   */
/*                   The queue header blocks are stored in order     */
/*                   of their serial number.  The first queue        */
/*                   created has serial number 1.  Serial number 0   */
/*                   is reserved for the session queue.              */
/*                                                                   */
/*                   Before a queue is created, the chain of queue   */
/*                   headers is searched for gaps.  If we find a     */
/*                   gap in the sequence, we use the number that     */
/*                   belongs to the gap.  This is somewhat           */
/*                   inefficient, but has the advantages of          */
/*                   simplicity, reliability and understandability.  */
/*                                                                   */
/*                   The user can pass a NULL for the requested queue*/
/*                   name.  In that case, we assign an arbitrary     */
/*                   name.                                           */
/*                                                                   */
/*                   If the requested name already exists, we assign */
/*                   an arbitrary name to the new queue.  This name  */
/*                   is passed back to the caller, and the duplicate */
/*                   flag is set.                                    */
/*                                                                   */
/*                   Queue names must obey the rules for REXX        */
/*                   symbols.  Lower case characters in queue names  */
/*                   are translated to upper case.                   */
/*                                                                   */
/*  Input:           Buffer for the name of the created queue.       */
/*                   Size of this buffer.                            */
/*                   Requested queue name.  May be NULL.             */
/*                                                                   */
/*  Output:          Internal name for the queue.  Duplicate         */
/*                   indicator.  Status of create.                   */
/*                                                                   */
/*  Effects:         New queue created.                              */
/*                                                                   */
/*********************************************************************/
ULONG  APIENTRY RexxCreateQueue(
  PSZ     name,                        /* Internal name (returned).  */
  ULONG   size,                        /* Length of name buffer.     */
  PSZ     usrrequest,                  /* Desired name.              */
  PULONG  pdup)                        /* Duplicate name flag.       */
{
  ULONG        rc;
  ULONG        temp;

  APISTARTUP(QUEUECHAIN);              /* do common entry code       */

  if (usrrequest) {                    /* given a name?              */
                                       /* Is the user's name valid?  */
    if ((!val_queue_name(usrrequest)) ||
        (!rxstricmp(usrrequest,        /* cannot create a queue named*/
        "SESSION"))) {                 /* "SESSION"                  */
      APICLEANUP(QUEUECHAIN);          /* release shared resources   */
      return (RXQUEUE_BADQNAME);       /* return with proper code    */
    }

    if (search(usrrequest)) {          /* name already exists ...    */
      usrrequest = NULL;               /* generate a new name        */
      *pdup = YES;                     /* Tell caller we have dup    */
    }
  }

  rc = RXQUEUE_OK;                     /* start with a good rc       */
  if (usrrequest) {                    /* if a good name, then we    */
                                       /* need enough space to return*/
                                       /* the name                   */
    if (strlen(usrrequest) >= size)    /* big enough?                */
      rc = RXQUEUE_STORAGE;            /* nope, set a bad return code*/
  }
  else {                               /* need space for default name*/
    if ((INTERNAL_NAME_SIZE + 1) >= size )
      rc = RXQUEUE_STORAGE;            /* this is error also         */
  }


  if (!rc) {                           /* if no errors to this point */
//  EnterMustComplete();               /* make sure we can complete  */
    if (!(rc = queue_allocate(         /* the queue is allocated     */
          usrrequest,
          &temp))) {
      QHDATA(temp)->next = apidata->base;/* Insert queue into chain. */
      apidata->base = temp;            /* and update the anchor      */
                                       /* copy the new name into buf */
      strcpy(name,QHDATA(temp)->queue_name);
    }
//  ExitMustComplete();                /* end of critical section    */
  }
  APICLEANUP(QUEUECHAIN);              /* release shared resources   */
  return (rc);                         /* and exit                   */
}


/*********************************************************************/
/*                                                                   */
/*  Function:         RexxDeleteQueue()                              */
/*                                                                   */
/*  Description:      Delete a queue.                                */
/*                                                                   */
/*  Function:         Delete all entries in a queue, then delete     */
/*                    the queue header.                              */
/*                                                                   */
/*                                                                   */
/*  Input:            external queue name.                           */
/*                                                                   */
/*  Effects:          Queue and all its entries deleted.             */
/*                                                                   */
/*********************************************************************/
ULONG  APIENTRY RexxDeleteQueue(
  PSZ name)                            /* name of queue to delete    */
{
  ULONG        rc;                     /* return code from call      */
  ULONG        curr_item;              /* current queue item         */
  ULONG        next_item;              /* next queue item            */
  ULONG        previous;               /* previous list entry        */
  ULONG        current;                /* current list entry         */
  BOOL found = FALSE;
                                       /* Exception handler record   */

  if (!val_queue_name(name))           /* Did the user supply a      */
                                       /* valid name?                */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */
  else if (!rxstricmp(name,"SESSION")) /* trying to delete "SESSION" */
    return (RXQUEUE_BADQNAME);         /*   then signal an error     */


  APISTARTUP(QUEUECHAIN);              /* do common entry code       */

  rc = RXQUEUE_NOTREG;                 /* default to bad name        */
  previous = NULL;                     /* no previous one yet        */
  current = apidata->base;             /* get queue base             */
  while ( (current) && (!found) )
  {
    if (!rxstricmp(name,               /* if we have a name match    */
                  QHDATA(current)->queue_name))
    {
      found = TRUE;
      if (QHDATA(current)->waiting)    /* if someone waiting on this */
      {
        rc=RXQUEUE_ACCESS;             /* tell the caller            */
        break;                         /* and get out of here        */
      }
//    EnterMustComplete();             /* start of critical section  */
                                       /* while there is an item     */
      curr_item = QHDATA(current)->queue_first;
      while(curr_item != 0 )
      {
        next_item = QIDATA(curr_item)->next;
//      QHDATA(current)->queue_first = QIDATA(curr_item)->next;
                                       /* this was the last item ?   */
//      if(QHDATA(current)->queue_first == 0 )
//        QHDATA(current)->queue_last = 0; /* reset the last ptr     */
        release_queue_item(curr_item, QMEMNAMEDQUE, current);/* return storage for this one */
        curr_item = next_item;
      }
//      if (!previous)                   /* if releasing first item    */
//                                       /* just remove from front     */
//        apidata->base = QHDATA(current)->next;
//      else
//        QHDATA(previous)->next =       /* we need to close up the    */
//           QHDATA(current)->next;      /* chain                      */
      delete_queue_sem(current);       /* get rid of semaphores      */
      RxFreeMemQue(current,               /* get rid of queue header    *//* always a named queue */
                sizeof(QUEUEHEADER),
                QMEMNAMEDQUE, current);
      rc=RXQUEUE_OK;                   /* set good return code       */
//    apidata->qmemsizeused -= sizeof(QUEUEHEADER);
//    ExitMustComplete();              /* end of critical section    */
      break;                           /* and get out of here        */
    }
    previous = current;                /* save predecessor           */
    current = QHDATA(current)->next;   /* step to next block         */
  }
  CheckForMemory();    /* give memory decreasing algorithm a chance. */
  APICLEANUP(QUEUECHAIN);              /* release shared resources   */
  return (rc);                         /* return with return code    */
}


/*********************************************************************/
/*                                                                   */
/*  Function:         RexxQueryQueue()                               */
/*                                                                   */
/*  Description:      Return size of a named queue.                  */
/*                                                                   */
/*  Function:         Return the count of elements in a named queue. */
/*                                                                   */
/*  Input:            external queue name                            */
/*                                                                   */
/*  Effects:          Count of queue elements.                       */
/*                                                                   */
/*********************************************************************/
ULONG  APIENTRY RexxQueryQueue(
  PSZ    name,                        /* Queue to query.             */
  PULONG count)                       /* Length of queue (returned)  */
{
  ULONG        rc;
  ULONG current;

  if (!val_queue_name(name))           /* Did the user supply a      */
                                       /* valid name?                */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */

  APISTARTUP(QUEUECHAIN);              /* do common entry code       */

  *count=0;                            /* initialize the count       */
  if (!rxstricmp(name,"SESSION")) {    /* trying to delete "SESSION" */
    current = search_session();        /* get current session queue  */
    if (current) {                     /* found or allocated?        */
      *count = QHDATA(current)->item_count;/*return the current count*/
      rc = RXQUEUE_OK;                 /* set return code            */
    }
    else
      rc = RXQUEUE_NOTREG;             /* report not here            */
  }
  else if ((current = search(name))) { /* if the queue exists        */
    *count = QHDATA(current)->item_count;/* return the current count */
    rc = RXQUEUE_OK;                   /* set return code            */
  }
  else                                 /* doesn't exist              */
    rc = RXQUEUE_NOTREG;               /* set the error code         */

  APICLEANUP(QUEUECHAIN);              /* release shared resources   */
  return (rc);                         /* return with return code    */
}

/*********************************************************************/
/*                                                                   */
/*  Function:         RexxAddQueue()                                 */
/*                                                                   */
/*  Description:      Add entry to a queue.                          */
/*                                                                   */
/*  Function:         Allocate memory for queue entry and control    */
/*                    block.  Move data into entry & set up          */
/*                    control info.  Add entry to queue chain.       */
/*                                                                   */
/*  Input:            external queue name, entry data, data size,    */
/*                    LIFO/FIFO flag.                                */
/*                                                                   */
/*  Effects:          Memory allocated for entry.  Entry added to    */
/*                    queue.                                         */
/*                                                                   */
/*********************************************************************/
ULONG  APIENTRY RexxAddQueue(
  PSZ       name,
  PRXSTRING data,
  ULONG     flag)
{
  ULONG        rc;
  ULONG        current;
  ULONG        item;                   /* Local (allocation) pointer.*/

  rc = RXQUEUE_OK;                   /* worked well                */
  if (!val_queue_name(name))           /* Did the user supply a      */
                                       /* valid name?                */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */

                                       /* first check the flag       */
  if (flag!=RXQUEUE_FIFO && flag!=RXQUEUE_LIFO)
    return (RXQUEUE_PRIORITY);         /* error, tell the caller     */

  APISTARTUP(QUEUECHAIN);              /* do common entry code       */

                                       /* try to allocate an element */
  if (alloc_queue_entry(data->strlength,&item,data->strptr))
  {
    rc = RXQUEUE_MEMFAIL;              /* out of memory, stop        */
  }
  else
  {
    if (!rxstricmp(name,"SESSION"))      /* trying to delete "SESSION" */
    {
      current = search_session();        /* get current session queue  */
    }
    else
    {
      current = search(name);            /* if the queue exists        */
    }
    if (!current)                        /* no queue pointer, then     */
    {
      rc = RXQUEUE_NOTREG;               /* we have an unknown queue   */
    }
    else if (QHDATA(current)->queue_first == 0) /* if queue is empty, then */
    {
      QHDATA(current)->queue_first =   /* just add to the front      */
         QHDATA(current)->queue_last = item;
    }
    else                               /* otherwise, we need to do   */
    {
      if (flag==RXQUEUE_LIFO)          /* it either LIFO             */
      {
        QIDATA(item)->next=QHDATA(current)->queue_first;
        QHDATA(current)->queue_first=item;
      }
      else                             /* or FIFO                    */
      {                                /* add to the end             */
        QIDATA(QHDATA(current)->queue_last)->next=item;
        QHDATA(current)->queue_last=item;/* set new last pointer     */
        QIDATA(item)->next = NULL;     /* nothing after this one     */
      }
    }
  }
  if (!rc)                             /* RXQUEUE_OK)                */
  {
    QHDATA(current)->item_count++;     /* update the item count      */
    if (QHDATA(current)->waiting)      /* if someone waiting on this */
    {                                  /* queue, then post the event */
                                       /* semaphore.                 */
      PostEventSem(QHDATA(current)->waitsem);
    }
  }
//ExitMustComplete();                  /* end of critical section    */

  APICLEANUP(QUEUECHAIN);              /* release shared resources   */
  return (rc);                         /* return with return code    */
}


/*$PE*/
/*********************************************************************/
/*                                                                   */
/*  Function:         RexxPullQueue()                                */
/*                                                                   */
/*  Description:      Pull an entry from a queue.                    */
/*                                                                   */
/*  Function:         Locate the queue, return its top entry to      */
/*                    the caller, and tell the queue data            */
/*                    manager to delete the entry.                   */
/*                                                                   */
/*                    If the queue is empty, the caller can elect    */
/*                    to wait for someone to post an entry.          */
/*                                                                   */
/*  Notes:            Caller is responsible for freeing the returned */
/*                    memory.                                        */
/*                                                                   */
/*                    The entry's control block is stored in the     */
/*                    entry's memory.  We must therefore obtain      */
/*                    addressability to the entry's memory before    */
/*                    we can process the entry.                      */
/*                                                                   */
/*  Input:            external queue name, wait flag.                */
/*                                                                   */
/*  Output:           queue element, data size, date/time stamp.     */
/*                                                                   */
/*  Effects:          Top entry removed from the queue.  Message     */
/*                    queued to the queue data manager.              */
/*                                                                   */
/*********************************************************************/
ULONG  APIENTRY RexxPullQueue(
  PSZ         name,
  PRXSTRING   data_buf,
  PDATETIME   dt,
  ULONG       waitflag)
{
  ULONG        rc = RXQUEUE_OK;
  ULONG        mutexrc;                             //THUTHU
  ULONG        current;
  ULONG        item;
  INT          mutexsem;               /* semaphore to wait onTHUTHU */
  INT          waitsem;                /* semaphore to wait on       */
  INT          rexxapisem;             /* semaphore to wait onTHUTHU */
  INT          sessionflag;
                                       /* got a good wait flag?      */
  if (waitflag!=RXQUEUE_NOWAIT && waitflag!=RXQUEUE_WAIT)
    return (RXQUEUE_BADWAITFLAG);      /* no, just exit              */

  if (!val_queue_name(name))           /* Did the user supply a      */
                                       /* valid name?                */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */

  APISTARTUP(QUEUECHAIN);              /* do common entry code       */

  if (!rxstricmp(name,"SESSION")) {    /* trying to delete "SESSION" */
    current = search_session();        /* get current session queue  */
    sessionflag = QMEMSESSION;
  }
  else
  {
    current = search(name);            /* if the queue exists        */
    sessionflag = QMEMNAMEDQUE;
  }
  if (!current)                        /* no queue pointer, then     */
    rc = RXQUEUE_NOTREG;               /* we have an unknown queue   */
  else
  {
    item = NULL;                       /* set to no item initially   */
    if (QHDATA(current)->item_count)   /* if items in the queue      */
      item = QHDATA(current)->queue_first;/* get the first one       */
                                       /* if no items in the queue   */
                                       /* and we've been asked to    */
                                       /* wait for one               */
    while (!item && waitflag) {
      QHDATA(current)->waiting++;      /* update the waiters count  */

            /********************************************************/
            /* Extablish an exit handler in case we get terminated  */
            /* while waiting for data.                              */
            /********************************************************/

                                       /* now release lock on global */
                                       /* data so someone else can   */
//                                     /* add a queue entry          */
// APICLEANUP(QUEUECHAIN);
// EnterMustComplete();
                                       /* now request the queue sem  */
// if (RequestMutexSem(QHDATA(current)->enqsem))
// {                                /* if non-zero, then exit     */
//   detachall(QUEUECHAIN);         /* clean up everything        */
//   ExitMustComplete();
//   return (RXQUEUE_MEMFAIL);      /* error if we couldn't get it*/
// }
                                       /* now rerequest global, so   */
                                       /* we can ensure proper access*/
// if (RxAPIStartUp(QUEUECHAIN)) {

      /* The memory can now be rearranged. So search the queue again */
      /* to be sure that the pointer is  valid.                      */
//   if (!rxstricmp(name,"SESSION")) {
//     current = search_session();    /* get current session queue  */
//   }
//   else
//     current = search(name);        /* or named queue             */

//   QHDATA(current)->waiting--;    /* remove the wait            */
//   detachall(QUEUECHAIN);         /* clean up everything        */
//   ExitMustComplete();
//   return (RXQUEUE_MEMFAIL);      /* error if we couldn't get it*/
// }
      /* The memory can now be rearranged. So search the queue again */
      /* to be sure that the pointer is  valid.                      */
// if (!rxstricmp(name,"SESSION")) {
//   current = search_session();    /* get current session queue  */
// }
// else
//   current = search(name);        /* or named queue             */

/* now let me try to give others access to shared memory, although   */
/* i want to keep track of it, in case someone else has made an      */
/* entry in the queue. If another one has made an entry, he signals  */
/* an event and starts me again. Then, let me try to lock the base   */
/* semaphore again, rearrange the memory and catch the item in the   */
/* queue. Protection with Mutexes is needed in case another process  */
/* has the same idea.                                                */

// if (!QHDATA(current)->item_count) {/* if still nothing on queue*/
                                       /* clear event sem first      */
//   ResetEventSem(QHDATA(current)->waitsem);
      waitsem = QHDATA(current)->waitsem;      /*get local semaphore copy  */
      mutexsem = QHDATA(current)->enqsem;
      rexxapisem = apidata->rexxapisemaphore;
      QHDATA(current)->waitprocess = apidata->ProcessId;
      RxAPICleanUp(QUEUECHAIN, SIGCNTL_BLOCK);
      if(!RequestMutexSem(rexxapisem,mutexsem))
      {
        if (WaitEventSem(rexxapisem,waitsem))             /* theres something wrong   */
        {
          APISTARTUP(QUEUECHAIN);              /* do common entry code       */
          if (!rxstricmp(name,"SESSION"))       /* rearrange memory        */
          {
            current = search_session();
          }
          else
          {
            current = search(name);
          }
          APISTARTUP(QUEUECHAIN);              /* do common entry code       */
          ReleaseMutexSem(QHDATA(current)->enqsem);
          QHDATA(current)->waiting--;  /* remove the wait            */
          APICLEANUP(QUEUECHAIN);
          return (RXQUEUE_MEMFAIL);    /* error if we couldn't get it*/
        }
        else                           /* everythings gona be alright*/
        {
          APISTARTUP(QUEUECHAIN);              /* do common entry code       */
          if (!rxstricmp(name,"SESSION"))       /* rearrange memory        */
          {
            current = search_session();
          }
          else
          {
            current = search(name);
          }
        }
      }
      else /* the mutex semaphore does not work properly                     */
      {
         QHDATA(current)->waiting--;  /* remove the wait            */
         return (RXQUEUE_MEMFAIL);    /* error if we couldn't get it*/
      }
      item=QHDATA(current)->queue_first;       /* get the new item           */
      QHDATA(current)->waiting--;              /* remove the wait            */
      ReleaseMutexSem(QHDATA(current)->enqsem);
    }                                          /* end the while loop         */
            /********************************************************/
            /* Now that an add to our queue has woken us up, we     */
            /* want to remove the item from the queue.              */
            /*                                                      */
            /* Note that another thread may grab the item before    */
            /* we can get exclusive access to the API.  In that     */
            /* case, we wave off and go around again.               */
            /*                                                      */
            /* Note, too, that the current thread has already       */
            /* registered itself as holding the main queue API      */
            /* semaphore.  We do not want to create duplicate       */
            /* registration blocks in the queue header memory,      */
            /* therefore we set the queue semaphore directly with   */
            /* DosRequestMutexSem() instead of calling getQsem().   */
            /********************************************************/
//
//    QHDATA(current)->waiting--;      /* remove our wait flag       */
//                                     /* release the mutex          */
//    ReleaseMutexSem(QHDATA(current)->enqsem);
//    item=QHDATA(current)->queue_first;       /* get the new item   */
//  }


    if (item)                          /* if we got an item          */
    {
//    EnterMustComplete();             /* start of critical section  */
            /*********************************************************/
            /* We have located the queue and determined that an item */
            /* is available.  Obtain addressability to the queue     */
            /* item.                                                 */
            /*********************************************************/

                                       /* dechain, updating the end  */
                                       /* pointer if necessary       */
//      if ((QHDATA(current)->queue_first=QIDATA(item)->next) == 0 )
//        QHDATA(current)->queue_last = 0;                           /*let this be done by RxFreeMem */

      QHDATA(current)->item_count--;   /* reduce the queue size      */

      if (data_buf->strptr &&          /* given a default buffer?    */
          data_buf->strlength >= QIDATA(item)->size) {
        if (QIDATA(item)->size)        /* if actual data in element  */
                                       /* copy the data into the buf */
          memcpy(data_buf->strptr,
                 QDATA(QIDATA(item)->queue_element),
                 QIDATA(item)->size);
                                       /* set the proper length      */
        data_buf->strlength = QIDATA(item)->size;
        memcpy((PUCHAR)dt,             /* set the datetime info      */
               (PUCHAR)&(QIDATA(item)->addtime),
               sizeof(DATETIME));
        release_queue_item(item, sessionflag, current);      /* get rid if the queue item  */
      }
      else {                           /* give up memory directly    */
        if (QIDATA(item)->size) {      /* if not a null string       */
                                       /* allocate a new block       */
          if (!(data_buf->strptr = (PCHAR)malloc(QIDATA(item)->size))) {
//          ExitMustComplete();        /* end of critical section    */
            APICLEANUP(QUEUECHAIN);    /*   clean up everything      */
            return (RXQUEUE_MEMFAIL);  /* error if we couldn't get it*/
          }
          else
                                       /* copy the data over         */
            memcpy(data_buf->strptr,QDATA(QIDATA(item)->queue_element),
                   QIDATA(item)->size);
        }
        else                           /* set a non-null pointer     */
          data_buf->strptr=(PCHAR)1;
                                       /* set the length             */
        data_buf->strlength =QIDATA(item)->size;

        memcpy((PUCHAR)dt,             /* set the datetime info      */
               (PUCHAR)&(QIDATA(item)->addtime),
               sizeof(DATETIME));
        release_queue_item(item, sessionflag, current);      /* free up the queue item     */
      }
//    ExitMustComplete();              /* end of critical section    */
    }
    else
      rc=RXQUEUE_EMPTY;
  }
  CheckForMemory();    /* give memory decreasing algorithm a chance. */
  APICLEANUP(QUEUECHAIN);              /* release shared resources   */
  return (rc);                         /* return with return code    */
}


/*********************************************************************/
/*                                                                   */
/*  Function:        Queue_Detach;                                   */
/*                                                                   */
/*  Description:     Close the session queue                         */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
VOID Queue_Detach(ULONG pid)
{
  ULONG current;
  ULONG previous;
  ULONG curr_item;
  ULONG next_item;              /* next queue item            */

  if(apidata == NULL)                 /* nothing happend at all      */
    return;

/* this is for LINUX only. AIX does not create different PID's for   */
/* different threads. The 'start' and 'reply' methods do not cause   */
/* new PID's to be created. Only 'rexx newprg' create a new process  */
/* with new PID (not: call 'newprg') and therefor a new session queue*/
/* is (and must) be created.                                         */

/* concerning the waiting flag: Should be no problem, because this   */
/* routine should only be called, when all threads of the process    */
/* have ended, and no other process (because it is the sessionqueue) */
/* can wait on it. Therefor i don't have to care on that             */

//#ifdef LINUX
//  if(pid == getpgid(0)){            /* if this is the parent pocess*/
//#endif
    current = search_session();       /* get the session queue       */


/* let RxFreeMem do this for us                                        */

//    if(apidata->session_base == current)/* if this is the first one  */
//       apidata->session_base = QHDATA(current)->next;/* chain it out */
//    else{                             /* looking for the previous one*/
//      previous = apidata->session_base;/* take the first             */
//      while(QHDATA(previous)->next != current){
//        previous = QHDATA(previous)->next;/* next one please         */
//      }
//                                      /* now we chan chain it out    */
//      QHDATA(previous)->next = QHDATA(current)->next;
//    }
                               /* if this was actually the last queue*/


/* let me free the items of the session queue of this process          */

    curr_item = QHDATA(current)->queue_first;
    while(curr_item != 0)
    {
       next_item = QIDATA(curr_item)->next;
       release_queue_item(curr_item, QMEMSESSION, current);
       curr_item = next_item;
    }

/* and now the queue header                                            */

    delete_queue_sem(current);       /* get rid of semaphores      */
    RxFreeMemQue(current,               /* get rid of queue header    *//* always session queue */
              sizeof(QUEUEHEADER),
              QMEMSESSION, current);

    if(!(apidata->base) && !(apidata->session_base))
    {
      delete_queue_sem(current);      /* free the semaphores         */
      removeshmem(apidata->qbasememId);/* remove the queue mem pool  */
      detachshmem(apidata->qbase);    /* force the deletion          */
      apidata->qbase == NULL;         /* reset the memory pointer    */
      apidata->qmemsizeused = 1;
    }
    else                     /* we have still Named or/and Sessionqueues in shared memory */
    {
      CheckForMemory(); /* give memory decreasing algorithm a chance. */
    }                   /* Be aware CheckForMemory rearrangement algo */
                        /* rithm resets apidata values. After that lo */
                        /* cal copies of apidata values need a reiniti*/
                        /* alization                                  */
//#ifdef LINUX
//  }
//#endif
  /* It's possible that the process terminates until a RexxPullQueue */
  /* command is waiting on a queue. So let's clean up the semaphores */
  /* and the wait count of the appropriate queue.                    */
  /* don't forget to reinitialize the local copies of apidata values */

                                       /* first for the named queues */
  current = apidata->base;             /* get the anchor             */
  while(current){
    if((QHDATA(current)->waiting > 0 ) /* if somebody is waiting     */
       &&(QHDATA(current)->waitprocess == getpid())){  /* and I'm the one  */
      /* clear out the mutex sem of the queue                        */
                                       /* do the initialisation      */
      init_sema(apidata->rexxapisemaphore, QHDATA(current)->enqsem);
      QHDATA(current)->waiting--;      /* decrement the counter      */
    }
    current = QHDATA(current)->next;   /* test the next queue        */
  }
                                       /* now for the session queues */
  current = apidata->session_base;     /* get the anchor             */
  while(current){
    if((QHDATA(current)->waiting > 0 ) /* if somebody is waiting     */
       &&(QHDATA(current)->waitprocess == getpid())){  /* and I'm the one */
      /* clear out the mutex sem of the queue                        */
                                       /* do the initialisation      */
      init_sema(apidata->rexxapisemaphore, QHDATA(current)->enqsem);
      QHDATA(current)->waiting--;      /* decrement the counter      */
    }
    current = QHDATA(current)->next;   /* test the next queue        */
  }
  return;
}

/*********************************************************************/
/* Semapore functions for the queue semaphores                       */
/*********************************************************************/


ULONG CreateMutexSem(INT *handle)
{
  if(apidata->qsemcount < MAXSEM-1){ /* if two sem unused            */
    for(int i=1;i<MAXSEM+1;i++){     /* find an unused semaphore     */
      if(!apidata->qsemfree[i]){     /* if found one                 */
        apidata->qsemfree[i] = 1;    /* mark it as used              */
        *handle = i;                 /* and return it                */
        apidata->qsemcount++;        /* don't forget to count        */
        return ((ULONG)0);           /* worked well                  */
      }
    }
  }
  return ((ULONG)1);                 /* semaphore set is full        */
}


ULONG CreateEventSem(INT *handle)
{
  if(apidata->qsemcount < MAXSEM){   /* if one sem is unused         */
    for(int i=1;i<MAXSEM+1;i++){     /* find the unused semaphore    */
      if(!apidata->qsemfree[i]){     /* if found                     */
        apidata->qsemfree[i] = 1;    /* mark it as used              */
        *handle = i;                 /* and return it                */
        (apidata->qsemcount)++;      /* don't forget to count        */
        return ((ULONG)0);           /* worked well                  */
      }
    }
  }
  return ((ULONG)1);                 /* semaphore set is full        */
}

ULONG RequestMutexSem(INT rexxapisem, INT handle){

//return (locksem(apidata->rexxapisemaphore, handle));
  return (locksem(rexxapisem, handle));
}

VOID ReleaseMutexSem(INT handle){

  unlocksem(apidata->rexxapisemaphore, handle);
}


VOID ResetEventSem(INT handle){

                            /* do the initialisation                */
  init_sema(apidata->rexxapisemaphore, handle);
  locksem(apidata->rexxapisemaphore, handle);
}

ULONG WaitEventSem(INT rexxapisem, INT handle){
  /* wait until a post call wakes me up                            */
//return locksem(apidata->rexxapisemaphore, handle);
  return (locksem(rexxapisem, handle));
}


VOID CloseMutexSem(INT handle){

                                    /* do the initialisation        */
  init_sema(apidata->rexxapisemaphore, handle);
  apidata->qsemfree[handle] = NULL; /* mark it as unused            */
  --(apidata->qsemcount);           /* decrment counter             */
}

VOID CloseEventSem(INT handle){

                                    /* do the initialisation        */
  init_sema(apidata->rexxapisemaphore, handle);
  apidata->qsemfree[handle] = NULL; /* mark it as unused            */
  --(apidata->qsemcount);           /* decrment counter             */
}


VOID PostEventSem(INT handle){
  /* unlock the sem to wake the one who waits                      */
  unlocksem(apidata->rexxapisemaphore, handle);
}

