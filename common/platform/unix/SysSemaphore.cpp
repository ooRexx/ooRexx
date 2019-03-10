/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* Semaphore support for Unix                                                */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <memory.h>
#include <stdio.h>
#include <sys/time.h>
#include <algorithm>
#ifdef AIX
#include <sys/sched.h>
#include <time.h>
#endif

#if defined(OPSYS_SUN) || defined(__HAIKU__)
#include <sched.h>
#endif

#include <errno.h>

#include "SysSemaphore.hpp"

#ifndef HAVE_PTHREAD_MUTEX_TIMEDLOCK

// we test every 10 milliseconds
#define TIMESLICE 10000000L

/**
 * Check to see if we have gone beyond the appointed time.
 *
 * @param ts     The target time
 * @param nextInterval
 *               The return next wait interval if this still has not timed out.
 *
 * @return true if we have gone beyond the target time, false otherwise.
 */
bool checkTimeOut(const struct timespec *ts, long &nextInterval)
{

    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);       // get the current time

    // if target time seconds are greater, we have not timed out
    if (ts->tv_sec > currentTime.tv_sec)
    {
        return false;
    }
    // if the seconds are the same, we calculate the delta
    // and check
    if (ts->tv_sec == currentTime.tv_sec)
    {
        // calculate unconditionally, we use the sign to then determine if this was a time out
        nextInterval = ts->tv_sec - currentTime.tv_sec;
        if (nextInterval < 0)
        {
            return true;      // a negative result is a timeout
        }
        // we never wait longer than 10 milliseconds
        nextInterval = std::min(nextInterval, TIMESLICE);
        return false;
    }

    // the target seconds are less, so we are done
    return true;
}


/**
 * Fix for systems that don't have pthread_mutex_timedlock
 *
 * @param mutex  The mutex we're waiting on
 * @param abs_timeout
 *               The time timeout value
 *
 * @return zero or an error code.
 */
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abs_timeout)
{
    int pthread_rc;

    long waitInterval;

    // we might already be timed out
    if (checkTimeOut(abs_timeout, waitInterval))
    {
        return ETIMEDOUT;
    }

    while ((pthread_rc = pthread_mutex_trylock(mutex)) == EBUSY)
    {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = waitInterval;

        struct timespec slept;
        nanosleep(&ts, &slept);

        // check for a timeout immediately, this also gives us our
        // next wait interval
        if (checkTimeOut(abs_timeout, waitInterval))
        {
            return ETIMEDOUT;
        }
    }

    return pthread_rc;
}
#endif


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
    postedCount = 0;
    created = false;

    if (createSem)
    {
        create();
    }
}

/**
 * Create the semaphore
 */
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
        if (iRC == 0)
        {
#if defined( HAVE_PTHREAD_MUTEX_RECURSIVE ) /* Linux most likely */
            iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
#elif defined( HAVE_PTHREAD_MUTEX_ERRORCHECK )
            iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
            // no valid mutex errors found, likely a config.h problem
#else
#error Configuration error for pthread semaphores.  Check the defines in config.h
#endif
        }
        if (iRC == 0)
        {
            iRC = pthread_mutex_init(&(this->semMutex), &mutexattr);
        }
        if (iRC == 0)
        {
            iRC = pthread_mutexattr_destroy(&mutexattr); /* It does not affect       */
        }
        if (iRC == 0)                                 /* mutexes created with it  */
        {
            iRC = pthread_cond_init(&(this->semCond), NULL);
        }
#else
        iRC = pthread_mutex_init(&(this->semMutex), NULL);
        if (iRC == 0)
        {
            iRC = pthread_cond_init(&(this->semCond), NULL);
        }
#endif
        if (iRC != 0)
        {
            fprintf(stderr, " *** ERROR: At RexxSemaphore(), pthread_mutex_init - RC = %d !\n", iRC);
            if (iRC == EINVAL)
            {
                fprintf(stderr, " *** ERROR: Application was not built thread safe!\n");
            }
        }
        this->postedCount = 0;
        created = true;
    }
}

/**
 * Close an event semaphore
 */
void SysSemaphore::close()
{
    if (created)
    {
        pthread_cond_destroy(&(this->semCond));
        pthread_mutex_destroy(&(this->semMutex));
        created = false;
    }
}


/**
 * Post that an event has occurred.
 */
void SysSemaphore::post()
{
    int rc;

    rc = pthread_mutex_lock(&(this->semMutex));      //Lock the semaphores Mutex
    postedCount++;                                   //Increment post count
    rc  = pthread_cond_broadcast(&(this->semCond));  //allows any threads waiting to run
    rc = pthread_mutex_unlock(&(this->semMutex));    // Unlock access to Semaphore mutex
}


/**
 * Wait for an event semaphore to be posted.
 */
void SysSemaphore::wait()
{
    int rc;
    int schedpolicy, i_prio;
    struct sched_param schedparam;

    pthread_getschedparam(pthread_self(), &schedpolicy, &schedparam);
    i_prio = schedparam.sched_priority;
    schedparam.sched_priority = 100;
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &schedparam);
    rc = pthread_mutex_lock(&(this->semMutex));      // Lock access to semaphore
    if (this->postedCount == 0)                      // Has it been posted?
    {
        rc = pthread_cond_wait(&(this->semCond), &(this->semMutex)); // Nope, then wait on it.
    }
    pthread_mutex_unlock(&(this->semMutex));    // Release mutex lock
    schedparam.sched_priority = i_prio;
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &schedparam);
}


/**
 * Fill in a timespec given a timeout value.
 *
 * @param t      The time to wait, in milliseconds.
 * @param ts
 */
void SysSemaphore::createTimeOut(uint32_t t, timespec &ts)
{
    int result = 0;
    clock_gettime(CLOCK_REALTIME, &ts);       // get the current time
    time_t deltaSeconds = (time_t)(t / 1000); // get the seconds part of this
                                              // we need to convert to nanoseconds
    long deltaNanoSeconds = ((long)(t % 1000)) * 1000000l;

    ts.tv_nsec += deltaNanoSeconds;           // add fractions of seconds
    if (ts.tv_nsec > 1000000000l)             // did nanoseconds overflow?
    {
        ts.tv_nsec -= 1000000000l;            // correct microsecond overflow ..
        ts.tv_sec += 1;                       // .. by adding a second
    }

    ts.tv_sec += deltaSeconds;                // add requested seconds
}


/**
 * Wait for the event, with a timeout.
 *
 * @param t      The timeout value, in milliseconds.
 *
 * @return       true if the event occurrec, false if there was
 *               a timeout.
 */
bool SysSemaphore::wait(uint32_t t)           // takes a timeout in msecs
{
    struct timespec ts;                       // fill in the timeout spec
    int result = 0;
    createTimeOut(t, ts);

    pthread_mutex_lock(&(this->semMutex));    // Lock access to semaphore
    while (result == 0 && !this->postedCount) // Has it been posted? Spurious wakeups may occur
    {                                         // wait with timeout
        result = pthread_cond_timedwait(&(this->semCond), &(this->semMutex), &ts);
    }
    pthread_mutex_unlock(&(this->semMutex));    // Release mutex lock
    // a false return means this timed out
    return result != ETIMEDOUT;
}

/**
 * Reset the semaphore
 */
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
 * @param createSem
 * @param critical  Indicates this is a critical-time semaphore only held for a short period of time (ignored for unix-based)
 */
SysMutex::SysMutex(bool createSem, bool critical)
{
    if (createSem)
    {
        create();
    }
}


/**
 * Create a mutex for operation
 *
 * @param critical Indicates whether this is a "critical time" semaphore that will only be
 *                 held for short windows. This is a noop for
 *                 unix-based platforms.
 */
void SysMutex::create(bool critical)
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
    if (iRC == 0)
    {
#if defined( HAVE_PTHREAD_MUTEX_RECURSIVE ) /* Linux most likely */
        iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
#elif defined( HAVE_PTHREAD_MUTEX_ERRORCHECK )
        iRC = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
#else
        fprintf(stderr, " *** ERROR: Unknown 2nd argument to pthread_mutexattr_settype()!\n");
#endif
    }
    if (iRC == 0)
    {
        iRC = pthread_mutex_init(&(this->mutexMutex), &mutexattr);
    }
    if (iRC == 0)
    {
        iRC = pthread_mutexattr_destroy(&mutexattr); /* It does not affect       */
    }
#else                                             /* mutexes created with it  */
    iRC = pthread_mutex_init(&(this->mutexMutex), NULL);
#endif
    if (iRC != 0)
    {
        fprintf(stderr, " *** ERROR: At RexxMutex(), pthread_mutex_init - RC = %d !\n", iRC);
    }

    created = true;
}


/**
 * Lock the mutex with a wait time.
 *
 * @param t      The time to wait
 *
 * @return true of the lock was obtained, false otherwise.
 */
bool SysMutex::request(uint32_t t)
{
    if (!created)
    {
        return false;
    }

    struct timespec ts;                       // fill in the timeout spec
    SysSemaphore::createTimeOut(t, ts);
    return pthread_mutex_timedlock(&mutexMutex, &ts) == 0;
}


/**
 * Close the mutex semaphore.
 */
void SysMutex::close()
{
    if (created)
    {
        pthread_mutex_destroy(&(this->mutexMutex));
        created = false;
    }
}
