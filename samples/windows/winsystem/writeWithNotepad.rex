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
 *   Name: writeWithNotepad.rex
 *   Type: Example ooRexx program
 *
 *   Description:  An example of using the winsystm.cls classes to control the
 *                 Notepad application.  It is a subset of the usewmgr.rex
 *                 example, intended to be simplier and maybe easier to see what
 *                 is happening.
 *
 *                 This example uses some of the public functions provided by
 *                 the windowsSystem.frm package. That framework contains
 *                 functions shared with other example programs shipped with
 *                 ooRexx that use winsystm.cls.
 *
 *   External function            Source
 *   ---------------------------------------------------
 *   getPathToSystemExe()         windowsSystem.frm
 *   errorDialog()                ooDialog
 *   findTheWindow()              windowsSystem.frm
 *   sendTextWithPause()          windowsSystem.frm
 *   sendKeyWithPause()           windowsSystem.frm
 *   isVistaOrLater()             windowsSystem.frm
 *   getWindowTree()              windowsSystem.frm
 *   showWindowTree()             windowsSystem.frm
 *   findDescendent()             windowsSystem.frm
 *
 * The external functions used by this program allow you to read through the
 * code here and get the over-all picture of how the program works.  To
 * understand the finer details, you will need to look at the implementation
 * of the external functions contained in windowsSystem.frm.
 */

-- Get the full path to notepad to be sure we start the right application.
notepadExe = getPathToSystemExe("notepad.exe")

-- Quit if we can't get the path.  Let the user know why we are quitting.
if notepadExe == .nil then do
  msg = "Faild to locate a definitive path to the Windows" || '0d0a'x || -
        "Notepad application.  This sample can not execute."
  return errorDialog(msg)
end

-- This will start notepad as a separate process.
"start" notepadExe

-- When Notepad is started without a file name as an argument, its title will be
-- 'Untitled - Notepad'  We try to find that window, quitting if we can not.
-- Note that findTheWindow() tries several times to locate the window, but will
-- eventually time out.  Again, we let the user know why we are quitting if we
-- do.

np = findTheWindow("Untitled - Notepad")
if np == .nil then do
  msg = "Could not find the Notepad application window." || '0d0a0d0a'x || -
        "This sample will have to abort."
  return errorDialog(msg)
end

-- Make sure the size of the Notepad window is wide enough for our text.
np~resize(650, 500)

-- Move Notepad to the foreground.
np~ToForeground

-- Get the edit window, which is the first (and perhaps only) child window of
-- the Notepad window.
editWnd = np~FirstChild

-- Put some text into Notepad, if the user reads it, she will have a clue as to
-- what is going to happen.
j = SysSleep(.01)
j = sendTextWithPause(editWnd, "Hello,")
j = sendKeyWithPause(editWnd, "RETURN")
j = sendKeyWithPause(editWnd, "RETURN")
j = sendTextWithPause(editWnd, "When the Notepad Save dialog window opens, wait a second or two.  ")
j = sendKeyWithPause(editWnd, "RETURN")
j = sendTextWithPause(editWnd, "Then a dialog window will open that will allow you to explore the")
j = sendKeyWithPause(editWnd, "RETURN")
j = sendTextWithPause(editWnd, "window hierarchy of the Save dialog.  When you close that dialog, ")
j = sendKeyWithPause(editWnd, "RETURN")
j = sendTextWithPause(editWnd, "Notepad will close automatically.  (If things work correctly.)")
j = sendKeyWithPause(editWnd, "RETURN")
j = sendKeyWithPause(editWnd, "RETURN")
j = sendTextWithPause(editWnd, "The show is over. Good bye in several seconds.")
j = sendKeyWithPause(editWnd, "RETURN")

-- Get a WindowsManager object, we'll use it below to cancel the Save dialog.
winMgr = .WindowsManager~new

-- The Notepad interface is much changed in Vista.
if isVistaOrLater() then buttonName = "Do&n't Save"
else buttonName = "&No"

-- Pause a little bit to allow the user to read the text.
j = SysSleep(5)

-- Close Notepad by using the Close item in the system menu of the application.
np~SendSysCommand("Close")

/* When Notepad closes, it will put up a dialog asking if we want to save the
 * changes we just wrote to a file.  We will wait a second for the dialog and
 * then use the No button (Don't Save button in Vista) to discard our text.
 *
 * Vista has made this rather interesting.  With the Vista Notepad Save dialog,
 * the buttons are not immediate children of the dialog.  Rather the buttons are
 * several layers deep in the window hierarchy.  Therefore we need to use the
 * findDescendent() function to locate the proper button.
 *
 * Initially, functions like getWindowTree() and showWindowTree() or
 * printWindowTree() were very useful in discovering the interface changes in
 * Notepad on Vista.
 */

-- Wait a second and then grab the foreground window.  If we get a window and
-- the window has the title of Notepad, we will assume it is the Save dialog.
-- If we encounter errors, inform the user as to what they were.
j = SysSleep(1)
fg = winMgr~ForegroundWindow
if fg \== .nil, fg~title = "Notepad" then do
  j = SysSleep(.1)

  -- Display the window hierarchy of the Save dialog.
  tree = getWindowTree(fg)
  success = showWindowTree(tree)

  -- Get the No or Don't Save button.  Then set the focus to the button and
  -- send the space key to it.  This simulates a user pushing the button and
  -- has the effect of closing Notepad automatically.
  button = findDescendent(fg, buttonName)
  if button \== .nil then do
    fg~focusItem(button)
    button~SendKey("SPACE")
  end
  else do
    msg = 'Could not locate the' buttonName~changestr('&', "") 'button used to ' || '0d0a'x || -
          'discard the text changes.  findDescendent() failed' || '0d0a0d0a'x || -
          'You will have to close the Notepad windows manually.'
    j = errorDialog(msg)
  end
end
else do
  msg = 'Failed to connect with the Save Dialog for Notepad' || '0d0a0d0a'x || -
        'You may have to close the Notepad windows manually.'
  j = errorDialog(msg)
end

return 0

::requires "windowsSystem.frm"
