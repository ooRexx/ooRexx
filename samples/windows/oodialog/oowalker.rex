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
 *  oowalker.rex  Animation demonstration (using a .rc file for the dialog
 *  template.)
 *
 *  The only real difference between this sample and the oowalk2.rex sample is
 *  the way the bitmaps for the animation are loaded.  Look at the initDialog()
 *  method of the .WalkerDialog to see this difference.
 *
 *  An animated button is used to simulate a man walking across the dialog.  The
 *  user can adjust the pace of the waling by setting the amount the man is
 *  moved each time the button is redrawn and the delay between redrawing the
 *  button.  The button is drawn using a sequence of bitmaps that show the man
 *  with different leg and arm positions.  The user can also set how the man
 *  'walking' off the edge of the dialog is handled, smoothly or bouncy, by
 *  using the "smooth corner wrap" check box.
 *
 *  A 'got cha' point (!!!) is periodically drawn and if the man is close enough
 *  to this 'danger,' the man is 'gotten' and the animation is stopped.  Each
 *  time the button is drawn, the !!! is over-drawn, so the !!! is only seen
 *  as a flicker.
 *
 *  Normally the dialog ends when the walker is gotten.  However there is a
 *  check box that allows the user to cancel this behavior and then restart the
 *  animation.
 */

 curdir = directory()
 parse source . . me
 mydir = me~left(me~lastpos('\')-1)              /* where is code     */
 mydir = directory(mydir)                        /* current is "my"   */
 env = 'ENVIRONMENT'
 win = value('WINDIR',,env)
 sp = value('SOUNDPATH',,env)
 sp = value('SOUNDPATH',win';'mydir'\WAV;'sp,env)

 dlg = .WalkerDialog~new(data.)

 if dlg~InitCode \= 0 then exit
 if dlg~Load("rc\walker.rc") > 0 then exit
 dlg~Execute("SHOWTOP")

 ret = directory(curdir)
 return

/*---------------------------- requires -----------------------------*/

::requires "ooDialog.cls"

/*---------------------------- walker dialog ------------------------*/

::class 'WalkerDialog' subclass UserDialog

::method initDialog
   expose bitmaps spriteButton quitCheckBox restartButton okButton

   -- Make button 105 a bitmap button.  Bitmap buttons are owner-drawn and
   -- ooDialog will manage the 'drawing' of the button by painting a series of
   -- bitmaps on the button, using the AnimatedButton class.
   self~installBitmapButton(105, '', 0);

   -- Load the bitmaps from the bitmap files into an array.
   bitmaps = .array~new(8)
   do i= 1 to 8
      bitmaps[i] = self~loadBitmap("bmp\wlkfig"i".bmp")
   end

   -- Create the animated button class and pass the array of bitmaps.
   spriteButton = .WalkButton~new(105, bitmaps, 0, 10, 2, 70, 120, 60, 10, 10, self)

   -- Use 'bouncy' operation when hitting edges.
   spriteButton~setSmooth(.false)

   -- Get things set up.
   self~connectButtonEvent(107, "CLICKED", onRestart)
   quitCheckBox = self~newCheckBox(106)
   restartButton = self~newPushButton(107)
   okButton = self~newPushButton(IDOK)
   quitCheckBox~check
   spriteButton~fillData(data.)
   spriteButton~suspendGotCha(.false)
   self~setDataStem(data.)
   ret = Play("tada.wav", n)

   -- Animate the button.
   spriteButton~run

::method doDataStemGet unguarded
  use strict arg data.
  self~getDataStem(data.)

::method doDataStemSet unguarded
  use strict arg data.
  self~setDataStem(data.)

::method onGotCha
   expose okButton
   use strict arg animatedButton, x, y
   okButton~disable
   self~writetoButton(105, x, y, "Got-cha", "Arial", 28, "BOLD")
   ret = play('gotcha.wav')
   animatedButton~stop
   call msSleep 1000

::method maybeQuit
   expose quitCheckBox restartButton okButton
   if quitCheckBox~checked then return self~ok

   restartButton~enable
   okButton~enable

::method onRestart
   expose spriteButton restartButton
   restartButton~disable
   spriteButton~stopped = .false

   -- Suspend the 'GotCha' before we start the animated button.  Otherwise the
   -- walker will be 'gotten' as soon as he starts.
   spriteButton~suspendGotCha(.true)
   spriteButton~run

   -- Give the walker time to 'walk' out of the danger zone before reactivating
   -- the 'GotCha.'
   delay = self~newEdit(103)~getText
   j = msSleep(2 * delay)
   spriteButton~suspendGotCha(.false)

::method ok
   self~stopAnimation
   return self~ok:super

::method cancel
   self~stopAnimation
   return self~cancel:super

::method stopAnimation private
   expose bitmaps spriteButton
   spriteButton~stop

   -- Wait until the animation sequence is finished.
   do while spriteButton~isRunning
     j = msSleep(30)
   end

   -- Now it is safe to delete the bitmaps.
   do i= 1 to 8
      self~removeBitmap(bitmaps[i])
   end


/*------------------------------ animated button --------------------*/

::class 'WalkButton' subclass AnimatedButton

::method run
   expose xDanger yDanger running
   xDanger = 300; yDanger = 70; running = .true

   reply 0

   do until(self~stopped = 1) | (self~parentStopped = 1)
      self~doAnimatedSequence
   end

   -- Have the walker do one more sequence, gives the appearance of walking 'in
   -- in place.'
   self~doAnimatedSequence

   -- We are no longer running, tell the parent dialog to maybe quit.
   running = .false
   self~parentDlg~maybeQuit

::method doAnimatedSequence private
   expose xDanger yDanger

   self~moveseq
   self~parentDlg~doDataStemGet(data.)
   do k over data.
      if data.k~datatype('N') = 0 then data.k = 0
   end
   self~setmove(data.101, data.102)
   self~setdelay(data.103)
   self~setsmooth(data.104)
   if self~stopped = 0 then
      self~parentDlg~writetoButton(105,xDanger,yDanger,"!!!","Arial",20,"BOLD")

::method isRunning unguarded
   expose running
   return running

::method suspendGotCha unguarded
   expose gotCha
   use strict arg suspend
   if suspend then gotCha = .false
   else gotCha = .true

::method hitright
   ret = play('ding.wav', 'YES')
   return 1

::method hitleft
   ret = play('ding.wav', 'YESY')
   return 1

::method hitbottom
   self~getsprite(s.)
   ret = play('chord.wav', 'YES')
   s.movey = -s.movey
   self~setsprite(s.)
   self~fillData(data.)
   self~parentDlg~doDataStemSet(data.)
   return 0

::method hittop
   expose sprite.
   self~getsprite(s.)
   ret = play('chimes.wav', 'YES')
   s.movey = -s.movey
   self~setsprite(s.)
   self~fillData(data.)
   self~parentDlg~doDataStemSet(data.)
   return 0

::method fillData
   expose quitCheckBox
   use arg data.
   if \ quitCheckBox~isA(.CheckBox) then quitCheckBox = self~parentDlg~newCheckBox(106)
   self~getSprite(msprite.)
   data.101 = msprite.movex
   data.102 = msprite.movey
   data.103 = msprite.delay
   data.104 = msprite.smooth

   if quitCheckBox~checked then data.106 = 1
   else data.106 = 0

::method movepos
   use arg px, py
   if self~stopped=0 then do
      self~movepos:super(px,py)
      self~checkDanger
   end

::method checkDanger
   expose xDanger yDanger gotCha
   self~getpos(cur.)
   if abs(cur.x+10-xDanger) <= 10 & abs(cur.y+55-yDanger) <= 30 then do
      if gotCha then self~parentDlg~onGotCha(self, xDanger-50, yDanger)
   end
