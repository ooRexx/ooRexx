/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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

::constant CCM_GETUNICODEFORMAT     "0x2006"
::constant CCM_GETVERSION           "0x2008"
::constant CCM_SETUNICODEFORMAT     "0x2005"
::constant CCM_SETVERSION           "0x2007"

::method new class external "LIBRARY oodialog dlgctrl_new_cls"
::method init class external "LIBRARY oodialog dlgctrl_init_cls"

::attribute id get unguarded    -- The numerical resource ID for this control
::attribute hDlg get unguarded  -- The window handle of that dialog
::attribute oDlg get unguarded  -- The ooRexx dialog object this control belongs to

::method init external "LIBRARY oodialog dlgctrl_init"
::method unInit external "LIBRARY oodialog dlgctrl_unInit"

::method addUserSubclass unguarded external "LIBRARY oodialog dlgctrl_addUserSubclass" -- Do not document for 4.2.0
::method assignFocus unguarded external "LIBRARY oodialog dlgctrl_assignFocus"
::method clearRect unguarded external "LIBRARY oodialog dlgctrl_clearRect"
::method connectCharEvent unguarded
  use strict arg methodName = 'onChar'
  return self~connectEvent('CHAR', methodName)

::method connectEvent unguarded external "LIBRARY oodialog dlgctrl_connectEvent"       -- Do not document for 4.2.0
::method connectFKeyPress unguarded external "LIBRARY oodialog dlgctrl_connectFKeyPress"
::method connectKeyPress unguarded external "LIBRARY oodialog dlgctrl_connectKeyPress"
::method data unguarded external "LIBRARY oodialog dlgctrl_data"
::method "data=" unguarded external "LIBRARY oodialog dlgctrl_dataEquals"
::method disconnectKeyPress unguarded external "LIBRARY oodialog dlgctrl_disconnectKeyPress"
::method hasKeyPressConnection unguarded external "LIBRARY oodialog dlgctrl_hasKeyPressConnection"
::method getTextSizeDlg unguarded external "LIBRARY oodialog dlgctrl_getTextSizeDlg"
::method group unguarded external "LIBRARY oodialog dlgctrl_tabGroup"
::method putInBag unguarded external "LIBRARY oodialog dlgctrl_putInBag"  -- Internal use only.
::method redrawRect unguarded external "LIBRARY oodialog dlgctrl_redrawRect"
::method setColor unguarded external "LIBRARY oodialog dlgctrl_setColor"
::method setParent unguarded external "LIBRARY oodialog dlgctrl_setParent"
::method setSysColor unguarded external "LIBRARY oodialog dlgctrl_setColor"
::method setWindowTheme unguarded external "LIBRARY oodialog dlgctrl_setWindowTheme"
::method tabStop unguarded external "LIBRARY oodialog dlgctrl_tabGroup"
::method textSize unguarded external "LIBRARY oodialog dlgctrl_textSize"
::method useUnicode unguarded
   use strict arg doUseUnicode
   return self~sendWinIntMsg(self~CCM_SETUNICODEFORMAT, doUseUnicode, 0)

::method useVersion unguarded
   use strict arg version
   return self~sendWinIntMsg(self~CCM_SETVERSION, version, 0)

::method usingUnicode unguarded
   use strict arg
   return (self~sendWinIntMsg(self~CCM_GETUNICODEFORMAT, 0, 0) <> 0)

::method usingVersion unguarded
   use strict arg
   return self~sendWinIntMsg(self~CCM_GETVERSION, 0, 0)

-- DEPRECATED START
::method captureMouse unguarded
   mouse = .Mouse~new(self)
   return mouse~capture

::method getFocus unguarded
   forward to (self~oDlg)

::method getMouseCapture unguarded
   use strict arg
   mouse = .Mouse~new(self)
   return mouse~getCapture

::method getTextSize unguarded
   forward message "getTextSizeDlg" continue
   return result~width result~height

::method isMouseButtonDown unguarded
   use strict arg whichButton = "LEFT"
   mouse = .Mouse~new(self)
   return mouse~isButtonDown(whichButton)

::method processMessage
   forward message "sendMessage"

::method releaseMouseCapture unguarded
   use strict arg
   mouse = .Mouse~new(self)
   return mouse~releaseCapture

::method setFocus unguarded
   forward to (self~oDlg)

::method tabToPrevious unguarded
   forward to (self~oDlg)

::method tabToNext unguarded
   forward to (self~oDlg)

::method value unguarded external "LIBRARY oodialog dlgctrl_data"
::method "value=" unguarded external "LIBRARY oodialog dlgctrl_dataEquals"
-- DEPRECATED END


/******** Button Classes ******************************************************/

::class 'ButtonControl' subclass DialogControl public
::class 'Button' subclass ButtonControl public

::constant BM_CLICK              "0x00F5"

::method click unguarded
   forward message "push"

::method getIdealSize unguarded external "LIBRARY oodialog bc_getIdealSize"
::method getImage unguarded external "LIBRARY oodialog bc_getImage"
::method getImageList unguarded external "LIBRARY oodialog bc_getImageList"
::method getTextMargin unguarded external "LIBRARY oodialog bc_getTextMargin"
::method push unguarded
   return self~sendWinIntMsg(self~BM_CLICK, 0, 0)

::method setImage unguarded external "LIBRARY oodialog bc_setImage"
::method setImageList unguarded external "LIBRARY oodialog bc_setImageList"
::method setTextMargin unguarded external "LIBRARY oodialog bc_setTextMargin"
::method state unguarded external "LIBRARY oodialog bc_getState"
::method "state=" unguarded external "LIBRARY oodialog bc_setState"
::method "style=" unguarded external "LIBRARY oodialog bc_setStyle"

-- Bitmap button methods ALL DEPRECATED.

-- Almost all the bitmap button methods were implemented in DialogExtensions.
-- The whole 'bitmap' button idea is antiquated, so the DialogExtensions methods
-- are documented, the button methods are deprecated and not documented.
::method changeBitmap unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~ID
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) message "CHANGEBITMAPBUTTON" arguments (newarg)

::method dimBitmap unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~ID
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) arguments (newarg)

::method displaceBitmap unguarded
   use strict arg x, y
   return self~oDlg~setBitmapPosition(self~ID, x, y)

::method drawBitmap unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+2)
   newarg[1] = self~hwnd
   newarg[2] = self~id
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+2] = arga[i]; end
   forward to (self~oDlg) arguments (newarg)

::method getBitmapPosition unguarded
   use strict arg pos
   return self~oDlg~getBitmapPosition(self~ID, pos)

::method getBitmapSizeX unguarded
   return self~oDlg~getBitmapSizeX(self~ID)

::method getBitmapSizeY unguarded
   return self~oDlg~getBitmapSizeY(self~ID)

::method getBmpDisplacement unguarded
   return self~oDlg~getBmpDisplacement(self~ID)

::method scroll unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~ID
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) message "SCROLLBUTTON" arguments (newarg)

::method scrollBitmapFromTo unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~id
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) arguments (newarg)

::method scrollText unguarded
   arga = arg(1,"A")
   newarg = .array~new(arga~Items+1)
   newarg[1] = self~hwnd
   do i = 1 to arga~Items; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end
   forward to (self~oDlg) arguments (newarg)

::method setBitmapPosition unguarded
   use strict arg x, y = 0
   if arg(2, "E") then return self~oDlg~setBimapPosition(self~id, x, y)
   else return self~oDlg~setBimapPosition(self~id, x)

-- Bitmap button methods END ALL DEPRECATED.

/* For internal testing only, do not use this method */
::method test external "LIBRARY oodialog bc_test"
::method test class external "LIBRARY oodialog bc_test_cls"


::class 'RadioButton' subclass Button public
::method checkInGroup class external "LIBRARY oodialog rb_checkInGroup_cls"

::method check unguarded external "LIBRARY oodialog rb_check"
::method checked unguarded external "LIBRARY oodialog rb_checked"
::method getCheckState unguarded external "LIBRARY oodialog rb_getCheckState"
::method uncheck unguarded external "LIBRARY oodialog rb_uncheck"

-- DEPRECATED
::method isChecked external "LIBRARY oodialog rb_isChecked"

-- DEPRECATED
::method indeterminate external "LIBRARY oodialog rb_indeterminate"

::class 'CheckBox' subclass RadioButton public

::method isIndeterminate unguarded external "LIBRARY oodialog ckbx_isIndeterminate"
::method setIndeterminate unguarded external "LIBRARY oodialog ckbx_setIndeterminate"


/******** ComboBox Class ******************************************************/

::class 'ComboBox' subclass DialogControl public

::constant CB_SETEDITSEL               "0x0142"
::constant CB_DELETESTRING             "0x0144"
::constant CB_GETCOUNT                 "0x0146"
::constant CB_GETCURSEL                "0x0147"
::constant CB_RESETCONTENT             "0x014B"
::constant CB_SETCURSEL                "0x014E"
::constant CB_SHOWDROPDOWN             "0x014F"
::constant CB_GETDROPPEDSTATE          "0x0157"
::constant CB_GETTOPINDEX              "0x015b"
::constant CB_GETHORIZONTALEXTENT      "0x015d"
::constant CB_SETHORIZONTALEXTENT      "0x015e"

::method add unguarded external "LIBRARY oodialog cb_add"
::method addDirectory unguarded external "LIBRARY oodialog cb_addDirectory"
::method closeDropDown unguarded
   use strict arg
   return self~sendWinIntMsg(self~CB_SHOWDROPDOWN, 0, 0)

::method delete unguarded
   use strict arg index = (self~selectedIndex)
   ret = self~sendWinIntMsg(self~CB_DELETESTRING, index - 1, 0)
   if ret == 0 then self~deleteAll
   return ret

::method deleteAll unguarded
   use strict arg
   return self~sendWinIntMsg(self~CB_RESETCONTENT, 0, 0)

::method find unguarded external "LIBRARY oodialog cb_find"
::method getCue unguarded external "LIBRARY oodialog cb_getCue"
::method getComboBoxInfo unguarded external "LIBRARY oodialog cb_getComboBoxInfo"
::method getEditControl unguarded external "LIBRARY oodialog cb_getEditControl"
::method getFirstVisible unguarded
   return self~sendWinIntMsg(self~CB_GETTOPINDEX, 0, 0) + 1

::method getHorizontalExtent unguarded
   use strict arg
   return self~sendWinIntMsg(self~CB_GETHORIZONTALEXTENT, 0, 0)

::method getItemData unguarded external "LIBRARY oodialog cb_getItemData"
::method getItemHeight unguarded external "LIBRARY oodialog cb_getItemHeight"
::method getMinVisible unguarded external "LIBRARY oodialog cb_getMinVisible"
::method getText unguarded external "LIBRARY oodialog cb_getText"
::method insert unguarded external "LIBRARY oodialog cb_insert"
::method isDropDown unguarded external "LIBRARY oodialog cb_isDropDown"
::method isDropDownList unguarded external "LIBRARY oodialog cb_isDropDown"
::method isDropDownOpen unguarded
   use strict arg
   return self~sendWinIntMsg(self~CB_GETDROPPEDSTATE, 0, 0)

::method isGrandchild unguarded external "LIBRARY oodialog cb_isGrandchild"
::method isSimple unguarded external "LIBRARY oodialog cb_isDropDown"
::method items unguarded
   use strict arg
   return self~sendWinIntMsg(self~CB_GETCOUNT, 0, 0)

::method modify unguarded
   use strict arg index = (self~selectedIndex), newEntry
   if index <= 0 then return -1
   self~delete(index)
   return self~insert(index, newEntry)

::method openDropDown unguarded
   use strict arg
   return self~sendWinIntMsg(self~CB_SHOWDROPDOWN, 1, 0)

::method removeFullColor unguarded external "LIBRARY oodialog cb_removeFullColor"
::method removeItemData unguarded external "LIBRARY oodialog cb_removeItemData"
::method select unguarded external "LIBRARY oodialog cb_select"
::method selected unguarded
   use strict arg
   return self~getText(self~selectedIndex)

::method selectedIndex unguarded
   use strict arg
   return self~sendWinIntMsg(self~CB_GETCURSEL, 0, 0) + 1

::method selectIndex unguarded
   use strict arg ndx = 0
   ret = self~sendWinIntMsg(self~CB_SETCURSEL, ndx - 1, 0)
   if ret == -1, ndx <> 0 then return -1
   return ret + 1

::method setCue unguarded external "LIBRARY oodialog cb_setCue"
::method setEditSelection unguarded
   use strict arg startndx = 0, endndx = 0
   if endndx == 0 then endndx = -1
   lParam = .DlgUtil~makeLParam(startndx - 1, endndx)
   return self~sendWinIntMsg(self~CB_SETEDITSEL, 0, lParam) == -1

::method setFullColor unguarded external "LIBRARY oodialog cb_setFullColor"
::method setHorizontalExtent unguarded
   use strict arg extent
   self~sendWinIntMsg(self~CB_SETHORIZONTALEXTENT, extent, 0)
   return 0

::method setItemData unguarded external "LIBRARY oodialog cb_setItemData"
::method setItemHeight unguarded external "LIBRARY oodialog cb_setItemHeight"
::method setMinVisible unguarded external "LIBRARY oodialog cb_setMinVisible"

-- DEPRECATED
::method editSelection
   forward message "setEditSelection"



/******** DateTimePicker Class ************************************************/

::class 'DateTimePicker' subclass DialogControl public

::constant DTM_GETMCFONT           "0x100A"
::constant DTM_SETMCFONT           "0x1009"

::method closeMonthCal unguarded external "LIBRARY oodialog dtp_closeMonthCal"
::method getDateTime unguarded external "LIBRARY oodialog dtp_getDateTime"
::method getInfo unguarded external "LIBRARY oodialog dtp_getInfo"
::method getIdealSize unguarded external "LIBRARY oodialog dtp_getIdealSize"
::method getMonthCal unguarded external "LIBRARY oodialog dtp_getMonthCal"
::method getMonthCalColor unguarded external "LIBRARY oodialog dtp_getMonthCalColor"

::method getMonthCalFont unguarded
   use strict arg
   return self~sendWinIntMsgH(self~DTM_GETMCFONT, 0, 0)

::method getMonthCalStyle unguarded external "LIBRARY oodialog dtp_getMonthCalStyle"
::method getRange unguarded external "LIBRARY oodialog dtp_getRange"

::method setDateTime unguarded external "LIBRARY oodialog dtp_setDateTime"
::method setFormat unguarded external "LIBRARY oodialog dtp_setFormat"
::method setMonthCalColor unguarded external "LIBRARY oodialog dtp_setMonthCalColor"

::method setMonthCalFont unguarded
   use strict arg hFont, redraw = .true
   return self~sendWinHandleMsg(self~DTM_SETMCFONT, hFont, redraw)

::method setMonthCalStyle unguarded external "LIBRARY oodialog dtp_setMonthCalStyle"
::method setRange unguarded external "LIBRARY oodialog dtp_setRange"


/******** Edit Class **********************************************************/

::class 'EditControl' subclass DialogControl public
::class 'Edit' subclass EditControl public

::constant EM_SETSEL               "0x00B1"
::constant EM_SCROLL               "0x00B5"
::constant EM_LINESCROLL           "0x00B6"
::constant EM_SCROLLCARET          "0x00B7"
::constant EM_GETMODIFY            "0x00B8"
::constant EM_SETMODIFY            "0x00B9"
::constant EM_GETLINECOUNT         "0x00BA"
::constant EM_LINELENGTH           "0x00C1"
::constant EM_LIMITTEXT            "0x00C5"
::constant EM_CANUNDO              "0x00C6"
::constant EM_UNDO                 "0x00C7"
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

::constant WM_CUT                  "0x0300"
::constant WM_COPY                 "0x0301"
::constant WM_PASTE                "0x0302"
::constant WM_CLEAR                "0x0303"

::attribute passwordChar unguarded set
    use strict arg char
    ch = char~c2d
    if self~isSingleLine, ch > 0, ch < 256 then self~sendWinIntMsg(self~EM_SETPASSWORDCHAR, ch, 0)

::attribute passwordChar unguarded get
    if \ self~isSingleLine then return ""
    char = self~sendWinIntMsg(self~EM_GETPASSWORDCHAR, 0, 0)
    if char \= 0 then return char~d2c
    else return ""

::method addStyle unguarded external "LIBRARY oodialog e_style"
::method canUndo unguarded
    use strict arg
    return self~sendWinIntMsg(self~EM_CANUNDO, 0, 0) != 0

::method clearText unguarded
    use strict arg
    self~sendWinIntMsg(self~WM_CLEAR, 0, 0)
    return 0

::method copyText unguarded
    use strict arg
    self~sendWinIntMsg(self~WM_COPY, 0, 0)
    return 0

::method cutText unguarded
    use strict arg
    self~sendWinIntMsg(self~WM_CUT, 0, 0)
    return 0

::method disableInternalResize unguarded
  use strict arg
  return self~connectEvent('WM_SIZE')

::method ensureCaretVisibility unguarded
    use strict arg
    self~sendWinIntMsg(self~EM_SCROLLCARET, 0, 0)
    return 0

::method firstVisibleLine unguarded
    use strict arg
    return self~sendWinIntMsg(self~EM_GETFIRSTVISIBLELINE, 0, 0) + 1

::method getCue unguarded external "LIBRARY oodialog e_getCue"
::method getLine unguarded external "LIBRARY oodialog e_getLine"
::method getMargins unguarded
    use strict arg
    marg = self~sendWinIntMsg(self~EM_GETMARGINS, 0, 0)
    m = .directory~new
    m~right = .DlgUtil~hiWord(marg)
    m~left = .DlgUtil~loWord(marg)
    return m

::method getRect unguarded external "LIBRARY oodialog e_getRect"
::method getStyle unguarded external "LIBRARY oodialog e_style"
::method hideBalloon unguarded external "LIBRARY oodialog e_hideBallon"
::method isGrandChild unguarded external "LIBRARY oodialog e_isGrandchild"
::method isModified unguarded
    use strict arg
    return self~sendWinIntMsg(self~EM_GETMODIFY, 0, 0) <> 0

::method isSingleLine unguarded external "LIBRARY oodialog e_isSingleLine"
::method lineScroll unguarded
    use strict arg cx, cy
    if self~isSingleLine then return 1
    self~sendWinIntMsg(self~EM_LINESCROLL, cx, cy)
    return 0

::method lineFromIndex unguarded external "LIBRARY oodialog e_lineFromIndex"
::method lineIndex unguarded external "LIBRARY oodialog e_lineIndex"
::method lineLength unguarded external "LIBRARY oodialog e_lineLength"
::method lines unguarded
    use strict arg
    return self~sendWinIntMsg(self~EM_GETLINECOUNT, 0, 0)

::method margins unguarded
    m = self~getMargins
    return m~left m~right

::method noContextMenu unguarded
  use strict arg
  return self~connectEvent('CONTEXTMENU')

::method pasteText unguarded
    use strict arg
    self~sendWinIntMsg(self~WM_PASTE, 0, 0)
    return 0

::method removeStyle unguarded external "LIBRARY oodialog e_style"
::method replaceSelText unguarded external "LIBRARY oodialog e_replaceSelText"
::method replaceStyle unguarded external "LIBRARY oodialog e_style"
::method scrollCommand unguarded
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

::method select unguarded
    use strict arg start = 1, end = 0
    self~sendWinIntMsg(self~EM_SETSEL, start - 1, end - 1)
    return 0

::method selected unguarded
    s = self~selection
    return s~startChar s~endChar

::method selection unguarded external "LIBRARY oodialog e_selection"
::method setCue unguarded external "LIBRARY oodialog e_setCue"
::method setLimit unguarded
    use strict arg limit
    self~sendWinUintMsg(self~EM_LIMITTEXT, limit, 0)
    return 0

::method setMargins unguarded
    use arg left, right
    if arg(1, 'O'), arg(2, 'O') then do
       self~sendWinIntMsg(self~EM_SETMARGINS, 0, self~EC_USEFONTINFO)
    end
    else do
       flag = 0
       if arg(1, 'E') then flag = self~EC_LEFTMARGIN
       if arg(2, 'E') then flag = .DlgUtil~or(flag, self~EC_RIGHTMARGIN)
       lParam = .DlgUtil~makeLParam(left, right)
       self~sendWinIntMsg(self~EM_SETMARGINS, flag, lParam)
    end
    return 0

::method setModified unguarded
    use strict arg bool = .true
    self~sendWinIntMsg(self~EM_SETMODIFY, bool, 0)
    return 0

::method setReadOnly unguarded
    use strict arg bool = .true
    return self~sendWinIntMsg(self~EM_SETREADONLY, bool, 0) <> 0

::method setRect unguarded external "LIBRARY oodialog e_setRect"
::method setTabStops unguarded external "LIBRARY oodialog e_setTabStops"
::method showBalloon unguarded external "LIBRARY oodialog e_showBallon"

::method "tab=" unguarded
    use strict arg dlgunits
    self~setTabStops(.array~of(dlgunits))

::method undo unguarded
    use strict arg
    return self~sendWinIntMsg(self~EM_UNDO, 0, 0) != 0

::method wantReturn unguarded
  use strict arg mthName = 'onReturn'
  return self~connectEvent('WANTRETURN', mthName)



/******** GroupBox Class ******************************************************/

::class 'GroupBox' subclass DialogControl public
::method "style=" unguarded external "LIBRARY oodialog gb_setStyle"


/******** ListBox Class *******************************************************/

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

::method add unguarded external "LIBRARY oodialog lb_add"
::method addDirectory unguarded external "LIBRARY oodialog lb_addDirectory"
::method "columnWidth=" unguarded -- Dialog units, not accurate
   use strict arg dlgunits
   if dlgunits~datatype("N") = 0 then return
   self~columnWidthPx = trunc(dlgunits * self~factorX)

::method "columnWidthPx=" unguarded
   use strict arg pixels
   self~sendWinIntMsg(self~LB_SETCOLUMNWIDTH, pixels, 0)

::method delete unguarded
   use strict arg index = (self~selectedIndex)
   if index == 0 then return -1
   return self~sendWinIntMsg(self~LB_DELETESTRING, index - 1, 0)

::method deleteAll unguarded
   self~sendWinIntMsg(self~LB_RESETCONTENT, 0, 0)
   return 0

::method deSelectIndex unguarded external "LIBRARY oodialog lb_deselectIndex"
::method deselectRange unguarded
   use strict arg fromNdx = 1, toNdx = (self~items)
   if self~isSingleSelection then return -1
   lParam = .DlgUtil~makeLParam(fromNdx - 1, toNdx - 1)
   return self~sendWinIntMsg(self~LB_SELITEMRANGE, .false, lParam)

::method find unguarded external "LIBRARY oodialog lb_find"
::method getFirstVisible unguarded
   return self~sendWinIntMsg(self~LB_GETTOPINDEX, 0, 0) + 1

::method getText unguarded external "LIBRARY oodialog lb_getText"
::method hitTestInfo unguarded external "LIBRARY oodialog lb_hitTestInfo"
::method insert unguarded external "LIBRARY oodialog lb_insert"
::method isSingleSelection unguarded external "LIBRARY oodialog lb_isSingleSelection"
::method itemHeight unguarded     -- Dialog units, not accurate
   return self~itemHeightPx / self~factorY

::method "itemHeight=" unguarded  -- Dialog units, not accurate
   use strict arg dlgunits
   if dlgunits~datatype("N") = 0 then return
   self~itemHeightPx = trunc(dlgunits * self~factorY)

::method itemHeightPx unguarded
   return self~sendWinIntMsg(self~LB_GETITEMHEIGHT, 0, 0)

::method "itemHeightPx=" unguarded
   use strict arg pixels
   self~sendWinIntMsg(self~LB_SETITEMHEIGHT, pixels, 0)

::method items unguarded
   return self~sendWinIntMsg(self~LB_GETCOUNT, 0, 0)

::method makeFirstVisible unguarded
   use strict arg ndx = 1
   return self~sendWinIntMsg(self~LB_SETTOPINDEX, ndx - 1, 0)

::method modify unguarded
   use strict arg index = (self~selectedIndex), newItem
   if index <= 0 then return -1
   self~delete(index)
   return self~insert(index, newItem)

::method select unguarded external "LIBRARY oodialog lb_select"
::method selected unguarded
   return self~getText(self~selectedIndex)

::method selectedIndex unguarded external "LIBRARY oodialog lb_selectedIndex"
::method selectedItems unguarded
   return self~sendWinIntMsg(self~LB_GETSELCOUNT, 0, 0)

::method selectIndex unguarded external "LIBRARY oodialog lb_selectIndex"
::method selectedIndexes unguarded                                    -- TODO be nice to return an array
   return self~oDlg~getListBoxData(self~id)

::method selectRange unguarded
   use strict arg fromNdx = 1, toNdx = (self~items)
   if self~isSingleSelection then return -1
   lParam = .DlgUtil~makeLParam(fromNdx - 1, toNdx - 1)
   return self~sendWinIntMsg(self~LB_SELITEMRANGE, .true, lParam)

::method setTabulators unguarded external "LIBRARY oodialog generic_setListTabulators"
::method setWidth unguarded       -- Dialog units, not accurate
   use strict arg dlgunits
   if dlgunits~datatype("N") = 0 then return -1
   px = trunc(dlgunits * self~factorX)
   self~setWidthPx(px)

::method setWidthPx unguarded
   use strict arg pixels
   self~sendWinIntMsg(self~LB_SETHORIZONTALEXTENT, pixels, 0)
   return 0

::method width unguarded          -- Dialog units, not accurate
   return self~widthPx / self~factorX

::method widthPx unguarded
   return self~sendWinIntMsg(self~LB_GETHORIZONTALEXTENT, 0, 0)



/******** MonthCalendar Class *************************************************/

::class 'MonthCalendar' subclass DialogControl public

::constant MCM_GETMAXSELCOUNT      "0x1003"
::constant MCM_SETMAXSELCOUNT      "0x1004"
::constant MCM_GETMAXTODAYWIDTH    "0x1015"
::constant MCM_GETMONTHDELTA       "0x1013"
::constant MCM_SETMONTHDELTA       "0x1014"

::attribute date unguarded external "LIBRARY oodialog _mc_date"

::method addStyle unguarded external "LIBRARY oodialog mc_addRemoveStyle"
::method getCalendarBorder unguarded external "LIBRARY oodialog mc_getCalendarBorder"
::method getCalendarCount unguarded external "LIBRARY oodialog mc_getCalendarCount"
::method getCALID unguarded external "LIBRARY oodialog mc_getCALID"
::method getColor unguarded external "LIBRARY oodialog mc_getColor"
::method getCurrentView unguarded external "LIBRARY oodialog mc_getCurrentView"
::method getFirstDayOfWeek unguarded external "LIBRARY oodialog mc_getFirstDayOfWeek"
::method getGridInfo unguarded external "LIBRARY oodialog mc_getGridInfo"
::method getMaxSelection unguarded
   use strict arg
   return self~sendWinIntMsg(self~MCM_GETMAXSELCOUNT, 0, 0)

::method getMaxTodayWidth unguarded
   use strict arg
   return self~sendWinIntMsg(self~MCM_GETMAXTODAYWIDTH, 0, 0)

::method getMinRect unguarded external "LIBRARY oodialog mc_getMinRect"
::method getMonthDelta unguarded
   use strict arg
   return self~sendWinIntMsg(self~MCM_GETMONTHDELTA, 0, 0)

::method getMonthRange unguarded external "LIBRARY oodialog mc_getMonthRange"
::method getRange unguarded external "LIBRARY oodialog mc_getRange"
::method getSelectionRange unguarded external "LIBRARY oodialog mc_getSelectionRange"
::method getStyle unguarded external "LIBRARY oodialog mc_getStyle"
::method getToday unguarded external "LIBRARY oodialog mc_getToday"
::method hitTest unguarded external "LIBRARY oodialog mc_hitTestInfo"  -- Error in 4.2.0 docs, name should have been hitTestInfo. Need to keep for backwards compatibility
::method hitTestInfo unguarded external "LIBRARY oodialog mc_hitTestInfo"
::method removeStyle unguarded external "LIBRARY oodialog mc_addRemoveStyle"
::method replaceStyle unguarded external "LIBRARY oodialog mc_replaceStyle"
::method setCalendarBorder unguarded external "LIBRARY oodialog mc_setCalendarBorder"
::method setCALID unguarded external "LIBRARY oodialog mc_setCALID"
::method setColor unguarded external "LIBRARY oodialog mc_setColor"
::method setCurrentView unguarded external "LIBRARY oodialog mc_setCurrentView"
::method setDayState unguarded external "LIBRARY oodialog mc_setDayState"
::method setDayStateQuick unguarded external "LIBRARY oodialog mc_setDayStateQuick"
::method setFirstDayOfWeek unguarded external "LIBRARY oodialog mc_setFirstDayOfWeek"
::method setMaxSelection unguarded
   use strict arg count
   return self~sendWinIntMsg(self~MCM_SETMAXSELCOUNT, count, 0) <> 0

::method setMonthDelta unguarded
   use strict arg amount
   return self~sendWinIntMsg(self~MCM_SETMAXTODAYWIDTH, amount, 0)

::method setRange unguarded external "LIBRARY oodialog mc_setRange"
::method setSelectionRange unguarded external "LIBRARY oodialog mc_setSelectionRange"
::method setToday unguarded external "LIBRARY oodialog mc_setToday"
::method sizeRectToMin unguarded external "LIBRARY oodialog mc_sizeRectToMin"


/******** ProgressBar Class ***************************************************/

::class 'ProgressBar' subclass DialogControl public

-- WM_USER == 0x400 -> 1024
::constant PBM_SETPOS          "0x0402"
::constant PBM_DELTAPOS        "0x0403"
::constant PBM_SETSTEP         "0x0404"
::constant PBM_STEPIT          "0x0405"
::constant PBM_GETPOS          "0x0408"
::constant PBM_SETBARCOLOR     "0x0409"
::constant PBM_SETBKCOLOR      "0x2001"

::method backgroundColor unguarded
   use strict arg newColor
   return self~sendWinUintMsg(self~PBM_SETBKCOLOR, 0, newColor)

::method barColor unguarded
   use strict arg newColor
   return self~sendWinUintMsg(self~PBM_SETBARCOLOR, 0, newColor)

::method getFullRange unguarded external "LIBRARY oodialog pbc_getFullRange"
::method getPos unguarded
   use strict arg
   return self~sendWinIntMsg(self~PBM_GETPOS, 0, 0)

::method setFullRange unguarded external "LIBRARY oodialog pbc_setFullRange"
::method setMarquee unguarded external "LIBRARY oodialog pbc_setMarquee"
::method setPos unguarded
   use strict arg position
   return self~sendWinIntMsg(self~PBM_SETPOS, position, 0)

::method setStep unguarded
  use strict arg newStep = 10
   return self~sendWinIntMsg(self~PBM_SETSTEP, newStep, 0)

::method step unguarded
   use strict arg delta = 0
   msg = self~PBM_STEPIT
   if arg(1, 'E') then msg = self~PBM_DELTAPOS
   return self~sendWinIntMsg(msg, delta, 0)


-- DEPRECATED
::method getRange unguarded external "LIBRARY oodialog pbc_getFullRange"
::method setRange unguarded
   forward message "setFullRange" continue
   return result~min result~max



/******** ScrollBar Class *****************************************************/


::class 'ScrollBar' subclass DialogControl public

::constant LINEUP           0
::constant LINELEFT         0
::constant LINEDOWN         1
::constant LINERIGHT        1
::constant PAGEUP           2
::constant PAGELEFT         2
::constant PAGEDOWN         3
::constant PAGERIGHT        3
::constant THUMBPOSITION    4
::constant THUMBTRACK       5
::constant TOP              6
::constant LEFT             6
::constant BOTTOM           7
::constant RIGHT            7
::constant ENDSCROLL        8

::method determinePosition unguarded
   use strict arg posdata, single = 1, page = 10
   code = .DlgUtil~loWord(posdata)
   pos = self~getPos
   r = self~getRange
   if .SystemErrorCode <> 0 then return pos -- getRange failed

   select
      /* Line up */
      when code == self~LINEUP then pos = max(r~min, pos - single)
      /* Line down */
      when code == self~LINEDOWN then pos = min(r~max, pos + single)
      /* page up */
      when code == self~PAGEUP then pos = max(r~min, pos - page)
      /* page down */
      when code == self~PAGEDOWN then pos = min(r~max, pos + page)
      /* track position */
      when code == self~THUMBPOSITION then pos = .DlgUtil~hiWord(posdata)
      /* tracking */
      when code == self~THUMBTRACK then pos = .DlgUtil~loWord(posdata)
      /* top */
      when code == self~TOP then pos = r~min
      /* bottom */
      when code == self~BOTTOM then pos = r~max
      otherwise nop;
   end
   self~setPos(pos)
   return pos

::method getPos unguarded external "LIBRARY oodialog sb_getPosition"
::method getRange unguarded external "LIBRARY oodialog sb_getRange"
::method position unguarded external "LIBRARY oodialog sb_getPosition"
::method range unguarded
   r = self~getRange
   return r~min r~max

::method setPos unguarded external "LIBRARY oodialog sb_setPosition"
::method setRange unguarded external "LIBRARY oodialog sb_setRange"



/******** Static Class ********************************************************/

::class 'StaticControl' subclass DialogControl public
::class 'Static' subclass StaticControl public

::method getIcon unguarded external "LIBRARY oodialog stc_getIcon"
::method setIcon unguarded external "LIBRARY oodialog stc_setIcon"
::method getImage unguarded external "LIBRARY oodialog stc_getImage"
::method setImage unguarded external "LIBRARY oodialog stc_setImage"


/******** TrackBar Class ******************************************************/

::class 'SliderControl' subclass DialogControl public
::class 'TrackBar' subclass SliderControl public

-- TB_xx event notification reason codes
::constant UP                   0
::constant DOWN                 1
::constant PAGEUP               2
::constant PAGEDOWN             3
::constant POSITION             4
::constant DRAG                 5
::constant TOP                  6
::constant BOTTOM               7
::constant ENDTRACK             8

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

::method clearSelRange unguarded
   use strict arg redraw = .true
   self~sendWinIntMsg(self~TBM_CLEARSEL, redraw, 0)
   return 0

::method clearTicks unguarded
   use strict arg redraw = .true
   self~sendWinIntMsg(self~TBM_CLEARTICS, redraw, 0)
   return 0

::method countTicks unguarded
   return self~sendWinIntMsg(self~TBM_GETNUMTICS, 0, 0)

::method getLineStep unguarded
   return self~sendWinIntMsg(self~TBM_GETLINESIZE, 0, 0)

::method getMax unguarded
   return self~sendWinIntMsg(self~TBM_GETRANGEMAX, 0, 0)

::method getMin unguarded
   use strict arg
   return self~sendWinIntMsg(self~TBM_GETRANGEMIN, 0, 0)

::method getPageStep unguarded
   return self~sendWinIntMsg(self~TBM_GETPAGESIZE, 0, 0)

::method getRange unguarded external "LIBRARY oodialog trckbar_getRange"
::method getSelEnd unguarded
   use strict arg
   return self~sendWinIntMsg(self~TBM_GETSELEND, 0, 0)

::method getSelRange unguarded external "LIBRARY oodialog trckbar_getSelRange"
::method getSelStart unguarded
   use strict arg
   return self~sendWinIntMsg(self~TBM_GETSELSTART, 0, 0)

::method getTick unguarded
   use strict arg tic
   return self~sendWinIntMsg(self~TBM_GETTIC, tic, 0)

::method initRange unguarded
   use strict arg min = 0, max = 100, redraw = .false
   if max < min then return -1
   self~sendWinIntMsg(self~TBM_SETRANGE, redraw, .DlgUtil~makeLParam(min, max))
   return 0

::method initSelRange unguarded
   use strict arg min = 0, max = (self~getMax), redraw = .false
   if max < min then return -1
   self~sendWinIntMsg(self~TBM_SETSEL, redraw, .DlgUtil~makeLParam(min, max))
   return 0

::method pos unguarded
   use strict arg
   return self~sendWinIntMsg(self~TBM_GETPOS, 0, 0)

::method "pos=" unguarded
   forward message "setPos"

::method range unguarded
   r = self~getRange
   return r~min r~max

::method selRange
   sr = self~getSelRange
   return sr~start sr~end

::method setSelEnd unguarded
   use strict arg max, redraw = .true
   self~sendWinIntMsg(self~TBM_SETSELEND, redraw, min)
   return 0

::method setLineStep unguarded
   use strict arg step
   return self~sendWinIntMsg(self~TBM_SETLINESIZE, 0, step)

::method setMax unguarded
   use strict arg max, redraw = .true
   self~sendWinIntMsg(self~TBM_SETRANGEMAX, redraw, max)
   return 0

::method setMin unguarded
   use strict arg min, redraw = .true
   self~sendWinIntMsg(self~TBM_SETRANGEMIN, redraw, min)
   return 0

::method setPageStep unguarded
   use strict arg step
   return self~sendWinIntMsg(self~TBM_SETPAGESIZE, 0, step)

::method setPos unguarded
   use strict arg p, redraw = .false
   self~sendWinIntMsg(self~TBM_SETPOS, redraw, p)
   return 0

::method setSelStart unguarded
   use strict arg min, redraw = .true
   self~sendWinIntMsg(self~TBM_SETSELSTART, redraw, min)
   return 0

::method setTickAt unguarded
   use strict arg pos
   return self~sendWinIntMsg(self~TBM_SETTIC, 0, pos)

::method setTickFrequency unguarded
   use strict arg freq
   return self~sendWinIntMsg(self~TBM_SETTICFREQ, freq, 0)



/******** Tab Class & Releated ************************************************/

::class 'OwnedTabInfo' subclass Object public

::attribute nID
::attribute pages
::attribute sID
::attribute tab

::method init
  use strict arg id, pages = (.array~new)

::class 'TabControl' subclass DialogControl public
::class 'Tab' subclass TabControl public

-- TCM_FIRST == 0x1300  4864
::constant TCM_GETITEMCOUNT    "0x1304"
::constant TCM_GETROWCOUNT     "0x132C"
::constant TCM_GETCURSEL       "0x130B"
::constant TCM_SETCURSEL       "0x130C"
::constant TCM_SETCURFOCUS     "0x1330"
::constant TCM_GETCURFOCUS     "0x132F"
::constant TCM_DELETEALLITEMS  "0x1309"
::constant TCM_DELETEITEM      "0x1308"
::constant TCM_SETMINTABWIDTH  "0x1331"

::method addFullSeq unguarded external "LIBRARY oodialog tab_addFullSeq"
::method addSequence unguarded external "LIBRARY oodialog tab_addSequence"

-- adjustToRectangle() and requiredWindowSize() are the old methods.  This set
-- up produces the same results as the code did, which seems to be opposite of
-- what the doc says.
--
-- calcWindowRect() and calcDisplayRect() are named and documented according to
-- MSDN documentation.  (Which could be wrong.)

::method adjustToRectangle unguarded
   use strict arg left, top, right, bottom
   r = .Rect~new(left, top, right, bottom)
   self~calcWindowRect(r)
   return r~left r~top r~right r~bottom

::method calcDisplayRect unguarded external "LIBRARY oodialog tab_calcRect"
::method calcWindowRect unguarded external "LIBRARY oodialog tab_calcRect"
::method delete unguarded
   use strict arg item
   ret = self~sendWinIntMsg(self~TCM_DELETEITEM, item, 0)
   return ret == 0

::method deleteAll unguarded
   use strict arg
   ret = self~sendWinIntMsg(self~TCM_DELETEALLITEMS, 0, 0)
   return ret == 0

::method focus unguarded
   use strict arg item
   self~sendWinIntMsg(self~TCM_SETCURFOCUS, item, 0)
   return 0

::method focused unguarded
   use strict arg
   return self~sendWinIntMsg(self~TCM_GETCURFOCUS, 0, 0)

::method getImageList unguarded external "LIBRARY oodialog tab_getImageList"
::method getItemRect unguarded external "LIBRARY oodialog tab_getItemRect"
::method insert unguarded external "LIBRARY oodialog tab_insert"
::method itemInfo unguarded external "LIBRARY oodialog tab_itemInfo"
::method items unguarded
   use strict arg
   return self~sendWinIntMsg(self~TCM_GETITEMCOUNT, 0, 0)

::method last unguarded
   use strict arg
   return self~items - 1

::method modify unguarded external "LIBRARY oodialog tab_modify"
::method posRectangle unguarded
   use strict arg item
   r = .Rect~new
   if self~getItemRect(item, r) then return r~top r~left r~bottom r~right
   else return ""

::method requiredWindowSize unguarded
   use strict arg left, top, right, bottom
   r = .Rect~new(left, top, right, bottom)
   self~calcDisplayRect(r)
   return r~left r~top r~right r~bottom

::method rows unguarded
   use strict arg
   return self~sendWinIntMsg(self~TCM_GETROWCOUNT, 0, 0)

::method select unguarded external "LIBRARY oodialog tab_select"
::method selected unguarded external "LIBRARY oodialog tab_selected"
::method selectedIndex unguarded
   use strict arg
   return self~sendWinIntMsg(self~TCM_GETCURSEL, 0, 0)

::method selectIndex unguarded
   use strict arg item
   return self~sendWinIntMsg(self~TCM_SETCURSEL, item, 0)

::method setImageList unguarded external "LIBRARY oodialog tab_setImageList"
::method setItemSize unguarded external "LIBRARY oodialog tab_setItemSize"
::method setMinTabWidth unguarded
   use strict arg width
   return self~sendWinIntMsg(self~TCM_SETMINTABWIDTH, 0, width)

::method setPadding unguarded external "LIBRARY oodialog tab_setPadding"

::method setSize unguarded
   forward message "setItemSize" continue
   return result~width result~height

-- DEPRECATED
::method removeImages unguarded
   return self~setImageList(.nil)

-- DEPRECATED
::method setImages unguarded external "LIBRARY oodialog tab_setImageList"



/******** UPDown Class ********************************************************/

::class 'UpDown' subclass DialogControl public

::constant UDM_SETBASE             "0x046D"
::constant UDM_GETBASE             "0x046E"
::constant UDM_SETPOS32            "0x0471"

::method deltaPosReply class external "LIBRARY oodialog ud_deltaPosReply_cls"

::method getAcceleration unguarded external "LIBRARY oodialog ud_getAcceleration"
::method getBase unguarded
   use strict arg
   return self~sendWinIntMsg(self~UDM_GETBASE, 0, 0)

::method getBuddy unguarded external "LIBRARY oodialog ud_getBuddy"
::method getRange unguarded external "LIBRARY oodialog ud_getRange"
::method getPosition unguarded external "LIBRARY oodialog ud_getPosition"
::method setAcceleration unguarded external "LIBRARY oodialog ud_setAcceleration"
::method setBase unguarded
   use strict arg newBase
   return self~sendWinIntMsg(self~UDM_SETBASE, newBase, 0)

::method setBuddy unguarded external "LIBRARY oodialog ud_setBuddy"
::method setPosition unguarded
   use strict arg newPos
   return self~sendWinIntMsg(self~UDM_SETPOS32, 0, newPos)

::method setRange unguarded external "LIBRARY oodialog ud_setRange"
