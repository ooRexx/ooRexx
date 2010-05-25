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
 *  oophil.rex  An ooDialog demonstration of the Philosopher's Forks
 */


  curdir = directory()
  parse source . . me
  mydir = me~left(me~lastpos('\')-1)             /* install directory */
  mydir = directory(mydir)                       /* current is "my"   */

/*---------------------- run default parameters ----------------------*/

  parms.107 = 80                 /* sleeping time 80 * 100 ms = 8 sec */
  parms.108 = 50                 /* eating   time 50 * 100 ms = 5 sec */
  parms.109 = 3                  /* 3 repetitions                     */
  parms.104 = 0                  /* radio button left  fork first off */
  parms.105 = 0                  /*              right fork first off */
  parms.106 = 1                  /*              any   fork first on  */

/*---------------------- dialogs & resources -------------------------*/

  v.anidialogID = 100            /* animation dialog graphical        */
  v.anidialog = 'rc\oophil2.rc'
  v.setdialogID = 101            /* setup dialog for parameters       */
  v.setdialog = 'rc\oophil1.rc'

/*---------------------- animation dialog IDs ------------------------*/

  v.idp       = 100        /* phil    101-105             */
  v.idf       = 105        /* fork    106-110             */
  v.idhr      = 111        /* hand-r  121,131,141,151,161 */
  v.idhl      = 112        /* hand-l  122,132,142,152,162 */
  v.idcake    = 120        /* cake                        */
  v.idpc      = 110        /* piece of cake  111-115      */

/*---------------------- animation audio files -----------------------*/

  v.help      = 'wav\philhelp.wav'
  v.stop      = 'wav\philstop.wav'
  v.eat       = 'wav\phileat.wav'
  v.sleep     = 'wav\philslep.wav'
  v.wait      = 'wav\philwait.wav'
  v.ouch      = 'wav\philouch.wav'
  v.cakewhere = 'wav\philstrt.wav'
  v.cakenew   = 'wav\philcake.wav'

  v.helptext  = "<<< The values of this dialog determine the" ,
                "behavior of the Philosophers' Forks execution <<<"

/*----------------------- bitmaps (will be memory loaded) ------------*/

  vb.bmpblank  = 'bmp\blank.bmp'
  vb.bmpphil   = 'bmp\philphil.bmp'
  vb.bmpwait   = 'bmp\philwait.bmp'
  vb.bmpeat    = 'bmp\phileat.bmp'
  vb.bmpeat2   = 'bmp\phileat2.bmp'
  vb.bmpsleep  = 'bmp\philslep.bmp'
  vb.bmpouch   = 'bmp\philouch.bmp'
  vb.bmpfork   = 'bmp\fork.bmp'
  vb.bmphandr  = 'bmp\handrite.bmp'
  vb.bmphandl  = 'bmp\handleft.bmp'
  vb.bmphandrf = 'bmp\handfkri.bmp'
  vb.bmphandlf = 'bmp\handfkle.bmp'
  vb.bmppiece  = 'bmp\cakepiec.bmp'
  do i=1 to 11
     icake = 'BMPCAKE'i
     vb.icake = 'bmp\cake'i'.bmp'
  end

/*---------------------- main logic ----------------------------------*/

  setUpDlg = .SetUpDialog~new(parms., v., vb.)
  setUpDlg~execute("SHOWTOP")

  ret = directory(curdir)
  return


/*---------------------- requires ooDialog ----------------------------*/

::requires 'ooDialog.cls'


/*---------------------- setup dialog ---------------------------------*/

::class 'SetupDialog' subclass UserDialog

::method init
   expose v. vb.
   use arg parms., v., vb.
   self~init:super(parms.)
   self~load(v.setdialog, v.setdialogID, 'CENTER')

::method initDialog
   expose msg v. vb.
   msg = .nil

   -- Set the up down controls.  They all have a range from 0 to 1000.  Then
   -- we connect the delta postion event notification for the sleeping and
   -- eating time controls so that we can increment them by 5 rather than 1.
   do id = 107 to 109
     self~newUpDown(id)~setRange(0, 1000)
   end

   self~connectUpDownEvent(107, "DELTAPOS", onDelta)
   self~connectUpDownEvent(108, "DELTAPOS", onDelta)

   -- Load the bitmaps in to memory
   do i over vb.
     v.i = self~LoadBitmap(vb.i)
   end

-- The user incremented (or decremented) one of the up down controls.  We
-- intercept the notification so that we can increment (or decrement) by 5
-- rather than 1.
::method onDelta
  use arg pos, delta, id, hwnd

  return .UpDown~deltaPosReply(.true, .false, delta * 5)

::method help
   expose msg v.
   if msg = .NIL then         ret = Play(v.help, 'yes')
   else if msg~completed then ret = Play(v.help, 'yes')
   else                       ret = Play()
   msg = self~start("scrollInButton", 110, v.helptext, -
                    "Arial", 30, "BOLD", 0, 4, 8)
   return 0

::method ok                                   /* run philosophers      */
   expose msg v.
   if msg \= .nil then
      if msg~completed=0 then self~scrollInButton(110)
   self~getDataStem(parms.)
                                              /* philosopher dialog    */
   dlg = .phildlg~new(v.)
   if dlg~executeAsync(,"SHOWTOP") = 0 then do
      dlg~myExecute(parms.)                   /* philosopher animation */
      dlg~endAsyncExecution
   end
   else call errorDialog "Couldn't execute Philosophers Forks Dialog"

::method cancel
   expose msg v. vb.
   if msg \= .nil then
      if \msg~completed then self~scrollInButton(110)
   do i over vb.                              /* bitmaps out of memory */
      self~removeBitmap(v.i)
   end
   self~cancel:super


/*---------------------- animation dialog -----------------------------*/

::class 'PhilDlg' subclass UserDialog

::method init
   expose v.
   use arg v.
   self~init:super
   self~load(v.anidialog, v.anidialogID, 'CENTER')

::method initDialog
   expose f1 f2 f3 f4 f5 p1 p2 p3 p4 p5 v.

   self~disableControl(1)                       /* disable stop button */
   do i = 1 to 5
      ret = self~installBitmapButton(v.idp  + i,    '', v.bmpphil ,,,,"STRETCH INMEMORY")
      ret = self~installBitmapButton(v.idf  + i,    '', v.bmpfork ,,,,"STRETCH INMEMORY")
      ret = self~installBitmapButton(v.idhl + 10*i, '', v.bmpblank,,,,"STRETCH INMEMORY")
      ret = self~installBitmapButton(v.idhr + 10*i, '', v.bmpblank,,,,"STRETCH INMEMORY")
      ret = self~installBitmapButton(v.idpc + i,    '', v.bmpblank,,,,"STRETCH INMEMORY")
   end
   ret    = self~installBitmapButton(v.idcake,      '', v.bmpblank,,,,"STRETCH INMEMORY")

   f1 = .fork~new(1, self)                      /* create 5 forks      */
   f2 = .fork~new(2, self)
   f3 = .fork~new(3, self)
   f4 = .fork~new(4, self)
   f5 = .fork~new(5, self)

   p1 = .phil~new(1,f5,f1, self)                /* create 5 philos.    */
   p2 = .phil~new(2,f1,f2, self)
   p3 = .phil~new(3,f2,f3, self)
   p4 = .phil~new(4,f3,f4, self)
   p5 = .phil~new(5,f4,f5, self)

::method myExecute                              /* animate dialog      */
   expose f1 f2 f3 f4 f5 p1 p2 p3 p4 p5
   use arg parms.
   T.sleep = parms.101
   T.eat = parms.102
   T.veat = trunc(T.eat / 2)
   T.vsleep = trunc(T.sleep / 2)
   if parms.104 = 1 then T.side = 100           /* left fork first     */
   else if parms.105 = 1 then T.side = 0        /* right               */
                         else T.side = 50       /* random              */
   T.repeats = parms.103

   self~cake('init')                            /* set up the cake     */

   m1 = p1~start("run",T.)                      /* run 5 philsophers   */
   m2 = p2~start("run",T.)
   m3 = p3~start("run",T.)
   m4 = p4~start("run",T.)
   m5 = p5~start("run",T.)
   self~enableControl(1)                        /* enable stop button  */

   -- wait untill the 5 philsopers are done, or the stop button is pushed.
   do while(m1~completed+m2~completed+m3~completed+m4~completed+m5~completed <5) & \self~finished
      j = SysSleep(.340)
   end
   m1~result                                    /* check 5 phils       */
   m2~result
   m3~result
   m4~result
   m5~result
   self~ok:super                                /* finish dialog    */

::method ok unguarded                           /* Stop button      */
   expose f1 f2 f3 f4 f5 p1 p2 p3 p4 p5 v.
   self~disableControl(1)
   call play v.stop,'yes'
   self~ok:super                                /* sets finished    */
   f1~layDown                                   /* take away forks  */
   f2~layDown
   f3~layDown
   f4~layDown
   f5~layDown

::method setPhil unguarded                      /* philosoph bitmap */
   expose v.
   use arg num, bmp
   self~changeBitmapButton(v.idp + num, value('v.bmp'bmp),,,,'STRETCH INMEMORY')

::method setFork unguarded                      /* fork bitmap      */
   expose v.
   use arg num, bmp
   self~changeBitmapButton(v.idf + num, value('v.bmp'bmp),,,,'STRETCH INMEMORY')

::method setLeft unguarded                      /* left hand bitmap */
   expose v.
   use arg num, bmp
   self~changeBitmapButton(v.idhl + num*10, value('v.bmp'bmp),,,,'STRETCH INMEMORY')

::method setRight unguarded                     /* righthand bitmap */
   expose v.
   use arg num, bmp
   self~changeBitmapButton(v.idhr + num*10, value('v.bmp'bmp),,,,'STRETCH INMEMORY')

::method setPiece unguarded                     /* cakepiece bitmap */
   expose v.
   use arg num, bmp
   self~changeBitmapButton(v.idpc + num, value('v.bmp'bmp),,,,'STRETCH INMEMORY')

::method cake unguarded                         /* cake bitmap      */
   expose curCake v.
   if arg() = 1 then do
        curCake = -1
        self~audio('cakewhere')
        call SysSleep(2)
   end
   curCake = (curCake+1)//11
   i = curCake + 1
   self~changeBitmapButton(v.idcake, value('v.bmpcake'i),,,,'STRETCH INMEMORY')
   if curCake=10 then self~audio('cakenew')

::method audio unguarded                        /* play a sound     */
   expose v.
   use arg act
   ret = play(value('v.'act), 'yes')


/*---------------------- philosopher ---------------------------------*/

::class phil                                      /*** philosophers ***/

::method init                                     /* initialize       */
   expose num rFork lFork dlg
   use arg num, rFork, lFork, dlg

::method run                                      /* run the philosop.*/
   expose num rFork lFork dlg
   use arg T.
   x =  random(1,100,time('S')*num)
   do i=1 to T.repeats until dlg~finished         /* - run the loop   */
         stime = random(T.sleep-T.vsleep,T.sleep+T.vsleep)
         if dlg~finished then leave               /* - stop clicked   */
         self~sleep(stime)                        /* - call sleep     */
         if dlg~finished then leave               /* - stop clicked   */
         self~wait                                /* - call wait      */
         if random(1,100) < T.side then do        /* - pick up forks  */
            self~pickLeft(T.eat>20)               /* - - left first   */
            self~pickRight(T.eat>20)
         end
         else do                                  /* - - right first  */
            self~pickRight(T.eat>20)
            self~pickLeft(T.eat>20)
         end
         etime = random(T.eat-T.veat,T.eat+T.veat)
         if dlg~finished then leave               /* - stop clicked   */
         self~eat(etime)                          /* - call eat       */
         self~layDownLeft                         /* - free forks     */
         self~layDownRight
   end
   self~done
   return 1

::method sleep                                    /* philosoph sleeps */
      expose num dlg
      use arg ds
      dlg~setPhil(num, 'sleep')
      if num=1 & ds>=20 then dlg~audio('sleep')
      if ds > 0 then call msSleep ds*100

::method eat                                      /* philosoph eats   */
      expose num dlg
      use arg ds
      dlg~setPhil(num, 'eat')
      dlg~cake                                    /* - cake smaller   */
      dlg~setPiece(num, 'piece')                  /* - he gets piece  */
      if ds > 0 then do
         if num=1 & ds>=20 then dlg~audio('eat')
         do i = 1 to ds/5 while \dlg~finished    /* - eat,check stop */
            call msSleep 300
            if random(1,50)=11 then
                 dlg~~audio('ouch')~setPhil(num, 'ouch')
            else dlg~setPhil(num, 'eat2')
            call msSleep 200
            dlg~setPhil(num, 'eat')
         end
         if \dlg~finished then call msSleep ds//10 * 100
      end
      dlg~setPiece(num, 'blank')

::method wait                                     /* philosoph waits  */
      expose num dlg
      dlg~setPhil(num, 'wait')

::method pickLeft                                 /* pick left fork   */
      expose num dlg lFork
      use arg sound
      dlg~setLeft(num, 'handl')
      lFork~pickUp(num=1 & sound)
      dlg~setLeft(num, 'handlf')

::method pickRight                                /* pick right fork  */
      expose num dlg rFork
      use arg sound
      dlg~setRight(num, 'handr')
      rFork~pickUp(num=1 & sound)
      dlg~setRight(num, 'handrf')

::method layDownLeft                              /* down left fork   */
      expose num dlg lFork
      dlg~setLeft(num, 'blank')
      lFork~layDown

::method layDownRight                             /* down right fork  */
      expose num dlg rFork
      dlg~setRight(num, 'blank')
      rFork~layDown

::method done                                     /* philosopher done */
      expose num dlg
      dlg~setPhil(num, 'blank')


/*---------------------- fork ----------------------------------------*/

::class fork                                      /*** forks **********/

::method init                                     /* initialize       */
   expose used num dlg
   use arg num, dlg
   used = 0                                       /* - forks are free */

::method pickUp                                   /* pickUp the fork  */
   expose used num dlg
   use arg sound
   if used & sound then dlg~audio('wait')
   guard on when used = 0                         /* - wait until free*/
   used = 1                                       /* - set occupied   */
   dlg~setFork(num, 'blank')

::method layDown unguarded                        /* layDown the fork */
   expose used num dlg
   dlg~setFork(num, 'fork')
   used = 0                                       /* - set to free    */
