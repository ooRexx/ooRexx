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
/****************************************************************************/
/* Name       : USEWMGR.REX                                                 */
/* Type       : Object REXX Script                                          */
/* Description: Demonstrates the use of the WindowsManager,                 */
/*              the WindowObject and the WindowsClipboard class             */
/*                                                                          */
/* Note: Because of language dependencies this example will not run         */
/*       correctly on all language versions of Windows.                     */
/*       But it's easy to adapt.                                            */
/*                                                                          */
/****************************************************************************/

/* start the windows calculator */
"start calc"

/* get an instance of the WindowsManager class */
winMgr = .WindowsManager~new
if winMgr~initcode \= 0 then exit

/* display title and coordinates of the current foreground window */
fg = winMgr~ForegroundWindow
if fg \= .Nil then say "Current foreground window:" fg~Title "at" fg~Coordinates

/* find the Calculator Window */
call FindCalculator

/* switch to the scientific view */
calc~ProcessMenuCommand("&View","&Scientific")
call syssleep 2

/* because the calculator changes it's handle when the view is changed, get the window again */
call FindCalculator

/* Set a few commands of the calculator */
divide = 90
multi  = 91
plus   = 92
minus  = 93
point  = 85
System = SysVersion()
/* calculator differs on Windows 95 */
if countstr("WINDOWS95",System~Translate) \= 0 then do
  one    = "1"~c2d
  three  = "3"~c2d
  five   = "5"~c2d
  seven  = "7"~c2d
  equals = 111

end
else do
 	equals = 112
  one    = 125
  three  = 127
  five   = 129
  seven  = 131
end

/* Create a clipboard object */
cb = .WindowsClipboard~new

/* Switch between Scientific and Standard view using the calculator's menu */
call FindCalculator
calc~ProcessMenuCommand("&View","S&tandard")
call syssleep 2
call FindCalculator
calc~ProcessMenuCommand("&View","&Scientific")
call syssleep 2
call FindCalculator
calc~Title = "Calculator - Open Object Rexx is in charge!"

/* Get a list of all child windows */
cwins. = calc~EnumerateChildren

say "The calculator has the following child windows:"
do i = 1 to cwins.0
  say "    " cwins.i.!Class cwins.i.!ID "with title" cwins.i.!Title "at" cwins.i.!Coordinates
  if cwins.i.!Children = 1 then say "            -> children"
  /* Keep the handle of the "Grads" radio button */
  --if cwins.i.!Title = "Grads" | cwins.i.!Title = "Gradiends" then do
   if countstr("Grad",cwins.i.!Title) \= 0 then GradsBtn.Handle = cwins.i.!Handle
end

/* Do a few calculations and retrieve the results via the clipboard */
calc~SendMenuCommand(seven)
calc~SendMenuCommand(plus)
calc~SendMenuCommand(five)
calc~SendMenuCommand(equals)

/* Empty clipboard so that we can wait until data is available */
cb~Empty

/* Copy result to clipboard, wait until the data is available from the clipboard */
/* and us paste to display the result */
calc~ProcessMenuCommand("&Edit","&Copy")
do while cb~IsDataAvailable = 0; nop; end
say "Calculator says: 7+5="cb~Paste
call syssleep 2
calc~SendMenuCommand(divide)
calc~SendMenuCommand(one)
calc~SendMenuCommand(Point)
calc~SendMenuCommand(five)
calc~SendMenuCommand(equals)
cb~Empty
calc~ProcessMenuCommand("&Edit","&Copy")
do while cb~IsDataAvailable = 0; nop; end
say "Calculator says: 7+5/1.5="cb~Paste
calc~SendMenuCommand(multi)
calc~SendMenuCommand(three)
calc~SendMenuCommand(equals)
cb~Empty
calc~ProcessMenuCommand("&Edit","&Copy")
do while cb~IsDataAvailable = 0; nop; end
say "Calculator says: 7+5/1.5*3="cb~Paste
call syssleep 2
/* Switch to hexadecimal mode */
calc~PushButton("Hex")
cb~Empty
calc~ProcessMenuCommand("&Edit","&Copy")
do while cb~IsDataAvailable = 0; nop; end
say "Calculator says: 7+5/1.5="cb~Paste " (HEX)"
call syssleep 2

/* Switch back to decimal mode */
calc~PushButton("Dec")

/* Copy a number to the clipboard and from there to the calculator */
/* to demonstrate the easier way to enter numbers */
cb~Copy("123456789")
calc~ProcessMenuCommand("&Edit","&Paste")
call syssleep 2

/* Now simulate a left mouse button click on the "Grads" radio button */
if GradsBtn.Handle \= 0 then do
  CalcHandle = calc~Handle
  calc~AssocWindow(GradsBtn.Handle)
  calc~SendMouseClick("LEFT","DOWN",2,2)
   calc~SendMouseClick("LEFT","UP",2,2)
   calc~AssocWindow(CalcHandle)
end

call syssleep 2

calc~SendSysCommand("Close")  /* Quit calculator */

/* Work with the notepad */
"start notepad"
np = .Nil
i = 1

/* Get a WindowObject instance that is associated with Notepad */
do while np == .Nil & i < 20
   np = winMgr~Find("Untitled - Notepad")
   if np = .Nil then call SysSleep 1
   i = i + 1
end
if np = .Nil then do
    say "Sample terminated: Couldn't find notepad!"
    winMgr~Deinstall
end

/* Move Notepad to foreground */
np~ToForeground
/* the edit window is the first (and perhaps only) child of Notepad */
npe = np~FirstChild
npe~SendText("Hallo,")
/* Sends the virtual keys and some text strings to the window */
npe~SendKey("RETURN")
npe~SendText("This is a sample that mainly demonstrates the use of the WindowsManager class.")
npe~SendKey("RETURN")
npe~SendKey("RETURN")
call syssleep 1
npe~SendText("It also introduces the WindowObject, the MenuObject, and the WindowsClipboard classes.")
npe~SendKey("RETURN")
npe~SendKey("RETURN")
npe~SendKey("RETURN")
call syssleep 1
npe~SendText("In 1 second intervals I'm going to minimize, maximize, restore, disable, and reenable Notepad.")
call syssleep 3
np~Minimize
call syssleep 1
np~Maximize
call syssleep 1
np~Restore
call syssleep 1

/* Change the size of the Notepad window */
np~Resize(650, 280)

/* Make the main window (incl. menu) inaccessible */
np~Disable
npe~SendKey("RETURN")
npe~SendText("Select a menu item. It won't be possible.")
call syssleep 5
/* Re-enable the main window */
np~Enable
npe~SendKey("RETURN")
npe~SendText("Now the menu is enabled again.")
call syssleep 1
npe~SendKey("RETURN")
npe~SendText("You can do the same with the editor window.")
/* We disable the edit window so it gets grayed */
npe~Disable
call syssleep 4
npe~Enable
npe~SendKey("RETURN")
npe~SendText("Now let's hide the editor window.")
call syssleep 2
/* Let's hide the edit window (not the main window) for a moment */
npe~Hide
np~Restore
call syssleep 2
npe~Restore
npe~SendKey("RETURN")
npe~SendKey("RETURN")
call syssleep 3
npe~SendKey("RETURN")
npe~SendKey("RETURN")
npe~SendText("The show is over. Good bye!")
call syssleep 2

np~SendSysCommand("Close")  /* Close notepad */

/* Now wait a second for the dialog that asks whether to save the changes */
call syssleep 1
fg = winMgr~ForegroundWindow
if fg \= .Nil then do
  /* If it is the dialog then decline to save */
  if fg~Title = "Notepad" then fg~PushButton("&No")
end
/* Deinstall the WindowsManager and clean up */
winMgr~deinstall

exit

/* Procedure to search for the calculator window                       */
/* When the view of the calculator is changed, it get's a new handle.  */
/* So after each view change, the calcualator n+must be seacrhed again */
/* This is a specialty of the calculator                               */
FindCalculator: procedure EXPOSE calc winMgr

i = 0
calc = .Nil
/* Get a WindowObject instance that is associated with the calculator */
do while calc == .Nil & i < 20  /* wait max. 20 seconds to find it */
  calc = winMgr~Find("Calculator")
  if calc = .Nil then call SysSleep 1
  i = i + 1
end
/* Display error message if the calculator window was not found */
if calc = .Nil then do
  say "Sample terminated: Couldn't find calculator!"
  winMgr~Deinstall
end
else
  /* Make the calculator the top-most window */
  calc~ToForeground

return

/* access class definitions */
::requires "WINSYSTM.CLS"
