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
   Exercise06: ProductListView
   productListView.rex -  A list of products. 			  v00-03 12Oct11

   Contains: 	   class "ProductListView; class HRS (for human-readable strings)
   Pre-requisites: ProductListView.rc, ProductListView.h, ProdList.ico

   Description: An "intermediate" component - called by OrderMgmt,
                calls "CustomerView".

   Changes:
   v00-01 26Aug11.
   v00-02 19Sep11: Corrected for standalone invocation.
   v00-03 12Oct11: Added methods for menu items (show msg box - "Not Implemented")
		   Also added calss HRS for display strings.

   Outstanding Problems: None reported.
*******************************************************************************/

::REQUIRES "ooDialog.cls"
::REQUIRES "Product\ProductView.rex"


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ProductListView						  v00-01 26Aug11
  -------------
  The view of a list of products.
  Changes:
    v00-01: First version
    v00-02: Corrected for standalone invocawtion.

  [interface (idl format)]  <<optional>>
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS ProductListView SUBCLASS RcDialog PUBLIC

  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    expose rootDlg
    use arg rootDlg
    .Application~useGlobalConstDir("O","Product\ProductListView.h")
    --say ".ProductListView-newInstance-01: rootDlg =" rootDlg
    dlg = self~new("Product\ProductListView.rc", "IDD_PRODLIST_DIALOG")
    --say ".ProductListView-newInstance-02."
    dlg~activate(rootDlg)


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
    say "ProductListView-createMenuBar-01."
    menuBar = .ScriptMenuBar~new("Product\ProductListView.rc", "IDR_PRODLIST_MENU", , , .true, self)
    return .true


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate unguarded
    expose rootDlg
    use arg rootDlg
    --say "ProductListView-activate-01: root =" root

    if rootDlg = "SA" then do			-- If standalone operation required
      rootDlg = self				      -- To pass on to children
      self~execute("SHOWTOP","IDI_PRODLIST_DLGICON")
    end
    else self~popupAsChild(rootDlg, "SHOWTOP", ,"IDI_PRODLIST_DLGICON")
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose menuBar lvProducts btnShowProduct
    -- Called by ooDialog after SHOWTOP.

    menuBar~attachTo(self)

    say "ProductListView-initDialog-01"; say
    lvProducts = self~newListView("IDC_PRODLIST_LISTVIEW");
    lvProducts~addExtendedStyle(GRIDLINES FULLROWSELECT)
    lvProducts~insertColumnPX(0,"Number",60,"LEFT")
    lvProducts~insertColumnPX(1,"Name",150,"LEFT")
    lvProducts~insertColumnPX(2,"Zip",50,"LEFT")
    self~connectListViewEvent("IDC_PRODLIST_LISTVIEW","CLICK",itemSelected)
    self~connectListViewEvent("IDC_PRODLIST_LISTVIEW","ACTIVATE",openItem)
    self~connectButtonEvent("IDC_PRODLIST_SHOWPRODUCT","CLICKED",showProduct)

    self~loadList

  /*----------------------------------------------------------------------------
    Event-Handler Methods - Menu Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newProduct UNGUARDED
    self~noMenuFunction(.HRS~plNewCust)

  /*- - Help - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    self~noMenuFunction(.HRS~plHelpAbout)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD noMenuFunction UNGUARDED
    use arg title
    ret = MessageDialog(.HRS~plNoMenu, self~hwnd, title, 'WARNING')


  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    Event Handling Methods - List Items
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD itemSelected unguarded
    expose lvProducts
    use arg id, itemIndex, columnIndex, keyState
    say "ProductListView-itemSelected: itemIndex, columnIndex, keyState:" itemIndex columnIndex keyState
    say "ProductListView-itemSelected: item selected is:"lvProducts~selected
    self~enableControl("IDC_PRODLIST_SHOWPRODUCT")
    --text = list~itemText(itemIndex)
    --colText = list~itemText(itemIndex, 1)
    --parent~insertNewItem(text, colText)


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD openItem UNGUARDED
    say "ProductListView-openItem-01: item selected =" item
    self~showProduct


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showProduct UNGUARDED
    expose lvProducts rootDlg
    item = lvProducts~selected
    say "ProductListView-showProduct-01: item selected =" item
    info=.Directory~new
    if lvProducts~getItemInfo(item, info) then do
      --say "ProductListView-showProduct-02: info~text =" info~text
      --say "ProductListView-showProduct-03; root =" root
-- NEXT 4 STMTS ADDED FOR EXERCISE06:
      .local~my.idProductData  = .ProductData~newInstance	-- create a ProductData instance
      .local~my.idProductModel = .ProductModel~newInstance	-- create a ProductModel instance
      .local~my.idProductData~activate
      .local~my.idProductModel~activate
      .ProductView~newInstance(rootDlg,"CU003")
      --say "ProductListView-showProduct-04: after startProductView"
    end
    else do
      say "NO ITEM SeLeCTED!"
    end


  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD loadList
    expose lvProducts
    lvProducts~addRow(2, ,"CU003","Widget, 5in")
    lvProducts~addRow(4, ,"CU025","Slodget, case of 24", "RG7 3UP")
    lvProducts~addRow(3, ,"DX210","Driblet, 5in, 10-pack, no delivery.", "021956")
    do i = 1 to 50
      lvProducts~addRow(i, , "Line" i, i)
    end
    lvProducts~setColumnWidth(1)	-- set width of 2nd coluimn to longest text entry.

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  HRS (Human-Readable Strings for CustomerListView)		  v00-01 12Oct11
  ---
  The HRS class provides constant character strings for user-visible messages
  issued by the CustomerListView class.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


::CLASS HRS PRIVATE		-- Human-Readable Strings
  ::CONSTANT plNoMenu       "This menu item is not yet implemented."
  ::CONSTANT plNewCust      "New Product"
  ::CONSTANT plHelpAbout    "Help - About"

/*============================================================================*/


