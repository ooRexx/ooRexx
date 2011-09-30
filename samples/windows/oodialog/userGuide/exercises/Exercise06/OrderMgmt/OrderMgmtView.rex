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
   Exercise 06: OrderMgmtView.rex 				  v00-02 30Sep11

   Contains: 	   class: "OrderMgmtView"

   This is a subclass of OrderMgmtBaseView, and provides only the "application"
   function of Order Management.

   Pre-requisites: Class "OrderMgmtBaseView

   Description: A sample Order Management View clas - part of the sample
        	Order Management component.

   Outstanding Problems: None reported.

   Changes:
   v00-01 21Aug11: First version.
   v00-02 30Sep11: Added OrderForm.
------------------------------------------------------------------------------*/

call "OrderMgmt\RequiresList.rex"

::REQUIRES "ooDialog.cls"
::REQUIRES "OrderMgmt\OrderMgmtBaseView.rex"

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  OrderManagementView						  v00-01 21Aug11
  --------------------
  The "application" part of the OrderManagement View" component. This class
  provides for ...

  interface iOrderManagementView {
    void newInstance();
  }
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


::CLASS OrderMgmtView SUBCLASS OrderMgmtBaseView PUBLIC

  /*----------------------------------------------------------------------------
    Instance Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    Dialog Setup Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ::METHOD initRecords PRIVATE  -- called from superclass' init method.
    expose records
    records = .array~new()

    rec = .directory~new
    rec~ID = "CustomerList"
    rec~name = "Customer List"
    records[1] = rec

    rec = .directory~new
    rec~ID = "ProductList"
    rec~name = "Product List"
    records[2] = rec

    rec = .directory~new
    rec~ID = "OrderList"
    rec~name = "Sales Orders"
    records[3] = rec

    rec = .directory~new
    rec~ID = "OrderForm"
    rec~name = "New Order"
    records[4] = rec

    return records


  /*----------------------------------------------------------------------------
    Event-Handling Methods
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ::METHOD onDoubleClick UNGUARDED
    expose records
    --use arg id
    say "OrderMgmtView-Double-Clicked-01."
    -- Get the index of the item with the focus, use the index to retrieve
    -- the item's record:
    index = self~lv~focused		-- lv is an attribute of the superclass.
    record = records[index+1]
    say "OrderMgmtView-02-onDoubleClick: Record ID =" record~ID
    self~showModel(record)


  -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ::METHOD showModel UNGUARDED
    /* Surface the view of an icon (i.e. view of the model represented by the icon).
       Ideally, if already instantiated, surface it, else makeInstance.
       In this version, get as many as you like - but all have the same data!.*/
    use arg record				-- record is a directory object.
    className = record~ID
    say "OrderMgmtView-showModel-01: className =" className
    viewClassName = className||"View"
    --root = self
    interpret "."||viewClassName||"~newInstance(self)"
    say "OrderMgmtView-showModel-02:"
    --.CustomerListView~newInstance(self,root)

