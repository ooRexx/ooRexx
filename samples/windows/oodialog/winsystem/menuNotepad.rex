/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2008 Rexx Language Association. All rights reserved.    */
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

/**
 *   Name: menuNotepad.rex
 *   Type: Example ooRexx program
 *
 *   Description:  This example displays the hierarchy of the Notepad menu.
 *
 *                 The MenuObject class in winsystm.cls allows you to work with
 *                 the menus in a Windows window.  You can do things like
 *                 simulate a user selecting a menu item.  This in turn allows
 *                 you to 'remote control' a window.  However in order to
 *                 simulate selecting a menu item you need to know either its
 *                 text or its id.  These items can be difficult to learn
 *                 becuase often times a menu item's text has hidden characters
 *                 in it and the id of menu items are usually only known by the
 *                 application developer.
 *
 *                 This examle shows how to use the MenuObject to discover the
 *                 text and id numbers of any item in the Notepad menu.
 *
 *                 The example uses some of the public functions provided by
 *                 the windowsSystem.frm package. That framework contains
 *                 functions shared with other example programs shipped with
 *                 ooRexx that use winsystm.cls.
 *
 *                 This example does not have much comment.  To understand more
 *                 of the details of starting and closing Notepad automatically,
 *                 take a closer look at the writeWithNotepad.rex example.
 *
 *   External function            Source
 *   ---------------------------------------------------
 *   getPathToSystemExe()         windowsSystem.frm
 *   errorDialog()                ooDialog
 *   findTheWindow()              windowsSystem.frm
 *   MenuDetailer                 windowsSystem.frm
 *
 * The external functions used by this program allow you to read through the
 * code here and get the over-all picture of how the program works.  To
 * understand the finer details, you will need to look at the implementation
 * of the external functions contained in windowsSystem.frm.
 */

notepadExe = getPathToSystemExe("notepad.exe")

if notepadExe == .nil then do
  msg = "Faild to locate a definitive path to the Windows" || '0d0a'x || -
        "Notepad application.  This sample can not execute."
  return errorDialog(msg)
end

"start" notepadExe

np = findTheWindow("Untitled - Notepad")
if np == .nil then do
  msg = "Could not find the Notepad application window." || '0d0a0d0a'x || -
        "This sample with have to abort."
  return errorDialog(msg)
end

-- To really understand how the MenuDetailer class works, take a look at its
-- implementation in windowsSystem.frm.

-- The MenuDetailer can either print or display the menu hierarchy.  Here we do
-- both.  print() shows the hierarchy on the console in text format.  display()
-- shows it in a graphical dialog.
md = .MenuDetailer~new(np)
md~print
md~display

-- Close the Notepad window.
np~SendSysCommand("Close")

::requires 'windowsSystem.frm'
