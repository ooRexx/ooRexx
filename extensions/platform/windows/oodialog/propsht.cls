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
/* Property Sheet Dialog Class                                                */
/*                                                                            */
/******************************************************************************/
::requires "CatDlg.cls"
::requires "AdvCtrl.cls"
::requires "MsgExt.cls"

/* This class creates and controls a dialog that has more than one panel */
/* using a tab control to select the single pages */

::class 'PropertySheet' subclass CategoryDialog public inherit MessageExtensions AdvancedControls

::method Init
   argarr = Arg(1, "A")
   newarg = .array~new(argarr~items+1)
   if argarr~hasindex(1) = 1 then newarg[1] = argarr[1] /* dlgData */
   if argarr~hasindex(2) = 1 then newarg[6] = argarr[2] /* CatLabels */
   if argarr~hasindex(3) = 1 then newarg[2] = argarr[3] /* tabx */
   if argarr~hasindex(4) = 1 then newarg[3] = argarr[4] /* taby */
   if argarr~hasindex(5) = 1 then newarg[5] = argarr[5] /* options */
   if argarr~hasindex(6) = 1 then newarg[9] = argarr[6] /* include file */
   newarg[4] = 0
   forward class (super) arguments (newarg)


::method DefineDialog protected
   if self~baseptr = 0 then return 0
   self~ConnectTabNotify(9001, "SELCHANGE", "CHANGEPAGE")

   do i = 1 to self~catalog['count']
      catnames.i = self~catalog['names'][i]
   end

   self~catalog['category'] = 0

   size = self~getTextSizeDlg("Hg")

   self~catalog['page']['x'] = self~catx
   self~catalog['page']['y'] = self~caty
   self~catalog['page']['w'] = self~SizeX - self~catx*2
   self~catalog['page']['h'] = self~SizeY - self~catalog['page']['y'] - 27

   self~CategoryPage

   p = self~catalog['page']
   self~AddTabControl(9001,,p['x'], p['y'], p['w'], p['h'],self~CatStyle || " CAT CLIPSIBLINGS")

   self~DlgHandle = UsrCreateDialog(self~Adm, "PARENT", self~DialogItemCount, 0, self~BasePtr, self~AutoDetect, 1, 0, 0)
   self~activePtr = 0
   if self~DlgHandle = 0 then return 0

   do i = 1 to self~catalog['count']
      self~DialogItemCount = 0
      self~catalog['category'] = i
      self~CreateCategoryDialog(p['x']+2, p['y']+(size~height*2), p['w']-10, p['h']-(size~height*3),p['font'], p['fsize'], p['style'] || " CHILD HIDDEN", p['expected'])
      msg = "self~" || self~catalog['names'][i]~space(0)
      interpret(msg)
      ret = UsrCreateDialog(self~Adm, "CHILD", self~DialogItemCount, self~catalog['base'][i], self~DlgHandle, i)
      self~activePtr = 0
      self~catalog['handles'][i] = ret
   end

   /* This method adds controls to the base window of a 'CategoryDialog' */

::method CategoryPage protected
   p = self~catalog['page']
   lbtext = p['leftbtntext']
   rbtext = p['rightbtntext']
   if lbtext~pos(" ") > 0 then lbtext = '"' || lbtext || '"'
   if rbtext~pos(" ") > 0 then rbtext = '"' || rbtext || '"'
   if (p['btnwidth'] > 0) then bw = p['btnwidth']; else bw = 35
   self~AddButtonGroup(p['w']+p['x']+2-bw*2.25,,
      self~SizeY - 15,bw,, "&Ok 1 OK &Cancel 2 CANCEL", 1, "FIX DEFAULT")
   if self~Wizard = 1 then
      self~AddButtonGroup(p['x']-2,self~SizeY - 15,bw,,
                          ,lbtext" 11 PreviousPage" rbtext" 12 NextPage", 1, "FIX")


::method InitDialog protected
   /* this is to correct a redraw problem of a focused list view control that gets reactivated */
   self~AddUserMsg(PropSheetActivated, "0x00000006", "0xFFFFFFFF")  /* WM_ACTIVATE */

   tc = self~GetTabControl(9001, 0)
   if tc \= .Nil then do
      do i = 1 to self~catalog['count']
         tc~Insert(,self~catalog['names'][i])
      end
      tc~SelectIndex(0)
   end
   if self~Wizard = 1 then self~DisableItem(11)

   /* call InitXXX for each category if such a method exists */
   do i = 1 to self~catalog['count']
      self~catalog['category'] = i
      msg = "Init" || self~catalog['names'][i]~space(0)
      if self~HasMethod(msg) = 1 then interpret("self~"||msg)   /* does user class provide the Init method? If so call it */
   end

   /* set the mark to the first radio button */
   self~catalog['category'] = 1
   self~ShowWindow(self~catalog['handles'][self~catalog['category']])


   /* this is to correct a redraw problem of a focused list view control. If the list control has the focus,
   was overlapped by another window and is activated, the list is overlapped by the tab control */
::method PropSheetActivated
   use arg wP, lp
   if Arg() = 0 then code = 0
   else code = .DlgUtil~loWord(wp)
   if code \= 0 then do
       self~Update
       wnd = .DialogControl~new
       wnd~AssignWindow(self~catalog['handles'][self~catalog['category']], self)
       wnd~Update
   end

::method GetSelectedPage unguarded
   return self~GetTabControl(9001, 0)~SelectedIndex+1


::method ChangePage unguarded
   use arg NewPage

   oldPage = self~catalog['category']
   /* if ChangePage is called automatically then 2 arguments are passed (WParam and LParam) */
   if Arg(2,'o') = 0 | NewPage > 9000 then NewPage = self~GetSelectedPage
   else do
      self~GetTabControl(9001, 0)~SelectIndex(NewPage-1)
   end
   if (NewPage = self~catalog['category']) |,
      (NewPage < 1 | NewPage> self~catalog['count']) then return 0
   self~HideWindowFast(self~catalog['handles'][self~catalog['category']])
   self~catalog['category'] = NewPage
   if self~Wizard = 1 then
   do
      if NewPage = 1 then
      do
         self~DisableItem(11)
         self~EnableItem(12)
      end; else
      if NewPage = self~catalog['count'] then
      do
         self~EnableItem(11)
         self~DisableItem(12)
      end; else
      do
         self~EnableItem(11)
         self~EnableItem(12)
      end
   end
   self~ShowWindow(self~catalog['handles'][self~catalog['category']])

   self~PageHasChanged(oldPage, NewPage)
   return NewPage

