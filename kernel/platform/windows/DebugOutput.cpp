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
/****************************************************************************/
/* Object REXX Kernel                                             windbg.c  */
/*                                                                          */
/* Windows debugging routines for Object REXX                               */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*  External entry functions:                                               */
/*      DbgPrintf - log a single line to log file                           */
/*                                                                          */
/****************************************************************************/
#include <stdarg.h>
#include <string.h>
#include <process.h>
#include "RexxCore.h"

static bool fWriteDebug = true;

// TODO:  Move this out of kernel into the OLE support where it's used.

VOID SysCall DbgPrintf(char * pszDbgArgs, ...)
{
   char debugHeader[50];
   char debugFileName[255];
   char outBuffer[255];
   SYSTEMTIME st;
   HANDLE dbgFile;
   DWORD dwWritten;
   va_list argPtr;

   /* find output filename */
   if (GetEnvironmentVariable("REXX_DEBUG_LOG", debugFileName, sizeof(debugFileName)) == 0)
   {
      strcpy( debugFileName, "C:\\REXXDBG.LOG");
   } /* endif */

   /* write debug info only if desired */
   if ( (stricmp( debugFileName, "NO" ) != 0) && fWriteDebug )
   {
      dbgFile = CreateFile(debugFileName, GENERIC_WRITE, 0, NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                           NULL);
      if (dbgFile != INVALID_HANDLE_VALUE)
      {
        SetFilePointer(dbgFile, 0, NULL, FILE_END);

        /* Create time stamp and process information */
        GetSystemTime(&st);
        sprintf(debugHeader, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d PID %4.4X - ",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
                st.wMilliseconds, _getpid());
        WriteFile(dbgFile, debugHeader, strlen(debugHeader), &dwWritten, NULL);

        va_start(argPtr, pszDbgArgs);
        vsprintf(outBuffer, pszDbgArgs, argPtr);
        va_end(argPtr);

        WriteFile(dbgFile, outBuffer, strlen(outBuffer), &dwWritten, NULL);
        CloseHandle( dbgFile );
     } /* endif */
   } /* endif */
} /* end of function DbgPrintf */
