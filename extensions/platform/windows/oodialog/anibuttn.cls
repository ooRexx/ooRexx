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

/**
 * Windows Dialog Interface for Open Object Rexx (ooRexx.)
 *
 * Animated button class.
 */

::class 'AnimatedButton' public

::attribute parentDlg
::attribute stopped

::method init
     expose spritewin spos. sprite.

     if ARG() > 2 then
        use arg bid, bmpfrom, bmpto, movx, movy, bmpsizex, bmpsizey, dely, ix, iy, dlg
     else do
        use arg istring, dlg
        parse var istring bid "," bmpfrom"," bmpto"," movx"," movy"," bmpsizex"," bmpsizey"," dely"," ix"," iy
     end

     self~stopped = 0
     if bmpfrom~hasmethod("HASINDEX") then
         sprite.from = bmpfrom
     else do
         if bmpfrom~datatype("N") = 1 & bmpto \= 0 then sprite.from = bmpfrom
         else sprite.from = .local[bmpfrom]
     end
     sprite.to = bmpto
     sprite.movex = movx
     sprite.movey = movy
     sprite.sizex = bmpsizex
     sprite.sizey = bmpsizey
     sprite.delay = dely
     sprite.buttonid = bid
     sprite.smooth = 1
     sprite.step = 1

     self~parentDlg = dlg
     spritewin = self~parentDlg~getControlHandle(sprite.buttonid)
     parse value self~parentDlg~getWindowRect(spritewin) with spos.left spos.top spos.right spos.bottom

     spos.bottom = spos.bottom - spos.top
     spos.right = spos.right - spos.left
     spos.top = 0
     spos.left = 0

     spos.x = ix
     spos.y = iy


::method movepos unguarded
     expose spritewin spos. sprite.
     use arg move.x, move.y
     oldpos.x = spos.x
     oldpos.y = spos.y
     reset.x = 0
     reset.y = 0
     ss.x = 0
     ss.y = 0
     sr.x = sprite.sizex
     sr.y = sprite.sizey
     if sprite.smooth \= 1 then
     do
        ss.x = sprite.sizex
        ss.y = sprite.sizey
        sr.x = 0
        sr.y = 0
     end

     if (move.x > 0) then
     do
       if (spos.x + move.x + ss.x) >= spos.right then reset.x = 1
       else spos.x = spos.x + move.x
     end
     else if (move.x < 0) then do
       if (spos.x + move.x + sr.x) <= spos.left then reset.x = 1
       else spos.x = spos.x + move.x
     end

     if (move.y > 0) then
     do
        if (spos.y + move.y + ss.y) >= spos.bottom then reset.y = 1
        else spos.y = spos.y + move.y
     end
     else if (move.y < 0) then do
        if (spos.y + move.y + sr.y*(sprite.to\=0) ) <= spos.top then reset.y = 1
        else spos.y = spos.y + move.y
     end

     if self~parentDlg~finished = 1 then return 1

     if (reset.x + reset.y) > 0 then do
        if reset.x = 1 then do
           if (move.x > 0) then
           do
              if self~hitright = 1 then spos.x = spos.left - sr.x
           end
           else if (move.x < 0) then do
              if self~hitleft = 1 then spos.x = spos.right - ss.x
           end
        end
        if reset.y = 1 then do
           if (move.y > 0) then
           do
              if self~hitbottom = 1 then spos.y = spos.top - sr.y*(sprite.to\=0)
           end
           else if (move.y < 0) then do
              if self~hittop = 1 then spos.y = spos.bottom - ss.y
           end
        end
        if self~parentDlg~finished = 1 then return 1
        self~parentDlg~ChangeBitmapButton(sprite.buttonid, 0)
        /* self~parentDlg~ClearRect(spritewin, oldpos.x, oldpos.y, (oldpos.x + sprite.sizex), (oldpos.y + sprite.sizey)) */
     end
     if self~parentDlg~finished = 1 then return 1
     self~parentDlg~setBitmapPosition(sprite.buttonid, spos.x, spos.y);


::method setpos unguarded
     expose spritewin spos. sprite.
     use arg new.x, new.y

     if self~parentDlg~finished = 1 then return 1
     self~parentDlg~ChangeBitmapButton(sprite.buttonid, 0)
     self~parentDlg~ClearRect(spritewin, spos.x, spos.y, (spos.x + sprite.sizex), (spos.y + sprite.sizey))
     spos.x = new.x
     spos.y = new.y
     self~parentDlg~setBitmapPosition(sprite.buttonid, spos.x, spos.y);

::method getpos
     expose spos.
     use arg mypos.
     mypos.x = spos.x
     mypos.y = spos.y


::method moveseq unguarded
     expose spritewin spos. sprite.

     if sprite.to > 0 then
     do
        opts = ''
        do actsprite = sprite.from by sprite.step to sprite.to
           if (sprite.movex \= 0) | (sprite.movey \=0) then
              self~movepos(sprite.movex, sprite.movey)
           if self~parentDlg~finished = 1 then return 1
           self~parentDlg~ChangeBitmapButton(sprite.buttonid, actsprite,,,,opts)
           if (sprite.delay \= 0) then ret = msSleep(sprite.delay)
        end
     end
     else do
        opts = "INMEMORY"
        do actsprite over sprite.from
           if (sprite.movex \= 0) | (sprite.movey \=0) then
              self~movepos(sprite.movex, sprite.movey)

           if self~parentDlg~finished = 1 then return 1
           self~parentDlg~ChangeBitmapButton(sprite.buttonid, actsprite,,,,opts)
           if (sprite.delay \= 0) then ret = msSleep(sprite.delay)
        end
     end


::method moveto unguarded
     expose spos. sprite.
     use arg newx, newy

     if (sprite.movex > 0) then mx = 1; else mx = 0
     if (sprite.movey > 0) then my = 1; else my = 0
     if (mx = 1) & (my = 1) then
     do until (spos.x > newx) & (spos.y > newy)
        self~moveseq
     end
     else if (mx = 0) & (my = 0) then
     do until (spos.x < newx) & (spos.y < newy)
        self~moveseq
     end
     else if (mx = 1) & (my = 0) then
     do until (spos.x > newx) & (spos.y < newy)
        self~moveseq
     end
     else if (mx = 0) & (my = 1) then
     do until (spos.x < newx) & (spos.y > newy)
        self~moveseq
     end


::method run unguarded
   do until(self~stopped = 1) | (self~ParentStopped) | self~parentDlg~finished = 1
      self~moveseq
   end
   return 0

::method Parentstopped
   return self~parentDlg~finished

::method stop
   self~stopped = 1

::method setfromto
   expose sprite.
   use arg from, ito
   sprite.from = from
   sprite.to = ito

::method setmove
   expose sprite.
   use arg xx,yy
   sprite.movex = xx
   sprite.movey = yy

::method setdelay
   expose sprite.
   use arg da
   sprite.delay = da

::method setsmooth
   expose sprite.
   use arg sm
   sprite.smooth = sm

::method setstep
   expose sprite.
   use arg st
   sprite.step = st

::method getsprite
   expose sprite.
   use arg osprite.
   do k over sprite.
      osprite.k = sprite.k
   end

::method setsprite
   expose sprite.
   use arg isprite.
   do k over isprite.
      sprite.k = isprite.k
   end

::method hitright unguarded
   return 1

::method hitleft unguarded
   return 1

::method hitbottom unguarded
   return 1

::method hittop unguarded
   return 1
