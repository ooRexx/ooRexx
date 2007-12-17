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

#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "RexxNativeAPI.h"
#include "RexxDateTime.hpp"
#include "Interpreter.hpp"

extern SEV rexxTimeSliceSemaphore;
extern HANDLE rexxTimeSliceTimerOwner;

#define TIMESLICE_STACKSIZE 2048
#define TIMESLICEMS 10

#ifdef TIMESLICE
extern int REXXENTRY RexxSetYield(process_id_t procid, thread_id_t threadid);
extern bool rexxTimeSliceElapsed;
#endif

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysGetCurrentTime                            */
/*                                                                   */
/*   Function:          gets the time and date from the system clock */
/*********************************************************************/
void SysGetCurrentTime(
  RexxDateTime *Date )                 /* returned data structure    */
/*********************************************************************/
/* Function:  Return a time stamp to the kernel date/time functions. */
/*********************************************************************/
{
  SYSTEMTIME SystemDate; /* system date structure      */

  GetLocalTime(&SystemDate);        /* via Windows                */

  Date->hours = SystemDate.wHour;
  Date->minutes = SystemDate.wMinute;
  Date->seconds = SystemDate.wSecond;
  Date->microseconds = SystemDate.wMilliseconds * 1000;
  Date->day = SystemDate.wDay;
  Date->month = SystemDate.wMonth;
  Date->year = SystemDate.wYear;
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   TimeSliceControl                             */
/*                                                                   */
/*   The thread function for the time slice timer                    */
/*                                                                   */
/*********************************************************************/

DWORD WINAPI TimeSliceControl(void * args)
{
#ifdef TIMESLICE
   do
   {
      Sleep(TIMESLICEMS);
      rexxTimeSliceElapsed = true;
   } while (!Interpreter::isTerminated());
   rexxTimeSliceTimerOwner = 0;
#endif
   return 0;
}


void SysStartTimeSlice( void )
/******************************************************************************/
/* Function:  Make sure we have a Timer running and reset TimeSlice Sem       */
/******************************************************************************/
{
#ifdef TIMESLICE
   ULONG thread;
   if (rexxTimeSliceTimerOwner == 0) {           /* Is there a timer?         */

     /* create a time slice timer thread */

     rexxTimeSliceTimerOwner = CreateThread(NULL, TIMESLICE_STACKSIZE, TimeSliceControl, NULL, 0, &thread);
     SetThreadPriority(rexxTimeSliceTimerOwner,THREAD_PRIORITY_NORMAL+1);  /* set a higher priority */
  }
  rexxTimeSliceElapsed = false;
#endif
}


void SysStopTimeSlice( void )
/******************************************************************************/
/* Function:  Stop the time slice timer thread                                */
/******************************************************************************/
{
#ifdef TIMESLICE
   TerminateThread(rexxTimeSliceTimerOwner, 0);
   rexxTimeSliceTimerOwner = 0;
#endif
}


/**
 * Wait for a window timer message or for an event to be signaled.
 *
 * @param hev  Handle to an event object.  If this event is signaled, the
 *             function returns.
 */
static void waitTimerOrEvent(HANDLE hev)
{
    MSG msg = {0};

    /** Wait for a window timer message or for the event to be signaled. Note
     *  that MsgWaitForMultipleObjects() will return if a *new* message is
     *  placed in the message queue.  Once PeekMesage() is called, all messages
     *  in the queue need to be processed before MsgWaitForMultipleObjects() is
     *  called.
     */
    do
    {
        while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
            // Check for the timer message.
            if ( msg.message == WM_TIMER )
            {
                return;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // Check if signaled.
            if ( WaitForSingleObject(hev, 0) == WAIT_OBJECT_0 )
            {
                return;
            }
        }
    } while ( MsgWaitForMultipleObjects(1, &hev, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0 + 1 );
}


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   alarm_starTimer                              */
/*                                                                   */
/*   Function:          starts a timer and waits for it to expire.   */
/*                      An event semaphore is created that can be    */
/*                      used to cancel the timer.                    */
/*                                                                   */
/*   Arguments:         numdays - number of whole days until timer   */
/*                      should expire.                               */
/*                      alarmtime - fractional portion (less than a  */
/*                      day) until timer should expire, expressed in */
/*                      milliseconds.                                */
/*********************************************************************/
RexxMethod2(void, alarm_startTimer,
            wholenumber_t, numdays,
            wholenumber_t, alarmtime)
{
    bool fState = false;                 /* Initial state of semaphore        */
    unsigned int msecInADay = 86400000;  /* number of milliseconds in a day   */
    REXXOBJECT cancelObj;                /* object to check for cancel        */
    int cancelVal;                       /* value of cancel                   */
    HANDLE SemHandle = 0;                /* Event-semaphore handle            */
    unsigned int TimerHandle = 0;        /* Timer handle                      */

    /* Create an event semaphore that can be used to cancel the alarm. */
    SemHandle = CreateEvent(NULL, TRUE, fState, NULL);
    if ( !SemHandle )
    {
        /* Couldn't create the semaphore, raise an exception. */
        send_exception(Error_System_service);
        return;
    }

    /* Set the state variables. */
    ooRexxVarSet("EVENTSEMHANDLE", ooRexxInteger((long)SemHandle));
    ooRexxVarSet("TIMERSTARTED", ooRexxTrue);

    if ( numdays > 0 )
    {
        /** Alarm is for some day in the future, start a timer that wakes up
         *  once a day.
         */
        TimerHandle = SetTimer(NULL, 0, msecInADay, NULL);
        if ( TimerHandle == 0 )
        {
            /* Couldn't create a timer, raise an exception. */
            CloseHandle(SemHandle);
            send_exception(Error_System_service);
            return;
        }

        while ( numdays > 0 )
        {
            /* Wait for the WM_TIMER message or for the alarm to be canceled. */
            waitTimerOrEvent(SemHandle);

            /* Check if the alarm is canceled. */
            cancelObj = ooRexxVarValue("CANCELED");
            cancelVal = REXX_INTEGER_VALUE(cancelObj);
            if ( cancelVal == 1 )
            {
                /* Alarm is canceled, delete timer, close semaphore, return. */
                KillTimer(NULL, TimerHandle);
                CloseHandle(SemHandle);
                return;
            }
            numdays--;
        }
        /* Done with the daily timer, delete it. */
        KillTimer(NULL, TimerHandle);
    }

    if ( alarmtime > 0 )
    {
        /* Start a timer for the fractional portion of the alarm. */
        TimerHandle = SetTimer(NULL, 0, alarmtime, NULL);
        if ( !TimerHandle )
        {
            /* Couldn't create a timer, raise an exception. */
            CloseHandle(SemHandle);
            send_exception(Error_System_service);
            return;
        }

        /* Wait for the WM_TIMER message or for the alarm to be canceled. */
        waitTimerOrEvent(SemHandle);

        /** Total alarm time has expired, or the alarm was canceled.  We don't
         *  care which, just clean up and return.
         */
        KillTimer(NULL, TimerHandle);
    }

    CloseHandle(SemHandle);
    return;
}


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   alarm_stopTimer                              */
/*                                                                   */
/*   Function:          stops an asynchronous timer.                 */
/*                                                                   */
/*   Arguments:         eventSemHandle - handle to event semaphore   */
/*                      used to signal the timer should be canceled. */
/*********************************************************************/
RexxMethod1(void, alarm_stopTimer,
            size_t, eventSemHandle)
{
    /* Post the event semaphore to signal the alarm should be canceled. */
    if ( ! EVPOST((HANDLE)eventSemHandle) )
    {
        /* Raise an error if the semaphore could not be posted. */
        send_exception(Error_System_service);
    }
    return;
}

