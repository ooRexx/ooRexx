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
/* REXX AIX/Linux Support                                          aixrexx.h  */
/*                                                                            */
/* AIX/Linux master definition file                                           */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* Template for system specific declarations for Object REXX.  The following  */
/* sections are a series of required and optional sections where system       */
/* specific information is provided to the rest of the interpreter code.      */
/* Wherever possible, reasonable defaults are provided for these settings.    */
/******************************************************************************/

#ifndef AIXREXX_H                      /* provide a define here to protect  */
#define AIXREXX_H                      /* against multiple includes         */

/******************************************************************************/
/* REQUIRED:  The implemenation must decide on the C_STACK_SIZE defining      */
/* constants that are compiler/linker dependent.                              */
/******************************************************************************/
#define MIN_C_STACK 1024*16
#define TOTAL_STACK_SIZE 1024*512
#define C_STACK_SIZE TOTAL_STACK_SIZE

// No definition needed on Linux systems.
#define SysCall

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef LINUX
#define FNONBLOCK       O_NONBLOCK
#endif


#define RXTRACE_SUPPORT

/******************************************************************************/
/* OPTIONAL:  Perform stack checking on new message invocations.  If this type*/
/* of information is not available, then do not include this define           */
/******************************************************************************/

#define STACKCHECK

/******************************************************************************/
/* OPTIONAL:  No time slicing support for REXX activation yields is available.*/
/* Code will yield after a given count of instructions.                       */
/* This option is mutually exclusive with TIMESLICE.                          */
/******************************************************************************/

#define NOTIMER

/******************************************************************************/
/* OPTIONAL:  Enable concurrency timeslice dispatching support.  Default is   */
/* only yield at specific event points.                                       */
/******************************************************************************/

/* #define TIMESLICE  */

/******************************************************************************/
/* OPTIONAL:  If the implementation enables external scripting support, then  */
/* additional hand-shaking with an the exernal environment is enabled for     */
/* providing default values for uninitialized variables.                      */
/******************************************************************************/
//#define SCRIPTING

/******************************************************************************/
/* REQUIRED:  Define the REXX type for semaphores.  These can be system       */
/* specific semaphore types or the REXX define OSEM.                          */
/******************************************************************************/

#define _POSIX_THREADS_

#ifdef OPSYS_AIX41
#define SysThreadYield()   pthread_yield()
#else
#define SysThreadYield()   sched_yield()
#endif
#include "ThreadSupport.hpp"
#include "SystemSemaphores.h"

#define KMTX int               /* kernel semaphore ID               */
#define SMTX RexxMutex *       /* semaphore data types              */
#define HMTX SMTX
#define SEV  RexxSemaphore *
                             // semaphore definitions and init
extern int SecureFlag;


#define SysSharedSemaphoreDefn  \
                               SMTX  rexx_wait_queue_semaphore = 0; \
                               SEV   rexxTimeSliceSemaphore = 0;    \
                               size_t rexxTimeSliceTimerOwner;
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

typedef void *(* PTHREADFN)(void *);    /* define a thread function          */

/******************************************************************************/
/* REQUIRED:  Definitions for REXX semaphore functions.  These default to     */
/* the REXX library semaphore package, but can be redefined to map directly   */
/* to system specific functions too.                                          */
// If you are using the OREXX Thread package, look at winrexx.h for correct
// setup of these macros.
/******************************************************************************/

#define MTXCR(s)      s = new RexxMutex// create a mutex semaphore
#define MTXCROPEN(s,n) s = new RexxMutex // dummy for Windows
                                       // request wait on a semaphore
#define MTXRQ(s)      s->request()
#define MTXRL(s)      s->release()     // clear a semaphore
/* Mutex semaphores are in AIX defined as C++ Object pointers and          */
/* therefore must be deleted after use                                     */
#define MTXCL(s)      delete s
//#define MTXCL(s)           // no need to close in thread package
                                       // no wait, return if can't get it
#define MTXRI(s)      s->requestImmediate()
#define MTXNOPEN(s,n)                  // only used for shared stuff
#define MTXNCR(s,n)   MTXCR(s)
#define EVCR(s)       s = new RexxSemaphore
#define EVCROPEN(s,n) s = new RexxSemaphore // dummy for Windows
#define EVPOST(s)     s->post()
#define EVSET(s)      s->reset()
#define EVWAIT(s)     s->wait()
#define EVCL(s)
//#define EVCLOSE(s)
/* Event semaphores are in AIX defined as C++ Object pointers and          */
/* therefore must be deleted after use                                     */
#define EVCLOSE(s)    delete s
#define EVOPEN(s)
//#define EVCLEAR(s)    s->reset()     // commented out by weigold
#define EVCLEAR(s)    s=0              // and inserted new line
#define EVEXIST(s)

/******************************************************************************/
/* REQUIRED:  Define a routine to determine if an activation needs to yield.  */
/*   Use whatever mechanism is approcpriate for given System.                 */
/*   OS/2 uses the DosStartTimer, continious Asynch Timer.                    */
/******************************************************************************/

bool SysTimeSliceElapsed();

/******************************************************************************/
/* REQUIRED:  Routine to start a new TimeSlice period.                        */
/******************************************************************************/

void SysStartTimeSlice( void );

/******************************************************************************/
/* REQUIRED:  Routine to alloc memory passed to external environments         */
/******************************************************************************/

#define SysAllocateExternalMemory(s) malloc((s))
#define SysFreeExternalMemory(p) free((p))

/******************************************************************************/
/* REQUIRED:  Define the string used for the default initial address setting. */
/******************************************************************************/

#if defined(AIX)
#define SYSINITIALADDRESS "ksh"
#elif defined(OPSYS_SUN)
#define SYSINITIALADDRESS "sh"
#else
#define SYSINITIALADDRESS "bash"
#endif


#ifdef __APPLE__
# include <crt_externs.h>
inline char **getEnvironment()
{
     return (*_NSGetEnviron());
}
#else
inline char **getEnvironment()
{
   extern char **environ;
   return environ;
}
#endif

/******************************************************************************/
/* REQUIRED:  Define the macro to call the line_write_function                */
/******************************************************************************/

#define line_write(b,c) fwrite(b,1,c,stream_info->stream_file)

/******************************************************************************/
/* REQUIRED:  Define the name of the image file that is saved and restored.   */
/******************************************************************************/

#define BASEIMAGE     "rexx.img"

/******************************************************************************/
/* REQUIRED:  Define the name of the program called to load create the saved  */
/* image file.                                                                */
/******************************************************************************/

#define BASEIMAGELOAD "CoreClasses.orx" /* MHES 29122004 */

/******************************************************************************/
/* REQUIRED:  Define a name for the initialization semaphore.  If not required*/
/* for your system, just make this any string.                                */
/******************************************************************************/

#define INIT_SEM_NAME "INIT_SEM"

/******************************************************************************/
/* Priority values used for adjusting thead priorities                        */
/******************************************************************************/


#ifdef AIX
#define HIGH_PRIORITY   127
#endif

/******************************************************************************/
/* REQUIRED:  Name of the file used to store the external message repository  */
/******************************************************************************/
#define REXXMESSAGEFILE    "rexx.cat"

/******************************************************************************/
/* REQUIRED:  Define any additional native methods that are to be accessed as */
/* external REXX methods.                                                     */
/******************************************************************************/

#define SYSTEM_INTERNAL_METHODS() \
   INTERNAL_METHOD(sysRxfuncadd) \
   INTERNAL_METHOD(sysRxfuncdrop) \
   INTERNAL_METHOD(sysRxfuncquery) \
   INTERNAL_METHOD(sysBeep) \
   INTERNAL_METHOD(sysFilespec) \
   INTERNAL_METHOD(sysDirectory) \
   INTERNAL_METHOD(sysSetLocal) \
   INTERNAL_METHOD(sysEndLocal) \
   INTERNAL_METHOD(function_queueExit)

/******************************************************************************/
/* REQUIRED:  Definitions for stream I/O.  Each should be tailored to the     */
/* needs/quirks of the platform file systems.                                 */
/* The following lines are examples from the OS/2 code.                       */
/******************************************************************************/

#ifdef INCL_REXX_STREAM                /* asking for file system stuff?     */
/******************************************************************************/
/* REQUIRED:  include whatever library system specific include files are      */
/* needed.  If it is necessary to override any standard routines, do this     */
/* here.                                                                      */
/******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#if defined(AIX)
#include <usersec.h>
#else
#include <pwd.h>
#endif

/******************************************************************************/
/* REQUIRED:  define the path delimiter and end of line information           */
/* appropriate for this file system.                                          */
/******************************************************************************/

#define delimiter '/'
#define line_end "\n"
#define line_end_size 1

/* adjust scan pointer */
//#define adjust_scan_at_line_end()
#endif


#if defined(AIX) || defined(LINUX)
  #define  OLDMETAVERSION       22     /* highest meta version number:                  */
#else
  #define  OLDMETAVERSION       30     // each platform should have it's own old meta version
#endif

#define OPT_CHAR  '-'                  /* Option char for UNIX               */

/******************************************************************************/
/* OPTIONAL:  Overrides for any functions defined in sysdef.h.  These         */
/* can map the calls directly to inline code or comment them out all together.*/
/* The following lines are examples from the OS/2 code.                       */
/******************************************************************************/

#define SysRelinquish()

#if defined(AIX)
#define SysName() new_string("AIX", 3)
#define SysINTName() new_string("AIX",3)
#elif defined(OPSYS_SUN)
#define SysName() new_string("SUNOS", 5)
#define SysINTName() new_string("SUNOS",5)
#else
#define SysName() new_string("LINUX", 5)
#define SysINTName() new_string("LINUX",5)
                                       // Thread yielding functions in
                                       // threading package
#endif

#ifdef AIX
#define SysIsThreadEqual(pth1, pth2) (pthread_equal(pth1, pth2))
#endif

#define SysThreadArg(a) a->args
#define SysInitialAddressName() OREF_INITIALADDRESS

#define SysGetTempFileName()  tmpnam(NULL)

/******************************************************************************/
/* OPTIONAL:  Finally, any other global defined constants for system specific */
/* code usage.                                                                */
/******************************************************************************/

#define DEFRXSTRING 256                /* Default RXSTRING return size      */

#ifdef __cplusplus
extern "C" {
#endif
char * APIENTRY RexxGetVersionInformation();
#ifdef SEMAPHORE_DEBUG
void SysRequestMutexSem (SMTX psem);   /* request a mutex semaphore         */
                                       /* request a mutex (immediate return)*/
int SysRequestImmediateMutexSem (SMTX psem);
#define REXXTIMESLICE 100              /* 100 milliseconds (1/10 second)    */
#endif
                                       /* moved from olcrtmis.h             */
#define stricmp(s1, s2) strcasecmp(s1, s2)
                                       /* both functions can only be used   */
                                       /* without a return value & radix=10 */
#define _ultoa(val, str, radix)  sprintf(str, "%u", val)

#include "APIDefinitions.h"

#ifdef __cplusplus
}
#endif

#endif
