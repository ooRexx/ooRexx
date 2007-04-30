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
/*****************************************************************************/
/* REXX AIX Support                                             aixthrds.h   */
/*                                                                           */
/* Threading support for AIX/LINUX                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef LINUX_THREADS_DEFINED
#define LINUX_THREADS_DEFINED

#include <pthread.h>
#include <stdlib.h>

void SysThreadInit(void);
int SysQueryThreadID();
#ifdef AIX
void RxLumCntl( long * result );
#endif

class RexxSemaphore {
   public:
     RexxSemaphore();
     ~RexxSemaphore();
     void post();
     void wait();
     void wait(long); /* needed for async timer in aixtime.c */
     void reset();
     unsigned long posted();
  private:
     pthread_cond_t  semCond;
     pthread_mutex_t semMutex;
     unsigned long   value;
};

class RexxMutex {
  public:
     RexxMutex();
     ~RexxMutex();
#ifdef OPSYS_AIX43
     int request() { int rc; rc = pthread_mutex_lock(&mutexMutex); if(rc) mutex_value++;return rc;}  /* @THU007A */
     int release() { int rc; if (!mutex_value) {rc = pthread_mutex_unlock(&mutexMutex);} else{ mutex_value--;rc= 0;};return rc; SysThreadYield();}/* @THU007A return before yield else guard problem*/
#else
     int request() { return pthread_mutex_lock(&mutexMutex);}
//   int release() { int rc; rc = pthread_mutex_unlock(&mutexMutex);
     int release() { int rc; rc = pthread_mutex_unlock(&mutexMutex); return(rc);} /* @MIC010A */
#endif
     int requestImmediate() { return pthread_mutex_trylock(&mutexMutex);}
  private:
     pthread_mutex_t mutexMutex;
//     unsigned long mutex_value;
     long mutex_value;
};

typedef struct _sysThreadArg {
        long    threadId;
        pthread_attr_t  threadAttr;
        void*   args;
} SysThreadArg;


#endif

