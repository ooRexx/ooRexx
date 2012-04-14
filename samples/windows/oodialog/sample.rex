/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2012 Rexx Language Association. All rights reserved.    */
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
/* samples\ooDialog\sample.rex    ooDialog Samples - Main                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/


  -- A directory manager saves the current directory and can later go back to
  -- that directory.  It also sets up the environment we need.  The class
  -- itself is located in samplesSetup.rex
  mgr = .DirectoryManager~new()

  d = .SampleDlg~new("rc\sample.rc", 100)
  if d~initCode <> 0 then do
     mgr~goBack
     return 99
  end
  d~execute("SHOWTOP")
  mgr~goBack
  return 0


/*---------------------------- requires ------------------------------*/

::requires "ooDialog.cls"
::requires 'samplesSetup.rex'

/*---------------------------- main dialog ---------------------------*/

::class 'SampleDlg' subclass RcDialog

::method initDialog
   self~installBitmapButton(101, "VIDEO",    "bmp\s2arch.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(102, "PET",      "bmp\s2anim.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(103, "PHIL",     "bmp\s2philf.bmp" ,,,,"FRAME STRETCH")
   self~installBitmapButton(104, "GRAPHD",   "bmp\s2scroll.bmp",,,,"FRAME STRETCH")
   self~installBitmapButton(105, "WALKER",   "bmp\s2walker.bmp",,,,"FRAME STRETCH")
   self~installBitmapButton(106, "BANDIT",   "bmp\s2jack.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(107, "USER",     "bmp\s2input.bmp" ,,,,"FRAME STRETCH")
   self~installBitmapButton(108, "WIZ97",    "bmp\s2mov.bmp"   ,,,,"FRAME STRETCH")
   self~installBitmapButton(109, "TREE",     "bmp\s2tree.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(110, "LIST",     "bmp\s2list.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(111, "PROGRESS", "bmp\s2prog.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(112, "PROPERTY", "bmp\s2prop.bmp"  ,,,,"FRAME STRETCH")
   self~backgroundBitmap("bmp\s2backg.bmp", "USEPAL")

::method video
   self~loadApp("oovideo.rex")

::method pet
   self~loadApp("AnimalGame.rex", 3000)

::method phil
   self~loadApp("oophil.rex")

::method walker
   self~loadApp("oowalker.rex")

::method bandit
   self~loadApp("oobandit.rex", 3300)

::method graphd
   self~loadApp("oograph.rex", 3000)

::method user
   self~loadApp("oostddlg.rex", 2800)

::method wiz97
   self~loadApp("propertySheet.tabs\ticketWizard.rex")

::method tree
   self~loadApp("oodtree.rex")

::method list
   self~loadApp("propertySheet.tabs\oodListViews.rex")

::method progress
   self~loadApp("oodpbar.rex")

::method property
   self~loadApp("propertySheet.tabs\PropertySheetDemo.rex")


::method cancel
   call play "byebye.wav"
   self~cancel:super
   return 0

::method ok
   self~cancel

::method help                               /* About button */
   call play "sample.wav", "YES"
   d = .TimedMessage~new("Illustration of ooDialog Function", ,
                         "Open Object Rexx ooDialog Samples", 5000)
   d~execute
   return 0

::method loadApp
   use arg appname, pauseTime = 2000
   ret = play("start.wav", "yes")
   d = .TimedMessage~new("Application will be started, please wait","Samples", pauseTime)
   d~execute
   /* save current directory */
   curDir = Directory()
   call (appname)
   /* switch back to previous directory */
   ret = Directory(curDir)
   /* make sure main window is enabled and the topmost window */
   self~enable
   self~toTheTop
