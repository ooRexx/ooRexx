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
/*   Function:  Miscellaneous system specific routines               */
/*                                                                   */
/*********************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include <stdlib.h>
#include <process.h>
#include "malloc.h"
#include "SystemVersion.h"
#include <signal.h>

/* special flag for the LPEX message loop problem */
extern BOOL UseMessageLoop = FALSE;

extern ULONG mustCompleteNest;         /* Global variable for MustComplete  */
extern "C" void activity_thread (RexxActivity *objp);


unsigned int iClauseCounter=0;         // count of clauses
unsigned int iTransClauseCounter=0;    // count of clauses in translator

extern "C" _declspec(dllimport) HANDLE ExceptionQueueSem;
extern ULONG ExceptionHostProcessId;
extern HANDLE ExceptionHostProcess;
extern BOOL ExceptionConsole;
static int SignalCount = 0;

RexxString *SysName( void )
/******************************************************************************/
/* Function: Get System Name                                                  */
/******************************************************************************/
{

  char chVerBuf[26];                   // buffer for version
  int isys;

  isys = which_system_is_running();

  if (isys == 0) strcpy(chVerBuf, "Windows");     // Windows 3.1
  else
    if (isys == 1) strcpy(chVerBuf, "WindowsNT"); // Windows NT
  else strcpy(chVerBuf, "Windows95");                                              // Windows 95

  return new_string(chVerBuf);                     /* return as a string                */
}


void SysTermination(void)
{
}

//void SysInitialize(void)
/******************************************************************************/
/* Function:   Perform system specific initialization.  For OS/2, this means  */
/*             declare the exit list processor.                               */
/******************************************************************************/
//{
//   DosExitList(EXLST_ADD,(PFNEXITLIST)exit_handler);
//}

RexxString *SysVersion(void)
/******************************************************************************/
/* Function:   Return the system specific version identifier that is stored   */
/*             in the image.                                                  */
/******************************************************************************/
{
  char chVerBuf[8];                   // buffer for version
  OSVERSIONINFO vi;
  // dont forget to change sysmeths.cmd

  vi.dwOSVersionInfoSize = sizeof(vi);  // if not set --> violation error

  GetVersionEx(&vi);              // get version with extended api
                                       /* format into the buffer            */
  wsprintf(chVerBuf,"%i.%02i",(int)vi.dwMajorVersion,(int)vi.dwMinorVersion);
  return new_string(chVerBuf);     /* return as a string                */
}


PFN SysLoadProcedure(
  RexxInteger * LibraryHandle,         /* library load handle               */
  RexxString  * Procedure)             /* required procedure name           */
/******************************************************************************/
/* Function:  Resolve a named procedure in a library                          */
/******************************************************************************/
{
  PFN     Function;                    /* resolved function address         */
  HMODULE Handle;                      /* DLL module handle                 */
  const char *Name;                    /* pointer to the module name        */


  Name = Procedure->getStringData();   /* use the ASCII-Z form of this      */
                                       /* get the module handle             */
  Handle = (HMODULE)LibraryHandle->getValue();
                                       /* try to get the function address   */
  if ( !(Function =(PFN)GetProcAddress(Handle, Name)) )
                                       /* report an exception               */
    reportException(Error_External_name_not_found_method, Procedure);
  return Function;                     /* return the pointer information    */
}

RexxInteger * SysLoadLibrary(
     RexxString * Library)             /* required library name             */
/******************************************************************************/
/* Function:  Load a named library, returning the library handle              */
/******************************************************************************/
{
  HMODULE Handle;                      /* DLL module handle                 */
  const char *Name;                    /* pointer to the module name        */

  Name = Library->getStringData();     /* use the ASCII-Z form of this      */
                                       /* get the module handle             */
  if ( Handle = GetModuleHandle( (LPCTSTR) Name) ) {
                                       /* got it?  Now get ordinal 1        */
                                       /* now load ordinal routine 1 to see */
                                       /* if this has been loaded in this   */
                                       /* process already                   */
    if (GetProcAddress( Handle, (LPCSTR)(DWORD)1) )
      return new_integer((LONG)Handle);/* return handle as an integer       */
    else {                             /* not already loaded                */
                                       /* try to load the module            */

      if (Handle = LoadLibrary((LPCTSTR)Name))
        return new_integer((LONG)Handle);
                                        /* return the new handle info        */
    }
  }
                                       /* try to load the module             */
  else if (!(Handle = LoadLibrary((LPCTSTR)Name)))
                                       /* report an error                    */
    reportException(Error_Execution_library, Library);
  return new_integer((LONG)Handle);    /* return the new handle info         */
}
RexxString * SysGetCurrentQueue(void)
/******************************************************************************/
/* Function:  Return the current queue object name                            */
/******************************************************************************/
{
  RexxString * queue;                  /* current queue object              */
  RexxString * queue_name;             /* name of the queue object          */

                                       /* get the default queue             */
  queue = (RexxString *)ActivityManager::localEnvironment->at(OREF_REXXQUEUE);

  if (queue == OREF_NULL)              /* no queue?                         */
    queue_name = OREF_SESSION;         /* use the default name              */
  else
    queue_name = queue->stringValue(); /* get the actual queue name         */  // retrofit by IH
  return queue_name;                   /* return the name                   */
}



/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysRelinquish                                */
/*                                                                   */
/*   Function:          Performs peekmsg to yield to other processes */
/*                                                                   */
/*********************************************************************/

void SysRelinquish(void)
{
    MSG msg;

   /*  If there is a msg in the message queue, dispatch it to the appropriate
    *  window proc.
    */

    if (!UseMessageLoop) return;

    if (PeekMessage (&msg,   // message structure
      NULL,                  // handle of window receiving the message
      0,                     // lowest message to examine
      0,
      PM_REMOVE))            // highest message to examine
    {
      TranslateMessage(&msg);// Translates virtual key codes
      DispatchMessage(&msg); // Dispatches message to window
    }
}

void SysSetupProgram(
  RexxActivation *activation)          /* current running program           */
/******************************************************************************/
/* Function:  Do system specific program setup                                */
/******************************************************************************/
{
  TCHAR RxTraceBuf[8];

                                       /* scan current environment,         */
  if (GetEnvironmentVariable("RXTRACE", RxTraceBuf, 8)) {
    if (!stricmp(RxTraceBuf, "ON"))    /* request to turn on?               */
                                       /* turn on tracing                   */
      activation->setTrace(TRACE_RESULTS, DEBUG_ON);
  }
}

RexxString * SysSourceString(
  RexxString * callType,               /* type of call token                */
  RexxString * programName )           /* program name token                */
/******************************************************************************/
/* Function:  Produce a system specific source string                         */
/******************************************************************************/
{
  RexxString * rsSysName;              // buffer for version

  RexxString * source_string;          /* final source string               */

        char  *outPtr;                  /* copy pointer                     */
  const char  *chSysName;               /* copy pointer                     */

  rsSysName = SysName();               /* start with the system stuff       */
  chSysName= rsSysName->getStringData();

  source_string = raw_string(rsSysName->getLength() + 2 + callType->getLength() + programName->getLength());

  outPtr = source_string->getWritableData();  /* point to the result data          */
  strcpy(outPtr, chSysName);           /* copy the system name              */
  outPtr +=rsSysName->getLength();     /* step past the name                */
  *outPtr++ = ' ';                     /* put a blank between               */
                                       /* copy the call type                */
  memcpy(outPtr, callType->getStringData(), callType->getLength());
  outPtr += callType->getLength();     /* step over the call type           */
  *outPtr++ = ' ';                     /* put a blank between               */
                                       /* copy the system name              */
  memcpy(outPtr, programName->getStringData(), programName->getLength());
  return source_string;                /* return the source string          */
}


/* HOL005M begin */
BOOL __stdcall WinConsoleCtrlHandler(DWORD dwCtrlType)
/******************************************************************************/
/* Arguments:  Report record, registration record, context record,            */
/*             dispatcher context                                             */
/*                                                                            */
/* DESCRIPTION : For Control Break conditions issue a halt to activation      */
/*               Control-C or control-Break is pressed.                       */
/*                                                                            */
/*  Returned:  Action code                                                    */
/******************************************************************************/
{
    /* set halt condition for all threads of this process */
  char envp[65];

  if ((dwCtrlType == CTRL_CLOSE_EVENT) || (dwCtrlType == CTRL_SHUTDOWN_EVENT)) return FALSE;  /* send to system */

  /* if RXCTRLBREAK=NO then ignore SIGBREAK exception */
  if (((dwCtrlType == CTRL_BREAK_EVENT) || (dwCtrlType == CTRL_LOGOFF_EVENT)) &&
      (GetEnvironmentVariable("RXCTRLBREAK", envp, 64) > 0)
      && (strcmp("NO",envp) == 0))
    return TRUE;    /* ignore signal */

  if (dwCtrlType == CTRL_LOGOFF_EVENT) return FALSE;    /* send to system */

  /* Ignore Ctrl+C if console is running in console */
  if (ExceptionConsole)
  {
      GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, ExceptionHostProcessId);
      return TRUE;   /* ignore signal */
  }

  if (ExceptionQueueSem)
      EVPOST(ExceptionQueueSem);
  if (ExceptionHostProcess)
  {
      GenerateConsoleCtrlEvent(CTRL_C_EVENT, ExceptionHostProcessId);
      TerminateProcess(ExceptionHostProcess, -1);
  }

  if (dwCtrlType == CTRL_C_EVENT)
  {
      SignalCount++;
      if (SignalCount > 1) return FALSE;    /* send signal to system */
  }

  ActivityManager::haltAllActivities();
  return TRUE;      /* ignore signal */
}


int WinExceptionFilter( int xCode )
/******************************************************************************/
/* Function:  Exception Filter used by Windows exception handling             */
/******************************************************************************/
{
  int nRtn = EXCEPTION_CONTINUE_SEARCH;
  if (!HandleException)
    nRtn = EXCEPTION_CONTINUE_EXECUTION;

  /* Put any needed code for special handling here */
  return nRtn;
}


char *SysGetThreadStackBase (size_t StackSize)
/******************************************************************************/
/* Function:  Return a pointer to the current stack base                      */
/******************************************************************************/
{
#if 0  /* this code didn't work */
   CONTEXT con;
   MEMORY_BASIC_INFORMATION mbi;
   PBYTE pbStackCrnt, pbStackBase;
   SYSTEM_INFO si;


   // Use _alloca() to get the current stack pointer
   pbStackCrnt = (PBYTE) _alloca(1);

   // Find the base of the stack's reserved region.
   VirtualQuery(pbStackCrnt, &mbi, sizeof(mbi));
   pbStackBase = (PBYTE) mbi.AllocationBase;

//   if (!GetThreadContext(GetCurrentThread(), &con)) printf("\n Bad error: thread context not received");
//   return (char *)(con.Esp - StackSize);   /* return calculated stack base         */
   return (char *) pbStackBase;
#else
   LONG temp;
   return (char *) ((ULONG)&temp - (ULONG)StackSize);
#endif
}


DWORD WINAPI call_thread_function(void * Arguments)
{
   /* call the real threadfunction */
   activity_thread((RexxActivity *)Arguments);
   return 0;
}


int SysCreateThread (
  PTHREADFN ThreadProcedure,           /* address of thread procedure       */
  size_t    StackSize,                 /* required stack size               */
  PVOID     Arguments )                /* thread procedure argument block   */
/******************************************************************************/
/* Function:  Create a new thread                                             */
/******************************************************************************/
{
  DWORD res;
  HANDLE ht;

  ht = CreateThread(NULL, StackSize, call_thread_function, Arguments, 0, &res);
  if (!ht)
  {
      reportException(Error_System_service_service, "ERROR CREATING THREAD");
     return 0;   /* error */
  }
  ((RexxActivity *)Arguments)->hThread = ht;
  return res;
}


void SysSetThreadPriority(long tid, HANDLE han, int prio)
{
  ULONG pri;

                                       /* critical priority?                */
  if (prio >= HIGH_PRIORITY) {
    pri= THREAD_PRIORITY_ABOVE_NORMAL +1; /* the class is regular, but move    */
                                         /* to the head of the class          */
  }                                    /* medium priority                   */
  else if (prio >= MEDIUM_PRIORITY) {
    pri = THREAD_PRIORITY_NORMAL;      /* normal class,                     */
                                       /* dead in the middle of it all      */
  }
  else {                               /* low priority                      */
    pri = THREAD_PRIORITY_IDLE +1;     /* give us idle only, but make it    */
                                       /* important idle time only          */
  }

  SetThreadPriority( han, pri);
}


