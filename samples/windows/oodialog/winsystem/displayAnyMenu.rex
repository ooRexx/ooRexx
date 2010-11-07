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
 *   Name: displayAnyMenu.rex
 *   Type: Example ooRexx program
 *
 *   Description:  Displays the menu hierarchy for a window that the user picks.
 *
 *                 The user is prompted for the title of an open, top-level,
 *                 window.  This program then parses the details of the menubar
 *                 for that window and displays them graphically.
 *
 *                 The example uses a mixture of ooDialog and winsystm.
 *
 *                 In addition, the example uses some of the public functions
 *                 provided by the windowsSystem.frm package. That framweowork
 *                 is an example of how to extract common function into a
 *                 package and share that function with any number of ooRexx
 *                 programs.
 *
 *   The table below lists the external functions and classes used in this
 *   program and shows where to go to examine the source code for them.
 *
 *   External function            Source
 *   ---------------------------------------------------
 *   RcDialog                     ooDialog
 *   errorDialog()                ooDialog
 *   askDialog()                  ooDialog
 *   infoDialog()                 ooDialog
 *   MenuDetailer                 windowsSystem.frm
 *   findTheWindow()              windowsSystem.frm
 */

-- Prompt the user for a window title
dlg = .UserPrompt~new("winSystemDlgs.rc", IDD_USER_PROMPT, , "winSystemDlgs.h")

if dlg~initCode <> 0 then do
  msg = "Error initializing the UserPrompt dialog." || '0d0a0d0a'x || -
        "The dislplayAnyMenu example will have to quit."
  j = errorDialog(msg)
  return 99
end

-- A return of 2 means the user canceled, and we will just quit.
if dlg~execute("SHOWTOP") == 2 then return 2

-- The UserPrompt dialog validates the data, i.e. it enforces that either a
-- valid WindowObject is obtained, or that the user cancels.  Here, we ask the
-- dialog for the window object that the user picked, create a MenuDetailer
-- object using that window object, and then display the menu.

topWnd = dlg~getUserPick
md = .MenuDetailer~new(topWnd)
md~display

-- Return 0 for okay
return 0

::requires 'windowsSystem.frm'

/** class UserPrompt
 * Our user prompt dialog subclasses the RcDialog.  The RcDialog class takes
 * care of all the details of the dialog and we just over-ride the ok method
 * to take care of validating the user input.  We also add a method,
 * getUserPick(), which is used to return the window object representing the
 * window the user selected.
 */
::class 'UserPrompt' subclass RcDialog inherit AdvancedControls MessageExtensions

/** ok()
 * The over-ride of the ok() method is basically simple.  It verifies that the
 * user entered something in the edit control and then attempts to contstruct
 * a .WindowObject using the text from the edit control as the title of a
 * window.
 *
 * The method will only end the dialog with the 'ok' value if a valid window
 * object is constructed.  Otherwise it forces the user to either try again,
 * or to cancel.
 */
::method ok
  expose wnd edit

  -- Get the edit control object and then the text the user entered.  We only
  -- instantiate the edit control object the first time through.

  if \ edit~isA(.EditControl) then edit = self~newEdit(IDC_EDIT_NAME)
  title = edit~getText~strip

  -- Check that the user entered something.
  if title~length == 0 then do
    msg = "You must enter the title of a window whose menu" || '0d0a'x || -
          "you want displayed." || '0d0a0d0a'x || -
          "Would you rather cancel?"
    if askDialog(msg) then return self~cancel
    else return
  end

  -- Warn the user that constructing the window object may take a little bit of
  -- time.
  msg = "It may take a second or two find the window" || '0d0a'x || -
        "you picked.  Be patient." || '0d0a0d0a'x || -
        "This dialog will be disabled during the search."
  j = infoDialog(msg)

  -- Disable the dialog so that an impatient user can not fiddle with it.
  self~disable
  edit~disable
  self~disableItem(IDOK)
  self~disableItem(IDCANCEL)

  -- The big step, get a window object with the title entered by the user.
  wnd = findTheWindow(title)

  -- Re-enable the dialog.
  self~enable
  edit~enable
  self~enableItem(IDOK)
  self~enableItem(IDCANCEL)

  -- If we don't have a window object, give the user a chance to try again, or
  -- to cancel.  If the user does not want to try again, we end the dialog by
  -- invoking our cancel() method.
  if wnd == .nil then do
    msg = "Failed to find the window with title:" || '0d0a'x || -
          " " title || '0d0a0d0a'x || -
          "The window title has to match exactly." || '0d0a0d0a'x || -
          "Try again?"
    if askDialog(msg) then return
    else return self~cancel
  end

  -- Okay, we have a window object.  We end the dialog by inoking the superclass
  -- ok() method.
  return self~ok:super

/** cancel()
 * Just to keep things tidy, if the user cancels, we make sure that wnd is .nil.
 * Then if getUserPick() is called, .nil is returned.  Presumably, if the user
 * cancels, getUserPick() would not be invoked.  However, if someone were to
 * take this dialog class implementation and extend it, this simple detail might
 * be overlooked.
 */
::method cancel
  expose wnd
  wnd = .nil
  return self~cancel:super

/** getUserPick()  Return the window object to the caller */
::method getUserPick
  expose wnd
  return wnd
