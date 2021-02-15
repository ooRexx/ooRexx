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
/* ooDialog User Guide - Support
   Exercise 08: View.rex

   Component							  v01-00 13May13
   ---------
   A superclass for all components (View, Model and data). Part of the MVF.

   Contains: 	   class: "Component"

   Description: *** To be provided. ***

   Pre-requisites: None.

   Outstanding Problems: None reported.

   Changes:
     v01-00 13May13: First Version.

------------------------------------------------------------------------------*/



/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  Object						  	  v01-00 13May13
  ------
  The superclass for all application components.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

--::CLASS View SUBCLASS RcDialog PUBLIC
::CLASS Component PUBLIC MIXINCLASS Object


  /*----------------------------------------------------------------------------
    init - initialises the dialog - not used.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::method init
    say "Component-init-01."

  ::METHOD saySomething
    say "Component-saySomething: Hi there!."
    return .true
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    Event Management
    --------------------------------------------------------------------------*/

  /*----------------------------------------------------------------------------
    registerInterest
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD registerInterest
    use strict arg eventName, interestedObject
    eventMgr = .local~my.EventMgr
    --say "Component~registerInterest-01: event =" eventName||"; object =" interestedObject
    r = eventMgr~registerInterest(eventName,interestedObject)
    return r
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    triggerEvent
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD triggerEvent
    use arg eventName
    eventMgr = .local~my.EventMgr
    --say "Component-triggerEvent-01: event =" eventName||"; r =" r
    r = eventMgr~triggerEvent(eventName)  -- if r = 0 then no-one's registered.
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    deRegisterInterest
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD deRegisterInterest
    use arg eventName, uninterestedObject
    eventMgr = .local~my.EventMgr
    --say "Component-deRegisterIntrest-01: event =" eventName||"; object =" uninterestedObject||" r =" r
    r = eventMgr~deregisterInterest(eventName, uninterestedObject)  -- if r = 0 then no-one's registered.
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*============================================================================*/
