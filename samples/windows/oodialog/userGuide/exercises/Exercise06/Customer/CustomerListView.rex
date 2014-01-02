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
   Exercise 06: The Customer ListView				  v01-00 06Jun12

   Contains: classes "CustomerListView" and "HRSclv".

   Pre-requisite: CustomerListView.rc, CustomerListView.h, CustList.ico

   Description: Provides a list of Customers and supports viewing any given
                Customer via a double-click on that Customer's item in the list.
                This is an "Intermediate" component - it is invoked by OrderMgmt,
                and invokes CustomerView.

   v01-00 06Jun12: First Version.

   Outstanding Problems: None reported.
*******************************************************************************/


.Application~addToConstDir("Customer\CustomerListView.h")


::REQUIRES "ooDialog.cls"
::REQUIRES "customer\customerview.rex"


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  CustomerListView						  v01-04 19Feb12
  ----------------
  The view of a list of Customers.
  Changes:
    v01-01: First version
    v01-02: Corrected for standalone invocation.
    v01-03 28Jan12: Changed name of HRS class to HRSplv.
    v01-04 11Feb12: moved .application~setDefaults() to app startup file.
                    changed to .application~addToConstDir() here.
    v01-05 19Feb12: Moved .Application~addToConstDir statement from newInstance
                    method to top of file - just before ::requires statement(s).
    v01-06 29Mar12: Very minor mods - all just minor clean-ups. All comments removed


  [interface (idl format)]  <<optional>>
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS CustomerListView SUBCLASS RcDialog PUBLIC

  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    use arg rootDlg
    dlg = self~new("Customer\CustomerListView.rc", "IDD_CUSTLIST_DIALOG")
    dlg~activate(rootDlg)				-- Must be the last statement.


  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
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
    menuBar = .ScriptMenuBar~new("Customer\CustomerListView.rc", "IDR_CUSTLIST_MENU", , , .true)
    return .true


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    expose rootDlg
    use arg rootDlg
    if rootDlg = "SA" then do			-- If standalone operation required
      rootDlg = self				      -- To pass on to children
      self~execute("SHOWTOP","IDI_CUSTLIST_DLGICON")
    end
    else self~popupAsChild(rootDlg, "SHOWTOP", ,"IDI_CUSTLIST_DLGICON")
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose menuBar lvCustomers btnShowCustomer
    -- Called by ooDialog after SHOWTOP.

    menuBar~attachTo(self)

    lvCustomers = self~newListView("IDC_CUSTLIST_LIST");
    lvCustomers~addExtendedStyle("GRIDLINES FULLROWSELECT")
    lvCustomers~insertColumnPX(0,"Number",60,"LEFT")
    lvCustomers~insertColumnPX(1,"Name",220,"LEFT")
    lvCustomers~insertColumnPX(2,"Zip",80,"LEFT")
    self~connectListViewEvent("IDC_CUSTLIST_LIST","CLICK",itemSelected)		-- Single click
    self~connectButtonEvent("IDC_CUSTLIST_SHOWCUST","CLICKED",showCustomer)
    self~connectListViewEvent("IDC_CUSTLIST_LIST","ACTIVATE",openItem)	 	-- Double-click

    self~loadList


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Menu Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newCustomer UNGUARDED
    self~noMenuFunction(.HRSclv~newCust)

  /*- - Help - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    self~noMenuFunction(.HRSclv~helpAbout)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD noMenuFunction UNGUARDED
    use arg title
    ret = MessageDialog(.HRSclv~noMenu, self~hwnd, title, 'WARNING')


  /*----------------------------------------------------------------------------
    Event Handling Methods - List Items
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD itemSelected UNGUARDED
    use arg id, itemIndex, columnIndex, keyState
    /* This method is fired when the user clicks on a row in the ListView.
       If the user clicks on an empty row, then itemIndex is set to -1, else
       the itemIndex is set to the 0-based row number.
       If the user double-clicks on a row, this method is fired in response
       to the first click but not to the second. If the row is empty, the second
       click of the double-click is ignored, else the double-click method is
       fired.
    */
    if itemIndex > -1 then self~enableControl("IDC_CUSTLIST_SHOWCUST")
    else self~disableControl("IDC_CUSTLIST_SHOWCUST")


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD openItem UNGUARDED
    -- User double-clicked on an item in the ListView.
    -- Note: does not get fired if double-click was on an empty row.
    self~showCustomer


  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showCustomer UNGUARDED
    expose lvCustomers rootDlg
    item = lvCustomers~selected
    if item = -1 then do		-- if no item selected.
      ret = MessageDialog(.HRSclv~nilSelected, self~hwnd, title, 'WARNING')
      return
    end
    info=.Directory~new
    if lvCustomers~getItemInfo(item, info) then do
      say "CustomerListView-showCustomer-02: info~text =" info~text
      .local~my.idCustomerData  = .CustomerData~new	-- create Customer Data instance
      .local~my.idCustomerModel = .CustomerModel~new	-- create Customer Model instance
      .local~my.idCustomerData~activate
      .local~my.idCustomerModel~activate
      .CustomerView~newInstance(rootDlg,"CU003")
      self~disableControl("IDC_CUSTLIST_SHOWCUST")
    end
    else do
      say "CustomerListView-showCustomer-04: ~getItemInfo returned .false."
    end

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD loadList
    expose lvCustomers
    lvCustomers~addRow( , ,"CU001", "ABC Inc.",   "TX 20152")
    lvCustomers~addRow( , ,"CU002", "Frith Inc.", "CA 30543")
    lvCustomers~addRow( , ,"CU003", "LMN & Co",   "NY 47290-1201")
    lvCustomers~addRow( , ,"CU005", "EJ Smith",   "NJ 12345")
    lvCustomers~addRow( , ,"CU010", "Red-On Inc.","AZ 12345")
    lvCustomers~addRow( , ,"AB15784", "Joe Bloggs & Co Ltd","LB7 4EJ")
    /*do i = 1 to 50
      lvCustomers~addRow(i, , "Line" i, i)
    end*/
    lvCustomers~setColumnWidth(1)	-- set width of 2nd column to longest text entry.

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  HRSclv (Human-Readable Strings for CustomerListView)		  v01-00 06Jun12
  ---
  The HRSclv class provides constant character strings for user-visible messages
  issued by the CustomerListView class.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS HRSclv PRIVATE		-- Human-Readable Strings
  ::CONSTANT noMenu       "This menu item is not yet implemented."
  ::CONSTANT newCust      "New Customer"
  ::CONSTANT helpAbout    "Help - About"
  ::CONSTANT nilSelected  "Please select an item first."

/*============================================================================*/

