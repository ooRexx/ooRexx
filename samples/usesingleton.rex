#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/*  Description:                                                              */
/*  usesingleton.rex demonstrates how to use the Singleton class              */
/******************************************************************************/

do clz over .test, .testSingleton   -- iterate over the two classes
   rounds=3
   say "creating" rounds "objects of type:" clz
   do i=1 to rounds
      say "   round #" i":" clz~new -- create new instance
   end
   say
end

/* ========================================================================= */
/** This Test class counts the number of instances that get created for it.  */
::class Test
/* ------------- class method and class attribute definitions -------------- */
::method init class  -- class constructor
  expose counter
  counter=0          -- make sure attribute is initialized to 0

::attribute counter get private class  -- getter method that increases counter
  expose counter     -- access attribute
  counter+=1         -- increase counter by 1
  return counter     -- return new counter value
/* ------------- instance method and instance attribute definitions -------- */
::attribute nr get   -- getter method

::method init        -- constructor that sets the value of attribute nr
  expose nr          -- expose attribute
  nr=self~class~counter -- new instance: fetch new counter from class and save it

::method makestring  -- a string representation of the object
  expose nr          -- expose attribute
                     -- return a string representation
  return "a" self~class~id"[nr="nr",identityHash="self~identityHash"]"

/* ========================================================================= */
/** This class makes sure that only a single instance of it gets created by
*   using Singleton as its metaclass.
*/
::class TestSingleton subclass Test metaclass Singleton
