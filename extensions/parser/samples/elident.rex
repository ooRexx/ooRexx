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
/* elident.rex - Check that a program is equal to its Element API parsing     */
/* ======================================================================     */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241206    0.1  First public release                                      */
/* 20241208    0.1a c/CLASSIC_COMMENT/STANDARD_COMMENT/                       */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20251110    0.3a Change the name to elident.rex                            */
/* 20251211         Add Executor support, move to /bin                        */
/* 20251218         Add TUTOR support                                         */
/* 20251221    0.4a Add --itrace option, improve error messages               */
/* 20251226         Send error messages to .error, not .output                */
/* 20251227         Use .SysCArgs when available                              */
/* 20260103         Use compound variable parts                               */
/* 20260307    0.5  Implement experimental option                             */
/* 20260314         Use InitCLI() from CLISupport.cls                         */
/* 20261115    0.7  Use CATCH instead of SIGNAL ON SYNTAX (RFE #31; needs     */
/*                  ooRexx r13514 or later)                                   */
/* 20261119         A syntax error in FILE is the condition USER              */
/*                  REXX.PARSER.ERROR (THROW, RFE #31; ooRexx r13546 or       */
/*                  later), not SYNTAX 98.900                                 */
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
    Exit ErrorHandler( file, source, co, itrace )
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
      When "--executor", "-xtr"           Then executor     = 1
      When "--cms", "-cms",               -
           "--rexxvm", "-rexxvm"          Then cms          = 1
      When "-e", "-exp", "--experimental" Then experimental = 1
      When "-it", "--itrace"              Then itrace       = 1
      When "-u", "--tutor", "--unicode"   Then unicode      = 1
      Otherwise Call Error "Invalid option '"option"'."
    End
    Call ProcessOptions
  End

  If args~items > 0 Then Call Error "Invalid argument '"args[1]"'."

  file = option

  fullPath = .context~package~findProgram(file)

  If fullPath == .Nil Then Call Error "File '"file"' does not exist."

  -- Read the whole file into an array
  chunk = CharIn(fullPath,1,Chars(fullPath))
  Call Stream fullPath,"c","close"
  source = chunk~makeArray
  -- Makearray has a funny definition that ignores a possible
  -- last empty line.
  If Right(chunk,1) = "0a"X Then source~append("")

  options = .Array~new
  If executor     Then options~append(("EXECUTOR",     1))
  If cms          Then options~append(("CMS",          1))
  If unicode      Then options~append(("UNICODE",      1))
  If experimental Then options~append(("EXPERIMENTAL", 1))

  parser = .Rexx.Parser~new( file, source, options )

  currentLineNo = 1
  currentLine   = ""

  element = parser~firstElement -- Same as parser~package~prolog~body~begin
  Do Counter elements Until element == .Nil
    If element~from \== element~to Then Do
      category = element~category
      elementLine  = element~from~word(1)
      If      elementLine > currentLineNo          Then Call ChangeLine
      If      category == .EL.STANDARD_COMMENT     Then Call StandardComment
      Else If category == .EL.DOC_COMMENT          Then Call StandardComment
      Else If category == .EL.DOC_COMMENT_MARKDOWN Then Call StandardComment
      Else If category == .EL.RESOURCE_DATA        Then Call ResourceData
      Else If category == .EL.EXPOSED_COMPOUND_VARIABLE | -
              category == .EL.COMPOUND_VARIABLE    Then Call CompoundVariable
      Else    currentLine ||= element~source
    End
    element = element~next
  End

  Return

Help:
  Say .Resources[Help]~makeString        -
    ~caselessChangeStr("myName", myName) -
    ~caselessChangeStr("myHelp", myHelp)
  Exit 1

CompoundVariable:
  Do part Over element~parts
    currentLine ||= part~source
  End
  Return

StandardComment:
  lastLine = element~to~word(1)
  start = element~from~word(2)
  end   = element~  to~word(2)
  If elementLine == lastLine Then Do
    currentLine ||= source[currentLineNo][ start, end-start ]
    Return
  End
  elementLine += 1
  currentLine ||= SubStr( source[currentLineNo], start )
  Call ChangeLine
  currentLineNo = lastLine
  currentLine   = source[currentLineNo][1, end - 1]
Return

ResourceData:
  lastLine = element~to~word(1)
  currentLineNo = lastLine + 1
  currentLine   = ""
Return

ChangeLine:
  Do While elementLine > currentLineNo
    If source[currentLineNo] \== currentLine Then Do
      Say( "Difference found in line number" currentLineNo":" )
      Say "Source line is '"source[currentLineNo]"',"
      Say "Parsed line is '"currentLine"'."
      Exit 1
    End
    currentLineNo += 1
    currentLine    = ""
  End
Return

--------------------------------------------------------------------------------

Error:
 .Error~Say(Arg(1))
  Exit 1

::Resource HELP
myname -- Checks that the Parser' stream of elements is identical to a FILE.

Usage: myname [OPTION]... [FILE]

If the only option is -h or --help, or if no arguments are present,
then display this help and exit.

Options:

-it, --itrace             Print internal trace on error
-u, --tutor, --unicode    Enable TUTOR-flavored Unicode
-e, -exp, --experimental  Enable Experimental features
-xtr, --executor          Activate support for Executor language extensions
-cms, --cms               Activate support for CMS (Classic Rexx) syntax (also -rexxvm, --rexxvm)

The 'myname' program is part of the Rexx Parser package,
see https://rexx.epbcn.com/rexx-parser/. It is distributed under
the Common Public License 1.0 (https://www.oorexx.org/license.html).

Copyright (c) 2024-2026 Josep Maria Blasco <josep.maria.blasco@epbcn.com>.
Copyright (c) 2026 Rexx Language Association.

See myHelp for details.
::END

::Requires "parser.cls"
::Requires "parser/BaseClassesAndRoutines.cls"
::Requires "parser/ErrorHandler.cls"
::Requires "parser/CLISupport.cls"