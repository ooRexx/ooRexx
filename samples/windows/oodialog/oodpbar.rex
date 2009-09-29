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
/* Name: oodpbar.rex                                                        */
/* Type: Object REXX Script using OODialog                                  */
/* Resource: oodpbar.rc                                                     */
/*                                                                          */
/* Description:                                                             */
/* This file has been created by the Object REXX Workbench OODIALOG         */
/* template generator.                                                      */
/*                                                                          */
/****************************************************************************/


/* Install signal handler to catch error conditions and clean up */
signal on any name CleanUp

MyDialog = .MyDialogClass~new
if MyDialog~InitCode = 0 then do
  rc = MyDialog~Execute("SHOWTOP")
end

/* Add program code here */

exit   /* leave program */


/* ---- signal handler to destroy dialog if error condition was raised  ----*/
CleanUp:
   call errorDialog "Error" rc "occurred at line" sigl":" errortext(rc),
                     || "a"x || condition("o")~message
   if MyDialog~IsDialogActive then MyDialog~StopIt


::requires "ooDialog.cls"    /* This file contains the OODIALOG classes */

/* ---------------------------- Directives ---------------------------------*/

::class MyDialogClass subclass UserDialog inherit AdvancedControls

::method Init
  forward class (super) continue /* call parent constructor */
  InitRet = Result

  if self~Load("rc\Oodpbar.rc", ) \= 0 then do
     self~InitCode = 1
     return 1
  end

  /* Connect dialog control items to class methods */
  self~ConnectButton(1,"Ok")
  self~ConnectButton(2,"Cancel")
  self~ConnectButton(9,"Help")

  /* Initial values that are assigned to the object attributes */

  /* Add your initialization code here */
  return InitRet


::method InitDialog
  InitDlgRet = self~InitDialog:super

  /* Initialize progress bar ID_A */
  curPB = self~GetProgressBar("ID_A")
  if curPB \= .Nil then do
     curPB~SetRange(0,100)
     curPB~SetStep(50)
     curPB~SetPos(10)
  end

  /* Initialize progress bar ID_B */
  curPB = self~GetProgressBar("ID_B")
  if curPB \= .Nil then do
     curPB~SetRange(0,100)
     curPB~SetStep(40)
     curPB~SetPos(20)
  end

  /* Initialize progress bar ID_C */
  curPB = self~GetProgressBar("ID_C")
  if curPB \= .Nil then do
     curPB~SetRange(0,100)
     curPB~SetStep(30)
     curPB~SetPos(30)
  end

  /* Initialize progress bar ID_D */
  curPB = self~GetProgressBar("ID_D")
  if curPB \= .Nil then do
     curPB~SetRange(0,100)
     curPB~SetStep(20)
     curPB~SetPos(40)
  end

  /* Initialize progress bar ID_E */
  curPB = self~GetProgressBar("ID_E")
  if curPB \= .Nil then do
     curPB~SetRange(0,100)
     curPB~SetStep(10)
     curPB~SetPos(50)
  end

  /* Initialization Code (e.g. fill list and combo boxes) */
  return InitDlgRet


::method DefineDialog
  result = self~DefineDialog:super
  if result = 0 then do
     /* Additional dialog items (e.g. AddInputGroup) */
  end


/* --------------------- message handler -----------------------------------*/

  /* Method Ok is connected to item 1 */
::method Ok
  /* step each progress bar by the step set with the SetStep method */
  curPB = self~GetProgressBar("ID_A")
  curPB~Step
  curPB = self~GetProgressBar("ID_B")
  curPB~Step
  curPB = self~GetProgressBar("ID_C")
  curPB~Step
  curPB = self~GetProgressBar("ID_D")
  curPB~Step
  curPB = self~GetProgressBar("ID_E")
  curPB~Step
  return 0

  /* Method Cancel is connected to item 2 */
::method Cancel
  resCancel = self~Cancel:super  /* make sure self~InitCode is set to 2 */
  self~Finished = resCancel      /* 1 means close dialog, 0 means keep open */
  return resCancel

  /* Method Help is connected to item 9 */
::method Help
  self~Help:super
