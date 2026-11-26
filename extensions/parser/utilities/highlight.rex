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
/* highlight.rex - Highlight a Rexx program                                   */
/* ========================================                                   */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241228    0.1d First public release                                      */
/* 20241229    0.1e Add support for ANSI highlighing                          */
/* 20241231         Add support for LaTeX highlighing                         */
/* 20250102    0.1f Add --prolog and --noprolog options                       */
/* 20250103         Add -u --tutor and --unicode options                      */
/* 20250105         Add --patch="patch specs" and --patchfile=file            */
/* 20250107         Change -t and --term for -a and --ansi                    */
/* 20250108         Add --pad= option                                         */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx.parser        */
/* 20250526    0.2b Add --css opt. & "-" to select .input (thanks, Rony!)     */
/* 20250529    0.2c Add support for detailed string and number highlighting   */
/* 20250706    0.2d Add support for detailed doc-comment highlighting         */
/* 20251029    0.2e Change .stdin to .input (thanks, Rony!)                   */
/* 20251114    0.3a Add support for Experimental features                     */
/* 20251125         Add support for Executor                                  */
/* 20251220    0.4a Disallow -xtr, etc when .md, .html, .htm                  */
/* 20251221         Add --itrace option, improve error messages               */
/* 20251226         Send error messages to .error, not .output                */
/* 20251226         Don't allow -s or --style for .md, improve error msgs     */
/* 20251227         Use .SysCArgs when available                              */
/* 20251228         Add support for --default                                 */
/* 20251230         Add support for --continue                                */
/* 20260102         Standardize help options to -h and --help                 */
/* 20260314    0.5  Use InitCLI() from CLISupport.cls                         */
/* 20261011         Help: an option takes its value as the next argument      */
/* 20261115         Use CATCH instead of SIGNAL ON SYNTAX (RFE #31; needs     */
/*                  ooRexx r13514 or later)                                   */
/*                  An error of this program is no longer taken for an error  */
/*                  in FILE (the handler ended in a 97.1)                     */
/* 20261115         The errors in FILE are reported by IsAParseError, from    */
/*                  ErrorHandler.cls, as in the other utilities; a bad option */
/*                  or a block with no end in a .md file gets a message, not  */
/*                  a traceback                                               */
/* 20261119         No ::Requires of ANSI.ErrorText.cls, which is gone:       */
/*                  ErrorHandler uses ERRORTEXT                               */
/* 20261119         A syntax error in FILE is the condition USER              */
/*                  REXX.PARSER.ERROR (THROW, RFE #31; ooRexx r13546 or       */
/*                  later), not SYNTAX 98.900                                 */
/* 20261121         Rexx programs only: the fenced code blocks of a .md or    */
/*                  .html file are fencedcode.rex's (RexxPub), and so are     */
/*                  --default and --continue. No ::Requires of FencedCode.cls */
/* 20261121         --css: the style is looked for in parser/css/ first       */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261122         --css puts the style inside the page (StyleFile), and     */
/*                  links a rexx-STYLE.css of the current directory after     */
/*                  it; no more links to rexx.epbcn.com and file:////. -l     */
/*                  without RexxPub's LaTeX driver says so, without a         */
/*                  traceback                                                 */
/* 20261126    1.0  A bad --patch or --patchfile is said in one line, without */
/*                  a traceback                                               */
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
  Catch co = Syntax                 -- An error of this program
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

  -- We will store our processed options in a stem
  options. = 0

  -- Set default mode to ANSI if called from the command line...
  Parse Source . how .
  If how == "COMMAND"
    Then options.mode = ANSI
    -- ...otherwise, assume HTML.
    Else options.mode = HTML

  myPath              = FileSpec("Location",.context~package~name)
  sep                 = .File~separator
  patch               = .Nil

  Loop While args~size > 0, args[1] \== "-", args[1][1] == "-"
    option = args[1]
    args~delete(1)

    Select Case Lower(option)
      When "--help"            Then Signal Help
      When "-s", "--style"     Then Do
        If args~size == 0 Then
          Call Error "Missing style after '"option"' option."
        options.style = args[1]
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
        patch = Patch( args[1], option )
        args~delete(1)
      End
      When "--patchfile"       Then Do
        If args~size == 0 Then
          Call Error "Missing value after '"option"' option."
        value = args[1]
        args~delete(1)
        file = Stream(value,"C", "Q Exists")
        If file == "" Then Call Error "File '"value"' not found."
        patch = Patch( File2Array( file ), option "'"value"'" )
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
      When "-xtr", "--executor"  Then options.executor     = 1
      When "-cms", "--cms",       -
           "-rexxvm", "--rexxvm" Then options.cms          = 1
      When "--css"               Then options.css          = 1
      When "-u", "--tutor", -
           "--unicode"           Then options.unicode = 1
      When "--noprolog"          Then options.prolog       = 0
      When "--prolog"            Then options.prolog       = 1
      When "-e", "-exp", "--experimental" Then
                                      options.experimental = 1

      Otherwise Call Error "Invalid option '"option"'."
    End
  End

  If options.css, options.mode \== "HTML" Then
    Call Error "The --css option cannot be used in" options.mode "mode."

  -- The LaTeX driver is RexxPub's: without it, -l says so (not a traceback)
  If options.mode == "LATEX", \.environment~hasIndex("LATEX.HIGHLIGHTER") Then
    Call Error "LaTeX mode needs the LaTeX driver (rexxpub/Latex.Driver.cls),",
      "which is part of RexxPub, and is not in the search path."

  -- This means that "highlight -h" will display help, which is good.
  If args~items == 0 Then Signal Help

  If args~items > 1 Then Call Error "Invalid argument '"args[2]"'."

  file = args[1]

  -- The fenced code blocks of a Markdown or HTML file are fencedcode's
  If file~caselessEndsWith(".md") | file~caselessEndsWith(".html") | -
     file~caselessEndsWith(".htm") Then Do
   .Error~Say( myName "highlights Rexx programs. To highlight the Rexx fenced" )
   .Error~Say( "code blocks of" file", use fencedcode: see" )
   .Error~Say( "https://rexx.epbcn.com/rexx-parser/doc/utilities/fencedcode/" )
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

  -- Pick selected style
  If Options.style == 0 Then mystyle = "dark"
  Else                       mystyle = Options.style

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
  hl =  .Highlighter~new(file, source, options.)
Return hl~parse( patch )

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

-- A patch that cannot be read is a mistake in the command line: said in
-- one line, without a traceback
Patch:
  Do
    Return .StylePatch~of( Arg(1) )
  Catch co = Syntax 88.900
    Call Error "Error in" Arg(2)":" co~additional~lastItem
  End

--------------------------------------------------------------------------------

Help:
  Say .Resources[Help]~makeString        -
    ~caselessChangeStr("myName", myName) -
    ~caselessChangeStr("myHelp", myHelp)
  Exit 1

--------------------------------------------------------------------------------

::Requires "parser/Highlighter.cls"
::Requires "parser/CLISupport.cls"
::Requires "parser/ErrorHandler.cls"
::Requires "parser/StyleSheet.cls"

::Resource Help
myname - Highlight a Rexx program

Usage: myname [OPTIONS] FILE

If the only option is -h or --help, or if no arguments are present,
then display this help and exit.

FILE is a Rexx program, or "-" to read it from .input. To highlight the
Rexx fenced code blocks of a Markdown or HTML file, use fencedcode.

Options:
  -a,  --ansi               Select ANSI mode
       --css                A whole page, with the style inside (HTML only)
  -d,  --docbook            Select DocBook mode
       --doccomments detailed|block Select highlighting level for doc-comments
  -xtr,--executor           Enable support for Executor
  -cms,--cms                Enable support for CMS (Classic Rexx) syntax (also -rexxvm, --rexxvm)
  -e, -exp, --experimental  Enable Experimental features
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
  -s,  --style STYLE        Use "rexx-STYLE.css" (default is "dark")
       --tutor              Enable TUTOR-flavored Unicode
  -u,  --unicode            Enable TUTOR-flavored Unicode
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