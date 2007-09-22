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
/******************************************************************************/
/* REXX WIN32 Support                                           winrexx.h     */
/*                                                                            */
/* WIN32 master definition file                                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Template for system specific declarations for Object REXX.  The following  */
/* sections are a series of required and optional sections where system       */
/* specific information is provided to the rest of the interpreter code.      */
/* Wherever possible, reasonable defaults are provided for these settings.    */
/******************************************************************************/

#ifndef WINREXX_H                        /* provide a define here to protect  */
#define WINREXX_H                        /* against multiple includes         */

/******************************************************************************/
/* REQUIRED:  The implemenation must decide on the C_STACK_SIZE defining      */
/* constants that are compiler/linker dependent.                              */
/******************************************************************************/
#define MIN_C_STACK 1024*32
#define TOTAL_STACK_SIZE 1024*512
#define C_STACK_SIZE 60000

/******************************************************************************/
/* REQUIRED:  The following type definitions are used throughout the REXX     */
/* kernel code, so definitions are required for all of these.  If the system  */
/* in questions provides definitions for these via other include              */
/* files, any of these items can be deleted from the system specific file and */
/* and replaced by any replacement #includes at this point.                   */
/******************************************************************************/
 /* Windows style definitions */
 #include <windows.h>                   // required for windows apps

/* include windows debugging function definition */
#include "DebugOutput.h"

//#define TRUE            1
//#define FALSE           0

//#define ULONG           unsigned long   // Unsigned long integer 32-bits
//#define PULONG          ULONG *         // Pointer to an unsigned long (32 bits)
//#define PVOID           void *          // Pointer to any type
//#define SHORT           short           // Short integer
//#define USHORT          unsigned short  // Unsigned short integer (16 bits)
//#define PSHORT          SHORT *         // Pointer to a signed short (16 bits)
//#define PUSHORT         USHORT *        // Pointer to an unsigned short (16 bits)
//#define UCHAR           unsigned char   // Unsigned Windows character
//#define PUCHAR          UCHAR *         // Pointer to an unsigned Windows character
//#define CHAR            char            // Windows character
//#define PCHAR           CHAR *          // Pointer to a Windows character
//#define INT             int             // Signed integer
//#define PINT            int *           // Pointer to a signed integer
//#define PCH             PCHAR           // Pointer to a Windows character
//#define PSZ             PCHAR           // Pointer to a null-terminated Windows character string
//#define CONST           const           // Variable whose value is to remain constant during execution
//#define BYTE            unsigned char   // Byte (8 bits)
//#define BOOL            unsigned long   // Boolean variable (should be TRUE or FALSE)
#define UBYTE           unsigned char     // Not defined in windows
// #define TID             long           // defined in wintypes.h
// #define PID             long
//#define VOID            void            // any type
#define near                              //
#define far                               //
#define _loadds                           //
#define _Optlink
#define SysCall _cdecl
#define SysCallV __cdecl

typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;

#define UINT64_MAX (~((uint64_t)0))
#define INT64_MAX  ((int64_t)(UINT64_MAX >> 1))
#define INT64_MIN  ((int64_t)UINT64_MAX)


/******************************************************************************/
/* OPTIONAL:  If the implementation is going to support a cross-process       */
/* shared memory model, then include a define for SHARED                      */
/******************************************************************************/

/* enable the flag so RexxStart can be called within threads */
#define SHARED                         // no shared memory sup

/******************************************************************************/
/* OPTIONAL:  If the implementation is going to support SOM, include a define */
/* for SOM.                                                                   */
/******************************************************************************/

//#define SOM
//#define SOMV2                          /* both these are required yet       */

/******************************************************************************/
/* OPTIONAL:  If the implementation is going to support multiple threads,     */
/* include a define for THREADS.  If not defined, then the REPLY instruction  */
/* will not work.                                                             */
/******************************************************************************/
#define THREADS

/******************************************************************************/
/* OPTIONAL:  Perform stack bounds checking on new message invocations.  If   */
/* this is a non-stack based calling convention, or it is not possible to     */
/* determine the bounds of the stack, leave this undefined.                   */
/******************************************************************************/
#define STACKCHECK

/******************************************************************************/
/* OPTIONAL:  Need support for non-ansi string routines strdup, strupr,       */
/* strlwr, stricmp, and memicmp.                                              */
/******************************************************************************/

//#define NEED_NON_ANSI

/******************************************************************************/
/* OPTIONAL:  No time slicing support for REXX activation yields is available.*/
/* Code will yield after a given count of instructions.                       */
/******************************************************************************/
//
// timers in windows only get called back at yield boundaries
//
//#define NOTIMER                        // used in activa, activi for
                                        // count based yielding

/******************************************************************************/
/* OPTIONAL:  Enable concurrency timeslice dispatching support.  Default is   */
/* only yield at specific event points.                                       */
/******************************************************************************/
// Make this clearer? Mutually exclusive with NOTIMER?
#define TIMESLICE
#define FIXEDTIMERS

/******************************************************************************/
/* OPTIONAL:  Enable the storage of activities in a directory instead of an   */
/* array. If this option is disabled, all threadIDs will be stored in a table */
/* (the threadTable) to get the index of the activity                         */
/* (Important for high IDs like on Windows 95)                                */
/******************************************************************************/
#define HIGHTID
/* this symbol should be also defined using HIGHTID because of                */
/* SetThreadPriority. This API needs the threads handle, not it's ID.         */
/* Without the ThreadTable we don't get the handle so we have to store it     */
/* within the Activity (in addition to the threads ID)                        */
#define THREADHANDLE


/******************************************************************************/
/* OPTIONAL:  Enable the new handling of a GUARD ON WHEN statement            */
/* The access to a variable used as a GUARD ON WHEN expression will be        */
/* serialized. That means, that an activity waiting for the guard will be     */
/* able to check the expression after every modification that has been done   */
/* to the expression variable                                                 */
/******************************************************************************/
// not ready yet (if it is defined, rexx runs slow)
//#define NEWGUARD

/******************************************************************************/
/* OPTIONAL:  If the implementation enables external scripting support, then  */
/* additional hand-shaking with an the exernal environment is enabled for     */
/* providing default values for uninitialized variables.                      */
/******************************************************************************/
#define SCRIPTING


/******************************************************************************/
/* REQUIRED:  Define the REXX type for semaphores.  These can be system       */
/* specific semaphore types or the REXX define OSEM.                          */
/******************************************************************************/
/*#include "oryxthrd.h"      not needed, defines moved in here           */
#define SMTX HANDLE                 /* semaphore data types              */
#define SEV  HANDLE
#define SysSharedSemaphoreDefn HANDLE rexx_kernel_semaphore = NULL;     \
                               HANDLE rexx_resource_semaphore = NULL;   \
                               HANDLE rexx_start_semaphore = NULL;      \
                               HANDLE rexx_wait_queue_semaphore = NULL; \
                               HANDLE rexxTimeSliceSemaphore = NULL;    \
                               ULONG rexxTimeSliceTimerOwner;


/******************************************************************************/
/* REQUIRED:  Define the REXX type for exceptions.  These can be system       */
/* specific exception registration info or any place holder type if this      */
/* doesn't apply.                                                             */
/******************************************************************************/

#define SYSEXCEPTIONBLOCK LONG

/******************************************************************************/
/* REQUIRED:  Define the REXX type for a "thread function" that starts off    */
/* a new thread.                                                              */
/******************************************************************************/

typedef void (* PTHREADFN)(void *);    /* define a thread function          */

/******************************************************************************/
/* REQUIRED:  Define any special requirements for external entry calls back   */
/* into the interpreter.  The default is no special requirements.             */
/******************************************************************************/

#define REXXENTRY APIENTRY

/******************************************************************************/
/* REQUIRED:  This was needed for Windows. Any entry points containing        */
/* variable length arguments need to use __cdecl calling convention.          */
/******************************************************************************/

#define VLAREXXENTRY __cdecl           /* external entry points       */
#define VLAENTRY __cdecl               /* internal entry points       */

/******************************************************************************/
/* REQUIRED:  Definitions for REXX semaphore functions.  These default to     */
/* the REXX library semaphore package, but can be redefined to map directly   */
/* to system specific functions too.                                          */
/******************************************************************************/
//  Check these changes in oryxthrd.h

//SECURITY_ATTRIBUTES SA_inherit = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

#ifdef _DEBUG
// #define TRACE_SEMAPHORES    /* not necessary to trace MTX and EV macros */
#endif

/* create or open a named mutex semaphore */

#define MTXCROPEN(s,n) /*printf("MTXCROPEN called with %x/%s in %s line %d\n", s, n, __FILE__, __LINE__);*/ MTXCR(s)

/* create mutex only if it is NULL */
#ifdef TRACE_SEMAPHORES
#define MTXCR(s)      if (!s) {   \
s = CreateMutex(NULL, FALSE, NULL); \
printf("Created MTX %x in %s line %d\n", s, __FILE__, __LINE__);} \
else printf("MTXCR handle %x not null in %s line %d\n", s, __FILE__, __LINE__)

#else
#define MTXCR(s)      if (!s) s = CreateMutex(NULL, FALSE, NULL)  // create a mutex semaphore
#endif


                                       // request wait on a semaphore
#define MTXRQ(s)      /* WaitForSingleObject(s, INFINITE) */ \
                      do \
                      {    \
                         SysRelinquish(); \
                      } while (WaitForSingleObject(s, 1) == WAIT_TIMEOUT);
#define MTXRL(s)      ReleaseMutex(s) // clear a semaphore
#ifdef TRACE_SEMAPHORES
#define MTXCL(s)      if (!CloseHandle(s)) \
                          printf("CloseHandle failed to close MTX %x --> %d\n", s, GetLastError()); \
                      else printf("Closed MTX handle %x\n", s)

#else
#define MTXCL(s)      CloseHandle(s)                 // no need to close in thread package
#endif
                                       // no wait, return if can't get it
#define MTXRI(s)      WaitForSingleObject(s,SEM_IMMEDIATE_RETURN)
#define MTXNOPEN(s,n) s = OpenMutex(MUTEX_ALL_ACCESS, TRUE, n)                 // only used for shared stuff
#define MTXNCR(s,n)   s = CreateMutex(NULL, FALSE, n)

#ifdef TRACE_SEMAPHORES
#define EVCR(s)      if (!s) {   \
s = CreateEvent(NULL, TRUE, TRUE, NULL); \
printf("Created EV %x in %s line %d\n", s, __FILE__, __LINE__);} \
else printf("EVCR handle %x not null in %s line %d\n", s, __FILE__, __LINE__)

#else
#define EVCR(s)       if (!s) s = CreateEvent(NULL, TRUE, TRUE, NULL)
#endif

/* create or open a named event semaphore */
#define EVCROPEN(s,n) /* printf("EVCROPEN called with %x/%s in %s line %d\n", s, n, __FILE__, __LINE__);*/ EVCR(s)

#ifdef NEWGUARD
/* these macros are needed to serialize the access to a variable that */
/* is used as a GUARD ON WHEN expression */
/* AEVCR creates an automatic event (only one thread can get it) */
#define AEVCR(s)      s = CreateEvent(NULL, FALSE, TRUE, NULL)
/* AEVSIGNALED is true if the semaphore is in the signaled state */
/* important: it will return immediately */
#define AEVSIGNALED(s) (!WaitForSingleObject(s,SEM_IMMEDIATE_RETURN))
#endif

#define EVPOST(s)     SetEvent(s)
#define EVSET(s)      ResetEvent(s)
#define EVWAIT(s)     /* WaitForSingleObject(s, INFINITE) */ MTXRQ(s)
#define EVCL(s)       // CloseHandle(s)          // we don't use openevent so no close is needed

#ifdef TRACE_SEMAPHORES
#define EVCLOSE(s)    if (!CloseHandle(s)) \
                          printf("CloseHandle failed to close EV %x --> %d\n", s, GetLastError());\
                          else printf("Closed EV handle %x\n", s)

#else
#define EVCLOSE(s)    CloseHandle(s)     // for the parts where the semaphore should be closed
#endif

#define EVOPEN(s)      // should be a duplicate there
#define EVCLEAR(s)    s=NULL // clear an OSEM
#define EVEXIST(s)    ((s != INVALID_HANDLE_VALUE) && (s != NULL))

/******************************************************************************/
/* REQUIRED:  Definitions for entering and exiting code sections that         */
/* manipulate globally defined resources.                                     */
/******************************************************************************/

#define SysEnterResourceSection() MTXRQ(resource_semaphore);
#define SysExitResourceSection() MTXRL(resource_semaphore);

/******************************************************************************/
/* REQUIRED:  Definitions for entering and exiting critical code sections.    */
/* These can be defined out to nothing if these have no meaning.              */
/******************************************************************************/

extern CRITICAL_SECTION Crit_Sec;

/* don't initialize Crit_Sec twice */
#define SysThreadInit() if (!Crit_Sec.DebugInfo) InitializeCriticalSection(&Crit_Sec)
#define SysEnterCriticalSection() EnterCriticalSection(&Crit_Sec)
#define SysExitCriticalSection()  LeaveCriticalSection(&Crit_Sec)

#define SEM_IMMEDIATE_RETURN 0L
#define SEM_INDEFINITE_WAIT  INFINITE

/******************************************************************************/
/* REQUIRED:  Define a routine to determine if an activation needs to yield.  */
/*   Use whatever mechanism is approcpriate for given System.                 */
/******************************************************************************/

#ifndef NO_SYSTIMESLICEELAPSED
inline BOOL SysTimeSliceElapsed( void )
{
  extern BOOL rexxTimeSliceElapsed;
                                       /* see if the timer was called*/
  if (rexxTimeSliceElapsed)
  {
     rexxTimeSliceElapsed = FALSE;
     return 1;
  }
  else
     return 0;                                                                  /* 0 because no wait, only query */
}
#endif

/******************************************************************************/
/* REQUIRED:  Routine to start a new TimeSlice period.                        */
/******************************************************************************/

void SysStartTimeSlice(void);

/* Windows needs a special line write function to check stdout output */
LONG line_write_check(char * , LONG , FILE * );

/******************************************************************************/
/* REQUIRED:  Routines to alloc memory passed to external environments        */
/******************************************************************************/

#define SysAllocateExternalMemory(s) GlobalAlloc(0, (s))
#define SysFreeExternalMemory(p) GlobalFree((p))


/******************************************************************************/
/* REQUIRED:  Define the macro to call the line_write_function                */
/******************************************************************************/

#define line_write(b,c) checked_line_write(b,c)

/******************************************************************************/
/* REQUIRED:  Define the string used for the default initial address setting. */
/******************************************************************************/

#define SYSINITIALADDRESS "CMD"

/******************************************************************************/
/* REQUIRED:  Define the name of the image file that is saved and restored.   */
/******************************************************************************/

#define BASEIMAGE     "rexx.img"

/******************************************************************************/
/* REQUIRED:  Define the name of the program called to load create the saved  */
/* image file.                                                                */
/******************************************************************************/

#define BASEIMAGELOAD "CoreClasses.orx"

/******************************************************************************/
/* REQUIRED:  Define a name for the initialization semaphore.  If not required*/
/* for your system, just make this any string.                                */
/******************************************************************************/

#define INIT_SEM_NAME "INIT_SEM"

/******************************************************************************/
/* OPTIONAL:  Define name of rexxsaa.h toolkit header file for                */
/******************************************************************************/
// Call ours REXXWIN.H
#define SYSREXXSAA "REXX.H"

/******************************************************************************/
/* REQUIRED:  Define the REXX type to hold information for a WIndow Environmen*/
/* These can be system specifc handles/etc for messageQueuue/Windows/etc      */
/* or any place holder type if this doesn't apply                             */
/******************************************************************************/
typedef PVOID SYSWINDOWINFO;


/******************************************************************************/
/* REQUIRED:  Name of the file used to store the external message repository  */
/******************************************************************************/
#define REXXMESSAGEFILE    ((PSZ)"winatab.rc")


#define SYSTEM_INTERNAL_METHODS() \
   INTERNAL_METHOD(sysRxfuncadd) \
   INTERNAL_METHOD(sysRxfuncdrop) \
   INTERNAL_METHOD(sysRxfuncquery) \
   INTERNAL_METHOD(sysBeep) \
   INTERNAL_METHOD(sysSetLocal) \
   INTERNAL_METHOD(sysEndLocal) \
   INTERNAL_METHOD(sysDirectory) \
   INTERNAL_METHOD(sysFilespec) \
   INTERNAL_METHOD(sysMessageBox) \
   INTERNAL_METHOD(server_init_local) \
   INTERNAL_METHOD(server_c_sominit) \
   INTERNAL_METHOD(server_findsomclass) \
   INTERNAL_METHOD(server_somclass) \
   INTERNAL_METHOD(server_somname) \
   INTERNAL_METHOD(server_somparent) \
   INTERNAL_METHOD(server_somproxy) \
   INTERNAL_METHOD(server_somtrace) \
   INTERNAL_METHOD(function_queueExit)

#ifdef INCL_REXX_STREAM                /* asking for file system stuff?     */
/******************************************************************************/
/* REQUIRED:  include whatever library system specific include files are      */
/* needed.  If it is necessary to override any standard routines, do this     */
/* here.                                                                      */
/******************************************************************************/

#include <sys\types.h>
#include <sys\stat.h>

/******************************************************************************/
/* REQUIRED:  define the path delimiter and end of line information           */
/* appropriate for this file system.                                          */
/******************************************************************************/

#define delimiter '\\'
#define line_end "\r\n"
#define line_end_size 2
                                       /* adjust scan pointer               */
#define adjust_scan_at_line_end() if (*scan != '\0' && *scan == line_end[1]) scan++

#endif

/******************************************************************************/
/* REQUIRED:  Define the REXX type for a native method function               */
/******************************************************************************/

                                       /* pointer to native method function */
typedef char *(far REXXENTRY *PNMF)(void **);

/******************************************************************************/
/* REQUIRED:  Define the macro for pointer subtraction                        */
/******************************************************************************/

#define PTRSUB(f,s) ((char *)(f)-(char *)(s))

/******************************************************************************/
/* OPTIONAL:  Overrides for any functions defined in sysdef.h.  These         */
/* can map the calls directly to inline code or comment them out all together.*/
/******************************************************************************/

#define SysINTName SysName

#define DEFRXSTRING 256                 /* Default RXSTRING return size      */

#if 0
#define SysSetThreadPriority(a, b) SetThreadPriority((HANDLE)a, b)
#endif
                                        // Thread yielding funcs in olthread.c
#define SysThreadYield()

#ifdef HIGHTID
#define SysQueryThreadID() GetCurrentThreadId()
#endif

#ifdef THREADHANDLE
#define SysQueryThread() GetCurrentThread()
#endif

//#define SysGetThreadStackBase() NULL

//#define SysCreateThread(PTHREADFN, INT, PVOID) WinCreateThread(PTHREADFN, INT, PVOID)

//DWORD WinCreateThread(PTHREADFN pThFunc, INT StackSize, PVOID arg);

INT SysCreateThread (
  PTHREADFN ThreadProcedure,           /* address of thread procedure       */
  INT       StackSize,                 /* required stack size               */
  PVOID     Arguments );                /* thread procedure argument block   */


#define SysValidateAddressName(OREF)   // OS2MISC: Validates address
                                       // command name Need on win?
#define SysAllocateHeap();             // no shared memory on windows yet
#define SysReleaseHeap();              // no shared memory on windows yet

void SysTermination();              // No initialization / termination yet

#define SysInitialize();               //
                                       // our exception handling
#define SysRegisterExceptions(SYSEXCEPTIONBLOCK) HandleException = TRUE;
#define SysDeregisterExceptions(SYSEXCEPTIONBLOCK) HandleException = FALSE;
                                       // no signal handling
#define SysDeregisterSignals(SYSEXCEPTIONBLOCK)
#define SysRegisterSignals(SYSEXCEPTIONBLOCK)
                                       // no wps environment, please port!
#define SysInitializeWindowEnv() NULL  // activi.c has an assignment
#define SysTerminateWindowEnv(SYSWINDOWINFO)
                                       //Temp workaround for message handling
#define SysMessageHeader(INT) OREF_NULL
//#define SysSaveProgram(a,b)            // Unable to save a program image
                                       // in Windows, no EA's
#define SysClauseBoundary(a)

// retrofit by IH
#define SysInitialAddressName() OREF_INITIALADDRESS
#define SysInitializeThread()

#define SysTerminateThread(t)
#define SysGetTempFileName()  tmpnam(NULL)


/******************************************************************************/
/* OPTIONAL:  Finally, any other global defined constants for system specific */
/* code usage.                                                                */
/******************************************************************************/
#include "wintypes.h"
 // Re-directed output...
 #include <stdio.h>                    // for following file ref...

 // Exception handling
 typedef BOOL __stdcall CONSOLECTRLHANDLER(DWORD);
 CONSOLECTRLHANDLER WinConsoleCtrlHandler;

 #define WinBeginExceptions SetConsoleCtrlHandler(&WinConsoleCtrlHandler, TRUE);\
                            __try {
 #define WinEndExceptions } __except ( WinExceptionFilter(GetExceptionCode( ))) {  }\
                            SetConsoleCtrlHandler(&WinConsoleCtrlHandler, FALSE);

 extern BOOL HandleException;
 int WinExceptionFilter( int xCode );


// Options character used for command line programs
#define OPT_CHAR  '/'                  /* Option char for win                */

#define  OLDMETAVERSION       29       /* highest meta version number:                  */


#endif

