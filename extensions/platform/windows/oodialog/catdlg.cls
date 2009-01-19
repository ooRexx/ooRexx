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
/* Category Dialog Class                                                      */
/*                                                                            */
/******************************************************************************/
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
   use arg DlgData., cdx, cdy, cdcx, cdstyle, cdtable, cdlabel, cdmax, includeFile

   newArgs = .array~new(2)
   if Arg(1, 'E') then newArgs[1] = Arg(1)
   if Arg(9, 'E') then newArgs[2] = Arg(9)
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

   if Arg(1,'o') = 1 then self~UseStem = 0; else self~UseStem = 1
   if Arg(2,'o') = 1 then self~catx = 10; else self~catX = cdx
   if Arg(3,'o') = 1 then self~caty = 4; else self~caty = cdy
   if Arg(4,'o') = 1 then self~catcx = 0; else self~catcx = cdcx
   if Arg(5,'o') = 1 then self~catstyle = ""; else self~catstyle = cdstyle~translate
   if Arg(6,'o') = 1 then cdtable = ""
   if Arg(7,"o") = 1 then self~catlabel = "Page:"; else self~catlabel = cdlabel
   if Arg(8,"o") = 1 then self~catmax = 0; else self~catmax = cdmax

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


::method StartIt   /* don't call parent startit */
   if self~DlgHandle = 0 then return 0
   self~InitDialog
   return self~DlgHandle

::method DefineDialog protected
   if self~baseptr = 0 then return 0
   if self~catstyle~wordpos('DROPDOWN') = 0 then
      do i = 9001 to (9000 + self~catalog['count'])
         self~ConnectControl(i, "CHANGEPAGE")
      end
   else self~ConnectList(9001, "CHANGEPAGE")

   do i = 1 to self~catalog['count']
      catnames.i = self~catalog['names'][i]
   end

   self~catalog['category'] = 0
   n = self~catstyle~wordpos("TOPLINE")
   if n > 0 then self~catmax = 1
   m = self~catstyle~wordpos("DROPDOWN")

   if (n > 0 | m > 0) & self~catstyle~wordpos('NOBORDER') = 0 then self~catstyle = self~catstyle||" NOBORDER"

   if self~catalog['page']['h'] = .Nil then self~catalog['page']['h'] = self~SizeY - 40

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
      if self~catalog['page']['x'] = .Nil then self~catalog['page']['x'] = 1
      if m > 0 then do
        if self~catalog['page']['y'] = .Nil then self~catalog['page']['y'] = self~caty*2+textHeight+4
      end
      else if self~catalog['page']['y'] = .Nil then self~catalog['page']['y'] = self~caty+textHeight+5

      if self~catalog['page']['w'] = .Nil then self~catalog['page']['w'] = self~SizeX - 2
   end
   else do
      if self~catmax = 0 then do
        if self~catalog['page']['x'] = .Nil then self~catalog['page']['x'] = self~catX+10+self~catcx
      end
      else
        if self~catalog['page']['x'] = .Nil then self~catalog['page']['x'] = self~catX+10+self~catcx * ((self~catalog['count']-1)%self~catmax+1)

      if self~catalog['page']['y'] = .Nil then self~catalog['page']['y'] = 5

      if self~catalog['page']['w'] = .Nil then self~catalog['page']['w'] = self~SizeX - (self~catX+self~catcx+20)
   end

   self~CategoryPage

   if m > 0 then
   do
      if self~catmax = 0 then self~catmax = 5
          if self~catalog['count'] < self~catmax then self~catmax = self~catalog['count']
      if self~catmax < self~catalog['count'] then
           if self~catlabel='' then
                self~AddComboBox(9001,,self~catX,self~caty,self~catcx,(self~catmax+1)*(textHeight+4),,
                                 self~catstyle~delword(m, 1) || " CAT VSCROLL")
           else self~AddComboInput(9001,, self~catX, self~caty,, self~catcx, self~catmax,,
                                   self~catlabel, self~catstyle~delword(m, 1) || " CAT VSCROLL")
      else if self~catlabel='' then
                self~AddComboBox(9001,,self~catX,self~caty,self~catcx,(self~catmax+1)*(textHeight+4),,
                                 self~catstyle~delword(m, 1) || " CAT")
           else self~AddComboInput(9001,, self~catX, self~caty,, self~catcx, self~catmax,,
                                   self~catlabel, self~catstyle~delword(m, 1) || " CAT")
   end
   else do
      if (n >0) then newstyle = self~catstyle~delword(n, 1)
                else newstyle = self~catstyle
      if self~catalog['page']['font'] <> "" then
         self~AddRadioStem(9001, self~catX, self~caty, self~catcx, catnames., self~catmax,,
                           newstyle || " CAT",,self~catalog['page']['font'],self~catalog['page']['fsize'])
      else
         self~AddRadioStem(9001, self~catX, self~caty, self~catcx, catnames., self~catmax, newstyle || " CAT")
   end

   self~DlgHandle = UsrCreateDialog(self~Adm, "PARENT", self~DialogItemCount, 0, self~BasePtr, self~AutoDetect, 1, 0, 0)
   self~activePtr = 0
   if self~DlgHandle = 0 then return 0

   do i = 1 to self~catalog['count']
      self~DialogItemCount = 0
      self~catalog['category'] = i
      p = self~catalog['page']
      self~CreateCategoryDialog(p['x'], p['y'], p['w'], p['h'],p['font'], p['fsize'], p['style'] || " CHILD HIDDEN", p['expected'])
      msg = "self~" || self~catalog['names'][i]~space(0)
      interpret(msg)
      ret = UsrCreateDialog(self~Adm, "CHILD", self~DialogItemCount, self~catalog['base'][i], self~DlgHandle, i)
      self~activePtr = 0
      self~catalog['handles'][i] = ret
   end

   /* This method returns the current page */

::method CurrentCategory unguarded
   return self~catalog['category']

   /* This method adds controls to the base window of a 'CategoryDialog' */

::method CategoryPage protected

   size = self~getTextSizeDlg("Hg")
   textHeight = size~height

   if self~catstyle~wordpos("TOPLINE") > 0 then
      self~addBlackFrame(0, 2, self~SizeX, textHeight + (self~caty+2))
   else if self~catstyle~wordpos("DROPDOWN") > 0 then
      self~addBlackFrame(0, 2, self~SizeX, textHeight + (self~caty*2)+1)

   lbtext = self~catalog['page']['leftbtntext']
   rbtext = self~catalog['page']['rightbtntext']
   if lbtext~pos(" ") > 0 then lbtext = '"' || lbtext || '"'
   if rbtext~pos(" ") > 0 then rbtext = '"' || rbtext || '"'
   if (self~catalog['page']['btnwidth'] > 0) then
   do
      self~AddButtonGroup(self~SizeX-self~catalog['page']['btnwidth']*2-15,,
         self~SizeY - 15,self~catalog['page']['btnwidth'],, "&Ok 1 OK &Cancel 2 CANCEL", 1, "FIX DEFAULT")
      if self~Wizard = 1 then
         self~AddButtonGroup(5,self~SizeY - 15,self~catalog['page']['btnwidth'],,
                             ,lbtext" 11 PreviousPage" rbtext" 12 NextPage", 1, "FIX")
   end
   else do
      self~AddOkCancelRightBottom
      if self~Wizard = 1 then
         self~AddButtonGroup(5, self~SizeY - 15,,, lbtext" 11 PreviousPage" rbtext" 12 NextPage", 1, "FIX")
   end
   self~addBlackFrame(0, self~SizeY - 20, self~SizeX, 1)

   /* */

::method CreateCategoryDialog protected
   use arg x, y, cx, cy, fontName, fontSize, options = "", expected = 200

   if expected > 1 then expected = 200
   options = options~translate

   if arg(8, "O") | fontName == "" then fontName = self~fontName
   if arg(9, "O") | fontSize = 0 then fontSize = self~fontSize

   ret = UsrDefineDialog(x, y, cx, cy, "", "", fontName, fontSize,,
                                                   options, expected)

   parse var ret base Ptr
   self~catalog['base'][self~catalog['category']] = base
   self~activePtr = Ptr
   return Ptr

   /* */

::method InitDialog protected
   /* set the mark to the first radio button */
   if self~catstyle~wordpos('DROPDOWN') > 0 then
   do
      do i = 1 to self~catalog['count']
         self~AddComboEntry(9001, self~catalog['names'][i])
      end
      self~SetComboLine(9001, self~catalog['names'][1])
   end
   else self~SendMessageToItem(9001,"0x000000F1",1,0)
   if self~Wizard = 1 then self~DisableItem(11)

   /* call InitXXX for each category if such a method exists */
   do i = 1 to self~catalog['count']
      self~catalog['category'] = i
      msg = "Init" || self~catalog['names'][i]~space(0)
      if self~HasMethod(msg) = 1 then interpret("self~"||msg)   /* does user class provide the Init method? If so call it */
   end

   /* activate first page */
   self~catalog['category'] = 1
   self~ShowWindow(self~catalog['handles'][self~catalog['category']])

   /* */

::method InitCategories protected
   return

   /* */

::method GetDefaultOpts protected
   use arg options
   if self~catalog['category'] = 0 then
   do
      if Arg(1,"o") = 1 | options = "OPTIONS" | options = .Nil then options = "FIX";
      else do
          if options~wordpos("FIX") = 0 then options = options || " FIX"
      end
   end
   else if Arg(1,"o") = 1 | options = "OPTIONS" | options = .Nil then options = ""
   return options~translate

   /* This method returs the currently selected page. Return value 1 means */
   /* the first page is selected. */

::method GetSelectedPage unguarded
   if self~catstyle~wordpos('DROPDOWN') = 0 then
   do
      np = 9001
      do while self~GetRadioButton(np) = 0 & np < (9001 + self~catalog['count'])
         np = np + 1
      end
   end
   else do
      cur = self~GetComboLine(9001)
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
   if Arg(2,'o') = 0 | NewPage > 9000 then NewPage = self~GetSelectedPage
   else do
      if self~catstyle~wordpos("DROPDOWN") = 0 then do
          /* SetRadioButton only works for registered radio buttons */
          /* uncheck the old radio button and check the new one */
          self~SendMessageToItem(9000+OldPage,"0x000000F1",0,0)
          self~SendMessageToItem(9000+NewPage,"0x000000F1",1,0)
      end
      else self~SetCurrentComboIndex(9001, NewPage)
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


::method PageHasChanged unguarded
   use arg Oldpage, NewPage
   return

::method PreviousPage unguarded
   oldPage = self~CurrentCategory
   if oldPage > 1 then self~ChangePage(oldPage-1)

::method NextPage unguarded
   oldPage = self~CurrentCategory
   if oldPage < self~catalog['count'] then self~ChangePage(oldPage+1)

  /* The following methods */

::method ConnectEntryLine
   use arg id, fname
   forward message "AddAttribute" continue
   if result = -1 then return -1              /* new id in result */
   return DataTable(self~Adm,"ADD",result,0, self~catalog['category']);

::method ConnectComboBox
   use arg id, fname, opts
   forward message "AddAttribute" continue
   id = result; if result = -1 then return -1              /* new id in result */
   if opts~translate~wordpos("LIST") = 0 then type = 0; else type = 5
   return DataTable(self~Adm,"ADD",id,type, self~catalog['category']);

::method ConnectCheckBox
   use arg id, fname
   forward message "AddAttribute" continue
   if result = -1 then return -1              /* new id in result */
   return DataTable(self~Adm,"ADD",result,1, self~catalog['category']);

::method ConnectRadioButton
   use arg id, fname
   forward message "AddAttribute" continue
   if result = -1 then return -1              /* new id in result */
   return DataTable(self~Adm,"ADD",result,2, self~catalog['category']);

::method ConnectListBox
   use arg id, fname
   forward message "AddAttribute" continue
   if result = -1 then return -1              /* new id in result */
   return DataTable(self~Adm,"ADD",result,3, self~catalog['category']);

::method ConnectMultiListBox
   use arg id, fname
   forward message "AddAttribute" continue
   if result = -1 then return -1              /* new id in result */
   return DataTable(self~Adm,"ADD",result,4, self~catalog['category']);

::method ConnectSeparator
   use arg id
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return DataTable(self~Adm,"ADD",id,9, self~catalog['category']);

::method BoxMessage unguarded private
   use arg category, id, msg, wP, lP
   if category~Datatype("N") = 0 then category = self~catalog['category']  /* assume current page is addressed */
   if category > 9000 then forward class (super)
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return SendWinMsg("PTR", self~catalog['handles'][category], id, msg, wP, lP)

::method AddCategoryComboEntry unguarded
   use arg id, data, category
   if Arg(2,"o") = 1 then return -1
   return self~BoxMessage(category, id, 0x00000143, 0, "T" || data)

::method InsertCategoryComboEntry unguarded
   use arg id, item, data, category
   if Arg(3,"o") = 1 then return -1
   if Arg(2,"o") = 1 then item = self~GetCurrentCategoryComboIndex(id, category)
   return self~BoxMessage(category, id, 0x0000014A, item-1, "T" || data)

/* don't use string but the index */
::method DeleteCategoryComboEntry unguarded
   use arg id, index, category
   if Arg(2,"o") = 1 then index = self~GetCurrentCategoryComboIndex(id, category)
   return self~BoxMessage(category, id, 0x00000144, index-1, 0)

::method FindCategoryComboEntry unguarded
   use arg id, data, category
   if Arg(2,'o') = 1 | data = "DATA" then return -1
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
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   len = SendWinMsg("DLG", self~catalog['handles'][category], id, 0x00000149, ndx-1,0)
   if len <= 0 then return ""
   else return SendWinMsg("PTR", self~catalog['handles'][category], id, 0x00000148, ndx-1,"G" || len+1)

::method ChangeCategoryComboEntry unguarded
   use arg id, item, data, category
   if Arg(2,"o") = 1 then item = self~GetCurrentCategoryComboIndex(id, category)
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
   if Arg(2,"o") = 1 then return -1
   return self~BoxMessage(category, id, 0x00000180, 0, "T" || data)

::method InsertCategoryListEntry unguarded
   use arg id, item, data, category
   if Arg(3,"o") = 1 then return -1
   if Arg(2,"o") = 1 then item = self~GetCurrentCategoryListIndex(id, category)
   return self~BoxMessage(category, id, 0x00000181, item-1, "T" || data)

/* don't use string but the index */
::method DeleteCategoryListEntry unguarded
   use arg id, index, category
   if Arg(2,"o") = 1 then index = self~GetCurrentCategoryListIndex(id, category)
   return self~BoxMessage(category, id, 0x00000182, index-1, 0)

::method FindCategoryListEntry unguarded
   use arg id, data, category
   if Arg(2,"o") = 1 then return -1
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
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   len = SendWinMsg("DLG", self~catalog['handles'][category], id, 0x0000018A, ndx-1,0)
   if len <= 0 then return ""
   else return SendWinMsg("PTR", self~catalog['handles'][category], id, 0x00000189, ndx-1,"G" || len+1)

::method ChangeCategoryListEntry unguarded
   use arg id, item, data, category
   if Arg(2,"o") = 1 then item = self~GetCurrentCategoryListIndex(id, category)
   if item <= 0 then return -1
   self~DeleteCategoryListEntry(id, item, category)
   return self~InsertCategoryListEntry(id, item, data, category)

::method SetCategoryListTabulators unguarded
   if arg() < 3 then return 1
   id = arg(1)
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   cs = self~catalog['handles'][arg(arg())]", "id
   do i=2 to arg()-1
      cs = cs", "arg(i)
   end
   interpret("call SetLBTabStops "cs)


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


::method GetCategoryEntryLine unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalGetItemData(id, self~catalog['handles'][category], 0)

::method GetCategoryListLine unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalGetItemData(id, self~catalog['handles'][category], 3)

::method GetCategoryComboLine unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalGetItemData(id, self~catalog['handles'][category], 0)

::method GetCategoryCheckBox unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalGetItemData(id, self~catalog['handles'][category], 1)

::method GetCategoryRadioButton unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalGetItemData(id, self~catalog['handles'][category], 2)

::method GetCategoryMultiList unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalGetItemData(id, self~catalog['handles'][category], 4)

::method GetCategoryValue unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalGetItemData(id, self~catalog['handles'][category])

::method GetCategoryAttrib unguarded
   use arg aname, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   i = self~DataConnection~First
   do while (i \= .NIL)
      if self~DataConnection[i] = aname then
      do
         s = "self~"aname"= self~GetValue("i", category)"
         interpret(s)
         return
      end
      i = self~DataConnection~Next(i)
   end


::method SetCategoryStaticText unguarded
   use arg id, data, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalSetItemData(id, data, self~catalog['handles'][category], 0)

::method SetCategoryEntryLine unguarded
   use arg id, data, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalSetItemData(id, data, self~catalog['handles'][category], 0)

::method SetCategoryListLine unguarded
   use arg id, data, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalSetItemData(id, data, self~catalog['handles'][category], 3)

::method SetCategoryComboLine unguarded
   use arg id, data, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalSetItemData(id, data, self~catalog['handles'][category], 0)

::method SetCategoryCheckBox unguarded
   use arg id, data, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalSetItemData(id, data, self~catalog['handles'][category], 1)

::method SetCategoryRadioButton unguarded
   use arg id, data, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalSetItemData(id, data, self~catalog['handles'][category], 2)

::method SetCategoryMultiList unguarded
   use arg id, data, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalSetItemData(id, data, self~catalog['handles'][category], 4)

::method SetCategoryValue unguarded
   use arg id, data, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return self~InternalSetItemData(id, data, self~catalog['handles'][category])

::method SetCategoryAttrib unguarded
   use arg aname, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   i = self~DataConnection~First
   do while (i \= .NIL)
      if self~DataConnection[i] = aname then
      do
         s = "self~SetCategoryValue("i", self~"aname", category)"
         interpret(s)
         return
      end
      i = self~DataConnection~Next(i)
   end


::method EnableCategoryItem unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   h = self~GetItem(id, self~catalog['handles'][category])
   if h = 0 then return 1
   return Wnd_Desktop("ENABLE", h, 1)

::method DisableCategoryItem unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   h = self~GetItem(id, self~catalog['handles'][category])
   if h = 0 then return 1
   return Wnd_Desktop("ENABLE",h, 0)

::method HideCategoryItem unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   h = self~GetItem(id, self~catalog['handles'][category])
   if h = 0 then return 1
   return WndShow_Pos("S",h, "HIDE")

::method ShowCategoryItem unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   h = self~GetItem(id, self~catalog['handles'][category])
   if h = 0 then return 1
   return WndShow_Pos("S",h, "NORMAL")

::method GetItem unguarded
   use arg id, hDlg
   if Arg(2,'o') = 1 then hDlg = self~DlgHandle
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return 0
   hnd = HandleDlg("ITEM", id, hDlg)
   if hnd = 0 & Arg(2,'o') = 1 then do
       hnd = HandleDlg("ITEM", id, self~catalog['handles'][self~catalog['category']])  /* try current page */
       if hnd = 0 then do i = 1 to self~catalog['count'] while hnd = 0   /* try all pages */
            hnd = HandleDlg("ITEM", id, self~catalog['handles'][i])
       end
   end
   return hnd

::method SendMessageToCategoryItem unguarded
   parse arg id,a2,a3,a4, category
   if Arg(5,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   return SendWinMsg("DLG", self~catalog['handles'][category],id,a2,a3,a4)

::method SetCategoryItemFont unguarded
   use arg id, hFont, bRedraw, category     /* WM_SETFONT */
   if Arg(4,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   if Arg(3, 'o') = 1 then bRedraw = 1
   return SendWinMsg("ANY",self~GetItem(id, self~catalog['handles'][category]), "0x0030", hFont, bRedraw)

::method FocusCategoryItem unguarded
   use arg id, category
   if Arg(2,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   return SendWinMsg("ANY",self~catalog['handles'][category], "0x0028", self~GetItem(id, self~catalog['handles'][category]), 1)

::method ResizeCategoryItem unguarded
   use arg id, width, high, showOptions, category
   if Arg(5,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   if Arg(4,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   hnd = self~GetItem(id, self~catalog['handles'][category])
   if hnd = 0 then return -1
   return WndShow_Pos("P",hnd, 0, 0, width * self~FactorX, high* self~FactorY,,
                           "NOMOVE "||showOptions)

   /* This method will move a dialog item to an other position */

::method MoveCategoryItem unguarded
   use arg id, xPos, yPos, showOptions, category
   if Arg(5,'o') = 1 | category~DataType("N") = 0 then category = self~catalog['category']  /* try current */
   if Arg(4,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   h = self~GetItem(id, self~catalog['handles'][category])
   if h = 0 then return -1
   return WndShow_Pos("P",h, xPos * self~FactorX, yPos * self~FactorY, 0, 0,,
                           "NOSIZE "||showOptions)


/* set/get the horizontal scroll width of a list box */

::method SetCategoryListWidth unguarded
   use arg id, dlgunits, category
   if Arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(category, id, 0x00000194, dlgunits*self~FactorX, 0)

::method GetCategoryListWidth unguarded
   use arg id, category
   return self~BoxMessage(category, id, 0x00000193, 0, 0)/self~FactorX

::method SetCategoryListColumnWidth unguarded
   use arg id, dlgunits, category
   if Arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(category, id, 0x00000195, dlgunits*self~FactorX, 0)

::method SetCategoryListItemHeight unguarded
   use arg id, dlgunits, category
   if Arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(category, id, 0x000001A0, 0, dlgunits*self~FactorY)

::method GetCategoryListItemHeight unguarded
   use arg id, category
   return self~BoxMessage(category, id, 0x000001A1, 0, 0)/self~FactorY


/* These methods are necessary for Win32 controls when used in a cetegory dialog */
::method ConnectTreeControl
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1                /* current category */
   return DataTable(self~Adm,"ADD", result,6, self~catalog['category']);         /* new id in result  6 == Tree Type */

::method ConnectListControl
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1                /* current category */
   return DataTable(self~Adm,"ADD",result,7, self~catalog['category']);         /* new id in result  7 == List Type */

::method ConnectSliderControl
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1                /* current category */
   return DataTable(self~Adm,"ADD",result,8, self~catalog['category']);         /* new id in result  8 == Track Bar Type */

::method ConnectTabControl
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1                /* current category */
   return DataTable(self~Adm,"ADD",result,9, self~catalog['category']);         /* new id in result  9 == Tab Type */
