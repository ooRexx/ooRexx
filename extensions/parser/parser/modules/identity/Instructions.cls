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
/* Instructions.cls -- Clone instructions (part of the identity compiler)     */
/* ======================================================================     */
/*                                                                            */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20251116    0.3a First version                                             */
/* 20261113         CATCH and FINALLY clauses of DO, LOOP and SELECT (RFE     */
/*                  #31): CompileCatchSections                                */
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
/* 20261127         RAISE and THROW: the sign of an ERROR or FAILURE code     */
/*                  (feature request #892)                                    */
/*                                                                            */
/******************************************************************************/

  Call "../Load.Parser.Module.rex"

  pkgLocal = .context~package~local

  -- Set to 1 to activate debug
  pkgLocal~DEBUG = 0

::Requires "compile.cls"

/******************************************************************************/
/******************************************************************************/
/* INSTRUCTIONS                                                               */
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* ADDRESS INSTRUCTIONS                                                       */
/******************************************************************************/

::Method "Address.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for ADDRESS keyword
  element = PrepareClause( element, stream, context )

  -- Clone it
  element = Clone( element, stream, context )

  -- Clone ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  If self~environment \== .Nil Then Do
    element = self~environment~compile( element, stream, context )
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    If self~expression \== .Nil Then Do
      element = self~expression~compile( element, stream, context )
      Loop While element~ignorable
        element = Clone( element, stream, context )
      End
    End
  End

  If self~environmentExpression \== .Nil Then Do
    If element < .EL.SUBKEYWORD Then Do -- VALUE
      element = Clone( element, stream, context )
      Loop While element~ignorable
        element = Clone( element, stream, context )
      End
    End
    element = self~environmentExpression~compile( element, stream, context )
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End

  If element < .EL.END_OF_CLAUSE Then Return element

  -- WITH
  element = Clone( element, stream, context )
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Loop While element \< .EL.END_OF_CLAUSE
    -- INPUT, OUTPUT, ERROR
    redir = element~value
    element = Clone( element, stream, context )
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End

    If WordPos(element~value, "APPEND REPLACE") > 0 Then Do
      element = Clone( element, stream, context )
      Loop While element~ignorable
        element = Clone( element, stream, context )
      End
    End

    option = element~value
    element = Clone( element, stream, context )
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End

    If option \== "NORMAL" Then Do
      redirection = self~send( redir )      -- input, output or error
      element = redirection~target~compile( element, stream, context )
      Loop While element~ignorable
        element = Clone( element, stream, context )
      End
    End

  End

  Return element


/******************************************************************************/
/* ASSIGNMENT INSTRUCTIONS                                                    */
/******************************************************************************/

::Method "Assignment.Instruction::compile"
  Use Strict Arg element, stream, context

  element = PrepareClause( element, stream, context )

  -- Now element is the lhs variable

  element = self~target~compile( element, stream, context )

  expression = self~expression

  If expression == .Nil Then Do -- When "x =;" is allowed
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
    Return element
  End

  begin = expression~firstElement
  Loop While element \== begin
    element = Clone( element, stream, context )
  End

  element = expression~compile( element, stream, context )

  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* CALL INSTRUCTIONS                                                          */
/******************************************************************************/

::Method "Call.Instruction::compile"

  Use Strict Arg element, stream, context

  -- Look for CALL keyword
  element = PrepareClause( element, stream, context )

  -- Clone it
  element = Clone( element, stream, context )

  -- Clone ignorables until function name
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone function name
  element = Clone( element, stream, context )

  -- Clone ignorables until expression list
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  If element \== .EL.END_OF_CLAUSE Then Do
    -- Compile arguments
    element = self~arguments~compile( element, stream, context )
    -- Skip until EOC
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
  End

  Return element

/******************************************************************************/
/* CALL VALUE INSTRUCTIONS                                                    */
/******************************************************************************/

::Method "Call.Value.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for CALL keyword
  element = PrepareClause( element, stream, context )

  -- Clone it
  element = Clone( element, stream, context )

  -- Clone ignorables until left parenthesis
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone the parenthesis
  element = Clone( element, stream, context )

  -- Clone ignorables until expression starts
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile the expression
  element = self~expression~compile( element, stream, context )

  -- Clone ignorables until right parenthesis
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone the parenthesis
  element = Clone( element, stream, context )

  -- Clone ignorables until arguments or EOC
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  If element \== .EL.END_OF_CLAUSE Then Do
    -- Compile arguments
    element = self~arguments~compile( element, stream, context )
    -- Skip until EOC
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
  End

  Return element

/******************************************************************************/
/* CALL OFF INSTRUCTIONS                                                      */
/******************************************************************************/

::Method "Call.Off.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for CALL keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* CALL ON INSTRUCTIONS                                                       */
/******************************************************************************/

::Method "Call.On.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for CALL keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* COMMAND INSTRUCTIONS                                                       */
/******************************************************************************/

::Method "Command.Instruction::compile"

  Use Strict Arg element, stream, context

  -- Skip to expression start
  element = PrepareClause( element, stream, context )

  expression = self~expression

  -- Compile expression
  element = expression~compile( element, stream, context )
  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* DO INSTRUCTIONS                                                            */
/******************************************************************************/

::Method "Do.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Do.Clause will call PrepareClause

  element = self~doClause~compile( element, stream, context )

  Loop instruction Over self~instructions
    element = instruction~compile( element, stream, context )
  End

  element = CompileCatchSections( self, element, stream, context )

  element = self~endClause~compile( element, stream, context )

  Return element

/******************************************************************************/
/* DROP INSTRUCTIONS                                                          */
/******************************************************************************/

::Method "Drop.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for DROP keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* EXIT INSTRUCTIONS                                                          */
/******************************************************************************/

::Method "Exit.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for EXIT keyword
  element = PrepareClause( element, stream, context )

  -- Skip EXIT
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  expression = self~expression

  If expression \== .Nil Then Do
    -- Compile expression
    element = expression~compile( element, stream, context )
    -- Skip until EOC
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
  End

  Return element

/******************************************************************************/
/* EXPOSE INSTRUCTIONS                                                        */
/******************************************************************************/

::Method "Expose.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for EXPOSE keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* FORWARD INSTRUCTIONS                                                       */
/******************************************************************************/

::Method "Forward.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for FORWARD keyword
  element = PrepareClause( element, stream, context )

  -- Skip it
  element = Clone( element, stream, context )

  Do Until element \< .EL.SUBKEYWORD
    -- Skip ignorables
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
    If element < .EL.SUBKEYWORD Then Do
      last = element
      -- Skip ignorables
      Loop While element \< .EL.END_OF_CLAUSE
        element = Clone( element, stream, context )
      End
      Select Case Lower( last~value )
        When "continue"  Then Nop
        When "arguments" Then
          element = self~arguments~compile( element, stream, context )
        When "array" Then
          element = self~array    ~compile( element, stream, context )
        When "message" Then
          element = self~message  ~compile( element, stream, context )
        When "class" Then
          element = self~theClass ~compile( element, stream, context )
        When "to" Then
          element = self~to       ~compile( element, stream, context )
      End
      Iterate
    End
  End

  Return element

/******************************************************************************/
/* GUARD INSTRUCTIONS                                                         */
/******************************************************************************/

::Method "Guard.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for GUARD keyword
  element = PrepareClause( element, stream, context )

  -- Skip it
  element = Clone( element, stream, context )

  -- Skip ignorables until ON or OFF
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Skip ON or OFF
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- EOC? We are done
  If element < .EL.END_OF_CLAUSE Then Return element

  -- That's WHEN. Skip it
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile expression
  element = self~whenExpressions~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element


/******************************************************************************/
/* IF INSTRUCTIONS                                                            */
/******************************************************************************/

::Method "If.Instruction::compile"
  Use Strict Arg element, stream, context

  -- If.Clause will call PrepareClause

  element = self~ifClause         ~compile( element, stream, context )
  element = self~thenClause       ~compile( element, stream, context )
  element = self~thenInstruction  ~compile( element, stream, context )

  elseClause = self~elseClause
  If elseClause \== .Nil Then Do
    element = elseClause          ~compile( element, stream, context )
    element = self~elseInstruction~compile( element, stream, context )
  End

  Return element

/******************************************************************************/
/* IMPLICIT EXIT INSTRUCTIONS                                                 */
/******************************************************************************/

::Method "Implicit.Exit.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for implicit EXIT marker keyword
  element = PrepareClause( element, stream, context )

  -- Skip it
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

::Method "Interpret.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for INTERPRET keyword
  element = PrepareClause( element, stream, context )

  -- Skip INTERPRET
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  expression = self~expression

  -- Compile expression
  element = expression~compile( element, stream, context )
  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

::Method "Iterate.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for ITERATE keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

::Method "Leave.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for LEAVE keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* LOOP INSTRUCTIONS                                                          */
/******************************************************************************/

::Method "Loop.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Loop.Clause will call PrepareClause

  element = self~loopClause~compile( element, stream, context )

  Loop instruction Over self~instructions
    element = instruction~compile( element, stream, context )
  End

  element = CompileCatchSections( self, element, stream, context )

  element = self~endClause~compile( element, stream, context )

  Return element

/******************************************************************************/
/* MESSAGE ASSIGNMENT INSTRUCTIONS                                            */
/******************************************************************************/

::Method "Message.Assignment.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Skip until start of lhs
  element = PrepareClause( element, stream, context )

  -- Compile the lhs
  element = self~target~compile( element, stream, context )

  -- Clone everything until the expression begins
  rhs   = self~expression
  begin = rhs~firstElement
  Loop While element \== begin
    element = Clone( element, stream, context )
  End

  -- Compile the rhs
  element = rhs~compile( element, stream, context )

  -- Skip to EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* MESSAGE INSTRUCTIONS                                                       */
/******************************************************************************/

::Method "Message.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Skip until start of expression
  element = PrepareClause( element, stream, context )

  -- Compile the expression
  element = self~expression~compile( element, stream, context )

  -- Skip to EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* NAMESPACE QUALIFIED CALL INSTRUCTIONS                                      */
/******************************************************************************/

::Method "Namespace.Qualified.Call.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for CALL keyword
  element = PrepareClause( element, stream, context )

  -- Clone it
  element = Clone( element, stream, context )

  -- Clone ignorables until namespace
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone it
  element = Clone( element, stream, context )

  -- Clone ignorables until colon
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone it
  element = Clone( element, stream, context )

  -- Clone ignorables until function name
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone function name
  element = Clone( element, stream, context )

  -- Clone ignorables until argument list
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  If element \== .EL.END_OF_CLAUSE Then Do
    -- Compile arguments
    element = self~arguments~compile( element, stream, context )
    -- Skip until EOC
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
  End

  Return element

/******************************************************************************/
/* NOP INSTRUCTIONS                                                           */
/******************************************************************************/

::Method "Nop.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for NOP keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* NUMERIC INSTRUCTIONS                                                       */
/******************************************************************************/

::Method "Numeric.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for NUMERIC keyword
  element = PrepareClause( element, stream, context )

  Do Until element \< .EL.SUBKEYWORD
    -- Skip it
    element = Clone( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End

  -- No EOC?
  If element \< .EL.END_OF_CLAUSE Then Do
    -- This must be an expression
    element = self~expression~compile( element, stream, context )
  End

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* OPTIONS INSTRUCTIONS                                                       */
/******************************************************************************/

::Method "Options.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for OPTIONS keyword
  element = PrepareClause( element, stream, context )

  -- Skip OPTIONS
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  expression = self~expression

  -- Compile expression
  element = expression~compile( element, stream, context )
  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* PROCEDURE INSTRUCTIONS                                                     */
/******************************************************************************/

::Method "Procedure.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for PROCEDURE keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* PUSH INSTRUCTIONS                                                          */
/******************************************************************************/

::Method "Push.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for PUSH keyword
  element = PrepareClause( element, stream, context )

  -- Skip PUSH
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  expression = self~expression

  If expression \== .Nil Then Do
    -- Compile expression
    element = expression~compile( element, stream, context )
    -- Skip until EOC
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
  End

  Return element

/******************************************************************************/
/* QUEUE INSTRUCTIONS                                                         */
/******************************************************************************/

::Method "Queue.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for QUEUE keyword
  element = PrepareClause( element, stream, context )

  -- Skip QUEUE
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  expression = self~expression

  If expression \== .Nil Then Do
    -- Compile expression
    element = expression~compile( element, stream, context )
    -- Skip until EOC
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
  End

  Return element

/******************************************************************************/
/* RAISE INSTRUCTIONS                                                         */
/******************************************************************************/

::Method "Raise.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for RAISE keyword
  element = PrepareClause( element, stream, context )

  -- Skip RAISE
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Skip subkeyword (condition name or PROPAGATE)
  element = Clone( element, stream, context )

  condition = self~condition
  If condition == .Nil Then Nop               -- PROPAGATE
  Else If condition~code \== .Nil Then Do
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    -- The sign of an ERROR or FAILURE code (feature request #892)
    If condition~sign \== .Nil Then Do
      element = Clone( element, stream, context )
      Loop While element~ignorable
        element = Clone( element, stream, context )
      End
    End
    -- Compile code
    element = condition~code~compile( element, stream, context )
  End
  Else If condition~userCondition \== .Nil Then Do
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    -- Skip usercondition
    element = Clone( element, stream, context )
  End

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Loop While element \< .EL.END_OF_CLAUSE
    keyword = element
    -- Skip keyword
    element = Clone( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    -- Process associated expression
    Select Case keyword~value
      When "ADDITIONAL" Then
        element = self~additional ~compile( element, stream, context )
      When "ARRAY" Then
        If self~array = "" Then Do -- ARRAY ()
          -- Clone "("
          element = Clone( element, stream, context )
          Loop While element~ignorable
            element = Clone( element, stream, context )
          End
          -- Clone ")"
          element = Clone( element, stream, context )
        End
        Else
          element = self~array    ~compile( element, stream, context )
      When "DESCRIPTION" Then
        element = self~description~compile( element, stream, context )
      When "EXIT" Then
        If self~exit \== "" Then
          element = self~exit     ~compile( element, stream, context )
      When "RETURN" Then
        If self~return \== "" Then
          element = self~return   ~compile( element, stream, context )
    End
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End

  Return element

/******************************************************************************/
/* REPLY INSTRUCTIONS                                                         */
/******************************************************************************/

::Method "Reply.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for REPLY keyword
  element = PrepareClause( element, stream, context )

  -- Skip REPLY
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  expression = self~expression

  If expression \== .Nil Then Do
    -- Compile expression
    element = expression~compile( element, stream, context )
    -- Skip until EOC
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
  End

  Return element

/******************************************************************************/
/* RETURN INSTRUCTIONS                                                        */
/******************************************************************************/

::Method "Return.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for RETURN keyword
  element = PrepareClause( element, stream, context )

  -- Skip RETURN
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  expression = self~expression

  If expression \== .Nil Then Do
    -- Compile expression
    element = expression~compile( element, stream, context )
    -- Skip until EOC
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
  End

  Return element

/******************************************************************************/
/* SAY INSTRUCTIONS                                                           */
/******************************************************************************/

::Method "Say.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for SAY keyword
  element = PrepareClause( element, stream, context )

  -- Skip SAY
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  expression = self~expression

  If expression \== .Nil Then Do
    -- Compile expression
    element = expression~compile( element, stream, context )
    -- Skip until EOC
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End
  End

  Return element

/******************************************************************************/
/* SELECT INSTRUCTIONS                                                        */
/******************************************************************************/

::Method "Select.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Select.Clause will call PrepareClause

   element = self~selectClause~compile( element, stream, context )

  Do section Over self~whenSections
    whenClause      = section~whenClause
    thenClause      = section~thenClause
    thenInstruction = section~thenInstruction

    element         = whenClause~compile(      element, stream, context )
    element         = thenClause~compile(      element, stream, context )
    element         = thenInstruction~compile( element, stream, context )
  End

  otherwiseClause = self~otherwiseClause
  If otherwiseClause \== .Nil Then Do
    element = otherwiseClause~compile( element, stream, context )

    Do instruction Over self~otherwiseInstructions
      element = instruction~compile(   element, stream, context )
    End
  End

  element = CompileCatchSections( self, element, stream, context )

  element = self~endClause~compile(    element, stream, context )

  Return element

/******************************************************************************/
/* SIGNAL INSTRUCTIONS                                                        */
/******************************************************************************/

::Method "Signal.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for SIGNAL keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* SIGNAL OFF INSTRUCTIONS                                                    */
/******************************************************************************/

::Method "Signal.Off.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for SIGNAL keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* SIGNAL ON INSTRUCTIONS                                                     */
/******************************************************************************/

::Method "Signal.On.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for SIGNAL keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* SIGNAL VALUE INSTRUCTIONS                                                  */
/******************************************************************************/

::Method "Signal.Value.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for SIGNAL keyword
  element = PrepareClause( element, stream, context )

  -- Skip it
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- VALUE?
  If element < .EL.SUBKEYWORD Then Do
    -- Skip it
    element = Clone( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End

  element = self~expression~compile( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* TRACE INSTRUCTIONS                                                         */
/******************************************************************************/

::Method "Trace.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for TRACE keyword
  element = PrepareClause( element, stream, context )

  -- Skip it
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ ignorable
    element = Clone( element, stream, context )
  End

  Select
    When element < .ALL.NUMBERS Then
      element = self~number~compile( element, stream, context )
    When element < .ALL.SYMBOLS Then
      element = self~symbol~compile( element, stream, context )
    When element < .ALL.STRINGS Then
      element = self~theString~compile( element, stream, context )
    When element < .EL.SUBKEYWORD Then Do -- VALUE
      -- Skip "VALUE"
      element = Clone( element, stream, context )
      -- Skip ignorables
      Loop While element~ ignorable
        element = Clone( element, stream, context )
      End
      element = self~expression~compile( element, stream, context )
    End
    When self~expression == .Nil Then Nop
    Otherwise -- Expression
      element = self~expression~compile( element, stream, context )
  End

  -- Skip ignorables
  Loop While element~ ignorable
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* UPPER INSTRUCTIONS                                                         */
/******************************************************************************/

::Method "Upper.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for UPPER keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* USE ARG INSTRUCTIONS                                                       */
/******************************************************************************/

::Method "Use.Arg.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for USE keyword
  element = PrepareClause( element, stream, context )

  -- Skip over it
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Do we expect STRICT?
  If self~strict Then Do
    -- Skip over it
    element = Clone( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End

  -- Do we expect AUTO?
  If self~auto Then Do
    -- Skip over it
    element = Clone( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End

  -- Do we expect NAMED?
  If self~named Then Do
    -- Skip over it
    element = Clone( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End

  -- Skip over ARG
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- USE ARG; or (more frequent) USE STRICT ARG;
  If element < .EL.END_OF_CLAUSE Then Return element

  arguments  = self~arguments
  nArguments = arguments~items
  Do Counter c argument Over arguments

    -- Compile name
    name    = argument~target
    element = name~compile( element, stream, context )

    If argument~defaultValue \== .Nil Then Do
      -- Skip ignorables until "="
      Loop While element~ignorable
        element = Clone( element, stream, context )
      End
      -- Skip "="
      element = Clone( element, stream, context )
      -- Skip ignorables until expr start
      Loop While element~ignorable
        element = Clone( element, stream, context )
      End
      -- Compile exp
      expr    = argument~defaultValue
      element = expr~compile( element, stream, context )
   End

    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    If c < nArguments Then Do
      -- Skip ","
      element = Clone( element, stream, context )
      -- Skip ignorables
      Loop While element~ignorable
        element = Clone( element, stream, context )
      End
    End
  End

  -- The ellipsis ("..."), and the comma before it
  If self~ellipsis \== .Nil Then
    Loop While element \< .EL.END_OF_CLAUSE
      element = Clone( element, stream, context )
    End

  Return element

/******************************************************************************/
/* USE LOCAL INSTRUCTIONS                                                     */
/******************************************************************************/

::Method "Use.Local.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for USE keyword
  element = PrepareClause( element, stream, context )

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* CompileCatchSections: the CATCH and FINALLY clauses of a DO, LOOP or       */
/* SELECT, and their code (RFE #31)                                           */
/******************************************************************************/

::Routine CompileCatchSections
  Use Strict Arg block, element, stream, context

  sections = block~catchSections~copy
  If block~finallySection \== .Nil Then sections~append( block~finallySection )
  Do section Over sections
    element = section[1]~compile( element, stream, context )
    Do instruction Over section~instructions
      element = instruction~compile( element, stream, context )
    End
  End

  Return element
