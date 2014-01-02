@REM /*----------------------------------------------------------------------------*/
@REM /*                                                                            */
@REM /* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
@REM /* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.         */
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
@echo off

IF %SRC_DRV%x == x GOTO HELP_SRC_DRV
IF %SRC_DIR%x == x GOTO HELP_SRC_DRV

del %SRC_DRV%%SRC_DIR%\Win32Rel\dllfuncs.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\eng2rexx.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\engfact.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\nameditem.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\scrptdebug.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\orxIDispatch.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\OrxDispID.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\orxscrpt.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\classfactory.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\utilities.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\OrxScrptError.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\ORXSCRPT.dll >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\ORXSCRPT.exp >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\ORXSCRPT.lib >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\ORXSCRPT.map >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Rel\*.log >nul 2>&1

del %SRC_DRV%%SRC_DIR%\Win32Dbg\dllfuncs.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\eng2rexx.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\engfact.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\nameditem.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\scrptdebug.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\orxIDispatch.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\OrxDispID.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\orxscrpt.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\classfactory.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\utilities.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\OrxScrptError.obj >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\ORXSCRPT.dll >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\ORXSCRPT.exp >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\ORXSCRPT.lib >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\ORXSCRPT.map >nul 2>&1
del %SRC_DRV%%SRC_DIR%\Win32Dbg\*.log >nul 2>&1

goto END

:HELP_SRC_DRV
echo This clean, cleans only orxscrpt.
echo To use it, you have to set SRC_DRV and SRC_DIR correctly
echo in the environment.  Or, edit this *.bat file by hand

:END
