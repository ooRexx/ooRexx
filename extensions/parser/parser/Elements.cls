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
/* Elements.cls                                                               */
/* ============                                                               */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20251206    0.3a Split Scanner.cls in two, move all classes here           */
/* 20260620    0.6  CMS: Implement multi-line strings                         */
/* 20260930         Error.Element: the package, and atTheClause (issue #8)    */
/* 20261025         Element~continuation: a continuation that is also the     */
/*                  blank operator (Rony, 25-Oct)                             */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261122         StringOrSymbol.Element~term removed: it was set only for  */
/*                  compound variables, and nobody read it                    */
/* 20261124         element~clause: the clause of an element (ownerClause,    */
/*                  in its EL.END_OF_CLAUSE)                                  */
/*                                                                            */
/******************************************************************************/

 .environment ~                     Element =                     .Element
 .environment ~               Error.Element =               .Error.Element
 .environment ~            Inserted.Element =            .Inserted.Element
 .environment ~          Inserted.Semicolon =          .Inserted.Semicolon
 .environment ~                Line.Comment =                .Line.Comment
 .environment ~ Operator.Character.Sequence = .Operator.Character.Sequence
 .environment ~               Resource.Data =               .Resource.Data
 .environment ~            Standard.Comment =            .Standard.Comment
 .environment ~  Special.Character.Sequence =  .Special.Character.Sequence
 .environment ~              Symbol.Element =              .Symbol.Element
 .environment ~      StringOrSymbol.Element =      .StringOrSymbol.Element
 .environment ~      Tail.Separator.Element =      .Tail.Separator.Element
 .environment ~          WhiteSpace.Element =          .WhiteSpace.Element

::Requires "Globals.cls"
::Requires "BaseClassesAndRoutines.cls"

/******************************************************************************/
/******************************************************************************/
/* The ELEMENT class                                                          */
/******************************************************************************/
/******************************************************************************/

::Class Element Public -- SubClass Directory

-- Elements are stored in a doubly-linked list

::Attribute next
::Attribute prev

-- remove -- Remove an element from the chain by unlinking it
-- The element must not be the head or the tail of the chain (not checked)

::Method remove
  self~next~prev = self~prev
  self~prev~next = self~next

-- Every element has a starting ("from") and an ending position ("to").

::Attribute from
::Attribute to

-- Every element has a value and a source, which may be identical or not.
-- The source is generally the element as it appears in the source file
-- The value is interpreted. For example, symbols are uppercased,
-- strings have double quotes removed, etc.

::Attribute source
::Attribute value

-- All elements have a category, and, additionally, taken constants have a
-- subcategory. Categories and subcategories are defined in Globals.cls.

::Attribute category
::Attribute subCategory

-- Parentheses, brackets and braces are paired
-- We also use this attribute to store the end of a ::RESOURCE in the
-- directive marker "::".

::Attribute closing

-- Shebangs, comments, and some form of whitespace can be ignored (but may
-- act as separators)

::Attribute ignorable

-- When a variable starts a term that is being assigned a value,
-- in assignment instructions, but also in some other contexts, like
-- PARSE or USE ARG instructions, we mark is as "assigned".

::Attribute assigned

-- An approximate value. It is used to determine if an opening brace is
-- (incorrectly) paired with a closing brace in another clause.

::Attribute clauseNumber

-- A continuation character ("-" or ",") at the end of a line can also be the
-- blank concatenation operator ("say 1 -" / "2" is "say 1 2"): the parser
-- makes it an EL.OP.BLANK, and this says that, in the source, it is the
-- continuation (Rony, 25-Oct-2026: the highlighter showed it as an operator)

::Attribute continuation

--

::Method init
  Expose next prev from to category subcategory source value -
    ignorable assigned assignment closing continuation ownerClause

  self~init:super
  ownerClause = .Nil
  next        = .Nil
  prev        = .Nil
  from        = ""
  to          = ""
  category    = ""
  subcategory = ""
  source      = ""
  value       = ""
  assigned    = 0
  ignorable   = 0
  closing     = .Nil
  continuation = 0

-- clause -- The clause this element belongs to. A clause ends with its
-- EL.END_OF_CLAUSE, and what comes after it belongs to the next clause.
-- Only the EL.END_OF_CLAUSE elements store their clause (ownerClause, set by
-- Rexx.Clause~init with setOwnerClause); the others look for the next one.

::Attribute ownerClause Get

::Method setOwnerClause
  Expose ownerClause
  Use Strict Arg ownerClause

::Method clause
  Expose ownerClause
  If ownerClause \== .Nil Then Return ownerClause
  element = self~next
  Loop While element \== .Nil, element~ownerClause == .Nil
    element = element~next
  End
  If element == .Nil Then Return .Nil
  Return element~ownerClause

::Method isAssigned
  Expose assigned
  Return assigned
::Method setAssigned
  Expose assigned
  assigned = 1
::Method isInserted
  Return self~from == self~to

::Method "<"
  Use Strict Arg category
  If category~isA( .String ) Then Return   category~contains( self~category )
  Return self ~  "<" : super( category )

::Method "\<"
  Use Strict Arg category
  If category~isA( .String ) Then Return \ category~contains( self~category )
  Return self ~ "\<" : super( category )

::Method "<<"
  Use Strict Arg subcategory
  If subcategory~isA( .String ) Then Do
    If self~category == .EL.TAKEN_CONSTANT,  -
       subcategory~contains( self~subcategory)
      Then Return .True
      Else Return .False
  End
  Return self ~  "<<" : super( subcategory )

::Method "\<<"
  Use Strict Arg subcategory
  If subcategory~isA( .String ) Then Do
    If self~category == .EL.TAKEN_CONSTANT,  -
       subcategory~contains( self~subcategory)
      Then Return .False
      Else Return .True
  End
  Return self ~  "\<<" : super( subcategory )

--------------------------------------------------------------------------------

::Class Error.Element SubClass Element
::Attribute code
::Attribute additional
::Attribute package
-- 1 when the error is reported at the beginning of its clause (the
-- unmatched quotes and comments); 0 when it is reported where it is
::Attribute atTheClause
::Method init
  self~init:super
  Use Strict Arg package, code, element, additional
  self~package    =  package
  self~atTheClause = 0
  If element~isA(.Element) Then
    Parse Value element~from element~to With line col endLine endCol
  Else Parse Var element line col endLine endCol
  self~from       =  line col
  self~to         =  endLine endCol
  self~category   = .EL.SYNTAX_ERROR
  self~code       =  code
  self~additional =  additional

--------------------------------------------------------------------------------

::Class Resource.Data Public SubClass Element

--------------------------------------------------------------------------------

::Class Standard.Comment Public subclass Element
::Attribute parts
::Method value -- A placeholder
  If self~category == .EL.STANDARD_COMMENT Then Return "/* A COMMENT */"
  Else Return "/* A DOC-COMMENT */"
::Method init
  Expose         package  startLine  startCol  endLine  endCol parts
  self~init:super
  Use Strict Arg package, startLine, startCol, endLine, endCol
  self~parts      = .Array~new
  self~category   = .EL.STANDARD_COMMENT
  self~ignorable  = 1 -- Classic comments are always ignorable
  self~from       = startLine startCol
  self~to         = endLine   endCol
::Method isMultiLine
  Expose startLine endLine
  Return startLine \== endLine
::Method source
  Expose         package  startLine  startCol  endLine  endCol
  Do lineNo = startLine To endLine
    line = package~source[lineNo]
    Select case lineNo
      When startLine Then Do
        If lineNo == endLine Then ret = .Array~of( line[startCol, endCol-startCol] )
        Else ret = .Array~of( SubStr( line, startCol ) )
      End
      When endLine Then ret~append( line[1, endCol-1] )
      Otherwise ret~append( line )
    End
  End
  Return ret

--------------------------------------------------------------------------------

::Class Line.Comment Public subclass Element
::Attribute parts
::Method init
  Expose package line start end
  self~init:super
  Use Strict Arg package, line, start, endLine, end
  self~parts      = .Array~new
  self~category   = .EL.LINE_COMMENT
  self~ignorable  = 1 -- Line comments are always ignorable
  self~from       = line    start
  self~to         = endLine end
::Method source
  Expose package line start endLine end
  If self~category == .EL.LINE_COMMENT Then
    Return package~source[line][start, end-start]
  Parse Value self~from With startLine  startCol
  Parse Value self~to   With endLine    endCol
  Do lineNo = startLine To endLine
    line = package~source[lineNo]
    Select case lineNo
      When startLine Then Do
        If lineNo == endLine Then
          ret = .Array~of( line[startCol, endCol-startCol] )
        Else ret = .Array~of( SubStr( line, startCol ) )
      End
      When endLine Then ret~append( line[1, endCol-1] )
      Otherwise ret~append( line )
    End
  End
  Return ret
::Method value; Return self~source

--------------------------------------------------------------------------------

::Class Operator.Character.Sequence Public subclass Element

-- "source" is the operator char as it appears in the source program, and,
-- as such, it cannot be changed.
::Attribute source Get

-- "value" is the current value of this character sequence. Initially, it
-- is the same value as "source", but it may change, for example when
-- encountering certain sequences that denote compound operators.
::Attribute value

::Method init
  Expose                        line  start  end  source  value
  self~init:super
  Use Strict Arg self~category, line, start, end, source
  value = source

::Method from
  Expose line start
  Return line start

::Method to
  Expose line end
  Return line end

--------------------------------------------------------------------------------

::Class Special.Character.Sequence Public subclass Element

::Attribute from
::Attribute source

-- "value" is the current value of this character sequence. Initially, it
-- is the same value as "source", but it may change, for example when
-- encountering certain sequences, like ": :"
::Attribute value

::Method init
  Expose                        line  start  end  source  value from
  self~init:super
  Use Strict Arg self~category, line, start, end, source
  value = source
  from = line start

::Method to
  Expose line end
  Return line end

--------------------------------------------------------------------------------

::Class Tail.Separator.Element Public subclass Element
::Constant value  "."
::Constant source "."
::Method init
  Expose                        line  col source
  self~init:super
  Use Strict Arg self~category, line, col
::Method from
  Expose line col
  Return line col
::Method to
  Expose line col
  Return line (col+1)

--------------------------------------------------------------------------------

::Class StringOrSymbol.Element Public subclass Element
::Attribute invocationType
::Attribute traceOnly                   -- For labels
::Attribute labelFor                    -- For labels
::Attribute parts                       -- of a compound variable
::Method value
  source = self~source
  If self~isAString Then Do -- Strings
    length = Length( source )
    c = Upper( Right(source,1) )
    Select Case c
      When "X" Then Return     X2C(SubStr(source,2,length-3))
      When "B" Then Return X2C(B2X(SubStr(source,2,length-3)))
      When "Y", "P", "G", "T" Then
                    Return         SubStr(source,2,length-3)~changeStr(c||c,c)
      Otherwise     Return         SubStr(source,2,length-2)~changeStr(c||c,c)
    End
  End
  Else Do -- Symbols
    Return Upper( source )
  End
::Method init
  Expose                        source  startLine  startCol  endLine  endCol
  self~init:super
  Use Strict Arg self~category, source, startLine, startCol, endLine, endCol
  self~traceOnly = 0
  self~parts = .Array~new
::Method isMultiLine
  Expose startLine endLine
  Return startLine \== endLine
-- "source" returns the full source text of the element, with the quotes
-- included. For a multi-line string the contents of each line are
-- concatenated with no line separator (record-oriented semantics).
::Method source
  Expose source startLine startCol endLine endCol
  If startLine == endLine Then
    Return source[startLine][ startCol, endCol-startCol ]
  ret = ""
  Do lineNo = startLine To endLine
    line = source[lineNo]
    Select Case lineNo
      When startLine Then ret =       SubStr( line, startCol )
      When endLine   Then ret = ret || line[1, endCol-1]
      Otherwise           ret = ret || line
    End
  End
  Return ret
-- "sourceLines" returns an array with the source text of the element on
-- each of the lines it spans (quotes and suffix included, in their real
-- positions). For a single-line element the array has just one item.
-- This is the partition the highlighter uses to render a multi-line
-- string keeping the original source layout.
::Method sourceLines
  Expose source startLine startCol endLine endCol
  If startLine == endLine Then
    Return .Array~of( source[startLine][ startCol, endCol-startCol ] )
  ret = .Array~new
  Do lineNo = startLine To endLine
    line = source[lineNo]
    Select Case lineNo
      When startLine Then ret~append( SubStr( line, startCol ) )
      When endLine   Then ret~append( line[1, endCol-1] )
      Otherwise           ret~append( line )
    End
  End
  Return ret
::Method isAString
  Expose source startLine startCol
  Return Pos( source[startLine][startCol], "'""") > 0
::Method from
  Expose startLine startCol
  Return startLine startCol
::Method to
  Expose endLine endCol
  Return endLine endCol

--------------------------------------------------------------------------------

::Class UString.Element Public SubClass StringOrSymbol.Element
::Method value
  Expose value
  Return value
::Method init
  Expose value
  Use Strict Arg category, source, startLine, startCol, endLine, endCol, value
  self~init:super(category, source, startLine, startCol, endLine, endCol)

--------------------------------------------------------------------------------

::Class WhiteSpace.Element Public subclass Element
::Method source
  Expose sourceLine start end
  Return sourceLine[ start, end-start ]
::Method value
  Expose start end sourceLine
  Return sourceLine[start, end-start]
::Method init
  Expose         line  start  end  sourceLine
  self~init:super
  Use Strict Arg line, start, end, sourceLine
  self~category = .EL.WHITESPACE
::Method from
  Expose line start
  Return line start
::Method to
  Expose line end
  Return line end

-- splitAfterFirst -- keep the first character, and put the rest in a new
-- whitespace element, linked right after this one (.Nil when there is no
-- rest). Used for the blank concatenation operator: "a    b" has ONE blank
-- operator, and the other three blanks are whitespace.
::Method splitAfterFirst
  Expose line start end sourceLine
  If end - start <= 1 Then Return .Nil
  rest = .WhiteSpace.Element~new( line, start + 1, end, sourceLine )
  end  = start + 1
  rest~ignorable = 1
  rest~prev = self
  rest~next = self~next
  If self~next \== .Nil Then self~next~prev = rest
  self~next = rest
  Return rest

--------------------------------------------------------------------------------

::Class Shebang Public subclass Element
::Method category; Return .EL.SHEBANG
::Method init
  Expose package line start end
  self~init:super
  Use Strict Arg package, line, start, end
  self~ignorable = 1 -- Shebangs are always ignorable
::Method source
  Expose package line start end
  Return package~source[line][start, end-start]
::Method value; Return self~source
::Method from
  Expose line start
  Return line start
::Method to
  Expose line end
  Return line end

--------------------------------------------------------------------------------

::Class Inserted.Element Public subclass Element
::Method after Class
  Use Strict Arg element
  Parse Value element ~ from With line col
  new = self~new(line, col)
  next = element~next
  element~next = new
  new  ~prev = element
  If next \== .Nil Then Do
    next~prev = new
    new ~next = next
  End
  Return new
::Method init
  Expose                        line  col
  self~init:super
  Use Strict Arg self~category, line, col
  self~from = line col
  self~to   = line col

--------------------------------------------------------------------------------

::Class Inserted.Semicolon Public SubClass Inserted.Element
::Constant value ";"
::Method init
  Use Strict Arg line, col
  self~init:super(.EL.END_OF_CLAUSE, line, col)

--------------------------------------------------------------------------------

::Class Inserted.Implicit.Exit Public SubClass Inserted.Element
::Constant value ""
::Method init
  Use Strict Arg line, col
  self~init:super(.EL.IMPLICIT_EXIT, line, col)
