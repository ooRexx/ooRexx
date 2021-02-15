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
   Exercise 08:          					  v02-00 24May13

   The ProductModel, ProductListModel, and ProductData Classes

   Contains:  classes "ProductModel", "ProductListModel, "ProductData",
              and "ProductDT".
   Pre-requisites: None.

   Outstanding Problems:
   None.

   Changes:
   v00-02: 21Jly11
   v00-03: Correct "return" statement not in right place (typo in code!!)
           Renamed the ProductDT attributes (initial "prod" deemed extraneous)
         - 26Aug11: added some comments - no change to function.
   v00-04 21Aug12: ProducListModel added. Modified to fit the MV Framework.
   v02-00 13Jan12: Ex07: Some 'say' instructions removed or commented out.
          01Apr13: After ooDialog 4.2.2, Support folder moved to exercise
                   folder, so change to ::Requires needed.
          24May13: Minor changes to comments for Ex08.

------------------------------------------------------------------------------*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ProductModel							  v00-02 12Jly11
  ------------
  The "model" part of the Product component.

  interface productModel{
    aProductModel newInstance()  -- Class method.
    null	  activate()
    aProductDT     query()
  };
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::REQUIRES "Support\GenericFile.rex"
::REQUIRES "Support\Model.rex"

::CLASS ProductModel SUBCLASS Model PUBLIC

/*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    -- Creates an instance and returns it.
    use strict arg instanceName						--Ex07
    forward class (super) continue					--Ex07
    modelId = RESULT							--Ex07
    return modelId							--Ex07


/*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD init
    -- Gets its data from ProductData.
    expose prodData
    use strict arg prodData						--Ex07
    return self								--Ex07


  ::METHOD query PUBLIC
    -- Returns data requested (no argument = return all)
    -- self~myData (super's attribute) is a DT. So ask the data component for its
    -- directory version of the data (an attribute of ProductData).
    expose prodData
    --say "ProductModel-query-01: prodData =" prodData
    dir = .directory~new
    return prodData
/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ProductListModel						  v00-01 20Aug12
  ----------------
  The model for a list of Products.
  Changes:
    v00-01 20Aug12: First version
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS ProductListModel SUBCLASS Model PUBLIC

  /*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    use arg instanceName
    --self~myInstanceName = instanceName
    self~wantList = .true				-- set super's attribute
    forward class (super) continue
    id = RESULT
    --say "ProductListModel-newInstance-01: id =" id
    return id


  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD init
    expose arrData
    use strict arg data
    --self~myData = data
    --say "ProductListModel-init-01: myData =" self~myData
    return self

--  ::METHOD query PUBLIC
  /*----------------------------------------------------------------------------
    query - returns an array of all Product data.
            In MVF this method is invoked by the RcView (or ResView) superclass.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    --say "ProductListModel-query-01."
--    return self~myData

/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ProductData							  v01-00 20Jly11
  ------------
  The "data" part of the Product component.
  [interface (idl format)]
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS ProductData SUBCLASS GenericFile PUBLIC

  ::ATTRIBUTE created CLASS
  ::ATTRIBUTE dirData

/*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newInstance CLASS PUBLIC
    use strict arg instanceName
    if self~created = "CREATED" then do
      self~created = .true
      productDataId = self~new()
      --if r = .true then self~created = .true
      return productDataId
    end
    else do
      say "... singleton component, so can't have more than one instance."
      return .false
    end


/*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init PUBLIC
    expose filename numRecords
    filename = "Product\ProductFile.txt";  columns = 6
    numRecords = self~init:super(filename, columns)
    --say "ProductData-init-01: numRecords:" numRecords
    return self

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getData PUBLIC	--  ???? Use the DT???
    expose data
    --say "ProductData-getData-01."
    return data

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    Find - forward to super, then pack data into a ProductDT.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getRecord PUBLIC
    use strict arg dataKey
    --say "ProductData-getRecord-00: dataKey = <"||dataKey||">"
    forward class (super) continue
    dirData = RESULT			-- Generic File returns a directory.
    --say "ProductData-getRecord-01: dirData =" dirData
    self~dirData = dirData
    if dirData = .false then return .false
    -- Now convert dirData to a DT, pack it into dirData then return dirData:
    dt = .ProductDT~new
    dt~number      = dirData["ProdNo"]
    dt~name        = dirData["ProdName"]
    dt~price       = dirData["ListPrice"]
    dt~uom         = dirData["UOM"]
    dt~description = dirData["Description"]
    dt~size        = dirData["Size"]
    dirData["DT"] = dt
    return dirData


/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ProductDT - A business data type for Product data.		  v00-02 07Aug11
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/

::CLASS ProductDT PUBLIC

  --		dtName		XML Name	Description
  --		---------	----------	-------------------------------
  --		ProductDT	product
  ::ATTRIBUTE	number		-- number	Product Number
  ::ATTRIBUTE	name		-- name		Product Description
  ::ATTRIBUTE	price		-- price	Product Price (rightmost two digits are 100ths of currency unit)
--::ATTRIBUTE   currency	-- currency	Three-letter currency code
  ::ATTRIBUTE	uom		-- uom		Product Unit of Measure
  ::ATTRIBUTE   description	-- descrip	Product Description
  ::ATTRIBUTE   size		-- size		Produce Size Category (S/M/L)

  ::METHOD list PUBLIC
    expose number name price uom description size
    say "---------------"
    say "ProductDT-List:"
    say "Number: " number   "Name:" name
    say "Price:" price "UOM:" uom  "Size:" size
    say "Description:" description
    say "---------------"
/*============================================================================*/
