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
/*                                                                   */
/*   Subroutine Name:   SysGetCurrentTime                            */
/*                                                                   */
/*   Function:          gets the time and date from the system clock */
/*********************************************************************/

#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "RexxNativeAPI.h"

extern SEV rexxTimeSliceSemaphore;
extern ULONG  RexxTimeSliceTimer;
extern ULONG  rexxTimeSliceTimerOwner;
extern RexxInteger * ProcessName;

extern BOOL UseMessageLoop;  /* for VAC++ */

void CALLBACK TimerProc( HWND, UINT, UINT, DWORD);
void CALLBACK alarmTimerProc( HWND, UINT, UINT, DWORD);

static INT dayc[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
static HANDLE SemHandle = 0;                  /* Event-semaphore handle repository */
static UINT TimerHandle = 0;                   /* Timer handle                      */
static TIMERPROC lpTimerProc;

#define TIMESLICE_STACKSIZE 2048
#define TIMESLICEMS 10
#define ALARMSLEEP 330

#ifdef TIMESLICE
extern SEV   RexxTerminated;           /* Termination complete semaphore.   */
extern INT REXXENTRY RexxSetYield(PID procid, TID threadid);
extern BOOL rexxTimeSliceElapsed;
#endif

void SysGetCurrentTime(
  REXXDATETIME *Date )                 /* returned data structure    */
/*********************************************************************/
/* Function:  Return a time stamp to the kernel date/time functions. */
/*********************************************************************/
{
  SYSTEMTIME SystemDate; /* system date structure      */

  GetLocalTime(&SystemDate);        /* via Windows                */

  Date->hours = SystemDate.wHour;
  Date->minutes = SystemDate.wMinute;
  Date->seconds = SystemDate.wSecond;
  Date->hundredths = (UINT)SystemDate.wMilliseconds / (UINT )10;
 Date->microseconds = (UINT )SystemDate.wMilliseconds * (UINT )1000;
  Date->day = SystemDate.wDay;
  Date->month = SystemDate.wMonth;
  Date->year = SystemDate.wYear;
  Date->weekday = SystemDate.wDayOfWeek;
  Date->yearday = (USHORT)(dayc[Date->month - 1] + Date->day);
  if (Date->month > 2 && (!(Date->year % 4)) && ((Date->year % 100) || (! (Date->year % 400))))
    Date->yearday++;                   /* adjust for leap year       */
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysTimeSliceElapsed                          */
/*                                                                   */
/*     Used to see if iClauseCounter == CLAUSESPERYIELD              */
/*     This is used in windows as the duration of a time slice       */
/*                                                                   */
/*********************************************************************/

DWORD WINAPI TimeSliceControl(void * args)
{
#ifdef TIMESLICE
   MSG msg;
   do
   {
      Sleep(TIMESLICEMS);
   /* instead of SysRelinquish -> performance */
      if ((UseMessageLoop) && (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)))
      {
         TranslateMessage(&msg);// Translates virtual key codes
         DispatchMessage(&msg); // Dispatches message to window
      }
      rexxTimeSliceElapsed = TRUE;
   } while (RexxTerminated && (WaitForSingleObject(RexxTerminated, 0) != WAIT_OBJECT_0));
   rexxTimeSliceTimerOwner = 0;
#endif
   return 0;
}


void SysStartTimeSlice( void )
/******************************************************************************/
/* Function:  Make sure we ahve a Timer running and reset TimeSlice Sem       */
/******************************************************************************/
{
#ifdef TIMESLICE
   ULONG thread;
   if (!rexxTimeSliceTimerOwner) {           /* Is there a timer?         */

	 /* create a thread with low priority to check the message queue for WM_TIMER */

     rexxTimeSliceTimerOwner = (ULONG)CreateThread(NULL, TIMESLICE_STACKSIZE, TimeSliceControl, NULL, 0, &thread);
     SetThreadPriority((HANDLE)rexxTimeSliceTimerOwner,THREAD_PRIORITY_NORMAL+1);  /* set a higher priority */
  }
  rexxTimeSliceElapsed = FALSE;
#endif
}


void SysStopTimeSlice( void )
/******************************************************************************/
/* Function:  Stop the thread that examines the message queue for WM_TIMER    */
/******************************************************************************/
{
#ifdef TIMESLICE
   TerminateThread((HANDLE)rexxTimeSliceTimerOwner, 0);
   rexxTimeSliceTimerOwner = 0;
#endif
}


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   alarm_starTimer                              */
/*                                                                   */
/*   Function:          starts an asynchronous, single_interval      */
/*                      timer. When the timer pops, it will post an  */
/*                      event semaphore.                             */
/*                                                                   */
/*   Arguments:         alarm timer - time interval before the event */
/*                       semaphore is posted                         */
/*********************************************************************/
RexxMethod2(void, alarm_startTimer,
                     long, numdays,
                     long, alarmtime)
									    // retrofit by IH

{
  APIRET rc;                           /* return code                       */
  BOOL fState = FALSE;                 /* Initial state of semaphore        */
                                       /* Time-out value                    */
  ULONG ulTimeout = INFINITE;
  long msecInADay = 86400000;          /* number of milliseconds in a day   */
  long timerIntervall = 60000;		   /* resolution 1 min */
  REXXOBJECT cancelObj;                /* place object to check for cancel  */
  long cancelVal;                      /* value of cancel                   */
  MSG msg; 			       /* to retrieve WM_TIMER */
  HANDLE SemHandle = 0;                /* Event-semaphore handle repository */
  UINT TimerHandle = 0;                /* Timer handle                      */
                                       /* Create an event semaphore to be   */
                                       /* posted by the timer               */
  SemHandle = CreateEvent(NULL, TRUE, fState, NULL);
  if (!SemHandle) {                    /* Error received ?                  */
                                       /* raise error                       */
     send_exception(Error_System_service);
     return;
  }
                                       /* set the state variables           */
  RexxVarSet("EVENTSEMHANDLE",RexxInteger((long)SemHandle));
  RexxVarSet("TIMERSTARTED",RexxTrue);

  while (numdays > 0) {                /* is it some future day?            */

                                       /* start timer to wake up after a day*/
	TimerHandle = SetTimer(NULL, 0, msecInADay, NULL);
    if (TimerHandle == 0) {                     /* Error received?          */
      send_exception(Error_System_service);
      return;
    }

	/* wait for WM_TIMER message */
    if (UseMessageLoop)  /* without peekmessage for VAC++ */
  	do
       if (PeekMessage (&msg,   // message structure
           NULL,                  // handle of window receiving the message
          0,                     // lowest message to examine
          0,
          PM_REMOVE))            // highest message to examine
        {
          TranslateMessage(&msg);// Translates virtual key codes
          DispatchMessage(&msg); // Dispatches message to window
        } else Sleep(ALARMSLEEP);
	while ((msg.message != WM_TIMER) && (WaitForSingleObject(SemHandle, 0) != WAIT_OBJECT_0));
	else WaitForSingleObject(SemHandle, INFINITE);

	EVPOST(SemHandle);

#if 0
                                       /* wait for semaphore to be posted   */
    rc = WaitForSingleObject(SemHandle, ulTimeout);
    if (rc !=  WAIT_OBJECT_0)
    {                                   /* raise error                       */
      send_exception(Error_System_service);
      return;
    }
#endif
                                       /* get the cancel state              */
    cancelObj = RexxVarValue("CANCELED");
    cancelVal = REXX_INTEGER_VALUE(cancelObj);

    if (cancelVal == 1) {              /* If alarm cancelled?               */
      rc = KillTimer(NULL, TimerHandle);      /* stop timer                        */
                                       /* Close the event semaphore         */
      rc = CloseHandle(SemHandle);
      if (rc == FALSE) {                   /* Error received?                   */
                                       /* raise error                       */
       send_exception(Error_System_service);
       return;
      }
      return;
    }
    else {
                                       /* Reset the event semaphore         */
      rc = EVSET(SemHandle);
      if (rc == FALSE) {                   /* Error received?                   */
                                       /* raise error                       */
        send_exception(Error_System_service);
        return;
      }
    }
    numdays--;                         /* Decrement number of days          */
  }

                                       /* start the timer                   */
  TimerHandle = SetTimer(NULL, 0, alarmtime, NULL);
  if (!TimerHandle) {                       /* Error received?                   */
                                       /* raise error                       */
     send_exception(Error_System_service);
     return;
  }

	/* wait for WM_TIMER message */
  if (UseMessageLoop)  /* without peekmessage for VAC++ */
  do
       if (PeekMessage (&msg,   // message structure
           NULL,                  // handle of window receiving the message
          0,                     // lowest message to examine
          0,
          PM_REMOVE))            // highest message to examine
        {
          TranslateMessage(&msg);// Translates virtual key codes
          DispatchMessage(&msg); // Dispatches message to window
        } else Sleep(ALARMSLEEP);
  while ((msg.message != WM_TIMER) && (WaitForSingleObject(SemHandle, 0) != WAIT_OBJECT_0));
  else WaitForSingleObject(SemHandle, INFINITE);

  EVPOST(SemHandle);

#if 0
                                       /* wait for semaphore to be posted   */
  rc = WaitForSingleObject(SemHandle, ulTimeout);
  if (rc !=  WAIT_OBJECT_0)
  {                                     /* raise error                       */
     send_exception(Error_System_service);
     return;
  }
#endif
                                       /* We don't care about error here    */
                                       /* Since the timer may have          */
                                       /* already popped.                   */
  rc = KillTimer(NULL, TimerHandle);          /* stop timer                        */
  if (rc == FALSE) {                       /* Error received?                   */
                                       /* raise error                       */
     send_exception(Error_System_service);
     return;
  }
                                       /* Close the event semaphore         */
  rc = CloseHandle(SemHandle);
  if (rc == FALSE) {                       /* Error received?                   */
                                       /* raise error                       */
     send_exception(Error_System_service);
     return;
  }
  return;
}





/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   alarm_stopTimer                              */
/*                                                                   */
/*   Function:          stops an asynchronous timer.                 */
/*                                                                   */
/*   Arguments:         timer - timer handle                         */
/*                        shared between start & stop timer          */
/*********************************************************************/


RexxMethod1(void, alarm_stopTimer,
               long, eventSemHandle)
{
  APIRET rc;                           /* return code                       */
  HANDLE    hev = (HANDLE)eventSemHandle;    /* event semaphore handle            */

#if 0
                                       /* Open the event semaphore          */
  hev = OpenEvent(EVENT_ALL_ACCESS, TRUE, NULL);
  if (hev == 0) {                       /* Error received?                   */
                                       /* raise error                       */
     send_exception(Error_System_service);
     return;
  }
#endif
                                       /* Post the event semaphore          */
  rc = EVPOST(hev);
  if (rc == FALSE) {                       /* Error received?                   */
                                       /* raise error                       */
     send_exception(Error_System_service);
     return;
  }

#if 0
                                       /* Close the event semaphore         */
  rc = CloseHandle(hev);
  if (rc == FALSE) {                       /* Error received?                   */
                                       /* raise error                       */
     send_exception(Error_System_service);
     return;
  }

  rc = KillTimer(NULL, TimerHandle);
#endif

return;
}



