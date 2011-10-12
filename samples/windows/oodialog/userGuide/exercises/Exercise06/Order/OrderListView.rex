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
   Exercise 06: The OrderListView class
   OrderList.rex 						  v01-01 12Oct11

   Contains: class "OrderListView"
   Pre-requisite files: OrderListView.rc, OrderListView.h.

   Changes: This is the first version.

   Description: Provides a list of Orders and supports viewing any given
                Order via a double-click on that Order's item in the list.
                This is an "Intermediate" component - it is invoked by OrderMgmt,
                and invokes OrderView.

   v01-00 19Sep11: First Version
   v01-01 12Oct11: Added menu select methods (all saying not implemented).
   		   Added an HRS class for text strings.

   Outstanding Problems: None reported.
*******************************************************************************/

::REQUIRES "ooDialog.cls"
::REQUIRES "Order\OrderView.rex"

::CLASS OrderListView SUBCLASS RcDialog PUBLIC

  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    use arg rootDlg
    .Application~useGlobalConstDir("O","Order\OrderListView.h")
    say ".OrderListView-newInstance-01: root =" rootDlg
    dlg = self~new("Order\OrderListView.rc", "IDD_ORDLIST_LISTVIEW")
    say ".OrderListView-newInstance-02."
    dlg~activate(rootDlg)				-- Must be the last statement.


  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD init
    forward class (super) continue
    if \ self~createMenuBar then do		-- if there was a problem
      self~initCode = 1
      return
    end


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD createMenuBar
    -- Creates the menu bar on the dialog.
    expose menuBar
    say "OrderListView-createMenuBar-01."
    menuBar = .ScriptMenuBar~new("Order\OrderListView.rc", "IDR_ORDLIST_MENU", , , .true, self)
    return .true


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate unguarded
    expose rootDlg
    use arg rootDlg
    say "OrderListView-activate-01: root =" rootDlg
    --trace i
    if rootDlg = "SA" then do			-- If standalone operation required
      rootDlg = self				      -- To pass on to children
      self~execute("SHOWTOP","IDI_ORDLIST_DLGICON")
    end
    else self~popupAsChild(rootDlg, "SHOWTOP", ,"IDI_ORDLIST_DLGICON")
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose menuBar lvOrders btnShowOrder
    -- Called by ooDialog after SHOWTOP.

    menuBar~attachTo(self)

    say "OrderListView-initDialog-01"; say
    lvOrders = self~newListView("IDC_ORDLIST_LIST");
    lvOrders~addExtendedStyle(GRIDLINES FULLROWSELECT)
    lvOrders~insertColumnPX(0,"OrderNo",60,"LEFT")
    lvOrders~insertColumnPX(1,"CustNo",80,"LEFT")
    lvOrders~insertColumnPX(2,"CustName",130,"LEFT")
    lvOrders~insertColumnPX(3,"Date",80,"LEFT")
    self~connectListViewEvent("IDC_ORDLIST_LIST","CLICK",itemSelected)
    self~connectListViewEvent("IDC_ORDLIST_LIST","ACTIVATE",openItem)
    self~connectButtonEvent("IDC_ORDLIST_SHOWORDER","CLICKED",showOrder)
    --btnShowOrder = self~newPushButton("IDC_SHOW_Order")

    self~loadList


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD loadList
    expose lvOrders

    lvOrders~addRow( 1, ,"DM00263", "CU003",   "ABC Inc.",   "21Nov12")
    lvOrders~addRow( 2, ,"DM10473", "AB15784", "Frith Inc.", "12Oct12")
    lvOrders~addRow( 3, ,"DM13003", "CU001",   "LMN & Co",   "07Jun12")
    lvOrders~addRow( 4, ,"AS49005", "CU003",   "EJ Smith",   "30Aug12")
    lvOrders~addRow( 5, ,"AM72010", "CU005",   "Red-On Inc.","17Jan13")
    lvOrders~addRow( 6, ,"OZ15784", "CU003",   "Joe Bloggs & Co Ltd","28Feb13")
    /*do i = 1 to 50
      lvOrders~addRow(i, , "Line" i, i)
    end*/
    lvOrders~setColumnWidth(1)	-- set width of 2nd column to longest text entry.


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Menu Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newOrder UNGUARDED
    self~noMenuFunction(.HRS~olNewOrder)

  /*- - Help - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    self~noMenuFunction(.HRS~olHelpAbout)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD noMenuFunction UNGUARDED
    use arg title
    ret = MessageDialog(.HRS~olNoMenu, self~hwnd, title, 'WARNING')


  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD itemSelected unguarded
    expose lvOrders --btnShowOrder
    use arg id, itemIndex, columnIndex, keyState
    say "OrderListView-itemSelected: itemIndex, columnIndex, keyState:" itemIndex columnIndex keyState
    say "OrderListView-itemSelected: item selected is:"lvOrders~selected
    self~enableControl("IDC_ORDLIST_SHOWORDER")
    --text = list~itemText(itemIndex)
    --colText = list~itemText(itemIndex, 1)
    --parent~insertNewItem(text, colText)


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD openItem UNGUARDED
    say "OrderListView-openItem-01: item selected =" item
    self~showOrder


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showOrder unguarded
    expose lvOrders rootDlg
    item = lvOrders~selected
    say "OrderListView-showOrder-01: item selected =" item
    info=.Directory~new
    if lvOrders~getItemInfo(item, info) then do
      say "OrderListView-showOrder-02: info~text =" info~text
      --call startOrderView self
      say "OrderListView-showOrder-03; root =" rootDlg
      .local~my.idOrderData  = .OrderData~new	-- create Order Data instance
      .local~my.idOrderModel = .OrderModel~new	-- create Order Model instance
      .local~my.idOrderData~activate
      .local~my.idOrderModel~activate
      .OrderView~newInstance(rootDlg,"DM00263")
      say "OrderListView-showOrder-03: after startOrderView"
    end
    else do
      say "NO ITEM SeLeCTED!"
    end
/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  HRS (Human-Readable Strings for OrderListView)		  v00-01 12Oct11
  ---
  The HRS class provides constant character strings for user-visible messages
  issued by the CustomerListView class.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


::CLASS HRS PRIVATE		-- Human-Readable Strings
  ::CONSTANT olNoMenu       "This menu item is not yet implemented."
  ::CONSTANT olNewOrder     "New Order"
  ::CONSTANT olHelpAbout    "Help - About"

/*============================================================================*/


