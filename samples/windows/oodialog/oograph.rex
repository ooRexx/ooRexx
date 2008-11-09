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
/****************************************************************************/
/* Name: OOGRAPH.REX                                                       */
/* Type: Object REXX Script                                                 */
/*                                                                          */
/* Description: Graphical demonstration                                     */
/*                                                                          */
/****************************************************************************/

 parse source . . me
 mydir = me~left(me~lastpos('\')-1)              /* where is code     */
 env = 'ENVIRONMENT'
 sp = value('SOUNDPATH',,env)
 sp = value('SOUNDPATH',mydir'\WAV;'sp,env)

 d = .GraphDialog~new()
 if d~InitCode \= 0 then do
   say "Dialog init did not work"
   exit
 end

 d~createCenter(570 / d~FactorX, 440 / d~FactorY, ,
                "Graphical Demonstration of Open Object Rexx and OODialog Capabilities")
 d~execute("SHOWTOP")
 d~deinstall
 return

/*-------------------------------- requires --------------------------*/

::requires "OODIALOG.CLS"

/*-------------------------------- dialog class ----------------------*/

::class GraphDialog subclass UserDialog

::method DefineDialog
   expose but2size
   but2pos  = 140 / self~FactorY
   but2size = 300 / self~FactorY
   self~AddBitmapButton(101,1,10,self~SizeX-1, 110 / self~FactorY,,,"bmp\install.bmp",,,,"USEPAL")
   self~AddBitmapButton(102,10,but2pos,self~SizeX - 20,but2size,,,"bmp\install2.bmp")

   self~AddWhiteFrame(10, self~SizeY - 52, self~SizeX-20, 24,"HIDDEN", 203)
   self~AddButton(103,12, self~SizeY - 50, self~SizeX-24, 20,,,"OWNER NOTAB")
   self~DisplaceBitmap(101,self~SizeX * self~FactorX+10, 0)
   self~DisplaceBitmap(102, -450, 100)
   self~AddButtonGroup(self~SizeX-220, self~SizeY - 18,60,12, ,
            "&Bitmap-Viewer 111 BmpView &Draw-Color-Demo 112 OODraw &Cancel 2 CANCEL", 1, "DEFAULT")

::method initDialog

   -- We set the background color of these buttons to the same backgroud color
   -- as the dialog, so that the buttons blend into the dialog.
   COLOR_BTNFACE = 15
   self~setItemSysColor(103, COLOR_BTNFACE)
   self~setItemSysColor(101, COLOR_BTNFACE)
   self~setItemSysColor(102, COLOR_BTNFACE)

::method Run unguarded
   expose m but2size
   bmppos = but2size - 125 / self~FactorY
   self~DisableItem(111)  /* disable push buttons */
   self~DisableItem(112)
   self~DisableItem(2)

   ret = play("inst.wav", yes)
   self~ScrollBitmapFromTo(101, self~SizeX * self~FactorX, 5, 5, 5, -12, 0, 1)
   self~ScrollBitmapFromTo(102, 30, bmppos, 30, 0, 0, -3, 2, 1)

   self~ResizeItem(102, self~SizeX-40, self~SizeY-120, "NOREDRAW")

   self~ShowItem(103)     /* show scroll button */
   self~ShowItem(203)
   self~EnableItem(2)     /* Enable push buttons */
   self~EnableItem(111)
   self~EnableItem(112)
                          /* asynchronuous scroll */
   m = self~start("ScrollInButton",103,"This OODialog sample demonstrates dynamic dialog creation", ,
                   "Arial", 36, "BOLD", 2,2,2,6)
   do while self~finished = 0 & m~completed = 0
      self~HandleMessages
   end
   if m~completed = 0 then self~ScrollInButton(103) /* end scroll */

   do while self~finished = 0
      /* scroll asynchronously so scrolling can be interrupted when button is pressed */
      m = self~start("ScrollInButton", 103, "... please press Bitmap-Viewer or Draw-Color-Demo buttons to run graphical applications ...", ,
                     "Arial", 32, "SEMIBOLD", 7, 2,4)

      do while self~finished = 0 & m~completed = 0
         self~HandleMessages
      end
      /* if still scrolling, end the scroll */
      if m~completed = 0 then self~ScrollInButton(103)
   end

::method BmpView                  /* invoke Bitmap Viewer OOBMPVU.REX */
   expose m
   if m~completed = 0 then self~ScrollInButton(103)
   self~hideWindow(self~get)
   call "oobmpvu.rex"
   self~~showWindow(self~get)~toTheTop
   self~ScrollInButton(103)       /* restart scrolling */

::method OODraw                   /* Invoke Color Draw Demo OODRAW.REX */
   expose m
   if m~completed = 0 then self~ScrollInButton(103)
   self~hideWindow(self~get)
   call "oodraw.rex"
   self~~showWindow(self~get)~toTheTop
   self~ScrollInButton(103)       /* restart scrolling */
