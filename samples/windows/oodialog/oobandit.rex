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
/* OODialog\Samples\oobandit.rex  Jack slot machine                         */
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

       /* 1ms fast, 500ms slow, 200ms start, equals random every 25th */
 d = .banditdlg~new(1,500,200,25)
 d~Execute("SHOWTOP")
 d~deinstall
 ret = directory(curdir)
 return

/*---------------------------------- requires ------------------------*/

::requires "OODIALOG.CLS"

/*---------------------------------- dialog class --------------------*/

::class banditdlg subclass UserDialog

::method init
   expose kind3 speedx minspeed maxspeed maxcycle equal misses initpot
   use arg minspeed, maxspeed, speedx, kind3
   minspeed = max(1,minspeed)
   maxcycle = 200; initpot = 1000; equal = 0; misses = 0
   self~init:super()
   self~InitCode = self~createcenter(255,140,"Jack Slot Machine -",
                                     "Stop for 3 of the same kind", , , "System", 8)

::method DefineDialog
   expose bmp. speedx jackx jacky
   self~DefineDialog:super
   bmp.1 = self~LoadBitmap("bmp\tiger.bmp")
   bmp.2 = self~LoadBitmap("bmp\chihuahu.bmp")
   bmp.3 = self~LoadBitmap("bmp\eleph2.bmp")
   bmp.4 = self~LoadBitmap("bmp\horse.bmp")
   bmp.5 = self~LoadBitmap("bmp\sealion.bmp")
   bmp.6 = self~LoadBitmap("bmp\moose.bmp")
   bmp.7 = self~LoadBitmap("bmp\rhinoce.bmp")
   bmp.8 = self~LoadBitmap("bmp\goat.bmp")
   self~addBlackFrame(10,5,235,20,"BORDER") /* jackpot line */
   self~addBlackFrame(107,7,41,16,"BORDER")
   self~addBlackFrame(108,8,39,14,"BORDER")
   self~addText(40,10,60,10,"Jackpot  $$$")
   self~addEntryLine(1200,,110,9,35,11)
   self~addText(175,10,60,10,"$$$  Jackpot")
   jackx = 20*self~factorx; jacky = 20*self~factory
   self~addBitmapButton(1201,10, 30,75,90, "",,bmp.1,,,,"INMEMORY STRETCH USEPAL")
   self~addBitmapButton(1202,90, 30,75,90, "",,bmp.1,,,,"INMEMORY STRETCH")
   self~addBitmapButton(1203,170,30,75,90, "",,bmp.1,,,,"INMEMORY STRETCH")
   self~addButtonGroup(10,125,34,12,"&Stop 1 OK &Cancel 2 Cancel",1,"DEFAULT")
   self~addText(105,126,40,9,'Speed (ms):','RIGHT')
   self~addEntryLine(1205,'speed',150,125,15,12)
   self~addText(168,126,15,9,'Fast','RIGHT')
   self~addText(230,126,15,9,'Slow','LEFT')
   self~addScrollBar(1206,188,125,40,12,"HORIZONTAL")
   self~speed = speedx

::method InitDialog
   expose minspeed maxspeed
   self~InitDialog:super
   self~connectScrollBar(1206,'FASTER','SLOWER','DRAG',minspeed,maxspeed,self~speed)

::method Run
   expose x y z bmp. kind3 cycle maxcycle equal misses
   rand =  random(1,8,time('S')*7) /* init random */
   ret = play("WHISTLE.WAV")
   do cycle = maxcycle by -1 to 1 until self~finished
      if self~checkspeed = 0 then leave
      sleep = max(1, min(100,self~speed/2) )
      do j = 1 to self~speed/sleep
	 if self~Finished = 0 then do
            self~HandleMessages
            call msSleep sleep
         end
      end
      self~HandleMessages
      if self~Finished then return 0
      if random(1,kind3) = 3 then
           do; x = equal // 8 + 1; y = x; z = x; equal = equal + 1;  end
      else do; x = random(1, 8); y = random(1, 8); z = random(1, 8); end
      self~ChangeBitmapButton(1201, bmp.x,,,,"INMEMORY STRETCH")
      self~ChangeBitmapButton(1202, bmp.y,,,,"INMEMORY STRETCH")
      self~ChangeBitmapButton(1203, bmp.z,,,,"INMEMORY STRETCH")
   end
   ret = TimedMessage('Sorry, you did not get the jackpot in' misses 'tries on' equal 'chances', ,
                      'End of run',4000)
   return 1

::method slower
   self~combineElwithSB(1205,1206,+20)
   self~checkspeed

::method faster
   self~combineElwithSB(1205,1206,-20)
   self~checkspeed

::method drag
   use arg wparam, lparam
   self~combineElwithSB(1205,1206,0, wparam)
   self~checkspeed

::method checkspeed
   expose minspeed maxspeed cycle initpot jackx jacky
   self~getAttrib('speed')
   if self~speed < minspeed then self~speed = minspeed
   if self~speed > maxspeed then self~speed = maxspeed
   self~setAttrib('speed')
   jackpot = trunc(cycle * initpot / self~speed)
   /*self~write(jackx,jacky,"Jackpot $$$" right(jackpot,6,'_'),"Arial",24,'BOLD')*/
   self~setEntryLine(1200,right(jackpot,9))
   return jackpot

::method OK
   expose x y z misses initpot
   self~finished = 0
   if ((x=y) & (y=z)) then do
      ret = Play("tada.wav")
      self~setWindowTitle(self~get,"Congratulations !")
      do i=40 by 20 to 120
         self~write(i*self~factorx,i*self~factory,"Congratulations...","Arial",14,'BOLD')
      end
      money = strip(self~getEntryLine(1200))
      self~write(10*self~factorx+5,75*self~factory,"You won the jackpot:" money,"Arial",18,'BOLD')
      do i=1 to min(money%500 + 1,10)
         ret = play("jackpot.wav")
         money = max(0,money - 500)
         self~setEntryLine(1200,right(money,9))
      end
      call msSleep 1000
      return self~cancel
   end
   misses = misses + 1
   ret = Play("nope.wav", "yes")
   if ((x=y) | (y=z) | (x=z)) then do
        ret = infoDialog("2 equal, not bad, try again... jackpot reduced 25%")
        initpot = trunc(initpot * .75)
        end
   else do
        ret = infoDialog("Not a chance, try again... jackpot is halfed!")
        initpot = trunc(initpot / 2)
   end
   if initpot=1 then ret = infoDialog("One more chance to hit the jackpot....")
   self~checkspeed
   return 0

::method cancel
   call Play "byebye.wav"
   do i = 1 to 8
      self~RemoveBitmap(bmp.i)
   end
   self~finished = 1
   return 1
