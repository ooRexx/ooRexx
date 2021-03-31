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
/*  synchronousConcurrency   Open Object Rexx Samples                         */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This uses a work queue to synchronize activity between two independent    */
/*  threads                                                                   */
/******************************************************************************/

-- the queue is used to send data between the two threads
queue = .workqueue~new

-- our inputs and outputs
-- (we use jabberwocky.txt from the same samples directory)
input = .stream~new(.File~new("jabberwocky.txt", .File~new(.context~package~name)~parent))
output = .output

-- the file reader reads lines from the input stream and writes to
-- the work queue.  The file writer reads lines from the work queue and
-- writes them to the output stream
reader = .filereader~new(input, queue)
writer = .filewriter~new(output, queue)

-- a work queue.  Threads pulling lines from the queue will block
-- until lines are available
::class workQueue
::method init
  expose queue stopped actionpending
  queue = .queue~new
  stopped = .false
  actionPending = .false

-- add an item to the work queue.  This is a
-- guarded method, which means this is a synchronized access
::method addItem guarded
  expose queue actionPending
  use arg item
  -- add the item to the queue
  queue~queue(item)
  -- indicate there's something new.  This is a condition variable
  -- that any will wake up any thread that's waiting on access.  They'll
  -- be able to get access once we exit
  actionPending = .true

-- another method for coordinating access with the other thread.  This indicates
-- it is time to shut down
::method stop guarded
  expose actionPending stopped
  -- indicate this has been stopped and also flip the condition variable to
  -- wake up any waiters
  stopped = .true
  actionPending = .true

-- read the next item off of the queue.  .nil indicates we've reached
-- the last item on the queue.  This is also a guarded method, but we'll use
-- the GUARD ON instruction to wait for work if the queue is currently empty
::method nextItem guarded
  expose queue stopped actionPending
  -- we might need to loop a little to get an item
  do forever
    -- if there's something on the queue, pull the front item and return
    if \queue~isEmpty then return queue~pull
    -- if the other thread says it is done sending us stuff, time to shut down
    if stopped then return .nil
    -- nothing on the queue, not stopped yet, so reset actionPending, release
    -- the guard and wait until there's pending to work on.
    actionPending = .false
    guard on when actionPending
  end

-- one half of the synchronization effort.  This will read lines and
-- add them to the work queue.  The thread will terminate once we hit end-of-file
::class filereader
::method init
  -- accept a generic stream...the data source need not be a file
  use arg stream, queue

  reply   -- now multithreaded

  signal on notready
  loop forever
     queue~addItem(stream~linein)
  end
  -- we come here on an EOF condition.  Indicate we're done and terminate
  -- the thread
  notready:
  queue~stop

-- the other end of this.  This class will read lines from a work queue
-- and write it to a stream
::class filewriter
::method init
  -- accept a generic stream...the data source need not be a file
  use arg stream, queue

  reply   -- now multithreaded

  loop forever
     item = queue~nextItem
     -- .nil means last item received
     if item == .nil then return
     -- write to the stream
     stream~lineout(item)
  end
