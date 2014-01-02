/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
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
 * This example program demonstrates one approach to adding drag-and-drop to an
 * ooDialog program using the .Mouse class.
 */

  sd = locate()
  .application~setDefaults('O', sd'dragNdrop.h', .false)

  dlg = .DreamTeamDlg~new(sd'dragNdrop.rc', IDD_NFL_DLG)

  if dlg~initCode == 0 then do
    dlg~execute("SHOWTOP", IDI_DLG_OOREXX)
  end

::requires "ooDialog.cls"

::class 'DreamTeamDlg' subclass RcDialog

::method init
  expose dropOkCursor noDropCursor cursorIsNoDrop dragging
  forward class (super) continue

  -- This will load a cursor from a file.  The file has to actually be a cursor
  -- or the method will fail.  The operating system will refuse to load anything
  -- but an actual cursor.  The return is an .Image object.
  dropOkCursor = .Mouse~loadCursorFromFile(.application~srcDir"dragging.cur")
  if dropOkCursor == 0 then do
    say 'Error loading Drop Ok Cursor system error code:' .SystemErrorCode
    self~initCode = 1
    return self~initCode
  end

  -- This loads the system's predefined NO cursor.  Again the return is a .Image
  -- object.
  noDropCursor = .Mouse~loadCursor("NO")
  if noDropCursor == 0 then do
    say 'Error loading No Drop Cursor, system error code:' .SystemErrorCode
    self~initCode = 1
    return self~initCode
  end

  cursorIsNoDrop = .false
  dragging       = .false

  return 0

::method initDialog
  expose lvNFL lvWest lvEast noDropCursor dragItem nfl2west nfl2east

  lvNFL = self~newListView(IDC_LV_NFL)
  self~setUpListView(lvNFL)

  lvWest = self~newListView(IDC_LV_WEST)
  self~setUpListView(lvWest)

  lvEast = self~newListView(IDC_LV_EAST)
  self~setUpListView(lvEast)

  self~setMice

  self~mapDropTargets

  dragItem = .nil


/** mapDropTargets()
 *
 */
::method mapDropTargets private
  expose lvNFL lvWest lvEast nfl2west nfl2east west2nfl west2east east2nfl east2west

  nfl2west = lvWest~clientRect
  lvWest~mapWindowPoints(lvNFL~hwnd, nfl2west)

  nfl2east = lvEast~clientRect
  lvEast~mapWindowPoints(lvNFL~hwnd, nfl2east)

  west2nfl = lvNFL~clientRect
  lvNFL~mapWindowPoints(lvWest~hwnd, west2nfl)

  west2east = lvEast~clientRect
  lvEast~mapWindowPoints(lvWest~hwnd, west2east)

  east2nfl = lvNFL~clientRect
  lvNFL~mapWindowPoints(lvEast~hwnd, east2nfl)

  east2west = lvWest~clientRect
  lvWest~mapWindowPoints(lvEast~hwnd, east2west)


::method doDrag private
  expose oldCursor noDropCursor cursorIsNoDrop dragging
  use arg lv, index, p, mouse

  mouse~capture
  oldCursor = mouse~setCursor(noDropCursor)

  di = .DragItem~new(lv, index, p, mouse)

  cursorIsNoDrop = .true
  dragging       = .true

  return di


::method nflOnLBdown unguarded
  expose lvNFL dragItem
  use arg keyState, p, mouse

  index = lvNFL~hitTestInfo(p)

  if keyState \== 'lButton' | index = -1 then return .false

  lvNFL~assignFocus
  lvNFL~focus(index)
  lvNFL~select(index)

  if mouse~dragDetect(p) then do
    dragItem = self~doDrag(lvNFL, index, p, mouse)
    return .true
  end

  return .false

::method nflOnLBup unguarded
  expose oldCursor cursorIsNoDrop dragging lvNFL lvWest lvEast dragItem nfl2west nfl2east
  use arg keyState, p, mouse

  if dragging then do
    okayToDrop = (cusorIsNoDrop \== .true)

    dragging = .false
    cursorIsNoDrop = .false
    mouse~releaseCapture
    mouse~setCursor(oldCursor)

    if okayToDrop & dragItem \== .nil then do
      if p~inRect(nfl2west) then do
        dragItem~target = lvWest
      end
      else if p~inRect(nfl2east) then do
        dragItem~target = lvEast
      end
      else do
        -- Theoretically this can not happen, but just to be sure...
        dragItem = .nil
        return 0
      end

      -- The point p is in client coordinates of the NFL list view.  We are
      -- going to map that point back to the client coordinates of the list view
      -- where the row is being dropped.  Then the drag item can determine where
      -- the insertion poing for the row should be in the target list view.
      lvNFL~mapWindowPoints(dragItem~target~hwnd, p)

      dragItem~drop(p)
      dragItem = .nil
    end
  end

  return 0


::method nflOnMove unguarded
  expose dragging dropOkCursor noDropCursor cursorIsNoDrop nfl2west nfl2east
  use arg keyState, p, mouse

  if dragging then do
    if p~inRect(nfl2west) then do
      if cursorIsNoDrop then do
        mouse~setCursor(dropOkCursor)
        cursorIsNoDrop = .false
      end
    end
    else if p~inRect(nfl2east) then do
      if cursorIsNoDrop then do
        mouse~setCursor(dropOkCursor)
        cursorIsNoDrop = .false
      end
    end
    else do
      if \ cursorIsNoDrop then do
        mouse~setCursor(noDropCursor)
        cursorIsNoDrop = .true
      end
    end
  end

  return 0

::method westOnLBdown unguarded
  expose lvWest  dragItem
  use arg keyState, p, mouse

  index = lvWest~hitTestInfo(p)

  if keyState \== 'lButton' | index = -1 then return .false

  lvWest~assignFocus
  lvWest~focus(index)
  lvWest~select(index)

  if mouse~dragDetect(p) then do
    dragItem = self~doDrag(lvWest, index, p, mouse)
    return .true
  end

  return .false


::method westOnLBup unguarded
  expose oldCursor cursorIsNoDrop dragging lvNFL lvWest lvEast dragItem west2nfl west2east
  use arg keyState, p, mouse

  if dragging then do
    okayToDrop = (cusorIsNoDrop \== .true)

    dragging = .false
    cursorIsNoDrop = .false
    mouse~releaseCapture
    mouse~setCursor(oldCursor)

    if okayToDrop & dragItem \== .nil then do
      if p~inRect(west2nfl) then do
        dragItem~target = lvNFL
      end
      else if p~inRect(west2east) then do
        dragItem~target = lvEast
      end
      else do
        -- Theoretically this can not happen, but just to be sure...
        dragItem = .nil
        return 0
      end

      -- The point p is in client coordinates of the West list view.  We are
      -- going to map that point back to the client coordinates of the list view
      -- where the row is being dropped.  Then the drag item can determine where
      -- the insertion poing for the row should be in the target list view.
      lvWest~mapWindowPoints(dragItem~target~hwnd, p)

      dragItem~drop(p)
      dragItem = .nil
    end
  end

  return 0


::method westOnMove unguarded
  expose dragging dropOkCursor noDropCursor cursorIsNoDrop west2nfl west2east
  use arg keyState, p, mouse

  if dragging then do
    if p~inRect(west2nfl) then do
      if cursorIsNoDrop then do
        mouse~setCursor(dropOkCursor)
        cursorIsNoDrop = .false
      end
    end
    else if p~inRect(west2east) then do
      if cursorIsNoDrop then do
        mouse~setCursor(dropOkCursor)
        cursorIsNoDrop = .false
      end
    end
    else do
      if \ cursorIsNoDrop then do
        mouse~setCursor(noDropCursor)
        cursorIsNoDrop = .true
      end
    end
  end

  return 0

::method eastOnLBdown unguarded
  expose lvEast  dragItem
  use arg keyState, p, mouse

  index = lvEast~hitTestInfo(p)

  if keyState \== 'lButton' | index = -1 then return .false

  lvEast~assignFocus
  lvEast~focus(index)
  lvEast~select(index)

  if mouse~dragDetect(p) then do
    dragItem = self~doDrag(lvEast, index, p, mouse)
    return .true
  end

  return .false


::method eastOnLBup unguarded
  expose oldCursor cursorIsNoDrop dragging lvNFL lvWest lvEast dragItem east2nfl east2west
  use arg keyState, p, mouse

  if dragging then do
    okayToDrop = (cusorIsNoDrop \== .true)

    dragging = .false
    cursorIsNoDrop = .false
    mouse~releaseCapture
    mouse~setCursor(oldCursor)

    if okayToDrop & dragItem \== .nil then do
      if p~inRect(east2nfl) then do
        dragItem~target = lvNFL
      end
      else if p~inRect(east2west) then do
        dragItem~target = lvWest
      end
      else do
        -- Theoretically this can not happen, but just to be sure...
        dragItem = .nil
        return 0
      end

      -- The point p is in client coordinates of the East list view.  We are
      -- going to map that point back to the client coordinates of the list view
      -- where the row is being dropped.  Then the drag item can determine where
      -- the insertion poing for the row should be in the target list view.
      lvEast~mapWindowPoints(dragItem~target~hwnd, p)

      dragItem~drop(p)
      dragItem = .nil
    end
  end

  return 0


::method eastOnMove unguarded
  expose dragging dropOkCursor noDropCursor cursorIsNoDrop east2nfl east2west
  use arg keyState, p, mouse

  if dragging then do
    if p~inRect(east2nfl) then do
      if cursorIsNoDrop then do
        mouse~setCursor(dropOkCursor)
        cursorIsNoDrop = .false
      end
    end
    else if p~inRect(east2west) then do
      if cursorIsNoDrop then do
        mouse~setCursor(dropOkCursor)
        cursorIsNoDrop = .false
      end
    end
    else do
      if \ cursorIsNoDrop then do
        mouse~setCursor(noDropCursor)
        cursorIsNoDrop = .true
      end
    end
  end

  return 0



::method setMice private
  expose mLVnfl mLVwest mLVeast lvNFL lvWest lvEast

  mLVnfl  = .Mouse~new(lvNFL)
  mLVwest = .Mouse~new(lvWest)
  mLVeast = .Mouse~new(lvEast)

  mLVnfl~connectEvent('LBUTTONDOWN', nflOnLBdown)
  mLVnfl~connectEvent('LBUTTONUP', nflOnLBup)
  mLVnfl~connectEvent('MOUSEMOVE', nflOnMove)

  mLVwest~connectEvent('LBUTTONDOWN', westOnLBdown)
  mLVwest~connectEvent('LBUTTONUP', westOnLBup)
  mLVwest~connectEvent('MOUSEMOVE', westOnMove)

  mLVeast~connectEvent('LBUTTONDOWN', eastOnLBdown)
  mLVeast~connectEvent('LBUTTONUP', eastOnLBup)
  mLVeast~connectEvent('MOUSEMOVE', eastOnMove)

::method setUpListView private
  use strict arg lv

  lv~addExtendedStyle("GRIDLINES DOUBLEBUFFER FULLROWSELECT")
  lv~insertColumnPX(0, 'Player', 105, 'LEFT')
  lv~insertColumnPX(1, 'Team', 115, 'CENTER')
  lv~insertColumnPX(2, 'Position', 65, 'CENTER')
  lv~insertColumnPX(3, 'Rating', 65, 'CENTER')

  if lv~id == .constDir[IDC_LV_NFL] then do
    playerRows = self~getPlayers
    do l over playerRows
      lv~addRow( , , l~player, l~team, l~pos, l~rating)
    end
  end

::method getPlayers private

  rows = .Array~new

  fileObj = .stream~new(.application~srcDir"nflPlayers.txt")
  players = fileObj~makeArray

  do line over players
    line = line~strip
    if line~length == 0 | line~abbrev("#") | line~abbrev("/*") then iterate

    parse var line name', 'team', 'p', 'rating .

    r = .Directory~new
    r~player = name
    r~team   = team
    r~pos    = p
    r~rating = rating
    rows~append(r)
  end

  fileObj~close

  return rows


/* Class: DragItem - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\

    Drag items are an encapsulation of the data needed to drag and drop one row
    of a list view to another.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'DragItem'

::attribute target

::method init
  expose lv index row target
  use arg lv, index, p, mouse

  row = .Directory~new
  lv~getItemInfo(index, row)
  row~player = row~text

  lv~getItemInfo(index, row, 1)
  row~team = row~text

  lv~getItemInfo(index, row, 2)
  row~pos = row~text

  lv~getItemInfo(index, row, 3)
  row~rating = row~text

  target = .nil

::method drop
  expose lv index row target
  use strict arg pt = 0

  if target \== .nil then do
    -- Drop the row at the end of the list by default.
    targetIndex = target~items

    if pt~isA(.Point) then do
      i = target~hitTestInfo(pt)
      if i <> -1 then targetIndex = i
    end

    target~addRow(targetIndex, , row~player, row~team, row~pos, row~rating)

    lv~delete(index)

    count = lv~items
    if count <> 0 then do
      if index == count then lv~select(index - 1)
      else lv~select(index)
    end
  end


