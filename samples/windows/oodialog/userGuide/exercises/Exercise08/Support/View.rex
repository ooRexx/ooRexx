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
/* ooDialog User Guide - Support
   Exercise 08: View.rex

   ViewMixin							  v01-02 18Jun13
   ---------
   A mixin superclass for View components (part of the Model-View Framework).

   Contains: 	   class: "ViewMixin"

   Description: A mixin superclass for all xxxView components.

   Pre-requisites: MVF.

   Outstanding Problems: None reported.

   Changes:
     v01-00 12May13: First Version.
     v01-01 06Jun13: Added drag/drop methods. Also store model id as an
                     attribute ('myModel') to save subclases having to do it.
                     Note - some drag/drop methods are there as catch-alls for
                     when a subclass does not implement them. Default action
                     still to be verified.
            18Jun13: Changed "drop" method to "dmDrop" and "pickup" to "dmPickup"
                     (both sent to DragMgr).

------------------------------------------------------------------------------*/

::REQUIRES "ooDialog.cls"
::REQUIRES "support\Component"

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ViewMixin						  	  v01-00 13May13
  ---------
  A mixin superclass for View classes.  the Order Management dialog. Handles interest registration,
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

--::CLASS View SUBCLASS RcDialog PUBLIC
::CLASS View PUBLIC MIXINCLASS PlainBaseDialog

  ::ATTRIBUTE viewMgr
  ::ATTRIBUTE objectMgr
  ::ATTRIBUTE dragMgr
  ::ATTRIBUTE myModel

  /*----------------------------------------------------------------------------
    initView - initialises the mixin instance - invoked from ???
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    --::method init  -- Results in hang!
  ::METHOD initView
    --say "View-initView-01."
    self~objectMgr = .local~my.ObjectMgr	-- Needed to clear up when dialog closed.
    self~viewMgr = .local~myViewMgr
    -- Direct Manipulation:
    self~dragMgr = .local~my.DragMgr
    --say "View-initView-01: dragMgr =" self~dragMgr

    return
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    activate - must be invoked by subclass.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    expose viewClass viewInstance		-- needed for tidy-up on close.
    use arg modelId
    -- Store model id for use by subclasses:
    self~myModel = modelId
    --say "View-activate-01: self =" self
    -- Get View Instance name and View Class for tidy-up when dialog is closed.
    viewInstance = self~identityHash
    dlgName = self~objectName
    parse var dlgName . viewClass
    modelData = modelId~query
    --say "View-activate-02."
    return modelData
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    showModel - forwards to ObjectMgrloadList - for invocation by subclasses.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showModel
    use strict arg  modelClass, modelInstance, parentDlg
    r = self~ObjectMgr~showModel(modelClass, modelInstance, parentDlg)
    if r = .false then say "View-showModel - ObjectMgr returned .false."
    return r
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    loadList - must be invoked by subclass.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  --::METHOD loadList Wait till check out how do ShowModel for List.
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    leaving - invoked by ooDialog when a dialog closes.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD leaving UNGUARDED
    expose viewClass viewInstance
    --say "View-leaving-01. objectMgr =" objectMgr
    self~objectMgr~removeView(viewClass, viewInstance)
    self~dragMgr~removeDlg(self) 		-- closing, so tell DragManager
    -- Note - we do not remove the Model. Should we? If so, not from here!
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    Popup Offsets
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    setOffsetParent - set the parent dialog id for later offsetting of a child
                      dialog.
         **** Note: This method not used in Exercise07. ****                  */
  ::METHOD setOffsetParent
    use strict arg parentDlg
    viewMgr = .local~my.ViewMgr
    viewMgr~parentOffsetDlg = self

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    offset - offsets a "child" dialog from its "parent" dialog (i.e. the dialog
             from which the child is "popped up").
         **** Note: This method not used in Exercise07. ****                  */
  ::METHOD offset
    --say "RcView-offset-1."
    offset    = .local~my.ViewMgr~dlgOffset
    parentDlg = .local~my.ViewMgr~parentOffsetDlg
    popupPos  = parentDlg~getRealPos
    popupPos~incr(offset,offset)
    self~moveTo(popupPos, "SHOWWINDOW")
    self~ensureVisible()

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    initDialog - invokes offset.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    say "ViewMixin-initDialog-01."
    self~offset
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    Drag/Drop Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    dmSetAsSource - called by a view component to define itself as a drag source.
		    Typically invoked from subclass' initDialog method.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD dmSetAsSource PRIVATE		-- DM setup method
    -- Each source dialog should invoke this only once.
    -- Invoking it more than once may well result in errors.
    -- Note: a dialog may be both source and target.
    expose mouse dmSourceControl sourceWin
    use arg dmSourceCursorFile, dmSourceArea, dmSourceControl
    if dmSourceCursorFile = .nil then do
      say "View-dmSetAsSource-00:" .HRS~dmSrcNulCursor
      return .false
    end
    --say "View-dmSetAsSource-01: dmSourceCursorFile, dmSourceArea, dmSourceControl:"
    --say "   '"||dmSourceCursorFile||"', "||dmSourceArea", "||dmSourceControl

    if dmSourceArea = "DMSOURCEAREA" then do		-- set default pickup area
      dmSourceArea = self~clientRect()
      dmSourceArea~left += 10; dmSourceArea~top += 10; -
        dmSourceArea~right -= 10; dmSourceArea~bottom -= 10
      --say "View-dmSetAsSource-02 - default pickup client area =" dmSourceArea
    end
    --else say "View-dmSetAsSource-03 - pickup client area =" dmSourceArea

    if dmSourceControl = "DMSOURCECONTROL" then do 	-- The source is a dialog.
      --say "View-dmSetAsSource-04: source is a dialog."
      sourceWin = self
      mouse = .Mouse~new(sourceWin)
      mouse~connectEvent('MOUSEMOVE',dmOnMove)
      mouse~connectEvent('LBUTTONDOWN', dmOnLBdown)
      mouse~connectEvent('LBUTTONUP', dmOnLBup)
      --self~dragMgr~setSource(self, mouse, dmSourceCursorFile, dmSourceArea, .nil)
      self~dragMgr~setSource(self, mouse, dmSourceCursorFile, dmSourceArea, self)  -- ***
    end
    else do					        -- The source is a control (such as a ListView).
      --say "View-dmSetAsSource-05: source is a control."
      sourceWin = dmSourceControl
      mouse = .Mouse~new(dmSourceControl)
      mouse~connectEvent('MOUSEMOVE',dmOnMove)
      mouse~connectEvent('LBUTTONDOWN', dmOnLBdown)
      mouse~connectEvent('LBUTTONUP', dmOnLBup)
      --self~dragMgr~setSource(self, mouse, dmSourceCursorFile, dmSourceArea, dmSourceControl)
      self~dragMgr~setSource(sourceWin, mouse, dmSourceCursorFile, dmSourceArea, self)  -- ***
    end
    --mouse~connectEvent('MOUSELEAVE', dmLeave)
    --say "View-dmSetAsSource-06: source is:" sourceWin


    return .true

  /*----------------------------------------------------------------------------
    dmSetAsTarget - called by a view component to define itself as a drag target.
		    Typically invoked from subclass' initDialog method.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD dmSetAsTarget				-- DM setup method
    -- Each target dialog should invoke this only once.
    -- Invoking it more than once may well result in errors.
    -- Note: a dialog may be both source and target.
    expose dmIsTargetDlg
    use arg dmDropArea
    --say "View-dmSetAsTarget-01."

    if dmDropArea = "DMDROPAREA" then do		-- set default. Better is to check the type.
      dmDropArea = self~clientRect()
      dmDropArea~left += 10; dmDropArea~top += 10; dmDropArea~right -= 30; dmDropArea~bottom -= 30
    end

    self~dragMgr~setTarget(self, self~hwnd, dmDropArea)

    return .true

  -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ::METHOD dmOnLBdown
    expose sourceWin
    use arg keyState, mousePos
    --say "View-dmOnLBdown-00; self, keystate, mousePos =" self||"," keystate||"," mousePos
    info = self~dmGetItemInfo				-- for listviews
    if info = 0 then nop --say "View-dmOnLBdown-01 - info is zero."

    else do
      nop --say "View-dmOnLBdown-02; info, sourceWin =" info||"," sourceWin
      -- store the info somewhere - how about "drag data"?.
      -- Drag data = classname, instance name.
    end
    --self~DragMgr~dmPickup(self, keyState, mousePos, dragData) - not right yet
    self~DragMgr~dmPickup(sourceWin, keyState, mousePos)  -- pre-listview
    return 0

  ::METHOD dmGetItemInfo	-- Dummy method for when sublcass does not
                                -- implement it.
    return 0


  -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ::METHOD dmOnMove
    expose sourceWin
    use arg keyState, mousePos
    --say "View-dmOnMove: self, sourceWin =" self||"," sourceWin
    self~dragMgr~moving(sourceWin, self, keystate, mousePos)
    --say "View-dmOnMove"
    return 0

  -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ::METHOD dmOnLBup
    use arg keyState, mousePos
    --say "View-dmOnLBup-01; self =" self
    self~dragMgr~dmDrop(self, keyState, mousePos)
    --return r -- throws error, as done no return at all.
    return 0
    say 'DMSource-onLButtonUp: the mouse is at ('mousePos~x',' mousePos~y') with these qualifiers:' keyState

  -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/*  ::METHOD dmNeverDrop
    -- Invoked by a target object to prevent any drop (e.g. if a sales order is
    -- complete and should not now be altered). The red/white target icon is
    -- changed to grey.
    expose dmTargetIconImage dmIcons dmTargetInactive
    use arg dmTargetInactive
    if dmTargetInactive then dmTargetIconImage~setImage(dmIcons[dmTgtInactiveIcon])
    else dmTargetIconImage~setImage(dmIcons[dmTgtReadyIcon])
*/

  ::METHOD dmQueryDrop
    use arg dmSourceDlg, mousePos
    --say "View-dmQueryDrop-01."
    return .true			-- Default is to accept the drop.

  ::METHOD dmDrop
    use arg sourceDlg
    --say "View-dmDrop-01."
    return .true

/*  ::METHOD cancel
    expose dmDragMgr
    say "View-Cancel-01."
    dmDragMgr~removeDlg(self) 		-- closing, so tell DragManager
    return self~cancel:super
*/
/*
  ::METHOD ok
    say "View-ok-01."
    self~dragMgr~removeDlg(self) 		-- closing, so tell DragManager
    return self~ok:super
*/


  /*----------------------------------------------------------------------------
    Event Management Methods. *** INCOMPLETE ***
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  --::METHOD triggerEvent
  --  use strict arg event
  --  idEventMgr = .local~my.EventMgr

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD viewDoIt
    --say "View-viewDoIt-01."


/*============================================================================*/



/*==============================================================================
  Human-Readable Strings (HRS)					  v00-01 13Jan12
  --------
   The HRS class provides constant character strings for user-visible messages.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS HRS PRIVATE		-- Human-Readable Strings
  ::CONSTANT dmSrcNulCursor  "View-dmSetAsSource - Error: Source Cursor is null."
  ::CONSTANT dmTgtBadParam   "View-dmSetAsTarget - Error: null dlg or null hwnd or both."


