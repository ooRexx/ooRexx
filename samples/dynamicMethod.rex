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
/*  dynamicMethod.rex        Open Object Rexx Samples                         */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program demonstrates now methods can be dynamically added to an      */
/*  object instance.                                                          */
/******************************************************************************/

-- the dynamic var instance does not have "FOO" or "FOO=" attribute methods
-- initially
d = .dynamicvar~new
-- verify the methods are not there
if d~hasMethod("FOO") | d~hasMethod("FOO=") then say "An object of type" d~class~id "has FOO methods"
   else say "No FOO methods found on an object of type" d~class~id

-- now set a FOO value and display it. This will dynamically create the
-- methods.
d~foo = 123
say
say "Value of FOO is" d~foo
say
-- verify the methods ARE there
if d~hasMethod("FOO") | d~hasMethod("FOO=") then say "An object of type" d~class~id "has FOO methods"
   else say "No FOO methods found on an object of type" d~class~id

-- a class that allows dynamic variables.  Since this is a mixin, this
-- capability can be added to any class using multiple inheritance
::class dynamicvar MIXINCLASS object
-- the unknown method will get invoked any time an unknown method is
-- used.  This UNKNOWN method will add attribute methods for the given
-- name that will be available on all subsequent uses.
::method unknown
  use strict arg messageName, arguments

  -- check for an assignment or fetch, and get the proper
  -- method name
  if messageName~right(1) == '=' then do
     variableName = messageName~left(messageName~length - 1)
  end
  else do
     variableName = messageName
  end

  -- define a pair of methods to set and retrieve the instance variable.  These are
  -- created at the object scope
  self~setMethod(variableName, 'expose' variableName'; return' variableName)
  self~setMethod(variableName'=', 'expose' variableName'; use strict arg value;' variableName '= value' )

  -- reinvoke the original message.  This will now go to the dynamically added methods
  forward to(self) message(messageName) arguments(arguments)
