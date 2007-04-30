#!/usr/bin/rexx
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/*  pipe.rex            Open Object Rexx Samples                              */
/*                                                                            */
/*  A pipeline implementation                                                 */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program demonstrates the use of ::class and ::method directives to   */
/*  create a simple implementation of a CMS-like pipeline function.           */
/******************************************************************************/


/**
 * Base filter class.  Most sub classes need only override the process() method to
 * implement a filter.  The transformed results are passed down the filter chain
 * by calling the result method.
 */

::class filter public                       -- base filter class
::method init
expose next
next = .nil

::method '|' class                          -- concatenate an instance of a filter with following filter
use arg follower
me = self~new                               -- create a new filter instance
return me|follower                          -- perform the hook up

::method '|'
use arg follower
follower = follower~new                     -- make sure this is an instance
self~append(follower)                       -- do the chain append logic
return self                                 -- we're our own return value

::method append                             -- append a filter to the entire chain
expose next
use arg follower
if .nil == next then do                     -- if we're the end already, just update the next
    next = follower
end
else do
    next~append(follower)                   -- have our successor append it.
end

::method insert                             -- insert a filter after this one, but before the next
expose next
user arg newFilter

newFilter~next = next                       -- just hook into the chain
next = newFilter


::method '[]' class                         -- create a filter instance with arguments
forward to (self) message('NEW')            -- just forward this as a new message

::method go                                 -- execute using a provided object
expose source                               -- get the source supplier
use arg source                              -- set to the supplied object
self~begin                                  -- now go feed the pipeline

::method next attribute                     -- next stage of the filter
::method source attribute                   -- source of the initial data
                                            -- that they are class objects for
::method new                                -- the filter chaining process
return self                                 -- just return ourself

::method begin                              -- start pumping the pipeline
expose source                               -- access the data and next chain

engine = source~supplier                    -- get a data supplier
do while engine~available                   -- while more data
  self~process(engine~item)                 -- pump this down the pipe
  engine~next                               -- get the next data item
end
self~done                                   -- signal that processing is finished

::method process                            -- default data processing
use arg value                               -- get the data item
self~result(value)                          -- send this down the line

::method result                             -- handle the result from a process method
expose next
use arg data
if .nil <> next then do
    next~process(data)                      -- only forward if we have a successor
end

::method done                               -- process "end-of-pipe" condition
expose next
if .nil <> next then do
    next~done                               -- only forward if we have a successor
end

::class sorter public subclass filter       -- sort piped data
::method init                               -- sorter initialization method
expose items                                -- list of sorted items
items = .list~new                           -- create a new list
self~init:super                             -- forward the initialization

::method process                            -- process sorter piped data item
expose items                                -- access internal state data
use arg value                               -- access the passed value
index = items~first                         -- get the first list item
do while .nil <> index                      -- while we still have an index
  if items[index] > value then do           -- found the insertion point?
                                            -- insert this
    items~insert(value, items~previous(index))
    return                                  -- we're finished
  end
  index = items~next(index)                 -- step to the next item
end
items~insert(value)                         -- add this item to the end

::method done                               -- process the "end-of-pipe"
expose items                                -- expose the list
index = items~first                         -- get the first item
do while .nil <> index                      -- while we still have items
  self~result(items[index])                 -- send along this item
  index = items~next(index)                 -- step to the next item
end

::class reverser public subclass filter     -- a string reversal filter
::method process                            -- filter processing item
use arg value                               -- get the data item
self~result(value~reverse)                  -- send it along in reversed form

::class displayer subclass filter public
::method process                            -- process a data item
use arg value                               -- get the data value
say value                                   -- display this item
forward class(super)

::class selector public subclass filter     -- a string selector filter
::method init                               -- process initial strings
expose patterns                             -- access the exposed item
patterns = arg(1,'a')                       -- get the patterns list
self~init:super                             -- forward the initialization

::method process                            -- process a selection filter
expose patterns                             -- expose the pattern list
use arg value                               -- access the data item
do i = 1 to patterns~size                   -- loop through all the patterns
                                            -- this pattern in the data?
  if (value~pos(patterns[i]) <> 0) then do
    self~result(value)                      -- send it along
    leave                                   -- stop the loop
  end
end

::class stemcollector subclass filter public -- collect items in a stem
::method init                               -- initialize a collector
expose stem.                                -- expose target stem
use arg stem.                               -- get the stem variable target
stem.0 = 0                                  -- start with zero items
self~init:super                             -- forward the initialization

::method process                            -- process a stem filter item
expose stem.                                -- expose the stem
use arg value                               -- get the data item
stem.0 = stem.0 + 1                         -- stem the item count
stem.[stem.0] = value                       -- save the value
forward class(super)

::class arraycollector subclass filter public -- collect items in an array
::method init                               -- initialize a collector
expose array index                          -- expose target stem
use arg array                               -- get the stem variable target
index = 0
self~init:super                             -- forward the initialization

::method process                            -- process a stem filter item
expose array index                          -- expose the stem
use arg value                               -- get the data item
index = index + 1
array[index] = value                        -- stem the item count
self~process:super(value)                   -- allow superclass to send down pipe


/**
 * a base class for filters that split the processing stream into two or more
 * filters.  The default behavior is to broadcast each line down all of the branches.
 * To customize, override process() and route the transformed lines down the
 * appropriate branch(es) using result with a target index specified.  If you wish
 * to use the default broadcast behavior, just call self~process:super(newValue) to
 * perform the broadcast.
 */

::class splitter subclass filter public
::method init
expose filters
filters = arg(1, 'A')                       -- just save the arguments as an array

::method append                             -- override for the single append version
expose filters
use arg follower

do filter over filters                      -- append the follower to each of the filter chains
    filter~append(follower)
end

::method insert                             -- this doesn't make sense for a fan out
raise syntax 93.963                         -- Can't do this, so raise an unsupported error

::method result                             -- broadcast a result to a particular filter
expose filters
use arg which, value                        -- which is the fiter index, value is the result
filters[which]~process(value);              -- have the filter handle this

::method done                               -- broadcast a done message down all of the branches
expose filters

do filter over filters
    filter~done
end

::method process                            -- process the filter stream
expose filters
use arg value

do filter over filters                      -- send this down all of the branches
    filter~process(value)
end

/**
 * A simple splitter sample that splits the stream based on a pivot value.
 * strings that compare < the pivot value are routed to filter 1.  All other
 * strings are routed to filter 2
 */

::class pivot subclass splitter public
::method init
expose pivotvalue
use arg pivotvalue                         -- just store the filter value

-- finish up with the superclass
forward class (super) arguments (arg(2, 'A'))


::method process                           -- process the split
expose pivotvalue
use arg value

if value < pivotvalue then do              -- simple split test
    self~result(1, value)
end
else do
    self~result(2, value)
end


