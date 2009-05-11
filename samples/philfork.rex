#!/usr/bin/rexx
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/****************************************************************************/
/* Name: philfork.rex                                                       */
/* Type: Open Object Rexx Script                                            */
/*                                                                          */
/* Description: Philosophers' Forks: Console window version                 */
/*                                                                          */
/****************************************************************************/


/*---------------------- main logic ----------------------------------*/

   arg parms
   if parms = '' then parms = '8 6 any 2'         /* default values   */
   parse var parms psleep peat pside prepeats
   T.eat = peat
   T.sleep = psleep
   T.veat = trunc(peat / 2)
   T.vsleep = trunc(psleep / 2)
   if      pside = 'L' then T.side = 100          /* left fork first  */
   else if pside = 'R' then T.side = 0            /* right            */
                       else T.side = 50           /* random           */
   T.repeats = prepeats

   f1 = .fork~new(1)                              /* create 5 forks   */
   f2 = .fork~new(2)
   f3 = .fork~new(3)
   f4 = .fork~new(4)
   f5 = .fork~new(5)
   p1 = .phil~new(1,f5,f1)                        /* create 5 philos. */
   p2 = .phil~new(2,f1,f2)
   p3 = .phil~new(3,f2,f3)
   p4 = .phil~new(4,f3,f4)
   p5 = .phil~new(5,f4,f5)

   m1 = p1~start("run",T.)                        /* run 5 philsophers*/
   m2 = p2~start("run",T.)
   m3 = p3~start("run",T.)
   m4 = p4~start("run",T.)
   m5 = p5~start("run",T.)

   m1~result                                      /* wait for finish  */
   m2~result
   m3~result
   m4~result
   m5~result
   return 0

/*---------------------- philosopher ---------------------------------*/

::class phil                                      /*** philosophers ***/

::method init                                     /* initialize       */
   expose num rfork lfork out                     /* - store forks    */
   use arg num, rfork, lfork
   out = ' '~copies(15*num-14)

::method run                                      /* run the philosop.*/
   expose num rfork lfork out
   use arg T.
   x =  random(1,100,time('S')*num)
   say out 'Philosopher-'num
   do i=1 to T.repeats                            /* - run the loop   */
      stime = random(T.sleep-T.vsleep,T.sleep+T.vsleep)
      say out 'Sleep-'stime
      rc=SysSleep(stime)                          /* - sleep          */
      say out 'Wait'
      if random(1,100) < T.side then do           /* - pick up forks  */
         lfork~pickup(1,'left',num)
         rfork~pickup(2,'right',num)
         end
      else do                                     /* - same, right    */
         rfork~pickup(1,'right',num)
         lfork~pickup(2,'left',num)
      end
      etime = random(T.eat-T.veat,T.eat+T.veat)
      say out 'Eat-'etime
      rc=SysSleep(etime)                          /* - eat            */
      lfork~laydown(num)                          /* - lay down forks */
      rfork~laydown(num)
   end
   say out 'Done'                                 /* loop finished    */
   return 1

/*---------------------- fork ----------------------------------------*/

::class fork                                      /*** forks **********/

::method init                                     /* initialize       */
   expose used
   used = 0                                       /* - forks are free */

::method pickup                                   /* pickup the fork  */
   expose used
   guard on when used = 0                         /* - wait until free*/
   used = 1                                       /* - set occupied   */

::method laydown unguarded                        /* laydown the fork */
   expose used
   used = 0                                       /* - set to free    */
