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

  -- We will store our processed options in a stem
  options. = 0

  -- Set default mode to ANSI if called from the command line...
  Parse Source . how .
  If how == "COMMAND"
    Then options.mode = ANSI
    -- ...otherwise, assume HTML.
    Else options.mode = HTML

  options.default     = ""
  options.style       = "dark"   -- Use attributes in the fences, or --default

  myPath              = FileSpec("Location",.context~package~name)
  sep                 = .File~separator
  patch               = .Nil

  Loop While args~size > 0, args[1] \== "-", args[1][1] == "-"
    option = args[1]
    args~delete(1)

    Select Case Lower(option)
      When "--help"            Then Signal Help
      When "--default"         Then Do
        If args~size == 0 Then
          Call Error "Missing attributes after '"option"' option."
        options.default = args[1]
        args~delete(1)
      End
      When "--doccomments"     Then Do
        If args~size == 0 Then
          Call Error "Missing value after '"option"' option."
        value = args[1]
        args~delete(1)
        If WordPos(value,"detailed block") == 0 Then
          Call Error "Invalid value for --doccomments: '"value"'."
        options.doccomments = value
      End
      When "--patch"           Then Do
        If args~size == 0 Then
          Call Error "Missing value after '"option"' option."
        patch = .StylePatch~of( args[1] )
        args~delete(1)
      End
      When "--patchfile"       Then Do
        If args~size == 0 Then
          Call Error "Missing value after '"option"' option."
        value = args[1]
        args~delete(1)
        file = Stream(value,"C", "Q Exists")
        If file == "" Then Call Error "File '"value"' not found."
        value = File2Array( file )
        patch = .StylePatch~of( value )
      End

      When "--startfrom"         Then options.startFrom    = Natural(value)
      When "-w", "--width"       Then options.width        = Natural(value)
      When "--pad"               Then options.pad          = Natural(value)

      When "-it", "--itrace"     Then options.itrace       =  1
      When "-h", "--html"        Then options.mode         =  HTML
      When "-l", "--latex"       Then options.mode         =  LaTeX
      When "-d", "--docbook"     Then options.mode         =  DOCBOOK
      When "-n", "--numberlines" Then options.numberlines  = .True
      When "-a", "--ansi"        Then options.mode         =  ANSI
      When "--css"               Then options.css          = 1
      When "--noprolog"          Then options.prolog       = 0
      When "--prolog"            Then options.prolog       = 1
      When "--continue"          Then options.continue     = 1

      Otherwise Call Error "Invalid option '"option"'."
    End
  End

  If options.css, options.mode \== "HTML" Then
    Call Error "The --css option cannot be used in" options.mode "mode."

  -- This means that "highlight -h" will display help, which is good.
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

  -- A possible copy could reside in our installation: parser/css/, next to
  -- us, or the css/ directory of the tree
  cssPath = mypath"parser"sep"css"sep"rexx-"mystyle".css"
  If Stream(cssPath,"c","query exists") == "" Then
    cssPath = mypath".."sep"css"sep"rexx-"mystyle".css"
  cssPath = ChangeStr("\",cssPath,"/")
  -- A possible copy could reside in the current directory
  local   = Stream(Directory()||sep"rexx-"mystyle".css","c","query exists")

  Do line Over .Resources[HTML]
    Select Case line
      When "[*CSS*]" Then Do
        Say  "    <link rel='stylesheet' href='https://rexx.epbcn.com/rexx-parser/css/rexx-"mystyle".css'></link>"
        Say  "    <link rel='stylesheet' href='file:///"cssPath"'></link>"
        If local \== "" Then Say  "    <link rel='stylesheet' href='rexx-"mystyle".css'></link>"
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

After:
  If args~size == 0 Then
    Call Error "Missing value after '"option"' option."
  Return args~delete(1)

Natural:
  If args~size == 0 Then
    Call Error "Missing number after '"option"' option."
  n = args[1]
  args~delete(1)
  If DataType(n,"W"), n > 0 Then Return n
  Call Error "Positive whole number expected after '"option"', found '"n"'."

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

::Resource Help
myname - Highlight the Rexx fenced code blocks of a Markdown or HTML file

Usage: myname [OPTIONS] FILE

If the only option is -h or --help, or if no arguments are present,
then display this help and exit.

Process all the Rexx fenced code blocks in FILE (a .md or .html file, or
"-" to read it from .input) and highlight them. The style, Executor, CMS,
Experimental and Unicode are attributes of each fence (or --default).
To highlight a Rexx program, use highlight.

Options:
  -a,  --ansi               Select ANSI mode
       --continue           Continue when a fenced code block is in error
       --css                Include links to css files (HTML only)
  -d,  --docbook            Select DocBook mode
       --default attributes Select default attributes for code blocks
       --doccomments detailed|block Select highlighting level for doc-comments
  -h,  --html               Select HTML mode
  -it, --itrace             Printing internal traceback on error
  -l,  --latex              Select LaTeX mode
       --noprolog           Do not print a prolog (LaTeX only)
  -n,  --numberlines        Print line numbers
       --patch "PATCHES"    Apply semicolon-separated PATCHES
       --patchfile FILE     Load patches from FILE
       --pad N              Pad doc-comments and ::resources to N characters
       --prolog             Print a prolog (LaTeX only)
       --startFrom N        Start line numbers at N
  -w,  --width N            Ensure that all lines have width >= N (ANSI only)

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