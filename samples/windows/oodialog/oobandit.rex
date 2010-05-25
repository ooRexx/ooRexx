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
 *  oobandit.rex  An ooDialog example, the Jackpot Slot Machine.
 *
 *  This example demonstrates one way of animating bitmaps, by drawing them on
 *  the face of a button.  It also shows how to correctly use dialog units in
 *  a UserDialog to size and place the controls.
 *
 *  Creating the dialog and its controls is done this way:
 *
 *  The pixel size of the bitmaps is known.  Although bitmaps can be stretched
 *  by the OS to fit a specific size, bitmaps look best displayed in their
 *  actual size.  The pixel size of the bitmaps is first converted to the
 *  correct dialog unit size, correct for the actual dialog to be constructed.
 *
 *  Then the size of the dialog and the size and placement of the dialog
 *  controls are calculated around the bitmap size.
 */

   curdir = directory()
   parse source . . me
   mydir = me~left(me~lastpos('\')-1)              /* where is code     */
   mydir = directory(mydir)                        /* current is "my"   */
   env = 'ENVIRONMENT'
   win = value('WINDIR',,env)
   sp = value('SOUNDPATH',,env)
   sp = value('SOUNDPATH',win';'mydir'\WAV;'sp,env)

         /* 1ms fast, 500ms slow, 200ms start, equals random every 25th */
   d = .BanditDlg~new(1, 1000, 1000, 25)
   d~execute("SHOWTOP")

   ret = directory(curdir)
   return 0

/*---------------------------------- requires ------------------------*/

::requires "ooDialog.cls"

/*---------------------------------- dialog class --------------------*/

::class 'BanditDlg' subclass UserDialog

::constant BITMAP_X           152
::constant BITMAP_Y           178
::constant FONT_NAME          "MS Shell Dlg"
::constant FONT_SIZE          14

::constant MARGIN_X           10
::constant MARGIN_Y           5
::constant JACKPOT_LINE_Y     22
::constant TEXT_Y             12

::constant BUTTON_X           35
::constant BUTTON_Y           15

::method init
   expose kind3 initialSpeed minSpeed maxSpeed maxCycle cycle equal misses initPot won bitMapSize dlgSize
   use arg minSpeed, maxSpeed, initialSpeed, kind3

   self~init:super()

   -- Set the font the dialog will use when created.  Without this step, dialog
   -- units can not be calculated correctly.
   self~setDlgFont(self~FONT_NAME, self~FONT_SIZE)

   -- Set our various instance variables.
   minSpeed = max(1,minSpeed)
   maxCycle = 200; initPot = 1000; equal = 0; misses = 0; cycle = maxCycle; won = .false

   -- Calculate the size of a bitmap in dialog units.
   bitMapSize = .Size~new(self~BITMAP_X, self~BITMAP_Y)
   self~pixel2dlgUnit(bitMapSize)

   -- Calculate the size of this dialog based on the bitmap size.
   dlgSize = self~calcSize(bitMapSize)

   title = "Jackpot Slot Machine - Stop on 3 of a Kind and Win $$$"
   self~initCode = self~createcenter(dlgSize~width, dlgSize~height, title)

::method defineDialog
   expose bmp. initialSpeed dlgSize bitMapSize

   -- Load the bitmaps into memory.
   bmp.1 = self~LoadBitmap("bmp\tiger.bmp")
   bmp.2 = self~LoadBitmap("bmp\chihuahu.bmp")
   bmp.3 = self~LoadBitmap("bmp\eleph2.bmp")
   bmp.4 = self~LoadBitmap("bmp\horse.bmp")
   bmp.5 = self~LoadBitmap("bmp\sealion.bmp")
   bmp.6 = self~LoadBitmap("bmp\moose.bmp")
   bmp.7 = self~LoadBitmap("bmp\rhinoce.bmp")
   bmp.8 = self~LoadBitmap("bmp\goat.bmp")
   bmp.0 = 8

   -- Note that for a static text control, the CENTERIMAGE flag has the effect
   -- of vertically centering the text within the control.

   -- Create the jackpot line.  First a frame around the whole thing.
   x = self~MARGIN_X
   y = self~MARGIN_Y
   self~createBlackFrame(-1, x, y, dlgSize~width - (2 * self~MARGIN_X), self~JACKPOT_LINE_Y, "BORDER")

   -- Static text on the right, centered over the 1st bitmap
   txt = "Jackpot  $$$"
   txtSize = self~getTextSizeDU(txt)
   x += trunc((bitMapSize~width / 2) - (txtSize~width / 2))
   y += self~MARGIN_Y
   self~createStaticText(-1, x, y, txtSize~width, self~TEXT_Y, "CENTER CENTERIMAGE", txt)

   -- The jackpot number, could be up to 9 digits.  Just a static control with a
   -- fancy frame, centered over the middle bitmap
   txt = "888888888"
   txtSize = self~getTextSizeDU(txt)
   x = trunc((dlgSize~width / 2) - ((txtSize~width + 6) / 2))
   self~createBlackFrame(-1,   x + 0, y - 2, txtSize~width + 6, self~TEXT_Y + 4, "BORDER")
   self~createBlackFrame(-1,   x + 1, y - 1, txtSize~width + 4, self~TEXT_Y + 2, "BORDER")
   self~createStaticText(1200, x + 3, y - 0, txtSize~width + 0, self~TEXT_Y + 0, "RIGHT CENTERIMAGE")

   -- Static text on the left, centered over the 3rd bitmap.
   txt = "$$$  Jackpot"
   txtSize = self~getTextSizeDU(txt)
   x = (bitMapSize~width * 2) + (3 * self~MARGIN_X)            -- The left edge of the 3rd bitmap ...
   x += trunc((bitMapSize~width / 2) - (txtSize~width / 2))    -- ... and center
   self~createStaticText(-1, x, y, txtSize~width, self~TEXT_Y, "CENTER CENTERIMAGE", txt)

   -- Now place the bitmaps
   x = self~MARGIN_X
   y = (2 * self~MARGIN_Y) + self~JACKPOT_LINE_Y
   self~createBitmapButton(1201, x, y, bitMapSize~width, bitMapSize~height, "INMEMORY  USEPAL", , , bmp.1)

   x += bitMapSize~width + self~MARGIN_X
   self~createBitmapButton(1202, x, y, bitMapSize~width, bitMapSize~height, "INMEMORY ", , , bmp.1)

   x += bitMapSize~width + self~MARGIN_X
   self~createBitmapButton(1203, x, y, bitMapSize~width, bitMapSize~height, "INMEMORY ", , , bmp.1)

   -- Stop and cancel buttons, placed at left margin and under bitmaps
   x = self~MARGIN_X
   y += bitMapSize~height + self~MARGIN_Y
   self~createPushButtonGroup(x, y, self~BUTTON_X, self~BUTTON_Y, "&Stop 1100 onStop &Cancel 2 Cancel", .false, "DEFAULT")

   -- A group box to hold the speed adjustment controls
   txt = 'Speed (in ms) lower is faster'
   x += bitMapSize~width + self~MARGIN_X
   cy = dlgSize~height - y - self~MARGIN_Y
   self~createGroupBox(-1, x, y, (bitMapSize~width * 2) + self~MARGIN_X, cy, , txt)

   -- And finally the speed adjustment controls them selves.  The top of a group box
   -- is higher than the top line of the group box (to allow for text.)  So in order
   -- for the speed controls to look centered with the group box lines, we need to
   -- calculate the center, adjust for the height of the controls, and then push it
   -- "down a bit."  I arbitrarily choose 3 as 'a bit.'
   x += self~MARGIN_X
   y += trunc((cy / 2) - (self~TEXT_Y / 2)) + 3
   txt = ' Faster :'
   txtSize = self~getTextSizeDU(txt)
   self~createStaticText(-1, x, y, txtSize~width, self~TEXT_Y, 'RIGHT CENTERIMAGE', txt)

   x += txtSize~width + 2
   self~createEdit(1205, x , y, 35, self~TEXT_Y, "NUMBER")

   x += 35
   self~createUpDown(1206, x, y, 25, self~TEXT_Y, "RIGHT ARROWKEYS AUTOBUDDY BUDDYINT HORIZONTAL NOTHOUSANDS", 'speed')

   x += 2
   self~createStaticText(-1, x, y, 30, self~TEXT_Y, 'LEFT CENTERIMAGE', ': Slower')

   -- Set the up down position to the initial speed.
   self~speed = initialSpeed

::method initDialog
   expose minSpeed maxSpeed notStopped jackPotCtrl speedCtrl

   self~newUpDown(1206)~setRange(minSpeed, maxSpeed)

   speedCtrl = self~newEdit(1205)
   speedCtrl~setLimit(maxSpeed~length - 1)
   ret = speedCtrl~connectKeyEvent(onKey); say 'connectKeyEvent ret:' ret

   jackPotCtrl = self~newStatic(1200)

   notStopped = .true
   self~disableControl(1100)
   self~start("bandit")

::method onKey unguarded
  expose speedCtrl
  use arg key, shift, control, alt, info
  say 'Key press:' key 'shift?' shift 'control?' control 'alt?' alt 'info:' info
  say
  say 'speed text:' speedCtrl~getText

  s = speedCtrl~selection
  say 'selection:' s~startChar s~endChar

  if control then return .false -- Don't allow cut and paste
  if key == 57 then return .false
  else return .true

::method bandit unguarded
   expose x y z bmp. kind3 cycle maxCycle equal misses notStopped won

   rand =  random(1, 8, time('S') * 7) /* init random */
   ret = play("WHISTLE.WAV")

   -- The user could have canceled while the whistle was playing..
   if self~finished then return 0

   self~enableControl(1100)

   do cycle = maxCycle by -1 to 1 until self~finished
      if self~checkSpeed = 0 then leave

      sleep = format(max(1, min(100, self~speed / 2)), , 0)
      do j = 1 to self~speed / sleep
	       if  \self~finished then call msSleep sleep
      end

      if self~finished then return 0
      guard on when notStopped     -- Don't change the bitmaps out from under the user.

      if random(1, kind3) = 3 then do
         x = equal // 8 + 1; y = x; z = x; equal += 1
      end
      else do
         x = random(1, 8); y = random(1, 8); z = random(1, 8)
      end

      -- It's an error to invoke changeBitmapButton if the underlying dialog
      -- no longer exists.  (Which may be if the user hit cancel.)
      if \self~isDialogActive then return 0

      self~changeBitmapButton(1201, bmp.x,,,,"INMEMORY STRETCH")
      self~changeBitmapButton(1202, bmp.y,,,,"INMEMORY STRETCH")
      self~changeBitmapButton(1203, bmp.z,,,,"INMEMORY STRETCH")

      guard off
      if self~finished then return 0
   end

   if \won then do
      self~disableControls

      msg = 'Sorry, you did not get the jackpot in '     || .endOfLine ||   -
            misses 'tries.' || .endOfLine~copies(2)                    ||   -
            'There were' equal 'chances. ('equal' three' || .endOfLine || -
            'of a kind were shown.)'
      title = 'End of run'
      ret = messageDialog(msg, self~hwnd, title, "OK", "INFORMATION")
   end

   -- Clean up is in our cancel method, so we invoke that rather than self~ok:super
   if \self~finished then return self~cancel
   else return 1

::method disableControls private
   self~disableControl(1100)
   self~disableControl(IDCANCEL)
   self~disableControl(1205)
   self~disableControl(1206)

::method onStop
   expose x y z misses initPot notStopped won jackpotCtrl

   notStopped = .false  -- Prevent the 'bandit' from changing the bitmaps

   if ((x=y) & (y=z)) then do
      -- All 3 bitmaps are the same, jackpot.
      won = .true
      ret = play("tada.wav")
      self~setWindowTitle(self~get,"Congratulations !")
      do i = 40 by 20 to 120
         self~write(i*self~factorx,i*self~factory,"Congratulations...","Arial",14,'BOLD')
      end
      money = jackpotCtrl~getText
      self~write(10*self~factorx+5,75*self~factory,"You won the jackpot:" money,"Arial",18,'BOLD')
      do i=1 to min(money%500 + 1,10)
         ret = play("jackpot.wav")
         money = max(0,money - 500)
         jackpotCtrl~setText(money)
      end
      jackpotCtrl~setText(0)
      call msSleep 1000
      return self~cancel
   end

   -- Not 3 of a kind
   misses += 1
   ret = play("nope.wav", "yes")

   if ((x=y) | (y=z) | (x=z)) then do
        ret = infoDialog("2 equal, not bad, try again... jackpot reduced 25%")
        initPot = trunc(initPot * .75)
   end
   else do
        ret = infoDialog("Not a chance, try again... jackpot is halfed!")
        initPot = trunc(initPot * .5)
   end

   if initPot = 1 then ret = infoDialog("One more chance to hit the jackpot....")
   self~checkSpeed
   notStopped = .true  -- Unblock the 'bandit'
   return 0

::method checkSpeed
   expose minSpeed maxSpeed cycle initPot jackpotCtrl

   if self~finished then return 0

   self~getDataAttribute('speed')

   -- Although the edit control is numbers only, and the up down control won't
   -- allow the user to spin outside of the range, or use the arrow keys to move
   -- outside of the range, it is possible for the user to delete all the
   -- numbers in the edit control, or to type in numbers larger than the
   -- maximum.  In which case the up down control seems to return the empty
   -- string for its position ??
   if self~speed == "" then do
     say 'Up down position: ' self~newUpDown(1206)~getPosition
     say 'Edit control text:' self~newEdit(1205)~getText
   end

   money = trunc(cycle * initPot / self~speed)
   jackpotCtrl~setText(money)

   return money

::method cancel
   expose notStopped bmp.

   self~disableControls
   self~finished = 1

   call Play "byebye.wav"
   notStopped = .true     -- Be sure the bandit() method is not blocked

   -- You can not remove the bitmap handles while the dialog is still displayed
   -- on the screen.  As long as the dialog is showing, the os will try to
   -- repaint  the dialog when needed.  If the bitmaps are destroyed, the
   -- program will crash.  Wait until the underlying dialog is closed.
   do while self~isDialogActive
     j = SysSleep(.334)
   end

   do i = 1 to bmp.0
     self~removeBitmap(bmp.i)
   end
   return 1

::method calcSize private
   use strict arg bitMapSize

   s = .Size~new

   -- For the width of the dialog, we have 3 bitmaps, an X magin on both sides,
   -- and we space the bitmaps apart horizontaly using the X margin.  3 bitmaps
   -- and 4 X margins
   s~width = (3 * bitMapSize~width) + (4 * self~MARGIN_X)

   -- The height is sligthly more complicated.  It goes like this from top to
   -- bottom: Y margin, jackpot line, Y margin, bitmap height, Y margin, push
   -- button group height, Y margin.
   s~height = (4 * self~MARGIN_Y) + self~JACKPOT_LINE_Y + bitMapSize~height +  -
              self~getButtonGroupHeight

   return s

::method getButtonGroupHeight private
   expose h

   if \h~dataType('W') then do
      -- To calculate the height of the push button group, we need to know the
      -- height of a button, the number of buttons (2), and the vertical spacing
      -- between buttons.  It so happens that I know the vertical spacing is 1/2
      -- the button height, truncated.
      h = (2 * self~BUTTON_Y) + trunc(.5 * self~BUTTON_Y)
   end
   return h

