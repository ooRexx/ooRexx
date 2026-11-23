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
/* BaseClassesAndRoutines.cls                                                 */
/* ==========================                                                 */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241206    0.1  First public release                                      */
/* 20241226    0.1d Add GlobalOption routine                                  */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250421    0.2b Modify InternalError to raise a HALT condition            */
/* 20250722    0.2d Add Set.Directive.SubKeyword                              */
/* 20251026    0.3a Add support for Executor <number><extra> constructs       */
/* 20251201         (Executor) Add support for source literals                */
/* 20251202         Fix line no reported for syntax error 13.001              */
/* 20251226    0.4a Send error messages to .error, not .output                */
/* 20251226         Add support for "=" and "==" at end of expression         */
/* 20251227         Add ArgArray routine                                      */
/* 20260314         Add File2Array routine                                    */
/* 20260519    0.5  Add LoadProgramSource routine                             */
/* 20260924    0.7  Syntax: the error directory has the COLUMN too            */
/* 20260924         PrepareExpression: an unclosed "(" is diagnosed as ooRexx */
/*                  does, after what it holds (35.1, 35.901 or 36.901)        */
/* 20260928         PrepareExpression: a symbol after "~" or "~~" is not a    */
/*                  keyword (IF d~then THEN, DO i = 1 TO d~to)                */
/* 20260930         PrepareExpression: neither is a symbol after the ":" of a */
/*                  namespace (IF a ns: THEN is 18.1, as ooRexx)              */
/* 20260930         Syntax: the error directory has the CLAUSE too, the text  */
/*                  of the clause in error as ooRexx shows it after "*-*"     */
/*                  (issue #8: it was the whole line)                         */
/*                  SyntaxWithHint: the error directory has a HINT too, a     */
/*                  sentence that says what was probably meant (issue #39)    */
/*                  ClauseInError is public (the warnings use it)             */
/* 20260930         ClauseInError: the colons of the labels that the parser   */
/*                  has not marked yet (a 37.x of the scanner: "a: say )"     */
/*                  shows "say )", as ooRexx)                                 */
/* 20260930         Syntax: a clause that begins with an error of the scanner */
/*                  ('"01 "x' as the first thing in the source) raised the    */
/*                  error again and again (11.1)                              */
/* 20261025         A continuation made the blank operator keeps              */
/*                  ~continuation (Rony, 25-Oct)                              */
/* 20261115         Use CATCH instead of SIGNAL ON (RFE #31; needs ooRexx     */
/*                  r13514 or later)                                          */
/* 20261119         Syntax and SyntaxWithHint throw a Rexx.Parser.Error (new  */
/*                  class) as the USER condition REXX.PARSER.ERROR (THROW,    */
/*                  RFE #31), instead of raising SYNTAX 98.900 with a         */
/*                  Directory at the end of its ADDITIONAL. The prolog        */
/*                  refuses to run without ooRexx r13546 or later             */
/* 20261119         The prolog needs ooRexx r13550 or later: an uncaught      */
/*                  THROW is error 4.901, with its DESCRIPTION                */
/* 20261119         Rexx.Parser.Error~description has no final period, as     */
/*                  the substituted texts of ooRexx (error 4.901 showed two)  */
/* 20261121         The prolog needs ooRexx r13562 or later (the SourceForge  */
/*                  build that follows r13550)                                */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261122         Rexx.Instruction~terminating is 0 by default (init        */
/*                  assigned a local variable, and the attribute returned its */
/*                  own name)                                                 */
/* 20261123         No table constructors ("[...]", inspired by NetRexx and   */
/*                  Lua, with EXPERIMENTAL): an experiment that was not       */
/*                  followed                                                  */
/*                                                                            */
/******************************************************************************/

  -- The parser throws its errors (THROW, RFE #31), and so needs an ooRexx
  -- that has it: in an older one, a THROW would be a command. Since r13550,
  -- a THROW that nobody catches is error 4.901, which shows its DESCRIPTION.
  -- The minimum is r13562, the SourceForge build that follows it
  revision = .RexxInfo~revision
  If \DataType( revision, "W" ) | revision < 13562 Then
    Raise Syntax 98.900 Array( "The Rexx Parser needs ooRexx 5.3.0 r13562" -
      "or later (THROW); this is r"revision )

 .environment~Lambda.Do.Block  = .Lambda.Do.Block
 .environment~Rexx.Clause      = .Rexx.Clause
 .environment~Rexx.Directive   = .Rexx.Directive
 .environment~Rexx.Instruction = .Rexx.Instruction

/******************************************************************************/
/* A Rexx Clause                                                              */
/******************************************************************************/

::Class Rexx.Clause Public

::Attribute labels           -- Label clauses that identify this clause
::Attribute nullClauses      -- Null clauses preceding this clause
::Attribute docComment       -- Documentation comment preceding this clause
::Attribute begin            -- First element in clause (a semicolon)
::Attribute end              -- Last element in clause (a semicolon)
::Attribute package          -- The package we are currently parsing
::Method Init

  Expose package begin end labels nullClauses docComment

  Use Strict Arg package, begin, end

  labels      = Array()
  nullClauses = Array()
  docComment  = .Nil

  body = package~currentBodies~top

  -- body may be .Nil at the start of a body scan
  If \self~isA(.Block.Instruction) Then
    If body \== .Nil Then body~clauses~append( self )

::Method makeArray
  Use Strict Arg indent = 0
  Return Array(self~string(indent))

/******************************************************************************/
/* A Rexx Instruction                                                         */
/******************************************************************************/

::Class Rexx.Instruction Public SubClass Rexx.Clause
::Constant isBlockInstruction 0
::Attribute terminating -- True when all branches end with EXIT or RETURN
::Method init
  Expose terminating
  Use Strict Arg package, begin, end
  self~init:super( package, begin, end )
  terminating = 0

/******************************************************************************/
/* A Directive                                                                */
/******************************************************************************/

::Class Rexx.Directive Public SubClass Rexx.Clause
::Attribute body

/******************************************************************************/
/* ArgArray                                                                   */
/******************************************************************************/

-- Transform a command-like subroutine call arguments into a
-- .SysCArgs-compatible array.
--
-- Note that we don't accept escapes or escaped quotes.

::Routine ArgArray Public
  Parse Arg arg
  arg          = Strip(arg)
  array        = .Array~new

  thisArg      = ""
  insideQuotes = 0

  Loop i = 1 To Length(arg)
    c = arg[i]
    Select Case c
      When '"' Then Do
        If i > 1, arg[i-1] == '"' Then thisArg ||= '"'
        insideQuotes = \ insideQuotes
      End
      -- Process whitespace
      When " ", "09"X Then Do
        Select
          -- Inside quotes? This is part of the argument
          When insideQuotes Then thisArg ||= c
          -- Not inside quotes? Whitespace ends the argument
          When thisArg \== "" Then Call GotAnArgument
          -- Between arguments? Simply skip whitespace
          Otherwise Nop
        End
      End
      -- All other characters
      Otherwise Do
        thisArg   ||= c
      End
    End
  End

  -- We might need to add the last argument to the array
  If thisArg \== "" Then array~append(thisArg)

  Return array

GotAnArgument:
  array~append(thisArg)
  thisArg = ""
Return

/******************************************************************************/
/* Basic and common routines                                                  */
/******************************************************************************/

-- Create collections in a more intuitive way

::Routine Array       Public; Return Arg(1,"A")
::Routine Set         Public; Return .Set~sendWith("of",Arg(1,"A"))
::Routine StringTable Public
  If Arg() == 0 Then Return .StringTable~new
  Return .StringTable~sendWith("of",Arg(1,"A"))

/******************************************************************************/
/* Global.Option                                                              */
/******************************************************************************/

::Routine Global.Option Public
  Parse Upper Arg key
  If .Parser.Options~hasIndex( key ) Then Return .Parser.Options[ key ]
  Return ""

/******************************************************************************/
/* Generate syntax errors                                                     */
/*   When we detect a syntax error in the program being parsed, we build a   */
/*   Rexx.Parser.Error (see below) and throw it as the USER condition         */
/*   REXX.PARSER.ERROR (THROW, RFE #31), with the error object as its         */
/*   ADDITIONAL. A SYNTAX condition is always an error of the parser itself,  */
/*   or of its caller.                                                        */
/*                                                                            */
/*   Syntax( code, element, ... ): "element" places the error (see            */
/*   NewParseError); the rest are the substitutions of the message, or an     */
/*   array of them.                                                           */
/******************************************************************************/

::Routine Syntax Public
  Use Strict Arg code, element, ...
  NewParseError( "", Arg(1,"A") )~throw

/******************************************************************************/
/* SyntaxWithHint                                                             */
/*                                                                            */
/*   SyntaxWithHint( hint, code, element, ... ) is Syntax( code, element,     */
/*   ... ), and the error has a hint too: a sentence that says what the       */
/*   programmer probably meant, for the cases where the ooRexx message alone  */
/*   is not clear (issue #39). ooRexx has no hints; the error is the same.    */
/*                                                                            */
/******************************************************************************/

::Routine SyntaxWithHint Public
  Use Strict Arg hint, code, element, ...
  args = Arg(1,"A")
  args~delete(1)
  NewParseError( hint, args~makeArray )~throw

/******************************************************************************/
/* NewParseError( hint, args ) -- the Rexx.Parser.Error of a Syntax call      */
/*                                                                            */
/*   args are the arguments of Syntax: the code, the element, and the         */
/*   substitutions (or an array of them).                                     */
/******************************************************************************/

::Routine NewParseError
  Use Strict Arg hint, args

  code    = args[1]
  element = args[2]

  If args[3]~isA(.Array) Then additional = args[3]
  Else additional = args~section(3)

  Parse Var code major"."minor

  -- The element that the clause in error is looked for from (see below)
  anchor = .Nil
  If \element~isA(.String) Then anchor = element
  If additional \== .Nil, \additional~isEmpty, additional[1]~isA(.Element) Then
    anchor = additional[1]
  If element~isA(.Error.Element) Then anchor = element

  -- An error that the scanner has detected: its position is the error's
  -- (but see Scanner~fatal)
  If element~isA(.Error.Element), \element~atTheClause Then
    Parse Value element~from With line col

  -- In some few cases, Arg(2) contains "line col" as a string
  Else If element~isA(.String) Then Parse Var element line col

  -- If Arg(3) (or the first element in Arg(3) if Arg(3) is an array) is
  -- an element, we use its from attribute to place the message
  Else If additional \== .Nil, -
         \additional~isEmpty,  -
          additional[1]~isA(.Element) Then
    Parse Value additional[1]~from With line col

  -- In other cases, Arg(2) is an anchor, any element in the clause in error.
  -- We need to determine the location or the first element in that clause,
  -- to accurately place the error message.
  Else Do
    -- We first go back to the first end-of-clause before the given element,...
    Do While element \< .EL.END_OF_CLAUSE
      prev = TheElementBefore(element)
      If prev == .Nil Then Leave -- In some rare cases, prev may be .Nil
      element = prev
    End
    -- ...and then we move forward one element, so that we are now located
    -- at the beginning of the clause. (Not with TheElementAfter: when the
    -- clause begins with the error of the scanner, as a bad hexadecimal
    -- string that is the first thing in the source, it would raise the
    -- error again, and again. Its "from" is then the beginning.)
    next = element~next
    Loop While next \== .Nil, next~ignorable
      next = next~next
    End
    If next \== .Nil Then element = next

    Parse Value element~from With line col
  End

  minor = minor + 0
  code = major"."minor
  Do i = 1 To additional~items
    If additional[i]~isA(.Element) Then
      additional[i] = additional[i]~value
  End

  clause = ""
  If anchor \== .Nil Then Do
    clause = ClauseInError( anchor )
    If clause == .Nil Then clause = ""
  End
  Return .Rexx.Parser.Error~new( code, additional~makeArray, line, col, -
    clause, hint )

/******************************************************************************/
/* Rexx.Parser.Error -- a syntax error in the program being parsed            */
/*                                                                            */
/*   It is created by Syntax and SyntaxWithHint, and thrown (~throw) as the   */
/*   USER condition REXX.PARSER.ERROR, with itself as the ADDITIONAL of the   */
/*   condition and its description as the DESCRIPTION. On its way up, the     */
/*   parser (~withSource, ~in) and whoever knows more (the Highlighter, the   */
/*   fenced code blocks: ~in) complete it and throw it again.                 */
/*                                                                            */
/*   code           the error, "20.937"                                       */
/*   substitutions  the values for the message (an Array)                     */
/*   line, column   where it is                                               */
/*   clause         the clause in error, as ooRexx shows it after "*-*"       */
/*                  (issue #8), or "" when it is not known                    */
/*   sourceLine     the line in error (~withSource), or ""                    */
/*   hint           what was probably meant (issue #39), or ""                */
/*   name           the program, or ""                                        */
/*   stackFrames    the stack frames where the error was found (for the       */
/*                  internal traces of the utilities)                         */
/*                                                                            */
/******************************************************************************/

::Class Rexx.Parser.Error Public

::Attribute code          Get
::Attribute substitutions Get
::Attribute line          Get
::Attribute column        Get
::Attribute clause        Get
::Attribute sourceLine    Get
::Attribute hint          Get
::Attribute name          Get
::Attribute stackFrames   Get

::Method init
  Expose code substitutions line column clause hint name stackFrames -
    sourceLine
  Use Strict Arg code, substitutions, line, column, clause = "", hint = ""
  name = ""
  sourceLine = ""
  stackFrames = .context~stackFrames

::Method major
  Expose code
  Parse Var code major"."
  Return major

::Method minor
  Expose code
  Parse Var code "."minor
  Return minor

-- The secondary message, with its substitutions
::Method message
  Expose code substitutions
  Return ErrorText( code, substitutions )

-- The message of the major code
::Method majorMessage
  Return ErrorText( self~major )

-- Where the error is: "in x.rex, line 3", or "at line 3"
::Method where Private
  Expose line name
  If name == "" Then Return "at line" line
  Return "in" name", line" line

-- "Error 20.937 in x.rex, line 3: Symbol expected after THROW keyword"
-- The DESCRIPTION of the condition, and the string value of the error.
-- Without the final period of the message: ooRexx adds its own when it
-- shows a substituted text, as error 4.901 does with the DESCRIPTION of an
-- uncaught THROW
::Method description
  message = self~message
  If message~endsWith(".") Then message = Left( message, Length(message) - 1 )
  Return "Error" self~code self~where":" message

::Method makeString
  Return self~description

::Method string
  Return self~description

-- The report of the error as ooRexx shows it, one line per item: the
-- clause (or else the line), the major and the secondary messages, and the
-- hint, if any. The major message says "running NAME line N", as ooRexx
::Method report
  Expose line clause hint name sourceLine
  report = .Array~new
  If clause \== "" Then report~append( Right(line,6) "*-*" clause )
  Else report~append( Right(line,6) "*-*" sourceLine )
  If name == "" Then
    report~append( "Error" self~major "running INSTORE line" line": " -
      self~majorMessage )
  Else
    report~append( "Error" self~major "running" name "line" line": " -
      self~majorMessage )
  report~append( "Error" self~code": " self~message )
  If hint \== "" Then report~append( "Hint:" hint )
  Return report

-- The line in error, from the source (before ~in moves the line)
::Method withSource
  Expose line sourceLine
  Use Strict Arg source
  If sourceLine == "", source~hasIndex( line ) Then sourceLine = source[line]
  Return self

-- The error is in "name", "offset" lines below where its line says (a
-- fenced code block in a document, say)
::Method in
  Expose name line
  Use Strict Arg name, offset = 0
  line += offset
  Return self

-- Throw the error: USER REXX.PARSER.ERROR, with this object as ADDITIONAL
::Method throw
  Throw User Rexx.Parser.Error Description (self~description) -
    Additional (self)

/******************************************************************************/
/* ClauseInError                                                              */
/*                                                                            */
/*   The text of the clause that holds an element, as ooRexx shows it after   */
/*   "*-*" when it reports a syntax error (issue #8):                         */
/*                                                                            */
/*   - A clause ends with a ";" (which it includes) or with the end of the    */
/*     line (and then it includes all the line: trailing blanks, comments).   */
/*   - The end of clause that the parser inserts around THEN and ELSE does    */
/*     not end the clause ("if 1 then say (" is shown whole).                 */
/*   - A label ends a clause only before the element: "a: say (" shows        */
/*     "say (", but a 47.x on the label "a" shows "a: say (". The scanner's   */
/*     errors (37.x: "a: say )") come before the parser has marked the end    */
/*     of the labels: their colons are recognized here.                       */
/*   - Leading blanks and comments are not shown.                             */
/*   - A clause that spans several lines (continuations, multi-line comments  */
/*     or strings) is shown with its lines concatenated, with nothing in      */
/*     between.                                                               */
/*                                                                            */
/*   Returns .Nil when the end of the clause is not known (the scanner has    */
/*   not reached it): the caller shows the whole line then.                   */
/*                                                                            */
/******************************************************************************/

::Routine ClauseInError Public
  -- strict: any end of clause ends it, also those around THEN, ELSE and
  -- OTHERWISE (for the last instruction, see below)
  Use Strict Arg anchor, strict = 0

  -- An end of clause that ends the clause: a ";", or the end of a line
  -- (an inserted one, followed by nothing or by something on a later line).
  -- An inserted end of clause after the ":" of a label ends it too, when
  -- we go back.

  -- Back to the beginning of the clause. An anchor that ends a clause
  -- belongs to the clause that it ends.
  first = anchor
  If Ends( anchor ) Then first = anchor~prev
  If first == .Nil Then Return .Nil
  Loop
    prev = first~prev
    If prev == .Nil  Then Leave
    If Ends( prev )  Then Leave
    If AfterALabel( prev ) Then Leave
    If ALabelColon( prev ) Then Leave
    If strict, prev < .EL.END_OF_CLAUSE Then Leave
    first = prev
  End
  -- Leading blanks, comments and inserted elements are not shown
  Loop While first~isInserted | first~ignorable | first < .ALL.WHITESPACE_LIKE
    If first == anchor Then Leave
    first = first~next
    If first == .Nil Then Return .Nil
  End

  -- Forward to its end. The scanner stops at the errors that it detects,
  -- and then the rest of the clause is not in the chain.
  last     = anchor
  complete = 1
  Loop Until Ends( last )
    If last~next == .Nil Then Do
      complete = 0
      Leave
    End
    last = last~next
  End
  If \complete, \anchor~isA(.Error.Element) Then Return .Nil

  text   = ""
  lastTo = .Nil
  element = first
  Loop
    If \element~isInserted, \element~isA(.Error.Element) Then Do
      source = element~source
      -- A multi-line comment is an array of lines
      If source~isA(.Array) Then source = source~makeString("L", "")
      text ||= source
      lastTo = element~to
    End
    If element == last Then Leave
    element = element~next
  End

  -- Nothing but inserted elements (the end of the source, for a 14.x):
  -- ooRexx shows the last instruction before them ("Say 1", not
  -- "When 0 Then Say 1")
  If complete, text == "" Then Do
    prev = first~prev
    Loop While prev \== .Nil
      If \prev~isInserted, \prev~ignorable, prev \< .ALL.WHITESPACE_LIKE Then
        Return ClauseInError( prev, 1 )
      prev = prev~prev
    End
    Return .Nil
  End

  If complete Then Return text

  -- An error of the scanner: the rest of the clause is in the rest of
  -- the line, up to a ";" (not in a string or a comment)
  If lastTo == .Nil Then lastTo = anchor~from
  Parse Var lastTo line col
  source = anchor~package~source
  -- An invalid character (13.x), a bad hexadecimal or binary string
  -- (15.x): ooRexx shows the clause up to it
  Parse Value anchor~code With major "."
  If major == 13 | major == 15 Then Do
    Parse Value anchor~to With toLine toCol
    If toLine == line Then Return text || source[line][col, toCol - col]
  End
  Return text || RestOfTheClause( source, line, col )

Ends: Procedure
  Use Arg element
  If element \< .EL.END_OF_CLAUSE Then Return 0
  If \element~isInserted          Then Return 1 -- A ";"
  Parse Value element~from With line .
  next = element~next
  Loop While next \== .Nil
    If \next~isInserted Then Leave
    next = next~next
  End
  If next == .Nil Then Return 1
  Parse Value next~from With nextLine .
Return nextLine > line

-- The rest of a clause, from a position: up to a ";" that is not in a
-- string or a comment, or to the end of the line. An unclosed comment goes
-- on in the lines that follow (concatenated, as ooRexx shows them).
RestOfTheClause: Procedure
  Use Arg source, line, col
  rest  = ""
  text  = SubStr( source[line], col )
  depth = 0
  Loop
    len = Length( text )
    i = 1
    Loop While i <= len
      c = text[i]
      Select
        When depth > 0 Then Do
          If c == "/", text[i+1] == "*" Then Do
            depth += 1
            i += 1
          End
          Else If c == "*", text[i+1] == "/" Then Do
            depth -= 1
            i += 1
          End
        End
        When c == "'" | c == '"' Then Do
          j = Pos( c, text, i + 1 )
          If j == 0 Then Return rest || text
          i = j
        End
        When c == "/", text[i+1] == "*" Then Do
          depth = 1
          i += 1
        End
        When c == "-", text[i+1] == "-" Then Return rest || text
        When c == ";" Then Return rest || Left( text, i )
        Otherwise Nop
      End
      i += 1
    End
    rest ||= text
    If depth == 0 Then Return rest
    line += 1
    If line > source~items Then Return rest
    text = source[line]
  End

AfterALabel: Procedure
  Use Arg element
  If element \< .EL.END_OF_CLAUSE Then Return 0
  If \element~isInserted          Then Return 0
  prev = element~prev
  Loop While prev \== .Nil
    If \prev~ignorable, prev \< .ALL.WHITESPACE_LIKE Then Leave
    prev = prev~prev
  End
  If prev == .Nil Then Return 0
Return prev < .EL.COLON

-- The colon of a label that the parser has not marked yet (an error of the
-- scanner stops it before): a symbol or a string between the beginning of
-- the clause and the colon.
ALabelColon: Procedure
  Use Arg element
  If element \< .EL.COLON Then Return 0
  label = Before( element )
  If label == .Nil Then Return 0
  If label \< .ALL.SYMBOLS_AND_STRINGS Then Return 0
  prev = Before( label )
  If prev == .Nil Then Return 1
  If prev < .EL.END_OF_CLAUSE Then Return 1
Return ALabelColon( prev )

-- The element before one, skipping blanks and comments.
Before: Procedure
  Use Arg element
  prev = element~prev
  Loop While prev \== .Nil
    If \prev~ignorable, prev \< .ALL.WHITESPACE_LIKE Then Leave
    prev = prev~prev
  End
Return prev

/******************************************************************************/
/* Internal.Error                                                             */
/*                                                                            */
/* We print a panic error and raise the HALT condition.                       */
/*                                                                            */
/******************************************************************************/

::Routine Internal.Error Public
  Use Strict Arg text
 .Error~Say( Copies("-",80)    )
 .Error~Say( "Internal error:" )
 .Error~Say( "  "text          )
 .Error~Say( Copies("-",80)    )
  Raise HALT

/******************************************************************************/
/*   In higher levels of parsing, we will be only interested in non-ignorable */
/*   elements. In these cases, we will be interested in navigating from       */
/*   a certain, non-ignorable, element, to the next non-ignorable element.    */
/*                                                                            */
/*   TheElementAfter and TheElementBefore will take care of this              */
/*   non-ignorable navigation.                                                */
/******************************************************************************/

::Routine TheElementAfter Public
  Use Strict Arg element, ignoreBlanks = .False
  Loop Until \element~ignorable
    next = element~next
    If next~isNil Then
      Internal.Error( "TheElementAfter was about to return .Nil" )
    element = next
    If ignoreBlanks, element < .ALL.WHITESPACE_LIKE Then
      element~ignorable = 1
  End
  -- An error was detected by the scanner?
  If element~isA( .Error.Element ) Then Signal Error
  Return element

Error:
  code       = element~code
  additional = element~additional
  -- The error element itself: Syntax places the error at its "from", and
  -- looks for the clause in error from it
  Call Syntax code, element, additional      -- Does not return
--------------------------------------------------------------------------------

::Routine TheElementBefore Public
  Use Strict Arg element
  Loop Until \prev~ignorable
    prev = element~prev
    If prev~isNil Then
      Internal.Error( "TheElementBefore was about to return .Nil" )
    element = prev
  End
  Return element

/*   BEWARE: Our elements are not the same as the tokens defined by Rexx,     */
/*   since we include whitespace and comments as elements. We say that        */
/*   comments are ignorable elements (since they are always ignored), and     */
/*   most whitespace will also happen to be ignorable.                        */
/*                                                                            */
/*   Please note that the fact that a certain whitespace sequence is          */
/*   ignorable or not cannot be determined at the lowest levels of parsing.   */
/*   For example, a blank located between two simple variables acts,          */
/*   in principle, as a concatenation operator, but this ceases to be true    */
/*   when we find out that at least one of the simple variables happens to    */
/*   also be a keyword.                                                       */

--------------------------------------------------------------------------------
-- SetKeyword                                                                 --
--   Mark that this element is a keyword, and ignore whitespace               --
--   surrounding it.                                                          --
--------------------------------------------------------------------------------

::Routine SetKeyword Public
  Use Strict Arg keyword

  Call SetCategory keyword, .EL.KEYWORD

  before = TheElementBefore( keyword )
  If before < .ALL.WHITESPACE_LIKE Then before~ignorable = 1

  after  = TheElementAfter(  keyword )
  If after  < .ALL.WHITESPACE_LIKE Then after ~ignorable = 1

--------------------------------------------------------------------------------

::Routine SetSubkeyword Public
  Use Strict Arg keyword

  Call SetCategory keyword, .EL.SUBKEYWORD

  before = TheElementBefore( keyword )
  If before < .ALL.WHITESPACE_LIKE Then before~ignorable = 1

  after  = TheElementAfter(  keyword )
  If after  < .ALL.WHITESPACE_LIKE Then after ~ignorable = 1

--------------------------------------------------------------------------------

::Routine Set.Directive.Keyword Public
  Use Strict Arg keyword

  Call SetCategory keyword, .EL.DIRECTIVE_KEYWORD

  before = TheElementBefore( keyword )
  If before < .ALL.WHITESPACE_LIKE Then before~ignorable = 1

  after  = TheElementAfter(  keyword )
  If after  < .ALL.WHITESPACE_LIKE Then after~ignorable = 1

--------------------------------------------------------------------------------

::Routine Set.Directive.SubKeyword Public
  Use Strict Arg keyword

  Call SetCategory keyword, .EL.DIRECTIVE_SUBKEYWORD

  before = TheElementBefore( keyword )
  If before < .ALL.WHITESPACE_LIKE Then before~ignorable = 1

  after  = TheElementAfter(  keyword )
  If after  < .ALL.WHITESPACE_LIKE Then after~ignorable = 1

/******************************************************************************/
/* SetCategory -- A routine to set the category of an element                 */
/******************************************************************************/

::Routine SetCategory Public
  Use Strict Arg element, category

  element~category = category

/******************************************************************************/
/* SetConstantName                                                            */
/******************************************************************************/

::Routine SetConstantName Public
  Use Strict Arg element, subCategory

  element~   category = .EL.TAKEN_CONSTANT
  element~subCategory =  subCategory

/******************************************************************************/
/* PRINTLABELS                                                                */
/******************************************************************************/

::Routine PrintLabels Public
  labels = Arg(1)~labels
  If labels~isEmpty Then Return ""

  ret = ""

  Do i = 1 To labels~items
    label = labels[i]~label
    If label < .ALL.STRINGS Then
      print = label~source
    Else
      print = label~value
    ret ||= print": "
  End

  Return ret

--------------------------------------------------------------------------------
-- PREPAREEXPRESSION                                                          --
--   Performs a quick scan and sanitizing of an expression list.              --
--   It produces syntax errors for opening brackets preceded by whitespace.   --
--   It stops prescanning if one the provided keywords is found, and in this  --
--     case, it marks the keyword as such, which automatically marks          --
--     whitespace before and after the keyword, if any, as ignorable.         --
--   It inserts an implied semicolon before the "THEN" keyword.               --
--   It changes whitespace into blank concatenation operators when necessary  --
--   It inserts abuttal concatenation operators in the following contexts:    --
--     + a string or symbol immediately followed by another string or symbol; --
--     + ")" immediately followed by "(" or a string or a symbol;             --
--     + "]" immediately followed by "(" or a string or a symbol.             --
--   When Executor support is active, and in the special case of constructs   --
--     like "2i", a high-priority abuttal operator is inserted instead.       --
--   It scans for variable reference terms, in the form ">var" or "<var",     --
--     and substitutes the comparison operators for the corresponding         --
--     reference operators.                                                   --
--     Substitution is attempted in the following contexts:                   --
--     ">" or "<" at the beginning of an expression                           --
--     ">" or "<" following a (non-reference) operator                        --
--     ">" or "<" following a "(" or a "["                                    --
--     No check is done to verify that a simple or stem variable name appears --
--     to the right of the operator, since this can produce different errors  --
--     depending on the context (e.g., 20.930 or 20.931).                     --
--                                                                            --
--------------------------------------------------------------------------------

::Routine PrepareExpression Public

  Use Strict Arg package, begin, keywords = "", checkForAssignment = .False

  subExpression      = 0
  If keywords == ")" Then Do
    keywords         = ""
    subExpression    =  1
  End

  predecessor        = .Nil
  element            = begin
  nesting            = Array()

  Loop
    Select Case element~category
      -- Final versions of "=" and "==" in Executor are only generated
      -- just before an EOC.
      When .EL.END_OF_CLAUSE,              -
           .EL.EXECUTOR.FINAL.EQUAL,       -
           .EL.EXECUTOR.FINAL.DOUBLE.EQUAL Then Leave
      When .EL.SIMPLE_VARIABLE Then Do
        Select
          -- After "~" or "~~" a symbol is a message name, never a keyword
          -- ("If d~then Then", "Do i = 1 To d~to", as ooRexx).
          When nesting~isEmpty, IsAKeyword( element ), -
            \ IsAMessageName( element ) Then Do
            If .Options.Executor, predecessor \== .Nil, predecessor < .ALL.NUMBERS, -
              predecessor~to == element~from Then Do
              Call 35.001 "" -- Mimic Executor behaviour
            End
            Else Do
              -- If we generated a spurious blank operator, we fix it now
              Call FixSpuriousBlanks

              Call SetSubkeyword element
              If element~value == "THEN" Then Do
                Call InsertSemicolonBefore element
              End
              Leave
            End
          End
          Otherwise
            Call CheckAbuttal
        End
      End
      When .EL.OP.GREATER_THAN, .EL.OP.LOWER_THAN Then Do
        switch = 1
        Select
          When predecessor == .Nil               Then Nop
          When predecessor < .EL.COMMA           Then Nop
          When predecessor < .ALL.OPERATORS      Then Nop
          When predecessor < .ALL.LEFT_BRACES    Then Nop
          Otherwise switch = 0
        End
        If switch Then Call SwitchToReference
      End
      When .EL.LEFT_PARENTHESIS   Then Do
        nesting~append( element )
        Call CheckAbuttal
      End
      When .EL.LEFT_BRACKET Then Do
        If \predecessor~isNil, predecessor < .ALL.WHITESPACE_LIKE Then
          Call 35.001 "["
        nesting~append( element )
      End
      -- Jump over source literals
      When .EL.LEFT_CURLY_BRACKET Then Do
        Call CheckAbuttal
        predecessor = element
        element     = predecessor~closing
        Iterate
      End
      When .EL.RIGHT_CURLY_BRACKET Then Do
      End
      When .EL.RIGHT_PARENTHESIS Then Do
        nesting~delete( nesting~last )
        If subExpression, nesting~isEmpty Then Return
      End
      When .EL.RIGHT_BRACKET Then Do
        nesting~delete( nesting~last )
      End
      When .EL.WHITESPACE, .EL.CONTINUATION Then Do
        -- "a    b" concatenates with ONE blank: the first blank is the
        -- operator, and the rest stays whitespace (ignorable).
        If element < .EL.WHITESPACE, element~isA( .WhiteSpace.Element ) Then
          discard = element~splitAfterFirst
        -- A continuation that is the blank operator too: it is still shown
        -- as the continuation it is in the source (Element~continuation)
        If element < .EL.CONTINUATION Then element~continuation = 1
        Call SetCategory element, .EL.OP.BLANK
        element~value = " "
      End
      Otherwise
        Call CheckAbuttal
    End
    predecessor = element
    element    = TheElementAfter( element )
  End

  If nesting~isEmpty Then Return

  lastItem = nesting~lastItem
  Parse Value lastItem~from With line pos
  If lastItem < .EL.LEFT_PARENTHESIS Then Call UnclosedParenthesis
  -- An unclosed "[" that does not follow a term is 35.1 in ooRexx, whatever
  -- it holds.
  If lastItem < .EL.LEFT_BRACKET Then Do
    prev = lastItem~prev
    If prev == .Nil Then prev = lastItem
    If prev \< .ALL.SYMBOLS_AND_STRINGS, prev \< .ALL.RIGHT_BRACES Then Do
      element = lastItem
      Call 35.001 "["
    End
  End
  Select Case nesting~lastItem~value
    When "(" Then Signal 36.901
    When "[" Then Signal 36.902
  End

-- ooRexx parses what an unclosed "(" holds before it notices that the ")" is
-- missing, so that may be the error: nothing at all (35.1 at the "(", but
-- not after a function name or CALL, which expect arguments), a trailing
-- binary operator (35.1 at it), or a trailing "+" or "-" where a prefix
-- operator goes (35.901). A "-" or "," at the end of the line is a
-- continuation, and was made a blank above.
UnclosedParenthesis:
  binary = .ALL.OPS.ADDITIVE || .ALL.OPS.MULTIPLICATIVE || .EL.OP.POWER || -
    .EL.OP.CONCATENATION || .ALL.OPS.ALTERNATIVE || .EL.OP.AND || -
    .ALL.OPS.COMPARISON
  skip = .ALL.WHITESPACE_LIKE || .ALL.COMMENTS || .EL.OP.BLANK || -
    .EL.OP.ABUTTAL || .EL.CONTINUATION
  prev = lastItem~prev
  arguments = .False
  If prev \== .Nil Then arguments = prev < .ALL.SYMBOLS_AND_STRINGS
  If \ arguments Then Do                 -- CALL (expr) [args]
    prev = lastItem~prev
    Loop While prev \== .Nil, prev < skip
      prev = prev~prev
    End
    If prev \== .Nil, prev < ( .ALL.SYMBOLS || .EL.KEYWORD ), -
       Upper( prev~value ) == "CALL" Then Do
      prev = TheElementBefore( prev )
      arguments = prev < .EL.END_OF_CLAUSE
    End
  End
  significant = .Array~of( lastItem )
  element = lastItem~next
  Loop While element \< .EL.END_OF_CLAUSE
    -- (A compound operator is its first element; the others are ignorable.)
    If element \< skip, \ element~ignorable Then significant~append( element )
    element = element~next
  End
  -- A "," or "-" that ends the clause is a continuation to nothing.
  If significant~items > 1, significant~lastItem~value == "," | -
    significant~lastItem~value == "-" Then significant~delete( significant~last )
  last = significant~lastItem
  before = .Nil
  If significant~items > 1 Then before = significant[ significant~last - 1 ]
  If last == lastItem Then Do
    If arguments Then Return
    element = lastItem
    Call 35.001 lastItem~value
  End
  If last < ( .EL.OP.PLUS || .EL.OP.MINUS ), -
    ( before == lastItem | before < binary ) Then Do
    element = last
    Signal 35.901
  End
  If last < binary Then Do
    element = last
    Call 35.001 last~value
  End
  Return

FixSpuriousBlanks:
  If \predecessor~isNil, predecessor < .EL.OP.BLANK Then Do
    c = predecessor~source
    If c == "," | c == "-" Then
      Call SetCategory predecessor, .EL.CONTINUATION
    Else
      Call SetCategory predecessor, .EL.WHITESPACE
  End
  Return

SwitchToReference:
  Select Case element~category
    When .EL.OP.LOWER_THAN Then
      Call SetCategory element, .EL.OP.REFERENCE.LOWER_THAN
    When .EL.OP.GREATER_THAN Then
      Call SetCategory element, .EL.OP.REFERENCE.GREATER_THAN
  End
  next = TheElementAfter( element )
  -- Insert abuttals around symbols and strings
  If next < .ALL.SYMBOLS_AND_STRINGS Then Do
    next2next = TheElementAfter( next )
    If next2next < (.EL.LEFT_PARENTHESIS || .ALL.STRINGS) Then
      Call InsertAfter next, .EL.OP.ABUTTAL
    Else If next2next < .ALL.SYMBOLS Then Do
      If nesting == "", IsAKeyword(next2next) Then Return
      Call InsertAfter next, .EL.OP.ABUTTAL
    End
  End
  Return

-- A symbol that follows "~" or "~~" (maybe with blanks in between).
IsAMessageName:
  before = Arg(1)~prev
  Loop While before \== .Nil
    If \ before~ignorable, before \< .EL.OP.BLANK, -
      before \< .ALL.WHITESPACE_LIKE Then Leave
    before = before~prev
  End
  If before == .Nil Then Return .False
  -- "ns:name": the name of a namespace-qualified symbol (ooRexx takes the
  -- symbol after the ":" whatever it is).
  If before < .EL.COLON Then Return .True
  Return before < .ALL.OPS.MESSAGE_SEND & before \< .EL.LEFT_BRACKET

CheckAbuttal:
  If predecessor~isNil Then Return
  Select
    When element < .EL.LEFT_PARENTHESIS Then
      If predecessor < .ALL.RIGHT_BRACES Then Call InsertAbuttal
    When predecessor < .ALL.NUMBERS, element < .ALL.SYMBOLS, -
      predecessor~to == element~from
      Then Call InsertExecutorAbuttal
    When element < .ALL.SYMBOLS_AND_STRINGS Then Do
      Select
        When predecessor < .ALL.SYMBOLS_AND_STRINGS Then
          Call InsertAbuttal .True
        When predecessor < .ALL.RIGHT_BRACES        Then
          Call InsertAbuttal .True
        Otherwise Nop
      End
    End
    Otherwise Nop
  End
  Return

InsertAbuttal:
  Use Strict Arg checkKeyword = .False
  -- Do not insert an abuttal operator before a keyword
  If checkKeyword, nesting == "", IsAKeyword(element) Then Return
  Call InsertAfter predecessor, .EL.OP.ABUTTAL
  result~value = ""
Return

InsertExecutorAbuttal:
  -- No check for keywords, see
  -- https://github.com/JosepMariaBlasco/rexx-parser/discussions/22#discussioncomment-15085682
  Call InsertAfter predecessor, .EL.OP.EXECUTOR.ABUTTAL
  result~value = ""
Return

IsAKeyword:
  If Arg(1) \< .ALL.SYMBOLS Then Return 0
  Return WordPos( Arg(1)~value, keywords ) > 0

-- Incorrect expression detected at "&1".
35.001: Syntax( 35.001, element, Arg(1) )

-- Prefix operator "&1" is not followed by an expression term.
35.901: Syntax( 35.901, element, element~value )

-- Left parenthesis "(" in position &1 on line &2
-- requires a corresponding right parenthesis ")".
36.901: Syntax( 36.901, lastItem, pos, line )

-- Square bracket "[" in position &1 on line &2
-- requires a corresponding right square bracket "]".
36.902: Syntax( 36.902, lastItem, pos, line )

--------------------------------------------------------------------------------
-- Lambda.Do.Block                                                            --
--------------------------------------------------------------------------------

::Routine Lambda.Do.Block Public

  Use Arg package, DO

  Call SetCategory DO, .EL.KEYWORD

  element = TheElementAfter(DO)

  If element \< .EL.END_OF_CLAUSE Then Signal 21.900

  instructions = Array()

  ENDClauseIsOK = 1

  lastInstruction = .Nil

  package~blockInstructionsStacks~push(.Stack~new())
  package~blockInstructionsStacks~top~push(DO)

  Loop
    instruction = package~clauser~Next.Instruction( ENDClauseIsOK, , -
      .Array~of( 47.002 ) )
    If \instruction~labels~isEmpty Then Signal 47.002
    instrClass = instruction~class
    Select Case instrClass
      When .End.Clause                Then Signal End.Clause
      When .Expose.Instruction        Then Signal 99.907
      When .Implicit.Exit.Instruction Then Signal 14.001
      Otherwise Nop
    End
    instructions~append( instruction )
    lastInstruction = instruction
  End

End.Clause:
  END = TheElementAfter( instruction~begin )

  package~blockInstructionsStacks~pop

  Lambda.Do.Block = .Lambda.Do.Block~new( DO, END, instructions )

  DO~Lambda.Do.Block = Lambda.Do.Block

  Return Lambda.Do.Block


-- DO instruction on line &1 requires matching END.
14.001: Syntax( 14.001, instruction~end, doLine )


21.900: Syntax( 21.900, DO, "Data must not follow the DO keyword;",
   "found "element~source".")

-- Labels are not allowed within a DO/LOOP block; found "&1".
47.002: Syntax( 47.002, instruction~end, instruction~labels[1]~label )

-- EXPOSE must be the first instruction executed after a method invocation.
99.907: Syntax( 99.907, DO )

--------------------------------------------------------------------------------

::Class Lambda.Do.Block Public
::Attribute DO
::Attribute END
::Method begin; Return self~DO
::Attribute instructions
::Method Init
  Expose  DO  END  instructions
  Use Arg DO, END, instructions

--------------------------------------------------------------------------------
-- InsertSemicolonAfter                                                       --
--------------------------------------------------------------------------------

::Routine InsertSemicolonAfter Public

  Use Strict Arg element

  Parse Value element~to With line col

  semicolon      = .Inserted.Semicolon~new(line, col)

  -- Insert it between element and element~next

  next           = element~next

  semicolon~next = next
  semicolon~prev = element
  element~next     = semicolon
  next~prev      = semicolon

  -- Apply Rexx whitespace rules
  left           = TheElementBefore( semicolon )
  right          = TheElementAfter(  semicolon )
  If left  < .ALL.WHITESPACE_LIKE Then left ~ignorable = 1
  If right < .ALL.WHITESPACE_LIKE Then right~ignorable = 1

  Return semicolon

--------------------------------------------------------------------------------
-- InsertSemicolonBefore                                                      --
--------------------------------------------------------------------------------

::Routine InsertSemicolonBefore Public

  Use Strict Arg element

  Parse Value element~from With line col

  semicolon      = .Inserted.Semicolon~new(line, col)

  -- Insert it between element~prev and element

  left           = element~prev

  semicolon~next = element
  semicolon~prev = left
  left~next      = semicolon
  element~prev   = semicolon

  -- Apply Rexx whitespace rules
  left           = TheElementBefore( semicolon )
  right          = TheElementAfter(  semicolon )
  If left  < .ALL.WHITESPACE_LIKE Then left ~ignorable = 1
  If right < .ALL.WHITESPACE_LIKE Then right~ignorable = 1

  Return semicolon

--------------------------------------------------------------------------------
-- IsKeyword                                                                  --
--   If the element is not a symbol, or it is not the keyword we are looking  --
--   for, return 0. Else mark that this is a keyword, ignore whitespace to    --
--   the right, if any, and return 1.                                         --
--------------------------------------------------------------------------------

::Routine IsKeyword Public
  Use Strict Arg element, keyword

  -- Not a symbol? This is not a keyword
  If element \< .ALL.SYMBOLS_AND_KEYWORDS Then Return 0

  -- Not the keyword we are looking for
  If element~value \== keyword Then Return 0

  -- Mark that this is a keyword
  Call SetCategory element, .EL.KEYWORD

  -- And ignore whitespace to its right when necessary
  next = TheElementAfter( element )
  If next~category < .ALL.WHITESPACE_LIKE Then next~ignorable = 1

  Return 1

--------------------------------------------------------------------------------
-- Resource.Data -- For ::RESOURCE directives                                 --
--------------------------------------------------------------------------------

::Routine Resource.Data Public
  Use Strict Arg source, first, last

  resource_data          = .Resource.Data~new
  resource_data~category = .EL.RESOURCE_DATA
  resource_data~from     = first 1
  resource_data~to       = last (Length(source[last])+1)
  resource_data~source   = source
  Return                   resource_data

--------------------------------------------------------------------------------
-- InsertAfter                                                                --
--------------------------------------------------------------------------------

::Routine InsertAfter Public

  use Strict Arg element, class

  Parse Value element~to With line col

  inserted      = .Inserted.Element~new(class, line,col)

  -- Insert it between element and element~next

  next          = element~next

  inserted~next = next
  inserted~prev = element
  element~next  = inserted
  next~prev     = inserted

  Return          inserted

--------------------------------------------------------------------------------
-- File2Array -- Read a file into an array and close it                       --
--------------------------------------------------------------------------------

::Routine File2Array Public
  Use Strict Arg file
  array = CharIn(file, 1, Chars(file))~makeArray
  Call Stream file, "c", "close"
  Return array

--------------------------------------------------------------------------------
-- LoadProgramSource -- Read a program file into an array and close it        --
--------------------------------------------------------------------------------

::Routine LoadProgramSource Public
  Use Strict Arg file
  chunk = CharIn(file, 1, Chars(file))
  Call Stream file, "c", "close"
  p = pos("1A"X,chunk)
  If p == 0 Then array = chunk~makeArray
  Else           array = Left(chunk,p-1)~makeArray
  Return array