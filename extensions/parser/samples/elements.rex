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
/* elements.rex - Transform a file into a list of elements                    */
/* =======================================================                    */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241206    0.1  First public release                                      */
/* 20250103    0.1f Add TUTOR-flavored Unicode support                        */
/* 20250215    0.1g Rename to elements.rex                                    */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/*                  Binary directory is now "bin" instead of "cls"            */
/*                  Move "modules" directory inside "bin"                     */
/* 20250426    0.2b Fix compound count, simplify REQUIRES                     */
/* 20250606    0.2c Add --from and --to options                               */
/* 20250928    0.2e Fix crash when no args, add .rex to file when needed      */
/* 20251114    0.3a Add support for Experimental features                     */
/* 20251125         Add support for Executor                                  */
/* 20251221    0.4a Add --itrace option, improve error messages               */
/* 20251226         Send error messages to .error, not .output                */
/* 20251227         Use .SysCArgs when available                              */
/* 20260102         Standardize help options to -h and --help                 */
/* 20260228         Add --[no-]show-spaces options (suggestion by RGF)        */
/* 20260313    0.5  Move error handling to ErrorHandler.cls                   */
/* 20261115    0.7  Use CATCH instead of SIGNAL ON SYNTAX (RFE #31; needs     */
/*                  ooRexx r13514 or later)                                   */
/* 20261119         A syntax error in FILE is the condition USER              */
/*                  REXX.PARSER.ERROR (THROW, RFE #31; ooRexx r13546 or       */
/*                  later), not SYNTAX 98.900                                 */
/* 20261121         No ::Requires of modules/print/print.cls: nothing here    */
/*                  uses it (RFE #878: modules/ need not be installed)        */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/*                                                                            */
/******************************************************************************/

--------------------------------------------------------------------------------
-- Run Main, and handle its errors                                            --
--------------------------------------------------------------------------------

  Do
    Call Main
  Catch co = User Rexx.Parser.Error -- An error in FILE, found by the parser
    Exit ErrorHandler( fullPath, source, co, itrace )
  Catch co = Syntax        -- Anything else is an error of this program
   .Error~Say( "Error" co~code "in" co~program", line" co~position":" )
    Raise Propagate
  End

  Exit 0

--------------------------------------------------------------------------------
-- Main program                                                               --
--------------------------------------------------------------------------------

Main:

  CLIhelper = InitCLI()
  myName = CLIhelper~name
  myHelp = CLIhelper~help
  args   = CLIhelper~args

  unicode        = 0
  experimental   = 0
  executor       = 0
  cms            = 0
  opFrom         = 1
  itrace         = 0
  opTo           = "*"
  showSpaces     = 1

ProcessOptions:
  If args~items == 0 Then Signal Help

  option = args[1]
  args~delete(1)

  If option[1] == "-" Then Do
    Select Case Lower(option)
      When "-h", "--help"        Then Signal Help
      When "-u", "--tutor", -
        "--unicode"              Then unicode = 1
      When "-e", "-exp", -
        "--experimental"         Then experimental = 1
      When "-xtr", "--executor"  Then executor = 1
      When "-cms", "--cms",       -
           "-rexxvm", "--rexxvm" Then cms = 1
      When "-it", "--itrace"     Then itrace = 1
      When "--from"              Then opFrom = Integer()
      When "--to"                Then opTo   = Integer()
      When "--show-spaces"       Then showSpaces = 1
      When "--no-show-spaces"    Then showSpaces = 0
      Otherwise Call Error "Invalid option '"option"'."
    End
    Signal ProcessOptions
  End

  If args~items > 0 Then Call Error "Invalid argument '"args[1]"'."

  file = option

  fullPath = .context~package~findProgram(file)

  If fullPath == .Nil Then Call Error "File '"file"' does not exist."

  -- We need to compute the source separately to properly handle syntax errors
  source = LoadProgramSource(fullPath)

  -- Adjust "opTo" if necessary

  If opTo = "*"          Then opTo = source~items
  If opTo > source~items Then opTo = source~items

  -- Print a nice prolog
  Say myName".rex run on" Date() "at" Time()
  Say
  Say "Examining" fullPath"..."
  Say
  Say "Elements marked '>' are inserted by the parser."
  Say "Elements marked 'X' are ignorable."
  Say "Elements marked 'A' have isAssigned=1."
  Say "Compound symbol components are distinguished with a '->' mark."
  Say
  Say "[   from  :    to   ] >XA 'value' (class)"
  Say " --------- ---------  --- ---------------------------"

  -- Parse our program, and get the first element
  Options = .Array~new
  If Unicode      Then Options~append(("UNICODE", 1))
  If experimental Then Options~append(("EXPERIMENTAL", 1))
  If executor     Then Options~append(("EXECUTOR", 1))
  If cms          Then Options~append(("CMS", 1))
  parser = .Rexx.Parser~new(file, source, Options)

  element  = parser~firstElement

  -- Iterate over all elements and print them
  elements = 0
  compound = 0
  Do Counter elements Until element == .Nil
    Parse Value element~from With line .
    If line >  opTo   Then Leave
    If line >= opFrom Then Call Print element
    element = element~next
  End
  Say "Total:" elements "elements and" compound "compound symbol elements examined."

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

Integer:
  n = ""
  If args~items > 0 Then Do
    n = args[1]
    args~delete(1)
  End
  If DataType(n,"W"), n > 0 Then Return n
  Call Error "Positive whole number expected, found '"n"'."

--------------------------------------------------------------------------------

BadArgument:
  Call Error "Incorrect file specification:" file

--------------------------------------------------------------------------------

Print:
  class = element~category
  from = element~from
  to   = element~to
  Parse Var from fromLine fromCol
  Parse Var   to   toLine   toCol
  Call Chunk "["Extent(element)"]"
  Call Chunk (element~isInserted)~?(" >","  ")
  Call Chunk (element~ignorable)~?("X"," ")
  Call Chunk (element~isAssigned)~?("A"," ")
  If class \== .EL.RESOURCE_DATA Then Do
    value = element~value
    If showSpaces Then
      value = ChangeStr(" ",value,"␣")
  End
  Else value = "[... resource data ...]"
  Call Chunk " '"value"'"
  If class == .EL.TAKEN_CONSTANT Then Do
    Say " ("AorAN(ConstantName(element~subCategory))" taken_constant)"
  End
  Else Do
    Say " ("AorAn(CategoryName(element~category))")"
    If element < .ALL.COMPOUND_VARIABLES Then Call Compound
  End
  If value == .Nil Then
    Call Error "Unexpected .Nil value for element condition."
Return

--------------------------------------------------------------------------------

Extent: Procedure
  Use Arg element
  from = element~from
  to   = element~to
  Parse Var from fromLine fromCol
  Parse Var   to   toLine   toCol
Return Right(fromLine,5) Right(fromCol,3)":"Right(toLine,5) Right(toCol,3)

--------------------------------------------------------------------------------

Compound:
  compound += 1
  Do part Over element~parts
    Say " "Extent(part)"     -> '"part~value"' ("||,
      AorAN(.Parser.CategoryName[part~category])")"
  End
Return

--------------------------------------------------------------------------------

AorAN:
  If "AEIOU"~contains(Arg(1)[1]) Then Return "An" Arg(1)
  Return "A" Arg(1)

--------------------------------------------------------------------------------

Chunk:
  Call CharOut , Arg(1)
Return

--------------------------------------------------------------------------------
-- Help text                                                                  --
--------------------------------------------------------------------------------

::Requires "parser.cls"
::Requires "parser/Globals.cls"
::Requires "parser/ErrorHandler.cls"
::Requires "parser/CLISupport.cls"
::Requires "parser/BaseClassesAndRoutines.cls"

::Resource Help end "::End"
myname - Transform a file into its constituent elements and list them

Usage: myName [options] FILE

Options:
-xtr,--executor     Enable support for Executor
-cms,--cms          Enable support for CMS (Classic Rexx) syntax (also -rexxvm, --rexxvm)
-e,  --experimental Enable Experimental features (also -exp)
     --from [LINE]  Show elements starting at line LINE
     --help         Display this information
-it, --itrace       Print internal traceback on error
     --to   [LINE]  Stop showing elements after line LINE
     --tutor        Enable TUTOR-flavored Unicode
-u,  --unicode      Enable TUTOR-flavored Unicode
--show-spaces       Display spaces as "␣" (requires UTF8)
--no-show-spaces    Display spaces as spaces

The 'myname' program is part of the Rexx Parser package,
see https://rexx.epbcn.com/rexx-parser/. It is distributed under
the Common Public License 1.0 (https://www.oorexx.org/license.html).

Copyright (c) 2024-2026 Josep Maria Blasco <josep.maria.blasco@epbcn.com>.
Copyright (c) 2026 Rexx Language Association.

See myhelp for details.
::End