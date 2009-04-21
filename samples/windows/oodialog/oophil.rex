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
/* OODialog\Samples\oophil.rex    Philosophers' Forks                       */
/*                                                                          */
/*--------------------------------------------------------------------------*/


  curdir = directory()
  parse source . . me
  mydir = me~left(me~lastpos('\')-1)             /* install directory */
  mydir = directory(mydir)                       /* current is "my"   */

/*---------------------- run default parameters ----------------------*/

  parms.101 = 80                 /* sleeping time 80 * 100 ms = 8 sec */
  parms.102 = 50                 /* eating   time 50 * 100 ms = 5 sec */
  parms.103 = 3                  /* 3 repetitions                     */
  parms.104 = 0                  /* radio button left  fork first off */
  parms.105 = 0                  /*              right fork first off */
  parms.106 = 1                  /*              any   fork first on  */

/*---------------------- dialogs & resources -------------------------*/

  v.anidialog = 100              /* animation dialog graphical        */
  v.anidialog = 'rc\oophil2.rc'
  v.setdialog = 101              /* setup dialog for parameters       */
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

  setdlg = .setupdialog~new(parms., v., vb.)
  ret = setdlg~execute("SHOWTOP")
  setdlg~deinstall
  ret = directory(curdir)
  return


/*---------------------- requires OODIALOG ----------------------------*/

::requires 'oodialog.cls'


/*---------------------- setup dialog ---------------------------------*/

::class setupdialog subclass userdialog

::method init
   expose v. vb.
   use arg parms., v., vb.
   self~init:super(parms.)
   self~load(v.setdialog,101,'CENTER')

::method InitDialog
   expose msg v. vb.
   msg = .NIL
   self~InitDialog:super
   self~ConnectScrollBar(107, 'SLEEPDN', 'SLEEPUP',, 0, 1000, 1)
   self~ConnectScrollBar(108, 'EATDN', 'EATUP',, 0, 1000, 1)
   self~ConnectScrollBar(109, 'REPDN', 'REPUP',, 0, 1000, 1)
   do i over vb.                               /* bitmaps in memory */
     v.i = self~LoadBitmap(vb.i)
   end

::method SleepDN
   self~CombineELwithSB(101, 107, +5)
   return 0

::method SleepUP
   self~CombineELwithSB(101, 107, -5)
   return 0

::method EatDN
   self~CombineELwithSB(102, 108, +5)
   return 0

::method EatUP
   self~CombineELwithSB(102, 108, -5)
   return 0

::method RepDN
   self~CombineELwithSB(103, 109, +1)
   return 0

::method RepUP
   self~CombineELwithSB(103, 109, -1)
   return 0

::method help
   expose msg v.
   if msg = .NIL then         ret = Play(v.help, 'yes')
   else if msg~completed then ret = Play(v.help, 'yes')
   else                       ret = Play()
   msg = self~start("ScrollInButton",110, v.helptext, ,
                    "Arial", 42, "BOLD",0, 4, 8)
   return 0

::method OK                                   /* run philosophers      */
   expose msg v.
   if msg \= .nil then
      if msg~completed=0 then self~ScrollInButton(110)
   self~getDataStem(parms.)
                                              /* philosopher dialog    */
   dlg = .phildlg~new(v.)
   if dlg~ExecuteAsync(,"SHOWTOP") = 0 then do
      dlg~myexecute(parms.)                      /* philosopher animation */
      dlg~EndAsyncExecution
   end
   else call errorDialog "Couldn't execute Philosophers Forks Dialog"
   dlg~deinstall

::method Cancel
   expose msg v. vb.
   self~Cancel:super
   if msg \= .NIL then
      if msg~completed=0 then self~ScrollInButton(110)
   do i over vb.                              /* bitmaps out of memory */
      self~RemoveBitmap(v.i)
   end


/*---------------------- animation dialog -----------------------------*/

::class phildlg subclass UserDialog

::method init
   expose v.
   use arg v.
   self~init:super(empty.)
   self~load(v.anidialog,100,'CENTER')

::method InitDialog
   expose f1 f2 f3 f4 f5 p1 p2 p3 p4 p5 v.
   self~InitDialog:super
   self~disableItem(1)                          /* stop button      */
   do i = 1 to 5
      ret = self~ConnectBitmapButton(v.idp  + i,    '', v.bmpphil ,,,,"STRETCH INMEMORY")
      ret = self~ConnectBitmapButton(v.idf  + i,    '', v.bmpfork ,,,,"STRETCH INMEMORY")
      ret = self~ConnectBitmapButton(v.idhl + 10*i, '', v.bmpblank,,,,"STRETCH INMEMORY")
      ret = self~ConnectBitmapButton(v.idhr + 10*i, '', v.bmpblank,,,,"STRETCH INMEMORY")
      ret = self~ConnectBitmapButton(v.idpc + i,    '', v.bmpblank,,,,"STRETCH INMEMORY")
   end
   ret    = self~ConnectBitmapButton(v.idcake,      '', v.bmpblank,,,,"STRETCH INMEMORY")

   f1 = .fork~new(1, self)                      /* create 5 forks   */
   f2 = .fork~new(2, self)
   f3 = .fork~new(3, self)
   f4 = .fork~new(4, self)
   f5 = .fork~new(5, self)

   p1 = .phil~new(1,f5,f1, self)                /* create 5 philos. */
   p2 = .phil~new(2,f1,f2, self)
   p3 = .phil~new(3,f2,f3, self)
   p4 = .phil~new(4,f3,f4, self)
   p5 = .phil~new(5,f4,f5, self)

::method myexecute                              /* animate dialog   */
   expose f1 f2 f3 f4 f5 p1 p2 p3 p4 p5
   use arg parms.
   T.sleep = parms.101
   T.eat = parms.102
   T.veat = trunc(T.eat / 2)
   T.vsleep = trunc(T.sleep / 2)
   if parms.104 = 1 then T.side = 100           /* left fork first  */
   else if parms.105 = 1 then T.side = 0        /* right            */
                         else T.side = 50       /* random           */
   T.repeats = parms.103

   self~cake('init')                            /* set up the cake  */

   m1 = p1~start("run",T.)                      /* run 5 philsophers*/
   m2 = p2~start("run",T.)
   m3 = p3~start("run",T.)
   m4 = p4~start("run",T.)
   m5 = p5~start("run",T.)
   self~enableItem(1)                           /* stop button      */
                                                /* wait till done   */
   do while(m1~completed+m2~completed+m3~completed+m4~completed+m5~completed <5),
            & (self~finished = 0)
      self~HandleMessages                       /* stop button ?    */
   end
   m1~result                                    /* check 5 phils    */
   m2~result
   m3~result
   m4~result
   m5~result
   self~ok:super                                /* finish dialog    */

::method OK                                     /* Stop button      */
   expose f1 f2 f3 f4 f5 p1 p2 p3 p4 p5 v.
   self~DisableItem(1)
   call Play v.stop,'yes'
   self~ok:super                                /* sets finished    */
   f1~laydown                                   /* take away forks  */
   f2~laydown
   f3~laydown
   f4~laydown
   f5~laydown

::method setphil unguarded                      /* philosoph bitmap */
   expose v.
   use arg num, bmp
   self~ChangeBitmapButton(v.idp + num, value('v.bmp'bmp),,,,'STRETCH INMEMORY')

::method setfork unguarded                      /* fork bitmap      */
   expose v.
   use arg num, bmp
   self~ChangeBitmapButton(v.idf + num, value('v.bmp'bmp),,,,'STRETCH INMEMORY')

::method setleft unguarded                      /* left hand bitmap */
   expose v.
   use arg num, bmp
   self~ChangeBitmapButton(v.idhl + num*10, value('v.bmp'bmp),,,,'STRETCH INMEMORY')

::method setright unguarded                     /* righthand bitmap */
   expose v.
   use arg num, bmp
   self~ChangeBitmapButton(v.idhr + num*10, value('v.bmp'bmp),,,,'STRETCH INMEMORY')

::method setpiece unguarded                     /* cakepiece bitmap */
   expose v.
   use arg num, bmp
   self~ChangeBitmapButton(v.idpc + num, value('v.bmp'bmp),,,,'STRETCH INMEMORY')

::method cake unguarded                         /* cake bitmap      */
   expose curcake v.
   if arg() = 1 then do
        curcake = -1
        self~audio('cakewhere')
        call msSleep 2000
   end
   curcake = (curcake+1)//11
   i = curcake + 1
   self~ChangeBitmapButton(v.idcake, value('v.bmpcake'i),,,,'STRETCH INMEMORY')
   if curcake=10 then self~audio('cakenew')

::method audio unguarded                        /* play a sound     */
   expose v.
   use arg act
   ret = Play( value('v.'act), 'yes')


/*---------------------- philosopher ---------------------------------*/

::class phil                                      /*** philosophers ***/

::method init                                     /* initialize       */
   expose num rfork lfork dlg
   use arg num, rfork, lfork, dlg

::method run                                      /* run the philosop.*/
   expose num rfork lfork dlg
   use arg T.
   x =  random(1,100,time('S')*num)
   do i=1 to T.repeats until dlg~finished         /* - run the loop   */
         stime = random(T.sleep-T.vsleep,T.sleep+T.vsleep)
         self~sleep(stime)                        /* - call sleep     */
         if dlg~finished then leave               /* - stop clicked   */
         self~wait                                /* - call wait      */
         if random(1,100) < T.side then do        /* - pick up forks  */
            self~pickleft(T.eat>20)               /* - - left first   */
            self~pickright(T.eat>20)
            end
         else do                                  /* - - right first  */
            self~pickright(T.eat>20)
            self~pickleft(T.eat>20)
         end
         etime = random(T.eat-T.veat,T.eat+T.veat)
         if dlg~finished then leave               /* - stop clicked   */
         self~eat(etime)                          /* - call eat       */
         self~laydownleft                         /* - free forks     */
         self~laydownright
   end
   self~done
   return 1

::method sleep                                    /* philosoph sleeps */
      expose num dlg
      use arg ds
      dlg~setphil(num, 'sleep')
      if num=1 & ds>=20 then dlg~audio('sleep')
      if ds > 0 then call msSleep ds*100

::method eat                                      /* philosoph eats   */
      expose num dlg
      use arg ds
      dlg~setphil(num, 'eat')
      dlg~cake                                    /* - cake smaller   */
      dlg~setpiece(num, 'piece')                  /* - he gets piece  */
      if ds > 0 then do
         if num=1 & ds>=20 then dlg~audio('eat')
         do i = 1 to ds/5 while dlg~finished=0   /* - eat,check stop */
            call msSleep 300
            if random(1,50)=11 then
                 dlg~~audio('ouch')~setphil(num, 'ouch')
            else dlg~setphil(num, 'eat2')
            call msSleep 200
            dlg~setphil(num, 'eat')
         end
         call msSleep ds//10 * 100
      end
      dlg~setpiece(num, 'blank')

::method wait                                     /* philosoph waits  */
      expose num dlg
      dlg~setphil(num, 'wait')

::method pickleft                                 /* pick left fork   */
      expose num dlg lfork
      use arg sound
      dlg~setleft(num, 'handl')
      lfork~pickup(num=1 & sound)
      dlg~setleft(num, 'handlf')

::method pickright                                /* pick right fork  */
      expose num dlg rfork
      use arg sound
      dlg~setright(num, 'handr')
      rfork~pickup(num=1 & sound)
      dlg~setright(num, 'handrf')

::method laydownleft                              /* down left fork   */
      expose num dlg lfork
      dlg~setleft(num, 'blank')
      lfork~laydown

::method laydownright                             /* down right fork  */
      expose num dlg rfork
      dlg~setright(num, 'blank')
      rfork~laydown

::method done                                     /* philosopher done */
      expose num dlg
      dlg~setphil(num, 'blank')


/*---------------------- fork ----------------------------------------*/

::class fork                                      /*** forks **********/

::method init                                     /* initialize       */
   expose used num dlg
   use arg num, dlg
   used = 0                                       /* - forks are free */

::method pickup                                   /* pickup the fork  */
   expose used num dlg
   use arg sound
   if used & sound then dlg~audio('wait')
   guard on when used = 0                         /* - wait until free*/
   used = 1                                       /* - set occupied   */
   dlg~setfork(num, 'blank')

::method laydown unguarded                        /* laydown the fork */
   expose used num dlg
   dlg~setfork(num, 'fork')
   used = 0                                       /* - set to free    */
