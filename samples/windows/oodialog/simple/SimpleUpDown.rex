/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2009-2010 Rexx Language Association. All rights reserved.    */
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
 * A simple UserDialog with an UpDown control.
 */

  dlg = .SimpleDialog~new

  dlg~Execute("SHOWTOP")
  dlg~deinstall

return 0

::requires "ooDialog.cls"

::class 'SimpleDialog' subclass UserDialog

::method init
  forward class (super) continue

  self~addSymbolicIDs

  self~create(30, 30, 147, 69, "Simple UpDown Control", , "CENTER")


::method defineDialog

  self~createStatic(IDC_STATIC, 20, 21, 40, 12, "TEXT RIGHT", "Spin Me:")
  self~createEdit(IDC_EDIT_BUDDY, 62, 20, 65, 12, "RIGHT NUMBER")
  self~createUpDown(IDC_UPD, 81, 26, 12, 16, "WRAP ARROWKEYS AUTOBUDDY SETBUDDYINT")
  self~addButton(IDOK, 22, 45, 50, 14, "Ok", , "DEFAULT")
  self~addButton(IDCANCEL, 77, 45, 50, 14, "Cancel")

::method initDialog

  upDown = self~newUpDown(IDC_UPD)
  upDown~setRange(1, 20000)
  upDown~setPosition(1000)


::method addSymbolicIDs private

  self~constDir[IDC_UPD]        = 200
  self~constDir[IDC_EDIT_BUDDY] = 205


::method initAutoDetection
  self~noAutoDetection
