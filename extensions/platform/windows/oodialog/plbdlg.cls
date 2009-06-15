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

/******************************************************************************/
/* The class WindowBase implements methods that are common to all windows,    */
/* if it is a dialog or a dialog item                                         */
/******************************************************************************/

::class 'WindowBase' public MIXINCLASS object

::method Hwnd attribute
::method InitCode attribute        /* attribute to get the result of init */

    /* Size and Dialog Unit Factor */
::method SizeX attribute
::method SizeY attribute
::method FactorX attribute
::method FactorY attribute

::method Unknown unguarded
   use arg msgname
   call errorDialog msgname || " is not a method of " || self

::method init_WindowBase
   use arg hwnd

   self~SizeX = 0
   self~SizeY = 0
   parse value GetDialogFactor() with x y
   self~FactorX=x
   self~FactorY=Y

   if arg(1,'o') then self~Hwnd = 0
   else do
       self~hwnd = hwnd
       parse value Wnd_Desktop("RECT", hwnd) with x y cx cy
       self~SizeX = cx % self~FactorX
       self~SizeY = cy % self~FactorY
   end

   return 0


::method AssignWindow unguarded
   use arg hwnd
   self~Hwnd = hwnd
   parse value Wnd_Desktop("RECT", hwnd) with x y cx cy
   self~SizeX = cx % self~FactorX
   self~SizeY = cy % self~FactorY


::method Enable unguarded
   return Wnd_Desktop("ENABLE",self~Hwnd, 1)

::method Disable unguarded
   return Wnd_Desktop("ENABLE",self~Hwnd, 0)

::method Hide unguarded
   return WndShow_Pos("S", self~Hwnd, "HIDE")

::method Show unguarded
   return WndShow_Pos("S",self~Hwnd, "NORMAL")

::method IsVisible unguarded
   return WinAPI32Func("G", "WNDSTATE", "V", self~hwnd)

::method IsEnabled unguarded
   return WinAPI32Func("G", "WNDSTATE", "E", self~hwnd)

::method Resize unguarded
   use arg width, hight, showOptions
   if Arg(3,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   return WndShow_Pos("P",self~hwnd,0,0,width*self~FactorX,hight*self~FactorY,"NOMOVE "||showOptions )

::method Move unguarded
   use arg xPos, yPos, showOptions
   if Arg(3,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   return WndShow_Pos("P",self~hwnd, xPos*self~FactorX,yPos*self~FactorY,0,0,"NOSIZE "||showOptions)

::method GetSize unguarded
   parse value Wnd_Desktop("RECT",self~Hwnd) with x y cx cy
   return (cx - x) % self~FactorX || " " || (cy - y) % self~FactorY

::method GetPos unguarded
   parse value Wnd_Desktop("RECT",self~Hwnd) with x y cx cy
   return (x%self~FactorX) || " " || (y % self~FactorY)

::method "Title=" unguarded
   use arg text
   call Wnd_Desktop "SETTXT", self~hwnd, text

::method SetTitle unguarded
   use arg text
   call Wnd_Desktop "SETTXT", self~hwnd, text

::method Title unguarded
   return Wnd_Desktop("TXT", self~hwnd)

::method GetID unguarded
   return Wnd_Desktop("ID", self~hwnd)

::method getStyleRaw unguarded external "LIBRARY oodialog wb_getStyleRaw"
::method getExStyleRaw unguarded external "LIBRARY oodialog wb_getExStyleRaw"

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

::method Checkfile private
   use arg f
   if (f~lines = 0) then do
      f~close
      self~ProcessingLoad = 0
      ret = errorDialog("Error reading resource file!" f)
      return 1
   end
   return 0

::method errorfile private
   use arg f, s
   f~close
   self~ProcessingLoad = 0
   ret = errorDialog("Error reading resource file:" f "("s")")

::method checkId
  use strict arg id

  if id~datatype("W") then return id

  if \ self~ConstDir~isA(.directory) then return self~idError(id)

  if self~ConstDir[id~space(0)~translate] = .nil then return self~idError(id)

  return self~ResolveSymbolicId(id)

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

  if \ icon~Datatype('W') then icon = self~ResolveSymbolicId(icon)
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

/******************************************************************************/
/* The class PlainBaseDialog implements methods that are common to a dialog   */
/* by Resource Workshop (ResDialog) and to a dialog created by using Add...   */
/* methods (UserDialog).                                                      */
/******************************************************************************/

::class 'PlainBaseDialog' public inherit WindowBase ResourceUtils

::method init class external "LIBRARY oodialog pbdlg_init_cls"
::method setDefaultFont class external "LIBRARY oodialog pbdlg_setDefaultFont_cls"
::method getFontName class external "LIBRARY oodialog pbdlg_getFontName_cls"
::method getFontSize class external "LIBRARY oodialog pbdlg_getFontSize_cls"


::method Adm attribute protected   /* external adminstration buffer */
::method AutoDetect attribute      /* Automatic data field detection on/off */
::method DataConnection attribute protected /* store connections between ids and class attributes */
::method UseStem attribute protected   /* flag whether to use a stem to set/get data or not */
::method DlgData private
   expose DlgData.
   return DlgData.

::method Finished attribute
::method ParentDlg attribute protected

::attribute fontName get
::attribute fontName set private

::attribute fontSize get
::attribute fontSize set private

  /* A queue containing the methods that will be started concurrently before */
  /* execution of the dialog */
::method AutomaticMethods attribute protected

::method ChildDialogs attribute

/** PlainBaseDialog::init()  Performs the necessary intialization, including the
 * set up of the dialog management block, for all dialogs.
 */
::method Init protected
   expose Library Resource DlgData.
   use arg Library, Resource, DlgData., includeFile

   self~Adm = 0
   self~ParentDlg = .Nil

   /* Initialize the WindowBase */
   self~initCode = self~init_WindowBase

   /* prepare the dialog management for a new dialog */
   self~Adm = HandleDialogAdmin()  /* allocate a new dialog table */
   if self~Adm = 0 then do
      self~Deinstall
      self~InitCode = 1
      return 1
   end

   /* call methods to get dialog settings */
   self~InitAutoDetection
   /* create array storing connection between ids and attributes */
   self~DataConnection = .array~new(50)
   /* there was no stem argument so don't use stems */
   if Arg(3, 'o') = 1 then self~UseStem = 0; else self~UseStem = 1
   self~finished = 0
   self~AutomaticMethods = .queue~new    /* create a new queue */
   self~AddUserMsg("OK", 0x00000111, 0xFFFFFFFF, 1, 0xFFFFFFFF, 0, 0)
   self~AddUserMsg("Cancel", 0x00000111, 0xFFFFFFFF, 2, 0xFFFFFFFF, 0, 0)
   self~AddUserMsg("Help", 0x00000111, 0xFFFFFFFF, 9, 0xFFFFFFFF, 0, 0)
   self~ProcessingLoad = 0
   self~DlgHandle = 0
   self~ChildDialogs = .list~new

   self~ConstDir = .directory~new
   self~ConstDir["IDOK"] = 1
   self~ConstDir["IDCANCEL"] = 2
   self~ConstDir["IDHELP"] = 9

   -- Common in Microsoft compatible resource script files
   self~ConstDir["IDC_STATIC"] = -1

   -- ooDialog supplied resources, defined in oodSymbols.h
   self~ConstDir["IDI_DLG_OODIALOG"] = 11
   self~ConstDir["IDI_DLG_APPICON"] = 12
   self~ConstDir["IDI_DLG_APPICON2"] = 13
   self~ConstDir["IDI_DLG_OOREXX"] = 14
   self~ConstDir["IDI_DLG_DEFAULT"] = 12

   self~fontName = .PlainBaseDialog~getFontName
   self~fontSize = .PlainBaseDialog~getFontSize

   if arg(4, 'E') then
     self~ParseIncludeFile(includeFile)

   return 0

::method DlgHandle unguarded
   return self~hwnd

::method "DlgHandle=" unguarded
   use arg hwnd
   self~hwnd=hwnd

   /* This method will be called after the Windows dialog has been created. */
   /* It is useful to set data fields and initialize combo & list boxes */

::method InitDialog unguarded protected
   return 0

   /* This method will dispatch messages from dialog until OK or Cancel */
   /* push button has been pushed or ESC or Enter has been presed */

::method Run unguarded protected
   use strict arg sleeptime = 0

   do while self~finished = 0
      self~HandleMessages(sleeptime)
   end

   /* this method will create the dialog, show it (see method show), start */
   /* automatic methods and destroy the dialog. Data will be set and */
   /* received */

::method Execute unguarded
   expose DlgData.
   use arg sh, icon = 0, sleeptime, nomodal
   icon = self~resolveIconId(icon)
   if sleeptime~Datatype('N') = 0 then sleeptime = 0   /* default is no sleep for execute */
   if Arg(4, 'o') = 1 then nomodal = 0
   if self~startit(icon, nomodal) \= 0 then
   do
      if self~UseStem = 1 then self~SetDataStem(DlgData.); else self~SetData
      self~show(sh)
      do while self~AutomaticMethods~Items > 0
         p = self~AutomaticMethods~pull
         m = self~AutomaticMethods~pull
         c = self~AutomaticMethods~pull
         o = c~new(p, self)
         o~start(m)
      end
      self~run(sleeptime)
      if self~InitCode = 1 then do
         if (self~UseStem = 1) then self~GetDataStem(DlgData.)
             self~GetData
      end
      if self~Adm \= 0 then self~stopit
      return self~InitCode
   end
   else self~stopit
   return 0

   /* This method deletes the Windows dialog */

::method Stopit
   use arg caller
   if self~DlgHandle = 0 | self~Adm = 0 then return -1
   self~Leaving
   ret =  HandleDlg("STOP", self~DlgHandle)
   self~DlgHandle = 0
   self~Adm = 0

   /* we ourselves are a child dialog and not called by the parent's stopit */
   if self~ParentDlg \= .Nil & caller \= "PARENT" then self~ParentDlg~ChildDied(self)

   /* if we have child dialogs, stop all of them */
   if self~ChildDialogs \= .Nil then do
       if self~ChildDialogs~items > 0 then
       do child over self~ChildDialogs
          child~Stopit("PARENT")
       end
       self~ChildDialogs = .Nil
   end
   return ret


   /* remove a child dialog from the chain */
::method ChildDied
   use arg child
   if self~ChildDialogs \= .Nil then do
       ndx = self~ChildDialogs~first
       do while ndx \= .Nil
          if self~ChildDialogs[ndx] == child then do
              self~ChildDialogs~remove(ndx)
              ndx = .Nil
          end
          else ndx = self~ChildDialogs~next(ndx)
       end
   end


   /* This method returns the handle of the current dialog */

::method Get unguarded
   return HandleDlg("HNDL")

::method GetSelf unguarded
   return HandleDlg("HNDL", self~Adm)


   /* This method returns the handle of a dialog item */

::method GetItem unguarded
   use arg id, hDlg
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return 0
   if Arg(2,'o') = 1 then hDlg = self~DlgHandle
   return HandleDlg("ITEM",id, hDlg)

::method GetControlID unguarded
   use strict arg hWnd
   return WinAPI32Func("G", "ID", hWnd)

   /* This method shows the dialog */

::method Show unguarded
   use arg option = "NORMAL"
   if option = "" then option = "NORMAL"; else option = option~translate
   if option~wordpos("SHOWTOP") > 0 then self~ToTheTop
   return WndShow_Pos("S",self~DlgHandle, option)


   /* Use this method to make the dialog the front dialog */

::method ToTheTop unguarded
   return Wnd_Desktop("TOP",self~DlgHandle)


::method Minimize unguarded
   return WndShow_Pos("S",self~DlgHandle, "MIN")

::method Maximize unguarded
   return WndShow_Pos("S",self~DlgHandle, "MAX")

::method IsMaximized unguarded
   return WinAPI32Func("G", "WNDSTATE", "Z", self~DlgHandle)

::method IsMinimized unguarded
   return WinAPI32Func("G", "WNDSTATE", "I", self~DlgHandle)

::method Restore unguarded
   return WndShow_Pos("S",self~DlgHandle, "RESTORE")

-- Send the DM_REPOSITION  message to the dialog.
::method EnsureVisible unguarded
   return SendWinMsg("ANY", self~DlgHandle, "0x0402", 0, 0)


-- Connect WM_HELP messages (F1) to a method.
::method ConnectHelp
   use strict arg msgToRise
   return self~addUserMsg(msgToRise, "0x00000053", "0xFFFFFFFF", 0, 0, 0, 0, 0x00000101)

/* The following methods create a connection between a class method */
/* and a dialog item (dialog control). The connected method will be */
/* called whenever the dialog item will be activated (i.g. a push */
/* button will be pushed) */

   /* Connect a push button with a method */

::method ConnectButton
   use arg id, msgToRise
   if Arg(2, 'o') = 1 then msgToRise = ""
   if id~DataType("N") = 0 then do
       if self~ProcessingLoad = 1 then msgToRise = id
       id = self~ResolveSymbolicId(id)
       if id = -1 then return -1
   end
   if id < 3 | id = 9 then return 0
   return self~AddUserMsg(msgToRise, 0x00000111, 0xFFFFFFFF, id, 0xFFFFFFFF, 0, 0)


   /* Connect any dialog control with a method */

::method ConnectControl
   use arg id, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return self~AddUserMsg(msgToRise, 0x00000111, 0xFFFFFFFF, id, 0x0000FFFF, 0,0)


   /* Connect a list with a method */

::method ConnectList
   use arg id, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return self~AddUserMsg(msgToRise, 0x00000111, 0xFFFFFFFF, '0x0001'||id~d2x(4), 0xFFFFFFFF, 0, 0)

   /* Connect a list left double-click with a method */

::method ConnectListLeftDoubleClick
   use arg id, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return self~AddUserMsg(msgToRise, 0x00000111, 0xFFFFFFFF, '0x0002'||id~d2x(4), 0xFFFFFFFF, 0, 0)


   /* Connect any Windows message with a class method */

::method AddUserMsg protected unguarded
   use arg msgToRise, msgWindows, filt1, wP, filt2, lP, filt3, tag
   if Arg(3, 'o') = 1 | filt1 = "FILT1" then filt1 = 0xFFFFFFFF
   if Arg(5, 'o') = 1 | filt2 = "FILT2" then filt2 = 0
   if Arg(7, 'o') = 1 | filt3 = "FILT3" then filt3 = 0
   if Arg(4, 'o') = 1 | wP = "WP" then wP = 0
   if Arg(6, 'o') = 1 | lP = "LP" then lP = 0
   if msgToRise~space(0) = "" then return 1
   if Arg(8, 'o') = 1 then return AddUserMessage(self~Adm, msgWindows, filt1, wP, filt2, lP, filt3, msgToRise)
   else return AddUserMessage(self~Adm, msgWindows, filt1, wP, filt2, lP, filt3, msgToRise, tag)

::method ConnectFKeyPress unguarded
   use strict arg msgToRaise
   if \ msgToRaise~isA(.String) then return -1
   return WinAPI32Func("K", 'C', self~adm, self~dlgHandle, msgToRaise~translate, "FKEYS", "NONE")

::method ConnectKeyPress unguarded
   use strict arg msgToRaise, keys, filter = ""
   if \ msgToRaise~isA(.String) | \ keys~isA(.String) | \ filter~isA(.String) then return -1
   if filter == "" then return WinAPI32Func("K", 'C', self~adm, self~dlgHandle, msgToRaise~translate, keys~space(0))
   return WinAPI32Func("K", 'C', self~adm, self~dlgHandle, msgToRaise~translate, keys~space(0), filter~translate)

::method DisconnectKeyPress unguarded
   use arg msgToRaise
   if arg(1, 'O') then return WinAPI32Func("K", 'R', self~adm)
   if \ msgToRaise~isA(.String) then return -1
   return WinAPI32Func("K", 'R', self~adm, self~dlgHandle, msgToRaise~translate)

::method HasKeyPressConnection unguarded
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
   return DataTable(self~Adm,"ADD", result,0);                /* new id in result */

::method ConnectComboBox
   use arg id, attributeName, opts
   forward message "AddAttribute" continue
   id = result; if id = -1 then return -1              /* new id in result */
   if opts~translate~wordpos("LIST") = 0 then type = 0; else type = 5
   return DataTable(self~Adm,"ADD",id,type);

::method ConnectCheckBox
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1
   return DataTable(self~Adm,"ADD",result,1);                /* new id in result */

::method ConnectRadioButton
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1
   return DataTable(self~Adm,"ADD",result,2);                /* new id in result */

::method ConnectListBox
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1
   return DataTable(self~Adm,"ADD",result,3);                /* new id in result */

::method ConnectMultiListBox
   use arg id, attributeName
   forward message "AddAttribute" continue
   if result = -1 then return -1
   return DataTable(self~Adm,"ADD",result,4);                /* new id in result */

   /* Use this method to separate two radiobutton groups */

::method ConnectSeparator
   use arg id
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return DataTable(self~Adm,"ADD",id,9);

   /* This method adds an attribute to the object. The attribute is associated */
   /* with the windows control 'id' */

::method AddAttribute protected
   use arg id, attributeName
   if Arg(2,'o') = 1 then attributeName = "DATA"id~space(0)
   if id~DataType("N") = 0 then do
       if self~ProcessingLoad = 1 then attributeName = id
       id = self~ResolveSymbolicId(id)
   end
   if id < 1 then return -1
   if attributeName~space(0) = "" | self~HasMethod(attributeName) = 1 then
       attributeName = "DATA"id~space(0)
   else do
      attributeName = attributeName~space(0)~changestr('&','')~changestr(':','') /* remove blanks, &, and : */
      if attributeName~datatype('V')=0 then attributeName = "DATA"id~space(0)
   end

   self~DataConnection[id] = attributeName
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
   return Wnd_Desktop("TXT", self~GetItem(id))

   /* This changes the text of a static text control */

::method SetStaticText unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString, self~DlgHandle, 0)

   /* This method loads the data attributes into the dialog */

::method SetData unguarded
   i = self~DataConnection~First
   do while (i \= .NIL)
      s = "self~" || self~DataConnection[i]
      s = "InternDlgData."i || " = " || s
      interpret(s)
      i = self~DataConnection~Next(i)
   end
   ret = SetStemData(self~Adm, "InternDlgData.")

   /* This method receives data from dialog and copies them to the */
   /* associated attributes */

::method GetData unguarded
   ret = GetStemData(self~Adm, "InternDlgData.")

   i = self~DataConnection~First
   do while (i \= .NIL)
      s = "self~" || self~DataConnection[i]
      s = s ||"= InternDlgData."i
      interpret(s)
      i = self~DataConnection~Next(i)
   end


   /* This method returns an array that is filled with all the dialog  */
   /* item's data */

::method MakeArray unguarded

   a = .Array~new(self~DataConnection~items)
   i = self~DataConnection~First
   do j=1 while (i \= .NIL)
      cml="a["j"]=self~"self~DataConnection~at(i)
      interpret( cml )
      i = self~DataConnection~Next(i)
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
   ret = SetStemData(self~Adm, "InternDlgData.")

   /* This method gets the values of all the dialog's windows controls */
   /* and copies them to 'dataStem'. */

::method GetDataStem protected unguarded
   use arg dataStem.
   ret = GetStemData(self~Adm, "InternDlgData.")
   do k over InternDlgData.
      dataStem.k = InternDlgData.k
      symbolicID = self~resolveNumericID(k)
      if symbolicID \== -1 then dataStem.symbolicID = InternDlgData.k
   end


::method BoxMessage unguarded private
   use arg id, msg, wP, lP
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return SendWinMsg("PTR", Self~DlgHandle, id, msg, wP, lP)

   /* This method adds a string to the combobox */

::method AddComboEntry unguarded
   use arg id, string
   if Arg(2,'o') = 1 | string = "DATA" then return -1
   return self~BoxMessage(id, 0x00000143, 0, "T" || string) + 1

   /* This method inserts a string to the combobox */

::method InsertComboEntry unguarded
   use arg id, item, string
   if Arg(3,'o') = 1 | string = "DATA" then return -1
   if Arg(2,"o") = 1 then item = self~GetCurrentComboIndex(id)
   return self~BoxMessage(id, 0x0000014A, item-1, "T" || string) + 1

   /* This method deletes a string from the combo box. */
   /* For argument 'index' don't use string but the index instead */
   /* Use 'FindComboEntry' to retrieve the index of an item. */

::method DeleteComboEntry unguarded
   use arg id, index
   if Arg(2,"o") = 1 then index = self~GetCurrentComboIndex(id)
   return self~BoxMessage(id, 0x00000144, index-1, 0)


   /* This method finds a string in the combo box. It returns the */
   /* corresponding index */

::method FindComboEntry unguarded
   use arg id, string
   if Arg(2,'o') = 1 | string = "DATA" then return -1
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
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   len = SendWinMsg("DLG", Self~DlgHandle, id, 0x00000149, ndx-1,0)
   if len <= 0 then return ""
   else return SendWinMsg("PTR", Self~DlgHandle, id, 0x00000148, ndx-1,"G" || len+1)

   /* Changes the value of 'item' to 'string' */

::method ChangeComboEntry unguarded
   use arg id, item, string
   if Arg(2,"o") = 1 then item = self~GetCurrentComboIndex(id)
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
   if Arg(2,'o') = 1 | data = "DATA" then return -1
   return self~BoxMessage(id, 0x00000180, 0, "T" || data) + 1

   /* Inserts 'data' (a string) into the list box 'id'. The new item */
   /* will be inserted after that with 'index' */

::method InsertListEntry unguarded
   use arg id, index, data
   if Arg(3,'o') = 1 | data = "DATA" then return -1
   if Arg(2,"o") = 1 then index = self~GetCurrentListIndex(id)
   return self~BoxMessage(id, 0x00000181, index-1, "T" || data) + 1

   /* Deletes an item from a list box. */
   /* For argument 'index' don't use string but the index */

::method DeleteListEntry unguarded
   use arg id, index
   if Arg(2,"o") = 1 then index = self~GetCurrentListIndex(id)
   return self~BoxMessage(id, 0x00000182, index-1, 0)

   /* Returns the index of 'dataString' in the list box 'id' */

::method FindListEntry unguarded
   use arg id, dataString
   if Arg(2,'o') = 1 | dataString = "DATASTRING" then return -1
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
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   len = SendWinMsg("DLG", Self~DlgHandle, id, 0x0000018A, ndx-1,0)
   if len <= 0 then return ""
   else return SendWinMsg("PTR", Self~DlgHandle, id, 0x00000189, ndx-1,"G" || len+1)

   /* Changes the item at 'index' with 'dataString' */

::method ChangeListEntry unguarded
   use arg id, index, dataString
   if Arg(2,"o") = 1 then index = self~GetCurrentListIndex(id)
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
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if arg(2,"o") = 1 then return GetItemData(self~Adm, id)
   else if arg(3,"o") = 1 then return GetItemData(self~Adm, id, handle)
   else return GetItemData(self~Adm, id, handle, type)


   /* Returns the content of the entry line control with id 'id' */

::method GetEntryLine unguarded
   use arg id
   return self~InternalGetItemData(id, self~DlgHandle, 0)

   /* Returns the content of the currently selected item in a list box */

::method GetListLine unguarded
   use arg id
   return self~InternalGetItemData(id, self~DlgHandle, 3)

   /* Returns the content of the currently selected item in a combo box */

::method GetComboLine unguarded
   use arg id
   return self~InternalGetItemData(id, self~DlgHandle, 0)

   /* Returns 1 if the check box 'id' is selected, that is it has a check */
   /* mark. Otherwise it returns 0 */

::method GetCheckBox unguarded
   use arg id
   return self~InternalGetItemData(id, self~DlgHandle, 1)

   /* Returns 1 if the radio button 'id' is selected, otherwise it returns 0 */

::method GetRadioButton unguarded
   use arg id
   return self~InternalGetItemData(id, self~DlgHandle, 2)

   /* This method can be applied to a multiple choice list box. */
   /* It returns a string containing all the selected items' index. */
   /* The numbers are seperated by spaces. You can parse it using 'parse var' */

::method GetMultiList unguarded
   use arg id
   return self~InternalGetItemData(id, self~DlgHandle, 4)

   /* This method gets the value of a dialog item. You don't have to know */
   /* what kind of item it is when it has been connected before */

::method GetValue unguarded
    use arg id
    return self~InternalGetItemData(id, self~DlgHandle)

   /* Sets 'attributeName' with the value of the associated dialog field. */
   /* Returns  nothing! */

::method GetAttrib unguarded
   use arg attributeName
   i = self~DataConnection~First
   do while (i \= .NIL)
      if self~DataConnection[i] = attributeName~space(0) then
      do
         s = "self~"attributeName"= self~GetValue("i")"
         interpret(s)
         return
      end
      i = self~DataConnection~Next(i)
   end


 /* The following methods are to set the data entry of a single */
 /* dialog item */

::method InternalSetItemData unguarded
   use arg id, dataString, handle, type
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if arg(3,"o") = 1 then return SetItemData(self~Adm, id, dataString)
   else if arg(4,"o") = 1 then return SetItemData(self~Adm, id, dataString, handle)
   else return SetItemData(self~Adm, id, dataString, handle, type)

   /* Puts 'dataString' into the entry line control 'id' */

::method SetEntryLine unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString, self~DlgHandle, 0)

   /* Puts 'dataString' into the list box 'id' */

::method SetListLine unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString, self~DlgHandle, 3)

   /* Puts 'dataString' into the combo box 'id' */

::method SetComboLine unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString, self~DlgHandle, 0)

   /* Puts a check mark to the check box if 'data' equals 1 */
   /* and remove the check mark if 'data' equals 0 */

::method SetCheckBox unguarded
   use arg id, data
   return self~InternalSetItemData(id, data, self~DlgHandle, 1)

   /* Checks the radio button if 'data' equals 1 */
   /* and unchecks it if 'data' equals 0 */

::method SetRadioButton unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString, self~DlgHandle, 2)

   /* Use this method to select one ore more lines in a multiple choice */
   /* list box. Provide the index of all lines you want to select */
   /* (seperated by blanks) in argument 'data'. */

::method SetMultiList unguarded
   use arg id, data
   return self~InternalSetItemData(id, data, self~DlgHandle, 4)

   /* This method sets the value of a dialog item. You don't have */
   /* to know what kind of item it is if it has been connected before */

::method SetValue unguarded
   use arg id, dataString
   return self~InternalSetItemData(id, dataString)

   /* Put the value of 'attributeName' into the associated dialog field */

::method SetAttrib unguarded
   use arg attributeName
   i = self~DataConnection~First
   do while (i \= .NIL)
      if self~DataConnection[i] = attributeName~space(0) then
      do
         s = "self~SetValue("i", self~"attributeName")"
         interpret(s)
         return
      end
      i = self~DataConnection~Next(i)
   end


   /* Returns 1 (true) if Windows dialog is still existing */

::method IsDialogActive unguarded
   return HandleDlg("ACTIVE", self~DlgHandle)


   /* This method will handle one dialog message synchronously */

::method HandleMessages unguarded protected
   use arg sleeptime
   if self~Adm = 0 then do
       self~finished = 2
       return
   end
   if Arg(1,'o') = 1 then sleeptime = 0
   msg = GetDlgMsg(self~Adm)
   if msg~pos("1DLGDELETED1") > 0 then do
      if self~finished = 0 then self~finished = 2
   end
   else do
      if msg \= "" then interpret("self~"msg)
      else if sleeptime \= 0 then call msSleep sleeptime
   end

   /* This method will clear all pending dialog messages */

::method ClearMessages unguarded protected
   if self~Adm = 0 then return
   msg = GetDlgMsg(self~Adm)
   do while msg \= "" & self~Adm \= 0
       if msg~pos("1DLGDELETED1") > 0 then do
          if self~finished = 0 then self~finished = 2
       end
       msg = GetDlgMsg(self~Adm)
   end


 /* The following methods are to enable/disable and show/hide dialog items */
 /* The extension fast means that the method will not redraw the item */

::method EnableItem unguarded
   use arg id
   h = self~GetItem(id)
   if h = 0 then return -1
   return Wnd_Desktop("ENABLE",h, 1)

::method DisableItem unguarded
   use arg id
   h = self~GetItem(id)
   if h = 0 then return -1
   return Wnd_Desktop("ENABLE",h, 0)

::method HideItem unguarded
   use arg id
   h = self~GetItem(id)
   if h = 0 then return -1
   return WndShow_Pos("S",h, "HIDE")

::method ShowItem unguarded
   use arg id
   h = self~GetItem(id)
   if h = 0 then return -1
   return WndShow_Pos("S",h, "NORMAL")

::method HideWindow unguarded
   use arg hwnd
   return WndShow_Pos("S",hwnd, "HIDE")

::method ShowWindow unguarded  /* used for the category dialog */
   use arg hwnd
   return WndShow_Pos("S",hwnd, "NORMAL", self~Adm)

   /* resize a dialog including border */
::method Resize unguarded
   use arg width, hight, showOptions
   if Arg(3,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   parse value Wnd_Desktop("RECT", self~hwnd) with x y cx cy
   /* diffsx and diffsy are needed for the title and the frame */
   diffsx = cx - x - (self~SizeX * self~FactorX)
   diffsy = cy - y - (self~SizeY * self~FactorY)
   self~SizeX = width
   self~SizeY = hight
   return WndShow_Pos("P", self~DlgHandle, 0, 0, width * self~FactorX + diffsx,,
                          hight* self~FactorY + diffsy,"NOMOVE "||showOptions )

::method FocusItem unguarded
   use arg id
   return SendWinMsg("ANY", self~DlgHandle, "0x0028", self~GetItem(id), 1)

::method TabToNext unguarded
   return Wnd_Desktop("SETFOC", self~DlgHandle, 0, "N")

::method TabToPrevious unguarded
   return Wnd_Desktop("SETFOC", self~DlgHandle, 1, "P")

::method getTextSize unguarded
  forward message "getTextSizeDlg" continue
  return result~width result~height

::method getTextSizeDlg unguarded external "LIBRARY oodialog pbdlg_getTextSizeDlg"


 -- 2 methods to set / remove the WS_TABSTOP / WS_GROUP styles for any control.
::method SetTabStop unguarded
   use strict arg id, wantStop = .true
   if \ id~datatype("W") then id = self~ResolveSymbolicId(id)
   if id == -1 then return -1
   if \ wantStop~datatype('O')then return -3
   return HandleControlEx(self~DlgHandle, id, "X", "TAB", wantStop)

::method SetGroup unguarded
   use strict arg id, wantGroup = .true
   if \ id~datatype("W") then id = self~ResolveSymbolicId(id)
   if id == -1 then return -1
   if \ wantGroup~datatype('O')then return -3
   return HandleControlEx(self~DlgHandle, id, "X", "GROUP", wantGroup)

::method Center unguarded
   use arg showOptions
   if Arg(1,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   parse value self~GetSize with dcx dcy
   s = screenSize()
   return WndShow_Pos("P",self~DlgHandle, (s[3] - dcx * self~FactorX) %2, (s[4] - dcy * self~FactorY) %2, 0, 0,,
                           "NOSIZE "||showOptions)


   /* This method will be called to get the default setting for the */
   /* automatic data field detection. If you disable the ADD, you'll */
   /* have to use the Connect... methods to assign a dialog item */
   /* to a data stem field */

::method InitAutoDetection protected /* autodetection is enabled by default */
   self~AutoDetection

::method NoAutoDetection    /* disable autodetection */
   self~AutoDetect = 0

::method AutoDetection      /* enable autodetection */
   self~AutoDetect = 1

 /* The following methods are abstract methods that will be called */
 /* whenever the push buttons 1, 2, or 9 will be pressed */
 /* Method 'OK' will call method 'Validate' */
 /* If self~finished = 1 the dialog is destroyed, if 0 the dialog */
 /* execution will be continued */
 /* OK returns 1 if the dialog is going to be destroyed */

::method OK unguarded
   self~finished = self~Validate
   self~InitCode = 1
   return self~finished

 /* If self~finished = 1 the dialog is destroyed, if 0 the dialog */
 /* execution will be continued */

::method Cancel unguarded
   self~finished = 1
   if self~finished = 1 then self~InitCode = 2
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

/** deinstall()
 * Clean up memory allocated for this dialog.
 */
::method deInstall
   if self~Adm \= 0 then ret = HandleDialogAdmin(self~Adm)
   self~Adm = 0

::method uninit
   -- Check if the dialog table still needs to be freed.
   if self~Adm \= 0 then ret = HandleDialogAdmin(self~Adm)
   self~Adm = 0
