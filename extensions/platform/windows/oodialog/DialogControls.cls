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
::method init class external "LIBRARY oodialog dlgctrl_init_cls"

::method init external "LIBRARY oodialog dlgctrl_init"
::method unInit external "LIBRARY oodialog dlgctrl_unInit"

::attribute id get    -- The numerical resource ID for this control
::attribute oDlg get  -- The ooRexx dialog object this control belongs to
::attribute hDlg get  -- The window handle of that dialog

::constant CCM_SETUNICODEFORMAT     "0x2005"
::constant CCM_GETUNICODEFORMAT     "0x2006"

::method useUnicode
   use strict arg doUseUnicode
   return self~sendWinIntMsg(self~CCM_SETUNICODEFORMAT, doUseUnicode, 0)

::method usingUnicode
   use strict arg
   return (self~sendWinIntMsg(self~CCM_GETUNICODEFORMAT, 0, 0) <> 0)

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
::method textSize unguarded external "LIBRARY oodialog dlgctrl_textSize"
::method redrawRect unguarded external "LIBRARY oodialog dlgctrl_redrawRect"
::method clearRect unguarded external "LIBRARY oodialog dlgctrl_clearRect"
::method setColor external "LIBRARY oodialog dlgctrl_setColor"
::method setSysColor external "LIBRARY oodialog dlgctrl_setColor"
::method value unguarded external "LIBRARY oodialog dlgctrl_value"
::method "value=" unguarded external "LIBRARY oodialog dlgctrl_valueEquals"

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
::method getTextSizeDlg unguarded external "LIBRARY oodialog dlgctrl_getTextSizeDlg"

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
::method "tab="
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


/******************* ButtonControl Classes ************************************/

::class 'GroupBox' subclass DialogControl public
::method "style=" external "LIBRARY oodialog gb_setStyle"

::class 'ButtonControl' subclass DialogControl public

::constant BM_CLICK              "0x00F5"

::method state external "LIBRARY oodialog bc_getState"
::method "state=" external "LIBRARY oodialog bc_setState"
::method "style=" external "LIBRARY oodialog bc_setStyle"

::method push
   return self~sendWinIntMsg(self~BM_CLICK, 0, 0)

::method click
   forward message "push"

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

-- WM_USER == 0x400 -> 1024
::constant PBM_SETPOS          "0x0402"
::constant PBM_DELTAPOS        "0x0403"
::constant PBM_SETSTEP         "0x0404"
::constant PBM_STEPIT          "0x0405"
::constant PBM_GETPOS          "0x0408"
::constant PBM_SETBARCOLOR     "0x0409"
::constant PBM_SETBKCOLOR      "0x2001"

::method step
   use strict arg delta = 0
   msg = self~PBM_STEPIT
   if arg(1, 'E') then msg = self~PBM_DELTAPOS
   return self~sendWinIntMsg(msg, delta, 0)

::method setPos
   use strict arg position
   return self~sendWinIntMsg(self~PBM_SETPOS, position, 0)

::method getPos
   use strict arg
   return self~sendWinIntMsg(self~PBM_GETPOS, 0, 0)

::method setStep
  use strict arg newStep = 10
   return self~sendWinIntMsg(self~PBM_SETSTEP, newStep, 0)

::method barColor
   use strict arg newColor
   return self~sendWinUintMsg(self~PBM_SETBARCOLOR, 0, newColor)

::method backgroundColor
   use strict arg newColor
   return self~sendWinUintMsg(self~PBM_SETBKCOLOR, 0, newColor)

::method getFullRange external "LIBRARY oodialog pbc_getFullRange"
::method setMarquee external "LIBRARY oodialog pbc_setMarquee"
::method setFullRange external "LIBRARY oodialog pbc_setFullRange"

-- DEPRECATED
::method getRange external "LIBRARY oodialog pbc_getFullRange"
::method setRange
   forward message "setFullRange" continue
   return result~min result~max


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
   forward message "setPos"

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
::constant TCM_GETITEMCOUNT    "0x1304"
::constant TCM_GETROWCOUNT     "0x132C"
::constant TCM_GETCURSEL       "0x130B"
::constant TCM_SETCURSEL       "0x130C"
::constant TCM_SETCURFOCUS     "0x1330"
::constant TCM_GETCURFOCUS     "0x132F"
::constant TCM_DELETEALLITEMS  "0x1309"
::constant TCM_DELETEITEM      "0x1308"

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

::method getItemRect external "LIBRARY oodialog tab_getItemRect"
::method setPadding external "LIBRARY oodialog tab_setPadding"
::method setItemSize external "LIBRARY oodialog tab_setItemSize"
::method selected external "LIBRARY oodialog tab_selected"
::method select external "LIBRARY oodialog tab_select"
::method insert external "LIBRARY oodialog tab_insert"
::method itemInfo external "LIBRARY oodialog tab_itemInfo"
::method addSequence external "LIBRARY oodialog tab_addSequence"
::method addFullSeq external "LIBRARY oodialog tab_addFullSeq"
::method modify external "LIBRARY oodialog tab_modify"
::method setImageList external "LIBRARY oodialog tab_setImageList"
::method getImageList external "LIBRARY oodialog tab_getImageList"

::method setSize
   forward message "setItemSize" continue
   return result~cx result~cy

::method posRectangle
   use arg strict item
   r = .Rect~new
   if self~getItemRect(item, r) then return r~top r~left r~bottom r~right
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

-- DEPRECATED
::method setImages external "LIBRARY oodialog tab_setImageList"

-- DEPRECATED
::method removeImages
   return self~setImageList(.nil)


/************ List Control Class **********************************************/

::class 'ListControl' subclass DialogControl public

::method subclassEdit external "LIBRARY oodialog generic_subclassEdit"
::method restoreEditClass external "LIBRARY oodialog generic_subclassEdit"

-- LVM_FIRST = 0x1000  == 4096
::constant LVM_GETCOLUMNWIDTH     "0x101D"
::constant LVM_SETHOVERTIME       "0x1047"
::constant LVM_GETHOVERTIME       "0x1048"
::constant LVM_DELETECOLUMN       "0x101C"
::constant LVM_GETITEMSPACING     "0x1033"
::constant LVM_SETITEMCOUNT       "0x102F"
::constant LVM_SCROLL             "0x1014"
::constant LVM_GETCOUNTPERPAGE    "0x1028"
::constant LVM_ENSUREVISIBLE      "0x1013"
::constant LVM_UPDATE             "0x102A"
::constant LVM_REDRAWITEMS        "0x1015"
::constant LVM_GETSELECTEDCOUNT   "0x1032"
::constant LVM_GETITEMCOUNT       "0x1004"
::constant LVM_EDITLABELA         "0x1017"
::constant LVM_EDITLABELW         "0x1076"
::constant LVM_DELETEALLITEMS     "0x1009"
::constant LVM_DELETEITEM         "0x1008"
::constant LVM_GETTOPINDEX        "0x1027"

::method firstVisible
   return self~sendWinIntMsg(self~LVM_GETTOPINDEX , 0, 0)

::method delete
   use strict arg index
   return self~sendWinIntMsg(self~LVM_DELETEITEM, index, 0) <> 0

::method deleteAll
   use strict arg
   return self~sendWinIntMsg(self~LVM_DELETEALLITEMS, 0, 0) <> 0

::method edit
   use strict arg index
   if self~usingUnicode then return self~sendWinHandle2MsgH(self~LVM_EDITLABELW, index, 0)
   else return self~sendWinHandle2MsgH(self~LVM_EDITLABELA, index, 0)

::method endEdit
   use strict arg
   if self~usingUnicode then return self~sendWinHandle2MsgH(self~LVM_EDITLABELW, -1, 0)
   else return self~sendWinHandle2MsgH(self~LVM_EDITLABELA, -1, 0)

::method items
   return self~sendWinIntMsg(self~LVM_GETITEMCOUNT , 0, 0)

::method selectedItems
   return self~sendWinIntMsg(self~LVM_GETSELECTEDCOUNT , 0, 0)

::method redrawItems
   use strict arg first = 0, last = (self~items)
   return self~sendWinIntMsg(self~LVM_REDRAWITEMS, first, last) <> 0

::method updateItem
   use strict arg index
   return self~sendWinIntMsg(self~LVM_UPDATE, index, 0) <> 0

::method ensureVisible
   use strict arg index, partial = .false
   return self~sendWinIntMsg(self~LVM_ENSUREVISIBLE, index, partial) <> 0

::method deleteColumn
   use strict arg index
   return self~sendWinIntMsg(self~LVM_DELETECOLUMN, index, 0) <> 0

::method columnWidthPx
   use strict arg column
   return self~sendWinIntMsg(self~LVM_GETCOLUMNWIDTH, column, 0)

::method smallSpacing
   return self~sendWinIntMsg(self~LVM_GETITEMSPACING, 1, 0)

::method spacing
   return self~sendWinIntMsg(self~LVM_GETITEMSPACING, 0, 0)

::method setHoverTime
   use strict arg time = (-1)
   if time < -1 then time = -1
   return self~sendWinIntMsg(self~LVM_SETHOVERTIME, 0, time)

::method getHoverTime
   return self~sendWinIntMsg(self~LVM_GETHOVERTIME, 0, 0)

::method prepare4nItems
   use strict arg countItems
   return self~sendWinIntMsg(self~LVM_SETITEMCOUNT, countItems, 0)

::method scroll
   use strict arg x = 0, y = 0
   return self~sendWinIntMsg(self~LVM_SCROLL, x, y) <> 0

::method itemsPerPage
   return self~sendWinIntMsg(self~LVM_GETCOUNTPERPAGE, 0, 0)

::method arrange external "LIBRARY oodialog lv_arrange"
::method snapToGrid external "LIBRARY oodialog lv_arrange"
::method alignLeft external "LIBRARY oodialog lv_arrange"
::method alignTop external "LIBRARY oodialog lv_arrange"
::method addStyle external "LIBRARY oodialog lv_addRemoveStyle"
::method removeStyle external "LIBRARY oodialog lv_addRemoveStyle"
::method replaceStyle external "LIBRARY oodialog lv_replaceStyle"

::method stringWidthPx external "LIBRARY oodialog lv_stringWidthPx"
::method getColumnCount external "LIBRARY oodialog lv_getColumnCount"
::method getColumnOrder external "LIBRARY oodialog lv_getColumnOrder"
::method getColumnInfo  external "LIBRARY oodialog lv_getColumnInfo"
::method insertColumnPx external "LIBRARY oodialog lv_insertColumnPx"
::method modifyColumnPx external "LIBRARY oodialog lv_modifyColumnPx"
::method setColumnOrder external "LIBRARY oodialog lv_setColumnOrder"
::method setColumnWidthPx external "LIBRARY oodialog lv_setColumnWidthPx"

::method stringWidth  -- Dialog units, not accurate.
   use strict arg text
   return self~stringWidthPx(text) / self~factorX

::method columnInfo  -- Dialog units, not accurate.
   use strict arg index
   InternalLVColInfo. = self~getColumnInfo(index)
   if InternalLVColInfo.!Width~Datatype('N') = 1 then InternalLVColInfo.!Width = InternalLVColInfo.!Width / self~factorX
   return InternalLVColInfo.

::method columnWidth  -- Dialog units, not accurate.
   use strict arg colNum
   return self~columnWidthPx(colNum) / self~factorX

::method setColumnWidth  -- Dialog units, not accurate.
   use strict arg index, width = "AUTO"
   if width~datatype("W") then width = width * self~factorX
   return self~setColumnWidthPx(index, width)

::method modifyColumn  -- Dialog units, not accurate.
   use strict arg index, label = "", width = (-8), align = ""
   if arg(3,'E'), width~datatype('W') then width = width * self~factorX
   return self~modifyColumnPx(index, label, width, align)

::method insertColumn -- Dialog units, not accurate.
   use strict arg nr = 0, text, width, fmt = "L"
   width = trunc(width * self~factorX)
   return self~insertColumnPx(nr, text, width, fmt)

::method insert external "LIBRARY oodialog lv_insert"
::method add external "LIBRARY oodialog lv_add"
::method addRow external "LIBRARY oodialog lv_addRow"
::method addFullRow external "LIBRARY oodialog lv_addFullRow"  -- TODO review implementation before release
::method modify external "LIBRARY oodialog lv_modify"
::method setItemState external "LIBRARY oodialog lv_setItemState"
::method getExtendedStyle external "LIBRARY oodialog lv_getExtendedStyle"
::method getExtendedStyleRaw external "LIBRARY oodialog lv_getExtendedStyle"
::method replaceExtendedStyle external "LIBRARY oodialog lv_replaceExtendStyle"
::method addExtendedStyle external "LIBRARY oodialog lv_addClearExtendStyle"
::method removeExtendedStyle external "LIBRARY oodialog lv_addClearExtendStyle"
::method check external "LIBRARY oodialog lv_checkUncheck"
::method uncheck external "LIBRARY oodialog lv_checkUncheck"
::method isChecked external "LIBRARY oodialog lv_isChecked"
::method hasCheckBoxes external "LIBRARY oodialog lv_hasCheckBoxes"
::method getCheck external "LIBRARY oodialog lv_getCheck"

-- Intent is to return directory item with normal names and in itemInfo convert to old stem return
::method getItemInfo external "LIBRARY oodialog lv_getItemInfo"
::method itemInfo
   forward message "getItemInfo"

::method checkAll
   return self~check(-1)

::method uncheckAll
   return self~uncheck(-1)

::method last
   return self~items - 1

::method lastSelected
   if self~itemState(self~last)~wordpos("SELECTED") > 0 then  return self~last
   return self~previousSelected(self~last)

::method itemPos
   use strict arg item
   p = self~getItemPos(item)
   if p == 0 then return p
   else return p~x p~y

::method itemState external "LIBRARY oodialog lv_itemState"
::method itemText external "LIBRARY oodialog lv_itemText"
::method setItemText external "LIBRARY oodialog lv_setItemText"
::method next external "LIBRARY oodialog lv_getNextItem"
::method nextSelected external "LIBRARY oodialog lv_getNextItem"
::method nextLeft external "LIBRARY oodialog lv_getNextItem"
::method nextRight external "LIBRARY oodialog lv_getNextItem"
::method previous external "LIBRARY oodialog lv_getNextItem"
::method previousSelected external "LIBRARY oodialog lv_getNextItem"
::method selected external "LIBRARY oodialog lv_getNextItemWithState"
::method focused external "LIBRARY oodialog lv_getNextItemWithState"
::method dropHighlighted external "LIBRARY oodialog lv_getNextItemWithState"
::method find external "LIBRARY oodialog lv_find"
::method findPartial external "LIBRARY oodialog lv_find"
::method findNearestXY external "LIBRARY oodialog lv_findNearestXY"
::method select external "LIBRARY oodialog lv_setSpecificState"
::method deselect external "LIBRARY oodialog lv_setSpecificState"
::method focus external "LIBRARY oodialog lv_setSpecificState"
::method getItemPos external "LIBRARY oodialog lv_getItemPos"
::method setItemPos external "LIBRARY oodialog lv_setItemPos"
::method setImageList external "LIBRARY oodialog lv_setImageList"
::method getImageList external "LIBRARY oodialog lv_getImageList"

::method "bkColor=" external "LIBRARY oodialog lv_setColor"
::method "textColor=" external "LIBRARY oodialog lv_setColor"
::method "textBkColor=" external "LIBRARY oodialog lv_setColor"
::method bkColor external "LIBRARY oodialog lv_getColor"
::method textColor external "LIBRARY oodialog lv_getColor"
::method textBkColor external "LIBRARY oodialog lv_getColor"

-- DEPRECATED
::method setSmallImages
   newArgs = arg(1, 'A')
   newArgs[4] = 1
   forward message 'setImageList' arguments (newArgs)

-- DEPRECATED
::method setImages external "LIBRARY oodialog lv_setImageList"

-- DEPRECATED
::method removeSmallImages
   return self~removeImageList(.nil, 1)

-- DEPRECATED
::method removeImages
   return self~removeImageList(.nil, 0)


/************* Tree Control Class *********************************************/

::class 'TreeControl' subclass DialogControl public

::attribute rootArray private

::method init
   forward class (super) continue
   self~rootArray = .array~new(4)

::method subclassEdit external "LIBRARY oodialog generic_subclassEdit"
::method restoreEditClass external "LIBRARY oodialog generic_subclassEdit"

::method insert external "LIBRARY oodialog tv_insert"
::method modify external "LIBRARY oodialog tv_modify"
::method ItemInfo external "LIBRARY oodialog tv_itemInfo"
::method root external "LIBRARY oodialog tv_getSpecificItem"
::method selected external "LIBRARY oodialog tv_getSpecificItem"
::method dropHighlighted external "LIBRARY oodialog tv_getSpecificItem"
::method firstVisible external "LIBRARY oodialog tv_getSpecificItem"
::method parent external "LIBRARY oodialog tv_getNextItem"
::method child external "LIBRARY oodialog tv_getNextItem"
::method next external "LIBRARY oodialog tv_getNextItem"
::method nextVisible external "LIBRARY oodialog tv_getNextItem"
::method previous external "LIBRARY oodialog tv_getNextItem"
::method previousVisible external "LIBRARY oodialog tv_getNextItem"
::method collapse external "LIBRARY oodialog tv_expand"
::method collapseAndReset external "LIBRARY oodialog tv_expand"
::method expand external "LIBRARY oodialog tv_expand"
::method toggle external "LIBRARY oodialog tv_expand"
::method select external "LIBRARY oodialog tv_selectItem"
::method makeFirstVisible external "LIBRARY oodialog tv_selectItem"
::method dropHighlight external "LIBRARY oodialog tv_selectItem"
::method setImageList external "LIBRARY oodialog tv_setImageList"
::method getImageList external "LIBRARY oodialog tv_getImageList"

::method hitTestInfo external "LIBRARY oodialog tv_hitTestInfo"
::method hitTest
   forward message "hitTestInfo" continue
   info = result
   if info~hItem = 0 then return 0
   else return info~hItem info~location


-- TV_FIRST == 0x1100  (4352)
::constant TVM_DELETEITEM          "0x1101"
::constant TVM_GETCOUNT            "0x1105"
::constant TVM_GETINDENT           "0x1106"
::constant TVM_SETINDENT           "0x1107"
::constant TVM_GETVISIBLECOUNT     "0x1110"
::constant TVM_SORTCHILDREN        "0x1113"
::constant TVM_ENSUREVISIBLE       "0x1114"
::constant TVM_EDITLABELA          "0x110E"
::constant TVM_EDITLABELW          "0x1141"
::constant TVM_GETUNICODEFORMAT    "0x2006"
::constant TVM_ENDEDITLABELNOW     "0x1116"

::method items
   use strict arg
   return self~sendWinIntMsg(self~TVM_GETCOUNT, 0, 0)

::method visibleItems
   use strict arg
   return self~sendWinIntMsg(self~TVM_GETVISIBLECOUNT, 0, 0)

::method ensureVisible
   use strict arg hItem
   return self~sendWinHandle2Msg(self~TVM_ENSUREVISIBLE, 0, hItem)

-- The old code did not have recurse arg and had a comment saying "recursive not yet supported"
-- Although MSDN documents this message as recursively sorting the children if WPARAM is true,
-- it doesn't seem to work.
::method sortChildren
   use strict arg hitem, recurse = .false
   return self~sendWinHandle2Msg(self~TVM_SORTCHILDREN, recurse, hItem)

::method indent
   use strict arg
   return self~sendWinIntMsg(self~TVM_GETINDENT, 0, 0)

::method "indent="
   use strict arg indentAmount
   return self~sendWinIntMsg(self~TVM_SETINDENT, indentAmount, 0)

::method delete
   use strict arg hItem
   return self~sendWinHandle2Msg(self~TVM_DELETEITEM, 0, hItem)

::method deleteAll
   use strict arg
   return self~sendWinIntMsg(self~TVM_DELETEITEM, 0, 0)

::method edit
   use strict arg hitem
   if self~usingUnicode then return self~sendWinHandle2MsgH(self~TVM_EDITLABELW, 0, hItem)
   else return self~sendWinHandle2MsgH(self~TVM_EDITLABELA, 0, hItem)

::method endEdit
   use strict arg cancel = .false
   return (self~sendWinIntMsg(self~TVM_ENDEDITLABELNOW , cancel, 0) <> 0)

::method add
   do i = 1 to arg()
      if arg(i, 'E') then do
          if arg(i + 1, 'E') then image    = arg(i + 1); else image = -1
          if arg(i + 2, 'E') then selImage = arg(i + 2); else selImage = image
          if arg(i + 3, 'E') then opts     = arg(i + 3); else opts = ""
          if arg(i + 4, 'E') then children = arg(i + 4); else children = 0
          if i = 1 then do
              self~rootArray[i] = self~insert("ROOT", , arg(i), image, selimage, opts~translate, children)
              return self~rootArray[i]
          end
          else if self~rootArray~hasIndex(i - 1) then do
              if i = 1 then parent = "ROOT"; else parent = self~rootArray[i - 1]
              self~rootArray[i] = self~insert(parent, , arg(i), image, selimage, opts~translate, children)
              return self~rootArray[i]
          end
          else return 0
      end
   end

::method moveItem
   use strict arg hItem, hNewParent, redraw = .true, extended = ""
   if hItem = 0 then return 0
   if hNewParent = 0 then return 0

   extended = extended~translate
   if hItem = hNewParent | hNewParent = self~parent(hItem) | self~isAncestor(hItem, hNewParent) then return 0
   iinfo. = self~itemInfo(hItem)
   newRoot = self~insert(hNewParent, , iinfo.!Text, iinfo.!Image, iinfo.!SelectedImage, iinfo.!State, iinfo.!Children)

   if iinfo.!Children = 1 then do
      child = self~child(hItem)
      if child \= 0 then self~moveItem(child, newRoot, 0, "SIBLINGS")
   end

   if extended = "SIBLINGS" then do
      sibling = self~next(hItem)
      do while sibling \= 0
         self~moveItem(sibling, hNewParent, 0, "NODELETE")
         oldSib = sibling
         sibling = self~next(sibling)
         self~delete(oldSib)
      end
   end

   if extended \= "NODELETE" then self~delete(hItem)
   if redraw then self~update
   return newRoot

::method isAncestor
   use strict arg hParent, hItem
   if arg(1, 'O') | hParent = 0 then return .false
   if arg(2, 'O') | hItem = 0 then return .false
   if hItem = hParent then return .false
   ancestor = self~parent(hItem)
   do while ancestor \= 0 & ancestor \= hParent
       ancestor = self~Parent(ancestor)
   end
   if ancestor = hParent then return .true
   else return .false

-- DEPRECATED
::method setImages external "LIBRARY oodialog tv_setImageList"

-- DEPRECATED
::method removeImages
   return self~setImageList(.nil, 0)


::class 'DateTimePicker' subclass DialogControl public

::constant DTM_GETMCFONT           "0x100A"
::constant DTM_SETMCFONT           "0x1009"

::method closeMonthCal external "LIBRARY oodialog dtp_closeMonthCal"
::method getDateTime external "LIBRARY oodialog dtp_getDateTime"
::method getInfo external "LIBRARY oodialog dtp_getInfo"
::method getIdealSize external "LIBRARY oodialog dtp_getIdealSize"
::method getMonthCal external "LIBRARY oodialog dtp_getMonthCal"
::method getMonthCalColor external "LIBRARY oodialog dtp_getMonthCalColor"

::method getMonthCalFont
   use strict arg
   return self~sendWinIntMsgH(self~DTM_GETMCFONT, 0, 0)

::method getMonthCalStyle external "LIBRARY oodialog dtp_getMonthCalStyle"
::method getRange external "LIBRARY oodialog dtp_getRange"

::method setDateTime external "LIBRARY oodialog dtp_setDateTime"
::method setFormat external "LIBRARY oodialog dtp_setFormat"
::method setMonthCalColor external "LIBRARY oodialog dtp_setMonthCalColor"

::method setMonthCalFont
   use strict arg hFont, redraw = .true
   return self~sendWinHandleMsg(self~DTM_SETMCFONT, hFont, redraw)

::method setMonthCalStyle external "LIBRARY oodialog dtp_setMonthCalStyle"
::method setRange external "LIBRARY oodialog dtp_setRange"


::class 'MonthCalendar' subclass DialogControl public

::constant MCM_GETMAXSELCOUNT      "0x1003"
::constant MCM_SETMAXSELCOUNT      "0x1004"
::constant MCM_GETMAXTODAYWIDTH    "0x1015"
::constant MCM_GETMONTHDELTA       "0x1013"
::constant MCM_SETMONTHDELTA       "0x1014"

::attribute date external "LIBRARY oodialog _mc_date"

::method addStyle external "LIBRARY oodialog mc_addRemoveStyle"
::method removeStyle external "LIBRARY oodialog mc_addRemoveStyle"
::method replaceStyle external "LIBRARY oodialog mc_replaceStyle"

::method getCalendarBorder external "LIBRARY oodialog mc_getCalendarBorder"
::method getCalendarCount external "LIBRARY oodialog mc_getCalendarCount"
::method getCALID external "LIBRARY oodialog mc_getCALID"
::method getColor external "LIBRARY oodialog mc_getColor"
::method getCurrentView external "LIBRARY oodialog mc_getCurrentView"
::method getFirstDayOfWeek external "LIBRARY oodialog mc_getFirstDayOfWeek"
::method getGridInfo external "LIBRARY oodialog mc_getGridInfo"

::method getMaxSelection
   use strict arg
   return self~sendWinIntMsg(self~MCM_GETMAXSELCOUNT, 0, 0)

::method getMaxTodayWidth
   use strict arg
   return self~sendWinIntMsg(self~MCM_GETMAXTODAYWIDTH, 0, 0)

::method getMinRect external "LIBRARY oodialog mc_getMinRect"

::method getMonthDelta
   use strict arg
   return self~sendWinIntMsg(self~MCM_GETMONTHDELTA, 0, 0)

::method getMonthRange external "LIBRARY oodialog mc_getMonthRange"
::method getRange external "LIBRARY oodialog mc_getRange"
::method getSelectionRange external "LIBRARY oodialog mc_getSelectionRange"
::method getToday external "LIBRARY oodialog mc_getToday"
::method hitTest external "LIBRARY oodialog mc_hitTest"
::method setCalendarBorder external "LIBRARY oodialog mc_setCalendarBorder"
::method setCALID external "LIBRARY oodialog mc_setCALID"
::method setColor external "LIBRARY oodialog mc_setColor"
::method setCurrentView external "LIBRARY oodialog mc_setCurrentView"
::method setDayState external "LIBRARY oodialog mc_setDayState"
::method setDayStateQuick external "LIBRARY oodialog mc_setDayStateQuick"
::method setFirstDayOfWeek external "LIBRARY oodialog mc_setFirstDayOfWeek"

::method setMaxSelection
   use strict arg count
   return self~sendWinIntMsg(self~MCM_SETMAXSELCOUNT, count, 0) <> 0

::method setMonthDelta
   use strict arg amount
   return self~sendWinIntMsg(self~MCM_SETMAXTODAYWIDTH, amount, 0)

::method setRange external "LIBRARY oodialog mc_setRange"
::method setSelectionRange external "LIBRARY oodialog mc_setSelectionRange"
::method setToday external "LIBRARY oodialog mc_setToday"
::method sizeRectToMin external "LIBRARY oodialog mc_sizeRectToMin"


::class 'UpDown' subclass DialogControl public

::constant UDM_SETBUDDY            "0x0469"
::constant UDM_GETBUDDY            "0x046A"
::constant UDM_SETBASE             "0x046D"
::constant UDM_GETBASE             "0x046E"
::constant UDM_SETPOS32            "0x0471"

::method deltaPosReply class external "LIBRARY oodialog ud_deltaPosReply_cls"

::method getBase
   use strict arg
   return self~sendWinIntMsg(self~UDM_GETBASE, 0, 0)

::method setBase
   use strict arg newBase
   return self~sendWinIntMsg(self~UDM_SETBASE, newBase, 0)

::method getBuddy
   use strict arg
   return self~sendWinIntMsgH(self~UDM_GETBUDDY, 0, 0)

::method setBuddy
   use strict arg hwndBuddy
   return self~sendWinHandleMsgH(self~UDM_SETBUDDY, hwndBuddy, 0)

::method getPosition external "LIBRARY oodialog ud_getPosition"
::method setPosition
   use strict arg newPos
   return self~sendWinIntMsg(self~UDM_SETPOS32, 0, newPos)

::method getRange external "LIBRARY oodialog ud_getRange"
::method setRange external "LIBRARY oodialog ud_setRange"
::method getAcceleration external "LIBRARY oodialog ud_getAcceleration"
::method setAcceleration external "LIBRARY oodialog ud_setAcceleration"
