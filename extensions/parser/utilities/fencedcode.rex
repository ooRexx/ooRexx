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
/* fencedcode.rex - Highlight the Rexx fenced code blocks of a file           */
/* ================================================================           */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20261121         Split from highlight.rex, which keeps Rexx programs: the  */
/*                  fenced code blocks of a .md or .html file (RexxPub)       */
/*                  --css: the style is looked for in parser/css/ first       */
/* 20261121         Part of the highlighter, not of RexxPub (RFE #878): it    */
/*                  works without RexxPub, except for the cells of grid       */
/*                  tables and STATICSPOTS                                    */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261122         Only the options that FencedCode uses: --default,         */
/*                  --continue, --css and -it. The output is HTML, and the    */
/*                  rest (the mode, -n, --patch, --pad, --doccomments...) are */
/*                  attributes of the fences; the options, left over from     */
/*                  highlight.rex, did nothing. --css works without -h        */
/* 20261122         --css puts the style inside the page (StyleFile), and     */
/*                  links a rexx-STYLE.css of the current directory after     */
/*                  it, as highlight; no more links to rexx.epbcn.com and     */
/*                  file:////                                                 */
/*                                                                            */
/******************************************************************************/

--------------------------------------------------------------------------------
-- Run Main, and handle its errors                                            --
--------------------------------------------------------------------------------

  Do
    Call Main
  Catch co = User Rexx.Parser.Error -- An error in FILE, found by the parser
    Call ReportParseError co~additional, options.itrace == 1
    Exit - co~additional~major
  Catch co = Syntax
    -- A mistake in the document that FencedCode found is reported; anything
    -- else is an error of this program
    If \IsAParseError( co, options.itrace == 1 ) Then Do
     .Error~Say( "Error" co~code "in" co~program", line" co~position":" )
      Raise Propagate
    End
    Parse Value co~code With major"."
    Exit -major
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

  -- We will store our processed options in a stem. The output is HTML; how
  -- each block is highlighted is said by the attributes of its fence (or
  -- --default)
  options. = 0

  options.default     = ""
  options.style       = "dark"   -- The style sheet that --css puts in

  myPath              = FileSpec("Location",.context~package~name)
  sep                 = .File~separator

  Loop While args~size > 0, args[1] \== "-", args[1][1] == "-"
    option = args[1]
    args~delete(1)

    Select Case Lower(option)
      When "-h", "--help"      Then Signal Help
      When "--default"         Then Do
        If args~size == 0 Then
          Call Error "Missing attributes after '"option"' option."
        options.default = args[1]
        args~delete(1)
      End
      When "-it", "--itrace"   Then options.itrace       =  1
      When "--css"             Then options.css          =  1
      When "--continue"        Then options.continue     =  1

      Otherwise Call Error "Invalid option '"option"'."
    End
  End

  If args~items == 0 Then Signal Help

  If args~items > 1 Then Call Error "Invalid argument '"args[2]"'."

  file = args[1]

  -- A Rexx program is highlight's
  If file~caselessEndsWith(".rex") | file~caselessEndsWith(".cls") Then Do
   .Error~Say( myName "highlights the Rexx fenced code blocks of a Markdown" )
   .Error~Say( "or HTML file. To highlight the Rexx program" file"," )
   .Error~Say( "use highlight." )
    Exit 1
  End

  If file == "-" Then source = .Input~arrayIn
  Else Do
    fullPath = .context~package~findProgram(file)
    If fullPath == .Nil Then Call Error "File '"file"' does not exist."
    source = LoadProgramSource(fullPath)
  End

  If Options.css == 0 Then Do
    Say ProcessSource()
    Exit
  End

  mystyle = Options.style

  -- The style goes inside the page, so that it looks the same anywhere
  -- (StyleFile finds it as the Highlighter does, or says it is not there).
  -- A rexx-STYLE.css in the current directory, if there is one, is linked
  -- after it, so that it can be retouched without regenerating the page
  cssFile = StyleFile( mystyle )
  css     = CharIn( cssFile, 1, Chars(cssFile) )
  Call Stream cssFile, "C", "Close"
  local   = Stream(Directory()||sep"rexx-"mystyle".css","c","query exists")

  Do line Over .Resources[HTML]
    Select Case line
      When "[*CSS*]" Then Do
        Say "    <style>"
        Say css~strip("T", "0a"x)~strip("T", "0d"x)
        Say "    </style>"
        If local \== "" Then
          Say "    <link rel='stylesheet' href='rexx-"mystyle".css'>"
      End
      When "[*CONTENTS*]" Then Say ProcessSource()
      Otherwise Say line
    End
  End

  Return

ProcessSource:
  -- The style, Executor, CMS, Experimental and Unicode are the attributes of
  -- each fence (or --default): see doc/highlighter/fencedcode/
  Return FencedCode( file, source, , options. )

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

::Requires "parser/FencedCode.cls"
::Requires "parser/CLISupport.cls"
::Requires "parser/ErrorHandler.cls"
::Requires "parser/StyleSheet.cls"

::Resource Help
myname - Highlight the Rexx fenced code blocks of a Markdown or HTML file

Usage: myname [OPTIONS] FILE

If the only option is -h or --help, or if no arguments are present,
then display this help and exit.

Process all the Rexx fenced code blocks in FILE (a .md or .html file, or
"-" to read it from .input), highlight them as HTML, and write the file,
with its blocks highlighted, to .output. How each block is highlighted
(the style, line numbers, patches, Executor, CMS...) is said by the
attributes of its fence, or by --default. To highlight a Rexx program,
use highlight.

Options:
       --continue           Continue when a fenced code block is in error
       --css                Write a whole HTML page, with the style sheet
                            of the "dark" style inside
       --default attributes Default attributes for all the code blocks
  -it, --itrace             Print internal traceback on error

The 'myname' program is part of the Rexx Parser package,
see https://rexx.epbcn.com/rexx-parser/. It is distributed under
the Common Public License 1.0 (https://www.oorexx.org/license.html).

Copyright (c) 2024-2026 Josep Maria Blasco <josep.maria.blasco@epbcn.com>.
Copyright (c) 2026 Rexx Language Association.

See myhelp for details.
::END

::Resource HTML
<!doctype html>
<html lang='en'>
  <head>
[*CSS*]
  </head>
  <body>
[*CONTENTS*]
  </body>
</html>
::END