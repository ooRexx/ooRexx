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

REM Check for variables we require to be set by makeorx.bat.
IF %SRC_DRV%x == x GOTO HELP_SRC_DRV
IF %MKASM%x == x GOTO HELP_MKASM
IF %OR_ERRLOG%x == x GOTO HELP_LOG

REM
REM set up the directories for the generated files
REM
REM set OR_OUTDIR=O:\TESTDIR
if (%1)==(1) goto release
set OR_OUTDIR=%SRC_DRV%%SRC_DIR%\Win32Dbg
goto cont
:release
set OR_OUTDIR=%SRC_DRV%%SRC_DIR%\Win32Rel
:cont
REM
REM set up the directories for the source files
REM
set OR_REXXUTILSRC=%SRC_DRV%%SRC_DIR%\rexutils
set XPLATFORM=%OR_REXXUTILSRC%\windows
set OR_MAINSRC=%SRC_DRV%%SRC_DIR%
set OR_COMMONSRC=%SRC_DRV%%SRC_DIR%\common
set OR_COMMONPLATFORMSRC=%SRC_DRV%%SRC_DIR%\common\platform\windows
set OR_LIBSRC=%SRC_DRV%%SRC_DIR%\lib
set OR_APISRC=%SRC_DRV%%SRC_DIR%\api
set OR_APIWINSRC=%SRC_DRV%%SRC_DIR%\api\platform\windows
set OR_INTERPRETER_SRC=%SRC_DRV%%SRC_DIR%\interpreter
set OR_MESSAGESRC=%SRC_DRV%%SRC_DIR%\interpreter\messages
set OR_WINKERNELSRC=%SRC_DRV%%SRC_DIR%\platform\windows
set OR_REXXAPISRC=%SRC_DRV%%SRC_DIR%\rexxapi
set OR_OODIALOGSRC=%SRC_DRV%%SRC_DIR%\extensions\platform\windows\oodialog
set OR_OLEOBJECTSRC=%SRC_DRV%%SRC_DIR%\extensions\platform\windows\ole
set OR_ORXSCRIPTSRC=%SRC_DRV%%SRC_DIR%\extensions\platform\windows\orxscrpt
set OR_REGEXPSRC=%SRC_DRV%%SRC_DIR%\extensions\rxregexp
set OR_SAMPLESRC=%SRC_DRV%%SRC_DIR%\samples
set OR_APISAMPLESRC=%SRC_DRV%%SRC_DIR%\samples\windows\api
set OR_OODIALOGSAMPLES=%SRC_DRV%%SRC_DIR%\samples\windows\oodialog
set OR_UTILITIES=%SRC_DRV%%SRC_DIR%\utilities
set OR_EXTENSIONS=%SRC_DRV%%SRC_DIR%\extensions
set OR_HOSTEMUSRC=%OR_EXTENSIONS%\hostemu
set OR_HOSTEMUWINSRC=%OR_HOSTEMUSRC%\platform\windows

set INTERPRETER=%OR_INTERPRETER_SRC%
set INTERPRETER_CLASSES=%INTERPRETER%\classes
set CLASSES_SUPPORT=%INTERPRETER%\classes\support
set INTERPRETER_MESSAGES=%INTERPRETER%\messages
set INTERPRETER_RUNTIME=%INTERPRETER%\runtime
set INTERPRETER_API=%INTERPRETER%\api
set CONCURRENCY=%INTERPRETER%\concurrency
set BEHAVIOUR=%INTERPRETER%\behaviour
set MEMORY=%INTERPRETER%\memory
set PACKAGE=%INTERPRETER%\package
set EXECUTION=%INTERPRETER%\execution
set EXPRESSIONS=%INTERPRETER%\expression
set INSTRUCTIONS=%INTERPRETER%\instructions
set REXX_CLASSES=%INTERPRETER%\RexxClasses
set PARSER=%INTERPRETER%\parser
set INT_PLATFORM=%INTERPRETER%\platform\windows
set STREAM=%INTERPRETER%\streamLibrary

set OR_INTERPRETERPATH=%INTERPRETER%;%INTERPRETER_CLASSES%;%CLASSES_SUPPORT%;%INTERPRETER_RUNTIME%;%BEHAVIOUR%;%CONCURRENCY%;%EXECUTION%;%MEMORY%;%PACKAGE%;%INSTRUCTIONS%;%PARSER%;%KPLATFORM%;%INT_PLATFORM%;%STREAM%;%INTERPRETER_MESSAGES
set OR_INTERPRETERINCL=-I%INTERPRETER%\ -I%INTERPRETER_CLASSES%\ -I%CLASSES_SUPPORT%\ -I%INTERPRETER_RUNTIME%\ -I%BEHAVIOUR%\ -I%CONCURRENCY%\ -I%EXECUTION%\ -I%MEMORY%\ -I%PACKAGE%\ -I%EXPRESSIONS%\ -I%INSTRUCTIONS%\ -I%PARSER%\ -I%INT_PLATFORM%\ -I%STREAM%\ -I%INTERPRETER_MESSAGES%\
REM
REM set up the directory search orders for the source include files
REM
set OR_ORYXINCL=-I%OR_LIBSRC%\ -I%OR_COMMONSRC%\ -I%OR_COMMONPLATFORMSRC%\ -I%OR_APISRC%\ -I%OR_APIWINSRC%\ %OR_INTERPRETERINCL% -I%OR_WINKERNELSRC%\ -I%XPLATFORM% -I%OR_OODIALOGSRC%\ -I%OR_ORYXFSRC%\ -I%OR_OLEOBJECTSRC%\ -I%OR_ORXSCRIPTSRC%\ -I%OR_MESSAGESRC%\ -I%OR_HOSTEMUSRC%\
set OR_ORYXRCINCL=-I%INTERPRETER_MESSAGES%
REM
REM set up the search order for the dependency list
REM
set OR_SP={%OR_INTERPRETERPATH%;%OR_LIBSRC%;%OR_APISRC;%OR_APIWINSRC;%OR_WINKERNELSRC%;}
REM
REM set up the search order for the api samples
REM
set SAMPLEPATH=%OR_APISRC%;%OR_APIWINSRC%
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
ECHO *========================================================
ECHO The environment variable SRC_DRV is not set.  makeorx.bat
ECHO checks that this variable is set.  orxdb.bat should not
ECHO called directly.  Use makeorx.bat to build the Windows
ECHO version of the interpreter.
ECHO *========================================================

goto END

:HELP_LOG
ECHO *======================================================
ECHO The environment variable OR_ERRLOG is not set
ECHO This variable is set by makeorx.bat.  orxdb.bat should
ECHO not be called directly.  Use makeorx.bat to build the
ECHO Windows version of the intepreter.
ECHO *======================================================

goto END

:error
ENDLOCAL
exit /b 1

:END
ENDLOCAL
