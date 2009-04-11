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

   /* This method will do the same as Execute but it will do an asynchronous */
   /* dispatch of messages      */

::method ExecuteAsync unguarded
   use arg sleeptime, sh, icon = 0, nomodal
   if sleeptime~Datatype('N') = 0 then sleeptime = 10
   if Arg(2,'o') = 1 then sh = "DEFAULT"
   if Arg(4,'o') = 1 then nomodal = 0
   if self~startit(icon, nomodal) \= 0 then
   do
      if self~UseStem = 1 then self~SetDataStem(self~DlgData); else self~SetData
      self~show(sh)
      self~start("AsyncMessageHandling", sleeptime)
      do while self~AutomaticMethods~Items > 0
         p = self~AutomaticMethods~pull
         m = self~AutomaticMethods~pull
         c = self~AutomaticMethods~pull
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
      call sleepms(20)
   end
   if self~InitCode = 1 then do
      if (self~UseStem = 1) then self~GetDataStem(self~DlgData)
      self~GetData
   end
   if self~Adm \= 0 then self~stopit
   return self~InitCode


   /* This method is started asynchronously to handle dialog messages */

::method AsyncMessageHandling unguarded protected
   use arg sleeptime
   do while self~finished = 0 & self~Adm \= 0
      msg = GetDlgMsg(self~Adm)
      if msg~pos("1DLGDELETED1") > 0 then do
          if self~finished = 0 then self~finished = 2
      end
      else do
          if msg \= "" then interpret("self~"msg)
          else call sleepms(sleeptime) /* to yield cpu */
      end
   end

   /* Run a nonmodal dialog */
::method Popup
   use arg sh, sleeptime, icon = 0
   if Arg(1,'o') = 1 then sh = "DEFAULT"
   if sleeptime~Datatype('N') = 0 then sleeptime = 10
   /* start Execute and return to caller */
   self~start("Execute", sh, icon, sleeptime, "NOTMODAL")


::method PopupAsChild
   use arg parent, sh, sleeptime, icon = 0

   if Arg(1,'o') = 1 | parent = 0 then do
       call ErrorMessage "No parent dialog specified!"
       return -1
   end
   if Arg(2,'o') = 1 then sh = "DEFAULT"
   if sleeptime~Datatype('N') = 0 then sleeptime = 10
   parent~ChildDialogs~insert(self)  /* add to the parents child list (last item) */
   self~ParentDlg = parent

   /* start Execute and return to caller */
   self~start("Execute", sh, icon, sleeptime, "NOTMODAL")


::method SendMessageToItem unguarded
   parse arg id,a2,a3,a4
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return SendWinMsg("DLG",self~DlgHandle,id,a2,a3,a4)


 /* The following Methods are to set/get the behaviour of/from a scrollbar */
 /* You can use scrollbars to connect them with numerical entry fields */

::method SetSBRange unguarded
   use arg id, min, max, redraw
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(4, "o") = 1 then redraw = 1
   return HandleScrollBar("SR", self~GetItem(id),min,max, redraw)

::method GetSBRange unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return HandleScrollBar("GR",self~GetItem(id))

::method SetSBPos unguarded
   use arg id, pos, redraw
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3, "o") = 1 then redraw = 1
   return HandleScrollBar("SP",self~GetItem(id),pos, redraw)

::method GetSBPos unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return HandleScrollBar("GP",self~GetItem(id))

::method ConnectScrollBar
   use arg id, progup, progdn, progpos, min, max, pos, progpgup, progpgdn,,
           progtop, progbottom, progtrack, progendsc
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if ((Arg(5, 'o') = 0) & (min~Datatype('N') \= 0)) & ((Arg(6,'o') = 0) & (max~Datatype('N') \= 0)) then self~SetSBRange(id, min, max, 0)
   if (Arg(7,'o') = 0) & (pos~Datatype('N') \= 0) then self~SetSBPos(id, pos, 0)

   hnd = self~GetItem(id)
   wf = "0x0000FFFF"
   nf = "0xFFFFFFFF"
   /* Install hooks for WM_VSCROLL and WM_HSCROLL */
   self~AddUserMsg(progdn, 0x00000114, wf, 0x00000001, wf, hnd, nf)
   self~AddUserMsg(progup, 0x00000114, wf, 0x00000000, wf, hnd, nf)
   self~AddUserMsg(progdn, 0x00000115, wf, 0x00000001, wf, hnd, nf)
   self~AddUserMsg(progup, 0x00000115, wf, 0x00000000, wf, hnd, nf)
   if Arg(4,'o') = 0 then do
      self~AddUserMsg(progpos, 0x00000114, wf, 0x00000004, wf, hnd, nf)
      self~AddUserMsg(progpos, 0x00000115, wf, 0x00000004, wf, hnd, nf)
   end
   if Arg(8,'o') = 0 then do
      self~AddUserMsg(progpgup, 0x00000114, wf, 0x00000002, wf, hnd, nf)
      self~AddUserMsg(progpgup, 0x00000115, wf, 0x00000002, wf, hnd, nf)
   end
   if Arg(9,'o') = 0 then do
      self~AddUserMsg(progpgdn, 0x00000114, wf, 0x00000003, wf, hnd, nf)
      self~AddUserMsg(progpgdn, 0x00000115, wf, 0x00000003, wf, hnd, nf)
   end
   if Arg(10,'o') = 0 then do
      self~AddUserMsg(progtop, 0x00000114, wf, 0x00000006, wf, hnd, nf)
      self~AddUserMsg(progtop, 0x00000115, wf, 0x00000006, wf, hnd, nf)
   end
   if Arg(11,'o') = 0 then do
      self~AddUserMsg(progbottom, 0x00000114, wf, 0x00000007, wf, hnd, nf)
      self~AddUserMsg(progbottom, 0x00000115, wf, 0x00000007, wf, hnd, nf)
   end
   if Arg(12,'o') = 0 then do
      self~AddUserMsg(progtrack, 0x00000114, wf, 0x00000005, wf, hnd, nf)
      self~AddUserMsg(progtrack, 0x00000115, wf, 0x00000005, wf, hnd, nf)
   end
   if Arg(13,'o') = 0 then do
      self~AddUserMsg(progendsc, 0x00000114, wf, 0x00000008, wf, hnd, nf)
      self~AddUserMsg(progendsc, 0x00000115, wf, 0x00000008, wf, hnd, nf)
   end
   return 0


::method CombineELwithSB
   use arg elid, sbid, step, poscode

   pos = self~GetEntryLine(elid)
   parse value self~GetSBRange(sbid) with min max
   if pos~DataType("N") = 0 then return

   if Arg(4,"o") = 0 then do
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
   else if Arg(4,'o') = 0 then do
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
   if Arg(3,"o") = 1 then single = 1
   if Arg(4,"o") = 1 then page = 10
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
   if Arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(id, 0x00000194, dlgunits*self~FactorX, 0)

::method GetListWidth unguarded
   use arg id
   return self~BoxMessage(id, 0x00000193, 0, 0)/self~FactorX

::method SetListColumnWidth unguarded
   use arg id, dlgunits
   if Arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(id, 0x00000195, dlgunits*self~FactorX, 0)

::method SetListItemHeight unguarded
   use arg id, dlgunits
   if Arg(2,'o') = 1 | dlgunits~datatype("N") = 0 then return -1
   return self~BoxMessage(id, 0x000001A0, 0, dlgunits*self~FactorY)

::method GetListItemHeight unguarded
   use arg id
   return self~BoxMessage(id, 0x000001A1, 0, 0)/self~FactorY

::method SetItemFont unguarded
   use arg id, hFont, bRedraw     /* WM_SETFONT */
   if Arg(3, 'o') = 1 then bRedraw = 1
   return SendWinMsg("ANY", self~GetItem(id), "0x0030", hFont, bRedraw)

::method HideWindowFast unguarded
   use arg hwnd
   return WndShow_Pos("S", hwnd, "HIDE FAST")

::method ShowWindowFast unguarded
   use arg hwnd
   return WndShow_Pos("S", hwnd, "NORMAL FAST", self~Adm)

::method HideItemFast unguarded
   use arg id
   h = self~GetItem(id)
   if h = 0 then return -1
   return WndShow_Pos("S", h, "HIDE FAST")

::method ShowItemFast unguarded
   use arg id
   h = self~GetItem(id)
   if h = 0 then return -1
   return WndShow_Pos("S",h, "NORMAL FAST")

   /* This method returns the size and position of the given button */
::method GetButtonRect unguarded
   use arg id
   ret = self~GetItem(id)
   return WindowRect("GET", ret)

   /* Returns the size and position of the given window */

::method GetWindowRect unguarded
   use arg hwnd
   return WindowRect("GET",hwnd)

   /* This method will resize a dialog item */

::method ResizeItem unguarded
   use arg id, width, high, showOptions
   if Arg(4,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   hnd = self~GetItem(id)
   if hnd = 0 then return -1
   return WndShow_Pos("P",hnd, 0, 0, width * self~FactorX, high* self~FactorY,,
                           "NOMOVE "||showOptions)

   /* This method will move a dialog item to an other position */

::method MoveItem unguarded
   use arg id, xPos, yPos, showOptions
   if Arg(4,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   h = self~GetItem(id)
   if h = 0 then return -1
   return WndShow_Pos("P",h, xPos * self~FactorX, yPos * self~FactorY, 0, 0,,
                           "NOSIZE "||showOptions)

   /* This method will add a method that will be started automatically */
   /* and run concurrently when the dialog is executed. That is the   */
   /* method 'InClass'~'MethodName' will be started, when the dialog receives */
   /* the message 'execute' or execute async. You can use this method to */
   /* animate buttons */

::method AddAutoStartMethod
   use arg InClass, MethodName, Parameters
   if Arg(1,"o") = 0 then self~AutomaticMethods~push(InClass); else self~AutomaticMethods~push(self)
   if Arg(2,"o") = 0 then self~AutomaticMethods~push(MethodName)
   if Arg(3,"o") = 0 then self~AutomaticMethods~push(Parameters)



/******************************************************************************/
/* Bitmap extensions */


::method BkgBitmap attribute protected  /* handle to bitmap that will be displayed in the background */
::method BkgBrushBmp attribute protected /* handle to brushbmp that will be displayed in the background */

   /* This method loads a bitmap from file into memory */
   /* It returns a handle to the bitmap */

::method LoadBitmap
   use arg bmpFilename, option
   if Arg(2, 'o') = 1 then option = ""; else option = option~translate
   return LoadRemoveBitmap(self~Adm, bmpFilename, "LOAD" || option)

   /* This method will free a memory bitmap */

::method RemoveBitmap
   use arg hBitmap
   return LoadRemoveBitmap(self~Adm, hBitmap, "REMOVE")

   /* This method connects a bitmap and a class method with a push button */
   /* The given bitmap will be shown instead of a push button */

::method AddBitmapButton
   use arg id, x, y, cx, cy, text, fnk, bmp, focus, sel, disabl, opts
   if self~activePtr = 0 then return -2
   id = self~CheckId(id)
   if  id = -1 then return
   if Arg(4, 'o') = 1 then cx = 0
   if Arg(5, 'o') = 1 then cy = 0
   if Arg(6, 'o') = 1 then text = ""
   if Arg(7, 'o') = 1 then fnk = ""
   if Arg(9, 'o') = 1 then focus = ""
   if Arg(10, 'o') = 1 then sel = ""
   if Arg(11, 'o') = 1 then disabl = ""
   opts = self~GetDefaultOpts(opts) || " OWNER"
   if focus = "" & sel = "" & opts~wordpos("FRAME") = 0 & opts~wordpos("TAB") = 0 then opts = opts || " NOTAB"

   self~DialogItemCount = self~DialogItemCount + 1
   self~activePtr = UsrAddControl("BUT", self~activePtr, id, x, y, cx, cy, text, opts)
   self~ConnectBitmapButton(id, fnk~space(0), bmp, focus, sel, disabl, opts)


::method ConnectBitmapButton
   use arg id, msgToRise, bmpNormal, bmpFocused, bmpSelected, bmpDisabled, styleOptions
   if Arg(2,'o') = 1 then msgToRise = ""
   if Arg(4,'o') = 1 then bmpFocused = ""
   if Arg(5,'o') = 1 then bmpSelected = ""
   if Arg(6,'o') = 1 then bmpDisabled = ""
   if Arg(7,'o') = 1 then styleOptions = ""; else styleOptions = styleOptions~translate
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return BmpButton(self~Adm, "A", msgToRise, id, bmpNormal, bmpFocused, bmpSelected, bmpDisabled, styleOptions);


   /* This method changes the bitmap of a bitmap button */
   /* possible options are: (see ConnectBitmapButton) */

::method ChangeBitmapButton unguarded
   use arg id, bmpNormal, bmpFocused, bmpSelected, bmpDisabled, styleOptions
   if Arg(3,'o') = 1 then bmpFocused = ""
   if Arg(4,'o') = 1 then bmpSelected = ""
   if Arg(5,'o') = 1 then bmpDisabled = ""
   if Arg(6,'o') = 1 then styleOptions = ""; else styleOptions = styleOptions~translate
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return BmpButton(self~Adm, "C", id, bmpNormal, bmpFocused, bmpSelected, bmpDisabled, styleOptions);

   /* Sets the position of a bitmap within a button */

::method DisplaceBitmap unguarded
   use arg id, x, y
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return BmpButton(self~Adm, "S", id, x, y)

   /* Gets the position of a bitmap within a button */

::method GetBmpDisplacement unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return BmpButton(self~Adm, "G", id)

   /* This method will set a bitmap as the background brush. */
   /* (seems to work only on NT) */

::method TiledBackgroundBitmap
   use arg bmpFilename
   if self~BkgBrushBmp \= 0 then self~DeleteObject(self~BkgBrushBmp)
   self~BkgBrushBmp = self~CreateBrush(,bmpFilename)
   ret = SetBackground(self~Adm, "BRU", self~BkgBrushBmp)

::method BackgroundColor
   use arg color
   if self~BkgBrushBmp \= 0 then self~DeleteObject(self~BkgBrushBmp)
   self~BkgBrushBmp = self~CreateBrush(color)
   ret = SetBackground(self~Adm, "BRU", self~BkgBrushBmp)

   /* This method sets a bitmap as the background brush. */
   /* (seems to work only on NT) */
   /* argument option can be: */
   /*    "USEPAL" : This will use the color palette of the bitmap */

::method BackgroundBitmap
   use arg bmpFilename, option
   if Arg(2, 'o') = 1 then option = ""; else option = option~translate
   self~BkgBitmap = self~LoadBitmap(bmpFilename, option)
   ret = SetBackground(self~Adm, "BMP", self~BkgBitmap)


::method CreateBrush unguarded
    use arg color, brushSpecifier
    if Arg() = 0 then return HandleDC_Obj("B")
    if Arg(1, "o") = 1 then color = 1
    if Arg(2, "o") = 1 then return HandleDC_Obj("B", color)
    brushSpecifier = brushSpecifier~translate
    if brushSpecifier = "UPDIAGONAL" | brushSpecifier = "DOWNDIAGONAL" | brushSpecifier = "CROSS" | brushSpecifier = "DIAGCROSS",
    | brushSpecifier = "HORIZONTAL" | brushSpecifier = "VERTICAL" then return HandleDC_Obj("B", color, brushSpecifier)
    else return HandleDC_Obj("B", self~Adm, color, brushSpecifier)  /* brushSpecifier must be filename or resource ID */


::method SetItemSysColor
   use arg id, colorbk, colorfg
   if \ id~DataType("W") then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then
       return SetBackground(self~Adm, "COL", id, colorbk, "", "SYS")
   else
       return SetBackground(self~Adm, "COL", id, colorbk, colorfg, "SYS")

::method SetItemColor
   use arg id, colorbk, colorfg
   if \ id~DataType("W") then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then
       return SetBackground(self~Adm, "COL", id, colorbk)
   else
       return SetBackground(self~Adm, "COL", id, colorbk, colorfg)


   /* This method scrolls 'text' in the button */

::method ScrollInButton unguarded
   use arg id, text, fontName, fontSize, fontStyle, displaceY, step, sleep, color
   hnd = self~GetItem(id)
   if Arg() = 1 then
      return self~ScrollText(hnd)
   else
      return self~ScrollText(hnd, text, fontName, fontSize, fontStyle, displaceY, step, sleep, color)


   /* This method will scroll text in a window with the given size, font, */
   /* color. Text will be scrolled from right to left. */
   /* Call it twice and the scrolling will stop */
   /* See method WriteToWindow for valid fontStyles */

::method ScrollText unguarded
   use arg hwnd, text, fontName, fontSize, fontStyle,,
           displaceY, step, sleep, color

   if Arg(3, "o") = 1 | fontName = "FONTNAME" then fontName = "System"
   if Arg(4, "o") = 1 | fontSize = "FONTSIZE" then fontSize = 10
   if Arg(5, "o") = 1 | fontStyle = "FONTSTYLE" then fontStyle = ""
   if Arg(6, "o") = 1 | displaceY = "DISPLACEY" then displaceY = 0
   if Arg(7, "o") = 1 | step = "STEP" then step = 4
   if Arg(8, "o") = 1 | sleep = "SLEEP" then sleep = 10
   if Arg(9, "o") = 1 | color = "COLOR" then color = 0

   if self~ScrollNow = 0 & Arg() > 1 then
   do
      self~ScrollNow = 1
      ret = ScrollText(self~Adm, hwnd, text, fontName, fontSize, fontStyle,,
                       displaceY, step, sleep, color)
   end; else do
      -- Send the interrupt scroll message, 0x0a02 == WM_USER_INTERRUPTSCROLL
      ret = SendWinMsg("ANY",self~DlgHandle, "0x0a02", hwnd, 0)
   end
   self~ScrollNow = 0
   return ret




/******************************************************************************/
/* Graphics and drawing extensions */

   /* Connect WM_DRAWITEM with a method */

::method ConnectDraw
  use arg id, msgToRise
  if Arg(1,'o') = 1 then do
      if arg(2,'o') = 1 then msgToRise = "OnDraw"
      return self~AddUserMsg(msgToRise, 0x0000002B, 0xFFFFFFFF, 0, 0, 0, 0)
  end
  else do
     if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
     if id = -1 then return -1
     return self~AddUserMsg(msgToRise, 0x0000002B, 0xFFFFFFFF, '0x0000'||id~d2x(4), 0xFFFFFFFF, 0, 0)
  end

   /* This method draws the given button */

::method DrawButton unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   return WindowRect("BTN", self~Adm, id)

   /* This method erases the draw area of the given button */

::method ClearButtonRect unguarded
   use arg id
   hwnd = self~GetItem(id)
   return WindowRect("CLR", self~Adm, hwnd)

   /* This method erases the draw area of the given window */

::method ClearWindowRect unguarded
   use arg hwnd
   return WindowRect("CLR", self~Adm, hwnd)

   /* Clears the whole dialog  */
::method Clear unguarded
   parse value WindowRect("GET", self~Hwnd) with r.1 r.2 r.3 r.4
   return WindowRect("CLR", self~Adm, self~Hwnd,r.1,r.2,r.3,r.4)

   /* Clears the given rectangle left, top, right, bottom of the given window */

::method ClearRect unguarded
   use arg hwnd, left, top, right, bottom
   return WindowRect("CLR", self~Adm, hwnd,left,top,right,bottom)

   /* This method redraws the given button */

::method RedrawButton unguarded
   use arg id, erasebkg
   hnd = self~GetItem(id)
   if Arg(2, 'o') = 1 then erasebkg = 0
   return WindowRect("RDW", hnd, erasebkg)

   /* This method redraws the rectangle of the given window */

::method RedrawWindowRect unguarded
   use arg hwnd, erasebkg
   if Arg(1, 'o') = 1 then hwnd = self~hwnd
   if Arg(2, 'o') = 1 then erasebkg = 0
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
   use arg hwnd, xPos, yPos, text, fontName, fontSize, fontStyle, fgColor, bkColor
   if Arg(5, "o") = 1 then fontName = "System"
   if Arg(6, "o") = 1 then fontSize = 10
   if Arg(7, "o") = 1 then fontStyle = ""; else fontStyle = fontStyle~translate
   if Arg(8, "o") = 1 then fgColor = -1
   if Arg(9, "o") = 1 then bkColor = -1
   return WriteText(hwnd, xPos, yPos, text, fontName, fontSize, fontStyle, fgColor, bkColor)  /* more than 6 arguments means use font */


   /* This method allows you to write text to a button in the given font in */
   /* the given size to the given position. */
   /* See method WriteToWindow for valid fontStyles */

::method WriteToButton unguarded
   argarr = Arg(1, "A")
   argarr[1] = self~GetItem(argarr[1])  /* transform first argument into window handle */
   forward message (WriteToWindow) arguments (argarr)


   /* This method moves the rectangle within a button */

::method ScrollButton unguarded
   use arg id, xPos, yPos, left, top, right, bottom
   return ScrollTheWindow(self~Adm, self~GetItem(id), xPos, yPos, left, top, right, bottom, "yes")


   /* This method will return the device context of a window */

::method GetWindowDC unguarded
   use arg hwnd
   return HandleDC_Obj("G", hwnd)

   /* This method will return the device context of a button */

::method GetButtonDC unguarded
   use arg id
   return self~GetWindowDC(self~GetItem(id))

   /* This method will free the device context of a window */

::method FreeWindowDC unguarded
   use arg hwnd, dc
   return HandleDC_Obj("F", hwnd, dc)

   /* This method will free the device context of a button */

::method FreeButtonDC unguarded
   use arg id, dc
   hnd = self~GetItem(id)
   return self~FreeWindowDC(hnd, dc)

   /* This method will return the horizontal bitmap extension */

::method GetBitmapSizeX unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   parse value BmpButton(self~Adm, "E", id) with x .
   return x

   /* This method will return the vertical bitmap extension */

::method GetBitmapSizeY unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   parse value BmpButton(self~Adm, "E", id) with . y
   return y

   /* This method will draw the bitmap of a button */
   /* Use this method to move a bitmap or a part of it... */

::method DrawBitmap unguarded
   use arg hwnd, id, px, py, srcx, srcy, xlen, ylen
   if Arg(3, "o") = 1 then px = 0
   if Arg(4, "o") = 1 then py = 0
   if Arg(5, "o") = 1 then srcx = 0
   if Arg(6, "o") = 1 then srcy = 0
   if Arg(7, "o") = 1 then xlen = 0
   if Arg(8, "o") = 1 then ylen = 0
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(1, "o") = 1 then hwnd = self~GetItem(id)
   return BmpButton(self~Adm, "D", hwnd, id, px, py, srcx, srcy, xlen, ylen)


   /* This method will draw the bitmap step by step */

::method DimBitmap unguarded
   use arg id, bmp, cx, cy, stepx, stepy, steps

   if Arg(5, "o") = 1 then steps = 10
   if Arg(6, "o") = 1 then stepx = 2
   if Arg(7, "o") = 1 then stepy = 2

   dc = self~GetButtonDC(id)
   br = HandleDC_Obj("B",self~Adm, 0, bmp)
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

   if Arg(8, "o") = 1 then delay = 0
   if Arg(9, "o") = 1 then displace = 1
   xl = self~GetBitmapSizeX(id)
   yl = self~GetBitmapSizeY(id)

   if xl <= 0 | yl <= 0 then return 1
   if Arg(6, "o") = 1 then stepx = 0
   if Arg(7, "o") = 1 then stepy = 0

   if stepx = 0 & stepy = 0 then return 1

   curx=fromx
   cury=fromy
   if (stepx > 0) then cursx = -curx; else cursx = 0
   if (stepy > 0) then cursy = -cury; else cursy = 0
   if (cursx > xl) then cursx = xl
   if (cursy > yl) then cursy = yl

   hwnd = self~GetItem(id)
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
   if (delay > 0) then timer = WinTimer("START", delay); else timer = 0
   do while worktodo = 1

      sleft = curx
      srgt = curx+xl
      if srgt > r.right then srgt = r.right
      if sleft < 0 then sleft = 0

      s_top = cury
      sbot = cury+yl
      if sbot > r.bottom-stepy then sbot = r.bottom-stepy
      if s_top < 0 then s_top = 0

      ret = ScrollTheWindow(self~Adm, hwnd, stepx, stepy, sleft, s_top, srgt, sbot, "yes")
      if ret = 1 then do
         if (timer \== 0) then ret = WinTimer("STOP", timer)
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
      if (timer \== 0) then ret = WinTimer("WAIT", timer)

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
   if (timer \== 0) then ret = WinTimer("STOP", timer)
   self~DisplaceBitmap(id, tox, toy);
   self~RedrawButton(id,0)
   return 0

   /* This method will install an animated button and run it concurrently to main */
   /* activity */

::method ConnectAnimatedButton
   use arg buttonid, msgToRise, AutoClass, bmpfrom, bmpto, movex, movey, sizex, sizey, delay, xnow, ynow

   if Arg(2, "o") = 1 then msgToRise = ""
   if Arg(3, "o") = 1 then autoclass = .AnimatedButton
   if Arg(5, "o") = 1 | bmpfrom~datatype("N") \= 1 then
       self~ConnectBitmapButton(buttonid, msgToRise, .local[bmpfrom][1])
   else self~ConnectBitmapButton(buttonid, msgToRise, bmpfrom)

   if Arg(5, "o") = 1 then bmpto = 0
   if Arg(8, "o") = 1 then sizex = self~GetBitmapSizeX(buttonid)
   if Arg(9, "o") = 1 then sizey = self~GetBitmapSizeY(buttonid)
   if Arg(11, "o") = 1 then xnow = 0
   if Arg(12, "o") = 1 then ynow = 0

   self~AddAutoStartMethod(AutoClass, "RUN", buttonid || "," || bmpfrom ||"," || bmpto ||"," || movex || ",",
     || movey || "," || sizex || "," || sizey || "," || delay || "," || xnow || "," || ynow)



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  These menu bar methods are needed to provide backward compatibility for the
  deprecated menu methods.  The ooDialog programmer is *strongly* discouraged
  from using them.  If the deprecated methods are removed, these will be removed
  with them.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

::attribute menuBar get
::attribute menuBar set private

::method linkMenu
  use strict arg menu

  if \ menu~isA(.MenuBar) then return -1

  oldMenu = self~menuBar
  self~menuBar = menu
  return oldMenu

::method unlinkMenu
  menu = self~menuBar
  self~menuBar = .nil
  return menu

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  The following menu bar methods are deprecated, instead, use the Menu classes
  directly.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

::method enableMenuItem unguarded
    use strict arg id
    if self~menuBar == .nil then return -2
    return self~menuBar~enable(id)

::method disableMenuItem unguarded
    use strict arg id
    if self~menuBar == .nil then return -2
    return self~menuBar~disable(id)

::method checkMenuItem unguarded
    use strict arg id
    if self~menuBar == .nil then return -2
    return self~menuBar~check(id)

::method uncheckMenuItem unguarded
    use strict arg id
    if self~menuBar == .nil then return -2
    return self~menuBar~uncheck(id)

::method grayMenuItem unguarded
    use strict arg id
    if self~menuBar == .nil then return -2
    return self~menuBar~gray(id)

::method setMenuItemRadio unguarded
    use strict arg idstart, idend, idset
    if self~menuBar == .nil then return -2
    return self~menuBar~checkRadio(idstart, idend, idset)

::method getMenuItemState unguarded
    use strict arg id
    if self~menuBar == .nil then return -2
    return self~menuBar~getState(id)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  End deprecated menu bar methods.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/******************************************************************************/
/* New extensions */

::method ConnectResize
   use arg msgToRise
   self~AddUserMsg(msgToRise, 0x00000005, 0xFFFFFFFF, 0, 0, 0, 0)

::method ConnectMove
   use arg msgToRise
   self~AddUserMsg(msgToRise, 0x00000003, 0xFFFFFFFF, 0, 0, 0, 0)

::method ConnectPosChanged
   use arg msgToRise
   self~AddUserMsg(msgToRise, 0x00000047, 0xFFFFFFFF, 0, 0, 0, 0)

::method ConnectMouseCapture
   use arg msgToRise
   self~AddUserMsg(msgToRise, 0x00000215, 0xFFFFFFFF, 0, 0, 0, 0)

::method connectMenuItem
   use strict arg id, msgToRaise

   id = self~resolveResourceId(id)
   if id = -1 then return -1

   if id < 3 | id = 9 then return 0

   return self~addUserMsg(msgToRaise, 0x00000111, 0xFFFFFFFF, id, 0x0000FFFF, 0, 0)

-- Connect WM_CONTEXTMENU messages to a method.
::method connectContextMenu
   use strict arg msgToRaise, hwnd = .nil
   if hwnd \== .nil then return self~addUserMsg(msgToRaise, 0x0000007B, 0xFFFFFFFF, hwnd, 0xFFFFFFFF, 0, 0, 0x00000201)
   return self~addUserMsg(msgToRaise, 0x0000007B, 0xFFFFFFFF, 0, 0, 0, 0, 0x00000201)

::method PeekDialogMessage unguarded
   return GetDlgMsg(self~Adm, "Peek")

::method SetWindowRect unguarded
   use arg hnd, x, y, width, high, showOptions
   if Arg(6,"o") = 1 then showOptions = ""; else showOptions = showOptions~translate
   if hnd = 0 | hnd~datatype("N") = 0 then return -1
   return WndShow_Pos("P",hnd, x, y, width, high, showOptions)

   /* Redraws the given rectangle */
::method RedrawRect unguarded
   use arg hwnd, left, top, right, bottom, erasebkg
   if Arg(1,'o') = 1 then hwnd = self~hwnd
   if Arg(6, 'o') = 1 then erasebkg = 0
   return WindowRect("RDW", hwnd,left,top,right,bottom, erasebkg)

::method RedrawWindow unguarded
   use arg hnd
   return WndShow_Pos("S", hnd, "REDRAW")

::method GetFocus unguarded
   return Wnd_Desktop("GETFOC", self~DlgHandle)

::method SetFocus unguarded
   use arg hwnd
   return Wnd_Desktop("SETFOC", self~DlgHandle, hwnd)

::method SetFocusToWindow unguarded
   use arg hwnd
   return Wnd_Desktop("SETFOC", self~DlgHandle, hwnd, 'F')

::method GetMouseCapture
   return Wnd_Desktop("CAP", self~DlgHandle, "G")

::method CaptureMouse
   return Wnd_Desktop("CAP", self~DlgHandle, self~DlgHandle)

::method ReleaseMouseCapture
   return Wnd_Desktop("CAP", self~DlgHandle, "R")

::method IsMouseButtonDown
   use arg mb
   if Arg(1,'o') = 1 then mb = "LEFT"
   else mb = mb~translate~space(0)
   if mb = "LEFT" then return Wnd_Desktop("KSTAT", self~DlgHandle,1) /* VK_LBUTTON */
   if mb = "MIDDLE" then return Wnd_Desktop("KSTAT", self~DlgHandle,4) /* VK_MBUTTON */
   if mb = "RIGHT" then return Wnd_Desktop("KSTAT", self~DlgHandle,2) /* VK_RBUTTON */


::method Dump
    use arg adm
    if Arg() == 1 then do
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
        call DumpAdmin("ADMDUMP")
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

