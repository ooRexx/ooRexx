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
/* Expressions.cls                                                            */
/* ===============                                                            */
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
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250416    0.2a Add argument info to called routine chain                 */
/* 20250419         Fix NAry.Expression crashing on SecondPass.               */
/* 20250421    0.2b Argument lists in function calls not working properly.    */
/* 20250803    0.2d Add support for array terms                               */
/* 20251126    0.3a Add support for Executor <number><rest> constructs        */
/* 20251129         (Executor) The message name can be omitted after ~, ~~    */
/* 20251201         (Executor) Add support for source literals                */
/* 20251208         (Executor) Implement trailing blocks                      */
/* 20251209         (Executor) Implement named arguments                      */
/* 20251226    0.4a Add support for "=" and "==" at end of expression         */
/* 20260522    0.5  CMS: Add support for CMS Rexx expressions                 */
/* 20260601    0.6  Implement a fine-grained feature system                   */
/* 20260620         CMS: Implement multi-line strings                         */
/* 20260925         Indexed.Or.Invoked.Term: trailingBlock was not kept       */
/* 20260928         Expression.List: an empty logical list is 35.929 (IF,     */
/*                  WHEN, WHILE, UNTIL with no expression)                    */
/* 20260928         The function calls of a ::CONSTANT expression are kept by */
/*                  the package (constantCalls), to be classified too.        */
/* 20260929         Expression.List~arg(): the omitted expressions at the end */
/*                  count (the array term ("a", ) has two items)              */
/* 20260930         Expression.List: the "case" option (the WHEN of a SELECT  */
/*                  CASE): a missing expression is 35.934, as ooRexx          */
/*                  A stem in a ::CONSTANT expression ("::CONSTANT k (s.)")   */
/*                  failed with 97.1: there is no body there                  */
/* 20260930         20.917 was placed at an unassigned variable ("NEXT"), and */
/*                  rxcheck failed with 93.907 ("a~b:*")                      */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261122         The first element of a compound variable no longer gets a */
/*                  term (Element~term is gone)                               */
/* 20261123         No table constructors ("[...]", inspired by NetRexx and   */
/*                  Lua, with EXPERIMENTAL): an experiment that was not       */
/*                  followed                                                  */
/*                                                                            */
/******************************************************************************/

 .environment ~ Concatenation.Expression   = .Concatenation.Expression
 .environment ~ Expression.List            = .Expression.List
 .environment ~ Arguments                  = .Arguments
 .environment ~ And.Expression             = .And.Expression
 .environment ~ NAry.Expression            = .NAry.Expression
 .environment ~ Addition.Expression        = .Addition.Expression
 .environment ~ Comparison.Expression      = .Comparison.Expression
 .environment ~ Multiplication.Expression  = .Multiplication.Expression
 .environment ~ Or.Expression              = .Or.Expression
 .environment ~ Source.Literal             = .Source.Literal
 .environment ~ Lambda.Expression          = .Lambda.Expression
 .environment ~ Prefix.Expression          = .Prefix.Expression
 .environment ~ Power.Expression           = .Power.Expression
 .environment ~ SubExpression              = .SubExpression

 .environment ~ NameSpace.Qualified.ClassName = -
   .NameSpace.Qualified.ClassName
 .environment ~ NameSpace.Qualified.Function.Call = -
   .NameSpace.Qualified.Function.Call

 .environment ~               Rexx.Term  =               .Rexx.Term

 .environment ~              Array.Term  =              .Array.Term
 .environment ~  Compound.Variable.Term  =  .Compound.Variable.Term
 .environment ~ Indexed.or.Invoked.Term  = .Indexed.Or.Invoked.Term
 .environment ~      Function.Call.Term  =      .Function.Call.Term
 .environment ~     Literal.String.Term  =     .Literal.String.Term
 .environment ~            Message.Term  =            .Message.Term
 .environment ~      Stem.Variable.Term  =      .Stem.Variable.Term
 .environment ~             Symbol.Term  =             .Symbol.Term
 .environment ~ Variable.Reference.Term  = .Variable.Reference.Term

::Requires "BaseClassesAndRoutines.cls"

/******************************************************************************/
/******************************************************************************/
/* CLASSES DEFINING EXPRESSIONS                                               */
/******************************************************************************/
/******************************************************************************/

::Class Rexx.Expression      Public
::Constant isEmpty 0

/******************************************************************************/
/* This private class implements code common to many subexpression cases      */
/******************************************************************************/

::Class NAry.Expression SubClass Rexx.Expression
::Constant items 1
::Attribute args Get
::Method arg
  Arg n, option +1
  If Arg() == 0 Then Return 1
  If Arg() == 1 Then
    If n == 1 Then Return self
    Else Return .nil
  Return option = "E"
::Attribute begin
::Attribute end
::Method init
  Expose args begin end
  args = arg(1, "A")
  begin = args[1]~begin
  end = args~lastItem~end

/******************************************************************************/
/* ARGUMENTS                                                                  */
/*                                                                            */
/* With optional support for named arguments (Executor)                       */
/******************************************************************************/

::Routine Arguments Public

  Use Strict Arg package, begin, terminators = ""

  positionalArguments = Array()
  namedArguments      = Array()
  element             = begin
  trailingComma       = 0
  stopParsing         = .EL.END_OF_CLAUSE
  terminators       ||= stopParsing
  commasFound         = 0
  end                 = element              -- In case list is empty
  IgnoreBlanks        = .True

  newTerminators = terminators || .EL.COMMA

  Loop Label Positional While element \< .ALL.EXPRESSION_TERMINATORS

    If .Options.Executor, element < .EL.COLON Then Leave Positional

    If .Options.Executor, element < .ALL.VAR_SYMBOLS Then Do
      next = TheElementAfter( element )
      If next < .EL.COLON Then Leave Positional
    End

    trailingComma = 0

    Do While element < .EL.COMMA
      commasFound = 1
      positionalArguments~append( element )
      element = TheElementAfter( element )
      trailingComma = 1
    End

    end = element

  If element < terminators Then Leave

    If .Options.Executor, element < .EL.COLON Then Leave Positional

    If .Options.Executor, element < .ALL.VAR_SYMBOLS Then Do
      next = TheElementAfter( element )
      If next < .EL.COLON Then Leave Positional
    End

    trailingComma = 0

    newOrExpression = Or.Expression( package, element, newTerminators )
    element = newOrExpression~end
    positionalArguments~append( newOrExpression )

    end = element

  If element \< .EL.COMMA Then Leave

    positionalArguments~append( element )

    element = TheElementAfter( element )

    end = element

    trailingComma = 1
    commasFound = 1
  End

  If element \< (.EL.COLON || .ALL.VAR_SYMBOLS) Then Signal EndNamed

  Loop Label Positional While element \< .ALL.EXPRESSION_TERMINATORS
    If element < .EL.COLON Then Do
      element = TheElementAfter( element )
      If element \< .ALL.VAR_SYMBOLS Then Signal 20.900a
      Call SetConstantName element, .ARGUMENT.NAME
      namedArguments~append( (.Nil, element) )
      element = TheElementAfter( element, IgnoreBlanks )
      end = element
    End
    Else If element < .ALL.VAR_SYMBOLS Then Do
      name = element
      Call SetConstantName name, .ARGUMENT.NAME
      element = TheElementAfter( element )
      If element \< .EL.COLON Then Signal 99.900a
      element = TheElementAfter( element )
      If element < .EL.COMMA | element < terminators Then Do
        namedArguments~append( (name, .Nil) )
        end = element
      End
      Else Do
        newOrExpression = Or.Expression( package, element, newTerminators )
        end = newOrExpression~end
        namedArguments~append( (name, newOrExpression ) )
        element = end
      End
    End
    If element \< .EL.COMMA Then Leave
    element = TheElementAfter( element )
  End

EndNamed:
  If element \< terminators Then Call 35.001 element

  -- Can we do that when we may have named arguments afterwards?
  Do While positionalArguments~lastItem~isA(.Element)
    positionalArguments~delete( positionalArguments~last )
  End

  If positionalArguments~items == 0 Then
    .Arguments~new(begin, end, .Nil, namedArguments)

  Return .Arguments~new(begin, end, positionalArguments, namedArguments)

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, element, Arg(1) )

20.900a: Call Syntax 20.900, element, -
  'Symbol expected in named argument after ":", found "'element'"'

99.900a: Call Syntax 99.900, element, -
  'Colon expected after argument name, found "'element'"'

--------------------------------------------------------------------------------

::Class Arguments Public SubClass Rexx.Expression
::Attribute positionalArguments
::Attribute namedArguments
::Attribute begin
::Attribute end
::Attribute args
::Method arg
  Expose positionalArguments
  Arg n, option +1
  Select Case Arg()
    When 0 Then Return positionalArguments~size
    When 1 Then Return positionalArguments[n]
    Otherwise
      Select Case option
        When "E" Then Return  positionalArguments~hasIndex(n)
        When "O" Then Return \positionalArguments~hasIndex(n)
      End
  End
::Method isEmpty
  Expose args
  Return args == .Nil
::Method init
  Expose namedArguments args positionalArguments begin end

  Use Strict Arg begin, end, args, namedArguments
  positionalArguments = Array()

  If args == .Nil Then Return

  -- Calculate begin
  first = args~firstItem
  If first == .Nil Then begin = end
  Else If first~isA( .Special.Character.Sequence ) Then
    begin = first
  Else begin = first~begin

  -- Calculate the number of positionalArguments
  argN = 1
  Do i = 1 To args~items
    If args[i]~isA( .Special.Character.Sequence ) Then argN += 1
    Else positionalArguments[argN] = args[i]
  End

/******************************************************************************/
/* EXPRESSION LIST                                                            */
/*                                                                            */
/*   One of:                                                                  */
/*                                                                            */
/*     expression                           -- A classic expression, or...    */
/*     [expression] ("," [expression]) *    -- ...an array term.              */
/*                                                                            */
/*   A classic expression, or an array term (where trailing commas are        */
/*   meaningful), or an argument list (where trailing commas are discarded).  */
/******************************************************************************/

::Routine Expression.List Public

  Use Strict Arg package, begin, terminators = "", options = ""

  -- Logical expression lists can not have omitted expressions
  logical         = options~caselessContainsWord( "logical" )

  -- The WHEN of a SELECT CASE: 35.934 instead of 35.929
  missing         = 35.929
  If options~caselessContainsWord( "case" ) Then missing = 35.934

  -- Argument lists don't count final commas, contrary to array terms
  arguments       = options~caselessContainsWord( "arguments" )

  -- Executor commands can end in "=" or "=="
  executorCommand = options~caselessContainsWord( "executorcommand" )

  expressions     = Array()
  element         = begin
  trailingComma   = 0
  stopParsing     =                        -
       .EL.KEYWORD                         -
    || .EL.SUBKEYWORD                      -
    || .EL.END_OF_CLAUSE                   -
    || .ALL.ASSIGNMENTS
  If executorCommand Then stopParsing ||=  -
       .EL.EXECUTOR.FINAL.EQUAL            -
    || .EL.EXECUTOR.FINAL.DOUBLE.EQUAL

  terminators  ||= stopParsing
  commasFound    = 0
  end            = element              -- In case list is empty

  newTerminators = terminators || .EL.COMMA

  Loop While element \< .ALL.EXPRESSION_TERMINATORS

    trailingComma = 0

    Do While element < .EL.COMMA
      If \.Features.Array.Terms Then Signal 37.001
      commasFound = 1
      If logical Then Call Missing element
      expressions~append( element )
      element = TheElementAfter( element )
      trailingComma = 1
    End

    end = element

  If element < terminators Then Leave

    trailingComma = 0

    newOrExpression = Or.Expression( package, element, newTerminators )
    element = newOrExpression~end
    expressions~append( newOrExpression )

    end = element

  If element \< .EL.COMMA Then Leave

    If \.Features.Array.Terms Then Signal 37.001

    expressions~append( element )

    element = TheElementAfter( element )

    end = element

    trailingComma = 1
    commasFound = 1
  End

  If element \< terminators Then Call 35.001 element

  If trailingComma, logical Then Call Missing begin

  -- No expression at all (IF, WHEN, WHILE, UNTIL, GUARD ... WHEN): 35.929,
  -- as ooRexx.
  If logical, expressions~items == 0 Then Call Missing begin

  If arguments Then Do
    Do While expressions~lastItem~isA(.Element)
      expressions~delete( expressions~last )
    End
  End

  final = .Nil
  If .Options.Executor, end < .ALL.EXECUTOR.FINAL Then Do
    final = end
    end   = TheElementAfter( end )
  End

  Select Case expressions~items
    When 0 Then Return .Expression.List~new(.Nil, begin, end, final)
    -- Only one expression and no commas? --> A classic expression
    When 1 Then If \logical, \commasFound Then Return expressions[1]
    Otherwise Nop
  End

  Return .Expression.List~new(expressions, begin, end, final)

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, element, Arg(1) )


-- Missing expression in logical expression list (35.929), or in WHEN case
-- expression list (35.934).
Missing: Syntax( missing, Arg(1) )

-- Unexpected ",".
37.001: Syntax( 37.001, element)

--------------------------------------------------------------------------------

::Class Expression.List Public SubClass Rexx.Expression
::Attribute expressions
::Attribute begin
::Attribute end
::Attribute final
::Attribute args
::Method arg
  Expose expressions count
  Arg n, option +1
  Select Case Arg()
    -- The number of expressions, counting the omitted ones at the end (an
    -- array term: ("a", ) has two items, as in ooRexx).
    When 0 Then Return count
    When 1 Then Return expressions[n]
    Otherwise
      Select Case option
        When "E" Then Return  expressions~hasIndex(n)
        When "O" Then Return \expressions~hasIndex(n)
      End
  End
::Method isEmpty
  Expose args
  Return args == .Nil
::Method init
  Expose args expressions begin end final count
  Use Strict Arg args, begin, end, final
  expressions = Array()
  count       = 0

  If args \== .Nil Then Do

    -- Calculate begin
    first = args~firstItem
    If first~isA( .Special.Character.Sequence ) Then
      begin = first
    Else begin = first~begin

    -- Calculate the number of expressions (for argument lists)
    argN = 1
    Do i = 1 To args~items
      If args[i]~isA( .Special.Character.Sequence ) Then argN += 1
      Else expressions[argN] = args[i]
    End
    count = argN

  End

/******************************************************************************/
/* SOURCE.LITERAL                                                             */
/******************************************************************************/

::Routine Source.Literal Public

  Use Strict Arg package, begin, terminators = ""

  element = TheElementAfter(begin)

  instructions = Array()

  package~blockInstructionsStacks~push(.Stack~new())
  package~blockInstructionsStacks~top~push(begin)
  package~sourceLiteralStack~push(begin)
  preClauser = package~preClauser
  package~scannerStack~push( (preClauser~begin, preClauser~end ) )
  preClauser~begin = begin

  NoEndClause    = 0
  Source.Literal = 1

  coactivity = 0

  Loop Counter instrNo
    instruction = package~clauser~Next.Instruction( -
      NoEndClause,                                  -
      Source.Literal                                -
    )
    instrClass = instruction~class
    Select Case instrClass
      When .R.Directive               Then
        If instrNo > 1                Then Signal 99.900a
      When .Coactivity.Directive      Then
        If instrNo == 1               Then coactivity = 1
                                      Else Signal 99.900a
      When .Expose.Instruction        Then Do
        If instrNo \== 1 + coactivity Then Signal 99.900b
      End
      When .End.Clause                Then Signal 10.001
      When .Source.Literal.End        Then Leave
      Otherwise Nop
    End
    instructions~append( instruction )
    lastInstruction =    instruction
  End

  end    = instruction~end~next
  ignoreBlanks = .True
  If end~ignorable Then end = TheElementAfter(end, ignoreBlanks)

  package~sourceLiteralStack~pop
  package~blockInstructionsStacks~pop
  old = package~scannerStack~pop
  preClauser~begin = old[1]
  preClauser~end   = old[2]

  Source.Literal = .Source.Literal~new( begin, end, instructions )

  Return Source.Literal

-- END has no corresponding DO, LOOP, or SELECT.
10.001: Syntax( 10.001, instruction~begin )

-- Misplaced directive instruction
99.900a: Call Syntax 99.900, instruction~begin, -
  "Misplaced directive instruction"

-- EXPOSE must be the first instruction executed in a closure
99.900b: Call Syntax 99.900, instruction~begin, -
  "EXPOSE must be the first instruction executed in a closure"

-- Unrecognized directive instruction.
99.916: Syntax( 99.916, begin )

--------------------------------------------------------------------------------

::Class Source.Literal Public SubClass Rexx.Expression
::Attribute begin
::Attribute end
::Attribute instructions
::Method init
  Expose         begin  end  instructions
  Use Strict Arg begin, end, instructions

/******************************************************************************/
/* OR.EXPRESSION                                                              */
/*   expr       := expr_alias                                   (6.3.2.98)    */
/*   expr_alias := and_expression                               (6.3.2.99)    */
/*              | (expr_alias or_operator and_expression)                     */
/******************************************************************************/

::Routine Or.Expression Public

  Use Strict Arg package, begin, terminators

  and_expressions = Array()
  element           = begin
  newTerminators  = terminators || .ALL.OPS.ALTERNATIVE

  Loop

    newAndExpression = And.Expression( package, element, newTerminators )
    element = newAndExpression~end
    and_expressions~append( newAndExpression )

  If element \< .ALL.OPS.ALTERNATIVE Then Leave

    and_expressions~append( element )
    element        = TheElementAfter( element )

  End

  If and_expressions~items == 1 Then Return and_expressions[1]

  expression = .Or.Expression~sendWith("new",and_expressions)

  Return expression

--------------------------------------------------------------------------------

::Class Or.Expression Public SubClass NAry.Expression

/******************************************************************************/
/* AND.EXPRESSION                                                             */
/*   and_expression := comparison                               (6.3.2.101)   */
/*                    | and_expression '&' comparison                         */
/******************************************************************************/

::Routine And.Expression

  Use Strict Arg package, begin, terminators

  comparisons    = Array()
  element          = begin
  newTerminators = terminators || .ALL.OPS.CONJUNCTIVE

  Loop

    newComparison = Comparison.Expression( package, element, newTerminators )
    element = newComparison~end
    comparisons~append( newComparison )

  If element \< .ALL.OPS.CONJUNCTIVE Then Leave

    comparisons~append( element )
    element = TheElementAfter( element )

  End

  If comparisons~items == 1 Then Return comparisons[1]

  Return .And.Expression~sendWith("new",comparisons)

--------------------------------------------------------------------------------

::Class And.Expression  Public SubClass NAry.Expression

/******************************************************************************/
/* COMPARISON.EXPRESSION                                                      */
/*   comparison := concatenation                                (6.3.2.102)   */
/*              | comparison comparison_operator concatenation                */
/*   comparison_operator:= normal_compare | strict_compare      (6.3.2.103)   */
/*   normal_compare:= '=' | '\=' | '<>' | '><' | '>' | '<'      (6.3.2.104)   */
/*     | '>=' | '<=' | '\>' | '\<'                                            */
/*   strict_compare:= '==' | '\==' | '>>' | '<<' | '>>='        (6.3.2.105)   */
/*     | '<<=' | '\>>' | '\<<'                                                */
/******************************************************************************/

::Routine Comparison.Expression Public

  Use Strict Arg package, begin, terminators

  concatenations = Array()
  element           = begin
  newTerminators  = terminators || .ALL.OPS.COMPARISON

  Loop

    newConcatenation = Concatenation.Expression( package, element, newTerminators )
    element = newConcatenation~end
    concatenations~append( newConcatenation )

    If element \< .ALL.OPS.COMPARISON Then Leave

    concatenations~append( element )
    element      = TheElementAfter( element )

  End

  If concatenations~items == 1 Then Return concatenations[1]

  Return .Comparison.Expression~sendWith("new",concatenations)

--------------------------------------------------------------------------------

::Class Comparison.Expression Public SubClass NAry.Expression

/******************************************************************************/
/* CONCATENATION.EXPRESSION                                                   */
/*   concatenation := addition                                (6.3.2.106)     */
/*                 | concatenation [' ' | '||'] addition                      */
/******************************************************************************/

::Routine Concatenation.Expression Public

  Use Strict Arg package, begin, terminators

  additions       = Array()
  element         = begin
  newTerminators  = terminators || .ALL.OPS.CONCATENATION

  Loop

    newAddition = Addition.Expression( package, element, newTerminators )
    element = newAddition~end
    additions~append( newAddition )

  If element \< .ALL.OPS.CONCATENATION Then Leave

    additions~append( element )
    element      = TheElementAfter( element )

  End

  If additions~items == 1 Then Return additions[1]

  Return .Concatenation.Expression~sendWith("new",additions)

--------------------------------------------------------------------------------

::Class Concatenation.Expression Public SubClass NAry.Expression

/******************************************************************************/
/* ADDITION EXPRESSION                                                        */
/*   addition          :=   multiplication                       (6.3.2.107)  */
/*    | addition additive_operator multiplication                             */
/*   additive_operator := '+' | '-'                              (6.3.2.108)  */
/******************************************************************************/

::Routine Addition.Expression

  Use Strict Arg package, begin, terminators

  multiplications = Array()
  element           = begin
  newTerminators  = terminators || .ALL.OPS.ADDITIVE

  Loop

    newMultiplication = Multiplication.Expression( package, element, newTerminators )
    element = newMultiplication~end
    multiplications~append( newMultiplication )

  If element \< .ALL.OPS.ADDITIVE Then Leave

    multiplications~append( element )
    element      = TheElementAfter( element )

  End

  If multiplications~items == 1 Then Return multiplications[1]

  Return .Addition.Expression~sendWith("new",multiplications)

--------------------------------------------------------------------------------

::Class Addition.Expression Public SubClass NAry.Expression

/******************************************************************************/
/* MULTIPLICATION.EXPRESSION                                                  */
/*   multiplication := power_expression                         (6.3.2.109)   */
/*     | multiplication multiplicative_operator power_expression              */
/*   multiplicative_operator:= '*' | '/' | '//' | '%'           (6.3.2.110)   */
/******************************************************************************/

::Routine Multiplication.Expression

  Use Strict Arg package, begin, terminators

  powers         = Array()
  element          = begin
  newTerminators = terminators || .ALL.OPS.MULTIPLICATIVE

  Loop

    newPower = Power.Expression( package, element, newTerminators )
    element = newPower~end
    powers~append( newPower )

  If element \< .ALL.OPS.MULTIPLICATIVE Then Leave

    powers~append( element )
    element      = TheElementAfter( element )

  End

  If powers~items == 1 Then Return powers[1]

  Return .Multiplication.Expression~sendWith("new",powers)

--------------------------------------------------------------------------------

::Class Multiplication.Expression Public SubClass NAry.Expression

/******************************************************************************/
/* POWER.EXPRESSION                                                           */
/*   power_expression := prefix_expression                      (6.3.2.111)   */
/*     | power_expression '**' prefix_expression                              */
/******************************************************************************/

::Routine Power.Expression

  Use Strict Arg package, begin, terminators

  expressionArray = Array()
  element           = begin
  newTerminators  = terminators || .EL.OP.POWER

  Loop

    newPrefix = Prefix.Expression( package, element, newTerminators )
    element = newPrefix~end
    expressionArray~append( newPrefix )

  If element \< .EL.OP.POWER Then Leave

    expressionArray~append( element )
    element        = TheElementAfter( element )

  End

  If expressionArray~items == 1 Then Return expressionArray[1]

  Return .Power.Expression~sendWith("new",expressionArray)

--------------------------------------------------------------------------------

::Class Power.Expression Public SubClass NAry.Expression

/******************************************************************************/
/* PREFIX.EXPRESSION                                                          */
/*   prefix_expression := ('+' | '-' | '\') prefix_expression  (6.3.2.112)    */
/*    | term | Msg35.1                                                        */
/******************************************************************************/

::Routine Prefix.Expression Public

  Use Strict Arg package, begin, terminators

  prefixes = Array()
  element  = begin

  Loop While element < .PREFIX_OPERATORS
    -- Assign prefix operators when appropriate
    Select Case element~category
      When .EL.OP.PLUS     Then Call SetCategory element, .EL.OP.PREFIX.PLUS
      When .EL.OP.MINUS    Then Call SetCategory element, .EL.OP.PREFIX.MINUS
      Otherwise Nop -- "\", "not" and "#" are always prefix operators
    End
    prefixes~append( element )
    element =  TheElementAfter( element )
  End

  term = Term( package, element, terminators )

  If prefixes~isEmpty Then Return term

  Return .Prefix.Expression~new( prefixes, term )

--------------------------------------------------------------------------------

::Class Prefix.Expression Public SubClass Rexx.Expression
::Attribute begin
::Attribute end
::Attribute prefixes
::Attribute term
::Method arg
  Arg n, option +1
  If Arg() == 0 Then Return 1
  If Arg() == 1 Then
    If n == 1 Then Return self
    Else Return .nil
  Return option = "E"
::Method init
  Expose prefixes term begin end
  Use Strict Arg prefixes, term
  begin = prefixes[1]
  end = term~end

/******************************************************************************/
/******************************************************************************/
/* TERMS                                                                      */
/******************************************************************************/
/******************************************************************************/

::Class Rexx.Term Public
::Method arg
  Arg n, option +1
  If Arg() == 0 Then Return 1
  If Arg() == 1 Then
    If n == 1 Then Return self
    Else Return .nil
  Return option = "E"
::Constant isEmpty 0

--------------------------------------------------------------------------------
-- Term                                                                       --
--   A term may be:                                                           --
--    1) A function call                                                      --
--    2) A message term                                                       --
--    3) A variable reference term                                            --
--    4) A subexpression                                                      --
--    5) A symbol                                                             --
--    6) A literal string                                                     --
--    Case 7), an array term, is handled elsewhere in the parser.             --
--    8) A numeric symbol abutted to a non-numeric symbol (Executor only)     --
--  See rexxref 5.1.0 1.11.1. Terms and Expressions                           --
--------------------------------------------------------------------------------

::Routine Term
  Use Strict Arg package, begin, terminators

  element        = begin
  newTerminators = terminators || .ALL.OPS.MESSAGE_SEND

  term = Simple.Term( package, element, newTerminators )

  Return Simple.Message.Term( package, term )

--------------------------------------------------------------------------------
-- SIMPLE.TERM                                                                --
--   A simple term is a term that is neither a message term nor an            --
--   unparenthesized array term, namely:                                      --
--    1) A function call                                                      --
--    3) A variable reference term                                            --
--    4) A subexpression                                                      --
--    5) A symbol                                                             --
--    6) A literal string                                                     --
--    7) A numeric symbol abutted to a non-numeric symbol (Executor only)     --
--  See rexxref 5.1.0 1.11.1. Terms and Expressions                           --
--------------------------------------------------------------------------------

::Routine Simple.Term Public

  Use Strict Arg package, begin, terminators


  Select
    When begin < .EL.LEFT_CURLY_BRACKET Then
      Exit Source.Literal( package, begin, "" )
    When begin < .EL.LEFT_PARENTHESIS Then
      Exit SubExpression( package, begin, terminators )
    When begin < .ALL.SYMBOLS Then Do
      symbol = begin

      If .Options.Executor, symbol < .ALL.NUMBERS,      -
        symbol~next < .EL.OP.EXECUTOR.ABUTTAL Then Do
        Exit .Concatenation.Expression~sendWith("new",  -
          (                                        -
            Symbol.Term( package, begin ),              -
            begin~next,                                 -
            Symbol.Term( package, begin~next~next )     -
          )                                             -
        )
      End

      element = TheElementAfter( symbol )
      Select Case element~category
 -- Followed by ":"? This is a namespace-qualified symbol.
        -- (ooRexx extension; not recognized under CMS, where the
        -- dangling ":" falls through to an Error 35.)
        When .EL.COLON Then Do
          If .Features.Namespace.Qualified.Symbols Then
            Exit Namespace.Qualified.Symbol( package, symbol, element )
          Exit Symbol.Term( package, begin )
        End        -- Followed by "("? This is a function call
        When .EL.LEFT_PARENTHESIS Then
          Exit Function.Call.Term( package, symbol, element, terminators )
        When .EL.LEFT_CURLY_BRACKET Then
          Exit Function.Call.Term( package, symbol, element, terminators )
        Otherwise
          Exit Symbol.Term( package, begin )
      End
    End
    When begin < .ALL.STRINGS Then Do
      name    = begin
      element = TheElementAfter( name )
      If element < .EL.LEFT_PARENTHESIS Then   -- A function call?
        Return Literal.String.Function.Call(   -
          package, name, element, terminators  -
        )
      If element < .EL.LEFT_CURLY_BRACKET Then -- A function call?
        Return Literal.String.Function.Call(   -
          package, name, element, terminators  -
        )
      Exit .Literal.String.Term~new( name )  -- That's a string literal
    End
    When begin < .ALL.OPS.REFERENCE Then
      Return Variable.Reference.Term( package, begin )
   Otherwise
      -- Incorrect expression detected at "element".
      -- Backtrack one item if expression ends abruptly, to produce
      -- a more understandable error message.

      oneBack = TheElementBefore( begin )

      If begin < .EL.END_OF_CLAUSE Then Call 35.001 oneback
      If begin < .EL.KEYWORD       Then Call 35.001 oneback
      Call 35.001 begin
  End

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, begin, Arg(1) )

--------------------------------------------------------------------------------
-- ARRAY.TERM                                                                 --
--------------------------------------------------------------------------------

::Class Array.Term Public SubClass Rexx.Term
::Attribute expression_list Get
::Method Init
  Expose expression_list
  Use Strict Arg expression_list
::Method begin
  Forward To (self~expression_list)
::Method end
  Forward To (self~expression_list)
::Method arg
  Forward To (self~expression_list)
::Method args
  Forward To (self~expression_list)
::Method isEmpty
  Forward To (self~expression_list)

--------------------------------------------------------------------------------
-- LITERAL.STRING.FUNCTION.CALL                                               --
--------------------------------------------------------------------------------

::Routine Literal.String.Function.Call

  Use Strict Arg package, name, left, terminators

  arguments     = .Nil
  trailingBlock = .Nil

  If left < .EL.LEFT_PARENTHESIS Then Do

    next = TheElementAfter( left )

    arguments = Arguments( package, next, .EL.RIGHT_PARENTHESIS )

    Call ExpectParenPair left, arguments~end

    left = arguments~end~next
  End
  If left < .EL.LEFT_CURLY_BRACKET Then
    trailingBlock = Source.Literal( package, left, "" )

  name~invocationType = Function

  -- A ::CONSTANT expression is in no body: its calls are kept by the
  -- package, and SecondPass resolves them with no labels, as ooRexx.
  body = package~currentBodies~top
  If body \== .Nil Then body~calledRoutineNames~append( -
    (name, arguments, trailingBlock)                   -
  )
  Else package~constantCalls~append( (name, arguments, trailingBlock) )

  Exit .Function.Call.Term~new(name, arguments, trailingBlock )

--------------------------------------------------------------------------------

::Class Literal.String.Term Public SubClass Rexx.Term
::Attribute begin
::Attribute end
::Method theString
  Expose string
  Return string
::Method init
  Expose begin string end
  Use Strict Arg string
  begin = string
  end = TheElementAfter( string )

/******************************************************************************/
/* SUBEXPRESSION                                                              */
/******************************************************************************/

::Routine SubExpression Public

  Use Strict Arg package, begin, terminators = ""

  lParen = begin

  next   = TheElementAfter( lParen )

  If next < .EL.RIGHT_PARENTHESIS Then Signal 35.001


  subExpression = Expression.List(   -
    package,                         -
    TheElementAfter( begin ),        - -- Where to begin parsing
    .EL.RIGHT_PARENTHESIS            - -- Clear terminators: only ")"
  )

  -- subExpression~begin = lParen
  rParen = subExpression~end

  -- In this context, an expression list is an array term.
  If subExpression~isA(.Expression.List) Then
    subExpression = .Array.Term~new( subExpression )

  Call ExpectParenPair lParen, rParen

  Return .SubExpression~new( lParen, subExpression, rParen )

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, begin, lParen )

--------------------------------------------------------------------------------

::Class SubExpression Public SubClass Rexx.Term
::Attribute begin
::Attribute end
::Attribute subExpression Get
::Method init
  Expose subExpression begin end
  Use Strict Arg begin, subExpression, rParen
  end = TheElementAfter( rParen )

--------------------------------------------------------------------------------
-- Simple.Message.Term                                                        --
--   Used when parsing templates too                                          --
--------------------------------------------------------------------------------

::Routine Simple.Message.Term Public

  Use Strict Arg package, term

  Loop

    element = term~end

  If (element \< .ALL.OPS.MESSAGE_SEND) Then Return term

    Select Case element~category
      When .EL.LEFT_BRACKET Then
        term = Indexed.Or.Invoked.Term( package, term, element, "" )
      Otherwise term = Message.Term( package, term, element, "" )   -- "~", "~~"
    End

  End

--------------------------------------------------------------------------------
-- SYMBOL.TERM                                                                --
--------------------------------------------------------------------------------

::Routine Symbol.Term Public

  Use Strict Arg package, symbol, inExpose = .False

  If symbol < .ALL.COMPOUND_VARIABLES Then
    Exit Compound.Variable.Term( package, symbol, inExpose )

  If symbol < .ALL.STEM_VARIABLES     Then
    Exit     Stem.Variable.Term( package, symbol, inExpose )

  varName       = symbol~value        -- Uppercased symbol
  varSource     = symbol~source       -- As written
  thisBody      = package~currentBodies~top

  -- Change the element category of variable symbols to reflect whether
  -- they are local or instance variables. A ::CONSTANT expression is in no
  -- body (thisBody is .Nil): its variables are local to the "::CONSTANT"
  -- method that ooRexx builds for them, and nothing is exposed there.
  If symbol < .EL.SIMPLE_VARIABLE, thisBody \== .Nil Then Do
    locals        = thisBody~locals
    If \locals~hasIndex( varName ) Then locals[ varName ] = varSource
    exposed       = thisBody~exposed
    useLocal      = thisBody~useLocal
    If inExpose Then Call Exposed
    Else If \exposed~isEmpty Then Do  -- This body starts with EXPOSE
      If  exposed~hasItem( varName ) Then Call Exposed
    End
    Else If useLocal \== .Nil Then Do  -- This body starts with USE LOCAL
      -- "Note that USE LOCAL will always keep Rexx special variables RC, RESULT,
      --  SIGL, SELF, and SUPER as local variables" (rexxref).
      If WordPos(varName, "RC RESULT SIGL SELF SUPER") == 0, -
        \useLocal~hasitem( varName ) Then Call Exposed
    End
  End

  Exit .Symbol.Term~new( package, symbol )

Exposed:
  Call SetCategory symbol, .EL.EXPOSED_SIMPLE_VARIABLE
Return

--------------------------------------------------------------------------------

::Class Symbol.Term Public SubClass Rexx.Term
::Attribute symbol
::Attribute begin
::Attribute end
::Constant tree "symbol"
::Method value
  Return self~symbol
::Method init
  Expose symbol begin end

  Use Strict Arg package, symbol

  begin = symbol
  end   = TheElementAfter( symbol )

--------------------------------------------------------------------------------
-- COMPOUND.VARIABLE.TERM                                                     --
--------------------------------------------------------------------------------

::Routine Compound.Variable.Term

  Use Strict Arg package, variable, inExpose

  thisBody      = package~currentBodies~top
  -- A ::CONSTANT expression is in no body: nothing exposed, no locals.
  If thisbody == .Nil Then Do
    exposed     = .Nil
    useLocal    = .Nil
    locals      = .Nil
  End
  Else Do
    exposed     = thisBody~exposed
    useLocal    = thisBody~useLocal
    locals      = thisBody~locals
  End

  Parse Value variable~from With line col
  source        = package~source

  Parse Value variable~value  With stemName"."tail
  Parse Value variable~source With stemSource"."tailSource
  stemName = stemName"."
  stemSource = stemSource"."
  offset = Length(stemName)

  stemClass = StemClass()

  Parse Value variable~source With stemSource"."
  If locals \== .Nil, \locals~hasIndex( stemName ) Then
    locals[ stemName ] = stemSource"."

  Select Case stemClass
    When .EL.STEM_VARIABLE         Then variableClass =         .EL.COMPOUND_VARIABLE
    When .EL.EXPOSED_STEM_VARIABLE Then variableClass = .EL.EXPOSED_COMPOUND_VARIABLE
  End

  Call SetCategory variable, variableClass

  stem = .StringOrSymbol.Element~new( -
    stemClass, source, line, col, line, col+offset -
  )

  parts = .Array~of(stem) -- Parts of the tail

  col += offset
  Do While tail~contains(".")
    Parse Var tail       fragment"."tail
    Parse Var tailSource fragmentSource"."tailSource
    If fragment \== "" Then Call AddFragment
    Call AddDot
  End
  fragment       = tail
  fragmentSource = tailSource
  Call AddFragment

  term = .Compound.Variable.Term~new( variable, parts )

  variable~parts = parts

  Exit term

StemClass:
  If inExpose Then Return .EL.EXPOSED_STEM_VARIABLE
  -- If this body starts with EXPOSE
  If exposed \== .Nil, \exposed~isEmpty Then Do
    If  exposed ~hasItem( stemName ) Then Return .EL.EXPOSED_STEM_VARIABLE
  End
  Else If useLocal \ == .Nil Then Do -- This body starts with USE LOCAL
    If \useLocal~hasitem( stemName ) Then Return .EL.EXPOSED_STEM_VARIABLE
  End
  Return .EL.STEM_VARIABLE

AddDot:
  dot = .Tail.Separator.Element~new(.EL.TAIL_SEPARATOR, line, col)
  parts ~ append( dot )
  col += 1
Return

AddFragment:
  If fragment == "" Then Return
  Select
    When DataType(fragment, "Variable") Then Do
      If locals \== .Nil, \locals~hasIndex( fragment ) Then
        locals[ fragment ] = fragmentSource
      partClass = .EL.SIMPLE_VARIABLE
      Select
        When exposed == .Nil Then Nop
        When exposed~hasItem( Upper(fragment) ) Then
          partClass = .EL.EXPOSED_SIMPLE_VARIABLE
        When exposed~isEmpty Then Do
          If useLocal \== .Nil, \ useLocal~hasitem( fragment ) Then
            partClass = .EL.EXPOSED_SIMPLE_VARIABLE
        End
        Otherwise Nop
      End
    End
    When DataType(fragment, "Whole")    Then partClass = .EL.INTEGER_NUMBER
    Otherwise                                partClass = .EL.SYMBOL_LITERAL
  End
  newCol = col+Length(fragment)
  fragment = .StringOrSymbol.Element~new(      -
    partClass, source, line, col, line, newCol -
  )
  col = newCol
  parts ~ append( fragment )
Return

--------------------------------------------------------------------------------

::Class Compound.Variable.Term Public SubClass Rexx.Term
::Attribute symbol
::Attribute parts
::Attribute begin
::Attribute end
::Method init
  Expose symbol parts begin end
  Use Strict Arg symbol, parts
  begin = symbol
  end   = TheElementAfter( symbol )

--------------------------------------------------------------------------------
-- STEM.VARIABLE.TERM                                                         --
--------------------------------------------------------------------------------

::Routine Stem.Variable.Term

  Use Strict Arg package, stem, inExpose

  thisBody   = package~currentBodies~top
  newClass   = .EL.EXPOSED_STEM_VARIABLE

  stemName   = stem~value
  stemSource = stem~source

  -- A ::CONSTANT expression is in no body: nothing exposed, no locals
  -- ("::CONSTANT k (s.)" failed with 97.1).
  If thisBody == .Nil Then Exit .Stem.Variable.Term~new( stem )

  -- Special handling for EXPOSE instructions
  If inExpose Then Call SetCategory stem, newClass
  Else Do -- Not an EXPOSE instruction
    exposed       = thisBody~exposed
    useLocal      = thisBody~useLocal
    If \exposed~isEmpty        Then Do  -- This body starts with EXPOSE
      If  exposed ~hasItem( stemName ) Then Call SetCategory stem, newClass
    End
    Else If useLocal \ == .Nil Then Do  -- This body starts with USE LOCAL
      If \useLocal~hasitem( stemName ) Then Call SetCategory stem, newClass
    End
  End

  locals        = thisBody~locals
  If \locals~hasIndex( stemName ) Then locals[ stemName ] = stemSource

  Exit .Stem.Variable.Term~new( stem )

--------------------------------------------------------------------------------

::Class Stem.Variable.Term Public SubClass Rexx.Term
::Constant items 1
::Attribute begin
::Attribute end
::Attribute stem
::Method symbol
  Expose stem
  Return stem
::Method init
  Expose stem begin end
  Use Strict Arg stem
  begin = stem
  end   = TheElementAfter( stem )

--------------------------------------------------------------------------------
-- Namespace-qualified symbols                                                --
--------------------------------------------------------------------------------

::Routine Namespace.Qualified.Symbol Public
  Use Strict Arg package, nameSpace, colon

  Call SetConstantName nameSpace, .NAMESPACE.NAME

  symbol    = TheElementAfter( colon )
  If symbol \< .ALL.SYMBOLS Then Signal 20.923
  element = TheElementAfter( symbol )
  If element \< .EL.LEFT_PARENTHESIS, element \< .EL.LEFT_CURLY_BRACKET Then
    Exit .NameSpace.Qualified.ClassName~new( nameSpace, symbol )

  arguments     = .Nil
  trailingBlock = .Nil

  If element < .EL.LEFT_PARENTHESIS Then Do
    routineName  = symbol
    Call SetConstantName routineName, .EXTERNAL.PACKAGE.FUNCTION.NAME
    lParen       = element

    next = TheElementAfter( lParen )

    arguments = Arguments( package, next, .EL.RIGHT_PARENTHESIS )

    Call ExpectParenPair lParen, arguments~end
  End

  If element < .EL.LEFT_CURLY_BRACKET Then
    trailingBlock = Source.Literal( package, element, "" )

  Exit .NameSpace.Qualified.Function.Call~new(       -
    nameSpace, routineName, arguments, trailingBlock -
  )

-- Symbol expected as a name of namespace-qualified symbol.
20.923: Syntax( 20.923, colon )

--------------------------------------------------------------------------------
-- FUNCTION.CALL.TERM                                                         --
--------------------------------------------------------------------------------

::Routine Function.Call.Term Public

  Use Strict Arg package, name, left, terminators

  arguments     = .Nil
  trailingBlock = .Nil

  If left < .EL.LEFT_PARENTHESIS Then Do

    next = TheElementAfter( left )

    arguments = Arguments( package, next, .EL.RIGHT_PARENTHESIS )

    Call ExpectParenPair left, arguments~end

    left = arguments~end~next
  End

  If left < .EL.LEFT_CURLY_BRACKET Then
    trailingBlock = Source.Literal( package, left, "" )

  name~invocationType = Function

  -- A ::CONSTANT expression is in no body: its calls are kept by the
  -- package, and SecondPass resolves them with no labels, as ooRexx.
  body = package~currentBodies~top
  If body \== .Nil Then body~calledRoutineNames~append( -
    (name, arguments, trailingBlock)                   -
  )
  Else package~constantCalls~append( (name, arguments, trailingBlock) )

  Exit .Function.Call.Term~new( name, arguments, trailingBlock )

--------------------------------------------------------------------------------

::Class Function.Call.Term Public SubClass Rexx.Term
::Attribute argumentList
::Attribute trailingBlock
::Attribute begin
::Attribute end
::Attribute name
::Method init
  Expose name argumentList trailingBlock begin end
  Use Strict Arg name, argumentList, trailingBlock
  begin = name
  If trailingBlock \== .Nil Then
    end = trailingBlock~end
  Else end   = TheElementAfter( argumentList~end )

--------------------------------------------------------------------------------

::Class NameSpace.Qualified.Function.Call Public SubClass Rexx.Term
::Attribute nameSpace
::Attribute name
::Attribute arguments
::Attribute trailingBlock
::Attribute begin
::Attribute end
::Method init
  Expose nameSpace name arguments trailingBlock begin end
  Use Strict Arg nameSpace, name, arguments, trailingBlock
  begin = nameSpace
  If trailingBlock \== .Nil Then end = trailingBlock~end
  Else end = TheElementAfter(arguments~end)

--------------------------------------------------------------------------------

::Class NameSpace.Qualified.ClassName Public SubClass Rexx.Term
::Attribute nameSpace
::Attribute className
::Attribute begin
::Attribute end
::Method init
  Expose nameSpace className begin end
  Use Strict Arg nameSpace, className
  Call SetConstantName nameSpace, .NAMESPACE.NAME
  Call SetConstantName className, .CLASS.NAME
  begin = nameSpace
  end   = TheElementAfter( className )

--------------------------------------------------------------------------------
-- Indexed.Or.Invoked.                                                        --
--                                                                            --
-- Cfr. Dallas report                                                         --
--   message_term := term '['[ expression_list ] (']' | Msg36.2)              --
-- See also rexxref 5.1.0, 1.11.4. Message Terms                              --
--   message_term := receiver '[' [[expression] ,]* ']'                       --
--                                                                            --
--------------------------------------------------------------------------------

::Routine Indexed.Or.Invoked.Term Public

  Use Strict Arg package, receiver, lBracket, terminators

  -- Store parenthesis location information
  Parse Value lBracket~from With line pos

  If lBracket < .EL.LEFT_BRACKET
    Then expect = .EL.RIGHT_BRACKET
    Else expect = .EL.RIGHT_PARENTHESIS

  next = TheElementAfter( lBracket )

  arguments = Arguments( package, next, terminators || expect )

  -- Expecting a closing bracket now
  If lBracket < .EL.LEFT_BRACKET Then Do
    If arguments~end \< expect Then Signal 36.902
  End
  Else Do
    If arguments~end \< expect Then Signal 36.901
  End

  element = arguments~end~next

  trailingBlock = .Nil

  If element < .EL.LEFT_CURLY_BRACKET Then
    trailingBlock = Source.Literal( package, element, "" )

  Return .Indexed.Or.Invoked.Term~new(           -
    lBracket, receiver, arguments, trailingBlock -
  )

-- Left parenthesis "(" in position &1 on line &2 requires
-- a corresponding right parenthesis ")".
36.901: Syntax( 36.901, lBracket, pos, line)

-- Square bracket "[" in position &1 on line &2 requires
-- a corresponding right square bracket "]".
36.902: Syntax( 36.902, lBracket, pos, line)

--------------------------------------------------------------------------------

::Class Indexed.Or.Invoked.Term Public SubClass Rexx.Term
::Attribute bracket
::Attribute begin
::Attribute end
::Attribute term          Get
::Attribute arguments     Get
::Attribute trailingBlock Get
::Method init
  Expose term arguments bracket begin end trailingBlock
  Use Strict Arg bracket, term, arguments, trailingBlock
  rBracket = arguments~end
  begin    = term~begin
  If trailingBlock == .Nil Then end = TheElementAfter( rBracket )
  Else                          end = trailingBlock~end

--------------------------------------------------------------------------------
-- Message.Term                                                               --
--                                                                            --
-- Cfr. Dallas report                                                         --
--   message_term:= term ('~' | '~~') method_name [arguments]                 --
-- See also rexxref 5.1.0, 1.11.4. Message Terms                              --
--   message_term:= receiver ('~' | '~~') messagename [: symbol] [arguments]  --
--                                                                            --
--------------------------------------------------------------------------------

::Routine Message.Term Public

  Use Strict Arg package, receiver, operator, terminators = ""

  -- 'operator' is either '~' or '~~'

  messageName = TheElementAfter( operator )

  checkMessageName = 1
  scope = .Nil
  If .Options.Executor Then Do
    If messageName < (          -
      .EL.LEFT_PARENTHESIS   || -
      .EL.LEFT_CURLY_BRACKET || -
      .EL.COLON) Then
      checkMessageName = 0
  End

  If checkMessageName Then Do
    If messageName \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.909
    left = TheElementAfter( messageName )
  End
  Else Do
    left        = messageName
    messageName = .Nil
  End

  If left < .EL.COLON Then Do
    scope     = TheElementAfter( left )

    If scope \< .ALL.MESSAGE_SCOPE_ELEMENTS Then Signal 20.917

    left = TheElementAfter( scope )
  End

  arguments     = .Nil
  trailingBlock = .Nil

  If left < .EL.LEFT_CURLY_BRACKET Then Do
    trailingBlock = Source.Literal( package, left, "" )
  End
  Else If left < .EL.LEFT_PARENTHESIS Then Do

    next = TheElementAfter( left )

    arguments = Arguments( package, next, .EL.RIGHT_PARENTHESIS )

    right = arguments~end

    Call ExpectParenPair left, right

    If right~next < .EL.LEFT_CURLY_BRACKET Then Do
      trailingBlock = Source.Literal( package, right~next, "" )
    End

  End

  term = .Message.Term~new(                                            -
    receiver, operator, messageName, scope, arguments, trailingBlock -
  )

  Exit term

-- String or symbol expected after tilde (~).
19.909: Syntax( 19.909, messageName )

-- Symbol expected after superclass colon (:).
20.917: Syntax( 20.917, scope )

--------------------------------------------------------------------------------

::Class Message.Term Public SubClass Rexx.Term
::Attribute begin
::Attribute end
::Attribute term
::Attribute operator
::Attribute messageName
::Attribute scope
::Attribute arguments
::Attribute trailingBlock
::Constant tree "term operator messagename arguments"
::Method init
  Expose term operator messageName scope arguments trailingBlock begin end
  Use Strict Arg term, operator, messageName, scope, arguments, trailingBlock
  begin = term~begin

  If messageName \== .Nil Then Call SetConstantName messageName, .METHOD.NAME

  end = FindEnd()

  Return

FindEnd:
  If trailingBlock \== .Nil Then Return trailingBlock~end
  If arguments \== .Nil Then Do
    rParen = arguments~end
    Return TheElementAfter( rParen )
  End
  If scope       \== .Nil Then Return TheElementAfter( scope )
  If messageName \== .Nil Then Return TheElementAfter( messageName )

--------------------------------------------------------------------------------
-- VARIABLE.REFERENCE.TERM                                                    --
--  See rexxref 5.1.0 1.11.7. Variable Reference Term                         --
--------------------------------------------------------------------------------
::Routine Variable.Reference.Term

  Use Strict Arg package, operator

  variable     = TheElementAfter( operator )

  If variable < .ALL.REFERENCED_SYMBOLS Then
    Return .Variable.Reference.Term~new(operator, variable)

  Signal 20.930

-- Simple variable or stem symbol expected after
-- > or < prefix operator; found &1.
20.930: Syntax( 20.930, operator, variable )

--------------------------------------------------------------------------------

::Class Variable.Reference.Term Public SubClass Rexx.Term
::Attribute operator Get
::Attribute variable Get
::Attribute begin
::Attribute end
::Method init
  Expose operator variable begin end
  Use Strict Arg operator, variable
  begin = operator
  end = TheElementAfter( variable )

--------------------------------------------------------------------------------
-- ExpectParenPair                                                            --
--------------------------------------------------------------------------------

::Routine ExpectParenPair Public
  Use Strict Arg lParen, rParen
  If rParen < .EL.RIGHT_PARENTHESIS Then Return
  Parse Value lParen~from With line Pos

-- Left parenthesis "(" in position &1 on line &2 requires
-- a corresponding right parenthesis ")".
36.901: Syntax( 36.901, lParen, pos, line)