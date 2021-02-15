/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
 * Very, very simple example of a BinaryMenuBar.  Please use the ooDialog
 * reference manual to look up the meaning of the arguments to the methods of
 * the menu objects.
 */

   .application~useGlobalConstDir('O')

   .constDir[IDC_STATIC]       =  200
   .constDir[IDM_MENUBAR]      = 1000
   .constDir[IDM_POP_FILE]     = 1200
   .constDir[IDM_MI_LEAVE]     = 1201
   .constDir[IDM_MI_TEST_ITEM] = 1202
   .constDir[IDM_MI_CONTEXT]   = 1203
   .constDir[IDM_SEPARATOR]    = 1204
   .constDir[IDM_POP_HELP]     = 1300
   .constDir[IDM_MI_ABOUT]     = 1301

   dlg = .MenuDlg~new
   dlg~execute('ShowTop')

::requires 'ooDialog.cls'

::class 'MenuDlg' subclass Userdialog

::method init
   forward class (super) continue

   self~createCenter(160, 80, 'Dialog with a menu')

::method defineDialog

   text = "Choosing the 'Leave' menu item will close the dialog " -
          "with 'Cancel', the 'Exit' menu item will close with 'Ok.'"

   self~createStaticText(IDC_STATIC, 10, 10, 140, 60, , text)


::method initDialog
   expose staticText

   -- A subtle point here.  We can either attach automatically, or connect menu
   -- items automatically, be we can not do both.  If we attach automatically,
   -- then the menu bar is empty at this point, so there are no menu items to
   -- automatically connect.  In this case, we attach automatically, then when
   -- the menu bar is populated, we automatically connect all menu items.
   menu = .BinaryMenuBar~new(.nil, IDM_MENUBAR, , self)

   subMenu = .PopupMenu~new(IDM_POP_HELP)
   subMenu~insertItem(IDM_MI_ABOUT, IDM_MI_ABOUT, "About")

   menu~insertPopup(IDM_POP_HELP, IDM_POP_HELP, subMenu, "Help")

   subMenu = .PopupMenu~new(IDM_POP_FILE)
   subMenu~insertItem(IDOK, IDOK, "Exit")
   subMenu~insertItem(IDOK, IDM_MI_LEAVE, "Leave")
   subMenu~insertItem(IDM_MI_LEAVE, IDM_MI_TEST_ITEM, "Test Item")
   subMenu~insertItem(IDM_MI_LEAVE, IDM_MI_CONTEXT, "Context")
   subMenu~insertSeparator(IDM_MI_LEAVE, IDM_SEPARATOR)

   menu~insertPopup(IDM_POP_HELP, IDM_POP_FILE, subMenu, "File")

   menu~connectAllCommandEvents

   staticText = self~newStatic(IDC_STATIC)

::method about
   expose staticText

   staticText~setText(self~getMsg("About"))

::method testItem
   expose staticText

   staticText~setText(self~getMsg("Test Item"))

::method context
   expose staticText

   staticText~setText(self~getMsg("Context"))

::method leave
   expose staticText

   -- User will probably not see this blur by, so we sleep for a second.
   staticText~setText(self~getMsg("Leave"))
   j = SysSleep(1)

   self~cancel:super

::method getMsg private
   use strict arg item

   return "Menu item:".endOfLine~copies(2) || '9'x || item || -
          .endOfLine~copies(2) || "was last selected."


-- We over-ride the cancel method and do nothing so that the user can only
-- close the dialog through the menu.
::method cancel unguarded
   expose staticText

   msg = "You must use the 'Leave' or 'Exit' menu items to close the dialog."
   staticText~setText(msg)
