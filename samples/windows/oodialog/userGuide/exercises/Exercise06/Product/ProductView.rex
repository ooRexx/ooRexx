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
   Exercise 06: ProductView.rex - The ProductView component       v01-01 01Apr13

   Contains: 	   classes "ProductView", "AboutDialog", and "HRSpv".

   Pre-requisites: ProductView.dll, ProductView.h, Pproduct.ico, ProductIcon.bmp,
   		   Support\NumberOnlyEditEx.cls (copied from ooDialog Samples
   		   into the folder Exercise06\Support)

   Description: A sample Product View component - part of the sample
        	Order Management application.

   Outstanding Problems: None reported.

   Changes:
   v01-00 03Jun12: First version for Exercise05.
   v01-01 06Jun12: Minor changes for Exercise06.
          01Apr13: After ooDialog 4.2.2, Support folder moved to exercise
                   folder, so change to ::Requires needed.

------------------------------------------------------------------------------*/

.Application~addToConstDir("Product\ProductView.h")


::requires "ooDialog.cls"
::requires "Support\NumberOnlyEditEx.cls"
::requires "Product\ProductModelData.rex"


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ProductView							  v01-01 06Jun12
  -----------
  The "view" part of the Product component. Now designed to operate from its own
  folder. Should be invoked from immediately outside the Product folder.
  [interface (idl format)]

  Changes:
  v01-00 03Jun12: First version.
     v01-01 06Jun12: Minor changes for Exercise06.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS ProductView SUBCLASS ResDialog PUBLIC

  ::ATTRIBUTE dialogState PRIVATE	-- States are: 'closable' or 'inUpdate".

  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newInstance CLASS PUBLIC UNGUARDED
    use arg rootDlg, productNo			--ADDED FOR EXERCISE06.
    if parent = "SA" then hasParent = .false; else hasParent = .true
    --say ".ProductView-newInstance-01: rootDlg =" rootDlg
    .Application~addToConstDir("Product\ProductView.h")
    -- Create an instance of ProductView and show it:
    dlg = .ProductView~new("Product\res\ProductView.dll", IDD_PRODUCT_VIEW)
    dlg~activate(rootDlg, productNo)			-- CHANGED FOR EXERCISE06.


  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    --say "ProductView-init-01."
    -- called first (result of .ProductView~new)
    forward class (super) continue


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate UNGUARDED
    use arg rootDlg, productNo							--ADDED FOR EXERCISE06.
    --say "ProductView-activate-01: rootDlg =" rootDlg
    self~dialogState = "closable"
    if rootDlg = "SA" then self~execute("SHOWTOP","IDI_PROD_DLGICON")		--ADDED FOR EXERCISE06.
    else self~popUpAsChild(rootDlg,"SHOWTOP",,"IDI_PROD_DLGICON")		--ADDED FOR EXERCISE06.
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    expose menuBar prodControls prodData
    --say "ProductView-initDialog-01"

    menuBar = .BinaryMenuBar~new(self, IDR_PRODUCT_VIEW_MENU, , self, .true)

    prodControls = .Directory~new
    prodControls[ecProdNo]         = self~newEdit("IDC_PROD_NO")
    prodControls[ecProdName]       = self~newEdit("IDC_PROD_NAME")
    prodControls[ecProdPrice]      = self~newEdit("IDC_PROD_LIST_PRICE")
    prodControls[ecUOM]            = self~newEdit("IDC_PROD_UOM")
    prodControls[ecProdDescr]      = self~newEdit("IDC_PROD_DESCRIPTION")
    prodControls[gbSizes]          = self~newEdit("IDC_PROD_SIZE_GROUP")
    prodControls[rbSmall]          = self~newRadioButton("IDC_PROD_RADIO_SMALL")
    prodControls[rbMedium]         = self~newRadioButton("IDC_PROD_RADIO_MEDIUM")
    prodControls[rbLarge]          = self~newRadioButton("IDC_PROD_RADIO_LARGE")
    prodControls[pbSaveChanges]    = self~newPushButton("IDC_PROD_SAVE_CHANGES")
    self~connectButtonEvent("IDC_PROD_SAVE_CHANGES","CLICKED",saveChanges)

    -- Use NumberOnlyEditEx.cls to enforce numeric only entry for Price and UOM:
    prodControls[ecProdPrice]~initDecimalOnly(2,.false)		-- 2 decimal places, no sign.
    prodControls[ecUOM]~initDecimalOnly(0,.false)		-- 0 decimal places, no sign.
    prodControls[ecProdPrice]~connectCharEvent(onChar)
    prodControls[ecUOM]~connectCharEvent(onChar)

    prodData = self~getData	-- Gets data from ProductModel into prodData
    self~showData	-- Show the data
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    Event Handler Methods - MenuBar Events:
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD updateProduct UNGUARDED
    expose prodControls

    -- Enable the controls to allow changes to the data:
    prodControls[ecProdName]~setReadOnly(.false)
    prodControls[ecProdPrice]~setReadOnly(.false)
    prodControls[ecUOM]~setReadOnly(.false)
    prodControls[ecProdDescr]~setReadOnly(.false)
    prodControls[rbSmall]~enable
    prodControls[rbMedium]~enable
    prodControls[rbLarge]~enable
    self~enableControl("IDC_PROD_SAVE_CHANGES")
    prodControls[pbSaveChanges]~state = "FOCUS"  -- Put input focus on the button
    self~tabToNext()				 -- put text cursor on Product Description
    						 --   (as if the user had pressed tab)
    self~dialogState = "inUpdate"

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD refreshData UNGUARDED
    self~disableControl("IDC_PROD_SAVE_CHANGES")
    self~showData
    self~dialogState = "closable"

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD print UNGUARDED
    -- say "ProductView-print-01"
    ans = MessageDialog(.HRSpv~printMsg)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD close UNGUARDED
    --say "ProductView-close-01"
    return self~cancel:super

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    --say "ProductView-about-01"
    dlg = .AboutDialog~new("ProductView.dll", IDD_PRODUCT_VIEW_ABOUT)
    dlg~execute("SHOWTOP")


  /*----------------------------------------------------------------------------
    Event Handler Methods - PushButton Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    "Save Changes" - Collects new data, checks if there has indeed been a
                     change, and if not, issues a warning msg. Disables the
                     button when valid changes made.                          */
  ::METHOD saveChanges UNGUARDED
    expose prodControls prodData

    -- Transform data from view format (as in controls) to app format (a directory):
    newProdData = self~xformView2App(prodControls)

    -- Check if anything's changed; if not, show msgbox and exit with controls enabled.
    -- If changed, go on to validate data.
    result = self~checkForChanges(newProdData)
    if result = .false then do
      msg = .HRSpv~nilSaved
      hwnd = self~dlgHandle
      answer = MessageDialog(msg,hwnd,.HRSpv~updateProd,"OK","WARNING","DEFBUTTON2 APPLMODAL")
      return
    end

    -- Now validate data:
    result = self~validate(newProdData)		-- returns a null string or error messages.
         					-- Better would be a set of error numbers.
    -- If no problems, then show msgbox and go on to disable controls.
    if result = "" then do
      msg = .HRSpv~saved
      hwnd = self~dlgHandle
      answer = MessageDialog(msg,hwnd,.HRSpv~updateProd,"OK","INFORMATION","DEFBUTTON1 APPLMODAL")
    end
    -- If problems, then show msgbox and leave user to try again or refresh or exit.
    else do
      msg = result||.EndOfLine||.HRSpv~notSaved
      hwnd = self~dlgHandle
      answer = MessageDialog(msg,hwnd,.HRSpv~updateProd,"OK","ERROR","DEFBUTTON1 APPLMODAL")
      return
    end

    -- Send new data to be checked by CustomerModel object (not implemented).

    -- Disable controls that were enabled by menu "ActionsFile-->Update" selection:
    prodControls[ecProdName]~setReadOnly(.true)
    prodControls[ecProdDescr]~setReadOnly(.true)
    prodControls[ecProdPrice]~setReadOnly(.true)
    prodControls[ecUom]~setReadOnly(.true)
    if newProdData~size \= "S" then prodControls[rbSmall]~disable
    if newProdData~size \= "M" then prodControls[rbMedium]~disable
    if newProdData~size \= "L" then prodControls[rbLarge]~disable
    self~disableControl("IDC_PROD_SAVE_CHANGES")
    self~dialogState = "closable"

    prodData = newProdData
    prodData~list
    return

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    Event Handler Methods - Keyboard Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD onChar UNGUARDED
    -- called for each character entered in the price or UOM fields.
    forward to (arg(6))

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    "OK" - This is a no-op method that over-rides the default Windows action
           of 'close window' for an Enter key 				    --*/
  ::METHOD ok unguarded
    return

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD cancel
    -- If in the process of updating, then ask whether any changes should be
    -- thrown away and dialog closed. If yes then close by calling the superclass,
    -- else nop. If not in update, then close immediately
    if self~dialogState = "inUpdate" then do
      ans = MessageDialog(.HRSpv~closeInUpdate, self~dlgHandle, .HRSpv~updateIP, "YESNO", "WARNING", "DEFBUTTON2")
      if ans = .PlainBaseDialog~IDYES then return self~cancel:super
      else nop
    end
    else return self~cancel:super


  /*----------------------------------------------------------------------------
    Application Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getData
    -- Get data from the ProductModel:
    --expose prodData
    --say "ProductView-getData-01."
    idProductModel = .local~my.idProductModel
    prodData = idProductModel~query		-- prodData is of type ProductDT
    return prodData


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showData
    -- Transfrom data (where necessary) to display format, and then disable controls.
    expose prodControls prodData
    --say "ProductView-showData-01."
    -- Set data in controls:
    prodControls[ecProdNo]~setText(   prodData~number       )
    prodControls[ecProdName]~setText( prodData~name         )
    -- Price in prodData has no decimal point - 2 decimal places are implied - hence /100 for display.
    prodControls[ecProdPrice]~setText(prodData~price/100    )
    prodControls[ecUOM]~settext(      proddata~uom          )
    prodControls[ecProdDescr]~setText(prodData~description  )
    size = prodData~size
    -- Disable controls
    prodControls[ecProdName]~setReadOnly(.true)
    prodControls[ecProdPrice]~setReadOnly(.true)
    prodControls[ecUOM]~setReadOnly(.true)
    prodControls[ecProdDescr]~setReadOnly(.true)
    prodControls[rbSmall]~disable
    prodControls[rbMedium]~disable
    prodControls[rbLarge]~disable
    -- But check correct button and enable it to highlight it to the user:
    select
      when size = "S" then do
        .RadioButton~checkInGroup(self,"IDC_PROD_RADIO_SMALL","IDC_PROD_RADIO_LARGE","IDC_PROD_RADIO_SMALL")
        prodcontrols[rbSmall]~enable
      end
      when size = "M" then do
        .RadioButton~checkInGroup(self,"IDC_PROD_RADIO_SMALL","IDC_PROD_RADIO_LARGE","IDC_PROD_RADIO_MEDIUM")
        prodcontrols[rbMedium]~enable
      end
      otherwise do
        .RadioButton~checkInGroup(self,"IDC_PROD_RADIO_SMALL","IDC_PROD_RADIO_LARGE","IDC_PROD_RADIO_LARGE")
        prodcontrols[rbLarge]~enable
      end
    end


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD checkForChanges
    -- Check whether any data has actually changed when "Save Changes" button
    -- has been pressed. Return .true if data changed, else returns .false.
    expose prodData
    use arg newProdData

    changed = .false
    select
      when newProdData~name        \= prodData~name        then changed = .true
      when newProdData~price       \= prodData~price       then changed = .true
      when newProdData~uom         \= prodData~uom         then changed = .true
      when newProdData~description \= prodData~description then changed = .true
      when newProdData~size        \= ProdData~size        then changed = .true
      otherwise nop
    end
    return changed


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD validate
    -- Validation: 1. Check price/UOM not changed > 50% up or down.
    --             2. Cannot change from Large to Small without UOM increasing
    --                 by at least 100%; nor from Small to Large without
    --                 decreasing by more than 50%.
    --             3. Product Description <= 100 characters.
    --             4. Product Name <= 30 characters.
    --  Returns string of messages - the null string if all OK.
    expose prodData
    use arg newProdData
    msg = ""

    -- Check Price (catches decimal point errors also):
    price = prodData~price; newPrice = newProdData~price
    oldUom   = prodData~uom;       newUom   = newProdData~uom 	-- 'oldUom - avoids name conflict with 'uom' in newProddata~uom.
    if ((price/oldUom)*1.5 < newPrice/newUom) | (newPrice/newUom < (price/oldUom)/2) then do
      msg = msg||.HRSpv~badRatio||" "
    end

    -- Check Size vs UOM:
    if prodData~size = "L" & newProdData~size = "S" -    -- Large to Small
        & prodData~uom/2 < newProdData~uom then do
      msg = msg||.HRSpv~uomTooBig||" "
    end
      if prodData~size = "S" & newProdData~size = "L" -    -- Small to Large
        & prodData~uom*2 > newProdData~uom then do
      msg = msg||.HRSpv~uomTooSmall||" "
    end

    -- Check Product Description length:
    if newProdData.description~length > 80 then do
      msg = msg||.HRSpv~descrTooBig||" "
    end

    -- Check Product Name length:
    if newProdData~name~length > 30 then do
      msg = msg||.HRSpv~prodNameTooBig
    end

    return msg


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD xformView2App
    -- Transforms Product Data from View form (in the GUI controls) to
    --   App form (a directory with address as an array).
    expose prodControls
    prodData = .ProductDT~new
    prodData~number = prodControls[ecProdNo]~gettext()
    prodData~name = prodControls[ecProdName]~getText()
    price = prodControls[ecProdPrice]~getText()
    -- Data entered has or assumes a decimal point; but data in "application"
    --   is a whole number (e.g. $42.42 is recorded in the database as "4242").
    --   So re-format data from decimal to whole number:
    priceTwoDecs = price~format(,2)		-- force 2 dec positions
    -- Re-display price properly formatted (in case the user did not format correctly - e.g. entered "42" or "38.4"):
    prodControls[ecProdPrice]~setText(priceTwoDecs)
    -- Now format price to "application" format:
    price = (priceTwoDecs*100)~format(,0)	-- multiply by 100 and then force whole number.
    prodData~price = price
    prodData~uom  = prodControls[ecUOM]~getText()
    prodData~description = prodControls[ecProdDescr]~getText()
    select
      when prodControls[rbSmall]~checked then prodData~size = "S"
      when prodControls[rbMedium]~checked then prodData~size = "M"
      otherwise prodData~size = "L"
    end

    return prodData

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  AboutDialog							  v01-00 03Jun12
  -------------
  The "About" class - shows a dialog box that includes a bitmap - part of the
  ProductView component.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS AboutDialog SUBCLASS ResDialog

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::method initDialog
    expose font image
    resImage = .ResourceImage~new(self)			  -- Create an instance of a resource image
    image = resImage~getImage(IDB_PROD_ICON)			  -- Create an image from the Product bitmap
    stImage = self~newStatic(IDC_PRODABT_ICON_PLACE)~setImage(image) -- Create a static text control and set the image in it
    font = self~createFontEx("Ariel", 12)			  -- Create up a largish font with which to display text and ...
    self~newStatic(IDC_PRODABT_STATIC2)~setFont(font)		  -- ... set the static text to use that font.
    -- Provide for a double-click in Product icon:
    self~connectStaticNotify("IDC_PRODABT_ICON_PLACE", "DBLCLK", showMsgBox)
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::method showMsgBox
    --say "AboutDialog-showMsgBox-01."
    ans = MessageDialog(.HRSpv~AboutDblClick)
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::method leaving
    expose font image
    self~deleteFont(font)
    image~release()
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  Human-Readable Strings (HRSpv)				  v01-00 03Jun12
  ------------------------------
   The HRSpv class provides constant character strings for user-visible messages.

  Changes:
  v00-03 11Feb12: Changed class name NRS to HRSpv
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS HRSpv PRIVATE		-- Human-Readable Strings
  ::CONSTANT AboutDblClick  "You double-clicked!"
  ::CONSTANT badRatio	    "The new price/UOM ratio cannot be changed more than 50% up or down."
  ::CONSTANT closeInUpdate  "Any changes made will be lost. Exit anyway?"
  ::CONSTANT descrTooBig    "The Product Description is too long."
  ::CONSTANT nilSaved       "Nothing was changed! Data not saved."
  ::CONSTANT notSaved       "Changes Not Saved!"
  ::CONSTANT prodNameTooBig "The Product Name is too long."
  ::CONSTANT saved	    "Changes saved."
  ::CONSTANT uomTooBig      "The new UOM is too large."
  ::CONSTANT uomTooSmall    "The new UOM is too small."
  ::CONSTANT updateIP       "Update in process"
  ::CONSTANT updateProd     "Update Product"
  ::CONSTANT printMsg       "The 'Print...' menu item is not yet implemented."

/*============================================================================*/

