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
/* Windows types and utility macros included in:                              */
/*  REXXSAA.h, winrexx.h, winapi.h                                            */
/******************************************************************************/
#ifndef WINTYPES_H                       /* provide a define here to protect  */
#define WINTYPES_H                       /* against multiple includes         */


#include <windows.h>                  // required for windows apps

 #define WIN32S  0x800000000         // high bit set if running WIN32S on 3.1

 extern int which_system_is_running();

 #define RUNNING_WIN31 (!which_system_is_running()) // returns true only for Windows 3.1
 #define RUNNING_95 (which_system_is_running()==2)  // returns true only for Windows 95
 #define RUNNING_NT (which_system_is_running())     // returns true for Windows NT and Windows 95

 // Misc types
 #ifdef __cplusplus
extern "C" {
#endif

typedef ULONG (APIENTRY *PFN)();                  // PFN pointer to a function
                                         // This is also in REXXSAA.H
 //typedef int * PFN()                   // OS2 defn from toolkit
#ifdef __cplusplus
}
#endif

 // Portable ptr subtraction
 #define CCHMAXPATH MAX_PATH              // used in os2file

  // These are also defined in Winrexx.h!
  typedef DWORD PID;                     // Used in some prototypes. Needs
  typedef DWORD TID;                     // to be changed when modules complete

//  #define _Packed                        // win32 undefined
//  #define APIRET ULONG                   // as defined in OS/2

#endif




