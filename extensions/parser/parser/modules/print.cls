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
/* print.cls -- The "print" module                                            */
/* ===============================                                            */
/*                                                                            */
/* When loaded, the nodes of the tree become printable. A program that uses   */
/* it needs only                                                              */
/*                                                                            */
/*   ::Requires "parser.cls"                                                  */
/*   ::Requires "parser/modules/print.cls"                                    */
/*                                                                            */
/* and then, for a parser = .Rexx.Parser~new(name, source):                   */
/*                                                                            */
/*   node~string            -- A node (a clause, an expression, a term, a     */
/*                             template...) as one line of normalized source: */
/*                             one blank where the source has blanks or a     */
/*                             continuation, symbols in uppercase, strings as */
/*                             written, no comments. A clause has its labels  */
/*                             in front, and an optional indent argument.     */
/*   instruction~makeArray  -- An instruction as an array of lines: a block   */
/*                             instruction (DO, LOOP, IF, SELECT) with its    */
/*                             parts indented.                                */
/*   package~makeArray      -- The whole package: the prolog, and each        */
/*                             directive with its body (or, for a ::RESOURCE, */
/*                             its data, as it is, and its delimiter).        */
/*                                                                            */
/* The printed source parses to the same elements as the original.            */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241206    0.1  First public release                                      */
/* 20250318    0.2  Change syntax to class::method                            */
/* 20250328         Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20260928    0.7  Attribute.Directive::String: options.external is .Nil     */
/*                  when there is no EXTERNAL. Method.Directive::String used  */
/*                  an options. stem the directive does not have (34.1): it   */
/*                  uses its getters. EXTERNAL specifications are quoted.     */
/* 20261113         CATCH and FINALLY clauses of DO, LOOP and SELECT (RFE     */
/*                  #31): Catch.Clause::String, Finally.Clause::String and    */
/*                  CatchSectionsArray                                        */
/* 20261118         Raise.Instruction::String works again (it failed with     */
/*                  97.1), with the instruction's attributes, and prints      */
/*                  THROW too                                                 */
/* 20261125         Rewritten. One string method for every node, from its     */
/*                  elements (Rexx.Node::String), instead of one per class    */
/*                  from variables that many classes no longer had: PARSE,    */
/*                  ITERATE and LEAVE failed (34.1, 97.1), and ADDRESS, ARG,  */
/*                  PULL, CALL, DO, LOOP, TRACE, RAISE ... ARRAY, the indexes */
/*                  and ::CLASS, ::OPTIONS and ::REQUIRES printed wrong.      */
/*                  Rexx.Package::makeArray, new.                             */
/* 20261125         The cut of the tree API: whenSections, otherwiseInstruc-  */
/*                  tions, catchSections and finallySection; the strings end  */
/*                  at lastElement (a directive, at that of its clause), not  */
/*                  at "end"; Rexx.Clause::makeArray and Code.Body::makeArray */
/*                  come here from the parser                                 */
/* 20261126    1.0  Moved from the rexx-parser repository (bin/modules/print) */
/*                  to parser/modules/, next to the identity compiler: Common */
/*                  Public License 1.0, and copyright also of the Rexx        */
/*                  Language Association, as the rest of ooRexx (RFE #878).   */
/*                  It calls the loader as the other modules, and requires    */
/*                  BaseClassesAndRoutines.cls                                */
/*                                                                            */
/******************************************************************************/

  Call "Load.Parser.Module.rex" "", .True

::Requires "../BaseClassesAndRoutines.cls"

/******************************************************************************/
/* STRING Methods                                                             */
/******************************************************************************/

-- Any node: its elements, from the first one to its end (excluded). A
-- clause has its labels in front, and is indented.

::Method "Rexx.Node::String"
  Use Strict Arg indent = 0
  If \self~isA(.Rexx.Clause) Then Do
    last = self~lastElement
    If last == .Nil Then Return ""
    Return ElementsText( TreeFirstElement( self ), last~next )
  End
  -- A directive goes on to the end of its body: its string is its clause
  clause = self
  If self~isA(.Rexx.Directive) Then clause = self~directiveClause
  -- The clause, without its semicolon
  text = ElementsText( TreeFirstElement( clause ), clause~lastElement )
  -- A clause that ends with "," or "-" ("Use Strict Arg ,;") needs its
  -- semicolon: at the end of a line, they are a continuation.
  If text~endsWith(",") | text~endsWith("-") Then text ||= ";"
  Return Copies(" ", indent)LabelsText(self)text

--------------------------------------------------------------------------------

-- A ::RESOURCE directive: its clause goes on to the delimiter. Its string is
-- up to the end of its first line (see Rexx.Package::makeArray).

::Method "Resource.Directive::String"
  Use Strict Arg indent = 0
  first = TreeFirstElement( self )
  stop  = first
  Loop While stop \== .Nil, stop \< .EL.END_OF_CLAUSE, -
    stop \< .EL.RESOURCE_DATA
    stop = stop~next
  End
  Return Copies(" ", indent)ElementsText( first, stop )

--------------------------------------------------------------------------------

::Method "Rexx.Package::String"
  Return self~makeArray~makeString("L", "0a"x)

--------------------------------------------------------------------------------
-- Elements                                                                   --
--------------------------------------------------------------------------------

::Method "Element::String"
  Return ElementText( self )

/******************************************************************************/
/* MAKEARRAY Methods                                                          */
/******************************************************************************/

::Method "Rexx.Package::makeArray"
  array = .Array~new
  Call Append array, self~prolog~body~makeArray
  Do directive Over self~directives
    array~append( directive~string )
    If directive~isA(.Resource.Directive) Then Do
      -- The data, as it is, and the delimiter
      If directive~fromLine \== "" Then Do
        source = self~source
        Do line = directive~fromLine To directive~toLine
          array~append( source[line] )
        End
      End
      array~append( directive~delimiter )
      Iterate
    End
    body = directive~body
    If body \== .Nil Then Call Append array, body~makeArray(2)
  End
  Return array

--------------------------------------------------------------------------------

::Method "Rexx.Clause::makeArray"
  Use Strict Arg indent = 0
  Return .Array~of( self~string(indent) )

--------------------------------------------------------------------------------

::Method "Code.Body::makeArray"
  Use Strict Arg indent = 0
  array = .Array~new
  Do instruction Over self~instructions
    Call Append array, instruction~makeArray(indent)
  End
  Return array

--------------------------------------------------------------------------------

::Method "Implicit.Exit.Instruction::makeArray"
  Use Strict Arg indent = 0
  -- Nothing to print, unless a label precedes it
  If self~labels~isEmpty Then Return .Array~new
  Return .Array~of( Copies(" ", indent)Strip( LabelsText(self), "T" ) )

--------------------------------------------------------------------------------

::Method "Do.Instruction::makeArray"
  Use Strict Arg indent = 0
  Return BlockArray( self, self~doClause, indent )

--------------------------------------------------------------------------------

::Method "Loop.Instruction::makeArray"
  Use Strict Arg indent = 0
  Return BlockArray( self, self~loopClause, indent )

--------------------------------------------------------------------------------

::Method "If.Instruction::makeArray"
  Use Strict Arg indent = 0
  array = .Array~of( self~ifClause~string(indent) self~thenClause~string )
  Call Append array, self~thenInstruction~makeArray(indent + 2)
  If self~elseInstruction \== .Nil Then Do
    array~append( self~elseClause~string(indent) )
    Call Append array, self~elseInstruction~makeArray(indent + 2)
  End
  Return array

--------------------------------------------------------------------------------

::Method "Select.Instruction::makeArray"
  Use Strict Arg indent = 0
  array = .Array~of( self~selectClause~string(indent) )
  Do section Over self~whenSections
    array~append( section~whenClause~string(indent + 2) -
      section~thenClause~string )
    Call Append array, section~thenInstruction~makeArray(indent + 4)
  End
  If self~otherwiseClause \== .Nil Then Do
    array~append( self~otherwiseClause~string(indent + 2) )
    Do instruction Over self~otherwiseInstructions
      Call Append array, instruction~makeArray(indent + 4)
    End
  End
  Call CatchSectionsArray array, self, indent
  array~append( self~endClause~string(indent) )
  Return array

/******************************************************************************/
/* Routines                                                                   */
/******************************************************************************/

-- BlockArray -- A DO or LOOP: its first clause, its instructions two columns
-- further in, its CATCH and FINALLY sections, and its END.

::Routine BlockArray
  Use Strict Arg block, firstClause, indent
  array = .Array~of( firstClause~string(indent) )
  Do instruction Over block~instructions
    Call Append array, instruction~makeArray(indent + 2)
  End
  Call CatchSectionsArray array, block, indent
  array~append( block~endClause~string(indent) )
  Return array

--------------------------------------------------------------------------------
-- CatchSectionsArray -- The CATCH and FINALLY clauses of a block, at the     --
-- indent of its END, with their code two columns further in (RFE #31).       --
--------------------------------------------------------------------------------

::Routine CatchSectionsArray
  Use Strict Arg array, block, indent
  sections = block~catchSections~copy
  If block~finallySection \== .Nil Then sections~append( block~finallySection )
  Do section Over sections
    array~append( section[1]~string(indent) )
    Do instruction Over section~instructions
      Call Append array, instruction~makeArray(indent + 2)
    End
  End

--------------------------------------------------------------------------------
-- LabelsText -- The labels before a clause, each one followed by ": ". A     --
-- label can be a string ("Do/Loop ... Over":).                               --
--------------------------------------------------------------------------------

::Routine LabelsText
  Use Strict Arg clause
  text = ""
  Do labelClause Over clause~labels
    text ||= ElementText( labelClause~label )": "
  End
  Return text

--------------------------------------------------------------------------------

::Routine Append
  Use Strict Arg array, lines
  Do line Over lines
    array~append( line )
  End

--------------------------------------------------------------------------------
-- ElementsText -- The elements from "first" up to "stop" (excluded), as one  --
-- line of normalized source. Between two elements that are printed:         --
--                                                                            --
--   * Blanks (a run of them, or a continuation), and comments, become one    --
--     blank; the blank of a blank concatenation too, also when it is a       --
--     continuation ("a" -, newline, "b").                                    --
--   * When the parser marked the gap as an abuttal, nothing; but if there    --
--     was a comment, "/**/", which keeps the two elements apart ("1/**/05"   --
--     is "105", and "105" would be one number).                              --
--   * An inserted semicolon (the end of a line inside a source literal) is   --
--     ";"; the other inserted elements (the implicit EXIT) are nothing.      --
--   * Other ignorable elements (the second character of "||") are nothing.  --
--------------------------------------------------------------------------------

::Routine ElementsText
  Use Strict Arg first, stop

  text    = ""
  blank   = 0
  comment = 0
  abuttal = 0

  element = first
  Loop While element \== .Nil, element \== stop
    Select
      When element~isA(.Inserted.Semicolon) Then Do
        text ||= ";"
        Call Reset
      End
      When element < .EL.OP.ABUTTAL         Then abuttal = 1
      When element~isA(.Inserted.Element)   Then Nop
      When element~isA(.WhiteSpace.Element) Then blank = 1
      When element < .EL.OP.BLANK           Then blank = 1  -- A "-" or ","
      When element~ignorable Then
        If element < .ALL.COMMENTS Then comment = 1
        Else If element < .ALL.WHITESPACE_LIKE | element~value == "," Then
          blank = 1                                  -- A continuation
      Otherwise
        If text \== "" Then Select
          When abuttal Then If comment Then text ||= "/**/"
          When blank | comment Then text ||= " "
          Otherwise Nop
        End
        text ||= ElementText( element )
        Call Reset
    End
    element = element~next
  End

  Return text

Reset:
  blank   = 0
  comment = 0
  abuttal = 0
  Return

--------------------------------------------------------------------------------
-- ElementText -- One element: a string as written, anything else by its     --
-- value (a symbol in uppercase).                                             --
--------------------------------------------------------------------------------

::Routine ElementText
  Use Strict Arg element
  If element~isA(.StringOrSymbol.Element), element~isAString Then
    Return element~source
  Return element~value
