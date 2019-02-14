/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* list of rexxutil system-specific functions additions                       */
/*                                                                            */
/******************************************************************************/

    INTERNAL_ROUTINE(SysCls,                      SysCls)
    INTERNAL_ROUTINE(SysCurPos,                   SysCurPos)
    INTERNAL_ROUTINE(SysCurState,                 SysCurState)
    INTERNAL_ROUTINE(SysDriveInfo,                SysDriveInfo)
    INTERNAL_ROUTINE(SysDriveMap,                 SysDriveMap)
    INTERNAL_ROUTINE(SysGetKey,                   SysGetKey)
    INTERNAL_ROUTINE(SysIni,                      SysIni)
    INTERNAL_ROUTINE(SysMkDir,                    SysMkDir)
    INTERNAL_ROUTINE(SysWinVer,                   SysWinVer)
    INTERNAL_ROUTINE(SysVersion,                  SysWinVer)
    INTERNAL_ROUTINE(SysTextScreenRead,           SysTextScreenRead)
    INTERNAL_ROUTINE(SysTextScreenSize,           SysTextScreenSize)
    INTERNAL_ROUTINE(SysBootDrive,                SysBootDrive)
    INTERNAL_ROUTINE(SysSystemDirectory,          SysSystemDirectory)
    INTERNAL_ROUTINE(SysFileSystemType,           SysFileSystemType)
    INTERNAL_ROUTINE(SysVolumeLabel,              SysVolumeLabel)
    INTERNAL_ROUTINE(SysCreateMutexSem,           SysCreateMutexSem)
    INTERNAL_ROUTINE(SysOpenMutexSem,             SysOpenMutexSem)
    INTERNAL_ROUTINE(SysCloseMutexSem,            SysCloseMutexSem)
    INTERNAL_ROUTINE(SysRequestMutexSem,          SysRequestMutexSem)
    INTERNAL_ROUTINE(SysReleaseMutexSem,          SysReleaseMutexSem)
    INTERNAL_ROUTINE(SysCreateEventSem,           SysCreateEventSem)
    INTERNAL_ROUTINE(SysOpenEventSem,             SysOpenEventSem)
    INTERNAL_ROUTINE(SysCloseEventSem,            SysCloseEventSem)
    INTERNAL_ROUTINE(SysResetEventSem,            SysResetEventSem)
    INTERNAL_ROUTINE(SysPostEventSem,             SysPostEventSem)
    INTERNAL_ROUTINE(SysPulseEventSem,            SysPulseEventSem)
    INTERNAL_ROUTINE(SysWaitEventSem,             SysWaitEventSem)
    INTERNAL_ROUTINE(SysSetPriority,              SysSetPriority)
    INTERNAL_ROUTINE(SysSwitchSession,            SysSwitchSession)
    INTERNAL_ROUTINE(SysWaitNamedPipe,            SysWaitNamedPipe)
    INTERNAL_ROUTINE(SysQueryProcess,             SysQueryProcess)
    INTERNAL_ROUTINE(SysSetFileDateTime,          SysSetFileDateTime)
    INTERNAL_ROUTINE(SysGetFileDateTime,          SysGetFileDateTime)
    INTERNAL_ROUTINE(RxWinExec,                   RxWinExec)
    INTERNAL_ROUTINE(SysWinEncryptFile,           SysWinEncryptFile)
    INTERNAL_ROUTINE(SysWinDecryptFile,           SysWinDecryptFile)
    INTERNAL_ROUTINE(SysGetErrorText,             SysGetErrorText)
    INTERNAL_ROUTINE(SysFromUniCode,              SysFromUniCode)
    INTERNAL_ROUTINE(SysToUniCode,                SysToUniCode)
    INTERNAL_ROUTINE(SysWinGetPrinters,           SysWinGetPrinters)
    INTERNAL_ROUTINE(SysWinGetDefaultPrinter,     SysWinGetDefaultPrinter)
    INTERNAL_ROUTINE(SysWinSetDefaultPrinter,     SysWinSetDefaultPrinter)

    INTERNAL_ROUTINE(SysShutDownSystem,           SysShutDownSystem)
    INTERNAL_ROUTINE(SysIsFileCompressed,         SysIsFileCompressed)
    INTERNAL_ROUTINE(SysIsFileEncrypted,          SysIsFileEncrypted)
    INTERNAL_ROUTINE(SysIsFileNotContentIndexed,  SysIsFileNotContentIndexed)
    INTERNAL_ROUTINE(SysIsFileOffline,            SysIsFileOffline)
    INTERNAL_ROUTINE(SysIsFileSparse,             SysIsFileSparse)
    INTERNAL_ROUTINE(SysIsFileTemporary,          SysIsFileTemporary)
    INTERNAL_ROUTINE(SysGetLongPathName,          SysGetLongPathName)
    INTERNAL_ROUTINE(SysGetShortPathName,         SysGetShortPathName)
