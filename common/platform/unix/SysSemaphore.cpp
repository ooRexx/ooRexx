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
/* Threading support for Unix                                                */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <pthread.h>
#include <memory.h>
#include <stdio.h>
#ifdef AIX
    #include <sys/sched.h>
    #include <time.h>
#endif

#if defined(OPSYS_SUN)
    #include <sched.h>
#endif

#include <errno.h>

#include "SysSemaphore.hpp"


/* ********************************************************************** */
/* ***                  SysSemaphore                                  *** */
/* ********************************************************************** */

/**
 * Create a semaphore with potential creation-time
 * initialization.
 *
 * @param create Indicates whether the semaphore should be created now.
 */
SysSemaphore::SysSemaphore(bool createSem)
{
    if (createSem)
    {
        create();
    }
}


void SysSemaphore::create()
{
    int iRC = 0;

    if (!created)
    {
                                        // Clear mutex/cond prior to init
    //  this->semMutex = NULL;
    //  this->semCond = NULL;

    /* The original settings for pthread_mutexattr_settype() were:
       AIX  : PTHREAD_MUTEX_RECURSIVE
       SUNOS: PTHREAD_MUTEX_ERRORCHECK
       LINUX: PTHREAD_MUTEX_RECURSIVE_NP
    */

    #if defined( HAVE_PTHREAD_MUTEXATTR_SETTYPE )
        pthread_mutexattr_t mutexattr;

        iRC = pthread_mutexattr_init(&mutexattr);
        if ( iRC == 0 )
        {
    #if defined( HAVE_PTHREAD_MUTEX_RECURSIVE_NP ) /* Linux most likely */
            iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    #elif defined( HAVE_PTHREAD_MUTEX_RECURSIVE )
            iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    #elif defined( HAVE_PTHREAD_MUTEX_ERRORCHECK )
            iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
    #else
            fprintf(stderr," *** ERROR: Unknown 2nd argument to pthread_mutexattr_settype()!\n");
    #endif
        }
        if ( iRC == 0 )
        {
            iRC = pthread_mutex_init(&(this->semMutex), &mutexattr);
        }
        if ( iRC == 0 )
        {
            iRC = pthread_mutexattr_destroy(&mutexattr); /* It does not affect       */
        }
        if ( iRC == 0 )                                 /* mutexes created with it  */
        {
            iRC = pthread_cond_init(&(this->semCond), NULL);
        }
    #else
        iRC = pthread_mutex_init(&(this->semMutex), NULL);
        if ( iRC == 0 )
        {
            iRC = pthread_cond_init(&(this->semCond), NULL);
        }
    #endif
        if ( iRC != 0 )
        {
            fprintf(stderr," *** ERROR: At RexxSemaphore(), pthread_mutex_init - RC = %d !\n", iRC);
            if ( iRC == EINVAL )
            {
                fprintf(stderr," *** ERROR: Application was not built thread safe!\n");
            }
        }
        this->postedCount = 0;
        created = true;
    }
}

void SysSemaphore::close()
{
    if (created)
    {
        pthread_cond_destroy(&(this->semCond));
        pthread_mutex_destroy(&(this->semMutex));
        created = false;
    }
}


void SysSemaphore::post()
{
    int rc;

    rc = pthread_mutex_lock(&(this->semMutex));      //Lock the semaphores Mutex
    postedCount++;                                   //Increment post count
    rc  = pthread_cond_broadcast(&(this->semCond));  //allows any threads waiting to run
    rc = pthread_mutex_unlock(&(this->semMutex));    // Unlock access to Semaphore mutex
}

void SysSemaphore::wait()
{
    int rc;
    int schedpolicy, i_prio;
    struct sched_param schedparam;

    pthread_getschedparam(pthread_self(), &schedpolicy, &schedparam);
    i_prio = schedparam.sched_priority;
    schedparam.sched_priority = 100;
    pthread_setschedparam(pthread_self(),SCHED_OTHER, &schedparam);
    rc = pthread_mutex_lock(&(this->semMutex));      // Lock access to semaphore
    if (this->postedCount == 0)                      // Has it been posted?
    {
        rc = pthread_cond_wait(&(this->semCond), &(this->semMutex)); // Nope, then wait on it.
    }
    pthread_mutex_unlock(&(this->semMutex));    // Release mutex lock
    schedparam.sched_priority = i_prio;
    pthread_setschedparam(pthread_self(),SCHED_OTHER, &schedparam);
}

bool SysSemaphore::wait(uint32_t t)           // takes a timeout in msecs
{
    struct timespec timestruct;
    time_t *Tpnt = NULL;

    int result = 0;
    timestruct.tv_nsec = 0;
    timestruct.tv_sec = t/1000+time(Tpnt);    // convert to secs and abstime
    pthread_mutex_lock(&(this->semMutex));    // Lock access to semaphore
    if (!this->postedCount)                   // Has it been posted?
    {
                                              // wait with timeout
        result = pthread_cond_timedwait(&(this->semCond),&(this->semMutex),&timestruct);
    }
    pthread_mutex_unlock(&(this->semMutex));    // Release mutex lock
    // a false return means this timed out
    return result != ETIMEDOUT;
}

void SysSemaphore::reset()
{
    pthread_mutex_lock(&(this->semMutex));      // Lock access to semaphore
    this->postedCount = 0;                      // Clear value
    pthread_mutex_unlock(&(this->semMutex));    // unlock access to semaphore
}

/* ********************************************************************** */
/* ***                  SysMutex                                      *** */
/* ********************************************************************** */

/**
 * Create a semaphore with potential creation-time
 * initialization.
 *
 * @param create Indicates whether the semaphore should be created now.
 */
SysMutex::SysMutex(bool createSem)
{
    if (createSem)
    {
        create();
    }
}

void SysMutex::create()
{
    // don't create this multiple times
    if (created)
    {
        return;
    }
    int iRC = 0;

/* The original settings for pthread_mutexattr_settype() were:
   SUNOS: PTHREAD_MUTEX_ERRORCHECK
   LINUX: PTHREAD_MUTEX_RECURSIVE_NP
*/
#if defined( HAVE_PTHREAD_MUTEXATTR_SETTYPE )
    pthread_mutexattr_t mutexattr;

    iRC = pthread_mutexattr_init(&mutexattr);
    if ( iRC == 0 )
    {
    #if defined( HAVE_PTHREAD_MUTEX_RECURSIVE_NP ) /* Linux most likely */
            iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    #elif defined( HAVE_PTHREAD_MUTEX_RECURSIVE )
            iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    #elif defined( HAVE_PTHREAD_MUTEX_ERRORCHECK )
            iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
    #else
            fprintf(stderr," *** ERROR: Unknown 2nd argument to pthread_mutexattr_settype()!\n");
    #endif
    }
    if ( iRC == 0 )
    {
        iRC = pthread_mutex_init(&(this->mutexMutex), &mutexattr);
    }
    if ( iRC == 0 )
    {
        iRC = pthread_mutexattr_destroy(&mutexattr); /* It does not affect       */
    }
#else                                             /* mutexes created with it  */
    iRC = pthread_mutex_init(&(this->mutexMutex), NULL);
#endif
    if ( iRC != 0 )
    {
        fprintf(stderr," *** ERROR: At RexxMutex(), pthread_mutex_init - RC = %d !\n", iRC);
    }

    created = true;
}


void SysMutex::close()
{
    if (created)
    {
        pthread_mutex_destroy(&(this->mutexMutex));
        created = false;
    }
}
