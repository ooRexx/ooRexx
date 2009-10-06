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
/* Name: DRAWING1.REX                                                       */
/* Type: Object REXX Script                                                 */
/*                                                                          */
/* Description: Sample to demonstrate drawing functionality                 */
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
   if dlg~IsDialogActive then dlg~StopIt


::requires "ooDialog.cls"

::class 'MyDialogClass' subclass UserDialog

::method Init
    ret = self~init:super;
    if ret = 0 then ret = self~Load("Drawings.RC", 100)
    self~connectButtonEvent(11, "CLICKED", "Circle")
    self~connectButtonEvent(12, "CLICKED", "MyRectangle")
    self~InitCode = ret
    return ret


::method Circle
    dc = self~GetButtonDC(10)
    pen = self~CreatePen(5, "SOLID", 1)
    oldpen = self~ObjectToDc(dc, pen)
    self~DrawArc(dc, 10, 10, 320, 200)
    self~ObjectToDc(dc, oldpen)
    self~DeleteObject(pen)
    self~FreeButtonDC(10, dc)

::method MyRectangle
    dc = self~GetButtonDC(10)
    pen = self~CreatePen(10, "DOT", 2)
    oldpen = self~ObjectToDc(dc, pen)
    self~Rectangle(dc, 10, 10, 320, 200)
    self~ObjectToDc(dc, oldpen)
    self~DeleteObject(pen)
    self~FreeButtonDC(10, dc)


