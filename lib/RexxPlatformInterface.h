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
/* REXX Library                                                 sysdef.h      */
/*                                                                            */
/* Prototypes for system specific functions called from system independent    */
/* code.  These prototypes will only be declared if the system specific macros*/
/* have not overridden these with a define.                                   */
/*                                                                            */
/******************************************************************************/
#ifndef SYSDEF_H
#define SYSDEF_H

#ifndef SysGetCurrentTime
void SysGetCurrentTime(REXXDATETIME *);/* get the current time              */
#endif

#ifndef SysVariablePool
                                       /* process external vpool requests   */
extern ULONG SysVariablePool(RexxNativeActivation *, PVOID, BOOL);
#endif

#ifndef SysResolveProgramName
                                       /* resolve rexx program file names   */
RexxString *SysResolveProgramName(RexxString *, RexxString *);
#endif

#ifndef SysRelinquish
void SysRelinquish(void);              /* allow the system to run           */
#endif

#ifndef SysSetThreadPriority
#ifdef THREADHANDLE
void SysSetThreadPriority(long, HANDLE, int);  /* give a thread priority            */
#else
void SysSetThreadPriority(long, int);  /* give a thread priority            */
#endif
#endif

#ifndef SysRegisterExceptions
                                       /* setup for exceptions              */
void SysRegisterExceptions(SYSEXCEPTIONBLOCK *);
#endif

#ifndef SysDeregisterExceptions
                                       /* clear exception handlers          */
void SysDeregisterExceptions(SYSEXCEPTIONBLOCK *);
#endif

#ifndef SysRegisterSignals
                                       /* register a signal handler         */
void SysRegisterSignals(SYSEXCEPTIONBLOCK *);
#endif

#ifndef SysDeregisterSignals
                                       /* deregister a signal handler       */
void SysDeregisterSignals(SYSEXCEPTIONBLOCK *);
#endif

#ifndef SysProcessName
RexxObject *SysProcessName(void);      /* get a process "name" object       */
#endif

#ifndef SysTermination
void SysTermination(void);             /* perform system specific cleanup   */
#endif

#ifndef SysInitialize
void SysInitialize(void);              /* perform system initialization     */
#endif

#ifndef SysName
RexxString *SysName(void);             /* get the system name               */
#endif

#ifndef SysVersion
RexxString *SysVersion(void);          /* get the system version            */
#endif

#ifndef SysValue
                                       /* system VALUE() builtin function   */
RexxObject *SysValue(RexxString *, RexxObject *, RexxString *);
#endif

#ifndef SysUserid
                                       /* system USERID() builtin function   */
RexxObject *SysUserid();
#endif

#ifndef SysAllocateResultMemory
PVOID SysAllocateResultMemory(LONG);   /* allocate a result memory block    */
#endif

#ifndef SysReleaseResultMemory
void SysReleaseResultMemory(PVOID);    /* release a result memory block     */
#endif

#ifndef SysExternalFunction
                                       /* call an external function         */
RexxObject * SysExternalFunction(RexxActivation *, RexxActivity *, RexxString *, RexxString *, RexxObject **, size_t, RexxString *, BOOL *);
#endif

#ifndef SysGetMacroCode
                                       /* load a method from a macro        */
RexxMethod * SysGetMacroCode(RexxString *);
#endif

#ifndef SysCommand
                                       /* invoke a command                  */
RexxObject * SysCommand(RexxActivation *, RexxActivity *, RexxString *, RexxString *, RexxString **);
#endif

#ifndef SysExitHandler
                                       /* invoke an exit                    */
BOOL SysExitHandler(RexxActivity *, RexxActivation *, RexxString *, LONG, LONG, PVOID, BOOL);
#endif

#ifndef SysThreadYield
void SysThreadYield(void);             /* yield thread control              */
#endif

#ifndef SysQueryThreadID
INT SysQueryThreadID(void);            /* query the current thread          */
#endif

#ifndef SysGetThreadStackBase
PCHAR SysGetThreadStackBase(INT);     /* query current thread stack start  */
#endif

#ifndef SysCreateThread
                                       /* create a new thread               */
INT SysCreateThread (PTHREADFN, INT, PVOID);
#endif

#ifndef SysLoadProcedure
                                       /* load a named procedure            */
PFN SysLoadProcedure (RexxInteger *, RexxString *);
#endif

#ifndef SysLoadLibrary
                                       /* load a named library              */
RexxInteger * SysLoadLibrary (RexxString *);
#endif

#ifndef SysValidateAddressName
                                       /* validate an address environment   */
void SysValidateAddressName(RexxString *);
#endif

#ifndef SysMessageText
RexxString *SysMessageText (INT);      /* retrieve an error message         */
#endif

#ifndef SysMessageHeader
RexxString *SysMessageHeader (INT);    /* get the header for an error msg   */
#endif

#ifndef SysReadProgram
RexxBuffer *SysReadProgram (PCHAR);    /* read a program into storage       */
#endif

#ifndef SysInitializeWindowEnv
                                       /* Initialize a window environemnt   */
SYSWINDOWINFO * SysInitializeWindowEnv(void);
#endif

#ifndef SysTerminateWindowEnv
                                       /* Termiante  a window environemnt   */
void SysTerminateWindowEnv(SYSWINDOWINFO *);
#endif

#ifndef SysGetCurrentQueue
RexxString *SysGetCurrentQueue(void);  /* Get the current queue name        */
#endif

#ifndef SysSetupProgram
void SysSetupProgram(RexxActivation *);/* System specific program setup     */
#endif

#ifndef SysRestoreProgram
                                       /* Restore a program image           */
RexxMethod *SysRestoreProgram(RexxString *);
#endif

#ifndef SysSaveProgram
                                       /* Save a program image              */
void SysSaveProgram(RexxString *, RexxMethod *);
#endif

#ifndef SysSourceString
                                       /* Create the source string          */
RexxString *SysSourceString(RexxString *, RexxString *);
#endif

#ifndef SysInitialAddressName
RexxString *SysInitialAddressName();   /* get the initial address name      */
#endif

#ifndef SysQualifyFileSystemName
                                       /* Qualify a file name               */
RexxString *SysQualifyFileSystemName(RexxString *);
#endif

#ifndef SysClauseBoundary
                                       /* Do system clause boundary stuff   */
void SysClauseBoundary(RexxActivation *);
#endif

#ifndef SysGetTempFileName
                                       /* Get a temporary file name for     */
                                       /* debug dumps                       */
PCHAR SysGetTempFileName(void);
#endif

#ifndef SysLoadImage
void SysLoadImage(char **, long *);    /* load the image file               */
#endif

#ifndef SysTerminateThread
                                       /* thread being terminated           */
void SysTerminateThread(TID threadid);
#endif

#ifndef SysIsThreadEqual
#define SysIsThreadEqual(t1, t2) (((long)(t1)) == ((long)(t2)))
#endif

#ifndef SysInitializeThread
                                       /* thread being started              */
void SysInitializeThread();
#endif

/* defect 2325: CHM added definition of PTRSUB2 for Doug Griswold           */
#ifndef PTRSUB2
#define PTRSUB2   PTRSUB
#endif

/******************************************************************************/
/* Priority values used for adjusting thead priorities                        */
/******************************************************************************/


#ifndef LOW_PRIORITY
#define LOW_PRIORITY    0
#endif

#ifndef MEDIUM_PRIORITY
#define MEDIUM_PRIORITY 100
#endif

#ifndef HIGH_PRIORITY
#define HIGH_PRIORITY   200
#endif

#endif
