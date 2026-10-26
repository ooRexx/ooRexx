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
/*  cmdhandlers.cls          Open Object Rexx Samples                         */
/*                                                                            */
/*  A collection of command environments written in Rexx                      */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  Each class in this file is a command environment: an object of a class    */
/*  that inherits .CommandHandler.  Once registered with                      */
/*                                                                            */
/*      .CommandHandler~register(name, handler)                               */
/*                                                                            */
/*  every command sent to ADDRESS name becomes a call to the handler's        */
/*  COMMAND method, with the command string and a .CommandContext that gives  */
/*  access to the code that issued it: its variables, the WITH redirection    */
/*  of the ADDRESS instruction, and the conditions the command raises.        */
/*  What COMMAND returns becomes RC.                                          */
/*                                                                            */
/*  The classes, from the simplest to the most complex:                       */
/*                                                                            */
/*    Echo        displays every command it receives                          */
/*    WordCount   a filter: counts the lines and words of its input           */
/*    Sorter      a filter: sorts its input into its output                   */
/*    Show        displays commands, enclosed in a wrapper; can also list     */
/*                the variables of the caller.  Useful at the interactive     */
/*                trace prompt                                                */
/*    NoCommands  for programs that should issue no commands: any command     */
/*                is reported and raises FAILURE                              */
/*    Safe        wraps an existing environment and refuses some commands     */
/*    Store       a key-value store: return codes, ERROR and SYNTAX           */
/*                conditions, and the variables (simple, stems) of the caller */
/*                                                                            */
/*  usecmdhandlers.rex shows them all at work.  The CommandHandler class and  */
/*  its companions are described in the ooRexx Reference; the chapter         */
/*  "Commands" of the Programming Guide has a tutorial.                       */
/******************************************************************************/


/******************************************************************************/
/* Echo: the simplest possible environment.  RC is always 0.                  */
/*                                                                            */
/*     .CommandHandler~register("ECHO", .Echo~new)                            */
/*     address echo "Hello, world"                                            */
/******************************************************************************/
::class Echo public inherit CommandHandler

::method command
  use arg command, caller
  say "ECHO:" command
  return 0                              -- becomes RC


/******************************************************************************/
/* WordCount: a filter that counts the lines and words of its input and       */
/* writes one line to its output: "n lines, m words".  As Sorter below, it    */
/* uses the WITH INPUT and WITH OUTPUT of the ADDRESS instruction when there  */
/* are, and the default input and output streams when there are not.  RC is   */
/* the number of words.                                                       */
/*                                                                            */
/*     address wc "" with input stem text. output stem result.                */
/*     address wc "" with input stem text.     -- the result to the console   */
/******************************************************************************/
::class WordCount public inherit CommandHandler

::method command
  use arg command, caller
  if caller~isRedirected("Input") then in = caller~input
  else in = .input~current
  if caller~isRedirected("Output") then out = caller~output
  else out = .output~current
  lines = 0; words = 0
  do while in~lines > 0
    lines += 1
    words += in~linein~words
  end
  out~lineout(lines "lines," words "words")
  return words


/******************************************************************************/
/* Sorter: a filter that sorts the lines of its WITH INPUT into its WITH      */
/* OUTPUT.  The command can be empty or one of these words:                   */
/*                                                                            */
/*   CASELESS     sorts ignoring case                                         */
/*   DESCENDING   sorts in descending order                                   */
/*                                                                            */
/* Without WITH INPUT it reads the default input stream (.input~current),     */
/* and without WITH OUTPUT it writes to the default output stream: a          */
/* handler decides what to do when there is no redirection.  RC is the        */
/* number of lines sorted.                                                    */
/******************************************************************************/
::class Sorter public inherit CommandHandler

::method command
  use arg command, caller
  if caller~isRedirected("Input") then in = caller~input
  else in = .input~current
  if caller~isRedirected("Output") then out = caller~output
  else out = .output~current

  lines = in~arrayIn
  options = command~upper
  if options~wordPos("CASELESS") > 0 then comparator = .CaselessComparator~new
  else comparator = .Comparator~new
  if options~wordPos("DESCENDING") > 0 then
    comparator = .InvertingComparator~new(comparator)
  lines~sortWith(comparator)
  out~arrayOut(lines)
  return lines~items


/******************************************************************************/
/* Show: every command is displayed enclosed in the current                   */
/* wrapper, "[]" to begin with.  Two commands are special:                    */
/*                                                                            */
/*   [wrap]    makes "wrap" the new wrapper: the command will be shown in     */
/*             its middle.  "[<>]" sets "<>"; "[]" sets no wrapper at all.    */
/*   SHOWVARS  lists the variables of the caller and their values.            */
/*                                                                            */
/* Its best use is at the interactive trace prompt: after "address show",     */
/* what you type is a command, so typing a variable name, or any              */
/* expression, displays its value, with no "say" needed.  But what is typed   */
/* at the prompt runs in the program itself: type "address" again before      */
/* resuming, or the program's own commands will go to SHOW.                   */
/******************************************************************************/
::class Show public inherit CommandHandler

::method init
  expose wrapper
  wrapper = '[]'
  forward class (super)

::method command
  expose wrapper
  use arg command, caller
  -- check for a change of the wrapper: command is '[wrap]', wrap -> wrapper
  fc = command~left(1)
  lc = command~right(1)
  if fc = '[', lc = ']' then do
    wrapper = command~substr(2, command~length - 2)
    return 0
  end
  if command~upper = 'SHOWVARS' then do -- show all the caller's vars and values
    theVars = caller~variables
    do v over theVars~allIndexes~sort
      say v":" theVars[v]
    end
    return 0
  end
  say (wrapper == '')~?(command, encase(command, wrapper))
  return 0

::routine encase
-- Function to encase a string inside another (default: '[]')
--  Takes up to 3 arguments, the first being the string to be 'encased', the
--  second (optional) being the wrapper for that string and the third (optional)
--  being the position within the wrapper after which the string is inserted.
--  The default wrapper is '[]' and the default position is the center of the
--  wrapper.
  use arg what='', wrapper='[]', where=''
  if where~datatype(W), where >= 0 then
    at = where
  else
    at = (wrapper~length) % 2
  return wrapper~insert(what, at)


/******************************************************************************/
/* NoCommands: for a program that is not supposed to issue any commands.      */
/* After "address nocommands", a clause that Rexx takes for a command (a      */
/* typing error such as "sya 'Hello'", a command left over from an older      */
/* version) is reported on the error stream and raises FAILURE, which is      */
/* traced unless the program traps it.                                        */
/******************************************************************************/
::class NoCommands public inherit CommandHandler

::method command
  use arg command, caller
  .error~lineout("NOCOMMANDS: this program should not issue commands:" command)
  return caller~raiseFailure(-1)


/******************************************************************************/
/* Safe: wraps an existing environment and refuses a few commands.            */
/*                                                                            */
/*     env = address()                     -- the default environment         */
/*     .CommandHandler~register(env, .Safe~new(env))                          */
/*                                                                            */
/* Registrations stack: the new handler covers the one in use for that name   */
/* (native or written in Rexx), and .CommandHandler~handler(name), asked      */
/* before registering, returns it.  Safe passes every command it does not     */
/* refuse to that handler, so the commands run as usual, with the same        */
/* redirection and return codes.  .CommandHandler~deregister(name) removes    */
/* Safe and uncovers the original environment again.                          */
/******************************************************************************/
::class Safe public inherit CommandHandler

::method init
  expose below refused
  use strict arg name, refused = "rm del erase format rmdir rd"
  below = .CommandHandler~handler(name) -- the environment it covers
  if below == .nil then
    raise syntax 93.900 array("SAFE: there is no environment named" name)
  forward class (super) arguments (.array~new)

::method command
  expose below refused
  use arg command, caller
  if refused~caselessWordPos(command~word(1)) > 0 then do
    .error~lineout("SAFE: refused:" command)
    return caller~raiseFailure(-1)
  end
  return below~command(command, caller) -- everything else runs as usual


/******************************************************************************/
/* Store: a key-value store, used through commands.                           */
/*                                                                            */
/*   PUT key value   stores value under key (keys are case-sensitive)         */
/*   GET key         sets the variable VALUE of the caller; ERROR, RC 1, if   */
/*                   there is no such key                                     */
/*   DROP key        removes the key; ERROR, RC 1, if there is no such key    */
/*   KEYS [name]     the keys, sorted, in a variable of the caller: with a    */
/*                   name that ends in a period, a stem (name.0 is the        */
/*                   count); with any other name, an Array.  KEYS alone is    */
/*                   KEYS KEYS.                                               */
/*   COUNT           RC is the number of keys                                 */
/*                                                                            */
/* Any other command is a SYNTAX error in the caller, at its ADDRESS clause,  */
/* as a built-in function reports a wrong argument.                           */
/******************************************************************************/
::class Store public inherit CommandHandler

::method init
  expose table
  table = .StringTable~new
  forward class (super)

::method command
  expose table
  use arg command, caller
  parse var command verb key data
  select case verb~upper
    when "PUT" then table[key] = data
    when "GET" then do
      if \table~hasIndex(key) then return caller~raiseError(1)
      caller["value"] = table[key]           -- a variable of the caller
    end
    when "DROP" then do
      if \table~hasIndex(key) then return caller~raiseError(1)
      table~remove(key)
    end
    when "KEYS" then do
      keys = table~allIndexes~sort
      if key == "" then key = "keys."
      if key~endsWith(".") then do           -- a stem
        caller~dropVar(key)                  -- start with an empty one
        do i = 1 to keys~items
          caller[key || i] = keys[i]
        end
        caller[key"0"] = keys~items
      end
      else caller[key] = keys                -- an Array
    end
    when "COUNT" then return table~items
    otherwise
      return caller~raise("SYNTAX", 98.900, "STORE: unknown command" verb)
  end
  return 0
