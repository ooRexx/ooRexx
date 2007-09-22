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
#include "PlatformDefinitions.h"
#include "ThreadSupport.hpp"
#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "RexxNativeAPI.h"                    /* Method macros */
#include "RexxDateTime.hpp"

#ifdef AIX
#include <time.h>
#else
#include <sys/time.h>
#endif

extern SEV rexxTimeSliceSemaphore;
extern ULONG  RexxTimeSliceTimer;
extern ULONG  rexxTimeSliceTimerOwner;
extern RexxInteger * ProcessName;

void SysGetCurrentTime(
  RexxDateTime *Date )                 /* returned data structure    */
{
//  time_t Tp;                         /* long int for               */
//  time_t *Tpnt = NULL;
//  time_t *Clock;
  struct tm *SystemDate;               /* system date structure ptr  */
  struct timeval tv;
//  Tp = time(Tpnt);                   /* get time long              */
//  Clock = &Tp;
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
BOOL SysTimeSliceElapsed( void )
{
//  ULONG postCount;
//                                       /* see how many times timer poped */
//  DosQueryEventSem(rexxTimeSliceSemaphore, & postCount);
//                                       /* return number of times it poped*/
  return (0);
}

void SysStartTimeSlice( void )
/******************************************************************************/
/* Function:  Make sure we ahve a Timer running and reset TimeSlice Sem       */
/******************************************************************************/
{
//  if (!rexxTimeSliceTimerOwner) {      /* Is there a current Owner?     */
//                                       /* nope, then none running.      */
//    DosStartTimer(100, (HSEM)rexxTimeSliceSemaphore, &RexxTimeSliceTimer);
//                                       /* owner is this process.        */
//    rexxTimeSliceTimerOwner = ProcessName->value;
//  }
//                                       /* reset the timeslice semaphore */
//                                       /* so we don't immediately give  */
//                                       /* it up is already posted.      */
//  EVSET(rexxTimeSliceSemaphore);
}
typedef struct {
  HEV sem;                             /* semaphore to wait on              */
  long time;                           /* timeout value                     */
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
void* async_timer(PVOID info)
{
                                       /* do wait with apprpriate timeout   */
  (((ASYNC_TIMER_INFO*)info)->sem)->wait(((ASYNC_TIMER_INFO*)info)->time);
  if (!(((ASYNC_TIMER_INFO*)info)->sem)->posted()) /* if sem not posted ..  */
    (((ASYNC_TIMER_INFO*)info)->sem)->post();      /* do it                 */
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
                     long, numdays,
                     long, alarmtime)
{
  APIRET rc;                           /* return code                       */
  RexxSemaphore sem;                   /* Event-semaphore                   */
  HEV semHandle;                       /* semaphore handle                  */
  long msecInADay = 86400000;          /* number of milliseconds in a day   */
  REXXOBJECT cancelObj;                /* place object to check for cancel  */
  long cancelVal;                      /* value of cancel                   */
  ASYNC_TIMER_INFO tinfo;              /* info for the timer thread         */

  semHandle = &sem;
                                       /* set the state variables           */
  RexxVarSet("EVENTSEMHANDLE",RexxInteger((long)semHandle));
  RexxVarSet("TIMERSTARTED",RexxTrue);
  /* setup the info for the timer thread                                    */
  tinfo.sem = semHandle;
  tinfo.time = msecInADay;

  while (numdays > 0) {                /* is it some future day?            */

                                       /* start timer to wake up after a day*/
    rc = SysCreateThread(async_timer, C_STACK_SIZE, (PVOID)&tinfo);
    if (!rc) {                         /* Error received?                   */
                                       /* raise error                       */
      send_exception(Error_System_service);
      return;
    }

    semHandle->wait();                 /* wait for semaphore to be posted   */
//#ifdef AIX
//    pthread_yield();                 /* give the timer thread a chance    */
//#else
//    sched_yield();                   /* give the timer thread a chance    */
    SysThreadYield();                  /* give the timer thread a chance    */
//#endif
    cancelObj = RexxVarValue("CANCELED");
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
  rc = SysCreateThread(async_timer, C_STACK_SIZE, (PVOID)&tinfo);
  if (!rc) {                           /* Error received?                   */
                                       /* raise error                       */
     send_exception(Error_System_service);
     return;
  }
  semHandle->wait();                   /* wait for semaphore to be posted   */
//#ifdef AIX
//    pthread_yield();                 /* give the timer thread a chance    */
//#else
//    sched_yield();                   /* give the timer thread a chance    */
    SysThreadYield();                  /* give the timer thread a chance    */
//#endif                               /* get the cancel state              */
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
               long, eventSemHandle)
{
  HEV    hev = (HEV)eventSemHandle;    /* event semaphore handle            */
  hev->post();                         /* Post the event semaphore          */
  return;
}












