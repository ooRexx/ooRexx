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
   Exercise 06: The OrderView class				  v00-03 28Sep11
   OrderFormView.rex

   Contains: class "OrderView".
   Pre-requisite files: OrderView.rc, OrderView.h.

   Description: A sample Order View component - part of the sample
        	Order Management application.
        	This is a "leaf" component - invoked by OrderListView.

   Outstanding Problems: None reported.

   Changes:
   v00-01 25Aug11.
   v00-02 19Sep11: Corrected standalone invocation.
   v00-03 28Sep11: Minor mod to comment.

------------------------------------------------------------------------------*/

::REQUIRES "ooDialog.cls"
::REQUIRES "Order\OrderModelData.rex"


/*==============================================================================
  OrderFormView							  v00-02 19Sep11
  -------------
  The "view" (or "gui") part of the Order component - part of the sample
  Order Management application.

  interface iOrderView {
    void new();
    void activate();
  }
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderView SUBCLASS RcDialog PUBLIC

  ::METHOD newInstance CLASS PUBLIC
    expose rootDlg
    use arg rootDlg, orderNo
    say ".OrderView-newInstance: rootDlg =" rootDlg
    .Application~useGlobalConstDir("O","Order\OrderView.h")
    dlg = self~new("Order\OrderView.rc", "IDD_ORDER_DIALOG")
    dlg~activate(rootDlg, orderNo)

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    -- creates the dialog instance but does not make it visible.
    expose menuBar
    say "OrderView-init-01"

    forward class (super) continue

    if \ self~createMenuBar then do		-- if there was a problem
      self~initCode = 1
      return
    end


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD createMenuBar
    -- Creates the menu bar on the dialog.
    expose menuBar
    say "OrderView-createMenuBar-01"
    menuBar = .ScriptMenuBar~new("Order\OrderView.rc", IDR_ORDER_MENU, , , .true, self)

    return .true


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate unguarded
    use arg rootDlg
    -- Shows the Dialog - i.e. makes it visible to the user.
    say "OrderView-activate-01"
    if rootDlg = "SA" then self~execute("SHOWTOP","IDI_ORDER_DLGICON")
    else self~popUpAsChild(rootDlg,"SHOWTOP",,"IDI_ORDER_DLGICON")
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    -- Called by ooDialog after SHOWTOP.
    expose menuBar custControls
    say "OrderView-initDialog-01"

    menuBar~attachTo(self)

    return
