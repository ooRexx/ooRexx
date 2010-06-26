/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2010 Rexx Language Association. All rights reserved.    */
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
 * Name: oograph.rex
 * Type: Open Object REXX Script
 *
 * Description: A dialog that allows the user to execute two graphical ooRexx
 *              examples.  One example displays bitmaps and the other example
 *              shows how to use the drawing methods provided by ooDialog.
 *
 *              The main program, this program, shows how to use the
 *              scrollBitmapFromTo() and scrollInButton() methods.
 */

   parse source . . me
   mydir = me~left(me~lastpos('\')-1)              /* where is code     */
   env = 'ENVIRONMENT'
   sp = value('SOUNDPATH', , env)
   sp = value('SOUNDPATH', mydir'\WAV;'sp, env)

   d = .GraphDialog~new()
   if d~initCode \= 0 then do
      say "Dialog init did not work"
      return d~initCode
   end

   title = "Graphical Demonstration of Open Object Rexx and ooDialog Capabilities"
   d~createCenter(trunc(770 / d~FactorX), trunc(470 / d~FactorY), title)
   d~execute("SHOWTOP")

   return 0

/*-------------------------------- requires --------------------------*/

::requires "ooDialog.cls"

/*-------------------------------- dialog class ----------------------*/

::class 'GraphDialog' subclass UserDialog

::method defineDialog
   expose but2size
   but2pos  = trunc(170 / self~factorY)
   but2size = trunc(300 / self~factorY)

   -- The two bitmap buttons are created larger than they need to be.  In particular,
   -- The 102 button height is much larger, it covers most of the lower part of the
   -- dialog.
   --
   -- Then, the bitmaps for the buttons are positioned (moved from the upper left corner
   -- of the button) by a large amount.  The 101 button is positioned far to the right,
   -- and the 102 button is positioned far to the bottom and far to the left.  In the
   -- showInterface() method, scrollBitmapFromTo() is used to scroll the bitmaps from
   -- their positions back to the upper left corner of the buttons.  This gives the 101
   -- button the appearance of scrolling from the right to the left, and the 102 button
   -- the appearance of scrolling from the bottom to the top.

   self~createBitmapButton(101, 1, 10, self~sizeX-1, trunc(130 / self~factorY), "USEPAL", , , "bmp\install.bmp")
   self~createBitmapButton(102, 20, but2pos, self~sizeX - 20, but2size, , , , "bmp\install2.bmp")

   pos = .Point~new(trunc(self~sizeX * self~factorX) + 10, 0)
   self~setBitmapPosition(101, pos)

   pos~x = 0
   pos~y = trunc(self~sizeY * self~factorY) + 10
   self~setBitmapPosition(102, pos)

   -- Add the other controls.
   self~createWhiteFrame(203, 10, self~SizeY - 62, self~sizeX - 20, 38, "HIDDEN")
   self~createPushButton(103, 12, self~SizeY - 60, self~sizeX - 24, 34, "OWNER NOTAB")

   groupArgs = "&Bitmap-Viewer 111 bitmapViewer &Draw-Color-Demo 112 ooDraw &Cancel 2 CANCEL"
   self~createPushButtonGroup(self~sizeX - 220, self~sizeY - 18, 60, 12, groupArgs, 1, "DEFAULT")

::method initDialog

   -- We set the background color of these buttons to the same backgroud color
   -- as the dialog, so that the buttons blend into the dialog.
   COLOR_BTNFACE = 15
   self~setControlSysColor(103, COLOR_BTNFACE)
   self~setControlSysColor(101, COLOR_BTNFACE)
   self~setControlSysColor(102, COLOR_BTNFACE)

   self~start("showInterface")

::method showInterface unguarded
   expose m but2size

   bmppos = trunc(but2size - 125 / self~FactorY)

   self~disableControl(111)  /* disable push buttons */
   self~disableControl(112)
   self~disableControl(2)

   ret = play("inst.wav", yes)

   -- Scroll the bitmaps from their displaced positions back to the upper left corners
   -- of the buttons.
   self~scrollBitmapFromTo(101, trunc(self~SizeX * self~FactorX), 5, 12, 5, -12, 0, 1)
   self~scrollBitmapFromTo(102, 30, bmppos, 30, 0, 0, -3, 2, 1)

   -- The size of the 102 button actually covers the controls under the button.  If the
   -- user clicks on any portion of the button, the button is repainted in the 'depressed'
   -- state.  Since the other controls are not repainted, this cause them (or parts of
   -- them) to disappear.  To prevent that, we resize the button to only take up the height
   -- needed for the bitamp.
   self~resizeControl(102, 450 + 32, 120 + 2, "NOREDRAW")

   self~showControl(103)     /* show scroll button */
   self~showControl(203)
   self~enableControl(2)     /* Enable push buttons */
   self~enableControl(111)
   self~enableControl(112)

   -- Start the Asynchronuous scrolling of the introductory text.
   text = "This ooDialog sample demonstrates dynamic dialog creation"
   m = self~start("ScrollInButton", 103, text, "Arial", 36, "BOLD", 0, 2, 2, 6)
   m~notify(.message~new(self, "scrollingFinished"))

   -- Now, wait until the scrolling finishes, or the user closes the main dialog.
   self~waitForEvent

   -- While the user has not closed the dialog, scroll the instruction text.
   do while \ self~finished
      text = "... please press Bitmap-Viewer or Draw-Color-Demo buttons to run graphical applications ..."
      m = self~start("scrollInButton", 103, text, "Arial", 32, "SEMIBOLD", 0, 2, 4)
      m~notify(.message~new(self, "scrollingFinished"))

      self~waitForEvent
   end

-- Wait until the haveEvent object variable turns .true.  In this program we just
-- watch for two events, the scrolling text has finished, or the user has closed
-- the dialog.
::method waitForEvent unguarded
  expose haveEvent

  haveEvent = .false
  guard on when haveEvent

-- This is the notification method for the scrolling text.  It is invoked when
-- the scrollInButton() method has finished.  The haveEvent object variable is
-- set to true which causes ourself to stop waiting.
::method scrollingFinished unguarded
  expose haveEvent
  haveEvent = .true

-- leaving() is invoked by the ooDialog framework when the underlying dialog is
-- closed.  It serves as a notification that the dialog is finished.  The default
-- implementation does nothing.  It can be, and if meant to be, over-ridden by
-- the programmer when desired.  We use it to signal ourself to stop waiting by
-- setting haveEvent to true.
::method leaving  unguarded
  expose haveEvent
  haveEvent = .true


-- Invoke the Bitmap Viewer program (oobmpvu.rex.)
::method bitmapViewer
   expose m

   -- Stop the scrolling and hide ourself.
   if \ m~completed then self~scrollInButton(103)
   self~hide

   call "oobmpvu.rex"

   -- Show ourself and become the topmost.
   self~~show~toTheTop

-- Invoke the Color Draw Demo program (oodraw.rex.)
::method ooDraw
   expose m

   -- Stop the scrolling and hide ourself.
   if m~completed = 0 then self~scrollInButton(103)
   self~hide

   call "oodraw.rex"

   -- Show ourself and become the topmost.
   self~~show~toTheTop

