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
   Exercise 04&06: The CustomerModel and CustomerData Classes	  v01-00 07Jun12

   Contains: 	   classes "CustomerModel" and "CustomerResource".
   Pre-requisites: None.

   Outstanding Problems:
   None.

   Changes:
   v01-00 07Jun12: First version.
------------------------------------------------------------------------------*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  CustomerModel							  v01-00 07Jun12
  ------------
  The "model" part of the Customer component.

   Changes:
     v01-00 07Jun12: First version.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS CustomerModel PUBLIC

/*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD newInstance CLASS PUBLIC
    -- Creates an instance and returns it.
    aCustomerModel = self~new
    return aCustomerModel


/*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD activate PUBLIC
    -- Gets its data from ProductData.
    expose idCustomerData
    idCustomerData = .local~my.idCustomerData


  ::METHOD query PUBLIC
    -- Returns data requested (no argument = return all)
    expose idCustomerData
    say "CustomerModel-query-01."
    data = idCustomerData~getData
    return data
/*============================================================================*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  CustomerData							  v01-00 07Jun12
  ------------
  The "data" part of the Customer component.
   Changes:
     v01-00 07Jun12: First version.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS CustomerData PUBLIC

/*----------------------------------------------------------------------------
    Class Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD newInstance CLASS PUBLIC
    aCustomerData = self~new
    return aCustomerData


/*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate PUBLIC
    expose custData
    custData = .directory~new
    custData[custNo]      = "AB15784"
    custData[custName]    = "Joe Bloggs & Co Ltd"
    arrCustAddr = .array~new
    arrCustAddr[1]        = "28 Frith Street"
    arrCustAddr[2]        = "Hardington"
    arrCustAddr[3]        = "Blockshire"
    custData[CustAddr]    = arrCustAddr
    custData[custZip]     = "LB7 4EJ"
    custData[custDiscount]= "B1"
    return


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getData PUBLIC
    expose custData
    say "CustomerData-getData-01."
    return custData

/*============================================================================*/


