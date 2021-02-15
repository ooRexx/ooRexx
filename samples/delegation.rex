#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/*  delegation.rex           Open Object Rexx Samples                         */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  Demonstrate the concept of a object method delegation, where one object   */
/*  can pass on a method to another object                                    */
/******************************************************************************/


delegator = .delegator~new   -- no delegate
say delegator~something
-- an invalid delegate type
delegator~delegate = "Some string"
say delegator~something
-- but we can try another method that the string does have
say delegator~reverse

-- an arbitrary object as a delegate
delegator~delegate = .thing~new
say delegator~heyThing

-- a directory object with a thing entry defined
d = .directory~new
d~thing = "A thing in a directory"
delegator~delegate = d
say delegator~thing

-- a class we can use as a delegate
::class thing
::method heyThing
  return "hey back at you!"

-- A delegator object.  The delegator will
-- pass on all unknown methods to the delegate object
::class delegator
::method init
  expose delegate
  use strict arg delegate = .nil

-- attribute method to allow the delegate to be changed
::attribute delegate

-- An UNKNOWN method to intercept unknown messages and pass them
-- along to the delegate object
::method unknown
  expose delegate
  use arg message, args
  if delegate == .nil then return "default implementation"

  -- Note:  We could use delegate~hasMethod(message) to check
  -- for a THING method, but this will fail of the object relies
  -- on an UNKNOWN method to handle the method.  By trapping
  -- NOMETHOD conditions, we can allow those calls to go
  -- through
  signal on nomethod

  -- use forward to dynamically invoke the target method.
  -- we specify CONTINUE so that our NOMETHOD handler can trap
  -- the error if the method does not exist
  forward to(delegate) message(message) arguments(args) continue
  -- return any returned result...if we actually got one.
  if var('RESULT') then return result
  return

nomethod:
  return "default implementation"
