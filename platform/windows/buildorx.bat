@REM /*----------------------------------------------------------------------------*/
@REM /*                                                                            */
@REM /* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
@REM /* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.         */
@REM /*                                                                            */
@REM /* This program and the accompanying materials are made available under       */
@REM /* the terms of the Common Public License v1.0 which accompanies this         */
@REM /* distribution. A copy is also available at the following address:           */
@REM /* http://www.oorexx.org/license.html                          */
@REM /*                                                                            */
@REM /* Redistribution and use in source and binary forms, with or                 */
@REM /* without modification, are permitted provided that the following            */
@REM /* conditions are met:                                                        */
@REM /*                                                                            */
@REM /* Redistributions of source code must retain the above copyright             */
@REM /* notice, this list of conditions and the following disclaimer.              */
@REM /* Redistributions in binary form must reproduce the above copyright          */
@REM /* notice, this list of conditions and the following disclaimer in            */
@REM /* the documentation and/or other materials provided with the distribution.   */
@REM /*                                                                            */
@REM /* Neither the name of Rexx Language Association nor the names                */
@REM /* of its contributors may be used to endorse or promote products             */
@REM /* derived from this software without specific prior written permission.      */
@REM /*                                                                            */
@REM /* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
@REM /* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
@REM /* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
@REM /* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
@REM /* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
@REM /* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
@REM /* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
@REM /* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
@REM /* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
@REM /* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
@REM /* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
@REM /*                                                                            */
@REM /*----------------------------------------------------------------------------*/
@ECHO Off
REM  This  build program is called from ORXDEBUG, ORXSHIP.bat
REM  It assumes certain environment variables are set, if they
REM  are not the make files will return error messages....
REM  These variables are defined in ORXDEBUG.bat and ORXSHIP.bat
REM
REM
REM  Move to source drive letter
REM
%SRC_DRV%
REM
REM
REM Save LIB and INCLUDE
REM
set RXSAVE_LIB=%LIB%
set RXSAVE_INCLUDE=%INCLUDE%
REM
REM *** REXXAPI 1st to build
REM
rem REM *** Rexxapi
rem REM Rexxapi before oryxk for ORDAPI.C
REM
@ECHO Building Rexxapi..
CD  %OR_ORYXASRC%
IF %OR_ERRLOG%x == x NMAKE /F REXXAPI.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F REXXAPI.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

REM
REM *** Kernel
REM
@ECHO Building Kernel....
CD  %OR_ORYXKSRC%
IF %OR_ERRLOG%x == x NMAKE /F KERNEL.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F KERNEL.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

REM *** orexxole
REM
:OREXXOLE
@ECHO Building OREXXOLE..
CD  %OR_ORYXOLESRC%
IF %OR_ERRLOG%x == x NMAKE /F OREXXOLE.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F OREXXOLE.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error


REM
REM *** These are the commmand lanuchers, need the kernel and rexxapi
REM
@echo off
@ECHO Building Command launchers
CD  %OR_ORYXWSRC%
IF %OR_ERRLOG%x == x NMAKE /F ORYXWIN.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F ORYXWIN.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error


ECHO Building REXX.IMG ...
CD %OR_OUTDIR%
REXX -IB >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

@ECHO Building RXSUBCOM and RXQUEUE..
CD  %OR_ORYXASRC%
IF %OR_ERRLOG%x == x NMAKE /F Rxsubcom.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F Rxsubcom.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error
IF %OR_ERRLOG%x == x NMAKE /F Rxqueue.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F Rxqueue.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error


REM
rem REM *** Rexxutil
rem REM
@ECHO Building Rexxutil..
CD  %OR_ORYXRSRC%
IF %OR_ERRLOG%x == x NMAKE /F REXXUTIL.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F REXXUTIL.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

@ECHO Building rxwinsys.dll
IF %OR_ERRLOG%x == x NMAKE /F RXWINSYS.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F RXWINSYS.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

REM *** rxsock
REM
@ECHO Building RxSock..
CD  %OR_ORYXRSRC%
IF %OR_ERRLOG%x == x NMAKE /F Rxsock.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F Rxsock.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

REM *** rxmath
REM
@ECHO Building RxMath..
CD  %OR_ORYXRSRC%
IF %OR_ERRLOG%x == x NMAKE /F rxmath.mak
IF NOT %OR_ERRLOG%x == x NMAKE /F rxmath.mak >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error


REM *** rxregexp
REM
@ECHO Building RXREGEXP...
CD  %OR_ORYXREGEXP%
IF %OR_ERRLOG%x == x NMAKE /F RXREGEXP.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F RXREGEXP.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error


REM *** oodialog
REM
@ECHO Building OODIALOG..
CD  %OR_ORYXOODSRC%
IF %OR_ERRLOG%x == x NMAKE /F OODIALOG.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F OODIALOG.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error


ECHO Building OODIALOG classes
CD %OR_OUTDIR%
REXX %OR_ORYXOODSRC%\M_OODCLS >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error


REM *** orxscrpt
REM
ECHO Generating security manager code for script engine
CD %OR_OUTDIR%

REXX %OR_ORYXAXSCRIPT%\rexx2inc.rex %OR_ORYXAXSCRIPT%\security.rex %OR_ORYXAXSCRIPT%\security.inc szSecurityCode >> rexx2inc.log 2>&1
CD %SRC_DIR%

@ECHO Building ORXSCRPT..
CD  %OR_ORYXAXSCRIPT%
IF %OR_ERRLOG%x == x NMAKE /F ORXSCRPT.MAK
IF NOT %OR_ERRLOG%x == x NMAKE /F ORXSCRPT.MAK >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error


REM *** API samples
REM
@ECHO Building API Samples..
set LIB=%LIB%;%OR_OUTDIR%
rem set INCLUDE=%INCLUDE%;%OR_OUTDIR%
set INCLUDE=%OR_OUTDIR%;%OR_ORYXAWSRC%;%INCLUDE%

CD  %OR_ORYXAPISAMPLES%\callrxnt
IF %OR_ERRLOG%x == x NMAKE /F callrxnt.mak
IF NOT %OR_ERRLOG%x == x NMAKE /F callrxnt.mak >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

CD  %OR_ORYXAPISAMPLES%\callrxwn
IF %OR_ERRLOG%x == x NMAKE /F callrxwn.mak
IF NOT %OR_ERRLOG%x == x NMAKE /F callrxwn.mak >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

CD  %OR_ORYXAPISAMPLES%\rexxexit
IF %OR_ERRLOG%x == x NMAKE /F rexxexit.mak
IF NOT %OR_ERRLOG%x == x NMAKE /F rexxexit.mak >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

CD  %OR_ORYXAPISAMPLES%\wpipe\wpipe1
IF %OR_ERRLOG%x == x NMAKE /F rexxapi1.mak
IF NOT %OR_ERRLOG%x == x NMAKE /F rexxapi1.mak >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

CD  %OR_ORYXAPISAMPLES%\wpipe\wpipe2
IF %OR_ERRLOG%x == x NMAKE /F rexxapi2.mak
IF NOT %OR_ERRLOG%x == x NMAKE /F rexxapi2.mak >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

CD  %OR_ORYXAPISAMPLES%\wpipe\wpipe3
IF %OR_ERRLOG%x == x NMAKE /F rexxapi3.mak
IF NOT %OR_ERRLOG%x == x NMAKE /F rexxapi3.mak >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error

@ECHO Building OODialog Samples..
CD  %OR_ORYXOODIALOGSAMPLES%\res
IF %OR_ERRLOG%x == x NMAKE /F res.mak
IF NOT %OR_ERRLOG%x == x NMAKE /F res.mak >>%OR_ERRLOG% 2>&1
if ERRORLEVEL 1 goto error


REM
REM Go to output directory and
REM
CD %OR_OUTDIR%

goto arounderr

:error
@echo ***! Error occured !** : build halted
set LIB=%RXSAVE_LIB%
set INCLUDE=%RXSAVE_INCLUDE%
%SRC_DRV%
CD %SRC_DIR%
exit /b 1

:arounderr
%SRC_DRV%
CD %SRC_DIR%
