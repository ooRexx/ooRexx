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

/* Windows Dialog Interface for Open Object Rexx (ooRexx)
 *
 * The basic dialog class
 *
 * PlainBaseDialog with its two inherited mixin classes, WindowBase and
 * ResourceUtils, is the base dialog for all dialogs in the ooDialog framwork.
 */
::requires "oodutils.cls"

/* The WindowBase class implements methods that are common to all windows.  The
 * methods only require the operating system window handle to function.
 */
::class 'WindowBase' public mixinclass object

::method init_windowBase external "LIBRARY oodialog wb_init_windowBase"

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

::method Resize unguarded
   use arg width, hight, showOptions
   if arg(3,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   return WndShow_Pos("P",self~hwnd,0,0,width*self~factorX,hight*self~factorY,"NOMOVE "||showOptions )

::method Move unguarded
   use arg xPos, yPos, showOptions
   if arg(3,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   return WndShow_Pos("P",self~hwnd, xPos*self~factorX,yPos*self~factorY,0,0,"NOSIZE "||showOptions)

::method GetSize unguarded
   parse value Wnd_Desktop("RECT",self~Hwnd) with x y cx cy
   return (cx - x) % self~factorX || " " || (cy - y) % self~factorY

::method GetPos unguarded
   parse value Wnd_Desktop("RECT",self~Hwnd) with x y cx cy
   return (x%self~factorX) || " " || (y % self~factorY)

::method GetRect unguarded
   return WindowRect("GET", self~Hwnd)

::method GetClientRect unguarded
   use arg hwnd
   if arg(1,'o') = 1 then hwnd = self~hwnd
   return WindowRect("GET", hwnd, "CLIENT")

::method SetRect unguarded
   use arg x, y, width, high, showOptions
   if arg(5,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   return WndShow_Pos("P",self~hwnd, x, y, width, high, showOptions)

::method SetForegroundWindow unguarded
    use arg hwnd
    return Wnd_Desktop("TOP", hwnd)

::method ForegroundWindow unguarded
   return Wnd_Desktop("FG")

::method ScreenToClient unguarded
    use arg x,y
    return Wnd_Desktop("COORD", self~hwnd, x, y, "S2C")

::method ClientToScreen unguarded
    use arg x,y
    return Wnd_Desktop("COORD", self~hwnd, x, y, "C2S")

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

::attribute processingLoad private    -- in LoadItems ?

::method parseIncludeFile
  use strict arg hFile

  -- Revisit. Should a syntax condition be raised if the file is not found or readable?
  hFile = SysSearchPath("PATH", hFile)
  if hFile == "" then return .false

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

::method checkId
  use strict arg id

  if id~datatype("W") then return id

  if \ self~ConstDir~isA(.directory) then return self~idError(id)

  if self~ConstDir[id~space(0)~translate] = .nil then return self~idError(id)

  return self~resolveSymbolicId(id)

::method idError private
  use strict arg id
  j = errorDialog("Adding dialog resource:" id "undefined" || '0d0a'x || -
                   "non-numeric identification number.")
  return -1

::method resolveSymbolicId unguarded
   use strict arg id
   id = self~ConstDir[id~space(0)~translate]
   if id == .nil then return -1
   return id

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

::method resolveIconID private
  use strict arg icon

  if \ icon~Datatype('W') then icon = self~resolveSymbolicId(icon)
  if icon = -1 then icon = 0
  else if icon >= 1, icon <= 4 then icon += 10
  return icon

::method resolveResourceID unguarded
  use strict arg id
  if id~datatype("W"), id > -1 then return id
  return self~resolveSymbolicId(id)

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

/**
 *  The PlainBaseDialog class is the base class of all ooDialog dialogs.  It
 *  implements those methods that are common to all dialogs, whether the dialog
 *  template comes from a binary resource, a resource script file, or is con-
 *  structed dynamically.
 */
::class 'PlainBaseDialog' public inherit WindowBase ResourceUtils

::method init class external "LIBRARY oodialog pbdlg_init_cls"
::method setDefaultFont class external "LIBRARY oodialog pbdlg_setDefaultFont_cls"
::method getFontName class external "LIBRARY oodialog pbdlg_getFontName_cls"
::method getFontSize class external "LIBRARY oodialog pbdlg_getFontSize_cls"

-- External adminstration buffer. Get can not be private for now see MenuBar::attachTo()
--
::attribute adm get

::attribute autoDetect              -- Automatic data field detection on/off
::attribute dataConnection private  -- store connections between ids and class attributes
::attribute useStem private         -- flag whether to use a stem to set/get data or not
::attribute finished                -- finsished state of dialog, not a logical, can be 0, 1, or 2

::method dlgData private
   expose dlgData.
   return dlgData.

::attribute parentDlg private
::attribute childDialogs

::attribute fontName get
::attribute fontName set private

::attribute fontSize get
::attribute fontSize set private

::attribute dlgHandle get unguarded external "LIBRARY oodialog pbdlg_getDlgHandle"
::attribute dlgHandle set private unguarded external "LIBRARY oodialog pbdlg_setDlgHandle"

-- A queue containing the methods that will be started concurrently before execution of the dialog
::attribute automaticMethods private


/** PlainBaseDialog::init()  Performs the necessary intialization, including the
 * set up of the dialog management block, for all dialogs.
 */
::method init external "LIBRARY oodialog pbdlg_init"
::method finalInit private
   use arg includeFile

   self~childDialogs = .list~new

   self~fontName = .PlainBaseDialog~getFontName
   self~fontSize = .PlainBaseDialog~getFontSize

   -- call methods to get dialog settings
   self~initAutoDetection
   -- create array storing connection between ids and attributese
   self~dataConnection = .array~new(50)

   self~automaticMethods = .queue~new

   self~constDir = .directory~new
   self~constDir["IDOK"] = 1
   self~constDir["IDCANCEL"] = 2
   self~constDir["IDHELP"] = 9

   -- Common in Microsoft compatible resource script files
   self~constDir["IDC_STATIC"] = -1

   -- ooDialog supplied resources, defined in oodSymbols.h
   self~constDir["IDI_DLG_OODIALOG"] = 11
   self~constDir["IDI_DLG_APPICON"] = 12
   self~constDir["IDI_DLG_APPICON2"] = 13
   self~constDir["IDI_DLG_OOREXX"] = 14
   self~constDir["IDI_DLG_DEFAULT"] = 12

   if arg(1, 'E') then self~parseIncludeFile(includeFile)

   return 0

::method unInit external "LIBRARY oodialog pbdlg_unInit"

::method setDlgFont
  use strict arg name, size = 8
  self~fontName = name
  self~fontSize = size


   /* This method will be called after the Windows dialog has been created. */
   /* It is useful to set data fields and initialize combo & list boxes */

::method initDialog unguarded protected
   return 0

   /* This method will dispatch messages from dialog until OK or Cancel */
   /* push button has been pushed or ESC or Enter has been presed */

::method run unguarded protected
   use strict arg sleeptime = 0

   do while self~finished = 0
      self~handleMessages(sleeptime)
   end

   /* this method will create the dialog, show it (see method show), start */
   /* automatic methods and destroy the dialog. Data will be set and */
   /* received */

::method execute unguarded
   expose dlgData.
   use arg sh, icon = 0, sleeptime, nomodal
   icon = self~resolveIconId(icon)
   if sleeptime~Datatype('N') = 0 then sleeptime = 0   /* default is no sleep for execute */
   if arg(4, 'o') = 1 then nomodal = 0
   if self~startit(icon, nomodal) \= 0 then
   do
      if self~useStem then self~SetDataStem(dlgData.); else self~SetData
      self~show(sh)
      do while self~automaticMethods~Items > 0
         p = self~automaticMethods~pull
         m = self~automaticMethods~pull
         c = self~automaticMethods~pull
         o = c~new(p, self)
         o~start(m)
      end
      self~run(sleeptime)
      if self~initCode = 1 then do
         if self~useStem then self~GetDataStem(dlgData.)
             self~GetData
      end
      if self~adm \= 0 then self~stopit
      return self~initCode
   end
   else self~stopit
   return 0

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
          child~Stopit("PARENT")
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


::method get unguarded external "LIBRARY oodialog pbdlg_get"
::method getSelf unguarded external "LIBRARY oodialog pbdlg_getDlgHandle"
::method isDialogActive unguarded external "LIBRARY oodialog pbdlg_isDialogActive"
::method getItem unguarded external "LIBRARY oodialog pbdlg_getItem"

::method GetControlID unguarded
   use strict arg hWnd
   return WinAPI32Func("G", "ID", hWnd)

   /* This method shows the dialog */

::method Show unguarded
   use arg option = "NORMAL"
   if option = "" then option = "NORMAL"; else option = option~translate
   if option~wordpos("SHOWTOP") > 0 then self~ToTheTop
   return WndShow_Pos("S",self~dlgHandle, option)


   /* Use this method to make the dialog the front dialog */

::method ToTheTop unguarded
   return Wnd_Desktop("TOP",self~dlgHandle)


::method Minimize unguarded
   return WndShow_Pos("S",self~dlgHandle, "MIN")

::method Maximize unguarded
   return WndShow_Pos("S",self~dlgHandle, "MAX")

::method IsMaximized unguarded
   return WinAPI32Func("G", "WNDSTATE", "Z", self~dlgHandle)

::method IsMinimized unguarded
   return WinAPI32Func("G", "WNDSTATE", "I", self~dlgHandle)

::method Restore unguarded
   return WndShow_Pos("S",self~dlgHandle, "RESTORE")

-- Send the DM_REPOSITION  message to the dialog.
::method EnsureVisible unguarded
   return SendWinMsg("ANY", self~dlgHandle, "0x0402", 0, 0)


-- Connect WM_HELP messages (F1) to a method.
::method connectHelp
   use strict arg msgToRise
   return self~addUserMsg(msgToRise, "0x00000053", "0xFFFFFFFF", 0, 0, 0, 0, 0x00000101)

::method connectButton
   use strict arg id, msgToRise
   if \ id~dataType("N") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if id < 3 | id = 9 then return .true
   return self~addUserMsg(msgToRise, 0x00000111, 0xFFFFFFFF, id, 0xFFFFFFFF, 0, 0)

::method connectControl
   use strict arg id, msgToRise
   if \ id~dataType("N") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return self~addUserMsg(msgToRise, 0x00000111, 0xFFFFFFFF, id, 0x0000FFFF, 0,0)

::method connectList
   use strict arg id, msgToRise
   if \ id~dataType("N") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return self~addUserMsg(msgToRise, 0x00000111, 0xFFFFFFFF, '0x0001'||id~d2x(4), 0xFFFFFFFF, 0, 0)

::method connectListLeftDoubleClick
   use strict arg id, msgToRise
   if \ id~dataType("N") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return self~addUserMsg(msgToRise, 0x00000111, 0xFFFFFFFF, '0x0002'||id~d2x(4), 0xFFFFFFFF, 0, 0)

::method addUserMsg unguarded external "LIBRARY oodialog pbdlg_addUserMessage"

::method connectFKeyPress unguarded
   use strict arg msgToRaise
   if \ msgToRaise~isA(.String) then return -1
   return WinAPI32Func("K", 'C', self~adm, self~dlgHandle, msgToRaise~translate, "FKEYS", "NONE")

::method connectKeyPress unguarded
   use strict arg msgToRaise, keys, filter = ""
   if \ msgToRaise~isA(.String) | \ keys~isA(.String) | \ filter~isA(.String) then return -1
   if filter == "" then return WinAPI32Func("K", 'C', self~adm, self~dlgHandle, msgToRaise~translate, keys~space(0))
   return WinAPI32Func("K", 'C', self~adm, self~dlgHandle, msgToRaise~translate, keys~space(0), filter~translate)

::method disconnectKeyPress unguarded
   use arg msgToRaise
   if arg(1, 'O') then return WinAPI32Func("K", 'R', self~adm)
   if \ msgToRaise~isA(.String) then return -1
   return WinAPI32Func("K", 'R', self~adm, self~dlgHandle, msgToRaise~translate)

::method hasKeyPressConnection unguarded
   use arg msgToRaise
   if arg(1, 'O') then ret = WinAPI32Func("K", 'Q', self~adm)
   else do
      if \ msgToRaise~isA(.String) then ret = .false
      else ret = WinAPI32Func("K", 'Q', self~adm, msgToRaise~translate)
   end
   if \ ret~datatype('O') then return .false
   else return ret

/* The following methods are to connect an item of the data stem with */
/* a data field of the dialog. The data stem will start with number 1 */
/* and the dialog fields will be assigned in the same order they are  */
/* called */

::method ConnectEntryLine
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1
   return DataTable(self~adm,"ADD", result,0);                /* new id in result */

::method ConnectComboBox
   use arg id, attributeName, opts
   forward message "AddAttribute" continue
   id = result; if id = -1 then return -1              /* new id in result */
   if opts~translate~wordpos("LIST") = 0 then type = 0; else type = 5
   return DataTable(self~adm,"ADD",id,type);

::method ConnectCheckBox
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1
   return DataTable(self~adm,"ADD",result,1);                /* new id in result */

::method ConnectRadioButton
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1
   return DataTable(self~adm,"ADD",result,2);                /* new id in result */

::method ConnectListBox
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1
   return DataTable(self~adm,"ADD",result,3);                /* new id in result */

::method ConnectMultiListBox
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1
   return DataTable(self~adm,"ADD",result,4);                /* new id in result */

   /* Use this method to separate two radiobutton groups */

::method ConnectSeparator
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return DataTable(self~adm,"ADD",id,9);

   /* This method adds an attribute to the object. The attribute is associated */
   /* with the windows control 'id' */

::method AddAttribute protected
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
   interpret("self~"attributeName"=''")   /* initial value = "" */
   return id


   /* This method changes the title of a window */

::method SetWindowTitle unguarded
   use arg hwnd, text
   return Wnd_Desktop("SETTXT", hwnd, text)

::method WindowTitle unguarded
   use arg handle
   return Wnd_Desktop("TXT", handle)

   /* This method returns the title of the item 'id'  */
::method ItemTitle unguarded
   use arg id
   return Wnd_Desktop("TXT", self~getItem(id))

   /* This changes the text of a static text control */

::method SetStaticText unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString, self~dlgHandle, 0)

   /* This method loads the data attributes into the dialog */

::method SetData unguarded
   i = self~dataConnection~First
   do while (i \= .NIL)
      s = "self~" || self~dataConnection[i]
      s = "InternDlgData."i || " = " || s
      interpret(s)
      i = self~dataConnection~Next(i)
   end
   ret = SetStemData(self~adm, "InternDlgData.")

   /* This method receives data from dialog and copies them to the */
   /* associated attributes */

::method GetData unguarded
   ret = GetStemData(self~adm, "InternDlgData.")

   i = self~dataConnection~First
   do while (i \= .NIL)
      s = "self~" || self~dataConnection[i]
      s = s ||"= InternDlgData."i
      interpret(s)
      i = self~dataConnection~Next(i)
   end


   /* This method returns an array that is filled with all the dialog  */
   /* item's data */

::method MakeArray unguarded

   a = .Array~new(self~dataConnection~items)
   i = self~dataConnection~First
   do j=1 while (i \= .NIL)
      cml="a["j"]=self~"self~dataConnection~at(i)
      interpret( cml )
      i = self~dataConnection~Next(i)
   end
   return a


  /* This method sets all windows controls to the value of the given */
  /* 'dataStem' stem variable */

::method SetDataStem protected unguarded
   use arg dataStem.
   do k over dataStem.
      if k~dataType('W') then numericID = k
      else numericID = self~ResolveSymbolicID(k)
      InternDlgData.numericID = dataStem.k
      if InternDlgData.numericID = "INTERNDLGDATA."numericID then InternDlgData.numericID = ""
   end
   ret = SetStemData(self~adm, "InternDlgData.")

   /* This method gets the values of all the dialog's windows controls */
   /* and copies them to 'dataStem'. */

::method GetDataStem protected unguarded
   use arg dataStem.
   ret = GetStemData(self~adm, "InternDlgData.")
   do k over InternDlgData.
      dataStem.k = InternDlgData.k
      symbolicID = self~resolveNumericID(k)
      if symbolicID \== -1 then dataStem.symbolicID = InternDlgData.k
   end


::method BoxMessage unguarded private
   use arg id, msg, wP, lP
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return SendWinMsg("PTR", self~dlgHandle, id, msg, wP, lP)

   /* This method adds a string to the combobox */

::method AddComboEntry unguarded
   use arg id, string
   if arg(2,'o') = 1 | string = "DATA" then return -1
   return self~BoxMessage(id, 0x00000143, 0, "T" || string) + 1

   /* This method inserts a string to the combobox */

::method InsertComboEntry unguarded
   use arg id, item, string
   if arg(3,'o') = 1 | string = "DATA" then return -1
   if arg(2,"o") = 1 then item = self~GetCurrentComboIndex(id)
   return self~BoxMessage(id, 0x0000014A, item-1, "T" || string) + 1

   /* This method deletes a string from the combo box. */
   /* For argument 'index' don't use string but the index instead */
   /* Use 'FindComboEntry' to retrieve the index of an item. */

::method DeleteComboEntry unguarded
   use arg id, index
   if arg(2,"o") = 1 then index = self~GetCurrentComboIndex(id)
   return self~BoxMessage(id, 0x00000144, index-1, 0)


   /* This method finds a string in the combo box. It returns the */
   /* corresponding index */

::method FindComboEntry unguarded
   use arg id, string
   if arg(2,'o') = 1 | string = "DATA" then return -1
   item = self~BoxMessage(id, 0x0000014C, 0, "T" || string)
   if item >= 0 then return item + 1; else return 0


   /* This returns the index of the currently selected item */

::method GetCurrentComboIndex unguarded
   use arg id
   return self~BoxMessage(id, 0x00000147, 0, 0)+1


   /* Sets the selection index (0 means nothing selected) */

::method SetCurrentComboIndex unguarded
   use arg id, ndx
   if arg(2,'o') = 1 then ndx = 0
   return self~BoxMessage(id, 0x0000014E, ndx-1, 0)

   /* Returns the number of items within the combo box */

::method GetComboItems unguarded
   use arg id
   return self~BoxMessage(id, 0x00000146, 0, 0)

   /* Returns the text string of entry index */

::method GetComboEntry unguarded
   use arg id, ndx
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   len = SendWinMsg("DLG", self~dlgHandle, id, 0x00000149, ndx-1,0)
   if len <= 0 then return ""
   else return SendWinMsg("PTR", self~dlgHandle, id, 0x00000148, ndx-1,"G" || len+1)

   /* Changes the value of 'item' to 'string' */

::method ChangeComboEntry unguarded
   use arg id, item, string
   if arg(2,"o") = 1 then item = self~GetCurrentComboIndex(id)
   if item <= 0 then return -1
   self~DeleteComboEntry(id, item)
   return self~InsertComboEntry(id, item, string)


   /* add all filenames in the directory to the combo box */
   /* argument fileAttributes can be none, one or more of (seperated by blanks): */

::method ComboAddDirectory unguarded
   use arg id, drvpath, fileAttributes
   READWRITE  = 0
   READONLY   = 1
   HIDDEN     = 2
   SYSTEM     = 4
   DIRECTORY  = 16
   ARCHIVE    = 32
   interpret( "opts = "fileAttributes~translate('+',' ') )
   return self~BoxMessage(id, 0x00000145, opts, "T" || drvpath) + 1


   /* Deletes all strings of the combo box */

::method ComboDrop unguarded
   use arg id
   return self~BoxMessage(id, 0x0000014B, 0, 0)


/* The following methods are to manipulate a list box. */
/*  For remarks see the combo box methods */


   /* Adds 'data' (a string) to the list box 'id' */

::method AddListEntry unguarded
   use arg id, data
   if arg(2,'o') = 1 | data = "DATA" then return -1
   return self~BoxMessage(id, 0x00000180, 0, "T" || data) + 1

   /* Inserts 'data' (a string) into the list box 'id'. The new item */
   /* will be inserted after that with 'index' */

::method InsertListEntry unguarded
   use arg id, index, data
   if arg(3,'o') = 1 | data = "DATA" then return -1
   if arg(2,"o") = 1 then index = self~GetCurrentListIndex(id)
   return self~BoxMessage(id, 0x00000181, index-1, "T" || data) + 1

   /* Deletes an item from a list box. */
   /* For argument 'index' don't use string but the index */

::method DeleteListEntry unguarded
   use arg id, index
   if arg(2,"o") = 1 then index = self~GetCurrentListIndex(id)
   return self~BoxMessage(id, 0x00000182, index-1, 0)

   /* Returns the index of 'dataString' in the list box 'id' */

::method FindListEntry unguarded
   use arg id, dataString
   if arg(2,'o') = 1 | dataString = "DATASTRING" then return -1
   index = self~BoxMessage(id, 0x0000018F, 0, "T" || dataString)
   if index >= 0 then return index + 1; else return 0

   /* Returns the index of the currently selected list box item */

::method GetCurrentListIndex unguarded
   use arg id
   return self~BoxMessage(id, 0x00000188, 0, 0) + 1

   /* Sets the selection index (0 means nothing selected) */

::method SetCurrentListIndex unguarded
   use arg id, ndx
   if arg(2,'o') = 1 then ndx = 0
   return self~BoxMessage(id, 0x00000186, ndx-1, 0)

   /* Returns the number of items within the list box */

::method GetListItems unguarded
   use arg id
   return self~BoxMessage(id, 0x0000018B, 0, 0)

   /* Returns the text string of entry index */

::method GetListEntry unguarded
   use arg id, ndx
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   len = SendWinMsg("DLG", self~dlgHandle, id, 0x0000018A, ndx-1,0)
   if len <= 0 then return ""
   else return SendWinMsg("PTR", self~dlgHandle, id, 0x00000189, ndx-1,"G" || len+1)

   /* Changes the item at 'index' with 'dataString' */

::method ChangeListEntry unguarded
   use arg id, index, dataString
   if arg(2,"o") = 1 then index = self~GetCurrentListIndex(id)
   if index <= 0 then return -1
   self~DeleteListEntry(id, index)
   return self~InsertListEntry(id, index, dataString)

-- Sets the tab stops for a ListBox.
::method SetListTabulators unguarded external "LIBRARY oodialog generic_setListTabulators"

   /* Adds all filenames in the directory 'drvPath' to the list box */
   /* argument 'fileAttributes' can be none, one or more of (seperated by blanks): */
::method ListAddDirectory unguarded
   use arg id, drvPath, fileAttributes
   READWRITE  = 0
   READONLY   = 1
   HIDDEN     = 2
   SYSTEM     = 4
   DIRECTORY  = 16
   ARCHIVE    = 32
   interpret( "opts = "fileAttributes~translate('+',' ') )
   return self~BoxMessage(id, 0x0000018D, opts, "T" || drvpath) + 1

   /* Removes all items from list box 'id' */

::method ListDrop unguarded
   use arg id
   return self~BoxMessage(id, 0x00000184, 0, 0)


/* The following methods are to get the data entry from a single */
/*   dialog item */

::method InternalGetItemData unguarded
   use arg id, handle, type
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if arg(2,"o") = 1 then return GetItemData(self~adm, id)
   else if arg(3,"o") = 1 then return GetItemData(self~adm, id, handle)
   else return GetItemData(self~adm, id, handle, type)


   /* Returns the content of the entry line control with id 'id' */

::method GetEntryLine unguarded
   use arg id
   return self~InternalGetItemData(id, self~dlgHandle, 0)

   /* Returns the content of the currently selected item in a list box */

::method GetListLine unguarded
   use arg id
   return self~InternalGetItemData(id, self~dlgHandle, 3)

   /* Returns the content of the currently selected item in a combo box */

::method GetComboLine unguarded
   use arg id
   return self~InternalGetItemData(id, self~dlgHandle, 0)

   /* Returns 1 if the check box 'id' is selected, that is it has a check */
   /* mark. Otherwise it returns 0 */

::method GetCheckBox unguarded
   use arg id
   return self~InternalGetItemData(id, self~dlgHandle, 1)

   /* Returns 1 if the radio button 'id' is selected, otherwise it returns 0 */

::method GetRadioButton unguarded
   use arg id
   return self~InternalGetItemData(id, self~dlgHandle, 2)

   /* This method can be applied to a multiple choice list box. */
   /* It returns a string containing all the selected items' index. */
   /* The numbers are seperated by spaces. You can parse it using 'parse var' */

::method GetMultiList unguarded
   use arg id
   return self~InternalGetItemData(id, self~dlgHandle, 4)

   /* This method gets the value of a dialog item. You don't have to know */
   /* what kind of item it is when it has been connected before */

::method GetValue unguarded
    use arg id
    return self~InternalGetItemData(id, self~dlgHandle)

   /* Sets 'attributeName' with the value of the associated dialog field. */
   /* Returns  nothing! */

::method GetAttrib unguarded
   use arg attributeName
   i = self~dataConnection~First
   do while (i \= .NIL)
      if self~dataConnection[i] = attributeName~space(0) then
      do
         s = "self~"attributeName"= self~GetValue("i")"
         interpret(s)
         return
      end
      i = self~dataConnection~Next(i)
   end


 /* The following methods are to set the data entry of a single */
 /* dialog item */

::method InternalSetItemData unguarded
   use arg id, dataString, handle, type
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if arg(3,"o") = 1 then return SetItemData(self~adm, id, dataString)
   else if arg(4,"o") = 1 then return SetItemData(self~adm, id, dataString, handle)
   else return SetItemData(self~adm, id, dataString, handle, type)

   /* Puts 'dataString' into the entry line control 'id' */

::method SetEntryLine unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString, self~dlgHandle, 0)

   /* Puts 'dataString' into the list box 'id' */

::method SetListLine unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString, self~dlgHandle, 3)

   /* Puts 'dataString' into the combo box 'id' */

::method SetComboLine unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString, self~dlgHandle, 0)

   /* Puts a check mark to the check box if 'data' equals 1 */
   /* and remove the check mark if 'data' equals 0 */

::method SetCheckBox unguarded
   use arg id, data
   return self~InternalSetItemData(id, data, self~dlgHandle, 1)

   /* Checks the radio button if 'data' equals 1 */
   /* and unchecks it if 'data' equals 0 */

::method SetRadioButton unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString, self~dlgHandle, 2)

   /* Use this method to select one ore more lines in a multiple choice */
   /* list box. Provide the index of all lines you want to select */
   /* (seperated by blanks) in argument 'data'. */

::method SetMultiList unguarded
   use arg id, data
   return self~InternalSetItemData(id, data, self~dlgHandle, 4)

   /* This method sets the value of a dialog item. You don't have */
   /* to know what kind of item it is if it has been connected before */

::method SetValue unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString)

   /* Put the value of 'attributeName' into the associated dialog field */

::method SetAttrib unguarded
   use arg attributeName
   i = self~dataConnection~First
   do while (i \= .NIL)
      if self~dataConnection[i] = attributeName~space(0) then
      do
         s = "self~SetValue("i", self~"attributeName")"
         interpret(s)
         return
      end
      i = self~dataConnection~Next(i)
   end



   /* This method will handle one dialog message synchronously */

::method handleMessages unguarded protected
   use arg sleeptime
   if self~adm = 0 then do
       self~finished = 2
       return
   end
   if arg(1,'o') = 1 then sleeptime = 0
   msg = GetDlgMsg(self~adm)
   if msg~pos("1DLGDELETED1") > 0 then do
      if self~finished = 0 then self~finished = 2
   end
   else do
      if msg \= "" then interpret("self~"msg)
      else if sleeptime \= 0 then call msSleep sleeptime
   end

   /* This method will clear all pending dialog messages */

::method ClearMessages unguarded protected
   if self~adm = 0 then return
   msg = GetDlgMsg(self~adm)
   do while msg \= "" & self~adm \= 0
       if msg~pos("1DLGDELETED1") > 0 then do
          if self~finished = 0 then self~finished = 2
       end
       msg = GetDlgMsg(self~adm)
   end


 /* The following methods are to enable/disable and show/hide dialog items */
 /* The extension fast means that the method will not redraw the item */

::method EnableItem unguarded
   use arg id
   h = self~getItem(id)
   if h = 0 then return -1
   return Wnd_Desktop("ENABLE",h, 1)

::method DisableItem unguarded
   use arg id
   h = self~getItem(id)
   if h = 0 then return -1
   return Wnd_Desktop("ENABLE",h, 0)

::method HideItem unguarded
   use arg id
   h = self~getItem(id)
   if h = 0 then return -1
   return WndShow_Pos("S",h, "HIDE")

::method ShowItem unguarded
   use arg id
   h = self~getItem(id)
   if h = 0 then return -1
   return WndShow_Pos("S",h, "NORMAL")

::method HideWindow unguarded
   use arg hwnd
   return WndShow_Pos("S",hwnd, "HIDE")

::method ShowWindow unguarded  /* used for the category dialog */
   use arg hwnd
   return WndShow_Pos("S",hwnd, "NORMAL", self~adm)

   /* resize a dialog including border */
::method Resize unguarded
   use arg width, hight, showOptions
   if arg(3,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   parse value Wnd_Desktop("RECT", self~hwnd) with x y cx cy
   /* diffsx and diffsy are needed for the title and the frame */
   diffsx = cx - x - (self~sizeX * self~factorX)
   diffsy = cy - y - (self~sizeY * self~factorY)
   self~sizeX = width
   self~sizeY = hight
   return WndShow_Pos("P", self~dlgHandle, 0, 0, width * self~factorX + diffsx,,
                          hight* self~factorY + diffsy,"NOMOVE "||showOptions )

::method FocusItem unguarded
   use arg id
   return SendWinMsg("ANY", self~dlgHandle, "0x0028", self~getItem(id), 1)

::method TabToNext unguarded
   return Wnd_Desktop("SETFOC", self~dlgHandle, 0, "N")

::method TabToPrevious unguarded
   return Wnd_Desktop("SETFOC", self~dlgHandle, 1, "P")

::method getTextSize unguarded
  forward message "getTextSizeDlg" continue
  return result~width result~height

::method getTextSizeDlg unguarded external "LIBRARY oodialog pbdlg_getTextSizeDlg"


 -- 2 methods to set / remove the WS_TABSTOP / WS_GROUP styles for any control.
::method SetTabStop unguarded
   use strict arg id, wantStop = .true
   if \ id~datatype("W") then id = self~resolveSymbolicId(id)
   if id == -1 then return -1
   if \ wantStop~datatype('O')then return -3
   return HandleControlEx(self~dlgHandle, id, "X", "TAB", wantStop)

::method SetGroup unguarded
   use strict arg id, wantGroup = .true
   if \ id~datatype("W") then id = self~resolveSymbolicId(id)
   if id == -1 then return -1
   if \ wantGroup~datatype('O')then return -3
   return HandleControlEx(self~dlgHandle, id, "X", "GROUP", wantGroup)

::method Center unguarded
   use arg showOptions
   if arg(1,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   parse value self~GetSize with dcx dcy
   s = screenSize()
   return WndShow_Pos("P",self~dlgHandle, (s[3] - dcx * self~factorX) %2, (s[4] - dcy * self~factorY) %2, 0, 0,,
                           "NOSIZE "||showOptions)


   /* This method will be called to get the default setting for the */
   /* automatic data field detection. If you disable the ADD, you'll */
   /* have to use the Connect... methods to assign a dialog item */
   /* to a data stem field */

::method InitAutoDetection protected /* autodetection is enabled by default */
   self~AutoDetection

::method NoAutoDetection    /* disable autodetection */
   self~autoDetect = .false

::method AutoDetection      /* enable autodetection */
   self~autoDetect = .true

 /* The following methods are abstract methods that will be called */
 /* whenever the push buttons 1, 2, or 9 will be pressed */
 /* Method 'OK' will call method 'Validate' */
 /* If self~finished = 1 the dialog is destroyed, if 0 the dialog */
 /* execution will be continued */
 /* OK returns 1 if the dialog is going to be destroyed */

::method OK unguarded
   self~finished = self~Validate
   self~initCode = 1
   return self~finished

 /* If self~finished = 1 the dialog is destroyed, if 0 the dialog */
 /* execution will be continued */

::method Cancel unguarded
   self~finished = 1
   if self~finished = 1 then self~initCode = 2
   return self~finished

::method Help unguarded
   return 0

::method Leaving
   return

/** validate()
 * This method is an abstract method that will be called to determin whether the
 * dialog can be closed or not. This function will be called by method OK.  If
 * the return value is 1 the dialog can be closed, if it is 0 the dialog
 * values are invalid.
 */
::method validate unguarded
   return 1

-- DEPRECATED
::method deInstall
  return 0

::class 'Window' public inherit WindowBase
::method init  external "LIBRARY oodialog window_init"
::method unInit  external "LIBRARY oodialog window_unInit"
