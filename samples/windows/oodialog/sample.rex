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
/* OODialog\Samples\sample.rex    OODialog Samples - Main                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/


  curdir = directory()
  parse source . . me
  mydir = me~left(me~lastpos('\')-1)             /* where is code     */
  mydir = directory(mydir)                       /* current is "my"   */
  env = 'ENVIRONMENT'
  win = value('WINDIR',,env)
  sp = value('SOUNDPATH',,env)
  sp = value('SOUNDPATH',win';'mydir'\WAV;'sp,env)

  d = .SampleDlg~new
  if d~InitCode > 0 then exit
  d~Execute("SHOWTOP")
  d~deinstall
  ret = directory(curdir)
  return


/*---------------------------- requires ------------------------------*/

::requires "ooDialog.cls"

/*---------------------------- main dialog ---------------------------*/

::class SampleDlg subclass UserDialog

::method init
   self~init:super()
   self~InitCode = self~load("rc\sample.rc", 100)

::method initDialog
   self~InitDialog:super
   self~installBitmapButton(101, "VIDEO",    "bmp\s2arch.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(102, "PET",      "bmp\s2anim.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(103, "PHIL",     "bmp\s2philf.bmp" ,,,,"FRAME STRETCH")
   self~installBitmapButton(104, "GRAPHD",   "bmp\s2scroll.bmp",,,,"FRAME STRETCH")
   self~installBitmapButton(105, "WALKER",   "bmp\s2walker.bmp",,,,"FRAME STRETCH")
   self~installBitmapButton(106, "BANDIT",   "bmp\s2jack.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(107, "USER",     "bmp\s2input.bmp" ,,,,"FRAME STRETCH")
   self~installBitmapButton(108, "CATEGORY", "bmp\s2mov.bmp"   ,,,,"FRAME STRETCH")
   self~installBitmapButton(109, "TREE",     "bmp\s2tree.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(110, "LIST",     "bmp\s2list.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(111, "PROGRESS", "bmp\s2prog.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(112, "PROPERTY", "bmp\s2prop.bmp"  ,,,,"FRAME STRETCH")
   self~BackgroundBitmap("bmp\s2backg.bmp", "USEPAL")

::method video
   self~loadapp("oovideo.rex")

::method pet
   self~loadapp("oopet.rex", 3000)

::method phil
   self~loadapp("oophil.rex")

::method walker
   self~loadapp("oowalker.rex")

::method bandit
   self~loadapp("oobandit.rex", 3300)

::method graphd
   self~loadapp("oograph.rex", 3000)

::method user
   self~loadapp("oostddlg.rex", 2800)

::method category
   self~loadapp("ooticket.rex")

::method tree
   self~loadapp("oodtree.rex")

::method list
   self~loadapp("oodlist.rex")

::method progress
   self~loadapp("oodpbar.rex")

::method property
   self~loadapp("propdemo.rex")


::method cancel
   call Play "byebye.wav"
   self~CANCEL:super
   return 0

::method OK
   self~cancel

::method help                               /* About button */
   call Play "sample.wav","YES"
   d = .TimedMessage~new("Illustration of OODialog Function", ,
                         "Open Object Rexx OODialog Samples", 5000)
   d~execute
   return 0

::method loadapp
   use arg appname, pauseTime = 2000
   ret = Play("start.wav", "yes")
   d = .TimedMessage~new("Application will be started, please wait","Samples", pauseTime)
   d~execute
   /* save current directory */
   curDir = Directory()
   call (appname)
   /* switch back to previous directory */
   ret = Directory(curDir)
   /* make sure main window is enabled and the topmost window */
   self~enable
   self~tothetop
