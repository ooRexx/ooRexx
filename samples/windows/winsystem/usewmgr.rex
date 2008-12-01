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

/**
 *   Name: usewmgr.rex
 *   Type: Example ooRexx program
 *
 *   Description:  Demonstrates the use of the WindowsManager, the WindowObject,
 *                 and the WindowsClipboard classes.  Has some examples of
 *                 ooDialog usage.
 *
 *                 This example uses some of the public functions provided by
 *                 the windowsSystem.frm package. That framework contains
 *                 functions shared with other example programs shipped with
 *                 ooRexx that use winsystm.cls.
 *
 *   Note:         Because of language dependencies this example will not run
 *                 correctly on all language versions of Windows.  It assumes
 *                 the window titles are in English.  To run on a different
 *                 language version, change the window titles for the Calculator
 *                 and Notepad applications to the correct title for
 *                 the specific language version of Windows.
 */

-- First, give the user a clue as to what is happening.
msg = "This ooRexx example program demonstrates controlling a" || '0d0a'x || -
      "Windows application using the winsystm.cls classes." || '0d0a0d0a'x || -
      "Information will be displayed using message boxes like" || '0d0a'x || -
      "this one.  Push the ok button in them to continue the" || '0d0a'x || -
      "demonstration." || '0d0a0d0a'x || -
      "You will see both the Calculator and Notepad programs" || '0d0a'x || -
      "appear on you screen.  There are some short pauses" || '0d0a'x || -
      "between actions.  Be patient during them.  The appli-" || '0d0a'x || -
      "cations will be closed automatically" || '0d0a0d0a'x || -
      "In particular watch the Calculator program to see the" || '0d0a'x || -
      "changes as it is remotely controlled."

j = infoDialog(msg)

-- A simple demonstration of the Windows Manager.  Get an instance of the
-- WindowsManager class.  Then, display the title and coordinates of the current
-- foreground window.
winMgr = .WindowsManager~new

fg = winMgr~ForegroundWindow
if fg \= .nil then do
  msg = "Current foreground window" || '0d0a0d0a'x || -
        "  Window title:   " fg~Title || '0d0a'x || -
        "  Window location:" fg~Coordinates
  j = infoDialog(msg)
end
else do
  msg = "The WindowsManager object failed to find the foreground window."
  j = errorDialog(msg)
end

-- A more involved demonstration using the Windows Calculator application.  The
-- real work is done in the workWithCalc() routine.
calcPrg = getPathToSystemExe("calc.exe")
if calcPrg \== .nil then do
  ret = workWithCalc(calcPrg)
end
else do
  msg = "Faild to locate a definitive path to the Windows" || '0d0a'x || -
        "Calculator application.  Going to skip this part" || '0d0a'x || -
        "of the demonstration."
  j = errorDialog(msg)
end

-- An example similar to the Windows Calculator, but using Windows Notepad.
notepad = getPathToSystemExe("notepad.exe")
if notepad \== .nil then do
  ret = workWithNotepad(notepad)
end
else do
  msg = "Faild to locate a definitive path to the Windows" || '0d0a'x || -
        "Notepad application.  Going to skip this part of" || '0d0a'x || -
        "the demonstration."
  j = errorDialog(msg)
end

return 0


/* windowsSystem.frm contains public helper routines and also requires
 * winsystm.cls and oodplain.cls.  Requiring windowsSystem.frm gives us access
 * to the class definitions for WindowsManager, WindowsClipboard, and
 * WindowObject. oodplain.cls has the class definitions for the message dialogs.
 */
::requires "windowsSystem.frm"

::routine workWithCalc
  use strict arg calcPrg

  -- Start the Calculator program.
  "start" calcPrg

  -- Find the Calculator Window, if we don't find it, then end this portion of
  -- the demonstration.
  calc = findTheWindow("Calculator")
  if calc == .nil then return windowFailure("Calculator")

  -- Make the calculator the top-most window.
  calc~ToForeground
  call SysSleep 2

  -- Switch to the scientific view.
  calc~ProcessMenuCommand("&View","&Scientific")
  call SysSleep 2

  /* When the view of the Calculator is changed, it gets a new window handle.
   * Because of this, after each view change, the calculator must be found
   * again.  This is an oddity of the Calculator application.  Most applications
   * maintain the same window handle through out their execution.
   */
  calc = findTheWindow("Calculator")
  if calc == .nil then return windowFailure("Calculator")
  calc~ToForeground

  /* Each button in a dialog has a resource ID.  When the button is pushed or
   * clicked, the button sends a command (WM_COMMAND) notification to its
   * parent window.  The command notification contains the resource ID of the
   * button that was pushed.  This is how the dialog knows what button(s) the
   * user pushes or clicks.
   *
   * We can simulate a user pushing buttons by sending commands to the dialog
   * using the resource ID of the button we want pushed.  Here we define
   * variables with the IDs of some of the buttons in the calculator
   * application.  This makes the code easier to read.  You can verify that
   * these are the correct ID numbers by using the displayWindowTree.rex program
   * included with this distribution of ooRexx.
   */

  divide = 90
  multi  = 91
  plus   = 92
  minus  = 93
  point  = 85
  equals = 112
  one    = 125
  three  = 127
  five   = 129
  seven  = 131

  /* Create a clipboard object */
  cb = .WindowsClipboard~new

  /* Switch between Scientific and Standard view using the calculator's menu */
  calc = findTheWindow("Calculator")
  if calc == .nil then return windowFailure("Calculator")

  calc~ProcessMenuCommand("&View","S&tandard")
  call SysSleep 2

  calc = findTheWindow("Calculator")
  if calc == .nil then return windowFailure("Calculator")

  calc~ProcessMenuCommand("&View","&Scientific")
  call SysSleep 2

  calc = findTheWindow("Calculator")
  if calc == .nil then return windowFailure("Calculator")

  calc~Title = "Calculator - Open Object Rexx is in charge!"

  -- Do a few calculations and retrieve the results via the clipboard
  calc~sendCommand(seven)
  calc~sendCommand(plus)
  calc~sendCommand(five)
  calc~sendCommand(equals)

  -- Empty clipboard so that we can wait until data is available
  cb~empty

  /* Copy result to clipboard, wait until the data is available from the
   * clipboard and then use paste to display the result.
   */
  calc~processMenuCommand("&Edit", "&Copy")
  do while cb~IsDataAvailable = 0; nop; end
  msg = "Calculator says: 7+5="cb~Paste
  j = infoDialog(msg)

  call SysSleep 2
  calc~sendCommand(divide)
  calc~sendCommand(one)
  calc~sendCommand(Point)
  calc~sendCommand(five)
  calc~sendCommand(equals)
  cb~Empty

  calc~processMenuCommand("&Edit", "&Copy")
  do while cb~IsDataAvailable = 0; nop; end
  msg = "Calculator says: 7+5/1.5="cb~Paste
  j = infoDialog(msg)

  calc~sendCommand(multi)
  calc~sendCommand(three)
  calc~sendCommand(equals)
  cb~Empty

  calc~processMenuCommand("&Edit","&Copy")
  do while cb~IsDataAvailable = 0; nop; end
  msg = "Calculator says: 7+5/1.5*3="cb~Paste
  j = infoDialog(msg)

  call SysSleep 2

  -- Switch to hexadecimal mode.
  calc~pushButton("Hex")
  cb~empty
  calc~processMenuCommand("&Edit","&Copy")
  do while cb~isDataAvailable = 0; nop; end
  msg = "Calculator says: 7+5/1.5="cb~Paste " (HEX)"
  j = infoDialog(msg)

  call SysSleep 2

  -- Switch back to decimal mode.
  calc~pushButton("Dec")

  -- Copy a number to the clipboard and from there to the calculator to
  -- demonstrate another way to enter numbers.
  cb~copy("123456789")
  calc~processMenuCommand("&Edit", "&Paste")
  call SysSleep 2

  -- Simulate a left mouse button click on the "Grads" radio button.
  gradsRB = findDescendent(calc, "Grads")
  if gradsRB \== .nil then do
    gradsRB~sendMouseClick("LEFT", "DOWN", 2, 2)
    gradsRB~sendMouseClick("LEFT", "UP", 2, 2)
  end

  call SysSleep 2

  -- Close the Calculator application by pressing Close in the system menu.
  calc~sendSysCommand("Close")

return 0

::routine workWithNotepad
  use strict arg notePrg

  -- Start the Notepad program.
  "start" notePrg

  np = findTheWindow("Untitled - Notepad")
  if np == .nil then return windowFailure("Notepad")

  -- Move Notepad to foreground.
  np~toForeground

  -- The edit window is the first (and perhaps only) child window of Notepad.
  npe = np~firstChild

  -- The sendText() method sets the text of a window.
  npe~sendText("Hello,")

  -- The sendKey() method sends a key press to a window.  This can be used to
  -- send virtual key pressess to the window.
  npe~sendKey("RETURN")

  -- Because we are sending messages to an application running in a separate
  -- process, on today's fast, multi-threaded, multi-cored, systems, it is
  -- possible for the messages to arrive in the Notepad application in a
  -- different order than they are being sent here.  This was probably not
  -- possible on Windows 3.0 when this example was first developed.
  --
  -- To avoid this out of order messaging, we use a helper function from the
  -- windowsSystem.frm framework.
  j = sendTextWithPause(npe, "This is a sample that mainly demonstrates the use")
  j = sendKeyWithPause(npe, "RETURN")
  j = sendTextWithPause(npe, "of the WindowsManager class.")
  j = sendKeyWithPause(npe, "RETURN")
  j = sendKeyWithPause(npe, "RETURN")

  j = sendTextWithPause(npe, "It also introduces the WindowObject, the MenuObject,")
  j = sendKeyWithPause(npe, "RETURN")
  j = sendTextWithPause(npe, "and the WindowsClipboard classes.")
  j = sendKeyWithPause(npe, "RETURN")
  j = sendKeyWithPause(npe, "RETURN")

  npe~sendText("In 1 second intervals I'm going to minimize, maximize, restore,")
  j = sendKeyWithPause(npe, "RETURN")
  npe~sendText("disable, and reenable Notepad.")
  j = sendKeyWithPause(npe, "RETURN")

  call SysSleep 3
  np~minimize
  call SysSleep 1
  np~maximize
  call SysSleep 1
  np~restore
  call SysSleep 1

  -- Change the size of the Notepad window.
  np~resize(650, 280)

  -- Make the main window (including the menu) inaccessible.
  np~disable

  j = sendKeyWithPause(npe, "RETURN")
  j = sendKeyWithPause(npe, "RETURN")
  j = sendTextWithPause(npe, "Select a menu item. It won't be possible.")
  call SysSleep 5

  -- Re-enable the main window.
  np~enable

  j = sendKeyWithPause(npe, "RETURN")
  j = sendTextWithPause(npe, "Now the menu is enabled again.")
  call SysSleep 1

  j = sendKeyWithPause(npe, "RETURN")
  j = sendTextWithPause(npe, "You can do the same with the editor window.")

  -- We disable the edit window so it gets grayed.
  npe~disable
  call SysSleep 4

  npe~enable
  j = sendKeyWithPause(npe, "RETURN")
  npe~sendText("Now let's hide the editor window.")
  call SysSleep 2

  -- Let's hide the edit window (not the main window) for a moment.e */
  npe~hide
  np~restore
  call SysSleep 2

  npe~restore
  npe~sendKey("RETURN")
  npe~sendKey("RETURN")
  call SysSleep 3
  npe~sendKey("RETURN")
  npe~sendKey("RETURN")
  npe~sendText("The show is over. Good bye in 3 seconds!")

  -- Get a WindowsManager object, we'll use it below to cancel the Save dialog.
  winMgr = .WindowsManager~new

  -- On Vista, the interface to Notepad is slightly changed.
  if isVistaOrLater() then buttonName = "Do&n't Save"
  else buttonName = "&No"
  call SysSleep 2

  -- Close Notepad by pressing the Close menu item in the system menu.
  np~sendSysCommand("Close")

  -- Wait a second for the dialog that asks whether to save the changes.
  call SysSleep 1
  fg = winMgr~foregroundWindow
  if fg \== .nil, fg~title = "Notepad" then do
    call SysSleep .1

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

/** windowFailure()
 * A simple helper function to put up an error message box.
 */
::routine windowFailure
  use strict arg name

  msg = "Failed to locate the" name "application window." || '0d0a0d0a'x || -
        "Ending the" name "demonstration prematurely."
  j = errorDialog(msg)

return .false

