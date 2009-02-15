/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* Additional Notification Messages and Notification Handlers                 */
/*                                                                            */
/******************************************************************************/

::class 'MessageExtensions' Mixinclass Object public


::method ConnectCommonNotify
   use arg id, type, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   type = type~translate
   select
       when type = "OUTOFMEMORY" then lp =-1
       when type = "CLICK" then lp =-2
       when type = "DBLCLK" then lp =-3
       when type = "ENTER" then lp =-4
       when type = "RCLICK" then lp =-5
       when type = "RDBLCLK" then lp =-6
       when type = "GOTFOCUS" then lp =-7
       when type = "LOSTFOCUS" then lp =-8
       otherwise return -1
   end
   return self~AddUserMsg(msgToRise, 0x0000004E, "0xFFFFFFFF", id, "0xFFFFFFFF", lp, "0xFFFFFFFF")

::method ConnectTreeNotify
   use arg id, type, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   notifySize = 0
   type = type~translate
   select
       when type = "SELCHANGING" then lp = -401
       when type = "SELCHANGED" then lp = -402
       when type = "EXPANDING" then lp = -405
       when type = "EXPANDED" then lp = -406
       when type = "BEGINDRAG" then lp = -407
       when type = "BEGINRDRAG" then lp = -408
       when type = "DELETE" then lp = -409
       when type = "BEGINEDIT" then lp = -410
       when type = "ENDEDIT" then lp = -411
       when type = "DEFAULTEDIT" then do
            self~AddUserMsg("DefTreeEditStarter", 0x0000004E, "0xFFFFFFFF", id, "0xFFFFFFFF", -410, "0xFFFFFFFF", notifySize)
            lp = -411
            msgToRise = "DefTreeEditHandler"
       end
       when type = "KEYDOWN" then lp = -412
       otherwise return -1
   end
   return self~AddUserMsg(msgToRise, 0x0000004E, "0xFFFFFFFF", id, "0xFFFFFFFF", lp, "0xFFFFFFFF", notifySize)


::method ConnectListNotify
   use arg id, type, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   type = type~translate
   select
       when type = "CHANGING"   then lp = -100
       when type = "CHANGED"    then lp = -101
       when type = "INSERTED"   then lp = -102
       when type = "DELETE"     then lp = -103
       when type = "DELETEALL"  then lp = -104
       when type = "BEGINEDIT"  then lp = -105
       when type = "ENDEDIT"    then lp = -106
       when type = "DEFAULTEDIT" then do
            self~AddUserMsg("DefListEditStarter", 0x0000004E, "0xFFFFFFFF", id, "0xFFFFFFFF", -105, "0xFFFFFFFF", notifySize)
            lp = -106
            msgToRise = "DefListEditHandler"
       end
       when type = "COLUMNCLICK" then lp = -108
       when type = "BEGINDRAG"  then lp = -109
       when type = "BEGINRDRAG" then lp = -111
       when type = "ACTIVATE"   then lp = -114
       when type = "KEYDOWN" then lp = -155
       otherwise return -1
   end
   return self~AddUserMsg(msgToRise, 0x0000004E, "0xFFFFFFFF", id, "0xFFFFFFFF", lp, "0xFFFFFFFF")


 -- In the Windows API, when a list-view control sends a notification messsage,
 -- there is more information delievered than there was when ooDialog was first
 -- designed.  Use the connectListViewNotify method for method connections that
 -- can take advantage of this information.  A tag is added to the message to
 -- signal that the extra information should be processed.  The tag is an
 -- unsigned long made up of flags which are defined in oovutil.h
::method connectListViewNotify
   use arg id, type, msgToRise
   if id~DataType("W") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   type = type~translate
   select
      when type = "CLICK" then do
         lp = -2
         tag = 0x00000007
      end
      when type = "CHECKBOXCHANGED" then do
         lp = -101
         tag = 0x00000307
      end
      when type = "SELECTCHANGED" then do
         lp = -101
         tag = 0x00000507
      end
      when type = "FOCUSCHANGED" then do
         lp = -101
         tag = 0x00000907
      end
      when type~abbrev("SELECTFOCUS") then do
         lp = -101
         tag = 0x00000D07
      end
      otherwise return -1
   end
   return self~AddUserMsg(msgToRise, 0x0000004E, "0xFFFFFFFF", id, "0xFFFFFFFF", lp, "0xFFFFFFFF", tag)


::method ConnectButtonNotify
   use arg id, type, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   type = type~translate
   select
       when type = "CLICKED"        then lp = 0
       when type = "PAINT"          then lp = 1
       when type = "HILITE"         then lp = 2
       when type = "UNHILITE"       then lp = 3
       when type = "DISABLE"        then lp = 4
       when type = "DBLCLK"         then lp = 5
       when type = "GOTFOCUS"       then lp = 6
       when type = "LOSTFOCUS"      then lp = 7

       when type = "HOTITEM" then do
         -- Sends WM_NOTIFY and notify code is: BCN_HOTITEMCHANGE=0xfffffb1f (decimal=-1249) TAG_BUTTON = 0x00000004
         return self~AddUserMsg(msgToRise, 0x0000004E, "0xFFFFFFFF", id, "0xFFFFFFFF", 0xFFFFFB1F, "0xFFFFFFFF", 4)
       end

       otherwise return -1
   end
   return self~AddUserMsg(msgToRise, 0x00000111, "0xFFFFFFFF", lp * X2D("10000") + id, "0xFFFFFFFF", 0, 0) /* buttons sends WM_COMMAND when BS_NOTIFY */


::method ConnectEditNotify
   use arg id, type, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   type = type~translate
   select
       when type = "GOTFOCUS"   then lp = "100"
       when type = "LOSTFOCUS"  then lp = "200"
       when type = "CHANGE"     then lp = "300"
       when type = "UPDATE"     then lp = "400"
       when type = "ERRSPACE"   then lp = "500"
       when type = "MAXTEXT "   then lp = "501"
       when type = "HSCROLL"    then lp = "601"
       when type = "VSCROLL"    then lp = "602"
       otherwise return -1
   end
   return self~AddUserMsg(msgToRise, 0x00000111, "0xFFFFFFFF", lp~x2d * X2D("10000") + id, "0xFFFFFFFF", 0, 0) /* also send WM_COMMAND */



::method ConnectListBoxNotify
   use arg id, type, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   type = type~translate
   select
       when type = "SELCHANGE"  then lp = 1
       when type = "DBLCLK"     then lp = 2
       when type = "SELCANCEL"  then lp = 3
       when type = "GOTFOCUS"   then lp = 4
       when type = "LOSTFOCUS"  then lp = 5
       when type = "ERRSPACE"   then lp = -2
       when type = "KEYDOWN" then do   /* can first be called from InitDialog */
           return self~AddUserMsg(msgToRise, 0x0000002E, "0xFFFFFFFF", 0, 0, self~GetItem(id), "0xFFFFFFFF") /* WM_VKEYTOITEM */
       end
       otherwise return -1
   end
   return self~AddUserMsg(msgToRise, 0x00000111, "0xFFFFFFFF", lp * X2D("10000") + id, "0xFFFFFFFF", 0, 0) /* also send WM_COMMAND */


::method ConnectComboBoxNotify
   use arg id, type, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   type = type~translate
   select
       when type = "SELCHANGE"    then lp = 1
       when type = "DBLCLK"       then lp = 2
       when type = "GOTFOCUS"     then lp = 3
       when type = "LOSTFOCUS"    then lp = 4
       when type = "CHANGE"       then lp = 5
       when type = "UPDATE"       then lp = 6
       when type = "DROPDOWN"     then lp = 7
       when type = "CLOSEUP"      then lp = 8
       when type = "SELENDOK"     then lp = 9
       when type = "SELENDCANCEL" then lp = 10
       when type = "ERRSPACE"     then lp = -1
       otherwise return -1
   end
   return self~AddUserMsg(msgToRise, 0x00000111, "0xFFFFFFFF", lp * X2D("10000") + id, "0xFFFFFFFF", 0, 0) /* also send WM_COMMAND */


::method ConnectScrollBarNotify         /* can only be called in InitDialog */
   use arg id, type, msgToRise, category
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(4,'o') = 1 then hnd = self~GetItem(id)
   else hnd = self~GetItem(id, category)
   if hnd = 0 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   type = type~translate
   select
       when type = "UP"    then wp = 0
       when type = "DOWN"    then wp = 1
       when type = "PAGEUP"    then wp = 2
       when type = "PAGEDOWN"    then wp = 3
       when type = "POSITION"    then wp = 4
       when type = "DRAG"    then wp = 5
       when type = "TOP"    then wp = 6
       when type = "BOTTOM"    then wp = 7
       when type = "ENDSCROLL"    then wp = 8
       otherwise return -1
   end
   ret = self~AddUserMsg(msgToRise, 0x00000114, "0xFFFFFFFF", wp, "0x0000FFFF", hnd, "0xFFFFFFFF") /* WM_HSCROLL */
   if ret <> 0 then return ret
   return self~AddUserMsg(msgToRise, 0x00000115, "0xFFFFFFFF", wp, "0x0000FFFF", hnd, "0xFFFFFFFF") /* WM_VSCROLL */


::method ConnectTabNotify
   use arg id, type, msgToRise
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   type = type~translate
   select
       when type = "KEYDOWN"     then lp = -550
       when type = "SELCHANGE"   then lp = -551
       when type = "SELCHANGING" then lp = -552
       otherwise return -1
   end
   return self~AddUserMsg(msgToRise, 0x0000004E, "0xFFFFFFFF", id, "0xFFFFFFFF", lp, "0xFFFFFFFF")


::method ConnectSliderNotify   /* can only be called in InitDialog */
   use arg id, type, msgToRise, category
   if id~DataType("N") = 0 then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if Arg(4,'o') = 1 then hwnd = self~GetItem(id)
   else hwnd = self~GetItem(id, category)
   if hwnd = 0 then return -1
   if Arg(3,'o') = 1 then msgToRise = "On" || type
   type = type~translate
   select
       when type = "UP"       then wp =0
       when type = "DOWN"     then wp =1
       when type = "PAGEUP"   then wp =2
       when type = "PAGEDOWN" then wp =3
       when type = "POSITION" then wp =4
       when type = "DRAG"     then wp =5
       when type = "TOP"      then wp =6
       when type = "BOTTOM"   then wp =7
       when type = "ENDTRACK" then wp =8
       otherwise return -1
   end
   ret = self~AddUserMsg(msgToRise, 0x00000114, "0xFFFFFFFF", wp, "0x0000FFFF", hwnd, "0xFFFFFFFF")  /* WM_HSCROLL */
   if ret <> 0 then return ret
   return self~AddUserMsg(msgToRise, 0x00000115, "0xFFFFFFFF", wp, "0x0000FFFF", hwnd, "0xFFFFFFFF")  /* WM_VSCROLL */


::method connectStaticNotify
   use strict arg id, type, msgToRaise = ""
   if \ id~DataType("W") then id = self~ResolveSymbolicId(id)
   if id = -1 then return -1
   if msgToRaise == "" then msgToRaise = "On" || type
   type = type~translate
   select
       when type = "CLICK"   then lp = 0
       when type = "DBLCLK"  then lp = 1
       when type = "ENABLE"  then lp = 2
       when type = "DISABLE" then lp = 3
       otherwise return -1
   end
   return self~AddUserMsg(msgToRaise, 0x00000111, "0xFFFFFFFF", lp~x2d * X2D("10000") + id, "0xFFFFFFFF", 0, 0)


::method DefTreeEditStarter
    use arg id, dummy, category
    if Arg(3,'o') = 1 then tc = self~GetTreeControl(id)
    else tc = self~GetTreeControl(id, category)
    if tc \= .Nil then tc~SubclassEdit


::method DefTreeEditHandler
    use arg id, item, newText, category
    if Arg(3,'o') = 1 then return
    if Arg(4,'o') = 1 then tc = self~GetTreeControl(id)
    else tc = self~GetTreeControl(id, category)
    if tc \= .Nil then do
        tc~Modify(item,newText)
        tc~RestoreEditClass
    end

::method DefListEditStarter
    use arg id, dummy, category
    if Arg(3,'o') = 1 then lc = self~GetListControl(id)
    else lc = self~GetListControl(id, category)
    if lc \= .Nil then lc~SubclassEdit

::method DefListEditHandler
    use arg id, item, newText, category
    if Arg(3,'o') = 1 then return
    if Arg(4,'o') = 1 then lc = self~GetListControl(id)
    else lc = self~GetListControl(id, category)
    if lc \= .Nil then do
        lc~Modify(item,0,newText)
        lc~RestoreEditClass
    end

::method DefListDragHandler
   use arg id, item, pt
   lc = self~GetListControl(id)
   hc = lc~Cursor_Cross   /* change cursor and store current */
   parse value lc~GetRect with left top right bottom
   parse var pt oldx oldy
   origin = lc~ItemPos(item)
   lmb = self~IsMouseButtonDown("LEFT")
   rmb = self~IsMouseButtonDown("RIGHT")
   do while (lmb \= 0 | rmb \= 0) & \(lmb \= 0 & rmb \= 0)
     pos = self~CursorPos
     parse var pos x y
     parse value lc~ScreenToClient(x, y) with newx newy
     hs = lc~HScrollPos; vs = lc~VScrollPos
     sx = x-right
     sy = y-bottom
      in_rx = (sx <= 30) & (newx >= -30)
      in_ry = (sy <= 30) & (newy >= -30)
      if (in_rx & in_ry) then do    /* is the mouse cursor inside the drag rectangle */
          if x<left then sx = newx - 10; else if x>right then sx = sx + 30; else sx = 0
          if y<top then sy = newy - 10; else if y>bottom then sy = sy + 30; else sy = 0
          newx = newx+hs;  newy = newy +vs;
          if newx < 0 then newx = 0
          if newy < 0 then newy = 0
          if (in_rx & oldx \= newx) | (in_ry & oldy \= newy) then do
           lc~SetItemPos(item, newx, newy)
             oldx = newx
             oldy = newy
             if sx \= 0 | sy \= 0 then do
                lc~Scroll(sx, sy)
                call sleepms 30
             end
        end
      end
      else do    /* no, so force the mouse cursor back inside the rectangle */
         if newx < -30 then newx = -30
         if sx > 30 then newx = (right-left) + 28
         if newy < -30 then newy = -30
         if sy > 30 then newy = (bottom-top) + 28
         parse value lc~ClientToSCreen(newx, newy) with x y
         self~SetCursorPos(x, y)
      end
      lmb = self~IsMouseButtonDown("LEFT")
      rmb = self~IsMouseButtonDown("RIGHT")
   end
   if (lmb \= 0 & rmb \= 0) then do  /* if both buttons pressed restore origin. pos */
      parse var origin x y
      lc~SetItemPos(item, x, y)
   end
   lc~RestoreCursorShape(hc)  /* restore old cursor */
   pos = self~CursorPos
   parse var pos x y
   self~SetCursorPos(x+1, y+1)  /* move cursor to force redraw */



::method DefTreeDragHandler
   use arg id, item, pt
   tc = self~GetTreeControl(id)
   hc = tc~Cursor_Cross   /* change cursor and store current */
   parse value tc~GetRect with left top right bottom
   oldItem = 0
   nocurs = 0
   lmb = self~IsMouseButtonDown("LEFT")
   rmb = self~IsMouseButtonDown("RIGHT")
   call time "R"
   do while (lmb \= 0 | rmb \= 0) & \(lmb \= 0 & rmb \= 0)
     pos = self~CursorPos
     parse var pos x y
     parse value tc~ScreenToClient(x, y) with newx newy
      ht = tc~HitTest(newx, newy)
      if ht \= 0 & ht~wordpos("ONITEM") > 0 then do
          parse var ht newParent where
          /* check if droptarget is the current parent or one of the dragged item's children */
          if newParent \= Item & newParent \= tc~Parent(Item) & tc~IsAncestor(Item, newParent) = 0 then do
             is. = tc~ItemInfo(newParent)
             if is.!State~Wordpos("INDROP") = 0 then
             do
                 call time "R"
                 tc~DropHighlight(newParent)
                 if nocurs \= 0 then do
                    tc~RestoreCursorShape(nocurs)  /* restore old cursor (cross) */
                    nocurs = 0
                 end
             end
             else if time("E") > 1 then do  /* expand node after 1 second */
                 if is.!Children \= 0 & is.!State~Wordpos("EXPANDED") = 0 then tc~expand(newParent)
             end
          end
          else do
             if nocurs = 0 then do
                nocurs = tc~Cursor_No  /* set no cursor and retrieve current cursor (cross) */
                tc~DropHighlight  /* remove drop highlight */
             end
          end
      end
      else do
          if newParent \= 0 then do
              /* necessary to redraw cursor when moving on a valid item again */
              tc~DropHighlight  /* remove drop highlight */
              newParent = 0
          end

          if nocurs = 0 then nocurs = tc~Cursor_No  /* set no cursor and retrieve current cursor (cross) */
          /* handle scrolling */
          fvItem = tc~FirstVisible
          if (y<top) & (fvItem \= tc~Root) then do
              tc~MakeFirstVisible(tc~PreviousVisible(fvItem))
              if top-y < 200 then call sleepms 200-(top-y)
          end
          else if (y>bottom) & (tc~NextVisible(fvItem) \= 0) then do
              tc~MakeFirstVisible(tc~NextVisible(fvItem))
              if y-bottom < 200 then call sleepms 200-(y-bottom)
          end

      end
      lmb = self~IsMouseButtonDown("LEFT")
      rmb = self~IsMouseButtonDown("RIGHT")
   end
   if ht~wordpos("ONITEM") > 0 & lmb = 0 & rmb = 0 then do /* if mouse on item and both mouse buttons up */
       item = tc~MoveItem(Item, newParent, 1)  /* move item under newParent */
   end
   tc~DropHighlight(0)  /* remove drop highlight */
   tc~select(item)      /* select item */
   tc~EnsureVisible(item)
   tc~RestoreCursorShape(hc)  /* restore old cursor */
   pos = self~CursorPos
   parse var pos x y
   self~SetCursorPos(x+1, y+1)  /* move cursor to force redraw */
