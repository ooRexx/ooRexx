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
   Exercise 06: The OrderFormView class				  v00-04 12Oct11
   OrderFormView.rex

   Contains: class "OrderFormView", class "HRS".
   Pre-requisite files: OrderFormView.rc, OrderFormView.h.

   Changes:
   v00-01 25Aug11: First version.
   v00-02 28Sep11: Changed to be the beginnings of a "new order" - will be a
   		   tabbed dialog. Old order form is now view got from OrderList.
   v00-03 30Sep11: Added msgbox for when user hits Esc (cancel). Also added
                   HRS class for user-visible strings.
   v00-04 12Oct11: Changed DoThis menu item and added two more - now have
   		   Cancel/Place/Save Order. Also added "not implemented" message
   		   for each menu item.
------------------------------------------------------------------------------*/

::requires "ooDialog.cls"


/*==============================================================================
  OrderFormView							  v00-02 28Sep11
  -------------
  The "view" (or "gui") Data Entry part of the Sales Order component.

  interface iOrderFormView {
    void new();
    void activate();
  }
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderFormView SUBCLASS RcDialog PUBLIC

  ::METHOD newInstance CLASS PUBLIC
    use arg rootDlg, orderNo
    .Application~useGlobalConstDir("O","Order\OrderFormView.h")
    dlg = self~new("Order\OrderFormView.rc", "IDD_ORDFORM_DIALOG")
    --say ".OrderFormView-newInstance: rootDlg =" rootDlg
    dlg~activate(rootDlg, orderNo)

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    -- creates the dialog instance but does not make it visible.
    expose menuBar
    say "OrderFormView-init-01"

    forward class (super) continue

    if \ self~createMenuBar then do		-- if there was a problem
      self~initCode = 1
      return
    end


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD createMenuBar
    -- Creates the menu bar on the dialog.
    expose menuBar
    say "OrderFormView-createMenuBar-01"
    menuBar = .ScriptMenuBar~new("Order\OrderFormView.rc", IDR_ORDFORM_MENU, , , .true, self)

    return .true


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate unguarded
    use arg rootDlg
    -- Shows the Dialog - i.e. makes it visible to the user.
    say "OrderFormView-activate-01"
    if rootDlg = "SA" then self~execute("SHOWTOP","IDI_ORDFORM_DLGICON")
    else self~popUpAsChild(rootDlg,"SHOWTOP",,"IDI_ORDFORM_DLGICON")
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    -- Called by ooDialog after SHOWTOP.
    expose menuBar custControls
    say "OrderFormView-initDialog-01"

    menuBar~attachTo(self)

    btnCancelOrder = self~newPushButton("IDC_CANCEL")
    btnPlaceOrder = self~newPushButton("IDC_ORDFORM_PLACEORDER")
    self~connectButtonEvent("IDC_CANCEL","CLICKED",cancel)
    self~connectButtonEvent("IDC_ORDFORM_PLACEORDER","CLICKED",placeOrderBtn)

    return

  /*----------------------------------------------------------------------------
    Event-Handler Methods - Menu Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD placeOrder UNGUARDED
    self~noMenuFunction(.HRS~ofPlaceOrder)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD saveOrder UNGUARDED
    self~noMenuFunction(.HRS~ofSaveOrder)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD CancelOrder UNGUARDED
    self~cancel

  /*- - Help - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    self~noMenuFunction(.HRS~ofHelpAbout)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD noMenuFunction UNGUARDED
    use arg title
    ret = MessageDialog(.HRS~ofNoMenu, self~hwnd, title, 'WARNING')

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    -- "Cancel" - This method over-rides the default Windows action of
    -- 'cancel window' for an Escape key.
  ::METHOD cancel
    response = askDialog(.HRS~ofQExit, "N")
    if response = 1 then forward class (super)
    return


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Button Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD placeOrderBtn UNGUARDED
    ret = MessageDialog(.HRS~ofNoBtn, self~hwnd, "Place Order Button", 'WARNING')

/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  Human-Readable Strings (HRS)					  v00-02 12Oct11
  --------
   The HRS class provides constant character strings for user-visible messages.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS HRS PRIVATE		-- Human-Readable Strings
  ::CONSTANT ofQExit       "Are you sure you want to cancel this Order and throw away all changes?"
  ::CONSTANT ofNoMenu      "This menu item is not yet implemented."
  ::CONSTANT ofNoBtn       "This button is not yet implemented."
  ::CONSTANT ofPlaceOrder  "Place Order"
  ::CONSTANT ofSaveOrder   "Save Order"
  ::CONSTANT ofCancelOrder "Cancel Order"
  ::CONSTANT ofHelpAbout   "Help - About"

/*============================================================================*/

