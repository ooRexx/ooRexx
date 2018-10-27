/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
 * The EventNotification class has methods to facilitate the mapping of events
 * in the Windows dialog to methods in the Rexx dialog object.
 */
::class 'EventNotification' mixinclass Object public

::method init_eventNotification private external "LIBRARY oodialog en_init_eventNotification"

::method addUserMsg unguarded external "LIBRARY oodialog en_addUserMessage"
::method connectActivate external "LIBRARY oodialog en_connectWmEvent"        -- WM_ACTIVATE
::method connectAllSBEvents external "LIBRARY oodialog en_connectAllSBEvents"
::method connectButtonEvent external "LIBRARY oodialog en_connectButtonEvent"
::method connectComboBoxEvent external "LIBRARY oodialog en_connectComboBoxEvent"
::method connectCommandEvents external "LIBRARY oodialog en_connectCommandEvents"
::method connectDateTimePickerEvent external "LIBRARY oodialog en_connectDateTimePickerEvent"
::method connectDraw external "LIBRARY oodialog en_connectDraw"               -- WM_DRAWITEM
::method connectEachSBEvent external "LIBRARY oodialog en_connectEachSBEvent"
::method connectEditEvent external "LIBRARY oodialog en_connectEditEvent"
::method connectFKeyPress unguarded external "LIBRARY oodialog en_connectFKeyPress"
::method connectHelp external "LIBRARY oodialog en_connectWmEvent"            -- WM_HELP (F1)
::method connectKeyPress unguarded external "LIBRARY oodialog en_connectKeyPress"
::method connectListBoxEvent external "LIBRARY oodialog en_connectListBoxEvent"
::method connectListViewEvent external "LIBRARY oodialog en_connectListViewEvent"
::method connectMonthCalendarEvent external "LIBRARY oodialog en_connectMonthCalendarEvent"
::method connectMove external "LIBRARY oodialog en_connectWmEvent"            -- WM_MOVE
::method connectNotifyEvent external "LIBRARY oodialog en_connectNotifyEvent"
::method connectPosChanged external "LIBRARY oodialog en_connectWmEvent"      -- WM_WINDOWPOSCHANGED
::method connectReBarEvent external "LIBRARY oodialog en_connectReBarEvent"
::method connectResize external "LIBRARY oodialog en_connectWmEvent"          -- WM_SIZE
::method connectResizing external "LIBRARY oodialog en_connectWmEvent"        -- WM_SIZING
::method connectScrollBarEvent external "LIBRARY oodialog en_connectScrollBarEvent"
::method connectSizeMoveEnded external "LIBRARY oodialog en_connectWmEvent"   -- WM_EXITSIZEMOVE
::method connectStaticEvent external "LIBRARY oodialog en_connectStaticEvent"
::method connectStatusBarEvent external "LIBRARY oodialog en_connectStatusBarEvent"
::method connectTabEvent external "LIBRARY oodialog en_connectTabEvent"
::method connectToolBarEvent external "LIBRARY oodialog en_connectToolBarEvent"
::method connectToolTipEvent external "LIBRARY oodialog en_connectToolTipEvent"
::method connectTrackBarEvent external "LIBRARY oodialog en_connectTrackBarEvent"
::method connectTreeViewEvent external "LIBRARY oodialog en_connectTreeViewEvent"
::method connectUpDownEvent external "LIBRARY oodialog en_connectUpDownEvent"
::method disconnectKeyPress unguarded external "LIBRARY oodialog en_disconnectKeyPress"
::method hasKeyPressConnection unguarded external "LIBRARY oodialog en_hasKeyPressConnection"

::method defListDragHandler unguarded
   use arg id, item, pt, isLMB, lc

   hc = lc~cursor_Cross   /* change cursor and store current */
   parse value lc~getRect with left top right bottom
   parse var pt oldx oldy
   origin = lc~itemPos(item)
   lmb = self~isMouseButtonDown("LEFT")
   rmb = self~isMouseButtonDown("RIGHT")
   do while (lmb \= 0 | rmb \= 0) & \(lmb \= 0 & rmb \= 0)
     pos = self~cursorPos
     parse var pos x y
     parse value lc~screenToClient(x, y) with newx newy
     hs = lc~hScrollPos; vs = lc~VScrollPos
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
           lc~setItemPos(item, newx, newy)
             oldx = newx
             oldy = newy
             if sx \= 0 | sy \= 0 then do
                lc~scroll(sx, sy)
                call msSleep 30
             end
        end
      end
      else do    /* no, so force the mouse cursor back inside the rectangle */
         if newx < -30 then newx = -30
         if sx > 30 then newx = (right-left) + 28
         if newy < -30 then newy = -30
         if sy > 30 then newy = (bottom-top) + 28
         parse value lc~clientToSCreen(newx, newy) with x y
         self~setCursorPos(x, y)
      end
      lmb = self~isMouseButtonDown("LEFT")
      rmb = self~isMouseButtonDown("RIGHT")
   end
   if (lmb \= 0 & rmb \= 0) then do  /* if both buttons pressed restore origin. pos */
      parse var origin x y
      lc~setItemPos(item, x, y)
   end
   lc~restoreCursorShape(hc)  /* restore old cursor */
   pos = self~cursorPos
   parse var pos x y
   self~setCursorPos(x+1, y+1)  /* move cursor to force redraw */
   return 0


::method defTreeDragHandler unguarded
   use arg id, item, pt
   reply 0
   tc = self~newTreeView(id)
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
                tc~DropHighlight(newParent)  /* remove drop highlight */
             end
          end
      end
      else do
          if newParent \= 0 then do
              /* necessary to redraw cursor when moving on a valid item again */
              tc~DropHighlight(newParent)  /* remove drop highlight */
              newParent = 0
          end

          if nocurs = 0 then nocurs = tc~Cursor_No  /* set no cursor and retrieve current cursor (cross) */
          /* handle scrolling */
          fvItem = tc~FirstVisible
          if (y<top) & (fvItem \= tc~Root) then do
              tc~MakeFirstVisible(tc~PreviousVisible(fvItem))
              if top-y < 200 then call msSleep 200-(top-y)
          end
          else if (y>bottom) & (tc~NextVisible(fvItem) \= 0) then do
              tc~MakeFirstVisible(tc~NextVisible(fvItem))
              if y-bottom < 200 then call msSleep 200-(y-bottom)
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
   return 0


-- DEPRECATED (all methods to end of file.)
::method connectControl
   forward message "CONNECTCOMMANDEVENTS"
::method connectCommonNotify
   forward message "CONNECTNOTIFYEVENT"
::method connectTreeNotify
   forward message "CONNECTTREEVIEWEVENT"
::method connectListNotify
   forward message "CONNECTLISTVIEWEVENT"
::method connectListViewNotify
   forward message "CONNECTLISTVIEWEVENT"
::method connectButtonNotify
   forward message"CONNECTBUTTONEVENT"
::method connectEditNotify
   forward message "CONNECTEDITEVENT"
::method connectListBoxNotify
   forward message "CONNECTLISTBOXEVENT"
::method connectComboBoxNotify
   forward message "CONNECTCOMBOBOXEVENT"
::method connectScrollBarNotify
   forward message "CONNECTSCROLLBAREVENT"
::method connectTabNotify
   forward message "CONNECTTABEVENT"
::method connectSliderNotify
   forward message "CONNECTTRACKBAREVENT"
::method connectStaticNotify
   forward message "CONNECTSTATICEVENT"
::method connectScrollBar
   forward message "CONNECTEACHSBEVENT"

::method connectMenuItem
   use strict arg id, msgToRaise
   return .Menu~connectCommandEvent(id, msgToRaise, self)

::method connectButton
   use strict arg id, msgToRaise
   return self~connectButtonEvent(id, "CLICKED", msgToRaise)

::method connectList
   use strict arg id, msgToRaise
   return self~connectListBoxEvent(id, "SELCHANGE", msgToRaise)

::method connectListLeftDoubleClick
   use strict arg id, msgToRise
   return self~connectListBoxEvent(id, "DBLCLK", msgToRaise)

-- WM_CAPTURECHANGED
::method connectMouseCapture
   use strict arg methodName = 'onCaptureChanged'
   mouse = .Mouse~new(self)
   return (mouse~connectEvent('CAPTURECHANGED', methodName, .false) \== .true)

