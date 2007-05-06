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
SET COPYCMD=/Y

REM MHES 29122004 Added set CPU=ix86
set CPU=ix86

REM Check for the 'package' option
if %2x == x (
  SET DOPACKAGE=0
  GOTO SKIP
)

if %2 == PACKAGE (
  SET DOPACKAGE=1
  SET PACKAGE_REL=0
  SET PACKAGE_DBG=0
) ELSE GOTO HELP

:SKIP

REM Check that SRC_DIR and SRC_DRV are set
IF %SRC_DRV%x == x GOTO HELP_SRC_DRV
IF %SRC_DIR%x == x GOTO HELP_SRC_DRV

REM Check for the type of build
IF %1x == x GOTO HELP
IF %1 == NODEBUG GOTO NODEBUG
IF %1 == DEBUG GOTO DEBUG
IF %1 == BOTH GOTO BOTH

:HELP
ECHO Syntax: makeorx BUILD_TYPE [PACKAGE]
ECHO Where BUILD_TYPE is required and exactly one of DEBUG NODEBUG BOTH
ECHO Where PACKAGE is optional.  If present and exactly PACKAGE the
ECHO Windows ooRexx install package will be built.
GOTO END

:HELP_SRC_DRV
ECHO *==============================================================
ECHO One of the environment variables SRC_DRV or SRC_DIR is not set
ECHO Set the SRC_DRV variable to the build directory drive letter
ECHO Set the SRC_DIR variable to the full build directory path
ECHO e.g.
ECHO "SET SRC_DRV=F:"
ECHO "SET SRC_DIR=\oorexx\interpreter_3x"
ECHO *======================================================
GOTO END

:NODEBUG
SET MKNODEBUG=1
SET MKDEBUG=0
IF %DOPACKAGE% == 1 SET PACKAGE_REL=1
GOTO BUILD

:DEBUG
SET MKNODEBUG=0
SET MKDEBUG=1
IF %DOPACKAGE% == 1 SET PACKAGE_DBG=1
GOTO BUILD

:BOTH
SET MKNODEBUG=1
SET MKDEBUG=1
IF %DOPACKAGE% == 1 (
  SET PACKAGE_REL=1
  SET PACKAGE_DBG=1
)

:BUILD
IF %MKNODEBUG% == 0 GOTO BLDDEBUG

ECHO Building Open Object REXX for Windows - Non-Debug Version
SET MKASM=1
SET BLDRELEASE=1
GOTO STARTBUILD

:BLDDEBUG
ECHO Building Open Object REXX for Windows - Debug Version
SET MKASM=0
SET BLDRELEASE=0

REM Don't loop building the debug version, forever.
SET MKDEBUG=0

:STARTBUILD
killer rxapi.exe
CALL ORXDB %BLDRELEASE%

IF ERRORLEVEL 1 GOTO PACKAGE_CLEANUP

REM Make sure we are back in the root build directory.
CD %SRC_DRV%%SRC_DIR%

REM Check if we still need to build the debug version.
IF %MKDEBUG% == 1 GOTO BLDDEBUG

REM Check if we are building the installer package.
IF %DOPACKAGE% == 1 (
  FOR /F "eol=# delims== tokens=1,2*" %%i IN (oorexx.ver) DO (
    IF %%i == ORX_MAJOR SET MAJOR_NUM=%%j
    IF %%i == ORX_MINOR SET MINOR_NUM=%%j
    IF %%i == ORX_MOD_LVL SET LVL_NUM=%%j
  )
) ELSE (
  SET DOPACKAGE=
  GOTO END
)

IF %MAJOR_NUM%x == x GOTO SET_FAILED
SET NODOTS=%MAJOR_NUM%%MINOR_NUM%%LVL_NUM%
SET DOTVER=/DVERSION=%MAJOR_NUM%.%MINOR_NUM%.%LVL_NUM%
SET NODOTVER=/DNODOTVER=%NODOTS%
SET SRCDIR=/DSRCDIR=%SRC_DRV%%SRC_DIR%

REM If not making the debug version skip to packaging the release version
IF %PACKAGE_DBG% == 0 GOTO PACKAGE_RELEASE

SET BINDIR=/DBINDIR=%SRC_DRV%%SRC_DIR%\Win32Dbg
cd platform\windows
makensis %DOTVER% %NODOTVER% %SRCDIR% %BINDIR% oorexx.nsi

REM Rename the deug package so it is not overwritten if the release package
REM is created.
ren ooRexx%NODOTS%.exe ooRexx%NODOTS%-debug.exe
move ooRexx%NODOTS%-debug.exe ..\..\
cd ..\..\

REM If not making the release version skip to package clean up.
IF %PACKAGE_REL% == 0 GOTO PACKAGE_CLEANUP

:PACKAGE_RELEASE
SET BINDIR=/DBINDIR=%SRC_DRV%%SRC_DIR%\Win32Rel
cd platform\windows
makensis %DOTVER% %NODOTVER% %SRCDIR% %BINDIR% oorexx.nsi
move ooRexx%NODOTS%.exe ..\..\
cd ..\..\

:PACKAGE_CLEANUP
SET DOPACKAGE=
SET PACKAGE_REL=
SET PACKAGE_DBG=
SET MAJOR_NUM=
SET MINOR_NUM=
SET LVL_NUM=
SET NODOTS=
SET DOTVER=
SET NODOTVER=
SET SRCDIR=
SET BINDIR=

GOTO END

:SET_FAILED
ECHO Could not set the package major minor numbers.
ECHO Skipping the installer package step.

:END
REM Make sure we are back in the root build directory.
CD %SRC_DRV%%SRC_DIR%
