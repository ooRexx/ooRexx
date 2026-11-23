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
/* identtest.rex - Test Rexx programs in a file tree                          */
/* =================================================                          */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20251211    0.3a First public release                                      */
/* 20251215    0.4a Add toggles for Executor support, .TestGroup files        */
/*                  Change name to identtest (thanks, JLF!)                   */
/* 20251221         Add --itrace option, improve error messages               */
/* 20251226         Send error messages to .error, not .output                */
/* 20251227         Use .SysCArgs when available                              */
/* 20260102         Standardize help options to -h and --help                 */
/* 20260314    0.5  Use InitCLI() from CLISupport.cls                         */
/* 20261123         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261123         Moved from bin/ to the samples (RFE #878)                 */
/* 20261123         SysFileTree with the I option when not on Windows: on     */
/*                  Unix the search is case-sensitive, and the masks are in   */
/*                  uppercase (*.REX), so it found no files                   */
/*                                                                            */
/******************************************************************************/

  -- identtest
  --
  -- Usage: cd to a directory run it.
  --
  -- By default, the script will recursively examine the whole directory
  -- tree and run the elident and trident tests against all .rex and .cls
  -- files. The script will stop when all files have been processed, or when
  -- an error is encountered.
  --
  -- Run "indentest --help" to show a list of the available options.
  --
  -- See also the list of exceptions below
  --


  CLIhelper    = InitCLI()
  myName       = CLIhelper~name
  myHelp       = CLIhelper~help
  args         = CLIhelper~args

  elements           = 1
  tree               = 1
  itrace             = 0
  executor           = 0
  cls                = 1
  cms                = 0
  SysFileTreeOptions = "FSO"
  -- On Unix, SysFileTree is case-sensitive, and the masks are in uppercase
  If .RexxInfo~platform \== "WINDOWS" Then SysFileTreeOptions ||= "I"
  start              = 0

  If args~items == 0 Then Signal Help

ProcessOptions:
  Loop While args~items > 0

    option = args[1]
    args~delete(1)

    Select Case Lower(option)
      When "start"                         Then start     = 1
      When "--help",               "-h"    Then Signal Help
      When "--itrace",            "-it"    Then itrace    = 1
      When "--executor",         "-xtr"    Then executor  = 1
      When "--nocls",             "-nc"    Then cls       = 0
      When "--cms",              "-cms",   -
           "--rexxvm",           "-rexxvm" Then cms       = 1
      When "--noelements",        "-ne"    Then elements  = 0
      When "--notree",            "-nt"    Then tree      = 0
      Otherwise Call Error "Invalid option '"option"'."
    End

  End

  If \start Then Signal Help

  option = ""
  If Executor Then option = option "-xtr"
  If cms      Then option = option "-cms"
  option = Strip(option)

  cd = Directory()~changeStr("\","/")

  extensions = "rex testgroup jrexx oodTestGroup rxj rxo testUnit rxu"

  If cls Then extensions = "cls" extensions

  exception. = 0

  -- Exceptions for .rex
  ----------------------

  -- The following exceptions are tailored to the Executor tree.

  -- expression=
  exceptions = "executor-demo-array.rex executor-demo-extensions.rex"            -
    "executor-demo-text-compatibility-auto-conv.rex"                             -
    "executor-demo-text-compatibility.rex executor-demo-text-internal_checks.rex"-
    "executor-demo-text-unicode.rex executor-demo-text.rex"                      -
    "ooRexxShell-demo-interpreters.rex ooRexxShell-demo-queries.rex"             -
    "diary_examples.rex include_concatenation_infos.rex"                         -
    "include_concatenation_infos.rex include_conversion.rex"                     -
    "include_conversion_infos.rex"

  -- Real error
  exceptions ||= " classic_rexx.rex ooRexxTry.rex"

  -- malformed expression

  exceptions ||= " include_conversion_to_unicode.rex include_conversion_to_unicode16.rex include_conversion_to_unicode32.rex" -
    "include_conversion_to_unicode8.rex include_conversion_to_utf16be.rex include_conversion_to_utf16le.rex"                 -
    "include_conversion_to_utf32be.rex include_conversion_to_utf32le.rex include_conversion_to_utf8.rex"                     -
    "include_conversion_to_wtf16be.rex include_conversion_to_wtf16le.rex include_conversion_to_wtf8.rex"                     -
    "main_concatenation.rex main_conversion.rex"

  -- test/trunk (SVN)
  exceptions ||=  " tmpTest_ExternalCode_Compiled.rex tmpTest_ExternalCode_CompiledAndEncoded.rex"

  -- samples/ (ooRexx installations)
  exceptions ||=  " MSInternetExplorer_search.rex"

  -- Experimental features of the Rexx Parser
  exceptions ||=  " extends.rex tables.rex"

  extension = "REX"

  Loop filename Over exceptions~makeArray(" ")
    exception.extension.filename = 1
  End

  -- Exceptions for .testgroup
  ----------------------

  exceptions = "PARSE.testGroup"

  extension = "TESTGROUP"

  Loop filename Over exceptions~makeArray(" ")
    exception.extension.filename = 1
  End

  -- Exceptions for .rxj
  ----------------------

  exceptions = "swing_buttons1.rxj swing_buttons2.rxj" -- Real error, invalid template

  extension = "RXJ"

  Loop filename Over exceptions~makeArray(" ")
    exception.extension.filename = 1
  End

  Loop extension Over extensions~makeArray(" ")
    extension = Upper(extension)
    Call SysFileTree "*."extension, "TREE", SysFileTreeOptions
    If extension == "RXU" Then options? = "-u" option
    Else options? = option
    options? = Strip(options?)
    If itrace Then options? = "-it" options?
    options? = Strip(options?)
    Loop i = 1 To tree.0
      -- If tree.i contains whitespace, enclose it between quotes
      If Space(tree.i,0) \== tree.i Then tree.i = '"'tree.i'"'
      name = FileSpec("N",tree.i)
      If exception.extension.name Then Iterate

      Say "Checking" tree.i"..."
      Say "--> Elements"
      Call ElIdent options? tree.i
      If result \== 0 Then Exit 1
      Say "--> Tree"
      Call TrIdent options? tree.i
      If result \== 0 Then Exit 1
    End
  End

  Exit 0

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

::Resource Help End "::End"
myname -- Run identity tests against a collection of files

Usage:
  [rexx] myname [OPTIONS...] ["start"]

If the only option is -h or --help, or if no arguments are present,
then display this help and exit.

Options:
  -h,   --help           Display this help
  -xtr, --executor       Support Executor syntax
  -cms, --cms            Support CMS (Classic Rexx) syntax (also -rexxvm, --rexxvm)
  -it, --itrace          Print internal traceback on error
  -nc, --nocls           Don't analyze .cls files
  -ne, --noelements      Don't run the elident test
  -nr, --norex, --norexx Don't analyze .rex files
  -nt, --notree          Don't run the trident test

If you specify "start", the myname tests are started; otherwise, this
help is displayed.

The 'myname' program is part of the Rexx Parser package,
see https://rexx.epbcn.com/rexx-parser/. It is distributed under
the Common Public License 1.0 (https://www.oorexx.org/license.html).

Copyright (c) 2024-2026 Josep Maria Blasco <josep.maria.blasco@epbcn.com>.
Copyright (c) 2026 Rexx Language Association.

See myhelp for details.
::End

::Requires "parser/CLISupport.cls"
