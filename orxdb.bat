@REM
@REM Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.
@REM Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.
@REM
@REM This program and the accompanying materials are made available under
@REM the terms of the Common Public License v1.0 which accompanies this
@REM distribution. A copy is also available at the following address:
@REM http://www.oorexx.org/license.html
@REM
@REM Redistribution and use in source and binary forms, with or
@REM without modification, are permitted provided that the following
@REM conditions are met:
@REM
@REM Redistributions of source code must retain the above copyright
@REM notice, this list of conditions and the following disclaimer.
@REM Redistributions in binary form must reproduce the above copyright
@REM notice, this list of conditions and the following disclaimer in
@REM the documentation and/or other materials provided with the distribution.
@REM
@REM Neither the name of Rexx Language Association nor the names
@REM of its contributors may be used to endorse or promote products
@REM derived from this software without specific prior written permission.
@REM
@REM THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
@REM "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
@REM LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
@REM FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
@REM OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
@REM SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
@REM TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
@REM OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
@REM OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
@REM NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
@REM SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
@REM
@echo off
SETLOCAL
IF %MKASM%x == x GOTO HELP_MKASM
REM The newer versions changed the name of the vars setup.  We require this
REM to be done before attempting the build now.
REM set variables for 32 bit compiler
REM if (%OR_WIN32%)==(1) goto novars
REM   call vcvars32 ix86
REM   set OR_WIN32=1

:novars
REM
REM set up the drive for the source files
REM
IF %SRC_DRV%x == x GOTO HELP_SRC_DRV
REM
REM set up the drive for the source files
REM
REM set SRC_DIR=\OrxDev_OSS


REM set the registry key for version check
REM regedit /s %SRC_DRV%%SRC_DIR%\full.reg
REM
REM set up the directories for the generated files
REM
REM set OR_OUTDIR=O:\TESTDIR
if (%1)==(1) goto release
set OR_OUTDIR=%SRC_DRV%%SRC_DIR%\Win32Dbg
set OR_ERRLOG=%OR_OUTDIR%\Win32Dbg.Log
goto cont
:release
set OR_OUTDIR=%SRC_DRV%%SRC_DIR%\Win32Rel
set OR_ERRLOG=%OR_OUTDIR%\Win32Rel.Log
:cont
REM
REM set up the directories for the source files
REM
set OR_ORYXRSRC=%SRC_DRV%%SRC_DIR%\rexutils
REM MHES 29122004 - added OR_ORYXRSRCW needed for winsystm.cls
set OR_ORYXRSRCW=%SRC_DRV%%SRC_DIR%\rexutils\windows
set XPLATFORM=%OR_ORYXRSRC%\windows
set OR_ORYXSRC=%SRC_DRV%%SRC_DIR%
set OR_ORYXLSRC=%SRC_DRV%%SRC_DIR%\lib
set OR_ORYXAPI=%SRC_DRV%%SRC_DIR%\api
set OR_ORYXAPIWIN=%SRC_DRV%%SRC_DIR%\api\platform\windows
set OR_ORYXKSRC=%SRC_DRV%%SRC_DIR%\kernel
set OR_ORYXMSRC=%SRC_DRV%%SRC_DIR%\kernel\messages
set OR_ORYXWSRC=%SRC_DRV%%SRC_DIR%\platform\windows
set OR_ORYXASRC=%SRC_DRV%%SRC_DIR%\rexxapi
set OR_ORYXAWSRC=%SRC_DRV%%SRC_DIR%\rexxapi\windows
set OR_ORYXOODSRC=%SRC_DRV%%SRC_DIR%\platform\windows\oodialog
set OR_ORYXOLESRC=%SRC_DRV%%SRC_DIR%\platform\windows\ole
set OR_ORYXAXSCRIPT=%SRC_DRV%%SRC_DIR%\platform\windows\orxscrpt
set OR_ORYXREGEXP=%SRC_DRV%%SRC_DIR%\rxregexp
REM MHES 31122004 - added API and OODialog samples
set OR_ORYXSAMPLES=%SRC_DRV%%SRC_DIR%\samples
set OR_ORYXAPISAMPLES=%SRC_DRV%%SRC_DIR%\samples\windows\api
set OR_ORYXOODIALOGSAMPLES=%SRC_DRV%%SRC_DIR%\samples\windows\oodialog

set KERNEL=%OR_ORYXKSRC%
set KCLASSES=%KERNEL%\classes
set KMESSAGES=%KERNEL%\messages
set KCORE=%KERNEL%\runtime
set KAPI=%KERNEL%\api
set KEXPR=%KERNEL%\expression
set KINST=%KERNEL%\instructions
set KMAIN=%KERNEL%\main
set KNATIVE=%KERNEL%\native
set KEXTRAS=%KERNEL%\extras
set KREXX=%KERNEL%\RexxClasses
set KPARSER=%KERNEL%\parser
set KPLATFORM=%KERNEL%\platform\common
set KWINDOWS=%KERNEL%\platform\windows
set KSTREAM=%KERNEL%\streamLibrary

set APLATFORM=%OR_ORYXASRC%\windows

set OR_KERNELPATH=%KERNEL%;%KCLASSES%;%KCORE%;%KEXPR%;%KINST%;%KMAIN%;%KNATIVE%;%KEXTRAS%;%KPARSER%;%KPLATFORM%;%KWINDOWS%;%KSTREAM%;%KMESSAGES
set OR_KERNELINCL=-I%KERNEL%\ -I%KCLASSES%\ -I%KCORE%\ -I%KEXPR%\ -I%KINST%\ -I%KMAIN%\ -I%KNATIVE%\ -I%KEXTRAS%\ -I%KPARSER%\ -I%KPLATFORM%\ -I%KWINDOWS%\ -I%KSTREAM%\ -I%KMESSAGES%\
REM
REM set up the directory search orders for the source include files
REM
set OR_ORYXINCL=-I%OR_ORYXLSRC%\ -I%OR_ORYXAPI%\ -I%OR_ORYXAPIWIN%\ %OR_KERNELINCL% -I%OR_ORYXWSRC%\ -I%OR_ORYXASRC%\ -I%APLATFORM% -I%XPLATFORM% -I%OR_ORYXOODSRC%\ -I%OR_ORYXFSRC%\ -I%OR_ORYXOLESRC%\ -I%OR_ORYXAXSCRIPT%\ -I%OR_ORYXMSRC%\
set OR_ORYXRCINCL=-I%KMESSAGES%
REM
REM set up the search order for the dependency list
REM
set OR_SP={%OR_KERNELPATH%;%OR_ORYXLSRC%;%OR_ORYXAPI;%OR_ORYXAPIWIN;%OR_ORYXWSRC%;%OR_ORYXASRC%;}
REM
REM set up the search order for the api samples
REM
set SAMPLEPATH=%OR_ORYXAPI%;%OR_ORYXAPIWIN%
REM
REM set up the windows link flag to indicate that link debug info is wanted
REM
set NODEBUG=%1
set OPTIMIZE=%1
set CPLUSPLUS=1
set NOCRTDLL=1
set REXXDEBUG=0

REM create output directory
if not exist %OR_OUTDIR% md %OR_OUTDIR%

IF NOT %MKASM%==1 GOTO :build
if not exist %OR_OUTDIR%\ASM md %OR_OUTDIR%\ASM

:build
REM Call build program
call %SRC_DRV%%SRC_DIR%\platform\windows\buildorx
if ERRORLEVEL 1 goto error

cd %OR_OUTDIR%

:CONTINUE
cd %SRC_DIR%

goto END

:HELP_MKASM
ECHO *======================================================
ECHO The environment variable MKASM is not set
ECHO Set the variable to 0 - create no assembler listings
ECHO                     1 - create assembler listings
ECHO e.g. "SET MKASM=0"
ECHO *======================================================

goto END

:HELP_SRC_DRV
ECHO *======================================================
ECHO The environment variable SRC_DRV is not set
ECHO Set the variable to the build directory drive letter
ECHO e.g. "SET SRC_DRV=F:"
ECHO *======================================================

goto END

:error
ENDLOCAL
exit /b 1

:END
ENDLOCAL
