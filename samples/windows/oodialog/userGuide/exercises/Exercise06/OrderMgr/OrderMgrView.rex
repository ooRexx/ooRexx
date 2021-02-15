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
/* ooDialog User Guide
   Exercise 06: OrderMgrView.rex 				  v01-00 30Jly13

   Contains: 	   class: "OrderMgrView", "HRSomv"

   Description: A sample Order Manager View class - part of the sample
        	Order Manager component.

   Outstanding Problems: None reported.

   Changes:
     v01-00 07Jun12: First Version (ooDialog 4.2.1) - used dlgArea for re-sizing.
            18Jan13: Second Version (ooDialog 4.2.2) - uses ResizingAdmin for
                     re-sizing.
            30Jly14: Corrected .ImageList~create(...) statement in 
                     createIconList method.
------------------------------------------------------------------------------*/

-- Use the global .constDir for symbolic IDs - load them from OrderMgrView.h
.Application~addToConstDir("OrderMgr\OrderMgrView.h")

call "OrderMgr\RequiresList.rex"

::REQUIRES "ooDialog.cls"

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderMgrView							  v01-00 18Jan13
  --------------------
  To the user, this class is the Order Management Application. It provides
  access to the various functions required for managing Sales orders.
  In this Exercise, however, only very basic application function is provided.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderMgrView SUBCLASS RcDialog PUBLIC INHERIT ResizingAdmin

  ::ATTRIBUTE lv PRIVATE	-- The ListView that contains the icons.

  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    say ".OrderMgrView-newInstance-01."
    dlg = .OrderMgrView~new("OrderMgr\OrderMgrView.rc", IDD_ORDMGR)
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
    say "OrderMgrView-init-01."
    menuBar = .ScriptMenuBar~new("OrderMgr\OrderMgrView.rc", IDR_ORDMGR_MENU, , , .true)
    self~createIconList
    records = self~initRecords


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD defineDialog
    say "OrderMgrView-defineDialog-01."


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    say "OrderMgrView-activate-01."
    self~execute("SHOWTOP", IDI_DLG_OOREXX)


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose menuBar records iconList
    say "OrderMgrView-initDialog-01."
    menuBar~attachTo(self)

    -- Create a proxy for the List View and store in instance variable 'lv'.
    self~lv = self~newListView(IDC_ORDMGR_ICONS)

    -- Add the Image List to the ListView:
    self~lv~setImageList(iconList, NORMAL)
    -- Add icons (i.e. records) to the ListView:
    do i=1 to records~items
      self~lv~addRow(, i-1, records[i]~name)
    end

    self~connectListViewEvent(IDC_ORDMGR_ICONS, "ACTIVATE", "onDoubleClick")
    -- Following line required to allow icons to be dragged around the listview.
    self~connectListViewEvent(IDC_ORDMGR_ICONS, "BEGINDRAG", "DefListDragHandler")
    self~connectButtonEvent("IDC_ORDMGR_EXIT", "CLICKED",exitApp)
    self~connectButtonEvent("IDC_ORDMGR_RESET","CLICKED",resetIcons)
    self~setTitle(.HRSomv~WindowTitle)		-- set dialog title.

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD createIconList PRIVATE
    -- This method simulates getting the icon "data" for the OrderMgr view.
    -- The icon data is loaded into 'iconList' which is an 'ImageList' as
    -- required by the ListView control.
    expose iconList
    --say "OrderMgrView-createIconList."
    imgCustList  = .Image~getImage("customer\bmp\CustList.bmp")
    imgProdList  = .Image~getImage("product\res\ProdList.bmp")
    imgOrderList = .Image~getImage("order\bmp\OrderList.bmp")
    imgOrderForm = .Image~getImage("order\bmp\OrderForm.bmp")
    iconList = .ImageList~create(.Size~new(64, 64), .Image~toId(ILC_COLOR4), 4, 0)
    -- Boldly assume no errors in creating the Image List or in the ~getImage statements.
    iconList~add(imgCustList)   -- item 0 in iconList (item 1 in records)
    iconList~add(imgProdList)   -- item 1 in iconList (item 2 in records)
    iconList~add(imgOrderList)  -- item 2 in iconList (item 3 in records)
    iconList~add(imgOrderForm)  -- item 3 in iconList (item 4 in records)
    imgCustList~release
    imgProdList~release
    imgOrderList~release
    imgOrderForm~release
    return

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initRecords PRIVATE
    -- This method simulates getting the "data" for the OrderMgr view.
    expose records
    records = .array~new()

    rec = .directory~new
    rec~ID = "CustomerList"
    rec~name = "Customer List"
    records[1] = rec

    rec = .directory~new
    rec~ID = "ProductList"
    rec~name = "Product List"
    records[2] = rec

    rec = .directory~new
    rec~ID = "OrderList"
    rec~name = "Sales Orders"
    records[3] = rec

    rec = .directory~new
    rec~ID = "OrderForm"
    rec~name = "New Order"
    records[4] = rec

    return records

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



  /*----------------------------------------------------------------------------
    Event-Handler Methods - Menu Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - Orders  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newOrder UNGUARDED
    expose records
    self~showModel(records[4])

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD orderList UNGUARDED
    expose records
    self~showModel(records[3])

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD orderSearch UNGUARDED
    self~noMenuFunction(.HRSomv~OrdSrch)

  /*- - Customers - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD customerList UNGUARDED
    expose records
    self~showModel(records[1])

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD customerSearch UNGUARDED
    self~noMenuFunction(.HRSomv~CustSrch)

  /*- - Products  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD productList
    expose records
    self~showModel(records[2])

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD productSearch UNGUARDED
    self~noMenuFunction(.HRSomv~ProdSrch)

  /*- - New - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD product UNGUARDED
    self~noMenuFunction(.HRSomv~NewProd)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD customer UNGUARDED
    self~noMenuFunction(.HRSomv~NewCust)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD order UNGUARDED
    self~newOrder

  /*- - Help - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    self~noMenuFunction(.HRSomv~HelpAbout)


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD noMenuFunction UNGUARDED
    use arg title
    ret = MessageDialog(.HRSomv~NoMenu, self~hwnd, title, 'WARNING')


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Icon Double-Click
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD onDoubleClick UNGUARDED
    expose records
    --use arg id
    --say "OrderMgrView-onDoubleClick-01."
    -- Get the index of the item with the focus, use the index to retrieve
    -- the item's record:
    index = self~lv~focused		-- lv is an attribute of the superclass.
    record = records[index+1]
    --say "OrderMgrView-onDoubleClick-02: Record ID =" record~ID
    self~showModel(record)


  /*----------------------------------------------------------------------------
    Event-Handler Methods - PushButton Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD resetIcons
    r = self~lv~arrange

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD exitApp UNGUARDED
    --say "OrderMgrView-exitApp-01."
    self~cancel

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD cancel
    --say "OrderMgrView-cancel-01."
    response = askDialog(.HRSomv~QExit, "N")
    if response = 1 then forward class (super)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD ok
    -- Invoked when enter key pressed - if passed to superclass, cancels dialog.
    return  -- do not close dialog - appears as a no-op to the user.


  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showModel UNGUARDED
    /* Surface the view of an icon (i.e. view of the model represented by the icon).
       Ideally, if already instantiated, surface it, else makeInstance.
       In this version, get as many as you like - but all have the same data!.*/
    use arg record				-- record is a directory object.
    className = record~ID
    --say "OrderMgrView-showModel-01: className =" className
    viewClassName = className||"View"
    interpret "."||viewClassName||"~newInstance(self)"
    --say "OrderMgrView-showModel-02:"

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  HRSomv (Human-Readable Strings for OrderMgrView)		  v01-00 07Jun12
  ---
  The HRSomv class provides constant character strings for user-visible messages
  issued by the OrderMgrBaseView class.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


::CLASS HRSomv PRIVATE		-- Human-Readable Strings
  ::CONSTANT QExit        "Are you sure you want to close all windows and exit the application?"
  ::CONSTANT NoMenu       "This menu item is not yet implemented."
  ::CONSTANT OM           "Order Manager"
  ::CONSTANT OrdSrch      "Order Search"
  ::CONSTANT CustSrch     "Customer Search"
  ::CONSTANT ProdSrch     "Product Search"
  ::CONSTANT NewCust      "New Customer"
  ::CONSTANT NewProd      "New Product"
  ::CONSTANT HelpAbout    "Help - About"
  ::CONSTANT WindowTitle  "Sales Order Management"	-- Dialog Caption
  ::CONSTANT Reset        "Reset Icons"			-- PushButton
  ::CONSTANT ExitApp      "Exit Application"		-- PushButton
/*============================================================================*/
