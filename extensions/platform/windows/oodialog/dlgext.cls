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
/******************************************************************************/
/*                                                                            */
/*       Windows Dialog Interface for Object REXX                             */
/*                                                                            */
/* Dialog class extensions                                                    */
/*                                                                            */
/******************************************************************************/

::class 'DialogExtensions' public MIXINCLASS object

   /* This method will do the same as execute but it will do an asynchronous */
   /* dispatch of messages      */

::method ExecuteAsync unguarded
   use arg sleeptime, sh, icon = 0, nomodal
   if sleeptime~Datatype('N') = 0 then sleeptime = 10
   if arg(2,'o') = 1 then sh = "DEFAULT"
   if arg(4,'o') = 1 then nomodal = 0
   if self~startIt(icon, nomodal) \= 0 then do
      if self~useStem then self~SetDataStem(self~dlgData); else self~SetData
      self~show(sh)
      self~start("AsyncMessageHandling", sleeptime)
      do while self~automaticMethods~Items > 0
         p = self~automaticMethods~pull
         m = self~automaticMethods~pull
         c = self~automaticMethods~pull
         o = c~new(p, self)
         o~start(m)
      end
      return 0
   end
   else self~stopit
   return 1

   /* This method ends the asynchronous execution of a dialog */

::method EndAsyncExecution unguarded
   do while self~finished = 0
      call msSleep 20
   end
   if self~initCode = 1 then do
      if self~useStem then self~GetDataStem(self~dlgData)
      self~GetData
   end
   if self~adm \= 0 then self~stopit
   return self~initCode


   /* This method is started asynchronously to handle dialog messages */

::method AsyncMessageHandling unguarded protected
   use arg sleeptime
   do while self~finished = 0 & self~adm \= 0
      msg = getDlgMsg(self~adm)
      if msg~pos("1DLGDELETED1") > 0 then do
          if self~finished = 0 then self~finished = 2
      end
      else do
          if msg \= "" then interpret("self~"msg)
          else call msSleep sleeptime /* to yield cpu */
      end
   end

   /* Run a nonmodal dialog */
::method Popup
   use arg sh, sleeptime, icon = 0
   if arg(1,'o') = 1 then sh = "DEFAULT"
   if sleeptime~Datatype('N') = 0 then sleeptime = 10
   /* start execute and return to caller */
   self~start("execute", sh, icon, sleeptime, "NOTMODAL")


::method PopupAsChild
   use arg parent, sh, sleeptime, icon = 0

   if arg(1,'o') = 1 | parent = 0 then do
       call errorDialog "No parent dialog specified!"
       return -1
   end
   if arg(2,'o') = 1 then sh = "DEFAULT"
   if sleeptime~Datatype('N') = 0 then sleeptime = 10
   parent~childDialogs~insert(self)  /* add to the parents child list (last item) */
   self~parentDlg = parent

   /* start execute and return to caller */
   self~start("execute", sh, icon, sleeptime, "NOTMODAL")


::method SendMessageToItem unguarded
   parse arg id,a2,a3,a4
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return SendWinMsg("DLG",self~dlgHandle,id,a2,a3,a4)


 /* The following Methods are to set/get the behaviour of/from a scrollbar */
 /* You can use scrollbars to connect them with numerical entry fields */

::method SetSBRange unguarded
   use arg id, min, max, redraw
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if arg(4, "o") = 1 then redraw = 1
   return HandleScrollBar("SR", self~getItem(id),min,max, redraw)

::method GetSBRange unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return HandleScrollBar("GR",self~getItem(id))

::method SetSBPos unguarded
   use arg id, pos, redraw
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if arg(3, "o") = 1 then redraw = 1
   return HandleScrollBar("SP",self~getItem(id),pos, redraw)

::method GetSBPos unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return HandleScrollBar("GP",self~getItem(id))

::method connectScrollBar
   use arg id, progUp, progDn, progPos, min, max, pos, progPgUp, progPgDn, progTop, progBottom, progTrack, progEndSc

   if \ id~dataType("W") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1

   if arg(5, 'E'), min~datatype('N'), arg(6,'E'), max~datatype('N') then self~setSBRange(id, min, max, 0)
   if arg(7, 'E'), pos~datatype('N') then self~setSBPos(id, pos, 0)

   hwnd = self~getItem(id)
   wf = "0x0000FFFF"
   nf = "0xFFFFFFFF"
   /* Install hooks for WM_VSCROLL and WM_HSCROLL */
   self~addUserMsg(progDn, 0x00000114, wf, 0x00000001, wf, hwnd, nf)
   self~addUserMsg(progUp, 0x00000114, wf, 0x00000000, wf, hwnd, nf)
   self~addUserMsg(progDn, 0x00000115, wf, 0x00000001, wf, hwnd, nf)
   self~addUserMsg(progUp, 0x00000115, wf, 0x00000000, wf, hwnd, nf)
   if arg(4, 'E'), progPos \== "" then do
      self~addUserMsg(progPos, 0x00000114, wf, 0x00000004, wf, hwnd, nf)
      self~addUserMsg(progPos, 0x00000115, wf, 0x00000004, wf, hwnd, nf)
   end
   if arg(8, 'E'), progPgUp \== "" then do
      self~addUserMsg(progPgUp, 0x00000114, wf, 0x00000002, wf, hwnd, nf)
      self~addUserMsg(progPgUp, 0x00000115, wf, 0x00000002, wf, hwnd, nf)
   end
   if arg(9, 'E'), progPgDn \== "" then do
      self~addUserMsg(progPgDn, 0x00000114, wf, 0x00000003, wf, hwnd, nf)
      self~addUserMsg(progPgDn, 0x00000115, wf, 0x00000003, wf, hwnd, nf)
   end
   if arg(10, 'E'), progTop \== "" then do
      self~addUserMsg(progTop, 0x00000114, wf, 0x00000006, wf, hwnd, nf)
      self~addUserMsg(progTop, 0x00000115, wf, 0x00000006, wf, hwnd, nf)
   end
   if arg(11, 'E'), progBottom \== "" then do
      self~addUserMsg(progBottom, 0x00000114, wf, 0x00000007, wf, hwnd, nf)
      self~addUserMsg(progBottom, 0x00000115, wf, 0x00000007, wf, hwnd, nf)
   end
   if arg(12, 'E'), progTrack \== "" then do
      self~addUserMsg(progTrack, 0x00000114, wf, 0x00000005, wf, hwnd, nf)
      self~addUserMsg(progTrack, 0x00000115, wf, 0x00000005, wf, hwnd, nf)
   end
   if arg(13, 'E'), progEndSc \== "" then do
      self~addUserMsg(progEndSc, 0x00000114, wf, 0x00000008, wf, hwnd, nf)
      self~addUserMsg(progEndSc, 0x00000115, wf, 0x00000008, wf, hwnd, nf)
   end
   return 0


::method CombineELwithSB
   use arg elid, sbid, step, poscode

   pos = self~GetEntryLine(elid)
   parse value self~GetSBRange(sbid) with min max
   if pos~DataType("N") = 0 then return

   if arg(4,"o") = 0 then do
       sbc = .DlgUtil~loWord(poscode)
       if sbc = 0 & step > 0 then step = -step
       else if sbc = 1 & step < 0 then step = +step
   end

   if (step > 0) then do
      if pos < max then pos = pos + step
   end
   else if (step < 0) then do
      if pos > min then pos = pos + step
   end
   else if arg(4,'o') = 0 then do
      /* calc the position returned by message */
      pos = .DlgUtil~hiWord(poscode)
   end
   self~SetSBPos(sbid, pos)
   self~SetEntryLine(elid,pos)
   return 0


::method ConnectAllSBEvents
   use arg id, prog, min, max, pos
   return self~ConnectScrollBar(id, prog, prog, prog, min, max, pos, prog, prog, prog, prog)

::method DetermineSBPosition unguarded
   use arg id, posdata, single, page
   if arg(3,"o") = 1 then single = 1
   if arg(4,"o") = 1 then page = 10
   code = .DlgUtil~loWord(posdata)
   parse value self~GetSBRange(id) with rmin rmax
   pos = self~GetSBPos(id)
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
      when code = 5 then pos = .DlgUtil~hiWord(posdata)
      /* top */
      when code = 6 then pos = rmin
      /* bottom */
      when code = 7 then pos = rmax
      otherwise nop;
   end
   self~SetSBPos(id, pos)
   return pos


/* set/get the horizontal scroll width of a list box */

::method SetListWidth unguarded
   use arg id, dlgunits
   if arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(id, 0x00000194, dlgunits*self~factorX, 0)

::method GetListWidth unguarded
   use arg id
   return self~BoxMessage(id, 0x00000193, 0, 0)/self~factorX

::method SetListColumnWidth unguarded
   use arg id, dlgunits
   if arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(id, 0x00000195, dlgunits*self~factorX, 0)

::method SetListItemHeight unguarded
   use arg id, dlgunits
   if arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(id, 0x000001A0, 0, dlgunits*self~factorY)

::method GetListItemHeight unguarded
   use arg id
   return self~BoxMessage(id, 0x000001A1, 0, 0)/self~factorY

::method SetItemFont unguarded
   use arg id, hFont, bRedraw     /* WM_SETFONT */
   if arg(3, 'o') = 1 then bRedraw = 1
   return SendWinMsg("ANY", self~getItem(id), "0x0030", hFont, bRedraw)

::method HideWindowFast unguarded
   use arg hwnd
   return WndShow_Pos("S", hwnd, "HIDE FAST")

::method ShowWindowFast unguarded
   use arg hwnd
   return WndShow_Pos("S", hwnd, "NORMAL FAST", self~adm)

::method HideItemFast unguarded
   use arg id
   h = self~getItem(id)
   if h = 0 then return -1
   return WndShow_Pos("S", h, "HIDE FAST")

::method ShowItemFast unguarded
   use arg id
   h = self~getItem(id)
   if h = 0 then return -1
   return WndShow_Pos("S",h, "NORMAL FAST")

   /* This method returns the size and position of the given button */
::method GetButtonRect unguarded
   use arg id
   ret = self~getItem(id)
   return WindowRect("GET", ret)

   /* Returns the size and position of the given window */

::method GetWindowRect unguarded
   use arg hwnd
   return WindowRect("GET",hwnd)

   /* This method will resize a dialog item */

::method ResizeItem unguarded
   use arg id, width, high, showOptions = ""
   showOptions = showOptions~translate
   hwnd = self~getItem(id)
   if hwnd = 0 then return -1
   return WndShow_Pos("P",hwnd, 0, 0, width * self~factorX, high* self~factorY,,
                           "NOMOVE "||showOptions)

   /* This method will move a dialog item to an other position */

::method MoveItem unguarded
   use arg id, xPos, yPos, showOptions = ""
   showOptions = showOptions~translate
   hwnd = self~getItem(id)
   if h = 0 then return -1
   return WndShow_Pos("P", hwnd, xPos * self~factorX, yPos * self~factorY, 0, 0,,
                           "NOSIZE "||showOptions)

   /* This method will add a method that will be started automatically */
   /* and run concurrently when the dialog is executed. That is the   */
   /* method 'InClass'~'MethodName' will be started, when the dialog receives */
   /* the message 'execute' or execute async. You can use this method to */
   /* animate buttons */

::method AddAutoStartMethod
   use arg InClass, MethodName, Parameters
   if arg(1,"o") = 0 then self~automaticMethods~push(InClass); else self~automaticMethods~push(self)
   if arg(2,"o") = 0 then self~automaticMethods~push(MethodName)
   if arg(3,"o") = 0 then self~automaticMethods~push(Parameters)



/******************************************************************************/
/* Bitmap extensions */


::method BkgBitmap attribute protected  /* handle to bitmap that will be displayed in the background */
::method BkgBrushBmp attribute protected /* handle to brushbmp that will be displayed in the background */

   /* This method loads a bitmap from file into memory */
   /* It returns a handle to the bitmap */

::method LoadBitmap
   use arg bmpFilename, option = ""
   option = option~translate
   return LoadRemoveBitmap(self~adm, bmpFilename, "LOAD" || option)

   /* This method will free a memory bitmap */

::method RemoveBitmap
   use arg hBitmap
   return LoadRemoveBitmap(self~adm, hBitmap, "REMOVE")

-- This method adds a bitmap button and optionally connects a method with a push
-- button.  The given bitmap(s) will be shown instead of a push button.  The
-- optional msgToRise arg can be the name of a method to invoke when the button
-- is pushed.  The empty string signals the implementing code to skip adding a
-- method to the message table.
::method addBitmapButton
   use arg id, x, y, cx = 0, cy = 0, text = "", fnk = "", bmp, focus = "", sel = "", disabl = "", opts = ""
   if self~activePtr = 0 then return -2
   id = self~checkID(id)
   if  id = -1 then return
   opts = opts~translate || " OWNER"
   if focus = "" & sel = "" & opts~wordpos("FRAME") = 0 & opts~wordpos("TAB") = 0 then opts = opts || " NOTAB"

   self~dialogItemCount += 1
   self~activePtr = UsrAddControl("BUT", self~activePtr, id, x, y, cx, cy, text, opts)
   return self~connectBitmapButton(id, fnk~space(0), bmp, focus, sel, disabl, opts)


-- The only way to get the bitmaps into the bitmap table is through this method.
-- Therefore, we have to allow an omitted msgToRise.  If msgToRise == "", then
-- in BmpButton() the function skips adding a method to the message table.
::method connectBitmapButton
   use strict arg id, msgToRise = "", bmpNormal, bmpFocused = "", bmpSelected = "", bmpDisabled = "", styleOptions = ""
   styleOptions = styleOptions~translate
   if \ id~datatype("W") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return BmpButton(self~adm, "A", msgToRise, id, bmpNormal, bmpFocused, bmpSelected, bmpDisabled, styleOptions);


   /* This method changes the bitmap of a bitmap button */
   /* possible options are: (see ConnectBitmapButton) */

::method ChangeBitmapButton unguarded
   use arg id, bmpNormal, bmpFocused = "", bmpSelected = "", bmpDisabled = "", styleOptions = ""
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   styleOptions = styleOptions~translate
   return BmpButton(self~adm, "C", id, bmpNormal, bmpFocused, bmpSelected, bmpDisabled, styleOptions);

   /* Sets the position of a bitmap within a button */

::method DisplaceBitmap unguarded
   use arg id, x, y
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return BmpButton(self~adm, "S", id, x, y)

   /* Gets the position of a bitmap within a button */

::method GetBmpDisplacement unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return BmpButton(self~adm, "G", id)

   /* This method will set a bitmap as the background brush. */
   /* (seems to work only on NT) */

::method TiledBackgroundBitmap
   use arg bmpFilename
   if self~BkgBrushBmp \= 0 then self~DeleteObject(self~BkgBrushBmp)
   self~BkgBrushBmp = self~CreateBrush(,bmpFilename)
   ret = SetBackground(self~adm, "BRU", self~BkgBrushBmp)

::method BackgroundColor
   use arg color
   if self~BkgBrushBmp \= 0 then self~DeleteObject(self~BkgBrushBmp)
   self~BkgBrushBmp = self~CreateBrush(color)
   ret = SetBackground(self~adm, "BRU", self~BkgBrushBmp)

   /* This method sets a bitmap as the background brush. */
   /* (seems to work only on NT) */
   /* argument option can be: */
   /*    "USEPAL" : This will use the color palette of the bitmap */

::method BackgroundBitmap
   use arg bmpFilename, option = ""
   option = option~translate
   self~BkgBitmap = self~LoadBitmap(bmpFilename, option)
   ret = SetBackground(self~adm, "BMP", self~BkgBitmap)


::method CreateBrush unguarded
    use arg color = 1, brushSpecifier = ""
    if arg() = 0 then return HandleDC_Obj("B")
    if brushSpecifier == "" then return HandleDC_Obj("B", color)
    brushSpecifier = brushSpecifier~translate
    if brushSpecifier = "UPDIAGONAL" | brushSpecifier = "DOWNDIAGONAL" | brushSpecifier = "CROSS" | brushSpecifier = "DIAGCROSS",
    | brushSpecifier = "HORIZONTAL" | brushSpecifier = "VERTICAL" then return HandleDC_Obj("B", color, brushSpecifier)
    else return HandleDC_Obj("B", self~adm, color, brushSpecifier)  /* brushSpecifier must be filename or resource ID */


::method SetItemSysColor
   use arg id, colorbk, colorfg
   if \ id~DataType("W") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if arg(3,'o') = 1 then
       return SetBackground(self~adm, "COL", id, colorbk, "", "SYS")
   else
       return SetBackground(self~adm, "COL", id, colorbk, colorfg, "SYS")

::method SetItemColor
   use arg id, colorbk, colorfg
   if \ id~DataType("W") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if arg(3,'o') = 1 then
       return SetBackground(self~adm, "COL", id, colorbk)
   else
       return SetBackground(self~adm, "COL", id, colorbk, colorfg)


   /* This method scrolls 'text' in the button */

::method ScrollInButton unguarded
   use arg id, text, fontName, fontSize, fontStyle, displaceY, step, sleep, color
   hwnd = self~getItem(id)
   if arg() = 1 then
      return self~ScrollText(hwnd)
   else
      return self~ScrollText(hwnd, text, fontName, fontSize, fontStyle, displaceY, step, sleep, color)


   /* This method will scroll text in a window with the given size, font, */
   /* color. Text will be scrolled from right to left. */
   /* Call it twice and the scrolling will stop */
   /* See method WriteToWindow for valid fontStyles */

::method ScrollText unguarded
   use arg hwnd, text, fontName, fontSize, fontStyle,,
           displaceY, step, sleep, color

   if arg(3, "o") = 1 | fontName = "FONTNAME" then fontName = "System"
   if arg(4, "o") = 1 | fontSize = "FONTSIZE" then fontSize = 10
   if arg(5, "o") = 1 | fontStyle = "FONTSTYLE" then fontStyle = ""
   if arg(6, "o") = 1 | displaceY = "DISPLACEY" then displaceY = 0
   if arg(7, "o") = 1 | step = "STEP" then step = 4
   if arg(8, "o") = 1 | sleep = "SLEEP" then sleep = 10
   if arg(9, "o") = 1 | color = "COLOR" then color = 0

   if self~ScrollNow = 0 & arg() > 1 then
   do
      self~ScrollNow = 1
      ret = ScrollText(self~adm, hwnd, text, fontName, fontSize, fontStyle,,
                       displaceY, step, sleep, color)
   end; else do
      -- Send the interrupt scroll message, 0x0a02 == WM_USER_INTERRUPTSCROLL
      ret = SendWinMsg("ANY",self~dlgHandle, "0x0a02", hwnd, 0)
   end
   self~ScrollNow = 0
   return ret




/******************************************************************************/
/* Graphics and drawing extensions */

   /* Connect WM_DRAWITEM with a method */

::method connectDraw
  use arg id, msgToRise
  if arg(2,'O') then msgToRise = "OnDraw"
  if arg(1,'O') then do
      return self~addUserMsg(msgToRise, 0x0000002B, 0xFFFFFFFF, 0, 0, 0, 0)
  end
  else do
     if \ id~dataType("W") then id = self~resolveSymbolicId(id)
     if id = -1 then return -1
     return self~addUserMsg(msgToRise, 0x0000002B, 0xFFFFFFFF, '0x0000'||id~d2x(4), 0xFFFFFFFF, 0, 0)
  end

   /* This method draws the given button */

::method DrawButton unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return WindowRect("BTN", self~adm, id)

   /* This method erases the draw area of the given button */

::method ClearButtonRect unguarded
   use arg id
   hwnd = self~getItem(id)
   return WindowRect("CLR", self~adm, hwnd)

   /* This method erases the draw area of the given window */

::method ClearWindowRect unguarded
   use arg hwnd
   return WindowRect("CLR", self~adm, hwnd)

   /* Clears the whole dialog  */
::method Clear unguarded
   parse value WindowRect("GET", self~Hwnd) with r.1 r.2 r.3 r.4
   return WindowRect("CLR", self~adm, self~Hwnd,r.1,r.2,r.3,r.4)

   /* Clears the given rectangle left, top, right, bottom of the given window */

::method ClearRect unguarded
   use arg hwnd, left, top, right, bottom
   return WindowRect("CLR", self~adm, hwnd,left,top,right,bottom)

   /* This method redraws the given button */

::method RedrawButton unguarded
   use arg id, erasebkg
   hwnd = self~getItem(id)
   if arg(2, 'o') = 1 then erasebkg = 0
   return WindowRect("RDW", hwnd, erasebkg)

   /* This method redraws the rectangle of the given window */

::method RedrawWindowRect unguarded
   use arg hwnd, erasebkg
   if arg(1, 'o') = 1 then hwnd = self~hwnd
   if arg(2, 'o') = 1 then erasebkg = 0
   return WindowRect("RDW", hwnd, erasebkg)


   /* This method allows you to write text to a window in the given font, in */
   /* the given size, and to the given position */
   /* argument 'fontStyle' can be one or more off: */
   /*    "THIN"            "UNDERLINE"    */
   /*    "EXTRALIGHT"      "ITALIC"       */
   /*    "LIGHT"           "STRIKEOUT"    */
   /*    "MEDIUM"                */
   /*    "SEMIBOLD"              */
   /*    "EXTRABOLD"             */
   /*    "BOLD"                  */
   /*    "HEAVY"                 */

::method WriteToWindow unguarded
   use arg hwnd, xPos, yPos, text, fontName = "System", fontSize = 10, fontStyle = "", fgColor = (-1), bkColor = (-1)
   fontStyle = fontStyle~translate
   return WriteText(hwnd, xPos, yPos, text, fontName, fontSize, fontStyle, fgColor, bkColor)  /* more than 6 arguments means use font */


   /* This method allows you to write text to a button in the given font in */
   /* the given size to the given position. */
   /* See method WriteToWindow for valid fontStyles */

::method WriteToButton unguarded
   argarr = arg(1, "A")
   argarr[1] = self~getItem(argarr[1])  /* transform first argument into window handle */
   forward message (WriteToWindow) arguments (argarr)


   /* This method moves the rectangle within a button */

::method ScrollButton unguarded
   use arg id, xPos, yPos, left, top, right, bottom
   return ScrollTheWindow(self~adm, self~getItem(id), xPos, yPos, left, top, right, bottom, "yes")


   /* This method will return the device context of a window */

::method GetWindowDC unguarded
   use arg hwnd
   return HandleDC_Obj("G", hwnd)

   /* This method will return the device context of a button */

::method GetButtonDC unguarded
   use arg id
   return self~GetWindowDC(self~getItem(id))

   /* This method will free the device context of a window */

::method FreeWindowDC unguarded
   use arg hwnd, dc
   return HandleDC_Obj("F", hwnd, dc)

   /* This method will free the device context of a button */

::method FreeButtonDC unguarded
   use arg id, dc
   hwnd = self~getItem(id)
   return self~FreeWindowDC(hwnd, dc)

   /* This method will return the horizontal bitmap extension */

::method GetBitmapSizeX unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   parse value BmpButton(self~adm, "E", id) with x .
   return x

   /* This method will return the vertical bitmap extension */

::method GetBitmapSizeY unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   parse value BmpButton(self~adm, "E", id) with . y
   return y

   /* This method will draw the bitmap of a button */
   /* Use this method to move a bitmap or a part of it... */

::method DrawBitmap unguarded
   use arg hwnd, id, px, py, srcx, srcy, xlen, ylen
   if arg(3, "o") = 1 then px = 0
   if arg(4, "o") = 1 then py = 0
   if arg(5, "o") = 1 then srcx = 0
   if arg(6, "o") = 1 then srcy = 0
   if arg(7, "o") = 1 then xlen = 0
   if arg(8, "o") = 1 then ylen = 0
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if arg(1, "o") = 1 then hwnd = self~getItem(id)
   return BmpButton(self~adm, "D", hwnd, id, px, py, srcx, srcy, xlen, ylen)


   /* This method will draw the bitmap step by step */

::method DimBitmap unguarded
   use arg id, bmp, cx, cy, stepx, stepy, steps

   if arg(5, "o") = 1 then steps = 10
   if arg(6, "o") = 1 then stepx = 2
   if arg(7, "o") = 1 then stepy = 2

   dc = self~GetButtonDC(id)
   br = HandleDC_Obj("B",self~adm, 0, bmp)
   pen = HandleDC_Obj("P",0, "NULL", 0)
   obr = self~ObjectToDC(dc, br)
   open = self~ObjectToDC(dc, pen)

   ret = DCDraw("DIM", dc, cx, cy, stepx, stepy, steps)

   self~ObjectToDC(dc, obr)
   self~ObjectToDC(dc, open)
   self~DeleteObject(pen)
   self~FreeButtonDC(id, dc)

   /* This method will scroll a bitmap from one position to an other */
   /* within a button */

::method ScrollBitmapFromTo unguarded
   use arg id, fromX, fromY, toX, toY, stepX, stepY, delay, displace

   if arg(8, "o") = 1 then delay = 0
   if arg(9, "o") = 1 then displace = 1
   xl = self~GetBitmapSizeX(id)
   yl = self~GetBitmapSizeY(id)

   if xl <= 0 | yl <= 0 then return 1
   if arg(6, "o") = 1 then stepx = 0
   if arg(7, "o") = 1 then stepy = 0

   if stepx = 0 & stepy = 0 then return 1

   curx=fromx
   cury=fromy
   if (stepx > 0) then cursx = -curx; else cursx = 0
   if (stepy > 0) then cursy = -cury; else cursy = 0
   if (cursx > xl) then cursx = xl
   if (cursy > yl) then cursy = yl

   hwnd = self~getItem(id)
   if hwnd = 0 then return -1

   parse value self~GetWindowRect(hwnd) with r.left r.top r.right r.bottom
   r.right = r.right - r.left
   r.bottom = r.bottom - r.top
   drawx = 1
   drawy = 1

   if (stepx >= 0) then
   do
      left = max(fromx, 0)
      lenx = xl - cursx
   end
   else do
      left = fromx
      lenx = min(r.right-fromx, xl)
   end
   if (stepy >= 0) then
   do
      top = max(fromy, 0)
      leny = yl - cursy
   end
   else do
      top = fromy
      leny = min(r.bottom-fromy, yl)
   end

   self~DrawBitmap(hwnd, id, left, top, cursx, cursy, lenx, leny)
   if (stepx < 0) then cursx = min(lenx, xl); else cursx = cursx-stepx;
   if (stepy < 0) then cursy = min(leny, yl); else cursy = cursy-stepy;

   worktodo = 1
   if (delay > 0) then timer = winTimer("START", delay); else timer = 0
   do while worktodo = 1

      sleft = curx
      srgt = curx+xl
      if srgt > r.right then srgt = r.right
      if sleft < 0 then sleft = 0

      s_top = cury
      sbot = cury+yl
      if sbot > r.bottom-stepy then sbot = r.bottom-stepy
      if s_top < 0 then s_top = 0

      ret = ScrollTheWindow(self~adm, hwnd, stepx, stepy, sleft, s_top, srgt, sbot, "yes")
      if ret = 1 then do
         if (timer \== 0) then ret = winTimer("STOP", timer)
         return 1
      end

      if (stepx < 0) then left = r.right + stepx; else left = curx
      rgt = curx+xl

      if rgt > r.right then rgt = r.right
      if left < 0 then left = 0

      if (stepy < 0) then top = r.bottom+stepy; else top = cury
      bot = cury+yl
      if bot > r.bottom then bot = r.bot
      if top < 0 then top = 0

      if stepx \= 0 & drawx=1 then self~DrawBitmap(hwnd, id, left, top, cursx, cursy, abs(stepx), yl)
      if stepy \= 0 & drawy=1 then self~DrawBitmap(hwnd, id, left, top, cursx, cursy, xl, abs(stepy))
      if (timer \== 0) then ret = winTimer("WAIT", timer)

      drawx = 0
      if (stepx <= 0) then do
         if cursx < xl then do
            cursx = cursx - stepx
            if cursx < xl then drawx = 1
         end
      end
      else do
         if cursx > 0 then do
            cursx = cursx - stepx
            if cursx > 0 then drawx = 1
         end
      end
      curx = curx + stepx

      drawy = 0
      if (stepy <= 0) then do
         if cursy < yl then do
            cursy = cursy - stepy
            if cursy < yl then drawy = 1
         end
      end
      else do
         if cursy > 0 then do
            cursy = cursy - stepy
            if cursy > 0 then drawy = 1
         end
      end
      cury = cury + stepy

                 /* so repaint will draw the cursor at the current position */
      if displace=1 then self~DisplaceBitmap(id, curx, cury);

      worktodo = 0
      if (stepx>0) & (curx+stepx <= tox) then worktodo = 1
      if (stepy>0) & (cury+stepy <= toy) then worktodo = 1
      if (stepx<0) & (curx+stepx >= tox) then worktodo = 1
      if (stepy<0) & (cury+stepy >= toy) then worktodo = 1
   end
   if (timer \== 0) then ret = winTimer("STOP", timer)
   self~DisplaceBitmap(id, tox, toy);
   self~RedrawButton(id,0)
   return 0

   /* This method will install an animated button and run it concurrently to main */
   /* activity */

-- The only way to get the bitmap(s) into the bitmap table is through this
-- method.  Therefore msgToRise has to be optional.  The empty string will
-- eventually cause the bitmaps to be added to the bitmap table and the addition
-- of a method being added to the message table will be skipped.
::method connectAnimatedButton
   use arg buttonEd, msgToRise = "", autoClass = (.AnimatedButton), bmpFrom, bmpTo = 0, moveX, moveY, sizeX, sizeY, -
           delay, xNow = 0, yNow = 0

   -- Note documentation for bmpfrom, it is *not* a bitmap handle.
   if arg(5, "o") | \ bmpFrom~datatype("N") then
       ret = self~connectBitmapButton(buttonId, msgToRise, .local[bmpFrom][1])
   else
       ret = self~connectBitmapButton(buttonId, msgToRise, bmpFrom)

   if ret <> 0 then return ret

   if arg(8, "O") then sizeX = self~getBitmapSizeX(buttonId)
   if arg(9, "O") then sizeY = self~getBitmapSizeY(buttonId)

   self~addAutoStartMethod(autoClass, "RUN", buttonId || "," || bmpFrom ||"," || bmpTo ||"," || moveX || ",",
     || moveY || "," || sizeX || "," || sizeY || "," || delay || "," || xNow || "," || yNow)



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  These menu bar methods are needed to provide backward compatibility for the
  deprecated menu methods.  The Rexx programmer is *strongly* discouraged from
  using them.  If the deprecated methods are removed, these will be removed with
  them.

  In addition, it is highly likely that the results will be unpredictable.

  To obtain the menuBar attached to the dialog, use the getMenuBar() method.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

::attribute menuBar private
::attribute isLinked private

::method linkMenu
  use strict arg menu

  signal on syntax

  if \ menu~isA(.MenuBar) then raise syntax 93.948 array(1, "MenuBar")

  oldMenu = self~menuBar
  self~menuBar = menu
  self~isLinked = .true
  return oldMenu

syntax:
  c = condition('O')
  if c~traceBack~isA(.List) then c~traceBack~empty
  raise propagate

::method unlinkMenu
  menu = self~menuBar
  self~menuBar = .nil
  self~isLinked = .false
  return menu

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  The following menu bar methods are deprecated, instead, use the Menu classes
  directly.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

::method enableMenuItem unguarded
    use strict arg id

    signal on syntax

    if self~menuBar == .nil then raise syntax 93.900 array("there is no MenuBar attached to this dialog")
    if self~menuBar~isA(.UserMenuBar), \self~menuBar~isComplete then raise syntax 93.900 array("the UserMenuBar is not complete")
    return self~menuBar~enable(id)

syntax:
  c = condition('O')
  if c~traceBack~isA(.List) then c~traceBack~empty
  raise propagate

::method disableMenuItem unguarded
    use strict arg id

    signal on syntax

    if self~menuBar == .nil then raise syntax 93.900 array("There is no MenuBar attached to this dialog")
    if self~menuBar~isA(.UserMenuBar), \self~menuBar~isComplete then raise syntax 93.900 array("the UserMenuBar is not complete")
    return self~menuBar~disable(id)

syntax:
  c = condition('O')
  if c~traceBack~isA(.List) then c~traceBack~empty
  raise propagate

::method checkMenuItem unguarded
    use strict arg id

    signal on syntax

    if self~menuBar == .nil then raise syntax 93.900 array("There is no MenuBar attached to this dialog")
    if self~menuBar~isA(.UserMenuBar), \self~menuBar~isComplete then raise syntax 93.900 array("the UserMenuBar is not complete")
    return self~menuBar~check(id)

syntax:
  c = condition('O')
  if c~traceBack~isA(.List) then c~traceBack~empty
  raise propagate

::method uncheckMenuItem unguarded
    use strict arg id

    signal on syntax

    if self~menuBar == .nil then raise syntax 93.900 array("There is no MenuBar attached to this dialog")
    if self~menuBar~isA(.UserMenuBar), \self~menuBar~isComplete then raise syntax 93.900 array("the UserMenuBar is not complete")
    return self~menuBar~uncheck(id)

syntax:
  c = condition('O')
  if c~traceBack~isA(.List) then c~traceBack~empty
  raise propagate

::method grayMenuItem unguarded
    use strict arg id

    signal on syntax

    if self~menuBar == .nil then raise syntax 93.900 array("There is no MenuBar attached to this dialog")
    if self~menuBar~isA(.UserMenuBar), \self~menuBar~isComplete then raise syntax 93.900 array("the UserMenuBar is not complete")
    return self~menuBar~gray(id)

syntax:
  c = condition('O')
  if c~traceBack~isA(.List) then c~traceBack~empty
  raise propagate

::method setMenuItemRadio unguarded
    use strict arg idstart, idend, idset

    signal on syntax

    if self~menuBar == .nil then raise syntax 93.900 array("There is no MenuBar attached to this dialog")
    if self~menuBar~isA(.UserMenuBar), \self~menuBar~isComplete then raise syntax 93.900 array("the UserMenuBar is not complete")
    return self~menuBar~checkRadioItem(idstart, idend, idset)

syntax:
  c = condition('O')
  if c~traceBack~isA(.List) then c~traceBack~empty
  raise propagate

::method getMenuItemState unguarded
    use strict arg id

    signal on syntax

    if self~menuBar == .nil then raise syntax 93.900 array("There is no MenuBar attached to this dialog")
    if self~menuBar~isA(.UserMenuBar), \self~menuBar~isComplete then raise syntax 93.900 array("the UserMenuBar is not complete")
    return self~menuBar~getItemState(id)

syntax:
  c = condition('O')
  if c~traceBack~isA(.List) then c~traceBack~empty
  raise propagate

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  End deprecated menu bar methods.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/******************************************************************************/
/* New extensions */

::method connectResize
   use strict arg msgToRise
   self~addUserMsg(msgToRise, 0x00000005, 0xFFFFFFFF, 0, 0, 0, 0)

::method connectMove
   use strict arg msgToRise
   self~addUserMsg(msgToRise, 0x00000003, 0xFFFFFFFF, 0, 0, 0, 0)

::method connectPosChanged
   use strict arg msgToRise
   self~addUserMsg(msgToRise, 0x00000047, 0xFFFFFFFF, 0, 0, 0, 0)

::method connectMouseCapture
   use strict arg msgToRise
   self~addUserMsg(msgToRise, 0x00000215, 0xFFFFFFFF, 0, 0, 0, 0)

::method connectMenuItem   -- Deprecated
   use strict arg id, msgToRaise
   return .Menu~connectItem(id, msgToRaise, self)

::method PeekDialogMessage unguarded
   return getDlgMsg(self~adm, .true)

::method SetWindowRect unguarded
   use arg hwnd, x, y, width, high, showOptions = ""
   showOptions = showOptions~translate
   return WndShow_Pos("P",hwnd, x, y, width, high, showOptions)

   /* Redraws the given rectangle */
::method RedrawRect unguarded
   use arg hwnd, left, top, right, bottom, erasebkg
   if arg(1,'o') = 1 then hwnd = self~hwnd
   if arg(6, 'o') = 1 then erasebkg = 0
   return WindowRect("RDW", hwnd,left,top,right,bottom, erasebkg)

::method RedrawWindow unguarded
   use arg hwnd
   return WndShow_Pos("S", hwnd, "REDRAW")

::method GetFocus unguarded
   return Wnd_Desktop("GETFOC", self~dlgHandle)

::method SetFocus unguarded
   use arg hwnd
   return Wnd_Desktop("SETFOC", self~dlgHandle, hwnd)

::method SetFocusToWindow unguarded
   use arg hwnd
   return Wnd_Desktop("SETFOC", self~dlgHandle, hwnd, 'F')

::method GetMouseCapture
   return Wnd_Desktop("CAP", self~dlgHandle, "G")

::method CaptureMouse
   return Wnd_Desktop("CAP", self~dlgHandle, self~dlgHandle)

::method ReleaseMouseCapture
   return Wnd_Desktop("CAP", self~dlgHandle, "R")

::method IsMouseButtonDown
   use arg mb = "LEFT"
   mb = mb~translate~space(0)
   if mb = "LEFT" then return Wnd_Desktop("KSTAT", self~dlgHandle,1) /* VK_LBUTTON */
   if mb = "MIDDLE" then return Wnd_Desktop("KSTAT", self~dlgHandle,4) /* VK_MBUTTON */
   if mb = "RIGHT" then return Wnd_Desktop("KSTAT", self~dlgHandle,2) /* VK_RBUTTON */


::method Dump
    use arg adm
    if arg() == 1 then do
        say "Dump of current dialog administration"
        say "====================================="
        call DumpAdmin "ADMDUMP", adm
        say  "Slot: " admdump.Slot
        say  "hThread: " admdump.hThread
        say  "hDialog: " admdump.hDialog
        say  "hMenu: " admdump.hMenu
        say  "BkgBrush: " admdump.BkgBrush
        say  "BkgBitmap: " admdump.BkgBitmap
        say  "TopMost: " admdump.TopMost
        say  "CurrentChild: " admdump.CurrentChild
        say  "DLL: " admdump.Dll
        say  "Queue: " admdump.queue
        say  "BmpButtons: " admdump.BmpButtons
        do i = 1 to admdump.BmpButtons
            say "    " admdump.BmpTab.i.id "->" admdump.BmpTab.i.normal "," admdump.BmpTab.i.focused "," admdump.BmpTab.i.selected "," admdump.BmpTab.i.disabled
        end
        say  "Messages: " admdump.Messages
        do i = 1 to admdump.Messages
            say "    " admdump.MsgTab.i.msg "," admdump.MsgTab.i.param1 "," admdump.MsgTab.i.param2 "==>" admdump.MsgTab.i.method
        end
        say  "DataItems: " admdump.DataItems
        do i = 1 to admdump.DataItems
            say "    " admdump.DataTab.i.id "("admdump.DataTab.i.type","admdump.DataTab.i.category")"
        end
        say  "ColorItems: " admdump.ColorItems
        do i = 1 to admdump.ColorItems
            say "    " admdump.ColorTab.i.id "(" admdump.ColorTab.i.Foreground "," admdump.ColorTab.i.Background ")"
        end
    end
    else do
        say "Dump of administration state"
        say "============================"
        call DumpAdmin "ADMDUMP"
        d = Digits()
        NUMERIC DIGITS 15
        do i = 1 to admdump.0
            say  "AdmBlock: " admdump.i.AdmBlock
            say  "Slot: " admdump.i.Slot
            say  "hThread: " admdump.i.hThread
            say  "hDialog: " admdump.i.hDialog
            if admdump.i.hDialog \=0 then say "Window title:" self~WindowTitle(x2d(admdump.i.hDialog))
            say  "hMenu: " admdump.i.hMenu
            say  "BkgBrush: " admdump.i.BkgBrush
            say  "BkgBitmap: " admdump.i.BkgBitmap
            say  "TopMost: " admdump.i.TopMost
            say  "CurrentChild: " admdump.i.CurrentChild
            say  "DLL: " admdump.i.Dll
            say  "Queue: " admdump.i.queue
            say  "BmpButtons: " admdump.i.BmpButtons
            say  "Messages: " admdump.i.Messages
            say  "DataItems: " admdump.i.DataItems
            say  "ColorItems: " admdump.i.ColorItems
            say "--------------------------"
        end
        NUMERIC DIGITS d
    end

