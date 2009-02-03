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
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* OODialog\Samples\oowalk2.rex   Animation demonstration (using DLL)       */
/*                                                                          */
/*--------------------------------------------------------------------------*/


 curdir = directory()
 parse source . . me
 mydir = me~left(me~lastpos('\')-1)              /* where is code     */
 mydir = directory(mydir)                        /* current is "my"   */
 env = 'ENVIRONMENT'
 win = value('WINDIR',,env)
 sp = value('SOUNDPATH',,env)
 sp = value('SOUNDPATH',win';'mydir'\WAV;'sp,env)

 dlg = .walkerdlg~new('res\oowalk2.dll',100,data.)
 if dlg~InitCode \= 0 then exit
 dlg~Execute("SHOWTOP")
 dlg~deinstall
 ret = directory(curdir)
 return

/*---------------------------- requires -----------------------------*/

::requires "OODIALOG.cls"

/*---------------------------- walker dialog ------------------------*/

::class walkerdlg subclass ResDialog

::method InitDialog
   self~InitDialog:super
   self~ConnectBitmapButton(100, '', 0);

::method Run
                         /* bitmaps have IDs 201-208 in oowalker.dll */

   sb = .walkbutton~new(100, 201,208, 10, 2, 70, 120, 60, 10, 10, self)

   sb~setsmooth(0)                /* bouncy operation on edges       */
   sb~setstep(1)                  /* size of step through bitmap IDs */

   sb~filldata(data.)
   self~setdatastem(data.)
   ret = Play("tada.wav", n)
   sb~run                                      /* animate the button */
   return 0


/*------------------------------ animated button --------------------*/

::class WalkButton subclass AnimatedButton

::method run
   expose xdanger ydanger
   xdanger = 300; ydanger = 70
   do until(self~stopped = 1) | (self~ParentStopped = 1)
      self~moveseq
      self~ParentDlg~getdatastem(data.)
      do k over data.
         if data.k~datatype('N') = 0 then data.k = 0
      end
      self~ParentDlg~HandleMessages
      self~setmove(data.101, data.102)
      self~setdelay(data.103)
      self~setsmooth(data.104)
      if self~stopped = 0 then
         self~parentdlg~writetoButton(100,xdanger,ydanger,"!!!","Arial",20,"BOLD")
   end
   return 0

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
   self~filldata(data.)
   self~ParentDlg~setdatastem(data.)
   return 0

::method hittop
   expose sprite.
   self~getsprite(s.)
   ret = play('chimes.wav', 'YES')
   s.movey = -s.movey
   self~setsprite(s.)
   self~filldata(data.)
   self~ParentDlg~setdatastem(data.)
   return 0

::method filldata
   use arg data.
   self~GetSprite(msprite.)
   data.101 = msprite.movex
   data.102 = msprite.movey
   data.103 = msprite.delay
   data.104 = msprite.smooth

::method movepos
   use arg px, py
   if self~stopped=0 then do
      self~movepos:super(px,py)
      self~checkDanger
   end

::method checkDanger
   expose xdanger ydanger
   self~getpos(cur.)
   if abs(cur.x+10-xdanger) <= 10 & abs(cur.y+55-ydanger) <= 30 then do
      self~parentdlg~writetoButton(100,xdanger-50,ydanger,"Got-cha","Arial",28,"BOLD")
      ret = play('gotcha.wav')
      self~stop
      call sleepms(1000)
   end
