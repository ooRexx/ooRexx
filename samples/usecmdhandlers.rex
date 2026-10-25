#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026 Rexx Language Association. All rights reserved.         */
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
/*  usecmdhandlers.rex       Open Object Rexx Samples                         */
/*                                                                            */
/*  Show the command environments written in Rexx of cmdhandlers.cls          */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program registers each of the command environments of                */
/*  cmdhandlers.cls and sends commands to them with ADDRESS, as it would to   */
/*  any other environment.  Every section is independent.  The program        */
/*  issues no system commands except "echo", which works everywhere.          */
/******************************************************************************/

call title "Echo: the simplest environment"
.CommandHandler~register("ECHO", .Echo~new)
address echo "Hello, world"
say "rc =" rc
name = "Rexx"
address echo "Hello," name"!"             -- the command is an expression

call title "WordCount: a filter, with WITH INPUT and WITH OUTPUT"
.CommandHandler~register("WC", .WordCount~new)
text = .array~of("the quick brown fox", "jumps over", "the lazy dog")
address wc "" with input using (text) output stem out.
say out.1 "(rc =" rc")"                   -- 3 lines, 9 words (rc = 9)
address wc "" with input using (text)     -- no WITH OUTPUT: to the console

call title "Sorter: another filter"
.CommandHandler~register("SORT", .Sorter~new)
names = .array~of("pear", "Apple", "banana", "Cherry", "fig")
sorted = .array~new
address sort "" with input using (names) output using (sorted)
say "sorted:     " sorted~toString("L", " ")"  (rc =" rc")"
address sort "caseless" with input using (names) output using (sorted)
say "caseless:   " sorted~toString("L", " ")
address sort "caseless descending",
  with input using (names) output using (sorted)
say "descending: " sorted~toString("L", " ")

call title "Show: commands displayed in a wrapper"
.CommandHandler~register("SHOW", .Show~new)
call showDemo

call title "NoCommands: for programs that should issue no commands"
.CommandHandler~register("NOCOMMANDS", .NoCommands~new)
call noCommandsDemo

call title "Safe: wrapping the default environment"
call safeDemo

call title "Store: return codes, conditions and the caller's variables"
.CommandHandler~register("STORE", .Store~new)
call storeDemo

call title "The environments known to this interpreter instance"
envs = .CommandHandler~environments
do env over envs~allIndexes~sort
  say "  " ('"'env'"')~left(14) envs[env]~class~id
end

-- environments written in Rexx can be removed again
do env over "ECHO SHOW NOCOMMANDS STORE WC SORT"~subWords
  .CommandHandler~deregister(env)
end
exit


/* Show.  Its best use is at the interactive trace prompt, where it cannot    */
/* be shown in a program that runs by itself; try this one:                   */
/*                                                                            */
/*     .CommandHandler~register("SHOW", .Show~new)                            */
/*     name = "Widget"; items = 3; price = 4.5                                */
/*     trace ?r                                                               */
/*     total = items * price                                                  */
/*     say name":" total                                                      */
/*     ::requires "cmdhandlers.cls"                                           */
/*                                                                            */
/* At the first pause, type "address show", then "items * price" or           */
/* "showvars"; type "address" again before going on.                          */
showDemo: procedure
  address show                            -- SHOW is now the default
  item = "Widget"; price = 4.5
  "Hello"                                 -- [Hello]
  item                                    -- [Widget]: an expression
  "[<>]"                                  -- a new wrapper
  price * 2                               -- <9.0>
  "[]"                                    -- no wrapper
  "price is" price                        -- price is 4.5
  "showvars"                              -- all the variables of this routine
  return                                  -- ADDRESS is restored on return


/* NoCommands.  Two mistakes that Rexx takes for commands, both caught.       */
noCommandsDemo: procedure
  address nocommands                      -- every command goes there now
  say "Hello"
  sya "Hello"                             -- a typing error
  name = "*.tmp"
  "rm" name                               -- left over from an older version
  say "rc =" rc
  return


/* Safe.  It covers the default environment, whatever its name ("CMD" on      */
/* Windows; "sh", "bash", ... on Unix-like systems), and passes it the        */
/* commands it does not refuse.  The refused ones never reach it.             */
safeDemo: procedure
  env = address()
  .CommandHandler~register(env, .Safe~new(env))
  "echo this one runs"
  say "rc =" rc
  "rm -rf nothing-to-see"                 -- refused: FAILURE, traced
  say "rc =" rc
  .CommandHandler~deregister(env)         -- the default environment, as it was
  return


/* Store.  The variables set by the handler are those of this routine.        */
storeDemo: procedure
  address store
  "PUT colour blue"
  "PUT size 42"
  "PUT shape round"
  "GET colour"                            -- sets the variable VALUE
  say "colour is" value
  call on error name noKey
  "GET weight"                            -- no such key: ERROR, RC 1
  call off error
  "COUNT"
  say "keys:" rc
  "DROP size"
  "KEYS"                                  -- sets the stem KEYS.
  do i = 1 to keys.0
    say "  key" i":" keys.i
  end
  "KEYS list"                             -- sets LIST to an Array
  say "  as an Array:" list~toString("L", ", ")
  call badCommand
  return

noKey:
  say "no such key (ERROR, rc =" rc")"
  return

badCommand:
  signal on syntax
  address store "FROB something"          -- not a STORE command: SYNTAX
  return
syntax:
  c = condition("O")
  say "SYNTAX" c~code "at line" c~position":" c~message
  return


title:
  say
  say "=====" arg(1)
  return


::requires "cmdhandlers.cls"
