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
/* REQUIRED:  Define the name of the image file that is saved and restored.   */
/******************************************************************************/

#define BASEIMAGE     "rexx.img"

/******************************************************************************/
/* REQUIRED:  Define the name of the program called to load create the saved  */
/* image file.                                                                */
/******************************************************************************/

#define BASEIMAGELOAD "CoreClasses.orx" /* MHES 29122004 */

/******************************************************************************/
/* REQUIRED:  Name of the file used to store the external message repository  */
/******************************************************************************/
#define REXXMESSAGEFILE    "rexx.cat"

/******************************************************************************/
/* REQUIRED:  define the path delimiter and end of line information           */
/* appropriate for this file system.                                          */
/******************************************************************************/

#define delimiter '/'
#define line_end "\n"
#define line_end_size 1

/******************************************************************************/
/* OPTIONAL:  Finally, any other global defined constants for system specific */
/* code usage.                                                                */
/******************************************************************************/

#define DEFRXSTRING 256                /* Default RXSTRING return size      */

#ifdef __cplusplus
extern "C" {
#endif

#define REXXTIMESLICE 100              /* 100 milliseconds (1/10 second)    */

#include "APIDefinitions.h"

#ifdef __cplusplus
}
#endif

/*
 * OPTIONAL: For EBCDIC systems.
 */
#if defined(__MVS__)
   #define EBCDIC
#endif

#endif
