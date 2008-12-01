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
 *   Name: menuCalc.rex
 *   Type: Example ooRexx program
 *
 *   Description:  Demonstrates how to control an application programmatically
 *                 using the winsystm classes.  Does some simple calculations
 *                 using the Windows Calculator application.
 *
 *                 This program is similar to userwmgr.rex but more simple to
 *                 better show what is going on.
 *
 *                 This example uses some of the public functions provided by
 *                 the windowsSystem.frm package.  That framweowork is an
 *                 example of how to extract common function into a package and
 *                 share that function with any number of ooRexx programs.
 *
 *   To really understand how this program works, you will need to examine the
 *   the code for the external functions in windowsSystem.frm.
 *
 *   The table below lists the external functions and classes used in this
 *   program and shows where to go to examine the source code for them.
 *
 *   External function            Source
 *   ---------------------------------------------------
 *   errorDialog()                ooDialog
 *   infoDialog()                 ooDialog
 *   getPathToSystemExec()        windowsSystem.frm
 *   findTheWindow()              windowsSystem.frm
 */

-- Get the complete path to the calculator executable.  This ensures we start
-- the correct application.
calcExe = getPathToSystemExe("calc.exe")

-- Abort if we have an error.
if calcExe == .nil then do
  msg = "Faild to locate a definitive path to the Windows" || '0d0a'x || -
        "Calculator application.  This sample can not execute."
  return errorDialog(msg)
end

-- Start calculator as a separate process.
"start" calcExe

-- It takes some finite period of time for an application to start.  This can
-- vary substantially depending on the system load.  findTheWindow() allows for
-- this, but will eventually time out if the window can not be located.
calc = findTheWindow("Calculator")
if calc == .nil then do
  msg = "Could not find the Calculator application window." || '0d0a0d0a'x || -
        "This example program will have to abort."
  return errorDialog(msg)
end

-- Make the calculator the top-most window.
calc~ToForeground

/* Each button in a dialog has a resource ID.  When the button is pushed or
 * clicked, the button sends a command (WM_COMMAND) notification to its parent
 * window.  The command notification contains the resource ID of the button that
 * was pushed.  This is how the dialog knows what button(s) the user pushes or
 * clicks.
 *
 * We can simulate a user pushing buttons by sending commands to the dialog
 * using the resource ID of the button we want pushed.  Here we define variables
 * with the IDs of some of the buttons in the calculator application.  You can
 * verify that these are the correct ID numbers by exploring the window hiearchy
 * of the calculator application using the displayWindowTree.rex example program
 * that is included with the ooRexx distribution.
 */

divide   = 90
multiply = 91
plus     = 92
minus    = 93
equals   = 112

clear     = 81
memClear  = 113
memReturn = 114
memSave   = 115

zero   = 124
one    = 125
two    = 126
three  = 127
four   = 128
five   = 129
six    = 130
seven  = 131
eight  = 132
nine   = 133

-- We also want to get a WindowObject that represents the answer field in the
-- calculator application so that we can get the answer.  It so happens that
-- this is the first child window of the calculator.
answerField = calc~firstChild

-- Check that we got the right window, abort on error.  The answer field is a
-- window whose class is EDIT.
if \ answerField~isA(.WindowObject) | answerField~wClass \== "Edit" then do
  msg = "Failed to locate the answer field in the Calculator application." || '0d0a0d0a'x || -
        "This example program will have to abort."
  j = errorDialog(msg)

  -- Try to close the calculator application, although we may be hosed and this
  -- won't work.
  calc~sendSysCommand("Close")
  return 99
end

-- We will calculate 10 * 16.  After each button push we will pause a bit so
-- that the user can see the numbers appear on the calculator.  Then we will get
-- the answer and display a message box showing the whole thing.  This starts
-- our message.
msg = "10 * 16 ="

calc~sendCommand(one)
j = SysSleep(.5)
calc~sendCommand(zero)
j = SysSleep(.5)
calc~sendCommand(multiply)
j = SysSleep(.5)
calc~sendCommand(one)
j = SysSleep(.5)
calc~sendCommand(six)
j = SysSleep(.5)
calc~sendCommand(equals)
j = SysSleep(.5)

-- Now get the answer.  The 'title' of a window is the text of the window.  In
-- this case the 'title' of the answer field window is the answer of the
-- calculation.
msg = msg answerField~title
j = infoDialog(msg)

-- Push the clear button
calc~sendCommand(clear)

-- Do one more calculation.
msg = "(5 + 5) * 16 / (2 + 1) ="

calc~sendCommand(memClear)

calc~sendCommand(two)
j = SysSleep(.5)
calc~sendCommand(plus)
j = SysSleep(.5)
calc~sendCommand(one)
j = SysSleep(.5)
calc~sendCommand(equals)
j = SysSleep(.5)

calc~sendCommand(memSave)

calc~sendCommand(five)
j = SysSleep(.5)
calc~sendCommand(plus)
j = SysSleep(.5)
calc~sendCommand(five)
j = SysSleep(.5)
calc~sendCommand(equals)
j = SysSleep(.5)
calc~sendCommand(multiply)
j = SysSleep(.5)
calc~sendCommand(one)
j = SysSleep(.5)
calc~sendCommand(six)
j = SysSleep(.5)
calc~sendCommand(equals)
j = SysSleep(.5)

calc~sendCommand(divide)
calc~sendCommand(memReturn)
calc~sendCommand(equals)

-- Now get the answer.
msg = msg answerField~title
j = infoDialog(msg)

-- Close the calculator by pressing Close in the system menu.
calc~sendSysCommand("Close")

::requires 'windowsSystem.frm'
