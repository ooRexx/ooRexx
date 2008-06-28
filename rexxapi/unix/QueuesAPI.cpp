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
/*  Entry Points:   LONG  REXXENTRY RexxCreateQueue()-create a queue           */
/*                  LONG  REXXENTRY RexxDeleteQueue()-destroy a                */
/*                      queue                                                 */
/*                  LONG  REXXENTRY RexxQueryQueue() -query a queue            */
/*                  LONG  REXXENTRY RexxAddQueue()   -add data                 */
/*                  LONG  REXXENTRY RexxPullQueue()  -retrieve data            */
/*                  LONG  REXXENTRY RexxInitDataQueueInit()-start              */
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

extern REXXAPIDATA  *apidata;  /* Global state data                  */

/*********************************************************************/
/*                Function prototypes for local routines.            */
/*********************************************************************/

static int    val_queue_name(const char *);
static size_t search(const char *);
static int    queue_allocate(const char *, size_t *);
static int    create_queue_sem(size_t);
static void delete_queue_sem(size_t);
static void   release_queue_item(size_t, int, size_t);
void GetDateTime(REXXDATETIME *);
int CreateMutexSem(int *);
int CreateEventSem(int *);
void CloseMutexSem(int);
void CloseEventSem(int);
void PostEventSem(int);
int  RequestMutexSem(int, int);
void ResetEventSem(int);
int  WaitEventSem(int, int);
void ReleaseMutexSem(int);

char rxqueue_name_mask[]="S%02dQ%010u";

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
static int val_queue_name(
  const char *   usrname)              /* User's name.               */
{
   int         rc;                     /* Return code.               */
   const char *valptr;                 /* Used to validate name.     */
   char        ch;
   size_t      namelen;                /* Length of the user's name. */

   if (!usrname)                       /* NULL is OK.                */
      return (1);                      /* return code indication     */

   namelen = strlen(usrname);
   if ((rc = ((0 < namelen) && (namelen <= MAX_NAME_LENGTH)))) {

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
static size_t search(const char *  name)
{
    size_t current;                         /* Current queue element      */
    size_t previous;                        /* Previous queue element     */

    previous = 0;                        /* no previous yet            */
    current = apidata->base;             /* get current base pointer   */

    while (current)
    {                    /* while more queues          */
                         /* if we have a match         */
        if (!strcasecmp(name,QHDATA(current)->queue_name))
        {
            if (previous)
            {                  /* if we have a predecessor   */
                QHDATA(previous)->next =       /* rearrange the chain to     */
                                               QHDATA(current)->next;       /* move this to the front     */
                QHDATA(current)->next = apidata->base;
                apidata->base = current;
            }
            return(current);                /* we are finished            */
        }
        previous = current;                /* remember this block        */
        current = QHDATA(current)->next;   /* step to the next block     */
    }
    return 0;                           /* not found, tell caller so  */
}


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
static int    queue_allocate(
  const char * name,                   /* External queue name.       */
  size_t*pnew)                         /* New queue header (returned)*/
{
   size_t size;                        /* size to allocate           */
   size_t tag;                         /* unique queue identifier    */

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
        tag = (size_t)QDATA(*pnew);   /* get value of pointer        */
        char *newname = QHDATA(*pnew)->queue_name;/* new name will be buffer  */
        for (;;) {                    /* now create a unique name    */
          sprintf(newname,            /* create a new queue name     */
                  rxqueue_name_mask,  /* from the session id and the */
                  apidata->SessionId, /* address of the control block*/
                  tag);
                                      /* if unique, we're done       */
          if (!search(newname))
            break;                    /* get out                     */
          tag++;                      /* try a new number            */
        }
      }
      else {
        char *newname = QHDATA(*pnew)->queue_name;
                                      /* copy the name over          */
        strcpy(newname, name);
                                      /* make it uppercase           */
        while(*newname != '\0')
        {
            *newname = toupper(*newname);
            newname++;
        }
      }
      return (0);                     /* worked fine                 */
   }

   return(1);                         /* have an error               */
}


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
static int create_queue_sem(
    size_t  queue)                     /* new queue                  */
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
static void   release_queue_item(size_t item, int flag, size_t current)
{
  /* Release header and data together to increase performance of the */
  /* memory manager                                                  */


  if (QIDATA(item)->size)              /* empty entry ?              */
  {                              /* delete header and data            */
     RxFreeMemQue(item,RXROUNDUP(sizeof(QUEUEITEM)+QIDATA(item)->size, SHM_OFFSET),flag, current);
  }
  else                                       /* no data !            */
  {                                    /* just delete header          */
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
static void delete_queue_sem(
    size_t  queue)                     /* deleted queue              */
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
  size_t current)                      /* queue to delete            */
{
  size_t curr_item;
  size_t next_item;
  size_t temp;

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
    apidata->qbase = NULL;          /* reset the memory pointer    */
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
size_t search_session(void)
{
size_t       current;                  /* Current queue element      */
size_t       previous;                 /* Previous queue element     */
size_t       next;

  previous = 0;                        /* no previous yet            */
  current = apidata->session_base;     /* get current base pointer   */
  apidata->SessionId = getpgid(0);     /* get the session id         */

  /* look for queue garbage                                          */
  while (current) {                    /* while more queues          */
    next = QHDATA(current)->next;
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
      return 0;                        /* can't find this            */
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

static int alloc_queue_entry(
  size_t      size,                   /* size of queue entry.        */
  size_t     *element,                /* Address of queue element    */
  const char *data)                   /* actual queue data           */
{
   size_t   addsize;
                                      /* first allocate header block */


   /* no chance, QUEUEITEM and element must be allocated together    */
   /* because otherwhile the rearrangement fails. Conceptually this  */
   /* should not be a problem THU                                    */
   addsize = RXROUNDUP( size + sizeof(QUEUEITEM), SHM_OFFSET );

   if (RxAllocMem(element, addsize, QMEM))
   {
      return (1);
   }

   QIDATA(*element)->size = size;     /* fill in the size            */
                                      /* fill in the data and time   */
   GetDateTime(&(QIDATA(*element)->addtime));

   if (size == 0)
   {
      QIDATA(*element)->queue_element = 0;
   }
   else
   {
     QIDATA(*element)->queue_element = (*element) + sizeof(QUEUEITEM);
     memcpy(QDATA(QIDATA(*element)->queue_element),data,size);
   }
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
void GetDateTime(REXXDATETIME *datetime){
  tm *dt;                             /* struct to hold info          */
  time_t t;

  t = time(NULL);
  dt = localtime(&t);                 /* get the info                 */

  datetime->hours        = dt->tm_hour;
  datetime->minutes      = dt->tm_min;
  datetime->seconds      = dt->tm_sec;
  datetime->hundredths   = 0;         /* not possible                 */
  datetime->day          = dt->tm_mday;
  datetime->month        = dt->tm_mon;
  datetime->year         = 1900 + (dt->tm_year);
  datetime->weekday      = dt->tm_wday;
  datetime->microseconds = 0;         /* not possible                 */
  datetime->yearday      = dt->tm_yday;
}

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
RexxReturnCode REXXENTRY RexxCreateQueue(
  char *name,                          /* Internal name (returned).  */
  size_t  size,                        /* Length of name buffer.     */
  const char *usrrequest,              /* Desired name.              */
  size_t *pdup)                        /* Duplicate name flag.       */
{
  RexxReturnCode       rc;
  size_t       temp;

  APISTARTUP(QUEUECHAIN);              /* do common entry code       */

  if (usrrequest) {                    /* given a name?              */
                                       /* Is the user's name valid?  */
    if ((!val_queue_name(usrrequest)) ||
        (!strcasecmp(usrrequest,        /* cannot create a queue named*/
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
    if (!(rc = queue_allocate(         /* the queue is allocated     */
          usrrequest,
          &temp))) {
      QHDATA(temp)->next = apidata->base;/* Insert queue into chain. */
      apidata->base = temp;            /* and update the anchor      */
                                       /* copy the new name into buf */
      strcpy(name,QHDATA(temp)->queue_name);
    }
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
RexxReturnCode REXXENTRY RexxDeleteQueue(
  const char * name)                   /* name of queue to delete    */
{
  RexxReturnCode       rc;                     /* return code from call      */
  size_t       curr_item;              /* current queue item         */
  size_t       next_item;              /* next queue item            */
  size_t       previous;               /* previous list entry        */
  size_t       current;                /* current list entry         */
  bool found = false;
                                       /* Exception handler record   */

  if (!val_queue_name(name))           /* Did the user supply a      */
                                       /* valid name?                */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */
  else if (!strcasecmp(name,"SESSION")) /* trying to delete "SESSION" */
    return (RXQUEUE_BADQNAME);         /*   then signal an error     */


  APISTARTUP(QUEUECHAIN);              /* do common entry code       */

  rc = RXQUEUE_NOTREG;                 /* default to bad name        */
  previous = 0;                        /* no previous one yet        */
  current = apidata->base;             /* get queue base             */
  while ( (current) && (!found) )
  {
    if (!strcasecmp(name,               /* if we have a name match    */
                  QHDATA(current)->queue_name))
    {
      found = true;
      if (QHDATA(current)->waiting)    /* if someone waiting on this */
      {
        rc=RXQUEUE_ACCESS;             /* tell the caller            */
        break;                         /* and get out of here        */
      }
                                       /* while there is an item     */
      curr_item = QHDATA(current)->queue_first;
      while(curr_item != 0 )
      {
        next_item = QIDATA(curr_item)->next;
        release_queue_item(curr_item, QMEMNAMEDQUE, current);/* return storage for this one */
        curr_item = next_item;
      }
      delete_queue_sem(current);       /* get rid of semaphores      */
      RxFreeMemQue(current,               /* get rid of queue header    *//* always a named queue */
                sizeof(QUEUEHEADER),
                QMEMNAMEDQUE, current);
      rc=RXQUEUE_OK;                   /* set good return code       */
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
RexxReturnCode REXXENTRY RexxQueryQueue(
  const char *name,                    /* Queue to query.             */
  size_t *count)                       /* Length of queue (returned)  */
{
  RexxReturnCode       rc;
  size_t current;

  if (!val_queue_name(name))           /* Did the user supply a      */
                                       /* valid name?                */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */

  APISTARTUP(QUEUECHAIN);              /* do common entry code       */

  *count=0;                            /* initialize the count       */
  if (!strcasecmp(name,"SESSION")) {    /* trying to delete "SESSION" */
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
RexxReturnCode REXXENTRY RexxAddQueue(
  const char *name,
  PCONSTRXSTRING data,
  size_t    flag)
{
  RexxReturnCode       rc;
  size_t       current;
  size_t       item;                   /* Local (allocation) pointer.*/

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
    if (!strcasecmp(name,"SESSION"))      /* trying to delete "SESSION" */
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
        QIDATA(item)->next = 0;        /* nothing after this one     */
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
  APICLEANUP(QUEUECHAIN);              /* release shared resources   */
  return (rc);                         /* return with return code    */
}


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
RexxReturnCode REXXENTRY RexxPullQueue(
  const char *name,
  PRXSTRING   data_buf,
  REXXDATETIME *dt,
  size_t      waitflag)
{
  RexxReturnCode       rc = RXQUEUE_OK;
  size_t       current;
  size_t       item;
  int          mutexsem;               /* semaphore to wait on       */
  int          waitsem;                /* semaphore to wait on       */
  int          rexxapisem;             /* semaphore to wait on       */
  int          sessionflag;
                                       /* got a good wait flag?      */
  if (waitflag!=RXQUEUE_NOWAIT && waitflag!=RXQUEUE_WAIT)
    return (RXQUEUE_BADWAITFLAG);      /* no, just exit              */

  if (!val_queue_name(name))           /* Did the user supply a      */
                                       /* valid name?                */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */

  APISTARTUP(QUEUECHAIN);              /* do common entry code       */

  if (!strcasecmp(name,"SESSION")) {    /* trying to delete "SESSION" */
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
    item = 0;                          /* set to no item initially   */
    if (QHDATA(current)->item_count)   /* if items in the queue      */
      item = QHDATA(current)->queue_first;/* get the first one       */
                                       /* if no items in the queue   */
                                       /* and we've been asked to    */
                                       /* wait for one               */
    while (!item && waitflag) {
      QHDATA(current)->waiting++;      /* update the waiters count  */

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
          if (!strcasecmp(name,"SESSION"))       /* rearrange memory        */
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
          if (!strcasecmp(name,"SESSION"))       /* rearrange memory        */
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
    if (item)                          /* if we got an item          */
    {
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
        if (dt != NULL)
        {
            memcpy((char *)dt,    /* set the datetime info      */
                   (char *)&(QIDATA(item)->addtime),
                   sizeof(REXXDATETIME));
        }
        release_queue_item(item, sessionflag, current);      /* get rid if the queue item  */
      }
      else {                           /* give up memory directly    */
        if (QIDATA(item)->size) {      /* if not a null string       */
                                       /* allocate a new block       */
          if (!(data_buf->strptr = (char *)malloc(QIDATA(item)->size))) {
            APICLEANUP(QUEUECHAIN);    /*   clean up everything      */
            return (RXQUEUE_MEMFAIL);  /* error if we couldn't get it*/
          }
          else
                                       /* copy the data over         */
            memcpy(data_buf->strptr,QDATA(QIDATA(item)->queue_element),
                   QIDATA(item)->size);
        }
        else                           /* set a non-null pointer     */
          data_buf->strptr=(char *)1;
                                       /* set the length             */
        data_buf->strlength =QIDATA(item)->size;

        if (dt != NULL)
        {
            memcpy((char *)dt,    /* set the datetime info      */
                   (const char *)&(QIDATA(item)->addtime),
                   sizeof(REXXDATETIME));
        }
        release_queue_item(item, sessionflag, current);      /* free up the queue item     */
      }
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
void Queue_Detach(process_id_t pid)
{
  size_t current;
  size_t curr_item;
  size_t next_item;              /* next queue item            */

  if(apidata == NULL)                 /* nothing happend at all      */
    return;

    current = search_session();       /* get the session queue       */
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
      apidata->qbase = NULL;          /* reset the memory pointer    */
      apidata->qmemsizeused = 1;
    }
    else                     /* we have still Named or/and Sessionqueues in shared memory */
    {
      CheckForMemory(); /* give memory decreasing algorithm a chance. */
    }                   /* Be aware CheckForMemory rearrangement algo */
                        /* rithm resets apidata values. After that lo */
                        /* cal copies of apidata values need a reiniti*/
                        /* alization                                  */
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


int CreateMutexSem(int *handle)
{
  if(apidata->qsemcount < MAXSEM-1){ /* if two sem unused            */
    for(int i=1;i<MAXSEM+1;i++){     /* find an unused semaphore     */
      if(!apidata->qsemfree[i]){     /* if found one                 */
        apidata->qsemfree[i] = 1;    /* mark it as used              */
        *handle = i;                 /* and return it                */
        apidata->qsemcount++;        /* don't forget to count        */
        return 0;                    /* worked well                  */
      }
    }
  }
  return 1;                          /* semaphore set is full        */
}


int CreateEventSem(int *handle)
{
  if(apidata->qsemcount < MAXSEM){   /* if one sem is unused         */
    for(int i=1;i<MAXSEM+1;i++){     /* find the unused semaphore    */
      if(!apidata->qsemfree[i]){     /* if found                     */
        apidata->qsemfree[i] = 1;    /* mark it as used              */
        *handle = i;                 /* and return it                */
        (apidata->qsemcount)++;      /* don't forget to count        */
        return 0;                    /* worked well                  */
      }
    }
  }
  return 1;                          /* semaphore set is full        */
}

int RequestMutexSem(int rexxapisem, int handle){
  return (locksem(rexxapisem, handle));
}

void ReleaseMutexSem(int handle){

  unlocksem(apidata->rexxapisemaphore, handle);
}


void ResetEventSem(int handle){

                            /* do the initialisation                */
  init_sema(apidata->rexxapisemaphore, handle);
  locksem(apidata->rexxapisemaphore, handle);
}

int WaitEventSem(int rexxapisem, int handle){
  /* wait until a post call wakes me up                            */
//return locksem(apidata->rexxapisemaphore, handle);
  return (locksem(rexxapisem, handle));
}


void CloseMutexSem(int handle){

                                    /* do the initialisation        */
  init_sema(apidata->rexxapisemaphore, handle);
  apidata->qsemfree[handle] = 0;    /* mark it as unused            */
  --(apidata->qsemcount);           /* decrment counter             */
}

void CloseEventSem(int handle){

                                    /* do the initialisation        */
  init_sema(apidata->rexxapisemaphore, handle);
  apidata->qsemfree[handle] = 0;    /* mark it as unused            */
  --(apidata->qsemcount);           /* decrment counter             */
}


void PostEventSem(int handle){
  /* unlock the sem to wake the one who waits                      */
  unlocksem(apidata->rexxapisemaphore, handle);
}

