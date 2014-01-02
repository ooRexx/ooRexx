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
/* ooDialog User Guide - Exercise07

  ProductListView						  v02-00 01Apr13
  -------------
  The view of a list of products.

  Changes:
    v01-00 06Jun12: First version
    v02-00 21Aug12: Modified for Exercise07 to use the Model-View Framework.
           11Jan13: Commented-out 'say' instructions.
           01Apr13: After ooDialog 4.2.2, Support folder moved to exercise
                    folder, so change to ::Requires needed.

  Contains: classes "ProductListView, HRSplv (for human-readable strings)

  Pre-requisites: ProductListView.rc, ProductListView.h, ProdList.ico

  Description: An "intermediate" component - called by OrderMgr,
                invokes "ProductView".

   Outstanding Problems: None reported.
*******************************************************************************/

.Application~addToConstDir("Product\ProductListView.h")


::REQUIRES "ooDialog.cls"
::REQUIRES "Product\ProductView.rex"
::REQUIRES "Support\RcView.rex"


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ProductListView						  v02-00 21Aug12
  -------------
  The view of a list of products.
  Changes:
    v01-00 06Jun12: First version
    v02-00 21Aug12: Modified to use the Model-View Framework.
    		    Comment "Ex07" shows where changes from Ex06 have been made.
           09Jan13: 'Say' instructions commented out or removed.
                    Standalone operation removed (not now needed).
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS ProductListView SUBCLASS RcView PUBLIC

  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    use arg idModel, rootDlg						-- Ex07
    --say ".ProductListView-newInstance-01: rootDlg =" rootDlg
    dlg = self~new("Product\ProductListView.rc", "IDD_PRODLIST_DIALOG")
    --say ".ProductListView-newInstance-02."
    dlg~activate(idModel,rootDlg)					-- Ex07
    return dlg								-- Ex07


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
    --say "ProductListView-createMenuBar-01."
    menuBar = .ScriptMenuBar~new("Product\ProductListView.rc", "IDR_PRODLIST_MENU", , , .true)
    return .true


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate unguarded
    expose rootDlg modelData idModel						-- Ex07
    use arg idModelInstance, rootDlg					-- Ex07
    forward class (super) continue		-- Ex07: required to get Model's data
    modelData = RESULT				-- Ex07: model's data returned by super
    --say "ProductListView-activate-01: root =" root
    self~popupAsChild(rootDlg, "SHOWTOP", ,"IDI_PRODLIST_DLGICON")
    return self								-- Ex07


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose menuBar lvProducts btnShowProduct modelData idModel		-- Ex07
    -- Called by ooDialog after SHOWTOP.

    menuBar~attachTo(self)

    --say "ProductListView-initDialog-01"; say
    lvProducts = self~newListView("IDC_PRODLIST_LISTVIEW");
    lvProducts~addExtendedStyle(GRIDLINES FULLROWSELECT)
    lvProducts~insertColumnPX(0,"Number", 60,"LEFT")
    lvProducts~insertColumnPX(1,"Name",  160,"LEFT")
    lvProducts~insertColumnPX(2,"Price",  50,"RIGHT")			-- Ex07 (Right-adjust)
    lvProducts~insertColumnPX(3,"UOM",    40,"RIGHT")			-- Ex07 (added)
    self~connectListViewEvent("IDC_PRODLIST_LISTVIEW","CLICK",itemSelected)
    self~connectListViewEvent("IDC_PRODLIST_LISTVIEW","ACTIVATE",openItem)
    self~connectButtonEvent("IDC_PRODLIST_SHOWPRODUCT","CLICKED",showProduct)

    -- Set model instance name into the Titlebar:
    --parse var self~objectName
    self~loadList


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Menu Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newProduct UNGUARDED
    self~noMenuFunction(.HRSplv~newProd)

  /*- - Help - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    self~noMenuFunction(.HRSplv~helpAbout)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD noMenuFunction UNGUARDED
    use arg caption
    ret = MessageDialog(.HRSplv~noMenu, self~hwnd, caption, 'WARNING')

  /*----------------------------------------------------------------------------
    Event Handling Methods - List Items
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD itemSelected unguarded
    expose lvProducts
    use arg id, itemIndex, columnIndex, keyState
    --say "ProductListView-itemSelected: itemIndex, columnIndex, keyState:" itemIndex columnIndex keyState
    --say "ProductListView-itemSelected: item selected is:"lvProducts~selected
    if itemIndex > -1 then self~enableControl("IDC_PRODLIST_SHOWPRODUCT")
    else self~disableControl("IDC_PRODLIST_SHOWPRODUCT")


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD openItem UNGUARDED
    --say "ProductListView-openItem-01: item selected =" item
    self~showProduct

  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD showProduct UNGUARDED
    expose lvProducts rootDlg
    item = lvProducts~selected
    --say "ProductListView-showProduct-01: item selected =" item
    if item = -1 then do		-- if no item selected.
      ret = MessageDialog(.HRSplv~nilSelected, self~hwnd, title, 'WARNING')
      return
    end
    info=.Directory~new
    if lvProducts~getItemInfo(item, info) then do
      --say "ProductListView-showProduct-02: info~text =" info~text
      --say "ProductListView-showProduct-03; root =" root
      objectMgr = .local~my.ObjectMgr
      objectMgr~showModel("ProductModel", info~text, rootDlg)		--Ex07
      --say "ProductListView-showProduct-04: after startProductView"
      self~disableControl("IDC_PRODLIST_SHOWPRODUCT")
    end
    else do
      say "ProductListView-showProduct-05: ~getItemInfo returned .false."
    end

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD loadList
    expose lvProducts modelData						-- Ex07
    --say "ProductListView-loadList-01: dataArray =" modelData		-- Ex07
    rows = modelData[count]						-- Ex07 - number of rows
    arrData = modelData[records]
    --say "ProductListView-loadList-02:Dims =" arrData~dimension(1) arrData~dimension(2)
    do i = 1 to rows
      -- Change file price to display format (i.e. cents to dollars.cents):
      displayPrice = (arrData[i,3]/100)~format(,2)
      lvProducts~addRow( , ,arrData[i,1], arrData[i,2], displayPrice, arrData[i,4])
    end
    lvProducts~setColumnWidth(1)	-- set width of 2nd column to longest text entry.

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  HRSplv (Human-Readable Strings for ProductListView)		  v01-00 06Jun12
  ------
  The HRS class provides constant character strings for user-visible messages
  issued by the ProductListView class.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


::CLASS HRSplv PRIVATE		-- Human-Readable Strings
  ::CONSTANT noMenu       "This menu item is not yet implemented."
  ::CONSTANT newProd      "New Product"
  ::CONSTANT helpAbout    "Help - About"
  ::CONSTANT nilSelected  "Please select an item first."

/*============================================================================*/


