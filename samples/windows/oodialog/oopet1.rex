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
/* OODialog\Samples\oopet1.rex    Animals Riddle (bitmaps from DLL)         */
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

 do i = 1001 to 1008
    b.i = "unknown animal"
 end

 d = .peddialog~new("RES\OOPet.DLL",103, b.)
 if d~InitCode \= 0 then exit
 d~execute("SHOWTOP")
 d~deinstall
 ret = directory(curdir)
 return

/*------------------------------- requires ---------------------------*/

::requires "OODIALOG.CLS"

/*------------------------------- dialog class -----------------------*/

::class peddialog subclass Resdialog

::method InitDialog
   expose correct beenhelped
   self~InitDialog:super
   call Play "guess.wav", yes
   self~ConnectBitmapButton(2001, "IDRHINO",     201 ,,   ,,"FRAME")
   self~ConnectBitmapButton(2002, "IDTIGER",     202 ,,   ,,"FRAME")
   self~ConnectBitmapButton(2003, "IDELEPH",     203 ,,   ,,"FRAME")
   self~ConnectBitmapButton(2004, "IDMOOSE",     204 ,,   ,,"FRAME")
   self~ConnectBitmapButton(2005, "IDGOAT",      205 ,,   ,,"FRAME")
   self~ConnectBitmapButton(2006, "IDCHIHUAHUA", 206 ,,209,,"FRAME")
   self~ConnectBitmapButton(2007, "IDSEA",       207 ,,   ,,"FRAME")
   self~ConnectBitmapButton(2008, "IDHORSE",     208 ,,   ,,"FRAME")
   correct = .array~of("rhinoceros","tiger","elephant","moose","goat","chihuahua","seal", "horse")
   beenhelped = 0

::method Validate
   expose correct beenhelped
   self~GetDataStem(A.)
   wrongstr = ''
   do i = 1001 to 1008
      if A.i \= correct[i-1000] & A.i~space(0) \= "" then do
         if wrongstr = '' then wrongstr = i-1000": "A.i
                          else wrongstr = wrongstr", " i-1000": "A.i
      end
   end
   if wrongstr = '' then do
      if beenhelped=0 then call Play("clap.wav")
      call Play "yourgood.wav","YES"
      if beenhelped=1 then
           ret = TimedMessage("You got them all right.... with my help ","E N D",2000)
      else ret = TimedMessage("You got them all right","B R A V O",2000)
      return 1
      end
   else do
      call Play "nope.wav"
      ret = ErrorMessage("The following answer(s) is/are incorrect: "wrongstr)
      return 0
   end

::method IDTIGER
   ret = Play("TIGER.WAV","YES")
   ret = TimedMessage("Hold that t...., hold that .i...","A song about me",2000)

::method IDELEPH
   ret = Play("ELEPHANT.WAV","YES")
   ret = TimedMessage("I blow my nose like a trumpet","African Heavy Weight",2000)

::method IDMOOSE
   ret = Play("MOOSE.WAV","YES")
   ret = TimedMessage("My name rhymes with a sweet brown dessert","Chocolate ......",2000)

::method IDRHINO
   ret = Play("RHINO.WAV","YES")
   ret = TimedMessage("I only fear the 2 guys on my right","I am strong",2000)

::method IDGOAT
   ret = Play("GOAT.WAV","YES")
   ret = TimedMessage("My relatives climb the Matterhorn","Mountain ....",2000)

::method IDCHIHUAHUA
   ret = Play("NOPE.WAV","YES")

::method IDSEA
   ret = Play("SEALION.WAV","YES")
   ret = TimedMessage("I am slick in the water","Hint 4 you",2000)

::method IDHORSE
   ret = Play("HORSE.WAV","YES")
   ret = InfoMessage("My son won the Kentucky Derby")

::method help
   expose correct beenhelped
   beenhelped = 1
   call Play("help.wav")
   self~GetDataStem(A.)
   do i = 1001 to 1008
      A.i = correct[i-1000]
   end
   self~SetDataStem(A.)
