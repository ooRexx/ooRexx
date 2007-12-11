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
/*********************************************************************/
/* REXX AIX Support                                     aixtime.c    */
/*                                                                   */
/*   Subroutine Name:   SysGetCurrentTime                            */
/*                                                                   */
/*   Function:          gets the time and date from the system clock */
/*                                                                   */
/*********************************************************************/

#include <pthread.h>
#if defined(OPSYS_SUN)
#include <sched.h>
#endif
#include "RexxCore.h"
#include "ThreadSupport.hpp"
#include "IntegerClass.hpp"
#include "RexxNativeAPI.h"                    /* Method macros */
#include "RexxDateTime.hpp"

#ifdef AIX
#include <time.h>
#else
#include <sys/time.h>
#endif

extern SEV rexxTimeSliceSemaphore;
extern size_t  rexxTimeSliceTimerOwner;

void SysGetCurrentTime(
  RexxDateTime *Date )                 /* returned data structure    */
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
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysTimeSliceElapsed                          */
/*     Used to check to see if a TimeSLice elapsed.  This is done    */
/*     by checking the Semophore that is to be Posted whenever       */
/*     the time interval expires.                                    */
/*                                                                   */
/*********************************************************************/
bool SysTimeSliceElapsed()
{
  return false;
}

void SysStartTimeSlice()
/******************************************************************************/
/* Function:  Make sure we ahve a Timer running and reset TimeSlice Sem       */
/******************************************************************************/
{
}

typedef struct {
  SEV sem;                             /* semaphore to wait on              */
  size_t time;                         /* timeout value                     */
} ASYNC_TIMER_INFO;

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   async_timer                                  */
/*                                                                   */
/*   Function:          acts as a timer thread. Waits on a timer     */
/*                      semaphore with the given timeout.            */
/*                                                                   */
/*   Arguments:         info - struct which holds the semaphore      */
/*                        handle and the timeout value in msecs.     */
/*********************************************************************/
void* async_timer(void *info)
{
                                       /* do wait with apprpriate timeout   */
  (((ASYNC_TIMER_INFO*)info)->sem)->wait(((ASYNC_TIMER_INFO*)info)->time);
  if (!(((ASYNC_TIMER_INFO*)info)->sem)->posted()) /* if sem not posted ..  */
    (((ASYNC_TIMER_INFO*)info)->sem)->post();      /* do it                 */
  return NULL;
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

RexxMethod2(void, alarm_startTimer,
                     wholenumber_t, numdays,
                     wholenumber_t, alarmtime)
{
  APIRET rc;                           /* return code                       */
  RexxSemaphore sem;                   /* Event-semaphore                   */
  SEV semHandle;                       /* semaphore handle                  */
  int  msecInADay = 86400000;          /* number of milliseconds in a day   */
  REXXOBJECT cancelObj;                /* place object to check for cancel  */
  int  cancelVal;                      /* value of cancel                   */
  ASYNC_TIMER_INFO tinfo;              /* info for the timer thread         */

  semHandle = &sem;
                                       /* set the state variables           */
  ooRexxVarSet("EVENTSEMHANDLE", ooRexxInteger((uintptr_t)semHandle));
  ooRexxVarSet("TIMERSTARTED", ooRexxTrue);
  /* setup the info for the timer thread                                    */
  tinfo.sem = semHandle;
  tinfo.time = msecInADay;

  while (numdays > 0) {                /* is it some future day?            */

                                       /* start timer to wake up after a day*/
    rc = SysCreateThread(async_timer, C_STACK_SIZE, (void *)&tinfo);
    if (!rc) {                         /* Error received?                   */
                                       /* raise error                       */
      send_exception(Error_System_service);
      return;
    }

    semHandle->wait();                 /* wait for semaphore to be posted   */
    SysThreadYield();                  /* give the timer thread a chance    */
    cancelObj = ooRexxVarValue("CANCELED");
    cancelVal = REXX_INTEGER_VALUE(cancelObj);

    if (cancelVal == 1) {              /* If alarm cancelled?               */
      return;                          /* get out                           */
    }
    else {
      semHandle->reset();              /* Reset the event semaphore         */
    }
    numdays--;                         /* Decrement number of days          */
  }
  tinfo.sem = semHandle;               /* setup the info for timer thread   */
  tinfo.time = alarmtime;
                                       /* start the timer                   */
  rc = SysCreateThread(async_timer, C_STACK_SIZE, (void *)&tinfo);
  if (!rc) {                           /* Error received?                   */
                                       /* raise error                       */
     send_exception(Error_System_service);
     return;
  }
  semHandle->wait();                   /* wait for semaphore to be posted   */
    SysThreadYield();                  /* give the timer thread a chance    */
  return;
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


RexxMethod1(void, alarm_stopTimer,
               size_t, eventSemHandle)
{
  SEV    sev = (SEV)eventSemHandle;    /* event semaphore handle            */
  sev->post();                         /* Post the event semaphore          */
  return;
}












