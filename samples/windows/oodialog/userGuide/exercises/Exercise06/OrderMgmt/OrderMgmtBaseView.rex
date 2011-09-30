/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2011 Rexx Language Association. All rights reserved.    */
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
   OrderMgmtBaseView.rex					  v00-03 30Sep11

   Contains: classes "OrderMgmtBaseBase", HRS (private).

   Pre-requisite files: OrderManagementBaseView.rc, OrderManagementBaseView.h.

   Changes:
     v00-01 22Aug11: First version.
     v00-02 28Sep11: Add OrderList icon (a bitmap).
     v00-03 29Sep11: Corrected wrong window size on open.

   To Do: - Fix close by system (top right icon on window) - should bring up
            "are you sure" msg.
          - Fix no-warning close when hit enter.
          - Add Find Customer, Find Product (buttons or menu items?)
          - Tidy up comments in code.

   Possible future additions:
          - A configure option to allow user to decide whether to use buttons or
            menus for find Customer/Product/Order (this could illustrate dynamic
            adding of menu items, buttons, icons.)

------------------------------------------------------------------------------*/

::requires "ooDialog.cls"
--::requires "CustomerListView.rex"

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderMgmtBaseView						  v00-01 22Aug11
  -----------------
  The base "view" (or "gui") part of the OrderManagement component (part of the
  sample Order Management application). This class provides for (a) handling
  of a re-sizeable dilaog, and also for providing the icons for the main control
  in the dialog which is a ListView control.

  interface iOrderManagementBaseView {
    void newInstance();
  }
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderMgmtBaseView SUBCLASS UserDialog PUBLIC

  ::ATTRIBUTE lv PRIVATE	-- The ListView that contains the icons.

  /*----------------------------------------------------------------------------
    Class Methods
  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC UNGUARDED
    say ".OrderMgmtBaseView-newInstance-01: Start."
    -- Enable use of symbolic IDs in menu creation, and turn off AutoDetection
    -- (the third parameter:
    .Application~setDefaults("O", "OrderMgmt\OrderMgmtBaseView.h", .false)
    -- Create an instance of OrderMgmtBaseView and show it:
    dlg = self~new

    say ".OrderMgmtBaseView-newInstance-02: dlg~Activate."
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

    success = self~createCenter(310, 220, .HRS~omWindowTitle,     -
                                'ThickFrame MinimizeBox MaximizeBox', , -
                                'MS Sans Serif', 8)
    if \ success then do
      self~initCode = 1
      return
    end

    self~createImageLists
    records = self~initRecords

    self~connectResize('onResize')
    self~connectResizing('onResizing')
    self~connectSizeMoveEnded('onSizeMoveEnded')

    menuBar = .ScriptMenuBar~new("OrderMgmt\OrderMgmtBaseView.rc", IDR_ORDMGMT_MENU, , , .true, self)



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
    u~noResizePut('IDC_ORDMGMT_EXIT')
    u~noResizePut('IDC_ORDMGMT_RESET')

    e = .dlgArea~new(u~x       , u~y       , u~w,        u~h('90%'))   -- ListView area
    s = .dlgArea~new(u~x       , u~y('90%'), u~w,        u~hr      )   -- Button Area (only one button)

    -- Note - in the following, use of single or double quotes around the symbolic ID produces a "could not parse" error message.
    self~createListView(IDC_ORDMGMT_ICONS,   e~x,       e~y, e~w,        e~h,        'icon')
    self~createPushButton(IDC_ORDMGMT_RESET, s~x('0%'), s~y, s~w('20%'), s~h('95%'), ,.HRS~omReset, resetIcons)
    self~createPushButton(IDC_ORDMGMT_EXIT,  s~x(-60),  s~y, s~w('20%'), s~h('95%'), ,.HRS~omExit, exitApp)  -- Works, but button moves back/forward on resize.

    self~connectListViewEvent(IDC_ORDMGMT_ICONS, "ACTIVATE", "onDoubleClick")
    -- Following line requires to allow icons to be dragged around the listview.
    self~connectListViewEvent(IDC_ORDMGMT_ICONS, "BEGINDRAG", "DefListDragHandler")


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    say "OrderMgmtBaseView-activate."
    self~execute('SHOWTOP') -- Try showing dialog at end of initDialog to get sizing right.
    --self~execute("HIDE")

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose minWidth minHeight normalIcons records menuBar u lastSizeInfo sizing --lv
    say "OrderMgmtBaseView-initDialog."
    menuBar~attachTo(self)

    -- Create a proxy for the List View and set icons into it.
    self~lv = self~newListView(IDC_ORDMGMT_ICONS)
    self~lv~setImageList(normalIcons, .Image~toID(LVSIL_NORMAL))
    -- Add icons (i.e. records) to the ListView:
    do i=1 to records~items
      self~lv~addRow(, i-1, records[i]~name)
    end

    -- Any underlying edit controls internally resize themselves as the dialog
    -- they are contained in is resized.  We don't want that, so we disable that
    -- behavior in the underlying edit control as follows:
    --      self~newEdit(IDC_EDIT)~disableInternalResize
    -- But no edit controls in Ordermanagement View.

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



  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  -- The RESIZING event happens when the user is resizing the dialog, but (os: the first one) *before*
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
  --os say "Direction:" direction

  select
    when direction == 'TOP' then do
      if r~bottom - r~top < minHeight then do
        r~top = r~bottom - minHeight
        say "r~top:" r~top "   r~bottom:" r~bottom
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
  say "OrderMgmtBaseView-onResize."
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
    say "OrderMgmtBaseView-onSizeMoveEnded."
    -- If we were resizing, force the dialog controls to redraw themselves.
    if sizing then do
      u~resize(self, lastSizeInfo)
      self~redrawClient(.true)
    end

    -- We are not resizing anymore.
    sizing = .false
    return 0



  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - --
    Icon (ListView contents) Methods:
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD createImageLists private
    expose normalIcons
    imgCustList  = .Image~getImage("customer\bmp\CustList.bmp")
    imgProdList  = .Image~getImage("product\res\ProdList.bmp")
    imgOrderList = .Image~getImage("order\bmp\OrderList.bmp")
    imgOrderForm = .Image~getImage("order\bmp\OrderForm.bmp")
    tmpIL = .ImageList~create(.Size~new(64, 64), .Image~toID(ILC_COLOR4), 4, 0)
    if \imgCustList~isNull,  \tmpIL~isNull then do  -- check for errors in images/imagelist
      tmpIL~add(imgCustList)   -- item 0 in the list
      tmpIL~add(imgProdList)   -- item 1 in the list
      tmpIL~add(imgOrderList)  -- item 2 in the list
      tmpIL~add(imgOrderForm)  -- item 3 in the list
      imgCustList~release
      imgProdList~release
      imgOrderList~release
      imgOrderForm~release
      normalIcons = tmpIL
    end
    else do
      normalIcons = .nil
    end
    return



  -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ::METHOD exitApp UNGUARDED
    self~cancel

  ::METHOD cancel
    response = askDialog(.HRS~omQExit, "N")
    if response = 1 then forward class (super)

  ::METHOD ok
    -- Invoked when enter key pressed - if passed to superclass, cancels dialog.
    return  -- do not close dialog - appears as a no-op to the user.


  -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ::METHOD resetIcons
    expose lv
    say "Reset Icons."
    r = lv~arrange
    say "r =" r



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  HRS (Human-Readable Strings for OrderMgmtViewBase)		  v00-01 21Aug11
  ---
  The HRS class provides constant character strings for user-visible messages
  issued by the OrderMgmtBaseView class.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


::CLASS HRS PRIVATE		-- Human-Readable Strings
  ::CONSTANT omWindowTitle  "Sales Order Management"
  ::CONSTANT omReset        "Reset Icons"
  ::CONSTANT omExit         "Exit Application"
  ::CONSTANT omQExit        "Are you sure you want to close all windows and exit the application?"
