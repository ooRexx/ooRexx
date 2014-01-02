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
   Exercise 08: EventMgr.rex 				  	  v01-00 11May13

   Contains: 	   class: "EventMgr"

   Description: The Event Manager records interest in events, and sends
                notification messages when the event is triggered by receipt of
                a "triggerEvent" message.

   Pre-requisites: None.

   Outstanding Problems: None reported.

   Changes:
       11May13: First Version.

------------------------------------------------------------------------------*/

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  EventMgr							  v01-00 09May13
  --------
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

  ::CLASS EventMgr PUBLIC

  ::ATTRIBUTE dirEvents PRIVATE

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    -- Initiated by the MVF object.
    self~dirEvents = .directory~new
    .local~my.EventMgr = self
    return self


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD registerInterest PUBLIC
    use strict arg event, object
    -- Check if dir has an entry for this event yet
    -- NOTE: In this version:
    --       Multiple registrations by an object for same event NOT checked!
    --       That means if it doesn't de-register, it gets multiple notifies!
    if self~dirEvents[event] = .nil then do
      --say "EventList-registerInterest-01: No event array for '"||event||"'"
      arr = .array~new
      self~dirEvents[event]=arr
    end
    --say "EventMgr-registerInterest-02: event= '" ||event||"' object = '"||object
    arr = self~dirEvents[event]
    if arr~hasItem(object) then nop -- no point in registering twice!
    else arr~append(object)
    return .true

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD triggerEvent PUBLIC
    -- If event not registered, returns .false; else invokes "notify" on all
    -- objects registered for the event and returns .true.
    use strict arg event
    --say "EventMgr-triggerEvent-01: event =" event
    arr = self~dirEvents[event]
    if arr = .nil then return .false	-- event not registered.
    else do
      do i over arr
        --say "EventMgr-triggerEvent-02: sending 'notify' for" event "to" i
        i~notify(event)
      end
      return .true
    end

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD deRegisterInterest PUBLIC
    use strict arg event, object
    -- Returns .false if event not registered or if object not registered;
    -- else removes the object from the event array and returns .true.
    use strict arg event, object
    --say "EventMgr-deRegisterInterest-01: event =" event
    arr = self~dirEvents[event]
    if arr = .nil then return .false	-- event not registered.
    r = arr~removeItem(object)
    if r = .nil then do			-- object not found
      return .false
    end
    return .true

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD list
    say "EventMgr-list-01."
    say "----Event-List---------------"
    do i over self~dirEvents
      arr = self~dirEvents[i]
      str = ""
      do j over arr
        str = str||j||", "
      end
      say "Event '"||i||"':" str
    end
    say "-----------------------------"; say
