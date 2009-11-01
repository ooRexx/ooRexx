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
 * Dialog Control Classes.
 */


-- Base class for all dialog controls
::class 'DialogControl' public inherit WindowBase WindowExtensions

::method new class external "LIBRARY oodialog dlgctrl_new_cls"

::method init external "LIBRARY oodialog dlgctrl_init"

::attribute id get    -- The numerical resource ID for this control
::attribute oDlg get  -- The ooRexx dialog object this control belongs to
::attribute hDlg get  -- The window handle of that dialog

::method getTextSizeDlg unguarded external "LIBRARY oodialog dlgctrl_getTextSizeDlg"
::method unInit external "LIBRARY oodialog dlgctrl_unInit"

::method message2Parent unguarded private /* This is the replacement for BoxMessage */
   use arg msg, wP, lP
   return SendWinMsg("PTR", self~hDlg, self~Id, msg, wP, lP)

::method setColor
   use arg colorbk, colorfg
   if arg(2,'o') = 1 then
       return SetBackground(self~oDlg~adm, "COL", self~id, colorbk)  /* creates an entry in the dialogs color table */
   else
       return SetBackground(self~oDlg~adm, "COL", self~id, colorbk, colorfg)  /* set both, background and foreground color */

::method setSysColor
   use arg colorbk, colorfg
   if arg(2,'o') = 1 then
       return SetBackground(self~oDlg~adm, "COL", self~id, colorbk, "", "SYS")  /* creates an entry in the dialogs color table */
   else
       return SetBackground(self~oDlg~adm, "COL", self~id, colorbk, colorfg, "SYS")  /* set both, background and foreground color */

::method getFocus unguarded
   forward to (self~oDlg)

::method setFocus unguarded
   forward to (self~oDlg)

::method tabToNext unguarded
   forward to (self~oDlg)

::method tabToPrevious unguarded
   forward to (self~oDlg)

::method assignFocus external "LIBRARY oodialog dlgctrl_assignFocus"
::method tabStop unguarded external "LIBRARY oodialog dlgctrl_tabGroup"
::method group unguarded external "LIBRARY oodialog dlgctrl_tabGroup"
::method redrawRect unguarded external "LIBRARY oodialog dlgctrl_redrawRect"
::method clearRect unguarded external "LIBRARY oodialog dlgctrl_clearRect"

::method value unguarded  -- TODO this method won't work if this control is on a category page.
                          -- Implement in native code
    --return self~oDlg~getControlData(self~id, self~hDlg)
    return self~oDlg~getControlData(self~id)

::method "value=" unguarded  -- TODO same as a above
    use strict arg data
    --self~oDlg~setControlData(self~Id, data, self~hDlg)
    self~oDlg~setControlData(self~Id, data)


::method getMouseCapture
   forward to (self~oDlg)

::method releaseMouseCapture
   forward to (self~oDlg)

::method isMouseButtonDown
   forward to (self~oDlg)

::method captureMouse external "LIBRARY oodialog dlgctrl_captureMouse"
::method connectFKeyPress unguarded external "LIBRARY oodialog dlgctrl_connectFKeyPress"
::method connectKeyPress unguarded external "LIBRARY oodialog dlgctrl_connectKeyPress"
::method disconnectKeyPress unguarded external "LIBRARY oodialog dlgctrl_disconnectKeyPress"
::method hasKeyPressConnection unguarded external "LIBRARY oodialog dlgctrl_hasKeyPressConnection"

-- DEPRECATED
::method processMessage
  forward message "sendMessage"

-- DEPRECATED
::method assignWindow
  return 0

-- DEPRECATED
::method getTextSize unguarded
  forward message "getTextSizeDlg" continue
  return result~width result~height


/****************************************************** Tree Control Class *********************************************************/


::class 'TreeControl' subclass DialogControl public

::method RootArray attribute private

::method init
   forward class (super) continue
   self~RootArray = .array~new(4)

::method Insert
   use arg parent, after, text, image, selImage, opts, children
   if arg(1,'o') = 1 then parent = "ROOT"
   if arg(2,'o') = 1 then after = "LAST"
   if arg(4,'o') = 1 then image = -1
   if arg(5,'o') = 1 then selImage = image
   if arg(6,'o') = 1 then opts = ""
   if arg(7,'o') = 1 then children = 0
   return HandleTreeCtrl("INS", self~Hwnd, parent, after, text, image, opts~translate, children, selImage)

::method Add
   do i = 1 to arg()
      if arg(i,'E') = 1 then do
          if arg(i+1,'e') = 1 then image = arg(i+1); else image = -1
          if arg(i+2,'e') = 1 then selImage = arg(i+2); else selImage = image
          if arg(i+3,'e') = 1 then opts = arg(i+3); else opts = ""
          if arg(i+4,'e') = 1 then children = arg(i+4); else children = 0  /* for dynamic children */
          if i=1 then do
              self~RootArray[i] = self~Insert("ROOT",,arg(i),image,selimage,opts~translate, children)
              return self~RootArray[i]
          end
          else if self~RootArray~HasIndex(i-1)=1 then do
              if i=1 then parent = "ROOT"; else parent = self~RootArray[i-1]
              self~RootArray[i] = self~Insert(parent,,arg(i),image,selimage,opts~translate, children)
              return self~RootArray[i]
          end
          else return 0
      end
   end


::method Modify
   use arg hItem, text = "", image = (-1), selImage = (-1), state = "", children = (-1)
   if arg(1, 'O') then hItem = self~Selected
   if hitem = 0 then return -1

   return HandleTreeCtrl("SET", self~Hwnd, hItem, text, image, state~translate, children, selImage)

::method ItemInfo
   use arg hItem
   if arg(1, 'O') | hItem = 0 then return -1
   ret = HandleTreeCtrl("GET", self~Hwnd, hItem, "InternalTVItemInfo")
   return InternalTVItemInfo.

::method MoveItem
   use arg hItem, hNewParent, redraw = 1, extended = ""
   if arg(1, 'O') | hItem = 0 then return 0
   if arg(2, 'O') | hNewParent = 0 then return 0

   extended = extended~translate
   if hItem = hNewParent | hNewParent = self~Parent(hItem) | self~IsAncestor(hItem, hNewParent) = 1 then return 0
   iinfo. = self~ItemInfo(hItem)
   newRoot = self~Insert(hNewParent,,iinfo.!Text, iinfo.!Image, iinfo.!SelectedImage, iinfo.!State, iinfo.!Children)
   if iinfo.!Children = 1 then do
       child = self~Child(hItem)
       if child \= 0 then self~MoveItem(child, newRoot, 0, "SIBLINGS")
   end
   if extended = "SIBLINGS" then do
       sibling = self~next(hItem)
       do while sibling \= 0
           self~MoveItem(sibling, hNewParent, 0, "NODELETE")
           oldsib = sibling
           sibling = self~next(sibling)
           self~Delete(oldsib)
       end
   end
   if extended \= "NODELETE" then self~Delete(hItem)
   if redraw = 1 | redraw~left(1)~translate = "Y" then self~Update
   return newRoot  /* return new handle */


::method IsAncestor
   use arg hParent, hItem
   if arg(1, 'O') | hParent = 0 then return 0
   if arg(2, 'O') | hItem = 0 then return 0
   if hItem = hParent then return 0
   act = self~Parent(hItem)
   do while act \= 0 & act \= hParent
       act = self~Parent(act)
   end
   if act = hParent then return 1
   else return 0


::method Items
   return HandleTreeCtrl("CNT", self~Hwnd)

::method VisibleItems
   return HandleTreeCtrl("CNTVIS", self~Hwnd)

::method Root
   return HandleTreeCtrl("GETHND", self~Hwnd,0,"ROOT")

::method Parent
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("GETHND", self~Hwnd, hitem, "PARENT")

::method Child
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("GETHND", self~Hwnd, hitem, "CHILD")

::method Selected
   return HandleTreeCtrl("GETHND", self~Hwnd, 0, "CARET")

::method DropHighlighted
   return HandleTreeCtrl("GETHND", self~Hwnd, 0, "DROP")

::method FirstVisible
   return HandleTreeCtrl("GETHND", self~Hwnd, 0, "FIRSTVISIBLE")

::method Next
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("GETHND", self~Hwnd, hitem, "NEXT")

::method NextVisible
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("GETHND", self~Hwnd, hitem, "NEXTVISIBLE")

::method Previous
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("GETHND", self~Hwnd, hitem, "PREVIOUS")

::method PreviousVisible
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("GETHND", self~Hwnd, hitem, "PREVIOUSVISIBLE")

::method Delete
   use arg hItem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("DEL",self~hwnd, hItem)

::method DeleteAll
   return HandleTreeCtrl("DEL", self~Hwnd,"ROOT")

::method Collapse
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("EXPAND", self~Hwnd, hitem, "COLLAPSE")

::method CollapseAndReset
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("EXPAND", self~Hwnd, hitem, "COLLAPSE RESET")

::method Expand
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("EXPAND", self~Hwnd, hitem, "EXPAND")

::method Toggle
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("EXPAND", self~Hwnd, hitem, "TOGGLE")

::method EnsureVisible
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("ENVIS", self~Hwnd, hitem)

::method Indent
      return HandleTreeCtrl("GETIND", self~Hwnd)

::method "Indent="
   use arg ind
   if arg(1, 'O') then return -1
   ret = HandleTreeCtrl("SETIND", self~Hwnd, ind)

::method Edit
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("EDIT", self~Hwnd, hitem)

::method EndEdit
   use arg cancel
   if arg(1, 'O') then cancel = 0
   return HandleTreeCtrl("EEDIT", self~Hwnd, cancel)

::method SubclassEdit
   return HandleTreeCtrl("SUBCL_EDIT", self~Hwnd)

::method RestoreEditClass
   return HandleTreeCtrl("RESUB_EDIT", self~Hwnd)

::method Select
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("SEL", self~Hwnd, hitem, "")

::method MakeFirstVisible
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("SEL", self~Hwnd, hitem, "FIRSTVIS")

::method DropHighlight
   use arg hitem
   if arg(1, 'O') then do
      if digits() == 9 then
         hItem = "0x00000000"
      else
         hItem = "0x0000000000000000"
   end
   return HandleTreeCtrl("SEL", self~Hwnd, hitem, "DROP")

::method SortChildren
   use arg hitem
   if arg(1, 'O') | hItem = 0 then return -1
   return HandleTreeCtrl("SORT", self~Hwnd, hitem, 0) /* recursive not yet supported */


-- DEPRECATED
::method setImages external "LIBRARY oodialog tv_setImageList"

-- DEPRECATED
::method removeImages
   return self~setImageList(.nil, 0)

::method setImageList external "LIBRARY oodialog tv_setImageList"
::method getImageList external "LIBRARY oodialog tv_getImageList"

::method HitTest
   use arg x, y
   return HandleTreeCtrl("HIT", self~Hwnd, x, y)


/************ List Control Class **********************************************/

::class 'ListControl' subclass DialogControl public

-- LVM_FIRST = 0x1000  == 4096
::constant LVM_GETCOLUMNWIDTH                4125             -- LVM_FIRST + 29


::method lastItem attribute

::method Init
   forward class (super) continue
   self~LastItem = -1

::method ReplaceStyle
   use arg oldstyle, newstyle
   if arg(1,'O') | arg(2,'O') then return 0
   ret = HandleListCtrl("M","SETSTYLE", self~Hwnd, "R", oldstyle)
   return HandleListCtrl("M","SETSTYLE", self~Hwnd, "A", newstyle)

::method AddStyle
   use arg style
   if arg(1,'O') then return 0
   return HandleListCtrl("M","SETSTYLE", self~Hwnd, "A", style)

::method RemoveStyle
   use arg style
   if arg(1,'O') then return 0
   return HandleListCtrl("M","SETSTYLE", self~Hwnd, "R", style)

::method ReplaceExtendedStyle  -- Replace
   use strict arg oldstyle, newstyle
   return HandleListCtrlEx(self~Hwnd, "M", "STYLE", "R", oldstyle, newstyle)

::method AddExtendedStyle      -- Set
   use strict arg style
   return HandleListCtrlEx(self~Hwnd, "M", "STYLE", "S", style)

::method RemoveExtendedStyle   -- Clear
   use strict arg style
   return HandleListCtrlEx(self~Hwnd, "M", "STYLE", "C", style)

::method GetExtendedStyle      -- Get
   return HandleListCtrlEx(self~Hwnd, "M", "STYLE", "G")

::method GetExtendedStyleRaw   -- Long
   return HandleListCtrlEx(self~Hwnd, "M", "STYLE", "L")

::method SetHoverTime
   use arg time
   if arg(1, 'O') then time = -1
   if \ time~datatype('W') then raise syntax 93.905 array(1, time)
   if time < -1 then time = -1
   return HandleListCtrlEx(self~Hwnd, "M", "HOVER", time)

::method GetHoverTime
   return HandleListCtrlEx(self~Hwnd, "M", "HOVER")

::method Check
   use strict arg item
   if \ item~datatype('W') then raise syntax 93.905 array(1, item)
   return HandleListCtrlEx(self~Hwnd, "M", "CHK", item, .true)

::method CheckAll
   return HandleListCtrlEx(self~Hwnd, "M", "CHK", -1, .true)

::method Uncheck
   use strict arg item
   if \ item~datatype('W') then raise syntax 93.905 array(1, item)
   return HandleListCtrlEx(self~Hwnd, "M", "CHK", item, .false)

::method UncheckAll
   return HandleListCtrlEx(self~Hwnd, "M", "CHK", -1, .false)

::method GetCheck
   use strict arg item
   if \ item~datatype('W') then raise syntax 93.905 array(1, item)
   return HandleListCtrlEx(self~Hwnd, "M", "CHK", item)

::method IsChecked
   use strict arg item
   if \ item~datatype('W') then raise syntax 93.905 array(1, item)
   return (HandleListCtrlEx(self~Hwnd, "M", "CHK", item) == 1)


::method DeleteColumn
   use arg nr
   if arg(1,'o') = 1 then return -1
   return HandleListCtrl("C","DEL", self~Hwnd, nr)


::method ModifyColumn
   use arg nr, text, width, fmt
   if arg(1,'o') = 1 then return -1
   if arg(2,'o') = 1 then text = ""
   if arg(3,'o') = 1 then width = -1; else width = width*self~factorX
   if arg(4,'o') = 1 then fmt = ""
   return HandleListCtrl("C","SET", self~Hwnd, nr, text, width, fmt~translate)


::method ColumnInfo
   use arg nr
   if arg(1,'o') = 1 then return -1
   ret = HandleListCtrl("C","GET", self~Hwnd, nr, "InternalLVColInfo")
   if InternalLVColInfo.!Width~Datatype('N') = 1 then InternalLVColInfo.!Width = InternalLVColInfo.!Width / self~factorX
   return InternalLVColInfo.


::method stringWidthPx external "LIBRARY oodialog lv_stringWidthPx"
::method stringWidth  -- Dialog units, not accurate.
   use strict arg text
   return self~stringWidthPx(text) / self~factorX

::method columnWidthPx
  use strict arg column
  return self~sendWinIntMsg(self~LVM_GETCOLUMNWIDTH, column, 0)

::method columnWidth  -- Dialog units, not accurate.
   use strict arg colNum
   return self~columnWidthPx(colNum) / self~factorX

::method insertColumnPx external "LIBRARY oodialog lv_insertColumnPx"
::method insertColumn -- Dialog units, not accurate.
   use arg nr = 0, text, width, fmt = "L"
   width = trunc(width * self~factorX)
   return self~insertColumnPx(nr, text, width, fmt)

::method SetColumnWidth
   use arg nr, width
   if arg(1,'o') = 1 then return -1
   if arg(2,'o') = 1 then width = "AUTO"
   if width~datatype("N") = 1 then width = width * self~factorX
   return HandleListCtrl("C","SETWIDTH", self~Hwnd, nr, width~translate)

::method getColumnCount external "LIBRARY oodialog lv_getColumnCount"
::method getColumnOrder external "LIBRARY oodialog lv_getColumnOrder"
::method setColumnOrder external "LIBRARY oodialog lv_setColumnOrder"


::method Insert
   use arg nr, subnr, text, image
   if arg(2,'O') then subnr = 0
   if arg(1,'o') = 1 then do
        if subnr = 0 then nr = self~LastItem + 1
        else nr = self~LastItem
   end
   if arg(4,'o') = 1 then image = -1
   if subnr = 0 then do
       item = HandleListCtrl("I","INS", self~Hwnd, nr, text, image)
       if item \= -1 then self~LastItem = item
   end
   else item = HandleListCtrl("I","SET", self~Hwnd, nr, subnr, text, image)
   return item


::method Modify
   use arg nr, subnr, text, image
   if arg(1,'o') = 1 then nr = self~Selected
   if arg(2,'O') then subnr = 0
   if arg(4,'o') = 1 then image = -1
   return HandleListCtrl("I","SET", self~Hwnd, nr, subnr, text, image)

::method SetItemText
   use arg item, subitem, text
   if arg(1,'o') = 1 then return -1
   if arg(2,'O') then subitem = 0
   return HandleListCtrl("I","SET", self~Hwnd, item, subitem, text,"TXT")

::method SetItemState
   use arg item, state
   if arg(1,'o') = 1 then return -1
   return HandleListCtrl("I","SET", self~Hwnd, item, 0, state, "STATE")


::method Add
   do i = 1 to arg()
      if arg(i,'E') = 1 then do
          if arg(i+1,'e') = 1 then image = arg(i+1); else image = -1
          if i = 1 then do
              item = self~Insert(self~LastItem+1,i-1,arg(i),image)
              if item \= -1 then self~LastItem = item
          end
          else do
             item = self~Insert(self~LastItem,i-1,arg(i),image)
          end
          return item
      end
   end
   return -1


::method AddRow
   use arg nr, image, text
   if arg(1,'o') = 1 then nr = self~LastItem+1
   if arg(2,'o') = 1 then image = -1
   if arg(3,'o') = 1 then text = ""
   item = self~Insert(nr,0,text,image)
   if item \= -1 then do
       self~LastItem = item
       do i = 4 to arg()
           if arg(i,'E') = 1 then self~Insert(item,i-3,arg(i),-1)
       end
   end
   return item

::method addRowEx external "LIBRARY oodialog lv_addRowEx"  -- TODO review method name

::method Delete
   use arg item
   if arg(1,'o') = 1 then return -1
   return HandleListCtrl("I","DEL",self~hwnd, item)

::method DeleteAll
   return HandleListCtrl("I","DEL", self~Hwnd,"ALL")

::method Items
   return HandleListCtrl("M","CNT", self~Hwnd)

::method Last
   return self~Items - 1

::method Prepare4nItems
   use arg citems
   if arg(1,'o') = 1 then return -1
   return HandleListCtrl("M","SETCNT", self~Hwnd, citems)


::method SelectedItems
   return HandleListCtrl("M","CNTSEL", self~Hwnd)

::method ItemInfo
   use arg item, subitem
   if arg(1,'o') = 1 then return -1
   if arg(2,'O') then subitem = 0
   ret = HandleListCtrl("I","GET", self~Hwnd, item, subitem, "InternalLVItemInfo", "")
   return InternalLVItemInfo.

::method ItemText
   use arg item, subitem
   if arg(1,'o') = 1 then return -1
   if arg(2,'O') then subitem = 0
   return HandleListCtrl("I","GET", self~Hwnd, item, subitem, "", "TXT")

::method ItemState
   use arg item
   if arg(1,'o') = 1 then return -1
   return HandleListCtrl("I","GET", self~Hwnd, item, subitem, "", "STATE")

::method Select
   use arg item
   if arg(1,'o') = 1 then return -1
   return HandleListCtrl("I","SET", self~Hwnd, item, 0, "SELECTED", "STATE")

::method Deselect
   use arg item
   if arg(1,'o') = 1 then return -1
   return HandleListCtrl("I","SET", self~Hwnd, item, 0, "NOTSELECTED", "STATE")

::method Selected
   return HandleListCtrl("I", "GETNEXT", self~Hwnd, -1, "SELECTED")

::method LastSelected
   if self~ItemState(self~Last)~wordpos("SELECTED") > 0 then  return self~Last
   return self~PreviousSelected(self~Last)

::method Focused
   return HandleListCtrl("I", "GETNEXT", self~Hwnd, -1, "FOCUSED")

::method Focus
   use arg item
   if arg(1,'O') then return -1
   return HandleListCtrl("I", "SET", self~Hwnd, item, 0, "FOCUSED","STATE")

::method DropHighlighted
   return HandleListCtrl("I", "GETNEXT", self~Hwnd, -1, "DROP")

::method FirstVisible
   return HandleListCtrl("I", "GETNEXT", self~Hwnd, 0, "FIRSTVISIBLE")

::method NextSelected
   use arg startItem
   if arg(1,'O') then startItem = -1
   return HandleListCtrl("I", "GETNEXT", self~Hwnd, startItem, "BELOW TORIGHT SELECTED")

::method PreviousSelected
   use arg startItem
   if arg(1,'O') then startItem = -1
   return HandleListCtrl("I", "GETNEXT", self~Hwnd, startItem, "ABOVE TOLEFT SELECTED")

::method Next
   use arg startItem
   -- The Windows API appears to have a bug when the list contains a single item, insisting on returning
   -- 0.  This, rather unfortunately, can cause some infinite loops because iterating code is looking for
   -- a -1 value to mark the iteration end.
   if self~Items < 2 then return -1
   if arg(1,'O') then startItem = -1
   return HandleListCtrl("I","GETNEXT", self~Hwnd, startItem, "BELOW TORIGHT")

::method Previous
   use arg startItem
   -- The Windows API appears to have a bug when the list contains a single item, insisting on returning
   -- 0.  This, rather unfortunately, can cause some infinite loops because iterating code is looking for
   -- a -1 value to mark the iteration end.
   if self~Items < 2 then return -1
   if arg(1,'o') = 1 then startItem = -1
   return HandleListCtrl("I","GETNEXT", self~Hwnd, startItem, "ABOVE TOLEFT")

::method NextLeft
   use arg startItem
   -- The Windows API appears to have a bug when the list contains a single item, insisting on returning
   -- 0.  This, rather unfortunately, can cause some infinite loops because iterating code is looking for
   -- a -1 value to mark the iteration end.
   if self~Items < 2 then return -1
   if arg(1,'o') = 1 then startItem = -1
   return HandleListCtrl("I","GETNEXT", self~Hwnd, startItem, "TOLEFT")

::method NextRight
   use arg startItem
   -- The Windows API appears to have a bug when the list contains a single item, insisting on returning
   -- 0.  This, rather unfortunately, can cause some infinite loops because iterating code is looking for
   -- a -1 value to mark the iteration end.
   if self~Items < 2 then return -1
   if arg(1,'o') = 1 then startItem = -1
   return HandleListCtrl("I","GETNEXT", self~Hwnd, startItem, "TORIGHT")

::method SmallSpacing
   return HandleListCtrl("M","GETSPC", self~Hwnd, 1)

::method Spacing
   return HandleListCtrl("M","GETSPC", self~Hwnd, 0)

::method RedrawItems
   use arg first, last
   if arg(1,'o') = 1 then first = 0
   if arg(2,'o') = 1 then last = self~items
   return HandleListCtrl("M","REDRAW", self~Hwnd, first, last)

::method UpdateItem
   use arg item
   if arg(1,'o') = 1 then return -1
   return HandleListCtrl("M","UPDATE", self~Hwnd, item)

::method EnsureVisible
   use arg item, partial
   if arg(1,'o') = 1 then return -1
   if arg(2,'o') = 1 then partial = 0
   return HandleListCtrl("M", "ENVIS", self~Hwnd, item, partial)

-- DEPRECATED
::method setSmallImages
   newArgs = arg(1, 'A')
   newArgs[4] = 1
   forward message 'setImageList' arguments (newArgs)

-- DEPRECATED
::method setImages external "LIBRARY oodialog lv_setImageList"

-- DEPRECATED
::method RemoveSmallImages
   return self~removeImageList(.nil, 1)

-- DEPRECATED
::method removeImages
   return self~removeImageList(.nil, 0)

::method setImageList external "LIBRARY oodialog lv_setImageList"
::method getImageList external "LIBRARY oodialog lv_getImageList"

::method Find
   use arg txt, startItem, wrap
   if arg(2,'o') = 1 then startItem = -1
   if arg(3,'o') = 1 then wrap = 0
   if wrap = 1 | wrap~translate = "J" then opt = "WRAP"; else opt = ""
   return HandleListCtrl("I", "FIND", self~Hwnd, startItem, opt, txt)

::method FindPartial
   use arg txt, startItem, wrap
   if arg(2,'o') = 1 then startItem = -1
   if arg(3,'o') = 1 then wrap = 0
   if wrap = 1 | wrap~translate = "J" then opt = "PARTIAL WRAP"; else opt = "PARTIAL"
   return HandleListCtrl("I", "FIND", self~Hwnd, startItem, opt, txt)

::method FindNearestXY
   use arg x,y, direction
   if arg(1,'o') = 1 | arg(2,'o') = 1 then return -1
   if arg(3,'o') = 1 then direction = "DOWN"
   return HandleListCtrl("I", "FIND", self~Hwnd, -1, "NEAREST", x, y, direction~translate)

::method Arrange
   return HandleListCtrl("M", "ARRANGE", self~Hwnd, "DEFAULT")

::method SnapToGrid
   return HandleListCtrl("M", "ARRANGE", self~Hwnd, "SNAPTOGRID")

::method AlignLeft
   return HandleListCtrl("M", "ARRANGE", self~Hwnd, "LEFT")

::method AlignTop
   return HandleListCtrl("M", "ARRANGE", self~Hwnd, "TOP")

::method ItemPos
   use arg item
   if arg(1,'o') = 1 then return -1
   return HandleListCtrl("M", "GETPOS", self~Hwnd, item)

::method SetItemPos
   use arg item, x, y
   if arg(1,'o') = 1 then return -1
   if arg(2,'o') = 1 then x = 0
   if arg(3,'o') = 1 then y = 0
   return HandleListCtrl("M", "SETPOS", self~Hwnd, item, x, y)

::method Edit
   use arg hitem
   if arg(1,'o') = 1 then return -1
   return HandleListCtrl("I", "EDIT", self~Hwnd, hitem)

::method EndEdit
   return HandleListCtrl("I", "EDIT", self~Hwnd, -1)

::method SubclassEdit
   return HandleListCtrl("I", "SUBCL_EDIT", self~Hwnd)

::method RestoreEditClass
   return HandleListCtrl("I", "RESUB_EDIT", self~Hwnd)

::method ItemsPerPage
   return HandleListCtrl("M", "CNTPP", self~Hwnd)

::method Scroll
   use arg x, y
   if arg(1,'o') = 1 then x = 0
   if arg(2,'o') = 1 then y = 0
   return HandleListCtrl("M", "SCROLL", self~Hwnd, x, y)

::method BkColor
   return HandleListCtrl("M", "COLOR", self~Hwnd, "GETBK")

::method "BkColor="
   use arg color
   call HandleListCtrl "M", "COLOR", self~Hwnd, "SETBK", color

::method TextColor
   return HandleListCtrl("M", "COLOR", self~Hwnd, "GETTXT")

::method "TextColor="
   use arg color
   call HandleListCtrl "M", "COLOR", self~Hwnd, "SETTXT", color

::method TextBkColor
   return HandleListCtrl("M", "COLOR", self~Hwnd, "GETTXTBK")

::method "TextBkColor="
   use arg color
   call HandleListCtrl "M", "COLOR", self~Hwnd, "SETTXTBK", color




/***************** Progress Bar Class *********************************************************/

::class 'ProgressBar' subclass DialogControl public

::method step external "LIBRARY oodialog pbc_stepIt"
::method setStep external "LIBRARY oodialog pbc_setStep"

::method setPos external "LIBRARY oodialog pbc_setPos"
::method getPos external "LIBRARY oodialog pbc_getPos"

::method setRange external "LIBRARY oodialog pbc_setRange"
::method getRange external "LIBRARY oodialog pbc_getRange"

::method setMarquee external "LIBRARY oodialog pbc_setMarquee"

::method barColor external "LIBRARY oodialog pbc_setBarColor"
::method backgroundColor external "LIBRARY oodialog pbc_setBkColor"


/******************** Track Bar Class *********************************************************/

::class 'SliderControl' subclass DialogControl public

::method "pos="
   forward message (setPos)

::method pos
   return HandleOtherNewCtrls("SLIDER", "POS", self~hwnd, "G")

::method setPos
   use strict arg p, redraw = .false
   call HandleOtherNewCtrls "SLIDER", "POS", self~hwnd, p, redraw

::method initRange
   use strict arg min = 0, max = 100, redraw = .false
   if max < min then return -1
   return HandleOtherNewCtrls("SLIDER", "SETRANGE", self~hwnd, min, max, redraw)

::method setMin
   use strict arg min, redraw = .true
   return HandleOtherNewCtrls("SLIDER", "SETRANGE", self~hwnd, "L", min, redraw)

::method setMax
   use strict arg max, redraw = .true
   return HandleOtherNewCtrls("SLIDER", "SETRANGE", self~hwnd, "H", max, redraw)

::method range
   return HandleOtherNewCtrls("SLIDER", "GETRANGE", self~hwnd)

::method clearTicks
   use strict arg redraw = .true
   return HandleOtherNewCtrls("SLIDER", "TICS", self~hwnd, "C", redraw)

::method countTicks
   return HandleOtherNewCtrls("SLIDER", "TICS", self~hwnd, "N")

::method getTick
   use strict arg tic
   return HandleOtherNewCtrls("SLIDER", "TICS", self~hwnd, "G", tic)

::method setTickAt
   use strict arg pos
   return HandleOtherNewCtrls("SLIDER", "TICS", self~hwnd, "S", pos)

::method setTickFrequency
   use strict arg freq
   return HandleOtherNewCtrls("SLIDER", "TICS", self~hwnd, "F", freq)

::method getLineStep
   return HandleOtherNewCtrls("SLIDER", "GETSTEPS", self~hwnd, "L")

::method getPageStep
   return HandleOtherNewCtrls("SLIDER", "GETSTEPS", self~hwnd, "P")

::method setLineStep
   use strict arg step
   return HandleOtherNewCtrls("SLIDER", "SETSTEPS", self~hwnd, "L", step)

::method setPageStep
   use strict arg step
   return HandleOtherNewCtrls("SLIDER", "SETSTEPS", self~hwnd, "P", step)

::method initSelRange
   use arg min = 0, max, redraw = .false
   if arg(2,'o') then parse value self~Range with . max
   if max < min then return -1
   return HandleOtherNewCtrls("SLIDER", "SETSEL", self~hwnd, min, max, redraw)

::method setSelStart
   use strict arg min, redraw = .true
   return HandleOtherNewCtrls("SLIDER", "SETSEL", self~hwnd, "S", min, redraw)

::method setSelEnd
   use strict arg max, redraw = .true
   return HandleOtherNewCtrls("SLIDER", "SETSEL", self~hwnd, "E", max, redraw)

::method clearSelRange
   use strict arg redraw = .true
   return HandleOtherNewCtrls("SLIDER", "SETSEL", self~hwnd, "C", redraw)

::method selRange
   return HandleOtherNewCtrls("SLIDER", "GETSEL", self~hwnd)


/******************* Tab Control Class ****************************************/

::class 'TabControl' subclass DialogControl public

::method LastItem attribute

::method Init
   forward class (super) continue
   self~LastItem = -1

::method Insert
   use arg nr, text, image, lparam
   if arg(1,'o') = 1 then nr = self~LastItem + 1
   if arg(2,'o') = 1 then text = ""
   if arg(3,'o') = 1 then image = -1
   if arg(4,'o') = 1 then item = HandleOtherNewCtrls("TAB","INS", self~Hwnd, nr, text, image)
   else item = HandleOtherNewCtrls("TAB","INS", self~Hwnd, nr, text, image, lparam)
   if item \= -1 then self~LastItem = item
   return item


::method Modify
   use arg nr, text, image, lparam
   if arg(1,'o') = 1 then return -1
   if arg(2,'o') = 1 then text = ""
   if arg(3,'o') = 1 then image = -1
   if arg(4,'o') = 1 then return HandleOtherNewCtrls("TAB","SET", self~Hwnd, nr, text, image)
   else return HandleOtherNewCtrls("TAB","SET", self~Hwnd, nr, text, image, lparam)


/* adds text1, text2, text3.... */
::method AddSequence
   nr = self~LastItem
   do i= 1 to arg()
       nr = nr + 1
       nr = self~Insert(nr,arg(i))
       if nr = -1 then return -1    /* error occurred */
       self~LastItem = nr
   end
   return nr

/* adds text1, image1, lparam1, text2, image2, lparam2, text3.... */
::method AddFullSeq
   args = arg()
   nr = self~LastItem
   i = 1
   do while i <= args
       nr = nr + 1
       if i+2 <= args & arg(i+2,'e') = 1 then do
           if arg(i+1,'e') = 1 then nr = self~Insert(nr,arg(i), arg(i+1), arg(i+2))
           else nr = self~Insert(nr,arg(i),, arg(i+2))
       end
       else if i+1 <= args & arg(i+1,'e') = 1 then nr = self~Insert(nr,arg(i), arg(i+1))
       else nr = self~Insert(nr,arg(i))
       if nr = -1 then return -1   /* error occurred */
       self~LastItem = nr
       i = i+3
   end
   return nr

::method Items
   return HandleOtherNewCtrls("TAB","CNT",self~Hwnd)

::method Rows
   return HandleOtherNewCtrls("TAB","ROWCNT",self~Hwnd)

::method ItemInfo
   use arg hItem
   if arg(1,'o') = 1 then return -1
   ret = HandleOtherNewCtrls("TAB","GET",self~Hwnd, hItem, "InternalTCItemInfo")
   return InternalTCItemInfo.

::method Delete
   use arg item
   if arg(1,'o') = 1 then return -1
   return HandleOtherNewCtrls("TAB","DEL",self~Hwnd, item)

::method DeleteAll
   return HandleOtherNewCtrls("TAB","DEL",self~Hwnd,"ALL")

::method Last
   return self~Items - 1

::method Selected
   return HandleOtherNewCtrls("TAB","SEL",self~Hwnd,"GT")

::method SelectedIndex
   return HandleOtherNewCtrls("TAB","SEL",self~Hwnd,"GN")

::method Select
   use arg text
   return HandleOtherNewCtrls("TAB","SEL",self~Hwnd,"ST", text)

::method SelectIndex
   use arg item
   return HandleOtherNewCtrls("TAB","SEL",self~Hwnd,"SN", item)

::method Focus
   use arg item
   return HandleOtherNewCtrls("TAB","FOCUS",self~Hwnd, item)

::method Focused
   return HandleOtherNewCtrls("TAB","FOCUS",self~Hwnd, "G")

-- DEPRECATED
::method setImages external "LIBRARY oodialog tab_setImageList"

-- DEPRECATED
::method RemoveImages
   return self~setImageList(.nil)

::method setImageList external "LIBRARY oodialog tab_setImageList"
::method getImageList external "LIBRARY oodialog tab_getImageList"

::method SetPadding
   parse arg cx, cy
   return HandleOtherNewCtrls("TAB","PADDING",self~Hwnd, cx, cy)

::method SetSize
   parse arg cx, cy
   return HandleOtherNewCtrls("TAB","SIZE",self~Hwnd, cx, cy)

::method PosRectangle
   use arg item
   if arg(1,'o') = 1 then return ""
   return HandleOtherNewCtrls("TAB","RECT",self~Hwnd, item)

::method AdjustToRectangle
   use arg left, top, right, bottom
   return HandleOtherNewCtrls("TAB","ADJUST",self~Hwnd, 1, left, top, right, bottom)

::method RequiredWindowSize
   use arg left, top, right, bottom
   return HandleOtherNewCtrls("TAB","ADJUST",self~Hwnd, 0, left, top, right, bottom)


/**********       Old controls                     ****************************/

::class 'StaticControl' subclass DialogControl public

::method getIcon external "LIBRARY oodialog stc_getIcon"
::method setIcon external "LIBRARY oodialog stc_setIcon"
::method getImage external "LIBRARY oodialog stc_getImage"
::method setImage external "LIBRARY oodialog stc_setImage"


/************************* Edit Control Class *********************************/

::class 'EditControl' subclass DialogControl public

::method selection external "LIBRARY oodialog e_selection"
::method selected
    s = self~selection
    return s~startChar s~endChar

::method replaceSelText external "LIBRARY oodialog e_replaceSelText"
::method getLine external "LIBRARY oodialog e_getLine"
::method hideBalloon external "LIBRARY oodialog e_hideBallon"
::method showBalloon external "LIBRARY oodialog e_showBallon"
::method setCue external "LIBRARY oodialog e_setCue"
::method replaceStyle external "LIBRARY oodialog e_style"
::method addStyle external "LIBRARY oodialog e_style"
::method removeStyle external "LIBRARY oodialog e_style"
::method getStyle external "LIBRARY oodialog e_style"


::method select
    use arg start, end
    if arg(1,'o') = 1 then start = 1
    if arg(2,'o') = 1 then end = 0
    self~Message2Parent(0x00B1, start-1, end-1)

::method ScrollCommand
    use arg kind, reps
    if arg(1,'o') = 1 then kind = "UP"; else kind = kind~translate
    if arg(2,'o') = 1 then reps = 1
    select
        when kind = "UP" | kind = "LEFT" then sb = 0
        when kind = "DOWN" | kind = "RIGHT" then sb = 1
        when kind = "PAGEUP" | kind = "PAGELEFT" then sb = 2
        when kind = "PAGEDOWN" | kind = "PAGERIGHT" then sb = 3
    end
    do i = 1 to reps
        self~Message2Parent(0x00B5, sb, 0)
    end

::method LineScroll
    use arg cx, cy
    return \self~Message2Parent(0x00B6, cx, cy)

::method EnsureCaretVisibility
    return \self~Message2Parent(0x00B7, 0, 0)

::method IsModified
    return self~Message2Parent(0x00B8, 0, 0)

::method SetModified
    use arg bool
    if arg(1,'o') = 1 then bool = 1
    self~Message2Parent(0x00B9, bool, 0)

::method Lines
    return self~Message2Parent(0x00BA, 0, 0)

::method LineIndex
    use arg line
    if arg(1,'o') = 1 then line = 0
    return self~Message2Parent(0x00BB, line-1, 0) +1

::method LineLength
    use arg line
    if arg(1,'o') = 1 then line = -1; else line = line-1
    ndx = self~Message2Parent(0x00BB, line, 0)
    return self~Message2Parent(0x00C1, ndx, 0)

::method SetLimit
    use arg limit
    self~Message2Parent(0x00C5, limit, 0)

::method LineFromIndex
    use arg ndx
    if arg(1,'o') = 1 then ndx = -1; else ndx = ndx - 1
    return self~Message2Parent(0x00C9, ndx, 0) +1

::method "PasswordChar="
    use arg char
    self~Message2Parent(0x00CC, char~c2d, 0)

::method PasswordChar
    char = self~Message2Parent(0x00D2, 0, 0)
    if char \= 0 then return char~d2c
    else return ""

::method FirstVisibleLine
    return self~Message2Parent(0x00CE, 0, 0) + 1

::method SetReadOnly
    use arg bool
    if arg(1,'o') = 1 then bool = 1
    return \self~Message2Parent(0x00CF, bool, 0)

::method SetMargins
    use arg left, right
    flag = 0
    if arg(1,'o') = 1 & arg(2,'o') = 1 then flag = "0xFFFF" /* no arguments = auto margins */
    else do
        if arg(1,'e') = 1 then flag = flag + 1
        if arg(2,'e') = 1 then flag = flag + 2
    end
    self~Message2Parent(0x00D3, flag, right*X2D("10000") + left)

::method Margins
    marg = self~Message2Parent(0x00D4, 0, 0)
    right = .DlgUtil~hiWord(marg)
    left = .DlgUtil~loWord(marg)
    return left" "right

::method "Tab="
    use arg dlgunits
    self~Message2Parent(0x00CB, 1, "L" || dlgunits)


/******************* ButtonControl Classes ************************************/

::class 'GroupBox' subclass DialogControl public
::method "style=" external "LIBRARY oodialog gb_setStyle"

::class 'ButtonControl' subclass DialogControl public

::method state external "LIBRARY oodialog bc_getState"
::method "state=" external "LIBRARY oodialog bc_setState"
::method "style=" external "LIBRARY oodialog bc_setStyle"

::method push external "LIBRARY oodialog bc_click"
::method click external "LIBRARY oodialog bc_click"

::method getIdealSize external "LIBRARY oodialog bc_getIdealSize"
::method getTextMargin external "LIBRARY oodialog bc_getTextMargin"
::method setTextMargin external "LIBRARY oodialog bc_setTextMargin"
::method setImageList external "LIBRARY oodialog bc_setImageList"
::method getImageList external "LIBRARY oodialog bc_getImageList"
::method getImage external "LIBRARY oodialog bc_getImage"
::method setImage external "LIBRARY oodialog bc_setImage"

/* For internal testing only, do not use this method */
::method test external "LIBRARY oodialog bc_test"
::method test class external "LIBRARY oodialog bc_test_cls"

::method changeBitmap unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~ID
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) message "ChangeBitmapButton" Arguments (newarg)

::method displaceBitmap unguarded
   use arg x, y
   return self~oDlg~DisplaceBitmap(self~ID, x, y)

::method getBmpDisplacement unguarded
   return self~oDlg~GetBmpDisplacement(self~ID)

::method scrollText unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~hwnd
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) Arguments (newarg)

   /* This method moves the rectangle within a button and redraws the uncovered area */
::method scroll unguarded
   use arg xPos, yPos, left, top, right, bottom
   return ScrollTheWindow(self~oDlg~adm, self~hwnd, xPos, yPos, left, top, right, bottom, "yes")

::method getBitmapSizeX unguarded
   return self~oDlg~GetBitmapSizeX(self~ID)

::method getBitmapSizeY unguarded
   return self~oDlg~GetBitmapSizeY(self~ID)

   /* This method will draw the bitmap of a button */
   /* Use this method to move a bitmap or a part of it... */

::method drawBitmap unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+2)
   newarg[1] = self~hwnd
   newarg[2] = self~id
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+2] = arga[i]; end
   forward to (self~oDlg) Arguments (newarg)


   /* This method will draw the bitmap step by step */

::method dimBitmap unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~id
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) Arguments (newarg)


::method scrollBitmapFromTo unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~id
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) Arguments (newarg)


::class 'RadioButton' subclass ButtonControl public
::method checkInGroup class external "LIBRARY oodialog rb_checkInGroup_cls"

::method checked external "LIBRARY oodialog rb_checked"
::method check external "LIBRARY oodialog rb_check"
::method uncheck external "LIBRARY oodialog rb_uncheck"
::method getCheckState external "LIBRARY oodialog rb_getCheckState"

-- DEPRECATED
::method isChecked external "LIBRARY oodialog rb_isChecked"

-- DEPRECATED
::method indeterminate external "LIBRARY oodialog rb_indeterminate"

::class 'CheckBox' subclass RadioButton public

::method setIndeterminate external "LIBRARY oodialog ckbx_setIndeterminate"
::method isIndeterminate external "LIBRARY oodialog ckbx_isIndeterminate"

/**************** List Box Class **********************************************/

::class 'ListBox' subclass DialogControl public

::constant LB_GETHORIZONTALEXTENT  "0x0193"
::constant LB_SETHORIZONTALEXTENT  "0x0194"


::method setTabulators external "LIBRARY oodialog generic_setListTabulators"
::method getText external "LIBRARY oodialog lb_getText"
::method add external "LIBRARY oodialog lb_add"
::method insert external "LIBRARY oodialog lb_insert"
::method select external "LIBRARY oodialog lb_select"
::method find external "LIBRARY oodialog lb_find"
::method addDirectory external "LIBRARY oodialog lb_addDirectory"

::method Delete
   use arg index
   if arg(1,"o") = 1 then index = self~SelectedIndex
   return self~Message2Parent(0x00000182, index-1, 0)

::method DeleteAll
   return self~Message2Parent(0x00000184, 0, 0)

::method selectedIndex
   return self~Message2Parent(0x00000188, 0, 0) + 1

::method Selected
   return self~getText(self~SelectedIndex)

::method SelectIndex
   use arg ndx
   if arg(1,'o') = 1 then ndx = 0
   return self~Message2Parent(0x00000186, ndx-1, 0)

::method DeSelectIndex
   use arg ndx
   if arg(1,'o') = 1 then ndx = 0
   return self~Message2Parent(0x00000186, ndx-1, "D")

::method SelectRange
   use arg fromNdx, toNdx
   if arg(1,'o') = 1 then fromNdx = 1
   if arg(2,'o') = 1 then toNdx = self~Items

   lparam = (toNdx-1) * "10000"~x2d + (fromNdx-1)
   return self~Message2Parent(0x0000019B, 1, lparam)


::method deselectRange
   use strict arg fromNdx = 1, toNdx = (self~items)

   lparam = (toNdx-1) * "10000"~x2d + (fromNdx-1)
   return self~Message2Parent(0x0000019B, 0, lparam)


::method Items
   return self~Message2Parent(0x0000018B, 0, 0)

::method SelectedItems
   return self~Message2Parent(0x00000190, 0, 0)

::method selectedIndexes
   return self~oDlg~getListBoxData(self~id)

::method MakeFirstVisible
   use strict arg ndx = 1
   return self~Message2Parent(0x00000197, ndx - 1, 0)

::method GetFirstVisible
   return self~Message2Parent(0x0000018E, 0, 0) + 1

::method Modify
   use strict arg index = (self~selectedIndex), newItem
   if index <= 0 then return -1
   self~delete(index)
   return self~insert(index, newItem)

::method setWidthPx
   use strict arg pixels
   self~sendWinIntMsg(self~LB_SETHORIZONTALEXTENT, pixels, 0)
   return 0

::method setWidth       -- Dialog units, not accurate
   use strict arg dlgunits
   if dlgunits~datatype("N") = 0 then return -1
   px = trunc(dlgunits * self~factorX)
   self~setWidthPx(px)

::method Width
   return self~Message2Parent(0x00000193, 0, 0)/self~factorX

::method "ItemHeight="
   use arg dlgunits
   if arg(1,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   self~Message2Parent(0x000001A0, 0, dlgunits*self~factorY)

::method ItemHeight
   return self~Message2Parent(0x000001A1, 0, 0)/self~factorY

::method "ColumnWidth="
   use arg dlgunits
   if arg(1,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   self~Message2Parent(0x00000195, dlgunits*self~factorX, 0)


/********** Combo Box Class ***************************************************/

::class 'ComboBox' subclass DialogControl public

::constant CB_SETEDITSEL               "0x0142"
::constant CB_DELETESTRING             "0x0144"
::constant CB_GETCOUNT                 "0x0146"
::constant CB_GETCURSEL                "0x0147"
::constant CB_RESETCONTENT             "0x014B"
::constant CB_SETCURSEL                "0x014E"
::constant CB_SHOWDROPDOWN             "0x014F"
::constant CB_GETDROPPEDSTATE          "0x0157"

::method getText external "LIBRARY oodialog cb_getText"
::method add external "LIBRARY oodialog cb_add"
::method insert external "LIBRARY oodialog cb_insert"
::method select external "LIBRARY oodialog cb_select"
::method find external "LIBRARY oodialog cb_find"
::method addDirectory external "LIBRARY oodialog cb_addDirectory"

::method delete
   use strict arg index = (self~selectedIndex)
   ret = self~sendWinIntMsg(self~CB_DELETESTRING, index - 1, 0)
   if ret == 0 then self~deleteAll
   return ret

::method deleteAll
   return self~sendWinIntMsg(self~CB_RESETCONTENT, 0, 0)

::method selectedIndex
   return self~sendWinIntMsg(self~CB_GETCURSEL, 0, 0) + 1

::method selected
   return self~getText(self~selectedIndex)

::method selectIndex
   use strict arg ndx = 0
   return self~sendWinIntMsg(self~CB_SETCURSEL, ndx - 1, 0)

::method items
   return self~sendWinIntMsg(self~CB_GETCOUNT, 0, 0)

::method modify
   use strict arg index = (self~selectedIndex), newEntry
   if index <= 0 then return -1
   self~delete(index)
   return self~insert(index, newEntry)

::method openDropDown
   self~sendWinIntMsg(self~CB_SHOWDROPDOWN, 1, 0)

::method closeDropDown
   self~sendWinIntMsg(self~CB_SHOWDROPDOWN, 0, 0)

::method isDropDownOpen
   return self~sendWinIntMsg(self~CB_GETDROPPEDSTATE, 0, 0)

::method editSelection
   use strict arg startndx = 0, endndx = 0
   if endndx == 0 then endndx = -1
   lParam = .DlgUtil~makeLParam(startndx - 1, endndx)
   return self~sendWinIntMsg(self~CB_SETEDITSEL, 0, lParam) == -1


/*********** ScrollBar Class **************************************************/


::class 'ScrollBar' subclass DialogControl public

::method SetRange
   use arg min, max, redraw
   if arg(3, "o") = 1 then redraw = 1
   return HandleScrollBar("SR",self~hwnd,min,max, redraw)

::method Range
   return HandleScrollBar("GR",self~hwnd)

::method SetPos
   use arg pos, redraw
   if arg(2, "o") = 1 then redraw = 1
   return HandleScrollBar("SP",self~hwnd,pos, redraw)

::method Position
   return HandleScrollBar("GP",self~hwnd)

::method DeterminePosition
   use arg posdata, single, page
   if arg(2,"o") = 1 then single = 1
   if arg(3,"o") = 1 then page = 10
   code = .DlgUtil~loWord(posdata)
   parse value self~Range with rmin rmax
   pos = self~Position
   select
      /* Line up */
      when code = 0 then pos = max(rmin,pos - single)
      /* Line down */
      when code = 1 then pos = min(rmax,pos + single)
      /* page up */
      when code = 2 then pos = max(rmin,pos - page)
      /* page down */
      when code = 3 then pos = min(rmax,pos + page)
      /* track position */
      when code = 4 then pos = .DlgUtil~hiWord(posdata)
      /* tracking */
      when code = 5 then pos = .DlgUtil~loWord(posdata)
      /* top */
      when code = 6 then pos = rmin
      /* bottom */
      when code = 7 then pos = rmax
      otherwise nop;
   end
   self~SetPos(pos)
   return pos


::class 'DateTimePicker' subclass DialogControl public

::attribute dateTime external "LIBRARY oodialog _dtp_dateTime"


::class 'MonthCalendar' subclass DialogControl public

::attribute usesUnicode external "LIBRARY oodialog _mc_usesUnicode"
::attribute date external "LIBRARY oodialog _mc_date"

-- DEPRECATED
::class 'AdvancedControls' Mixinclass Object public
