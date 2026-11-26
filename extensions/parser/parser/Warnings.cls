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
/* Warnings.cls -- Optional warnings about a parsed package                   */
/* ========================================================                   */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20260930    0.7  First version (GitHub issue #39)                          */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261125         The cut of the tree API: the names, types and attributes  */
/*                  decided in the review of the Element and Tree APIs; the   */
/*                  attributes that only the parser writes are read-only      */
/*                  (set... methods)                                          */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Warnings( package ) -- Things that are legal, but most probably not what   */
/*                        the programmer meant                                */
/*                                                                            */
/*   A warning is not an error: ooRexx runs the program. The warnings are     */
/*   computed after the parse, from the tree and the element chain of a       */
/*   package that has no errors; the parser itself does not change.           */
/*                                                                            */
/*   Returns an array of directories, sorted by position, with the same keys  */
/*   as the error directory of the parser where they apply:                   */
/*                                                                            */
/*     ID        The kind of warning (see below).                             */
/*     POSITION  The line.                                                    */
/*     COLUMN    The column.                                                  */
/*     CLAUSE    The clause, as ooRexx would show it after "*-*".             */
/*     MESSAGE   What is wrong, in one sentence.                              */
/*                                                                            */
/*   Kinds of warnings:                                                       */
/*                                                                            */
/*     INHERIT.OPTION  "::CLASS c INHERIT d PUBLIC": after INHERIT, every     */
/*                     word is the name of a class to inherit, also PUBLIC,   */
/*                     PRIVATE, ABSTRACT, METACLASS, MIXINCLASS, SUBCLASS     */
/*                     and INHERIT (ooRexx looks for a class PUBLIC). Not     */
/*                     when the name is a string, is qualified with a         */
/*                     namespace, or is a class of the package.               */
/*                                                                            */
/*     LABEL.NAMESPACE "rexx:string~new": at the start of a clause, "rexx:"   */
/*                     is always a label (with or without blanks), so this    */
/*                     is the label "rexx" and "string~new", not the          */
/*                     namespace-qualified name "rexx:string" (ooRexx sends   */
/*                     NEW to the string "STRING"); "(rexx:string)~new" is    */
/*                     the qualified name. Only when the instruction is on    */
/*                     the same line as the label and starts with a symbol,   */
/*                     and is not a keyword instruction, an assignment or     */
/*                     another label.                                         */
/*                                                                            */
/******************************************************************************/

::Requires "BaseClassesAndRoutines.cls"

::Routine Warnings Public
  Use Strict Arg package

  warnings = .Array~new

  Call InheritOptions package, warnings
  Call LabelsBeforeASymbol package, warnings

  -- Sort by position
  keys = .Array~new
  Do i = 1 To warnings~items
    w = warnings[i]
    keys~append( Right(w~position,9,0)Right(w~column,9,0)Right(i,9,0) )
  End
  sorted = .Array~new
  table  = .StringTable~new
  Do i = 1 To warnings~items
    table[ keys[i] ] = warnings[i]
  End
  Do key Over keys~sort
    sorted~append( table[key] )
  End
  Return sorted

--------------------------------------------------------------------------------
-- A warning at an element                                                    --
--------------------------------------------------------------------------------

::Routine NewWarning
  Use Strict Arg warnings, id, element, message
  Parse Value element~from With line col
  warning = .Directory~new
  warning~id       = id
  warning~position = line
  warning~column   = col
  clause = ClauseInError( element )
  If clause == .Nil Then clause = ""
  warning~clause   = clause
  warning~message  = message
  warnings~append( warning )

--------------------------------------------------------------------------------
-- INHERIT.OPTION                                                             --
--------------------------------------------------------------------------------

::Routine InheritOptions
  Use Strict Arg package, warnings
  options = "PUBLIC PRIVATE ABSTRACT METACLASS MIXINCLASS SUBCLASS INHERIT"
  classes = package~classes
  Do directive Over package~directives
    If \directive~isA( .Class.Directive ) Then Iterate
    Do reference Over directive~inherits
      namespace = reference~namespace
      name      = reference~name
      If namespace \== .Nil               Then Iterate
      If IsAString( name )                Then Iterate
      value = name~value
      If WordPos( value, options ) == 0   Then Iterate
      If classes~hasIndex( value )        Then Iterate
      Call NewWarning warnings, "INHERIT.OPTION", name,                    -
        """"name~source""" after INHERIT is the name of a class to"        -
        "inherit, not an option of ::CLASS; the options go before INHERIT."
    End
  End

--------------------------------------------------------------------------------
-- LABEL.NAMESPACE                                                            --
--------------------------------------------------------------------------------

::Routine LabelsBeforeASymbol
  Use Strict Arg package, warnings

  -- The head of the element chain
  element = package~firstElement
  Loop While element~prev \== .Nil
    element = element~prev
  End

  Loop While element \== .Nil
    If element << .LABEL.NAME Then Call CheckLabel
    element = element~next
  End
  Return

CheckLabel:
  -- The blanks around the ":" are ignorable
  colon = TheElementAfter( element )
  If colon \< .EL.COLON Then Return   -- A SIGNAL target, for example
  -- A name that could be a namespace: not a string, nor ".x", nor "1"
  If \StartsLikeAName( element )     Then Return
  -- The first element of the instruction, after the end of clause that
  -- the parser inserts after the ":"
  next = TheElementAfter( TheElementAfter( colon ) )
  -- On another line, a label is what was meant
  If next~from~word(1) \== colon~to~word(1) Then Return
  If next < .EL.KEYWORD              Then Return  -- "a: say 1"
  If next << .LABEL.NAME             Then Return  -- "a: b:"
  If next~isAssigned                 Then Return  -- "a: b = 1"
  If next \< .ALL.VAR_SYMBOLS, next \< .EL.TAKEN_CONSTANT Then Return
  If \StartsLikeAName( next )        Then Return  -- ".x~y", "1"
  label  = element~source
  symbol = next~source
  Call NewWarning warnings, "LABEL.NAMESPACE", element,                    -
    "At the start of a clause, """label":"" is a label: this is the label" -
    """"label""" and an instruction that starts with """symbol""", not"   -
    "the namespace-qualified name """label":"symbol"""; to start an"      -
    "instruction with that name, write it in parentheses"                  -
    "(""("label":"symbol")"")."
  Return

--------------------------------------------------------------------------------
-- IsAString: a class name or a label can be a symbol or a string; after the  --
-- parse, both are taken constants, and only the source tells them apart.     --
--------------------------------------------------------------------------------

::Routine IsAString
  Use Strict Arg element
  Return Pos( Left( element~source, 1 ), "'""" ) > 0

--------------------------------------------------------------------------------
-- StartsLikeAName: a symbol that could be a namespace or a class name, not   --
-- a string, an environment symbol or a number.                               --
--------------------------------------------------------------------------------

::Routine StartsLikeAName
  Use Strict Arg element
  first = Left( element~source, 1 )
  Return Datatype( first, "M" ) | Pos( first, "_!?" ) > 0
