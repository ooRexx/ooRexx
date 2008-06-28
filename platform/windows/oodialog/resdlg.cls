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
/******************************************************************************/
/*                                                                            */
/*       Windows Dialog Interface for Object REXX                             */
/* Resource Dialog Class                                                      */
/*                                                                            */
/******************************************************************************/
::requires "BaseDlg.cls"

::class 'ResDialog' subclass BaseDialog public

::method Init
   expose Library Resource DlgData.
   use arg Library, Resource, DlgData., includeFile

   if arg(4, 'E') then ret = self~init:super(Library, Resource, DlgData., includeFile)
   else ret = self~init:super(Library, Resource, DlgData.)
   if ret \= 0 then return ret

   if Arg(3,'o') = 1 then self~UseStem = 0; else self~UseStem = 1
   if \ Resource~datatype("N") then Resource = self~ResolveSymbolicId(Resource)
   if Resource < 1 then self~InitCode = -1

   return self~InitCode

   /* create a Windows dialog out of the resource stored in Library */

::method StartIt
   expose Library Resource DlgData.
   use arg icon = 0, modal = 0

   if \ icon~DataType("W") then icon = self~ResolveSymbolicId(icon)
   if icon < 0 then icon = 0

   if modal <> 0 & modal <> 1 then do
     if modal~translate = "NOTMODAL" then modal = 1
     else modal = 0
   end

   self~DlgHandle = StartDialog(self~Adm, Library, Resource, self~AutoDetect, 1, icon, modal)
   if self~DlgHandle \= 0 then do
      /* if autodetection is on, create the data attributes */
      if self~AutoDetect = 1 then do
         id = 1; i=0
         do while id > 0
            parse value DataTable(self~Adm,"GET", i) with id typ cat
            hnd = self~GetItem(id)
            if (hnd \= 0) then do
                txt = Wnd_Desktop("TXT",hnd)
                self~AddAttribute(id, txt)
            end
            i = i +1
         end
      end
      self~InitDialog
   end

   return self~DlgHandle


::method StopIt
   forward class (super) continue
   ret = result
   if self~BkgBrushBmp \= 0 then self~DeleteObject(self~BkgBrushBmp)
   if self~BkgBitmap \= 0 then self~RemoveBitmap(self~BkgBitmap)
   return ret


::method SetMenu
   use strict arg id
   if \ id~DataType("W") then id = self~ResolveSymbolicId(id)
   if id < 1 then return -1

   return DialogMenu("ASSOC", self~DlgHandle, id)


