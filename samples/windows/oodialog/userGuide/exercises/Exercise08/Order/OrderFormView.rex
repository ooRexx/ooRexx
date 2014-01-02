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
   Exercise 08: The OrderFormView class				  v03-01 06Jun13
   OrderFormView.rex

   Contains: class "OrderFormView", class "HRSofv".
   Pre-requisite files: OrderFormView.rc, OrderFormView.h.

   Changes:
     v01-00 07Jun12: First version.
     v02-00 08Jan13: OrderFormView Modified to use the Model-View Framework (MVF).
                     Removed stand-alone startup (not now needed).
            25Feb13: Added control dialogs in tab sheet.
            27Feb13: Made Order Date functional.
            01Apr13: After ooDialog 4.2.2, Support folder moved to exercise
                     folder, so change to ::Requires needed.
     v03-00 27Apr13: Add ability to populate Order with Customer Details and
                     Order Lines.
            25May13: Now inherits directly from RcDialog plus the View &
                     Component mixins.
     v03-01 06Jun13: Added drag/drop code (method 'dmDrop') to make
                     OrderFormView a target. Also added code to the Customer
                     Details control dialog so that Customer can be provided by
                     drag/drop or by entering Cutomer Number.

------------------------------------------------------------------------------*/


.Application~addToConstDir("Order\OrderFormView.h")


::REQUIRES "ooDialog.cls"
::REQUIRES "support\View.rex"						-- v03

/*==============================================================================
  OrderFormView							  v02-01 06Jun13
  -------------
  The "view" (or "gui") Data Entry part of the Sales Order component.

  Changes:
  v01-00 07Jun12: First Version
  v02-00 05Oct12: Modified to use the Model-View Framework (MVF) including
                  removal of stand-alone startup.
         27Feb13: Changed to show tabs (control dialogs).
                  Corrected the Order Date control and limited order date to
                    one year ahead.
                  Commented-out say's.
         20May13: Now inherits directly from RcDialog plus the View & Component mixins
  v02-01 06Jun13: Added drag/drop code to make OrderFormView a target.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

--::CLASS OrderFormView SUBCLASS RcView PUBLIC
::CLASS OrderFormView SUBCLASS RcDialog PUBLIC INHERIT View Component	-- v03

  ::ATTRIBUTE tabContent
  ::ATTRIBUTE orderTotals

  ::METHOD newInstance CLASS PUBLIC
    use strict arg idModel, rootDlg
    --say; say ".OrderFormView-newInstance: rootDlg =" rootDlg
    dlg = self~new("Order\OrderFormView.rc", "IDD_ORDFORM_DIALOG")
    dlg~activate(idModel, rootDlg)
    return dlg

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    -- creates the dialog instance but does not make it visible.
    expose menuBar
    --say "OrderFormView-init-01"

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
    --say "OrderFormView-createMenuBar-01"
    menuBar = .ScriptMenuBar~new("Order\OrderFormView.rc", IDR_ORDFORM_MENU, , , .true)

    return .true


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate unguarded
    expose rootDlg idModelInstance orderData cd1 cd2 orderTotal custDiscount -
           taxRate eventMgr
    use strict arg idModelInstance, rootDlg
    forward class (super) continue
    orderData = RESULT

    -- set up tabs for Customer Details and Order Lines:
    cd1 = .CustomerDetailsDlg~new("Order\OrderFormView.rc", IDD_ORDFORM_CUST_DIALOG)
    cd2 = .OrderLinesDlg~new("Order\OrderFormView.rc", IDD_ORDFORM_ORDLINES_DIALOG)
    tabContent = .array~of(cd1, cd2)
    --say "OrderFormView-activate-01: tabContent =" tabContent[1]||"," tabContent[2]
    cd1~ownerDialog = self
    cd2~ownerDialog = self
    self~prep(tabContent)

    -- Send OrderFormView dlg id to the two Control Dialogs so that they can
    --   communicate with OrderFormView.
    cd1~setOrderFormDlg(self)
    cd2~setOrderFormDlg(self)
    cd2~rootDialog(rootDlg)		-- Tell cd2 what the root dialog is.

    -- Set up Order Totals and initialise CustDiscount:
    orderTotal = 0
    --orderTotals = .OrderTotals~new
    custDiscount = 0		-- Default customer discount
    taxRate = 0.05		-- 5% tax on discounted order total

    -- Tell EventMgr that we want to know when app closes:
    --eventMgr = .local~my.EventMgr
    --r = eventMgr~registerInterest("appClosing",self)
    self~registerInterest("appClosing",self)
    --say "OrderFormView-activate-03: eventMgr response =" r

    self~popUpAsChild(rootDlg,"SHOWTOP",,"IDI_ORDFORM_DLGICON")
    --say "OrderFormView-activate-04."
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD initDialog
    -- Called by ooDialog after SHOWTOP.
    expose menuBar ecOrderNo orderData tabContent tabControl orderDate -
           stCost stDisc stDiscCost stTax stTot orderTotals controlDialogsClosed
    --say "OrderFormView-initDialog-01"

    menuBar~attachTo(self)

    -- Tab stuff starts:
    cd1 = tabContent[1]
    cd2 = tabContent[2]
    cd1~execute
    cd2~execute

    -- Add the tabs to the tab control.
    tabControl = self~newTab(IDC_ORDFORM_TABS)
    tabControl~addSequence("Customer Details", "Order Lines")
    -- tab stuff ends

    ecOrderNo = self~newEdit("IDC_ORDFORM_ORDNO")
    --say "OrderFormView-initDialog-02: ecOrderNo =" ecOrderNo
    btnCancelOrder = self~newPushButton("IDC_CANCEL")
    btnPlaceOrder = self~newPushButton("IDC_ORDFORM_PLACEORDER")
    self~connectButtonEvent("IDC_CANCEL","CLICKED",cancel)
    self~connectButtonEvent("IDC_ORDFORM_PLACEORDER","CLICKED",placeOrderBtn)

    -- Get proxy for Order date and set its format. By default, it shows today.
    --Also, set allowable date range selected to between today and 1 year's time.
    orderDate = self~newDateTimePicker(IDC_ORDFORM_DATE);
    orderDate~setFormat("MMM dd',' yyyy")
    today = .DateTime~today
    maxOrderDate = today~addYears(1)
    orderDate~setRange(.array~of(today,maxOrderDate))
    -- Get proxies for Order Total Amounts:
    stCost =     self~newStatic("IDC_ORDFORM_TOTCOST")
    stDisc =     self~newStatic("IDC_ORDFORM_TOTDISC")
    stDiscCost = self~newStatic("IDC_ORDFORM_TOTDISCCOST")
    stTax =      self~newStatic("IDC_ORDFORM_TOTTAX")
    stTot =      self~newStatic("IDC_ORDFORM_ORDTOT")

    ecOrderNo~setText(orderData[formNumber])

    -- Tab stuff starts:
    -- Determine the position and size of the display area of the tab control.
    self~calculateDisplayArea
    -- Position and show the control dialog used for the first page of the tab.
    self~positionAndShow(1)
    -- tab stuff ends
    controlDialogsClosed = .false
/*
    Following did not work - it gave Cust Details on both tabs!!
    cd2 = tabContent[2]
    say "***** cd2 =" cd2
    cd2~ownerDialog = self	-- trial 21:45
    cd2~execute
*/
    -- Set as target for Drag/Drop:
        self~dmSetAsTarget:super()

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    -- This method not used - message goes to the class. Left in just for the
    -- time being. Delete when all drag/drop OK.
  --::METHOD dmQueryDrop
    --use strict arg sourceDlg, mousePos	-- try also without mousepos.
    --say "OrderFormView-dmQueryDrop-01."
    --return .true

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD dmDrop PUBLIC
    expose cd1 cd2
    use strict arg sourceModel, sourceDlg
    --say "OrderFormView-dmDrop-01; sourceModel, sourceDlg =" sourceModel||"," sourceDlg
    --say "OrderFormView-dmDrop-02: cd1, cd2 =" cd1||"," cd2
    parse var sourceModel . modelName
    select
      when modelName = "CUSTOMERMODEL" then do
        cd1~getCustomer(sourceModel); return .true
        end
      when modelName = "PRODUCTMODEL" then do
        --say "OrderFormView-dmDrop-03: Product dropped.";
        cd2~getProduct(sourceModel);  return .true
        end
    end
    return .false

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showTotals PUBLIC
    expose custDiscount taxRate stCost stDisc stDiscCost stTax stTot orderTotal
    use arg orderLineAmount
    --if custDiscount = "CUSTDISCOUNT" then custDiscount = 0  -- If user enters products first.
    --say "OrderFormView-showTotals-01: custDiscount, orderLineAmount =" custDiscount orderLineAmount

    orderTotal = orderTotal + orderLineAmount
    discount = (orderTotal * custDiscount)~format(,0)
    --say "OrderFormView-showTotals-01: discount =" discount
    discountedTotal = orderTotal - discount
    tax = (discountedTotal * taxRate)~format(,0)
    finalTotal = discountedTotal + tax
    --say "OrderFormView-showTotals-02: discount / tax =" discount||" / "||tax

    -- Format numbers from nnnnn to nnn.nn for display:
    x = myFormat(orderTotal); --say "OrderFormView-showTotals-03:" x
    stCost~setText(myFormat(orderTotal))
    --stCost~setText(         (orderTotal/100)~format(,2))
    stDisc~setText(myFormat(discount))
    --if discount = 0 then do; stDisc~setText("0.00"); end
    --else do; stDisc~setText((discount/100)~format(,2)); end
    stDiscCost~setText(myFormat(discountedTotal))
    --stDiscCost~setText(     (discountedTotal/100)~format(,2))
    stTax~setText(myFormat(tax))
    --stTax~setText(          (tax/100)~format(,2))
    stTot~setText(myFormat(finalTotal))
    --stTot~setText(          (finalTotal/100)~format(,2))



  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD setCustDiscount
    expose custDiscount
    use arg custDiscount
    --say "OrderFormView-setDustDiscount-01: discount =" custDiscount
    -- Use only first character - A, B or C:
    code = custDiscount~left(1)
    select
      when code = "A" then custDiscount = 0.15  -- 15% discount
      when code = "B" then custDiscount = 0.1   -- 10% discount
      when code = "C" then custDiscount = 0.05  -- 5%  discount
      otherwise            custDiscount = 0
    end
    --say "OrderFormView-setDustDiscount-01: discount =" custDiscount

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  --::METHOD updateTotals


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Button Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD placeOrderBtn UNGUARDED
    ret = MessageDialog(.HRSofv~NoBtn, self~hwnd, "Place Order Button", 'WARNING')


  /*----------------------------------------------------------------------------
    Event-Handler Methods - Menu Events
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD placeOrder UNGUARDED
    self~noMenuFunction(.HRSofv~PlaceOrder)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD saveOrder UNGUARDED
    self~noMenuFunction(.HRSofv~SaveOrder)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD CancelOrder UNGUARDED
    self~cancel

  /*- - Help - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD about UNGUARDED
    self~noMenuFunction(.HRSofv~HelpAbout)

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD noMenuFunction UNGUARDED
    use arg title
    ret = MessageDialog(.HRSofv~NoMenu, self~hwnd, title, 'WARNING')

  /*----------------------------------------------------------------------------
    Methods to tidy up when Order Form is closed.
   ---------------------------------------------------------------------------*/

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Cancel: This method over-rides the default Windows action of 'cancel window'
            for an Escape key. 'endExecution' (via the closeControlDialogs method
            is required else dialog hangs when user tried to close.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD cancel
    expose tabContent controlDialogsClosed
    --say "OrderFormView-cancel-01."
    if controlDialogsClosed = .true then do
      --say "OrderFormView-cancel-02: Control Dialogs closed."
      self~deRegisterInterest("appClosing",self) 		-- de-register interest in any events
      return self~cancel~super
    end
    else do -- Control dialogs not yet cancelled
      response = askDialog(.HRSofv~QExit, "N")
      --say "OrderFormView-cancel-03: response =" response
      if response = 1 then do   		   -- '1' means the 'Yes' button pressed
        self~deRegisterInterest("appClosing",self) 		-- de-register interest in any events
        self~closeControlDialogs
        return self~cancel:super
      end
      -- if response = 0 then do nothing - user chnaged his/her mind about closing.
    end

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD ok
    -- Invoked when enter key pressed - if passed to superclass, cancels dialog.
    say "OrderFormView-ok-01."
    return  -- do not close dialog - appears as a no-op to the user.


  /*----------------------------------------------------------------------------
    notify - Invoked by the Event Manager when a registered event occurs.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD notify PUBLIC
    expose controlDialogsClosed
    use strict arg event
    --say "OrderFormView-notify-01: event =" event
    if event = "appClosing" then do
      self~closeControlDialogs
      controlDialogsClosed = .true
    end


  /*----------------------------------------------------------------------------
    closeControlDialogs.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD closeControlDialogs PRIVATE
    expose tabContent
    do dlg over tabContent
      dlg~endExecution(.false)
    end


  /*----------------------------------------------------------------------------
    myFormat - A routine to format numbers into currency - e.g. converts
               "123456" into "1.234.56".
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::ROUTINE myFormat
    use arg number
    number = number~right(9,0)	-- left-pad with zeros
    parse var number mill 2 thou 5 hun 8 dec
    decs = "."||dec
    select
      when mill > 0 then number = mill||","||thou||","||hun||decs
      when thou > 0 then number = thou~strip(,0)||","||hun||decs
      when hun  > 0 then number = hun~strip(,0)||decs
      otherwise number = "0"||decs
    end
    return number
  /*--------------------------------------------------------------------------*/


  /*----------------------------------------------------------------------------
    deRegisterInterest - tell Event Manager that any events registered are no
                         longer of interest.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*  ::METHOD deRegisterInterest PRIVATE
    expose eventMgr
    --say "OrderFormView-deRegisterInterest-01."
    eventMgr~deRegisterInterest("appClosing",self)
*/

  /*----------------------------------------------------------------------------
    leaving - invoked by ooDialog when a dialog closes - but not when it's
              closed by closing the app - i.e. closing OrderMgrView.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD leaving UNGUARDED
    expose tabcontent cd1 cd2
    --say "OrderFormView-leaving-01."
/*    do dlg over tabContent
      dlg~endExecution(.false)
    end
*/    --forward class (super) continue


  /*----------------------------------------------------------------------------
    Methods to set up Control Dialogs
   ---------------------------------------------------------------------------*/

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    calculateDisplayArea
  - -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD calculateDisplayArea PRIVATE
    expose tabControl displayRect

    -- Given a rectangle describing the tab control's size and position, the tab
    -- control itself will calculate the display area's size and position.
    r = tabControl~windowRect
    tabControl~calcDisplayRect(r)

    -- Save the size of the display area, we need it later.
    s = .Size~new(r~right - r~left, r~bottom - r~top)

    -- Now we need to map the display area's position on the screen, to the client
    -- co-ordinates of the main dialog. The control dialog(s) are children windows
    -- of the main dialog, which is why we need to use the client-area of the
    -- dialog, not the client area of the tab control.
    p = .Point~new(r~left, r~top)
    self~screen2client(p)

    -- Create our display rectangle.  This is used in setWindowPosition(), which
    -- takes a point / size rectangle.  ooDialog defines a point / size rectangle
    -- as using the left and top attributes for the position of the upper left
    -- corner of a rectangle, using the right attribute for the width of the
    -- rectangle, and using the bottom attribute for the height of the rectangle.
    displayRect = .Rect~new(p~x, p~y, s~width, s~height)
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    positionAndShow()
    Used to resize and reposition one of the control dialogs so it occupies
    the display area of the tab control.
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD positionAndShow private
    expose tabControl tabContent displayRect lastSelected havePositioned
    use strict arg index
    --say "OrderFormView-positionAndShow-01; index =" index
    -- We can not position the control dialog until the underlying Windows dialog
    -- is created. If the system is heavily loaded for some reason, this may not
    -- have happened yet.  We need to wait for it.
    dlg = tabContent[index]
    do i = 1 to 10
      if dlg~hwnd <> 0 then leave
      z = SysSleep(.01)
    end
    --say "OrderFormView-positionAndShow-02."
    if dlg~hwnd == 0 then do
      say "Error creating dialog for the tab with index:" index", aborting"
      return self~cancel
    end

    if lastSelected <> 0 then tabContent[lastSelected]~hide

    -- Now resize and reposition the control dialog to the tab control's display
    -- area.  We need to position the control dialog *above* the tab control in
    -- the Z-order so that it shows.
    dlg~setWindowPos(tabControl~hwnd, displayRect, "SHOWWINDOW NOOWNERZORDER")
    --say "OrderFormView-positionAndShow-03."; say

    lastSelected = index
    havePositioned[index] = .true
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    onNewTab - Invoked when user selects another tab.
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD onNewTab
    expose tabControl tabContent havePositioned lastSelected
    --say "OrderFormView-onNewTab-01."
    index = tabControl~selectedIndex + 1
    --say "OrderFormView-onNewTab-02: index =" index
    dlg = tabContent[index]
    --say "OrderFormView-onNewTab-03: dlg, havePositioned[index] =" dlg havePositioned[index]

    if havePositioned[index] then do
      last = tabContent[lastSelected]
      last~hide
      dlg~show
      lastSelected = index
    end
    else do
      --dlg~ownerDialog = self
      --dlg~execute
      self~positionAndShow(index)
    end
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD prep
    expose tabContent lastSelected havePositioned
    use strict arg tabContent
    --say "OrderFormView-prep-01."
    -- The havePositioned array is used to determine if the page dialogs have been
    -- positioned or not.  Mark all 5 dialogs as not having been positioned yet.
    havePositioned = .array~of(.false, .false)
    -- No tab has been selected yet
    lastSelected = 0

    -- Connect the event handling methods to the events we are interested in.
    --self~connectButtonEvent(IDC_PB_PREVIOUS, CLICKED, onPrevious)
    --self~connectButtonEvent(IDC_PB_NEXT, CLICKED, onNext)
    self~connectTabEvent(IDC_ORDFORM_TABS, SELCHANGE, onNewTab)
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */




/*============================================================================*/



/*==============================================================================
  CustomerDetailsDlg - a Page in the OrderFormView
  ------------------
  The "view" (or "gui") Customer Ordering part of the OrderFormView component.
  ----------------------------------------------------------------------------*/
  ::CLASS CustomerDetailsDlg SUBCLASS RcControlDialog

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    initDialog
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD initDialog
    expose  pbFindCust ecCustNum ecCustName ecCustAddr ecCustDisc objectMgr
    --say "OrderFormView/CustomerDetailsDlg-initDialog-01: tabControl =" tabControl

    -- Get ObjectMgr object id for later use.
    objectMgr = .local~my.objectMgr

    ecCustNum   = self~newEdit("IDC_CUSTDTLS_NUM")
    ecCustName  = self~newEdit("IDC_CUSTDTLS_NAME")
    ecCustAddr  = self~newEdit("IDC_CUSTDTLS_ADDR")
    ecCustDisc  = self~newEdit("IDC_CUSTDTLS_DISC")
    pbFindCust = self~newPushButton("IDC_CUSTDTLS_FIND")
    -- pfFindCust is disabled in the .rc file, and is enabled when the focus
    -- is placed on the Customer Number field. The button is disabled when pushed.
    self~connectEditEvent("IDC_CUSTDTLS_NUM","GOTFOCUS",custNumGotFocus)
    self~connectButtonEvent("IDC_CUSTDTLS_FIND","CLICKED",findCustomer)
    --say "CustomerDetailsDlg-initDialog-01."

  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    setOrderFormDlg - Invoked by OrderFormView dialog so that this Control Dialog
                      can communicate with OrderFormView.
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD setOrderFormDlg
    expose dlgOrderForm
    use arg dlgOrderForm
  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    CustNumGotFocus - invoked when user puts focus on Customer Number field,
                      in which case the "Find Customer" pushbutton is enabled.
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD CustNumGotFocus UNGUARDED
    expose pbFindCust
    --say "CustomerDetailsDlg-CustNumGotFocus-01."
    pbFindCust~style = "DEFPUSHBUTTON"
    pbFindCust~enable
    --self~focusControl("IDC_CUSTDTLS_FIND")
  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    findCustomer - invoke when the "Find Customer" button is pressed.
                   Gets Customer details for the Cust Number in ecCustNum.
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD findCustomer UNGUARDED
    expose ecCustNum ecCustName ecCustAddr ecCustDisc pbFindCust objectMgr dlgOrderForm
    --say "CustomerDetailsDlg-findCust-01."
    custNo = ecCustNum~getLine(1)
    idCust = objectMgr~getComponentId("CustomerModel",custNo)
    --say "OrderFormView/CustomerDetailsDlg-findCustomer-01: idCustNo =" idCust
    if idCust = .false then do
      r = ErrorDialog(.HRSofv~noCust)
      pbFindCust~disable
      return
    end
    dirCustData = idCust~query
    if dirCustData = .false then do
      say "OrderFormView/CustomerDetailsDlg-findCustomer-02: query returned error."
      return
    end
    self~setCustomer(dirCustData)


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    getCustomer - invoked by main OrderFormView dialog when a Customer is
                  dropped on the Order Form.
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD getCustomer UNGUARDED
    expose ecCustNum
    use strict arg customerId
    dirCustData = customerId~query
    -- set Customer Number in dialog control - this not done by the setCustomer
    --   method because it's keyed in by the user when not using drag/drop.
    ecCustNum~setText(dirCustData["CustNo"])
    self~setCustomer(dirCustData)


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    setCustomer - invoke when a Customer is dropped on the Order Form.
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD setCustomer UNGUARDED
    expose ecCustNum ecCustName ecCustAddr ecCustDisc pbFindCust objectMgr dlgOrderForm
    use strict arg dirCustData
    -- Got Cust details - now populate controls (Name, Address, Discount):
    ecCustName~setText(dirCustData["CustName"])
    -- Replace commas with eols:
    strCustAddr = dirCustData["CustAddr"]~changeStr(",",.endOfLine)
    -- Add the zip:
    strCustAddr = strCustAddr||" "||dirCustData["Zip"]
    ecCustAddr~setText(strCustAddr)
    ecCustDisc~setText(dirCustData["CustDisc"])
    -- disble the "Find Customer" button.
    pbFindCust~disable
    -- Finally, tell the OrderFormView about what the Customer discount code is:
    dlgOrderForm~setCustDiscount(dirCustData["CustDisc"])
    -- Re-calc totals to take account of Customer Discount if Cust entered after
    -- order lines or a different Customer is selected half-way through the order.
    dlgOrderForm~showTotals(0)		-- provide ordeLineAmount as zero.
  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */


/*==============================================================================
  OrderLinesDlg - a Page in the OrderFormView
  -------------

  The "view" (or "gui") Products Ordering part of the OrderFormView.
  ----------------------------------------------------------------------------*/
::CLASS OrderLinesDlg SUBCLASS RcControlDialog INHERIT View

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    initDialog
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD initDialog
    expose objectMgr ecProdNum ecQty lvOrderItems

    -- Get ObjectMgr object id for later use.
    objectMgr = .local~my.objectMgr

    lvOrderItems = self~newListView("IDC_ORDLINES_LIST")
    lvOrderItems~addExtendedStyle(GRIDLINES FULLROWSELECT)
    lvOrderItems~insertColumnPX(0,"ProdNo",        50,"LEFT")
    lvOrderItems~insertColumnPX(1,"Product Name", 150,"LEFT")
    lvOrderItems~insertColumnPX(2,"UOM",           40,"RIGHT")
    lvOrderItems~insertColumnPX(3,"Qty",           40,"RIGHT")
    lvOrderItems~insertColumnPX(4,"Amount",        60,"RIGHT")
    --say "OrderLinesDlg-initDialog-01."

    -- Test an edit field:
    ecProdNum      = self~newEdit("IDC_ORDLINES_PRODNO")
    --say "OrderLinesDlg-initDialog-02; ecProdNum =" ecProdNum
    ecQty          = self~newEdit("IDC_ORDLINES_QTY")
    pbAddOrderLine = self~newPushButton("IDC_ORDLINES_ADD")
    self~connectEditEvent("IDC_ORDLINES_PRODNO","GOTFOCUS",prodNumGotFocus)
    self~connectButtonEvent("IDC_ORDLINES_ADD","CLICKED",addOrderLine)
    self~connectButtonEvent("IDC_ORDLINES_DELETE","CLICKED",deleteOrderLine)
    self~connectListViewEvent("IDC_ORDLINES_LIST","ACTIVATE",showProduct)	-- double-click

    --say "OrderLinesDlg-initDialog-03: ecProdNum =" ecProdNum
    -- Set focus on the Product Number field:
    self~focusControl("IDC_ORDLINES_PRODNO")
    --pbAddOrderLine~state = "FOCUS"
    self~initView		-- required by View mixin.

  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    setOrderFormDlg - Invoked by OrderFormView dialog so that this Control Dialog
                 can communicate with OrderFormView.
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD setOrderFormDlg
    expose OrderFormDlg
    use arg OrderFormDlg
    --say "OrderFormView/OrderLinesDlg-setOrderFormDlg-01."
  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    prodNumGotFocus
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD prodNumGotFocus UNGUARDED
    return
  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    addOrderLine - invoked when user presses the "Add OrderLine" button.
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD addOrderLine UNGUARDED
    expose ecProdNum ecQty lvOrderItems objectMgr OrderFormDlg
    --say "OrderFormView/OrderLinesDlg-addOrderLine-01."
    -- Get data that user has entered:
    prodNum    = ecProdNum~getLine(1)
    qtyOrdered = ecQty~getLine(1)
    --say "OrderFormView/OrderLinesDlg-addOrderLine-02: prodNum =" prodNum
    if qtyOrdered < 1 then do
      r = ErrorDialog(.HRSofv~noQty)
      return
    end
    -- Get product details from Product Model component:
    idProduct = objectMgr~getComponentId("ProductModel",prodNum)
    --say "OrderFormView/OrderLinesDlg-addOrderLine-03: idProduct =" idProduct
    if idProduct = .false then do
      r = ErrorDialog(.HRSofv~noProduct)
      --pbFindCust~disable
      return
    end
    dirProductData = idProduct~query
    if dirProductData = .false then do
      say "OrderFormView//OrderLinesDlg-addOrderLine-04: Product not found."
      return
    end

    -- State at this point: Product found and qty entered.

    -- Calculate Total Price:
    total = qtyOrdered*dirProductData["ListPrice"]
    --say "OrderFormView/OrderLinesDlg-addOrderLine-05: total =" total
    -- Ensure display total has 2 decimal places:
    --displayTotal = total/100~format(,2)
    displayTotal = myFormat(total)
    lvOrderItems~addRow( , , prodnum, dirProductData["ProdName"], -
                             dirProductData["UOM"], qtyOrdered, displayTotal)

    -- Send amount of Order to the OrderFormView dialog:
    OrderFormDlg~showTotals(total)

    -- Blank out fields ready for next order line:
    ecProdNum~settext("")
    ecQty~settext("")
    self~focusControl("IDC_ORDLINES_PRODNO")
  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    getProduct - invoked by main OrderFormView dialog when a Product is
                  dropped on the Order Form.
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD getProduct UNGUARDED
    expose ecProdNum
    use strict arg productId
    --say "OrderFormView/OrderLinesDlg-getProduct: ecProdNum =" ecProdNum
--trace i
    dirProdData = productId~query
    -- set Product Number in dialog control:
    ecProdNum~setText(dirProdData["ProdNo"])
    self~focusControl("IDC_ORDLINES_QTY")
--trace off

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    deleteOrderLine
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD deleteOrderLine
    expose lvOrderItems
    item = lvOrderItems~selected
    say "OrderFormView/OrderLinesDlg-01: item selected =" item
    if item = -1 then do		-- if no item selected.
      r = ErrorDialog(.HRSofv~noOrdLine)
      return
    end
    lvOrderItems~delete(item)

  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD rootDialog
    expose rootDlg
    use strict arg rootDlg

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    showProduct
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD showProduct UNGUARDED
    expose lvOrderItems rootDlg
    item = lvOrderItems~selected
    if item = -1 then do
      ret = MessageDialog(.HRSofv~nilSelected, self~hwnd, title, 'WARNING')
      return
    end
    info = .Directory~new
    if lvOrderItems~getItemInfo(item, info) then do
      --say "OrderLinesDlg-showProduct-01: info~text =" info~text "rootDlg =" rootDlg
      r = self~showModel:super("ProductModel", info~text, rootDlg)
    end
    else do
      say "OrderLinesDlg-showProduct-04: ~getItemInfo returned .false."
    end


  /*----------------------------------------------------------------------------
    leaving - invoked by ooDialog when a dialog closes.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD leaving UNGUARDED
    --expose objectMgr viewClass viewInstance
    --objectMgr~removeView(viewClass, viewInstance)
    --say "OrderFormView/OrderLinesDlg-leaving-01."
  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */

/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderTotals						          v01-00 03May13
  -----------
   This class is the set of Order Totals.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
  ::CLASS OrderTotals PUBLIC
    ::ATTRIBUTE amount
    ::ATTRIBUTE discount
    ::ATTRIBUTE discountedAmount
    ::ATTRIBUTE tax
    ::ATTRIBUTE taxedAmount

  ::METHOD init
    self~amount           = 0.00
    self~discount         = 0.00
    self~discountedAmount   = 0.00
    self~tax              = 0.00
    self~taxedAmount      = 0.00


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  PROBABLY REUNDANT!!
  Order Grid (OrderGrid)				          v01-00 03May13
  --------
   This class is a table or grid of Order Line Items.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

    ::CLASS OrderGrid PRIVATE

    ::ATTRIBUTE lines		-- Array of Order lines
    ::ATTRIBUTE totals
    ::ATTRIBUTE numLines

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    init - set up Order Grid
    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
  ::METHOD init
    self~lines  = .array~new
    self~totals = .array~new
    self~totals[1,1]=0;
    self~totals[2,1]=0; self~totals[2,2]=0
    self~totals[3,1]=0; self~totals[3,2]=0
    self~numLines = 0

  ::METHOD addLine
    use strict arg dirLine

  ::METHOD delLine
    use strict arg lineNo
  /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  Order Header		  				          v01-00 06May13
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
  ::CLASS OrderHeader
    ::attribute orderNumber
    ::attribute orderDate
    ::attribute custNo
    ::attribute custName
    ::attribute address
    ::attribute discount
/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  Order Line		  				          v01-00 06May13
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
  ::CLASS OrderLine
    ::attribute prodNo
    ::attribute prodName
    ::attribute UOM
    ::attribute qty
    ::attribute amount
/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  Human-Readable Strings (HRSofv)				  v01-00 07Jun12
  --------
   The HRSofv class provides constant character strings for user-visible messages.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS HRSofv PRIVATE	   -- Human-Readable Strings
  ::CONSTANT CancelOrder "Cancel Order"
  ::CONSTANT HelpAbout   "Help - About"
  ::CONSTANT nilSelected "Please select an item first."
  ::CONSTANT NoBtn       "This button is not yet implemented."
  ::CONSTANT NoCust      "Customer not found."
  ::CONSTANT NoOrdLine   "No Order Line selected."
  ::CONSTANT NoMenu      "This menu item is not yet implemented."
  ::CONSTANT NoProduct   "Product not found."
  ::CONSTANT NoQty       "No valid quantity."
  ::CONSTANT PlaceOrder  "Place Order"
  ::CONSTANT QExit       "Are you sure you want to cancel this Order and throw away all changes?"
  ::CONSTANT SaveOrder   "Save Order"
/*============================================================================*/

