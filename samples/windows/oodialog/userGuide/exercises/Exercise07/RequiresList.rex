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
   Exercise 07: The Order Management Application
   Startup.rex 							  v01-02 01Apr13

   Description: This file is the "application" or "root" or "starter" part
                of the sample Order Management application.

   Changes:
     v01-00 06Jun12: First version.
     v01-01 07Aug12: Support for ObjectMgr and ViewMgr added. MessageSender is
                     optional.
            11Jan13: Deleted Commented-out startup of MessageSender.
            21Mar13: Added Copyright notice.
     v01-02 01Apr13: After ooDialog 4.2.2, "Samples" folder changed name to
                     "Extras", so change to ::Requires needed.

------------------------------------------------------------------------------*/


/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  RequiresList							  v01-00 16Sep12
  ----
  The list of files containing classes invoked by the ObjectMgr

  Changes:
  v01-00 16Sep12: First version.
         11Jan13: Commented-out 'say' instruction.
         26May13: Corrected paths for ::REQUIRES afrer re-factoring.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
--say "RequiresList-01."
::REQUIRES "Customer\CustomerView.rex"
::REQUIRES "Customer\CustomerModelsData.rex"
::REQUIRES "Customer\CustomerListView.rex"
::REQUIRES "Product\ProductModelsData.rex"
::REQUIRES "Product\ProductListView.rex"
::REQUIRES "Order\OrderFormView.rex"
::REQUIRES "Order\OrderListView.rex"
::REQUIRES "Order\OrderModelsData.rex"
::REQUIRES "Order\OrderView.rex"
::REQUIRES "Extras\Wow4\WowView.rex"
::REQUIRES "Extras\Wow4\WowModel.rex"
::REQUIRES "Extras\Wow4\WowData.rex"
::REQUIRES "Extras\Person\PersonView.rex"
::REQUIRES "Extras\Person\PersonModelData.rex"
/*============================================================================*/

