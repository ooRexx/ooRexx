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
/*  A simple implementation of a linked list using objects.                   */
/*  object instance.                                                          */
/******************************************************************************/

list = .linkedlist~new
index = list~insert("abc")   -- insert a first item, keeping the index
list~insert("def")           -- adds to the end
list~insert("123", .nil)     -- adds to the begining
list~insert("456", index)    -- inserts between "abc" and "def"
list~remove(index)           -- removes "abc"

-- simple iteration through the list by following the links
say "Manual list traversal"
index = list~first           -- demonstrate traversal
loop while index \== .nil
    say index~value
    index = index~next
end

-- any object can use do/loop over traversal by
-- implementing a makearray method
say
say "Do ... Over traversal"
do value over list
    say value
end

-- the main list item, holding the anchor to the links.
::class linkedlist
::method init
  expose anchor

  -- create this as an empty list
  anchor = .nil

-- return first link element
::method first
  expose anchor
  return anchor

-- return last link element
::method last
  expose anchor

  -- this implementation doesn't track the last member of the
  -- list, so we have to run the list to the end.
  current = anchor
  loop while current \= .nil
      -- found the last one
      if current~next == .nil then return current
      current = current~next
  end
  -- empty
  return .nil

-- insert a value into the list, using the convention
-- followed by the built-in list class.  If the index item
-- is omitted, add to the end.  If the index item is .nil,
-- add to the end.  Otherwise, just chain to the provided link.
::method insert
  expose anchor
  use arg value

  newLink = .link~new(value)
  -- adding to the end
  if arg() == 1 then do
      if anchor == .nil then anchor = newLink
      else self~last~insert(newLink)
  end
  else do
      use arg, index
      if index == .nil then do
         if anchor \== .nil then newLink~next = anchor
         anchor = newLink
      end
      else index~insert(newLink)
  end
  -- the link item serves as an "index"
  return newLink

-- remove a link from the chain
::method remove
  expose anchor

  use strict arg index

  -- handle the edge case of removing the anchor
  -- item
  if index == anchor then anchor = anchor~next
  else do
      -- no back link, so we need to scan
      previous = self~findPrevious(index)
      -- invalid index, don't return any item
      if previous == .nil then return .nil
      previous~next = index~next
  end
  -- belt-and-braces, remove the link and return the value
  index~next = .nil
  return index~value

-- private method to find a link predecessor
::method findPrevious private
  expose anchor
  use strict arg index

  -- we're our own precessor if this first
  if index == anchor then return self

  current = anchor
  loop while current \== .nil
      if current~next == index then return current
      current = current~next
  end
  -- not found
  return .nil

-- helper method to allow DO ... OVER traversal
::method makearray
  expose anchor
  array = .array~new

  current = anchor
  loop while current \= .nil
      array~append(current~value)
      current = current~next
  end
  return array

-- instances of this class are the links of the chain.
::class link
::method init
  expose value next
  -- by default, initialize both data and next to empty.
  use strict arg value = .nil, next = .nil

-- allow external access to value and next link
::attribute value
::attribute next

-- insert a link after this link...we also
-- chain our current next node on to the inserted
-- node.
::method insert
  expose next
  use strict arg newNode
  newNode~next = next
  next = newNode

