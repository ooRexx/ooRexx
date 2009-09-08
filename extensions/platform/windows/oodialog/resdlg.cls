/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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


::method init
   expose library resource dlgData.
   use arg library, resource, dlgData., includeFile

   if arg(4, 'E') then ret = self~init:super(library, resource, dlgData., includeFile)
   else ret = self~init:super(library, resource, dlgData.)
   if ret \= 0 then return ret

   if arg(3,'o') then self~useStem = .false; else self~useStem = .true
   if \ resource~datatype("N") then resource = self~resolveSymbolicId(resource)
   if resource < 1 then self~initCode = -1

   return self~initCode

-- Create a Windows dialog out of the resource stored in Library.
--
-- From docs for the arg: modal
-- If a modal or a modeless dialog is to be created. A value of 0 signals a modal dialog should be
-- created. A value of 1, or the string "NOTMODAL" case insignificant, signals a modeless dialog
-- should be created. The default is 0.
--
-- So the arg 'modal' is misnamed, it should be modeless so that 0 (false) signals a modal dialog
-- and 1 (true) signals a modeless dialog.  Changing this, 9/7/2009
::method startIt
   expose library resource dlgData.
   use arg icon = 0, modeless = 0

   icon = self~resolveIconID(icon)

   if modeless <> 0 & modeless <> 1 then do
     if modeless~translate = "NOTMODAL" then modeless = 1
     else modeless = 0
   end

   if self~startDialog(library, resource, self~autoDetect, icon, modeless) then do
      -- if autodetection is on, create the data attributes
      if self~autoDetect then do
         ids = self~getDataTableIDs
         do id over ids
            hnd = self~getItem(id)
            if (hnd \= 0) then do
                txt = Wnd_Desktop("TXT",hnd)
                self~addAttribute(id, txt)
            end
         end
      end
      self~initDialog
      return .true
   end

   return .false

::method startDialog private external "LIBRARY oodialog resdlg_startDialog_pvt"
::method getDataTableIDs private external "LIBRARY oodialog resdlg_getDataTableIDs_pvt"

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
  if menuBar == 0 then return .false
  self~menuBar = menuBar
  return .true

