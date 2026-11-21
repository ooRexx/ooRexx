#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2025-2026 Josep Maria Blasco. All rights reserved.           */
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
/* rexxcheck.rex - Run the parser with all the early checks activated         */
/* ==================================================================         */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20250416    0.2a First release                                             */
/*             0.2a Add "[+|-]debug" option                                   */
/* 20250421    0.2b Add "[+|-]e" option                                       */
/*                  Add "[+|-]extraletters" option                            */
/*                  Add "[+|-]emptyassignments" option                        */
/* 20250426         Add ANSI.ErrorText support, -itrace option                */
/* 20250508         Fix typo (GitHub issue no. 10 - Thanks Geoff!)            */
/* 20250831    0.2e Add support for LEAVE and INTERPRET checks                */
/* 20250929         Add ".rex" to filename when appropriate                   */
/* 20251114    0.3a Add support for Experimental features                     */
/* 20251125         Add support for Executor                                  */
/* 20251129         -e option does not need quotes now                        */
/* 20251226    0.4a Send error messages to .error, not .output                */
/* 20251227         Use .SysCArgs when available                              */
/* 20260102         Standardize help options to -h and --help                 */
/* 20260314    0.5  Use InitCLI() from CLISupport.cls                         */
/* 20260521         Add support for CMS                                       */
/* 20260926    0.7  Add "[+|-]expose" and "[+|-]uselocal" (EXPOSE and USE     */
/*                  LOCAL out of a method: 98.992, 98.993 at run time)        */
/* 20260928         Add "[+|-]parse" (a PARSE positional pattern that is not  */
/*                  a whole number: 26.4 at run time)                         */
/*                  The help lists "[+|-]leave" and "[+|-]iterate".           */
/* 20260930         Add "--warnings" ("--warn"): things that are legal, but   */
/*                  most probably not what was meant (GitHub issue #39)       */
/* 20261115         Use CATCH instead of SIGNAL ON SYNTAX (RFE #31; needs     */
/*                  ooRexx r13514 or later)                                   */
/* 20261119         A syntax error in FILE is the condition USER              */
/*                  REXX.PARSER.ERROR (THROW, RFE #31; ooRexx r13546 or       */
/*                  later), not SYNTAX 98.900                                 */
/* 20261121         No ::Requires of modules/print/print.cls: nothing here    */
/*                  uses it (RFE #878: modules/ need not be installed)        */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261121         Renamed from rxcheck.rex (RFE #878: rexxtry, rexxcps...)  */
/* 20261121         A #! line, as the other utilities                         */
/*                                                                            */
/******************************************************************************/

  CLIhelper    = InitCLI()
  myName       = CLIhelper~name
  myHelp       = CLIhelper~help
  args         = CLIhelper~args

  If args~items == 0 Then Signal Help

  signal           = 1
  guard            = 1
  expose           = 1
  uselocal         = 1
  parsepos         = 1
  leave            = 1
  iterate          = 1
  bifs             = 1
  debug            = 0
  itrace           = 0
  experimental     = 0
  executor         = 0
  cms              = 0
  extraletters     = ""
  emptyassignments = 0
  warnings         = 0

  Loop While args~items > 0, "+-"~contains(Left(args[1],1))
    option = args[1]
    args~delete(1)
    Select Case Lower(option)
      When "-h", "--help" Then Signal Help
      When "--warnings", "--warn" Then warnings = 1
      When "-all" Then Do
        signal   = 0
        guard    = 0
        expose   = 0
        uselocal = 0
        parsepos = 0
        bifs     = 0
        leave    = 0
        iterate  = 0
      End
      When "+all" Then Do
        signal   = 1
        guard    = 1
        expose   = 1
        uselocal = 1
        parsepos = 1
        bifs     = 1
        leave    = 1
        iterate  = 1
      End
      When "-signal"       Then signal   = 0
      When "+signal"       Then signal   = 1
      When "-leave"        Then leave    = 0
      When "+leave"        Then leave    = 1
      When "-iterate"      Then iterate  = 0
      When "+iterate"      Then iterate  = 1
      When "-guard"        Then guard    = 0
      When "+guard"        Then guard    = 1
      When "-expose"       Then expose   = 0
      When "+expose"       Then expose   = 1
      When "-uselocal"     Then uselocal = 0
      When "+uselocal"     Then uselocal = 1
      When "-parse"        Then parsepos = 0
      When "+parse"        Then parsepos = 1
      When "-bifs"         Then bifs     = 0
      When "+bifs"         Then bifs     = 1
      When "-debug"        Then debug    = 0
      When "+debug"        Then debug    = 1
      When "-itrace"       Then itrace   = 0
      When "+itrace"       Then itrace   = 1
      When "+cms", "-cms",       -
           "+rexxvm", "-rexxvm" Then cms      = 1
      When "-xtr", "+xtr",-
           "-executor",  -
           "+executor"     Then executor = 1
      When "-experimental", -
           "+experimental", -
           "-exp", "+exp"  Then experimental = 1
      When "-emptyassignments", -
        "+emptyassignments" Then emptyassignments = 1
      When "-e", "+e"      Then Do
        code   = args~makeString("L", " ")
        source = .Array~of( code )
        fullPath = "INSTORE"
        Signal code
      End
      When "-extraletters", "+extraletters" Then Do
        If args~size ==  0 Then
          Call "Missing set of letters after '"option"' option."
        extraletters = args[1]
        args~delete(1)
      End
      Otherwise Call Error "Invalid option '"option"'."
    End
  End

  Select Case args~items
    When 0 Then Signal Help
    When 1 Then file = args[1]
    Otherwise Call Error "Unexpected argument '"args[2]"'."
  End

  fullPath = .context~package~findProgram(file)

  If fullPath == .Nil Then
    Call Error "File '"file"' does not exist."

  source = LoadProgramSource(fullPath)

Code:

  check = .Array~new
  If signal   Then check~append("SIGNAL")
  If guard    Then check~append("GUARD")
  If expose   Then check~append("EXPOSE")
  If uselocal Then check~append("USELOCAL")
  If parsepos Then check~append("PARSE")
  If bifs     Then check~append("BIFS")
  If leave    Then check~append("LEAVE")
  If iterate  Then check~append("ITERATE")
  Options = .Array~of( (earlyCheck, check ) )

  If extraletters \== "" Then Options~append(("EXTRALETTERS", extraletters))
  If emptyassignments    Then Options~append(("EMPTYASSIGNMENTS", emptyassignments))
  If experimental        Then Options~append(("EXPERIMENTAL", 1))
  If executor            Then Options~append(("EXECUTOR", 1))
  If cms                 Then Options~append(("CMS", 1))

  Do

    If debug Then .environment~rexxcheck.debug = 1

    package = .Rexx.Parser~new(fullPath,source,options)~package

    found = 0
    If warnings Then Do
      Do warning Over Warnings( package )
        found += 1
       .Error~Say( Right(warning~position,6) "*-*" warning~clause )
       .Error~Say( "Warning in" fullPath", line" warning~position":" -
          warning~message )
      End
    End

    Select
      When found == 0 Then Say "No errors found."
      When found == 1 Then Say "No errors found, 1 warning."
      Otherwise            Say "No errors found," found "warnings."
    End
  Catch co = User Rexx.Parser.Error -- An error in FILE, found by the parser
    Exit ErrorHandler( fullPath, source, co, itrace )
  Catch co = Syntax        -- Anything else is an error of this program
   .Error~Say( "Error" co~code "in" co~program", line" co~position":" )
    Raise Propagate
  End

  Exit

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
::Requires "parser/Warnings.cls"
::Requires "parser/CLISupport.cls"

::Resource help
myname -- Parse a program or a short code fragment

Usage:
  myname [OPTIONS] FILE
  myname [OPTIONS] -e "REXX CODE"

Runs the Rexx Parser against FILE or the supplied REXX CODE.
By default, the parser perform a series of early checks,
without needing to execute the program. Checks are performed
syntactically, and therefore they reach dead branches,
uncalled procedures and routines, etc.

If the only option is -h or --help, or if no arguments are present,
then display this help and exit.

Toggles:

  +all          Activate all toggles. This is the default.
  -all          Deactivate all toggles.
  [+|-]signal   Toggle detecting SIGNAL to inexistent labels.
  [+|-]leave    Toggle detecting incorrect LEAVEs, or LEAVEs to
                inexistent targets.
  [+|-]iterate  Toggle detecting incorrect ITERATEs, or ITERATEs to
                inexistent targets.
  [+|-]guard    Toggle checking that GUARD is in a method body.
  [+|-]expose   Toggle checking that EXPOSE is in a method body.
  [+|-]uselocal Toggle checking that USE LOCAL is in a method body.
  [+|-]parse    Toggle checking that PARSE positional patterns are
                whole numbers.
  [+|-]bifs     Check BIF arguments.

  [+|-]debug    (De)activate debug mode (not affected by "all").
  [+|-]itrace   Toggle printing internal traceback on error

Warnings:

  --warnings    Also report things that are legal, but most probably
  --warn        not what was meant: a ::CLASS option after INHERIT
                ("::CLASS c INHERIT d PUBLIC" inherits a class PUBLIC),
                or a label followed, on the same line, by an
                instruction that starts with a symbol ("ns:class~new"
                is the label "ns" and "class~new", with or without
                blanks; "(ns:class)~new" is the qualified name).
                Warnings do not change the return code.

Other options (all can be prefixed with "+" or "-"):

  -cms          Check CMS (Classic Rexx) syntax (also -rexxvm)
  -executor     Enable support for Executor
  -xtr          Enable support for Executor
  -experimental Enable experimental features
  -exp          Enable experimental features
  -emptyassignments  Allow assignments like "var =".
  -extraletters "extra"  Allow all the characters in "extra"
                to function as letters.


Executing short code fragments:

  -e code       Immediately parse a string of Rexx code.
                This has to be the last argument.

All toggles except "debug" are active by default

The 'myname' program is part of the Rexx Parser package,
see https://rexx.epbcn.com/rexx-parser/. It is distributed under
the Common Public License 1.0 (https://www.oorexx.org/license.html).

Copyright (c) 2024-2026 Josep Maria Blasco <josep.maria.blasco@epbcn.com>.
Copyright (c) 2026 Rexx Language Association.

See myhelp for details.
::END