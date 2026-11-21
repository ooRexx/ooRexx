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
/* PreClauser.cls                                                             */
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
/* 20241208    0.1a Accept options arg, pass it to scanner                    */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250531    0.2c Fix issue no. 13                                          */
/* 20251127    0.3a (Executor) Allow "var == expression;"                     */
/* 20251127         (Executor) Allow "keyword(..."                            */
/* 20251128         (Executor) Implement UPPER instruction                    */
/* 20251128         (Executor) Implement ::EXTENSION directive                */
/* 20251201         (Executor) Add support for source literals                */
/* 20251206         Use the new scanner                                       */
/* 20251218         Support message instructions starting with a keyword      */
/* 20251226    0.4a Add support for "=" and "==" at end of expression         */
/* 20260521    0.5  CMS: Ignore ooRexx-specific keyword instructions          */
/* 20260521         CMS: Allow UPPER instruction                              */
/* 20260601    0.6  Implement a fine-grained feature system                   */
/* 20260924         A shebang line is a null clause, not a command with an    */
/*                  empty expression (the shebang element stays in the        */
/*                  element chain: highlighting and trident are unchanged).   */
/* 20260924    0.7  INTERPRET option: labels (47.1) and directives (99.914)   */
/*                  are not allowed.                                          */
/* 20260930         firstInBody: the clause is the first one (null clauses    */
/*                  apart) of a code body, as ooRexx checks the placement of  */
/*                  EXPOSE and USE LOCAL (a label before them is 99.907).     */
/* 20260930         An error of the scanner is passed to Syntax as its error  */
/*                  element, to find the clause in error (issue #8)           */
/* 20261113         CATCH and FINALLY clauses (RFE #31); FINALLY implies a    */
/*                  semicolon after it, as OTHERWISE                          */
/* 20261118         THROW (RFE #31, proposed), not under CMS or Executor      */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/*                                                                            */
/******************************************************************************/

::Requires "Scanner.cls"
::Requires "BaseClassesAndRoutines.cls"

::Class PreClauser Public

::Attribute scanner
::Attribute begin
::Attribute end

/******************************************************************************/
/* FIRSTINBODY: whether the clause just returned is the first one of a code   */
/*   body (null clauses apart): the one before it (clauseBefore) is none, or  */
/*   a directive. ooRexx checks that EXPOSE and USE LOCAL are the first       */
/*   instruction that way, before it parses them (a label is an instruction  */
/*   too).                                                                    */
/******************************************************************************/

::Method firstInBody
  Expose clauseBefore
  If clauseBefore == "" Then Return .True
  Return clauseBefore~caselessEndsWith( ".directive" )

/******************************************************************************/
/*                                                                            */
/* NEXTCLAUSE Method                                                          */
/*                                                                            */
/* The way we have written our scanner, a clause is always delimited by two   */
/* semicolons (we add one to the beginning of the source to ensure this       */
/* happens.                                                                   */
/*                                                                            */
/* When we are asked for a new clause, we need to go from "begin" (which is   */
/* supposed to be a semicolon, present or implied) to the next semicolon.     */
/* This, or a part of this, will be our clause. Element sequences between     */
/* two semicolons are only broken into smaller clauses in some cases, where   */
/* a supplementary semicolon is added: after a label, before and after THEN,  */
/* and after ELSE, OTHERWISE and FINALLY.                                     */
/*                                                                            */
/* Returns a four-element array:                                              */
/*   a[1] is a character string identifying the clause type                   */
/*                                                                            */
/*        "NULL.CLAUSE"       A null clause                                   */
/*        "LABEL.CLAUSE"      A label                                         */
/*        "COMMAND"           a command                                       */
/*                            The command expression has been prepared        */
/*                            (see the PrepareExpression method below).       */
/*        "MESSAGE"           a message instruction (may be an extended       */
/*                            assignment). The instruction has been prepared  */
/*                            (see the PrepareExpression method below).       */
/*        "ASSIGNMENT"        a (possibly extended) assignment                */
/*        name".DIRECTIVE",   where "name" is a valid directive name.         */
/*        name".CLAUSE",      where "name" is one of DO, LOOP, SELECT,        */
/*                            IF, WHEN, THEN, ELSE, OTHERWISE, CATCH or       */
/*                            FINALLY.                                        */
/*                            The expressions after IF and WHEN are prepared  */
/*                            (see the PrepareExpression routine).            */
/*        name".INSTRUCTION"  for all ooRexx instructions that are            */
/*                            not identified as clauses.                      */
/*        "END.OF.SOURCE",    returned once, at end of source (EOS).          */
/*        "IMPLICIT.EXIT.INSTRUCTION" added before every directive that       */
/*                            is not inside a source literal, before "}"      */
/*                            (Executor only) and before EOS                  */
/*                                                                            */
/*   a[2] is the first element ("begin") of the delimiting semicolon pair     */
/*   a[3] is the second element ("end") of the delimiting semicolon pair      */
/*   a[4] is a n-element element array of a priori non-ignorable elements.    */
/*     1) They are "a priori non ignorable" because further analysis may      */
/*        determine that they are indeed ignorable. For example, in a DO      */
/*        clause like "DO i = 1 To 3", the blanks beyween "1" and "To" and    */
/*        between "To" and "3" are a priori non ignorable; only a full        */
/*        syntactic analysis of the DO clause can determine that "To" is      */
/*        indeed a clause (sub-)keyword, and therefore these blanks are       */
/*        in fact ignorable.                                                  */
/*     2) The array does not contain the "begin" and "end" markers. This      */
/*        means, for example, that a zero-length array will always be a       */
/*        null clause.                                                        */
/*                                                                            */
/******************************************************************************/

::Method nextClause
  Expose package scanner begin end elements instructions lastClauseWas

  -- First pass. This applies Rexx rules, as implemented by the scanner,
  -- to the element sequence

  next            = begin
  directive       = 0
  directiveMarker = .Nil
  Loop Until next < .EL.END_OF_CLAUSE
    -- An error detected by the scanner
    If next~isA( .Error.Element ) Then Signal Error
    If next < .EL.LEFT_CURLY_BRACKET, next~closing \== .Nil Then Do
      next = next~closing
    End
    Else Do
      If next < .EL.DIRECTIVE_START Then Do
        directive       = 1
        directiveMarker = next
      End
      If directive, next~prev < .EL.SIMPLE_VARIABLE Then Do
        If \package~sourceLiteralStack~isEmpty, -
          WordPos(next~prev~value,"R ROUTINE CO COACTIVITY") > 0 Then Do
          Call InsertSemicolonBefore next
          next = result
          next = next~prev~prev -- Backtrack two elements
        End
        directive = 0
      End
    End
    next = next~next
  End

  -- We now collect the (supposedly) non-ignorable elements in an array
  self~generateElementsArray

  -- Update end marker for ::RESOURCE directives
  If directiveMarker \== .Nil, directiveMarker~closing \== .Nil Then
    end = directiveMarker~closing

  c = elements~items

  -- Now the clause begins at 'begin' and ends at 'end', and it has
  -- exactly 'c' non ignorable elements in between.

  ------------------------------------------------------------------------------
  -- 0. No tokens: that's a null clause                                       --
  ------------------------------------------------------------------------------

  If c == 0 Then Exit self~Null.Clause

  ------------------------------------------------------------------------------
  -- 1. In some cases, the first token lets us decide the clause type         --
  ------------------------------------------------------------------------------

  t1    = elements[1]
  t1Cat = t1~category

  isAKeyword = 0
  If t1 < .ALL.VARIABLES_AND_KEYWORDS Then
    isAKeyword = instructions~hasItem( t1~value )

  -- Keyword instructions followed by whitespace
  If c > 1 Then Do
    t2 = elements[2]
    -- Very frequent case
    If isAKeyword, t2 < .ALL.WHITESPACE_LIKE Then Exit self~Keyword.Clause
  End
  Else Do -- c == 1
    Select
      When isAKeyword Then Exit self~Keyword.Clause
      When t1Cat == .EL.RIGHT_CURLY_BRACKET Then Exit self~Source.Literal.End
      When t1Cat == .EL.END_OF_SOURCE Then Exit self~End.Of.Source
      Otherwise Exit self~Command.Instruction
    End
  End

  -- Directives
  If t1Cat == .EL.DIRECTIVE_START Then Exit self~directive

  -- Case 1.2: The End-of-source marker
  If t1Cat == .EL.END_OF_SOURCE Then Exit self~End.Of.Source

  ------------------------------------------------------------------------------
  -- 2. OK, we have at least two tokens in the elements array.                --
  --                                                                          --
  -- Try to catch first all the cases which can be determined by inspecting   --
  -- the first two tokens                                                     --
  ------------------------------------------------------------------------------

  t2 = elements[2]

  -- Case 6: Symbol or string + colon? --> A label
  If t2 < .EL.COLON , t1 < .ALL.SYMBOLS_AND_STRINGS Then Exit self~Label.Clause

  -- Case 7: Symbol + assignment operator? --> An assignment
  If t1 < .ALL.SYMBOLS Then Do
    -- ooRexx catches "a == b" and flags it, in the case that this was intended
    -- to be "a = b". Executor removes this limitation.
    If \.Options.Executor, t2 < .EL.OP.STRICT.EQUAL Then Call 35.001 "=="
    If t2 < .EL.OP.EQUAL Then Call SetCategory t2, .EL.ASG.EQUAL
    If t2 < .ALL.ASSIGNMENTS Then Exit self~Assignment.Instruction
    If isAKeyword, t2 < .ALL.WHITESPACE_LIKE Then Exit self~Keyword.Clause
  End

  If IsAMessageInstruction(t1) Then
    Exit self~Message.Instruction

  If .Options.Executor, -
    t1 < .ALL.VARIABLES_AND_KEYWORDS, -
    t2 < .EL.LEFT_PARENTHESIS Then Exit self~Command.Instruction

  -- Case 4 (bis): keyword instructions
  If isAKeyword Then Exit self~Keyword.Clause

  -- Case 5 (ter): Command or message instruction
  Exit self~Command.Instruction

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, t1, Arg(1) )

Error:
  code       = next~code
  additional = next~additional
  -- The error element itself: Syntax places the error at its "from", and
  -- looks for the clause in error from it
  Call Syntax code, next, additional      -- Does not return

/******************************************************************************/
/* generateElementsArray collects all the a priori non-ignorable elements     */
/*   between two semicolons and stores them in an array.                      */
/*   It also updates the "end" marker                                         */
/******************************************************************************/

::Method generateElementsArray
  Expose begin end elements

  elements = Array()

  -- A shebang line is a clause with no instruction in it: a null clause
  -- (the shebang element stays in the element chain).
  If begin~next < .EL.SHEBANG Then Do
    end = begin~next~next
    Return
  End

  next = begin
  Loop Until next < .EL.RIGHT_CURLY_BRACKET
    next = TheElementAfter( next )
    If next < .EL.LEFT_CURLY_BRACKET, next~closing \== .Nil Then Do
      elements~append( next )
      next = next~closing
      elements~append( next )
      next = TheElementAfter( next )
    End
  If next < .EL.END_OF_CLAUSE Then Leave
    elements~append( next )
  End

  -- In Executor, mark final "=" and "==".
  If .Options.Executor,  -
    \elements~isEmpty, elements~lastitem < .ALL.EXECUTOR.PREFINAL Then Do
    lastitem = elements~lastitem
    If lastitem < .EL.OP.EQUAL
      Then Call SetCategory lastitem, .EL.EXECUTOR.FINAL.EQUAL
      Else Do -- "=="
        second = lastItem~next
        Loop While second \< .EL.OP.STRICT.EQUAL
          second = second~next
        End
        Call SetCategory lastitem, .EL.EXECUTOR.FINAL.DOUBLE.EQUAL
        Call SetCategory second,   .EL.EXECUTOR.FINAL.DOUBLE.EQUAL
      End
  End

  end = next

/******************************************************************************/
/* INIT                                                                       */
/*                                                                            */
/******************************************************************************/

::Method init
  Expose scanner begin lastClauseWas package  -
    instructions clauses directives assignments -
    lastRealClause clauseBefore

  Use Strict Arg package

  scanner = .Scanner~new( package )

  begin = scanner~head

  lastClauseWas = ""
  lastRealClause = ""                   -- The last clause but null clauses
  clauseBefore   = ""                   -- The one before it

  -- Keyword instructions, including some clauses like WHEN, THEN, ELSE
  -- or OTHERWISE.

  instructions = Set()

  Do keyword Over (address, arg, call, do, drop, else, end, exit, if,   -
    interpret, iterate, leave, nop, numeric, options, otherwise, parse, -
    procedure, pull, push, queue, return, say, select, signal, then,    -
    trace, when)
    instructions[] = keyword
  End

  -- The following are ooRexx-only

  If .Features.Expose.Instruction  Then instructions[] = expose
  If .Features.Forward.Instruction Then instructions[] = forward
  If .Features.Guard.Instruction   Then instructions[] = guard
  If .Features.Loop.Instruction    Then instructions[] = loop
  If .Features.Raise.Instruction   Then instructions[] = raise
  If .Features.Reply.Instruction   Then instructions[] = reply
  If .Features.Use.Instruction     Then instructions[] = use

  -- CATCH and FINALLY clauses of DO, LOOP and SELECT (RFE #31)

  If .Features.Catch.Clauses Then Do
    instructions[] = catch
    instructions[] = finally
  End

  -- THROW (RFE #31, proposed)

  If .Features.Throw.Instruction Then instructions[] = throw

  -- Executor and CMS-only

  If .Features.Upper.Instruction Then instructions[] = upper

  -- Directives

  directives = Set()

  Do keyword Over (annotate, attribute, class, constant, -
    method, options, requires, resource, routine)
    directives[] = keyword
  End

  If .Options.Executor Then Do
    directives[] = extension
  End

  -- These are pure clauses (i.e., not complete instructions).
  -- The rest of the keywords produce clauses that are also instructions.

  clauses = Set()

  Do keyword Over (Do, End, Loop, Select, If, When, Then, Else, Otherwise)
    clauses[] = keyword
  End

  If .Features.Catch.Clauses Then Do
    clauses[] = Catch
    clauses[] = Finally
  End

  -- Extended assignments

  assignments = Set()

  Do seq Over ("+", "-", "*", "/", "%", "&", "|", "//", "||", "&&", "**")
    assignments[] = seq"="
  End

/******************************************************************************/
/* Common method to return clauses                                            */
/******************************************************************************/

::Method Return
  Expose begin end elements lastClauseWas lastRealClause clauseBefore
  Use Strict Arg clauseType

  lastClauseWas = clauseType
  If clauseType \== Null.Clause Then Do
    clauseBefore   = lastRealClause
    lastRealClause = clauseType
  End

  clauseInfo = clauseType, begin, end, elements
  begin = end
  Return clauseInfo

/******************************************************************************/
/* ASSIGNMENT instructions                                                    */
/******************************************************************************/

::Method Assignment.Instruction

  Return self~return( Assignment.Instruction )

/******************************************************************************/
/* SOURCE LITERAL END                                                         */
/******************************************************************************/

::Method Source.Literal.End
  Expose lastClauseWas

  If lastClauseWas \== Implicit.Exit.Instruction Then
    Exit self~Implicit.Exit.Instruction

  Return self~return( Source.Literal.End )

/******************************************************************************/
/* END OF SOURCE                                                              */
/******************************************************************************/

::Method End.Of.Source
  Expose lastClauseWas

  If lastClauseWas \== Implicit.Exit.Instruction Then
    Exit self~Implicit.Exit.Instruction

  Return self~return( End.Of.Source )

/******************************************************************************/
/* NULL CLAUSES                                                               */
/******************************************************************************/

::Method Null.Clause;   Return self~return( Null.Clause )

/******************************************************************************/
/* IMPLICIT EXIT                                                              */
/******************************************************************************/

::Method Implicit.Exit.Instruction
  Expose begin end elements

  elements = Array()

  semicolon = InsertSemicolonAfter( begin )

  inserted = .Inserted.Implicit.Exit~after( begin )

  end = semicolon

  Return self~return( Implicit.Exit.Instruction )

/******************************************************************************/
/* DIRECTIVES                                                                 */
/******************************************************************************/

::Method directive
  Expose package elements directives lastClauseWas

  -- INTERPRET data must not contain directive instructions.
  directive_start = elements[1]
  If .Options.Interpret Then Signal 99.914

  If package~sourceLiteralStack~isEmpty, -
    lastClauseWas \== Implicit.Exit.Instruction Then
    Exit self~Implicit.Exit.Instruction

  directive_start = elements[1]

  If elements~items == 1          Then Signal 20.916

  name = elements[2]

  If name \< .ALL.SYMBOLS       Then Signal 20.916

  -- If we are inside a source literal (Executor only), we only recognize
  -- four directives
  If \ package~sourceLiteralStack~isEmpty Then Do
    If WordPos(name~value, "R ROUTINE CO COACTIVITY") == 0 Then Signal 99.916
  End
  Else If \ directives~hasItem( name~value ) Then Signal 99.916

  Call Set.Directive.Keyword name

  If elements~items > 2, elements[3]~ignorable Then elements~delete( 3 )

  Return self~return( name~value"."directive )

-- Symbol expected after ::.
20.916: Syntax( 20.916, directive_start )

-- Unrecognized directive instruction "&1".
99.900: Syntax( 99.900, directive_start, -
  'Unrecognized directive instruction "'name~value'"' -
)

-- Unrecognized directive instruction.
99.916: Syntax( 99.916, directive_start )

-- INTERPRET data must not contain directive instructions.
99.914: Syntax( 99.914, directive_start )

/******************************************************************************/
/* KEYWORD INSTRUCTIONS                                                       */
/******************************************************************************/

::Method Keyword.Clause
  Expose package begin end elements clauses

  keyword = elements[1]

  Call SetKeyword keyword

  -- keyword + blank -> ignorable blank. Recalculate elements
  If elements~items > 1, elements[2]~ignorable Then elements~delete( 2 )

  name = elements[1]~value

  If WordPos(name, "IF WHEN THEN ELSE OTHERWISE FINALLY") > 0 Then Signal (name)

Done:
  If clauses~hasItem( name ) Then Exit self~return( name"."clause      )
  Else                            Exit self~return( name"."instruction )

THEN: ELSE: OTHERWISE: FINALLY:
  -- Do not insert an extra semicolon if we already have an end-of-clause
  semicolon? = TheElementAfter( keyword )
  If semicolon? < .EL.END_OF_CLAUSE Then semicolon = semicolon?
  Else semicolon = InsertSemicolonAfter( keyword )
  elements    = Array( keyword )
  end       = semicolon
  Signal Done

IF: WHEN:
  If elements~items == 1 Then Signal Done
  Call PrepareExpression package, elements[2], "THEN"

  elements = Array()
  element = TheElementAfter( begin )
  Loop
    elements~append( element )
    next = TheElementAfter( element )
  If next < .EL.END_OF_CLAUSE Then Leave
    element = next
  End
  end = next
  Signal Done

/******************************************************************************/
/* MESSAGE INSTRUCTION                                                        */
/******************************************************************************/

::Method Message.Instruction
  Expose elements begin end

  element = elements[1]

  checkForAssignment = .True
  Call PrepareExpression package, element,,checkForAssignment

  self~generateElementsArray

  Return self~return( Message )

/******************************************************************************/
/* COMMAND                                                                    */
/******************************************************************************/

::Method Command.Instruction
  Expose elements begin end

  element = elements[1]

  Call PrepareExpression package, element

  -- Regen element and end, because PrepareExpression may have
  -- changed some elements.
  self~generateElementsArray

  Return self~return( Command )

/******************************************************************************/
/* LABEL                                                                      */
/******************************************************************************/

::Method Label.Clause
  Expose begin end elements

  label = elements[1]
  colon = elements[2]

  -- INTERPRET data must not contain labels.
  If .Options.Interpret Then Signal 47.001

  elements = ( label, colon )

  -- Don't insert a semicolon if we already have one
  If colon~next < .EL.END_OF_CLAUSE Then Nop
  Else end = InsertSemicolonAfter( colon )

  Return self~return( Label.Clause )

-- INTERPRET data must not contain labels; found "&1".
47.001: Syntax( 47.001, label, label )

--------------------------------------------------------------------------------
-- ISAMESSAGEINSTRUCTION                                                      --
--   Determines whether an expression is a command or a message instruction,  --
--   including an (extended) message term assignment (standard assignments    --
--   are processed elsewhere).                                                --
--------------------------------------------------------------------------------

--
-- "Terms are literal strings, symbols, message terms and sequences,
--  Array terms, Variable Reference terms, function calls, or subexpressions".
--

::Routine isAMessageInstruction

  Use Strict Arg element

  notAMessageInstruction = 0

  -- Subexpressions
  -- Parentheses balancing has been checked by the prescanner
  If element < .EL.LEFT_PARENTHESIS Then Do
    Call BalanceParens
    Signal ExpectMessageOperator
  End
  -- Literal strings, symbols, and function calls
  If element < .ALL.SYMBOLS_AND_STRINGS Then Do
    element = TheElementAfter( element )
    If element < .EL.LEFT_PARENTHESIS Then Call BalanceParens
    Signal ExpectMessageOperator
  End
  If element < (.EL.OP.LOWER_THAN || .EL.OP.GREATER_THAN) Then Do
    element = TheElementAfter( element )
    If element \< .ALL.REFERENCED_SYMBOLS Then Return 0
    element = TheElementAfter( element )
    Signal ExpectMessageOperator
  End
  -- Array terms cannot be lhs
  Return 0

ExpectMessageOperator:
  malformed = .False
  Loop
    If element \< .ALL.OPS.MESSAGE_SEND Then
      Return notAMessageInstruction
    If element < .EL.LEFT_BRACKET Then Call BalanceBrackets
    Else Do -- ~ or ~~
      element = TheElementAfter( element )
      If element \< .ALL.SYMBOLS_AND_STRINGS Then Return malformed
      element = TheElementAfter( element )
      If element < .EL.COLON Then Do
        element = TheElementAfter( element )
        If element \< .ALL.MESSAGE_SCOPE_ELEMENTS Then Return malformed
        element = TheElementAfter( element )
      End
      If element < .EL.LEFT_PARENTHESIS Then Call BalanceParens
    End
    If element < .EL.END_OF_CLAUSE Then Return 1
    If element < .EL.OP.EQUAL Then
      Call SetCategory element, .EL.ASG.EQUAL
    If element < .ALL.ASSIGNMENTS Then Return 1
  End

BalanceParens:
  If element~closing \== .Nil Then Do
    closing = element~closing
    element = TheElementAfter(closing)
    Return
  End
  Parse Value element~from With line col
-- Left parenthesis "(" in position &1 on line &2 requires
-- a corresponding right parenthesis ")".
36.901: Syntax( 36.901, element, col, line)

BalanceBrackets:
  If element~closing \== .Nil Then Do
    closing = element~closing
    element = TheElementAfter(closing)
    Return
  End
  Parse Value element~from With line col
-- Square bracket "[" in position &1 on line &2
-- requires a corresponding right square bracket "]".
36.902: Syntax( 36.902, element, col, line)
