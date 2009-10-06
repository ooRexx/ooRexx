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
 *   Name: displayWindowTree.rex
 *   Type: Example ooRexx program
 *
 *   Description:  Allows the user to pick an open top-level window and then
 *                 displays the window hiearchy for that window.  The program
 *                 uses both ooDialog and winsystm classes.
 *
 *                 This example uses some of the public functions provided by
 *                 the windowsSystem.frm package. That framework contains
 *                 functions shared with other example programs shipped with
 *                 ooRexx that use winsystm.cls.
 *
 *   External function            Source
 *   ---------------------------------------------------
 *   errorDialog()                ooDialog
 *   getWindowTree()              windowsSystem.frm
 *   showWindowTree()             windowsSystem.frm
 *
 */

-- Create and show our ooDialog dialog.  The logic of the program is contained
-- within the WindowListDlg class.
dlg = .WindowListDlg~new("winSystemDlgs.rc", IDD_WINDOW_List, , "winSystemDlgs.h")
if dlg~initCode == 0 then do
  dlg~execute("SHOWTOP")
  dlg~deinstall
  return 0
end
else do
  msg = "Failed to instantiate the .WindowListDlg object." || '0d0a0d0a'x || -
        "Aborting on error."
  j = errorDialog(msg)
  return 99
end


::requires "windowsSystem.frm"

::class 'WindowListDlg' public subclass RcDialog

/** initAutoDetection()
 * Prevent ooDialog from fooling with the initialization of our dialog controls.
 * We will initialize the controls the way we want them.
 */
::method initAutoDetection
  self~noAutoDetection


/** initdialog()
 * Initialize the state of our dialog controls.
 */
::method initDialog
  expose listView windows inError

  -- If some un-anticipated error happens we will change the behaviour of the
  -- dialog.  inError is used to flag an error.
  inError = .false

  -- Instantiate an array that will be used to hold all the top-level window
  -- objects.
  windows = .array~new

  -- If we can't get the list view control, something is wrong.
  listView = self~newListView(IDC_LV_WINDOWS)
  if listView == .nil then return self~putInErrorState("NOLISTVIEW")

  -- The extended list view styles can only be added after the list view control
  -- has been created.
  --
  -- The FULLROWSELECT style highlights the entire row when row is selected
  -- rather than just part of the row.
  --
  -- The LABLETIP style will cause a tool-tip like label to appear that displays
  -- the entire text of the column, if the text is longer than will fit in the
  -- column, when the user puts their mouse over the row.
  listView~addExtendedStyle("FULLROWSELECT LABELTIP")

  -- We want to set the width of column 0, (the first column,) to the width of
  -- the list view control, minus the width of a scroll bar if there is one.
  --
  -- We can find out the width of a scroll bar from the system metrics and can
  -- query the width of the list view control directly.
  --
  -- Note that the system metric figure is in pixels and the column width needs
  -- to be in dialog units.  Therefore the width of the scroll bar is more than
  -- we actually need, but it is close enough.
  parse value listView~getSize with width height

  SM_CXVSCROLL = 20
  width -= SystemMetrics(SM_CXVSCROLL)

  -- Insert our one and only column for the list view control.
  listView~insertColumn(0, "Window Title", width)

  -- Connect the push buttons to our event handling methods.
  self~connectButton(IDC_PB_SHOW, onShow)
  self~connectButton(IDC_PB_QUIT, onQuit)
  self~connectButton(IDC_PB_REFRESH, onRefresh)

  -- The logic / code to fill the list view control is in the onRefresh()
  -- method.  So, we simply invoke the method ourselves to populate the list
  -- view.
  self~onRefresh


/** onRefresh()
 * This method contains the logic to populate the list view control with the
 * titles of the current top-level windows.  The method is connected to the
 * Refresh button, so it is invoked when the user pushes that button.
 *
 * However, it can also be invoked internally to populate the list view.  This
 * is done when the dialog is initialized, from the initDialog() method.  And
 * it is also invokde if an invalid window handle is detected in the onShow()
 * method.
 */
::method onRefresh
  expose listView windows inError

  -- Be sure the windows array is empty
  windows~empty

  -- Be sure the list view control is empty
  listView~deleteAll

  -- Instantiate a WindowManager object and get the desktop window.
  mgr = .WindowsManager~new
  deskTop = mgr~desktopWindow

  -- Build an array of all the children windows of the desktop.  Skip any window
  -- that does not have a title or that is invisible.
  child = deskTop~firstChild
  do while child <> .nil
    if child~title~strip \== "", child~state~caselessWordpos("invisible") == 0 then windows~append(child)
    child = child~next
  end

  -- If we don't have any windows, something is wrong.
  if windows~isEmpty then return self~putInErrorState("NOWINDOWS")

  -- Now, for each window, add a row using the window title as the text for the
  -- row.
  do wnd over windows
    listView~addRow( , ,  wnd~title)
  end

  -- Select the first row, so that we are guaranteed that a row is selected.
  -- Note that the row numbers are zero-indexed.
  listView~select(0)


/** onShow()
 * The user pushed the 'Show' button.  In the unlikely event that we are in an
 * error state we will quit.  Otherwise, we will display the window hiearchy of
 * the window the user selected.
 */
::method onShow
  expose listView inError windows

  if inError then return self~cancel

  -- Get the row number selected.  Note that we are guarenteed that a row is
  -- selected because we selected a row to begin with and we have provided no
  -- way for the user to deselect a row.  Because of this we do not have to
  -- check for -1.  Since the rows are zero-indexed, we add 1 to the selected
  -- row number, to produce the index into our array of windows.
  wnd = windows[listView~selected + 1]

  -- It is conceivable / possible that the user has closed a window since the
  -- time the list view was populated.  This can be detected because the window
  -- state will be 'Illegal Handle.'
  if wnd~state == "Illegal Handle" then return self~badHandle

  -- Display the window hierarchy of the selected window.
  tree = getWindowTree(wnd)
  success = showWindowTree(tree)


/** onQuit()
 * The user pushed the 'Quit' button.  This is easy, just quit the dialog.
 */
::method onQuit
  return self~cancel


/** badHandle()
 * This private method is invoked when the user picks a window that has been
 * closed since the list view was originally populated.  We tell the user the
 * problem and refresh the list view.
 */
::method badHandle private
  expose listView

  -- Get the text of the selected item to use in the error message.
  title = listView~itemText(listView~selected)

  msg = "The window you selected appears to have been closed." || '0d0a0d0a'x || -
        "Window title:" title || '0d0a0d0a'x || -
        "The window list is being refreshed.  Select a new window."
  j = infoDialog(msg)
  self~onRefresh
  return 0


/** putInErrorState()
 * This private method is used when some un-recoverable error is detected.
 *
 * It displays an error message to the user based on the reason for the error,
 * sets the error state flag, and tries to disable the Show push button.
 */
::method putInErrorState private
  expose inError
  use strict arg reason

  inError = .true
  select
    when reason == "NOWINDOWS" then do
      msg = "For some reason no top-level windows were located." || '0d0a'x || -
            "This is not correct, the sample program is not"     || '0d0a'x || -
            "executing correctly."
    end
    when reason == "NOLISTVIEW" then do
      msg = "An internal error is preventing this sample program" || '0d0a'x || -
            "from executing correctly.  The ListControl object" || '0d0a'x || -
            "could not be obtained."
    end
    otherwise do
      msg = "The sample program is not executing correctly," || '0d0a'x || -
            "some unknown error."
    end
  end
  -- End select

  msg = msg || '0d0a0d0a'x || "Please just quit."
  j = errorDialog(msg)

  -- We will try to disable the 'Show' push button.  But, if the dialog is not
  -- working correctly, it could be that this won't work.
  pb = self~newPushButton(IDC_PB_SHOW)
  if pb <> .nil then pb~disable

  return 0
