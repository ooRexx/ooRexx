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
#include "PlatformDefinitions.h"
#include SYSREXXSAA
#include "ThreadSupport.hpp"
#include "APIDefinitions.h"

/*********************************************************************/
/*                                                                   */
/*  Registration types.                                              */
/*                                                                   */
/*********************************************************************/

#define  REGSUBCOMM    0               /* Register a subcommand.     */
#define  REGSYSEXIT    1               /* Register a system exit     */
#define  REGFUNCTION   2               /* Register a function.       */

#define MACROMEM       1
#define MSTDSIZE       0x40000         /* min size of macro memory pool (256k) */
#define SEMEM          2
#define SESTDSIZE      0x20000         /* min size of se memory pool (128k)   */
#define QMEM           3
#define QSTDSIZE       0x200000        /* min size of queue memory pool (1MB) */
#define QMEMSESSION    4
#define QMEMNAMEDQUE   5

#ifdef LINUX
# ifdef LIMITED_SOLARIS_SEMS
/* Solaris 8 and 9 only allow a maximum of 25 semaphores per set */
#  define MAXSEM         25             /* max number of queue semaphores (per user)*/
#  define MAXUTILSEM     25             /* max number of util semaphores  (per user)*/
# else
#  define MAXSEM         48             /* max number of queue semaphores (per user)*/
#  define MAXUTILSEM     32             /* max number of util semaphores  (per user)*/
# endif
#else  /* AIX */
/* #define MAXSEM         129          */
/* #define MAXSEM         32767        */
# define MAXSEM         4096
/* #define MAXUTILSEM     128          */
/* #define MAXUTILSEM     4096         */
# define MAXUTILSEM     1024
#endif
/* Defines which chain to attach/detach                              */

#define SECHAIN           0             /* Subcom,func and exits     */
#define QUEUECHAIN        1
#define MACROCHAIN        2
#define ALLCHAINS         3

/* Access to the structs evaluating the offsets                      */
#define SHM_OFFSET        4             /* Align to 4 not 1Byte      */

#define MDATA(offset) ((PMACRO)(apidata->macrobase+offset))
#define SEDATA(offset) ((PAPIBLOCK)(apidata->sebase+offset))
#define QHDATA(offset)  ((PQUEUEHEADER)(apidata->qbase+offset))
#define QIDATA(offset)   ((PQUEUEITEM)(apidata->qbase+offset))
#define QDATA(offset)   (apidata->qbase+offset)

ULONG  RegDeregFunc(PSZ, LONG );  /* Drop all api blocks from the chain */
INT rxstricmp(PSZ, PSZ);

#define APISTARTUP(chain){\
  if(RxAPIStartUp(chain))\
    printf("Error while entering common API code !");\
}

/* Definition of signal control to block or release                   */
#define SIGCNTL_BLOCK       0
#define SIGCNTL_RELEASE     1

#define APICLEANUP(chain) RxAPICleanUp(chain, SIGCNTL_RELEASE)

#define  REGNOOFTYPES      3           /* Number of types supported. */

typedef struct _QUEUEITEM {
    ULONG    next;
    ULONG    queue_element;
    ULONG    size;
    DATETIME addtime;
    } QUEUEITEM;
typedef QUEUEITEM *PQUEUEITEM;


typedef struct _QUEUEHEADER {
    ULONG   next;
    ULONG  waiting;                    /* count of processes waiting */
    ULONG item_count;                  /* number of items in queue   */
    PID   waitprocess;                 /* process waiting on queue   */
    KMTX  waitsem;                     /* event semaphore for pull   */
    KMTX  enqsem;                      /* pull exclusion semaphore   */
    ULONG      queue_first;            /* first queue item           */
    ULONG      queue_last;             /* last queue item            */
    CHAR   queue_name[MAXNAME];        /* queue name                 */
    PID queue_session;               /* session of queue (<=> process group ID of the greating porcess)*/
    } QUEUEHEADER;

typedef QUEUEHEADER *PQUEUEHEADER;     /* pointer to a queue header  */




typedef struct _MEMORYBLOCK {
    struct _MEMORYBLOCK *next;         /* Next block in chain        */
    PUCHAR  low_bound;                 /* Low bound of block         */
    PUCHAR  high_bound;                /* High address of block      */
    ULONG   allocations;               /* number of allocations made */
    } MEMORYBLOCK;
typedef MEMORYBLOCK *PMEMORYBLOCK;

typedef struct _MEMORYBASE {
    PMEMORYBLOCK   base;               /* base of allocated memory   */
    ULONG          count;              /* count of allocated blocks  */
    } MEMORYBASE;
typedef MEMORYBASE *PMEMORYBASE;


typedef struct _REXXAPIDATA {          /* Do not move next two items */
  LONG          init;                  /* Initialization flag        */
  HQUEUE        queue_handle;          /* Rexx queue handle          */
  ULONG         base;                  /* Base of queues             */
  ULONG         session_base;          /* Base for session queues    */
  PUCHAR        queue_buf_ptr;         /* Address of queue buffer    */
  PCHAR         qbase;                 /* ptr to the queue memory pool*/
  INT           qbasememId;            /* memory ID of the pool      */
  ULONG         qmemsize;              /* Size of the queue space    */
  ULONG         qmemsizeused;          /* THU006A */
  ULONG         trialcounter;          /* THU006A */
  ULONG         qmemtop;               /* number of bytes used in the queue space */
  INT           qsemfree[MAXSEM+1];    /* Indicates the unused semaphores */
  INT           qsemcount;             /* semaphore count            */
  KMTX          rexxapisemaphore;      /* Initialization semaphore and queue semaphores  */
  PID           init_processid;        /* Initial processid          */
  ULONG         num_sessions;          /* Number of possible sessions*/
  ULONG         baseblock[REGNOOFTYPES];/* Registration chains(offsets)*/
  PCHAR         sebase;                /* ptr to the se memory pool     */
  ULONG         sememsize;             /* current size of the se memory pool*/
  ULONG         sememtop;              /* number of bytes used in se space */
  INT           sebasememId;           /* memory ID of the pool      */
  MEMORYBASE    memory_base;           /* Memory management chain    */
  MEMORYBASE    macro_base;            /* Memory management chain    */
  PID           ProcessId;             /* Current process id         */
  TID           ThreadId;              /* Current thread id          */
  ULONG         SessionId;             /* Current session id         */
  ULONG         mbase;                 /* ptr to macro space fnc lst */
  PCHAR         macrobase;             /* Pointer to Macro Space memory pool    */
  INT           mbasememId;            /* memory ID of the pool      */
  ULONG         macrosize;             /* Size of the macro space    */
  ULONG         mmemtop;               /* number of bytes used in macro space */
  ULONG         macrocount;            /* count of fncs in macrospc  */
  ULONG         mcount;                /* count of fncs for tmp list */
  INT           rexxutilsems;          /* ID for the util semaphore set*/
  SEMCONT       utilsemfree[MAXUTILSEM];/* control array for util semaphores*/
//ULONG         lazy_block;            /* performance fix            */
//ULONG         lazy_size;             /* performance fix            */
  INT           moveareaqid;
  PCHAR         moveareaq;             /* performance fix for queue  */
}  REXXAPIDATA;

