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
/*  singleLinkedList.rex     Open Object Rexx Samples                         */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  Demonstrates on how to sort non-string objects using the built in         */
/*  sort methods.                                                             */
/******************************************************************************/

-- This is a collection of NHL Hockey champions for different years
a = .array~new

a~append(.pair~new("06-07", "Ducks"))
a~append(.pair~new("00-01", "Avalanche"))
a~append(.pair~new("02-03", "Devils"))
a~append(.pair~new("01-02", "Red Wings"))
a~append(.pair~new("03-04", "Lightning"))
a~append(.pair~new("04-05", "lockout"))
a~append(.pair~new("05-06", "Hurricanes"))
a~append(.pair~new("99-00", "Devils"))
a~append(.pair~new("07-08", "Red Wings"))
a~append(.pair~new("08-09", "Penguins"))

b = a~copy   -- make a copy before sorting

-- this sorts by using the pair compareTo method to order
-- the items
b~sort
say "Sorted by date using direct comparison"
do pair over b
   say pair
end

c = a~copy
-- this uses a custom comparator instead
c~sortWith(.paircomparator~new)
say
say "Sorted by date using a comparator (inverted)"
do pair over c
   say pair
end

c = a~copy
-- this uses a a comparator that sorts on the value rather than
-- the name
c~sortWith(.pairValueComparator~new)
say
say "Sorted on the team names"
do pair over c
   say pair
end

c = a~copy
-- And finally, use an inverting comparator to reverse the
-- order by chaining the comparators
-- the name
c~sortWith(.invertingComparator~new(.pairValueComparator~new))
say
say "Sorted on the team names, but inverted"
do pair over c
   say pair
end

-- a name/value mapping class that directly support the sort comparisons
::class pair inherit comparable
::method init
  expose name value
  use strict arg name, value

::attribute name
::attribute value

-- a handy string method for displaying
::method string
  expose name value
  return name "=" value

-- the compareto method is a requirement brought in
-- by the comparable mixin
::method compareTo
  expose name
  use strict arg other

  -- default ordering is to compare the name items
  return name~compareTo(other~name)

-- a comparator that shows an alternate way of sorting
::class pairComparator subclass comparator
-- compare two items for the sortWith method
::method compare
  use strict arg left, right
  -- perform the comparison on the names, but
  -- invert the result
  return -left~name~compareTo(right~name)

-- a comparator that comparse on the value of the pair rather
-- than the name
::class pairValueComparator subclass comparator
-- compare two items for the sortWith method
::method compare
  use strict arg left, right
  -- perform the comparison on the values
  -- invert the result
  return left~value~compareTo(right~value)
