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
 *   Description:  Displays the menu hierarchy of the Windows Calculator
 *                 application.  The menu hierachy for both the Calculator's
 *                 standard view and its scientific view are printed to the
 *                 console.
 *
 *                 This is a very simple program so that it is easeier to see
 *                 the details.
 *
 *                 The example uses a mixture of ooDialog and winsystm.
 *
 *                 In addition, the example uses some of the public functions
 *                 provided by the windowsSystem.frm package. That framweowork
 *                 is an example of how to extract common function into a
 *                 package and share that function with any number of ooRexx
 *                 programs.
 *
 *   The below table lists the external functions and classes used in this
 *   program and shows where to go to examine the source code for them.
 *
 *   External function            Source
 *   ---------------------------------------------------
 *   getPathToSystemExe()         windowsSystem.frm
 *   errorDialog()                ooDialog
 *   findTheWindow()              windowsSystem.frm
 *   MenuDetailer                 windowsSystem.frm
 */


-- Get the complete path to the calculator application to be sure we start the
-- the program we want.
calcExe = getPathToSystemExe("calc.exe")

-- If we could not get the complete path, just quit.
if calcExe == .nil then do
  msg = "Faild to locate a definitive path to the Windows" || '0d0a'x || -
        "Calculator application.  This sample can not execute."
  return errorDialog(msg)
end

-- This will start the calculator application as a separate process.
"start" calcExe

-- Get the calculator window object, quit if that fails.
calcWnd = getCalcWindow()
if calcWnd == .nil then return 99

-- First, display the menu for the 'Standard' view.  The calculator application
-- may already be in the standard view.  But, if it is not, we will first switch
-- to that view.
--
-- When the calculator program switches views, it actually creates a whole new
-- dialog window.  Because of that, if we do switch views we need to re-find the
-- window.
--
-- The way we switch to Standard view is to simulator clicking on the View menu,
-- and then clicking the Standard menu item.

if \ inStandardView(calcWnd) then do
  calcWnd~ProcessMenuCommand("&View","S&tandard")
  calcWnd = getCalcWindow()
  if calcWnd == .nil then return 99
end

-- To print the menu, we instantiate a MenuDetailer object using the Calculator
-- window object.  Then we tell the menu detailer object to 'print.'  Printing
-- sends the output to the console.

say; say "Printing the Calculator's Standard View menu"
menuDetail = .MenuDetailer~new(calcWnd)
menuDetail~print

-- Switch to the Scientific view and print that menu.  Switching to Scientific
-- view is done as above, we simulate clicking on the Scientific menut item.
--
-- When we switch views we need to re-find the window and likewise we will need
-- a new MenuDetailer object.

calcWnd~ProcessMenuCommand("&View","&Scientific")
calcWnd = getCalcWindow()
if calcWnd == .nil then return 99

say; say; say "Printing the Calculator's Scientific View menu"
menuDetail = .MenuDetailer~new(calcWnd)
menuDetail~print

-- Now close the Calculator by simulating clicking on the System menu, and then
-- clicking the Close menu item.
calcWnd~SendSysCommand("Close")

return 0

::requires 'windowsSystem.frm'

/** getCalcWindow()
 * A simple helper routine.  We need to find and re-find the calculator window
 * several times.  Each time we need to check for .nil and notify the user if
 * there is an error.
 *
 * The same steps, repeated more than once, is a good signal to put the code
 * into its own routine (or own method when using objects.)
 */
::routine getCalcWindow

  calcWnd = findTheWindow("Calculator")
  if calcWnd == .nil then do
    msg = "Could not find the Calculator application window." || '0d0a0d0a'x || -
          "This sample with have to abort."
    j = errorDialog(msg)
  end
return calcWnd

/** inStandardView()
 * Another simple helper routine.
 *
 * This tests if the calculator is in standard view by seeing if the menu item
 * for standard view is checked.
 *
 * Note that this one line is sufficient for the test:
 *
 *   return calc~menu~submenu(1)~isChecked(0)
 *
 * But, it is always possible that getting a menu or submenu will fail and
 * return .nil.  For that reason, and to make it a little easier to see what
 * the steps are, the test is in several steps.
 *
 * Note that the submenu and the menu item are obtained by position and the
 * position index is zero-based.
 */
::routine inStandardView
  use strict arg calc

  menuBar = calc~menu
  if menuBar <> .nil then do
    viewMenu = menuBar~subMenu(1)

    if viewMenu <> .nil then do
      return viewMenu~isChecked(0)
    end
  end

-- If we had an error, just return false.
return .false
