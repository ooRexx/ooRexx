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

/**
 * Windows Dialog Interface for Open Object Rexx (ooRexx.)
 *
 * Property Sheet Dialog Class.
 */
::requires "CatDlg.cls"
::requires "AdvCtrl.cls"

/* This class creates and controls a dialog that has more than one panel */
/* using a tab control to select the single pages */

::class 'PropertySheet' subclass CategoryDialog public

::method init
   argarr = arg(1, "A")
   newarg = .array~new(argarr~items+1)
   if argarr~hasindex(1) = 1 then newarg[1] = argarr[1] /* dlgData */
   if argarr~hasindex(2) = 1 then newarg[6] = argarr[2] /* CatLabels */
   if argarr~hasindex(3) = 1 then newarg[2] = argarr[3] /* tabx */
   if argarr~hasindex(4) = 1 then newarg[3] = argarr[4] /* taby */
   if argarr~hasindex(5) = 1 then newarg[5] = argarr[5] /* options */
   if argarr~hasindex(6) = 1 then newarg[9] = argarr[6] /* include file */
   newarg[4] = 0
   forward class (super) arguments (newarg)


::method defineDialog protected
   if self~basePtr = 0 then return 0
   self~connectTabEvent(9001, "SELCHANGE", "CHANGEPAGE")

   do i = 1 to self~catalog['count']
      catnames.i = self~catalog['names'][i]
   end

   self~catalog['category'] = 0

   size = self~getTextSizeDlg("Hg")

   self~catalog['page']['x'] = self~catx
   self~catalog['page']['y'] = self~caty
   self~catalog['page']['w'] = self~sizeX - self~catx*2
   self~catalog['page']['h'] = self~sizeY - self~catalog['page']['y'] - 27

   self~categoryPage

   p = self~catalog['page']
   self~createTab(9001, p['x'], p['y'], p['w'], p['h'], self~catStyle || " CAT CLIPSIBLINGS")

   if \ self~startParentDialog(0, .false) then return 0

   do i = 1 to self~catalog['count']
      self~catalog['category'] = i

      -- Start the dialog template for this page.  (Ignore an error, an exception will be raised)
      self~createCategoryDialog(p['x']+2, p['y']+(size~height*2), p['w']-10, p['h']-(size~height*3),p['font'], p['fsize'], p['expected'])

      -- Invoke the user defined method with the same name as the page name.  This is where the
      -- dialog control items for this page are defined.
      self~send(self~catalog['names'][i]~space(0))

      -- Create the underlying Windows dialog for this page.
      hChildDlg = self~startChildDialog(self~catalog['base'][i], i)
      self~catalog['base'][i] = 0
      self~catalog['handles'][i] = hChildDlg
   end

   /* This method adds controls to the base window of a 'CategoryDialog' */

::method categoryPage protected
   p = self~catalog['page']
   lbtext = p['leftbtntext']
   rbtext = p['rightbtntext']
   if lbtext~pos(" ") > 0 then lbtext = '"' || lbtext || '"'
   if rbtext~pos(" ") > 0 then rbtext = '"' || rbtext || '"'
   if (p['btnwidth'] > 0) then bw = p['btnwidth']; else bw = 35

   self~createPushButtonGroup(p['w']+p['x']+2-bw*2.25, self~sizeY - 15, bw, , "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")
   if self~wizard then do
      self~createPushButtonGroup(p['x']-2,self~sizeY - 15, bw, , lbtext" 11 PreviousPage" rbtext" 12 NextPage", 1, "")
   end


::method initDialog protected
   /* this is to correct a redraw problem of a focused list view control that gets reactivated */
   self~addUserMsg(PropSheetActivated, "0x00000006", "0xFFFFFFFF")  /* WM_ACTIVATE */

   tc = self~newTab(9001, 0)
   if tc \= .nil then do
      do i = 1 to self~catalog['count']
         tc~insert(,self~catalog['names'][i])
      end
      tc~selectIndex(0)
   end
   if self~wizard then self~disableItem(11)

   -- call initXXXX for each category, if such a method exists
   do i = 1 to self~catalog['count']
      self~catalog['category'] = i
      initMethod = "init" || self~catalog['names'][i]~space(0)
      if self~hasMethod(initMethod) then self~send(initMethod)
   end

   /* set the mark to the first radio button */
   self~catalog['category'] = 1
   self~showWindow(self~catalog['handles'][self~catalog['category']])


   /* this is to correct a redraw problem of a focused list view control. If the list control has the focus,
   was overlapped by another window and is activated, the list is overlapped by the tab control */
::method propSheetActivated
   use arg wP, lp
   if arg() = 0 then code = 0
   else code = .DlgUtil~loWord(wp)
   if code \= 0 then do
       self~update
       .Window~new(self~catalog['handles'][self~catalog['category']])~update
   end

::method getSelectedPage unguarded
   return self~newTab(9001, 0)~selectedIndex+1


::method changePage unguarded
   use arg newPage

   oldPage = self~catalog['category']
   /* if ChangePage is called automatically then 2 arguments are passed (WParam and LParam) */
   if arg(2,'o') = 0 | newPage > 9000 then newPage = self~getSelectedPage
   else do
      self~newTab(9001, 0)~selectIndex(newPage-1)
   end
   if (newPage = self~catalog['category']) |,
      (newPage < 1 | newPage> self~catalog['count']) then return 0
   self~hideWindowFast(self~catalog['handles'][self~catalog['category']])
   self~catalog['category'] = newPage
   if self~wizard then do
      if newPage = 1 then do
         self~disableItem(11)
         self~enableItem(12)
      end
      else if newPage = self~catalog['count'] then do
         self~enableItem(11)
         self~disableItem(12)
      end
      else do
         self~enableItem(11)
         self~enableItem(12)
      end
   end
   self~showWindow(self~catalog['handles'][self~catalog['category']])

   self~pageHasChanged(oldPage, newPage)
   return newPage

