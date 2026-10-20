/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026      Rexx Language Association. All rights reserved.    */
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
/*----------------------------------------------------------------------------*/
/* logicalwrapper.cls: a logical value that can be recognized as one.         */
/*                                                                            */
/* LogicalWrapper~true and LogicalWrapper~false behave as the Rexx logical    */
/* values "1" and "0" everywhere, and isA(.LogicalWrapper) tells them apart   */
/* from plain strings.                                                        */
/* JsonBoolean (json.cls) and YamlBoolean (yaml.cls) are its subclasses.      */
/*----------------------------------------------------------------------------*/

/** A logical value that can be recognized as such. There are two instances per
*   class, <code>~true</code> and <code>~false</code>; they behave as the Rexx
*   strings <code>"1"</code> and <code>"0"</code> wherever a string is used
*   (IF, WHEN, operators, string methods, SAY), and <code>isA(.LogicalWrapper)</code>
*   tells them apart from plain strings. A subclass gets its own two instances.
*/
::class "LogicalWrapper" public inherit Comparable

/** The instance for <code>.true</code>. */
::attribute true  get class unguarded

/** The instance for <code>.false</code>. */
::attribute false get class unguarded

/** Only the class itself creates instances. */
::method new class private
  forward class (super)

/** Creates the two instances of this class (or subclass). */
::method activate class
  expose true false
  true  = self~new(.true)
  false = self~new(.false)

/** Returns the instance for a logical value: <code>.LogicalWrapper~of(a > b)</code>.
*   @param value a Rexx logical value (<code>0</code> or <code>1</code>), or a LogicalWrapper
*   @return <code>~true</code> or <code>~false</code> of the receiving class
*/
::method of class unguarded
  use strict arg value
  value = value~request("string")
  if value == .nil | \datatype(value, "O") then
    raise syntax 34.900 array ("Logical value must be 0 or 1; found" value)
  if value then return self~true
  return self~false

/** Stores the logical value. */
::method init
  expose value
  use strict arg value

/** @return the Rexx logical value, <code>"1"</code> or <code>"0"</code> */
::method value unguarded
  expose value
  return value

/** @return <code>"1"</code> or <code>"0"</code>, so that the object is a plain logical value for Rexx */
::method makeString unguarded
  expose value
  return value

/** @return <code>"1"</code> or <code>"0"</code>: what an Array shows for the object in
*   <code>makeString</code> and <code>toString</code>, which use the default name, not
*   <code>STRING</code> (found by oorexxnet, 17/10/2026)
*/
::method defaultName unguarded
  expose value
  return value

/** @return <code>"1"</code> or <code>"0"</code> (SAY, string concatenation of the object itself) */
::method string unguarded
  expose value
  return value

/** Any other message goes to the string value (operators, string methods). */
::method unknown unguarded
  expose value
  use arg messageName, arguments
  forward to (value) message (messageName) arguments (arguments)

/** The logical value's hash, consistent with <code>==</code>. */
::method hashCode unguarded
  expose value
  return value~hashCode

/** Equal: same logical value (another LogicalWrapper of any class, or a string). */
::method "=" unguarded
  use strict arg other
  return self~compareTo(other) = 0

::method "\=" unguarded
  use strict arg other
  return self~compareTo(other) \= 0

::method "<>" unguarded
  forward message ("\=")

::method "><" unguarded
  forward message ("\=")

/** Strictly equal: the same logical value, compared as strings. */
::method "==" unguarded
  expose value
  use strict arg other
  return value == other~request("string")

::method "\==" unguarded
  expose value
  use strict arg other
  return value \== other~request("string")

/** For Comparable: -1, 0 or 1 comparing the logical values. */
::method compareTo unguarded
  expose value
  use strict arg other
  otherValue = other~request("string")
  if otherValue == .nil then
    raise syntax 88.900 array ("Argument ""other"" ["other"] has no STRING method")
  if value < otherValue then return -1
  if value = otherValue then return  0
  return 1
