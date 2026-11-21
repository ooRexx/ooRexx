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
/* SecondPass.cls                                                             */
/* ==============                                                             */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241206    0.1  First public release                                      */
/* 20241209    0.1b New call system                                           */
/* 20241225    0.1d Add support for doc-comments                              */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250414         Move BIF info to BIFs.cls                                 */
/* 20250414         Add argument info to called routine chain                 */
/* 20250418    0.2a Support for BIF early checking                            */
/* 20250418         Fix SIGNAL early checking                                 */
/* 20250531    0.2c Move support for doc-comments to Scanner.cls              */
/* 20260924    0.7  INTERPRET option: when its value is a collection, it      */
/*                  holds the labels of the calling code (as ooRexx passes    */
/*                  them to the INTERPRET translation), to classify calls.    */
/* 20260928         The function calls of the ::CONSTANT expressions are      */
/*                  classified too, with no labels (as ooRexx: BIF, then      */
/*                  ::ROUTINE, then external).                                */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/*                                                                            */
/******************************************************************************/

::Requires "BaseClassesAndRoutines.cls"
::Requires "BIFs.cls"

/******************************************************************************/
--
-- * Label and ::Routine collection
-- * Determination of the call types: internal, BIF, ::Routine, external
--
/******************************************************************************/

::Routine SecondPass Public

  Use Strict Arg package

  Call LoadBIFInfo -- Defined in BIFs.cls

  Call ProcessBody package, package~prolog, package~prolog~body

  Do Counter n fragment Over package~directives
    body = fragment~body
    If \ body~isEmpty Then Call ProcessBody package, fragment, body
  End

  -- The calls of the ::CONSTANT expressions are in no body. ooRexx resolves
  -- them with no labels: a BIF, a ::ROUTINE, or an external routine.
  If package~constantCalls~items > 0 Then
    Call CheckCallTypes package, package~constantCalls, .StringTable~new

--------------------------------------------------------------------------------

::Routine ProcessBody

  Use Strict Arg package, fragment, body

  Call CheckCallTypes package, body~calledRoutineNames, BodyLabels( body )

  -- Check all SIGNAL and SIGNAL ON instructions.

  bodyLabels    = BodyLabels( body )

  checkSignal   = 0
  If .Parser.Options~hasIndex( earlyCheck ) Then
    checkSignal = .Parser.Options~earlyCheck~hasItem( signal )

  Do clause Over body~clauses
    Select Case clause~class
      When .Signal.Instruction, .Signal.On.Instruction Then
        If checkSignal Then Call CheckSignal clause, bodyLabels
      Otherwise Nop
    End
  End

  Exit

--------------------------------------------------------------------------------

::Routine CheckSignal

  Use Strict Arg clause, bodyLabels

  label = clause~label
  If label == .Nil Then Do -- SIGNAL ON only
    label = clause~condition
    If label~value == "USER" Then label = clause~userCondition
  End
  If \bodyLabels~hasIndex( label~value ) Then Signal 16.001

  Return

-- Label "&1" not found.
16.001: Syntax( 16.001, TheElementAfter( clause~begin ), label )

--------------------------------------------------------------------------------

-- Determine the nature of all (non namespace qualified) function
-- and procedure calls, and perform additional checks for BIFs (only
-- if the earlyCheck option is present and contains a BIFs item).
-- "calls" holds (name, arguments[, trailingBlock]) items; "bodyLabels",
-- the labels they are resolved against.

::Routine CheckCallTypes

  Use Strict Arg package, calls, bodyLabels

  If .Parser.Options~hasIndex( earlyCheck ) Then
    checkBIFs = .Parser.Options~earlyCheck~hasItem( BIFs )
  Else checkBIFs = .False

  routines   = package~routines

  Do routine Over calls
    name  = routine[1]
    label = name~value
    type  = name~invocationType
    Select
      When name < .ALL.SYMBOLS, bodyLabels~hasIndex(label) Then Do
        Select Case type
          When "FUNCTION" Then subCategory = .INTERNAL.FUNCTION.NAME
          When "CALL"     Then subCategory = .INTERNAL.SUBROUTINE.NAME
        End
        name~labelFor = bodyLabels[label]~labelFor
      End
      When .Parser.BIF[ label ] \== .Nil     Then Do
        Select Case type
          When "FUNCTION" Then subCategory = .BUILTIN.FUNCTION.NAME
          When "CALL"     Then subCategory = .BUILTIN.SUBROUTINE.NAME
        End
        If checkBIFs Then Call CheckBIFArgs name, routine[2]
      End
      When routines[ label ] \== .Nil Then Do
        Select Case type
          When "FUNCTION" Then subCategory = .PACKAGE.FUNCTION.NAME
          When "CALL"     Then subCategory = .PACKAGE.SUBROUTINE.NAME
        End
      End
      Otherwise Do
        Select Case type
          When "FUNCTION" Then subCategory = .EXTERNAL.FUNCTION.NAME
          When "CALL"     Then subCategory = .EXTERNAL.SUBROUTINE.NAME
        End
      End
    End
    Call SetConstantName name, subCategory
  End

--------------------------------------------------------------------------------

-- The labels calls and SIGNAL are resolved against: those of the body, or,
-- for INTERPRET data, those of the calling code when the INTERPRET option
-- holds them (INTERPRET data has no labels of its own).
::Routine BodyLabels
  Use Strict Arg body
  If .Options.Interpret Then Do
    labels = .Parser.Options~interpret
    If labels~isA( .Collection ) Then Return labels
  End
  Return body~labels
