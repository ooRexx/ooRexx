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
/* erexx.rex - Run an Experimental REXX program                               */
/* ============================================                               */
/*                                                                            */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20251113    0.3a First version                                             */
/* 20251127         Add support for Executor                                  */
/* 20251221    0.4a Add --itrace option, improve error messages               */
/* 20251226         Send error messages to .error, not .output                */
/* 20251227         Use .SysCArgs when available                              */
/* 20260102         Standardize help options to -h and --help                 */
/* 20260314    0.5  Use InitCLI() from CLISupport.cls                         */
/* 20261115    0.7  Use CATCH instead of SIGNAL ON SYNTAX (RFE #31; needs     */
/*                  ooRexx r13514 or later)                                   */
/* 20261115         Call "EnableExperimentalFeatures", quoted, so that it is  */
/*                  found on Linux                                            */
/* 20261119         A syntax error in FILE is the condition USER              */
/*                  REXX.PARSER.ERROR (THROW, RFE #31; ooRexx r13546 or       */
/*                  later), not SYNTAX 98.900                                 */
/* 20261123         The identity compiler is the one that comes with ooRexx   */
/*                  (parser/modules/identity.cls, RFE #878)                   */
/* 20261123         No table constructors: initializers.cls is gone, as they  */
/*                  are from the parser                                       */
/* 20261123         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261123         Moved from bin/ to the samples (RFE #878); the class      */
/*                  extensions and their enabler are in                       */
/*                  parser/modules/experimental/, and print.cls is not        */
/*                  required                                                  */
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

  Exit

--------------------------------------------------------------------------------
-- Main program                                                               --
--------------------------------------------------------------------------------

Main:

  CLIhelper    = InitCLI()
  myName       = CLIhelper~name
  myHelp       = CLIhelper~help
  args         = CLIhelper~args

  list     = 0
  executor = 0
  itrace   = 0

ProcessOptions:
  If args~items == 0 Then Signal Help

  option = args[1]
  args~delete(1)

  If option[1] == "-" Then Do
    Select Case lower(option)
      When "-l"                 Then list     = 1
      When "-h", "--help"       Then Signal Help
      When "-it", "--itrace"    Then itrace = 1
      When "-xtr", "--executor" Then executor = 1
      Otherwise Call Error "Invalid option '"option"'."
    End
    Signal ProcessOptions
  End

  filename = option

  moreArgs = args~makeString("L"," ")

  fullPath = .context~package~findProgram(filename)

  If fullPath == .Nil Then
    fullPath = .context~package~findProgram(filename".erx")

  If fullPath == .Nil Then Do
   .Error~Say( "Error 3:  Failure during initialization." )
   .Error~Say( "Error 3.901:  Failure during initialization: Program "filename" was not found." )
    Exit -3
  End

  -- We need to compute the source separately to properly handle syntax errors
  chunk = CharIn(fullPath,1,Chars(fullPath))
  source = chunk~makeArray
  -- Makearray has a funny definition that ignores a possible
  -- last empty line.
  If Right(chunk,1) == "0a"X Then source~append("")
  Call Stream fullPath, "C", "Close"

  options = .Array~of((Experimental,1))
  If executor Then options~append(("EXECUTOR",1))

  -- Parse our program
  parser = .Rexx.Parser~new(fullPath, source, options)

  package = parser~package

  element = parser~firstElement

  output = .Array.OutputStream~new

  -- Compile the program
  package~compile(element, output, .StringTable~new)

  -- Add a call to the enabler for experimental features
  If output[1][1] == "#" Then insertAt = 2 -- A shebang
  Else                        insertAt = 1
  -- Quoted, with the path that the search path finds, as "parser.cls"
  enabler = "parser/modules/experimental/EnableExperimentalFeatures.rex"
  output[insertAt] = 'Call "'enabler'" .Methods; 'output[insertAt]

  If list Then Do
    Say output
    Exit 0
  End

  -- Run the program we just compiled
 .routine~new(fullPath, output)~call(moreArgs)

  -- We are done
  Exit result

Help:
  Say .Resources[Help]~makeString        -
    ~caselessChangeStr("myName", myName) -
    ~caselessChangeStr("myHelp", myHelp)
  Exit 1

--------------------------------------------------------------------------------

Error:
 .Error~Say(Arg(1))
  Exit 1

--------------------------------------------------------------------------------

::Requires "parser.cls"                      -- The Parser
::Requires "parser/BaseClassesAndRoutines.cls"    -- For ArgArray et al
::Requires "parser/ErrorHandler.cls"              -- Standard error handling
::Requires "parser/CLISupport.cls"                -- InitCLI()
::Requires "parser/modules/identity.cls"         -- The Identity compiler

-- Load experimental modules

::Requires "parser/modules/experimental/classextensions.cls"

--------------------------------------------------------------------------------

::Class Array.OutputStream Public SubClass Array Inherit OutputStream

::Method Init
  Expose written
  written = 0

::Method Say
  Expose written

  Use Strict Arg string = ""

  If written == 0 Then self~append( string )
  Else                 self[self~last] ||= string

  written = 0

  Return 0

::Method CharOut
  Expose written

  Use Strict Arg string -- We don't implement start

  If written == 0 Then self~append( string )
  Else                 self[self~last] ||= string

  written = 1

--------------------------------------------------------------------------------

::Resource Help
myname -- Run a Rexx program with Experimental features

Usage:
  myname [OPTIONS] [FILE]

Runs the Rexx Parser against FILE, compiles the Experimental features,
and runs the resulting program.

If the only option is -h or --help, or if no arguments are present,
then display this help and exit.

Options:

  -l                Print the translated program and exit
  -it, --itrace     Print internal trace on error
  -xtr, --executor  Activate Executor support


The 'myname' program is part of the Rexx Parser package,
see https://rexx.epbcn.com/rexx-parser/. It is distributed under
the Common Public License 1.0 (https://www.oorexx.org/license.html).

Copyright (c) 2024-2026 Josep Maria Blasco <josep.maria.blasco@epbcn.com>.
Copyright (c) 2026 Rexx Language Association.

See myhelp for details.
::END