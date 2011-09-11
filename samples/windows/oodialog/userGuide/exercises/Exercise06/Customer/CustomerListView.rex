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
   Exercise 06: The CustomerListView class
   CustomerList.rex 						  v01-00 10Sep11

   Contains: class "CustomerListView"
   Pre-requisite files: CustomerListView.rc, CustomerListView.h.

   Changes: This is the first version.

   Description: Provides a list of Customers and supports viewing any given
                Customer via a double-click on that Customer's item in the list.

   v00-01 10Sep11 First Version

   Outstanding Problems: None reported.
*******************************************************************************/

::REQUIRES "ooDialog.cls"
::REQUIRES "customer\customerview.rex"

::CLASS CustomerListView SUBCLASS RcDialog PUBLIC

  ::ATTRIBUTE hasParent CLASS

  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    expose root hasParent
    use arg parent, root
    if parent = "SA" then hasParent = .false; else hasParent = .true
    .Application~useGlobalConstDir("O","Customer\CustomerListView.h")
    say ".CustomerListView-newInstance-01: root =" root
    dlg = self~new("Customer\CustomerListView.rc", "IDD_DIALOG1")
    say ".CustomerListView-newInstance-02."
    dlg~activate(parent, root)				-- Must be the last statement.


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
    say "CustomerListView-createMenuBar-01."
    menuBar = .ScriptMenuBar~new("Customer\CustomerListView.rc", "IDR_MENU1", , , .true, self)
    return .true


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate unguarded
    expose root
    use arg parent, root
    say "CustomerListView-activate-01: root =" root
    trace i
    if .CustomerListView~hasParent then,
      self~popupAsChild(root, "SHOWTOP", ,"IDI_CUSTLIST_DLGICON")
    else self~execute("SHOWTOP","IDI_CUSTLIST_DLGICON")
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose menuBar lvCustomers btnShowCustomer
    -- Called by ooDialog after SHOWTOP.

    menuBar~attachTo(self)

    say "CustomerListView-initDialog-01"; say
    lvCustomers = self~newListView("IDC_LIST1");
    lvCustomers~addExtendedStyle(GRIDLINES FULLROWSELECT)
    lvCustomers~insertColumnPX(0,"Number",60,"LEFT")
    lvCustomers~insertColumnPX(1,"Name",220,"LEFT")
    lvCustomers~insertColumnPX(2,"Zip",80,"LEFT")
    self~connectListViewEvent("IDC_LIST1","CLICK",itemSelected)
    self~connectListViewEvent("IDC_LIST1","ACTIVATE",openItem)
    self~connectButtonEvent("IDC_SHOW_CUSTOMER","CLICKED",showCustomer)
    --btnShowCustomer = self~newPushButton("IDC_SHOW_CUSTOMER")

    self~loadList


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD loadList
    expose lvCustomers

    lvCustomers~addRow( 1, ,"CU001", "ABC Inc.",   "TX 20152")
    lvCustomers~addRow( 2, ,"CU002", "Frith Inc.", "CA 30543")
    lvCustomers~addRow( 3, ,"CU003", "LMN & Co",   "NY 47290-1201")
    lvCustomers~addRow( 4, ,"CU005", "EJ Smith",   "NJ 12345")
    lvCustomers~addRow( 5, ,"CU010", "Red-On Inc.","AZ 12345")
    lvCustomers~addRow( 6, ,"AB15784", "Joe Bloggs & Co Ltd","LB7 4EJ")
    /*do i = 1 to 50
      lvCustomers~addRow(i, , "Line" i, i)
    end*/
    lvCustomers~setColumnWidth(1)	-- set width of 2nd column to longest text entry.


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD itemSelected unguarded
    expose lvCustomers --btnShowCustomer
    use arg id, itemIndex, columnIndex, keyState
    say "CustomerListView-itemSelected: itemIndex, columnIndex, keyState:" itemIndex columnIndex keyState
    say "CustomerListView-itemSelected: item selected is:"lvCustomers~selected
    self~enableControl("IDC_SHOW_CUSTOMER")
    --text = list~itemText(itemIndex)
    --colText = list~itemText(itemIndex, 1)
    --parent~insertNewItem(text, colText)


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD openItem UNGUARDED
    say "CustomerListView-openItem-01: item selected =" item
    self~showCustomer


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showCustomer unguarded
    expose lvCustomers root
    item = lvCustomers~selected
    say "CustomerListView-showCustomer-01: item selected =" item
    info=.Directory~new
    if lvCustomers~getItemInfo(item, info) then do
      say "CustomerListView-showCustomer-02: info~text =" info~text
      --call startCustomerView self
      say "CustomerListView-showCustomer-03; root =" root
.local~my.idCustomerData  = .CustomerData~new	-- create Customer Data instance
.local~my.idCustomerModel = .CustomerModel~new	-- create Customer Model instance
.local~my.idCustomerData~activate
.local~my.idCustomerModel~activate
      if .CustomerListView~hasParent then .CustomerView~newInstance(self,root,"CU003")
      else .CustomerView~newInstance(self,self,"CU003")
      say "CustomerListView-showCustomer-03: after startCustomerView"
    end
    else do
      say "NO ITEM SeLeCTED!"
    end
/*============================================================================*/
