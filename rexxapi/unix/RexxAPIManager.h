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
#include "rexx.h"
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

int    RegDeregFunc(const char *, int);  /* Drop all api blocks from the chain */
int rxstricmp(const char *, const char *);

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
    size_t   next;
    size_t   queue_element;
    size_t   size;
    REXXDATETIME addtime;
    } QUEUEITEM;
typedef QUEUEITEM *PQUEUEITEM;


typedef struct _QUEUEHEADER {
    size_t  next;
    size_t waiting;                    /* count of processes waiting */
    size_t item_count;                 /* number of items in queue   */
    process_id_t waitprocess;          /* process waiting on queue   */
    int   waitsem;                     /* event semaphore for pull   */
    int   enqsem;                      /* pull exclusion semaphore   */
    size_t     queue_first;            /* first queue item           */
    size_t     queue_last;             /* last queue item            */
    char   queue_name[MAXNAME];        /* queue name                 */
    process_id_t queue_session;        /* session of queue (<=> process group ID of the greating porcess)*/
    } QUEUEHEADER;

typedef QUEUEHEADER *PQUEUEHEADER;     /* pointer to a queue header  */




typedef struct _MEMORYBLOCK {
    struct _MEMORYBLOCK *next;         /* Next block in chain        */
    char   *low_bound;                 /* Low bound of block         */
    char   *high_bound;                /* High address of block      */
    size_t  allocations;               /* number of allocations made */
    } MEMORYBLOCK;
typedef MEMORYBLOCK *PMEMORYBLOCK;

typedef struct _MEMORYBASE {
    PMEMORYBLOCK   base;               /* base of allocated memory   */
    size_t         count;              /* count of allocated blocks  */
    } MEMORYBASE;
typedef MEMORYBASE *PMEMORYBASE;


typedef struct _REXXAPIDATA {          /* Do not move next two items */
  int           init;                  /* Initialization flag        */
  size_t        queue_handle;          /* Rexx queue handle          */
  size_t        base;                  /* Base of queues             */
  size_t        session_base;          /* Base for session queues    */
  char         *queue_buf_ptr;         /* Address of queue buffer    */
  char         *qbase;                 /* ptr to the queue memory pool*/
  int           qbasememId;            /* memory ID of the pool      */
  size_t        qmemsize;              /* Size of the queue space    */
  size_t        qmemsizeused;          /* THU006A */
  size_t        trialcounter;          /* THU006A */
  size_t        qmemtop;               /* number of bytes used in the queue space */
  int           qsemfree[MAXSEM+1];    /* Indicates the unused semaphores */
  int           qsemcount;             /* semaphore count            */
  int           rexxapisemaphore;      /* Initialization semaphore and queue semaphores  */
  process_id_t  init_processid;        /* Initial processid          */
  size_t        num_sessions;          /* Number of possible sessions*/
  size_t        baseblock[REGNOOFTYPES];/* Registration chains(offsets)*/
  char         *sebase;                /* ptr to the se memory pool     */
  size_t        sememsize;             /* current size of the se memory pool*/
  size_t        sememtop;              /* number of bytes used in se space */
  int           sebasememId;           /* memory ID of the pool      */
  MEMORYBASE    memory_base;           /* Memory management chain    */
  MEMORYBASE    macro_base;            /* Memory management chain    */
  process_id_t  ProcessId;             /* Current process id         */
  thread_id_t   ThreadId;              /* Current thread id          */
  process_id_t  SessionId;             /* Current session id         */
  size_t        mbase;                 /* ptr to macro space fnc lst */
  char         *macrobase;             /* Pointer to Macro Space memory pool    */
  int           mbasememId;            /* memory ID of the pool      */
  size_t        macrosize;             /* Size of the macro space    */
  size_t        mmemtop;               /* number of bytes used in macro space */
  size_t        macrocount;            /* count of fncs in macrospc  */
  size_t        mcount;                /* count of fncs for tmp list */
  int           rexxutilsems;          /* ID for the util semaphore set*/
  SEMCONT       utilsemfree[MAXUTILSEM];/* control array for util semaphores*/
  int           moveareaqid;
  char         *moveareaq;             /* performance fix for queue  */
}  REXXAPIDATA;

