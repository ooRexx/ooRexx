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
   Exercise 08: The Customer ListView				  v03-00 24May13

   Contains: classes "CustomerListView" and "HRSclv".

   Pre-requisite: CustomerListView.rc, CustomerListView.h, CustList.ico

   Description: Provides a list of Customers and supports viewing any given
                Customer via a double-click on that Customer's item in the list.
                This is an "Intermediate" component - it is invoked by OrderMgmt,
                and invokes CustomerView.

   v01-00 06Jun12: First Version.
   v02-00 08Jan13: Removed stand-alone startup code (not needed after Ex06)
                   Commented out say's.
   v03-00 24May13: Updated to use View and Component mixins.

   Outstanding Problems: None reported.
*******************************************************************************/


.Application~addToConstDir("Customer\CustomerListView.h")

::REQUIRES "ooDialog.cls"
::REQUIRES "customer\customerview.rex"
::REQUIRES "Support\View.rex"
::REQUIRES "Support\Component.rex"


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  CustomerListView						  v03-00 24May13
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
    v02-00 17Aug12: Exercise07 - modified to use the MVF.
           08Jan13: Removed stand-alone startup (not now needed).
    v03-00 24May13: Inherits directly from RcDialog plus the View & Component mixins



  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

--::CLASS CustomerListView SUBCLASS RcView PUBLIC
::CLASS CustomerListView SUBCLASS RcDialog PUBLIC INHERIT View Component
  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    use arg idCustomerListModel, rootDlg
    --say "CustomerListView-newInstance-01: instName, rootDlg =" idCustomerListModel rootDlg
    dlg = self~new("Customer\CustomerListView.rc", "IDD_CUSTLIST_DIALOG")
    --say "CustomerListView-newInstance-02: dlg =" dlg
    dlg~activate(idCustomerListModel, rootDlg)				-- Must be the last statement.
    return dlg

  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    forward class (super) continue
    self~initView
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
    expose rootDlg modelData
    use arg idCustomerListModel, rootDlg
    --say "CustomerListView-activate-01."
    forward class (super) continue			-- required for MVF
    modelData = RESULT					-- super gets my data!
    --say "CustomerListView-activate-02: rootDlg =" rootDlg
    self~popupAsChild(rootDlg, "SHOWTOP", ,"IDI_CUSTLIST_DLGICON")
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose menuBar lvCustomers btnShowCustomer modelData
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
    --self~connectListViewEvent("IDC_CUSTLIST_LIST","BEGINDRAG",beginDM)	 	-- Button 1 down

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
      --say "CustomerListView-showCustomer-02: info~text =" info~text
      objectMgr = .local~my.ObjectMgr					-- Ex07
      objectMgr~showModel("CustomerModel", info~text, rootDlg)		-- Ex07
      self~disableControl("IDC_CUSTLIST_SHOWCUST")			-- Ex07
    end
    else do
      say "CustomerListView-showCustomer-04: ~getItemInfo returned .false."
    end

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD loadList
    expose lvCustomers modelData					-- Ex07
    --say "CustomerListView-LoadList-00: modelData =" modelData
    -- modelData is a directory.
    --say "CustomerListView-loadList-01: No Records =" modelData[count]
    rows = modelData[count]				-- Ex07 - number of rows
    arrData = modelData[records]
    --say "CustomerListView-loadList-02:Dims =" arrData~dimension(1) arrData~dimension(2)
    do i = 1 to rows				-- Ex07 - omit the header line.
      --say "CustomerListView-loadList-02: arr[i,1 =" arrData[i,1]
      lvCustomers~addRow( , ,arrData[i,1],arrData[i,2],arrData[i,5])
    end
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

