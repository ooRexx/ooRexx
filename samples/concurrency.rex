#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/*  concurrency.rex          Open Object Rexx Samples                         */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This is a simple demonstration of multi-threaded execution with some      */
/*  syncronization between the thread.                                        */
/******************************************************************************/

-- this will launch 3 threads, with each thread given a message to print out.
-- I've added a stoplight to make each thread wait until given a go signal,
-- plus some sleeps to give the threads a chance to randomize the execution
-- order a little.
launcher = .launcher~new
say "Launching in 5..."
call syssleep 1
say "...4..."
call syssleep 1
say "...3..."
call syssleep 1
say "...2..."
call syssleep 1
say "...1..."
call syssleep 1
say "Launch!"

launcher~launch

::class launcher
-- the launcher method.  Guarded is the default, but let's make this
-- explicit here
::method launch guarded

  runner1 = .runner~new(self, "Thread 1:  Enjoy")
  runner2 = .runner~new(self, "Thread 2:  Object")
  runner3 = .runner~new(self, "Thread 3:  Rexx")

  Say "Threads started but blocked"

  -- let's give the threads a chance to settle in to the
  -- starting line
  call syssleep 1

  guard off   -- release the launcher lock.  This is the starter's gun
  Say "Threads now unblocked"
  call syssleep 1
  -- now block execution a couple of times
  loop 4
     guard on
     say "Threads blocked again for 2 seconds"
     call syssleep 2
     say "Unblocking threads again"
     guard off
     call syssleep .2
  end

-- this is a guarded method that the runners will call.  They
-- will block until the launch method releases the object guard
::method block guarded

::class runner
::method init
  use arg launcher, text
  reply  -- this creates the new thread

  call syssleep .5  -- try to mix things up by sleeping
  launcher~block    -- wait for the go signal
  -- display this 5 times
  loop 10
     say text
     call syssleep .5  -- add another sleep here
     launcher~block    -- wait for the go signal
  end
