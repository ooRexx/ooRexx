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
/* REXX Library                                                 oldata.c      */
/*                                                                            */
/* Local process data                                                         */
/*                                                                            */
/******************************************************************************/
#include <stdio.h>
#include "RexxLibrary.h"
                                       /* local SOM/proxy server            */
RexxObject * ProcessLocalServer = OREF_NULL;
int   ProcessBusyActs = 0;             /* number of busy activities         */
int   ProcessInitializations = 0;      /* number of active initializations  */
BOOL  ProcessColdStart = TRUE;         /* performing a coldstart            */
BOOL  ProcessDoneInit = FALSE;         /* initialization is done            */
BOOL  ProcessDoneTerm = FALSE;         /* termination is done               */
BOOL  ProcessFirstThread = TRUE;       /* this is the first thread          */
int   ProcessNumActs = 0;              /* number of activities created      */
                                       /* array of local activities         */
#ifdef HIGHTID
ActivityTable * ProcessLocalActs = OREF_NULL;
#else
RexxArray * ProcessLocalActs = OREF_NULL;
#endif

BOOL  ProcessTerminating = FALSE;      /* termination in process            */
                                       /* local environment directory       */
RexxDirectory * ProcessLocalEnv = OREF_NULL;
BOOL  ProcessRestoreImage = TRUE;      /* restoring the saved image         */
BOOL  ProcessSaveImage = FALSE;        /* saving the image                  */
ULONG ProcessMustCompleteNest = 0;     /* Global variable for MustComplete  */
                                       /* Next line added be THU            */
BOOL  RexxStartedByApplication = TRUE; /* is REXX started by system or appl */
#ifdef WIN32
extern SEV RexxTerminated = NULL;      /* Semaphore to be posted at shutdown*/
extern SEV RexxServerWait = NULL;      /* Semaphore for Sever waiting on msg*/
#else
SEV   RexxTerminated;                  /* Semaphore to be posted at shutdown*/
SEV   RexxServerWait;                  /* Semaphore for Sever waiting on msg*/
#endif
RexxInteger * ProcessName = OREF_NULL; /* Process name/id                   */
ULONG RexxTimeSliceTimer;              /* Time Slice timer handle.          */
BOOL rexxTimeSliceElapsed = FALSE;     /* the time slice interlock flag     */
                                       /* Most currently accessed pool      */

MemorySegmentPool *ProcessCurrentPool = NULL;
