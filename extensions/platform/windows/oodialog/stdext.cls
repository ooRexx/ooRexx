/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/*                                                                            */
/*       Windows Dialog Interface for Object REXX                             */
/* Standard Dialog Classes                                                    */
/*                                                                            */
/******************************************************************************/
::requires "USERDLG.CLS"
::requires "ADVCTRL.CLS"

::class 'StateIndicator' subclass UserDialog public

::method Xext attribute
::method TitleTxt attribute
::method Msg attribute
::method Dims attribute

   /* The class' constructor */
::method init
   use arg message, title, fixedx
   if arg(3,'o') = 1 then self~XExt = 0; else self~XExt = fixedx
   self~Msg = message
   self~TitleTxt = title
   self~dims = .directory~new
   self~init:super("")
   if self~initCode = 0 then self~Popup

::method DefineDialog
   self~addText(10,10,self~dims["sx"]+2,self~dims["sy"], self~msg,, 100)

::method InitAutoDetection
   self~NoAutoDetection

::method CalcSize
   size = self~getTextSizeDlg(self~msg)
   scrSize = screenSize()
   self~dims["sx"] = size~width
   self~dims["sy"] = size~height
   if self~XExt = 0 then do
       self~dims["cx"] = self~dims["sx"]+20
       self~dims["cy"]= self~dims["sy"]*2.2+20
   end
   else do
       self~dims["cx"] = self~XExt
       lines = 1
       if self~dims["sx"] + 20 > self~dims["cx"] then do
           lines = Trunc(self~dims["sx"] / (self~dims["cx"] -20) + 0.98)
           self~dims["sx"] = self~dims["cx"] - 20
           self~dims["sy"] = Trunc(self~dims["sy"]*lines*1.1)
       end
       if lines >1 then self~dims["cy"] = self~dims["sy"]+20
       else self~dims["cy"] = self~dims["sy"]*2.2+20
   end
   self~dims["x"] = trunc((scrSize[1]-self~dims["cx"]) /2)
   self~dims["y"] = trunc((scrSize[2] - self~dims["cy"] - 20) /2)

::method Popup
   self~CalcSize
   self~create(self~dims["x"],self~dims["y"], self~dims["cx"], self~dims["cy"], self~TitleTxt,"NOMENU",,,1 )
   self~startit
   self~show("SHOWTOP")

::method Destroy
   self~stopit
   self~Deinstall

::method "Message="
   parse arg message
   self~msg = message
   self~CalcSize
   self~ResizeItem(100, self~dims["sx"]+2, self~dims["sy"])
   if self~XExt = 0 then do
       self~Resize(self~dims["cx"], self~dims["cy"], "HIDE")
   end
   else do
       parse value self~GetWindowRect(self~dlgHandle) with l t r b
       if self~dims["cy"] > (b-t) / self~factorY then self~Resize(self~dims["cx"], self~dims["cy"], "NORMAL")
   end
   self~SetStaticText(100, message)
   if self~XExt = 0 then self~Move(self~dims["x"], self~dims["y"], "NORMAL")




::class 'ProgressIndicator' subclass StateIndicator public inherit AdvancedControls

::method LowValue attribute
::method HighValue attribute
::method IncStep attribute

   /* The class' constructor */
::method init
   use arg message, title, low, high, step, fixedx
   if arg(6,'o') = 1 then fixedx = 0
   if arg(3,'o') = 1 then self~LowValue = 0; else self~LowValue = low
   if arg(4,'o') = 1 then self~HighValue = 100; else self~HighValue = high
   if arg(5,'o') = 1 then self~IncStep = 10; else self~IncStep = step
   self~init:super(message, title, fixedx)

::method initDialog
   pb = self~GetProgressBar(101)
   if pb \= .nil then do
       pb~SetRange(self~LowValue, self~HighValue)
       pb~SetPos(self~LowValue)
       pb~SetStep(self~IncStep)
   end

::method DefineDialog
   self~AddProgressBar(101,5,5, self~sizex-10,14)
   self~addText(10,25,self~dims["sx"]+2,self~dims["sy"], self~msg,, 100)


::method CalcSize
   self~CalcSize:super
   self~dims["cy"] = self~dims["cy"]+10  /* 15 more for the progress bar */
   scrSize = screenSize()
   self~dims["x"] = trunc((scrSize[1]-self~dims["cx"]) /2)
   self~dims["y"] = trunc((scrSize[2] - self~dims["cy"] - 20) /2)

::method "Progress="
   use arg newpos
   self~GetProgressBar(101)~SetPos(newpos)

::method Increase
   if arg(1,'e') = 1 then self~GetProgressBar(101)~Step(arg(1))
   else self~GetProgressBar(101)~Step

::method Complete
   self~GetProgressBar(101)~SetPos(self~HighValue)

::method "Message="
   forward class (super) continue
   self~ResizeItem(101,  self~sizex-10, 14)
