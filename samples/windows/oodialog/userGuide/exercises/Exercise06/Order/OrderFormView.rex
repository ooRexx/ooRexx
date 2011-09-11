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
   Exercise 06: The OrderFormView class				  v00-01 25Aug11
   OrderFormView.rex

   Contains: class "OrderFormView".
   Pre-requisite files: OrderFormView.rc, OrderFormView.h.

   Changes:

------------------------------------------------------------------------------*/

::requires "ooDialog.cls"


/*==============================================================================
  OrderFormView							  v00-01 25Aug11
  -------------
  The "view" (or "gui") part of the OrderForm component - part of the sample
  Order Management application.

  interface iOrderFormView {
    void new();
    void activate();
  }
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderFormView SUBCLASS RcDialog PUBLIC

  ::ATTRIBUTE hasParent CLASS

  ::METHOD newInstance CLASS PUBLIC
    expose hasParent
    use arg parent, root, orderNo
    if parent = "SA" then hasParent = .false; else hasParent = .true
    .Application~useGlobalConstDir("O","Order\OrderFormView.h")
    dlg = self~new("Order\OrderFormView.rc", "IDD_ORDFORM_DIALOG")
    say ".OrderFormView-newInstance: root =" root
    dlg~activate(root, orderNo)

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
    use arg parentDlg
    -- Shows the Dialog - i.e. makes it visible to the user.
    say "OrderFormView-activate-01"
    -- version 01.00: self~execute("SHOWTOP")		-- MUST be last!
    --self~popup("SHOWTOP") -- This blocks the code that did the dlg~new.
    if .OrderFormView~hasParent then,
      self~popUpAsChild(parentDlg,"SHOWTOP",,"IDI_ORDFORM_DLGICON")
    else self~execute("SHOWTOP","IDI_ORDFORM_DLGICON")
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    -- Called by ooDialog after SHOWTOP.
    expose menuBar custControls
    say "OrderFormView-initDialog-01"

    menuBar~attachTo(self)

    return
