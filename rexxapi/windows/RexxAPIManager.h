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


#ifndef WINRXAPI_H_INCLUDED
#define WINRXAPI_H_INCLUDED
#include <setjmp.h>

#include "APIServiceTables.h"
#define  MAXNAMESIZE      30           /* maximum internal name size */
#define  REGNOOFTYPES      3           /* Number of types supported. */
#define  NUMBEROFCOMBLOCKS 3
#define  PAGE_SIZE      4096           /* Size for  4K allocations    */
#define  COMREGSIZE    PAGE_SIZE
#define  API_QUEUE_INITIAL_EXTLEVEL  2
#define  API_MACRO_INITIAL_EXTLEVEL  4
#define  MAXQUEUENAME   1024

#define  RXAPI_VERSION     1023 /* should be increased whenever RexxinitExports is modified */
/*Structure Definitions*/


/*********************************************************************/

typedef struct _QUEUEITEM {
    struct _QUEUEITEM *next;
    PUCHAR   queue_element;
    ULONG    size;
    SYSTEMTIME addtime;
    } QUEUEITEM;
typedef QUEUEITEM *PQUEUEITEM;

#define qitem_size sizeof(QUEUEITEM)

/*********************************************************************/

typedef struct _QUEUEHEADER {
    struct _QUEUEHEADER *next;
    ULONG  waiting;                    /* count of processes waiting */
    ULONG item_count;                  /* number of items in queue   */
    ULONG  waitprocess;                /* waitprocess not needed */
    HANDLE  waitsem;                   /* event semaphore for pull   */
    ULONG process_count;               /* to count processes */
                                       /* the session queue is used in */
    PQUEUEITEM queue_first;            /* first queue item           */
    PQUEUEITEM queue_last;             /* last queue item            */
    PSZ   queue_name;                  /* pointer to queue name      */
    ULONG queue_session;               /* session of queue           */
    } QUEUEHEADER;

typedef QUEUEHEADER *PQUEUEHEADER;     /* pointer to a queue header  */

#define qhdr_size sizeof(QUEUEHEADER)

typedef  struct {
   UINT message;
   WPARAM wParam;
   LPARAM lParam;
   LRESULT result;
   BOOL done;
   } RXAPI_MESSAGE;


typedef struct _GLOBALREXXAPIDATA {    /* Do not move next two items */
  LONG          init;                  /* Initialization flag        */
  PQUEUEHEADER  base;                  /* Base of queues             */
  PQUEUEHEADER  session_base;          /* Base for session queues    */
  PAPIBLOCK baseblock[REGNOOFTYPES];   /* Registration chains        */
  ULONG         SessionId;             /* Current session id         */
  PUCHAR        macrobase;             /* Pointer to Macro Space     */
  ULONG         macrocount;            /* count of fncs in macrospc  */
  PUCHAR        InternalMacroPtr;      /* macro Pointer for save/load*/
  ULONG         mcount;                /* number of macros that fit  */
  PCHAR           ListArgPtr;               /* argument list for save/load*/
  ULONG           ListArgCount;           /* argument cnt for save/load */
  HANDLE        comhandle[NUMBEROFCOMBLOCKS];
  PVOID         comblock[NUMBEROFCOMBLOCKS];
  ULONG         comblockQueue_ExtensionLevel;
  ULONG         comblockMacro_ExtensionLevel;
  PID           MemMgrPid;
  ULONG         MemMgrVersion;         /* to check if dlls match */
  RXAPI_MESSAGE msg;
  LONG          UID;
}  REXXAPIDATA;

#define   RX    (*RexxinitExports)        /* Access to global data      */

typedef struct _LOCALREXXAPIDATA {          /* Do not move next two items */
  LONG          local_init;            /* Initialization flag        */
  HANDLE        MutexSem[3];           /* API, Queue, and Macro mutex */
  HANDLE        MsgMutex;               /* RxAPI message mutex */
  HANDLE        MsgEvent;               /* RxAPI message event */
  HANDLE        ResultEvent;            /* RxAPI message result event */
  HANDLE        comhandle[NUMBEROFCOMBLOCKS];
  PVOID         comblock[NUMBEROFCOMBLOCKS];
  ULONG         comblockQueue_ExtensionLevel;
  ULONG         comblockMacro_ExtensionLevel;
  HANDLE        hFMap;                  /* Handle to major memory mapped file */
  LONG          UID;
}  LOCALREXXAPIDATA;

#define   LRX   RexxinitLocal         /* Access to global data      */

   /* this data is used to communicate between rexx and the memory manager */
typedef  struct {
   WORD AddFlag;
   WORD PullFlag;
   HANDLE WaitSem;
   ULONG ProcessID;
   CHAR qName[MAXQUEUENAME];
   QUEUEITEM queue_item;              /* first queue item           */
   } RXQUEUE_TALK;


   /* this data is used to communicate between rexx and the memory manager */
typedef  struct {
    char data[COMREGSIZE - sizeof(ULONG)];
    ULONG curAPI;
   } RXREG_TALK;


   /* this data is used to communicate between rexx and the memory manager */
typedef struct {
      CHAR           name[NAMESIZE];   /* function name              */
      RXSTRING       image;            /* pcode+literals image       */
      ULONG          i_size;           /* size of image              */
      ULONG          srch_pos;         /* search order position      */
      } RXMACRO_TALK;


#define  RXFMLIFO   1                  /* fifo/lifo addition         */

/***    Queing Services */
#ifdef INCL_RXQUEUE

/***    Request flags for External Data Queue access */

#define RXQUEUE_FIFO          0    /* Access queue first-in-first-out */
#define RXQUEUE_LIFO          1    /* Access queue last-in-first-out  */

#define RXQUEUE_NOWAIT        0    /* Wait for data if queue empty    */
#define RXQUEUE_WAIT          1    /* Don't wait on an empty queue    */
#define RXQUEUE_ENDWAIT          2


/***    Return Codes from RxQueue interface */

#define RXQUEUE_OK            0        /* Successful return           */
#define RXQUEUE_NOTINIT       1000     /* Queues not initialized      */

#define RXQUEUE_STORAGE       1        /* Ret info buf not big enough */
#define RXQUEUE_SIZE          2        /* Data size > 64K-64          */
#define RXQUEUE_DUP           3        /* Attempt-duplicate queue name*/
#define RXQUEUE_NOEMEM     1002        /* failure in api manager      */
#define RXQUEUE_BADQNAME      5        /* Not a valid queue name      */
#define RXQUEUE_PRIORITY      6        /* Not accessed as LIFO|FIFO   */
#define RXQUEUE_BADWAITFLAG   7        /* Not accessed as WAIT|NOWAIT */
#define RXQUEUE_EMPTY         8        /* No data in queue            */
#define RXQUEUE_NOTREG        9        /* Queue does not exist        */
#define RXQUEUE_ACCESS       10        /* Queue busy and wait active  */
#define RXQUEUE_MAXREG       11        /* No memory to create a queue */
#define RXQUEUE_MEMFAIL      12        /* Failure in memory management*/
#define RXQUEUE_WAITACTIVE   13



/***    RexxCreateQueue - Create an External Data Queue */



ULONG  APIENTRY RexxCreateQueue (
        PSZ,                           /* Name of queue created       */
        ULONG,                         /* Size of buf for ret name    */
        PSZ,                           /* Requested name for queue    */
        PULONG ) ;                     /* Duplicate name flag.        */


/***    RexxDeleteQueue - Delete an External Data Queue */

ULONG  APIENTRY RexxDeleteQueue (
        PSZ );                         /* Name of queue to be deleted */


/*** RexxQueryQueue - Query an External Data Queue for number of      */
/***                  entries                                         */

ULONG  APIENTRY RexxQueryQueue (
        PSZ,                           /* Name of queue to query      */
        PULONG );                      /* Place to put element count  */


/***    RexxAddQueue - Add an entry to an External Data Queue */

ULONG  APIENTRY RexxAddQueue (
        PSZ,                           /* Name of queue to add to     */
        PRXSTRING,                     /* Data string to add          */
        ULONG );                       /* Queue type (FIFO|LIFO)      */


/***    RexxPullQueue - Retrieve data from an External Data Queue */

ULONG  APIENTRY RexxPullQueue (
        PSZ,                           /* Name of queue to read from  */
        PRXSTRING,                     /* RXSTRING to receive data    */
        SYSTEMTIME * PDATETIME,
        ULONG );                       /* wait status (WAIT|NOWAIT)   */


#endif /* INCL_RXQUEUE */

#define TERM         0xffff    /* service termination flag           */
#define TEST         0xfffe    /* service test flag                  */
#define INIT         0xfffd    /* service initialization             */
#define CREATE_QUEUE 0xfffc    /* queue creation                     */
#define DELETE_QUEUE 0xfffb    /* queue deletion                     */
#define FREE_MEMORY  0xfffa    /* free memory                        */
#define FREE_POOL    0xfff9    /* free memory pool                   */
#define INIT_POOL    0xfff8    /* initialize memory pool             */


#define DosEnterMustComplete(ptr) EnterCriticalSection(ptr)
#define DosExitMustComplete(ptr) LeaveCriticalSection(ptr)

#define MUTEXCOUNT 6
#define API_API 0
#define API_QUEUE 1
#define API_MACRO 2
#define API_MESSAGE 3
#define API_MSGEVENT 4
#define API_RESULTEVENT 5

// the simple string #defines are replaced with two
// tables containing the names, once with "Global\"
// and once without
#define MUTEXNAME_EXISTENCE   APInamedObjects[0]
#define MUTEXNAME_START       APInamedObjects[1]
#define MUTEXNAME_API         APInamedObjects[2]
#define MUTEXNAME_QUEUE       APInamedObjects[3]
#define MUTEXNAME_MACRO       APInamedObjects[4]
#define MUTEXNAME_MESSAGE     APInamedObjects[5]
#define MUTEXNAME_MSGEVENT    APInamedObjects[6]
#define MUTEXNAME_RESULTEVENT APInamedObjects[7]
#define FMAPNAME_INITEXPORTS  APInamedObjects[8]
#define FMAPNAME_COMBLOCK(i)  APInamedObjects[9+i]

#define APISTARTUP_MACRO() \
if (rc=RxAPIStartUp(API_MACRO)) return (rc)

#define APISTARTUP_QUEUE() \
if (rc=RxAPIStartUp(API_QUEUE)) return (rc)

#define APISTARTUP_API() \
if (rc=RxAPIStartUp(API_API)) return (rc)


#define APICLEANUP_MACRO() \
  if (LRX.MutexSem[API_MACRO]) ReleaseMutex(LRX.MutexSem[API_MACRO])

#define APICLEANUP_QUEUE() \
  if (LRX.MutexSem[API_QUEUE]) ReleaseMutex(LRX.MutexSem[API_QUEUE])

#define APICLEANUP_API() \
  if (LRX.MutexSem[API_API]) ReleaseMutex(LRX.MutexSem[API_API])

#define API_RUNNING() \
    ((RexxinitExports != NULL) && (RX.MemMgrPid != 0) && (RX.init != -1))

#ifdef __cplusplus
extern "C" {
#endif

VOID RxFreeProcessSubcomList(ULONG pid);
ULONG RxQueueDetach(ULONG pid);
ULONG RxInterProcessInit(BOOL sessionqueue);

#ifdef INCL_RXMACRO

    APIRET APIENTRY RexxExecuteMacroFunction (
           PSZ,                   /* name of function to locate      */
           PRXSTRING );           /* pointer to return pcode+lits    */
#endif /* INCL_RXMACRO */

#ifdef __cplusplus
}
#endif

ULONG           RxAPIStartUp(int chain);
LONG            FillAPIComBlock(HAPIBLOCK *,
                                PSZ,
                                PSZ,
                                PSZ);
ULONG  RxGetModAddress( PSZ       dll_name,
                        PSZ       function_name,
                        PULONG    error_codes,
                        PFN *     function_address,
                        PULONG    call_type);
ULONG get_session(void);
ULONG Initialize( VOID ) ;

extern __declspec(dllexport) SECURITY_ATTRIBUTES * SetSecurityDesc(SECURITY_ATTRIBUTES * sa);
LRESULT MySendMessage(UINT msg, WPARAM wP, LPARAM lP);

#endif


