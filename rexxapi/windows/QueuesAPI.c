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
/*********************************************************************/
/*                                                                   */
/*  Program Name:       WINQAPI.C                                    */
/*                                                                   */
/*  Description:        Library to contain API functions for         */
/*                      REXX-SAA/PL queueing services.               */
/*                                                                   */
/*  Entry Points:   LONG  APIENTRY RexxCreateQueue()-create a queue  */
/*                  LONG  APIENTRY RexxDeleteQueue()-destroy a       */
/*                      queue                                        */
/*                  LONG  APIENTRY RexxQueryQueue() -query a queue   */
/*                  LONG  APIENTRY RexxAddQueue()   -add data        */
/*                  LONG  APIENTRY RexxPullQueue()  -retrieve data   */
/*                  LONG  APIENTRY RexxInitDataQueueInit()-start     */
/*                       queuing system                              */
/*                                                                   */
/*********************************************************************/
/* Please note the following:                                        */
/*                                                                   */
/* Functions in this module manipulate data that must be available   */
/* to all processes that call REXX API functions.  These processes   */
/* may invoke the REXX interpreter, or make direct calls to the      */
/* API routines.                                                     */
/*                                                                   */
/* In addition, functions in this module may establish data that     */
/* must persist after the calling process terminates.                */
/*                                                                   */
/* To satisfy these requirements, the system must maintain a process */
/* that serves as a data repository.  Functions in this module then  */
/* give critical data to the repository, and the data persists as    */
/* long as the repository process continues running.                 */
/*                                                                   */
/*********************************************************************/
#define INCL_RXQUEUE
#define INCL_RXSUBCOM
#include <rexx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "APIServiceTables.h"
#include "RexxAPIManager.h"
#include "ASCIISymbols.h"
#include "Characters.h"
#include "APIUtil.h"
#include "RexxAPIService.h"
#include "APIServiceMessages.h"
#include "APIServiceSystem.h"

extern UCHAR first_char[];             /* character type table       */
extern UCHAR lower_case_table[];       /* lower case table for Rexx  */
extern UCHAR upper_case_table[];       /* upper case table for Rexx  */

#define ALREADY_INIT      1    /* indicator of queue manager status  */
#define YES               1
#define NO                0
/* the name_size had to be changed because of win95 using high session IDs */
#define INTERNAL_NAME_SIZE  20  /* size of internal queue name*/

#define ENVBUFSIZE 256

/*********************************************************************/
/*                Function prototypes for local routines.            */
/*********************************************************************/

PQUEUEHEADER  qusearch(PSZ);
PQUEUEHEADER  search_session(ULONG pid, PULONG cnt);
ULONG  search_session_in_API(PULONG cnt, BOOL newprocess);
LONG   val_queue_name(PSZ);
LONG   allocate_queue_entry(PQUEUEITEM *,ULONG, PUCHAR);
BOOL   CheckQueueComBlock();
void ReturnQueueItem(PQUEUEITEM element);
LONG alloc_queue_entry(ULONG size,PQUEUEITEM * element,PUCHAR data);

extern ULONG queue_get_pid(DWORD * envchars);
extern BOOL MapComBlock(int chain);
extern void UnmapComBlock(int chain);

/* functions called by RXAPI.EXE */
extern _declspec(dllexport) APIRET APIAddQueue(void);
extern _declspec(dllexport) APIRET APIPullQueue(void);
extern _declspec(dllexport) APIRET APICreateQueue(ULONG Pid, BOOL newProcess);
extern _declspec(dllexport) APIRET APISessionQueue(ULONG Pid, BOOL newProcess);
extern _declspec(dllexport) APIRET APIDeleteQueue(ULONG Pid, BOOL SessionQ);
extern _declspec(dllexport) LONG APIQueryQueue(void);

extern REXXAPIDATA * RexxinitExports;   /* Global state data  */

/* Now needed for local init because RX is process global */
extern LOCALREXXAPIDATA  RexxinitLocal;   /* Global state data  */

/* the masks had to be changed because of win95 using high session IDs */
CHAR name_mask[]="S%08xQ%010u";        /* here to be overlayed       */
CHAR rxqueue_name_mask[]="S%08xQ%010u";

#define PPVOID VOID**
#define get_process() GetCurrentProcess()
#define get_session() GetCurrentProcessId()

/* nest should not be shared between processes, but each process must have its own instance */
extern _declspec(dllexport) CRITICAL_SECTION nest={0}; /* must complete nest count   */

extern HANDLE ExceptionQueueSem = NULL;

RXQUEUE_TALK * FillQueueComBlock(BOOL add, DWORD addflag, DWORD waitflag, PCHAR data, ULONG datalen,
                                        HANDLE waitsem, PSZ name, ULONG pid)
{
    RXQUEUE_TALK * icom;

    icom = LRX.comblock[API_QUEUE];
    /* do we need to extend the communication block? */
    if (add && (datalen + sizeof(RXQUEUE_TALK) > LRX.comblockQueue_ExtensionLevel * PAGE_SIZE))
    {
       if (MySendMessage(RXAPI_QUEUECOMEXTEND,
                       (WPARAM)datalen + sizeof(RXQUEUE_TALK),
                       (LPARAM)0)) return NULL;
       if (!CheckQueueComBlock()) return NULL;
       icom = LRX.comblock[API_QUEUE];
    }

    icom->AddFlag = (WORD)addflag;
    icom->PullFlag = (WORD)waitflag;
    icom->WaitSem = waitsem;
    if (!pid && name)
    {
        ULONG namelen = strlen(name);
        if (namelen >= MAXQUEUENAME) namelen = MAXQUEUENAME-1;
        memcpy(icom->qName, name, namelen);
        icom->qName[namelen] = '\0';
    }
    icom->ProcessID = pid;

    if (add)
    {
        if (datalen)
        {
            icom->queue_item.queue_element = (PUCHAR)(icom + 1);  /* set absolute pointer */
            memcpy(icom->queue_item.queue_element,data,datalen);
        }
        icom->queue_item.size = datalen;
        icom->queue_item.queue_element = (PCHAR) sizeof(RXQUEUE_TALK);   /* set relative pointer */
    }
    return LRX.comblock[API_QUEUE];
}


void ReturnQueueItem(PQUEUEITEM element)
{
   RXQUEUE_TALK * intercom;
   intercom = (RXQUEUE_TALK *) RX.comblock[API_QUEUE];

   intercom->queue_item.queue_element = (PUCHAR)(intercom + 1);   /* absolute address */
   memcpy(intercom->queue_item.queue_element,element->queue_element, element->size);
   intercom->queue_item.size = element->size;
   intercom->queue_item.addtime = element->addtime;
   intercom->queue_item.queue_element = (PCHAR) sizeof(RXQUEUE_TALK);   /* return relative address */
}



/*********************************************************************/
/*                                                                   */
/*  Function:        qusearch()                                      */
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
PQUEUEHEADER  qusearch(
  PSZ  name)
{
  PQUEUEHEADER current;                  /* Current queue element      */
  PQUEUEHEADER previous;                 /* Previous queue element     */

  previous = NULL;                     /* no previous yet            */
  current = RX.base;                   /* get current base pointer   */

  while (current) {                    /* while more queues          */
                                       /* if we have a match         */
    if (!rxstricmp(name,current->queue_name)) {
      if (previous) {                  /* if we have a predecessor   */
        EnterCriticalSection(&nest);   /* make sure we can complete  */
        previous->next =               /* rearrange the chain to     */
          current->next;               /* move this to the front     */
        current->next = RX.base;
        RX.base = current;
        LeaveCriticalSection(&nest);    /* end of must complete part  */
      }
      return (current);                /* we are finished            */
    }
    previous = current;                /* remember this block        */
    current = current->next;           /* step to the next block     */
  }
  return (NULL);                       /* not found, tell caller so  */
}


/*********************************************************************/
/*                                                                   */
/*  Function:        search_session;                                 */
/*                                                                   */
/*  Description:     Return a pointer to the queue for the current   */
/*                   session.                                        */
/*                                                                   */
/*                                                                   */
/*  Function:        Search the list of session queues for the       */
/*                   correct queue.                                  */
/*                                                                   */
/*  Input:           process id of parent process                    */
/*                                                                   */
/*  Output:          Address of the queue header.                    */
/*                                                                   */
/*********************************************************************/
PQUEUEHEADER  search_session(ULONG pid, PULONG cnt)
{
   PQUEUEHEADER current;               /* Current queue element      */
   PQUEUEHEADER previous;              /* Previous queue element     */
   DWORD  envvalue;
   DWORD  sid;

   previous = NULL;                     /* no previous yet            */
   current = RX.session_base;           /* get current base pointer   */

   sid = pid;
   if (current)
   {
      if (!pid)
         sid = queue_get_pid(&envvalue);
      while (current) {                    /* while more queues          */
                                          /* if we have a match         */
         if (current->queue_session == sid) {
            if (previous) {                  /* if we have a predecessor   */
               EnterCriticalSection(&nest);   /* start of must complete part*/
               previous->next =               /* rearrange the chain to     */
                    current->next;               /* move this to the front     */
               current->next = RX.session_base;
               RX.session_base = current;
               LeaveCriticalSection(&nest);    /* end of must complete part  */
            }
            *cnt = current->item_count;
            return (current);                /* we are finished            */
         }
         previous = current;                /* remember this block        */
         current = current->next;           /* step to the next block     */
      }
   }

                        /* not found, create one      */
   if (!sid) sid = get_session();

   *cnt = 0;

   return (PQUEUEHEADER) APICreateQueue(sid, TRUE);
}


/*********************************************************************/
/*                                                                   */
/*  Function:        create_queue_sem                                */
/*                                                                   */
/*  Description:     Create synchroniztion semaphores for a Rexx     */
/*                   queue.                                          */
/*                                                                   */
/*                                                                   */
/*  Function:        Create a mutex semaphore                        */
/*                                                                   */
/*  Input:           Queue header block                              */
/*                                                                   */
/*  Output:          Success/failure return code.                    */
/*                                                                   */
/*********************************************************************/
static INT create_queue_sem(
    PQUEUEHEADER   queue)              /* new queue                  */
{
  SECURITY_ATTRIBUTES sa;

  if (!(queue->waitsem = CreateEvent(SetSecurityDesc(&sa), FALSE, FALSE, NULL))) {
    return (RXQUEUE_MEMFAIL);          /* call this a memory error   */
  }
  return (0);
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
    PQUEUEHEADER   queue)              /* deleted queue              */
{
  CloseHandle(queue->waitsem);
  return;
}



/*********************************************************************/
/*                                                                   */
/*  Function:        queue_get_pid;                                    */
/*                                                                   */
/*  Description:     Read the environment variable RXQUEUESESSION    */
/*                   to get the pid of the queue owner               */
/*                                                                   */
/*********************************************************************/
ULONG queue_get_pid(DWORD * envchars)
{
  CHAR   envbuffer[ENVBUFSIZE+1];
  *envchars = GetEnvironmentVariable("RXQUEUESESSION", (LPTSTR) envbuffer, ENVBUFSIZE);
  if (*envchars)
     return atoi(envbuffer);
  else
     return get_session();     /* get the session id         */
}


/*********************************************************************/
/*                                                                   */
/*  Function:        search_session_in_API;                          */
/*                                                                   */
/*  Description:     Return a pointer to the queue for the current   */
/*                   session. All queues are managed by RXAPI        */
/*                                                                   */
/*                                                                   */
/*  Function:        Search the list of session queues for the       */
/*                   correct queue.                                  */
/*                                                                   */
/*  Input:           cnt, flag to create a new one.                  */
/*                                                                   */
/*  Output:          pid of the queue owner.                         */
/*                                                                   */
/*********************************************************************/
ULONG  search_session_in_API(PULONG cnt, BOOL newprocess)
{
  DWORD  envvalue;
  CHAR   envbuffer[ENVBUFSIZE+1];
  ULONG pid;

  pid = queue_get_pid(&envvalue);
  *cnt = (ULONG) MySendMessage(RXAPI_QUEUESESSION,
                                   (WPARAM)pid,
                                   (LPARAM)newprocess);

  if (!envvalue)
  {
     itoa(pid, envbuffer, 10);
     SetEnvironmentVariable("RXQUEUESESSION", (LPTSTR) envbuffer);
  }
  return (pid);                         /* and exit                   */
}




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
       (namelen <= NAMESIZE-1) ) ) {

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
/*  Output:         0 worked fine, 1 error.                          */
/*                                                                   */
/*  Effects:        Storage allocated.                               */
/*                                                                   */
/*********************************************************************/
LONG   queue_allocate(
  PSZ                 name,            /* External queue name.       */
  PQUEUEHEADER       *pnew,            /* New queue header (returned)*/
  PUSHORT             pusDupe)         /* duplicate queue name       */
{
   LONG  size;                         /* size to allocate           */
   ULONG tag;                          /* unique queue identifier    */


   /* this has been moved up to here because of an error on win95 */
   /* we first must check if the name already exists. If so, */
   /* we will have to allocate more memory for a longer name (IH)*/

   if (qusearch(name)) {           /* name already exists ...    */
     name = NULL;                  /* generate a new name        */
     *pusDupe = 1;                 /* name is duplicate, set flag */
   }

   if (name)                           /* if we have a good name     */
                                       /* allocate a block large     */
                                       /* enough to hold the name    */
     size = strlen(name) + 1 + sizeof(QUEUEHEADER);
   else
     size = sizeof(QUEUEHEADER) + INTERNAL_NAME_SIZE + 1;

  /* Allocate the header record.  If it fails, terminate processing. */

   RX.SessionId = get_session();

   *pnew = GlobalAlloc(GMEM_ZEROINIT|GMEM_FIXED,
                               size);

   if (*pnew) {
                                      /* initialize the block        */
      memset((*pnew),0,sizeof(QUEUEHEADER));
                                      /* point to name location      */
      (*pnew)->queue_name = (PSZ)((*pnew) + 1);

      if (!name) {                    /* If no name                  */
        tag = (ULONG)*pnew;           /* get value of pointer        */
        name = (*pnew)->queue_name;   /* new name will be buffer     */
        for (;;) {                    /* now create a unique name    */
          sprintf(name,               /* create a new queue name     */
                  rxqueue_name_mask,  /* from the session id         */
                  RX.SessionId,
                  (ULONG)tag);
                                      /* if unique, we're done       */
          if (!qusearch(name))
            break;                    /* get out                     */
          tag++;                      /* try a new number            */
        }
      }
      else {
                                      /* copy the name over          */
        strcpy((*pnew)->queue_name,name);
                                      /* make it uppercase           */
        memupper((*pnew)->queue_name, strlen((*pnew)->queue_name));
      }
      if (create_queue_sem(*pnew)) /* create queue semaphores    */
      {
         GlobalFree(pnew);                  /* release the header         */
         return RXQUEUE_MEMFAIL;
      }

      EnterCriticalSection(&nest);
      (*pnew)->next = RX.base;          /* Insert queue into chain.    */
      RX.base = *pnew;                /* and update the anchor       */
      (*pnew)->process_count = 0;
      LeaveCriticalSection(&nest);
      return (RXQUEUE_OK);                   /* worked fine                 */
   }
   return(RXQUEUE_MEMFAIL);                  /* have an error               */
}


/*********************************************************************/
/*                                                                   */
/*  Function:       queue_allocate_session()                         */
/*                                                                   */
/*  Description:    Allocate storage for the session queue.          */
/*                                                                   */
/*  see queue_allocate                                               */
/*  instead of RX_base, RX.session_base is set                       */
/*********************************************************************/
LONG   queue_allocate_session(
  PQUEUEHEADER       *pnew)            /* New queue header (returned)*/
{
   LONG  size;                         /* size to allocate           */

   size = strlen("SESSION") + 1 + sizeof(QUEUEHEADER);
   RX.SessionId = get_session();
   *pnew = GlobalAlloc(GMEM_ZEROINIT|GMEM_FIXED,
                               size);
   if (*pnew) {
                                      /* initialize the block        */
      memset((*pnew),0,sizeof(QUEUEHEADER));
                                      /* point to name location      */
      (*pnew)->queue_name = (PSZ)((*pnew) + 1);

      strcpy((*pnew)->queue_name,"SESSION");
                                      /* make it uppercase           */
      memupper((*pnew)->queue_name, strlen((*pnew)->queue_name));

      if (create_queue_sem(*pnew)) {   /* create queue semaphores    */
         GlobalFree(pnew);                  /* release the header         */
         return RXQUEUE_MEMFAIL;
      }

      EnterCriticalSection(&nest);
      (*pnew)->next = RX.session_base;    /* Insert queue into chain.    */
      RX.session_base = *pnew;         /* and update the anchor       */
      (*pnew)->process_count = 0;
      LeaveCriticalSection(&nest);
      return (RXQUEUE_OK);                      /* worked fine                 */
   }
   return(RXQUEUE_MEMFAIL);                     /* have an error               */
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

LONG   alloc_queue_entry(
  ULONG       size,                   /* size of queue entry.        */
  PQUEUEITEM  * element,                /* Address of queue element    */
  PUCHAR      data)                   /* actual queue data           */
{
   LONG     rc = RXQUEUE_OK;          /* Function result.            */
                                      /* first allocate header block */
   *element = (PQUEUEITEM) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(QUEUEITEM)+size);
   if (!*element) return RXQUEUE_MEMFAIL;

   (*element)->queue_element = (PUCHAR)((*element)+1);
                                      /* next get the queue element  */
                                      /* copy the queue data in      */
                                      /* if we actually have any     */
   if (size)
       memcpy((*element)->queue_element,data,size);
   (*element)->size = size;                 /* fill in the size            */
   (void)GetSystemTime(&((*element)->addtime));
                                      // add create time ew
   return(RXQUEUE_OK);
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
ULONG  APIENTRY RexxCreateQueue(
  PSZ     name,                        /* Internal name (returned).  */
  ULONG   size,                        /* Length of name buffer.     */
  PSZ     usrrequest,                  /* Desired name.              */
  PULONG  pdup)                        /* Duplicate name flag.       */
{
  ULONG        rc= RXQUEUE_OK;
  RXQUEUE_TALK * intercom;

  if (usrrequest) {                    /* given a name?              */
                                       /* Is the user's name valid?  */
    if ((!val_queue_name(usrrequest)) ||
        (!rxstricmp(usrrequest,        /* cannot create a queue named*/
        "SESSION"))) {                 /* "SESSION"                  */
      return (RXQUEUE_BADQNAME);       /* return with proper code    */
    }

  }

  if (usrrequest) {                    /* if a good name, then we    */
                                       /* need enough space to return*/
                                       /* the name                   */
    if (strlen(usrrequest) >= size)    /* big enough?                */
      return RXQUEUE_STORAGE;            /* nope, set a bad return code*/
  }
  else {                               /* need space for default name*/
    if ((INTERNAL_NAME_SIZE + 1) >= size )
      return RXQUEUE_STORAGE;            /* this is error also         */
  }

  APISTARTUP_QUEUE();

  intercom = FillQueueComBlock(FALSE, 0, RXQUEUE_NOWAIT, NULL, NULL, NULL, usrrequest, 0);  /* just fillin queue name */
  if (!intercom)
     rc = RXQUEUE_MEMFAIL;         /* out of memory, stop        */
  else
     rc = MySendMessage(RXAPI_QUEUECREATE,
                             (WPARAM)0,
                             (LPARAM)0);
  if (rc == RXQUEUE_OK)
     strncpy(name, intercom->qName, size);
  else rc = RXQUEUE_MEMFAIL;

  /* copy duplicate flag from AddFlag */
  *pdup = intercom->AddFlag;

  APICLEANUP_QUEUE();
  return (rc);                         /* and exit                   */
}



APIRET APISessionQueue(ULONG Pid, BOOL newProcess)
{
    ULONG result = RXQUEUE_OK;
    PQUEUEHEADER pnew;

    result = (LRESULT)0;

    pnew = search_session(Pid, &result);
    if (pnew && newProcess) pnew->process_count++;
    return result;
}



APIRET APICreateQueue(ULONG Pid, BOOL newProcess)
{
    ULONG result = RXQUEUE_OK;
    LONG rc;
    PQUEUEHEADER pnew;
    RXQUEUE_TALK * intercom;

    intercom = (RXQUEUE_TALK *) RX.comblock[API_QUEUE];

    if (Pid && newProcess)    /* come here when called from search_session */
    {
        rc = queue_allocate_session(&pnew);
        if (!rc)
        {
            pnew->queue_session = Pid;
            strcpy(pnew->queue_name, "SESSION");
            return (APIRET) pnew;
        } else return NULL;
    }
    else
        /* The AddFlag is used to return the duplicate flag. This has not been */
        /* used before and was always set to 0, so it is safe to use now.      */
        rc = queue_allocate(intercom->qName,&pnew, (PUSHORT)&(intercom->AddFlag));

    if (!rc)
        strcpy(intercom->qName,pnew->queue_name);

    result = rc;
    return result;
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
/*  Notes:            Must tell the queue data manager to            */
/*                    delete the queue entries.                      */
/*                                                                   */
/*  Input:            external queue name.                           */
/*                                                                   */
/*  Effects:          Queue and all its entries deleted.             */
/*                                                                   */
/*********************************************************************/
ULONG  APIENTRY RexxDeleteQueue(
  PSZ name)                            /* name of queue to delete    */
{
  ULONG        rc = RXQUEUE_NOTREG;    /* return code from call      */
  RXQUEUE_TALK * intercom;

  if (!rxstricmp(name,"SESSION"))      /* trying to delete "SESSION" */
    return (RXQUEUE_BADQNAME);         /*   then signal an error     */

  if (!val_queue_name(name))           /* Did the user supply a      */
                                       /* valid name?                */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */

  APISTARTUP_QUEUE();

  intercom = FillQueueComBlock(FALSE, 0, RXQUEUE_NOWAIT, NULL, NULL, NULL, name, 0);  /* just fillin queue name */
  if (!intercom)
     rc = RXQUEUE_MEMFAIL;         /* out of memory, stop        */
  else
     rc  = (ULONG)MySendMessage(RXAPI_QUEUEDELETE,
                               (WPARAM)0,
                               (LPARAM)0);
  APICLEANUP_QUEUE();
  return (rc);
}


APIRET APIDeleteQueue(ULONG Pid, BOOL SessionQ)
{
    PQUEUEITEM   curr_item;              /* current queue item         */
    PQUEUEITEM   next_item;              /* next queue item            */
    PQUEUEHEADER previous=NULL;          /* previous list entry        */
    PQUEUEHEADER current;                /* current list entry         */
    ULONG rc = RXQUEUE_NOTREG;
    RXQUEUE_TALK * intercom;

    intercom = (RXQUEUE_TALK *) RX.comblock[API_QUEUE];

    if (!SessionQ)
        current = RX.base;                   /* get queue base             */
    else
        current = RX.session_base;          /* get session queue base             */

    while (current) {                    /* loop until done            */
        if ((SessionQ && (current->queue_session == Pid))
           || (!SessionQ && !rxstricmp(intercom->qName,current->queue_name)))
        {
            if (current->waiting) {          /* if someone waiting on this */
                rc=RXQUEUE_ACCESS;             /* tell the caller            */
                break;                         /* and get out of here        */
            }
            /* is more than one process owner of session queue */
            if (SessionQ && (current->process_count > 1)) {
                rc = current->process_count;
                current->process_count--;
                break;                         /* get out of here        */
            }

            EnterCriticalSection(&nest);     /* start of critical section  */
            curr_item = current->queue_first;/* get head of the queue      */
            while (curr_item) {              /* while more items on queue  */
                next_item = curr_item->next;   /* get point to next one      */
                GlobalFree(curr_item);         /* return storage for this one*/
                curr_item = next_item;         /* step to the next item      */
            }
            if (!previous) {                  /* if releasing first item    */
                if (SessionQ) RX.session_base = current->next;  /* just remove from front     */
                else RX.base = current->next;
            } else previous->next = current->next; /* we need to close up the chain   */
            delete_queue_sem(current);       /* get rid of semaphores      */
            GlobalFree(current);

            rc=RXQUEUE_OK;                   /* set good return code       */
            LeaveCriticalSection(&nest);      /* end of critical section    */
            break;                           /* and get out of here        */
        }
        previous = current;                /* save predecessor           */
        current = current->next;           /* step to next block         */
    }
    return(rc);
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
  ULONG         rc = RXQUEUE_NOTINIT;
  ULONG            pid;
  int           cn;
  RXQUEUE_TALK * intercom;

  *count=0;                            /* initialize the count       */

  if (!val_queue_name(name))           /* Did the user supply a      */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */

  APISTARTUP_QUEUE();

  if (!rxstricmp(name,"SESSION")) {    /* trying to delete "SESSION" */
      pid = search_session_in_API(count, FALSE);        /* get current session queue  */
      if (pid)                      /* found or allocated?        */
          rc = RXQUEUE_OK;                 /* set return code            */
  }
  else
  {
      intercom = FillQueueComBlock(FALSE, 0, RXQUEUE_NOWAIT, NULL, NULL, NULL, name, 0);  /* just fillin queue name */
      if (intercom)
      {
          cn = (ULONG)MySendMessage(RXAPI_QUEUEQUERY,
                                     (WPARAM)0,
                                     (LPARAM)0);
          if (cn != -1) {
             rc = RXQUEUE_OK;
             *count = cn;
          } else rc = RXQUEUE_NOTREG;
      }    else rc = RXQUEUE_MEMFAIL;         /* out of memory, stop        */
  }
  APICLEANUP_QUEUE();

  return (rc);                         /* return with return code    */
}



LONG APIQueryQueue(void)
{
    PQUEUEHEADER current;
    RXQUEUE_TALK * intercom;

    intercom = (RXQUEUE_TALK *) RX.comblock[API_QUEUE];

    if ((current = qusearch(intercom->qName)))    /* if the queue exists        */
        return current->item_count;    /* return the current count   */
    else                               /* doesn't exist              */
        return -1;                     /* set the error code         */
}


/* The following function is to get access to an event semaphore
   of the RXAPI.EXE process */
HANDLE GetAccessToHandle(ULONG procid, HANDLE hnd)
{
   HANDLE Lclhndl;
   HANDLE result;
   BOOL rc;

   /* get acces to RXAPI */
   Lclhndl = OpenProcess(STANDARD_RIGHTS_REQUIRED|
                         PROCESS_DUP_HANDLE,
                         TRUE,
                         procid);

   rc = DuplicateHandle(Lclhndl,
                   hnd,
                   GetCurrentProcess(),
                   (LPHANDLE)&result,
                   DUPLICATE_SAME_ACCESS,
                   TRUE,
                   DUPLICATE_SAME_ACCESS);

   CloseHandle(Lclhndl);
   return result;
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
  ULONG  pid;
  ULONG  count;
  RXQUEUE_TALK * intercom;

  if (!val_queue_name(name))           /* Did the user supply a valid name? */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */

                                       /* first check the flag       */
  if (flag!=RXQUEUE_FIFO && flag!=RXQUEUE_LIFO)
    return (RXQUEUE_PRIORITY);         /* error, tell the caller     */

  if (!API_RUNNING()) return (RXQUEUE_MEMFAIL);

  if (!CheckQueueComBlock())
      return(RXQUEUE_MEMFAIL);

  if (!rxstricmp(name,"SESSION"))
     pid = queue_get_pid(&count);
  else
     pid = 0;

  APISTARTUP_QUEUE();
  intercom = FillQueueComBlock(TRUE, flag, RXQUEUE_NOWAIT, data->strptr, data->strlength, NULL, name, pid);
  if (!intercom)
     rc = RXQUEUE_MEMFAIL;         /* out of memory, stop        */
  else
     rc = (ULONG)MySendMessage(RXAPI_QUEUEADD,
                               (WPARAM)0,
                               (LPARAM)GetCurrentProcessId());
  APICLEANUP_QUEUE();

  return (rc);                        /* return with return code    */
}




APIRET APIAddQueue()
{
    ULONG result = RXQUEUE_OK;
    PQUEUEITEM item;
    PQUEUEHEADER current;
    RXQUEUE_TALK * intercom;
    ULONG count;

    intercom = (RXQUEUE_TALK *) RX.comblock[API_QUEUE];

    if (intercom->ProcessID)
        current = search_session(intercom->ProcessID, &count);
    else
        current = qusearch(intercom->qName);   /* if the queue exists        */
    if (!current)                    /* no queue pointer, then     */
        result = RXQUEUE_NOTREG;    /* we have an unknown queue   */
                                    /* try to allocate an element */
    else {
         /* calculate absolute address */
        PUCHAR dataptr = (PUCHAR) ((ULONG) intercom + (ULONG) intercom->queue_item.queue_element);

        result = alloc_queue_entry(intercom->queue_item.size, &item, dataptr);

        if (result == RXQUEUE_OK)
        {
               if (current->queue_first==NULL) /* if queue is empty,  */
                current->queue_first = current->queue_last = item;  /* just add to front   */
              else {                   /* otherwise, we need to do   */
                if (intercom->AddFlag==RXQUEUE_LIFO) {    /* it either LIFO       */
                    item->next=current->queue_first;
                    current->queue_first=item;
                }
                else {                 /* or FIFO                    */
                                       /* add to the end             */
                    current->queue_last->next=item;
                    current->queue_last=item; /* set new last pointer */
                      item->next = NULL;  /* nothing after this one     */
                   }
            }
            current->item_count++;   /* update the item count      */
            if (current->waiting)   /* if someone waiting on this */
                SetEvent(current->waitsem);
        }
    }
    return result;
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
ULONG  APIENTRY RexxPullQueue(
  PSZ         name,
  PRXSTRING   data_buf,
  SYSTEMTIME * dt,
  ULONG       waitflag)
{

  ULONG        rc = RXQUEUE_OK;
  PQUEUEITEM   item;
  ULONG envvalue;
  ULONG pid=0;
  RXQUEUE_TALK * intercom;
  HANDLE wsem;
  ULONG result;
                                       /* got a good wait flag?      */
  if (waitflag!=RXQUEUE_NOWAIT && waitflag!=RXQUEUE_WAIT)
    return (RXQUEUE_BADWAITFLAG);      /* no, just exit              */

  if (!val_queue_name(name))           /* Did the user supply a valid name? */
    return (RXQUEUE_BADQNAME);         /* No, get out.               */

  if (!API_RUNNING()) return (RXQUEUE_MEMFAIL);

  if (!CheckQueueComBlock())
      return(RXQUEUE_MEMFAIL);

  if (!rxstricmp(name,"SESSION"))
     pid = queue_get_pid(&envvalue);

  APISTARTUP_QUEUE();

  intercom = FillQueueComBlock(FALSE, 0, waitflag, NULL, 0, NULL, name, pid);

  result = (LRESULT)MySendMessage(RXAPI_QUEUEPULL,
                                   (WPARAM)0,
                                   (LPARAM)GetCurrentProcessId());

  if ((waitflag==RXQUEUE_WAIT) && (result == RXQUEUE_WAITACTIVE))
  {
      wsem = GetAccessToHandle(RX.MemMgrPid, intercom->WaitSem);
      ResetEvent(wsem);

      ExceptionQueueSem = wsem;

                  /********************************************************/
                  /* Now that other threads have access to the queuing    */
                  /* API, another process may have already added the item */
                  /* we want.  In that case, the semaphore will be posted.*/
                  /********************************************************/
           /* we need to release semaphore before waiting for data,
              otherwise RexxAddQueue hangs in APISTARTUP */

      APICLEANUP_QUEUE();

      /* wait for the event that something has been added to this queue */
      if (WaitForSingleObject(wsem, INFINITE) != WAIT_OBJECT_0)
      {
         CloseHandle(wsem);
         return (RXQUEUE_MEMFAIL);    /* error if we couldn't get it*/
      }

      /* request mutex again */
      APISTARTUP_QUEUE();

      CloseHandle(wsem);
      /* com block surely have been modified while in wait for single object */
      /* so first check size */
      if (!CheckQueueComBlock())
          return(RXQUEUE_MEMFAIL);
      /* and then fill it again */        /* to not increase but decrease ->waiting */
      /* name must be set again to com block because other queue API could have overwritten it */
      intercom = FillQueueComBlock(FALSE, 0, RXQUEUE_ENDWAIT, NULL, 0, NULL, name, pid);

      result = (LRESULT)MySendMessage(RXAPI_QUEUEPULL,
                                    (WPARAM)0,
                                    (LPARAM)GetCurrentProcessId());
      intercom->PullFlag = (WORD)waitflag;  /* put back the real flag */
  }

  if (result != RXQUEUE_OK)
  {
      APICLEANUP_QUEUE();
      return (result);
  }

  item = &intercom->queue_item;
  /* calculate absolue address */
  item->queue_element = (PUCHAR) ((ULONG) intercom + (ULONG) intercom->queue_item.queue_element);

  if (data_buf->strptr &&    /* given a default buffer?    */
     data_buf->strlength >= item->size) {
     if (item->size)         /* if actual data in element  */
                             /* copy the data into the buf */
        memcpy(data_buf->strptr,
               item+1,
               item->size);
                             /* set the proper length      */
     data_buf->strlength = item->size;
     memcpy((PUCHAR)dt,      // set the systemtime info    */
            (PUCHAR)&item->addtime,
            sizeof(SYSTEMTIME));
                            /* get rid if the queue item  */
  } else {                   /* give up memory directly    */
     if (item->size) {        /* if not a null string       */
                            /* allocate a new block       */
         if ( (data_buf->strptr = GlobalAlloc(GMEM_FIXED, item->size)) == NULL) {
             rc = RXQUEUE_MEMFAIL;
                            /* error if we couldn't get it*/
         } else
                            /* copy the data over         */
             memcpy(data_buf->strptr,item+1,
                    item->size);
     } else                   /* set a non-null pointer     */
         data_buf->strptr=NULL;    /* was (PUCHAR)1 before but trapped in SysReleaseResultMemory */
                               /* set the length             */
     memcpy((PUCHAR)dt,       // set the systemtime info    */
            (PUCHAR)&item->addtime,
            sizeof(SYSTEMTIME));
     data_buf->strlength =item->size;
  }
  APICLEANUP_QUEUE();
  return (rc);                         /* return with return code    */
}



APIRET APIPullQueue()
{
    ULONG result = RXQUEUE_OK;
    PQUEUEITEM item;
    PQUEUEHEADER current;
    RXQUEUE_TALK * intercom;
    ULONG count;

    result = (LRESULT)NULL;

    intercom = (RXQUEUE_TALK *) RX.comblock[API_QUEUE];

    if (!intercom->ProcessID)
        current = qusearch(intercom->qName);     /* get current session queue  */
    else
        current = search_session(intercom->ProcessID, &count);

    if (!current)                 /* no queue pointer, then     */
        result = RXQUEUE_NOTREG;    /* we have an unknown queue   */
    else {
        item = NULL;                /* set to no item initially   */
        if (current->item_count)    /* if items in the queue      */
           item = current->queue_first; /* get the first one      */

        /* if no items in the queue and we've been asked to wait for one   */
        if  (!item && (intercom->PullFlag==RXQUEUE_WAIT)) {
           current->waiting++;              /* update the waiters count  */
           intercom->WaitSem = current->waitsem;
           intercom->ProcessID = GetCurrentProcessId();
           return(RXQUEUE_WAITACTIVE);   /* don't wait within RXAPI.EXE */
        }
        if (item) {                 /* if we got an item          */
                                    /* dechain, updating the end  */
                                    /* pointer if necessary       */
            if ((current->queue_first=item->next)==NULL)
                current->queue_last=NULL;
            current->item_count--;    /* reduce the queue size      */

            ReturnQueueItem(item);
            GlobalFree(item);      /* get rid if the queue item  */
            result = RXQUEUE_OK;
        } else
            result = RXQUEUE_EMPTY;    /* queue is empty             */
        if (intercom->PullFlag==RXQUEUE_ENDWAIT)
            current->waiting--;  /* decrease the waiters count */
    }
    intercom->WaitSem = NULL;
    return result;
}


/* Check whether or not local com block has same size as global com block */
BOOL CheckQueueComBlock()
{
    if (!LRX.comblock[API_QUEUE]) return FALSE;
    if (LRX.comblockQueue_ExtensionLevel != RX.comblockQueue_ExtensionLevel)
    {
        UnmapComBlock(API_QUEUE);
        return MapComBlock(API_QUEUE);
    }
    return (BOOL) LRX.comblock[API_QUEUE];
}



/*********************************************************************/
/*                                                                   */
/*  Function:        RxQueueDetach;                                   */
/*                                                                   */
/*  Description:     Close the session queue                         */
/*                                                                   */
/*********************************************************************/
ULONG RxQueueDetach(ULONG pid)
{
   ULONG envcount;

   if (!API_RUNNING()) return (RXQUEUE_MEMFAIL);

   if (!pid) pid = queue_get_pid(&envcount);
   return MySendMessage(RXAPI_QUEUESESSIONDEL,
                                         (WPARAM)pid,
                                         (LPARAM)0);
}
