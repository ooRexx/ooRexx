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
/* ANSI.Highlighter.Driver.cls - LaTeX highlighter driver                     */
/* ======================================================                     */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20250101    0.1e First version                                             */
/* 20250105    0.1f Support style patches                                     */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250622    0.2d Change fg/bg char separator to ":"                        */
/* 20261031         A dotted underline ("D") is underlined                    */
/* 20261121         Moved from bin/HLDrivers/ to bin/parser/HLDrivers/ (RFE   */
/*                  #878: everything but parser.cls lives in parser/)         */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261122         width is the width of the whole visible line, its number  */
/*                  and " | " included (it was one column more without        */
/*                  numbers, and two less with them); a UTF-8 character       */
/*                  takes one column, not one per byte                        */
/*                                                                            */
/******************************************************************************/


If \.environment~hasIndex( ANSI.Highlighter ) Then Do
  .environment~ANSI.Highlighter = .ANSI.Highlighter
End

::Class ANSI.Highlighter SubClass Highlighter.Driver Public

::Method Init
  Expose Options. output
  Use Strict Arg Options., output
  self~init:super(Options., output)

::Method Prolog
  Expose Options. linenoHL linebarHL
  self~emit( "1B"x"[0m" )

  If Options.numberLines Then Do
    linenoHL  = self~hl2SGR("rx-lineno")
    linebarHL = self~hl2SGR("rx-linebar")
    lineno    = Right(options.startFrom, Options.!numberWidth)
    self~emit( ANSI(linenoHL,  lineno) )
    self~emit( ANSI(linebarHL, " | " ) )
  End

::Method startLine
  Expose Options. linenoHL linebarHL
  Use Strict Arg lineNo

  self~EnsureWidth

  self~say( "" )

  If Options.numberLines Then Do
    lineno = Right(lineNo, options.!numberWidth)
    self~emit( ANSI(linenoHL,  lineno) )
    self~emit( ANSI(linebarHL, " | " ) )
  End

::Method Highlight
  Expose Options.

  Use Strict Arg category, subCategory, variant, tags, string

  patch = Options.!Patch
  If patch == .Nil Then extra = ""
  Else Do
    patch. = patch~getPatch
    Tag2Attr. = patch.!Tag2Attr
    Select
      When variant     \== "" Then extra = Tag2Attr.category.subCategory.variant
      When subCategory \== "" Then extra = Tag2Attr.category.subCategory
      Otherwise                    extra = Tag2Attr.category
    End
  End

  self~emit( ANSI(self~hl2SGR(tags,extra), string) )

::Method hl2SGR
  Use Strict Arg tags, extra = ""

  Parse Value self~tags2highlight(tags, extra) -
    With bold italic underline color":"background

  command = ""
  If bold              Then command ||= "1;"
  If italic            Then command ||= "3;"
  If underline \== 0   Then command ||= "4;"
  If color      \== "" Then command ||= "38;2;"color2SGR(color)
  If background \== "" Then command ||= "48;2;"color2SGR(background)

  Return command

::Method Epilog
  self~EnsureWidth

::Method EnsureWidth
  Expose Options. output
  style = Options.style
  Parse Value self~CachedCSS(style, "rexx") With ":"background
  If background = "" Then Return
  background = color2SGR(background)
  -- width is the width of the whole visible line: the line number and
  -- its " | " included, when there are numbers
  len = Columns( output[ output~last ] )
  If len >= Options.width Then Return
  self~emit( ANSI( "48;2;"background, Copies(" ",Options.width-len)) )
Return

-- The visible columns of a line: its characters (not its bytes: a UTF-8
-- character takes one column), without the SGR sequences
Columns: Procedure
  Use Strict Arg string
  len = 0
  esc = "1B"x
  Do While string~contains(esc)
    Parse Var string before(esc)"m"string
    len += Chars8(before)
  End
  Return len + Chars8(string)

Chars8: Procedure
  Use Strict Arg s
  -- The continuation bytes of UTF-8 (80-BF) are not characters
  t    = s~translate( Copies("00"x, 64), XRange("80"x, "BF"x) )
  cont = CountStr("00"x, t) - CountStr("00"x, s)
  Return Length(s) - cont

----------

::Routine color2SGR -- hhhhhh --> r;g;b;
  Parse Arg aa +2 bb +2 cc +2
  Return aa~x2d";"bb~x2d";"cc~x2d";"

----------

::Routine ANSI
  ESC = "1B"x

  Parse Arg SGR, string
  If SGR~endsWith(";") Then SGR = Left(SGR,Length(SGR)-1)

  Return ESC"["SGR"m"Arg(2)ESC"[0m"
