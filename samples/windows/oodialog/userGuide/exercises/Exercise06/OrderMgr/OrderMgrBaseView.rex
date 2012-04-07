/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2012 Rexx Language Association. All rights reserved.    */
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
/* ooDialog User Guide
   Exercise 06: The OrderManagementBaseView class
   OrderMgrBaseView.rex						  v00-08 19Feb12

   Contains: classes "OrderMgrBaseBase", HRSombv (private).

   Pre-requisite files: OrderMgrBaseView.rc, OrderMgrBaseView.h.

   Changes:
     v00-01 22Aug11: First version.
     v00-02 28Sep11: Add OrderList icon (a bitmap).
     v00-03 29Sep11: Corrected wrong window size on open.
     v00-04 03Oct11: Re-factor code to move all app function (including ListView
                     setup) to the OrderMgrView sublcass. No function/appearance
                     change.
     v00-05 28Jan12: Changed class name HRS to HRSombv to allow for multiple
     		     HRS classes in same file at some future time.
     v00-06 15Feb12: Changes to comments only.
     v00-07 11Feb12: Add .application~setDefaults to this file.
     v00-08 19Feb12: OrderMgrBaseView: moved .Application~ stmt to top of file.


   To Do: - Add Find Customer, Find Product (buttons or menu items?)
          - Tidy up comments in code.

   Possible future additions:
          - A configure option to allow user to decide whether to use buttons or
            menus for find Customer/Product/Order (this could illustrate dynamic
            adding of menu items, buttons, icons.)

------------------------------------------------------------------------------*/


.Application~addToConstDir("OrderMgr\OrderMgrBaseView.h")


::REQUIRES "ooDialog.cls"

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderMgrBaseView						  v00-07 19Feb12
  -----------------
  The base "view" (or "gui") part of the OrderMgr component (part of the
  sample Order Management application). This class provides for (a) handling
  of a re-sizeable dilaog, and also for providing the icons for the main control
  in the dialog which is a ListView control.

   Changes:
   v00-06 11Feb12: Moved .application~setDefaults() to app startup file.
                   changed to .application~addToConstDir() here.
   v00-07 19Feb12: Moved .Application~addToConstDir statement from newInstance
                   method to top of file - just before ::requires statement(s).


  interface iOrderMgrBaseView {
    void newInstance();
  }
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderMgrBaseView SUBCLASS UserDialog PUBLIC

  ::ATTRIBUTE lv PRIVATE	-- The ListView that contains the icons.

  /*----------------------------------------------------------------------------
    Class Methods
  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC UNGUARDED
    --say ".OrderMgrBaseView-newInstance-01: Start."
    -- Enable use of symbolic IDs in menu creation, and turn off AutoDetection
    -- (the third parameter:
    -- Create an instance of OrderMgrBaseView and show it:
    dlg = self~new

    --say ".OrderMgrBaseView-newInstance-02: dlg~Activate."
    dlg~activate


  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD init
    expose menuBar records

    forward class (super) continue

    success = self~createCenter(310, 220, .HRSombv~WindowTitle,     -
                                'ThickFrame MinimizeBox MaximizeBox', , -
                                'MS Sans Serif', 8)
    if \ success then do
      self~initCode = 1
      return
    end

    self~connectResize('onResize', .true)
    self~connectResizing('onResizing')
    self~connectSizeMoveEnded('onSizeMoveEnded')

    menuBar = .ScriptMenuBar~new("OrderMgr\OrderMgrBaseView.rc", IDR_ORDMGR_MENU, , , .true)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD defineDialog
    expose u sizing minMaximized

    u = .dlgAreaU~new(self)  --creates a dialog area coterminous with the calling dialog. It is a subclass of the
  			   --DlgArea class and therefore inherits all the methods
    if u~lastError \= .nil then call errorDialog u~lastError

    -- Tell the DialogAreaU object to not invoke the update method.
    u~updateOnResize = .false

  -- We use these variables to track when to redraw, or not.
    sizing = .false
    minMaximized = .false

    --Add controls to the set of controls that won't be resized on a resize event.
    u~noResizePut('IDC_ORDMGR_EXIT')
    u~noResizePut('IDC_ORDMGR_RESET')

    e = .dlgArea~new(u~x       , u~y       , u~w,        u~h('90%'))   -- ListView area
    s = .dlgArea~new(u~x       , u~y('90%'), u~w,        u~hr      )   -- Button Area (only one button)

    -- Note - in the following, use of single or double quotes around the symbolic ID produces a "could not parse" error message.
    self~createListView(IDC_ORDMGR_ICONS,   e~x,       e~y, e~w,        e~h,        "ICON")
    self~createPushButton(IDC_ORDMGR_RESET, s~x('0%'), s~y, s~w('20%'), s~h('95%'), ,.HRSombv~Reset, resetIcons)
    self~createPushButton(IDC_ORDMGR_EXIT,  s~x(-60),  s~y, s~w('20%'), s~h('95%'), ,.HRSombv~ExitApp, exitApp)

    self~connectListViewEvent(IDC_ORDMGR_ICONS, "ACTIVATE", "onDoubleClick")
    -- Following line required to allow icons to be dragged around the listview.
    self~connectListViewEvent(IDC_ORDMGR_ICONS, "BEGINDRAG", "DefListDragHandler")

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    --say "OrderMgrBaseView-activate."
    self~execute('SHOWTOP') -- Try showing dialog at end of initDialog to get sizing right.
    --self~execute("HIDE")

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose minWidth minHeight iconList records menuBar u lastSizeInfo sizing
    --say "OrderMgrBaseView-initDialog."
    menuBar~attachTo(self)

    -- Create a proxy for the List View and store in instance variable 'lv'.
    self~lv = self~newListView(IDC_ORDMGR_ICONS)

    -- Any underlying edit controls internally resize themselves as the dialog
    -- they are contained in is resized.  We don't want that, so we disable that
    -- behavior in any underlying edit control as follows (e.g):
    --      self~newEdit(IDC_EDIT)~disableInternalResize
    -- (But no edit controls in OrderMgr View.)

    -- Restrict the minimum width and minimum height to the original
    -- width and height
    minWidth = self~pixelCX
    minHeight = self~pixelCY

    -- Make sure the height of the menubar is taken into account (DlgAreaU does
    -- not take this into account so have to fix things manually here.)
    -- (1) Get the height of a single line menu bar in pixels ("SM_CYMENU" is one
    -- of the many MSDN System Metrics; it has a value of 15.)
    SM_CYMENU = 15
    height = .DlgUtil~getSystemMetrics(SM_CYMENU)
    -- Get the actual size of the dialog in pixels.
    s = self~getRealSize
    lastSizeInfo = .DlgUtil~makeLParam(s~width, s~height)
    sizing = .true
    s~height += height
    self~resizeTo(s)
    self~onSizeMoveEnded


  /*----------------------------------------------------------------------------
    Sizing Control Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  -- The RESIZING event happens when the user is resizing the dialog, but *before*
  -- the size of the dialog is actually changed.  The first arg is a .RECT object
  -- that describes the new size.  If we change the coordinates in the rectangle,
  -- and reply .true.  The new size of the dialog will be our changed coordinates.
  --
  -- Here, if the new size is smaller than we want, we just change the rectangle
  -- coordinates to our minimum size.  The 'direction' argument tells us which
  -- edge of the dialog the user is dragging.  We use that to decide which
  -- coordinate(s) to change.
  --
  -- A maximum size for the dialog could be enforced in a similar way.
::METHOD onResizing UNGUARDED
  expose minWidth minHeight
  use arg r, direction

  select
    when direction == 'TOP' then do
      if r~bottom - r~top < minHeight then do
        r~top = r~bottom - minHeight
        --say "r~top:" r~top "   r~bottom:" r~bottom
        return .true
      end
    end
    when direction == 'BOTTOM' then do
      if r~bottom - r~top < minHeight then do
        r~bottom = r~top + minHeight
        return .true
      end
    end
    when direction == 'LEFT' then do
      if r~right - r~left < minWidth then do
        r~left = r~right - minWidth
        return .true
      end
    end
    when direction == 'RIGHT' then do
      if r~right - r~left < minWidth then do
        r~right = r~left + minWidth
        return .true
      end
    end
    when direction == 'BOTTOMLEFT' then do
      didChange = .false

      if r~bottom - r~top < minHeight then do
        r~bottom = r~top + minHeight
        didChange = .true
      end

      if r~right - r~left < minWidth then do
        r~left = r~right - minWidth
        didChange = .true
      end

      return didChange
    end
    when direction == 'BOTTOMRIGHT' then do
      didChange = .false

      if r~bottom - r~top < minHeight then do
        r~bottom = r~top + minHeight
        didChange = .true
      end

      if r~right - r~left < minWidth then do
        r~right = r~left + minWidth
        didChange = .true
      end

      return didChange
    end
    when direction == 'TOPLEFT' then do
      didChange = .false

      if r~bottom - r~top < minHeight then do
        r~top = r~bottom - minHeight
        didChange = .true
      end

      if r~right - r~left < minWidth then do
        r~left = r~right - minWidth
        didChange = .true
      end

      return didChange
    end
    when direction == 'TOPRIGHT' then do
      didChange = .false

      if r~bottom - r~top < minHeight then do
        r~top = r~bottom - minHeight
        didChange = .true
      end

      if r~right - r~left < minWidth then do
        r~right = r~left + minWidth
        didChange = .true
      end

      return didChange
    end
    otherwise
      nop
  end
  -- End select
  return .false


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD onResize unguarded
  expose u sizing minMaximized lastSizeInfo
  use arg sizingType, sizeinfo
  --say "OrderMgrBaseView-onResize."
  --os - this methed sent while re-sizing.
  -- Save the size information so we know the final size of the dialog.
  lastSizeInfo = sizeInfo

  -- The size / move ended event does not occur when the user maximizes,
  -- minimizes, or restores from maximized / minimized.  Because of that, we
  -- need to redraw the client area under those conditions.

  if sizingType == self~SIZE_MAXIMIZED | sizingType == self~SIZE_MINIMIZED then do
    minMaximized = .true
    if sizingType == self~SIZE_MAXIMIZED then do
      u~resize(self, sizeinfo)
      self~redrawClient(.true)
    end
  end
  else if sizingType == self~SIZE_RESTORED, minMaximized then do
    minMaximized = .false
    u~resize(self, sizeinfo)
    self~redrawClient(.true)
  end
  else do
    -- We are resizing now.
    sizing = .true
  end

  return 0


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  -- This event is raised when only move the window.
  -- Also, first move get sizing .true, all after that get sizing .false.
  -- This because get "onResize" when dlg opens without any user action,
  -- and the onResize method sets sizing to .true.

  ::METHOD onSizeMoveEnded UNGUARDED
    expose u sizing lastSizeInfo
    --say "OrderMgrBaseView-onSizeMoveEnded."
    -- If we were resizing, force the dialog controls to redraw themselves.
    if sizing then do
      u~resize(self, lastSizeInfo)
      self~redrawClient(.true)
    end

    -- We are not resizing anymore.
    sizing = .false
    return 0



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  HRSombv (Human-Readable Strings for OrderMgrViewBase)		  v00-01 21Aug11
  ---
  The OmHRS class provides constant character strings for user-visible messages
  issued by the OrderMgrBaseView class.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


::CLASS HRSombv PRIVATE	  -- Human-Readable Strings
  ::CONSTANT WindowTitle  "Sales Order Management"	-- Dialog Caption
  ::CONSTANT Reset        "Reset Icons"			-- PushButton
  ::CONSTANT ExitApp      "Exit Application"		-- PushButton
/*============================================================================*/
