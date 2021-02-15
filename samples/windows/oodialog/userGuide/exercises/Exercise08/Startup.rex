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
   Exercise 08: The Order Management Application
   Startup.rex 							  v01-02 25May13

   Description: This file is the "application" or "root" or "starter" part
                of the sample Order Management application.

   Changes:
     v01-00 06Jun12: First version.
     v01-01 07Aug12: Support for ObjectMgr and ViewMgr added. MessageSender is
                     optional.
            11Jan13: Deleted Commented-out startup of MessageSender.
            01Apr13: After ooDialog 4.2.2, Support folder moved to exercise
                     folder, so change to ::Requires needed.
     v01-02 11May13: Added instantiation of Event Manager (class EventMgr)
            25May13: mods to comments only.

------------------------------------------------------------------------------*/

parse arg pwOption
if pwOption = "enterPW" then do
  pwd = PasswordBox("Please enter your password","Sign In")
  if pwd \= "Password" then exit
end

-- Set application defaults:
.Application~setDefaults("O", , .false)

-- Create Object Manager, View Manager, and Event Manager:
om = .ObjectMgr~new
vm = .ViewMgr~new
em = .EventMgr~new
dm = .DragMgr~new

-- Start OrderMgrView:
.OrderMgrView~newInstance

::REQUIRES "OrderMgr\OrderMgrView.rex"
::REQUIRES "Support\ObjectMgr.rex"
::REQUIRES "Support\ViewMgr.rex"
::REQUIRES "Support\EventMgr.rex"
::REQUIRES "Support\DragMgr.rex"
::REQUIRES "Support\MessageSender.rex"

/******************************************************************************/
