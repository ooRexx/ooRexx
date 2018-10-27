/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
 *              scrollBitmapFromTo() and scrollInControl() methods.
 *
 * Note: this program uses the public routine, locate(), to get the full path
 * name to the directory this source code file is located. In places, the
 * variable holding this value has been callously abbreviated to 'sd' which
 * stands for source directory.
 *
 */

  -- Use the global .constDir for symbolic IDs
  .application~useGlobalConstDir('O')

  .constDir[IDC_BMP_TOP]      = 101
  .constDir[IDC_BMP_MIDDLE]   = 102
  .constDir[IDC_ST_WFRAME]    = 203
  .constDir[IDC_PB_OWNERDRAW] = 103
  .constDir[IDC_PB_VIEWER]    = 111
  .constDir[IDC_PB_DRAW]      = 112

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
::requires "samplesSetup.rex"

/*-------------------------------- dialog class ----------------------*/

::class 'GraphDialog' subclass UserDialog

::method defineDialog
   expose but2size sd
   but2pos  = trunc(170 / self~factorY)
   but2size = trunc(300 / self~factorY)

   -- The two bitmap buttons are created larger than they need to be.  In particular,
   -- The IDC_BMP_MIDDLE button height is much larger, it covers most of the lower part of the
   -- dialog.
   --
   -- Then, the bitmaps for the buttons are positioned (moved from the upper left corner
   -- of the button) by a large amount.  The IDC_BMP_TOP button is positioned far to the right,
   -- and the IDC_BMP_MIDDLE button is positioned far to the bottom and far to the left.  In the
   -- showInterface() method, scrollBitmapFromTo() is used to scroll the bitmaps from
   -- their positions back to the upper left corner of the buttons.  This gives the IDC_BMP_TOP
   -- button the appearance of scrolling from the right to the left, and the IDC_BMP_MIDDLE button
   -- the appearance of scrolling from the bottom to the top.

   -- install.bmp   550 x 100 pixels
   -- install2.bmp  450 x 120 pixels

   sd = locate()

   self~createBitmapButton(IDC_BMP_TOP, 1, 10, self~sizeX-1, trunc(130 / self~factorY), "USEPAL", , , sd"bmp\install.bmp")
   self~createBitmapButton(IDC_BMP_MIDDLE, 20, but2pos, self~sizeX - 20, but2size, , , , sd"bmp\install2.bmp")

   pos = .Point~new(trunc(self~sizeX * self~factorX) + 10, 0)
   self~setBitmapPosition(IDC_BMP_TOP, pos)

   pos~x = 0
   pos~y = trunc(self~sizeY * self~factorY) + 10
   self~setBitmapPosition(IDC_BMP_MIDDLE, pos)

   -- Add the other controls.
   self~createWhiteFrame(IDC_ST_WFRAME, 10, self~SizeY - 62, self~sizeX - 20, 38, "HIDDEN")
   self~createPushButton(IDC_PB_OWNERDRAW, 12, self~SizeY - 60, self~sizeX - 24, 34, "OWNER NOTAB")

   groupArgs = "&Bitmap-Viewer"   .constDir[IDC_PB_VIEWER] "bitmapViewer " || -
               "&Draw-Color-Demo" .constDir[IDC_PB_DRAW]   "ooDraw "       || -
               "&Cancel"          .constDir[IDCANCEL]      "cancel"
   self~createPushButtonGroup(self~sizeX - 220, self~sizeY - 18, 60, 12, groupArgs, 1, "DEFAULT")

::method initDialog

   -- We set the background color of these buttons to the same backgroud color
   -- as the dialog, so that the buttons blend into the dialog.
   COLOR_BTNFACE = 15
   self~setControlSysColor(IDC_PB_OWNERDRAW, COLOR_BTNFACE)
   self~setControlSysColor(IDC_BMP_TOP, COLOR_BTNFACE)
   self~setControlSysColor(IDC_BMP_MIDDLE, COLOR_BTNFACE)

   self~start("showInterface")

::method showInterface unguarded
   expose m but2size

   bmppos = trunc(but2size - 125 / self~FactorY)

   self~disableControl(IDC_PB_VIEWER)  /* disable push buttons */
   self~disableControl(IDC_PB_DRAW)
   self~disableControl(IDCANCEL)

   ret = play("inst.wav", yes)

   -- Scroll the bitmaps from their displaced positions back to the upper left corners
   -- of the buttons.
   self~scrollBitmapFromTo(IDC_BMP_TOP, trunc(self~SizeX * self~FactorX), 5, 12, 5, -12, 0, 1)
   self~scrollBitmapFromTo(IDC_BMP_MIDDLE, 30, bmppos, 30, 0, 0, -3, 2, 1)

   -- The size of the IDC_BMP_MIDDLE button actually covers the controls under the button.  If the
   -- user clicks on any portion of the button, the button is repainted in the 'depressed'
   -- state.  Since the other controls are not repainted, this cause them (or parts of
   -- them) to disappear.  To prevent that, we resize the button to only take up the height
   -- needed for the bitamp.
   self~resizeControl(IDC_BMP_MIDDLE, 450 + 32, 120 + 2, "NOREDRAW")

   self~showControl(IDC_PB_OWNERDRAW)     /* show scroll button */
   self~showControl(IDC_ST_WFRAME)
   self~enableControl(IDCANCEL)     /* Enable push buttons */
   self~enableControl(IDC_PB_VIEWER)
   self~enableControl(IDC_PB_DRAW)

   -- Start the Asynchronuous scrolling of the introductory text.
   text = "This ooDialog sample demonstrates dynamic dialog creation"
   m = self~start("scrollInControl", IDC_PB_OWNERDRAW, text, "Arial", 36, "BOLD", 0, 2, 2, 6)
   m~notify(.message~new(self, "scrollingFinished"))

   -- Now, wait until the scrolling finishes, or the user closes the main dialog.
   self~waitForEvent

   -- While the user has not closed the dialog, scroll the instruction text.
   do while self~isDialogActive
      text = "... please press Bitmap-Viewer or Draw-Color-Demo buttons to run graphical applications ..."
      m = self~start("scrollInControl", IDC_PB_OWNERDRAW, text, "Arial", 32, "SEMIBOLD", 0, 2, 4)
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
-- the scrollInControl() method has finished.  The haveEvent object variable is
-- set to true which causes ourself to stop waiting.
::method scrollingFinished unguarded
  expose haveEvent
  haveEvent = .true

-- leaving() is invoked by the ooDialog framework when the underlying dialog is
-- closed.  It serves as a notification that the dialog is finished.  The
-- default implementation does nothing.  It can be, and ii meant to be, over-
-- ridden by the programmer when desired.  We use it to signal ourself to stop
-- waiting by setting haveEvent to true.
::method leaving  unguarded
  expose haveEvent
  haveEvent = .true


-- Invoke the Bitmap Viewer program (oobmpvu.rex.)
::method bitmapViewer
   expose m sd

   -- Stop the scrolling and hide ourself.
   if \ m~completed then self~scrollInControl(IDC_PB_OWNERDRAW)
   self~hide

   call (sd"oobmpvu.rex")

   -- Show ourself and become the topmost.
   self~~show~toTheTop

-- Invoke the Color Draw Demo program (oodraw.rex.)
::method ooDraw
   expose m sd

   -- Stop the scrolling and hide ourself.
   if \ m~completed then self~scrollInControl(IDC_PB_OWNERDRAW)
   self~hide

   call (sd"oodraw.rex")

   -- Show ourself and become the topmost.
   self~~show~toTheTop

