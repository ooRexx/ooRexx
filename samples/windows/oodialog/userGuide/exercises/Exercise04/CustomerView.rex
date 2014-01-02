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
   Exercise04: The Customer component             		  v01-00 01Jun12

   Contains: 	   class "CustomerView";  routine "startCustomerView".

   Pre-requisites: CustomerView.rc, CustomerView.h, CustomerModelView.rex.

   Description: A sample Customer View component - part of the sample
        	Order Management application.

   Changes:
   v01-00 01Jun12: First version.

-------------------------------------------------------------------------*/

::requires "ooDialog.cls"


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  CustomerView							  v01-00 01Jun12
  -------------
  The "view" (or "gui") part of the Customer component - part of the sample
  Order Management application.

  Changes:
    v01-00: First version

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS "CustomerView" SUBCLASS RcDialog PUBLIC

  /*----------------------------------------------------------------------------
    Dialog Creation Methods:
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Init - creates the dialog instance but does not make it visible.        --*/
  ::METHOD init
    expose menuBar
    say "CustomerView-init-01."

    forward class (super) continue

    if \ self~createMenuBar then do		-- if there was a problem
      self~initCode = 1
      return
    end


  /*-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Create Menu Bar - Creates the menu bar on the dialog.                   --*/
  ::METHOD createMenuBar
    expose menuBar
    say "CustomerView-createMenuBar-01."
    menuBar = .ScriptMenuBar~new("CustomerView.rc", IDR_CUST_MENU, , , .true)
    return .true


  /*-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Activate - Shows the Dialog - i.e. makes it visible to the user.        --*/
  ::METHOD activate unguarded
    say "CustomerView-activate-01."
    self~execute("SHOWTOP")
    return


  /*-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    InitDialog - Called by ooDialog 					   -- */
  ::METHOD initDialog
    expose menuBar custControls
    say "CustomerView-initDialog-01."
    menuBar~attachTo(self)
    -- Create objects that map to the edit controls defined by the "customer.rc"
    --   so they can be programmatically used elsewhere in the class:
    custControls = .Directory~new
    custControls[ecCustNo]         = self~newEdit("IDC_CUST_EDT_CUSTNO")
    custControls[ecCustName]       = self~newEdit("IDC_CUST_EDT_CUSTNAME")
    custControls[ecCustAddr]       = self~newEdit("IDC_CUST_EDT_CUSTADDR")
    custControls[ecCustZip]        = self~newEdit("IDC_CUST_EDT_CUSTZIP")
    custControls[ecCustDiscount]   = self~newEdit("IDC_CUST_EDT_DISCOUNT")
    custControls[stLastOrder] = self~newStatic("IDC_CUST_STC_LASTORDERDETAILS")
    -- Create an object for the "Record Change" pushbutton in order to be able
    --   to change its focus later:
    custControls[btnRecordChanges] = self~newPushButton("IDC_CUST_BTN_RECORDCHANGES")
    -- Define event handler methods for push-buttons:
    self~connectButtonEvent("IDC_CUST_BTN_RECORDCHANGES","CLICKED",recordChanges)
    self~connectButtonEvent("IDC_CUST_BTN_SHOWLASTORDER","CLICKED",showLastOrder)
    -- Get app data and then show it:
    self~getData
    self~showData
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    MenuBar Methods:
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    New Customer... Not fully implemented - merely tells user to use the
                    Customer List object.           		            --*/
  ::METHOD newCustomer unguarded
    msg = "Creating a new Customer is not yet implemented."
    ret = InfoDialog(msg)


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Update - Sets fields to edit mode so that user can change the data.
             Business Rule: Customer Number cannot be changed.              --*/
  ::METHOD update unguarded
    expose custControls
    say "CustomerView-Update-01"
    custControls[ecCustName]~setReadOnly(.false)
    custControls[ecCustAddr]~setReadOnly(.false)
    custControls[ecCustZip]~setReadOnly(.false)
    custControls[ecCustDiscount]~setReadOnly(.false)
    self~enableControl("IDC_CUST_BTN_RECORDCHANGES")
    custControls[btnRecordChanges]~state = "FOCUS"  -- Put focus on the button
    self~focusControl("IDC_CUST_EDT_CUSTNAME")      -- place cursor in the CustName edit control.


  /*-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Last Order - Displays info about the last order placed by this customer.--*/
  ::METHOD lastOrder unguarded
    expose custControls
    use arg button
    orderDate="31/12/11";   orderNum = "ZZ999";   orderTotal = "$999.99"
    lastOrder = orderDate "   " orderNum "   " orderTotal
    custControls[stLastOrder]~setText(lastOrder)


  /*-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Print - Not implemented yet                                             --*/
  ::METHOD print unguarded
    msg = "The 'Print...' menu item is not yet implemented."
    ret = MessageDialog(msg, self~hwnd, 'Print', 'WARNING')


  /*----------------------------------------------------------------------------
    PushButton Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Record Changes - Collects new data, checks if there has indeed been a
                       change, and if not, issues a warning msg and disables
                       the button.                                          --*/
  ::METHOD recordChanges unguarded
    expose custControls custData newCustData
    say "CustomerView-recordChanges-01"

    newCustData = .directory~new
    newCustData[custName] = custControls[ecCustName]~getLine(1)
    newCustData[custAddr] = .array~new
    do i=1 to custControls[ecCustAddr]~lines
      newCustData[custAddr][i] = custControls[ecCustAddr]~getLine(i)
    end
    newCustData[custZip] = custControls[ecCustZip]~getLine(1)
    newCustData[custDiscount] = custControls[ecCustDiscount]~getLine(1)

    -- Check if anything's changed:
    result = self~checkForChanges
    if result then say "CustomerView-recordChanges-01: There were changes!"
    else say "CustomerView-recordChanges-02: No Changes Found"

    /* Send new data to be checked by CustomerModel (not implemented). */

    /* Disable controls that were enabled by menu "File-Update" selection: */
    custControls[ecCustName]~setReadOnly(.true)
    custControls[ecCustAddr]~setReadOnly(.true)
    custControls[ecCustZip]~setReadOnly(.true)
    custControls[ecCustDiscount]~setReadOnly(.true)
    self~disableControl("IDC_CUST_BTN_RECORDCHANGES")


  /*----------------------------------------------------------------------------
    Show Last Order - displays mock sales order info in the Last_Order_Details
                        field; info is hard-coded in this method.           --*/
  ::METHOD showLastOrder unguarded
    expose CustControls
    -- Notionally get last order from "SalesOrder" component.
    orderDate="12/2/11";   orderNum = "AB123";   orderTotal = "$524.58"
    lastOrder = orderDate "   " orderNum "   " orderTotal
    custControls[stLastOrder]~setText(lastOrder)


  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    Get Data - gets data from the CustomerModel component and displays it in the
    	       appropriate controls.			                    --*/
  ::METHOD getData
    expose custData
    say "CustomerView-getData-01."
    idCustomerModel = .local~my.idCustomerModel
    custData = idCustomerModel~query

  /*----------------------------------------------------------------------------
    showData - displays data in the dialog's controls.                        */
  ::METHOD showData
    expose custData custControls
    say "CustomerView-showData-01."
    -- Show CustNo and CustName:
    custControls[ecCustNo]~setText(custData[custNo])
    custControls[ecCustName]~setText(custData[custName])
    -- Re-format Cust Address from an array into a string with line-ends
    -- after each array element except the last, then show it.
    arrCustAddr = custData[CustAddr]
    strCustAddr = ""
    do i=1 to arrCustAddr~items
      if i < arrCustAddr~items then do
        strCustAddr = strCustAddr||arrCustAddr[i] || .endofline
      end
      else do
        strCustAddr = strCustAddr || arrCustAddr[i]
      end
    end
    custControls[ecCustAddr]~setText(strCustAddr)
    -- Finally, show Zip and Discount:
    custControls[ecCustZip]~setText(custData[custZip])
    custControls[ecCustDiscount]~setText(custData[custDiscount])
    --custControls[stLastOrder]~setText("Press Me")


  /*--------------------------------------------------------------------------
    checkForChanges - after "Record Changes" actioned by the user, check whether
    any data has actually changed. If it has: (a) assign new data to old data;
    (b) return .true. If it hasn't: return .false.
    Note: cannot just compare the two directories since data format in Address
    is different.                                                           --*/
  ::METHOD checkForChanges
    expose custData newCustData
    changed = .false
    if newCustData[custName] \= custData[custName] then do
      custData[custName] = newCustData[custName]
      changed = .true
    end
    if custData[CustAddr]~items \= newCustData[CustAddr]~items then changed = .true
    else
      do i=1 to custData[custAddr]~items
        if custData[custAddr][i] \= newCustData[custAddr][i] then do
          changed = .true
          leave
      end
    end
    if newCustData[custZip] \= custData[custZip] then do
      custdata[custZip] = newCustData[custZip]
      changed = .true
    end
    if newCustData[custDiscount] \= custData[custDiscount] then do
      custData[custDiscount] = newCustData[custDiscount]
      changed = .true
    end
    -- If no changes after all, display message box:
    if \changed then do
      msg = "CustomerView-checkForChanges-01: Nothing was changed! Update not done."
      hwnd = self~dlgHandle
      answer = MessageDialog(msg,hwnd,"Update Customer","OK","WARNING","DEFBUTTON2 APPLMODAL")
      end
    else do
      say "CustomerView-checkForChanges-02: changed =" changed
      custData = newCustData
    end
    return changed

/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  StartCustomerView						  v01-00 01Jun12
  -----------------
  A routine that creates the CustomerView dialog.

  Changes:
    v01-00: First version
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::ROUTINE StartCustomerView PUBLIC
  say "StartCustomerView Routine-01: Start."
  .Application~setDefaults("O", "CustomerView.h", .false)
  dlg = .CustomerView~new("CustomerView.rc", "IDD_CUST_DIALOG")
  say "StartCustomerView Routine-02: dlg~activate."
  dlg~activate
  say "StartCustomerView Routine-03: End."
/*============================================================================*/
