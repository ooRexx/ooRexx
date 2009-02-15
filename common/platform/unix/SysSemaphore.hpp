/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/* REXX Unix Support                                                         */
/*                                                                           */
/* Semaphore support for Unix systems                                        */
/*                                                                           */
/*****************************************************************************/

#ifndef SysSemaphore_DEFINED
#define SysSemaphore_DEFINED

#include <pthread.h>
#include <stdlib.h>
#include "rexx.h"

class SysSemaphore {
public:
     SysSemaphore() : postedCount(0), created(false) { }
     SysSemaphore(bool);
     ~SysSemaphore() { ; }
     void create();
     inline void open() { ; }
     void close();
     void post();
     void wait();
     bool wait(uint32_t);
     void reset();
     inline bool posted() { return postedCount != 0; }

protected:
     pthread_cond_t  semCond;
     pthread_mutex_t semMutex;
     int postedCount;
     bool created;
};

class SysMutex {
public:
     SysMutex() : created(false) { ; }
     SysMutex(bool);
     ~SysMutex() { ; }
     void create();
     inline void open() { ; }
     void close();
     inline void request() { pthread_mutex_lock(&mutexMutex); }
     inline void release() { pthread_mutex_unlock(&mutexMutex); }
     inline bool requestImmediate() { return pthread_mutex_trylock(&mutexMutex) == 0;}

protected:
     pthread_mutex_t mutexMutex;
     bool created;
};
#endif
