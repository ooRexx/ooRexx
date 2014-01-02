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
/* ooDialog User Guide
   Exercise 07: The OrderView class				  v02-00 01Apr13

   OrderFormView.rex

   Contains: class "OrderView".
   Pre-requisite files: OrderView.rc, OrderView.h.

   Description: A sample Order View component - part of the sample
        	Order Management application.
        	This is a "leaf" component - invoked by OrderListView.

   Outstanding Problems: None reported.

   Changes:
     v01-00 07Jun12: First Version (Exercise06)
     v02-00 08Jan13: Ex07 - changed to use the MVF.
            11Jan13: Removed stand-alone operation.
                     Commented-out 'say' instructions.
            01Apr13: After ooDialog 4.2.2, Support folder moved to exercise
                     folder, so change to ::Requires needed.

------------------------------------------------------------------------------*/


.Application~addToConstDir("Order\OrderView.h")


::REQUIRES "ooDialog.cls"
::REQUIRES "Order\OrderModelsData.rex"
::REQUIRES "Support\RcView.rex"


/*==============================================================================
  OrderView							  v01-01 25Aug12
  -------------
  The "view" (or "gui") part of the Order component - part of the sample
  Order Management application.

  Changes:
    v01-00 07Jun12: First Version.
    v02-00 25Aug12: Ex07 - changed to use the MVF.
           08Jan13: Extraneous comments removed; some 'say' stmts commented out.
           11Jan13: Commented-out 'say' instructions.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderView SUBCLASS RcView PUBLIC				-- Ex07

  ::METHOD newInstance CLASS PUBLIC
    use strict arg idModel, rootDlg					-- Ex07
    --say ".OrderView-newInstance: rootDlg =" rootDlg
    dlg = self~new("Order\OrderView.rc", "IDD_ORDER_DIALOG")
    dlg~activate(idModel, rootDlg)					-- Ex07
    return dlg								-- Ex07

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    -- creates the dialog instance but does not make it visible.
    expose menuBar
    --say "OrderView-init-01"

    forward class (super) continue

    if \ self~createMenuBar then do		-- if there was a problem
      self~initCode = 1
      return
    end


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD createMenuBar
    -- Creates the menu bar on the dialog.
    expose menuBar
    --say "OrderView-createMenuBar-01"
    menuBar = .ScriptMenuBar~new("Order\OrderView.rc", IDR_ORDER_MENU, , , .true)

    return .true


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    expose rootDlg orderData
    use strict arg idModelInstance, rootDlg				-- Ex07
    forward class (super) continue		-- Ex07 - MVF: required to get Model's data
    orderData = RESULT				-- Ex07 - MVF: model's data returned by super
    --say "OrderView-activate-00: orderData = " orderData

    -- Shows the Dialog - i.e. makes it visible to the user.
    --say "OrderView-activate-01: OrderNumber = " orderData["OrderNo"] orderData~orderNo
    self~popUpAsChild(rootDlg,"SHOWTOP",,"IDI_ORDER_DLGICON")
    return self								-- Ex07


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    -- Called by ooDialog after SHOWTOP.
    expose menuBar orderControls orderData				-- Ex07
    --say "OrderView-initDialog-01"

    menuBar~attachTo(self)

    orderControls = .Directory~new
    orderControls[ecOrderNameAddr] = self~newEdit(    "IDC_ORDER_NAMEADDR")
    orderControls[ecOrderDate]     = self~newEdit(    "IDC_ORDER_DATE"    )
    orderControls[ecOrderNo]       = self~newEdit(    "IDC_ORDER_ORDNO"   )
    orderControls[ecCustNo]        = self~newEdit(    "IDC_ORDER_CUSTNO"  )
    orderControls[lvOrderItems]    = self~newListView("IDC_ORDER_ITEMS"   )
    orderControls[stNetCost]       = self~newStatic(  "IDC_ST_NET"        )
    orderControls[stDiscountPC]    = self~newStatic(  "IDC_ST_DISCOUNT_PC")
    orderControls[stDiscount]      = self~newStatic(  "IDC_ST_DISCOUNT"   )
    orderControls[stTaxPC]         = self~newStatic(  "IDC_ST_TAX_PC"     )
    orderControls[stTax]           = self~newStatic(  "IDC_ST_TAX"        )
    orderControls[stTotal]         = self~newStatic(  "IDC_ST_TOTAL"      )

    -- Each Order Item consists of ProdNo, ProdName, Qty:
    orderItems = orderControls[lvOrderItems]
    orderItems~addExtendedStyle(GRIDLINES FULLROWSELECT)
    orderItems~insertColumnPX(0,"Product No.",110,"LEFT")
    orderItems~insertColumnPX(1,"Product Name",160,"LEFT")
    orderItems~insertColumnPX(2,"Quantity",85,"LEFT")
    self~connectListViewEvent("IDC_ORDER_ITEMS","ACTIVATE",showProduct)	-- double-click

    --font = self~createFontEx("Ariel", 10)
    font = self~createFontEx("Courier", 8)
    orderControls[lvOrderItems]~setFont(font)
    self~showData


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Menu Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showProduct
    expose orderControls rootDlg
    --say "OrderView-showProduct-01: Show Product requested."
    productItem = orderControls[lvOrderItems]~selected
    --say "OrderView-showProduct-01: item selected =" productItem
    if item = -1 then do		-- if no item selected.
      ret = MessageDialog(.HRSolv~nilSelected, self~hwnd, title, 'WARNING')
      return
    end

    info=.Directory~new
    if orderControls[lvOrderItems]~getItemInfo(productItem, info) then do
      --say "OrderView-showOrder-02: info~text =" info~text
      objectMgr = .local~my.ObjectMgr
      objectMgr~showModel("ProductModel", info~text, self)		--Ex07
      self~disableControl("IDC_ORDLIST_SHOWORDER")
    end
    else do
      say "OrderListView-showOrder-04: ~getItemInfo returned .false."
    end

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD anAction UNGUARDED
    self~noMenuFunction(.HRSov~anAction)

  /*- - Help - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    self~noMenuFunction(.HRSov~HelpAbout)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD noMenuFunction UNGUARDED
    use arg title
    ret = MessageDialog(.HRSov~NoMenu, self~hwnd, title, 'WARNING')


  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showData
    expose orderControls orderData
/*    say "--------------------"
    say "OrderView-showData-00: contents of orderData:"
    do i over orderData
      say i orderData[i]
    end
    say; say "Order Lines:"
    xorderLines = orderData[OrderLines]
    do i over xorderLines
      say i
    end
    say; say "Order Line Headers:"
    xorderLines = orderData[OrderLineHdrs]
    do i over xorderLines
      say i
    end
    say "End of Contents of orderData."
    say "---------------------"; say
*/
    --say "orderData['CustNo']:" orderData['CustNo']  -- orderData~CustNo, orderData~'CustNo' - neither work.
    --say "--------------------"
    -- Format & show the address:
    custAddr = orderData["CustAddr"]
    parse var custAddr street "," city "," state
    --say "OrderView-showData-01: address =" street city state orderData["Zip"]
    eol = .endOfLine
    custNameAddr = orderData["CustName"]||eol||street||eol||city||eol||state orderData["Zip"]
    orderControls[ecOrderNameAddr]~setText(custNameAddr)
    -- Format & show the order date:
    orderDate = orderData["Date"]
    yy = orderDate~left(2); dd = orderDate~right(2); mm = orderDate~substr(3,2)
    orderDate = mm||"/"||dd||"/"||yy
    orderControls[ecOrderDate]~setText(orderDate)
    -- Show the Order NUmber:
    orderControls[ecOrderNo]~setText(orderData["OrderNo"])
    -- Show the Customer NUmber:
    orderControls[ecCustNo]~setText(orderData["CustNo"])

    -- Show the Order Items (aka Order Lines):
    lvOrderLines = orderControls[lvOrderItems]
    arr = orderData[OrderLines]
    --say "OrderView-showData-02: order lines array dims =" arr~dimension
    --do i = 1 to arr~dimension(1)
      --say arr[i,1] "-" arr[i,2]  arr[i,3]  arr[i,4]
    --end
    do i=1 to arr~dimension(1)
      qty = arr[i,3]~right(8," ")
      lvOrderLines~addRow( , , arr[i,2], arr[i,4], qty )
    end
    --lvOrderLines~setColumnWidth(0)	-- set width of 2nd column to longest text entry.
    --lvOrderLines~setColumnWidth(1)
    --lvOrderLines~setColumnWidth(3)
    --say "OrderView-showData-02: arrOrderLines =" arrOrderLines
    --orderControls[lvOrderItems]


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    -- "Cancel" - This method over-rides the default Windows action of
    -- 'cancel window' for an Escape key.
  ::METHOD cancel
    response = askDialog(.HRSov~QExit, "N")
    if response = 1 then forward class (super)
    return

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  Human-Readable Strings (HRSov)				  v01-00 07Jun12
  --------
   The HRSofv class provides constant character strings for user-visible messages.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS HRSov PRIVATE	   -- Human-Readable Strings
  ::CONSTANT anAction    "An Action"
  ::CONSTANT NoMenu      "This menu item is not implemented."
  ::CONSTANT QExit       "Are you sure you want to cancel this Order View?"
  ::CONSTANT HelpAbout   "About Sales Order"

/*============================================================================*/

