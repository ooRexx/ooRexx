#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2024-2026 Josep Maria Blasco. All rights reserved.           */
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
/*                                                                            */
/* trident.rex - Check that a program is equal to its Tree API parsing        */
/* ===================================================================        */
/*                                                                            */
/* This program compiles a ooRexx source program and produces an identical    */
/* program using the Tree API. You can build over it to produce other         */
/* language processors that do things more interesting than simply cloning    */
/* the source.                                                                */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20250707    0.2d First version                                             */
/* 20251110    0.2e Rename to "trident.rex" (was "clonetree")                 */
/* 20251211    0.3a Implement Executor support                                */
/* 20251218         Add TUTOR support                                         */
/* 20251221    0.4a Add --itrace option, improve error messages               */
/* 20251226         Send error messages to .error, not .output                */
/* 20251227         Use .SysCArgs when available                              */
/* 20260102         Standardize help options to -h and --help                 */
/* 20260307    0.5  Add experimental support                                  */
/* 20260314         Use InitCLI() from CLISupport.cls                         */
/* 20261115    0.7  Use CATCH instead of SIGNAL ON SYNTAX (RFE #31; needs     */
/*                  ooRexx r13514 or later)                                   */
/* 20261119         A syntax error in FILE is the condition USER              */
/*                  REXX.PARSER.ERROR (THROW, RFE #31; ooRexx r13546 or       */
/*                  later), not SYNTAX 98.900                                 */
/* 20261123         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261123         Moved from bin/ to the samples (RFE #878); requires       */
/*                  parser/modules/identity.cls, and not print.cls            */
/*                                                                            */
/******************************************************************************/

--------------------------------------------------------------------------------
-- Run Main, and handle its errors                                            --
--------------------------------------------------------------------------------

  Do
    Call Main
  Catch co = User Rexx.Parser.Error -- An error in FILE, found by the parser
    Exit ErrorHandler( fullpath, source, co, itrace )
  Catch co = Syntax        -- Anything else is an error of this program
   .Error~Say( "Error" co~code "in" co~program", line" co~position":" )
    Raise Propagate
  End

  Exit 0

--------------------------------------------------------------------------------
-- Main program                                                               --
--------------------------------------------------------------------------------

Main:

  CLIhelper    = InitCLI()
  myName       = CLIhelper~name
  myHelp       = CLIhelper~help
  args         = CLIhelper~args

  executor     = 0
  experimental = 0
  unicode      = 0
  itrace       = 0
  cms          = 0

ProcessOptions:
  If args~items == 0 Then Signal Help

  option = args[1]
  args~delete(1)

  If option[1] == "-" Then Do
    Select Case Lower(option)
      When "-h", "--help"                 Then Signal Help
      When "--itrace", "-it"              Then itrace = 1
      When "--executor", "-xtr"           Then executor = 1
      When "--cms", "-cms",               -
           "--rexxvm", "-rexxvm"          Then cms = 1
      When "-e", "-exp", "--experimental" Then experimental = 1
      When "-u", "--tutor", "--unicode"   Then unicode = 1
      Otherwise Call Error "Invalid option '"option"'."
    End
    Signal ProcessOptions
  End

  file = option

  If args~items > 0 Then Call Error "Unexpected argument '"args[1]"'."

  fullPath = .context~package~findProgram(file)

  If fullPath == .Nil Then
    Call Error "File '"file"' does not exist."

  -- We need to compute the source separately to properly handle syntax errors
  chunk = CharIn(fullPath,1,Chars(fullPath))
  source = chunk~makeArray
  -- Makearray has a funny definition that ignores a possible
  -- last empty line.
  If Right(chunk,1) == "0a"X Then source~append("")
  Call Stream fullPath, "C", "Close"

  options = .Array~new
  If executor     Then options~append(("EXECUTOR",     1))
  If cms          Then options~append(("CMS",          1))
  If unicode      Then options~append(("UNICODE",      1))
  If experimental Then options~append(("EXPERIMENTAL", 1))

  -- Parse our program
  parser = .Rexx.Parser~new(fullPath, source, options)

  package = parser~package

  element = parser~firstElement

  output = .Array.OutputStream~new

  package~compile(element, output, .StringTable~new)

  Do i = 1 To Min(source~items, output~items)
    If source[i] \== output[i] Then Do
      Say "Difference found in line number" i":"
      Say "Source line is '"source[i]"',"
      Say "Parsed line is '"output[i]"'."
      Exit 1
    End
  End

  If source~items == output~items +1, source~lastItem == "" Then Exit 0

  If source~items \== output~items Then Do
    Say "No. of source lines and parsed lines are different:"
    Say "Source:" source~items
    Say "Parsed:" output~items
    Exit 1
  End

  -- We are done
  Return

--------------------------------------------------------------------------------

Error:
 .Error~Say(Arg(1))
  Exit 1

--------------------------------------------------------------------------------

Help:
  Say .Resources[Help]~makeString        -
    ~caselessChangeStr("myName", myName) -
    ~caselessChangeStr("myHelp", myHelp)
  Exit 1

--------------------------------------------------------------------------------

::Requires "parser.cls"
::Requires "parser/ErrorHandler.cls"
::Requires "parser/CLISupport.cls"

::Requires "parser/modules/identity.cls"         -- The Identity compiler

--------------------------------------------------------------------------------

::Class Array.OutputStream Public SubClass Array Inherit OutputStream

::Method Init
  Expose written
  written = 0

::Method Say
  Expose written

  Use Strict Arg string = ""

  If written == 0 Then self~append( string )
  Else                self[self~last] ||= string

  written = 0

  Return 0

::Method CharOut
  Expose written

  Use Strict Arg string -- We don't implement start

  If written == 0 Then self~append( string )
  Else                 self[self~last] ||= string

  written = 1

--------------------------------------------------------------------------------

::Resource HELP
myname -- Verify if the identity compiler returns a perfect copy of a program.

Usage: myname [OPTION]... [FILE]

If the only option is -h or --help, or if no arguments are present,
then display this help and exit.

Options:

--executor, -xtr          Activate support for Executor language extensions
--cms, -cms               Activate support for CMS (Classic Rexx) syntax (also --rexxvm, -rexxvm)
-e, -exp, --experimental  Enable Experimental features
-u, --tutor, --unicode    Enable TUTOR-flavored Unicode
--itrace, -it             Print internal trace on error

The 'myname' program is part of the Rexx Parser package,
see https://rexx.epbcn.com/rexx-parser/. It is distributed under
the Common Public License 1.0 (https://www.oorexx.org/license.html).

Copyright (c) 2024-2026 Josep Maria Blasco <josep.maria.blasco@epbcn.com>.
Copyright (c) 2026 Rexx Language Association.

See myhelp for details.
::END