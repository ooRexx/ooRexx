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

REM Note: This batch file will not work if command extensions are disabled.
REM       Command extensions are enabled by default in W2K, XP, W2K3, and Vista
REM       They are probably enabled in NT.

REM Note: needed for XCOPY command under Win 2000 to suppress the overwrite question
REM Win NT 4.0 ignores this environment variable
SET COPYCMD=/Y

REM MHES 29122004 Added set CPU=ix86
set CPU=ix86

REM Check that we have at least the first option
IF %1x == x GOTO HELP

REM Check for the 'package' option
if %2x == x (
  SET DOPACKAGE=0
  GOTO DOC_CHECK_DONE
)

if %2 == PACKAGE (
  SET DOPACKAGE=1
  SET PACKAGE_REL=0
  SET PACKAGE_DBG=0
) ELSE GOTO HELP

REM  The package step is being done, check for the docs.
goto DOC_CHECK

:DOC_CHECK_DONE

REM Check that SRC_DIR and SRC_DRV are set
IF %SRC_DRV%x == x GOTO HELP_SRC_DRV
IF %SRC_DIR%x == x GOTO HELP_SRC_DRV

REM Check for the type of build
:CHECK_BUILD_TYPE
IF %1 == NODEBUG GOTO NODEBUG
IF %1 == DEBUG GOTO DEBUG
IF %1 == BOTH GOTO BOTH

:HELP
ECHO Syntax: makeorx BUILD_TYPE [PACKAGE] [DOC_LOCATION]
ECHO Where BUILD_TYPE is required and exactly one of DEBUG NODEBUG BOTH
ECHO Where PACKAGE is optional.  If present and exactly PACKAGE the
ECHO Windows ooRexx install package will be built.
ECHO.
ECHO If creating the install package, the ooRexx PDF documentation must be
ECHO located in the doc subdirectory of the root build directory.  If it is
ECHO not, an attempt will be made to copy it from the directory specified by
ECHO the third optional argument: DOC_LOCATION.  Note that alternatively,
ECHO DOC_LOCATION can be specified as an environment variable.  I.e.,
ECHO.
ECHO set DOC_LOCATION=C:\myDocs
ECHO makeorx NODEBUG PACKAGE
ECHO.
ECHO and
ECHO.
ECHO makeorx NODEBUG PACKAGE C:\myDocs
ECHO.
ECHO are equivalent commands.
GOTO ENV_VARS_CLEANUP

:HELP_SRC_DRV
ECHO *==============================================================
ECHO One of the environment variables SRC_DRV or SRC_DIR is not set
ECHO Set the SRC_DRV variable to the build directory drive letter
ECHO Set the SRC_DIR variable to the full build directory path
ECHO e.g.
ECHO "SET SRC_DRV=F:"
ECHO "SET SRC_DIR=\oorexx\interpreter_3x"
ECHO *======================================================
GOTO ENV_VARS_CLEANUP

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

IF ERRORLEVEL 1 GOTO ENV_VARS_CLEANUP

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
) ELSE GOTO ENV_VARS_CLEANUP

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

REM If not making the release version skip to environment variables clean up.
IF %PACKAGE_REL% == 0 GOTO ENV_VARS_CLEANUP

:PACKAGE_RELEASE
SET BINDIR=/DBINDIR=%SRC_DRV%%SRC_DIR%\Win32Rel
cd platform\windows
makensis %DOTVER% %NODOTVER% %SRCDIR% %BINDIR% oorexx.nsi
move ooRexx%NODOTS%.exe ..\..\
cd ..\..\

:ENV_VARS_CLEANUP
SET CPU=
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
SET MISSING_DOC=

GOTO END

:SET_FAILED
ECHO Could not set the package major minor numbers.
ECHO Skipping the installer package step.
GOTO ENV_VARS_CLEANUP

REM - - - - - - - - - - - - END exits this batch file - - - - - - - - - - - - -
:END
REM Make sure we are back in the root build directory.
CD %SRC_DRV%%SRC_DIR%
exit /b

REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM   Labels below this point handle some of the tedious chores for this
REM   batch file.  They then use goto to return to a point of execution above.
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

:DOC_CHECK
if not exist doc\nul md doc

SET MISSING_DOC=0
if not exist doc\readme.pdf   SET MISSING_DOC=1
if not exist doc\rexxpg.pdf   SET MISSING_DOC=1
if not exist doc\rexxref.pdf  SET MISSING_DOC=1
if not exist doc\rxmath.pdf   SET MISSING_DOC=1
if not exist doc\rxsock.pdf   SET MISSING_DOC=1
if not exist doc\rxftp.pdf    SET MISSING_DOC=1
if not exist doc\oodialog.pdf SET MISSING_DOC=1

if %MISSING_DOC% EQU 0 goto DOC_CHECK_DONE

REM  Missing some doc, try to copy it from a specified location.
if %DOC_LOCATION%x == x (
  if %3x == x (
    ECHO The package option is specified, but some doc is missing and the
    ECHO location to copy the doc from can not be determined.
    ECHO.
    GOTO HELP
  ) else SET DOC_LOCATION=%3
)

if not exist doc\readme.pdf (
 if not exist %DOC_LOCATION%\readme.pdf goto NO_DOC_ERR
)
copy %DOC_LOCATION%\readme.pdf doc 1>nul 2>&1

if not exist doc\rexxpg.pdf (
 if not exist %DOC_LOCATION%\rexxpg.pdf goto NO_DOC_ERR
)
copy %DOC_LOCATION%\rexxpg.pdf doc 1>nul 2>&1

if not exist doc\rexxref.pdf (
 if not exist %DOC_LOCATION%\rexxref.pdf goto NO_DOC_ERR
)
copy %DOC_LOCATION%\rexxref.pdf doc 1>nul 2>&1

if not exist doc\rxmath.pdf (
 if not exist %DOC_LOCATION%\rxmath.pdf goto NO_DOC_ERR
)
copy %DOC_LOCATION%\rxmath.pdf doc 1>nul 2>&1

if not exist doc\rxsock.pdf (
 if not exist %DOC_LOCATION%\rxsock.pdf goto NO_DOC_ERR
)
copy %DOC_LOCATION%\rxsock.pdf doc 1>nul 2>&1

if not exist doc\rxftp.pdf (
 if not exist %DOC_LOCATION%\rxftp.pdf goto NO_DOC_ERR
)
copy %DOC_LOCATION%\rxftp.pdf doc 1>nul 2>&1

if not exist doc\oodialog.pdf (
 if not exist %DOC_LOCATION%\oodialog.pdf goto NO_DOC_ERR
)
copy %DOC_LOCATION%\oodialog.pdf doc 1>nul 2>&1

GOTO DOC_CHECK_DONE

:NO_DOC_ERR
ECHO Failed to locate some doc file(s) for the package option, aborting.
GOTO ENV_VARS_CLEANUP
