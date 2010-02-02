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
 * The basic dialog class
 *
 * PlainBaseDialog with its two inherited mixin classes, WindowBase and
 * ResourceUtils, is the base dialog for all dialogs in the ooDialog framwork.
 */
::requires "UtilityClasses.cls"

/* The WindowBase class implements methods that are common to all windows.  The
 * methods only require the operating system window handle to function.
 */
::class 'WindowBase' public mixinclass object

::method init_windowBase private external "LIBRARY oodialog wb_init_windowBase"

::attribute initCode

::attribute hwnd get external "LIBRARY oodialog wb_getHwnd"

-- Size and Dialog Unit Factor
::attribute sizeX get external "LIBRARY oodialog wb_getSizeX"
::attribute sizeX set external "LIBRARY oodialog wb_setSizeX"

::attribute sizeY get external "LIBRARY oodialog wb_getSizeY"
::attribute sizeY set external "LIBRARY oodialog wb_setSizeY"

::attribute factorX get external "LIBRARY oodialog wb_getFactorX"
::attribute factorX set external "LIBRARY oodialog wb_setFactorX"

::attribute factorY get external "LIBRARY oodialog wb_getFactorY"
::attribute factorY set external "LIBRARY oodialog wb_setFactorY"

-- True size in pixels
::attribute pixelX get external "LIBRARY oodialog wb_getPixelX"
::attribute pixelY get external "LIBRARY oodialog wb_getPixelY"

::method sendMessage unguarded external "LIBRARY oodialog wb_sendMessage"
::method sendMessageHandle unguarded external "LIBRARY oodialog wb_sendMessage"

::method sendWinIntMsg unguarded external "LIBRARY oodialog wb_sendWinIntMsg"
::method sendWinIntMsgH unguarded external "LIBRARY oodialog wb_sendWinIntMsg"
::method sendWinUintMsg unguarded external "LIBRARY oodialog wb_sendWinUintMsg"
::method sendWinHandleMsg unguarded external "LIBRARY oodialog wb_sendWinHandleMsg"
::method sendWinHandleMsgH unguarded external "LIBRARY oodialog wb_sendWinHandleMsg"
::method sendWinHandle2Msg unguarded external "LIBRARY oodialog wb_sendWinHandle2Msg"
::method sendWinHandle2MsgH unguarded external "LIBRARY oodialog wb_sendWinHandle2Msg"

::method enable unguarded external "LIBRARY oodialog wb_enable"
::method disable unguarded external "LIBRARY oodialog wb_enable"
::method isVisible unguarded external "LIBRARY oodialog wb_isVisible"
::method isEnabled unguarded external "LIBRARY oodialog wb_isEnabled"

::method show unguarded external "LIBRARY oodialog wb_show"
::method hide unguarded external "LIBRARY oodialog wb_show"
::method showFast unguarded external "LIBRARY oodialog wb_showFast"
::method hideFast unguarded external "LIBRARY oodialog wb_showFast"
::method display unguarded external "LIBRARY oodialog wb_display"

::method getWindowLong private unguarded external "LIBRARY oodialog wb_getWindowLong_pvt"
::method getID unguarded
  return self~getWindowLong(-12)

::method getStyleRaw unguarded
  return self~getWindowLong(-16)

::method getExStyleRaw unguarded
  return self~getWindowLong(-20)

::method update unguarded external "LIBRARY oodialog wb_redrawClient"
::method draw unguarded external "LIBRARY oodialog wb_redrawClient"
::method redrawClient unguarded external "LIBRARY oodialog wb_redrawClient"
::method redraw unguarded external "LIBRARY oodialog wb_redraw"

::method "title=" unguarded external "LIBRARY oodialog wb_setText"
::method setTitle unguarded external "LIBRARY oodialog wb_setText"
::method title unguarded external "LIBRARY oodialog wb_getText"
::method setText unguarded external "LIBRARY oodialog wb_setText"
::method getText unguarded external "LIBRARY oodialog wb_getText"

::method setRect unguarded external "LIBRARY oodialog wb_setRect"
::method resizeTo unguarded external "LIBRARY oodialog wb_resizeMove"
::method moveTo unguarded external "LIBRARY oodialog wb_resizeMove"
::method getRealSize unguarded external "LIBRARY oodialog wb_getSizePos"
::method getRealPos unguarded external "LIBRARY oodialog wb_getSizePos"
::method windowRect unguarded external "LIBRARY oodialog wb_windowRect"
::method clientRect unguarded external "LIBRARY oodialog wb_clientRect"
::method clear unguarded external "LIBRARY oodialog wb_clear"

::method resize unguarded           -- old method, this is not accurate.
   use strict arg width, height, showOptions = ""
   p = .Point~new(0, 0)
   s = .Size~new(trunc(width * self~factorX), trunc(height * self~factorY))
   return self~setRect(p, s, "NOMOVE " || showOptions);

::method move unguarded             -- old method, this is not accurate.
   use strict arg xPos, yPos, showOptions = ""
   p = .Point~new(trunc(xPos * self~factorX), trunc(yPos * self~factorY))
   s = .Size~new(0, 0)
   return self~setRect(p, s, "NOSIZE " || showOptions);

::method getSize unguarded          -- old method, this is not accurate.
   forward message "getRealSize" continue
   size = result
   return (size~width % self~factorX) || " " || (size~height % self~factorY)

::method getPos unguarded           -- old method, this is not accurate.
   forward message "getRealPos" continue
   p = result
   return (p~x % self~factorX) || " " || (p~y % self~factorY)

::method getRect unguarded          -- old method returns coordinates as a string
   forward message "windowRect" continue
   rect = result
   return rect~left rect~top rect~right rect~bottom

::method getClientRect unguarded    -- old method returns coordinates as a string
   forward message "clientRect" continue
   rect = result
   return rect~left rect~top rect~right rect~bottom

::method foregroundWindow unguarded external "LIBRARY oodialog wb_foreGroundWindow"
::method screen2client unguarded external "LIBRARY oodialog wb_screenClient"
::method client2screen unguarded external "LIBRARY oodialog wb_screenClient"

::method screenToClient unguarded   -- old method returns coordinates as a string
    use strict arg x, y
    p = .Point~new(x, y)
    if self~screen2client(p) then return p~x p~y
    else return 0 0

::method clientToScreen unguarded   -- old method returns coordinates as a string
    use strict arg x, y
    p = .Point~new(x, y)
    if self~client2screen(p) then return p~x p~y
    else return 0 0

::method unknown unguarded
   use arg msgname
   call errorDialog msgname || " is not a method of " || self

-- DEPRECATED
::method getSystemMetrics unguarded
   use strict arg index
   return .DlgUtil~getSystemMetrics(index)


::class 'ResourceUtils' public mixinclass object

/* Directory object containing symbolic constants */
::attribute constDir get
::attribute constDir set private

::attribute processingLoad private    -- in loadItems ?

::method parseIncludeFile
  use strict arg hFile

  if \ SysIsFile(hFile) then do
    -- TODO Revisit. Should a syntax condition be raised if the file is not found or readable?
    hFile = SysSearchPath("PATH", hFile)
    if hFile == "" then return .false
  end

  f = .stream~new(hFile)
  f~open(read)

  do while f~state == "READY"
    line = f~linein~strip('L')

    if line~abbrev("#ifdef") then do
      self~skipThroughIfDefs(f, line)
      iterate
    end

    if line~abbrev("#define") & line~words == 3 then do
      parse var line def symbol numericID .
      if numericID~datatype('W') then self~ConstDir[symbol~translate] = numericID
    end
  end
  f~close
  return .true

::method skipThroughIfDefs private
  use arg fObj, line

  if \ line~abbrev("#ifdef") & \ line~abbrev("#ifndef") then return
  ifDefStack = .queue~new
  ifDefStack~push(line)
  do while fObj~state == "READY"
    l = fObj~linein~strip("L")
    select
      when l~abbrev("#endif") then ifDefStack~pull
      when l~abbrev("#ifdef") then ifDefStack~push(l)
      when l~abbrev("#ifndef") then ifDefStack~push(l)
      otherwise nop
    end
    if ifDefStack~items == 0 then leave
  end

::method checkfile private
   use arg f
   if (f~lines = 0) then do
      f~close
      self~processingLoad = 0
      ret = errorDialog("Error reading resource file!" f)
      return 1
   end
   return 0

::method errorfile private
   use arg f, s
   f~close
   self~processingLoad = 0
   ret = errorDialog("Error reading resource file:" f "("s")")

::method idError private external "LIBRARY oodialog rsrcUtils_idError"
::method resolveIconID private external "LIBRARY oodialog rsrcUtils_resolveIconID_pvt"
::method resolveSymbolicId unguarded external "LIBRARY oodialog rsrcUtils_resolveResourceID"
::method resolveResourceID unguarded external "LIBRARY oodialog rsrcUtils_resolveResourceID"
::method checkID external "LIBRARY oodialog rsrcUtils_checkID"

/**
 * Return the symbolic ID from the ConstDir that matches the numeric ID.
 * The docs will advise users of ooDialog to use unique numbers for all
 * resources.
 */
::method resolveNumericID unguarded
  use arg numericID
  if \ numericID~datatype('W') then return -1
  symbol = self~constDir~index(numericID)
  if symbol == .nil then return -1
  else return symbol

::method mergeSymbols
  use strict arg otherSymbolSrc

  select
    when otherSymbolSrc~isA(.directory) then do
      if self~constDir == otherSymbolSrc then return 0
      self~constDir~putAll(otherSymbolSrc)
      otherSymbolSrc~putAll(self~constDir)
    end

    when otherSymbolSrc~isA(.ResourceUtils) then do
      if self~constDir == otherSymbolSrc~constDir then return 0
      self~constDir~putAll(otherSymbolSrc~constDir)
      otherSymbolSrc~constDir~putAll(self~constDir)
    end

    otherwise return -1
  end
  -- End select
  return 0

::class 'Window' public inherit WindowBase
::method init  external "LIBRARY oodialog window_init"
::method unInit  external "LIBRARY oodialog window_unInit"

::class 'DialogProxy' public
::attribute initCode get
  return 1

/**
 *  The PlainBaseDialog class is the base class of all ooDialog dialogs.  It
 *  implements those methods that are common to all dialogs, whether the dialog
 *  template comes from a binary resource, a resource script file, or is con-
 *  structed dynamically.
 */
::class 'PlainBaseDialog' public inherit WindowBase ResourceUtils EventNotification

::constant DM_REPOSITION                 "0x0402"
::constant WM_SETFONT                    "0x0030"
::constant WM_NEXTDLGCTL                 "0x0028"
::constant WM_USER_INTERRUPTSCROLL       "0x0A02"


::method init class external "LIBRARY oodialog pbdlg_init_cls"
::method setDefaultFont class external "LIBRARY oodialog pbdlg_setDefaultFont_cls"
::method getFontName class external "LIBRARY oodialog pbdlg_getFontName_cls"
::method getFontSize class external "LIBRARY oodialog pbdlg_getFontSize_cls"
::method new class external "LIBRARY oodialog pbdlg_new_cls"

-- External adminstration buffer. Get can not be private for now see MenuBar::attachTo()
::attribute adm get

-- Automatic data field detection on/off
::attribute autoDetect get external "LIBRARY oodialog pbdlg_getAutoDetect"
::attribute autoDetect set external "LIBRARY oodialog pbdlg_setAutoDetect"

::attribute dataConnection private  -- store connections between ids and class attributes
::attribute useStem private         -- flag whether to use a stem to set/get data or not
::attribute finished                -- finsished state of dialog, not a logical, can be 0, 1, or 2

::method dlgData private
   expose dlgData.
   return dlgData.

::attribute parentDlg private
::attribute childDialogs

::attribute fontName get external "LIBRARY oodialog pbdlg_getFontName"
::attribute fontSize get external "LIBRARY oodialog pbdlg_getFontSize"
::attribute dlgHandle get unguarded external "LIBRARY oodialog pbdlg_getDlgHandle"

-- A queue containing the methods that will be started concurrently before execution of the dialog
::attribute automaticMethods private

/** PlainBaseDialog::init()  Performs the necessary intialization, including the
 * set up of the dialog management block, for all dialogs.
 */
::method init external "LIBRARY oodialog pbdlg_init"
::method unInit external "LIBRARY oodialog pbdlg_unInit"
::method setDlgFont external "LIBRARY oodialog pbdlg_setDlgFont"

::method initDialog unguarded private
   return 0

-- Dispatches window messages until the dialog is terminated.
::method run unguarded private
   use strict arg sleepTime = 1
   if \ sleepTime~datatype('W') then sleepTime = 1

   do while self~finished = 0
      self~handleMessages(sleepTime)
   end

-- Creates the dialog, shows it, and destroys it.  Dialog "data" is set and received.
::method execute unguarded
   expose dlgData.
   use strict arg sh = "NORMAL", icon = 0, sleepTime = 1, modeless = .false

   icon = self~resolveIconId(icon)
   if self~startIt(icon, modeless) \= 0 then do
      if self~useStem then self~setDataStem(dlgData.)
      else self~setData
      self~show(sh)
      do while self~automaticMethods~Items > 0
         p = self~automaticMethods~pull
         m = self~automaticMethods~pull
         c = self~automaticMethods~pull
         o = c~new(p, self)
         o~start(m)
      end

      self~run(sleepTime)

      if self~initCode = 1 then do
         if self~useStem then self~getDataStem(dlgData.)
         self~getData
      end
      if self~adm \= 0 then self~stopIt
      return self~initCode
   end
   else do
      self~stopIt
   end
   return 0

-- Note: handleMessages() and clearMessages() were two methods that were previously marked protected.
-- My believe is that there was some confusion in the original developement that led to methods being
-- marked protected when they were meant to be marked private.  However, changing these 2 methods to
-- private:  1.) breaks an existing example 2.) the doc on clearMessages() would lead one to think that
-- it is a public method.  So, the methods are changed to public rather than private.

-- Retrieve and dispatch, if available, one window message.
::method handleMessages unguarded
   use strict arg sleepTime = 1
   if self~adm = 0 then do
       self~finished = 2
       return
   end

   msg = self~getDlgMsg
   if msg~pos("1DLGDELETED1") > 0 then do
      if self~finished = 0 then self~finished = 2
   end
   else do
      if msg \= "" then do
         parse var msg methodName '(' args ')'
         argList = args~makeArray(255~d2c)
         self~sendWith(methodName, argList)
      end
      else do
         call msSleep sleepTime
      end
   end

-- Clear all pending window messages
::method clearMessages unguarded
   if self~adm = 0 then return
   msg = self~getDlgMsg
   do while msg \= "" & self~adm \= 0
       if msg~pos("1DLGDELETED1") > 0 then do
          if self~finished = 0 then self~finished = 2
       end
       msg = self~getDlgMsg
   end

::method getDlgMsg external "LIBRARY oodialog pbdlg_getDlgMsg"
::method isDialogActive unguarded external "LIBRARY oodialog pbdlg_isDialogActive"
::method stopIt external "LIBRARY oodialog pbdlg_stopIt"
::method finalStopIt private
   use arg caller

   -- If we ourselves are a child dialog and not called by the parent's stopIt(), then
   -- remove ourself from the parent dialog's child list.  (Child dialogs are independent
   -- and the user could close them at any time.)
   if self~parentDlg \= .nil & caller \= "PARENT" then self~parentDlg~ChildDied(self)

   -- If we have child dialogs, then stop all of them.  Child dialogs are always started
   -- through popupAsChild() and are always closed when the parent closes.
   if self~childDialogs \= .nil then do
       if self~childDialogs~items > 0 then
       do child over self~childDialogs
          child~stopit("PARENT")
       end
       self~childDialogs = .nil
   end
   return 0


-- Remove a child dialog from the chain of child dialogs.
::method childDied
   use arg child
   if self~childDialogs \= .nil then do
       ndx = self~childDialogs~first
       do while ndx \= .nil
          if self~childDialogs[ndx] == child then do
              self~childDialogs~remove(ndx)
              ndx = .nil
          end
          else ndx = self~childDialogs~next(ndx)
       end
   end

::method sendMessageToWindow unguarded external "LIBRARY oodialog pbdlg_sendMessageToWindow"
::method sendMessageToWindowH unguarded external "LIBRARY oodialog pbdlg_sendMessageToWindow"
::method sendMessageToControl unguarded external "LIBRARY oodialog pbdlg_sendMessageToControl"
::method sendMessageToControlH unguarded external "LIBRARY oodialog pbdlg_sendMessageToControl"

::method get unguarded external "LIBRARY oodialog pbdlg_get"
::method getSelf unguarded external "LIBRARY oodialog pbdlg_getDlgHandle"
::method getControlID unguarded external "LIBRARY oodialog pbdlg_getControlID"
::method show unguarded external "LIBRARY oodialog pbdlg_show"
::method restore unguarded external "LIBRARY oodialog pbdlg_show"
::method toTheTop unguarded external "LIBRARY oodialog pbdlg_toTheTop"
::method minimize unguarded external "LIBRARY oodialog pbdlg_doMinMax"
::method maximize unguarded external "LIBRARY oodialog pbdlg_doMinMax"
::method isMaximized unguarded external "LIBRARY oodialog pbdlg_doMinMax"
::method isMinimized unguarded external "LIBRARY oodialog pbdlg_doMinMax"
::method getWindowText unguarded external "LIBRARY oodialog pbdlg_getWindowText"
::method setWindowText unguarded external "LIBRARY oodialog pbdlg_setWindowText"
::method getControlText unguarded external "LIBRARY oodialog pbdlg_getControlText"
::method setControlText unguarded external "LIBRARY oodialog pbdlg_setControlText"
::method getControlHandle unguarded external "LIBRARY oodialog pbdlg_getControlHandle"
::method pixel2dlgUnit external "LIBRARY oodialog pbdlg_pixel2dlgUnit"
::method dlgUnit2pixel external "LIBRARY oodialog pbdlg_dlgUnit2pixel"
::method backgroundBitmap external "LIBRARY oodialog pbdlg_backgroundBitmap"
::method tiledBackgroundBitmap external "LIBRARY oodialog pbdlg_tiledBackgroundBitmap"
::method backgroundColor external "LIBRARY oodialog pbdlg_backgroundColor"

::method ensureVisible unguarded
   return self~sendWinIntMsg(self~DM_REPOSITION, 0, 0)


/* The following methods 'connect' a Windows dialog control with a 'data' attribute of the Rexx
 * dialog object.  For each dialog control an attribute of the dialog object is created and an
 * entry is added to the data table to 'connect' the control and the attribute.  When auto detection
 * is on the 'data' (which is the value of the attribute) attribute is used to set the state of the
 * control on start up of the dialog.  Likewise, when the windows dialog is closed, the 'data' (the state)
 * of the control is used to set the value of the attribute.
 */

::method connectCheckBox unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectRadioButton unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectEdit unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectComboBox unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectListBox unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectTreeView unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectListView unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectTrackBar unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectTab unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectDateTimePicker unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectMonthCalendar unguarded external "LIBRARY oodialog pbdlg_connect_ControName"
::method connectSeparator unguarded external "LIBRARY oodialog pbdlg_connect_ControName"

::method addAttribute private
   use arg id, attributeName
   if arg(2,'o') = 1 then attributeName = "DATA"id~space(0)
   if id~DataType("N") = 0 then do
       if self~ProcessingLoad = 1 then attributeName = id
       id = self~resolveSymbolicId(id)
   end
   if id < 1 then return -1
   if attributeName~space(0) = "" | self~HasMethod(attributeName) = 1 then
       attributeName = "DATA"id~space(0)
   else do
      attributeName = attributeName~space(0)~changestr('&','')~changestr(':','') /* remove blanks, &, and : */
      if attributeName~datatype('V')=0 then attributeName = "DATA"id~space(0)
   end

   self~dataConnection[id] = attributeName
   self~setmethod(attributeName, "expose "attributeName"; return "attributeName)
   self~setmethod(attributeName || "=", "expose "attributeName"; use arg "attributeName)
   self~sendWith(attributeName || '=', .array~of(""))  /* initial value = "" */
   return id


::method setDlgDataFromStem private unguarded external "LIBRARY oodialog pbdlg_setDlgDataFromStem_pvt"
::method putDlgDataInStem private unguarded external "LIBRARY oodialog pbdlg_putDlgDataInStem_pvt"

-- This method sets the 'data' of all the connected dialog controls using the value of the associated attribute.
-- The Windows dialog controls are associated with with a data attribute of the Rexx dialog object through the
-- connectXXX methods.  connectListBox(), connectComboBox(), etc..
::method setData unguarded
   id = self~dataConnection~first
   do while (id \= .nil)
      data = self~send(self~dataConnection[id])
      InternDlgData.id = data
      id = self~dataConnection~next(id)
   end
   return self~setDlgDataFromStem(InternDlgData.)


-- This method gets the 'data' of all the connected dialog controls and uses that data to set the value of the
-- associated attribute.  See comment above for setData()
::method getData unguarded
   self~putDlgDataInStem(InternDlgData.)

   id = self~dataConnection~first
   do while (id \= .nil)
      self~sendWith(self~dataConnection[id]'=', .array~of(InternDlgData.id))
      id = self~dataConnection~next(id)
   end


-- The method returns an array that contains the value of each of the attributes associated with a dialog
-- control.  MM What good is this, you don't know which control is associated with each of the 'data' pieces?
::method makeArray unguarded

   a = .array~new(self~dataConnection~items)
   id = self~dataConnection~first
   do j = 1 while (id \= .nil)
      attributeName = self~dataConnection~at(id)
      data = self~send(attributeName)
      a[j] = data
      id = self~dataConnection~next(id)
   end
   return a


-- This method sets the 'data' of each of the connected dialog controls using the values of the specified stem.
-- Each index of the stem should be the resource ID of a connected dialog control and the value at that index is
-- the 'data' to set the control with.
::method setDataStem unguarded
   use arg dataStem.
   if dataStem.~items == 0 then return 1
   do k over dataStem.
      if k~dataType('W') then numericID = k
      else numericID = self~ResolveSymbolicID(k)
      InternDlgData.numericID = dataStem.k
      if InternDlgData.numericID = "INTERNDLGDATA."numericID then InternDlgData.numericID = ""
   end
   return self~setDlgDataFromStem(InternDlgData.)


-- This method gets the 'data' of all of the connected dialog controls and sets the indexes of the specified
-- stem with the 'data'.  The resource ID of each dialog control is used as an index and the 'data' of that
-- dialog control is set as the value.  In addition, if the numeric resource ID has a matching symbolic ID in
-- the constDir, then the symbolic ID is used as a second index and set with the 'data' of the the dialog control.
::method getDataStem unguarded
   use arg dataStem.
   self~putDlgDataInStem(InternDlgData.)
   do k over InternDlgData.
      dataStem.k = InternDlgData.k
      symbolicID = self~resolveNumericID(k)
      if symbolicID \== -1 then dataStem.symbolicID = InternDlgData.k
   end


-- The following methods are used to get the 'data' from a single dialog control

-- This method gets the 'data' of a dialog control. You don't have to know what kind of control it is
-- if it has been 'connected' previously, i.e. connectTreeView(), connectTab(), etc..
::method getControlData unguarded external "LIBRARY oodialog pbdlg_getControlData"
::method getCheckBoxData unguarded external "LIBRARY oodialog pbdlg_getControlData"
::method getRadioButtonData unguarded external "LIBRARY oodialog pbdlg_getControlData"
::method getEditData unguarded external "LIBRARY oodialog pbdlg_getControlData"
::method getListBoxData unguarded external "LIBRARY oodialog pbdlg_getControlData"
::method getComboBoxData unguarded external "LIBRARY oodialog pbdlg_getControlData"

-- This method gets the 'data' of a dialog control and uses that data to set the value of the attribute
-- associated with the dialog control through one of the connectXXX methods.  I.e., connectEdit(),
-- connectListBox(), etc..  It returns nothing to the caller, the action is setting the value of the
-- associated attribute.
::method getControlDataAttribute unguarded
   use strict arg attributeName
   id = self~dataConnection~First
   do while (id \= .nil)
      if self~dataConnection[id] = attributeName~space(0) then do
         data = self~getControlData(id)
         self~sendWith(attributeName'=', .array~of(data))
         return
      end
      id = self~dataConnection~next(id)
   end


-- The following methods are used to set the 'data' of a single dialog item

-- This method sets the 'data' of a dialog control. You don't have to know what kind of control it is
-- if it has been 'connected' previously, i.e. connectTreeView(), connectTab(), etc..
::method setControlData unguarded external "LIBRARY oodialog pbdlg_setControlData"

::method setStaticData unguarded external "LIBRARY oodialog pbdlg_setControlData"
::method setCheckBoxData unguarded external "LIBRARY oodialog pbdlg_setControlData"
::method setRadioButtonData unguarded external "LIBRARY oodialog pbdlg_setControlData"
::method setEditData unguarded external "LIBRARY oodialog pbdlg_setControlData"
::method setListBoxData unguarded external "LIBRARY oodialog pbdlg_setControlData"
::method setComboBoxData unguarded external "LIBRARY oodialog pbdlg_setControlData"

-- This method sets the 'data' of a dialog control using the value of the attribute associated with
-- the dialog control through one of the connectXXX methods.  I.e., connectEdit(), connectListBox()
::method setControlDataAttribute unguarded
   use strict arg attributeName
   id = self~dataConnection~first
   do while (id \= .nil)
      if self~dataConnection[id] = attributeName~space(0) then do
         data = self~send(attributeName)
         return self~setControlData(id, data)
      end
      id = self~dataConnection~next(id)
   end


-- Methods to manipulate a ComboBox

::method addComboEntry unguarded
   use strict arg id, string
   comboBox = self~newComboBox(id)
   if comboBox == .nil then return -1
   else return comboBox~add(string)

::method insertComboEntry unguarded
   use strict arg id, index = 0, string
   comboBox = self~newComboBox(id)
   if comboBox == .nil then return -1
   if arg(2, 'O') then return comboBox~insert( , string)
   else return comboBox~insert(index, string)

::method findComboEntry unguarded
   use strict arg id, string
   comboBox = self~newComboBox(id)
   if comboBox == .nil then return -1
   return comboBox~find(string)

::method getComboEntry unguarded
   use strict arg id, index
   comboBox = self~newComboBox(id)
   if comboBox == .nil then return -1
   return comboBox~getText(index)

::method comboAddDirectory unguarded
   use strict arg id, drivepath, fileAttributes = ""
   comboBox = self~newComboBox(id)
   if comboBox == .nil then return -1
   return comboBox~addDirectory(drivePath, fileAttributes)

::method deleteComboEntry unguarded
   use strict arg id, index = (self~getCurrentComboIndex(id))
   if index <= 0 then return -1
   comboBox = self~newComboBox(id)
   if comboBox == .nil then return -1
   return comboBox~delete(index)

::method getCurrentComboIndex unguarded
   use strict arg id
   comboBox = self~newComboBox(id)
   if comboBox == .nil then return -1
   return comboBox~selectedIndex

::method setCurrentComboIndex unguarded
   use strict arg id, ndx = 0
   comboBox = self~newComboBox(id)
   if comboBox == .nil then return -1
   return comboBox~selectIndex(ndx)

::method getComboItems unguarded
   use strict arg id
   comboBox = self~newComboBox(id)
   if comboBox == .nil then return -1
   return comboBox~items

::method changeComboEntry unguarded
   use strict arg id, item = (self~getCurrentComboIndex(id)), string
   if item <= 0 then return -1
   self~deleteComboEntry(id, item)
   return self~insertComboEntry(id, item, string)

::method comboDrop unguarded
   use strict arg id
   comboBox = self~newComboBox(id)
   if comboBox == .nil then return -1
   return comboBox~deleteAll

-- Methods to manipulate a ListBox

::method setListTabulators unguarded external "LIBRARY oodialog generic_setListTabulators"

::method addListEntry unguarded
   use strict arg id, text
   listBox = self~newListBox(id)
   if listBox == .nil then return -1
   else return listBox~add(text)

::method insertListEntry unguarded
   use strict arg id, index = (-2), text
   listBox = self~newListBox(id)
   if listBox == .nil then return -1
   if arg(2, 'O') then return listBox~insert( , text)
   else return listBox~insert(index, text)

::method findListEntry unguarded
   use strict arg id, text
   listBox = self~newListBox(id)
   if listBox == .nil then return -1
   return listBox~find(text)

::method getListEntry unguarded
   use strict arg id, index
   listBox = self~newListBox(id)
   if listBox == .nil then return -1
   return listBox~getText(index)

::method listAddDirectory unguarded
   use strict arg id, drivePath, fileAttributes = ""
   listBox = self~newListBox(id)
   if listBox == .nil then return -1
   return listBox~addDirectory(drivePath, fileAttributes)

::method deleteListEntry unguarded
   use strict arg id, index = (self~getCurrentListIndex(id))
   if index == 0 then return -1
   listBox = self~newListBox(id)
   if listBox == .nil then return -1
   return listBox~delete(index)

::method getCurrentListIndex unguarded
   use strict arg id
   listBox = self~newListBox(id)
   if listBox == .nil then return -1
   return listBox~selectedIndex

::method setCurrentListIndex unguarded
   use strict arg id, ndx = 0
   listBox = self~newListBox(id)
   if listBox == .nil then return -1
   return listBox~selectIndex(ndx)

::method getListItems unguarded
   use strict arg id
   listBox = self~newListBox(id)
   if listBox == .nil then return -1
   return listBox~items

::method changeListEntry unguarded
   use strict arg id, index = (self~getCurrentListIndex(id)), newString
   if index <= 0 then return -1
   self~deleteListEntry(id, index)
   return self~insertListEntry(id, index, newString)

::method listDrop unguarded
   use strict arg id
   listBox = self~newListBox(id)
   if listBox == .nil then return -1
   return listBox~deleteAll


::method enableControl unguarded external "LIBRARY oodialog pbdlg_enableDisableControl"
::method disableControl unguarded external "LIBRARY oodialog pbdlg_enableDisableControl"
::method showControl unguarded external "LIBRARY oodialog pbdlg_showControl"
::method hideControl unguarded external "LIBRARY oodialog pbdlg_showControl"
::method showControlFast unguarded external "LIBRARY oodialog pbdlg_showControl"
::method hideControlFast unguarded external "LIBRARY oodialog pbdlg_showControl"
::method showWindow unguarded external "LIBRARY oodialog pbdlg_showWindow"
::method showWindowFast unguarded external "LIBRARY oodialog pbdlg_showWindow"
::method hideWindow unguarded external "LIBRARY oodialog pbdlg_showWindow"
::method hideWindowFast unguarded external "LIBRARY oodialog pbdlg_showWindow"

::method focusControl unguarded external "LIBRARY oodialog pbdlg_focusControl"
::method setFocusToWindow unguarded external "LIBRARY oodialog pbdlg_setFocus"
::method tabToNext unguarded external "LIBRARY oodialog pbdlg_tabTo"
::method tabToPrevious unguarded external "LIBRARY oodialog pbdlg_tabTo"
::method setFocus unguarded external "LIBRARY oodialog pbdlg_setFocus"
::method getFocus unguarded external "LIBRARY oodialog pbdlg_getFocus"
::method getTextSizeDlg unguarded external "LIBRARY oodialog pbdlg_getTextSizeDlg"
::method setTabStop unguarded external "LIBRARY oodialog pbdlg_setTabGroup"
::method setGroup unguarded external "LIBRARY oodialog pbdlg_setTabGroup"
::method center unguarded external "LIBRARY oodialog pbdlg_center"

-- initAutoDetection() is invoked by init() to set the default for automatic data field detection.
-- Therefore, subclasses can over-ride this method to change the default, which is enabled.
::method initAutoDetection private
   self~autoDetection            -- autodetection is enabled by default

::method noAutoDetection
   self~autoDetect = .false      -- disable autodetection

::method autoDetection
   self~autoDetect = .true       -- enable autodetection

 /* The following methods are abstract methods that will be called */
 /* whenever the push buttons 1, 2, or 9 will be pressed */
 /* Method 'OK' will call method 'Validate' */
 /* If self~finished = 1 the dialog is destroyed, if 0 the dialog */
 /* execution will be continued */
 /* OK returns 1 if the dialog is going to be destroyed */

::method ok unguarded
   self~finished = self~validate
   self~initCode = 1
   return self~finished

 /* If self~finished = 1 the dialog is destroyed, if 0 the dialog */
 /* execution will be continued */

::method cancel unguarded
   self~finished = 1
   if self~finished = 1 then self~initCode = 2
   return self~finished

::method help unguarded
   return 0

::method leaving
   return

/** validate()
 * This method is an abstract method that will be called to determin whether the
 * dialog can be closed or not. This function will be called by method OK.  If
 * the return value is 1 the dialog can be closed, if it is 0 the dialog
 * values are invalid.
 */
::method validate unguarded
   return 1

::method newStatic unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newPushButton unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newRadioButton unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newCheckBox unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newGroupBox unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newEdit unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newComboBox unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newListBox unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newScrollBar unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newTreeView unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newListView unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newTab unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newProgressBar unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newTrackBar unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newMonthCalendar unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newDateTimePicker unguarded external "LIBRARY oodialog pbdlg_newControl"
::method newUpDown unguarded external "LIBRARY oodialog pbdlg_newControl"
::method putControl private unguarded external "LIBRARY oodialog pbdlg_putControl_pvt"


-- DEPRECATED  (to end of file)
::method deInstall
  return 0

::method connectEntryLine unguarded
   forward message 'connectEdit'
::method connectMultiListBox unguarded
   forward message 'connectListBox'
::method connectTreeControl unguarded
   forward message 'connectTreeView'
::method connectListControl unguarded
   forward message 'connectListView'
::method connectSliderControl unguarded
   forward message 'connectTrackBar'
::method connectTabControl unguarded
   forward message 'connectTab'

::method getEntryLine unguarded
   forward message 'getEditData'
::method getListLine unguarded
   forward message 'getListBoxData'
::method getComboLine unguarded
   forward message 'getComboBoxData'
::method getCheckBox unguarded
   forward message 'getCheckBoxData'
::method getRadioButton unguarded
   forward message 'getRadioButtonData'
::method getMultiList unguarded
   forward message 'getListBoxData'
::method getValue unguarded
   forward message 'getControlData'
::method getAttrib unguarded
   forward message 'getControlDataAttribute'

::method setStaticText unguarded
   forward message 'setStaticData'
::method setEntryLine unguarded
   forward message 'setEditData'
::method setListLine unguarded
   forward message 'setListBoxData'
::method setComboLine unguarded
   forward message 'setComboBoxData'
::method setCheckBox unguarded
   forward message 'setCheckBoxData'
::method setRadioButton unguarded
   forward message 'setRadioButtonData'
::method setMultiList unguarded
   forward message 'setListBoxData'
::method setValue unguarded
   forward message 'setControlData'
::method setAttrib unguarded
   forward message 'setControlDataAttribute'

::method setWindowTitle unguarded
   forward message 'setWindowText'
::method windowTitle unguarded
   forward message 'getWindowText'

::method sendMessageToItem unguarded
   forward message "sendMessageToControl"
::method getItem unguarded
   forward message "getControlHandle"
::method itemTitle unguarded
   forward message 'getControlText'
::method enableItem unguarded
   forward message 'enableControl'
::method disableItem unguarded
   forward message 'disableControl'
::method showItem unguarded
   forward message "showControl"
::method hideItem unguarded
   forward message "hideControl"
::method showItemFast unguarded
   forward message "showControlFast"
::method hideItemFast unguarded
   forward message "hideControlFast"
::method focusItem unguarded
   forward message "focusControl"

::method getStaticControl unguarded
   forward message 'newStatic'
::method getButtonControl unguarded
   forward message 'newPushButton'
::method getRadioControl unguarded
   forward message 'newRadioButton'
::method getCheckControl unguarded
   forward message 'newCheckBox'
::method getGroupBox unguarded
   forward message 'newGroupBox'
::method getEditControl unguarded
   forward message 'newEdit'
::method getComboBox unguarded
   forward message 'newComboBox'
::method getListBox unguarded
   forward message 'newListBox'
::method getScrollBar unguarded
   forward message 'newScrollBar'
::method getTreeControl unguarded
   forward message 'newTreeView'
::method getListControl unguarded
   forward message 'newListView'
::method getTabControl unguarded
   forward message 'newTab'
::method getProgressBar unguarded
   forward message 'newProgressBar'
::method getSliderControl unguarded
   forward message 'newTrackBar'

::method getTextSize unguarded
   forward message "getTextSizeDlg" continue
   return result~width result~height

