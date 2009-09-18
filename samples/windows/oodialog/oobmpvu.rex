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
 d~CreateCenter(300, 200, "Bitmap Viewer")
 d~execute("SHOWTOP")
 d~deinstall
 ret = directory(curdir)
 return

/*-------------------------------- requires --------------------------*/

::requires "oodialog.cls"

/*-------------------------------- dialog class ----------------------*/

::class BmpDialog subclass UserDialog

::method DefineDialog
   ret = directory("bmp")
   self~AddText(10,10,,, "&Filename: ")
   self~AddComboBox(101,"Filename",60,10,130,80,"VSCROLL")
   self~ConnectList(101,"FileSelected")
   self~createBitmapButton(102, 13, 33, self~SizeX - 26, self~SizeY - 30 - 36, , , , "blank.bmp")
   self~AddButtonGroup(100, self~sizeY - 18,,, "&Show 1 OK &Cancel 2 CANCEL", 1)
   self~AddBlackFrame(10,30,self~SizeX - 20, self~SizeY - 30 - 30)

::method InitDialog
   self~AddComboEntry(101, "...")
   self~ComboAddDirectory(101, "*.bmp", "READWRITE")
   self~ComboAddDirectory(101, "*.dib", "READWRITE")

::method FileSelected                        /* drop-down selection */
   self~getData
   if self~Filename = "..." then self~OK
   else self~ShowBitmap

::method OK                                  /* show button */
   self~GetData
   if self~Filename = "..." then
   do
      self~Filename = fileNameDialog("*.*", self~DlgHandle)
      if self~Filename \= "0" then
      do
         self~ComboDrop(101)
         self~AddComboEntry(101, "...")
         self~ComboAddDirectory(101, filespec("drive", self~Filename) || filespec("path", self~Filename) || "*.bmp", "READWRITE")
         self~ComboAddDirectory(101, filespec("drive", self~Filename) || filespec("path", self~Filename) || "*.dib", "READWRITE")
         self~setAttrib("Filename")
      end
   end
   self~ShowBitmap
   return 0

::method ShowBitmap                          /* draw the bitmap */
   self~ChangeBitmapButton(102,0)
   self~ChangeBitmapButton(102,self~Filename,,,,"USEPAL")
