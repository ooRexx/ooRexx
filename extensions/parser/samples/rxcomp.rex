#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026 Josep Maria Blasco. All rights reserved.                */
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
/* rxcomp.rex - Compare two rexx files element-wise                           */
/* ================================================                           */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20260117    0.4a First release                                             */
/* 20260313    0.5  Move error handling to ErrorHandler.cls                   */
/* 20260314         Use InitCLI() from CLISupport.cls                         */
/* 20261115    0.7  Use CATCH instead of SIGNAL ON SYNTAX (RFE #31; needs     */
/*                  ooRexx r13514 or later)                                   */
/* 20261119         A syntax error in FILE is the condition USER              */
/*                  REXX.PARSER.ERROR (THROW, RFE #31; ooRexx r13546 or       */
/*                  later), not SYNTAX 98.900                                 */
/* 20261121         No ::Requires of modules/print/print.cls: nothing here    */
/*                  uses it (RFE #878: modules/ need not be installed)        */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261121         A #! line, as the other utilities                         */
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

  Exit

--------------------------------------------------------------------------------
-- Main program                                                               --
--------------------------------------------------------------------------------

Main:

  CLIhelper    = InitCLI()
  myName       = CLIhelper~name
  myHelp       = CLIhelper~help
  args         = CLIhelper~args

  If args~items == 0 Then Signal Help

  unicode      = 0
  experimental = 0
  executor     = 0
  itrace       = 0

ProcessOptions:
  If args~items == 0 Then Signal Help

  option = args[1]
  args~delete(1)

  If option[1] == "-" Then Do
    Select Case Lower(option)
      When "-h", "--help"       Then Signal Help
      When "-u", "--tutor", -
        "--unicode"             Then unicode = 1
      When "-e", "-exp",  -
        "--experimental"        Then experimental = 1
      When "-xtr", "--executor" Then executor = 1
      When "-it", "--itrace"    Then itrace = 1
      Otherwise Call Error "Invalid option '"option"'."
    End
    Signal ProcessOptions
  End

  sfile1 = option -- Remember file1 as written

  file1 = .context~package~findProgram(option)

  If file1 == .Nil Then Call Error "File '"option"' does not exist."

  If args~items == 0 Then Call Error "Missing filename after '"option"'."

  option = args[1]
  args~delete(1)

  sfile2 = option -- Remember file2 as written

  file2 = .context~package~findProgram(option)

  If file2 == .Nil Then Call Error "File '"option"' does not exist."

  If args~items > 0 Then Call Error "Invalid parameter '"args[1]"'."

  Options = .Array~new
  If Unicode      Then Options~append(("UNICODE", 1))
  If experimental Then Options~append(("EXPERIMENTAL", 1))
  If executor     Then Options~append(("EXECUTOR", 1))

  source   = LoadProgramSource(file1)
  fullpath = file1
  parser1  = .Rexx.Parser~new(file1, source, Options)
  element1 = parser1~firstElement

  source   = LoadProgramSource(file2)
  fullpath = file2
  parser2  = .Rexx.Parser~new(file2, source, Options)
  element2 = parser2~firstElement

  Loop While element1 \< .EL.END_OF_SOURCE, element2 \< .EL.END_OF_SOURCE
    Call Next
    from1 = element1~from
    from2 = element2~from
    to1   = element1~to
    to2   = element2~to
    cat1  = element1~category
    cat2  = element2~category
    src1  = element1~source
    src2  = element2~source
    If cat1 \== cat2 Then Call CategoriesDiffer
    If element1 < .ALL.SYMBOLS_AND_KEYWORDS Then Call CaselessCompare
    Else If element1 < .EL.TAKEN_CONSTANT, Pos(src1[1], "'""") == 0 Then Call CaselessCompare
    Else Call Compare
  End

  If element1 \< .EL.END_OF_SOURCE Then Do
    Say file2 "exhausted but" file1 "continues at ["from1"-"to1"]: '"src1"'."
    Exit 1
  End

  If element2 \< .EL.END_OF_SOURCE Then Do
    Say file1 "exhausted but" file2 "continues at ["from2"-"to2"]: '"src2"'."
    Exit 1
  End

  Return

CategoriesDiffer:
  Say sfile1 "@" from1"-"to1 "is '"src1"' (a" .Parser.CategoryName[cat1]"), but" .EndOfLine || -
    sfile2 "@" from2"-"to2 "is '"src2"' (a" .Parser.CategoryName[cat2]")."
  Exit 1

CaselessCompare:
  val1  = element1~value
  val2  = element2~value
  If val1 == val2 Then Return
  Say "Elements '"val1"' ["from1"-"to1"] and '"val2"' ["from2"-"to2"] differ."
  Exit 1

Compare:
  If src1 == src2 Then Return
  Say "Elements '"src1"' ["from1"-"to1"] (a" .Parser.CategoryName[cat1]") and '"src2"' ["from2"-"to2"] (a" .Parser.CategoryName[cat2]") differ."
  Exit 1

Next:
  Call Next1
  Call Next2
  Return

Next1:
  element1 = element1~next
  If element1 < .EL.END_OF_SOURCE Then Return
  If element1~ignorable           Then Signal Next1
  If element1~from == element1~to Then Signal Next1
  Return

Next2:
  element2 = element2~next
  If element2 < .EL.END_OF_SOURCE Then Return
  If element2~ignorable           Then Signal Next2
  If element2~from == element2~to Then Signal Next2
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
::Requires "parser/BaseClassesAndRoutines.cls"

--------------------------------------------------------------------------------

::Resource Help end "::End"
myname - Compare two Rexx files element-wise

Usage: myName [options] FILE1 FILE2

Options:
-xtr,--executor     Enable support for Executor
-e,  --experimental Enable Experimental features (also -exp)
     --help         Display this information
-it, --itrace       Print internal traceback on error
     --tutor        Enable TUTOR-flavored Unicode
 -u, --unicode      Enable TUTOR-flavored Unicode

The 'myname' program is part of the Rexx Parser package,
see https://rexx.epbcn.com/rexx-parser/. It is distributed under
the Common Public License 1.0 (https://www.oorexx.org/license.html).

Copyright (c) 2024-2026 Josep Maria Blasco <josep.maria.blasco@epbcn.com>.
Copyright (c) 2026 Rexx Language Association.

See myhelp for details.
::End