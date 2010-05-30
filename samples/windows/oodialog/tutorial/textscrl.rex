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
/* Name: TEXTSCRL.REX                                                       */
/* Type: Object REXX Script                                                 */
/*                                                                          */
/* Description:  Scrolling text sample.                                     */
/*                                                                          */
/****************************************************************************/

signal on any name CleanUp

dlg = .MyDialogClass~new
if dlg~InitCode <> 0 then exit
dlg~Execute("SHOWTOP")
dlg~deinstall
exit

/* ------- signal handler to destroy dialog if condition trap happens  -----*/
CleanUp:
   call errorDialog "Error" rc "occurred at line" sigl":" errortext(rc),
                     || "a"x || condition("o")~message
   if dlg~isDialogActive then do
      dlg~finished = .true
      dlg~stopIt
   end


::requires "ooDialog.cls"

::class 'MyDialogClass' subclass UserDialog

::method init
    ret = self~init:super;
    if ret = 0 then ret = self~load("Textscrl.RC", 100)
    if ret = 0 then do
        self~data13 = "Arial"
        self~text = "This is a scrolling text demonstration"
        self~data14 = 24
        self~connectButtonEvent(11, "CLICKED", "Display")
    end
    self~initCode = ret
    return ret


::method initDialog

   -- Set the background color of the button to the backgroud color of a button.
   COLOR_BTNFACE = 15
   self~setItemSysColor(10, COLOR_BTNFACE)


::method display
    self~getData
    self~scrollInButton(10, self~text, self~data13, self~data14, "BOLD")

