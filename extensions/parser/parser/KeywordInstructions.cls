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
/* KeywordInstructions.cls                                                    */
/* =======================                                                    */
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
/* 20241216    0.1f Mutate EL.PERIOD -> EL.PARSE_PERIOD in PARSE templates    */
/*                  Mark template target variables and USE targets            */
/*                  as "assigned"                                             */
/* 20251215    0.2  Fix setAssigned for DROP, EXPOSE, USE LOCAL and           */
/*                  PROCEDURE EXPOSE                                          */
/* 20250328         Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250416    0.2a Add argument info to called routine chain                 */
/* 20250419         Fix bug nos. 3, 6 & 7                                     */
/* 20250803    0.2d Add support for array terms                               */
/* 20250808         Add support for length positional patterns                */
/* 20251128    0.3a Add the UPPER instruction (e.g., for Executor)            */
/* 20251130         (Executor) Implement USE NAMED and USE NAMED AUTO         */
/* 20251130         (Executor) Implement FORWARD NAMEDARGUMENTS               */
/* 20251210         (Executor) Implement named arguments                      */
/* 20260521    0.5  CMS: Implement PARSE EXTERNAL and PARSE NUMERIC           */
/* 20260522         CMS: Disallow PARSE LOWER and CASELESS                    */
/* 20260522         CMS: Implement SIGNAL                                     */
/* 20260522         CMS: Disallow CALL (expr)                                 */
/* 20260522         CMS: Disallow CALL namespace:name                         */
/* 20260601    0.6  Implement a fine-grained feature system                   */
/* 20260621         CMS: Implement TRACE !                                    */
/* 20260923    0.7  ADDRESS: add redirection accessors (with,                 */
/*                  redirectionType/Target/Option)                            */
/* 20260923         CALL ON/OFF, SIGNAL OFF: add condition, userCondition     */
/*                  and (CALL ON) trapName accessors                          */
/* 20260924         CALL ON ... NAME: the clause ends at its end, not at NAME */
/* 20260924         INTERPRET option: EXPOSE (99.908), FORWARD (99.923),      */
/*                  GUARD (99.912), REPLY (99.924) and USE LOCAL (99.915)     */
/*                  are not allowed.                                          */
/* 20260924         TRACE: the sign of TRACE -n / +n is kept (sign)           */
/* 20260926         USE LOCAL: a constant symbol is 31.2 or 31.3 (was 20.927) */
/* 20260928         PARSE: a positional pattern that is not a whole number is */
/*                  an error when it runs (26.4), as in ooRexx; at parse time */
/*                  only with earlyCheck PARSE. After a prefix, any constant  */
/*                  symbol (as ooRexx); nothing after it: 38.901.             */
/*                  GUARD WHEN: an expression that refers to no variable the  */
/*                  method exposes is 99.913 (a translation error, as         */
/*                  ooRexx). Guard.Instruction~on (a getter).                 */
/* 20260929         PARSE: a template that ends in "." or a string, followed  */
/*                  by blanks and a comma, is not the last one (Template.List */
/*                  lost the templates after it: PARSE ARG a . , b).          */
/* 20260930         EXPOSE and USE LOCAL that are not the first instruction   */
/*                  of the body (a label before them too) are 99.907 and      */
/*                  99.910 before their list is parsed, as ooRexx. "(a b)" in */
/*                  a variable list is 46.1 (46.901 is a missing ")" only).   */
/*                  USE ARG: the instruction keeps the count of positions     */
/*                  (count: "USE ARG ," has one, as in ooRexx). "USE ARG a =" */
/*                  is 35.930, "USE ARG = 1" is 35.1 (were 35.1, 89.1).       */
/*                  ADDRESS VALUE with no expression is 35.914.               */
/*                  PARSE: an environment symbol as a target (".x", without   */
/*                  a message) is 31.3, as in ooRexx (it was 38.1).           */
/*                  EXPOSE: the 99.907 has a hint (issue #39) after a label,  */
/*                  after a PROCEDURE clause, and out of a method.            */
/* 20261001         ITERATE naming a SELECT or a simple DO is 28.5 only with  */
/*                  the ITERATE early check, as 28.2 and 28.4: ooRexx raises  */
/*                  it when the ITERATE runs. Without it, ~iteration is that  */
/*                  block, so that a compiler can raise the 28.5.             */
/*                  EXPOSE (x): x is exposed (the body's exposed list got the */
/*                  ")"; DROP (x) and PROCEDURE EXPOSE (x) added it too). The */
/*                  GUARD check (99.913) looks at the body's exposedSymbols,  */
/*                  without "(x)", as ooRexx.                                 */
/* 20261113         CATCH and FINALLY (RFE #31): LEAVE and ITERATE in a CATCH */
/*                  clause that refer to its block are 28.6 and 28.7 with the */
/*                  early check; LEAVE, ITERATE, RETURN, EXIT, SIGNAL and     */
/*                  FORWARD (not CONTINUE) that leave a FINALLY clause are    */
/*                  99.951 (FinallyExitCheck)                                 */
/* 20261113         USE ARG: the 46.903 is ooRexx bug #1960 (r13416)          */
/* 20261117         FINALLY, ooRexx r13535: RAISE with RETURN or EXIT, and    */
/*                  REPLY, in a FINALLY clause are 99.951; FORWARD without    */
/*                  CONTINUE records itself in the body for the 99.952 of     */
/*                  the FINALLY clause (Clauses.cls, Catch.Sections)          */
/* 20261118         THROW (RFE #31, proposed, prototype on r13540): RAISE     */
/*                  without RETURN and EXIT, with its own messages: 20.937,   */
/*                  25.940, 25.941 (also RETURN and EXIT), 35.938, 35.939.    */
/*                  Throw.Instruction, a subclass of Raise.Instruction        */
/* 20261119         Comments of 25.011 and 99.951 as ERRORTEXT has them:      */
/*                  CheckMessages.rex did not look in bin/parser/ until now   */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261122         Forward.Instruction~namedArguments works (init did not    */
/*                  expose it, and the attribute returned its own name)       */
/* 20261124         Prefixed.Template~begin works (init did not expose it,    */
/*                  and the attribute returned its own name), and it is the   */
/*                  prefix, not the element after it                          */
/* 20261124         childAttributes of the instructions; children and parent  */
/*                  of the templates                                          */
/* 20261124         Template.List, Template and Prefixed.Template inherit     */
/*                  Rexx.Node                                                 */
/*                                                                            */
/******************************************************************************/

 .environment~             Address.Instruction =             .Address.Instruction
 .environment~                 Arg.Instruction =                 .Arg.Instruction
 .environment~                Call.Instruction =                .Call.Instruction
 .environment~          Call.Value.Instruction =          .Call.Value.Instruction
 .environment~            Call.Off.Instruction =            .Call.Off.Instruction
 .environment~             Call.On.Instruction =             .Call.On.Instruction
 .environment~ NameSpace.Qualified.Call.Instruction = -
                                             .NameSpace.Qualified.Call.Instruction

 .environment~                Drop.Instruction =                .Drop.Instruction
 .environment~                Exit.Instruction =                .Exit.Instruction
 .environment~              Expose.Instruction =              .Expose.Instruction
 .environment~             Forward.Instruction =             .Forward.Instruction
 .environment~               Guard.Instruction =               .Guard.Instruction
 .environment~           Interpret.Instruction =           .Interpret.Instruction
 .environment~             Iterate.Instruction =             .Iterate.Instruction
 .environment~               Leave.Instruction =               .Leave.Instruction
 .environment~                Loop.Instruction =                .Loop.Instruction
 .environment~                 Nop.Instruction =                 .Nop.Instruction
 .environment~             Numeric.Instruction =             .Numeric.Instruction
 .environment~               Parse.Instruction =               .Parse.Instruction
 .environment~                Push.Instruction =                .Push.Instruction
 .environment~ Optional.Expression.Instruction = .Optional.Expression.Instruction
 .environment~             Options.Instruction =             .Options.Instruction
 .environment~                Pull.Instruction =                .Pull.Instruction
 .environment~           Procedure.Instruction =           .Procedure.Instruction
 .environment~               Queue.Instruction =               .Queue.Instruction
 .environment~               Raise.Instruction =               .Raise.Instruction
 .environment~               Throw.Instruction =               .Throw.Instruction
 .environment~               Reply.Instruction =               .Reply.Instruction
 .environment~              Return.Instruction =              .Return.Instruction
 .environment~                 Say.Instruction =                 .Say.Instruction
 .environment~              Signal.Instruction =              .Signal.Instruction
 .environment~        Signal.Value.Instruction =        .Signal.Value.Instruction
 .environment~          Signal.Off.Instruction =          .Signal.Off.Instruction
 .environment~           Signal.On.Instruction =           .Signal.On.Instruction
 .environment~               Trace.Instruction =               .Trace.Instruction
 .environment~               Upper.Instruction =               .Upper.Instruction
 .environment~             Use.Arg.Instruction =             .Use.Arg.Instruction
 .environment~           Use.Local.Instruction =           .Use.Local.Instruction

 .environment~                        Template =                        .Template
 .environment~                   Template.List =                   .Template.List
 .environment~          Optional.Template.List =          .Optional.Template.List
 .environment~               Prefixed.Template =               .Prefixed.Template

 .environment~                  Trap.Arguments =                  .Trap.Arguments

::Requires "BaseClassesAndRoutines.cls"
::Requires "Expressions.cls"
::Requires "Globals.cls"

/******************************************************************************/
/* ADDRESS instruction                                                        */
/******************************************************************************/

::Routine Address.Instruction Public
  Use Strict Arg package, begin, end, elements

  ADDRESS = elements[1]

  element        = TheElementAfter( ADDRESS )

  options.     = .Nil
  ignoreBlanks = .True

  If element < .EL.END_OF_CLAUSE Then Signal Done

  If element \< .ALL.SYMBOLS_AND_STRINGS Then Signal AfterValue
  If element < .ALL.SYMBOLS, element~value == "VALUE" Then Signal (Value)
  Call SetConstantName element, .ENVIRONMENT.NAME

  options.environment = element

  element = TheElementAfter( element, ignoreBlanks )
  If element < .EL.END_OF_CLAUSE Then Signal Done
  If element < .ALL.SYMBOLS, .Features.Address.With, -
    element~value == "WITH" Then Signal With

  Call Prepare

  expression = Expression.List( package, element )
  options.["EXPRESSION"] = expression
  element = expression~end
  If element < .EL.END_OF_CLAUSE Then Signal Done
  Signal With

Value:
  Call SetSubkeyword element
  element = TheElementAfter( element )
  Call Prepare
  -- No expression after VALUE (the end, or WITH): 35.914, as ooRexx
  If element < .EL.END_OF_CLAUSE | element < .EL.SUBKEYWORD Then Signal 35.914
  Signal ValueExpression
AfterValue:
  Call Prepare
ValueExpression:
  options.env_expression = Expression.List( package, element )
  element = options.env_expression~end
  If element < .EL.END_OF_CLAUSE Then Signal Done
  Signal With

Prepare:
  If .Features.Address.With Then Call PrepareExpression package, element, "WITH"
  Else                           Call PrepareExpression package, element
Return

With:
  seen. = 0
  duplicate.input  = 25.930
  duplicate.output = 25.931
  duplicate.error  = 25.932
  Call SetSubkeyword element -- Consume "WITH"
  options.with     = 1
  element = TheElementAfter( element )
  If element < .EL.END_OF_CLAUSE Then Signal 20.933
  Loop
    If element \< .ALL.SYMBOLS_AND_KEYWORDS Then Signal 25.934
    tValue = element~value
    If WordPos(tValue, "INPUT OUTPUT ERROR") == 0 Then Signal 25.934
    If seen.tValue Then Signal (duplicate.tValue)
    seen.tValue    = 1
    options.tValue = 1
    Call SetSubkeyword element
    element = TheElementAfter( element )
    If element \< .ALL.SYMBOLS Then Signal 25.933
    If tValue \== "INPUT" Then Do
      replaceOrAppend = element~value
      If WordPos(replaceOrAppend, "REPLACE APPEND") > 0 Then Do
        Call SetSubkeyword element
        options.[tValue"_"replaceOrAppend] = 1
        element = TheElementAfter( element )
        If element \< .ALL.SYMBOLS Then Signal 25.933
      End
    End
    IOType = element~value
    If WordPos(IOType, "NORMAL STEM STREAM USING") == 0 Then Signal 25.933
    Call SetSubkeyword element
    eValue = element~value
    Select Case element~value
      When "NORMAL" Then options.[tValue"_"normal] = 1
      When "STEM"   Then Do
        element = TheElementAfter( element )
        If element \< .EL.STEM_VARIABLE Then Signal 20.932
        element~setAssigned
        options.[tValue"_"eValue] = element
      End
      When "STREAM", "USING" Then Do
        next = TheElementAfter( element )
        If next < .EL.END_OF_CLAUSE Then Signal 35.935
        element = next
        If element < .ALL.CONSTANT_SYMBOLS_AND_STRINGS Then
          options.[tValue"_"eValue] = element
        Else Do
          If element \< .EL.LEFT_PARENTHESIS Then Signal 35.001
          Call prepareExpression package, element, "INPUT OUTPUT ERROR"
          subExpression = SubExpression( package, element )
          options.[tValue"_"eValue] = subExpression
          element = subExpression~end
          If element < .EL.END_OF_CLAUSE Then Signal Done
          Iterate
        End
      End
    End
    element = TheElementAfter( element, ignoreBlanks )
    If element < .EL.END_OF_CLAUSE Then Signal Done
  End

Done:
  Return .Address.Instruction~new( package, begin, end, options. )

-- Stem symbol expected after STEM keyword.
20.932: Syntax( 20.932, ADDRESS )

-- Symbol expected after WITH keyword.
20.933: Syntax( 20.933, ADDRESS )

-- Duplicate INPUT keyword found.
25.930: Syntax( 25.930, ADDRESS )

-- Duplicate OUTPUT keyword found.
25.931: Syntax( 25.931, ADDRESS )

-- Duplicate ERROR keyword found.
25.932: Syntax( 25.932, ADDRESS )

-- Acceptable ADDRESS WITH I/O types are NORMAL, STEM,
-- STREAM, and USING; found "&1".
25.933: Syntax( 25.933, ADDRESS, element )

-- WITH must be followed by one of the keywords
-- INPUT, OUTPUT, or ERROR; found "&1".
25.934: Syntax( 25.934, ADDRESS, element )

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, ADDRESS, element )

-- Missing expression following VALUE keyword of an ADDRESS instruction.
35.914: Syntax( 35.914, ADDRESS )

-- Missing expression following &1 keyword of a &2 instruction.
35.935: Syntax( 35.935, ADDRESS, element, "ADDRESS" )

--------------------------------------------------------------------------------

::Class Address.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "ENV_EXPRESSION EXPRESSION"
::Attribute begin          Get
::Attribute end            Get
::Attribute environment    Get
::Attribute expression     Get
::Attribute env_expression Get
::Attribute options.       Get

::Method init
  Expose begin end environment expression env_expression options.
  Use Strict Arg package, begin, end, options.
  self~init:super(package, begin, end)
  If options.~hasIndex(environment)
    Then environment     = options.environment
    Else environment     = .Nil
  If options.~hasIndex(expression)
    Then expression      = options.expression
    Else expression      = .Nil
  If options.~hasIndex(env_expression)
    Then env_expression  = options.env_expression
    Else env_expression  = .Nil

--------------------------------------------------------------------------------
-- Redirection accessors (ADDRESS ... WITH ...)
--
-- 'stream' is one of INPUT, OUTPUT or ERROR (case does not matter).
--
--   with                     -- .True when the instruction has a WITH clause.
--   redirectionType(stream)  -- "NORMAL", "STEM", "STREAM" or "USING" when the
--                               stream was specified, "" when it was not.
--   redirectionTarget(stream)-- The target: a stem element (STEM), a string or
--                               constant symbol element, or a subexpression
--                               (STREAM and USING). .Nil for NORMAL and for a
--                               stream that was not specified.
--   redirectionOption(stream)-- "APPEND" or "REPLACE" when specified (OUTPUT
--                               and ERROR only), "" otherwise.
--------------------------------------------------------------------------------

::Method with
  Expose options.
  Return options.~hasIndex("WITH")

::Method redirectionType
  Expose options.
  Use Strict Arg stream
  stream = Upper(stream)
  Do type Over .Array~of("NORMAL", "STEM", "STREAM", "USING")
    If options.~hasIndex(stream"_"type) Then Return type
  End
  Return ""

::Method redirectionTarget
  Expose options.
  Use Strict Arg stream
  stream = Upper(stream)
  type   = self~redirectionType(stream)
  If type == "" | type == "NORMAL" Then Return .Nil
  Return options.[stream"_"type]

::Method redirectionOption
  Expose options.
  Use Strict Arg stream
  stream = Upper(stream)
  If options.~hasIndex(stream"_APPEND")  Then Return "APPEND"
  If options.~hasIndex(stream"_REPLACE") Then Return "REPLACE"
  Return ""

/******************************************************************************/
/* ARG instruction                                                            */
/******************************************************************************/

::Routine Arg.Instruction Public
  Use Strict Arg package, begin, end, elements
  Return Optional.Template.List(                   -
    package, .Arg.Instruction, begin, end, elements  -
  )

--------------------------------------------------------------------------------

::Class Arg.Instruction Public SubClass Optional.Template.List
::Constant instructionName "ARG"

/******************************************************************************/
/* CALL instruction                                                           */
/******************************************************************************/

::Routine Call.Instruction Public
  Use Strict Arg package, begin, end, elements

  CALL = elements[1]

  ignoreBlanks = .True
  expression   = .Nil

  element = TheElementAfter( CALL )

  If element < .ALL.VAR_SYMBOLS, WordPos(element~value, "ON OFF") > 0 Then Do
    If element~value == "ON"
      Then Return Call.On.Instruction(  package, begin, end, elements )
      Else Return Call.Off.Instruction( package, begin, end, elements )
  End

  If element < .EL.LEFT_PARENTHESIS, .Features.Evaluated.Call Then Signal GetExpression
  If element \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.002

  nameSpace = .Nil
  name = element
  If element < .ALL.SYMBOLS Then Do
    next = TheElementAfter( element, ignoreBlanks )
    If .Features.Namespace.Qualified.Symbols, next < .EL.COLON Then Do
      nameSpace = element
      name = TheElementAfter( next, ignoreBlanks )
      If name \< .ALL.SYMBOLS Then Signal 20.922
      element = TheElementAfter( name, ignoreBlanks )
    End
    Else element = next
  End
  Else element = TheElementAfter( name, ignoreBlanks )
  Signal ProcessArguments

GetExpression:
  Parse Value element~from With line col
  Call PrepareExpression package, element, ""
  element = TheElementAfter( element, ignoreBlanks )
  expression = Or.Expression( package, element, .EL.RIGHT_PARENTHESIS )
  rParen = expression~end
  If rParen \< .EL.RIGHT_PARENTHESIS Then Signal 36.901
  element = TheElementAfter( rParen, ignoreBlanks ) -- Consume the RPAREN
  -- prepareExpression above may have promoted whitespace after the
  -- closing ")" to a blank operator.
  If element < .EL.OP.BLANK Then Do
    c = element~source
    If c == "-" | c == "," Then
      Call SetCategory element, .EL.CONTINUATION       -- Demote, ..
    Else
      Call SetCategory element, .EL.WHITESPACE         -- Demote, ..
    element~ignorable = 1                              -- .. make ignorable..
    element = TheElementAfter( element, ignoreBlanks ) -- .. and skip.
  End
  Signal ProcessArguments

ProcessArguments:

  Call PrepareExpression package, element, ""

  arguments = Arguments( package, element, .EL.END_OF_CLAUSE )

  element = arguments~end
  If expression~isNil Then Do
    If nameSpace == .Nil Then Do
      name~invocationType = "CALL"
      package~currentBodies~top~calledRoutineNames~append( (name, arguments) )
      -- We don't need to assign a subcategory here, because we
      -- will assign a finer subcategory in the second pass.
      Return .Call.Instruction~new( package, begin, element, name, arguments )
    End
    Else Do
      Call SetConstantName nameSpace, .NAMESPACE.NAME
      Call SetConstantName name     , .EXTERNAL.PACKAGE.SUBROUTINE.NAME
      Return .NameSpace.Qualified.Call.Instruction~new(      -
        package, begin, element, nameSpace, name, arguments -
      )
    End
  End
  Else
    Return .Call.Value.Instruction~new(           -
      package, begin, element, expression, arguments -
    )

-- String or symbol expected after CALL keyword.
19.002: Syntax( 19.002, CALL )

-- Symbol expected as a routine name of qualified CALL instruction.
20.922: Syntax( 20.922, CALL, name )

-- Left parenthesis "(" in position &1 on line &2
-- requires a corresponding right parenthesis ")".
36.901: Syntax( 36.901, CALL, col, line )

--  A simple call, i.e. call name args                                        --

::Class Call.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "ARGUMENTS"
::Attribute begin     Get
::Attribute end       Get
::Attribute name      Get
::Attribute arguments Get
::Method init
  Expose begin end name arguments
  Use Strict Arg package, begin, end, name, arguments
  self~init:super(package, begin, end)

-- A namespace-qualified call, i.e. call namespace:name args

::Class NameSpace.Qualified.Call.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "ARGUMENTS"
::Attribute nameSpace
::Attribute name
::Attribute arguments
::Method init
  Expose nameSpace name arguments
  Use Strict Arg package, begin, end, nameSpace , name, arguments
  self~init:super(package, begin, end)

-- Call (expression) args                                                     --

::Class Call.Value.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPRESSION ARGUMENTS"
::Attribute expression
::Attribute arguments
::Method init
  Expose expression arguments
  Use Strict Arg package, begin, end, expression, arguments
  self~init:super( package, begin, end )

/******************************************************************************/

-- Common constant for CALL ON & OFF
::Routine CallOnOffConditions
  Return "ERROR FAILURE HALT NOTREADY USER ANY "

/******************************************************************************/
/* Call Off instruction                                                       */
/******************************************************************************/

::Routine Call.Off.Instruction Public
  Use Strict Arg package, begin, end, elements

  OFF = elements[2]

  Call SetSubkeyword OFF

  ignoreBlanks = .True

  condition = TheElementAfter( OFF )

  If condition \< .ALL.SYMBOLS Then Signal 20.912

  name = condition~value

  If WordPos(name, CallOnOffConditions() ) == 0 Then Signal 25.002

  Call SetSubkeyword condition

  userCondition = .Nil

  If name == "USER" Then Do
    userCondition = TheElementAfter( condition, ignoreBlanks )
    If userCondition \< .ALL.SYMBOLS Then Signal 20.915
    Call SetConstantName userCondition, .USER.CONDITION.NAME
    element = TheElementAfter( userCondition, ignoreBlanks )
  End
  Else element = TheElementAfter( condition, ignoreBlanks )

  If element \< .EL.END_OF_CLAUSE Then Signal 21.904

  Return .Call.Off.Instruction~new(        -
    package, begin, element, condition, userCondition -
  )

-- Data must not follow the condition name; found "&1".
21.904: Syntax( 21.904, OFF, element )

-- Symbol expected after OFF keyword.
20.912: Syntax( 20.912, OFF )

-- Symbol expected after USER keyword.
20.915: Syntax( 20.915, OFF )

-- CALL OFF must be followed by one of the keywords ERROR, FAILURE,
-- HALT, NOTREADY, USER, or ANY; found "&1".
25.002: Syntax( 25.002, OFF, condition )

--------------------------------------------------------------------------------

::Class Call.Off.Instruction Public SubClass Rexx.Instruction
::Attribute condition     Get  -- the condition keyword element (ERROR, ANY...)
::Attribute userCondition Get  -- the USER condition name element, or .Nil
::Method init
  Expose condition userCondition
  Use Strict Arg package, begin, end, condition, userCondition = .Nil
  self~init:super( package, begin, end )

/******************************************************************************/
/* CALL ON instruction                                                        */
/******************************************************************************/

-- Special class to identify the arguments to a trap specified in
-- a CALL ON instruction
::Class Trap.Arguments Public
::Method Arg
  Arg n, option +1
  If Arg() == 0 Then Return 1
  If option == "E" Then Return n == 1
  Return n \== 1

::Routine Call.On.Instruction Public
  Use Strict Arg package, begin, end, elements

  ON = elements[2]

  Call SetSubkeyword ON

  ignoreBlanks = .True

  condition = TheElementAfter( ON )

  If condition \< .ALL.SYMBOLS Then Signal 20.911

  name = condition~value

  If WordPos(name, CallOnOffConditions() ) == 0 Then Signal 25.001

  Call SetSubkeyword condition

  userCondition = .Nil

  If name == "USER" Then Do
    userCondition = TheElementAfter( condition, ignoreBlanks )
    If userCondition \< .ALL.SYMBOLS Then Signal 20.915
    Call SetConstantName userCondition, .USER.CONDITION.NAME
    element = TheElementAfter( userCondition, ignoreBlanks )
  End
  Else element = TheElementAfter( condition, ignoreBlanks )

  If element < .EL.END_OF_CLAUSE Then
    Exit .Call.On.Instruction~new(                    -
      package, begin, element, condition, userCondition -
    )

  If \( element < .ALL.VAR_SYMBOLS ) Then Signal 25.914
  If element~value \== "NAME"   Then Signal 25.914

  Call SetSubkeyword element

  trapName = TheElementAfter( element )

  If trapName \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.003

  trapName~invocationType = "CALL"

  package~currentBodies~top~calledRoutineNames~append( -
    (trapName, .Trap.Arguments~new)              -
  )

  end = TheElementAfter( trapName, ignoreBlanks )

  If end \< .EL.END_OF_CLAUSE Then Signal 21.903

  Return .Call.On.Instruction~new(-
    package, begin, end, condition, userCondition, trapName -
  )

-- String or symbol expected after NAME keyword.
19.003: Syntax( 19.003, ON )

-- Data must not follow the NAME trap name; found "&1".
21.903: Syntax( 21.903, ON, end )

-- Symbol expected after ON keyword.
20.911: Syntax( 20.911, ON )

-- Symbol expected after USER keyword.
20.915: Syntax( 20.915, ON )

-- CALL ON must be followed by one of the keywords ERROR, FAILURE, HALT,
-- NOTREADY, USER, or ANY; found "&1".
25.001: Syntax( 25.001, ON, condition )

-- CALL ON condition must be followed by the keyword NAME; found "&1".
25.914: Syntax( 25.914, ON, element )

--------------------------------------------------------------------------------

::Class Call.On.Instruction Public SubClass Rexx.Instruction
::Attribute condition     Get  -- the condition keyword element (ERROR, ANY...)
::Attribute userCondition Get  -- the USER condition name element, or .Nil
::Attribute trapName      Get  -- the NAME element (symbol or string), or .Nil
::Method init
  Expose condition userCondition trapName
  Use Strict Arg package, begin, end, condition, -
    userCondition = .Nil, trapName = .Nil
  self~init:super( package, begin, end )

/******************************************************************************/
/* DROP instruction                                                           */
/******************************************************************************/

::Routine Drop.Instruction Public
  Use Strict Arg package, begin, end, elements

  Return Name.List.Instruction( package, .Drop.Instruction, begin, end, elements )

--------------------------------------------------------------------------------
--   nameList format: a list of valid symbols; a parenthesized symbol is      --
--   stored as a 1-element array.                                             --
--------------------------------------------------------------------------------

::Class Drop.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "NAMELIST"
::Attribute nameList
::Method init
  Expose nameList
  Use Strict Arg package, begin, end, nameList
  self~init:super( package, begin, end )

/******************************************************************************/
/* EXIT instruction                                                           */
/******************************************************************************/

::Routine Exit.Instruction Public
  Use Strict Arg package, begin, end, elements
  instruction = Optional.Expression.Instruction( -
    package, .Exit.Instruction, begin, end, elements  -
  )
  -- EXIT must not leave a FINALLY clause (RFE #31)
  Call FinallyExitCheck package, elements[1], "EXIT"
  Return instruction

--------------------------------------------------------------------------------

::Class Exit.Instruction Public SubClass Optional.Expression.Instruction
::Constant instructionName "EXIT"
::Method init
  Use Strict Arg package, begin, end, expression = .Nil
  self~init:super( package, begin, end, expression )
  self~terminating = .True

/******************************************************************************/
/* EXPOSE instruction                                                         */
/******************************************************************************/

::Routine Expose.Instruction Public
  Use Strict Arg package, begin, end, elements

  -- INTERPRET data must not contain EXPOSE.
  If .Options.Interpret Then Signal 99.908

  -- Not the first instruction (a label before it too): ooRexx checks that
  -- before the list.
  If package~sourceLiteralStack~isEmpty, -
    \ package~preClauser~firstInBody Then Signal 99.907

  Return Name.List.Instruction( package, .Expose.Instruction, begin, end, elements )

-- INTERPRET data must not contain EXPOSE.
99.908: Syntax( 99.908, elements[1] )

-- EXPOSE must be the first instruction executed after a method invocation.
99.907:
  hint = ExposeHint( package, elements[1] )
  If hint == "" Then Syntax( 99.907, elements[1] )
  SyntaxWithHint( hint, 99.907, elements[1] )

--------------------------------------------------------------------------------
-- ExposeHint: what an EXPOSE that is not the first instruction (99.907)      --
-- probably meant, from the clause before it (issue #39):                     --
--                                                                            --
--   - a label ("a: expose x", or the label alone on the line before): in a   --
--     method, the label is a clause; out of a method, PROCEDURE EXPOSE;      --
--   - a PROCEDURE clause with nothing more ("procedure" and "expose x" in    --
--     two clauses): PROCEDURE EXPOSE, in the same clause;                    --
--   - anything else, out of a method: PROCEDURE EXPOSE.                      --
--                                                                            --
-- Returns "" when there is no hint (a method body with instructions before   --
-- the EXPOSE: the ooRexx message says it all).                               --
--------------------------------------------------------------------------------

::Routine ExposeHint
  Use Strict Arg package, EXPOSE

  inAMethod = package~currentBodies~top~isAMethodBody

  -- The last element of the clause before
  last = PreviousClauseEnd( EXPOSE )

  Select
    When last == .Nil Then Nop
    When last < .EL.COLON Then Do
      label = PreviousClauseEnd( last )
      If inAMethod Then Return -
        "A label is a clause: EXPOSE must come before any label in a method."
      Return "Out of a method, variables are exposed with PROCEDURE EXPOSE" -
        "(for example, """label~source": PROCEDURE EXPOSE ..."")."
    End
    When last < .EL.KEYWORD, last~value == "PROCEDURE" Then Do
      before = PreviousClauseEnd( last, 0 )
      If before == .Nil | before < .EL.END_OF_CLAUSE Then Return -
        "PROCEDURE and EXPOSE are two instructions here; to expose variables" -
        "to a procedure, write PROCEDURE EXPOSE in the same clause."
    End
    Otherwise Nop
  End

  If inAMethod Then Return ""
  Return "EXPOSE is only valid in a method; in an internal routine," -
    "variables are exposed with PROCEDURE EXPOSE."

--------------------------------------------------------------------------------
-- PreviousClauseEnd: the element before another one, skipping ignorable      --
-- elements; with skipEnds (the default), ends of clause too.                 --
-- .Nil when there is none.                                                   --
--------------------------------------------------------------------------------

::Routine PreviousClauseEnd
  Use Strict Arg element, skipEnds = 1
  element = element~prev
  Loop While element \== .Nil
    Select
      When element~ignorable                   Then Nop
      When skipEnds, element < .EL.END_OF_CLAUSE Then Nop
      Otherwise Leave
    End
    element = element~prev
  End
  Return element

--------------------------------------------------------------------------------
--   exposed format: a list of valid symbols; a parenthesized symbol is       --
--   stored as a 1-element array.                                             --
--------------------------------------------------------------------------------

::Class Expose.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPOSED"
::Attribute exposed
::Method init
  Expose exposed
  Use Strict Arg package, begin, end, exposed
  self~init:super( package, begin, end)

/******************************************************************************/
/* FORWARD instruction                                                        */
/******************************************************************************/

::Routine Forward.Instruction Public
  Use Strict Arg package, begin, end, elements

  FORWARD = elements[1]

  -- INTERPRET data must not contain FORWARD.
  If .Options.Interpret Then Signal 99.923

  seen.         = 0
  options.      = 0
  ignoreBlanks  = .True

  element = TheElementAfter( FORWARD )
  keywords = "CONTINUE ARGUMENTS ARRAY MESSAGE CLASS TO"
  If .Options.Executor Then keywords ||= " NAMEDARGUMENTS"

  duplicate.to             = 25.917
  duplicate.arguments      = 25.918
  duplicate.namedarguments = 25.900
  duplicate.array          = 25.918
  duplicate.continue       = 25.919
  duplicate.class          = 25.921
  duplicate.message        = 25.922

  missing.array            = 35.924
  missing.to               = 35.925
  missing.arguments        = 35.926
  missing.namedarguments   = 35.900
  missing.message          = 35.927
  missing.class            = 35.928

  Loop
    If element  < .EL.END_OF_CLAUSE       Then Signal Done
    If element \< .ALL.VARIABLES_AND_KEYWORDS Then Signal 25.916
    tValue = element~value
    If WordPos( tValue, keywords ) == 0 Then Signal 25.916
    Call SetSubkeyword element
    element = TheElementAfter( element )

    If seen.tValue Then Signal (duplicate.tValue)
    seen.tValue = 1
    If tValue = "ARGUMENTS" then seen.array     = 1
    If tValue = "ARRAY"     then seen.arguments = 1

    If tValue == "CONTINUE" Then Do
      options.continue = 1
      Iterate
    End

    If element < .EL.END_OF_CLAUSE Then Signal (missing.tValue)

    Select Case tValue
      When "ARRAY" Then Do
        If element \< .EL.LEFT_PARENTHESIS Then Signal 35.924
        next = TheElementAfter( element )
        Call PrepareExpression package, element, keywords
        expression = Arguments( package, next, .EL.RIGHT_PARENTHESIS )
        element = TheElementAfter(expression~end)
        options.tValue = expression
      End
      Otherwise Do -- Includes ARGUMENTS and NAMEDARGUMENTS
        If element < .ALL.CONSTANT_SYMBOLS_AND_STRINGS Then Do
          options.tValue = element
          element = TheElementAfter( element, ignoreBlanks )
        End
        Else Do
          If element \< .EL.LEFT_PARENTHESIS Then Signal 35.001
          Call PrepareExpression package, element, keywords
          subExpression = SubExpression( package, element )
          options.tValue = subExpression
          element = subExpression~end
        End
      End
    End
  End

Done:

  -- FORWARD ends the method, as RETURN does (but not FORWARD CONTINUE):
  -- it must not leave a FINALLY clause (RFE #31)
  -- Nor be in a block that has one (ooRexx r13535): the FINALLY clause comes
  -- later, so it is the one that checks (99.952; Clauses.cls, Catch.Sections)
  If \options.continue Then Do
    Call FinallyExitCheck package, FORWARD, "FORWARD"
    package~currentBodies~top~local~lastForward = FORWARD
  End

  Return .Forward.Instruction~new( package, begin, end, options. )

-- Duplicate NAMEDARGUMENTS keyword found.
25.900: Syntax( 25.900, FORWARD, "Duplicate NAMEDARGUMENTS keyword found" )

-- Unknown keyword on FORWARD instruction; found "&1".
25.916: Syntax( 25.916, FORWARD, element )

-- Duplicate TO keyword found.
25.917: Syntax( 25.917, FORWARD )

-- Duplicate ARGUMENTS or ARRAY keyword found.
25.918: Syntax( 25.918, FORWARD )

-- Duplicate RETURN or CONTINUE keyword found.
25.919: Syntax( 25.919, FORWARD )

-- Duplicate CLASS keyword found.
25.921: Syntax( 25.921, FORWARD )

-- Duplicate MESSAGE keyword found.
25.922: Syntax( 25.922, FORWARD )

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, FORWARD, element )

-- Missing expression following NAMEDARGUMENTS keyword of a FORWARD instruction.
35.900: Syntax( 35.900, FORWARD, "Missing expression following" -
  "NAMEDARGUMENTS keyword of a FORWARD instruction"             -
)

-- Missing "(" on expression list of the ARRAY keyword.
35.924: Syntax( 35.924, FORWARD )

-- Missing expression following TO keyword of a FORWARD instruction.
35.925: Syntax( 35.925, FORWARD )

-- Missing expression following ARGUMENTS keyword of a FORWARD instruction.
35.926: Syntax( 35.926, FORWARD )

-- Missing expression following MESSAGE keyword of a FORWARD instruction.
35.927: Syntax( 35.927, FORWARD )

-- Missing expression following CLASS keyword of a FORWARD instruction.
35.928: Syntax( 35.928, FORWARD )

-- INTERPRET data must not contain FORWARD.
99.923: Syntax( 99.923, FORWARD )

--------------------------------------------------------------------------------

::Class Forward.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "TO THECLASS MESSAGE ARGUMENTS ARRAY NAMEDARGUMENTS"
::Attribute Continue       Get
::Attribute Arguments      Get
::Attribute NamedArguments Get
::Attribute Array          Get
::Attribute Message        Get
::Attribute theClass       Get
::Attribute To             Get
::Method init
  Expose Continue Arguments NamedArguments Array Message theClass To

  Use Strict Arg package, begin, end, options.

  If options.~hasIndex("CONTINUE")  Then Continue  = options.continue
  Else                                   Continue  = .Nil
  If options.~hasIndex("ARGUMENTS") Then Arguments = options.arguments
  Else                                   Arguments = .Nil
  If options.~hasIndex("NAMEDARGUMENTS") Then
    NamedArguments = options.namedArguments
  Else                                   NamedArguments = .Nil
  If options.~hasIndex("ARRAY")     Then Array     = options.array
  Else                                   Array     = .Nil
  If options.~hasIndex("MESSAGE")   Then Message   = options.message
  Else                                   Message   = .Nil
  If options.~hasIndex("CLASS")     Then theClass  = options.class
  Else                                   theClass  = .Nil
  If options.~hasIndex("TO")        Then To        = options.to
  Else                                   To        = .Nil

  self~init:super( package, begin, end )

/******************************************************************************/
/* GUARD instruction                                                          */
/******************************************************************************/

::Routine Guard.Instruction Public
  Use Strict Arg package, begin, end, elements

  GUARD = elements[1]

  -- INTERPRET data must not contain GUARD.
  If .Options.Interpret Then Signal 99.912

  keyword = TheElementAfter( GUARD )

  If keyword \< .ALL.VAR_SYMBOLS            Then Signal 25.913
  If WordPos( keyword~value, "ON OFF") == 0 Then Signal 25.913

  onOff = keyword~value == "ON"

  Call SetSubkeyword keyword

  when = TheElementAfter( keyword )

  If when < .EL.END_OF_CLAUSE Then
    Return .Guard.Instruction~new( package, begin, end, onOff )

  If when \< .ALL.VAR_SYMBOLS Then Signal 25.912
  If when~value \== "WHEN"    Then Signal 25.912

  Call SetSubkeyword when

  element = TheElementAfter( when )

  Call PrepareExpression package, element
  expression = Expression.List( package, element, , "logical")

  If \GuardRefersToExposed( package, element ) Then Signal 99.913

  end = expression~end

  Return .Guard.Instruction~new( package, begin, end, onOff, expression )

-- GUARD ON or GUARD OFF must be followed by the keyword WHEN; found "&1".
25.912: Syntax( 25.912, GUARD, when )

-- GUARD must be followed by the keyword ON or OFF; found "&1".
25.913: Syntax( 25.913, GUARD, keyword )

-- INTERPRET data must not contain GUARD.
99.912: Syntax( 99.912, GUARD )

-- GUARD instruction did not include references to exposed variables.
99.913: Syntax( 99.913, GUARD )

--------------------------------------------------------------------------------

::Class Guard.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPRESSION"
::Attribute on         Get -- 1 for GUARD ON, 0 for GUARD OFF
::Attribute expression
::Method init
  Expose on expression
  Use Strict Arg package, begin, end, on, expression = .Nil
  self~init:super( package, begin, end )

--------------------------------------------------------------------------------
-- GuardRefersToExposed: whether the GUARD WHEN expression that starts at
-- "first" refers to a variable the method exposes, as ooRexx's
-- captureGuardVariable and isExposed. The references are the simple
-- variables and the stems, also those of a compound (its stem, and its
-- tails that are variables). A variable is exposed when the body starts
-- with an EXPOSE that names it (a symbol of the list; not a "(list)"), or
-- with a USE LOCAL that does not, and it is not a special variable.
-- ooRexx caches the compounds, and it does not look at a compound it has
-- seen before in the same code: "x = a.b; GUARD ON WHEN a.b" is 99.913
-- even when A. is exposed. That is a bug; we do not reproduce it.
--------------------------------------------------------------------------------

::Routine GuardRefersToExposed
  Use Strict Arg package, first

  body     = package~currentBodies~top
  exposed  = body~exposed
  symbols  = body~exposedSymbols   -- not "(x)": ooRexx does not see them
  useLocal = body~useLocal
  If exposed~isEmpty, useLocal == .Nil Then Return .False

  element = first
  Loop Until element == .Nil | element < .EL.END_OF_CLAUSE
    Select
      When element < .ALL.SIMPLE_VARIABLES | -
           element < .ALL.STEM_VARIABLES Then
        If IsExposed( element~value ) Then Return .True
      When element < .ALL.COMPOUND_VARIABLES Then Do
        Parse Upper Value element~value With stem "." tails
        If IsExposed( stem"." ) Then Return .True
        Do While tails \== ""
          Parse Var tails tail "." tails
          If tail == "" | Pos( Left( tail, 1 ), "0123456789" ) > 0 Then
            Iterate
          If IsExposed( tail ) Then Return .True
        End
      End
      Otherwise Nop
    End
    element = element~next
  End
  Return .False

IsExposed:
  If \exposed~isEmpty Then Return symbols~hasItem( Arg(1) )
  If WordPos( Arg(1), "RC RESULT SIGL SELF SUPER" ) > 0 Then Return .False
  Return \useLocal~hasItem( Arg(1) )

/******************************************************************************/
/* INTERPRET instruction                                                      */
/******************************************************************************/

::Routine Interpret.Instruction Public
  Use Strict Arg package, begin, end, elements

  INTERPRET = elements[1]

  If elements~items == 1 Then Signal 35.912

  element = elements[2]

  Call PrepareExpression package, element

  expression = Expression.List( package, element, .EL.END_OF_CLAUSE )

  Return .Interpret.Instruction~new( package, begin, end, expression )

-- Missing expression following INTERPRET keyword.
35.912: Syntax( 35.912, INTERPRET )

--------------------------------------------------------------------------------

::Class Interpret.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPRESSION"
::Attribute expression Get
::Method init
  Expose expression
  Use Strict Arg package, begin, end, expression
  self~init:super( package, begin, end )

/******************************************************************************/
/* ITERATE instruction                                                        */
/******************************************************************************/

::Routine Iterate.Instruction Public

  Use Strict Arg package, begin, end, elements

  ITERATE = elements[1]

  element = TheElementAfter( ITERATE )

  blockLabel       = .Nil
  blockInstruction = .Nil
  ignoreBlanks     = .True
  earlyCheck = .Parser.Options~earlyCheck
  earlyCheck = earlyCheck~hasItem( "ITERATE" )

  -- No more elements? Calculate the ITERATE target
  --
  -- See RexxRef 2.12, ITERATE: "If name is not specified,
  --   ITERATE continues with the current repetitive loop"
  If element < .EL.END_OF_CLAUSE Then Do
    blockInstructionsStack = package~blockInstructionsStacks~top
    owner = .Nil -- The block of the CATCH clause we are in (RFE #31)
    Do i = blockInstructionsStack~items To 1 By -1
      instruction = blockInstructionsStack[i]
      -- A FINALLY clause cannot be left (RFE #31)
      If instruction~isA(.Finally.Clause) Then Signal 99.951
      -- The block of a CATCH clause has already ended (RFE #31)
      If instruction~isA(.Catch.Clause) Then Do
        owner = instruction~owner
        Iterate
      End
      -- A SELECT instruction is not a loop
      If instruction~isA(.Select.Clause) Then Iterate
      -- A simple DO loop is not repetitive
      If instruction~isA(.Simple.Do.Clause) Then Iterate
      -- The block of the CATCH clause: at execution time, a 28.7
      If instruction == owner, earlyCheck Then Signal 28.007
      -- That's our target!
      blockInstruction = instruction
      Signal Done
    End
    -- No target. At execution time, this will produce a 28.2.
    -- We catch it now if so requested
    If earlyCheck Then Signal 28.002
    Signal Done
  End


  If element \< .ALL.SYMBOLS Then Signal 20.908
  label = element
  Call SetConstantName label, .BLOCK.INSTRUCTION.NAME

  element = TheElementAfter( element, ignoreBlanks )
  If element \< .EL.END_OF_CLAUSE Then Signal 21.908

  blockInstructionsStack = package~blockInstructionsStacks~top
  owner = .Nil -- The block of the CATCH clause we are in (RFE #31)
  Do i = blockInstructionsStack~items To 1 By -1
    instruction = blockInstructionsStack[i]
    -- A FINALLY clause cannot be left (RFE #31)
    If instruction~isA(.Finally.Clause) Then Signal 99.951
    -- The block of a CATCH clause has already ended (RFE #31)
    If instruction~isA(.Catch.Clause) Then Do
      owner = instruction~owner
      Iterate
    End
    blockLabel = instruction~name
    If blockLabel == .Nil, \instruction~isA(.Select.Clause) Then
      blockLabel = instruction~control
    If blockLabel \== .Nil, blockLabel~value == label~value Then Do
      -- The block of the CATCH clause: at execution time, a 28.7, which
      -- comes before the 28.5
      If instruction == owner, earlyCheck Then Signal 28.007
      -- A SELECT or a simple DO is not repetitive. At execution time, this
      -- will produce a 28.5. We catch it now if so requested; otherwise the
      -- target is still the block, so that a compiler can raise the 28.5
      -- when the ITERATE runs.
      If instruction~class = .Select.Clause | -
         instruction~class = .Simple.Do.Clause Then
        If earlyCheck Then Signal 28.005
      blockLabel       = label
      blockInstruction = instruction
      Signal Done
    End
  End
  -- No match. At execution time, this will produce a 28.4.
  If earlyCheck Then Signal 28.004
  -- Restore label, for diagnostic purposes
  blockLabel = label

Done:
  Return .Iterate.Instruction~new(            -
    package, begin, end, blockLabel, blockInstruction  -
  )

-- Symbol expected after ITERATE keyword.
20.908: Syntax( 20.908, ITERATE )

-- Data must not follow the ITERATE name; found "&1".
21.908: Syntax( 21.908, ITERATE, element )

-- ITERATE is valid only within a repetitive loop.
28.002: Syntax( 28.002, ITERATE )

-- Symbol following ITERATE ("&1") must either match the label
-- of a current loop or be omitted.
28.004: Syntax( 28.004, ITERATE, label )

-- Symbol following ITERATE ("&1") does not match
-- a repetitive block instruction.
28.005: Syntax( 28.005, ITERATE, label )

-- ITERATE in a CATCH clause must not refer to the block instruction
-- on line &1, which owns the clause.
28.007: Syntax( 28.007, ITERATE, TheElementAfter( owner~begin )~from~word(1) )

-- &1 must not leave the FINALLY clause on line &2.
99.951: Syntax( 99.951, ITERATE, "ITERATE", -
  TheElementAfter( instruction~begin )~from~word(1) )

--------------------------------------------------------------------------------

::Class Iterate.Instruction Public SubClass Rexx.Instruction
::Attribute name
::Attribute iteration
::Method init
  Expose name iteration
  Use Strict Arg package, begin, end, name, iteration
  self~init:super( package, begin, end )

/******************************************************************************/
/* LEAVE instruction                                                          */
/******************************************************************************/

::Routine Leave.Instruction Public
  Use Strict Arg package, begin, end, elements

  LEAVE = elements[1]

  element = TheElementAfter( LEAVE )

  blockLabel   = .Nil
  iteration    = .Nil
  ignoreBlanks = .True
  earlyCheck = .Parser.Options~earlyCheck
  earlyCheck = earlyCheck~hasItem( "LEAVE" )

  -- No more elements? Calculate the LEAVE target
  --
  -- See RexxRef 2.13, LEAVE: "If name is not specified,
  --   LEAVE ends the innermost active repetitive loop"
  If element < .EL.END_OF_CLAUSE Then Do
    blockInstructionsStack = package~blockInstructionsStacks~top
    owner = .Nil -- The block of the CATCH clause we are in (RFE #31)
    Do i = blockInstructionsStack~items To 1 By -1
      instruction = blockInstructionsStack[i]
      -- A FINALLY clause cannot be left (RFE #31)
      If instruction~isA(.Finally.Clause) Then Signal 99.951
      -- The block of a CATCH clause has already ended (RFE #31)
      If instruction~isA(.Catch.Clause) Then Do
        owner = instruction~owner
        Iterate
      End
      -- A SELECT instruction is not a loop
      If instruction~isA(.Select.Clause) Then Iterate
      -- A simple DO loop is not repetitive
      If instruction~isA(.Simple.Do.Clause) Then Iterate
      -- The block of the CATCH clause: at execution time, a 28.6
      If instruction == owner, earlyCheck Then Signal 28.006
      -- That's our target!
      iteration = instruction
      Signal Done
    End
    -- No target. At execution time, this will produce a 28.1.
    -- We catch it now if so requested
    If earlyCheck Then Signal 28.001
    Signal Done
  End

  -- Label should be a symbol.
  If element \< .ALL.SYMBOLS Then Signal 20.907

  -- Set the element subcategory
  label = element
  Call SetConstantName label, .BLOCK.INSTRUCTION.NAME

  -- Instruction should end here
  element = TheElementAfter( label, ignoreBlanks )
  If element \< .EL.END_OF_CLAUSE Then Signal 21.907

  -- Now the label can refer to any label or control variable in
  -- the block instruction stack.
  blockInstructionsStack = package~blockInstructionsStacks~top
  owner = .Nil -- The block of the CATCH clause we are in (RFE #31)
  Do i = blockInstructionsStack~items To 1 By -1
    instruction = blockInstructionsStack[i]
    -- A FINALLY clause cannot be left (RFE #31)
    If instruction~isA(.Finally.Clause) Then Signal 99.951
    -- The block of a CATCH clause has already ended (RFE #31)
    If instruction~isA(.Catch.Clause) Then Do
      owner = instruction~owner
      Iterate
    End
    blockLabel = instruction~name
    If blockLabel == .Nil, \instruction~isA(.Select.Clause) Then
      blockLabel = instruction~control
    If blockLabel \== .Nil, blockLabel~value == label~value Then Do
      -- The block of the CATCH clause: at execution time, a 28.6
      If instruction == owner, earlyCheck Then Signal 28.006
      blockLabel     = label
      iteration = instruction
      Signal Done
    End
  End
  -- No match. At execution time, this will produce a 28.3.
  If earlyCheck Then Signal 28.003
  -- Restore label, for diagnostic purposes
  blockLabel = label

Done:
  Return .Leave.Instruction~new( package, begin, end, blockLabel, iteration )

-- Symbol expected after LEAVE keyword.
20.907: Syntax( 20.907, LEAVE )

-- Data must not follow the LEAVE name; found "&1".
21.907: Syntax( 21.907, LEAVE, element )

-- LEAVE is valid only within a repetitive loop or labeled block instruction.
28.001: Syntax( 28.001, LEAVE )

-- Symbol following LEAVE ("&1") must either match the label
-- of a current loop or block instruction.
28.003: Syntax( 28.003, LEAVE, label )

-- LEAVE in a CATCH clause must not refer to the block instruction
-- on line &1, which owns the clause.
28.006: Syntax( 28.006, LEAVE, TheElementAfter( owner~begin )~from~word(1) )

-- &1 must not leave the FINALLY clause on line &2.
99.951: Syntax( 99.951, LEAVE, "LEAVE", -
  TheElementAfter( instruction~begin )~from~word(1) )

--------------------------------------------------------------------------------

::Class Leave.Instruction Public SubClass Rexx.Instruction
::Attribute name
::Attribute iteration
::Method init
  Expose name iteration
  Use Strict Arg package, begin, end, name, iteration
  self~init:super( package, begin, end )

/******************************************************************************/
/* NOP instruction                                                            */
/******************************************************************************/

::Routine Nop.Instruction Public
  Use Strict Arg package, begin, end, elements

  NOP = elements[1]

  If elements~items > 1  Then Signal 21.901

  Return .Nop.Instruction~new( package, begin, end )

-- Data must not follow the NOP keyword; found "&1".
21.901: Syntax( 21.901, NOP, elements[2] )

--------------------------------------------------------------------------------

::Class Nop.Instruction Public SubClass Rexx.Instruction

/******************************************************************************/
/* NUMERIC instruction                                                        */
/******************************************************************************/

::Routine Numeric.Instruction Public
  Use Strict Arg package, begin, end, elements

  NUMERIC = elements[1]

  ignoreBlanks = .True

  keyword = TheElementAfter( NUMERIC )

  If keyword \< .ALL.SYMBOLS Then Signal 20.905

  kValue = keyword~value

  If WordPos(kValue, "DIGITS FORM FUZZ") == 0 Then Signal 25.015

  Call SetSubkeyword keyword

  element = TheElementAfter( keyword, ignoreBlanks )

  Select Case kValue
    When "DIGITS" Then Call Expression "Digits"
    When "FUZZ"   Then Call Expression "Fuzz"
    When "FORM"   Then Nop
  End

  -- NUMERIC FORM

  If element \< .ALL.SYMBOLS Then Call Expression "Form"

  keyword = element
  kValue = keyword~value

  If WordPos(kValue, "SCIENTIFIC ENGINEERING VALUE") == 0 Then
    Signal 25.011

  Call SetSubkeyword keyword

  element = TheElementAfter( keyword )

  If kValue == "VALUE" Then
    If element < .EL.END_OF_CLAUSE Then Signal 35.917
    Else Call Expression "Form Value"

  If element \< .EL.END_OF_CLAUSE Then Signal 21.911

  If kValue == "SCIENTIFIC" Then Call Form "Form Scientific"
  Else                           Call Form "Form Engineering"

Form:
  Exit .Numeric.Instruction~new( package, begin, end, Arg(1), .Nil )

Expression:
  If element < .EL.END_OF_CLAUSE Then expression = .Nil
  Else Do
    Call PrepareExpression package, element
    expression = Expression.List( package, element )
  End

  Exit .Numeric.Instruction~new( package, begin, end, Arg(1), expression )

-- NUMERIC must be followed by one of the keywords
-- DIGITS, FORM, or FUZZ; found "&1".
20.905: Syntax( 20.905, NUMERIC, keyword )

-- Data must not follow the NUMERIC FORM specification; found "&1".
21.911: Syntax( 21.911, NUMERIC, element )

-- NUMERIC FORM must be followed by SCIENTIFIC, ENGINEERING, VALUE, or an
-- expression whose value is SCIENTIFIC or ENGINEERING; found "&1".
25.011: Syntax( 25.011, NUMERIC, keyword )

-- NUMERIC must be followed by one of the keywords
-- DIGITS, FORM, or FUZZ; found "&1".
25.015: Syntax( 25.015, NUMERIC, keyword )

-- Missing expression following VALUE keyword of a NUMERIC FORM instruction.
35.917: Syntax( 35.917, NUMERIC )

--------------------------------------------------------------------------------

::Class Numeric.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPRESSION"
::Attribute option     Get
::Attribute expression Get
::Method init
  Expose option expression
  Use Strict Arg package, begin, end, option, expression = .Nil
  self~init:super( package, begin, end)

/******************************************************************************/
/* OptionalExpressionInstruction                                              */
/*   Code common to all instructions that consist of a single keyword         */
/*   followed by an optional expression, like EXIT, PUSH, QUEUE, REPLY,       */
/*   RETURN and SAY.                                                          */
/*                                                                            */
/*   See rexxref 5.1.0 1.11.6. Array Term                                     */
/******************************************************************************/

::Routine Optional.Expression.Instruction Public

  Use Strict Arg package, class, begin, end, elements

  -- When there is an end-of-clause after the keyword,
  -- this is an empty expression
  If elements~items == 1 Then expression = .Nil
  Else Do
    element = elements[2]
    Call PrepareExpression package, element
    expression = Expression.List( package, element, .EL.END_OF_CLAUSE )
    -- In this context, an expression list is an array term
    If expression~isA(.Expression.List) Then
      expression = .Array.Term~new( expression )
  End

  Return class~new( package, begin, end, expression )

--------------------------------------------------------------------------------

::Class Optional.Expression.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPRESSION"
::Attribute expression Get
::Method init
  Expose expression
  Use Strict Arg package, begin, end, expression = .Nil
  self~init:super( package, begin, end )

/******************************************************************************/
/* OPTIONS instruction                                                        */
/******************************************************************************/

::Routine Options.Instruction Public
  Use Strict Arg package, begin, end, elements

  OPTIONS = elements[1]

  If elements~items == 1 Then Signal 35.913

  element = elements[2]

  Call PrepareExpression package, element

  expression = Expression.List( package, element, .EL.END_OF_CLAUSE )

  Return .Options.Instruction~new( package, begin, end, expression )

-- Missing expression following OPTIONS keyword.
35.913: Syntax( 35.913, OPTIONS )

--------------------------------------------------------------------------------

::Class Options.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPRESSION"
::Attribute expression Get
::Method init
  Expose expression
  Use Strict Arg package, begin, end, expression
  self~init:super( package, begin, end )

/******************************************************************************/
/* PARSE instruction                                                          */
/******************************************************************************/

::Routine Parse.Instruction Public
  Use Strict Arg package, begin, end, elements

  PARSE = elements[1]

  keyword      =  TheElementAfter( PARSE )
  options.     = .False

  ignoreBlanks = .True
  upperLower   = .False
  caseLess     = .False

  Loop
    If keyword \< .ALL.SYMBOLS      Then Signal 20.903
    kValue = keyword~value
    Select Case kValue
      When "UPPER"                  Then Call UpperLower
      When "LOWER"                  Then
        If .Features.Parse.Lower    Then Call UpperLower
        Else Signal 25.012
      When "CASELESS"               Then
        If .Features.Parse.Caseless Then Call CaseLess
        Else Signal 25.012
      When "VAR"                    Then Signal Var
      When "VALUE"                  Then Signal (Value)
      When "ARG", "LINEIN", "PULL", "SOURCE", "VERSION"
                                    Then Signal SimpleCase
      When "EXTERNAL"               Then
        If .Features.Parse.External Then Signal SimpleCase
        Else                        Signal 25.012
      When "NUMERIC"    Then
        If .Features.Parse.Numeric  Then Signal SimpleCase
        Else                        Signal 25.012
      Otherwise                     Signal 25.012
    End
  End

SimpleCase:
  Call SetSubkeyword keyword
  options.variant = kValue
  element = TheElementAfter( keyword )
  Signal Templates

Var:
  Call SetSubkeyword keyword
  name = TheElementAfter( keyword )
  If name \< .ALL.SYMBOLS Then Signal 20.904
  c = Left( name~value , 1 )
  If c == "."             Then Signal 31.003
  If c >>= "0", c <<= "9" Then Signal 31.002
  options.variant = "VAR"
  options.["VARNAME"] = name
  name~setAssigned

  -- Ensure that compound variable parts are collected if necessary
  options.["VARIABLE"] = Symbol.Term( package, name )

  element = TheElementAfter( name, ignoreBlanks )
  Signal Templates

Value:
  Call SetSubkeyword keyword
  element = TheElementAfter( keyword )
  If element < .EL.END_OF_CLAUSE Then Signal 38.003
  Call PrepareExpression package, element, "WITH"
  options.variant = "VALUE"
  expression = Expression.List( package, element )
  options.["EXPRESSION"] = expression
  element = expression~end
  If element < .EL.END_OF_CLAUSE Then Signal 38.003
  -- Now element is a .ALL.VAR_SYMBOLS with a value of WITH for sure
  Call SetSubkeyword keyword
  element = TheElementAfter( element )
  Signal Templates

UpperLower:
  If upperLower Then Signal 25.012
  upperLower = 1
  Signal FirstOptions

CaseLess:
  If caseLess Then Signal 25.012
  caseLess = 1
  Signal FirstOptions

FirstOptions:
  options.kValue = 1
  Call SetSubkeyword keyword
  keyword = TheElementAfter( keyword )
  Return

Templates:
  templateList = Template.List( package, element )
  Return .Parse.Instruction~new( package, begin, end, options., templateList )

-- Symbol expected after PARSE keyword.
20.903: Syntax( 20.903, PARSE )

-- Symbol expected after PARSE VAR.
20.904: Syntax( 20.904, PARSE )

-- PARSE must be followed by optional keywords LOWER or UPPER,
-- by an optional keyword CASELESS, and by one of the keywords
-- ARG, LINEIN, PULL, SOURCE, VALUE, VAR, or VERSION; found "&1".
25.012: Syntax( 25.012, PARSE, keyword )

-- Variable symbol must not start with a number; found "&1".
31.002: Syntax( 31.002, PARSE, name )

-- Variable symbol must not start with a "."; found "&1".
31.003: Syntax( 31.003, PARSE, name )

-- PARSE VALUE instruction requires WITH keyword.
38.003: Syntax( 38.003, PARSE )

--------------------------------------------------------------------------------

::Class Parse.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPRESSION VARIABLE TEMPLATES"
::Attribute Upper      Get
::Attribute Lower      Get
::Attribute Caseless   Get
::Attribute Variant    Get
::Attribute VarName    Get
::Attribute Variable    Get
::Attribute Expression Get
::Attribute Templates Get
::Method init
  Expose Upper Lower Caseless Variant VarName Variable Expression Templates

  Use Strict Arg package, begin, end, options., Templates

  self~init:super( package, begin, end )

  Upper      =  0
  Lower      =  0
  Caseless   =  0
  VarName    = .Nil
  Variable   = .Nil
  Expression = .Nil

  If options.~hasIndex("UPPER")    Then Upper    = 1
  If options.~hasIndex("LOWER")    Then Lower    = 1
  If options.~hasIndex("CASELESS") Then Caseless = 1
  Variant = options.["VARIANT"]
  If Variant == "VAR"   Then VarName    = options.["VARNAME"]
  If options.~hasIndex("VARIABLE") Then Variable = options.["VARIABLE"]
  If Variant == "VALUE" Then Expression = options.["EXPRESSION"]

/******************************************************************************/
/* PROCEDURE instruction                                                      */
/******************************************************************************/

::Routine Procedure.Instruction Public
  Use Strict Arg package, begin, end, elements

  If elements~items == 1 Then
    Return .Procedure.Instruction~new( package, begin, end, .Nil )

  PROCEDURE = elements[1]
  EXPOSE    = elements[2]

  If EXPOSE < .ALL.VAR_SYMBOLS, EXPOSE~value = "EXPOSE" Then Do
    Call SetSubkeyword EXPOSE
    If elements[3] < .ALL.WHITESPACE_LIKE Then elements~delete( 3 )
    Return Name.List.Instruction(                           -
      package, .Procedure.Instruction, begin, end, elements -
    )
  End

  Signal 25.017

-- PROCEDURE must be followed by the keyword EXPOSE or nothing; found "&1".
25.017: Syntax( 25.017, PROCEDURE, EXPOSE )

--------------------------------------------------------------------------------
--   exposed format: a list of valid symbols; a parenthesized symbol is       --
--   stored as a 1-element array.                                             --
--------------------------------------------------------------------------------

::Class Procedure.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPOSED"
::Attribute exposed
::Method init
  Expose exposed
  Use Strict Arg package, begin, end, exposed
  self~init:super( package, begin, end )

/******************************************************************************/
/* PULL instruction                                                           */
/******************************************************************************/

::Routine Pull.Instruction Public
  Use Strict Arg package, begin, end, elements
  Return Optional.Template.List(                    -
    package, .Pull.Instruction, begin, end, elements  -
  )

--------------------------------------------------------------------------------

::Class Pull.Instruction Public SubClass Optional.Template.List
::Constant instructionName "PULL"

/******************************************************************************/
/* PUSH instruction                                                           */
/******************************************************************************/

::Routine Push.Instruction Public
  Use Strict Arg package, begin, end, elements
  Return Optional.Expression.Instruction( -
    package, .Push.Instruction, begin, end, elements  -
  )

--------------------------------------------------------------------------------

::Class Push.Instruction Public SubClass Optional.Expression.Instruction
::Constant instructionName "PUSH"

/******************************************************************************/
/* QUEUE instruction                                                          */
/******************************************************************************/

::Routine Queue.Instruction Public
  Use Strict Arg package, begin, end, elements
  Return Optional.Expression.Instruction( -
    package, .Queue.Instruction, begin, end, elements  -
  )

--------------------------------------------------------------------------------

::Class Queue.Instruction Public SubClass Optional.Expression.Instruction
::Constant instructionName "QUEUE"

/******************************************************************************/
/* RAISE instruction                                                          */
/******************************************************************************/

::Routine Raise.Instruction Public
  Use Strict Arg package, begin, end, elements
  Return RaiseOrThrow( package, begin, end, elements, "RAISE" )

/******************************************************************************/
/* THROW instruction (RFE #31, proposed: DRAFT 0 of 16/11/2026, prototype on  */
/*   ooRexx trunk r13540). RAISE without RETURN and EXIT; its own messages    */
/*   for the errors that name the instruction.                                */
/******************************************************************************/

::Routine Throw.Instruction Public
  Use Strict Arg package, begin, end, elements
  Return RaiseOrThrow( package, begin, end, elements, "THROW" )

--------------------------------------------------------------------------------

::Routine RaiseOrThrow
  Use Strict Arg package, begin, end, elements, name

  RAISE = elements[1]

  throw = name == "THROW"

  ignoreBlanks = .True

  condition = TheElementAfter( RAISE )

  If condition \< .ALL.SYMBOLS Then
    If throw Then Signal 20.937
    Else          Signal 20.914

  expectCode      = "ERROR FAILURE SYNTAX"
  expectCondition = "USER"
  expectNothing   = "HALT LOSTDIGITS NOMETHOD NOSTRING NOTREADY NOVALUE",
                    "PROPAGATE"
  allKeywords     = expectCode expectCondition expectNothing

  cValue          = condition~value

  If WordPos( cValue, allKeywords ) == 0 Then Signal 25.906

  Call SetSubkeyword condition

  element = TheElementAfter( condition, ignoreBlanks )

  options = .Set~of("ADDITIONAL", "ARRAY", "DESCRIPTION", "EXIT", "RETURN")
  -- A thrown condition ends every level it leaves, and none of them returns
  -- anything: THROW has no RETURN and no EXIT (25.941)
  If throw Then options = .Set~of("ADDITIONAL", "ARRAY", "DESCRIPTION")

  additional  = .False -- ADDITIONAL or ARRAY already processed
  return      = .False -- RETURN or EXIT already processed
  description = .False -- DESCRIPTION already processed

  Select
    When cValue == "USER" Then Do
      If element \< .ALL.SYMBOLS Then Signal 20.915
      Call SetConstantName element, .USER.CONDITION.NAME
      condition = ( condition, element )
      Call NextElement
    End
    When WordPos( cValue, expectCode ) > 0 Then Do
      condition = ( condition, Expression() )
    End
    Otherwise -- Expect nothing
      condition = Array( condition )
  End

  options. = .Nil

  Loop
    kValue   = element~value
    keywords = options~makeArray~makeString("L"," ")
    Select
      When element < .EL.END_OF_CLAUSE Then Do
        If throw Then Exit .Throw.Instruction~new( -
          package, begin, end, condition, options.  -
        )
        Exit .Raise.Instruction~new( -
          package, begin, end, condition, options.  -
        )
      End
      When element \< .ALL.VARIABLES_AND_KEYWORDS Then Signal 25.907
      When WordPos( kValue, keywords ) == 0       Then Signal 25.907
      Otherwise
        Call SetSubkeyword element
        Call ("OPTION."kValue)
    End
  End

Option.Additional:
  If additional Then Signal 25.909
  additional = .True
  Call NextElement
  If element < .EL.END_OF_CLAUSE Then
    If throw Then Signal 35.939
    Else          Signal 35.923
  options.["ADDITIONAL"] = Expression()
  Return

Option.Array:
  If additional Then Signal 25.909
  additional = .True
  Call NextElement
  If element \< .EL.LEFT_PARENTHESIS Then Signal 35.924
  next = TheElementAfter(element)
  -- ARRAY () is allowed by the syntax
  If next < .EL.RIGHT_PARENTHESIS Then Do
    options.["ARRAY"] = ""
    element = TheElementAfter(next)
  End
  Else options.["ARRAY"] = Expression()
  Return

Option.Description:
  If description Then Signal 25.908
  description = .True
  Call NextElement
  If element < .EL.END_OF_CLAUSE Then
    If throw Then Signal 35.938
    Else          Signal 35.922
  options.["DESCRIPTION"] = Expression()
  Return

Option.Exit:
  If return Then Signal 25.911
  -- RETURN and EXIT give the routine a result of their own: they would
  -- replace what is pending in a FINALLY clause (ooRexx r13535)
  Call FinallyExitCheck package, RAISE, "RAISE"
  return = .True
  Call NextElement
  If element < .EL.END_OF_CLAUSE Then
       options.["EXIT"]   = ""
  Else options.["EXIT"]   = Expression()
  Return

Option.Return:
  If return Then Signal 25.911
  -- RETURN and EXIT give the routine a result of their own: they would
  -- replace what is pending in a FINALLY clause (ooRexx r13535)
  Call FinallyExitCheck package, RAISE, "RAISE"
  return = .True
  Call NextElement
  If element < .EL.END_OF_CLAUSE Then
       options.["RETURN"] = ""
  Else options.["RETURN"] = Expression()
  Return

NextElement:
  element = TheElementAfter( element, ignoreBlanks )
  Return

Expression:
  If element < .ALL.CONSTANT_SYMBOLS_AND_STRINGS Then Do
    value = element
    Call NextElement -- Consume element
    Return value
  End
  Else If element \< .EL.LEFT_PARENTHESIS Then Signal 35.001
  Else Do
    Call PrepareExpression package, element, options~makeArray~makeString("L"," ")
    expression = SubExpression( package, element, .EL.RIGHT_PARENTHESIS )
    element = expression~end
    Return expression
  End

-- Duplicate RETURN or EXIT keyword found.
25.911: Syntax( 25.911, RAISE )

-- Symbol expected after RAISE keyword.
20.914: Syntax( 20.914, RAISE )

-- Symbol expected after USER keyword.
20.915: Syntax( 20.915, RAISE )

-- Symbol expected after THROW keyword.
20.937: Syntax( 20.937, RAISE )

-- RAISE must be followed by one of the keywords ERROR, FAILURE, HALT,
-- LOSTDIGITS, NOMETHOD, NOSTRING, NOTREADY, NOVALUE, PROPAGATE, SYNTAX, or
-- USER; found "&1".
25.906:
  -- THROW must be followed by one of the keywords ... (the same list)
  If throw Then Syntax( 25.940, RAISE, condition )
  Syntax( 25.906, RAISE, condition )

-- Unknown keyword on RAISE instruction; found "&1".
25.907:
  -- Unknown keyword on THROW instruction; found "&1".
  If throw Then Syntax( 25.941, RAISE, element )
  Syntax( 25.907, RAISE, element )

-- Duplicate DESCRIPTION keyword found.
25.908: Syntax( 25.908, RAISE )

-- Duplicate ADDITIONAL or ARRAY keyword found.
25.909: Syntax( 25.909, RAISE )

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, RAISE, element )

-- Missing expression following DESCRIPTION keyword of a RAISE instruction.
35.922: Syntax( 35.922, RAISE )

-- Missing expression following ADDITIONAL keyword of a RAISE instruction.
35.923: Syntax( 35.923, RAISE )

-- Missing expression following DESCRIPTION keyword of a THROW instruction.
35.938: Syntax( 35.938, RAISE )

-- Missing expression following ADDITIONAL keyword of a THROW instruction.
35.939: Syntax( 35.939, RAISE )

-- Missing "(" on expression list of the ARRAY keyword.
35.924: Syntax( 35.924, RAISE )

--------------------------------------------------------------------------------

::Class Raise.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "ADDITIONAL ARRAY DESCRIPTION EXIT RETURN"
::Constant instructionName "RAISE"

::Attribute Condition     Get
::Attribute Code          Get
::Attribute UserCondition Get
::Attribute Propagate     Get
::Attribute Additional    Get
::Attribute Array         Get
::Attribute Description   Get
::Attribute Return        Get
::Attribute Exit          Get

::Method init

  Expose Condition Code UserCondition Propagate Additional Array -
    Description Return Exit

  Condition     = .Nil
  Code          = .Nil
  UserCondition = .Nil
  Propagate     = .False
  Additional    = .Nil
  Array         = .Nil
  Description   = .Nil
  Return        = .Nil
  Exit          = .Nil

  Use Strict Arg package, begin, end, cond, options.

  If cond~items == 1 Then Do
    cond = cond[1]
    If cond~value == "PROPAGATE" Then Propagate = 1
    Else                              Condition = cond
  End
  Else Do
    Condition = cond[1]
    If Condition~value == "USER" Then UserCondition = cond[2]
    Else                              Code          = cond[2]
  End

  self~init:super( package, begin, end )

  If options.~hasIndex("ADDITIONAL")  Then Additional  =  options.["ADDITIONAL" ]
  If options.~hasIndex("ARRAY")       Then Array       =  options.["ARRAY"      ]
  If options.~hasIndex("DESCRIPTION") Then Description =  options.["DESCRIPTION"]
  If options.~hasIndex("EXIT")        Then Exit        =  options.["EXIT"       ]
  If options.~hasIndex("RETURN")      Then Return      =  options.["RETURN"     ]

--------------------------------------------------------------------------------

-- THROW (RFE #31): a RAISE whose Return and Exit are always .Nil

::Class Throw.Instruction Public SubClass Raise.Instruction
::Constant instructionName "THROW"

/******************************************************************************/
/* REPLY instruction                                                          */
/******************************************************************************/

::Routine Reply.Instruction Public
  -- Message 99.919 is run-time only
  Use Strict Arg package, begin, end, elements
  -- INTERPRET data must not contain REPLY.
  If .Options.Interpret Then Signal 99.924
  instruction = Optional.Expression.Instruction( -
    package, .Reply.Instruction, begin, end, elements  -
  )
  -- REPLY returns a result to the caller: in a FINALLY clause it would
  -- replace what is pending (ooRexx r13535)
  Call FinallyExitCheck package, elements[1], "REPLY"
  Return instruction

-- INTERPRET data must not contain REPLY.
99.924: Syntax( 99.924, elements[1] )

--------------------------------------------------------------------------------

::Class Reply.Instruction Public SubClass Optional.Expression.Instruction
::Constant instructionName "REPLY"

/******************************************************************************/
/* RETURN instruction                                                         */
/******************************************************************************/

::Routine Return.Instruction Public
  Use Strict Arg package, begin, end, elements
  instruction = Optional.Expression.Instruction( -
    package, .Return.Instruction, begin, end, elements  -
  )
  -- RETURN must not leave a FINALLY clause (RFE #31)
  Call FinallyExitCheck package, elements[1], "RETURN"
  Return instruction

--------------------------------------------------------------------------------

::Class Return.Instruction Public SubClass Optional.Expression.Instruction
::Constant instructionName "RETURN"
::Method init
  Use Strict Arg package, begin, end, expression = .Nil
  self~init:super( package, begin, end, expression )
  self~terminating = .True

/******************************************************************************/
/* SAY instruction                                                            */
/******************************************************************************/

::Routine Say.Instruction Public
  Use Strict Arg package, begin, end, elements
  Return Optional.Expression.Instruction( -
    package, .Say.Instruction, begin, end, elements  -
  )

--------------------------------------------------------------------------------

::Class Say.Instruction Public SubClass Optional.Expression.Instruction
::Constant instructionName "SAY"

/******************************************************************************/
/* SIGNAL instruction  (signal labelname, signal value expression)            */
/******************************************************************************/

::Routine Signal.Instruction Public
  Use Strict Arg package, begin, end, elements

  SIGNAL = elements[1]

  If elements~items == 1 Then Signal 19.004

  ignoreBlanks = .True

  element = TheElementAfter( SIGNAL )

  value   = .False -- VALUE keyword found

  If element \< .ALL.SYMBOLS_AND_STRINGS Then Signal Expression

  If element < .ALL.VAR_SYMBOLS Then Do
    Select Case element~value
      When "ON"  Then
        Return Signal.On.Instruction(  package, begin, end, elements )
      When "OFF" Then
        Return Signal.Off.Instruction( package, begin, end, elements )
      When "VALUE" Then Do
        Call SetSubkeyword element
        element = TheElementAfter( element )
        If element < .EL.END_OF_CLAUSE Then Signal 35.915
        value = .True
        Signal Expression
      End
      Otherwise Nop
    End
  End

  labelName = element

  Call SetConstantName labelName, .LABEL.NAME

  element     = TheElementAfter( labelName, ignoreBlanks )
  If element \< .EL.END_OF_CLAUSE Then Signal 21.905

  -- SIGNAL must not leave a FINALLY clause (RFE #31)
  Call FinallyExitCheck package, SIGNAL, "SIGNAL"

  Return .Signal.Instruction~new( package, begin, end, labelName )

Expression:
  Call PrepareExpression package, element
  expression = Expression.List( package, element )
  element = expression~end

  -- SIGNAL must not leave a FINALLY clause (RFE #31)
  Call FinallyExitCheck package, SIGNAL, "SIGNAL"

  Return .Signal.Value.Instruction~new( package, begin, end, expression )

-- String or symbol expected after SIGNAL keyword.
19.004: Syntax( 19.004, SIGNAL )

-- Data must not follow the SIGNAL label name; found "&1".
21.905: Syntax( 21.905, SIGNAL, element )

-- Missing expression following VALUE keyword of a SIGNAL instruction.
35.915: Syntax( 35.915, SIGNAL )

--------------------------------------------------------------------------------

::Class Signal.Instruction Public SubClass Rexx.Instruction -- SIGNAL labelName
::Attribute label
::Method init
  Expose label
  Use Strict Arg package, begin, end, label
  self~init:super( package, begin, end )

--------------------------------------------------------------------------------

::Class Signal.Value.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPRESSION"
::Attribute expression
::Method init
  Expose expression
  Use Strict Arg package, begin, end, expression
  self~init:super( package, begin, end )

/******************************************************************************/
/* SIGNAL OFF instruction                                                     */
/******************************************************************************/

::Routine Signal.Off.Instruction Public

  Use Strict Arg package, begin, end, elements

  OFF = elements[2]

  Call SetSubkeyword OFF

  ignoreBlanks = .True

  condition = TheElementAfter( OFF )

  If condition \< .ALL.SYMBOLS Then Signal 20.912

  name = condition~value

  If WordPos(name, .Features.Signal.OnOff.Conditions ) == 0 Then Signal 25.004

  Call SetSubkeyword condition

  userCondition = .Nil

  If name == "USER" Then Do
    userCondition = TheElementAfter( condition, ignoreBlanks )
    If userCondition \< .ALL.SYMBOLS Then Signal 20.915
    Call SetConstantName userCondition, .USER.CONDITION.NAME
    element = TheElementAfter( userCondition, ignoreBlanks )
  End
  Else element = TheElementAfter( condition, ignoreBlanks )

  If element \< .EL.END_OF_CLAUSE Then Signal 21.904

  Return .Signal.Off.Instruction~new(-
    package, begin, end, condition, userCondition -
  )

-- Data must not follow the condition name; found "&1".
21.904: Syntax( 21.904, OFF, element )

-- Symbol expected after OFF keyword.
20.912: Syntax( 20.912, OFF )

-- Symbol expected after USER keyword.
20.915: Syntax( 20.915, OFF )

-- SIGNAL OFF must be followed by one of the keywords ERROR, FAILURE,
-- HALT, LOSTDIGITS, NOTREADY, NOMETHOD, NOSTRING, NOVALUE, SYNTAX,
-- USER, or ANY; found "&1".
25.004: Syntax( 25.004, OFF, condition )

--------------------------------------------------------------------------------

::Class Signal.Off.Instruction Public SubClass Rexx.Instruction
::Attribute condition     Get  -- the condition keyword element (ERROR, ANY...)
::Attribute userCondition Get  -- the USER condition name element, or .Nil
::Method init
  Expose condition userCondition
  Use Strict Arg package, begin, end, condition, userCondition = .Nil
  self~init:super( package, begin, end )

/******************************************************************************/
/* SIGNAL ON instruction                                                      */
/******************************************************************************/

::Routine Signal.On.Instruction Public

  Use Strict Arg package, begin, end, elements

  ON = elements[2]

  Call SetSubkeyword ON

  ignoreBlanks = .True

  condition = TheElementAfter( ON )

  If condition \< .ALL.SYMBOLS Then Signal 20.911

  name = condition~value

  If WordPos(name, .Features.Signal.OnOff.Conditions ) == 0 Then Signal 25.003

  Call SetSubkeyword condition

  userCondition = .Nil

  If name == "USER" Then Do
    userCondition = TheElementAfter( condition, ignoreBlanks )
    If userCondition \< .ALL.SYMBOLS Then Signal 20.915
    Call SetConstantName userCondition, .USER.CONDITION.NAME
    element = TheElementAfter( userCondition, ignoreBlanks )
  End
  Else element = TheElementAfter( condition, ignoreBlanks )

  If element < .EL.END_OF_CLAUSE Then Return .Signal.On.Instruction~new(-
    package, begin, end, condition, userCondition -
  )

  If element \< .ALL.VAR_SYMBOLS Then Signal 25.915
  If element~value \== "NAME"     Then Signal 25.915

  Call SetSubkeyword element

  trapName = TheElementAfter( element )

  If trapName \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.003

  Call SetConstantName trapName, .LABEL.NAME

  end = TheElementAfter( trapName, ignoreBlanks )

  If end \< .EL.END_OF_CLAUSE Then Signal 21.903

  Return .Signal.On.Instruction~new(-
    package, begin, end, condition, userCondition, trapName -
  )

-- String or symbol expected after NAME keyword.
19.003: Syntax( 19.003, ON )

-- Data must not follow the NAME trap name; found "&1".
21.903: Syntax( 21.903, ON, end )

-- Symbol expected after ON keyword.
20.911: Syntax( 20.911, ON )

-- Symbol expected after USER keyword.
20.915: Syntax( 20.915, ON )

-- SIGNAL ON must be followed by one of the keywords ERROR, FAILURE,
-- HALT, LOSTDIGITS, NOTREADY, NOMETHOD, NOSTRING, NOVALUE, SYNTAX,
-- USER, or ANY; found "&1".
25.003: Syntax( 25.003, ON, condition )

-- SIGNAL ON condition must be followed by the keyword NAME; found "&1".
25.915: Syntax( 25.915, ON, element )

--------------------------------------------------------------------------------

::Class Signal.On.Instruction Public SubClass Rexx.Instruction
::Attribute label
::Attribute condition
::Attribute userCondition
::Method init
  Expose condition userCondition label
  Use Strict Arg package, begin, end, condition, -
    userCondition = .Nil, label = .Nil
  self~init:super( package, begin, end )

/******************************************************************************/
/* TRACE instruction                                                          */
/******************************************************************************/

::Routine Trace.Instruction Public

  Use Strict Arg package, begin, end, elements

  TRACE = elements[1]

  options.     = .False
  ignoreBlanks = .True

  element = TheElementAfter( TRACE )

  If element < .EL.END_OF_CLAUSE Then Signal Done

  If element < .ALL.STRINGS Then Do
    options.string = element
    element = TheElementAfter( element, ignoreBlanks )
    Signal Processed
  End

  If element < (.EL.OP.MINUS || .EL.OP.PLUS), elements~size == 3 Then Do
    theSign              = element
    element              = TheElementAfter( element )
    theNumber            = element
    options.signedNumber = theSign, theNumber
    numberToCheck        = theSign~value theNumber~value
    If \DataType(numberToCheck,"W") Then Signal 26.007
  End

  If element < .ALL.NUMBERS Then Do
    options.number = element
    If \DataType(element~value,"W") Then Signal 24.001
    element = TheElementAfter( element, ignoreBlanks )
    Signal Processed
  End

  If element < .ALL.SYMBOLS Then Do
    If element~value == "VALUE" Then Signal (Value)
    value = element~value
    letters = "ACEFILNOR"
    If .Features.Trace.CMS Then Do
      prefix = "!?"
      letters ||= "S"
    End
    Else Do
      prefix = "?"
    End
    Loop While value \== "", Pos(value[1],prefix) > 0
      value = SubStr(value,2)
    End
    If value \== "", Pos( value[1], letters ) == 0 Then Signal 24.001
    options.symbol = element
    element = TheElementAfter( element, ignoreBlanks )
    Signal Processed
  End

  If element < .EL.LEFT_PARENTHESIS Then Do
    Call PrepareExpression package, element, ")"
    expression = Expression.List( package, element )
    options.value = expression
    element = expression~end
    Signal Processed
  End

Value:
  Call SetSubkeyword element
  element = TheElementAfter( element, ignoreBlanks )
  If element < .EL.END_OF_CLAUSE Then Signal 35.916
  Call PrepareExpression package, element
  expression = Expression.List( package, element )
  options.value = expression
  Signal Done

Processed:
  If element < .EL.END_OF_CLAUSE Then Signal Done
  Signal 21.906

Done:
  Return .Trace.Instruction~new( package, begin, end, options. )

-- Data must not follow the TRACE setting; found "&1".
21.906: Syntax( 21.906, TRACE, element )

-- TRACE request letter must be one of "ACEFILNOR"; found "&1".
24.001: Syntax( 24.001, TRACE, value[1] )

-- Number used in TRACE setting must be a whole number; found "&1".
26.007: Syntax( 26.007, TRACE, theNumber )

-- Missing expression following VALUE keyword of a TRACE instruction.
35.916: Syntax( 35.916, TRACE )

--------------------------------------------------------------------------------

::Class Trace.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "EXPRESSION"
::Attribute number     Get
::Attribute sign       Get    -- The "-" or "+" of TRACE -n / +n, or .Nil
::Attribute theString  Get
::Attribute symbol     Get
::Attribute expression Get
::Method init
  Expose number sign theString symbol expression

  Use Strict Arg package, begin, end, options.

  number     = .Nil
  sign       = .Nil
  theString  = .Nil
  symbol     = .Nil
  expression = .Nil

  If options.~hasIndex("NUMBER") Then number     = options.["NUMBER"]
  If options.~hasIndex("SIGNEDNUMBER") Then sign  = options.["SIGNEDNUMBER"][1]
  If options.~hasIndex("STRING") Then theString  = options.["STRING"]
  If options.~hasIndex("SYMBOL") Then symbol     = options.["SYMBOL"]
  If options.~hasIndex("VALUE" ) Then expression = options.["VALUE" ]

  self~init:super( package, begin, end )

/******************************************************************************/
/* UPPER instruction                                                          */
/******************************************************************************/

::Routine Upper.Instruction Public
  Use Strict Arg package, begin, end, elements

  Return Name.List.Instruction(                       -
    package, .Upper.Instruction, begin, end, elements -
  )

--------------------------------------------------------------------------------
--   nameList format: a list of valid symbols; a parenthesized symbol is      --
--   stored as a 1-element array.                                             --
--------------------------------------------------------------------------------

::Class Upper.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "NAMELIST"
::Attribute nameList
::Method init
  Expose nameList
  Use Strict Arg package, begin, end, nameList
  self~init:super( package, begin, end )

/******************************************************************************/
/* USE instruction                                                            */
/******************************************************************************/

::Routine Use.Instruction Public
  Use Strict Arg package, begin, end, elements

  USE = elements[1]

  strict = 0
  auto   = 0                            -- Executor only
  named  = 0                            -- Executor only

  keyword = TheElementAfter( USE )

  If   keyword < .ALL.VAR_SYMBOLS, keyword~value == "LOCAL" Then Do
    Call SetSubkeyword keyword
    Return Use.Local.Instruction( package, begin, keyword )
  End

  If   keyword < .ALL.VAR_SYMBOLS, keyword~value == "STRICT" Then Do
    Call SetSubkeyword keyword
    strict = 1
    keyword = TheElementAfter( keyword )
  End

  If .Options.Executor Then Do
    If keyword < .ALL.VAR_SYMBOLS, keyword~value == "AUTO"  Then Do
      Call SetSubkeyword keyword
      auto   = 1
      keyword = TheElementAfter( keyword )
    End
    If keyword < .ALL.VAR_SYMBOLS, keyword~value == "NAMED" Then Do
      Call SetSubkeyword keyword
      named  = 1
      keyword = TheElementAfter( keyword )
    End
  End

  If   keyword < .ALL.VAR_SYMBOLS, keyword~value \== "ARG"  Then Do
    If .Options.Executor Then Signal 25.900
    Else If strict       Then Signal 25.929
    Else                      Signal 25.905
  End

  Call SetSubkeyword keyword

  Return Use.Arg.Instruction( -
    package, begin, keyword, strict, auto, named -
  )

-- Invalid or misplaced USE subkeyword "&1".
25.900: Syntax( 25.900, USE, 'Invalid or misplaced USE subkeyword "'keyword'"' )

-- USE must be followed by the keyword ARG, LOCAL or STRICT; found "&1".
25.905: Syntax( 25.905, USE, keyword )

-- USE STRICT must be followed by the keyword ARG; found "&1".
25.929: Syntax( 25.929, USE, keyword )

/******************************************************************************/
/* USE ARG instruction                                                        */
/******************************************************************************/

::Routine Use.Arg.Instruction

  Use Strict Arg package, begin, keyword, strict, auto, named

  arguments      =  Array()
  ignoreBlanks   = .True
  argProcessed   = .False
  continue       = .EL.COMMA || .EL.END_OF_CLAUSE
  afterEqualSign = .EL.LEFT_PARENTHESIS || .ALL.CONST_SYMBOLS || -
                   .ALL.NUMBERS         || .ALL.STRINGS
  If .Options.Executor Then
    afterEqualSign ||= .EL.LEFT_CURLY_BRACKET

  element        = keyword

  Call NextElement
  Call PrepareExpression package, element

  N = 0

  Loop While element \< .EL.END_OF_CLAUSE
    Select
      When element < .EL.COMMA Then Do
        If \argProcessed Then N += 1
        argProcessed = .False -- Mark as processed
        Call NextElement      -- Consume the comma
        Iterate               -- Don't reset argProcessed at loop end
      End
      When element < .ALL.SYMBOLS Then Do
        If element~value == "..." Then Signal Ellipsis
        c = element~value[1]
        If c >= "0", c <= "9" Then Signal 31.002
        -- Check for "." moved below: .o is invalid, but .o~m is not
        -- Mark the element as assigned
        element~setAssigned
        -- Build a symbol term
        term = Symbol.Term( package, element )
        -- Now collect "[args]" and "~[~]method(args)" sequences
        term = Simple.Message.Term( package, term )
        If term~isA(.Symbol.Term), Left(term~symbol~value,1) == "." Then
          Signal 31.003
        element = term~end
        -- Consume blanks after ")" and "]", and the blank PrepareExpression
        -- made a concatenation (a b): the token in error is the "b"
        Call SkipBlank
        If element < continue Then Do
          N += 1
          arguments[N] = Array(term)
          Call BackTrack
        End
        Else Do
          -- A name followed by something else than ",", "=" or the end is
          -- 46.903 since ooRexx r13416 (bug #1960; 46.902 is now for references)
          If element \< .EL.OP.EQUAL Then Call 46.903 element
          Call NextElement
          -- Nothing after "=": 35.930; something else, 35.1 (ooRexx)
          If element < .EL.END_OF_CLAUSE Then Signal 35.930
          If \( element < afterEqualSign ) Then Signal 35.001
          If element < .EL.LEFT_PARENTHESIS Then Do
            subExpression = SubExpression( package, element, "" )
            N += 1
            arguments[N] = term, subExpression
            element = subExpression~end
            -- After the ")", only "," or the end (ooRexx: 35.930, v388)
            Call SkipBlank
            If \( element < continue ) Then Signal 35.930
          End
          Else If element < .EL.LEFT_CURLY_BRACKET Then Do
            literal = Source.Literal( package, element, "" )
            N += 1
            arguments[N] = term, literal
            element = literal~end
          End
          Else Do
            N += 1
            arguments[N] = term, element
            Call NextElement
            If \( element < continue ) Then Signal 35.930
          End
          Call BackTrack
        End
      End
      When element < .ALL.OPS.REFERENCE Then Do
        operator = element
        Call NextElement
        If element \< .ALL.REFERENCED_SYMBOLS Then Signal 20.931
        element~setAssigned
        reference = .Variable.Reference.Term~new(operator, element)
        N += 1
        arguments[N] = Array(reference)
        Call NextElement
        Call SkipBlank
        If element < .EL.OP.EQUAL Then Signal 99.950
        -- Anything else than "," or the end: 46.902 (ooRexx, which names it)
        If element \< continue Then Call 46.902 element
        Call BackTrack
      End
      -- An operator that can not start a term is 35.1 (ooRexx); a prefix
      -- one, 89.1
      When element < .ALL.OPERATORS,     -
           element \< .PREFIX_OPERATORS, -
           element \< .EL.LEFT_BRACKET   Then Signal 35.001
      Otherwise Signal 89.001
    End
    Call NextElement
    argProcessed = .True
  End

Return:

  -- The count of positions, as ooRexx: a comma after a variable ends it; any
  -- other comma is an omitted one ("USE ARG ," has one, "USE ARG a," too).
  count = N

Done:

  -- USE STRICT AUTO NAMED ARG requires ... at the end
  If strict, auto, named Then Do
    If arguments~isEmpty       Then Signal 99.900
    last = arguments~lastItem
    If last~items > 1          Then Signal 99.900
    If \last[1]~isA(.Element)  Then Signal 99.900
    If last[1] \< .EL.ELLIPSIS Then Signal 99.900
  End

  Return .Use.Arg.Instruction~new(                                 -
    package, begin, element, strict, auto, named, arguments, count -
  )

-- "..." is not a position
Ellipsis:
  Call SetCategory element, .EL.ELLIPSIS
  argProcessed = .True
  count = N
  N += 1
  arguments[N] = Array(element)
  element = TheElementAfter( element, ignoreBlanks )
  If element \< .EL.END_OF_CLAUSE Then Signal 99.930
  Signal Done

BackTrack:
  element = TheElementBefore( element )
  Return

SkipBlank:
  If element < .ALL.WHITESPACE_LIKE | element < .EL.OP.BLANK Then Do
    element~ignorable = 1
    element = TheElementAfter( element )
  End
  Return

NextElement:
  element = TheElementAfter( element, ignoreBlanks )
  Return

-- Simple variable or stem symbol expected after > or < USE ARG
-- reference operator; found &1.
20.931: Syntax( 20.931, keyword, element )

-- Variable symbol must not start with a number; found "&1".
31.002: Syntax( 31.002, keyword, element )

-- Variable symbol must not start with a "."; found "&1".
31.003: Syntax( 31.003, keyword, element )

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, keyword, element )

-- Invalid or missing expression following "=" token of a USE ARG instruction.
35.930: Syntax( 35.930, keyword )

-- Extra token ("&1") found in USE ARG variable reference;
-- "," or end of instruction expected.
46.902: Syntax( 46.902, keyword, Arg(1) )

-- Extra token ("&1") found in USE ARG name;
-- ",", "=" or end of instruction expected.
46.903: Syntax( 46.903, keyword, Arg(1) )

-- The USE instruction requires a comma-separated list
-- of variables or assignment message terms.
89.001: Syntax( 89.001, keyword, element )

-- STRICT AUTO requires the "..." argument marker
-- at the end of the argument list
99.900: Syntax( 99.900, keyword, 'STRICT AUTO requires the "..." argument' -
  'marker at the end of the argument list')

-- The "..." argument marker can only appear at the end of the argument list.
99.930: Syntax( 99.930, keyword )

-- A USE ARG default value is not allowed for variable references.
99.950: Syntax( 99.950, keyword )

--------------------------------------------------------------------------------

::Class Use.Arg.Instruction Public SubClass Rexx.Instruction
::Constant childAttributes "ARGUMENTS"
::Attribute strict    Get
::Attribute auto      Get
::Attribute named     Get
::Attribute arguments Get
::Attribute count     Get               -- Positions ("..." apart)
::Method init
  Expose strict auto named arguments count
  Use Strict Arg package, begin, end, strict, auto, named, arguments, count
  self~init:super( package, begin, end )

/******************************************************************************/
/* USE LOCAL instruction                                                      */
/******************************************************************************/

::Routine Use.Local.Instruction

  Use Strict Arg package, begin, LOCAL

  -- INTERPRET data must not contain USE LOCAL.
  If .Options.Interpret Then Signal 99.915

  -- Not the first instruction (a label before it too): ooRexx checks that
  -- before the list.
  If package~sourceLiteralStack~isEmpty, -
    \ package~preClauser~firstInBody Then Signal 99.910

  list          = Array()
  name          = TheElementAfter( LOCAL )
  ignoreBlanks  = .True

  body          = package~currentBodies~top
  UseLocal      = Set()
  body~UseLocal = UseLocal
  locals        = body~locals

  Loop
    Select
      When name < .EL.END_OF_CLAUSE Then Leave
      When name < .ALL.VAR_SYMBOLS  Then Do
        If name < .EL.COMPOUND_VARIABLE Then Signal 99.948
        name~setAssigned
        list~append( name )
        UseLocal[]         = name~value
        locals[name~value] = name~source
      End
      -- A constant symbol, as ooRexx: 31.3 (a period first), 31.2.
      When name < .ALL.SYMBOLS Then Do
        If Left( name~value, 1 ) == "." Then Signal 31.003
        Signal 31.002
      End
      Otherwise Signal 20.927
    End
    name = TheElementAfter( name, ignoreBlanks )
  End

  Return .Use.Local.Instruction~new( package, begin, name, list )

-- Symbol expected after USE LOCAL.
20.927: Syntax( 20.927, LOCAL )

-- Variable symbol must not start with a number; found "&1".
31.002: Syntax( 31.002, LOCAL, name )

-- Variable symbol must not start with a "."; found "&1".
31.003: Syntax( 31.003, LOCAL, name )

-- USE LOCAL cannot process compound variables; found "&1".
99.948: Syntax( 99.948, LOCAL, name )

-- INTERPRET data must not contain USE LOCAL.
99.915: Syntax( 99.915, LOCAL )

-- USE LOCAL must be the first instruction executed after a method invocation.
99.910: Syntax( 99.910, LOCAL )

--------------------------------------------------------------------------------

::Class Use.Local.Instruction Public SubClass Rexx.Instruction
::Attribute local
::Method init
  Expose local
  Use Strict Arg package, begin, end, local
  self~init:super( package, begin, end )

/******************************************************************************/
/******************************************************************************/
/* COMMON CODE                                                                */
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Code common to DROP, EXPOSE and PROCEDURE EXPOSE                           */
/******************************************************************************/

::Routine Name.List.Instruction Public
  Use Strict Arg package, class, begin, end, elements

  keyword = elements[1]

  If class == .Procedure.Instruction Then element = elements[3]
  Else                                    element = elements[2]

  inExpose = class == .Expose.Instruction

  If element == .Nil Then Call 20.902 elements[1]

  ignoreBlanks = .True

  nameList = Array( )

  Loop Until element < .EL.END_OF_CLAUSE
    If element < .EL.LEFT_PARENTHESIS Then Do
      element = TheElementAfter( element, ignoreBlanks )
      Call GetASymbol 1
      pName = Symbol.Term( -
        package,           -
        element,           -
        inExpose           -
      )
      -- EXPOSE (x) exposes x (and the variables it names): the symbol, not
      -- the ")" that follows it, and only for EXPOSE (not DROP or PROCEDURE).
      If inExpose Then
        package~currentBodies~top~exposed[] = element~value
      element = TheElementAfter( element, ignoreBlanks )
      If element  < .EL.END_OF_CLAUSE     Then Signal 46.901
      If element \< .EL.RIGHT_PARENTHESIS Then Signal 46.001
      nameList~append( Array( pName ) )
    End
    Else Do
      Call GetASymbol 0
      nameList~append( Symbol.Term(                  -
        package,                                     -
        element,                                     -
        inExpose                                     -
      ) )
      If class == .Expose.Instruction Then Do
        package~currentBodies~top~exposed[]        = element~value
        package~currentBodies~top~exposedSymbols[] = element~value
      End
    End
    element = TheElementAfter( element, ignoreBlanks )
  End

  Return class~new( package, begin, end, nameList )

GetASymbol:
  Arg insideParens
  If element \< .ALL.SYMBOLS Then
    If insideParens                 Then Signal 20.906
    Else
      If class == .Drop.Instruction Then Signal 20.901
      Else                               Call   20.902 element
  c = Left(element~value,1)
  If c == "."                       Then Signal 31.003
  If c >= "0", c <= "9"             Then Signal 31.002
  element~setAssigned
  Return

-- Variable symbol must not start with a number; found "&1".
31.002: Syntax( 31.002, keyword, element )

-- Variable symbol must not start with a "."; found "&1".
31.003: Syntax( 31.003, keyword, element )

-- Symbol expected after DROP keyword.
20.901: Syntax( 20.901, element )

-- Symbol expected after EXPOSE keyword.
20.902: Syntax( 20.902, Arg(1) )

-- Symbol expected after "(" of a variable reference.
20.906: Syntax( 20.906, element )

-- Extra token ("&1") found in variable reference list; ")" expected.
46.001: Syntax( 46.001, keyword, element )

-- Missing ")" in variable reference.
46.901: Syntax( 46.901, keyword )

/******************************************************************************/
/* OPTIONAL.TEMPLATE.LIST                                                     */
/*   Code common to all instructions that consist of a single keyword         */
/*   followed by an optional template list, like ARG or PULL                  */
/******************************************************************************/

::Routine Optional.Template.List Public

  Use Strict Arg package, class, begin, end, elements

  Return class~new(                  -
    package,                         -
    begin,                           -
    end,                             -
    Template.List(                   -
      package,                       -
      TheElementAfter( elements[1] ) -
    )                                -
  )

--------------------------------------------------------------------------------

::Class Optional.Template.List Public SubClass Rexx.Instruction
::Constant childAttributes "TEMPLATES"
::Attribute templates
::Method init
  Expose templates
  Use Strict Arg package, begin, end, templates = .Nil
  self~init:super( package, begin, end )

/******************************************************************************/
/* TEMPLATE.LIST                                                              */
/******************************************************************************/

::Routine Template.List Public

  Use Strict Arg package, begin

  templateList       = .Template.List~new()
  templateList~begin = begin
  element            = begin
  trailingComma      = 0
  ignoreBlanks       = .True

  Loop While element \< .EL.END_OF_CLAUSE

    Do While element < .EL.COMMA
      templateList~append( .Nil )
      element = TheElementAfter( element, ignoreBlanks )
      trailingComma = 1
    End

  If element < .EL.END_OF_CLAUSE Then Leave

    trailingComma = 0

    template = Template( package, element )

    templateList~append( template )

    element = template~end

    -- A template that ends in an element (a "." or a string) ends just
    -- after it: blanks may come before the comma.
    If element < .ALL.WHITESPACE_LIKE Then Do
      element~ignorable = 1
      element = TheElementAfter( element, ignoreBlanks )
    End

  If element \< .EL.COMMA Then Leave

    element = TheElementAfter( element, ignoreBlanks )
    trailingComma = 1

  End

  If trailingComma Then templateList~append(.Nil)

  templateList~end = element

  Return templateList

--------------------------------------------------------------------------------

::Class Template.List SubClass Array Inherit Rexx.Node
::Attribute begin
::Attribute end

--------------------------------------------------------------------------------
--                                                                            --
-- TEMPLATE (See ANSI 6.3.2.87-95):                                           --
--                                                                            --
--   template                  := (trigger | target | Msg38.1)+               --
--     target                  := VAR_SYMBOL | '.'                            --
--     trigger                 := pattern | positional                        --
--       pattern               := STRING | vrefp                              --
--         vrefp               := '(' (VAR_SYMBOL | Msg19.7) (')' | Msg46.1)  --
--       positional            := absolute_positional | relative_positional   --
--         absolute_positional := NUMBER | '=' position                       --
--           position          := NUMBER | vrefp | Msg38.2                    --
--         relative_positional := ('+' | '-') position                        --
--                                                                            --
--   ooRexx modifies the definition of "target" to also include message terms --
--   and allows full expressions between parenthesis in vrefp; additionally,  --
--   "length positional patterns" are introduced, starting with "<" and ">".  --
--                                                                            --
--------------------------------------------------------------------------------

::Routine Template Public

  Use Strict Arg package, begin

  template = .Template~new
  template~begin = begin

  ignoreBlanks = .true

  element = begin

  terminators      = .EL.COMMA || .EL.END_OF_CLAUSE

  -- A positional pattern that is not a whole number: 26.4 when it runs, as
  -- in ooRexx; here, only with earlyCheck PARSE.
  earlyCheck = .Parser.Options~earlyCheck~hasItem( "PARSE" )

  Loop Until element < terminators

    Select
      When element < .ALL.VAR_SYMBOLS Then Do

        -- Mark the element as assigned
        element~setAssigned

        -- Build a symbol term
        term = Symbol.Term( package, element )

        -- Now collect "[args]" and "~[~]method(args)" sequences
        term = Simple.Message.Term( package, term )

        element = term~end

        -- Consume blanks after ")" and "]"
        If element < .ALL.WHITESPACE_LIKE Then Do
          element~ignorable = 1
          ignoreBlanks = .True
          element = TheElementAfter( element, ignoreBlanks )
        End

        template~append(term)    -- target

        Iterate

      End
      When element < .EL.PERIOD      Then Do
        Call SetCategory element, .EL.PARSE_PERIOD
        template~append(element) -- target
      End
      When element < .ALL.STRINGS Then
        template~append(element)
      When element < .EL.ENVIRONMENT_SYMBOL Then Do
        -- An environment symbol can be the start of a message term target
        -- (e.g., .local~myvar). Check for a following message send operator.
        next = element~next
        If next \== .Nil, next < .ALL.OPS.MESSAGE_SEND Then Do
          term = Symbol.Term( package, element )
          term = Simple.Message.Term( package, term )
          element = term~end
          If element < .ALL.WHITESPACE_LIKE Then Do
            element~ignorable = 1
            ignoreBlanks = .True
            element = TheElementAfter( element, ignoreBlanks )
          End
          template~append(term)    -- target (message term)
          Iterate
        End
        -- Env symbol without message send: a target that is not a
        -- variable, 31.3 as in ooRexx (".x", ".local", "..")
        Signal 31.003
      End
      When element < .ALL.SYMBOLS Then Do
        If earlyCheck, \DataType(element~value,"I") Then Signal 26.004
        template~append(element)
      End
      When element < .ALL.POSITION_PREFIXES  Then Do
        positional = Positional.Trigger( package, element )
        template~append( positional )
        element = positional~end
        If element < .ALL.WHITESPACE_LIKE Then
          element = TheElementAfter( element, ignoreBlanks )
        Iterate
      End
      When element < .EL.LEFT_PARENTHESIS           Then Do
        subExpression = Subexpression.Template( package, element )
        template~append( subExpression )
        element = subExpression~end
        If element < .ALL.WHITESPACE_LIKE Then
          element = TheElementAfter( element, ignoreBlanks )
        Iterate
      End
      Otherwise Signal 38.001
    End

    element = TheElementAfter( element, ignoreBlanks )

  End

  If template~items == 0 Then
    template~end = template~begin
  Else Do
    lastItem = template~lastItem
    If lastItem~isA(.Element) Then
      template~end = lastItem~next
    Else
      template~end = lastItem~end
  End

  Return template

-- Positional pattern of PARSE template must be a whole number; found "&1".
26.004: Syntax( 26.004, begin, element )

-- Incorrect PARSE template detected at "&1".
38.001: Syntax( 38.001, begin, element )

-- Variable symbol must not start with a "."; found "&1".
31.003: Syntax( 31.003, begin, element )

--------------------------------------------------------------------------------

::Class Template SubClass Array Inherit Rexx.Node
::Attribute begin
::Attribute end

/******************************************************************************/
/* POSITIONAL.TRIGGER                                                         */
/*   ("=" | "+" | "-" | ">" | "<") position                                   */
/******************************************************************************/

::Routine Positional.Trigger Public

  Use Strict Arg package, prefix

  ignoreBlanks = .True

  element = TheElementAfter( prefix )

  -- Not a whole number: 26.4 when it runs (see Template).
  earlyCheck = .Parser.Options~earlyCheck~hasItem( "PARSE" )

  -- A number or a constant symbol (ooRexx: any symbol that is not a
  -- variable, as "1y" or ".x").
  If element < ( .ALL.NUMBERS || .ALL.CONST_SYMBOLS ) Then Do
    If earlyCheck, \DataType(element~value,"I") Then Signal 26.004
    Return .Prefixed.Template~new( prefix, element )
  End

  If element < .EL.LEFT_PARENTHESIS Then
    Return .Prefixed.Template~new(                       -
      prefix, Subexpression.Template( package, element ) -
    )

  -- Nothing after the prefix: 38.901, as ooRexx.
  If element < .EL.END_OF_CLAUSE Then Signal 38.901

  Signal 38.002

-- Positional pattern of PARSE template must be a whole number; found "&1".
26.004: Syntax( 26.004, prefix, element )

-- Incorrect PARSE position detected at "&1".
38.002: Syntax( 38.002, prefix, element )

-- Missing PARSE relative position.
38.901: Syntax( 38.901, prefix )

--------------------------------------------------------------------------------

::Class Prefixed.Template Public Inherit Rexx.Node
::Constant childAttributes "ELEMENTORSUBEXPRESSION"
::Attribute prefix Get
::Attribute elementOrSubExpression Get
::Attribute begin
::Attribute end
::Method init
  Expose prefix elementOrSubExpression begin end
  Use Strict Arg prefix, elementOrSubExpression
  -- A prefixed template begins at its prefix ("+", "-", "=", "<", ">")
  begin = prefix
  If elementOrSubExpression~isA( .Element ) Then
    end   = TheElementAfter( elementOrSubExpression )
  Else
    end   = elementOrSubExpression~end

/******************************************************************************/
/* SUBEXPRESSION.TEMPLATE                                                     */
/******************************************************************************/

::Routine Subexpression.Template

  Use Strict Arg package, lParen

  Call PrepareExpression package, lParen, ")"

  subExpression = SubExpression( package, lParen )

  element = subExpression~end

  -- Generally speaking, a closing parenthesis does not eat blanks
  -- to the right, but in this case we do need to ignore such blanks,
  -- if present.

  If element < .ALL.WHITESPACE_LIKE Then Do
    next = TheElementAfter( element )
    element~ignorable = 1
    subExpression~end = next
  End

  Return subExpression
/******************************************************************************/
/* FinallyExitCheck (RFE #31)                                                 */
/*                                                                            */
/*   The rule of FINALLY: the code of a FINALLY clause can only end by        */
/*   reaching its end, or by raising a condition. RETURN, EXIT, SIGNAL (to a  */
/*   label, or VALUE), FORWARD (not CONTINUE), REPLY and RAISE with RETURN or */
/*   EXIT (the last two, since ooRexx r13535) in it are 99.951, as in         */
/*   LanguageParser::checkFinallyExit(); LEAVE and ITERATE are checked when   */
/*   their target is looked for. The clause is on the block instructions      */
/*   stack while its code is parsed (Clauses.cls, Catch.Sections).            */
/******************************************************************************/

::Routine FinallyExitCheck Public
  Use Strict Arg package, keyword, name

  stack = package~blockInstructionsStacks~top
  Do i = stack~items To 1 By -1
    clause = stack[i]
    If clause~isA( .Finally.Clause ) Then Signal 99.951
  End
  Return

-- &1 must not leave the FINALLY clause on line &2.
99.951: Syntax( 99.951, keyword, name, -
  TheElementAfter( clause~begin )~from~word(1) )
