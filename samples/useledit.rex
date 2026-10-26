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
/*  useledit.rex             Open Object Rexx Samples                         */
/*                                                                            */
/*  Use the command environment of the package ledit.cls                      */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program requires ledit.cls, whose prolog registers the command       */
/*  environment LEDIT, and edits a buffer with it, as it would with any       */
/*  other environment.  Then it calls the edit macro ledwords.rex, which      */
/*  requires ledit.cls too: the package is not loaded again, and the macro    */
/*  works on the same buffer, because the environment belongs to the          */
/*  interpreter instance and both programs run in it.                         */
/*                                                                            */
/*  At the end the buffer is written to a temporary file, read back and       */
/*  the file deleted.                                                         */
/******************************************************************************/

call title "A buffer, filled with WITH INPUT"
poem = .array~of("The quick brown fox", "jumps over", "the lazy dog.")
address ledit "EDIT"                    -- a new, empty buffer
address ledit "INPUT" with input using (poem)
address ledit "INPUT and runs away."    -- one more line, after the last one
address ledit "TOP"
address ledit "TYPE *"

address ledit                           -- from now on, the default
call title "Moving around, changing lines"
"TOP"
"/lazy/"                                -- the same as LOCATE /lazy/
"TYPE"
"CHANGE /lazy/sleepy/"
"TYPE"
":1"                                    -- the first line
"CHANGE /quick/very quick/"
"TYPE"
"/cat/"                                 -- not there: ERROR, RC 2
say "LOCATE /cat/ gives rc =" rc
"NEXT 10"                               -- past the end: ERROR, RC 1
say "NEXT 10 gives rc =" rc
call on failure name notLedit          -- TRACE NORMAL displays it too
"FROB"                                  -- not a LEDIT command: FAILURE
call off failure

call title "Output into a variable, values from the editor"
":1"
address ledit "TYPE *" with output stem copy.  -- WITH needs ADDRESS
say "TYPE * gave" copy.0 "lines; the second one is:" copy.2
":2"
"EXTRACT /SIZE/LINE/CURLINE/"
say "size" size.1", current line" line.1":" curline.1

call title "An edit macro, on the same buffer"
call "ledwords.rex"
say "ledwords.rex returned" result
"EXTRACT /LINE/"
say "the current line is still" line.1

call title "Writing the buffer to a file"
file = .File~temporaryPath~absolutePath || .File~separator ||,
  "useledit" || random(1000, 9999) || ".txt"
"FILE" file
lines = .stream~new(file)~arrayIn
say "the file has" lines~items "lines; the last one is:" lines~lastItem
call SysFileDelete file
exit


notLedit:
  say condition("D") "gives FAILURE, rc =" rc
  return

title:
  say
  say "=====" arg(1)
  return


::requires "ledit.cls"
