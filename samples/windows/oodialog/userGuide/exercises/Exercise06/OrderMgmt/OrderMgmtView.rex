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
   Exercise 06: OrderMgmtView.rex 				  v00-03 12Oct11

   Contains: 	   class: "OrderMgmtView"

   This is a subclass of OrderMgmtBaseView, and provides only the "application"
   function of Order Management.

   Pre-requisites: Class "OrderMgmtBaseView

   Description: A sample Order Management View clas - part of the sample
        	Order Management component.

   Outstanding Problems: None reported.

   Changes:
   v00-01 21Aug11: First version.
   v00-02 30Sep11: Added OrderForm.
   v00-03 12Oct11: Re-factored - moved all ListView matters (except re-sizing)
                   from OrderMgmtBaseView to here.
                   Add methods for menu items.
------------------------------------------------------------------------------*/

call "OrderMgmt\RequiresList.rex"

::REQUIRES "ooDialog.cls"
::REQUIRES "OrderMgmt\OrderMgmtBaseView.rex"

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderManagementView						  v00-03 12Oct11
  --------------------
  The "application" part of the OrderManagement View" component. This class
  provides for all function except re-sizing and basic setup (OrderMgmtBaseView
  has the .h file and the .rc file for the menu).

  interface iOrderManagementView {
    void newInstance();
  }
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderMgmtView SUBCLASS OrderMgmtBaseView PUBLIC

  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD init
    expose records
    say "OrderMgmtView-init."
    self~init:super
    self~createIconList
    records = self~initRecords

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD createIconList PRIVATE
    -- This method simulates getting the icon "data" for the OrderManagement view.
    -- The icon data is loaded into 'iconList' which is an 'ImageList' as
    -- required by the ListView control.
    expose iconList
    say "OrdermgmtView-createIconList."
    --trace i
    imgCustList  = .Image~getImage("customer\bmp\CustList.bmp")
    imgProdList  = .Image~getImage("product\res\ProdList.bmp")
    imgOrderList = .Image~getImage("order\bmp\OrderList.bmp")
    imgOrderForm = .Image~getImage("order\bmp\OrderForm.bmp")
    iconList = .ImageList~create(.Size~new(64, 64), .Image~toID(ILC_COLOR4), 4, 0)
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
    -- This method simulates getting the "data" for the OrderManagement view.
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
  ::METHOD initDialog
    expose records iconList
    say "OrderMgmtView-initDialog."
    self~initDialog:super
    -- Add the Image List to the ListView:
    self~lv~setImageList(iconList, .Image~toID(LVSIL_NORMAL))
    -- Add icons (i.e. records) to the ListView:
    do i=1 to records~items
      self~lv~addRow(, i-1, records[i]~name)
    end


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Menu Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - Orders  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newOrder UNGUARDED
    expose records
    --say "OrdermgmtView-newOrder."
    self~showModel(records[4])

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD orderList UNGUARDED
    expose records
    self~showModel(records[3])

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD orderSearch UNGUARDED
    self~noMenuFunction(.HRS~omOrdSrch)

  /*- - Customers - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD customerList UNGUARDED
    expose records
    self~showModel(records[1])

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD customerSearch UNGUARDED
    self~noMenuFunction(.HRS~omCustSrch)

  /*- - Products  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD productList
    expose records
    self~showModel(records[2])

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD productSearch UNGUARDED
    self~noMenuFunction(.HRS~omProdSrch)

  /*- - New - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD product UNGUARDED
    self~noMenuFunction(.HRS~omNewProd)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD customer UNGUARDED
    self~noMenuFunction(.HRS~omNewCust)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD order UNGUARDED
    self~newOrder

  /*- - Help - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    self~noMenuFunction(.HRS~omHelpAbout)


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD noMenuFunction UNGUARDED
    use arg title
    ret = MessageDialog(.HRS~omNoMenu, self~hwnd, title, 'WARNING')


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Icon Double-Click
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD onDoubleClick UNGUARDED
    expose records
    --use arg id
    say "OrderMgmtView-Double-Clicked-01."
    -- Get the index of the item with the focus, use the index to retrieve
    -- the item's record:
    index = self~lv~focused		-- lv is an attribute of the superclass.
    record = records[index+1]
    say "OrderMgmtView-02-onDoubleClick: Record ID =" record~ID
    self~showModel(record)



  /*----------------------------------------------------------------------------
    Event-Handler Methods - PushButton Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD resetIcons
    r = self~lv~arrange

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD exitApp UNGUARDED
    self~cancel

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD cancel
    response = askDialog(.HRS~omQExit, "N")
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
    say "OrderMgmtView-showModel-01: className =" className
    viewClassName = className||"View"
    --root = self
    interpret "."||viewClassName||"~newInstance(self)"
    say "OrderMgmtView-showModel-02:"
    --.CustomerListView~newInstance(self,root)

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  HRS (Human-Readable Strings for OrderMgmtViewBase)		  v00-01 03Oct11
  ---
  The HRS class provides constant character strings for user-visible messages
  issued by the OrderMgmtBaseView class.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


::CLASS HRS PRIVATE		-- Human-Readable Strings
  ::CONSTANT omQExit        "Are you sure you want to close all windows and exit the application?"
  ::CONSTANT omNoMenu       "This menu item is not yet implemented."
  ::CONSTANT omOM           "Order Management"
  ::CONSTANT omOrdSrch      "Order Search"
  ::CONSTANT omCustSrch     "Customer Search"
  ::CONSTANT omProdSrch     "Product Search"
  ::CONSTANT omNewCust      "New Customer"
  ::CONSTANT omNewProd      "New Product"
  ::CONSTANT omHelpAbout    "Help - About"

/*============================================================================*/
