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
/* REXX AIX Support                                             aixthrds.c   */
/*                                                                           */
/* Threading support for AIX/LINUX                                           */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined( HAVE_PTHREAD_H )
# include <pthread.h>
#endif

#if defined( HAVE_MEMORY_H )
# include <memory.h>
#endif

#if defined( HAVE_SYS_SCHED_H )
# include <sys/sched.h>
#endif

#if defined( HAVE_TIME_H )
# include <time.h>
#endif

#if defined( HAVE_SCHED_H )
# include <sched.h>
#endif

#include "RexxCore.h"
#include "ThreadSupport.hpp"
#include "ActivityManager.hpp"
#include <errno.h>

#define THREAD_PRIORITY 100

void keyDestructor(void *)
{
  return;
}



void SysThreadingInit(void)
/* This must be done once per process                                 */
{
}

/*****************************************************************************/
/*  fixed timed scheduling policy doesn't work for AIX 4.1      */
/*  If i use a different Stack size (defaults to 96KB) there       */
/*  are no performance improvements.                               */
/*****************************************************************************/

thread_id_t SysCreateThread(PTHREADFN threadFnc, size_t stackSize, void *args)
{
   int             rc;
   pthread_t       newThread;
   pthread_attr_t  newThreadAttr;
   int schedpolicy;
   struct sched_param schedparam;

                              // Create an attr block for Thread.
   rc = pthread_attr_init(&newThreadAttr);
                              // Set the stack size.
#if defined(OPSYS_AIX43) || defined(LINUX) ||  defined OPSYS_SUN

/* scheduling on two threads controlled by the result method of the message object */
/* do not work proper without an enhanced priority                                 */

   pthread_getschedparam(pthread_self(), &schedpolicy, &schedparam);
   schedparam.sched_priority = 100;

#if defined(OPSYS_SUN) || defined(OPSYS_AIX43)
/* PTHREAD_EXPLICIT_SCHED ==> use scheduling attributes of the new object    */

   rc = pthread_attr_setinheritsched(&newThreadAttr, PTHREAD_EXPLICIT_SCHED);

/* Performance measurements show massive performance improvements > 50 %     */
/* using Round Robin scheduling instead of FIFO scheduling                   */
   rc = pthread_attr_setschedpolicy(&newThreadAttr, SCHED_RR); /* not supported AIX4.1 */
#endif
   rc = pthread_attr_setschedparam(&newThreadAttr, &schedparam);

#endif
   rc = pthread_attr_setstacksize(&newThreadAttr, stackSize);
                                              // Now create the thread
   rc = pthread_create(&newThread, &newThreadAttr, threadFnc, (void *)args);
                              // Bumop thread count by one. Threadid
   if (rc != 0)
   {
       reportException(Error_System_service_service, "ERROR CREATING THREAD");
       return 0;
   }
   rc = pthread_attr_destroy(&newThreadAttr);

   return (thread_id_t)newThread;
}

void SysInitializeThread()
{
}

void SysTerminateThread(thread_id_t threadid)
/* terminate a thread created with SysCreateThread()                             */
{
   pthread_detach(threadid);
}

thread_id_t SysQueryThreadID()
{
    return (thread_id_t)pthread_self();      /* just call the correct function */
}


void SysSetThreadPriority(thread_id_t tid, int  prio)
{
   int schedpolicy;
   struct sched_param schedparam;

   pthread_getschedparam((pthread_t) tid,  &schedpolicy, &schedparam);

/* Medium_priority(=100) is used for every new thread                */

   schedparam.sched_priority = prio;
   pthread_setschedparam((pthread_t) tid, schedpolicy, &schedparam);

}

char *SysGetThreadStackBase(size_t stacksize)
{
  size_t temp;
  return (char *) ((char *)&temp - stacksize);
}

/* ********************************************************************** */
/* ***                  RexxSemaphore                                 *** */
/* ********************************************************************** */
RexxSemaphore::RexxSemaphore()
{
  int iRC = 0;
                                    // Clear mutex/cond prior to init

#if defined( HAVE_PTHREAD_MUTEXATTR_SETTYPE )
  pthread_mutexattr_t mutexattr;

  iRC = pthread_mutexattr_init(&mutexattr);
  if ( iRC == 0 )
# if defined( HAVE_PTHREAD_MUTEX_RECURSIVE_NP ) /* Linux most likely */
     iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
# elif defined( HAVE_PTHREAD_MUTEX_RECURSIVE )
     iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
# elif defined( HAVE_PTHREAD_MUTEX_ERRORCHECK )
     iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
# else
     fprintf(stderr," *** ERROR: Unknown 2nd argument to pthread_mutexattr_settype()!\n");
# endif
  if ( iRC == 0 )
     iRC = pthread_mutex_init(&(this->semMutex), &mutexattr);
  if ( iRC == 0 )
     iRC = pthread_mutexattr_destroy(&mutexattr); /* It does not affect       */
  if ( iRC == 0 )                                 /* mutexes created with it  */
     iRC = pthread_cond_init(&(this->semCond), NULL);
#else
  iRC = pthread_mutex_init(&(this->semMutex), NULL);
  if ( iRC == 0 )
     iRC = pthread_cond_init(&(this->semCond), NULL);
#endif
  if ( iRC != 0 )
  {
    fprintf(stderr," *** ERROR: At RexxSemaphore(), pthread_mutex_init - RC = %d !\n", iRC);
    if ( iRC == EINVAL )
      fprintf(stderr," *** ERROR: Application was not built thread safe!\n");
  }
  this->value = 0;
}

RexxSemaphore::~RexxSemaphore()
{
  pthread_cond_destroy(&(this->semCond));
  pthread_mutex_destroy(&(this->semMutex));
}


void RexxSemaphore::post()
{
  int rc;

  rc = pthread_mutex_lock(&(this->semMutex));      //Lock the semaphores Mutex
  this->value += 1;                            //Increment post count
  rc  = pthread_cond_broadcast(&(this->semCond));   //allows any threads waiting to run
  rc = pthread_mutex_unlock(&(this->semMutex));    // Unlock access to Semaphore mutex
}

void RexxSemaphore::wait()
{
  int rc;
  int schedpolicy, i_prio;
  struct sched_param schedparam;

  pthread_getschedparam(pthread_self(), &schedpolicy, &schedparam);
  i_prio = schedparam.sched_priority;
  schedparam.sched_priority = 100;
  pthread_setschedparam(pthread_self(),SCHED_OTHER, &schedparam);
  rc = pthread_mutex_lock(&(this->semMutex));      // Lock access to semaphore
  if (!this->value)                         // Has it been posted?
    rc = pthread_cond_wait(&(this->semCond), &(this->semMutex)); // Nope, then wait on it.
  pthread_mutex_unlock(&(this->semMutex));    // Release mutex lock
  schedparam.sched_priority = i_prio;
  pthread_setschedparam(pthread_self(),SCHED_OTHER, &schedparam);
}

void RexxSemaphore::wait(long t)           // takes a timeout in msecs
{
  struct timespec timestruct;
  time_t *Tpnt = NULL;

  timestruct.tv_nsec = 0;
  timestruct.tv_sec = t/1000+time(Tpnt);    // convert to secs and abstime
  pthread_mutex_lock(&(this->semMutex));    // Lock access to semaphore
  if (!this->value)                         // Has it been posted?
                                            // wait with timeout
    pthread_cond_timedwait(&(this->semCond),&(this->semMutex),&timestruct);
  pthread_mutex_unlock(&(this->semMutex));    // Release mutex lock
}

void RexxSemaphore::reset()
{
  pthread_mutex_lock(&semMutex);      // Lock access to semaphore
  this->value = 0;                          // Clear value
  pthread_mutex_unlock(&semMutex);    // unlock access to semaphore
}

u_long RexxSemaphore::posted()
{
  return this->value;
}
/* ********************************************************************** */
/* ***                  RexxMutex                                     *** */
/* ********************************************************************** */
RexxMutex::RexxMutex()
{
  int iRC = 0;
                                      // Clear Mutex prior to Init call
//   this->mutexMutex = NULL;
   this->mutex_value = 0;

/* The original settings for pthread_mutexattr_settype() were:
   AIX43: PTHREAD_MUTEX_RECURSIVE
   SUNOS: PTHREAD_MUTEX_ERRORCHECK
   LINUX: PTHREAD_MUTEX_RECURSIVE_NP
*/
#if defined( HAVE_PTHREAD_MUTEXATTR_SETTYPE )
  pthread_mutexattr_t mutexattr;

  iRC = pthread_mutexattr_init(&mutexattr);
  if ( iRC == 0 )
# if defined( HAVE_PTHREAD_MUTEX_RECURSIVE_NP ) /* Linux most likely */
     iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
# elif defined( HAVE_PTHREAD_MUTEX_RECURSIVE )
     iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
# elif defined( HAVE_PTHREAD_MUTEX_ERRORCHECK )
     iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
# else
     fprintf(stderr," *** ERROR: Unknown 2nd argument to pthread_mutexattr_settype()!\n");
# endif
  if ( iRC == 0 )
     iRC = pthread_mutex_init(&(this->mutexMutex), &mutexattr);
  if ( iRC == 0 )
     iRC = pthread_mutexattr_destroy(&mutexattr); /* It does not affect       */
#else                                             /* mutexes created with it  */
   iRC = pthread_mutex_init(&(this->mutexMutex), NULL);
#endif
  if ( iRC != 0 )
  {
    fprintf(stderr," *** ERROR: At RexxMutex(), pthread_mutex_init - RC = %d !\n", iRC);
  }
}

 RexxMutex::~RexxMutex()
{
  this->mutex_value = 0;
  pthread_mutex_destroy(&(this->mutexMutex));
}

