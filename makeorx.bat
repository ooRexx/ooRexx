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
@ECHO OFF
REM Note: needed for XCOPY command under Win 2000 to suppress the overwrite question
REM Win NT 4.0 ignores this environment variable
SET COPYCMD=/y

REM MHES 29122004 Added set CPU=ix386
set CPU=ix86

IF %1x == x GOTO :HELP
IF %1 == NODEBUG GOTO :NODEBUG
IF %1 == DEBUG GOTO :DEBUG
IF %1 == BOTH GOTO :BOTH

:HELP
ECHO Call MAKEORX with DEBUG/NODEBUG/BOTH
ECHO as argument to specify desired output.
GOTO :END

:NODEBUG
SET MKNODEBUG=1
SET MKDEBUG=0
GOTO :BUILD

:DEBUG
SET MKNODEBUG=0
SET MKDEBUG=1
GOTO :BUILD

:BOTH
SET MKNODEBUG=1
SET MKDEBUG=1

:BUILD
IF %SRC_DRV%x == x GOTO HELP_SRC_DRV
REM regedit /s %SRC_DRV%%SRC_DIR%\fullprev_ver.reg
rem ECHO Generating message files with GENRXMSG
taskkill /F /IM rxapi.exe

IF %MKNODEBUG% == 0 GOTO :BLDDEBUG

ECHO Building Object REXX for Windows - Non-Debug Version
SET MKASM=1
SET BLDRELEASE=1
GOTO :STARTBUILD

:BLDDEBUG
SET MKASM=0
SET BLDRELEASE=0

:STARTBUILD
taskkill /F /IM rxapi.exe
CALL ORXDB %BLDRELEASE%

GOTO END

:HELP_SRC_DRV
ECHO *======================================================
ECHO The environment variabel SRC_DRV is not set
ECHO Set the variable to the build directory drive letter
ECHO e.g. "SET SRC_DRV=F:"
ECHO *======================================================

:END
CD %SRC_DRV%%SRC_DIR%
