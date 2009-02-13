@REM
@REM Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.
@REM Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.
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

REM  No sense in starting if SRC_DIR and SRC_DRV are not set.
IF %SRC_DRV%x == x GOTO HELP_SRC_DRV
IF %SRC_DIR%x == x GOTO HELP_SRC_DRV

REM  Might be needed for a build under Win 2000.
SET COPYCMD=/Y

REM  Check that we have at least the first option
IF %1x == x GOTO HELP

REM  If NO_BUILD_LOG is set, do not redirect to a log.
if %NO_BUILD_LOG%x == x (set USELOGFILE=1) else (set USELOGFILE=0)

REM  Check for the 'package' option
goto PACKAGE_CHECK
:PACKAGE_CHECK_DONE

REM  Check for the type of build
if %1 == NODEBUG goto NO_DEBUG
if %1 == DEBUG goto IS_DEBUG
if %1 == BOTH goto DO_BOTH

REM  Okay, the first arg is not right, show help and quit.
goto HELP

:BUILD_CHECK_DONE

REM  Figure out the compiler and if this is a 64-bit build.
goto DETERMINE_COMPILER
:DETERMINE_C0MPILER_DONE

REM  Print out the args and environment variables to help with debug if the
REM  build does not complete.
goto PRINT_OUT_VARS
:PRINT_OUT_VARS_DONE

REM  If the package step is being done, check for the docs.
IF %DOPACKAGE% == 1 goto DOC_CHECK
:DOC_CHECK_DONE

REM  Generate, (or use an existing,) oorexx.ver.incl file.
goto GENERATE_VERSION_FILE
:GENERATE_VERSION_FILE_DONE

:BUILD
IF %MKNODEBUG% == 0 GOTO BLDDEBUG

if %USELOGFILE% EQU 1 (
  ECHO Building Open Object REXX for Windows - Non-Debug Version >>%OR_ERRLOG%
) else (
  ECHO Building Open Object REXX for Windows - Non-Debug Version
)

SET MKASM=1
SET BLDRELEASE=1
GOTO STARTBUILD

REM  If we are building BOTH, we need to reset the log name.  We just set it
REM  unconditionally.
:BLDDEBUG
set OR_OUTDIR=%SRC_DRV%%SRC_DIR%\Win32Dbg
set OR_ERRLOG=%OR_OUTDIR%\Win32Dbg.log

if %USELOGFILE% EQU 1 (
  ECHO Building Open Object REXX for Windows - Debug Version >>%OR_ERRLOG%
) else (
  ECHO Building Open Object REXX for Windows - Debug Version
)

SET MKASM=0
SET BLDRELEASE=0

REM  Don't loop building the debug version, forever.
SET MKDEBUG=0

:STARTBUILD
killer rxapi.exe
CALL ORXDB %BLDRELEASE%

IF ERRORLEVEL 1 GOTO ENV_VARS_CLEANUP

REM  Make sure we are back in the root build directory.
CD %SRC_DRV%%SRC_DIR%

REM  Check if we still need to build the debug version.
IF %MKDEBUG% == 1 GOTO BLDDEBUG

REM Check if we are building the installer package.
IF %DOPACKAGE% == 0 GOTO ENV_VARS_CLEANUP

IF %MAJOR_NUM%x == x GOTO SET_FAILED
SET NODOTS=%MAJOR_NUM%%MINOR_NUM%%LVL_NUM%_%BLD_NUM%
SET DOTVER=/DVERSION=%MAJOR_NUM%.%MINOR_NUM%.%LVL_NUM%.%BLD_NUM%
SET NODOTVER=/DNODOTVER=%NODOTS%
SET SRCDIR=/DSRCDIR=%SRC_DRV%%SRC_DIR%

REM  If not making the debug version skip to packaging the release version
IF %PACKAGE_DBG% == 0 GOTO PACKAGE_RELEASE

SET BINDIR=/DBINDIR=%SRC_DRV%%SRC_DIR%\Win32Dbg
cd platform\windows\install
makensis %DOTVER% %NODOTVER% %SRCDIR% %BINDIR% oorexx.nsi

REM  Rename the deug package so it is not overwritten if the release package
REM  is created.
ren ooRexx%NODOTS%.exe ooRexx%NODOTS%-debug.exe
move ooRexx%NODOTS%-debug.exe ..\..\..\
cd ..\..\..\

REM  If not making the release version skip to environment variables clean up.
IF %PACKAGE_REL% == 0 GOTO ENV_VARS_CLEANUP

:PACKAGE_RELEASE
SET BINDIR=/DBINDIR=%SRC_DRV%%SRC_DIR%\Win32Rel
cd platform\windows\install
makensis %DOTVER% %NODOTVER% %SRCDIR% %BINDIR% oorexx.nsi
move ooRexx%NODOTS%.exe ..\..\..\
cd ..\..\..\

:ENV_VARS_CLEANUP
SET DOPACKAGE=
SET PACKAGE_REL=
SET PACKAGE_DBG=
SET MAJOR_NUM=
SET MINOR_NUM=
SET LVL_NUM=
SET BLD_NUM=
SET NODOTS=
SET DOTVER=
SET NODOTVER=
SET SRCDIR=
SET BINDIR=
SET MISSING_DOC=
SET SVN_REV=
SET USELOGFILE=

GOTO END

:SET_FAILED
if %USELOGFILE% EQU 1 (
  ECHO Could not set the package major minor numbers. >>%OR_ERRLOG%
  ECHO Skipping the installer package step. >>%OR_ERRLOG%
) else (
  ECHO Could not set the package major minor numbers.
  ECHO Skipping the installer package step.
)

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

REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM  :DOC_CHECK
REM    This section checks for the existence of the PDF doc files. If they are
REM    missing, it attempts to copy them from a location specified as the 3rd
REM    argument to this batch file or specified in the environmental variable:
REM    DOC_LOCATION
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
:DOC_CHECK
if not exist doc\nul md doc

SET MISSING_DOC=0
if not exist doc\readme.pdf        SET MISSING_DOC=1
if not exist doc\rexxpg.pdf        SET MISSING_DOC=1
if not exist doc\rexxref.pdf       SET MISSING_DOC=1
if not exist doc\rxmath.pdf        SET MISSING_DOC=1
if not exist doc\rxsock.pdf        SET MISSING_DOC=1
if not exist doc\rxftp.pdf         SET MISSING_DOC=1
if not exist doc\oodialog.pdf      SET MISSING_DOC=1
if not exist doc\winextensions.pdf SET MISSING_DOC=1

if %MISSING_DOC% EQU 0 goto DOC_CHECK_DONE

REM  Missing some doc, try to copy it from a specified location.
if %DOC_LOCATION%x == x (
  if %3x == x (
    if %USELOGFILE% EQU 1 (
      echo The package option is specified, but some doc is missing and the >>%OR_ERRLOG%
      echo location to copy the doc from can not be determined. >>%OR_ERRLOG%
      echo. >>%OR_ERRLOG%
    ) else (
      echo The package option is specified, but some doc is missing and the
      echo location to copy the doc from can not be determined.
      echo.
    )
    GOTO HELP
  ) else SET DOC_LOCATION=%3
)

if not exist doc\readme.pdf (
  if not exist %DOC_LOCATION%\readme.pdf (
    if %USELOGFILE% EQU 1 (echo readme.pdf is missing >>%OR_ERRLOG%) else (echo readme.pdf is missing)
    goto NO_DOC_ERR
  )
)
copy %DOC_LOCATION%\readme.pdf doc 1>nul 2>&1

if not exist doc\rexxpg.pdf (
  if not exist %DOC_LOCATION%\rexxpg.pdf (
    if %USELOGFILE% EQU 1 (echo rexxpg.pdf is missing >>%OR_ERRLOG%) else (echo rexxpg.pdf is missing)
    goto NO_DOC_ERR
  )
)
copy %DOC_LOCATION%\rexxpg.pdf doc 1>nul 2>&1

if not exist doc\rexxref.pdf (
  if not exist %DOC_LOCATION%\rexxref.pdf (
    if %USELOGFILE% EQU 1 (echo rexxref.pdf is missing >>%OR_ERRLOG%) else (echo rexxref.pdf is missing)
    goto NO_DOC_ERR
  )
)
copy %DOC_LOCATION%\rexxref.pdf doc 1>nul 2>&1

if not exist doc\rxmath.pdf (
  if not exist %DOC_LOCATION%\rxmath.pdf (
    if %USELOGFILE% EQU 1 (echo rxmath.pdf is missing >>%OR_ERRLOG%) else (echo rxmath.pdf is missing)
    goto NO_DOC_ERR
  )
)
copy %DOC_LOCATION%\rxmath.pdf doc 1>nul 2>&1

if not exist doc\rxsock.pdf (
  if not exist %DOC_LOCATION%\rxsock.pdf (
    if %USELOGFILE% EQU 1 (echo rxsock.pdf is missing >>%OR_ERRLOG%) else (echo rxsock.pdf is missing)
    goto NO_DOC_ERR
  )
)
copy %DOC_LOCATION%\rxsock.pdf doc 1>nul 2>&1

if not exist doc\rxftp.pdf (
  if not exist %DOC_LOCATION%\rxftp.pdf (
    if %USELOGFILE% EQU 1 (echo rxftp.pdf is missing >>%OR_ERRLOG%) else (echo rxftp.pdf is missing)
    goto NO_DOC_ERR
  )
)
copy %DOC_LOCATION%\rxftp.pdf doc 1>nul 2>&1

if not exist doc\oodialog.pdf (
  if not exist %DOC_LOCATION%\oodialog.pdf (
    if %USELOGFILE% EQU 1 (echo oodialog.pdf is missing >>%OR_ERRLOG%) else (echo oodialog.pdf is missing)
    goto NO_DOC_ERR
  )
)
copy %DOC_LOCATION%\oodialog.pdf doc 1>nul 2>&1

if not exist doc\winextensions.pdf (
  if not exist %DOC_LOCATION%\winextensions.pdf (
    if %USELOGFILE% EQU 1 (echo winextensions.pdf is missing >>%OR_ERRLOG%) else (echo winextensions.pdf is missing)
    goto NO_DOC_ERR
  )
)
copy %DOC_LOCATION%\winextensions.pdf doc 1>nul 2>&1

GOTO DOC_CHECK_DONE

:NO_DOC_ERR
if %USELOGFILE% EQU 1 (
  echo.  >>%OR_ERRLOG%
  echo Failed to locate at least one doc file for the package option, aborting. >>%OR_ERRLOG%
  echo.
) else (
  echo.
  echo Failed to locate at least one doc file for the package option, aborting.
  echo.
)
goto HELP


REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM  :GENERATE_VERSION_FILE
REM    This section generates, (or uses an existing,) oorexx.ver.incl file.
REM    If executing in a svn 'working copy' directory, it determines the current
REM    revision number and includes that information in the generated file.  If
REM    not a working directory, it checks for an existing oorexx.ver.incl file,
REM    which may have been included in a source file package when the package
REM    was created.  If not a svn directory, and no oorexx.ver.incl file, simply
REM    copy oorexx.ver to oorexx.ver.incl.
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
:GENERATE_VERSION_FILE

REM  First parse oorexx.ver to get the existing version numbers.
for /F "eol=# delims== tokens=1,2,3*" %%i in (oorexx.ver) do (
 if %%i == ORX_MAJOR set MAJOR_NUM=%%j
 if %%i == ORX_MINOR set MINOR_NUM=%%j
 if %%i == ORX_MOD_LVL set LVL_NUM=%%j
 if %%i == ORX_BLD_LVL set BLD_NUM=%%j
)

if not exist .svn\nul goto NOSVN

for /F "usebackq tokens=1,2,3,4*" %%i in (`svn info`) do if (%%i) == (Revision:) set SVN_REV=%%j

if %SVN_REV%x == x (
  echo Executing in a svn working copy, but could not determine the svn revision
  echo number.
  echo Do NOT use this environment for a release build.
  echo.
  goto NOSVN
)

REM  Now write out oorexx.ver.incl
if exist oorexx.ver.incl del /F /Q oorexx.ver.incl
for /F "delims== tokens=1,2,3*" %%i in (oorexx.ver) do (
 if %%i == ORX_BLD_LVL (
   echo %%i=%SVN_REV%>> oorexx.ver.incl
   set BLD_NUM=%SVN_REV%
 ) else (
   if %%i == ORX_VER_STR (
     echo %%i="%MAJOR_NUM%.%MINOR_NUM%.%LVL_NUM%.%SVN_REV%">> oorexx.ver.incl
   ) else (
     if %%jx == x (
       echo %%i>> oorexx.ver.incl
     ) else (
       echo %%i=%%j>> oorexx.ver.incl
     )
   )
 )
)
echo SVN_REVSION=%SVN_REV%>> oorexx.ver.incl
goto GENERATE_VERSION_FILE_DONE

:NOSVN
if exist oorexx.ver.incl (
   for /F "eol=# delims== tokens=1,2,3*" %%i in (oorexx.ver.incl) do (
    if %%i == ORX_BLD_LVL set BLD_NUM=%%j
    if %%i == SVN_REVISION set SVN_REV=%%j
   )
) else (
  copy oorexx.ver oorexx.ver.incl 1>nul 2>&1
  set SVN_REV=%BLD_NUM%
  echo SVN_REVSION=%SVN_REV%>> oorexx.ver.incl
)

goto GENERATE_VERSION_FILE_DONE


REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM  :PRINT_OUT_VARS
REM    Prints out the value of the args to this batch file and the value of
REM    the needed environment variables.  Having these values wrong can cause
REM    the build to abort early and / or not do what is expected.
REM
REM    We print these so that during an automated build there is some record
REM    to help determine why a build did not complete.
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
:PRINT_OUT_VARS
if %USELOGFILE% EQU 1 (
  echo. >>%OR_ERRLOG%
  echo Building Open Object REXX for Windows >>%OR_ERRLOG%
  echo Argument check --- >>%OR_ERRLOG%
  echo. >>%OR_ERRLOG%
  echo Arg 1 build type:   %1 >>%OR_ERRLOG%
  echo Arg 2 package:      %2 >>%OR_ERRLOG%
  echo Arg 3 doc location: %3 >>%OR_ERRLOG%
  echo. >>%OR_ERRLOG%
  echo Environment check --- >>%OR_ERRLOG%
  echo. >>%OR_ERRLOG%
  echo SRC_DRV: %SRC_DRV% >>%OR_ERRLOG%
  echo SRC_DIR: %SRC_DIR% >>%OR_ERRLOG%
  echo CPU: %CPU% >>%OR_ERRLOG%
  echo MSVCVER: %MSVCVER% >>%OR_ERRLOG%
  echo NO_BUILD_LOG: %NO_BUILD_LOG% >>%OR_ERRLOG%
  echo DOC_LOCATION: %DOC_LOCATION% >>%OR_ERRLOG%
  echo. >>%OR_ERRLOG%
) else (
  echo.
  echo Building Open Object REXX for Windows
  echo Argument check ---
  echo.
  echo Arg 1 build type: %1
  echo Arg 2 package: %2
  echo Arg 3 doc location: %3
  echo.
  echo Environment vars ---
  echo SRC_DRV: %SRC_DRV%
  echo SRC_DIR: %SRC_DIR%
  echo CPU: %CPU%
  echo MSVCVER: %MSVCVER%
  echo NO_BUILD_LOG: %NO_BUILD_LOG%
  echo DOC_LOCATION: %DOC_LOCATION%
  echo.
)
goto PRINT_OUT_VARS_DONE


REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM  :PACKAGE_CHECK
REM    Checks the second optional arg and sets the variables controlling whether
REM    the interpreter package is created or not.
REM
REM    If the second arg is incorrect, displays syntax and quits.
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
:PACKAGE_CHECK
if %2x == x (
  SET DOPACKAGE=0
  GOTO PACKAGE_CHECK_DONE
)

if %2 == PACKAGE (
  SET DOPACKAGE=1
  SET PACKAGE_REL=0
  SET PACKAGE_DBG=0
  GOTO PACKAGE_CHECK_DONE
) ELSE (
  GOTO HELP
)


REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM  :NO_DEBUG
REM    Sets the variables for a non-debug build and creates the output
REM    directory, if needed.
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
:NO_DEBUG
set MKNODEBUG=1
set MKDEBUG=0
IF %DOPACKAGE% == 1 SET PACKAGE_REL=1
set OR_OUTDIR=%SRC_DRV%%SRC_DIR%\Win32Rel
set OR_ERRLOG=%OR_OUTDIR%\Win32Rel.log
if not exist %OR_OUTDIR% md %OR_OUTDIR%
GOTO BUILD_CHECK_DONE


REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM  :IS_DEBUG
REM    Sets the variables for a debug build and creates the output directory, if
REM    needed.
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
:IS_DEBUG
set MKNODEBUG=0
set MKDEBUG=1
IF %DOPACKAGE% == 1 SET PACKAGE_DBG=1
set OR_OUTDIR=%SRC_DRV%%SRC_DIR%\Win32Dbg
set OR_ERRLOG=%OR_OUTDIR%\Win32Dbg.log
if not exist %OR_OUTDIR% md %OR_OUTDIR%
GOTO BUILD_CHECK_DONE


REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM  :DOBOTH
REM    Sets the variables to do both non-debug and debug builds.  We will create
REM    both output directories if needed.  The log name will first be used in
REM    the non-debug build.  Then be corrected in the debug build.
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
:DO_BOTH
set MKNODEBUG=1
set MKDEBUG=1
IF %DOPACKAGE% == 1 (
 set PACKAGE_REL=1
 set PACKAGE_DBG=1
)
set OR_OUTDIR=%SRC_DRV%%SRC_DIR%\Win32Rel
set OR_ERRLOG=%SRC_DRV%%SRC_DIR%\Win32Rel\Win32Rel.log
if not exist %OR_OUTDIR% md %OR_OUTDIR%
if not exist %SRC_DRV%%SRC_DIR%\Win32Dbg md %SRC_DRV%%SRC_DIR%\Win32Dbg
GOTO BUILD_CHECK_DONE


REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM  :DETERMINE_COMPILER
REM    This section attempts to determine the compiler version and build type.
REM    MSVCVER is set to a standard value that the make files can recognize.
REM    CPU is set to X86 or X64 to define if this is a 32-bit or 64-bit build
REM
REM    We don't check for failure here.  If MSVCVER does not get set, the build
REM    will fail later on with descriptive messages.  CPU will always get set,
REM    defaulting to X86.  If CPU comes out wrong, the builder will have to deal
REM    with it at that time.
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
:DETERMINE_COMPILER
cl > temp.txt.okayToDelete 2>&1

for /F "tokens=1,3,6-10,11" %%i in (temp.txt.okayToDelete) do (
  if %%i == Microsoft (
    if %%j == C/C++ (
      if %%l GTR 15.0 set MSVCVER=9.0
      if %%l GTR 14.0 (if %%l LSS 15 set MSVCVER=8.0)
      if %%l GTR 13.0 (if %%l LSS 14 set MSVCVER=7.0)
      if %%l GTR 12.0 (if %%l LSS 13 set MSVCVER=6.0)

      if %%n EQU 80x86 (set CPU=X86) else (
        if %%n EQU x64 (set CPU=X64) else (
          if %%n EQU AMD64 (set CPU=X64) else (set CPU=X86)
        )
      )
    ) else (
      if %%m GTR 15.0 set MSVCVER=9.0
      if %%m GTR 14.0 (if %%m LSS 15 set MSVCVER=8.0)
      if %%m GTR 13.0 (if %%m LSS 14 set MSVCVER=7.0)
      if %%m GTR 12.0 (if %%m LSS 13 set MSVCVER=6.0)

      if %%o EQU 80x86 (set CPU=X86) else (
        if %%o EQU x64 (set CPU=X64) else (
          if %%o EQU AMD64 (set CPU=X64) else (set CPU=X86)
        )
      )
    )
  )
)

del /F temp.txt.okayToDelete 1>nul 2>&1

goto DETERMINE_C0MPILER_DONE


REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM  :HELP
REM    This section just displays help and then goes to the clean up / exit.  It
REM    always echos the help to the screen, and will also echo it into a build
REM    log if one is being used.
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
:HELP

REM  Need to be sure some of the vars are set.  By default the log name and
REM  build directory will be set to release if they are not already set.
if %USELOGFILE%x == x set USELOGFILE=1
if %OR_OUTDIR%x == x  set OR_OUTDIR=%SRC_DRV%%SRC_DIR%\Win32Rel
if %OR_ERRLOG%x == x  set OR_ERRLOG=%OR_OUTDIR%\Win32Rel.log
if not exist %OR_OUTDIR% md %OR_OUTDIR%

REM  First echo to the screen the help, no matter what. So that someone building
REM  from the command line is sure to see the problem
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
ECHO.
ECHO By default all output is redirected to a log file.  To turn this off,
ECHO set the environment variable NO_BUILD_LOG to any value.  I.e.,
ECHO.
ECHO set NO_BUILD_LOG=1

REM  Now, if using a build log, echo the same thing into the log.
IF %USELOGFILE% equ 1 (
  ECHO Syntax: makeorx BUILD_TYPE [PACKAGE] [DOC_LOCATION] >>%OR_ERRLOG% 2>&1
  ECHO Where BUILD_TYPE is required and exactly one of DEBUG NODEBUG BOTH >>%OR_ERRLOG% 2>&1
  ECHO Where PACKAGE is optional.  If present and exactly PACKAGE the >>%OR_ERRLOG% 2>&1
  ECHO Windows ooRexx install package will be built. >>%OR_ERRLOG% 2>&1
  ECHO. >>%OR_ERRLOG% 2>&1
  ECHO If creating the install package, the ooRexx PDF documentation must be >>%OR_ERRLOG% 2>&1
  ECHO located in the doc subdirectory of the root build directory.  If it is >>%OR_ERRLOG% 2>&1
  ECHO not, an attempt will be made to copy it from the directory specified by >>%OR_ERRLOG% 2>&1
  ECHO the third optional argument: DOC_LOCATION.  Note that alternatively, >>%OR_ERRLOG% 2>&1
  ECHO DOC_LOCATION can be specified as an environment variable.  I.e., >>%OR_ERRLOG% 2>&1
  ECHO. >>%OR_ERRLOG% 2>&1
  ECHO set DOC_LOCATION=C:\myDocs >>%OR_ERRLOG% 2>&1
  ECHO makeorx NODEBUG PACKAGE >>%OR_ERRLOG% 2>&1
  ECHO. >>%OR_ERRLOG% 2>&1
  ECHO and >>%OR_ERRLOG% 2>&1
  ECHO. >>%OR_ERRLOG% 2>&1
  ECHO makeorx NODEBUG PACKAGE C:\myDocs >>%OR_ERRLOG% 2>&1
  ECHO. >>%OR_ERRLOG% 2>&1
  ECHO are equivalent commands. >>%OR_ERRLOG% 2>&1
  ECHO. >>%OR_ERRLOG% 2>&1
  ECHO By default all output is redirected to a log file.  To turn this off, >>%OR_ERRLOG% 2>&1
  ECHO set the environment variable NO_BUILD_LOG to any value.  I.e., >>%OR_ERRLOG% 2>&1
  ECHO. >>%OR_ERRLOG% 2>&1
  ECHO set NO_BUILD_LOG=1 >>%OR_ERRLOG% 2>&1
)

REM  Finally, output the args and environment variables so that the builder has
REM  a chance to spot any errors in them.
if %USELOGFILE% EQU 1 (
  echo. >>%OR_ERRLOG%
  echo Argument check --- >>%OR_ERRLOG%
  echo. >>%OR_ERRLOG%
  echo Arg 1 build type:   %1 >>%OR_ERRLOG%
  echo Arg 2 package:      %2 >>%OR_ERRLOG%
  echo Arg 3 doc location: %3 >>%OR_ERRLOG%
  echo. >>%OR_ERRLOG%
  echo Environment check --- >>%OR_ERRLOG%
  echo. >>%OR_ERRLOG%
  echo SRC_DRV: %SRC_DRV% >>%OR_ERRLOG%
  echo SRC_DIR: %SRC_DIR% >>%OR_ERRLOG%
  echo CPU: %CPU% >>%OR_ERRLOG%
  echo MSVCVER: %MSVCVER% >>%OR_ERRLOG%
  echo NO_BUILD_LOG: %NO_BUILD_LOG% >>%OR_ERRLOG%
  echo DOC_LOCATION: %DOC_LOCATION% >>%OR_ERRLOG%
  echo. >>%OR_ERRLOG%
) else (
  echo.
  echo Argument check ---
  echo.
  echo Arg 1 build type: %1
  echo Arg 2 package: %2
  echo Arg 3 doc location: %3
  echo.
  echo Environment vars ---
  echo SRC_DRV: %SRC_DRV%
  echo SRC_DIR: %SRC_DIR%
  echo CPU: %CPU%
  echo MSVCVER: %MSVCVER%
  echo NO_BUILD_LOG: %NO_BUILD_LOG%
  echo DOC_LOCATION: %DOC_LOCATION%
  echo.
)

GOTO ENV_VARS_CLEANUP


REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
REM  :HELP_SRC_DRV
REM     This section is used when SRC_DRV and / or SRC_DIR are not set.  It is
REM     the very first check and we don't bother to echo this into a log, we
REM     just quit after printing the text to the screen.
REM - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
