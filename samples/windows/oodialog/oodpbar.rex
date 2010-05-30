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
/****************************************************************************/
/* Name: oodpbar.rex                                                        */
/* Type: Object REXX Script using ooDialog                                  */
/* Resource: oodpbar.rc                                                     */
/*                                                                          */
/* Description:                                                             */
/* This file has been created by the Object REXX Workbench ooDialog         */
/* template generator.                                                      */
/*                                                                          */
/****************************************************************************/


/* Install signal handler to catch error conditions and clean up */
signal on any name CleanUp

myDialog = .MyDialogClass~new
if myDialog~initCode = 0 then do
  rc = myDialog~execute("SHOWTOP")
end

/* Add program code here */

exit   /* leave program */


/* ---- signal handler to destroy dialog if error condition was raised  ----*/
CleanUp:
   call errorDialog "Error" rc "occurred at line" sigl":" errortext(rc),
                     || "a"x || condition("o")~message
   if myDialog~isDialogActive then do
     myDialog~finished = .true
     myDialog~stopIt
   end


::requires "ooDialog.cls"    /* This file contains the ooDialog classes */

/* ---------------------------- Directives ---------------------------------*/

::class MyDialogClass subclass UserDialog

::method Init
  forward class (super) continue /* call parent constructor */
  InitRet = Result

  if self~Load("rc\Oodpbar.rc", ) \= 0 then do
     self~InitCode = 1
     return 1
  end

  /* Connect dialog control items to class methods. Ok, Cancel, and Help are
   * already connected by the super class init()
   */

  /* Initial values that are assigned to the object attributes */

  /* Add your initialization code here */
  return InitRet


::method InitDialog

  /* Initialize progress bar ID_A */
  curPB = self~newProgressBar("ID_A")
  if curPB \= .Nil then do
     curPB~SetRange(0,100)
     curPB~SetStep(50)
     curPB~SetPos(10)
  end

  /* Initialize progress bar ID_B */
  curPB = self~newProgressBar("ID_B")
  if curPB \= .Nil then do
     curPB~SetRange(0,100)
     curPB~SetStep(40)
     curPB~SetPos(20)
  end

  /* Initialize progress bar ID_C */
  curPB = self~newProgressBar("ID_C")
  if curPB \= .Nil then do
     curPB~SetRange(0,100)
     curPB~SetStep(30)
     curPB~SetPos(30)
  end

  /* Initialize progress bar ID_D */
  curPB = self~newProgressBar("ID_D")
  if curPB \= .Nil then do
     curPB~SetRange(0,100)
     curPB~SetStep(20)
     curPB~SetPos(40)
  end

  /* Initialize progress bar ID_E */
  curPB = self~newProgressBar("ID_E")
  if curPB \= .Nil then do
     curPB~SetRange(0,100)
     curPB~SetStep(10)
     curPB~SetPos(50)
  end

  /* Initialization Code (e.g. fill list and combo boxes) */
  return InitDlgRet


::method DefineDialog

  if result = 0 then do
     /* Additional dialog items (e.g. createEditInputGroup) */
  end


/* --------------------- message handler -----------------------------------*/

  /* Method Ok is connected to item 1 */
::method Ok
  /* step each progress bar by the step set with the SetStep method */
  curPB = self~newProgressBar("ID_A")
  curPB~Step
  curPB = self~newProgressBar("ID_B")
  curPB~Step
  curPB = self~newProgressBar("ID_C")
  curPB~Step
  curPB = self~newProgressBar("ID_D")
  curPB~Step
  curPB = self~newProgressBar("ID_E")
  curPB~Step
  return 0

  /* Method Cancel is connected to resource ID 2 */
::method Cancel
  /* Calling the superclass cancel will *always* close the dialog. If you want
   * to over-ride that behavior, for example to ask the user if she is sure she
   * wants to cancel, do not call the super class cancel first.  Here, commented
   * out, is one possible over-ride.
   */

  /*
  text = 'There are unsaved changes, are you'.endOfLine'sure you want to cancel?'
  title = "Abort All Changes !"
  ret = MessageDialog(text, self~hwnd, title, "YESNO", "WARNING", "DEFBUTTON2")
  say 'ret' ret
  if ret == self~constDir[IDYES] then return self~cancel:super
  else return 1
  */

  return self~Cancel:super

  /* Method Help is connected to resource ID 9. The super class does nothing,
   * you need to over-ride this method to actually do anything.
   */
::method Help
  self~Help:super  -- Will do absolutely nothing.

  text = "There is no help for oodpbar.rex"
  title = "The ooDialog ProgressBar Example"

  ret = MessageDialog(text, self~hwnd, title, "OK", "EXCLAMATION")

