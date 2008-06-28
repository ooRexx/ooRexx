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

#include "rexx.h"

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
    char    *queue_element;
    size_t   size;
    SYSTEMTIME addtime;
    } QUEUEITEM;
typedef QUEUEITEM *PQUEUEITEM;

#define qitem_size sizeof(QUEUEITEM)

/*********************************************************************/

typedef struct _QUEUEHEADER {
    struct _QUEUEHEADER *next;
    ULONG  waiting;                    /* count of processes waiting */
    size_t item_count;                 /* number of items in queue   */
    ULONG   waitprocess;               /* waitprocess not needed */
    HANDLE  waitsem;                   /* event semaphore for pull   */
    size_t process_count;              /* to count processes */
                                       /* the session queue is used in */
    PQUEUEITEM queue_first;            /* first queue item           */
    PQUEUEITEM queue_last;             /* last queue item            */
    char *queue_name;                  /* pointer to queue name      */
    ULONG queue_session;               /* session of queue           */
    } QUEUEHEADER;

typedef QUEUEHEADER *PQUEUEHEADER;     /* pointer to a queue header  */

#define qhdr_size sizeof(QUEUEHEADER)

#define RXQUEUE_WAITACTIVE   13
#define RXQUEUE_ENDWAIT          2

typedef  struct {
   UINT message;
   WPARAM wParam;
   LPARAM lParam;
   LRESULT result;
   BOOL done;
   } RXAPI_MESSAGE;


typedef struct _GLOBALREXXAPIDATA {    /* Do not move next two items */
  int           init;                  /* Initialization flag        */
  PQUEUEHEADER  base;                  /* Base of queues             */
  PQUEUEHEADER  session_base;          /* Base for session queues    */
  PAPIBLOCK baseblock[REGNOOFTYPES];   /* Registration chains        */
  process_id_t  SessionId;             /* Current session id         */
  char         *macrobase;             /* Pointer to Macro Space     */
  size_t        macrocount;            /* count of fncs in macrospc  */
  char         *InternalMacroPtr;      /* macro Pointer for save/load*/
  size_t        mcount;                /* number of macros that fit  */
  char         *ListArgPtr;            /* argument list for save/load*/
  size_t        ListArgCount;          /* argument cnt for save/load */
  HANDLE        comhandle[NUMBEROFCOMBLOCKS];
  void         *comblock[NUMBEROFCOMBLOCKS];
  size_t        comblockQueue_ExtensionLevel;
  size_t        comblockMacro_ExtensionLevel;
  process_id_t  MemMgrPid;
  ULONG         MemMgrVersion;         /* to check if dlls match */
  RXAPI_MESSAGE msg;
  LONG          UID;
}  REXXAPIDATA;

#define   RX    (*RexxinitExports)        /* Access to global data      */

typedef struct _LOCALREXXAPIDATA {          /* Do not move next two items */
  int           local_init;            /* Initialization flag        */
  HANDLE        MutexSem[3];           /* API, Queue, and Macro mutex */
  HANDLE        MsgMutex;               /* RxAPI message mutex */
  HANDLE        MsgEvent;               /* RxAPI message event */
  HANDLE        ResultEvent;            /* RxAPI message result event */
  HANDLE        comhandle[NUMBEROFCOMBLOCKS];
  void         *comblock[NUMBEROFCOMBLOCKS];
  size_t        comblockQueue_ExtensionLevel;
  size_t        comblockMacro_ExtensionLevel;
  HANDLE        hFMap;                  /* Handle to major memory mapped file */
  LONG          UID;
}  LOCALREXXAPIDATA;

#define   LRX   RexxinitLocal         /* Access to global data      */

   /* this data is used to communicate between rexx and the memory manager */
typedef  struct {
   WORD AddFlag;
   WORD PullFlag;
   HANDLE WaitSem;
   process_id_t ProcessID;
   char qName[MAXQUEUENAME];
   QUEUEITEM queue_item;              /* first queue item           */
   } RXQUEUE_TALK;


   /* this data is used to communicate between rexx and the memory manager */
typedef  struct {
    char data[COMREGSIZE - sizeof(size_t)];
    size_t curAPI;
   } RXREG_TALK;


   /* this data is used to communicate between rexx and the memory manager */
typedef struct {
      char           name[NAMESIZE];   /* function name              */
      RXSTRING       image;            /* pcode+literals image       */
      size_t         i_size;           /* size of image              */
      size_t         srch_pos;         /* search order position      */
      } RXMACRO_TALK;


#define  RXFMLIFO   1                  /* fifo/lifo addition         */

#define TERM         0xffff    /* service termination flag           */
#define TEST         0xfffe    /* service test flag                  */
#define INIT         0xfffd    /* service initialization             */
#define CREATE_QUEUE 0xfffc    /* queue creation                     */
#define DELETE_QUEUE 0xfffb    /* queue deletion                     */
#define FREE_MEMORY  0xfffa    /* free memory                        */
#define FREE_POOL    0xfff9    /* free memory pool                   */
#define INIT_POOL    0xfff8    /* initialize memory pool             */

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

void RxFreeProcessSubcomList(process_id_t pid);
RexxReturnCode RxQueueDetach(process_id_t pid);
ULONG RxInterProcessInit(BOOL sessionqueue);

#ifdef __cplusplus
}
#endif

ULONG           RxAPIStartUp(int chain);
int             FillAPIComBlock(HAPIBLOCK *, const char *, const char *, const char *);
int  RxGetModAddress(const char *dll_name, char *function_name, int *error_codes, REXXPFN *function_address);
BOOL  Initialize( VOID ) ;

extern _declspec(dllexport) SECURITY_ATTRIBUTES * SetSecurityDesc(SECURITY_ATTRIBUTES * sa);
LRESULT MySendMessage(UINT msg, WPARAM wP, LPARAM lP);

#endif


