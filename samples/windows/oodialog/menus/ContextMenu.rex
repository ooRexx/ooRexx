/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2011 Rexx Language Association. All rights reserved.    */
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

/* Demonstrates some of the Menu features in ooDialog.  This example focuses
 * on context menus.  Two different context menus are used in this example,
 * showing 2 different ways to create the context menus.
 *
 * One context menu is created by loading a menu from a resource script file.
 * Menus created from a resouce script are always menu bars.  In this example,
 * the menu bar is *not* attached to the dialog, it is simply used as the source
 * of the context menu.
 *
 * The second context menu is created dynamically using methods of the PopupMenu
 * class.
 */

  -- When using symbolic IDs with menus, as this program does, the programmer
  -- *must* use the global constant directory (.constDir.)  The use of the
  -- global constant directory is controlled by the .Application object.  The
  -- .Application object is also used to set other application-wide defaults.
  -- In this program, we also want to make the default for automatic data
  -- detection off.  Both of these requirements can be done with one method call
  -- to the .Application object.
  .application~setDefaults("O", 'ContextMenu.h', .false)

  -- This demonstrates that menu objects are distinct from dialogs.  You can
  -- create a menu completely separate from a dialog.  The menu object could
  -- then be used whereever it is convenient.
  --
  -- The code for creating this context menu is put in a separate function so
  -- that people can focus on only one thing at a time.  In the
  -- createListViewMenu() the context menu is created dynamically and passed
  -- back.
  contextMenu = createListViewMenu()

  -- Create a RcDialog as normally done.
  dlg = .ContextMenuDlg~new("ContextMenu.rc", IDD_CONTEXT)

  -- putMenu() is a method added to our dialog class.  It is used to pass the
  -- context menu object to the dialog.  There are any number of ways to
  -- accomplish this.  For instance the menud object could have been passed in
  -- as an argument to new() above.
  dlg~putMenu(contextMenu)

  -- Excuted the dialog as usual.
  if dlg~initCode = 0 then do
    dlg~execute("SHOWTOP")
  end
  else do
    say 'Error creating the context menu dialog initCode:' dlg~initCode
  end

return 0
-- End of entry point.

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::requires "ooDialog.cls"

::class 'ContextMenuDlg' subclass RcDialog

::method init
  expose lvPopup menuBar
  use arg rcScript, dlgID
  forward class (super) continue

  -- A menu bar can be loaded from a resource script file using the
  -- .ScriptMenuBar class.  This menu bar is created from the same resource
  -- script as the dialog is created from argument.  The menu with symbolic
  -- resource id of: IDM_RC_DLG is loaded into memory.
  menuBar = .ScriptMenuBar~new(rcScript, IDM_RC_DLG)

  -- If an error occurs in a menu object initialization, then a condition should
  -- have been raised.
  if menuBar == 0 then do
    say 'ERROR: menubar shold be good, not 0.'
    say 'A condition should have been raised.'
    say 'SystemErrorCode:' .SystemErrorCode SysGetErrortext(.SystemErrorCode)
    menuBar = .nil
  end

  -- Set the list view context menu to .nil for now.
  lvPopup = .nil


-- In initDialog() the underlying Windows objects are created.  We can now work
-- with them.
::method initDialog
  expose lvPopup menuBar dlgPopup

  dlgPopup = menubar~getPopup(1, .true)

  self~makeMenuConnections

  lvControl = self~getListControl(IDC_LV)
  lvHwnd = lvControl~hwnd
  say 'Dialog hwnd:   ' self~hwnd
  say 'List View hwnd:' lvHwnd
  say


::method onListViewContext
  expose lvPopup
  use arg hwnd, x, y
  say 'onListViewContext() hwnd:' hwnd 'x:' x 'y:' y

  ret = lvPopup~show(.Point~new(x, y), , , .true)
  if ret == -1 then do
    say 'lvPopup~show() failed SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end


::method onContextMenu
  expose dlgPopup
  use arg hwnd, x, y

  say 'onContextMenu() hwnd:    ' hwnd 'x:' x 'y:' y

  ret = dlgPopup~show(.Point~new(x, y), self)
  if ret == -1 then do
    say 'dlgPopup~show() failed SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end

::method onShowMenu unguarded
  use arg id
  say 'got menu command from id:' id '('self~resolveNumericID(id)')'

::method makeMenuConnections private
  expose lvPopup menuBar dlgPopup

  -- Now connect a click on a menu item in the short cut menus to this dialog.
  -- Here we just connect all items to one method, but you could connect some
  -- or all of the menu items to different methods.
  say 'Connecting all menu items in the dialog short cut menu to "onShowMenu()":'
  if \ menuBar~connectAllCommandEvents(onShowMenu, self) then do
    say 'Error conecting menu items. SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end

  say 'Connecting all menu items in the list-view short cut menu to "onShowMenu()":'
  if \ lvPopup~assignTo(self, .true, onShowMenu) then do
    say 'Error conecting menu items. SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end
  say

  -- Connect the context menu message (right mouse click) with two methods.  If
  -- you supply a window handle as the second, optional,  parameter, then the
  -- notifications are filtered by that window handle.  In this way you can show
  -- a context menu specific to a certain control.
  --
  -- Here we send all right-clicks on the list-view control to one method and
  -- all clicks on the dialog to a second method.  (Right clicks on the two
  -- buttons are ignored.
  if \ lvPopup~connectContextMenu(onListViewContext, self~getListControl(IDC_LV)~hwnd) then do
    say 'Error conecting context menu. SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end

  if \ dlgPopup~connectContextMenu(onContextMenu, self~hwnd, self) then do
    say 'Error conecting context menu. SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end



-- The putMenu() method simply provides a convenient way to pass in the menu
-- object created in the program entry code.
::method putMenu
  expose lvPopup
  use strict arg lvPopup


::routine createListViewMenu
  use strict arg

  m = .PopupMenu~new(IDM_LV_BAR)

  m~insertItem(IDM_LV_PROFESSION, IDM_LV_PROFESSION, "Order by Profession")
  m~insertItem(IDM_LV_PROFESSION, IDM_LV_FNAME, "Order by First Name")

  m~insertItem(IDM_LV_FNAME, IDM_LV_REVERSE, "Reverse Sort")
  m~insertItem(IDM_LV_REVERSE, IDM_LV_SORT, "Sort")
  m~insertItem(IDM_LV_REVERSE, IDM_LV_JUMBLE, "Jumble")

  m~insertItem(IDM_LV_PROFESSION, IDM_LV_LNAME, "Order by Last Name")

  m~insertSeparator(IDM_LV_FNAME, IDM_LV_SEP1)

  return m
