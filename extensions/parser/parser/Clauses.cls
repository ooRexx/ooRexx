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
/* Clauses.cls                                                                */
/* ===========                                                                */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241206    0.1  First public release                                      */
/* 20241208    0.1a Clauser: accept options arg, pass it to preclauser        */
/* 20250116    0.1f Add "assigned" attribute to variable assignment targets   */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250421    0.2b Allow optional empty assignments                          */
/* 20251017    0.2e Add support for Lambdas                                   */
/* 20251017    0.3a Add support for Executor "V=;"                            */
/* 20251201         (Executor) Add support for source literals                */
/* 20251226    0.4a Send error messages to .error, not .output                */
/* 20260522    0.5  CMS: Implement DO and SELECT clauses                      */
/* 20260601    0.6  Implement a fine-grained feature system                   */
/* 20260601         Implement empty assignments                               */
/* 20260924    0.7  SELECT CASE: the clause ends at its end, not at the last  */
/*                  token of the expression                                   */
/* 20260924         A repetitive DO without END is 14.5, as for LOOP; only a  */
/*                  simple DO (DO or DO LABEL name) is 14.1, as in ooRexx     */
/* 20260927         The look ahead of a DO clause left the blanks after its   */
/*                  first symbol ignorable: "do a b 2" repeated "AB 2" times  */
/*                  (Lookahead; a simple repetitive DO puts them back)        */
/* 20260928         "DO symbol ==" (LABEL and COUNTER too) is 35.1, as in     */
/*                  ooRexx                                                    */
/* 20260928         A label before THEN is 18.1 (IF) or 18.2 (WHEN), as in    */
/*                  ooRexx (it was 47.3, 47.4); a label before the END of a   */
/*                  SELECT with no OTHERWISE is 47.4 (it was accepted). In a  */
/*                  block, a label is an error as soon as it is seen, before  */
/*                  the clause that follows it is parsed, as in ooRexx        */
/*                  ("DO; x: SAY (" is 47.2, not 35.1).                       */
/* 20260930         A SELECT that starts with OTHERWISE is parsed to its END, */
/*                  and then it is 7.1, as in ooRexx (it was 7.2). A missing  */
/*                  expression in the WHEN of a SELECT CASE is 35.934         */
/* 20260930         10.003, 10.007: the substitutions in ooRexx's order       */
/*                  (symbol, then line); they were swapped. rexxmsg.xml       */
/*                  names them the wrong way round, but its positions         */
/*                  are right                                                 */
/* 20260930         An assignment to a constant symbol ("1 = 2", "3D = 5",    */
/*                  ".x = 1") is 31.2 or 31.3, as in ooRexx (it was accepted) */
/* 20261113         An END right after THEN or ELSE is 10.5 or 10.6, as in    */
/*                  ooRexx since r13490 (bug #2024; it was 10.1)              */
/* 20261113         CATCH and FINALLY clauses of DO, LOOP and SELECT (RFE     */
/*                  #31): Catch.Clause, Finally.Clause, Catch.Sections; the   */
/*                  block instructions get a catchSections attribute          */
/* 20261113         CATCH SYNTAX code: a number, a literal string or a simple */
/*                  variable (the spec of 13/11/2026, afternoon); a missing   */
/*                  comma after SYNTAX is no longer 21.904                    */
/* 20261117         FORWARD without CONTINUE in a block that has a FINALLY    */
/*                  clause, at any depth, is 99.952 at the FINALLY (ooRexx    */
/*                  r13535)                                                   */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261122         The implicit EXIT is terminating, as EXIT and RETURN      */
/* 20261124         The clauses and instructions of DO, LOOP, IF and SELECT   */
/*                  have the block instruction as their parent                */
/*                                                                            */
/******************************************************************************/

 .environment~                  End.Of.Source =                  .End.Of.Source

 .environment~                      Do.Clause =                      .Do.Clause
 .environment~                 Do.Over.Clause =                 .Do.Over.Clause
 .environment~                 Do.With.Clause =                 .Do.With.Clause
 .environment~                Do.While.Clause =                .Do.While.Clause
 .environment~                Do.Until.Clause =                .Do.Until.Clause
 .environment~              Do.Forever.Clause =              .Do.Forever.Clause
 .environment~    Simple.Repetitive.Do.Clause =    .Simple.Repetitive.Do.Clause
 .environment~           Do.Controlled.Clause =           .Do.Controlled.Clause
 .environment~               Simple.Do.Clause =               .Simple.Do.Clause
 .environment~                     End.Clause =                     .End.Clause
 .environment~                   Catch.Clause =                   .Catch.Clause
 .environment~                Catch.Condition =                .Catch.Condition
 .environment~                    Else.Clause =                    .Else.Clause
 .environment~                 Finally.Clause =                 .Finally.Clause
 .environment~                      If.Clause =                      .If.Clause
 .environment~                   Label.Clause =                   .Label.Clause
 .environment~                    Loop.Clause =                    .Loop.Clause
 .environment~               Loop.Over.Clause =               .Loop.Over.Clause
 .environment~               Loop.With.Clause =               .Loop.With.Clause
 .environment~              Loop.While.Clause =              .Loop.While.Clause
 .environment~              Loop.Until.Clause =              .Loop.Until.Clause
 .environment~            Loop.Forever.Clause =            .Loop.Forever.Clause
 .environment~  Simple.Repetitive.Loop.Clause =  .Simple.Repetitive.Loop.Clause
 .environment~         Loop.Controlled.Clause =         .Loop.Controlled.Clause
 .environment~             Simple.Loop.Clause =             .Simple.Loop.Clause
 .environment~                    Null.Clause =                    .Null.Clause
 .environment~               Iterative.Clause =               .Iterative.Clause
 .environment~               Otherwise.Clause =               .Otherwise.Clause
 .environment~                  Select.Clause =                  .Select.Clause
 .environment~                    Then.Clause =                    .Then.Clause
 .environment~                    When.Clause =                    .When.Clause

 .environment~         Assignment.Instruction =         .Assignment.Instruction
 .environment~            Command.Instruction =            .Command.Instruction
 .environment~                 Do.Instruction =                 .Do.Instruction
 .environment~                 If.Instruction =                 .If.Instruction
 .environment~      Implicit.Exit.Instruction =      .Implicit.Exit.Instruction
 .environment~          Iterative.Instruction =          .Iterative.Instruction
 .environment~               Loop.Instruction =               .Loop.Instruction
 .environment~ Message.Assignment.Instruction = .Message.Assignment.Instruction
 .environment~            Message.Instruction =            .Message.Instruction
 .environment~             Select.Instruction =             .Select.Instruction

 .environment~              Block.Instruction =              .Block.Instruction
 .environment~               Instruction.List =               .Instruction.List
 .environment~             Source.Literal.End =             .Source.Literal.End

::Requires "BaseClassesAndRoutines.cls"
::Requires "KeywordInstructions.cls"
::Requires "Directives.cls"
::Requires "Scanner.cls"
::Requires "PreClauser.cls"
::Requires "Expressions.cls"

/******************************************************************************/
/******************************************************************************/
/* The CLAUSER class                                                          */
/******************************************************************************/
/******************************************************************************/

::Class Clauser Public

::Attribute scanner
::Attribute preClauser

/******************************************************************************/
/* INIT                                                                       */
/******************************************************************************/

::Method init
  Expose package preClauser scanner bufferedInstructionFragment

  Use Strict Arg package

  preClauser = .PreClauser~new( package )
  scanner    =  preClauser~scanner

  bufferedInstructionFragment = .Nil

/******************************************************************************/
/* BACKTRACK                                                                  */
/******************************************************************************/

::Method backTrack
  Expose bufferedInstructionFragment

  bufferedInstructionFragment = Arg(1)

/******************************************************************************/
/* NextClause                                                                 */
/******************************************************************************/

::Method NextClause
  Expose package preClauser

  Use Strict Arg

  --
  -- The NextClause method of the PreClauser class returns an array
  -- consisting of four items:
  --
  -- array[1]: type:     is the clause type. Its value will be used to locate
  --                     the corresponding clause handling routine.
  -- array[2]: begin:    the (possibly inserted) semicolon preceding the clause
  -- array[3]: end:      the (possibly inserted) semicolon ending the clause
  -- array[4]: elements: a (possibly empty) array of elements comprised between
  --                     begin and end, without including these.
  --
  array = preClauser~nextClause

  name = array[1]

  -- Locate our routine
  routine = .context~package~findRoutine( name )

  -- This should not happen. It is here for debug purposes.
  If routine == .Nil Then Do
   .Error~Say( "Routine '"name"' not found!" )
    Raise HALT
  End

  array[1] = package

  -- Invoke our routine and pass all the parameters
  Return routine~callWith( array )

/******************************************************************************/
/* NEXT.INSTRUCTION.FRAGMENT                                                  */
/*   get a clause which is not a null clause, a label or a directive;         */
/*   it can be a full non-block instruction. Labels and null clauses will     */
/*   be collected and stored as attributes of the clause.                     */
/******************************************************************************/

::Method Next.Instruction.Fragment
  Expose package bufferedInstructionFragment

  -- labelError: where a label is not allowed (in a block), the error that
  -- a label is, raised as soon as the label is seen (before the clause
  -- that follows it is parsed), as ooRexx: (code) for 47.x (the label is
  -- the insert), or (code, insert).
  Use Strict Arg labelError = .Nil

  -- This allows us to implement a very simple backtrack mechanism
  If bufferedInstructionFragment \== .Nil Then Do
    save = bufferedInstructionFragment
    bufferedInstructionFragment = .Nil
    Return save
  End

  labels      = Array()
  nullClauses = Array()

  clause      = self~nextClause

  Loop
    Select Case clause~class
      When .Label.Clause Then Do
        If labelError \== .Nil Then Signal LabelError
        labels~append( clause )
      End
      When .Null.Clause  Then nullClauses~append( clause )
      Otherwise Leave
    End
    clause    = self~nextClause
  End

  clause~labels      = labels
  clause~nullClauses = nullClauses

  Do i = 1 To labels~items
    labels[i]~labelFor       = clause
    labels[i]~label~labelFor = clause
  End

  Return clause

LabelError:
  If labelError~items == 1 Then
    Syntax( labelError[1], clause~label, clause~label )
  Else
    Syntax( labelError[1], clause~label, labelError[2] )

/******************************************************************************/
/* NEXTINSTRUCTION                                                            */
/*   Get a instruction. This can be a simple instruction (i.e., a keyword     */
/*   instruction, a message instruction or a command), or a compound or       */
/*   "block" instruction. Labels and null clauses preceding the instruction   */
/*   are collected by the "nextInstructionFragment" method and are returned   */
/*   as part of the instruction. They can be accessed using the "~labels"     */
/*   and "~nullClauses" methods.                                              */
/******************************************************************************/

::Method Next.Instruction

  -- after: "THEN" or "ELSE" when the instruction is the one of a THEN or of
  -- an ELSE, where an END is 10.5 or 10.6, and a CATCH or FINALLY clause is
  -- 10.9, 10.11, 10.13 or 10.14
  Use Strict Arg ENDClauseIsOK = .False, Source.Literal = .False, -
    labelError = .Nil, after = ""

  clause = self~Next.Instruction.Fragment( labelError )

  firstElement = TheElementAfter( clause~begin )

  -- Simple, non-block clauses are automatically instructions
  If clause~isA( .Rexx.Instruction ) Then Return clause

  Select Case clause~class

    When            .If.Clause Then Return self~If.instruction(     clause )
    When            .Do.Clause, -
              .Do.While.Clause, -
              .Do.Until.Clause, -
               .Do.Over.Clause, -
               .Do.With.Clause, -
         .Do.Controlled.Clause, -
            .Do.Forever.Clause, -
  .Simple.Repetitive.Do.Clause, -
             .Simple.Do.Clause Then Return self~Do.instruction(     clause )
    When          .Loop.Clause, -
            .Loop.While.Clause, -
            .Loop.Until.Clause, -
             .Loop.Over.Clause, -
             .Loop.With.Clause, -
       .Loop.Controlled.Clause, -
          .Loop.Forever.Clause, -
.Simple.Repetitive.Loop.Clause, -
           .Simple.Loop.Clause Then Return self~Loop.instruction(   clause )
    When        .Select.Clause Then Return self~Select.instruction( clause )

    When          .Then.Clause Then Signal 8.001
    When          .Else.Clause Then Signal 8.002
    When          .When.Clause Then Signal 9.001
    When     .Otherwise.Clause Then Signal 9.002
    When         .Catch.Clause Then Select
      When after == "THEN"     Then Signal 10.009
      When after == "ELSE"     Then Signal 10.011
      When ENDClauseIsOK       Then Return clause
      Otherwise                     Signal 10.008
    End
    When       .Finally.Clause Then Select
      When after == "THEN"     Then Signal 10.013
      When after == "ELSE"     Then Signal 10.014
      When ENDClauseIsOK       Then Return clause
      Otherwise                     Signal 10.012
    End
    -- An END right after THEN or ELSE is 10.5 or 10.6 (bug #2024, r13490)
    When           .End.Clause Then Select
      When after == "THEN"     Then Signal 10.005
      When after == "ELSE"     Then Signal 10.006
      When ENDClauseIsOK       Then Return clause
      Otherwise                     Signal 10.001
    End
    When          .R.Directive Then
      If Source.Literal        Then Return clause
      Else                          Signal 99.916
    When .Coactivity.Directive Then
      If Source.Literal        Then Return clause
      Else                          Signal 99.916
    When   .Source.Literal.End Then
      If Source.Literal        Then Return clause
      Else                          Signal 90.900
    Otherwise
      Internal.Error("Next.Instruction: Unexpected clause class '"clause~class"'.")
  End

-- THEN has no corresponding IF or WHEN clause.
 8.001: Syntax(  8.001, firstElement )

-- ELSE has no corresponding THEN clause.
 8.002: Syntax(  8.002, firstElement )

-- WHEN has no corresponding SELECT.
 9.001: Syntax(  9.001, firstElement )

-- OTHERWISE has no corresponding SELECT.
 9.002: Syntax(  9.002, firstElement )

-- END has no corresponding DO, LOOP, or SELECT.
10.001: Syntax( 10.001, firstElement )

-- END must not immediately follow THEN.
10.005: Syntax( 10.005, firstElement )

-- END must not immediately follow ELSE.
10.006: Syntax( 10.006, firstElement )

-- CATCH has no corresponding DO, LOOP, or SELECT.
10.008: Syntax( 10.008, firstElement )

-- CATCH must not immediately follow THEN.
10.009: Syntax( 10.009, firstElement )

-- CATCH must not immediately follow ELSE.
10.011: Syntax( 10.011, firstElement )

-- FINALLY has no corresponding DO, LOOP, or SELECT.
10.012: Syntax( 10.012, firstElement )

-- FINALLY must not immediately follow THEN.
10.013: Syntax( 10.013, firstElement )

-- FINALLY must not immediately follow ELSE.
10.014: Syntax( 10.014, firstElement )

-- Unexpected "}" found
90.900: Syntax( 90.900, firstElement, 'Unexpected "}" found' )

-- Unrecognized directive instruction.
99.916: Syntax( 99.916, firstElement )

/******************************************************************************/
/* DO INSTRUCTION                                                             */
/******************************************************************************/

::Method Do.Instruction
  Expose package

  Use Strict Arg doClause

  DO     = TheElementAfter( doClause~begin )
  doLine = DO~from~word(1)

  instructions  = Array()
  catchSections = Array()

  ENDClauseIsOK = 1

  lastInstruction = doClause

  currentBody = package~currentBodies~top

  Loop
    instruction = self~Next.Instruction( ENDClauseIsOK, , .Array~of( 47.002 ) )
    If \instruction~labels~isEmpty Then Signal 47.002
    instrClass = instruction~class
    Select Case instrClass
      When .End.Clause                Then Signal End.Clause
      When .Catch.Clause, .Finally.Clause Then Signal Catch
      When .Expose.Instruction        Then Signal 99.907
      When .Implicit.Exit.Instruction Then Do
        If doClause~isA( .Simple.Do.Clause ) Then Signal 14.001
        Signal 14.005
      End
      When .Use.Arg.Instruction, .Arg.Instruction
                                      Then currentBody~signature = .Nil
      When .Parse.Instruction Then If instruction~variant == "ARG"
                                      Then currentBody~signature = .Nil
      Otherwise Nop
    End
    instructions~append( instruction )
    lastInstruction = instruction
  End

Catch:
  sections      = self~Catch.Sections( doClause, instruction )
  catchSections = sections[1]
  instruction   = sections[2]

End.Clause:
  END = TheElementAfter( instruction~begin )
  name = instruction~name
  If name \== "" Then Do
    doLabel = doClause~name
    If doLabel == .Nil        Then Do
      doLabel = doClause~control
      If doLabel == .Nil Then Signal 10.003
    End
    If name \== doLabel~value  Then Signal 10.002
  End
  Return .Do.Instruction~new( -
    package, doClause, instructions, instruction, catchSections -
  )

-- Symbol following END ("&1") must match block specification name ("&2")
-- on line &3 or be omitted.
10.002: Syntax( 10.002, END, name, doLabel, doLine )

-- END corresponding to block on line &2 must not have a symbol
-- following it because there is no LABEL or control variable; found "&1".
10.003: Syntax( 10.003, END, name, doLine )

-- DO instruction on line &1 requires matching END.
14.001: Syntax( 14.001, instruction~end, doLine )

-- DO or LOOP instruction on line &1 requires matching END.
14.005: Syntax( 14.005, instruction~end, doLine )

-- Labels are not allowed within a DO/LOOP block; found "&1".
47.002: Syntax( 47.002, instruction~end, instruction~labels[1]~label )

-- EXPOSE must be the first instruction executed after a method invocation.
99.907: Syntax( 99.907, TheElementAfter(instruction~begin) )

/******************************************************************************/
/* IF INSTRUCTION                                                             */
/******************************************************************************/

::Method If.Instruction
  Expose package

  Use Strict Arg ifClause

  IF = TheElementAfter( ifClause~begin )

  thenClause = self~Next.Instruction.Fragment( -
    .Array~of( 18.001, Word( IF~from, 1 ) ) -
  )

  THEN = TheElementAfter( thenClause~begin )

  -- A label before THEN: the THEN clause is not there (ooRexx: 18.1).
  If \thenClause~isA( .Then.Clause ) Then Signal 18.001
  If \thenClause~labels~isEmpty      Then Signal 18.001

  thenInstruction = self~Next.Instruction( , , .Array~of( 47.003 ), "THEN" )

  Call NoLabels thenInstruction

  currentBody = package~currentBodies~top

  Select Case thenInstruction~class
    When .Implicit.Exit.Instruction Then Signal 14.003
    When .Expose.Instruction        Then Call 99.907 thenInstruction
    When .Use.Arg.Instruction, .Arg.Instruction
                                    Then currentBody~signature = .Nil
    When .Parse.Instruction Then If thenInstruction~variant == "ARG"
                                    Then currentBody~signature = .Nil

    Otherwise Nop
  End

  elseClause = self~Next.Instruction.Fragment

  If \elseClause~isA(.Else.Clause) Then Do
    self~backTrack( elseClause )
    Return .If.Instruction~new(                      -
      package, ifClause, thenClause, thenInstruction -
    )
  End

  Call NoLabels elseClause

  ELSE = TheElementAfter( elseClause~begin )

  elseInstruction = self~Next.Instruction( , , .Array~of( 47.003 ), "ELSE" )

  Call NoLabels elseInstruction

  Select Case elseInstruction~class
    When .Implicit.Exit.Instruction Then Signal 14.004
    When .Expose.Instruction Then Call 99.907 elseInstruction
    When .Use.Arg.Instruction, .Arg.Instruction
                                    Then currentBody~signature = .Nil
    When .Parse.Instruction Then If elseInstruction~variant == "ARG"
                                    Then currentBody~signature = .Nil
    Otherwise Nop
  End

  Return .If.Instruction~new(    -
    package, ifClause,           -
    thenClause, thenInstruction, -
    elseClause, elseInstruction  -
  )

NoLabels:
  If \Arg(1)~labels~isEmpty Then Call 47.003 Arg(1)
  Return

-- THEN on line &1 must be followed by an instruction.
14.003: Syntax( 14.003, THEN, IF~from~word(1) )

-- ELSE on line &1 must be followed by an instruction.
14.004: Syntax( 14.004, ELSE, ELSE~from~word(1) )

-- IF instruction on line &1 requires matching THEN clause.
18.001: Syntax( 18.001, THEN, Word(IF~from, 1) )

-- Labels are not allowed within an IF block; found "&1".
47.003: Syntax( 47.003, arg(1)~begin, Arg(1)~labels[1]~label )

-- EXPOSE must be the first instruction executed after a method invocation.
99.907: Syntax( 99.907, TheElementAfter(Arg(1)~begin) )

/******************************************************************************/
/* LOOP INSTRUCTION                                                           */
/******************************************************************************/

::Method Loop.Instruction
  Expose package

  Use Strict Arg loopClause

  LOOP     = TheElementAfter( loopClause~begin )
  loopLine = LOOP~from~word(1)

  instructions  = Array()
  catchSections = Array()

  ENDClauseIsOK = 1

  lastInstruction = loopClause

  currentBody = package~currentBodies~top

  Loop
    instruction = self~Next.Instruction( ENDClauseIsOK, , .Array~of( 47.002 ) )
    If \instruction~labels~isEmpty    Then Signal 47.002
    Select Case instruction~class
      When .End.Clause                Then Signal End.Clause
      When .Catch.Clause, .Finally.Clause Then Signal Catch
      When .Expose.Instruction        Then Signal 99.907
      When .Implicit.Exit.Instruction Then Signal 14.005
      When .Use.Arg.Instruction, .Arg.Instruction
                                      Then currentBody~signature = .Nil
      When .Parse.Instruction Then If instruction~variant == "ARG"
                                      Then currentBody~signature = .Nil
      Otherwise Nop
    End
    instructions~append( instruction )
    lastInstruction = instruction
  End

Catch:
  sections      = self~Catch.Sections( loopClause, instruction )
  catchSections = sections[1]
  instruction   = sections[2]

End.Clause:
  END = TheElementAfter(instruction~begin)
  name = instruction~name
  If name \== "" Then Do
    loopLabel = loopClause~name
    If loopLabel == .Nil   Then Do
      loopLabel = loopClause~control
      If loopLabel == .Nil Then Signal 10.003
    End
    If name \== loopLabel~value  Then Signal 10.002
  End
  Return .Loop.Instruction~new( -
    package, loopClause, instructions, instruction, catchSections -
  )

-- Symbol following END ("&1") must match block specification name ("&2")
-- on line &3 or be omitted.
10.002: Syntax( 10.002, END, name, loopLabel, loopLine )

-- END corresponding to block on line &2 must not have a symbol
-- following it because there is no LABEL or control variable; found "&1".
10.003: Syntax( 10.003, END, name, loopLine )

-- DO or LOOP instruction on line &1 requires matching END.
14.005: Syntax( 14.005, instruction~end, LOOP~from~word(1) )

-- Labels are not allowed within a DO/LOOP block; found "&1".
47.002: Syntax( 47.002, instruction~end, instruction~labels[1]~label )

-- EXPOSE must be the first instruction executed after a method invocation.
99.907: Syntax( 99.907, TheElementAfter(instruction~begin) )

/******************************************************************************/
/* SELECT INSTRUCTION                                                         */
/******************************************************************************/

::Method Select.Instruction
  Expose package

  Use Strict Arg selectClause

  SELECT               = TheElementAfter( selectClause~begin )
  selectLine           = SELECT~from~word(1)

  whenThenInstructions = Array()

  otherwiseClause      = .Nil

  otherwiseSequence    = Array()

  catchSections        = Array()

  whenClause = self~Next.Instruction.Fragment( .Array~of( 47.004 ) )

  Call NoLabels whenClause

  If  whenClause~isA( .End.Clause  ) Then Signal 7.001

  -- No WHEN before CATCH or FINALLY: as with OTHERWISE, 7.1 is at the END.
  If  whenClause~isA( .Catch.Clause ) | whenClause~isA( .Finally.Clause ) -
    Then Do
    next = whenClause
    Signal Catch
  End

  -- No WHEN before OTHERWISE: ooRexx parses it all, and 7.1 is at the END.
  If  whenClause~isA( .Otherwise.Clause ) Then Do
    next = whenClause
    Signal Otherwise
  End

  If \whenClause~isA( .When.Clause ) Then Call   7.002 whenClause

  WHEN = TheElementAfter( whenClause~begin )

  currentBody = package~currentBodies~top

  Loop

    thenClause = self~next.Instruction.Fragment( -
      .Array~of( 18.002, WHEN~from~word(1) ) -
    )

    -- A label before THEN: the THEN clause is not there (ooRexx: 18.2).
    If \thenClause~isA(.Then.Clause) Then Signal 18.002
    If \thenClause~labels~isEmpty     Then Signal 18.002

    THEN     = TheElementAfter( thenClause~begin )
    thenLine = THEN~from~word(1)

    thenInstruction = self~Next.Instruction( , , .Array~of( 47.004 ), "THEN" )

    Call NoLabels thenInstruction

    Select Case thenInstruction~class
      When .Implicit.Exit.Instruction Then Signal 14.003
      When .Expose.Instruction Then Call 99.907 thenInstruction
      When .Use.Arg.Instruction, .Arg.Instruction
                                      Then currentBody~signature = .Nil
      When .Parse.Instruction Then If thenInstruction~variant == "ARG"
                                      Then currentBody~signature = .Nil
      Otherwise Nop
    End

    whenThenInstructions~append( (whenClause, thenClause, thenInstruction) )

    next = self~Next.Instruction.Fragment( .Array~of( 47.004 ) )

    Select Case next~class
      When .When.Clause               Then whenClause = next
      When .Otherwise.Clause          Then Signal Otherwise
      When .Catch.Clause, .Finally.Clause Then Signal Catch
      When .End.Clause                Then Do
        Call NoLabels next
        Signal End
      End
      When .Implicit.Exit.Instruction Then Signal 14.002
      Otherwise                            Call    7.002 next
    End

    Call NoLabels whenClause

    WHEN = TheElementAfter( whenClause~begin )

  End

Otherwise:

  ENDClauseIsOK   = 1
  otherwiseClause = next
  Call NoLabels otherwiseClause
  OTHERWISE       = TheElementAfter( otherwiseClause~begin )
  otherwiseLine   = OTHERWISE~from~word(1)

  last            = otherwiseClause

  Loop
    next = self~Next.Instruction( ENDClauseIsOK, , .Array~of( 47.004 ) )
    Call NoLabels next
    Select Case next~class
      When .End.Clause Then Signal End
      When .Catch.Clause, .Finally.Clause Then Signal Catch
      When .Implicit.Exit.Instruction Then Signal 14.901
      When .Expose.Instruction        Then Call 99.907 next
      When .Use.Arg.Instruction, .Arg.Instruction
                                      Then currentBody~signature = .Nil
      When .Parse.Instruction Then If next~variant == "ARG"
                                      Then currentBody~signature = .Nil
      Otherwise Nop
    End
    otherwiseSequence~append( next )
    last = next
  End

Catch:
  sections      = self~Catch.Sections( selectClause, next )
  catchSections = sections[1]
  next          = sections[2]

End:
  END = TheElementAfter( next~begin )
  name = next~name
  If name \== "" Then Do
    selectLabel = selectClause~name
    If selectLabel == .Nil  Then Signal 10.007
    If name \== selectLabel~value Then Signal 10.004
  End
  If whenThenInstructions~isEmpty Then Signal 7.001

  Return .Select.Instruction~new( -
    package,                      -
    selectClause,                 -
    whenThenInstructions,         -
    otherwiseClause,              -
    otherwiseSequence,            -
    next,                         -
    catchSections                 -
  )

NoLabels:
  If \Arg(1)~labels~isEmpty Then Call 47.004 Arg(1)
  Return

-- SELECT on line &1 requires WHEN.
 7.001: Syntax(  7.001, SELECT, selectLine )

-- SELECT on line &1 requires WHEN, OTHERWISE, or END.
 7.002: Syntax(  7.002, TheElementAfter( Arg(1)~begin ), selectLine )

-- Symbol following END ("&1") must match LABEL of SELECT specification
-- ("&2") on line &3 or be omitted.
10.004: Syntax( 10.004, END, name, selectLabel, selectLine )

-- END corresponding to SELECT on line &2 must not have a symbol
-- following it because there is no LABEL; found "&1".
10.007: Syntax( 10.007, END, name, selectLine )

-- SELECT instruction on line &1 requires matching END.
14.002: Syntax( 14.002, next~end , selectLine )

-- THEN on line &1 must be followed by an instruction.
14.003: Syntax( 14.003, THEN, WHEN~from~word(1)  )

-- OTHERWISE on line &1 requires matching END.
14.901: Syntax( 14.901, TheElementAfter(next~begin), otherwiseLine )

-- WHEN instruction on line &1 requires matching THEN clause.
18.002: Syntax( 18.002, TheElementAfter(thenClause~begin), WHEN~from~word(1) )

-- Labels are not allowed within a SELECT block; found "&1".
47.004: Syntax( 47.004, Arg(1)~end, Arg(1)~labels[1]~label )

-- EXPOSE must be the first instruction executed after a method invocation.
99.907: Syntax( 99.907, TheElementAfter(Arg(1)~begin) )

/******************************************************************************/
/* CATCH SECTIONS (RFE #31)                                                   */
/*                                                                            */
/*   The CATCH and FINALLY clauses of a DO, LOOP or SELECT, with their code,  */
/*   up to the END of the block. "clause" is the first CATCH or FINALLY       */
/*   clause, already parsed. Returns an array: the sections (an array of      */
/*   (clause, instructions) pairs, in source order) and the END clause.       */
/*                                                                            */
/*   While the code of a clause is parsed, the clause is on the block         */
/*   instructions stack, just above its block: LEAVE and ITERATE see that     */
/*   the block has ended (28.6, 28.7) or that they would leave a FINALLY      */
/*   clause (99.951), and so do RETURN, EXIT, SIGNAL and FORWARD (99.951).    */
/*   The END clause pops the clause and the block.                            */
/******************************************************************************/

::Method Catch.Sections
  Expose package

  Use Strict Arg blockClause, clause

  BLOCK     = TheElementAfter( blockClause~begin )
  blockLine = BLOCK~from~word(1)

  sections    = Array()
  stack       = package~blockInstructionsStacks~top
  currentBody = package~currentBodies~top
  finally     = .Nil

  Loop
    -- Nothing but the END follows the FINALLY clause of a block.
    If finally \== .Nil Then Signal 10.015

    clause~owner = blockClause
    If clause~isA( .Finally.Clause ) Then Do
      finally = clause
      -- A FORWARD without CONTINUE ends the method at once, and a FINALLY
      -- clause needs it to go on: it must not be in a block that has one,
      -- at any depth (ooRexx r13535). Blocks nest, so the last FORWARD of
      -- the body, when it comes after the start of this block, is inside it.
      lastForward = currentBody~local~lastForward
      If lastForward \== .Nil Then Do
        Parse Value lastForward~from With fLine fCol
        Parse Value BLOCK~from       With bLine bCol
        If fLine > bLine | ( fLine == bLine & fCol > bCol ) Then Signal 99.952
      End
    End

    stack~push( clause )

    instructions = Array()

    Loop
      -- In the code of a CATCH or FINALLY clause, a label is 47.2,
      -- whatever the block, as in ooRexx.
      instruction = self~Next.Instruction( 1, , .Array~of( 47.002 ) )
      If \instruction~labels~isEmpty Then Signal 47.002
      Select Case instruction~class
        When .End.Clause                    Then Leave
        When .Catch.Clause, .Finally.Clause Then Leave
        When .Expose.Instruction            Then Signal 99.907
        When .Implicit.Exit.Instruction     Then Signal Incomplete
        When .Use.Arg.Instruction, .Arg.Instruction
                                            Then currentBody~signature = .Nil
        When .Parse.Instruction Then If instruction~variant == "ARG"
                                            Then currentBody~signature = .Nil
        Otherwise Nop
      End
      instructions~append( instruction )
    End

    sections~append( ( clause, instructions ) )

    -- The END clause has already popped the clause and the block.
    If instruction~isA( .End.Clause ) Then Return ( sections, instruction )

    stack~pop
    clause = instruction
  End

-- The error is the one of the block that owns the clause.
Incomplete:
  Select
    When blockClause~isA( .Select.Clause    ) Then Signal 14.002
    When blockClause~isA( .Simple.Do.Clause ) Then Signal 14.001
    Otherwise                                      Signal 14.005
  End

-- A CATCH or FINALLY clause must not follow the FINALLY clause on line &1.
10.015: Syntax( 10.015, TheElementAfter( clause~begin ), -
  TheElementAfter( finally~begin )~from~word(1) )

-- DO instruction on line &1 requires matching END.
14.001: Syntax( 14.001, instruction~end, blockLine )

-- SELECT instruction on line &1 requires matching END.
14.002: Syntax( 14.002, instruction~end, blockLine )

-- DO or LOOP instruction on line &1 requires matching END.
14.005: Syntax( 14.005, instruction~end, blockLine )

-- Labels are not allowed within a DO/LOOP block; found "&1".
47.002: Syntax( 47.002, instruction~end, instruction~labels[1]~label )

-- EXPOSE must be the first instruction executed after a method invocation.
99.907: Syntax( 99.907, TheElementAfter(instruction~begin) )

-- FORWARD without CONTINUE on line &1 must not be in a block with a FINALLY
-- clause.
99.952: Syntax( 99.952, TheElementAfter( clause~begin ), fLine )

/******************************************************************************/
/******************************************************************************/
/* Clause processing                                                          */
/*                                                                            */
/*   See Directives.cls for directive clauses and KeywordInstructions.cls     */
/*   for simple (i.e., non-block) instruction clauses.                        */
/*                                                                            */
/*   This package section contains code to process                            */
/*     Assignments (classical, i.e., non message)                             */
/*     CATCH clauses (RFE #31)                                                */
/*     Commands                                                               */
/*     DO clauses (not the whole instruction!)                                */
/*     ELSE clauses                                                           */
/*     END clauses                                                            */
/*     FINALLY clauses (RFE #31)                                              */
/*     "}" (Executor only: source literal end markers)                        */
/*     The END-OF-SOURCE clause                                               */
/*     IF clauses (not the whole instruction!)                                */
/*     The implicit EXIT clause, generated automatically before each          */
/*       directive and before end of source                                   */
/*     Iterative clauses (code common to DO and LOOP)                         */
/*     Labels                                                                 */
/*     LOOP clauses (not the whole instruction!)                              */
/*     Message clauses (including message assignments)                        */
/*     Null clauses                                                           */
/*     THEN clauses                                                           */
/*     OTHERWISE clauses                                                      */
/*     SELECT clauses                                                         */
/*     WHEN clauses                                                           */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* ASSIGNMENT instruction                                                     */
/******************************************************************************/

::Routine Assignment.Instruction Public
  Use Strict Arg package, begin, end, elements

  variable = elements[1]

  -- A constant symbol cannot be assigned to: 31.3 (a period first) or 31.2
  -- (a digit first). ooRexx and ANSI flag it at translation time, as the
  -- parser already does for DO, DROP, USE ARG and USE LOCAL.
  If variable < .ALL.CONST_SYMBOLS | variable < .ALL.NUMBERS Then Do
    c = Left( variable~value, 1 )
    If c == "." Then Signal 31.003
    Signal 31.002
  End

  variable~setAssigned

  thisBody          = package~currentBodies~top
  locals            = thisBody~locals

  varName           = variable~value
  p                 = Pos(".",varName)
  If p == 0 | p == Length(varName) Then
    If \locals~hasIndex( varName ) Then
      locals[varName] = variable~source

  symbol = Symbol.Term( package, variable )

  operator = elements[2]

  If operator < .EL.OP.EQUAL Then
    Call SetCategory operator, .EL.ASG.EQUAL

  element = TheElementAfter( operator ) -- Start parsing here...

  Call PrepareExpression package, element

  expression = Expression.List( package, element, .EL.END_OF_CLAUSE )

  -- Allow empty assignments when the "emptyassignments" option
  -- has been specified with a value of "1".
  If expression~isEmpty Then Do
    If .Features.Empty.Assignments Then Nop
    Else Signal 35.918
  End

  -- In this context, an expression list is an array term.
  If expression~isA(.Expression.List) Then
    expression = .Array.Term~new( expression )

  Return .Assignment.Instruction~new(                 -
    package, begin, end, symbol, operator, expression -
  )

-- Missing expression following assignment instruction.
35.918: Syntax( 35.918, elements[1] )

-- Variable symbol must not start with a number; found "&1".
31.002: Syntax( 31.002, variable, variable )

-- Variable symbol must not start with a "."; found "&1".
31.003: Syntax( 31.003, variable, variable )

--------------------------------------------------------------------------------

::Class Assignment.Instruction Public SubClass Rexx.Instruction
::Attribute symbol     Get
::Attribute operator   Get
::Attribute expression Get
::Method init
  Expose symbol operator expression
  Use Strict Arg package, begin, end, symbol, operator, expression
  self~init:super( package, begin, end )

/******************************************************************************/
/* COMMAND                                                                    */
/******************************************************************************/

::Routine Command Public
  Use Strict Arg package, begin, end, elements

  ignoreBlanks = .True

  element = TheElementAfter( begin )

  -- The command expression has been prepared by the preclauser

  executorCommand = Copies(executorCommand,.Options.Executor)

  expression = Expression.List( package, element,, executorCommand)

  Return .Command.Instruction~new( package, begin, end, expression )

--------------------------------------------------------------------------------

::Class Command.Instruction Public SubClass Rexx.Instruction
::Attribute expression Get
::Method init
  Expose expression
  Use Strict Arg package, begin, end, expression
  self~init:super( package, begin, end )

/******************************************************************************/
/* DO Clause                                                                  */
/******************************************************************************/

::Routine Do.Clause Public
  Use Strict Arg package, begin, end, elements
  Return Iterative.Clause( package, .Do.Clause, begin, end, elements )

--------------------------------------------------------------------------------

::Class Do.Clause Public SubClass Iterative.Clause
::Constant InstructionName "DO"

--------------------------------------------------------------------------------

::Class           Do.Forever.Clause Public SubClass Do.Clause
::Class Simple.Repetitive.Do.Clause Public SubClass Do.Clause
::Class              Do.Over.Clause Public SubClass Do.Clause
::Class              Do.With.Clause Public SubClass Do.Clause
::Class             Do.While.Clause Public SubClass Do.Clause
::Class             Do.Until.Clause Public SubClass Do.Clause
::Class        Do.Controlled.Clause Public SubClass Do.Clause
::Class            Simple.Do.Clause Public SubClass Do.Clause

/******************************************************************************/
/* ELSE CLAUSE                                                                */
/******************************************************************************/

::Routine Else.Clause Public
  Use Strict Arg package, begin, end, elements

  Return .Else.Clause~new( package, begin, end )

--------------------------------------------------------------------------------

::Class Else.Clause Public SubClass Rexx.Clause

/******************************************************************************/
/* END Clause                                                                 */
/******************************************************************************/

::Routine End.Clause Public

  Use Strict Arg package, begin, end, elements

  ENDKeyword = elements[1]

  name         = TheElementAfter( ENDKeyword )
  options.     = .False
  ignoreBlanks = .True

  end = name
  If name < .EL.END_OF_CLAUSE Then Signal Done

  If name \< .ALL.SYMBOLS Then Signal 20.909
  Call SetConstantName name, .BLOCK.INSTRUCTION.NAME

  options.["NAME"] = name~value

  element = TheElementAfter( name, ignoreBlanks )

  end = element
  If element \< .EL.END_OF_CLAUSE Then Signal 21.909

Done:
  blockInstructionsStack = package~blockInstructionsStacks~top
  -- The END of a block with CATCH or FINALLY clauses: the last clause is
  -- on the stack, above its block (RFE #31).
  If blockInstructionsStack~items > 0 Then Do
    top = blockInstructionsStack~top
    If top~isA( .Catch.Clause ) | top~isA( .Finally.Clause ) Then
      blockInstructionsStack~pop
  End
  -- No block: Next.Instruction raises the error, which is 10.5 or 10.6
  -- after THEN or ELSE (bug #2024), and 10.1 otherwise.
  If blockInstructionsStack~items > 0 Then blockInstructionsStack~pop
  Return .End.Clause~new( package, begin, end, options. )

-- Symbol expected after END keyword.
20.909: Syntax( 20.909, ENDKeyword )

-- Data must not follow the END name; found "&1".
21.909: Syntax( 21.909, ENDKeyword, element )

--------------------------------------------------------------------------------

::Class End.Clause Public SubClass Rexx.Clause
::Attribute name Get
::Method init
  Expose name options.
  Use Strict Arg package, begin, end, options.
  name = ""
  If options.~hasIndex("NAME") Then name = options.["NAME"]
  self~init:super( package, begin, end )

/******************************************************************************/
/* END OF SOURCE                                                              */
/******************************************************************************/

::Routine End.Of.Source Public
  Use Strict Arg package, begin, end, elements

  Return .End.Of.Source~new( package, begin, end )

--------------------------------------------------------------------------------

::Class End.Of.Source Public SubClass Null.Clause

/******************************************************************************/
/* IF Clause                                                                  */
/******************************************************************************/

::Routine If.Clause Public

  Use Strict Arg package, begin, end, elements

  IF = elements[1]

  element = TheElementAfter( IF )       -- Start parsing here...
  terminators  = .EL.END_OF_CLAUSE      -- ";" terminates parsing

  -- The expression has already been prepared by the preparser

  expressions = Expression.List( package, element, terminators, "logical" )

  end = expressions~end

  Return .If.Clause~new( package, begin, end, expressions )

--------------------------------------------------------------------------------

::Class If.Clause Public SubClass Rexx.Clause
::Attribute expressions
::Method init
  Expose expressions
  Use Strict Arg package, begin, end, expressions
  self~init:super( package, begin, end )

/******************************************************************************/
/* IMPLICIT EXIT INSTRUCTION                                                  */
/******************************************************************************/

::Routine Implicit.Exit.Instruction Public
  Use Strict Arg package, begin, end, elements

  Return .Implicit.Exit.Instruction~new( package, begin, end )

--------------------------------------------------------------------------------

::Class Implicit.Exit.Instruction Public SubClass Rexx.Instruction
::Method init
  Use Strict Arg package, begin, end
  self~init:super( package, begin, end )
  self~terminating = .True

/******************************************************************************/
/* ITERATIVE.CLAUSE                                                           */
/*   Code common to the DO and LOOP clauses                                   */
/******************************************************************************/

::Routine Iterative.Clause

  Use Strict Arg package, class, begin, end, elements

  keyword = elements[1]

  ignoreBlanks  = .True
  options.      = .False
  conditional   = 0
  options.order = ""  -- Of TO, BY and FOR

  body          = package~currentBodies~top
  blockID       = body~blockID + 1
  body~blockID  = blockID
  options.id    = blockID

  element   = TheElementAfter( keyword )
  If element < .EL.END_OF_CLAUSE Then Signal Done

  next    = Lookahead( element )

  label   = \ .Features.Do.Label
  counter = \ .Features.Do.Counter

Continue:
  If element < .EL.END_OF_CLAUSE Then Signal Done
  If element < .ALL.SYMBOLS Then Do
    tValue = element~value
    If next < .EL.OP.EQUAL           Then Signal "Classic Do/Loop"
    -- "symbol ==" (LABEL and COUNTER too): any equal sign makes it the
    -- controlled form, and "==" is 35.1, as in ooRexx.
    If next < .EL.OP.STRICT.EQUAL    Then Signal 35.001
    If \label,   tValue == "LABEL"   Then Signal "Label option"
    If \counter, tValue == "COUNTER" Then Signal "Counter option"
    If next < .ALL.SYMBOLS Then Do
      Select Case next~value
        When "OVER"                  Then
          If .Features.Do.Over       Then     Signal "Do/Loop ... Over"
        When "ITEM", "INDEX"         Then
          If .Features.Do.With, tValue == "WITH"
                                     Then Signal "Do/Loop With ...  Over"
        Otherwise Nop
      End
    End
    Select Case tValue
      When "FOREVER"                 Then Signal "Do/Loop Forever"
      When "WHILE"                   Then Signal "Do/Loop While"
      When "UNTIL"                   Then Signal "Do/Loop Until"
      Otherwise Nop
    End
  End

  Signal                                         "Simple Repetitive Do/Loop"

"Classic Do/Loop":
  Call ControlVariable
  Call SetCategory next, .EL.ASG.EQUAL
  element = TheElementAfter( next, ignoreBlanks ) -- Consume "="
  Call PrepareExpression package, element, "TO BY FOR WHILE UNTIL"
  expression = Expression.List( package, element )
  If expression~isEmpty Then Signal 35.904
  options.["="] = expression
  seen. = 0
  Loop
    element = expression~end
    If element < .EL.END_OF_CLAUSE Then Signal Done
    tValue = element~value
    Select Case tValue
      When "WHILE" Then Signal "Do/Loop While"
      When "UNTIL" Then Signal "Do/Loop Until"
      Otherwise Do
        If seen.tValue Then Call 27.902 tValue
        seen.tValue = 1
        options.order ||= " "tValue
        Call SetSubkeyword element
        element = TheElementAfter( element )
        Call PrepareExpression package, element, "TO BY FOR WHILE UNTIL"
        expression = Expression.List( package, element )
        element = expression~end
        If expression~isEmpty Then Select Case tValue
          When "BY"  Then Signal 35.905
          When "TO"  Then Signal 35.906
          When "FOR" Then Signal 35.907
        End
        options.tValue = expression
      End
    End
  End
  Signal Conditional

"Do/Loop ... Over":
  Call SetSubkeyword next
  Call ControlVariable
  element = TheElementAfter( next, ignoreBlanks ) -- Consume "OVER"
  Call PrepareExpression package, element, "FOR WHILE UNTIL"
  expression = Expression.List( package, element )
  If expression~isEmpty Then Signal 35.911
  options.collection = expression
  for = 0
  Loop
    element = expression~end
    If element < .EL.END_OF_CLAUSE Then Signal Done
    tValue = element~value
    Select Case tValue
      When "WHILE" Then Signal "Do/Loop While"
      When "UNTIL" Then Signal "Do/Loop Until"
      Otherwise Do
        If for Then Call 27.902 "FOR"
        for = 1
        Call SetSubkeyword element
        element = TheElementAfter( element )
        Call PrepareExpression package, element, "FOR WHILE UNTIL"
        expression = Expression.List( package, element )
        element = expression~end
        If expression~isEmpty Then Signal 35.907
        options.["FOR"] = expression
      End
    End
  End

"Simple Repetitive Do/Loop":
  -- The symbol the look ahead passed is the start of the expression: the
  -- blanks after it are not ignorable (they are concatenations).
  Do blank Over lookaheadBlanks
    blank~ignorable = 0
  End
  Call PrepareExpression package, element, "WHILE UNTIL"
  expression = Expression.List( package, element )
  options.exprr = expression
  element = expression~end
  Signal Conditional

-- The element after Arg(1), blanks ignored. TheElementAfter marks those
-- blanks ignorable: the ones it marks are kept in lookaheadBlanks.
Lookahead:
  lookaheadBlanks = .Array~new
  lookaheadNext = Arg(1)~next
  Do While lookaheadNext~ignorable | lookaheadNext < .ALL.WHITESPACE_LIKE
    If \ lookaheadNext~ignorable Then lookaheadBlanks~append( lookaheadNext )
    lookaheadNext = lookaheadNext~next
  End
  Return TheElementAfter( Arg(1), ignoreBlanks )

ControlVariable:
  c = Left( element~value, 1 )
  If c == "."             Then Call 31.003 element
  If c >>= "0", c <<= "9" Then Call 31.002 element
  options.control = element
  element~setAssigned
  Return

Conditional:
  If element < .EL.END_OF_CLAUSE Then Signal Done

  If Keyword(element, "WHILE"    ) Then Signal "Do/Loop While"
  If Keyword(element, "UNTIL"    ) Then Signal "Do/Loop Until"

"Do/Loop While":
  If conditional Then Signal 27.001
  Call SetSubkeyword element
  element = TheElementAfter( element )
  Call PrepareExpression package, element, "WHILE UNTIL"
  expression = Expression.List( package, element, "", "logical" )
  options.while = expression
  element = expression~end
  conditional = 1
  Signal Conditional

"Do/Loop Until":
  If conditional Then Signal 27.001
  Call SetSubkeyword element
  element = TheElementAfter( element )
  Call PrepareExpression package, element, "WHILE UNTIL"
  expression = Expression.List( package, element, "", "logical" )
  options.until = expression
  element = expression~end
  conditional = 1
  Signal Conditional

Done:
  If class == .Do.Clause, options.~hasIndex("COUNTER") Then Do
    -- "ORDER" and "ID" are always there; "COUNTER", we just checked
    If options.~items == 3 Then Signal 27.905
    If options.~items == 4, options.~hasIndex("LABEL") Then Signal 27.905
  End
  If class == .Do.Clause Then Do
    If      options.~hasIndex("ITEM") | options.~hasIndex("INDEX") Then
      class = .Do.With.Clause
    Else If options.~hasIndex("COLLECTION") Then class = .Do.Over.Clause
    Else If options.~hasIndex("CONTROL")    Then class = .Do.Controlled.Clause
    Else If options.~hasIndex("FOREVER")    Then Do
      If options.~hasIndex("WHILE")         Then class = .Do.While.Clause
      Else If options.~hasIndex("UNTIL")    Then class = .Do.Until.Clause
      Else                                       class = .Do.Forever.Clause
    End
    Else If options.~hasIndex("EXPRR") Then
      class = .Simple.Repetitive.Do.Clause
    Else If options.~hasIndex("WHILE")      Then class = .Do.While.Clause
    Else If options.~hasIndex("UNTIL")      Then class = .Do.Until.Clause
    Else class = .Simple.Do.Clause
  End
  Else Do
    If      options.~hasIndex("ITEM") | options.~hasIndex("INDEX") Then
      class = .Loop.With.Clause
    Else If options.~hasIndex("COLLECTION") Then class = .Loop.Over.Clause
    Else If options.~hasIndex("CONTROL")    Then class = .Loop.Controlled.Clause
    Else If options.~hasIndex("FOREVER")    Then Do
      If options.~hasIndex("WHILE")         Then class = .Loop.While.Clause
      Else If options.~hasIndex("UNTIL")    Then class = .Loop.Until.Clause
      Else                                       class = .Loop.Forever.Clause
    End
    Else If options.~hasIndex("EXPRR") Then
      class = .Simple.Repetitive.Loop.Clause
    Else If options.~hasIndex("WHILE")      Then class = .Loop.While.Clause
    Else If options.~hasIndex("UNTIL")      Then class = .Loop.Until.Clause
    Else class = .Simple.Loop.Clause
  End
  iteration = class~new( package, begin, element, options. )
  package~blockInstructionsStacks~top~push( iteration )
  Return iteration

Keyword:
  If Arg(1) \< .ALL.VARIABLES_AND_KEYWORDS Then Return .False
  If Arg(1)~value \== Arg(2)   Then Return .False
  Call SetSubkeyword Arg(1)
  Return .True

"Label option":
  label = 1
  Call SetSubkeyword element
  labelName = next
  If labelName \< .ALL.SYMBOLS Then Signal 20.918
  Call SetConstantName labelName, .BLOCK.INSTRUCTION.NAME
  options.["LABEL"] = labelName
  element = TheElementAfter( labelName, ignoreBlanks )
  If element < .EL.END_OF_CLAUSE Then Signal Continue
  next  = Lookahead( element )
  Signal Continue

"Counter option":
  counter = 1
  Call SetSubkeyword element
  counterVar = next
  If counterVar \< .ALL.SYMBOLS Then Signal 20.934
  c = Left( counterVar~value, 1 )
  If c == "."             Then Call 31.003 counterVar
  If c >>= "0", c <<= "9" Then Call 31.002 counterVar
  counterVar~setAssigned
  options.["COUNTER"] = counterVar
  element = TheElementAfter( counterVar, ignoreBlanks )
  If element < .EL.END_OF_CLAUSE Then Signal Continue
  next  = Lookahead( element )
  Signal Continue

"Do/Loop Forever":
  Call SetSubkeyword element
  options.forever = 1
  element = next
  If element  < .EL.END_OF_CLAUSE Then Signal Done
  If element \< .ALL.VAR_SYMBOLS                    Then Signal 27.901
  If WordPos( element~value, "WHILE UNTIL" ) == 0   Then Signal 27.901
  Signal Conditional

"Do/Loop With ...  Over":
  Call SetSubkeyword element
  Call SetSubkeyword next
  element = next
  index = 0
  item  = 0
  for   = 0
  Loop
    If element \< .ALL.VARIABLES_AND_KEYWORDS              Then Signal 27.904
    tValue = element~value
    If WordPos( tValue, "ITEM INDEX OVER" ) == 0  Then Signal 27.904
    Call SetSubkeyword element
  If tvalue == "OVER" Then Leave
    If tValue == "ITEM" Then Do
      If item Then Call 27.902 tValue
      item = 1
    End
    Else Do
      If index Then Call 27.902 tValue
      index = 1
    End
    element = TheElementAfter( element  )
    If element \< .ALL.SYMBOLS Then Call 20.929 tValue
    c = Left( element~value, 1 )
    If c == "."             Then Call 31.003 element
    If c >>= "0", c <<= "9" Then Call 31.002 element
    element~setAssigned
    options.tValue = element
    element = TheElementAfter( element, ignoreBlanks )
  End
  element = TheElementAfter( element )
  Call PrepareExpression package, element, "FOR WHILE UNTIL"
  expression = Expression.List( package, element )
  If expression~isEmpty Then Signal 35.911
  options.supplier = expression
  element = expression~end
  If element < .ALL.SYMBOLS_AND_KEYWORDS, element~value == "FOR" Then Do
    Call SetSubkeyword element
    element = TheElementAfter( element )
    Call PrepareExpression package, element, "FOR WHILE UNTIL"
    expression = Expression.List( package, element )
    If expression~isEmpty Then Signal 35.907
    options.["FOR"] = expression
    element = expression~end
    If element < .ALL.SYMBOLS_AND_KEYWORDS, element~value == "FOR" Then
      Call 27.902 element
  End
  Signal Conditional

-- Symbol expected after LABEL keyword.
20.918: Syntax( 20.918, keyword )

-- Symbol expected after &1 keyword.
20.929: Syntax( 20.929, keyword, Arg(1) )

-- Symbol expected after COUNTER keyword.
20.934: Syntax( 20.934, keyword )

-- Only one WHILE or UNTIL condition can be used on the same loop.
27.001: Syntax( 27.001, keyword, element )

-- Incorrect data following FOREVER keyword on the loop; found "&1".
27.901: Syntax( 27.901, keyword, element )

-- DO or LOOP keyword &1 can be specified only once.
27.902: Syntax( 27.902, keyword, Arg(1) )

-- OVER keyword expected for a WITH loop.
27.904: Syntax( 27.904, keyword )

-- COUNTER keyword not allowed on a simple DO instruction.
27.905: Syntax( 27.905, keyword )

-- Variable symbol must not start with a number; found "&1".
31.002: Syntax( 31.002, keyword, Arg(1) )

-- Variable symbol must not start with a "."; found "&1".
31.003: Syntax( 31.003, keyword, Arg(1) )

-- Missing initial expression for DO or LOOP control variable.
35.904: Syntax( 35.904, keyword )

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, next, next )

-- Missing expression following BY keyword.
35.905: Syntax( 35.905, keyword )

-- Missing expression following TO keyword.
35.906: Syntax( 35.906, keyword )

-- Missing expression following FOR keyword.
35.907: Syntax( 35.907, keyword )

-- Missing expression following OVER keyword.
35.911: Syntax( 35.911, keyword )

--------------------------------------------------------------------------------

::Class Iterative.Clause Public SubClass Rexx.Clause
::Attribute name       Get
::Attribute ctr        Get
::Attribute exprr      Get
::Attribute control    Get
::Attribute forever    Get
::Attribute exprw      Get
::Attribute expru      Get
::Attribute collection Get
::Attribute supplier   Get
::Attribute index      Get
::Attribute item       Get
::Attribute expri      Get
::Attribute exprt      Get
::Attribute exprb      Get
::Attribute exprf      Get
::Attribute order      Get -- Of TO, BY and FOR
::Attribute id         Get
::Method init
  Expose name ctr expri exprt exprb exprf exprr order control forever -
    exprw expru type collection supplier item index id
  Use Strict Arg package, begin, end, options.

  -- ID and ORDER are always calculated
  id         = options.id
  order      = Strip(options.order)

  name       = .Nil                     -- LABEL name
  ctr        = .Nil                     -- COUNTER ctr
  control    = .Nil
  expri      = .Nil                     -- expression after "="
  exprt      = .Nil                     -- expression after "TO"
  exprb      = .Nil                     -- expression after "BY"
  exprf      = .Nil                     -- expression after "FOR"
  exprw      = .Nil                     -- expression after "WHILE"
  expru      = .Nil                     -- expression after "UNTIL"
  collection = .Nil
  supplier   = .Nil
  item       = .Nil
  index      = .Nil
  exprr      = .Nil
  forever    = 0

  If HasOption("label")      Then name       = Option("label")
  If HasOption("counter")    Then ctr        = Option("counter")
  If HasOption("control")    Then control    = Option("control")
  If HasOption("=")          Then expri      = Option("=")
  If HasOption("to")         Then exprt      = Option("to")
  If HasOption("by")         Then exprb      = Option("by")
  If HasOption("for")        Then exprf      = Option("for")
  If HasOption("while")      Then exprw      = Option("while")
  If HasOption("until")      Then expru      = Option("until")
  If HasOption("exprr")      Then exprr      = Option("exprr")
  If HasOption("collection") Then collection = Option("collection")
  If HasOption("supplier")   Then supplier   = Option("supplier")
  If HasOption("index")      Then index      = Option("index")
  If HasOption("item")       Then item       = Option("item")
  If HasOption("forever")    Then forever    = 1

  self~init:super( package, begin, end )
  Return

HasOption: Return options.~hasIndex(Upper(Arg(1)))
Option:    Return options.[         Upper(Arg(1))]

/******************************************************************************/
/* LABEL                                                                      */
/******************************************************************************/

::Routine Label.Clause Public
  Use Strict Arg package, begin, end, elements

  label = elements[1]

  Call SetConstantName label, .LABEL.NAME

  Return .Label.Clause~new( package, begin, end, label )

--------------------------------------------------------------------------------

::Class Label.Clause Public SubClass Rexx.Clause
::Attribute labelFor
::Attribute label
::Attribute traceOnly
::Method init
  Expose label labelFor traceOnly
  Use Strict Arg package, begin, end, label
  labelFor        = .Nil
  traceOnly       = .True
  label~traceOnly = .True
  self~init:super( package, begin, end )


/******************************************************************************/
/* LOOP Clause                                                                */
/******************************************************************************/

::Routine Loop.Clause Public
  Use Strict Arg package, begin, end, elements
  Return Iterative.Clause( package, .LOOP.Clause, begin, end, elements )

--------------------------------------------------------------------------------

::Class Loop.Clause Public SubClass Iterative.Clause
::Constant InstructionName "LOOP"

--------------------------------------------------------------------------------

::Class           Loop.Forever.Clause Public SubClass Loop.Clause
::Class Simple.Repetitive.Loop.Clause Public SubClass Loop.Clause
::Class              Loop.Over.Clause Public SubClass Loop.Clause
::Class              Loop.With.Clause Public SubClass Loop.Clause
::Class            Simple.Loop.Clause Public SubClass Loop.Clause
::Class             Loop.While.Clause Public SubClass Loop.Clause
::Class             Loop.Until.Clause Public SubClass Loop.Clause
::Class        Loop.Controlled.Clause Public SubClass Loop.Clause

/******************************************************************************/
/* MESSAGE                                                                    */
/******************************************************************************/

::Routine Message Public
  Use Strict Arg package, begin, end, elements
  ignoreBlanks = .True

  element = TheElementAfter( begin )

  -- The expression has already been prepared by the preparser

  expression = Expression.List( package, element )

  element = expression~end

  If element \< .ALL.ASSIGNMENTS Then Signal Message.Instruction

MessageAssignment:
  If element < .EL.OP.EQUAL Then
    Call SetCategory element, .EL.ASG.EQUAL
  lhs      = expression
  operator = element
  element  = TheElementAfter( operator )
  If element < .EL.END_OF_CLAUSE Then Signal 35.001
  rhs = Expression.List( package, element )
  -- In this context, an expression list is an array term.
  If rhs~isA(.Expression.List) Then rhs = .Array.Term~new( rhs )
  Return .Message.Assignment.Instruction~new( -
    package, begin, end, lhs, operator, rhs   -
  )

Message.Instruction:
  Return .Message.Instruction~new( package, begin, end, expression )

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, operator, operator )

--------------------------------------------------------------------------------

::Class Message.Instruction Public SubClass Rexx.Instruction
::Attribute expression
::Method init
  Expose expression
  Use Strict Arg package, begin, end, expression
  self~init:super( package, begin, end)

--------------------------------------------------------------------------------

::Class Message.Assignment.Instruction Public SubClass Rexx.Instruction
::Attribute lhs      Get
::Attribute operator Get
::Attribute rhs      Get
::Method init
  Expose lhs operator rhs
  Use Strict Arg package, begin, end, lhs, operator, rhs
  self~init:super( package, begin, end )

/******************************************************************************/
/* NULL CLAUSE                                                                */
/******************************************************************************/

::Routine Null.Clause Public
  Use Strict Arg package, begin, end, elements

  Return .Null.Clause~new( package, begin, end )

--------------------------------------------------------------------------------

::Class Null.Clause Public SubClass Rexx.Clause

/******************************************************************************/
/* THEN Clause                                                                */
/******************************************************************************/

::Routine Then.Clause Public
  Use Strict Arg package, begin, end, elements
  Return .Then.Clause~new( package, begin, end )

--------------------------------------------------------------------------------

::Class Then.Clause Public SubClass Rexx.Clause

/******************************************************************************/
/* OTHERWISE Clause                                                           */
/******************************************************************************/

::Routine Otherwise.Clause Public
  Use Strict Arg package, begin, end, elements

  OTHERWISE = elements[1]

  Call SetKeyword OTHERWISE

  Return .Otherwise.Clause~new( package, begin, end )

--------------------------------------------------------------------------------

::Class Otherwise.Clause Public SubClass Rexx.Clause

/******************************************************************************/
/* CATCH Clause (RFE #31)                                                     */
/*                                                                            */
/*   CATCH [variable =] condition [, condition]...                            */
/*                                                                            */
/*   condition is a condition name that SIGNAL ON accepts (USER name too).    */
/*   SYNTAX, ERROR and FAILURE may be followed by a code: for SYNTAX, an      */
/*   error number written as a number or a literal string, or a simple        */
/*   variable; for ERROR and FAILURE, a number (possibly signed), a literal   */
/*   string or a simple variable (the spec of 13/11/2026, after Rick McGuire  */
/*   on oorexx-devel). A variable is read when the clause is tested.          */
/*   The variable gets the condition object when the clause runs.             */
/*                                                                            */
/*   The errors, and their order, are those of LanguageParser::catchNew()     */
/*   in the RFE #31 patch (6be8c32), with the codes of the spec of 13/11.     */
/******************************************************************************/

::Routine Catch.Clause Public
  Use Strict Arg package, begin, end, elements

  CATCH = elements[1]

  Call SetKeyword CATCH

  ignoreBlanks = .True
  variable     = .Nil
  conditions   = Array()

  element = TheElementAfter( CATCH, ignoreBlanks )

  -- An optional "variable =", as in NetRexx's CATCH [variable =] exception.
  If element < .ALL.SYMBOLS Then Do
    second = TheElementAfter( element, ignoreBlanks )
    If second < .EL.OP.EQUAL Then Do
      variable = element
      If variable < .ALL.CONST_SYMBOLS | variable < .ALL.NUMBERS Then Do
        If Left( variable~value, 1 ) == "." Then Signal 31.003
        Signal 31.002
      End
      variable~setAssigned
      locals  = package~currentBodies~top~locals
      varName = variable~value
      p       = Pos( ".", varName )
      If p == 0 | p == Length( varName ) Then
        If \locals~hasIndex( varName ) Then
          locals[varName] = variable~source
      variable = Symbol.Term( package, variable )
      Call SetCategory second, .EL.ASG.EQUAL
      element = TheElementAfter( second, ignoreBlanks )
    End
  End

  -- A list of conditions, separated by commas
  Loop
    If element \< .ALL.SYMBOLS Then Signal 20.936

    name = element~value
    If WordPos( name, .Features.Signal.OnOff.Conditions ) == 0 Then
      Signal 25.939

    condition     = element
    userCondition = .Nil
    sign          = .Nil
    code          = .Nil

    Call SetSubkeyword condition

    element = TheElementAfter( condition, ignoreBlanks )

    Select
      -- USER conditions are "USER name", as for SIGNAL ON
      When name == "USER" Then Do
        If element \< .ALL.SYMBOLS Then Signal 20.915
        userCondition = element
        Call SetConstantName userCondition, .USER.CONDITION.NAME
        element = TheElementAfter( userCondition, ignoreBlanks )
      End
      -- SYNTAX code: an error number, written as a number or a literal
      -- string, or a simple variable, read when the clause is tested. No
      -- sign. A number, a literal or a symbol literal ("42.x") that is not
      -- an error number is 35.937; stems, compound variables, environment
      -- symbols, a period and a sign are 35.1, as for ERROR and FAILURE.
      -- "catch syntax notready" is SYNTAX with the variable NOTREADY as its
      -- code, as "catch error notready" is.
      When name == "SYNTAX",                                  -
        element \< .EL.END_OF_CLAUSE, element \< .EL.COMMA Then Do
        Select
          When element < .EL.SIMPLE_VARIABLE Then Nop
          When element < .ALL.NUMBERS        |                -
               element < .ALL.STRINGS        |                -
               element < .EL.SYMBOL_LITERAL  Then
            If \IsAnErrorNumber( element~value ) Then Signal 35.937
          Otherwise Signal 35.001
        End
        code    = element
        element = TheElementAfter( code, ignoreBlanks )
      End
      -- ERROR errorcode, FAILURE failurecode: a number, possibly signed
      -- (as on ::CONSTANT), a literal string, or a simple variable, read
      -- when the clause is tested. Stems, compound variables, environment
      -- symbols and constant symbols that are not numbers are 35.1, as is
      -- a sign followed by anything but a number.
      When WordPos( name, "ERROR FAILURE" ) > 0,              -
        element \< .EL.END_OF_CLAUSE, element \< .EL.COMMA Then Do
        If element < .ALL.OPS.ADDITIVE Then Do
          sign    = element
          If sign < .EL.OP.MINUS Then Call SetCategory sign, .EL.OP.PREFIX.MINUS
          Else                        Call SetCategory sign, .EL.OP.PREFIX.PLUS
          element = TheElementAfter( sign, ignoreBlanks )
          If element \< .ALL.NUMBERS Then Signal 35.001
        End
        Else If element \< .ALL.NUMBERS,       -
                element \< .ALL.STRINGS,       -
                element \< .EL.SIMPLE_VARIABLE Then Signal 35.001
        code    = element
        element = TheElementAfter( code, ignoreBlanks )
      End
      Otherwise Nop
    End

    conditions~append( -
      .Catch.Condition~new( condition, userCondition, sign, code ) -
    )

  If element < .EL.END_OF_CLAUSE Then Leave

    If element \< .EL.COMMA Then Signal 21.904

    element = TheElementAfter( element, ignoreBlanks )
  End

  Return .Catch.Clause~new( package, begin, end, variable, conditions )

-- Symbol expected after CATCH keyword or after a comma in a CATCH clause.
20.936: Syntax( 20.936, CATCH )

-- Symbol expected after USER keyword.
20.915: Syntax( 20.915, CATCH )

-- Data must not follow the condition name; found "&1".
21.904: Syntax( 21.904, element, element )

-- CATCH must be followed by one or more of the keywords ERROR, FAILURE,
-- HALT, LOSTDIGITS, NOTREADY, NOMETHOD, NOSTRING, NOVALUE, SYNTAX, USER,
-- or ANY, separated by commas; found "&1".
25.939: Syntax( 25.939, element, element )

-- Variable symbol must not start with a number; found "&1".
31.002: Syntax( 31.002, variable, variable )

-- Variable symbol must not start with a "."; found "&1".
31.003: Syntax( 31.003, variable, variable )

-- Incorrect expression detected at "&1".
35.001:
  If element < .EL.END_OF_CLAUSE Then Syntax( 35.001, element, ";" )
  Syntax( 35.001, element, element )

-- Incorrect error number following SYNTAX keyword of CATCH clause;
-- found "&1".
35.937: Syntax( 35.937, element, element )

--------------------------------------------------------------------------------
-- IsAnErrorNumber: an error number for CATCH SYNTAX, major or major.minor,  --
--   as LanguageParser::catchNew() checks it: one to three digits for the     --
--   major part (1 to 99), and one to four for the minor (1 to 999).          --
--------------------------------------------------------------------------------

::Routine IsAnErrorNumber Private
  Use Strict Arg text

  Parse Var text major "." minor
  If major == "" | Length( major ) > 3 | \DigitsOnly( major ) Then Return 0
  If major < 1 | major > 99                                   Then Return 0
  If Pos( ".", text ) == 0                                    Then Return 1
  If minor == "" | Length( minor ) > 4 | \DigitsOnly( minor ) Then Return 0
  If minor < 1 | minor > 999                                  Then Return 0
  Return 1

DigitsOnly: Return Verify( Arg(1), "0123456789" ) == 0

--------------------------------------------------------------------------------

::Class Catch.Clause Public SubClass Rexx.Clause
::Attribute variable   Get -- A Symbol.Term (the "variable" of "variable ="), or .Nil
::Attribute conditions Get -- An array of Catch.Condition objects
::Attribute owner          -- The DO, LOOP or SELECT clause of the block
::Method init
  Expose variable conditions
  Use Strict Arg package, begin, end, variable, conditions
  self~init:super( package, begin, end )

--------------------------------------------------------------------------------
-- One condition of a CATCH clause                                            --
--------------------------------------------------------------------------------

::Class Catch.Condition Public
::Attribute condition     Get -- The condition keyword element (SYNTAX, ANY...)
::Attribute userCondition Get -- The USER condition name element, or .Nil
::Attribute sign          Get -- The sign of an ERROR or FAILURE code, or .Nil
::Attribute code          Get -- The code element (SYNTAX, ERROR, FAILURE), or .Nil
::Method init
  Expose condition userCondition sign code
  Use Strict Arg condition, userCondition, sign, code

/******************************************************************************/
/* FINALLY Clause (RFE #31)                                                   */
/*                                                                            */
/*   As THEN, ELSE and OTHERWISE, FINALLY implies a semicolon after it        */
/*   (see PreClauser.cls).                                                    */
/******************************************************************************/

::Routine Finally.Clause Public
  Use Strict Arg package, begin, end, elements

  FINALLY = elements[1]

  Call SetKeyword FINALLY

  Return .Finally.Clause~new( package, begin, end )

--------------------------------------------------------------------------------

::Class Finally.Clause Public SubClass Rexx.Clause
::Attribute owner          -- The DO, LOOP or SELECT clause of the block

/******************************************************************************/
/* SELECT Clause                                                              */
/******************************************************************************/

::Routine Select.Clause Public

  Use Strict Arg package, begin, end, elements

  SELECT = elements[1]

  Call SetKeyword SELECT

  ignoreBlanks = .True
  options.     = .False

  body          = package~currentBodies~top
  blockID       = body~blockID + 1
  body~blockID  = blockID
  options.id    = blockID

  element = TheElementAfter( SELECT )

  If element  < .EL.END_OF_CLAUSE   Then Signal Done

  If \.Features.Select.Case, \.Features.Select.Label
                                    Then Call 21.902 element

  If element \< .ALL.VAR_SYMBOLS    Then Signal 25.923

  kValue = element~value
  If Pos(kValue, "LABEL CASE") == 0 Then Signal 25.923

  Call SetSubkeyword element

  If kValue == "LABEL" Then Do
    element = TheElementAfter( element )
    If element \< .ALL.SYMBOLS Then Signal 20.918
    Call SetConstantName element, .BLOCK.INSTRUCTION.NAME
    options.name = element
    element = TheElementAfter( element, ignoreBlanks )
  End

  If element < .EL.END_OF_CLAUSE Then Signal Done

  If element \< .ALL.VARIABLES_AND_KEYWORDS Then Signal 25.923

  If element~value \== "CASE"   Then Signal 25.923

  Call SetSubkeyword element

  element = TheElementAfter( element, ignoreBlanks )

  If element < .EL.END_OF_CLAUSE Then Signal 35.933

  Call PrepareExpression package, element
  options.case = Expression.List( package, element )
  element = end                         -- The clause ends after the expression

Done:
  selectClause = .Select.Clause~new( package, begin, element, options. )
  package~blockInstructionsStacks~top~push( selectClause )
  Return selectClause

-- Symbol expected after LABEL keyword.
20.918: Syntax( 20.918, SELECT )

-- Data must not follow the SELECT keyword; found "&1".
21.902: Syntax( 21.902, SELECT, Arg(1) )

-- SELECT must be followed by the keyword LABEL or CASE; found "&1".
25.923: Syntax( 25.923, SELECT, element )

-- Missing expression following CASE keyword of a SELECT instruction.
35.933: Syntax( 35.933, SELECT )

--------------------------------------------------------------------------------

::Class Select.Clause Public SubClass Rexx.Clause
::Attribute name Get
::Attribute case Get
::Attribute id   Get
::Method init
  Expose name case id
  Use Strict Arg package, begin, end, options.

  id   = options.id

  name = .Nil
  case = .Nil

  If options.~hasIndex("NAME") Then name = options.["NAME"]
  If options.~hasIndex("CASE") Then case = options.["CASE"]

  self~init:super( package, begin, end )

/******************************************************************************/
/* Source Literal End                                                         */
/******************************************************************************/

::Routine Source.Literal.End
  Use Strict Arg package, begin, end, elements

  Return .Source.Literal.End~new( package, begin, end )

--------------------------------------------------------------------------------

::Class Source.Literal.End Public SubClass Rexx.Clause
::Method init
  Use Strict Arg package, begin, end
  self~init:super( package, begin, end )

/******************************************************************************/
/* WHEN Clause                                                                */
/******************************************************************************/

::Routine When.Clause
  Use Strict Arg package, begin, end, elements

  WHEN = elements[1]

  Call SetKeyword WHEN

  element = TheElementAfter( WHEN )

  Call PrepareExpression package, element, "THEN"

  -- In a SELECT CASE, a missing expression is 35.934, not 35.929.
  options = "logical"
  blocks  = package~blockInstructionsStacks~top
  If blocks~items > 0, blocks~top~isA( .Select.Clause ) Then
    If blocks~top~case \== .Nil Then options = "logical case"

  expressions = Expression.List(package, element, .EL.END_OF_CLAUSE, options)

  Return .When.Clause~new( package, begin, end, expressions )

--------------------------------------------------------------------------------

::Class When.Clause Public SubClass Rexx.Clause
::Attribute expressions Get
::Method init
  Expose expressions
  Use Strict Arg package, begin, end, expressions
  self~init:super( package, begin, end )

/******************************************************************************/
/******************************************************************************/
/* Classes supporting block instructions                                      */
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Methods common to all block instructions                                   */
/******************************************************************************/

::Class Block.Instruction Public SubClass Rexx.Instruction
::Constant isBlockInstruction 1

-- The CATCH and FINALLY clauses of a DO, LOOP or SELECT (RFE #31): an array
-- of (clause, instructions) pairs, in source order, FINALLY last. Empty when
-- the block has none (and always for an IF).
::Method catchSections
  Expose catchSections
  If Var( "catchSections" ) Then Return catchSections
  Return Array()

::Method "CATCHSECTIONS="
  Expose catchSections
  Use Strict Arg catchSections

-- The FINALLY clause of the block, or .Nil
::Method finallyClause
  sections = self~catchSections
  If sections~isEmpty Then Return .Nil
  clause = sections~lastItem[1]
  If clause~isA( .Finally.Clause ) Then Return clause
  Return .Nil

/******************************************************************************/
/* An instruction list is an array with some additional methods               */
/******************************************************************************/

::Class Instruction.List Public SubClass Array
::Method begin
  Return self[1]~begin
::Method end
  Return self~lastItem~end

/******************************************************************************/
/* Methods common to Do.Instruction and Loop.Instruction                      */
/******************************************************************************/

::Class Iterative.Instruction Public SubClass Block.Instruction

/******************************************************************************/
/* DO Instruction                                                             */
/******************************************************************************/

::Class Do.Instruction Public SubClass Iterative.Instruction
::Attribute doClause
::Attribute instructions
::Attribute endClause
::Method init
  Expose                   doClause  instructions  endClause
  Use Strict Arg  package, doClause, instructions, endClause, -
    catchSections = ( Array() )
  self~catchSections = catchSections
  self~init:super(package, doClause~begin, endClause~end)
  Call AdoptChildren self, doClause, instructions, endClause, catchSections
::Method labels
  Expose doClause
  Return doClause~labels
::Method nullClauses
  Expose doClause
  Return doClause~nullClauses

/******************************************************************************/
/* IF Instruction                                                             */
/******************************************************************************/

::Class If.Instruction Public SubClass Block.Instruction
::Method labels
  Expose ifClause
  Return ifClause~labels
::Method nullClauses
  Expose ifClause
  Return ifClause~nullClauses
::Attribute ifClause        Get
::Attribute thenClause      Get
::Attribute thenInstruction Get
::Attribute elseClause      Get
::Attribute elseInstruction Get
::Method init
  Expose ifClause thenClause thenInstruction elseClause elseInstruction
  Use Strict Arg package,                      -
    ifClause,                                  -
    thenClause,        thenInstruction,        -
    elseClause = .Nil, elseInstruction = .Nil
  If elseInstruction~isNil Then
    self~init:super( package, ifClause~begin, thenInstruction~end )
  Else
    self~init:super( package, ifClause~begin, elseInstruction~end )
  Call AdoptChildren self, ifClause, thenClause, thenInstruction, -
    elseClause, elseInstruction

/******************************************************************************/
/* LOOP Instruction                                                           */
/******************************************************************************/

::Class Loop.Instruction Public SubClass Iterative.Instruction
::Attribute loopClause
::Method doClause -- Simplifies code
  Return self~loopClause
::Attribute instructions
::Attribute endClause
::Method init
  Expose loopClause instructions endClause
  Use Strict Arg package, loopClause, instructions, endClause, -
    catchSections = ( Array() )
  self~catchSections = catchSections
  self~init:super( package, loopClause~begin, endClause~end )
  Call AdoptChildren self, loopClause, instructions, endClause, catchSections
::Method labels
  Expose loopClause
  Return loopClause~labels
::Method nullClauses
  Expose loopClause
  Return loopClause~nullClauses

/******************************************************************************/
/* SELECT Instruction                                                         */
/******************************************************************************/

::Class Select.Instruction Public SubClass Block.Instruction

::Attribute SelectClause           Get
::Attribute   WhenThenInstructions Get
::Attribute   OtherwiseClause      Get
::Attribute     OtherwiseSequence  Get
::Attribute EndClause              Get

::Method init
  Expose                    -
    selectClause            -
      whenThenInstructions  -
      otherwiseClause       -
        otherwiseSequence   -
    endClause
  Use Strict Arg            -
    package,                -
    selectClause,           -
      whenThenInstructions, -
      otherwiseClause,      -
        otherwiseSequence,  -
    endClause,              -
    catchSections = ( Array() )

  self~catchSections = catchSections

  self~init:super(package, selectClause~begin, endClause~end )
  Call AdoptChildren self, selectClause, whenThenInstructions, -
    otherwiseClause, otherwiseSequence, endClause, catchSections

::Method labels
  Expose selectClause
  Return selectClause~labels
::Method nullClauses
  Expose selectClause
  Return selectClause~nullClauses
