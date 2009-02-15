/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <pthread.h>
#if defined(OPSYS_SUN)
#include <sched.h>
#endif
#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "RexxDateTime.hpp"
#include "SystemInterpreter.hpp"

#ifdef AIX
#include <time.h>
#else
#include <sys/time.h>
#endif


void SystemInterpreter::getCurrentTime(RexxDateTime *Date )
{
    struct tm *SystemDate;               /* system date structure ptr  */
    struct timeval tv;
    gettimeofday(&tv, NULL);

#ifdef AIX
    struct tm SD;                        /* system date area           */
    SystemDate = localtime_r((time_t *)&tv.tv_sec, &SD);
#else
    SystemDate = localtime((time_t *)&tv.tv_sec); /* convert           */
#endif

    Date->hours = SystemDate->tm_hour;
    Date->minutes = SystemDate->tm_min;
    Date->seconds = SystemDate->tm_sec;
    Date->microseconds = tv.tv_usec;
    Date->day = SystemDate->tm_mday;
    Date->month = ++SystemDate->tm_mon;
    Date->year = SystemDate->tm_year + 1900;

    struct tm *GMTDate;                  /* system date structure ptr  */
#ifdef AIX
    struct tm GD;                        /* system date area           */
    GMTDate = gmtime_r((time_t *)&tv.tv_sec, &GD);
#else
    GMTDate = gmtime((time_t *)&tv.tv_sec);
#endif
    // in microseconds
    Date->timeZoneOffset = ((int64_t)(tv.tv_sec - mktime(GMTDate))) * 1000000UL;
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   alarm_starTimer                              */
/*                                                                   */
/*   Function:          starts an asynchronous, single_interval      */
/*                      timer thread. When the timer pops, it will   */
/*                      post an event semaphore.                     */
/*                                                                   */
/*   Arguments:         alarm timer - time interval before the event */
/*                       semaphore is posted                         */
/*********************************************************************/

RexxMethod2(int, alarm_startTimer,
                     wholenumber_t, numdays,
                     wholenumber_t, alarmtime)
{
    SysSemaphore sem;                    /* Event-semaphore                   */
    int  msecInADay = 86400000;          /* number of milliseconds in a day   */

    /* set the state variables           */
    context->SetObjectVariable("EVENTSEMHANDLE", context->NewPointer(&sem));
    context->SetObjectVariable("TIMERSTARTED", context->True());

    while (numdays > 0)
    {                /* is it some future day?            */
        // use the semaphore to wait for an entire day.
        // if this returns true, then this was not a timeout, which
        // probably means this was cancelled.
        if (sem.wait(msecInADay))
        {
            /* Check if the alarm is canceled. */
            RexxObjectPtr cancelObj = context->GetObjectVariable("CANCELED");

            if (cancelObj == context->True())
            {
                return 0;
            }
            else
            {
                sem.reset();              /* Reset the event semaphore         */
            }
        }
        numdays--;                         /* Decrement number of days          */
    }

    // now we can just wait for the alarm time to expire
    sem.wait(alarmtime);
    return 0;
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   alarm_stopTimer                              */
/*                                                                   */
/*   Function:          stops an asynchronous timer.                 */
/*                                                                   */
/*   Arguments:         timer - timer handle                         */
/*                      eventSemHandle - event semaphore handle      */
/*                        shared between start & stop timer          */
/*********************************************************************/


RexxMethod1(int, alarm_stopTimer, POINTER, eventSemHandle)
{
    SysSemaphore *sev = (SysSemaphore *)eventSemHandle;    /* event semaphore handle            */
    sev->post();                         /* Post the event semaphore          */
    return 0;
}












