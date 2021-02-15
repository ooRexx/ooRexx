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
/*  interface.rex            Open Object Rexx Samples                         */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program demonstrates to define an interface class in ooRexx.         */
/*  object instance.                                                          */
/******************************************************************************/

-- shape is the interface class that defines the methods a shape instance
-- is expected to implement as abstract methods.  Instances of the shape
-- class need not directly subclass the interface, but can use multiple
-- inheritance to mark itself as implementing the interface.

r=.rectangle~new(5,2)
-- check for instance of
if r~isa(.shape) then say "a" r~name "is a shape"
say "r~area:" r~area
say

c=.circle~new(2)
-- check for instance of shape works even if inherited
if c~isa(.shape) then say "a" c~name "is a shape"
say "c~area:" c~area
say

-- a mixin is still a class and can be instantiated.  The abstract methods
-- will give an error if invoked
g=.shape~new
if g~isa(.shape) then say "a" g~class~id "is a shape"
-- invoking abstract method results in a runtime error.
say g~name
say "g~area:" g~area

-- the "MIXINCLASS" tag makes this avaiable for multiple inhertance
::class shape MIXINCLASS Object
  ::method area abstract
  ::method name abstract

-- directly subclassing the interface
-- to create a rectangle
::class rectangle subclass shape

  ::method init
    expose length width
    use strict arg length=0, width=0

  ::method area
    expose length width
    return length*width

  ::method name
    return "Rectangle"

-- inherits the shape methods
::class circle subclass object inherit shape

  ::method init
    expose radius
    use strict arg radius=0

  ::method area
    expose radius
    numeric digits 20
    return radius*radius*3.14159265358979323846

  ::method name
    return "Circle"
