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
IF %USELOGFILE% equ 1 ( NMAKE /F REXXAPI.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F REXXAPI.MAK )
if ERRORLEVEL 1 goto error

REM
REM *** Kernel
REM
@ECHO Building Kernel....
CD  %OR_ORYXKSRC%
IF %USELOGFILE% equ 1 ( NMAKE /F KERNEL.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F KERNEL.MAK )
if ERRORLEVEL 1 goto error

REM *** orexxole
REM
:OREXXOLE
@ECHO Building OREXXOLE..
CD  %OR_ORYXOLESRC%
IF %USELOGFILE% equ 1 ( NMAKE /F OREXXOLE.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F OREXXOLE.MAK )
if ERRORLEVEL 1 goto error


REM
REM *** Rexxutil. Note that RexxUtil needs to be built before rexx.img is created.
REM
@ECHO Building Rexxutil..
CD  %OR_ORYXRSRC%
IF %USELOGFILE% equ 1 ( NMAKE /F REXXUTIL.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F REXXUTIL.MAK )
if ERRORLEVEL 1 goto error


REM
REM *** These are the commmand lanuchers, need the kernel and rexxapi
REM
@ECHO Building Command launchers
CD  %OR_ORYXWSRC%
IF %USELOGFILE% equ 1 ( NMAKE /F ORYXWIN.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F ORYXWIN.MAK )
if ERRORLEVEL 1 goto error


@ECHO Building REXX.IMG ...
CD %OR_OUTDIR%
IF %USELOGFILE% equ 1 ( REXXIMAGE >>%OR_ERRLOG% 2>&1 ) else ( REXXIMAGE )
if ERRORLEVEL 1 goto error

@ECHO Building RXSUBCOM and RXQUEUE..
CD  %OR_ORYXASRC%
IF %USELOGFILE% equ 1 ( NMAKE /F Rxsubcom.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F Rxsubcom.MAK )
if ERRORLEVEL 1 goto error
IF %USELOGFILE% equ 1 ( NMAKE /F Rxqueue.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F Rxqueue.MAK )
if ERRORLEVEL 1 goto error


@ECHO Building rxwinsys.dll
CD  %OR_ORYXRSRC%
IF %USELOGFILE% equ 1 ( NMAKE /F RXWINSYS.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F RXWINSYS.MAK )
if ERRORLEVEL 1 goto error

REM *** rxsock
REM
@ECHO Building RxSock..
CD  %OR_ORYXRSRC%
IF %USELOGFILE% equ 1 ( NMAKE /F Rxsock.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F Rxsock.MAK )
if ERRORLEVEL 1 goto error

REM *** rxmath
REM
@ECHO Building RxMath..
CD  %OR_ORYXRSRC%
IF %USELOGFILE% equ 1 ( NMAKE /F rxmath.mak >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F rxmath.mak )
if ERRORLEVEL 1 goto error


REM *** rxregexp
REM
@ECHO Building RXREGEXP...
CD  %OR_ORYXREGEXP%
IF %USELOGFILE% equ 1 ( NMAKE /F RXREGEXP.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F RXREGEXP.MAK )
if ERRORLEVEL 1 goto error


REM *** oodialog
REM
@ECHO Building OODIALOG..
CD  %OR_ORYXOODSRC%
IF %USELOGFILE% equ 1 ( NMAKE /F OODIALOG.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F OODIALOG.MAK )
if ERRORLEVEL 1 goto error


ECHO Building OODIALOG classes
CD %OR_OUTDIR%
IF %USELOGFILE% equ 1 ( REXX %OR_ORYXOODSRC%\M_OODCLS.REX >>%OR_ERRLOG% 2>&1 ) else ( REXX %OR_ORYXOODSRC%\M_OODCLS.REX )
if ERRORLEVEL 1 goto error


CD %SRC_DIR%

REM @ECHO Building ORXSCRPT..
REM CD  %OR_ORYXAXSCRIPT%
REM IF %USELOGFILE% equ 1 ( NMAKE /F ORXSCRPT.MAK >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F ORXSCRPT.MAK )
REM if ERRORLEVEL 1 goto error


REM *** API samples
REM
@ECHO Building API Samples..
set LIB=%LIB%;%OR_OUTDIR%
set INCLUDE=%OR_OUTDIR%;%SAMPLEPATH%;%INCLUDE%
@ECHO Include path is %INCLUDE%

CD  %OR_ORYXAPISAMPLES%\callrxnt
IF %USELOGFILE% equ 1 ( NMAKE /F callrxnt.mak >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F callrxnt.mak )
if ERRORLEVEL 1 goto error

CD  %OR_ORYXAPISAMPLES%\callrxwn
IF %USELOGFILE% equ 1 ( NMAKE /F callrxwn.mak >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F callrxwn.mak )
if ERRORLEVEL 1 goto error

CD  %OR_ORYXAPISAMPLES%\rexxexit
IF %USELOGFILE% equ 1 ( NMAKE /F rexxexit.mak >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F rexxexit.mak )
if ERRORLEVEL 1 goto error

CD  %OR_ORYXAPISAMPLES%\wpipe\wpipe1
IF %USELOGFILE% equ 1 ( NMAKE /F rexxapi1.mak >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F rexxapi1.mak )
if ERRORLEVEL 1 goto error

CD  %OR_ORYXAPISAMPLES%\wpipe\wpipe2
IF %USELOGFILE% equ 1 ( NMAKE /F rexxapi2.mak >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F rexxapi2.mak )
if ERRORLEVEL 1 goto error

CD  %OR_ORYXAPISAMPLES%\wpipe\wpipe3
IF %USELOGFILE% equ 1 ( NMAKE /F rexxapi3.mak >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F rexxapi3.mak )
if ERRORLEVEL 1 goto error

@ECHO Building OODialog Samples..
CD  %OR_ORYXOODIALOGSAMPLES%\res
IF %USELOGFILE% equ 1 ( NMAKE /F res.mak >>%OR_ERRLOG% 2>&1 ) else ( NMAKE /F res.mak )
if ERRORLEVEL 1 goto error

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
