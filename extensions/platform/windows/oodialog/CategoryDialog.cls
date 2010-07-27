/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2010 Rexx Language Association. All rights reserved.    */
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
 *
 * This class creates and controls a dialog that has more than one panel. It is
 * similar to the 'notebook' control available in OS/2 or the PropertySheet
 * available in the Windows 95 user interface. You can switch among diferent
 * pages by clicking radio buttons. Each page has its own window controls. You
 * can setup a page out of a dialog resource script.
 */
::requires "UserDialog.cls"

::class 'CategoryDialog' subclass UserDialog public

-- A directory that contains internal data.  This should be private, but I'm not sure of the
-- consequences to existing programs if it is changed.  MM
::attribute catalog protected

::attribute staticID private
::attribute catX private
::attribute catY private
::attribute catCX private
::attribute catStyle private
::attribute catLabel private
::attribute catMax private
::attribute wizard private

-- arg cdstyle=[ "DROPDOWN" | "TOPLINE" | "NOBORDER" | "WIZARD"] */
::method init
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
   if arg(2,'o') = 1 then self~catX = 10; else self~catX = cdx
   if arg(3,'o') = 1 then self~catY = 4; else self~catY = cdy
   if arg(4,'o') = 1 then self~catCX = 0; else self~catCX = cdcx
   if arg(5,'o') = 1 then self~catStyle = ""; else self~catStyle = cdstyle~translate
   if arg(6,'o') = 1 then cdtable = ""
   if arg(7,"o") = 1 then self~catLabel = "Page:"; else self~catLabel = cdlabel
   if arg(8,"o") = 1 then self~catMax = 0; else self~catMax = cdmax

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

   self~initCategories()    /* user overwrite */

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

   if self~catStyle~wordpos("WIZZARD") > 0 | self~catStyle~wordpos("WIZARD") > 0 then do
      self~wizard = .true
      self~catStyle = self~catStyle~changestr("WIZZARD", "")
   end
   else self~wizard = .false

-- Documented "internal use only" in 3.2.0 - Do not document.
   self~catalog['id'] = .array~new(self~catalog['count'],100)
   self~catalog['base'] = .array~new(self~catalog['count'])
   self~catalog['handles'] = .array~new(self~catalog['count'])

   self~staticID = 9600

   return 0


-- Documented "internal use only" in 3.2.0 - Do not document.
::method startIt   /* don't call parent startIt */
   if self~dlgHandle = 0 then return 0
   self~initDialog
   return self~dlgHandle

::method defineDialog protected
   if self~basePtr = 0 then return 0
   if self~catStyle~wordpos('DROPDOWN') = 0 then
      do i = 9001 to (9000 + self~catalog['count'])
         self~connectButtonEvent(i, "CLICKED", "CHANGEPAGE")
      end
   else self~connectListBoxEvent(9001, "SELCHANGE", "CHANGEPAGE")

   do i = 1 to self~catalog['count']
      catnames.i = self~catalog['names'][i]
   end


-- Documented "internal use only" in 3.2.0 - Do not document.
   self~catalog['category'] = 0

   n = self~catStyle~wordpos("TOPLINE")
   if n > 0 then self~catMax = 1
   m = self~catStyle~wordpos("DROPDOWN")

   if (n > 0 | m > 0) & self~catStyle~wordpos('NOBORDER') = 0 then self~catStyle = self~catStyle||" NOBORDER"

   if self~catalog['page']['h'] = .nil then self~catalog['page']['h'] = self~sizeY - 40

   size = self~getTextSizeDu("Hg")
   textHeight = size~height

   if self~catCX = 0 & n = 0 then  /* dropdown or none */
   do
      do i = 1 to self~catalog['count']
         if self~catalog['page']['font'] <> "" then
              size = self~getTextSizeDlg(catnames.i,self~catalog['page']['font'],self~catalog['page']['fsize'])
         else
              size = self~getTextSizeDu(catnames.i)
         self~catCX = max(self~catCX,size~width)
      end
      if n = 0 then self~catCX = self~catCX + 20
   end

   if n > 0 | m > 0 then do  /* topline or dropdown */
      if self~catalog['page']['x'] = .nil then self~catalog['page']['x'] = 1
      if m > 0 then do
        if self~catalog['page']['y'] = .nil then self~catalog['page']['y'] = self~catY*2+textHeight+4
      end
      else if self~catalog['page']['y'] = .nil then self~catalog['page']['y'] = self~catY+textHeight+5

      if self~catalog['page']['w'] = .nil then self~catalog['page']['w'] = self~sizeX - 2
   end
   else do
      if self~catMax = 0 then do
        if self~catalog['page']['x'] = .nil then self~catalog['page']['x'] = self~catX+10+self~catCX
      end
      else
        if self~catalog['page']['x'] = .nil then self~catalog['page']['x'] = self~catX+10+self~catCX * ((self~catalog['count']-1)%self~catMax+1)

      if self~catalog['page']['y'] = .nil then self~catalog['page']['y'] = 5

      if self~catalog['page']['w'] = .nil then self~catalog['page']['w'] = self~sizeX - (self~catX+self~catCX+20)
   end

   self~categoryPage

   if m > 0 then do
      if self~catMax = 0 then self~catMax = 5
      if self~catalog['count'] < self~catMax then self~catMax = self~catalog['count']
      if self~catMax < self~catalog['count'] then
           if self~catLabel='' then
                self~createComboBox(9001, self~catX, self~catY, self~catCX, (self~catMax+1)*(textHeight+4), -
                                 self~catStyle~delword(m, 1) || " CAT VSCROLL")
           else self~createComboBoxInput(9001, self~catX, self~catY, , self~catCX, self~catMax,             -
                                         self~catLabel, self~catStyle~delword(m, 1) || " CAT VSCROLL")
      else if self~catLabel='' then
                self~createComboBox(9001, self~catX,self~catY,self~catCX,(self~catMax+1)*(textHeight+4),    -
                                    self~catStyle~delword(m, 1) || " CAT")
           else self~createComboBoxInput(9001, self~catX, self~catY, , self~catCX, self~catMax,             -
                                         self~catLabel, self~catStyle~delword(m, 1) || " CAT")
   end
   else do
      if (n >0) then newstyle = self~catStyle~delword(n, 1)
                else newstyle = self~catStyle
      if self~catalog['page']['font'] <> "" then
         self~createRadioButtonStem(9001, self~catX, self~catY, self~catCX, catnames., self~catMax,,
                           newstyle || " CAT",,self~catalog['page']['font'],self~catalog['page']['fsize'])
      else
         self~createRadioButtonStem(9001, self~catX, self~catY, self~catCX, catnames., self~catMax, newstyle || " CAT")
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
::method currentCategory unguarded
   return self~catalog['category']

-- This method adds controls to CategoryDialog dialog itself.  The dialog that is the parent to
-- the child dialogs that make up the pages of the CategoryDialog.
::method categoryPage protected

   size = self~getTextSizeDu("Hg")
   textHeight = size~height

   if self~catStyle~wordpos("TOPLINE") > 0 then
      self~createBlackFrame(-1, 0, 2, self~sizeX, textHeight + (self~catY+2))
   else if self~catStyle~wordpos("DROPDOWN") > 0 then
      self~createBlackFrame(-1, 0, 2, self~sizeX, textHeight + (self~catY*2)+1)

   lbtext = self~catalog['page']['leftbtntext']
   rbtext = self~catalog['page']['rightbtntext']
   if lbtext~pos(" ") > 0 then lbtext = '"' || lbtext || '"'
   if rbtext~pos(" ") > 0 then rbtext = '"' || rbtext || '"'

   wizardTextIdMsg = lbtext" 11 PreviousPage" rbtext" 12 NextPage"
   y = self~sizeY - 15

   if (self~catalog['page']['btnwidth'] > 0) then do
      cx = self~catalog['page']['btnwidth']
      x = trunc(self~sizeX - self~catalog['page']['btnwidth'] * 2 - 15)

      self~createPushButtonGroup(x, y, cx, , "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")
      if self~wizard then self~createPushButtonGroup(5, y, cx,  , wizardTextIdMsg, 1, "")
   end
   else do
      self~createOkCancelRightBottom
      if self~wizard then self~createPushButtonGroup(5, y, , , wizardTextIdMsg, 1, "")
   end

   self~createBlackFrame(-1, 0, self~sizeY - 20, self~sizeX, 1)

::method createCategoryDialog private external "LIBRARY oodialog catdlg_createCategoryDialog"

::method initDialog protected
   /* set the mark to the first radio button */
   if self~catStyle~wordpos('DROPDOWN') > 0 then do
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
   if self~catStyle~wordpos('DROPDOWN') = 0 then do
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
      if self~catStyle~wordpos("DROPDOWN") = 0 then do
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
   if self~wizard then do
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


::method sendMessageToCategoryControl unguarded external "LIBRARY oodialog catdlg_sendMessageToCategoryControl"
::method categoryComboBox unguarded private
   use strict arg id, pageID
   if pageID == -1 then pageID = self~catalog['category']
   return self~newComboBox(id, pageID)

::method addCategoryComboEntry unguarded
   use strict arg id, string, pageID = (-1)
   comboBox = self~categoryComboBox(id, pageID)
   if comboBox == .nil then return -1
   else return comboBox~add(string)

::method insertCategoryComboEntry unguarded
   use strict arg id, index = 0, string, pageID = (-1)
   comboBox = self~categoyComboBox(id, pageID)
   if comboBox == .nil then return -1
   if arg(2, 'O') then return comboBox~insert( , string)
   else return comboBox~insert(index, string)

::method getCategoryComboEntry unguarded
   use strict arg id, index, pageID = (-1)
   comboBox = self~categoyComboBox(id, pageID)
   if comboBox == .nil then return -1
   return comboBox~getText(index)

::method findCategoryComboEntry unguarded
   use strict arg id, string, pageID = (-1)
   comboBox = self~categoryComboBox(id, pageID)
   if comboBox == .nil then return -1
   return comboBox~find(string)

::method categoryComboAddDirectory unguarded
   use strict arg id, drivepath, fileAttributes = "", pageID = (-1)
   comboBox = self~categoryComboBox(id, pageID)
   if comboBox == .nil then return -1
   return comboBox~addDirectory(drivePath, fileAttibutes)

::method deleteCategoryComboEntry unguarded
   use strict arg id, index = (-2), pageID = (-1)
   comboBox = self~categoryComboBox(id, pageID)
   if comboBox == .nil then return -1
   if index == -2 then index = self~getCurrentCategoryComboIndex(id, pageID)
   if index <= 0 then return -1
   return comboBox~delete(index)

::method getCurrentCategoryComboIndex unguarded
   use strict arg id, pageID = (-1)
   comboBox = self~categoryComboBox(id, pageID)
   if comboBox == .nil then return -1
   return comboBox~selectedIndex

::method setCurrentCategoryComboIndex unguarded
   use strict arg id, ndx = 0, pageID = (-1)
   comboBox = self~categoryComboBox(id, pageID)
   if comboBox == .nil then return -1
   return comboBox~selectIndex(ndx)

::method getCategoryComboItems unguarded
   use strict arg id, pageID = (-1)
   comboBox = self~categoryComboBox(id, pageID)
   if comboBox == .nil then return -1
   return comboBox~items

::method changeCategoryComboEntry unguarded
   use strict arg id, item = (-2), newEntry, pageID = (-1)
   if item == -2 then item = self~getCurrentCategoryComboIndex(id, pageID)
   if item <= 0 then return -1
   ret = self~deleteCategoryComboEntry(id, item, pageID)
   if ret <= 0 then return -1
   return self~insertCategoryComboEntry(id, item, newEntry, pageID)

::method categoryComboDrop unguarded
   use strict arg id, pageID = (-1)
   comboBox = self~categoryComboBox(id, pageID)
   if comboBox == .nil then return -1
   return comboBox~deleteAll


-- Methods to manipulate a list box.

::method categoryListBox unguarded private
   use strict arg id, pageID
   if pageID == -1 then pageID = self~catalog['category']
   return self~newListBox(id, pageID)

::method setCategoryListTabulators unguarded external "LIBRARY oodialog generic_setListTabulators"

::method addCategoryListEntry unguarded
   use strict arg id, text, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   else return listBox~add(text)

::method insertCategoryListEntry unguarded
   use strict arg id, index = (-2), text, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   if arg(2, 'O') then return listBox~insert( , text)
   else return listBox~insert(index, text)

::method findCategoryListEntry unguarded
   use strict arg id, text, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   return listBox~find(text)

::method getCategoryListEntry unguarded
   use strict arg id, index, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   return listBox~getText(index)

::method categoryListAddDirectory unguarded
   use strict arg id, drivePath, fileAttributes = "", pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   return listBox~addDirectory(drivePath, fileAttributes)

::method deleteCategoryListEntry unguarded
   use strict arg id, index = (-2), pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   if index == -2 then index = self~getCurrentCategoryListIndex(id, pageID)
   if index <= 0 then return -1
   return listBox~delete(index)

::method getCurrentCategoryListIndex unguarded
   use strict arg id, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   return listBox~selectedIndex

::method setCurrentCategoryListIndex unguarded
   use strict arg id, ndx = 0, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   return listBox~selectIndex(ndx)

::method getCategoryListItems unguarded
   use strict arg id, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   return listBox~items

::method changeCategoryListEntry unguarded
   use strict arg id, item = (-2), newEntry, pageID = (-1)
   if item == -2 then item = self~getCurrentCategoryListIndex(id, pageID)
   if item <= 0 then return -1
   ret = self~deleteCategoryListEntry(id, item, pageID)
   if ret <= 0 then return -1
   return self~insertCategoryListEntry(id, item, newEntry, pageID)

::method categoryListDrop unguarded
   use strict arg id, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   return listBox~deleteAll

::method setCategoryListWidthPx unguarded
   use strict arg id, pixels, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   return listBox~setWidthPx(pixels)

::method getCategoryListWidthPx unguarded
   use strict arg id, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   return listBox~widthPx

::method setCategoryListColumnWidthPx unguarded
   use strict arg id, pixels, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   listBox~columnWidthPx = pixels
   return 0

::method setCategoryListItemHeightPx unguarded
   use strict arg id, pixels, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   listBox~listItemHeightPx = pixels
   return 0

::method getCategoryListItemHeightPx unguarded
   use strict arg id, pageID = (-1)
   listBox = self~categoryListBox(id, pageID)
   if listBox == .nil then return -1
   return listBox~itemHeightPx

-- The following methods are generic methods to work with a dialog control.

::method getControlHandle unguarded
   use arg id, hDlg

   forward class (super) continue
   hwnd = result

   if hwnd = 0 & arg(2,'O') then do
       if \ id~datatype("W") then id = self~resolveSymbolicId(id)
       if id == -1 then return -1

       -- Try current page.
       hwnd = self~getControlHandle:super(id, self~catalog['handles'][self~catalog['category']])

       -- If still no good, try all pages.
       if hwnd = 0 then do i = 1 to self~catalog['count'] while hwnd = 0
           hwnd = self~getControlHandle:super(id, self~catalog['handles'][i])
       end
   end
   return hwnd

::method enableCategoryControl unguarded
   use strict arg id, category = ""
   if arg(2, 'O') | \ category~datatype("W") then category = self~catalog['category']  /* try current */

   hwnd = self~getControlHandle(id, self~catalog['handles'][category])
   if hwnd = 0 then return 1
   .Window~new(hwnd)~enable

::method disableCategoryControl unguarded
   use strict arg id, category = ""
   if arg(2, 'O') | \ category~datatype("W") then category = self~catalog['category']  /* try current */

   hwnd = self~getControlHandle(id, self~catalog['handles'][category])
   if hwnd = 0 then return 1
   .Window~new(hwnd)~disable

::method hideCategoryControl unguarded
   use strict arg id, category = ""
   if arg(2, 'O') | \ category~datatype("W") then category = self~catalog['category']  /* try current */

   hwnd = self~getControlHandle(id, self~catalog['handles'][category])
   if hwnd = 0 then return 1
   return .Window~new(hwnd)~hide

::method showCategoryControl unguarded
   use strict arg id, category = ""
   if arg(2, 'O') | \ category~datatype("W") then category = self~catalog['category']  /* try current */

   hwnd = self~getControlHandle(id, self~catalog['handles'][category])
   if hwnd = 0 then return 1
   .Window~new(hwnd)~show

::method resizeCategoryControl unguarded
   use strict arg id, width, height, showOptions = "", category = ""
   if arg(5,'O') | \ category~datatype("W") then category = self~catalog['category']  /* try current */

   hwnd = self~getControlHandle(id, self~catalog['handles'][category])
   if hwnd = 0 then return -1
   size = .Size~new(width, height)
   return .Window~new(hwnd)~resizeTo(size, showOptions)

::method moveCategoryControl unguarded
   use strict arg id, xPos, yPos, showOptions = "", category = ""
   if arg(5,'O') | \ category~dataType("W") then category = self~catalog['category']  /* try current */

   hwnd = self~getControlHandle(id, self~catalog['handles'][category])
   if hwnd = 0 then return -1
   pos = .Point~new(xPos, yPos)
   return .Window~new(hwnd)~moveTo(pos, showOptions)

::method setCategoryControlFont unguarded
   use strict arg id, hFont, bRedraw = 1, category = ""
   if arg(4, 'O') | \ category~datatype("W") then category = self~catalog['category']  /* try current */

   hCtrl = self~getControlHandle(id, self~catalog['handles'][category])
   msg   = self~WM_SETFONT
   return self~sendWinHandleMsg(msg, hFont, bRedraw, hCtrl)

::method focusCategoryControl unguarded
   use strict arg id, category = ""
   if arg(2,'O') | \ category~datatype("W") then category = self~catalog['category']  /* try current */

   hCtrl = self~getControlHandle(id, self~catalog['handles'][category])
   hDlg  = self~catalog['handles'][category]
   msg   = self~WM_NEXTDLGCTL
   return self~sendWinHandleMsg(msg, hCtrl, 1, hDlg)


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

::method getDataAttribute unguarded
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

::method setDataAttribute unguarded
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



-- DEPRECATED and DEPRECATED Maybe to end of file
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
   forward message 'getDataAttribute'
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
   forward message 'setDataAttribute'
::method sendMessageToCategoryItem unguarded
   forward message "sendMessageToCategoryControl"
::method getItem unguarded
   forward message 'getControlHandle'
::method enableCategoryItem unguarded
   forward message 'enableCategoryControl'
::method disableCategoryItem unguarded
   forward message 'disableCategoryControl'
::method hideCategoryItem unguarded
   forward message 'hideCategoryControl'
::method showCategoryItem unguarded
   forward message 'showCategoryControl'
::method setCategoryItemFont unguarded
   forward message "setCategoryControlFont"
::method focusCategoryItem unguarded
   forward message "focusCategoryControl"

-- DEPRECATED (maybe?)
::method resizeCategoryItem unguarded            -- Old method using dialog units, this is not accurate
   use strict arg id, width, height, showOptions = "", category = ""
   if arg(5, 'O') | \ category~datatype("W") then category = self~catalog['category']

   hwnd = self~getControlHandle(id, self~catalog['handles'][category])
   if hwnd = 0 then return -1
   size = .Size~new(trunc(width * self~factorX), trunc(height * self~factorY))
   return .Window~new(hwnd)~resizeTo(size, showOptions)

::method moveCategoryItem unguarded              -- Old method using dialog units, this is not accurate
   use strict arg id, xPos, yPos, showOptions = "", category = ""
   if arg(5, 'O') | \ category~datatype("W") then category = self~catalog['category']  /* try current */

   hwnd = self~getControlHandle(id, self~catalog['handles'][category])
   if hwnd = 0 then return -1
   pos = .Point~new(trunc(xPos * self~factorX), trunc(yPos * self~factorY))
   return .Window~new(hwnd)~moveTo(pos, showOptions)

::method getCategoryListItemHeight unguarded     -- Old method using dialog units, this is not accurate
   use strict arg id, pageID = (-1)
   pixels = self~getCategoryListItemHeightPx(id, pageID)
   if pixels <= 0 then return pixels
   else return pixels / self~factorY

::method setCategoryListWidth unguarded          -- Old method using dialog units, this is not accurate
   use strict arg id, dlgunits, pageID = (-1)
   pixels = dlgunits * self~factorX
   return self~setCategoryListWidthPx(id, pixels, pageID)

::method getCategoryListWidth unguarded          -- Old method using dialog units, this is not accurate
   use strict arg id, pageID = (-1)
   pixels = self~getListWidthPx(id, pageID)
   if pixels <= 0 then return pixels
   else return pixels / self~factorX

::method setCategoryListColumnWidth unguarded    -- Old method using dialog units, this is not accurate
   use strict arg id, dlgunits, pageID = (-1)
   pixels = dlgunits * self~factorX
   return self~setCategoryListColumnWidthPx(id, pixels, pageID)

::method setCategoryListItemHeight unguarded     -- Old method using dialog units, this is not accurate
   use strict arg id, dlgunits, pageID = (-1)
   pixels = dlgunits * self~factorY
   return self~setCategoryItemHeightPx(id, pixels, pageID)

-- End DEPRECATED (maybe ?)
