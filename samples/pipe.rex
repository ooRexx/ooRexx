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
 * by calling the write method.
 */

::class filter public                       -- base filter class
::method init
expose next secondary
next = .nil
secondary = .nil                            -- all filters have a secondary output potential

::method '|' class                          -- concatenate an instance of a filter with following filter
use arg follower
me = self~new                               -- create a new filter instance
return me|follower                          -- perform the hook up

::method '>' class                          -- concatenate an instance of a filter with following filter
use arg follower
me = self~new                               -- create a new filter instance
return me>follower                          -- perform the hook up

::method '>>' class                         -- concatenate an instance of a filter with following filter
use arg follower
me = self~new                               -- create a new filter instance
return me>>follower                         -- perform the hook up

::method '|'
use arg follower
follower = follower~new                     -- make sure this is an instance
self~append(follower)                       -- do the chain append logic
return self                                 -- we're our own return value

::method '>'
use arg follower
follower = follower~new                     -- make sure this is an instance
self~append(follower)                       -- do the chain append logic
return self                                 -- we're our own return value

::method '>>'
use arg follower
follower = follower~new                     -- make sure this is an instance
self~appendSecondary(follower)              -- do the chain append logic
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

::method appendSecondary                    -- append a to the secondary output of entire chain
expose next secondary
use arg follower
if .nil == next then do                     -- if we're the end already, just update the next
    secondary = follower                    -- append this to the secondary port.
end
else do
    next~appendSecondary(follower)          -- have our successor append it.
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

::method secondary attribute                -- a potential secondary attribute
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
self~eof                                    -- signal that processing is finished

::method process                            -- default data processing
use arg value                               -- get the data item
self~write(value)                           -- send this down the line

::method write                              -- handle the result from a process method
expose next
use arg data
if .nil <> next then do
    next~process(data)                      -- only forward if we have a successor
end

::method writeSecondary                     -- handle a secondary output result from a process method
expose secondary
use arg data
if .nil <> secondary then do
    secondary~process(data)                 -- only forward if we have a successor
end

::method processSecondary                   -- handle a secondary output result from a process method
forward message('PROCESS')                  -- this by default is a merge operation

::method eof                                -- process "end-of-pipe" condition
expose next secondary
if .nil <> next then do
    next~eof                                -- only forward if we have a successor
end
if .nil <> secondary then do
    secondary~eof                           -- only forward if we have a successor
end

::method secondaryEof                       -- process "end-of-pipe" condition
-- we just ignore this one, and rely on the secondary

::method secondaryConnector                 -- retrieve a secondary connector for a filter
return new .SecondaryConnector(self)

::class SecondaryConnector subclass filter
::method init
expose filter
use strict arg filter                       -- this just hooks up
self~init:super                             -- forward the initialization

::method process                            -- processing operations connect with filter secondaries
expose filter
forward to(filter) message('processSecondary')

::method eof                                -- processing operations connect with filter secondaries
expose filter
forward to(filter) message('secondaryEof')



::class Sort public subclass filter         -- sort piped data
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

::method eof                                -- process the "end-of-pipe"
expose items                                -- expose the list
index = items~first                         -- get the first item
do while .nil <> index                      -- while we still have items
  self~write(items[index])                  -- send along this item
  index = items~next(index)                 -- step to the next item
end
forward class(super)                        -- make sure we propagate the done message

::class reverse public subclass filter      -- a string reversal filter
::method process                            -- filter processing item
use arg value                               -- get the data item
self~write(value~reverse)                   -- send it along in reversed form

::class upper public subclass filter        -- a uppercasing filter
::method process                            -- filter processing item
use arg value                               -- get the data item
self~write(value~upper)                     -- send it along in upper form

::class lower public subclass filter        -- a lowercasing filter
::method process                            -- filter processing item
use arg value                               -- get the data item
self~write(value~lower)                     -- send it along in lower form


::class changestr public subclass filter    -- a string replacement filter
::method init
expose old new count
use strict arg old, new, count = 999999999  -- old and new are required, default count is max value
self~init:super                             -- forward the initialization

::method process                            -- filter processing item
expose old new count
use arg value                               -- get the data item
self~write(value~changestr(old, new, count))  -- send it along in altered form


::class delstr public subclass filter       -- a string deletion filter
::method init
expose offset length
use strict arg offset, length               -- both are required.
self~init:super                             -- forward the initialization

::method process                            -- filter processing item
expose offset length
use arg value                               -- get the data item
self~write(value~delstr(offset, length))    -- send it along in altered form


::class left public subclass filter         -- a splitter filter
::method init
expose length
use strict arg length                       -- the length is the left part
self~init:super                             -- forward the initialization

::method process                            -- filter processing item
expose offset length
use arg value                               -- get the data item
self~write(value~left(length))              -- send the left portion along the primary stream
self~writeSecondary(value~substr(length + 1))  -- the secondary gets the remainder portion


::class right public subclass filter        -- a splitter filter
::method init
expose length
use strict arg length                       -- the length is the left part
self~init:super                             -- forward the initialization

::method process                            -- filter processing item
expose offset length
use arg value                               -- get the data item
self~write(value~substr(length + 1))        -- the remainder portion goes down main pipe
self~writeSecondary(value~left(length))     -- send the left portion along the secondary stream


::class insert public subclass filter       -- insert a string into each line
::method init
expose insert offset
use strict arg insert, offset               -- we need an offset and an insertion string
self~init:super                             -- forward the initialization

::method process                            -- filter processing item
expose insert offset
use arg value                               -- get the data item
self~write(value~insert(insert, offset))    -- send the left portion along the primary stream


::class overlay public subclass filter      -- insert a string into each line
::method init
expose overlay offset
use strict arg overlay, offset              -- we need an offset and an insertion string
self~init:super                             -- forward the initialization

::method process                            -- filter processing item
expose insert offset
use arg value                               -- get the data item
self~write(value~overlay(overlay, offset))  -- send the left portion along the primary stream


::class dropnull public subclass filter     -- drop null records
::method process                            -- filter processing item
use arg value                               -- get the data item
if value \== '' then do                     -- forward along non-null records
    self~write(value)
end


::class dropFirst public subclass filter    -- drop the first n records
::method init
expose count counter
use arg count

counter = 0
self~init:super                             -- forward the initialization

::method process
expose count counter
use arg value

counter += 1                                -- if we've dropped our quota, start forwarding
if counter > count then do
    self~write(value)
end
else do
    self~writeSecondary(value)              -- non-selected records go down the secondary stream
end


::class dropLast public subclass filter     -- drop the last n records
::method init
expose count array
use arg count

array = .array~new                          -- we need to accumulate these until the end
self~init:super                             -- forward the initialization

::method process
expose array
use arg value

array~append(value)                         -- just add to the accumulator

::method eof
expose count array

if array~items < count then do              -- didn't even receive that many items?
    loop line over array
        self~write(line)                    -- send everything down the main pipe
    end
end
else do
    first = array~items - count             -- this is the count of discarded items
    loop i = 1 to first
        self~writeSecondary(array[i])       -- the discarded ones go to the secondary pipe
    end

    loop i = first + 1 to array~items
        self~write(array[i])                -- the remainder ones go down the main pipe
    end
end


::class takeFirst public subclass filter    -- take the first n records
::method init
expose count counter
use arg count

counter = 0
self~init:super                             -- forward the initialization

::method process
expose count counter
use arg value

counter += 1                                -- if we've dropped our quota, stop forwarding
if counter > count then do
    self~writeSecondary(value)
end
else do
    self~write(value)                       -- still in the first bunch, send to main pipe
end


::class takeLast public subclass filter     -- drop the last n records
::method init
expose count array
use arg count

array = .array~new                          -- we need to accumulate these until the end
self~init:super                             -- forward the initialization

::method process
expose array
use arg value

array~append(value)                         -- just add to the accumulator

::method eof
expose count array

if array~items < count then do              -- didn't even receive that many items?
    loop line over array
        self~writeSecondary(line)           -- send everything down the secondary pipe
    end
end
else do
    first = array~items - count             -- this is the count of selected items
    loop i = 1 to first
        self~write(array[i])                -- the selected go to the main pipe
    end

    loop i = first + 1 to array~items
        self~writeSecondary(array[i])       -- the discarded ones go down the secondary pipe
    end
end



::class x2c public subclass filter          -- translate records to hex characters
::method process                            -- filter processing item
use arg value                               -- get the data item
self~write(value~x2c)


::class bitbucket public subclass filter    -- just consume the records
::method process                            -- filter processing item
nop                                         -- do nothing with the data

::class fanout public subclass filter       -- write records to both output streams
::method process                            -- filter processing item
use arg value                               -- get the data item
self~write(value)
self~writeSecondary(value)

::method eof                                -- make sure done messages get propagated along all streams
self~next~eof
self~secondary~eof


::class merge public subclass filter        -- merge the results from primary and secondary streams
::method init
expose mainDone secondaryEof                -- need pair of EOF conditions

mainDone = .false
secondaryEof = .false
self~init:super                             -- forward the initialization

::method eof
expose mainDone secondaryEof                -- need interlock flags

if secondaryEof then do                     -- the other input hit EOF already?
    forward class(super)                    -- handle as normal
end

mainDone = .true                            -- mark this branch as finished.

::method secondaryEof                       -- eof on the seconary input
expose mainDone secondaryEof                -- need interlock flags

secondaryEof = .true                        -- mark ourselves finished

if mainDone then do                         -- if both branches finished, do normal done.
    forward message('DONE')
end


::class fanin public subclass filter        -- process main stream, then secondary stream
::method init
expose mainDone secondaryEof array          -- need pair of EOF conditions

mainDone = .false
secondaryEof = .false
array = .array~new                          -- accumulator for secondary
self~init:super                             -- forward the initialization

::method processSecondary                   -- handle the secondary input
expose array
use arg value

array~append(value)                         -- just append to the end of the array

::method eof
expose mainDone secondaryEof array          -- need interlock flags

if secondaryEof then do                     -- the other input hit EOF already?
    loop i = 1 to array~items               -- need to write out the deferred items
        self~write(array[i])
    end
    forward class(super)                    -- handle as normal
end

mainDone = .true                            -- mark this branch as finished.

::method secondaryEof                       -- eof on the seconary input
expose mainDone secondaryEof                -- need interlock flags

secondaryEof = .true                        -- mark ourselves finished

if mainDone then do                         -- if both branches finished, do normal done.
    forward message('DONE')
end


::class duplicate public subclass filter    -- duplicate each record N times
::method init
expose copies
use strict arg copies = 1                   -- by default, we do one duplicate
self~init:super                             -- forward the initialization

::method process                            -- filter processing item
expose copies
use arg value                               -- get the data item
loop copies + 1                             -- write this out with the duplicate count
    self~write(value)
end


::class displayer subclass filter public
::method process                            -- process a data item
use arg value                               -- get the data value
say value                                   -- display this item
forward class(super)


::class all public subclass filter          -- a string selector filter
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
    self~write(value)                      -- send it along
    return                                  -- stop the loop
  end
end

self~writeSecondary(value)                 -- send all mismatches down the other branch, if there


::class startsWith public subclass filter   -- a string selector filter
::method init                               -- process initial strings
expose match                                -- access the exposed item
use arg match                               -- get the patterns list
self~init:super                             -- forward the initialization

::method process                            -- process a selection filter
expose match                                -- expose the pattern list
use arg value                               -- access the data item
if (value~pos(match) == 1) then do          -- match string occur in first position?
  self~write(value)                        -- send it along
end
else do
   self~writeSecondary(value)              -- send all mismatches down the other branch, if there
end



::class notall public subclass filter       -- a string de-selector filter
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
    self~writeSecondary(value)             -- send it along the secondary...don't want this one
    return                                  -- stop the loop
  end
end

self~write(value)                          -- send all mismatches down the main branch


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

::class between subclass filter public      -- write only records from first trigger record
                                            -- up to a matching record
::method init
expose startString endString started finished
use strict arg startString, endString
started = .false                            -- not processing any lines yet
finished = .false
self~init:super                             -- forward the initialization

::method process
expose startString endString started finished
use arg value
if \started then do                         -- not turned on yet?  see if we've hit the trigger
    if value~pos(startString) > 0 then do
        started = .true
        self~write(value)                  -- pass along
    end
    else do
        self~writeSecondary(value)         -- non-selected lines go to the secondary bucket
    end
    return
end

if \finished then do                        -- still processing?
    if value~pos(endString) > 0 then do     -- check for the end position
        finished = .true
    end
    self~write(value)                      -- pass along
end
else do
    self~writeSecondary(value)             -- non-selected lines go to the secondary bucket
end

::class after subclass filter public        -- write only records from first trigger record
::method init
expose startString started
use strict arg startString
started = .false                            -- not processing any lines yet
self~init:super                             -- forward the initialization

::method process
expose startString endString started
use arg value
if \started then do                         -- not turned on yet?  see if we've hit the trigger
    if value~pos(startString) = 0 then do
        self~writeSecondary(value)         -- pass along the secondary stream
        return
    end
    started = .true
end

self~write(value)                          -- pass along


::class before subclass filter public       -- write only records before first trigger record
::method init
expose endString finished
use strict arg endString
finished = .false
self~init:super                             -- forward the initialization

::method process
expose endString finished
use arg value

if \finished then do                        -- still processing?
    if value~pos(endString) > 0 then do     -- check for the end position
        finished = .true
    end
    self~write(value)                      -- pass along
end
else do
    self~writeSecondary(value)             -- non-selected lines go to the secondary bucket
end


::class buffer subclass filter public       -- write only records before first trigger record
::method init
expose buffer count delimiter
use strict arg count = 1, delimiter = ("")
buffer = .array~new
self~init:super                             -- forward the initialization

::method process
expose buffer
use arg value
buffer~append(value)                        -- just accumulate the value

::method eof
expose buffer count delimiter

loop i = 1 to count                         -- now write copies of the set to the stream
     if i > 1 then do
         self~write(delimiter)             -- put a delimiter between the sets
     end
     loop j = 1 to buffer~items             -- and send along the buffered lines
         self~write(buffer[i])
     end
end

forward class(super)                        -- and send the done message along


::class lineCount subclass filter public    -- count number of records passed through the filter
::method init
expose counter
counter = 0
self~init:super                             -- forward the initialization

::method process
expose counter
use arg value

counter += 1                                -- just bump the counter on each record

::method eof
expose counter

self~write(counter);                       -- write out the counter message

forward class(super)                        -- and send the done message along


::class charCount subclass filter public    -- count number of characters passed through the filter
::method init
expose counter
counter = 0
self~init:super                             -- forward the initialization

::method process
expose counter
use arg value

counter += value~length                     -- just bump the counter for the length of each record

::method eof
expose counter

self~write(counter);                       -- write out the counter message

forward class(super)                        -- and send the done message along


::class wordCount subclass filter public    -- count number of characters passed through the filter
::method init
expose counter
counter = 0
self~init:super                             -- forward the initialization

::method process
expose counter
use arg value

counter += value~words                      -- just bump the counter for the number of words

::method eof
expose counter

self~write(counter);                       -- write out the counter message

forward class(super)                        -- and send the done message along



/**
 * A simple splitter sample that splits the stream based on a pivot value.
 * strings that compare < the pivot value are routed to filter 1.  All other
 * strings are routed to filter 2
 */

::class pivot subclass filter public
::method init
expose pivotvalue
self~init:super                             -- forward the initialization
-- we did the initialization first, as we're about to override the filters
-- store the filter value and hook up the two output streams
use strict arg pivotvalue, self~next, self~secondary

::method process                           -- process the split
expose pivotvalue
use arg value

if value < pivotvalue then do              -- simple split test
    self~write(value)
end
else do
    self~writeSecondary(value)
end


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
self~init:super                             -- forward the initialization

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

::method eof                               -- broadcast a done message down all of the branches
expose filters

do filter over filters
    filter~eof
end

::method process                            -- process the filter stream
expose filters
use arg value

do filter over filters                      -- send this down all of the branches
    filter~process(value)
end


