/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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

if pm~InitCode \= 0 then exit


/* Create a shortcut to notepad editor with shortcut key CONTROL+ALT+N */
/* The shortcut should be personal for the current user                */
rc = pm~AddDesktopIcon("My Notepad 1","%SystemRoot%\system32\notepad.exe",,,"%HOMEDRIVE%%HOMEPATH%",,,"n")

if rc then say "Error creating shortcut: My Notepad 1"

/* Create a shortcut to run REXXTRY, use the REXX.ICO icon, working diretory is %TEMP%           */
/* The shortcut should be common for all users, argument is REXXTRY. shortcut key is CTRL+ALT+T. */
rc = pm~AddDesktopIcon("RexxTry","rexx.exe","rexx.ico",0,"%TEMP%","COMMON","rexxtry","T","MAXIMIZED")

if rc then say "Error creating shortcut: RexxTry"

/* Create a shortcut to NOTEPAD editor, working diretory is c:\temp */
/* The shortcut should be personal for the current user             */
rc = pm~AddDesktopIcon("My Notepad 2","notepad.exe", , ,VALUE( 'TEMP',, 'ENVIRONMENT' ) || '\',"PERSONAL", , ,"MAXIMIZED")

if rc then say "Error creating shortcut: My Notepad 2"

exit 0

::requires "winsystm.cls"
