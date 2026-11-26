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
/* Iterations.cls -- DO and LOOP instrs (part of the identity compiler)       */
/* ====================================================================       */
/*                                                                            */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20251120    0.3a First version                                             */
/* 20261123         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261123         Moved from bin/modules/identity/ to                       */
/*                  bin/parser/modules/identity/ (RFE #878: everything but    */
/*                  parser.cls lives in parser/); its requires and the call   */
/*                  to the loader, relative to this file                      */
/* 20261125         The cut of the tree API: the names, types and attributes  */
/*                  decided in the review of the Element and Tree APIs; the   */
/*                  attributes that only the parser writes are read-only      */
/*                  (set... methods)                                          */
/*                                                                            */
/******************************************************************************/

  Call "../Load.Parser.Module.rex"

  pkgLocal = .context~package~local

  -- Set to 1 to activate debug
  pkgLocal~DEBUG = 0

::Requires "compile.cls"

--------------------------------------------------------------------------------
-- Skip over DO/LOOP, and process COUNTER and LABEL phrases                   --
--------------------------------------------------------------------------------

::Routine InitBlock
  Use Strict Arg self, element, stream, context

  -- Skip until DO/LOOP
  element = PrepareClause(element, stream, context)

  -- Clone "DO"/"LOOP"
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Process LABEL and COUNTER phrases, if present
  Loop While element < .EL.SUBKEYWORD, -
    WordPos(element~value,"LABEL COUNTER") > 0
    If element~value == "LABEL" Then
      element = LabelPhrase( element, stream, context )
    Else
      element = CounterPhrase( element, stream, context )
  End

  Return element

--------------------------------------------------------------------------------
-- BY PHRASE                                                                  --
--------------------------------------------------------------------------------

::Routine ByPhrase
  Use Strict Arg self, element, stream, context

  -- Clone the "BY" keyword
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile byExpression
  element = self~byExpression~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element

--------------------------------------------------------------------------------
-- COUNTER PHRASE                                                             --
--------------------------------------------------------------------------------

::Routine CounterPhrase
  Use Strict Arg element, stream, context

  -- element is COUNTER. Clone it
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Now element is the counter
  element = element~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element

--------------------------------------------------------------------------------
-- FOR PHRASE                                                                 --
--------------------------------------------------------------------------------

::Routine ForPhrase
  Use Strict Arg self, element, stream, context

  -- No FOR phrase? Nothing to do
  If self~forExpression == .Nil Then Return element

  -- Clone the "FOR" keyword
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile forExpression
  element = self~forExpression~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element

--------------------------------------------------------------------------------
-- LABEL PHRASE                                                               --
--------------------------------------------------------------------------------

::Routine LabelPhrase
  Use Strict Arg element, stream, context

  -- element is LABEL. Clone it
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Now element is the label
  element = element~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element

--------------------------------------------------------------------------------
-- TO PHRASE                                                                  --
--------------------------------------------------------------------------------

::Routine ToPhrase

  Use Strict Arg self, element, stream, context

  -- Clone the "TO" keyword
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile toExpression
  element = self~toExpression~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element

--------------------------------------------------------------------------------
-- UNTIL PHRASE                                                               --
--------------------------------------------------------------------------------

::Routine UntilPhrase
  Use Strict Arg self, element, stream, context

  -- No UNTIL phrase? Nothing to do
  If self~untilExpressions == .Nil Then Return element

  -- Clone the "UNTIL" keyword
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile untilExpressions
  element = self~untilExpressions~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element

--------------------------------------------------------------------------------
-- WHILE PHRASE                                                               --
--------------------------------------------------------------------------------

::Routine WhilePhrase
  Use Strict Arg self, element, stream, context

  -- No WHILE phrase? Nothing to do
  If self~whileExpressions == .Nil Then Return element

  -- Clone the "WHILE" keyword
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile whileExpressions
  element = self~whileExpressions~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element

--------------------------------------------------------------------------------
-- Controlled DO/LOOP -- Common routine                                       --
--------------------------------------------------------------------------------

::Routine DoLoop.Controlled

  Use Strict Arg self, element, stream, context

  element = InitBlock( self, element, stream, context )

  -- Compile control variable
  element = element~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Equal sign
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile initialExpression
  element = self~initialExpression~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Loop phrase Over self~order~makeArray(" ")
    Select Case phrase
      When "TO"  Then element =  ToPhrase( self, element, stream, context )
      When "BY"  Then element =  ByPhrase( self, element, stream, context )
      When "FOR" Then element = ForPhrase( self, element, stream, context )
    End
  End

  -- Compile WHILE or UNTIL phrases, if present
  element = WhilePhrase( self, element, stream, context )
  element = UntilPhrase( self, element, stream, context )

  Return element

/******************************************************************************/
/* CONTROLLED DO CLAUSE                                                       */
/******************************************************************************/

::Method "Do.Controlled.Clause::compile"
  Use Strict Arg element, stream, context

  element = DoLoop.Controlled( self, element, stream, context )

  Return element

/******************************************************************************/
/* CONTROLLED LOOP CLAUSE                                                     */
/******************************************************************************/

::Method "Loop.Controlled.Clause::compile"
  Use Strict Arg element, stream, context

  element = DoLoop.Controlled( self, element, stream, context )

  Return element

--------------------------------------------------------------------------------
-- DO/LOOP FOREVER -- Common routine                                          --
--------------------------------------------------------------------------------

::Routine DoLoop.Forever
  Use Strict Arg self, element, stream, context

  element = InitBlock( self, element, stream, context )

  -- Skip FOREVER keyword
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile WHILE or UNTIL phrases, if present
  element = WhilePhrase( self, element, stream, context )
  element = UntilPhrase( self, element, stream, context )

  Return element

/******************************************************************************/
/* DO FOREVER CLAUSE                                                          */
/******************************************************************************/

::Method "Do.Forever.Clause::compile"
  Use Strict Arg element, stream, context

  element = DoLoop.Forever( self, element, stream, context )

  Return element

/******************************************************************************/
/* LOOP FOREVER CLAUSE                                                        */
/******************************************************************************/

::Method "Loop.Forever.Clause::compile"
  Use Strict Arg element, stream, context

  element = DoLoop.Forever( self, element, stream, context )

  Return element

--------------------------------------------------------------------------------
-- DO/LOOP OVER -- Common routine                                             --
--------------------------------------------------------------------------------

::Routine DoLoop.Over
  Use Strict Arg self, element, stream, context

  element = InitBlock( self, element, stream, context )

  -- Compile control variable
  element = element~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone "OVER"
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile collection expression
  element = self~collection~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile FOR, WHILE and UNTIL phrases, if present
  element =   ForPhrase( self, element, stream, context )
  element = WhilePhrase( self, element, stream, context )
  element = UntilPhrase( self, element, stream, context )

  Return element

/******************************************************************************/
/* DO OVER CLAUSE                                                             */
/******************************************************************************/

::Method "Do.Over.Clause::compile"
  Use Strict Arg element, stream, context

  element = DoLoop.Over( self, element, stream, context )

  Return element

/******************************************************************************/
/* LOOP OVER CLAUSE                                                           */
/******************************************************************************/

::Method "Loop.Over.Clause::compile"
  Use Strict Arg element, stream, context

  element = DoLoop.Over( self, element, stream, context )

  Return element

/******************************************************************************/
/* SIMPLE DO CLAUSE                                                           */
/******************************************************************************/

::Method "Simple.Do.Clause::compile"
  Use Strict Arg element, stream, context

  element = InitBlock( self, element, stream, context )

  Return element

/******************************************************************************/
/* SIMPLE LOOP CLAUSE                                                         */
/******************************************************************************/

::Method "Simple.Loop.Clause::compile"
  Use Strict Arg element, stream, context

  element = InitBlock( self, element, stream, context )

  Return element

--------------------------------------------------------------------------------
-- DO/LOOP OVER -- Common routine                                             --
--------------------------------------------------------------------------------

::Routine Simple.Repetitive.DoLoop
  Use Strict Arg self, element, stream, context

  element = InitBlock( self, element, stream, context )

  -- Compile repeatExpression
  element = self~repeatExpression~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile WHILE or UNTIL phrases, if present
  element = WhilePhrase( self, element, stream, context )
  element = UntilPhrase( self, element, stream, context )

  Return element

/******************************************************************************/
/* SIMPLE REPETITIVE DO CLAUSE                                                */
/******************************************************************************/

::Method "Simple.Repetitive.Do.Clause::compile"
  Use Strict Arg element, stream, context

  element = Simple.Repetitive.DoLoop( self, element, stream, context )

  Return element

/******************************************************************************/
/* SIMPLE REPETITIVE LOOP CLAUSE                                              */
/******************************************************************************/

::Method "Simple.Repetitive.Loop.Clause::compile"
  Use Strict Arg element, stream, context

  element = Simple.Repetitive.DoLoop( self, element, stream, context )

  Return element

/******************************************************************************/
/* DO UNTIL CLAUSE                                                            */
/******************************************************************************/

::Method "Do.Until.Clause::compile"
  Use Strict Arg element, stream, context

  element = InitBlock( self, element, stream, context )

  -- Compile UNTIL phrase
  element = UntilPhrase( self, element, stream, context )

  Return element

/******************************************************************************/
/* LOOP UNTIL CLAUSE                                                          */
/******************************************************************************/

::Method "Loop.Until.Clause::compile"
  Use Strict Arg element, stream, context

  element = InitBlock( self, element, stream, context )

  -- Compile UNTIL phrase
  element = UntilPhrase( self, element, stream, context )

  Return element

--------------------------------------------------------------------------------
-- DO/LOOP WITH -- Common routine                                             --
--------------------------------------------------------------------------------

::Routine DoLoop.With
  Use Strict Arg self, element, stream, context

  element = InitBlock( self, element, stream, context )

  -- Clone WITH
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile ITEM and INDEX phrases
  Loop While element < .EL.SUBKEYWORD, -
    WordPos(element~value,"ITEM INDEX") > 0
    kValue = element~value
    -- Clone subkeyword
    element = Clone( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    -- Compile ITEM or INDEX expressions
    If kValue == "ITEM" Then
      element = self~item ~compile( element, stream, context )
    Else
      element = self~index~compile( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End

  -- Clone OVER
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile supplier
  element = self~supplier~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile FOR phrase, if any
  element = ForPhrase( self, element, stream, context )

  -- Compile WHILE or UNTIL phrases, if present
  element = WhilePhrase( self, element, stream, context )
  element = UntilPhrase( self, element, stream, context )

  Return element

/******************************************************************************/
/* DO WITH CLAUSE                                                             */
/******************************************************************************/
::Method "Do.With.Clause::compile"
  Use Strict Arg element, stream, context

  element = DoLoop.With( self, element, stream, context )

  Return element

/******************************************************************************/
/* LOOP WITH CLAUSE                                                           */
/******************************************************************************/
::Method "Loop.With.Clause::compile"
  Use Strict Arg element, stream, context

  element = DoLoop.With( self, element, stream, context )

  Return element

/******************************************************************************/
/* DO WHILE CLAUSE                                                            */
/******************************************************************************/

::Method "Do.While.Clause::compile"
  Use Strict Arg element, stream, context

  element = InitBlock( self, element, stream, context )

  -- Compile WHILE phrase
  element = WhilePhrase( self, element, stream, context )

  Return element

/******************************************************************************/
/* LOOP WHILE CLAUSE                                                          */
/******************************************************************************/

::Method "Loop.While.Clause::compile"
  Use Strict Arg element, stream, context

  element = InitBlock( self, element, stream, context )

  -- Compile WHILE phrase
  element = WhilePhrase( self, element, stream, context )

  Return element
