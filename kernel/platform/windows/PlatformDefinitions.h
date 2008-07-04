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

void SysRelinquish(void);              /* allow the system to run           */

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

