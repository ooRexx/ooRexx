/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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


  -- Ensure we can find our resource files.
  srcDir = locate()

  d = .SampleDlg~new(srcDir"rc\sample.rc", 100)
  if d~initCode <> 0 then do
     return 99
  end
  d~execute("SHOWTOP")
  return 0


/*---------------------------- requires ------------------------------*/

::requires "ooDialog.cls"
::requires 'samplesSetup.rex'

/*---------------------------- main dialog ---------------------------*/

::class 'SampleDlg' subclass RcDialog

::method initDialog
   sd = .application~srcDir
   self~installBitmapButton(101, "VIDEO",    sd"bmp\s2arch.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(102, "PET",      sd"bmp\s2anim.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(103, "PHIL",     sd"bmp\s2philf.bmp" ,,,,"FRAME STRETCH")
   self~installBitmapButton(104, "GRAPHD",   sd"bmp\s2scroll.bmp",,,,"FRAME STRETCH")
   self~installBitmapButton(105, "WALKER",   sd"bmp\s2walker.bmp",,,,"FRAME STRETCH")
   self~installBitmapButton(106, "BANDIT",   sd"bmp\s2jack.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(107, "USER",     sd"bmp\s2input.bmp" ,,,,"FRAME STRETCH")
   self~installBitmapButton(108, "WIZ97",    sd"bmp\s2mov.bmp"   ,,,,"FRAME STRETCH")
   self~installBitmapButton(109, "TREE",     sd"bmp\s2tree.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(110, "LIST",     sd"bmp\s2list.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(111, "PROGRESS", sd"bmp\s2prog.bmp"  ,,,,"FRAME STRETCH")
   self~installBitmapButton(112, "PROPERTY", sd"bmp\s2prop.bmp"  ,,,,"FRAME STRETCH")
   self~backgroundBitmap(sd"bmp\s2backg.bmp", "USEPAL")

::method video
   self~loadApp("oovideo.rex")

::method pet
   self~loadApp("AnimalGame.rex", 3000)

::method phil
   self~loadApp("oophil.rex")

::method walker
   self~loadApp("oowalk2.rex")

::method bandit
   self~loadApp("oobandit.rex", 3300)

::method graphd
   self~loadApp("oograph.rex", 3000)

::method user
   self~loadApp("oodStandardDialogs.rex", 2800)

::method wiz97
   self~loadApp(.application~srcDir"propertySheet.tabControls\ticketWizard.rex")

::method tree
   self~loadApp(.application~srcDir"controls\TreeView\treeViewCustomDraw.rex")

::method list
   self~loadApp(.application~srcDir"propertySheet.tabControls\oodListViews.rex")

::method progress
   self~loadApp("oodpbar.rex")

::method property
   self~loadApp(.application~srcDir"propertySheet.tabControls\PropertySheetDemo.rex")


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

   call (appname)

   /* make sure main window is enabled and the topmost window */
   self~enable
   self~toTheTop
