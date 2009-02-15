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

#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "RexxDateTime.hpp"
#include "Interpreter.hpp"
#include "SystemInterpreter.hpp"
#include <time.h>

HANDLE SystemInterpreter::timeSliceTimerThread = 0;

#define TIMESLICE_STACKSIZE 2048
#define TIMESLICEMS 10


#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL

struct timezone
{
    int  tz_minuteswest; // minutes West of Greenwich
    int  tz_dsttime;     // the daylight savings time correction
};


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysGetCurrentTime                            */
/*                                                                   */
/*   Function:          gets the time and date from the system clock */
/*********************************************************************/
void SystemInterpreter::getCurrentTime(RexxDateTime *Date )
/*********************************************************************/
/* Function:  Return a time stamp to the kernel date/time functions. */
/*********************************************************************/
{
    FILETIME systemFileTime;
    FILETIME localFileTime;
    uint64_t sysTimeStamp = 0;
    uint64_t localTimeStamp = 0;
    // this retrieves the time as UTC, in a form we can do arithmetic with
    GetSystemTimeAsFileTime(&systemFileTime);
    // this converts the time to the local time zone
    FileTimeToLocalFileTime(&systemFileTime, &localFileTime);

    // now get these as long values so we can do math with them
    sysTimeStamp |= systemFileTime.dwHighDateTime;
    sysTimeStamp <<= 32;
    sysTimeStamp |= systemFileTime.dwLowDateTime;
    // the resolution of this is in tenths of micro seconds.  Convert to microseconds
    // which is a more realistic value
    sysTimeStamp = sysTimeStamp / 10UL;

    localTimeStamp |= localFileTime.dwHighDateTime;
    localTimeStamp <<= 32;
    localTimeStamp |= localFileTime.dwLowDateTime;
    localTimeStamp = localTimeStamp / 10UL;

    // ok, we can use this to calculate the timestamp directly
    Date->timeZoneOffset = localTimeStamp - sysTimeStamp;

    SYSTEMTIME localTime;

    // get a localized version of the time
    FileTimeToSystemTime(&localFileTime, &localTime);

    Date->hours = localTime.wHour;
    Date->minutes = localTime.wMinute;
    Date->seconds = localTime.wSecond;
    Date->microseconds = localTime.wMilliseconds * 1000;
    Date->day = localTime.wDay;
    Date->month = localTime.wMonth;
    Date->year = localTime.wYear;
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
      Interpreter::setTimeSliceElapsed();
   } while (!Interpreter::isTerminated());
   SystemInterpreter::setTimeSliceTimerThread(0);
#endif
   return 0;
}


void SystemInterpreter::startTimeSlice()
/******************************************************************************/
/* Function:  Make sure we have a Timer running and reset TimeSlice Sem       */
/******************************************************************************/
{
#ifdef TIMESLICE
   ULONG thread;
   if (timeSliceTimerThread == 0)
   {           /* Is there a timer?         */

     /* create a time slice timer thread */

     timeSliceTimerThread = CreateThread(NULL, TIMESLICE_STACKSIZE, TimeSliceControl, NULL, 0, &thread);
     SetThreadPriority(timeSliceTimerThread, THREAD_PRIORITY_NORMAL+1);  /* set a higher priority */
  }
  Interpreter::clearTimeSliceElapsed();
#endif
}


void SystemInterpreter::stopTimeSlice()
/******************************************************************************/
/* Function:  Stop the time slice timer thread                                */
/******************************************************************************/
{
#ifdef TIMESLICE
   TerminateThread(timeSliceTimerThread, 0);
   timeSliceTimerThread = 0;
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
RexxMethod2(int, alarm_startTimer,
            wholenumber_t, numdays,
            wholenumber_t, alarmtime)
{
    bool fState = false;                 /* Initial state of semaphore        */
    unsigned int msecInADay = 86400000;  /* number of milliseconds in a day   */
    HANDLE SemHandle = 0;                /* Event-semaphore handle            */
    UINT_PTR TimerHandle = 0;            /* Timer handle                      */

    /* Create an event semaphore that can be used to cancel the alarm. */
    SemHandle = CreateEvent(NULL, TRUE, fState, NULL);
    if ( !SemHandle )
    {
        context->RaiseException0(Rexx_Error_System_service);
        return 0;
    }

    /* set the state variables           */
    context->SetObjectVariable("EVENTSEMHANDLE", context->NewPointer(SemHandle));
    context->SetObjectVariable("TIMERSTARTED", context->True());

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
            context->RaiseException0(Rexx_Error_System_service);
            return 0;
        }

        while ( numdays > 0 )
        {
            /* Wait for the WM_TIMER message or for the alarm to be canceled. */
            waitTimerOrEvent(SemHandle);

            /* Check if the alarm is canceled. */
            RexxObjectPtr cancelObj = context->GetObjectVariable("CANCELED");

            if (cancelObj == context->True())
            {
                /* Alarm is canceled, delete timer, close semaphore, return. */
                KillTimer(NULL, TimerHandle);
                CloseHandle(SemHandle);
                return 0;
            }
            numdays--;
        }
        /* Done with the daily timer, delete it. */
        KillTimer(NULL, TimerHandle);
    }

    if ( alarmtime > 0 )
    {
        /* Start a timer for the fractional portion of the alarm. */
        TimerHandle = SetTimer(NULL, 0, (UINT)alarmtime, NULL);
        if ( !TimerHandle )
        {
            /* Couldn't create a timer, raise an exception. */
            CloseHandle(SemHandle);
            context->RaiseException0(Rexx_Error_System_service);
            return 0;
        }

        /* Wait for the WM_TIMER message or for the alarm to be canceled. */
        waitTimerOrEvent(SemHandle);

        /** Total alarm time has expired, or the alarm was canceled.  We don't
         *  care which, just clean up and return.
         */
        KillTimer(NULL, TimerHandle);
    }

    CloseHandle(SemHandle);
    return 0;
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
RexxMethod1(int, alarm_stopTimer, POINTER, eventSemHandle)
{
    /* Post the event semaphore to signal the alarm should be canceled. */
    if ( ! SetEvent((HANDLE)eventSemHandle) )
    {
        /* Raise an error if the semaphore could not be posted. */
        context->RaiseException0(Rexx_Error_System_service);
    }
    return 0;
}

