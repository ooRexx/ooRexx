/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/*                                                                            */
/*       Windows Dialog Interface for Object REXX                             */
/* Resource Dialog Class                                                      */
/*                                                                            */
/******************************************************************************/
::requires "BaseDlg.cls"

::class 'ResDialog' subclass BaseDialog public

::method setFontAttrib private external "LIBRARY oodialog resdlg_setFontAttrib_pvt"

::method init
   expose library resource dlgData.
   use arg library, resource, dlgData., includeFile

   if arg(4, 'E') then ret = self~init:super(library, resource, dlgData., includeFile)
   else ret = self~init:super(library, resource, dlgData.)
   if ret \= 0 then return ret

   if arg(3,'o') then self~useStem = 0; else self~useStem = 1
   if \ resource~datatype("N") then resource = self~resolveSymbolicId(resource)
   if resource < 1 then self~initCode = -1

   return self~initCode

-- Create a Windows dialog out of the resource stored in Library.
::method startIt
   expose library resource dlgData.
   use arg icon = 0, modal = 0

   icon = self~resolveIconID(icon)

   if modal <> 0 & modal <> 1 then do
     if modal~translate = "NOTMODAL" then modal = 1
     else modal = 0
   end

   self~dlgHandle = StartDialog(self~adm, library, resource, self~autoDetect, 1, icon, modal)
   if self~dlgHandle \= 0 then do
      self~setFontAttrib
      /* if autodetection is on, create the data attributes */
      if self~autoDetect = 1 then do
         id = 1; i=0
         do while id > 0
            parse value DataTable(self~adm,"GET", i) with id typ cat
            hnd = self~getItem(id)
            if (hnd \= 0) then do
                txt = Wnd_Desktop("TXT",hnd)
                self~addAttribute(id, txt)
            end
            i = i +1
         end
      end
      self~initDialog
   end

   return self~dlgHandle


::method stopIt
   forward class (super) continue
   ret = result
   if self~bkgBrushBmp \= 0 then self~deleteObject(self~bkgBrushBmp)
   if self~bkgBitmap \= 0 then self~removeBitmap(self~bkgBitmap)
   return ret

-- This method is deprecated, instead use the Menu classes directly.
::method setMenu
  use strict arg id
  menuBar = .BinaryMenuBar~new(self, id, self, 0, .true)
  if menuBar~initCode == 0 then self~menuBar = menuBar
  return menuBar~initCode

