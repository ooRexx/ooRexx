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
   Exercise 04b: The ProductModel and ProductData Classes         v00-03 08Aug11

   Contains:  classes "ProductModel", "ProductResource", and "ProductDT".
   Pre-requisites: None.

   Outstanding Problems:
   None.

   Changes:
   v00-02: 21Jly11
   v00-03: Correct "return" statement not in right place (typo in code!!)
           Renamed the ProductDT attributes (initial "prod" deemed extraneous)
         - 26Aug11: added some comments - no change to function.
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

::CLASS ProductModel PUBLIC

/*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    -- Creates an instance and returns it.
    aProductModel = self~new
    return aProductModel


/*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD activate PUBLIC
    -- Gets its data from ProductData.
    expose data
    idProductData = .local~my.idProductData
    data = idProductData~getData


  ::METHOD query PUBLIC
    -- Returns data requested (no argument = return all)
    expose data
    say "ProductModel-query."
    return data
/*============================================================================*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ProductData							  v01-00 20Jly11
  ------------
  The "data" part of the Product component.
  [interface (idl format)]
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS ProductData PUBLIC

/*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newInstance CLASS PUBLIC
    aProductData = self~new
    return aProductData


/*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate PUBLIC
    expose data
    data = .ProductDT~new
    data~number    = "CF300/X"
    data~name  = "Widget Box"
    data~price = "2895"
    data~uom   = "6"
    data~description = "A 10 litre case with flat sides capable of holding quite a lot of stuff."
    data~size  = "M"
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getData PUBLIC
    expose data
    say "ProductData-getData."
    return data

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