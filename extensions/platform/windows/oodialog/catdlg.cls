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
 * The Category Dialog Class.
 */
::requires "UserDlg.cls"

/* This class creates and controls a dialog that has more than one panel. */
/* It is similar to the 'notebook' control available in OS/2 or the PropertySheet */
/* available in the Windows 95 user interface. You can switch among */
/* diferent pages by clicking radio buttons. Each page has its own */
/* window controls. You can setup a page out of a dialog resource script. */

::class 'CategoryDialog' subclass UserDialog public

   /* arg style=[ "DROPDOWN" | "TOPLINE" | "NOBORDER" | "WIZARD"] */

::method Catalog attribute protected   /* A directory that contains internal data */
::method StaticID attribute protected  /*  */
::method CatX attribute private
::method CatY attribute private
::method CatCX attribute private
::method CatStyle attribute private
::method CatLabel attribute private
::method CatMax attribute private
::method Wizard attribute private

::method "!MarkAsCatDlg!"   /* this is to mark the class as a category dialog */

   /* arg style=[ "DROPDOWN" | "TOPLINE" | "NOBORDER" | "WIZARD"] */

::method Init
   use arg dlgData., cdx, cdy, cdcx, cdstyle, cdtable, cdlabel, cdmax, includeFile

   newArgs = .array~new(2)
   if arg(1, 'E') then newArgs[1] = arg(1)
   if arg(9, 'E') then newArgs[2] = arg(9)
   forward class (super) arguments (newArgs) continue
   if result <> 0 then return result

   self~catalog = .directory~new
   self~catalog['count'] = 0
   self~catalog['page'] = .directory~new
   self~catalog['page']['font'] = ""
   self~catalog['page']['fsize'] = 0
   self~catalog['page']['style'] = ""
   self~catalog['page']['btnwidth'] = 0
   self~catalog['page']['leftbtntext'] = "&Backward"
   self~catalog['page']['rightbtntext'] = "&Forward"
   self~catalog['page']['expected'] = 200

   if arg(1,'o') = 1 then self~useStem = .false; else self~useStem = .true
   if arg(2,'o') = 1 then self~catx = 10; else self~catX = cdx
   if arg(3,'o') = 1 then self~caty = 4; else self~caty = cdy
   if arg(4,'o') = 1 then self~catcx = 0; else self~catcx = cdcx
   if arg(5,'o') = 1 then self~catstyle = ""; else self~catstyle = cdstyle~translate
   if arg(6,'o') = 1 then cdtable = ""
   if arg(7,"o") = 1 then self~catlabel = "Page:"; else self~catlabel = cdlabel
   if arg(8,"o") = 1 then self~catmax = 0; else self~catmax = cdmax

   if cdtable \= '' then
   do
      self~catalog['names'] = .array~new(10)
      parse var cdtable name ret
      i = 0
      do while name \= ""
         i = i + 1
         self~catalog['names'][i] = name
         parse var ret name ret
      end
      self~catalog['count'] = i
   end

   self~InitCategories()    /* user overwrite */

   if cdtable = '' then
   do
      if self~catalog['count'] = 0 then
      do
         i = 0
         do ar over self~catalog['names']
            i = i +1
         end
         self~catalog['count'] = i
      end
   end

   if self~catstyle~wordpos("WIZZARD") > 0 then
   do
      self~Wizard = 1
      self~catstyle = self~catstyle~changestr("WIZZARD","")
   end
   else if self~catstyle~wordpos("WIZARD") > 0 then
   do
      self~Wizard = 1
      self~catstyle = self~catstyle~changestr("WIZARD","")
   end
   else self~Wizard = 0

   self~catalog['id'] = .array~new(self~catalog['count'],100)
   self~catalog['base'] = .array~new(self~catalog['count'])
   self~catalog['handles'] = .array~new(self~catalog['count'])
   self~staticID = 9600

   return 0


::method startIt   /* don't call parent startIt */
   if self~dlgHandle = 0 then return 0
   self~initDialog
   return self~dlgHandle

::method defineDialog protected
   if self~basePtr = 0 then return 0
   if self~catstyle~wordpos('DROPDOWN') = 0 then
      do i = 9001 to (9000 + self~catalog['count'])
         self~connectButtonEvent(i, "CLICKED", "CHANGEPAGE")
      end
   else self~connectListBoxEvent(9001, "SELCHANGE", "CHANGEPAGE")

   do i = 1 to self~catalog['count']
      catnames.i = self~catalog['names'][i]
   end

   self~catalog['category'] = 0
   n = self~catstyle~wordpos("TOPLINE")
   if n > 0 then self~catmax = 1
   m = self~catstyle~wordpos("DROPDOWN")

   if (n > 0 | m > 0) & self~catstyle~wordpos('NOBORDER') = 0 then self~catstyle = self~catstyle||" NOBORDER"

   if self~catalog['page']['h'] = .nil then self~catalog['page']['h'] = self~sizeY - 40

   size = self~getTextSizeDlg("Hg")
   textHeight = size~height

   if self~catcx = 0 & n = 0 then  /* dropdown or none */
   do
      do i = 1 to self~catalog['count']
         if self~catalog['page']['font'] <> "" then
              size = self~getTextSizeDlg(catnames.i,self~catalog['page']['font'],self~catalog['page']['fsize'])
         else
              size = self~getTextSizeDlg(catnames.i)
         self~catcx = max(self~catcx,size~width)
      end
      if n = 0 then self~catcx = self~catcx + 20
   end

   if n > 0 | m > 0 then do  /* topline or dropdown */
      if self~catalog['page']['x'] = .nil then self~catalog['page']['x'] = 1
      if m > 0 then do
        if self~catalog['page']['y'] = .nil then self~catalog['page']['y'] = self~caty*2+textHeight+4
      end
      else if self~catalog['page']['y'] = .nil then self~catalog['page']['y'] = self~caty+textHeight+5

      if self~catalog['page']['w'] = .nil then self~catalog['page']['w'] = self~sizeX - 2
   end
   else do
      if self~catmax = 0 then do
        if self~catalog['page']['x'] = .nil then self~catalog['page']['x'] = self~catX+10+self~catcx
      end
      else
        if self~catalog['page']['x'] = .nil then self~catalog['page']['x'] = self~catX+10+self~catcx * ((self~catalog['count']-1)%self~catmax+1)

      if self~catalog['page']['y'] = .nil then self~catalog['page']['y'] = 5

      if self~catalog['page']['w'] = .nil then self~catalog['page']['w'] = self~sizeX - (self~catX+self~catcx+20)
   end

   self~CategoryPage

   if m > 0 then do
      if self~catmax = 0 then self~catmax = 5
      if self~catalog['count'] < self~catmax then self~catmax = self~catalog['count']
      if self~catmax < self~catalog['count'] then
           if self~catlabel='' then
                self~createComboBox(9001, self~catX, self~caty, self~catcx, (self~catmax+1)*(textHeight+4), -
                                 self~catstyle~delword(m, 1) || " CAT VSCROLL")
           else self~createComboBoxInput(9001, self~catX, self~caty, , self~catcx, self~catmax,             -
                                         self~catlabel, self~catstyle~delword(m, 1) || " CAT VSCROLL")
      else if self~catlabel='' then
                self~createComboBox(9001, self~catX,self~caty,self~catcx,(self~catmax+1)*(textHeight+4),    -
                                    self~catstyle~delword(m, 1) || " CAT")
           else self~createComboBoxInput(9001, self~catX, self~caty, , self~catcx, self~catmax,             -
                                         self~catlabel, self~catstyle~delword(m, 1) || " CAT")
   end
   else do
      if (n >0) then newstyle = self~catstyle~delword(n, 1)
                else newstyle = self~catstyle
      if self~catalog['page']['font'] <> "" then
         self~createRadioButtonStem(9001, self~catX, self~caty, self~catcx, catnames., self~catmax,,
                           newstyle || " CAT",,self~catalog['page']['font'],self~catalog['page']['fsize'])
      else
         self~createRadioButtonStem(9001, self~catX, self~caty, self~catcx, catnames., self~catmax, newstyle || " CAT")
   end

   if \ self~startParentDialog(0, .false) then return 0

   do i = 1 to self~catalog['count']
      self~catalog['category'] = i
      p = self~catalog['page']

      -- Start the dialog template for this page.  (Ignore an error, an exception will be raised)
      self~CreateCategoryDialog(p['x'], p['y'], p['w'], p['h'],p['font'], p['fsize'], p['expected'])

      -- Invoke the user defined method with the same name as the page name.  This is where the
      -- dialog control items for this page are defined.
      self~send(self~catalog['names'][i]~space(0))

      -- Create the underlying Windows dialog.
      hChildDlg = self~startChildDialog(self~catalog['base'][i], i)
      self~catalog['base'][i] = 0
      self~catalog['handles'][i] = hChildDlg
   end

-- This method returns the current page index
::method CurrentCategory unguarded
   return self~catalog['category']

-- This method adds controls to CategoryDialog dialog itself.  The dialog that is the parent to
-- the child dialogs that make up the pages of the CategoryDialog.
::method CategoryPage protected

   size = self~getTextSizeDlg("Hg")
   textHeight = size~height

   if self~catstyle~wordpos("TOPLINE") > 0 then
      self~createBlackFrame(-1, 0, 2, self~sizeX, textHeight + (self~caty+2))
   else if self~catstyle~wordpos("DROPDOWN") > 0 then
      self~createBlackFrame(-1, 0, 2, self~sizeX, textHeight + (self~caty*2)+1)

   lbtext = self~catalog['page']['leftbtntext']
   rbtext = self~catalog['page']['rightbtntext']
   if lbtext~pos(" ") > 0 then lbtext = '"' || lbtext || '"'
   if rbtext~pos(" ") > 0 then rbtext = '"' || rbtext || '"'
   if (self~catalog['page']['btnwidth'] > 0) then
   do
      self~createPushButtonGroup(self~sizeX-self~catalog['page']['btnwidth']*2-15,,
         self~sizeY - 15,self~catalog['page']['btnwidth'],, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")
      if self~Wizard = 1 then
         self~createPushButtonGroup(5,self~sizeY - 15,self~catalog['page']['btnwidth'],,
                             ,lbtext" 11 PreviousPage" rbtext" 12 NextPage", 1, "")
   end
   else do
      self~createOkCancelRightBottom
      if self~Wizard = 1 then
         self~createPushButtonGroup(5, self~sizeY - 15,,, lbtext" 11 PreviousPage" rbtext" 12 NextPage", 1, "")
   end
   self~createBlackFrame(-1, 0, self~sizeY - 20, self~sizeX, 1)

::method createCategoryDialog private external "LIBRARY oodialog catdlg_createCategoryDialog"

::method initDialog protected
   /* set the mark to the first radio button */
   if self~catstyle~wordpos('DROPDOWN') > 0 then do
      do i = 1 to self~catalog['count']
         self~addComboEntry(9001, self~catalog['names'][i])
      end
      self~setComboBoxData(9001, self~catalog['names'][1])
   end
   else self~sendMessageToItem(9001,"0x000000F1",1,0)
   if self~wizard then self~disableItem(11)

   /* call InitXXX for each category if such a method exists */
   do i = 1 to self~catalog['count']
      self~catalog['category'] = i
      msg = "init" || self~catalog['names'][i]~space(0)
      -- Does user class provide the initXXX method? If so invoke it.
      if self~hasMethod(msg) then self~send(msg)
   end

   /* activate first page */
   self~catalog['category'] = 1
   self~showWindow(self~catalog['handles'][self~catalog['category']])

   /* */

::method initCategories protected
   return


   /* This method returs the currently selected page. Return value 1 means */
   /* the first page is selected. */

::method GetSelectedPage unguarded
   if self~catstyle~wordpos('DROPDOWN') = 0 then do
      np = 9001
      do while self~getRadioButtonData(np) = 0 & np < (9001 + self~catalog['count'])
         np = np + 1
      end
   end
   else do
      cur = self~getComboBoxData(9001)
      np = 1
      do while np <= ( self~catalog['count']) & cur \= self~catalog['names'][np]
         np = np + 1
      end
      if np > self~catalog['count'] then return 0
      np = np + 9000
   end
   return np-9000

   /* 'ChangePage' switch to the page 'NewPage' and shows it. */
   /* If 'NewPage is omitted the currently selected page will be activated - */
   /* (if not allready done). */


::method ChangePage unguarded
   use arg NewPage

   oldPage = self~catalog['category']
   /* if ChangePage is called automatically, 2 arg are passed (WParam and LParam) */
   if arg(2,'o') = 0 | NewPage > 9000 then NewPage = self~GetSelectedPage
   else do
      if self~catstyle~wordpos("DROPDOWN") = 0 then do
          /* SetRadioButton only works for registered radio buttons */
          /* uncheck the old radio button and check the new one */
          self~SendMessageToItem(9000+OldPage,"0x000000F1",0,0)
          self~SendMessageToItem(9000+NewPage,"0x000000F1",1,0)
      end
      else self~SetCurrentComboIndex(9001, NewPage)
   end
   if (NewPage = self~catalog['category']) | (NewPage < 1 | NewPage> self~catalog['count']) then return 0

   self~HideWindowFast(self~catalog['handles'][self~catalog['category']])
   self~catalog['category'] = NewPage
   if self~Wizard = 1 then do
      if NewPage = 1 then do
         self~DisableItem(11)
         self~EnableItem(12)
      end
      else if NewPage = self~catalog['count'] then do
         self~EnableItem(11)
         self~DisableItem(12)
      end
      else do
         self~EnableItem(11)
         self~EnableItem(12)
      end
   end
   self~ShowWindow(self~catalog['handles'][self~catalog['category']])

   self~PageHasChanged(oldPage, NewPage)
   return NewPage

::method PageHasChanged unguarded
   use arg Oldpage, NewPage
   return

::method PreviousPage unguarded
   oldPage = self~CurrentCategory
   if oldPage > 1 then self~ChangePage(oldPage-1)

::method NextPage unguarded
   oldPage = self~CurrentCategory
   if oldPage < self~catalog['count'] then self~ChangePage(oldPage+1)

::method BoxMessage unguarded private
   use arg category, id, msg, wP, lP
   if category~Datatype("N") = 0 then category = self~catalog['category']  /* assume current page is addressed */
   if category > 9000 then forward class (super)
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return SendWinMsg("PTR", self~catalog['handles'][category], id, msg, wP, lP)

::method AddCategoryComboEntry unguarded
   use arg id, data, category
   if arg(2,"o") = 1 then return -1
   return self~BoxMessage(category, id, 0x00000143, 0, "T" || data)

::method InsertCategoryComboEntry unguarded
   use arg id, item, data, category
   if arg(3,"o") = 1 then return -1
   if arg(2,"o") = 1 then item = self~GetCurrentCategoryComboIndex(id, category)
   return self~BoxMessage(category, id, 0x0000014A, item-1, "T" || data)

/* don't use string but the index */
::method DeleteCategoryComboEntry unguarded
   use arg id, index, category
   if arg(2,"o") = 1 then index = self~GetCurrentCategoryComboIndex(id, category)
   return self~BoxMessage(category, id, 0x00000144, index-1, 0)

::method FindCategoryComboEntry unguarded
   use arg id, data, category
   if arg(2,'o') = 1 | data = "DATA" then return -1
   item = self~BoxMessage(category, id, 0x0000014C, 0, "T" || data)
   if item > 0 then return item + 1; else return 0

::method GetCurrentCategoryComboIndex unguarded
   use arg id, category
   return self~BoxMessage(category, id, 0x00000147, 0, 0)+1

::method SetCurrentCategoryComboIndex unguarded
   use arg id, ndx, category
   if arg(2,'o') = 1 then ndx = 0
   return self~BoxMessage(category, id, 0x0000014E, ndx-1, 0)

::method GetCategoryComboItems unguarded
   use arg id, category
   return self~BoxMessage(category, id, 0x00000146, 0, 0)

::method GetCategoryComboEntry unguarded
   use arg id, ndx, category
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   len = SendWinMsg("DLG", self~catalog['handles'][category], id, 0x00000149, ndx-1,0)
   if len <= 0 then return ""
   else return SendWinMsg("PTR", self~catalog['handles'][category], id, 0x00000148, ndx-1,"G" || len+1)

::method ChangeCategoryComboEntry unguarded
   use arg id, item, data, category
   if arg(2,"o") = 1 then item = self~GetCurrentCategoryComboIndex(id, category)
   if item <= 0 then return -1
   self~DeleteCategoryComboEntry(id, item, category)
   return self~InsertCategoryComboEntry(id, item, data, category)

::method CategoryComboAddDirectory unguarded
   use arg id, drvpath, fattr, category
   READWRITE  = 0
   READONLY   = 1
   HIDDEN     = 2
   SYSTEM     = 4
   DIRECTORY  = 16
   ARCHIVE    = 32
   interpret( "opts = "fattr~translate('+',' ') )
   return self~BoxMessage(category, id, 0x00000145, opts, "T" || drvpath)

::method CategoryComboDrop unguarded
   use arg id, category
   return self~BoxMessage(category, id, 0x0000014B, 0, 0)


::method AddCategoryListEntry unguarded
   use arg id, data, category
   if arg(2,"o") = 1 then return -1
   return self~BoxMessage(category, id, 0x00000180, 0, "T" || data)

::method InsertCategoryListEntry unguarded
   use arg id, item, data, category
   if arg(3,"o") = 1 then return -1
   if arg(2,"o") = 1 then item = self~GetCurrentCategoryListIndex(id, category)
   return self~BoxMessage(category, id, 0x00000181, item-1, "T" || data)

/* don't use string but the index */
::method DeleteCategoryListEntry unguarded
   use arg id, index, category
   if arg(2,"o") = 1 then index = self~GetCurrentCategoryListIndex(id, category)
   return self~BoxMessage(category, id, 0x00000182, index-1, 0)

::method FindCategoryListEntry unguarded
   use arg id, data, category
   if arg(2,"o") = 1 then return -1
   item = self~BoxMessage(category, id, 0x0000018F, 0, "T" || data)
   if item > 0 then return item + 1; else return 0

::method GetCurrentCategoryListIndex unguarded
   use arg id, category
   return self~BoxMessage(category, id, 0x00000188, 0, 0) + 1

::method SetCurrentCategoryListIndex unguarded
   use arg id, ndx, category
   if arg(2,'o') = 1 then ndx = 0
   return self~BoxMessage(category, id, 0x00000186, ndx-1, 0)

::method GetCategoryListItems unguarded
   use arg id, category
   return self~BoxMessage(category, id, 0x0000018B, 0, 0)

::method GetCategoryListEntry unguarded
   use arg id, ndx, category
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   len = SendWinMsg("DLG", self~catalog['handles'][category], id, 0x0000018A, ndx-1,0)
   if len <= 0 then return ""
   else return SendWinMsg("PTR", self~catalog['handles'][category], id, 0x00000189, ndx-1,"G" || len+1)

::method ChangeCategoryListEntry unguarded
   use arg id, item, data, category
   if arg(2,"o") = 1 then item = self~GetCurrentCategoryListIndex(id, category)
   if item <= 0 then return -1
   self~DeleteCategoryListEntry(id, item, category)
   return self~InsertCategoryListEntry(id, item, data, category)

::method SetCategoryListTabulators unguarded external "LIBRARY oodialog generic_setListTabulators"


::method CategoryListAddDirectory unguarded
   use arg id, drvpath, fattr, category
   READWRITE  = 0
   READONLY   = 1
   HIDDEN     = 2
   SYSTEM     = 4
   DIRECTORY  = 16
   ARCHIVE    = 32
   interpret( "opts = "fattr~translate('+',' ') )
   return self~BoxMessage(category, id, 0x0000018D, opts, "T" || drvpath)


::method CategoryListDrop unguarded
   use arg id, category
   return self~BoxMessage(category, id, 0x00000184, 0, 0)


-- The following methods are to set or get the 'data' of a single dialog control on one of
-- the category pages of the CategroyDialog.  It is  necessary to know the category page number
-- of the dialog control in order for to correctly identify the control.  If the page number
-- argument is omitted, it is assumed the page number is the current page number.

::method getControlDataPage unguarded external "LIBRARY oodialog catdlg_getControlDataPage"
::method getCheckBoxDataPage unguarded external "LIBRARY oodialog catdlg_getControlDataPage"
::method getRadioButtonDataPage unguarded external "LIBRARY oodialog catdlg_getControlDataPage"
::method getEditDataPage unguarded external "LIBRARY oodialog catdlg_getControlDataPage"
::method getListBoxDataPage unguarded external "LIBRARY oodialog catdlg_getControlDataPage"
::method getComboBoxDataPage unguarded external "LIBRARY oodialog catdlg_getControlDataPage"

::method setControlDataPage unguarded external "LIBRARY oodialog catdlg_setControlDataPage"
::method setStaticDataPage unguarded external "LIBRARY oodialog catdlg_setControlDataPage"
::method setCheckBoxDataPage unguarded external "LIBRARY oodialog catdlg_setControlDataPage"
::method setRadioButtonDataPage unguarded external "LIBRARY oodialog catdlg_setControlDataPage"
::method setEditDataPage unguarded external "LIBRARY oodialog catdlg_setControlDataPage"
::method setListBoxDataPage unguarded external "LIBRARY oodialog catdlg_setControlDataPage"
::method setComboBoxDataPage unguarded external "LIBRARY oodialog catdlg_setControlDataPage"

::method getControlDataAttribute unguarded
   use arg attributeName, category
   if arg(2, 'O') | \ category~datatype("W") then category = self~catalog['category']  /* try current */
   id = self~dataConnection~first
   do while (id \= .nil)
      if self~dataConnection[id] = attributeName then do
         data = self~getControlData(id, category)
         self~sendWith(attributeName'=', .array~of(data))
         return
      end
      i = self~dataConnection~Next(i)
   end

::method setControlDataAttribute unguarded
   use arg attributeName, category
   if arg(2, 'O') | \ category~datatype("W") then category = self~catalog['category']  /* try current category page */
   id = self~dataConnection~first
   do while (id \= .nil)
      if self~dataConnection[id] = attributeName then do
         data = self~send(attributeName)
         return self~setControlData(id, data, category)
      end
      id = self~dataConnection~next(id)
   end

-- DEPRECATED
::method getCategoryEntryLine unguarded
   forward message 'getEditDataPage'
::method getCategoryListLine unguarded
   forward message 'getListBoxDataPage'
::method getCategoryComboLine unguarded
   forward message 'getComboBoxDataPage'
::method getCategoryCheckBox unguarded
   forward message 'getCheckBoxDataPage'
::method getCategoryRadioButton unguarded
   forward message 'getRadioButtonDataPage'
::method getCategoryMultiList unguarded
   forward message 'getListBoxDataPage'
::method getCategoryValue unguarded
   forward message 'getControlDataPage'
::method getCategoryAttrib unguarded
   forward message 'getControlDataAttribute'
::method setCategoryStaticText unguarded
   forward message 'setStaticDataPage'
::method setCategoryEntryLine unguarded
   forward message 'setEditDataPage'
::method setCategoryListLine unguarded
   forward message 'setListBoxDataPage'
::method setCategoryComboLine unguarded
   forward message 'setComboBoxDataPage'
::method setCategoryCheckBox unguarded
   forward message 'setCheckBoxDataPage'
::method setCategoryRadioButton unguarded
   forward message 'setRadioButtonDataPage'
::method setCategoryMultiList unguarded
   forward message 'setListBoxDataPage'
::method setCategoryValue unguarded
   forward message 'setControlDataPage'
::method setCategoryAttrib unguarded
   forward message 'setControlDataAttribute'


::method enableCategoryItem unguarded
   use arg id, category
   if arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   hwnd = self~getControlHandle(id, self~catalog['handles'][category])
   if .SystemErrorCode <> 0 then return 1
   .Window~new(hwnd)~enable

::method disableCategoryItem unguarded
   use arg id, category
   if arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   hwnd = self~getControlHandle(id, self~catalog['handles'][category])
   if .SystemErrorCode <> 0 then return 1
   .Window~new(hwnd)~disable

::method HideCategoryItem unguarded
   use arg id, category
   if arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   h = self~getItem(id, self~catalog['handles'][category])
   if h = 0 then return 1
   return WndShow_Pos("S",h, "HIDE")

::method ShowCategoryItem unguarded
   use arg id, category
   if arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   h = self~getItem(id, self~catalog['handles'][category])
   if h = 0 then return 1
   return WndShow_Pos("S",h, "NORMAL")

::method getItem unguarded
   use arg id, hDlg

   forward class (super) continue
   hwnd = result
   if hwnd == -1 then return -1

   if hwnd = 0 & arg(2,'O') then do
       if \ id~datatype("W") then id = self~resolveSymbolicId(id)
       if id == -1 then return -1

       -- Try current page.
       hwnd = self~getItem:super(id, self~catalog['handles'][self~catalog['category']])

       -- If still no good, try all pages.
       if hwnd = 0 then do i = 1 to self~catalog['count'] while hwnd = 0
           hwnd = self~getItem:super(id, self~catalog['handles'][i])
       end
   end
   return hwnd

::method SendMessageToCategoryItem unguarded
   parse arg id,a2,a3,a4, category
   if arg(5,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   return SendWinMsg("DLG", self~catalog['handles'][category],id,a2,a3,a4)

::method SetCategoryItemFont unguarded
   use arg id, hFont, bRedraw, category     /* WM_SETFONT */
   if arg(4,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   if arg(3, 'o') = 1 then bRedraw = 1
   return SendWinMsg("ANY",self~getItem(id, self~catalog['handles'][category]), "0x0030", hFont, bRedraw)

::method FocusCategoryItem unguarded
   use arg id, category
   if arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return SendWinMsg("ANY",self~catalog['handles'][category], "0x0028", self~getItem(id, self~catalog['handles'][category]), 1)

::method ResizeCategoryItem unguarded
   use arg id, width, high, showOptions, category
   if arg(5,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   if arg(4,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   hnd = self~getItem(id, self~catalog['handles'][category])
   if hnd = 0 then return -1
   return WndShow_Pos("P",hnd, 0, 0, width * self~factorX, high* self~factorY,,
                           "NOMOVE "||showOptions)

   /* This method will move a dialog item to an other position */

::method MoveCategoryItem unguarded
   use arg id, xPos, yPos, showOptions, category
   if arg(5,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   if arg(4,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   h = self~getItem(id, self~catalog['handles'][category])
   if h = 0 then return -1
   return WndShow_Pos("P",h, xPos * self~factorX, yPos * self~factorY, 0, 0,,
                           "NOSIZE "||showOptions)


/* set/get the horizontal scroll width of a list box */

::method SetCategoryListWidth unguarded
   use arg id, dlgunits, category
   if arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(category, id, 0x00000194, dlgunits*self~factorX, 0)

::method GetCategoryListWidth unguarded
   use arg id, category
   return self~BoxMessage(category, id, 0x00000193, 0, 0)/self~factorX

::method SetCategoryListColumnWidth unguarded
   use arg id, dlgunits, category
   if arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(category, id, 0x00000195, dlgunits*self~factorX, 0)

::method SetCategoryListItemHeight unguarded
   use arg id, dlgunits, category
   if arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(category, id, 0x000001A0, 0, dlgunits*self~factorY)

::method GetCategoryListItemHeight unguarded
   use arg id, category
   return self~BoxMessage(category, id, 0x000001A1, 0, 0)/self~factorY


