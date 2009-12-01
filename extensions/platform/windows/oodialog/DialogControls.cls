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

::requires "BaseDialog.cls"

-- Base class for all dialog controls
::class 'DialogControl' public inherit WindowBase WindowExtensions

::method new class external "LIBRARY oodialog dlgctrl_new_cls"

::method init external "LIBRARY oodialog dlgctrl_init"

::attribute id get    -- The numerical resource ID for this control
::attribute oDlg get  -- The ooRexx dialog object this control belongs to
::attribute hDlg get  -- The window handle of that dialog

::method getTextSizeDlg unguarded external "LIBRARY oodialog dlgctrl_getTextSizeDlg"
::method unInit external "LIBRARY oodialog dlgctrl_unInit"

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
::method setColor external "LIBRARY oodialog dlgctrl_setColor"
::method setSysColor external "LIBRARY oodialog dlgctrl_setColor"

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


::class 'StaticControl' subclass DialogControl public

::method getIcon external "LIBRARY oodialog stc_getIcon"
::method setIcon external "LIBRARY oodialog stc_setIcon"
::method getImage external "LIBRARY oodialog stc_getImage"
::method setImage external "LIBRARY oodialog stc_setImage"


/************************* Edit Control Class *********************************/

::class 'EditControl' subclass DialogControl public

::constant EM_SETSEL               "0x00B1"
::constant EM_SCROLL               "0x00B5"
::constant EM_LINESCROLL           "0x00B6"
::constant EM_SCROLLCARET          "0x00B7"
::constant EM_GETMODIFY            "0x00B8"
::constant EM_SETMODIFY            "0x00B9"
::constant EM_GETLINECOUNT         "0x00BA"
::constant EM_LINELENGTH           "0x00C1"
::constant EM_LIMITTEXT            "0x00C5"
::constant EM_LINEFROMCHAR         "0x00C9"
::constant EM_SETTABSTOPS          "0x00CB"
::constant EM_SETPASSWORDCHAR      "0x00CC"
::constant EM_GETFIRSTVISIBLELINE  "0x00CE"
::constant EM_SETREADONLY          "0x00CF"
::constant EM_GETPASSWORDCHAR      "0x00D2"
::constant EM_SETMARGINS           "0x00D3"
::constant EM_GETMARGINS           "0x00D4"

::constant EM_NEGATIVEONE          4294967295

::constant EC_LEFTMARGIN           1       -- 0x0001
::constant EC_RIGHTMARGIN          2       -- 0x0002
::constant EC_USEFONTINFO          65535   -- 0xFFFF

::method isSingleLine external "LIBRARY oodialog e_isSingleLine"
::method selection external "LIBRARY oodialog e_selection"
::method selected
    s = self~selection
    return s~startChar s~endChar

::method replaceSelText external "LIBRARY oodialog e_replaceSelText"
::method getLine external "LIBRARY oodialog e_getLine"
::method lineIndex external "LIBRARY oodialog e_lineIndex"
::method hideBalloon external "LIBRARY oodialog e_hideBallon"
::method showBalloon external "LIBRARY oodialog e_showBallon"
::method setCue external "LIBRARY oodialog e_setCue"
::method replaceStyle external "LIBRARY oodialog e_style"
::method addStyle external "LIBRARY oodialog e_style"
::method removeStyle external "LIBRARY oodialog e_style"
::method getStyle external "LIBRARY oodialog e_style"

::method setTabStops external "LIBRARY oodialog e_setTabStops"
::method "Tab="
    use strict arg dlgunits
    self~setTabStops(.array~of(dlgunits))

::method select
    use strict arg start = 1, end = 0
    self~sendWinIntMsg(self~EM_SETSEL, start - 1, end - 1)
    return 0

::method scrollCommand
    use arg kind = "UP", reps = 1
    kind = kind~translate
    if self~isSingleLine then return 0
    select
        when kind = "UP" | kind = "LEFT" then wParam = 0
        when kind = "DOWN" | kind = "RIGHT" then wParam = 1
        when kind = "PAGEUP" | kind = "PAGELEFT" then wParam = 2
        when kind = "PAGEDOWN" | kind = "PAGERIGHT" then wParam = 3
        otherwise wParam = 0
    end
    do i = 1 to reps
        ret = self~sendWinIntMsg(self~EM_SCROLL, wParam, 0)
    end
    return ret

::method lineScroll
    use strict arg cx, cy
    if self~isSingleLine then return 1
    self~sendWinIntMsg(self~EM_LINESCROLL, cx, cy)
    return 0

::method ensureCaretVisibility
    use strict arg
    self~sendWinIntMsg(self~EM_SCROLLCARET, 0, 0)
    return 0

::method isModified
    use strict arg
    return self~sendWinIntMsg(self~EM_GETMODIFY, 0, 0) <> 0

::method setModified
    use strict arg bool = .true
    self~sendWinIntMsg(self~EM_SETMODIFY, bool, 0)
    return 0

::method lines
    use strict arg
    return self~sendWinIntMsg(self~EM_GETLINECOUNT, 0, 0)

::method lineLength
    use strict arg line = (-1)
    if line == -1 then line = self~EM_NEGATIVEONE
    else line -= 1
    ndx = self~lineIndex(line)
    if ndx == -1 then return -1
    return self~sendWinIntMsg(self~EM_LINELENGTH, ndx - 1, 0)

::method setLimit
    use strict arg limit
    self~sendWinIntMsg(self~EM_LIMITTEXT, limit, 0)
    return 0

::method lineFromIndex
    use strict arg ndx = (-1)
    if ndx == -1 then ndx = self~EM_NEGATIVEONE
    else ndx -= 1
    return self~sendWinIntMsg(self~EM_LINEFROMCHAR, ndx, 0) + 1

::method "passwordChar="
    use strict arg char
    ch = char~c2d
    if self~isSingleLine, ch > 0, ch < 256 then self~sendWinIntMsg(self~EM_SETPASSWORDCHAR, ch, 0)

::method passwordChar
    char = self~sendWinIntMsg(self~EM_GETPASSWORDCHAR, 0, 0)
    if char \= 0 then return char~d2c
    else return ""

::method firstVisibleLine
    return self~sendWinIntMsg(EM_GETFIRSTVISIBLELINE, 0, 0) + 1

::method setReadOnly
    use strict arg bool = .true
    return self~sendWinIntMsg(self~EM_SETREADONLY, bool, 0) <> 0

::method setMargins
    use arg left, right
    if arg(1, 'O'), arg(2, 'O') then do
       self~sendWinIntMsg(self~EM_SETMARGINS, 0, self~EC_USEFONTINFO)
    end
    else do
       flag = 0
       if arg(1, 'E') then flag = self~EC_LEFTMARGIN
       if arg(2, 'E') then flag .DlgUtil~or(flag, self~EC_RIGHTMARGIN)
       lParam = .DlgUtil~makeLParam(right, left)
       self~sendWinIntMsg(self~EM_SETMARGINS, flag, lParam)
    end
    return 0

::method getMargins
    marg = self~sendWinIntMsg(self~EM_GETMARGINS, 0, 0)
    m = .directory~new
    m~right = .DlgUtil~hiWord(marg)
    m~left = .DlgUtil~loWord(marg)
    return m

::method margins
    m = self~getMargins
    return m~left m~right

-- TODO Tab= never seems to have been documented.  setTabStops() has been added.


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
::method scroll unguarded external "LIBRARY oodialog bc_scroll"

-- Bitmap button methods.
::method dimBitmap unguarded external "LIBRARY oodialog bc_dimBitmap"

-- The following methods are implemented in DialogExtensions rather than here
-- because many of them do not require that that underlying dialog is already
-- created.  There is no button control until after the dialog is created.
::method changeBitmap unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~ID
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) message "CHANGEBITMAPBUTTON" arguments (newarg)

::method setBitmapPosition unguarded
   use strict arg x, y = 0
   if arg(2, "E") then return self~oDlg~setBimapPosition(self~id, x, y)
   else return self~oDlg~setBimapPosition(self~id, x)

::method getBitmapPosition unguarded
   use strict arg pos
   return self~oDlg~getBitmapPosition(self~ID, pos)

::method displaceBitmap unguarded
   use strict arg x, y
   return self~oDlg~setBitmapPosition(self~ID, x, y)

::method getBmpDisplacement unguarded
   return self~oDlg~getBmpDisplacement(self~ID)

::method scrollText unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~hwnd
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) arguments (newarg)

::method getBitmapSizeX unguarded
   return self~oDlg~getBitmapSizeX(self~ID)

::method getBitmapSizeY unguarded
   return self~oDlg~getBitmapSizeY(self~ID)

::method drawBitmap unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+2)
   newarg[1] = self~hwnd
   newarg[2] = self~id
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+2] = arga[i]; end
   forward to (self~oDlg) arguments (newarg)

::method scrollBitmapFromTo unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~id
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) arguments (newarg)


/* For internal testing only, do not use this method */
::method test external "LIBRARY oodialog bc_test"
::method test class external "LIBRARY oodialog bc_test_cls"


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

::constant LB_DELETESTRING         "0x0182"
::constant LB_RESETCONTENT         "0x0184"
::constant LB_SETCURSEL            "0x0186"
::constant LB_GETCURSEL            "0x0188"
::constant LB_GETCOUNT             "0x018B"
::constant LB_GETTOPINDEX          "0x018E"
::constant LB_GETSELCOUNT          "0x0190"
::constant LB_GETHORIZONTALEXTENT  "0x0193"
::constant LB_SETHORIZONTALEXTENT  "0x0194"
::constant LB_SETCOLUMNWIDTH       "0x0195"
::constant LB_SETTOPINDEX          "0x0197"
::constant LB_SELITEMRANGE         "0x019B"
::constant LB_SETITEMHEIGHT        "0x01A0"
::constant LB_GETITEMHEIGHT        "0x01A1"

::method setTabulators external "LIBRARY oodialog generic_setListTabulators"
::method isSingleSelection external "LIBRARY oodialog lb_isSingleSelection"
::method getText external "LIBRARY oodialog lb_getText"
::method add external "LIBRARY oodialog lb_add"
::method insert external "LIBRARY oodialog lb_insert"
::method select external "LIBRARY oodialog lb_select"
::method selectIndex external "LIBRARY oodialog lb_selectIndex"
::method deSelectIndex external "LIBRARY oodialog lb_deselectIndex"
::method selectedIndex external "LIBRARY oodialog lb_selectedIndex"
::method find external "LIBRARY oodialog lb_find"
::method addDirectory external "LIBRARY oodialog lb_addDirectory"

::method delete
   use strict arg index = (self~selectedIndex)
   if index == 0 then return 0
   return self~sendWinIntMsg(self~LB_DELETESTRING, index - 1, 0)

::method deleteAll
   return self~sendWinIntMsg(self~LB_RESETCONTENT, 0, 0)

::method selected
   return self~getText(self~selectedIndex)

::method selectRange
   use strict arg fromNdx = 1, toNdx = (self~items)
   if self~isSingleSelection then return -1
   lParam = .DlgUtil~makeLParam(fromNdx - 1, toNdx - 1)
   return self~sendWinIntMsg(self~LB_SELITEMRANGE, .true, lParam)

::method deselectRange
   use strict arg fromNdx = 1, toNdx = (self~items)
   if self~isSingleSelection then return -1
   lParam = .DlgUtil~makeLParam(fromNdx - 1, toNdx - 1)
   return self~sendWinIntMsg(self~LB_SELITEMRANGE, .false, lParam)

::method items
   return self~sendWinIntMsg(self~LB_GETCOUNT, 0, 0)

::method selectedItems
   return self~sendWinIntMsg(self~LB_GETSELCOUNT, 0, 0)

::method selectedIndexes                                    -- TODO be nice to return an array
   return self~oDlg~getListBoxData(self~id)

::method makeFirstVisible
   use strict arg ndx = 1
   return self~sendWinIntMsg(self~LB_SETTOPINDEX, ndx - 1, 0)

::method getFirstVisible
   return self~sendWinIntMsg(self~LB_GETTOPINDEX, 0, 0) + 1

::method modify
   use strict arg index = (self~selectedIndex), newItem
   if index <= 0 then return -1
   self~delete(index)
   return self~insert(index, newItem)

::method setWidthPx
   use strict arg pixels
   self~sendWinIntMsg(self~LB_SETHORIZONTALEXTENT, pixels, 0)
   return 0

::method widthPx
   return self~sendWinIntMsg(self~LB_GETHORIZONTALEXTENT, 0, 0)

::method "itemHeightPx="
   use strict arg pixels
   self~sendWinIntMsg(self~LB_SETITEMHEIGHT, pixels, 0)

::method itemHeightPx
   return self~sendWinIntMsg(self~LB_GETITEMHEIGHT, 0, 0)

::method "columnWidthPx="
   use strict arg pixels
   self~sendWinIntMsg(self~LB_SETCOLUMNWIDTH, pixels, 0)

::method setWidth       -- Dialog units, not accurate
   use strict arg dlgunits
   if dlgunits~datatype("N") = 0 then return -1
   px = trunc(dlgunits * self~factorX)
   self~setWidthPx(px)

::method width          -- Dialog units, not accurate
   return self~widthPx / self~factorX

::method "itemHeight="  -- Dialog units, not accurate
   use strict arg dlgunits
   if dlgunits~datatype("N") = 0 then return
   self~itemHeightPx = trunc(dlgunits * self~factorY)

::method itemHeight     -- Dialog units, not accurate
   return self~itemHeightPx / self~factorY

::method "columnWidth=" -- Dialog units, not accurate
   use strict arg dlgunits
   if dlgunits~datatype("N") = 0 then return
   self~columnWidthPx = trunc(dlgunits * self~factorX)


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

::method setRange external "LIBRARY oodialog sb_setRange"
::method getRange external "LIBRARY oodialog sb_getRange"
::method setPos external "LIBRARY oodialog sb_setPosition"
::method getPosition external "LIBRARY oodialog sb_getPosition"
::method position external "LIBRARY oodialog sb_getPosition"

::method range
   r = self~getRange
   return r~min r~max

::method determinePosition
   use strict arg posdata, single = 1, page = 10
   code = .DlgUtil~loWord(posdata)
   pos = self~getPosition
   r = self~getRange
   if .SystemErrorCode <> 0 then return pos -- getRange failed

   select
      /* Line up */
      when code = 0 then pos = max(r~min, pos - single)
      /* Line down */
      when code = 1 then pos = min(r~max, pos + single)
      /* page up */
      when code = 2 then pos = max(r~min, pos - page)
      /* page down */
      when code = 3 then pos = min(r~max, pos + page)
      /* track position */
      when code = 4 then pos = .DlgUtil~hiWord(posdata)
      /* tracking */
      when code = 5 then pos = .DlgUtil~loWord(posdata)
      /* top */
      when code = 6 then pos = r~min
      /* bottom */
      when code = 7 then pos = r~max
      otherwise nop;
   end
   self~setPos(pos)
   return pos


/********* Progress Bar Class *************************************************/

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


/******** Track Bar Class ******************************************************/

::class 'SliderControl' subclass DialogControl public

-- TBM_ messages are defined as WM_USER + x
-- WM_USER == 0x0400

::constant TBM_GETPOS              "0x0400"
::constant TBM_GETRANGEMIN         "0x0401"
::constant TBM_GETRANGEMAX         "0x0402"
::constant TBM_GETTIC              "0x0403"
::constant TBM_SETTIC              "0x0404"
::constant TBM_SETPOS              "0x0405"
::constant TBM_SETRANGE            "0x0406"
::constant TBM_SETRANGEMIN         "0x0407"
::constant TBM_SETRANGEMAX         "0x0408"
::constant TBM_CLEARTICS           "0x0409"
::constant TBM_SETSEL              "0x040A"
::constant TBM_SETSELSTART         "0x040B"
::constant TBM_SETSELEND           "0x040C"
::constant TBM_GETNUMTICS          "0x0410"
::constant TBM_GETSELSTART         "0x0411"
::constant TBM_GETSELEND           "0x0412"
::constant TBM_CLEARSEL            "0x0413"
::constant TBM_SETTICFREQ          "0x0414"
::constant TBM_SETPAGESIZE         "0x0415"
::constant TBM_GETPAGESIZE         "0x0416"
::constant TBM_SETLINESIZE         "0x0417"
::constant TBM_GETLINESIZE         "0x0418"

::method "pos="
   forward message (setPos)

::method pos
   use strict arg
   return self~sendWinIntMsg(self~TBM_GETPOS, 0, 0)

::method setPos
   use strict arg p, redraw = .false
   self~sendWinIntMsg(self~TBM_SETPOS, redraw, p)
   return 0

::method initRange
   use strict arg min = 0, max = 100, redraw = .false
   if max < min then return -1
   self~sendWinIntMsg(self~TBM_SETRANGE, redraw, .DlgUtil~makeLParam(min, max))
   return 0

::method getMin
   use strict arg
   return self~sendWinIntMsg(self~TBM_GETRANGEMIN, 0, 0)

::method setMin
   use strict arg min, redraw = .true
   self~sendWinIntMsg(self~TBM_SETRANGEMIN, redraw, min)
   return 0

::method getMax
   return self~sendWinIntMsg(self~TBM_GETRANGEMAX, 0, 0)

::method setMax
   use strict arg max, redraw = .true
   self~sendWinIntMsg(self~TBM_SETRANGEMAX, redraw, max)
   return 0

::method getRange external "LIBRARY oodialog tb_getRange"
::method range
   r = self~getRange
   return r~min r~max

::method clearTicks
   use strict arg redraw = .true
   self~sendWinIntMsg(self~TBM_CLEARTICS, redraw, 0)
   return 0

::method countTicks
   return self~sendWinIntMsg(self~TBM_GETNUMTICS, 0, 0)

::method getTick
   use strict arg tic
   return self~sendWinIntMsg(self~TBM_GETTIC, tic, 0)

::method setTickAt
   use strict arg pos
   return self~sendWinIntMsg(self~TBM_SETTIC, 0, pos)

::method setTickFrequency
   use strict arg freq
   return self~sendWinIntMsg(self~TBM_SETTICFREQ, freq, 0)

::method getLineStep
   return self~sendWinIntMsg(self~TBM_GETLINESIZE, 0, 0)

::method getPageStep
   return self~sendWinIntMsg(self~TBM_GETPAGESIZE, 0, 0)

::method setLineStep
   use strict arg step
   return self~sendWinIntMsg(self~TBM_SETLINESIZE, 0, step)

::method setPageStep
   use strict arg step
   return self~sendWinIntMsg(self~TBM_SETPAGESIZE, 0, step)

::method initSelRange
   use strict arg min = 0, max = (self~getMax), redraw = .false
   if max < min then return -1
   self~sendWinIntMsg(self~TBM_SETSEL, redraw, .DlgUtil~makeLParam(min, max))
   return 0

::method getSelStart
   use strict arg
   return self~sendWinIntMsg(self~TBM_GETSELSTART, 0, 0)
   return 0

::method setSelStart
   use strict arg min, redraw = .true
   self~sendWinIntMsg(self~TBM_SETSELSTART, redraw, min)
   return 0

::method getSelEnd
   use strict arg
   return self~sendWinIntMsg(self~TBM_GETSELEND, 0, 0)
   return 0

::method setSelEnd
   use strict arg max, redraw = .true
   self~sendWinIntMsg(self~TBM_SETSELEND, redraw, min)
   return 0

::method clearSelRange
   use strict arg redraw = .true
   self~sendWinIntMsg(self~TBM_CLEARSEL, redraw, 0)
   return 0

::method getSelRange external "LIBRARY oodialog tb_getSelRange"
::method selRange
   sr = self~getSelRange
   return sr~start sr~end


/******************* Tab Control Class ****************************************/

::class 'TabControl' subclass DialogControl public

-- TCM_FIRST == 0x1300  4864
::constant TCM_GETITEMCOUNT    "0x1304)"
::constant TCM_GETROWCOUNT     "0x132C"
::constant TCM_GETCURSEL       "0x130B"
::constant TCM_SETCURSEL       "0x130C"
::constant TCM_SETCURFOCUS     "0x1330"
::constant TCM_GETCURFOCUS     "0x132F"
::constant TCM_DELETEALLITEMS  "0x1309"
::constant TCM_DELETEITEM      "0x1308"

::method LastItem attribute

::method Init
   forward class (super) continue
   self~LastItem = -1

::method setImageList external "LIBRARY oodialog tab_setImageList"
::method getImageList external "LIBRARY oodialog tab_getImageList"

::method items
   use strict arg
   return self~sendWinIntMsg(self~TCM_GETITEMCOUNT, 0, 0)

::method rows
   use strict arg
   return self~sendWinIntMsg(self~TCM_GETROWCOUNT, 0, 0)

::method last
   use strict arg
   return self~items - 1

::method selectedIndex
   use strict arg
   return self~sendWinIntMsg(self~TCM_GETCURSEL, 0, 0)

::method selectIndex
   use strict arg item
   return self~sendWinIntMsg(self~TCM_SETCURSEL, item, 0)

::method focused
   use strict arg
   return self~sendWinIntMsg(self~TCM_GETCURSEL, 0, 0)

::method focus
   use strict arg item
   self~sendWinIntMsg(self~TCM_SETCURSEL, item, 0)
   return 0

::method delete
   use strict arg item
   ret = self~sendWinIntMsg(self~TCM_DELETEITEM, item, 0)
   return ret == 0

::method deleteAll
   use strict arg
   ret = self~sendWinIntMsg(self~TCM_DELETEALLITEMS, 0, 0)
   return ret == 0

::method getRectangle external "LIBRARY oodialog tab_getRectangle"
::method setPadding external "LIBRARY oodialog tab_setPadding"
::method setItemSize external "LIBRARY oodialog tab_setItemSize"

::method setSize
   forward message "setItemSize" continue
   return result~cx result~cy

::method posRectangle
   use arg strict item
   r = .Rect~new
   if self~getRectangle(item, r) then return r~top r~left r~bottom r~right
   else return ""

-- adjustToRectangle() and requireWindowSize() are the old methods.  This set up
-- produces the same results as the code did, which seems to be opposite of what
-- doc says.
--
-- calcWindowRect() and calcDisplayRect() are named and documented according to
-- MSDN documentation.  (Which could be wrong.)

::method calcWindowRect external "LIBRARY oodialog tab_calcRect"
::method adjustToRectangle
   use strict arg left, top, right, bottom
   r = .Rect~new(left, top, right, bottom)
   self~calcWindowRect(r)
   return r~left r~top r~right r~bottom

::method calcDisplayRect external "LIBRARY oodialog tab_calcRect"
::method requiredWindowSize
   use strict arg left, top, right, bottom
   r = .Rect~new(left, top, right, bottom)
   self~calcDisplayRect(r)
   return r~left r~top r~right r~bottom


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

::method ItemInfo
   use strict arg hItem
   ret = HandleOtherNewCtrls("TAB","GET",self~Hwnd, hItem, "InternalTCItemInfo")
   return InternalTCItemInfo.

::method Selected
   return HandleOtherNewCtrls("TAB","SEL",self~Hwnd,"GT")

::method Select
   use strict arg text
   return HandleOtherNewCtrls("TAB","SEL",self~Hwnd,"ST", text)

-- DEPRECATED
::method setImages external "LIBRARY oodialog tab_setImageList"

-- DEPRECATED
::method RemoveImages
   return self~setImageList(.nil)


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
   use strict arg nr = 0, text, width, fmt = "L"
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


/************* Tree Control Class *********************************************/

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

::class 'DateTimePicker' subclass DialogControl public

::attribute dateTime external "LIBRARY oodialog _dtp_dateTime"


::class 'MonthCalendar' subclass DialogControl public

::attribute usesUnicode external "LIBRARY oodialog _mc_usesUnicode"
::attribute date external "LIBRARY oodialog _mc_date"

