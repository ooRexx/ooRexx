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
/* OODialog\Samples\oobmpvu.rex   BMP Display - GraphicDemo                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/


 curdir = directory()
 parse source . . me
 mydir = me~left(me~lastpos('\')-1)              /* where is code     */
 mydir = directory(mydir)                        /* current is "my"   */
 b.101 = ''
 d = .BmpDialog~new(b.)
 d~createCenter(300, 200, "Bitmap Viewer")
 d~execute("SHOWTOP")
 d~deinstall
 ret = directory(curdir)
 return

/*-------------------------------- requires --------------------------*/

::requires "ooDialog.cls"

/*-------------------------------- dialog class ----------------------*/

::class 'BmpDialog' subclass UserDialog

::method defineDialog
   ret = directory("bmp")
   self~createStaticText(-1, 10, 10, , , , "&Filename: ")
   self~createComboBox(101, 60, 10, 130, 80, "VSCROLL", "Filename")
   self~connectListBoxEvent(101, "SELCHANGE", "FileSelected")
   self~createBitmapButton(102, 13, 33, self~SizeX - 26, self~SizeY - 30 - 36, , , , "blank.bmp")
   self~createPushButtonGroup(100, self~sizeY - 18,,, "&Show 1 OK &Cancel 2 CANCEL", 1)
   self~createBlackFrame(-1, 10, 30, self~SizeX - 20, self~SizeY - 30 - 30)

::method initDialog
   self~addComboEntry(101, "...")
   self~comboAddDirectory(101, "*.bmp", "READWRITE")
   self~comboAddDirectory(101, "*.dib", "READWRITE")

::method fileSelected                        /* drop-down selection */
   self~getData
   if self~filename = "..." then self~OK
   else self~showBitmap

::method OK                                  /* show button */
   self~getData
   if self~filename = "..." then
   do
      self~filename = fileNameDialog("*.*", self~DlgHandle)
      if self~filename \= "0" then
      do
         self~comboDrop(101)
         self~addComboEntry(101, "...")
         self~comboAddDirectory(101, filespec("drive", self~Filename) || filespec("path", self~Filename) || "*.bmp", "READWRITE")
         self~comboAddDirectory(101, filespec("drive", self~Filename) || filespec("path", self~Filename) || "*.dib", "READWRITE")
         self~setControlDataAttribute("Filename")
      end
   end
   self~showBitmap
   return 0

::method showBitmap                          /* draw the bitmap */
   self~changeBitmapButton(102,0)
   self~changeBitmapButton(102,self~filename,,,,"USEPAL")
