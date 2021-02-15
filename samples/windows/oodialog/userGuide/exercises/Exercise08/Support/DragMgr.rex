/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
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
/* ooDialog User Guide - Exercise08

   Support - DragMgr					 	 v01-00  26Jun13
   ---------------------
   A singleton component that manages direct manipulation.

   Notes: A 'source' view is a dialog that's "picked up" by pressing and holding
          button 1 (usually the left button) of the mouse.
          A 'target' view is a dialog that's dropped onto by releasing
          button 1 over the target dialog.

   Interface DragMgr {
     setTarget
     setSource
     removeDlg
     list         lists tables of source & target dialogs.
     dmPickup 	  captures mouse - i.e. sets the mouse capture to the window
           	  of this mouse instance.
     moving	  If over a target (as supered by a dlg instance):
                  - check if this is a valid target AND is topmost window
		  - If over a valid drop area send dmQueryDrop(sourceDlg,mousePos)
		    to target.
		  - If response is .true then show dropOKCursor
		    else show noDropCursor (system-provided).
     dmDrop	  - If cursor is no-drop then no-op.
		    Else send dmDrop(sourceDlg) to drop target.
     }

  Changes:
    v01-00 06Jun13: First version.
           18Jun13: Changed 'drop' method name to 'dmDrop'.
           26Jun13: Change 'queryTables' method name to 'list' (to conform with
                    other Managers).

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::REQUIRES ooDialog.cls
::REQUIRES "Order\OrderModelsData.rex"

/*//////////////////////////////////////////////////////////////////////////////
  ============================================================================*/
::CLASS DragMgr PUBLIC

  ::ATTRIBUTE targetDialogs PRIVATE  -- dlg - hwnd, dropArea
  ::ATTRIBUTE sourceDialogs PRIVATE  -- dlg - mouse, dropOkCursor, pickupArea
  ::ATTRIBUTE dragInProgress
  --::ATTRIBUTE draggingSourceDlg PRIVATE


  /*----------------------------------------------------------------------------
    init - initialises the Drag Manager
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    expose icons dragging cursorIsNoDrop noDropCursor
    .local~my.DragMgr = self
    --say "DragMgr-init-01: .local~my.DragMgr =" .local~my.DragMgr
    self~dragInProgress = .false
    self~targetDialogs = .Table~new	-- Index = dlg id
    					-- Item = an Array: hwnd, dropArea,
    self~sourceDialogs = .Table~new	-- Index = dlg id,
    					-- Item = an Array: mouse, cursor, pickupArea
    noDropCursor = .Mouse~loadCursor("NO")
    if noDropCursor == 0 then do
      say "DragManager-init-02:" .HRSdm~badNoDropCursor .SystemErrorCode
    end

    cursorIsNoDrop = .false
    dragging       = .false

    return self


  /*----------------------------------------------------------------------------
    setTarget - Adds a target view component to the Targets table
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD setTarget PUBLIC
    --expose targetDialogs
    use strict arg dlg, hwnd, dropArea
    items = .Array~new
    items[1] = hwnd;  items[2] = dropArea
    self~targetDialogs[dlg] = items


  /*----------------------------------------------------------------------------
    setSource - Adds a source view component to the Targets table
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD setSource PUBLIC
    --expose sourceDialogs dropOkCursor
    --use strict arg dlg, mouse, cursorFile, pickupArea, control
    use strict arg sourceWin, mouse, cursorFile, pickupArea, srcDlg	-- ***
    -- cursorFile is relative path and filename e.g. "bmp\customer.cur".
    --say "DragManager-setSource-01: sourceWin, Area =" sourceWin||"," pickupArea

    dropOkCursor = .Mouse~loadCursorFromFile(cursorFile)
    if dropOkCursor == 0 then do
      say "DragManager-setSource-02:" .HRSdm~badOkCursor .SystemErrorCode
    end

    items    = .Array~new
    items[1] = mouse;  items[2] = dropOkCursor;
    items[3] = pickupArea;  items[4] = srcDlg
    self~sourceDialogs[sourceWin] = items


  /*----------------------------------------------------------------------------
    RemoveDlg - Removes a dialog from the Targets table when the dialog closes.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD removeDlg PUBLIC
    use strict arg dlg
    -- Check in SourceDlgs:
    self~sourceDialogs~remove(dlg)
    self~targetDialogs~remove(dlg)


  /*----------------------------------------------------------------------------
    dmPickup - Handles drag initiation when the user "picks up" a dialog.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD dmPickup PUBLIC
    expose cursorIsNoDrop dragging dropOkCursor mouse noDropCursor oldCursor overTarget
    use strict arg sourceWin, keyState, mousePos

    --say "DragMgr-dmPickup-00; sourceWin =" sourceWin
    arrItems = self~sourceDialogs[sourceWin]  	-- mouse,srcCursor,pickupArea,sourceDlg
    mouse = arrItems[1]
    dropOkCursor = arrItems[2]
    dragging = .false
--trace i
    if keyState \== 'lButton' | \ mousePos~inRect(arrItems[3]) then return .false
--trace off
    if mouse~dragDetect(mousePos) then do
      --say "DragManager-dmPickup-01: dropOkCursor =" dropOkCursor
      if dropOkCursor == 0 then do
        nop --say "DragManager-dmPickup-02:" .HRSdm~badOkCursor .SystemErrorCode
      end
      --say 'DragManager-dmPickup-03: Detected dragging.'
      mouse~capture()
      oldCursor = mouse~setCursor(noDropCursor)
      cursorIsNoDrop = .true
      dragging       = .true
      validTarget = .false
      overTarget = .false
    end
    --say "DragManager-dmPickup-04: dragging =" dragging
    return


  /*----------------------------------------------------------------------------
    moving - Handles mouse movements - detects when mouse over a dialog, and if
             it's a 'target' dialog, asks it whether it will accept a drop.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD moving PUBLIC
    expose cursorIsNoDrop dragging dropOkCursor mouse noDropCursor overTarget dropTarget
    use arg sourceWin, sourceDlg, keyState, mousePos
    -- overTarget - true when the cursor is over a potential target dialog,
    --   regardless of whether the target refuses a drop or not.
    -- sourceWin is the window (dialog or control) that has captured the mouse.
    -- Note: new APIs introduced in ooDialog Build 7449
    --say "DragManager-moving-00; sourceDlg, sourceWin, dragging =" sourceDlg||"," sourceWin||"," dragging
    if \dragging then return
    --say "DragManager-moving-00A: Flags:" cursorIsNoDrop dragging overTarget
    -- Find out if mouse is over a target dialog:
    targetDlg = 0
    do i over self~targetDialogs		-- items: 1 = hwnd, 2 = droparea
      targetHwnd = self~targetDialogs[i][1]
      droparea   = self~targetDialogs[i][2]
      --say "DragMgr-moving-00B: targetHwnd, dropArea =" targetHwnd||"," dropArea
      -- Prevent showing drop when over a target that is under another window:
      screenPt = mousePos~copy()			-- Point in source dialog (i.e. top left of dlg is 0,0)
      --say "DragManager-moving-01: screenPt =" screenPt
      sourceWin~client2screen(screenPt)
      --sourceDlg~client2screen(screenPt)			-- screenPt is set to screen position (i.e. top left of screen is 0,0)
      --say "DragManager-moving-02: screenPt =" screenPt
      screenHwnd = .DlgUtil~windowFromPoint(screenPt)	-- Get hwd of topmost window that mouse is over
      --say "DragManager-moving-03: screenHwnd =" screenHwnd

      pDlg = mousePos~copy
      --sourceDlg~mapWindowPoints(targetHwnd, pDlg)	-- MousePos relative to targetHwnd
      sourceWin~mapWindowPoints(targetHwnd, pDlg)	-- MousePos relative to targetHwnd
      --say "DragManager-moving-04: pDlg =" pDlg
      childHwnd = i~childWindowFromPoint(pDlg)		-- Get the hwnd visible under mouse pointer
      --say "DragManager-moving-05: childHwnd, pDlg =" childHwnd||"." pDlg

      if screenHwnd == childHwnd then do
      	p1 = mousePos~copy()
      	--sourceDlg~mapWindowPoints(targetHwnd,p1)
      	sourceWin~mapWindowPoints(targetHwnd,p1)
      	--say "DragManager-moving-06: p1 =" p1
        if p1~inRect(droparea) then do
          targetDlg = i
        -- next two statements for debug only
          --tgtNumber = i~queryNumber
          --say "DragManager-moving-07: Target found - hwnd =" targetHwnd tgtNumber
          leave					-- Stop looping - target found!
        end
      end
    end
    --say "DragManager-moving-060."
    if targetDlg \= 0 then do			-- If we're over a target
      if targetDlg = sourceDlg then return	-- If target is also the source
      	      					--   then it's not a target (in this version!).
      if \overTarget then do			-- if first time over target

        --Get the model class for the source View:
        objectMgr = .local~my.ObjectMgr
        sourceClassName = objectMgr~modelClassFromView(sourceDlg)
        targetClassName = objectMgr~modelClassFromView(targetDlg)
        --say "DragMgr-moving-06a: source Class =" sourceClassName
        --say "DragMgr-moving-06b: target Class =" targetClassName
        --parse value a~class with . className .
        interpret "r = ."||targetClassName||"~dmQueryDrop("||sourceClassName||")"
        --say "DragManager-moving-07a: first time - queryDrop returned" r
        if r = .true then do			-- if target accepts a drop
          validTarget = .true
          dropTarget = targetDlg		-- for drop method ...
        end
        else do					-- if target refuses drop
          validTarget = .false
          overTarget = .true
          dropTarget = .false
          return
        end
        -- It's a valid target!
        overTarget = .true
        mouse~setCursor(dropOkCursor)
        cursorIsNoDrop = .false
      end
    end
    else do					-- Not over a target
      if overTarget then do			-- Mouse has just come off target
        validTarget = .false
        overTarget = .false
        mouse~setCursor(noDropCursor)
        cursorIsNoDrop = .true
        dropTargetDlg = 0
      end
    end



  /*----------------------------------------------------------------------------
    dmDrop - Handles things when the user drops onto a target.
             Invoked by "View" (the superclass).
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD dmDrop PUBLIC
    expose cursorIsNoDrop dragging mouse oldCursor dropTarget
    use strict arg sourceDlg, keyState, mousePos
    --say "DragManager-dmDrop-01: the mouse is at ("mousePos~x", "mousePos~y")"
    --say "DragManager-dmDrop-02: cursorIsNoDrop =" cursorIsNoDrop

    if dragging then do
      okayToDrop = (cursorIsNoDrop \== .true)	-- if cind = .false then okToDrop is true;
              					-- if cind = .true then okToDrop is false
      dragging = .false
      cursorIsNoDrop = .false
      mouse~releaseCapture()
      mouse~setCursor(oldCursor)
      --say "DragManager-dmDrop-02a: mouse released; old cursor set."
      -- Jiggle the mouse so it is redrawn immediately (OS: seems to make no diff.)
      p = mouse~getCursorPos; p~incr; mouse~setCursorPos(p)

      if okayToDrop then do
      	--say "DragManager-dmDrop-03: sourceDlg =" sourceDlg
--        objectMgr = .local~my.ObjectMgr
--        sourceClassName = objectMgr~modelClassFromView(sourceDlg)
--        targetClassName = objectMgr~modelClassFromView(targetDlg)
        objectMgr = .local~my.ObjectMgr
        sourceModelId = objectMgr~modelIdFromView(sourceDlg)
        dropTarget~dmDrop(sourceModelId, sourceDlg)
        --say "DragManager-dmDrop-04: Drop Happened OK!!"
      end
      else nop --say "DragManager-dmDrop-05: Drop Did Not Occur."
    end

    return


  /*----------------------------------------------------------------------------
    list - A debug mehod that lists source and target dialogs on the console.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD list PUBLIC		-- Debug method
    say; say "======================================="
    say "DragManager Tables"
    say "-------------------------------------"
    say "Sources:"
    --say "asking dlg =" dlg
    do i over self~SourceDialogs
      say "i =" i
      items = self~SourceDialogs[i]
      say "mouse  = " items[1]
      say "cursor = " items[2]
      say "area   = " items[3]
      say "dialog = " items[4]
    end
    say "-------------------------------------"
    say "Targets:"
    arr = self~targetDialogs[dlg]
    do i over self~targetDialogs
      say "i =" i
      items = self~TargetDialogs[i]
      say "hwnd     = " items[1]
      say "dropArea = " items[2]
    end
    say "======================================="; say

/*============================================================================*/


/*==============================================================================
  Human-Readable Strings (HRSdm)				  v00-01 23Jan12
  --------
   The HRSdm class provides constant character strings for user-visible messages.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS HRSdm PRIVATE		-- Human-Readable Strings
  ::CONSTANT badNoDropCursor  "Error loading NoDrop Cursor, sys error code:"
  ::CONSTANT badOkCursor  "Error loading DropOK Cursor, sys error code:"
