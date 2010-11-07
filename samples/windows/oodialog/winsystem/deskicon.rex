/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.    */
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
/*****************************************************************************/
/* Name: DESKICON.REX                                                        */
/* Type: Open Object Rexx Script                                             */
/*                                                                           */
/* Description: Sample how to create a desktop icon using the AddDesktopIcon */
/*              method from the WindowsProgramManager class                  */
/*                                                                           */
/*****************************************************************************/

pm = .WindowsProgramManager~new

/* Create a shortcut to notepad editor with shortcut key CONTROL+ALT+N.
 * The shortcut will be a personal shortcut for the current user.  "PERSONAL"
 * is the default.
 */
rc = pm~AddDesktopIcon("My Notepad 1","%SystemRoot%\system32\notepad.exe",,,"%HOMEDRIVE%%HOMEPATH%",,,"n")

if rc then call errorDialog "Error.  Could not create the 'My Notepad 1' shortcut."

/* Create a shortcut to run REXXTRY, use the REXX.ICO icon, working diretory is
 * %TEMP%, th argument is REXXTRY, the shortcut key is CTRL+ALT+T. The shortcut
 * should be common for all users, but on Vista you can not write to the all
 * users area unless you have elevated Administrator pivileges.
 */
parse value SysVersion() with name version
if version >= 6 then do
  msg = "The next shortcut is intended to be an 'All Users' shortcut." || '0d0a0d0a'x || -
        "But, on Vista you can not write to the 'All Users' area without" || '0d0a'x || -
        "elevated privileges.  If you run this program with elevated" || '0d0a'x || -
        "privileges, then creating an 'All Users' shortcut will succeed," || '0d0a'x || -
        "otherwise it will fail.  You can choose to create an 'All Users'" || '0d0a'x || -
        "shortcut, or a 'Personal' shortcut.  (Creating a 'Personal'" || '0d0a'x || -
        "shortcut will always succeed.)"|| '0d0a0d0a'x || -
        "Create an 'All Users' shortcut?"

  isYes = askDialog(msg)
  if isYes then location = "COMMON"
  else location = "PERSONAL"
end
else do
  location = "COMMON"
end

rc = pm~AddDesktopIcon("RexxTry","rexx.exe","rexx.ico",0,"%TEMP%",location,"rexxtry","T","MAXIMIZED")

if rc then call errorDialog "Error.  Could not create the 'RexxTry' shortcut."

/* Create a shortcut to NOTEPAD editor, with the working diretory as c:\temp.
 * The shortcut will be a personal shortcut for the current user.
 */
rc = pm~AddDesktopIcon("My Notepad 2","notepad.exe", , ,VALUE( 'TEMP',, 'ENVIRONMENT' ) || '\',"PERSONAL", , ,"MAXIMIZED")

if rc then call errorDialog "Error.  Could not create the 'My Notepad 2' shortcut."

return 0

::requires "winsystm.cls"
::requires "ooDialog.cls"
