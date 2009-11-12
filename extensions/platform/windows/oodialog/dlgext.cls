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
 * Dialog class extensions.
 */

::class 'DialogExtensions' public mixinclass Object

-- Execute with asynchronous message dispatching.
::method executeAsync unguarded
   use strict arg sleepTime = 10, sh = "NORMAL", icon = 0, modeless = .false

   if self~startIt(icon, modeless) \= 0 then do
      if self~useStem then self~setDataStem(self~dlgData)
      else self~setData
      self~show(sh)
      self~start("asyncMessageHandling", sleepTime)
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

-- End the asynchronous execution of the dialog.
::method endAsyncExecution unguarded
   do while self~finished = 0
      call msSleep 20
   end
   if self~initCode = 1 then do
      if self~useStem then self~getDataStem(self~dlgData)
      self~getData
   end
   if self~adm \= 0 then self~stopit
   return self~initCode

-- Asynchronously dispatch windows messages.
::method asyncMessageHandling unguarded private
   use strict arg sleepTime = 10
   if \ sleepTime~datatype('W') then sleepTime = 10

   do while self~finished = 0 & self~adm \= 0
      msg = getDlgMsg(self~adm)
      if msg~pos("1DLGDELETED1") > 0 then do
         if self~finished = 0 then self~finished = 2
      end
      else do
         if msg \= "" then do
            parse var msg methodName '(' args ')'
            argList = args~space(0)~makeArray(',')
            self~sendWith(methodName, argList)
         end
         else do
            call msSleep sleepTime
         end
      end
   end

-- Run a modeless dialog
::method popup
   use strict arg sh = "NORMAL", sleepTime = 10, icon = 0
   if \ sleepTime~datatype('W') then sleepTime = 10
   self~start("execute", sh, icon, sleeptime, .true)

::method popupAsChild
   use strict arg parent, sh = "NORMAL", sleepTime = 10, icon = 0

   if \ parent~isA(.PlainBaseDialog) then do
       call errorDialog "Specified parent is not a dialog!"
       return -1
   end

   if \ sleepTime~datatype('W') then sleepTime = 10
   parent~childDialogs~insert(self)  /* add to the parents child list (last item) */
   self~parentDlg = parent

   self~start("execute", sh, icon, sleeptime, .true)

::method peekDialogMessage unguarded
   return getDlgMsg(self~adm, .true)


 /* Scrollbar methods */
::method setSBRange unguarded
   use arg id, min, max, redraw = .true
   sb = self~newScrollBar(id)
   if sb == .nil then return -1
   return sb~setRange(min, max, redraw)

::method GetSBRange unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return HandleScrollBar("GR",self~getControlHandle(id))

::method SetSBPos unguarded
   use arg id, pos, redraw
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if arg(3, "o") = 1 then redraw = 1
   return HandleScrollBar("SP",self~getControlHandle(id),pos, redraw)

::method GetSBPos unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return HandleScrollBar("GP",self~getControlHandle(id))

::method CombineELwithSB
   use arg elid, sbid, step, poscode

   pos = self~getEditData(elid)
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
   self~setEditData(elid,pos)
   return 0


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

::method setControlFont unguarded
   use strict arg id, hFont, bRedraw = .true
   hCtrl = self~getControlHandle(id)
   msg   = self~WM_SETFONT
   return self~sendWinHandleMsg(msg, hFont, bRedraw, hCtrl)

::method getWindowRect unguarded -- old method returns coordinates as a string
   forward message "windowRect" continue
   rect = result
   return rect~left rect~top rect~right rect~bottom

::method setWindowRect unguarded external "LIBRARY oodialog dlgext_setWindowRect"
::method getControlRect unguarded external "LIBRARY oodialog dlgext_getControlRect"
::method resizeControl unguarded external "LIBRARY oodialog dlgext_resizeMoveControl"
::method moveControl unguarded external "LIBRARY oodialog dlgext_resizeMoveControl"

::method clearRect unguarded external "LIBRARY oodialog dlgext_clearRect"
::method clearWindowRect unguarded external "LIBRARY oodialog dlgext_clearWindowRect"
::method clearControlRect unguarded external "LIBRARY oodialog dlgext_clearControlRect"
::method redrawRect unguarded external "LIBRARY oodialog dlgext_redrawRect"
::method redrawWindowRect unguarded external "LIBRARY oodialog dlgext_redrawWindowRect"
::method redrawControl unguarded external "LIBRARY oodialog dlgext_redrawControl"
::method setForegroundWindow unguarded external "LIBRARY oodialog dlgext_setForgroundWindow"

::method drawButton unguarded external "LIBRARY oodialog dlgext_drawButton"

::method redrawWindow unguarded
   use arg hwnd
   return .Window~new(hwnd)~redraw

-- DEPRECATED
::method getButtonRect unguarded -- old method returns coordinates as a string
   forward message "getControlRect" continue
   rect = result
   if \ rect~isA(.Rect) then return 1  -- mimic old behavior
   return rect~left rect~top rect~right rect~bottom

::method clearButtonRect unguarded
   forward message 'CLEARCONTROLRECT'

::method redrawButton unguarded
   forward message 'REDRAWCONTROL'

::method moveItem unguarded      -- old method, this is not accurate
   use strict arg id, xPos, yPos, showOptions = ""
   p = .Point~new(trunc(xPos * self~factorX), trunc(yPos * self~factorY))
   return self~moveControl(id, p, showOptions)

::method resizeItem unguarded    -- old method, this is not accurate
   use strict arg id, width, height, showOptions = ""
   s = .Size~new(trunc(width * self~factorX), trunc(height * self~factorY))
   return self~resizeControl(id, s, showOptions)

::method setItemFont unguarded
   forward message 'setControlFont'


/******************************************************************************/
/* Bitmap extensions */

::method BkgBitmap attribute protected   -- handle to bitmap that will be displayed in the background
::method BkgBrushBmp attribute protected -- handle to brushbmp that will be displayed in the background

-- This method will add a method that will be started automatically and run concurrently when
-- the dialog is executed. That is the  method 'InClass'~'MethodName' will be started, when
-- the dialog receives the message 'execute' or execute async. You can use this method to animate
-- buttons.

::method addAutoStartMethod
   use arg inClass = (self), methodName, parameters
   self~automaticMethods~push(inClass)
   if arg(2,"E") then self~automaticMethods~push(methodName)
   if arg(3,"E") then self~automaticMethods~push(parameters)

-- Set a bitmap as the background brush. The argument option can be:
--   "USEPAL" : This will use the color palette of the bitmap.
::method backgroundBitmap
   use strict arg bmpFilename, option = ""
   option = option~translate
   self~bkgBitmap = self~loadBitmap(bmpFilename, option)
   ret = SetBackground(self~adm, "BMP", self~bkgBitmap)

-- Install a bitmap button.
--
-- The only way to get the bitmaps into the bitmap table is through this method.
-- Therefore, we have to allow an omitted msgToRise.  If msgToRise == "", then
-- in BmpButton() the function skips adding a method to the message table.
::method installBitmapButton
   use strict arg id, msgToRise = "", bmpNormal, bmpFocused = "", bmpSelected = "", bmpDisabled = "", styleOptions = ""
   styleOptions = styleOptions~translate
   if \ id~datatype("W") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return BmpButton(self~adm, "A", msgToRise, id, bmpNormal, bmpFocused, bmpSelected, bmpDisabled, styleOptions);

-- Install an animated button and run it concurrently to the main activity.
--
-- The only way to get the bitmap(s) into the bitmap table is through this
-- method.  Therefore msgToRaise has to be optional.  The empty string will
-- eventually cause the bitmaps to be added to the bitmap table and the addition
-- of a method being added to the message table will be skipped.
::method installAnimatedButton
   use arg buttonID, msgToRaise = "", autoClass = (.AnimatedButton), bmpFrom, bmpTo = 0, moveX, moveY,   -
           sizeX, sizeY, delay, xNow = 0, yNow = 0

   -- Note documentation for bmpfrom, it is *not* a bitmap handle.
   if arg(5, "o") | \ bmpFrom~datatype("N") then
       ret = self~installBitmapButton(buttonID, msgToRaise, .local[bmpFrom][1])
   else
       ret = self~installBitmapButton(buttonID, msgToRaise, bmpFrom)

   if ret <> 0 then return ret

   if arg(8, "O") then sizeX = self~getBitmapSizeX(buttonID)
   if arg(9, "O") then sizeY = self~getBitmapSizeY(buttonID)

   self~addAutoStartMethod(autoClass, "RUN", buttonID || "," || bmpFrom ||"," || bmpTo ||"," || moveX    -
     || "," || moveY || "," || sizeX || "," || sizeY || "," || delay || "," || xNow || "," || yNow)

-- DEPRECATED  TODO don't forget to document name change / deprecated
::method connectAnimatedButton
   forward message "INSTALLANIMATEDBUTTON"
::method connectBitmapButton
   forward message "INSTALLBITMAPBUTTON"


-- Change the bitmap of a bitmap button.
::method ChangeBitmapButton unguarded
   use arg id, bmpNormal, bmpFocused = "", bmpSelected = "", bmpDisabled = "", styleOptions = ""
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   styleOptions = styleOptions~translate
   return BmpButton(self~adm, "C", id, bmpNormal, bmpFocused, bmpSelected, bmpDisabled, styleOptions);

-- Set the position of a bitmap within a button.
::method DisplaceBitmap unguarded
   use arg id, x, y
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return BmpButton(self~adm, "S", id, x, y)

-- Get the position of a bitmap within a button.
::method GetBmpDisplacement unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   return BmpButton(self~adm, "G", id)

-- Set a bitmap as the background brush.
::method TiledBackgroundBitmap
   use arg bmpFilename
   if self~BkgBrushBmp \= 0 then self~DeleteObject(self~BkgBrushBmp)
   self~BkgBrushBmp = self~CreateBrush(,bmpFilename)
   ret = SetBackground(self~adm, "BRU", self~BkgBrushBmp)

-- Return the horizontal bitmap extension.
::method GetBitmapSizeX unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   parse value BmpButton(self~adm, "E", id) with x .
   return x

-- Return the vertical bitmap extension.
::method GetBitmapSizeY unguarded
   use arg id
   if id~DataType("N") = 0 then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   parse value BmpButton(self~adm, "E", id) with . y
   return y

-- Draw the bitmap of a button.  Use this method to move a bitmap or a part of it...
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
   if arg(1, "o") = 1 then hwnd = self~getControlHandle(id)
   return BmpButton(self~adm, "D", hwnd, id, px, py, srcx, srcy, xlen, ylen)

-- Draw the bitmap step by step.
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

-- Scroll a bitmap from one position to another within a button.
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

   button = self~newPushButton(id)
   if button == .nil then return -1
   hwnd = button~hwnd

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

      ret = button~scroll(stepx, stepy, trunc(sleft), trunc(s_top), trunc(srgt), trunc(sbot))
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

::method BackgroundColor
   use arg color
   if self~BkgBrushBmp \= 0 then self~DeleteObject(self~BkgBrushBmp)
   self~BkgBrushBmp = self~CreateBrush(color)
   ret = SetBackground(self~adm, "BRU", self~BkgBrushBmp)

::method CreateBrush unguarded
    use arg color = 1, brushSpecifier = ""
    if arg() = 0 then return HandleDC_Obj("B")
    if brushSpecifier == "" then return HandleDC_Obj("B", color)
    brushSpecifier = brushSpecifier~translate
    if brushSpecifier = "UPDIAGONAL" | brushSpecifier = "DOWNDIAGONAL" | brushSpecifier = "CROSS" | brushSpecifier = "DIAGCROSS",
    | brushSpecifier = "HORIZONTAL" | brushSpecifier = "VERTICAL" then return HandleDC_Obj("B", color, brushSpecifier)
    else return HandleDC_Obj("B", self~adm, color, brushSpecifier)  /* brushSpecifier must be filename or resource ID */


::method setControlSysColor
   use arg id, colorbk, colorfg
   if \ id~DataType("W") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if arg(3,'o') = 1 then
       return SetBackground(self~adm, "COL", id, colorbk, "", "SYS")
   else
       return SetBackground(self~adm, "COL", id, colorbk, colorfg, "SYS")

::method setControlColor
   use arg id, colorbk, colorfg
   if \ id~DataType("W") then id = self~resolveSymbolicId(id)
   if id = -1 then return -1
   if arg(3,'o') = 1 then
       return SetBackground(self~adm, "COL", id, colorbk)
   else
       return SetBackground(self~adm, "COL", id, colorbk, colorfg)

::method setItemColor
   forward message "setControlColor"

::method setItemSysColor
   forward message "setControlSysColor"


   /* This method scrolls 'text' in the button */

::method scrollInButton unguarded
   use arg id, text, fontName, fontSize, fontStyle, displaceY, step, sleep, color
   hwnd = self~getControlHandle(id)
   if arg() = 1 then
      return self~ScrollText(hwnd)
   else
      return self~ScrollText(hwnd, text, fontName, fontSize, fontStyle, displaceY, step, sleep, color)


-- This method will scroll text in a window with the given size, font, color. Text will be scrolled from
-- right to left. Call it twice and the scrolling will stop.  See method WriteToWindow for valid fontStyles.
::method scrollText unguarded
   use strict arg hwnd, text = "", fontName = "System", fontSize = 10, fontStyle = "", -
                  displaceY = 0, step = 4, sleep = 10, color = 0

   if self~scrollNow = 0 & arg() > 1 then do
      self~scrollNow = 1
      ret = ScrollText(self~adm, hwnd, text, fontName, fontSize, fontStyle, displaceY, step, sleep, color)
   end
   else do
      self~sendWinHandleMsg(self~WM_USER_INTERRUPTSCROLL, hwnd, 0)
      ret = 0
   end
   self~scrollNow = 0
   return ret




/******************************************************************************/
/* Graphics and drawing extensions */

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

::method WriteToWindow unguarded external "LIBRARY oodialog dlgext_writeToWindow"


   /* This method allows you to write text to a button in the given font in */
   /* the given size to the given position. */
   /* See method WriteToWindow for valid fontStyles */

::method WriteToButton unguarded
   argarr = arg(1, "A")
   argarr[1] = self~getControlHandle(argarr[1])  /* transform first argument into window handle */
   forward message (WriteToWindow) arguments (argarr)


-- This method moves the rectangle within a button.  TODO be sure can take a .Rect for arg
::method ScrollButton unguarded
   -- use arg id, xPos, yPos, left, top, right, bottom
   newArgs = arg(2, "A")
   button = self~newPushButton(arg(1))
   if button == .nil then return -1
   return button~sendWith("Scroll", newArgs)

   /* This method will return the device context of a window */

::method GetWindowDC unguarded
   use arg hwnd
   return HandleDC_Obj("G", hwnd)

   /* This method will return the device context of a button */

::method GetButtonDC unguarded
   use arg id
   return self~GetWindowDC(self~getControlHandle(id))

   /* This method will free the device context of a window */

::method FreeWindowDC unguarded
   use arg hwnd, dc
   return HandleDC_Obj("F", hwnd, dc)

   /* This method will free the device context of a button */

::method FreeButtonDC unguarded
   use arg id, dc
   hwnd = self~getControlHandle(id)
   return self~FreeWindowDC(hwnd, dc)

::method CaptureMouse external "LIBRARY oodialog dlgext_captureMouse"
::method getMouseCapture external "LIBRARY oodialog dlgext_mouseCapture"
::method releaseMouseCapture external "LIBRARY oodialog dlgext_mouseCapture"
::method isMouseButtonDown external "LIBRARY oodialog dlgext_isMouseButtonDown"


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

