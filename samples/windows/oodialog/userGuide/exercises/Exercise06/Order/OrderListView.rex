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
   Exercise 06: The Order ListView 				  v01-04 19Feb12

   Contains: class "OrderListView", "HRSolv"

   Pre-requisite files: OrderListView.rc, OrderListView.h.

   Description: Provides a list of Orders and supports viewing any given
                Order via a double-click on that Order's item in the list.
                This is an "Intermediate" component - it is invoked by OrderMgmt,
                and invokes OrderView.

   v01-00 19Sep11: First Version
   v01-01 12Oct11: Added menu select methods (all saying not implemented).
   		   Added an HRS class for text strings.
   v01-02 28Jan12: Changed class name HRS to HRSolv to allow for multiple
     		   HRS classes in same file at some future time.
   v01-03 11Feb12: OrderListView - Changed .application()
   v01-04 19Feb12: OrderListView - Moved .Application~ stmt to top of file.


   Outstanding Problems: None reported.
*******************************************************************************/


.Application~addToConstDir("Order\OrderListView.h")


::REQUIRES "ooDialog.cls"
::REQUIRES "Order\OrderView.rex"


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderListView						  	  v00-05 19Feb12
  -------------
  The view of a list of products.
  Changes:
    v00-01: First version
    v00-02: Corrected for standalone invocation.
    v00-03: 28Jan12: Changed name of HRS class to HRSplv.
    v00-04: 11Feb12: Moved .application~setDefaults() to app startup file.
                     changed to .application~addToConstDir() here.
    v00-05: 19Feb12: Moved .Application~addToConstDir statement from newInstance
                     method to top of file - just before ::requires statement(s).


  [interface (idl format)]  <<optional>>
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderListView SUBCLASS RcDialog PUBLIC

  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    use arg rootDlg
    say ".OrderListView-newInstance-01: root =" "'"||rootDlg||"'"
    dlg = self~new("Order\OrderListView.rc", "IDD_ORDLIST_LISTVIEW")
    say ".OrderListView-newInstance-02."
    dlg~activate(rootDlg)				-- Must be the last statement.


  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
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

    self~loadList


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Menu Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newOrder UNGUARDED
    self~noMenuFunction(.HRSolv~newOrder)

  /*- - Help - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    self~noMenuFunction(.HRSolv~helpAbout)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD noMenuFunction UNGUARDED
    use arg title
    ret = MessageDialog(.HRSolv~noMenu, self~hwnd, title, 'WARNING')


  /*----------------------------------------------------------------------------
    Event Handling Methods - List Items
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD itemSelected unguarded
    expose lvOrders --btnShowOrder
    use arg id, itemIndex, columnIndex, keyState
    --say "OrderListView-itemSelected: itemIndex, columnIndex, keyState:" itemIndex columnIndex keyState
    --say "OrderListView-itemSelected: item selected is:"lvOrders~selected
    if itemIndex > -1 then self~enableControl("IDC_ORDLIST_SHOWORDER")
    else self~disableControl("IDC_ORDLIST_SHOWORDER")
    --text = list~itemText(itemIndex)
    --colText = list~itemText(itemIndex, 1)
    --parent~insertNewItem(text, colText)


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD openItem UNGUARDED
    say "OrderListView-openItem-01: item selected =" item
    self~showOrder


  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showOrder unguarded
    expose lvOrders rootDlg
    item = lvOrders~selected
    say "OrderListView-showOrder-01: item selected =" item
    if item = -1 then do		-- if no item selected.
      ret = MessageDialog(.HRSolv~nilSelected, self~hwnd, title, 'WARNING')
      return
    end
    info=.Directory~new
    if lvOrders~getItemInfo(item, info) then do
      say "OrderListView-showOrder-02: info~text =" info~text
      .local~my.idOrderData  = .OrderData~new	-- create Order Data instance
      .local~my.idOrderModel = .OrderModel~new	-- create Order Model instance
      .local~my.idOrderData~activate
      .local~my.idOrderModel~activate
      .OrderView~newInstance(rootDlg,"DM00263")
      --say "OrderListView-showOrder-03: after startOrderView"
      self~disableControl("IDC_ORDLIST_SHOWORDER")
    end
    else do
      say "OrderListView-showOrder-04: ~getItemInfo returned .false."
    end

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


/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  HRSolv (Human-Readable Strings for OrderListView)		  v00-02 28Jan12
  ---
  The HRSolv class provides constant character strings for user-visible messages
  issued by the OrderListView class.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


::CLASS HRSolv PRIVATE		-- Human-Readable Strings
  ::CONSTANT noMenu       "This menu item is not yet implemented."
  ::CONSTANT newOrder     "New Order"
  ::CONSTANT helpAbout    "Help - About"
  ::CONSTANT nilSelected  "Please select an item first."

/*============================================================================*/


