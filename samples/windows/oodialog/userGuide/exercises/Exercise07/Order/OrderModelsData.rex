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
   Exercise 07: The OrderModel and OrderData Classes	  	  v02-00 01Apr13

   Contains: 	   classes "OrderModel", OrderListModel, and "OrderData".  --Ex07
   Pre-requisites: None.

   Outstanding Problems:
   None.

   Changes:
   v01-00 07Jun12: First version.
   v02-00 08Jan13: Modified to use the MVF.
          01Apr13: After ooDialog 4.2.2, Support folder moved to exercise
                   folder, so change to ::Requires needed.

------------------------------------------------------------------------------*/


::REQUIRES "Support\Model.rex"
::REQUIRES "Support\GenericFile.rex"


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderModel							  v01-00 08Feb13
  ------------
  The "model" part of the Order component.

  Changes:
   v01-00 07Jun12: First version.
          24Aug12: Modified to use the MVF.
          11Jan13: Commented-out 'say' instructions.
          08Feb13: Minor changes to comments only.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderModel SUBCLASS Model PUBLIC				--Ex07

/*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    use strict arg instanceName						--Ex07
    forward class (super) continue					--Ex07
    modelId = RESULT							--Ex07
    return modelId							--Ex07


/*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    init - Superclass 'Model', in its newInstance method, queries the Data
           component for this instance's data, then does the self~new(myData)
           which invokes this 'init' method.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    expose orderData
    use strict arg orderData				-- Ex07: data provided by Superclass.
    self~myData = orderData				-- Ex07: store in superclass's attribute.
    --say "OrderModel-init-01: orderData =" orderData
    --say "OrderModel-init-02: self~myData =" self~myData

    -- MVF gives gets the Order's Header data for the OrderModel, and OrderDetails for
    -- the OrderListModel. Here we've got the Order Headers; now we need to ask
    -- the OrderData component for the Order Details for this Order only:
    --objMgr = .local.my~ObjectMgr
    --idOrderData = objMgr~getComponentId("ProductData", "The")
    --headerData = idOrderData~getHeaders( =

    return self								-- Ex07


--  ::METHOD query PUBLIC
--    use arg orderNo
--    if orderNo = "ORDERNO" then 	-- param not supplied
/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderFormModel						  v01-00 04Oct12
  --------------
  The model component for the OrderForm business component.

  Changes:
   v01-00 04Oct12.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderFormModel SUBCLASS Model PUBLIC

  ::ATTRIBUTE nextOrderNumber CLASS

/*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    newInstance -
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -.*/
  ::METHOD newInstance CLASS PUBLIC
    use strict arg instanceName				-- invoked by ObjectMgr
    --say ".OrderFormModel-newInstanceName-01."
    forward class (super) continue					--Ex07
    modelId = RESULT							--Ex07
    return modelId							--Ex07


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    getInstanceName - over-rides super's method.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -.*/
  ::METHOD getInstanceName CLASS PUBLIC
    --say ".OrderFormModel-getInstanceName-01."
    if self~nextOrderNumber = "NEXTORDERNUMBER" then do	-- No instance name set
      self~nextOrderNumber = "SO-4999"
    end
    number = self~nextOrderNumber~right(4)
    number += 1
    self~nextOrderNumber = "SO-"||number
    --say ".OrderFormModel-getInstanceName-02: instanceName =" self~nextOrderNumber
    return self~nextOrderNumber


  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    init -
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -.*/
  ::METHOD init
    expose orderData
    use strict arg orderData				-- Ex07: data provided by Superclass.
    self~myData = orderData				-- Ex07: store in superclass's attribute.
    --say "OrderModel-init-01: orderData =" orderData
    return self

/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderListModel						  v01-00 24Aug12
  --------------
  The "ListModel" for the SalesOrder business component.

  Changes:
   v01-00 24Aug12.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderListModel SUBCLASS Model PUBLIC

  ::ATTRIBUTE myData
/*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newInstance CLASS PUBLIC
      use strict arg instanceName
      self~wantList = .true			-- MVF - for List models only.
      forward class (super) continue		-- MVF: super does the ~new and
                                                --   passes data as a param on the init.
      modelId = RESULT				-- MVF
      --say ".OrderListModel-newInstance-01: id =" modelId
      return modelId				-- MVF - could just say 'return modelId'.


/*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    init - stores received data in attribute 'arrData' and returns self.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    expose arrData
    use strict arg arrData
    self~myData = arrData
    --say "OrderListModel-init-01: data = " self~myData
    --say "OrderListModel-init-02: type and dimensions:" self~myData self~myData~dimension
    return self


  /*----------------------------------------------------------------------------
    query - returns an array of all OrderHeader data.
            In MVF this method is invoked by the RcView (or ResView) superclass.
            But the list also needs the name of the Customer. So this model
            also gets the Customer Name from the appropriate Customer Model.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD query PUBLIC
    --say "OrderListModel-query-01: self~myData =" self~myData
    -- myData is an array of records. Now get Customer
    return self~myData

/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderData							  v01-00 07Jun12
  ------------
  The "data" part of the Order component.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS OrderData SUBCLASS GenericFile PUBLIC

  ::ATTRIBUTE created CLASS
  ::ATTRIBUTE dirOrderHeaders PUBLIC	-- a directory containing all records in
  					-- the SalesOrderHeaders.txt file joined
  					--  with Customer Name and Address from
  					--  the Customer file.
  ::ATTRIBUTE dirOrderLines PUBLIC	-- a directory containing all records in
  					-- the SalesOrderLines.txt file joined
  					-- with product Description from the
  					-- Product File.
/*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newInstance CLASS PUBLIC
    use strict arg instanceName
    if self~created = "CREATED" then do
      orderDataId = self~new()
      return orderDataId
    end
    else do
      say "... singleton component, so can't have more than one instance."
      return .false
    end


/*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    init - Invoked super to read the OrderHeader file, which is stored by super
           in super's 'fileAsDirectory' attribute - and also the 'raw' form of
           the file is stored in super's 'fileArray' attribute.
           The OrderDetail file is read separately by invoking super's 'readFile'
           method.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -.*/
  ::METHOD init								--Ex07
    expose filename numRecords
    -- Read the OrderHeaders file:
    filename = "Order\OrderHeadersFile.txt"; columns = 5
    -- invoke super to read the OrderHeaders file (read into super's
    -- fileAsDirectory attribute):
    numRecords = self~init:super(fileName, columns)
    self~dirOrderHeaders = self~fileAsDirectory
    -- List the OrderHeaders
    /*say "OrderData-init-01: numRecords:" numRecords
    if numRecords > 0 then do
      say "OrderData-init-02: Array is:"
      say self~fileArray~tostring		-- fileArray is super's attribute.
    end */
    -- Read the OrderLines file:
    self~dirOrderLines = self~readFile:super("Order\OrderLinesFile.txt", 3)

    -- At this point, the OrderHeader file is the dirOrderHeadersattribute,
    -- and the OrderLines file is in self~dirOrderLines.
    -- Now, for the OrderHeaders, do a 'join' with Customer to add the
    -- customer's name and address to the order headers in self~dirOrderHeaders:
    self~addCustomerInfo
    -- And now add Product info to the order lines in self~dirOrderLines:
    self~addProductInfo
    --self~listOrders
    return self								-- MVF


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    addCustomerInfo - Called by init - adds customer info from the CustomerData
    		      component to each of the OrderHeader lines.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -.*/
  ::METHOD addCustomerInfo PRIVATE
    -- (1) Add column headers (field names) from Customer to OrderHeaders:
    columnHeaders = self~dirOrderHeaders[headers]
    columnHeaders[6] = "CustName"; columnHeaders[7] = "CustDisc"
    columnHeaders[8] = "CustAddr"; columnHeaders[9] = "Zip"
    self~dirOrderHeaders[headers] = columnHeaders

    -- (2) Add values for custName, Discount, CustAddress and Zip for each order header line:
    arrData = self~dirOrderHeaders[records]
    --say "OrderData-getFile-02 arrData, dims =" arrData arrData~dimension
    -- (2a) First get id for CustomerData:
    objMgr = .local~my.ObjectMgr
    idCustData = objMgr~getComponentId("CustomerData", "The")
    if idCustData = .false then return .false
    -- (2b) For each order header, get the cust name & addr from CustomerData
    --      and add it to the OrderHeader in-memory file:
    do i=1 to self~dirOrderHeaders[count]		-- loop over the Order Headers records:
      -- get Customer record from CustomerData:
      --say "OrderData-getFile-04: arrOrderHeaders[i,2] =" arrOrderHeaders[i,2]
      orderCustNo = arrData[i,2]
      custDir = idCustdata~getRecord(orderCustNo)
      -- add customer's Name and Address to the end of the record:
      arrData[i,6] = custDir["CustName"]
      arrData[i,7] = custDir["CustDisc"]
      arrData[i,8] = custDir["CustAddr"]
      arrData[i,9] = custDir["Zip"]
    end
    self~dirOrderHeaders[records] = arrData
    return
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    addProductInfo - Called by init - adds product name from the ProductData
    		      component to each of the OrderLine records
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -.*/
  ::METHOD addProductInfo PRIVATE			-- Invoked by init.
    -- (1) Add column header from Product to the headers (field labels) array:
    columnHeaders = self~dirOrderLines[headers]
    columnHeaders[4] = "ProdName"
    self~dirOrderLines[headers] = columnHeaders

    -- (2) Add values for ProdName to each order header line:
    arrData = self~dirOrderLines[records]
    -- (2a) First get id for ProductData:
    objMgr = .local~my.ObjectMgr
    idProductData = objMgr~getComponentId("ProductData", "The")
    if idProductData = .false then return .false
    -- (2b) For each order line, get the product name from ProductData
    --      and add it to the OrderLines in-memory file:
    do i=1 to self~dirOrderLines[count]		-- loop over the Order Lines records:
      -- get Product record from ProductData:
      orderProductNo = arrData[i,2]
      prodDir = idProductData~getRecord(orderProductNo)
      orderProductName = prodDir["ProdName"]
      -- add product's Name to the end of the record:
      arrData[i,4] = orderProductName
    end
    self~dirOrderLines[records] = arrData
    return
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    dirData = .Directory~new
    do j=1 to columns
      header = self~FileHeaders[j]
      dirData[header] = self~fileRecords[recordNo,j]
    end


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    getRecord - returns a directory 'dirOrderRecord' containing all the data for
    		a given Order, including both Order Headers and Order Lines.
    		The format of the directory is:
                Index   Item
                ------- -------
                OrderNo		the order number  (from dirOrderHeaders}
                CustNo		customer number   (from dirOrderHeaders}
                Date		order date	  (from dirOrderHeaders}
                Disc		discount          (from dirOrderHeaders}
                Cmtd		committed?	  (from dirOrderHeaders}
                CustDisc        customer discount (from dirOrderHeaders)
                OrderLineHdrs   <a 1D array>	  (from dirOrderLines)
                OrderLines	<a 2D array>	  (from dirOrderLines)
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getRecord PUBLIC
    use strict arg orderNo
    if orderNo = "" then return .false

    -- (1) Get the data from the two directories that contain data from the two
    --     files. Call these the "src" (source) variables:
    arrSrcOrderHeaderLabels  = self~dirOrderHeaders[headers] -- note naming clash but never mind...
    arrSrcOrderHeaders       = self~dirOrderHeaders[records]
    numSrcOrderHeaders       = self~dirOrderHeaders[count]
    arrSrcOrderLineLabels    = self~dirOrderLines[headers]
    arrSrcOrderLines         = self~dirOrderLines[records]
    numSrcOrderLines         = self~dirOrderLines[count]

    -- (2) Find the Order Header record:
    found = .false
    do recordNo = 1 to numSrcOrderHeaders
      if arrSrcOrderHeaders[recordNo,1] = orderNo then do
      	found = .true;  leave
      end
    end
    if \found then return .false

    -- (3) Now build the dirOrderRecord
    -- (3a) Put the Order headers(i,e, the field labels) into dirOrderRecord:
    dirOrderRecord = .Directory~new
    -- Put the OrderHeader info into dirOrderRecord:
    columns = arrSrcOrderHeaderLabels~items
    do j=1 to columns
      itemName = arrSrcOrderHeaderLabels[j]
      dirOrderRecord[itemName] = arrSrcOrderHeaders[recordNo,j]
    end
    -- (3b) Put the Headers (Field Labels) for the order lines into the record:
    arrOLH = .Array~new
    arrOLH = arrSrcOrderLineLabels
    dirOrderRecord[OrderLineHdrs] = arrOLH
    -- (3c) Now Put OrderLines for this order into dirOrderRecord's array
    arrOrderLines = .Array~new
    rows = arrSrcOrderLines~dimension(1)	-- number of lines or rows
    j = 1
    do i =1 to rows			-- iterate over all rows in arrSrcOrderLines
      --say "Orderdata-getRecord-02:" orderNo arrSrcOrderLines[i,1]
      if arrSrcOrderLines[i,1] = orderNo then do
        --say "OrderData-getRecord-02a: j arrSrcOrderLines[i,1] orderNo:" j arrSrcOrderLines[i,1] orderNo
        arrOrderLines[j,1] = arrSrcOrderLines[i,1]
        arrOrderLines[j,2] = arrSrcOrderLines[i,2]
        arrOrderLines[j,3] = arrSrcOrderLines[i,3]
        arrOrderLines[j,4] = arrSrcOrderLines[i,4]
        j = j+1
      end
    end
    --say "OrderData-getRecord-03: arrOrderLines dimensions =" arrOrderLines~dimension(1) arrOrderLines~dimension(2)
    dirOrderRecord[OrderLines] = arrOrderLines

    return dirOrderRecord


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    getFile - Invoked by Model for an xxxListModel component;
              returns all OrderHeaders including customer info.		      */
  ::METHOD getFile PUBLIC
    return self~dirOrderHeaders


  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    listOrders - lists the file.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD listOrders PUBLIC
    say; say "----List-Orders--------------------------------"
    say "Number of Orders:" self~dirOrderHeaders[Count]
    arrOHH = self~dirOrderHeaders[Headers]
    say "Order Headers: Headers or Field Names ("||arrOHH~items "columns):"
    do i=1 to 9
      say "Order Headers FieldNames:" arrOHH[i]
    end
    say "  " arrOHH[1] arrOHH[2] arrOHH[3] arrOHH[4] arrOHH[5] arrOHH[6] arrOHH[7] arrOHH[8] arrOHH[9]
    say "Order Headers - Records"
    arrOHR = self~dirOrderHeaders[Records]
    do i = 1 to arrOHR~dimension(1)
      say i||".  " arrOHR[i,1] arrOHR[i,2] arrOHR[i,3] arrOHR[i,4]
      say "    " arrOHR[i,5] arrOHR[i,6] arrOHR[i,7]
      say "    " arrOHR[i,8] arrOHR[i,9]
    end
    say
    say "Number of Order Lines:" self~dirOrderLines[Count]
    arrOLH = self~dirOrderLines[Headers]
    say "Order Lines: Headers or FieldNames ("||arrOLH~items "columns):"
    say arrOLH[1] arrOLH[2] arrOLH[3] arrOLH[4]
    say "Order Lines - Records"
    arrOLR = self~dirOrderLines[Records]
    do i = 1 to arrOLR~dimension(1)
      say i||".  " arrOLR[i,1] arrOLR[i,2] arrOLR[i,3] arrOLR[i,4]
    end
    say "-----------------------------------------------"
    say
    return .true

/*============================================================================*/


