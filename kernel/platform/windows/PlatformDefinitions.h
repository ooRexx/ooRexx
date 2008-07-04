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

#define SysCall _cdecl

/******************************************************************************/
/* OPTIONAL:  Perform stack bounds checking on new message invocations.  If   */
/* this is a non-stack based calling convention, or it is not possible to     */
/* determine the bounds of the stack, leave this undefined.                   */
/******************************************************************************/
#define STACKCHECK

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

/* this symbol should be also defined using HIGHTID because of                */
/* SetThreadPriority. This API needs the threads handle, not it's ID.         */
/* Without the ThreadTable we don't get the handle so we have to store it     */
/* within the Activity (in addition to the threads ID)                        */
#define THREADHANDLE

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
#define SMTX HANDLE                 /* semaphore data types              */
#define SEV  HANDLE

/******************************************************************************/
/* REQUIRED:  Define the REXX type for exceptions.  These can be system       */
/* specific exception registration info or any place holder type if this      */
/* doesn't apply.                                                             */
/******************************************************************************/

typedef int SYSEXCEPTIONBLOCK;

/******************************************************************************/
/* REQUIRED:  Define the REXX type for a "thread function" that starts off    */
/* a new thread.                                                              */
/******************************************************************************/

typedef void (* PTHREADFN)(void *);    /* define a thread function          */

/******************************************************************************/
/* REQUIRED:  Definitions for REXX semaphore functions.  These default to     */
/* the REXX library semaphore package, but can be redefined to map directly   */
/* to system specific functions too.                                          */
/******************************************************************************/

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

void SysRelinquish(void);              /* allow the system to run           */

inline void waitHandle(HANDLE s)
{
    MSG msg = {0};

    // If already signaled, return.
    if ( WaitForSingleObject(s, 0) == WAIT_OBJECT_0 )
    {
        return;
    }

    /** Any thread that creates windows must process messages.  A thread that
     *  calls WaitForSingelObject with an infinite timeout risks deadlocking the
     *  system.  MS's solution for this is to use MsgWaitForMultipleObjects to
     *  wait on the object, or a new message arriving in the message queue. Some
     *  threads create windows indirectly, an example is COM with CoInitialize.
     *  Since we can't know if the current thread has a message queue that needs
     *  processing, we use MsgWaitForMultipleObjects.
     *
     *  Note that MsgWaitForMultipleObjects only returns if a new message is
     *  placed in the queue.  PeekMessage alters the state of all messages in
     *  the queue so that they are no longer 'new.'  Once PeekMessage is called,
     *  all the messages on the queue need to be processed.
     */
    do
    {
        while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // Check to see if signaled.
            if ( WaitForSingleObject(s, 0) == WAIT_OBJECT_0 )
            {
                return;
            }
        }
    } while ( MsgWaitForMultipleObjects(1, &s, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0 + 1 );
}

#define MTXRQ(s)      waitHandle(s);
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
#define MTXNOPEN(s,n) s = OpenMutex(MUTEX_ALL_ACCESS, true, n)                 // only used for shared stuff
#define MTXNCR(s,n)   s = CreateMutex(NULL, FALSE, n)

#ifdef TRACE_SEMAPHORES
#define EVCR(s)      if (!s) {   \
s = CreateEvent(NULL, true, true, NULL); \
printf("Created EV %x in %s line %d\n", s, __FILE__, __LINE__);} \
else printf("EVCR handle %x not null in %s line %d\n", s, __FILE__, __LINE__)

#else
#define EVCR(s)       if (!s) s = CreateEvent(NULL, true, true, NULL)
#endif

/* create or open a named event semaphore */
#define EVCROPEN(s,n) /* printf("EVCROPEN called with %x/%s in %s line %d\n", s, n, __FILE__, __LINE__);*/ EVCR(s)

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
/* REQUIRED:  Definitions for entering and exiting critical code sections.    */
/* These can be defined out to nothing if these have no meaning.              */
/******************************************************************************/

#define SEM_IMMEDIATE_RETURN 0L
#define SEM_INDEFINITE_WAIT  INFINITE

/* Windows needs a special line write function to check stdout output */
size_t line_write_check(const char * , size_t, FILE * );

/******************************************************************************/
/* REQUIRED:  Routines to alloc memory passed to external environments        */
/******************************************************************************/

#define SysAllocateExternalMemory(s) GlobalAlloc(0, (s))
#define SysFreeExternalMemory(p) GlobalFree((HGLOBAL)(p))


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
/* REQUIRED:  Name of the file used to store the external message repository  */
/******************************************************************************/
#define REXXMESSAGEFILE    "winatab.rc"


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
/* OPTIONAL:  Overrides for any functions defined in sysdef.h.  These         */
/* can map the calls directly to inline code or comment them out all together.*/
/******************************************************************************/

#define DEFRXSTRING 256                 /* Default RXSTRING return size      */

#define SysThreadYield()

#define SysAllocateHeap();             // no shared memory on windows yet
#define SysReleaseHeap();              // no shared memory on windows yet

extern bool HandleException;
                                       // our signal handling
inline void SysRegisterSignals(SYSEXCEPTIONBLOCK *e) { HandleException = true; }
inline void SysDeregisterSignals(SYSEXCEPTIONBLOCK *e) { HandleException = false; }

inline void SysRegisterExceptions(SYSEXCEPTIONBLOCK *e) { ; }
inline void SysDeregisterExceptions(SYSEXCEPTIONBLOCK *e) { ; }
                                       // in Windows, no EA's
#define SysClauseBoundary(a)

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

 #define WinBeginExceptions SetConsoleCtrlHandler(&WinConsoleCtrlHandler, true);\
                            __try {
 #define WinEndExceptions } __except ( WinExceptionFilter(GetExceptionCode( ))) {  }\
                            SetConsoleCtrlHandler(&WinConsoleCtrlHandler, FALSE);

 extern bool HandleException;
 int WinExceptionFilter( int xCode );


// Options character used for command line programs
#define OPT_CHAR  '/'                  /* Option char for win                */

#define  OLDMETAVERSION       29       /* highest meta version number:                  */


#endif

