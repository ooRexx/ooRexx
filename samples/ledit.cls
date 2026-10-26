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
/*  ledit.cls                Open Object Rexx Samples                         */
/*                                                                            */
/*  A command environment in a package: a small line editor                   */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This package is a command environment that any program can use.  Its      */
/*  prolog registers the environment, so a program that has                   */
/*                                                                            */
/*      ::requires "ledit.cls"                                                */
/*                                                                            */
/*  can send commands to ADDRESS LEDIT, or make LEDIT its default             */
/*  environment with a bare ADDRESS LEDIT, as it would with any other.  The   */
/*  program does not need to know how the environment is written: the         */
/*  package can even be distributed compiled, without its source:             */
/*                                                                            */
/*      rexxc ledit.cls ledit.rxc        -- then ::requires "ledit.rxc"       */
/*                                                                            */
/*  LEDIT keeps one buffer of lines, with a current line, and its commands    */
/*  are in the spirit of a CMS editor: a program written for it is an edit    */
/*  macro.  useledit.rex edits a buffer and calls the macro ledwords.rex,     */
/*  which works on the same buffer.                                           */
/*                                                                            */
/*  The environment, and so the buffer, belongs to the interpreter            */
/*  instance: every program that runs in it (the program that was started,    */
/*  the programs it calls, the packages it requires) sees the same LEDIT.     */
/*  The prolog runs once per instance, the first time a program requires      */
/*  the package.  A program started on its own, in another process, has       */
/*  its own instance, and so its own LEDIT, with an empty buffer.             */
/*                                                                            */
/*  The commands (the command word in any case; n is a number or *, which     */
/*  means all the lines):                                                     */
/*                                                                            */
/*    EDIT [file]         a new buffer: empty, or the lines of file; the      */
/*                        name is kept for FILE.  The current line is the     */
/*                        top of file                                         */
/*    INPUT [text]        adds text as a line after the current line, which   */
/*                        it becomes.  With WITH INPUT, adds all the lines    */
/*                        of the input instead                                */
/*    TOP, BOTTOM         makes the top of file (the line before the first)   */
/*                        or the last line the current line                   */
/*    NEXT [n], UP [n]    moves the current line down or up n lines           */
/*                        (default 1); ERROR, RC 1, if it reaches the end     */
/*                        or the top of file                                  */
/*    :n                  makes line n the current line; ERROR, RC 1, if      */
/*                        there is no such line                               */
/*    LOCATE /text/       finds text in the lines after the current line,     */
/*    /text/              and makes the first line found the current line;    */
/*                        ERROR, RC 2, if it is not found                     */
/*    CHANGE /old/new/ [n]  replaces old with new in n lines (default 1)      */
/*                        from the current line; ERROR, RC 2, if nothing      */
/*                        was changed.  Any character can be the delimiter    */
/*    DELETE [n]          deletes n lines (default 1) from the current one;   */
/*                        the line after them becomes the current line;       */
/*                        ERROR, RC 1, at the top of file                     */
/*    TYPE [n]            displays n lines (default 1) from the current       */
/*                        line, to WITH OUTPUT if there is one, otherwise     */
/*                        to the default output stream                        */
/*    EXTRACT /item/...   sets variables of the caller, as stems: item.0 is   */
/*                        1, item.1 the value.  The items are SIZE (the       */
/*                        number of lines), LINE (the number of the current   */
/*                        line, 0 at the top of file), CURLINE (its text)     */
/*                        and FNAME (the file name)                           */
/*    FILE [file]         writes the buffer to file, or to the file of        */
/*                        EDIT; ERROR, RC 5, if there is no file name         */
/*                                                                            */
/*  A command with a wrong operand raises ERROR, RC 5.  A command LEDIT       */
/*  does not know raises FAILURE, RC -1.  RC is 0 otherwise.                  */
/******************************************************************************/

-- The prolog: it runs once, when the first program of the interpreter
-- instance requires this package.  From then on, ADDRESS LEDIT reaches the
-- editor, in every program of the instance.
.CommandHandler~register("LEDIT", .LineEditor~new)


::class LineEditor public inherit CommandHandler

::method init
  expose lines current fname
  lines = .array~new                  -- the buffer
  current = 0                         -- 0 is the top of file
  fname = ""
  forward class (super)

::method command
  expose lines current fname
  use arg command, caller
  command = command~strip
  if command~startsWith("/") then command = "LOCATE" command
  else if command~startsWith(":") then command = ":" command~substr(2)
  parse var command verb operands
  operands = operands~strip

  select case verb~upper
    when "EDIT" then do
      lines = .array~new
      current = 0
      fname = operands
      if fname \== "", .File~new(fname)~exists then
        lines = .stream~new(fname)~arrayIn
    end

    when "INPUT" then do
      if caller~isRedirected("Input") then do
        in = caller~input
        do while in~lines > 0
          call insert in~linein
        end
      end
      else call insert operands
    end

    when "TOP" then current = 0
    when "BOTTOM" then current = lines~items

    when "NEXT", "UP" then do
      n = self~count(operands, caller)
      if n == .nil then return caller~raiseError(5)
      if verb~upper == "UP" then n = -n
      target = current + n
      current = max(0, min(target, lines~items))
      if current \= target then return caller~raiseError(1)
    end

    when ":" then do
      if \operands~dataType("W") then return caller~raiseError(5)
      if operands < 1 | operands > lines~items then
        return caller~raiseError(1)
      current = operands + 0
    end

    when "LOCATE" then do
      parsed = self~delimited(operands, 1)
      text = parsed[2]
      if \parsed[1] | text == "" then return caller~raiseError(5)
      do i = current + 1 to lines~items
        if lines[i]~pos(text) > 0 then do
          current = i
          return 0
        end
      end
      return caller~raiseError(2)
    end

    when "CHANGE" then do
      parsed = self~delimited(operands, 2)
      if \parsed[1] | parsed[2] == "" then return caller~raiseError(5)
      n = self~count(parsed[4], caller)
      if n == .nil then return caller~raiseError(5)
      changed = 0
      do i = max(current, 1) for n while i <= lines~items
        if lines[i]~pos(parsed[2]) > 0 then do
          lines[i] = lines[i]~changeStr(parsed[2], parsed[3])
          changed += 1
        end
      end
      if changed == 0 then return caller~raiseError(2)
    end

    when "DELETE" then do
      n = self~count(operands, caller)
      if n == .nil then return caller~raiseError(5)
      if current == 0 then return caller~raiseError(1)
      do n while current <= lines~items
        lines~delete(current)
      end
      current = min(current, lines~items)
    end

    when "TYPE" then do
      n = self~count(operands, caller)
      if n == .nil then return caller~raiseError(5)
      if caller~isRedirected("Output") then out = caller~output
      else out = .output~current
      do i = current for n while i <= lines~items
        if i == 0 then out~lineout("* * * Top of File * * *")
        else out~lineout(lines[i])
      end
    end

    when "EXTRACT" then do
      parse var operands delim +1 items
      if delim == "" then return caller~raiseError(5)
      do while items \== ""
        parse var items item (delim) items
        select case item~strip~upper
          when "SIZE" then value = lines~items
          when "LINE" then value = current
          when "CURLINE" then
            if current == 0 then value = ""
            else value = lines[current]
          when "FNAME" then value = fname
          when "" then iterate
          otherwise return caller~raiseError(5)
        end
        caller[item~strip".0"] = 1
        caller[item~strip".1"] = value
      end
    end

    when "FILE" then do
      if operands \== "" then fname = operands
      if fname == "" then return caller~raiseError(5)
      file = .stream~new(fname)
      file~open("write replace")
      file~arrayOut(lines)
      file~close
    end

    otherwise return caller~raiseFailure(-1)
  end
  return 0

insert:                               -- add a line after the current one
  if current == 0 then lines~insert(arg(1), .nil)   -- as the first line
  else lines~insert(arg(1), current)
  current += 1
  return

-- the number of lines of a command: empty for 1, * for all of them
::method count private
  use arg operand, caller
  if operand == "" then return 1
  if operand == "*" then return 999999999
  if operand~dataType("W"), operand >= 0 then return operand + 0
  return .nil

-- the delimited strings at the start of operands (/one/ or /one/two/; the
-- last delimiter can be left out) and what follows them: an array with
-- .true if there was a delimiter, the strings, and the rest
::method delimited private
  use arg operands, howMany
  parse var operands delim +1 rest
  parts = .array~of(delim \== "")
  do howMany
    p = rest~pos(delim)
    if p == 0 then do
      parts~append(rest)
      rest = ""
    end
    else do
      parts~append(rest~left(p - 1))
      rest = rest~substr(p + 1)
    end
  end
  parts~append(rest~strip)
  return parts
