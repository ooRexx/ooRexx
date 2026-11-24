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
/* parser.cls -- The main Rexx Parser class                                   */
/* ========================================                                   */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241206    0.1  First public release                                      */
/* 20241208    0.1a Pass options arg to clauser                               */
/* 20250103    0.1f Add unicode option                                        */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250419    0.2a Fix bug no. 4                                             */
/* 20250419    0.2a Fix bug no. 4                                             */
/* 20251128    0.3a (Executor) Add support for ::EXTENSION directive          */
/* 20251201         (Executor) Add support for source literals                */
/* 20260521    0.5  Add support for CMS                                       */
/* 20260601    0.6  Implement a fine-grained feature system                   */
/* 20260924    0.7  INTERPRET option: the source is INTERPRET data            */
/* 20260926         METHOD option: the prolog is the body of a method (the    */
/*                  source of Method~new: EXPOSE, USE LOCAL, GUARD).          */
/*                  EXPOSE and USE LOCAL as the first instruction out of a    */
/*                  method are errors when they run (98.992, 98.993), as in   */
/*                  ooRexx; at parse time only with earlyCheck EXPOSE and     */
/*                  USELOCAL.                                                 */
/* 20260928         Rexx.Package~constantCalls: the function calls of the     */
/*                  ::CONSTANT expressions, which are in no body.             */
/*                  ::ANNOTATE: the target must be defined before (99.945),   */
/*                  as in ooRexx.                                             */
/*                  ::METHOD and ::ATTRIBUTE: an EXTERNAL specification that  */
/*                  is not "LIBRARY lib [entry]" is 99.917 (in ooRexx's       */
/*                  order with the body errors).                              */
/* 20260930         EXPOSE out of a method (98.992): the error has a hint     */
/*                  (issue #39).                                              */
/* 20261001         Code.Body~exposedSymbols: the symbols of the EXPOSE list  */
/*                  (not "(x)"), what the GUARD check (99.913) looks at.      */
/* 20261020         A CR at the end of a source line is ignored, as in ooRexx */
/*                  (a file whose last line ends in a lone CR failed with     */
/*                  13.1).                                                    */
/* 20261113         Features.Catch.Clauses: CATCH and FINALLY (RFE #31), not  */
/*                  under CMS or Executor                                     */
/* 20261118         Features.Throw.Instruction: THROW (RFE #31, proposed),    */
/*                  not under CMS or Executor                                 */
/* 20261119         A syntax error in the source is a Rexx.Parser.Error,      */
/*                  thrown (THROW, RFE #31): init gives it the name of the    */
/*                  program and, when its clause is not known, its line       */
/* 20261121         Rename to parser.cls                                      */
/* 20261121         No ::Requires of HTMLColors.cls and StyleSheet.cls: the   */
/*                  parser does not use them, and the highlighter's drivers  */
/*                  require them themselves                                   */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261121         .Rexx.Parser~version: the release and the refresh, so     */
/*                  that a client can check the parser it has, also when it   */
/*                  comes with ooRexx and there is no readme.md (RFE #878)    */
/* 20261121         options may be .Nil, as when it is omitted                */
/* 20261121         A file that does not exist or cannot be read is a SYNTAX  */
/*                  error 3.1, as in Routine~newFile (it was an empty         */
/*                  program)                                                  */
/* 20261124         hasClass and hasRoutine: the checks for a duplicate class */
/*                  or routine, and for the target of ::ANNOTATE, built the   */
/*                  union of the private and public ones every time (O(n^2)   */
/*                  for a package with many). Noted by the ooRexx bugs chain  */
/* 20261124         The tree, upwards: the parent of a body is its directive  */
/*                  (the prolog's, the package); of a directive and of the    */
/*                  End.Of.Source, the package, whose parent is .nil.         */
/*                  Rexx.Package~endOfSource                                  */
/*                                                                            */
/******************************************************************************/

 .environment~Code.Body    = .Code.Body
 .environment~Rexx.Parser  = .Rexx.Parser
 .environment~Rexx.Package = .Rexx.Package
 .environment~Rexx.Routine = .Rexx.Routine

 .environment~Stack        = .Stack

::Requires "parser/DebugSettings.cls"
::Requires "parser/Globals.cls"
::Requires "parser/BaseClassesAndRoutines.cls"
::Requires "parser/UnicodeSupport.cls"
::Requires "parser/Scanner.cls"
::Requires "parser/PreClauser.cls"
::Requires "parser/Clauses.cls"
::Requires "parser/Elements.cls"
::Requires "parser/Expressions.cls"
::Requires "parser/KeywordInstructions.cls"
::Requires "parser/Directives.cls"

::Requires "parser/SecondPass.cls"


/******************************************************************************/
/*                                                                            */
/* The Rexx.Parser class                                                      */
/*                                                                            */
/******************************************************************************/

::Class Rexx.Parser Public

-- The version of the parser: the release and the refresh (YYYYMMDD), as in
-- "The current release is beta 0.7, refresh 20261121." of the readme.md of
-- the project. Two words, so that a client can say
--
--   Parse Value .Rexx.Parser~version With release refresh
--
-- and compare the refresh, a whole number, with the one it needs.
-- ****  Change it with every refresh, as the line of the readme.  ****
-- (A test of RexxPub, in Md2slidesVersion.testGroup, checks that both say
-- the same.)
::Constant version "0.7 20261122"

::Attribute package

::Method firstElement
  Return self~package~firstElement

::Method init
  Expose package

  Use Strict Arg name, source = .Nil, options = .Nil

  -- .Directory~of() crashes the interpreter
 .environment~Parser.Options       = .Directory~new
 .environment~Options.Experimental = 0
 .environment~Options.Executor     = 0
 .environment~Options.CMS          = 0
 .environment~Options.Unicode      = 0
 .environment~Options.Interpret    = 0
  If Arg(3,'E'), options \== .Nil Then Do
    .Validate~classType("options",options,.Array)
    If \ options~isEmpty Then Do
      -- Ensure that keys are all in uppercase
      Do option Over options
        option[1] = Upper(option[1])
      End
     .environment~Parser.Options = .Directory~sendWith("of", options)
      If .Parser.Options~hasIndex("EXPERIMENTAL") Then
        .environment~Options.Experimental = 1
      If .Parser.Options~hasIndex("EXECUTOR") Then
        .environment~Options.Executor     = 1
      If .Parser.Options~hasIndex("CMS") | -
         .Parser.Options~hasIndex("REXXVM") Then
        .environment~Options.CMS          = 1
      If .Parser.Options~hasIndex("UNICODE") Then
        .environment~Options.Unicode      = 1
      -- INTERPRET data: labels, directives, EXPOSE, FORWARD, GUARD,
      -- REPLY and USE LOCAL are not allowed.
      If .Parser.Options~hasIndex("INTERPRET") Then
        .environment~Options.Interpret    = 1
    End
  End

  -- Default options

  If \.Parser.Options~hasIndex( docComments ) Then
    .Parser.Options[ docComments ] = All

  If \.Parser.Options~hasIndex( earlyCheck ) Then
    .Parser.Options[ earlyCheck ] = Set()

  -- No source: name is a file. A file that does not exist or cannot be
  -- read is error 3.1, as in Routine~newFile
  If Arg(2,'O') | Arg(2) == .Nil Then Do
    If \SysIsFile(name) | Stream(name, "C", "Query Exists") == "" Then
      Raise Syntax 3.1 Array(name)
    If Stream(name, "C", "Open Read") \= "READY:" Then
      Raise Syntax 3.1 Array(name)
    Call Stream name, "C", "Close"
    source = LoadProgramSource(name)
  End
  Else If source~isA( .String ) Then source = Array( source )

  -- ooRexx ignores a carriage return at the end of a line. makeArray and
  -- LINEIN already drop the CR of a CR LF pair, but a file whose last line
  -- ends in a lone CR (no final LF) keeps it, and so may an array built
  -- by other means. Drop it here too, working on a copy so that the
  -- caller's array is left untouched.
  copied = 0
  Do i = 1 To source~size
    line = source[i]
    If \line~isA(.String) Then Iterate
    If line~right(1) \== "0D"X Then Iterate
    If \copied Then Do
      source = source~copy
      copied = 1
    End
    source[i] = line~left( line~length - 1 )
  End

  -- Features

 .environment~Features.Address.With                = \.Options.CMS
 .environment~Features.Array.Terms                 = \.Options.CMS
 -- CATCH and FINALLY (RFE #31)
 .environment~Features.Catch.Clauses               = \.Options.CMS -
                                                   & \.Options.Executor
 .environment~Features.Continuation.Minus          = \.Options.CMS
 .environment~Features.Do.Counter                  = \.Options.CMS
 .environment~Features.Do.Label                    = \.Options.CMS
 .environment~Features.Do.Over                     = \.Options.CMS
 .environment~Features.Do.With                     = \.Options.CMS
 .environment~Features.Directive.Start             = \.Options.CMS
 .environment~Features.Empty.Assignments           =  .Options.CMS | .Options.Executor -
   | Global.Option("emptyassignments") == 1
 .environment~Features.Evaluated.Call              = \.Options.CMS
 .environment~Features.Extended.Assignments        = \.Options.CMS
 .environment~Features.Expose.Instruction          = \.Options.CMS
 .environment~Features.Forward.Instruction         = \.Options.CMS
 .environment~Features.General_Letter.At_Sign      =  .Options.CMS | .Options.Executor
 .environment~Features.General_Letter.Cent.Latin1  =  .Options.CMS | .Options.Executor
 .environment~Features.General_Letter.Cent.UTF8    =  .Options.CMS | .Options.Executor
 .environment~Features.General_Letter.Dollar       =  .Options.CMS | .Options.Executor
 .environment~Features.General_Letter.Number_Sign  =  .Options.CMS | .Options.Executor
 .environment~Features.Guard.Instruction           = \.Options.CMS
 .environment~Features.Line.Comments               = \.Options.CMS
 .environment~Features.Loop.Instruction            = \.Options.CMS
 .environment~Features.Multiline.Strings           =  .Options.CMS
 .environment~Features.Namespace.Qualified.Symbols = \.Options.CMS
 .environment~Features.Operator.Circumflex.UTF8    =  .Options.Executor
 .environment~Features.Operator.NotSign.UTF8       =  .Options.CMS | .Options.Executor
 .environment~Features.Operator.Tilde.UTF8         = \.Options.CMS
 .environment~Features.Parse.Caseless              = \.Options.CMS
 .environment~Features.Parse.External              =  .Options.CMS
 .environment~Features.Parse.Lower                 = \.Options.CMS
 .environment~Features.Parse.Numeric               =  .Options.CMS
 .environment~Features.Raise.Instruction           = \.Options.CMS
 .environment~Features.Reply.Instruction           = \.Options.CMS
 .environment~Features.Select.Case                 = \.Options.CMS
 .environment~Features.Select.Label                = \.Options.CMS
 .environment~Features.Slash_as_negator            =  .Options.CMS | .Options.Executor
 .environment~Features.Special.Brackets            = \.Options.CMS
 .environment~Features.Special.Braces              =  .Options.Executor
 .environment~Features.Trace.CMS                   =  .Options.CMS
 -- THROW (RFE #31, proposed), as CATCH and FINALLY
 .environment~Features.Throw.Instruction           = \.Options.CMS -
                                                   & \.Options.Executor
 .environment~Features.Upper.Instruction           =  .Options.CMS | .Options.Executor
 .environment~Features.Use.Instruction             = \.Options.CMS
 If .Options.CMS Then
   .environment~Features.Signal.OnOff.Conditions   = -
     "ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX "
 Else
   .environment~Features.Signal.OnOff.Conditions   = -
     "ERROR FAILURE HALT LOSTDIGITS NOMETHOD"        -
     "NOSTRING NOTREADY NOVALUE SYNTAX USER ANY "

  -- Create the package object, and fire the parser. A syntax error in the
  -- source (a Rexx.Parser.Error, thrown) gets its name and, when the clause
  -- in error is not known, its line
  Do
    package = .Rexx.Package~new(name, source)
    Call Rexx.Package package
  Catch co = User Rexx.Parser.Error
    co~additional~in( name )~withSource( source )~throw
  End

/******************************************************************************/
/* CODE.BODY                                                                  */
/*                                                                            */
/*   A code body is an instruction list ended by an implicit EXIT.            */
/*   It may be the prolog of a package, or the body of a routine or a method. */
/*                                                                            */
/******************************************************************************/

::Routine Code.Body Public

  Use Strict Arg package, directive

  inAMethodBody = directive~class == .Method.Directive
  -- The METHOD option: the prolog is a method's body (Method~new).
  If directive == .Nil, .Parser.Options~hasIndex("METHOD") Then
    inAMethodBody = 1

  If directive~isA(.Attribute.Directive) Then Do
    -- Logic for attributes w and w/o bodies:
    --   No GET, no SET -> No body
    --   GET or SET     -> Body is optional
    --   ABSTRACT       -> No body
    options. = directive~options.
    If options.get | options.set Then inAMethodBody = 1
    If options.abstract          Then inAMethodBody = 1
  End

  body = .Code.Body~new( package, directive )
  -- The parent of a body is its directive; that of the prolog, the package
  If directive == .Nil Then body~setParent( package )
  Else                      body~setParent( directive )
  If inAMethodBody Then
    body~isAMethodBody = .True
  package~currentBodies~push(body)
  package~blockInstructionsStacks~push(.Stack~new())

  -- The following loop will always end, because lower levels of the parser
  -- have taken care that all clause sequences are ended by an implicit exit
  -- instruction. We keep all instructions in the list to be able to
  -- accurately determine the beginning and the end of the code block.

  NoFinalENDClause = 0
  checkguard       = 0
  checkexpose      = 0
  checkuselocal    = 0
  ExposeFound      = 0

  If .Parser.Options~hasIndex( earlyCheck ) Then Do
    checkguard     = .Parser.Options~earlyCheck~hasItem( guard )
    checkexpose    = .Parser.Options~earlyCheck~hasItem( expose )
    checkuselocal  = .Parser.Options~earlyCheck~hasItem( uselocal )
  End

  -- TODO EXPOSE, USE LOCAL and GUARD should be checked recursively.

  Loop Counter n Until instruction~class == .Implicit.Exit.Instruction
    instruction = package~clauser~Next.Instruction( NoFinalENDClause )
    Select Case instruction~class
      When    .Expose.Instruction Then Call CheckThatExposeIsValid
      When   .Use.Arg.Instruction Then Call ProcessUseArg
      When .Use.Local.Instruction Then Call CheckThatUseLocalIsValid
      When     .Guard.Instruction Then Call CheckThatGuardIsValid
      Otherwise Nop
    End
    body~addInstruction( instruction )
  End

  package~currentBodies~pop
  package~blockInstructionsStacks~pop

  Return body

ProcessUseArg:
  If n == 1 | (n == 2 & ExposeFound) Then body~signature =  instruction
  Else                                    body~signature = .Nil
Return

CheckThatGuardIsValid:
  If \checkGuard Then Return
  If inAMethodBody Then Return
  Signal 99.911

-- GUARD can only be issued in an object method invocation.
99.911: Syntax( 99.911, TheElementAfter(instruction~begin) )

-- EXPOSE as the first instruction out of a method is an error when it runs
-- (ooRexx raises 98.992 then); at parse time, only as an early check.
CheckThatExposeIsValid:
  ExposeFound = 1
  If n == 1 Then Do
    If inAMethodBody Then Return
    If \checkExpose Then Return
    Signal 98.992
  End
  Signal 99.907

-- EXPOSE must be the first instruction executed after a method invocation.
99.907: Syntax( 99.907, TheElementAfter(instruction~begin) )

-- The EXPOSE instruction may only be used from method invocations.
-- The hint (issue #39) says why the body is not a method's.
98.992:
  If directive~isA(.Routine.Directive) Then
    hint = "A ::ROUTINE has no object variables: EXPOSE is only valid in a" -
      "method. Pass the values as arguments, or use a ::METHOD."
  Else If directive == .Nil Then
    hint = "The prolog of a program is not a method: EXPOSE is only valid" -
      "in a method."
  Else Syntax( 98.992, TheElementAfter(instruction~begin) )
  SyntaxWithHint( hint, 98.992, TheElementAfter(instruction~begin) )

-- The same for USE LOCAL (98.993).
CheckThatUseLocalIsValid:
  If n == 1 Then Do
    If inAMethodBody Then Return
    If \checkUseLocal Then Return
    Signal 98.993
  End
  Signal 99.910

-- USE LOCAL must be the first instruction executed after a method invocation.
99.910: Syntax( 99.910, TheElementAfter(instruction~begin) )

-- The USE LOCAL instruction may only be used from method invocations.
98.993: Syntax( 98.993, TheElementAfter(instruction~begin) )

--------------------------------------------------------------------------------
-- The CODE.BODY class                                                        --
--   We implement it as a form of block instruction                           --
--------------------------------------------------------------------------------

::Class Code.Body SubClass Block.Instruction

::Constant tree "instructions"

::Attribute labels              -- All non trace-only labels
::Attribute clauses             -- All clauses
::Attribute exposed             -- Explicitly EXPOSEd variables
::Attribute exposedSymbols      -- Those of them that are symbols of the
                                -- EXPOSE list, not "(x)" (the GUARD check)
::Attribute uselocal            -- USE LOCAL variables (.Nil -> no USE LOCAL)
::Attribute locals              -- All local variables
::Attribute local               -- The body local directory
::Attribute isAMethodBody       -- This is a METHOD body
::Attribute classMethod         -- True when this is a CLASS method
::Attribute calledRoutineNames  -- Symbols or strings
::Attribute instructions        -- Array of instructions
::Attribute directive           -- .Nil for the prolog
::Attribute blockID             -- Used to identify DO/LOOP/SELECT blocks

-- The "signature" attribute is a simple aid for compiler and transpiler
-- implementors. If the body contains only a single USE ARG instruction,
-- AND this instruction is the first in the body (or the second after
-- EXPOSE, in the case of method bodies), AND the body does not have
-- any ARG or PARSE ARG instruction, then "signature" contains that
-- USE ARG instruction. Otherwise, "signature" is Nil.
::Attribute signature

::Method "[]"
  Expose instructions
  Use Strict Arg index
  Return instructions[index]

::Method init
  Expose instructions labels clauses calledRoutineNames -
    exposed exposedSymbols useLocal classMethod isAMethodBody -
    usesExitInstruction signature directive blockID locals local

  Use Strict Arg package, directive

  labels              = .StringTable~new
  clauses             = Array()
  calledRoutineNames  = Array()
  instructions        = .Instruction.List~new()
  exposed             = Set()
  exposedSymbols      = Set()
  useLocal            = .Nil
  classMethod         = .False
  isAMethodBody       = .False
  signature           = .Nil
  blockID             = 0
  locals              = .StringTable~new
  local               = .Directory~new

  If directive~class == .Method.Directive Then
    classMethod = directive~isClassMethod

  -- Set begin and end to nil, will be updated by addInstruction
  self~init:super( package, .Nil, .Nil )

::Method addInstruction
  Expose instructions exposed useLocal

  Use Strict Arg instruction

  -- This is directly called only for top-level instructions.
  --
  -- We update the body labels stringtable with all the non-trace-only
  -- top-level labels.

  labels     = instruction~labels       -- Our labels
  bodyLabels = self~labels              -- Pool of non trace-only labels
  Loop theLabel Over labels             -- One by one...
    value  = theLabel~label~value       -- ...pick the value (may be lowercase)
    If bodyLabels~hasIndex(value) Then  -- Duplicate label? That is allowed,...
      Nop                               -- ... but this is trace-only.
    Else Do                             -- A new, non trace-only label
      bodyLabels[value] = theLabel      -- Store it in the body label pool...
      theLabel~traceOnly = .False       -- ...and mark as not trace-only.
      theLabel~label~traceOnly = .False -- Mark the element itself too.
    End
  End

  -- Update self~begin and self~end to accomodate the new instruction
  If self~begin == .Nil Then Do
    nullClauses = instruction~nullClauses
    If nullClauses \== .Nil, nullClauses~items > 0
      Then null  = nullClauses[1]~begin
      Else null  = .Nil
    If labels      \== .Nil, labels     ~items > 0
      Then label = labels[1]~begin
      Else label = .Nil
    If null == .Nil Then
      If label == .Nil
        Then self~begin = instruction~begin
        Else self~begin = labels[1]~begin
    Else If label == .Nil Then self~begin = null
    Else Do
      Parse Value null ~from With  nullFrom  nullTo
      Parse Value label~from With labelFrom labelTo
      If      nullFrom < labelFrom Then self~begin = null
      Else If nullFrom > labelFrom Then self~begin = labels[1]~begin
      Else If nullTo   < labelTo   Then self~begin = null
      Else                              self~begin = labels[1]~begin
    End
  End
  self~end = instruction~end

  instructions~append( instruction )
  Call AdoptClause self, instruction

::Method isEmpty
  Expose instructions
  -- More than 1 instruction? Not empty
  If instructions~items > 1 Then Return .False
  -- Only 1 instruction (implicit exit) + no labels: empty
  Return instructions[1]~labels~isEmpty

::Method makeString
  Return self~makeArray
::Method makeArray
  Expose instructions
  Use Strict Arg indent = 0
  array = Array()
  Do i = 1 To instructions~items
    instruction = instructions[i]~makeArray(indent)
    Do j = 1 To instruction~items
      array~append(instruction[j])
    End
  End
  Return array

/******************************************************************************/
/* REXX.PACKAGE                                                               */
/******************************************************************************/

::Routine Rexx.Package Public

  Use Strict Arg package

  -- The package prolog is the first code body
  prolog = Code.Body( package, .Nil )
  package~addProlog( .Rexx.Routine~new("", prolog, .Nil ) )
  package~begin = prolog~begin

  Loop

    directive = package~clauser~next.Instruction.Fragment()

    Select Case directive~class

      When       .End.Of.Source Then Do
        directive~setParent( package )
        package~setEndOfSource( directive )
        Leave
      End

      When  .Annotate.Directive  Then Call Annotate   package, directive
      When .Attribute.Directive  Then Call Attribute  package, directive
      When     .Class.Directive  Then Call Class      package, directive
      When  .Constant.Directive  Then Call Constant   package, directive
      When .Extension.Directive  Then Call Extension  package, directive
      When    .Method.Directive  Then Call Method     package, directive
      When   .Options.Directive  Then Call Options    package, directive
      When  .Requires.Directive  Then Call Requires   package, directive
      When  .Resource.Directive  Then Call Resource   package, directive
      When   .Routine.Directive  Then Call Routine    package, directive

      -- These directives are only valid inside a source literal

      When .Coactivity.Directive Then Signal 99.916
      When          .R.Directive Then Signal 99.916

    End

  End

  package~end = directive~end

  Call SecondPass package

  Return package

-- Unrecognized directive instruction.
99.916: Syntax( 99.916, directive~begin )

--------------------------------------------------------------------------------

--------------------------------------------------------------------------------

::Class Rexx.Package Public

::Attribute options                 -- Global options, when NetRexxOptions = 1
::Attribute Clauser                 -- The clauser
::Attribute preClauser              -- The preClauser
::Attribute scanner                 -- The scanner
::Attribute currentClass            -- Current class (.Nil --> prolog)
::Attribute currentBodies           -- Stack of bodies being currently parsed
::Attribute constantCalls           -- Function calls in ::CONSTANT exprs
::Attribute directives              -- Sequential list of directives
::Attribute source                  -- [Similar to the ooRexx Package class]
::Attribute prolog                  -- [Similar to the ooRexx Package class]
::Attribute name                    -- [Similar to the ooRexx Package class]
::Attribute resources               -- [Similar to the ooRexx Package class]
::Attribute publicClasses           -- [Similar to the ooRexx Package class]
::Attribute privateClasses
::Attribute publicRoutines          -- [Similar to the ooRexx Package class]
::Attribute privateRoutines
::Attribute definedMethods          -- [Similar to the ooRexx Package class]
::Attribute blockInstructionsStacks -- Stack of stacks of active block instructions
::Attribute sourceLiteralStack      -- Stack of active source literals
::Attribute scannerStack            --
::Attribute begin
::Attribute end

::Constant tree "prolog"

::Method firstElement -- Convenient shortcut
  Return self~prolog~body~begin

::Method inTheProlog
  Expose currentClass
  Return currentClass == .Nil

::Method init
  Expose name source        -
    options                 -
    Clauser                 -
    preclauser              -
    scanner                 -
    privateClasses          -
    publicClasses           -
    privateRoutines         -
    publicRoutines          -
    definedMethods          -
    currentClass            -
    currentBodies           -
    constantCalls           -
    directives              -
    blockInstructionsStacks -
    resources               -
    sourceLiteralStack      -
    scannerStack            -
    endOfSource

  Use Strict Arg name, source

  self~init:super

  Clauser                 = .Clauser~new( self )
  preClauser              =  Clauser~preClauser
  scanner                 =  Clauser~scanner
  definedMethods          = .StringTable~new
  privateClasses          = .StringTable~new
  publicClasses           = .StringTable~new
  privateRoutines         = .StringTable~new
  publicRoutines          = .StringTable~new
  resources               = .StringTable~new
  currentClass            = .Nil
  currentBodies           = .Stack~new
  constantCalls           = .Array~new
  directives              =  Array()
  endOfSource             = .Nil
  blockInstructionsStacks = .Stack~new
  options                 = .Set~new
  sourceLiteralStack      = .Stack~new
  scannerStack            = .Stack~new

::Method addClass -- [As in in the ooRexx Package class]
  Expose privateClasses
  Use Strict Arg name, class
  privateClasses[name] = class

::Method parent -- The package is the root of the tree
  Return .Nil

::Attribute endOfSource Get -- The End.Of.Source clause

::Method setEndOfSource
  Expose endOfSource
  Use Strict Arg endOfSource

::Method addDirective
  Expose directives
  Use Strict Arg directive
  directives~append( directive )
  directive~setParent( self )

::Method addMethod -- Add a floating method, attribute or constant
  Expose definedMethods
  Use Strict Arg name, method
  definedMethods[name] = method

::Method addResource
  Expose resources
  Use Strict Arg name, resource
  resources[name] = resource

::Method addProlog
  Expose prolog
  Use Strict Arg prolog

::Method addRoutine -- [As in in the ooRexx Package class]
  Expose privateRoutines
  Use Strict Arg name, routine
  privateRoutines[name] = routine

::Method addPublicClass -- [As in in the ooRexx Package class]
  Expose publicClasses
  Use Strict Arg name, class
  publicClasses[name] = class

::Method addPublicRoutine -- [As in in the ooRexx Package class]
  Expose publicRoutines
  Use Strict Arg name, routine
  publicRoutines[name] = routine

::Method classes -- [As in in the ooRexx Package class]
  Expose privateClasses publicClasses
  Return privateClasses~union( publicClasses )

::Method routines -- [As in in the ooRexx Package class]
  Expose privateRoutines publicRoutines
  Return privateRoutines~union( publicRoutines )

-- hasClass and hasRoutine -- Is there a class (a routine) with this name,
-- private or public? As classes~hasIndex (routines~hasIndex), without
-- building the union.

::Method hasClass
  Expose privateClasses publicClasses
  Use Strict Arg name
  Return privateClasses~hasIndex( name ) | publicClasses~hasIndex( name )

::Method hasRoutine
  Expose privateRoutines publicRoutines
  Use Strict Arg name
  Return privateRoutines~hasIndex( name ) | publicRoutines~hasIndex( name )

::Method sourceLine -- [As in in the ooRexx Package class]
  Expose source
  Use Strict Arg n
  line = source[n]
  If line~isNil Then Return ""
  Return line

/******************************************************************************/
/* ANNOTATE                                                                   */
/******************************************************************************/

::Routine Annotate
  Use Strict Arg package, directive
  options. = directive~options.
  type = options.["DIRECTIVE"]
  If type \== "PACKAGE" Then Do
    name = Upper( options.["NAME"]~value )
    If \AnnotationTargetExists( package, type, name ) Then Signal 99.945
  End
  directive~body = Empty.Body( package, directive )
  package~addDirective(directive)
  Return

-- ::ANNOTATE target &1 "&2" not found.
99.945: Syntax( 99.945, TheElementAfter( directive~begin ), Lower( type ), name )

--------------------------------------------------------------------------------
-- The target of an ::ANNOTATE, as ooRexx's annotateDirective: it has to be
-- defined before. A class or a routine of the package; a method (any: an
-- instance one, else a class one) of the active class, or a floating one
-- before the first ::CLASS; a constant is a method made by ::CONSTANT; an
-- attribute, a method made by ::ATTRIBUTE or ::METHOD ... ATTRIBUTE, the
-- getter or the setter (the instance ones, or else the class ones).

::Routine AnnotationTargetExists
  Use Strict Arg package, type, name
  Select Case type
    When "CLASS"   Then Return package~hasClass( name )
    When "ROUTINE" Then Return package~hasRoutine( name )
    Otherwise Nop
  End
  If package~inTheProlog Then Do
    instanceMethods = package~definedMethods
    classMethods    = .StringTable~new
  End
  Else Do
    instanceMethods = package~currentClass~instanceMethods
    classMethods    = package~currentClass~classMethods
  End
  Select Case type
    When "METHOD" Then
      Return instanceMethods~hasIndex( name ) | classMethods~hasIndex( name )
    When "CONSTANT" Then Do
      method = instanceMethods[ name ]
      If method == .Nil Then method = classMethods[ name ]
      Return method~isA( .Constant.Method )
    End
    When "ATTRIBUTE" Then
      Do methods Over .Array~of( instanceMethods, classMethods )
        Do n Over .Array~of( name, name"=" )
          method = methods[ n ]
          If method~isA( .Attribute.Method ) Then Return .True
          If method~isA( .Rexx.Method ), method~directive~isAttribute Then
            Return .True
        End
      End
  End
  Return .False

/******************************************************************************/
/* ATTRIBUTE                                                                  */
/******************************************************************************/

::Routine Attribute
  Use Strict Arg package, directive
  codeBody = Code.Body( package, directive )
  options. = directive~options.
  If \codeBody~isEmpty Then Do
    instructionBegin = TheElementAfter( codeBody~instructions[1]~begin )
    If options.get & options.set        Then Signal 99.937
    Else If options.abstract            Then Signal 99.940
    Else If options.delegateName \== "" Then Signal 99.947
    Else If options.external \== .Nil   Then Signal 99.935
  End
  -- The EXTERNAL specification, after the body (ooRexx's order).
  If options.external \== .Nil, \ExternalSpecIsValid( options.external ) Then
    Signal 99.917
  name = directive~name
  attribute = .Attribute.Method~new(name, directive, codeBody)
  name = Upper( name~value )
  If options.get Then Do
    If package~inTheProlog Then Do
      If package~definedMethods~hasIndex(name) Then Signal 99.931
      package~addMethod( name, attribute )
    End
    Else Do
      lastClass = package~currentClass
      If options.["CLASS"] Then Do
        If lastClass~classMethods~hasIndex(name) Then Signal 99.931
        lastClass~addClassMethod( name, attribute )
      End
      Else Do
        If lastClass~instanceMethods~hasIndex(name) Then Signal 99.931
        lastClass~addInstanceMethod( name, attribute )
      End
    End
  End
  If options.set Then Do
    name = name"="
    If package~inTheProlog Then Do
      If package~definedMethods~hasIndex(name) Then Signal 99.931
      package~addMethod( name, attribute )
    End
    Else Do
      lastClass = package~currentClass
      If options.["CLASS"] Then Do
        If lastClass~classMethods~hasIndex(name) Then Signal 99.931
        lastClass~addClassMethod( name, attribute )
      End
      Else Do
        If lastClass~instanceMethods~hasIndex(name) Then Signal 99.931
        lastClass~addInstanceMethod( name, attribute )
      End
    End
  End
  directive~body = codeBody
  directive~attribute = attribute
  package~addDirective( directive )
Return

-- Duplicate ::ATTRIBUTE directive instruction.
99.931: Syntax( 99.931, TheElementAfter( directive~begin ) )

-- Incorrect external name specification "&1".
99.917: Syntax( 99.917, TheElementAfter( directive~begin ), options.external )

-- External attributes cannot have a method body.
99.935: Syntax( 99.935, instructionBegin )

-- Attribute methods without a SET or GET designation
-- cannot have a method body.
99.937: Syntax( 99.937, instructionBegin)

-- Abstract attributes cannot have a method body.
99.940: Syntax( 99.940, instructionBegin )

-- Delegate attributes cannot have a method body.
99.947: Syntax( 99.947, instructionBegin )

--------------------------------------------------------------------------------

::Class Attribute.Method

::Attribute name      Get
::Attribute body      Get
::Attribute directive Get

::Method init
  Expose         name  directive  body
  Use Strict Arg name, directive, body

/******************************************************************************/
/* CLASS                                                                      */
/******************************************************************************/

::Routine Class
  Use Strict Arg package, directive
  body = Empty.Body( package, directive )
  class = .Rexx.Class~new( directive~name, directive, body )
  package~currentClass = class
  name = Upper( directive~name~value )
  options. = directive~options.
  If package~hasClass( name ) Then Signal 99.901
  Select Case options.public
    When 1 Then package~addPublicClass( name, class )
    When 0 Then package~addClass(       name, class )
  End
  directive~body = body
  directive~theClass = class
  package~addDirective( directive )
  Return

-- Duplicate ::CLASS directive instruction.
99.901: Syntax( 99.901, TheElementAfter(directive~begin) )

--------------------------------------------------------------------------------

::Class Rexx.Class

::Attribute name      Get
::Attribute directive Get
::Attribute classMethods    Get -- Different from ooRexx Class class!
::Attribute instanceMethods Get -- Different from ooRexx Class class!

::Method init
  Expose         name  directive body classMethods instanceMethods
  Use Strict Arg name, directive, body
  classMethods    = .StringTable~new
  instanceMethods = .StringTable~new
::Method addClassMethod
  Expose classMethods
  Use Strict Arg name, method
  classMethods[name] = method
::Method addInstanceMethod
  Expose instanceMethods
  Use Strict Arg name, method
  instanceMethods[name] = method

/******************************************************************************/
/* CONSTANT                                                                   */
/******************************************************************************/

::Routine Constant
  Use Strict Arg package, directive
  codeBody = Code.Body( package, directive )
  If \codeBody~isEmpty Then Do
    instructionBegin = TheElementAfter( codeBody~instructions[1]~begin )
    Signal 99.938
  End
  constant = .Constant.Method~new(directive~name, directive, codeBody)
  name = Upper( directive~name~value )
  If package~inTheProlog Then Do
    If package~definedMethods~hasIndex(name) Then Signal 99.932
    package~addMethod( name, constant )
  End
  Else Do
    lastClass = package~currentClass
    If lastClass~classMethods~hasIndex(name) Then Signal 99.932
    lastClass~addClassMethod( name, constant )
    If lastClass~instanceMethods~hasIndex(name) Then Signal 99.932
    lastClass~addInstanceMethod( name, constant )
  End
  directive~body     = codeBody
  directive~constant = constant
  package~addDirective( directive )
  Return

-- Duplicate ::CONSTANT directive instruction.
99.932: Syntax( 99.932, TheElementAfter( directive~begin ) )

-- Constant methods cannot have a method body.
99.938: Syntax( 99.938, instructionBegin )

--------------------------------------------------------------------------------

::Class Constant.Method
::Method init
  Expose         name  directive  body
  Use Strict Arg name, directive, body
::Attribute name      Get
::Attribute directive Get
::Attribute body      Get

/******************************************************************************/
/* EXTENSION                                                                  */
/******************************************************************************/

::Routine Extension
  Use Strict Arg package, directive
  body = Empty.Body( package, directive )
  extension = .Rexx.Extension~new( directive~name, directive, body )
  package~currentClass = extension
  name = Upper( directive~name~value )
  options. = directive~options.
  directive~body      = body
  directive~extension = extension
  package~addDirective( directive )
  Return

--------------------------------------------------------------------------------

::Class Rexx.Extension

::Attribute name            Get
::Attribute directive       Get
::Attribute classMethods    Get -- Different from ooRexx Class class!
::Attribute instanceMethods Get -- Different from ooRexx Class class!
::Method init
  Expose         name  directive body classMethods instanceMethods

  Use Strict Arg name, directive, body
  classMethods    = .StringTable~new
  instanceMethods = .StringTable~new
::Method addClassMethod
  Expose classMethods
  Use Strict Arg name, method
  classMethods[name] = method
::Method addInstanceMethod
  Expose instanceMethods
  Use Strict Arg name, method
  instanceMethods[name] = method

/******************************************************************************/
/* METHOD                                                                     */
/******************************************************************************/

::Routine Method
  Use Strict Arg package, directive
  codeBody = Code.Body( package, directive )
  -- The EXTERNAL specification: before the body errors, but after them with
  -- ATTRIBUTE (ooRexx's order).
  external = directive~external
  If external \== .Nil, \directive~isAttribute Then Call CheckExternal
  If \codeBody~isEmpty Then Do
    firstInstruction = codeBody~instructions[1]
    instructionBegin = TheElementAfter( firstInstruction~begin )
    If directive~isAbstract                 Then Signal 99.933
    Else If directive~isAttribute           Then Signal 99.934
    Else If directive~external \== .Nil     Then Signal 99.936
    Else If directive~delegateName \== .Nil Then Signal 99.946
  End
  If external \== .Nil, directive~isAttribute Then Call CheckExternal
  name = Upper( directive~name~value )
  method = .Rexx.Method~new(name, directive, codeBody)
  If package~inTheProlog Then Do
    If package~definedMethods~hasIndex( name ) Then Signal 99.902
    package~addMethod( name, method )
  End
  Else Do
    lastClass = package~currentClass
    If directive~isClassMethod Then Do -- Class method
      If lastClass~classMethods~hasIndex(name) Then Signal 99.902
      lastClass~addClassMethod( name, method )
      If directive~isAttribute Then Do
        name = name"="
        If lastClass~classMethods~hasIndex( name ) Then Signal 99.902
        lastClass~addClassMethod( name, method )
      End
    End
    Else Do -- Instance method
      If lastClass~instanceMethods~hasIndex(name) Then Signal 99.902
      lastClass~addInstanceMethod( name, method )
      If directive~isAttribute Then Do
        name = name"="
        If lastClass~instanceMethods~hasIndex(name) Then Signal 99.902
        lastClass~addInstanceMethod( name, method )
      End
    End
  End
  directive~body = codeBody
  directive~method = method
  package~addDirective( directive )
Return

-- Duplicate ::METHOD directive instruction.
99.902: Syntax( 99.902, TheElementAfter( directive~begin ) )

CheckExternal:
  If ExternalSpecIsValid( external ) Then Return

-- Incorrect external name specification "&1".
99.917: Syntax( 99.917, TheElementAfter( directive~begin ), external )

-- Abstract methods cannot have a method body.
99.933: Syntax( 99.933, instructionBegin )

-- Attribute methods cannot have a method body.
99.934: Syntax( 99.934, instructionBegin )

-- External methods cannot have a method body.
99.936: Syntax( 99.936, instructionBegin )

-- Delegate methods cannot have a method body.
99.946: Syntax( 99.946, instructionBegin )

--------------------------------------------------------------------------------

::Class Rexx.Method
::Method init
  Expose         name  directive  body
  Use Strict Arg name, directive, body
::Attribute name      Get
::Attribute body      Get
::Attribute directive Get

/******************************************************************************/
/* OPTIONS                                                                    */
/******************************************************************************/

::Routine Options
  Use Strict Arg package, directive
  body = Empty.Body( package, directive )
  directive~body = body
  package~addDirective( directive )

/******************************************************************************/
/* REQUIRES                                                                   */
/******************************************************************************/

::Routine Requires
  Use Strict Arg package, directive
  body = Empty.Body( package, directive )
  directive~body = body
  package~addDirective( directive )

/******************************************************************************/
/* RESOURCE                                                                   */
/******************************************************************************/

::Routine Resource
  Use Strict Arg package, directive
  --package~scanner~resource.data( package, directive )
  body = Empty.Body( package, directive )
  directive~body = body
  package~addDirective( directive )

  name = Upper( directive~name~value )
  If package~resources~hasIndex(name) Then Signal 99.942
  package~addResource( name, directive )
  Exit

-- Duplicate ::RESOURCE directive instruction.
99.942: Syntax( 99.942, TheElementAfter( directive~begin ) )

/******************************************************************************/
/* ROUTINE                                                                    */
/******************************************************************************/

::Routine Routine
  Use Strict Arg package, directive

  name = directive~name
  codeBody = Code.Body( package, directive )
  If \codeBody~isEmpty Then Do
    instructionBegin = TheElementAfter( codeBody~instructions[1]~begin )
    If directive~external \== .Nil Then Signal 99.939
  End
  routine = .Rexx.Routine~new( name, codeBody, directive )
  name = Upper( name~value )
  If package~hasRoutine( name ) Then Signal 99.903
  If directive~private Then package~addRoutine(name, routine)
  Else                      package~addPublicRoutine(name, routine)

  directive~body    = codeBody
  directive~routine = routine

  package~addDirective( directive )

  Return

-- Duplicate ::ROUTINE directive instruction.
99.903: Syntax( 99.903, TheElementAfter( directive~begin ) )

-- External routines cannot have a code body.
99.939: Syntax( 99.939, instructionBegin )

--------------------------------------------------------------------------------

::Class Rexx.Routine
::Attribute begin     Get
::Attribute body      Get
::Attribute end       Get
::Attribute name      Get
::Attribute directive Get
::Constant tree "directive body"
::Method init
  Expose         name  body  directive begin end
  Use Strict Arg name, body, directive
  If directive \== .Nil
    Then begin = directive~begin
    Else begin = body~begin
  end = body~end

--------------------------------------------------------------------------------
-- Common routine: check that a directive body is empty (i.e., it consists    --
-- only of null clauses) and return that body. Otherwise, raise               --
-- a 99.916 error.                                                            --
--------------------------------------------------------------------------------

::Routine Empty.Body
  Use Strict Arg package, directive
  body = Code.Body( package, directive )
  If body~isEmpty Then Return body
  instruction = body~instructions[1]
  If \instruction~labels~isEmpty Then
    Call 99.916 instruction~labels[1]~label
  Call 99.916 TheElementAfter(instruction~begin)

-- Unrecognized directive instruction.
99.916: Syntax( 99.916, Arg(1) )

/******************************************************************************/
/* STACK (Utility class)                                                      */
/******************************************************************************/

::Class Stack SubClass Array Public
::Method top
  Return self~lastItem
::Method pop
  temp = self~top
  self~delete(self~last)
  return temp
::Method push
  self~append(Arg(1))