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
/* Directives.cls                                                             */
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
/* 20241209    0.1b Add .PACKAGE.FUNCTION.NAME, .PACKAGE.SUBROUTINE.NAME      */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20251027    0.2e Add support for ::OPTIONS NUMERIC [NO]INHERIT             */
/* 20251031         Add support for namespaces in ::CLASS directive           */
/* 20251101         Enhance and complete API for Requires.Directive           */
/* 20251128    0.3a (Executor) Implement ::EXTENSION directive                */
/* 20251201         (Executor) Add support for source literals                */
/* 20260308         Add GET to several attributes that are not settable       */
/* 20260928    0.7  ::ATTRIBUTE: ABSTRACT, DELEGATE and EXTERNAL exclude      */
/*                  each other (25.925, as ooRexx); the DELEGATE target is a  */
/*                  symbol (20.926); EXTERNAL with no string is 19.905;       */
/*                  options.external is .Nil when there is no EXTERNAL.       */
/*                  ::METHOD: the DELEGATE target is a symbol (20.926).       */
/*                  ::ROUTINE: EXTERNAL with no string is 19.905; the         */
/*                  specification is "LIBRARY|REGISTERED lib [entry]".        */
/*                  ExternalSpecIsValid: the forms of the EXTERNAL            */
/*                  specifications (99.917 when it is not one).               */
/*                  ::ANNOTATE: a signed value is a number (19.923, as        */
/*                  ooRexx); Annotate.Directive~options. (a getter).          */
/* 20260929         Options.Directive~settings: every setting of ::OPTIONS,   */
/*                  in order, as (NAME, value) pairs (the RVM applies them).  */
/*                  TRACE: the value in any case ('a', as ooRexx).            */
/* 20260930         ::ATTRIBUTE, ::CLASS, ::METHOD, ::ROUTINE: an option that */
/*                  is a string is an unknown keyword (25.9xx, as ooRexx).    */
/* 20260930         ::CONSTANT name (): no expression is 35.936 (was 35.1).   */
/* 20261110         A repeated keyword on ::ATTRIBUTE, ::CLASS, ::METHOD,     */
/*                  ::REQUIRES or ::ROUTINE is 25.936, and one that conflicts */
/*                  with another of its option is 25.937, as ooRexx since     */
/*                  r13404 (#2048; were 25.901-25.904 and 25.925, "unknown    */
/*                  keyword"). ::OPTIONS FORM with a bad keyword is 25.938,   */
/*                  as ooRexx since r13411 (#1966; was 25.11).                */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261122         Method.Directive~isConstant is 0: a ::METHOD is never a   */
/*                  constant (it was never assigned). Resource.Directive      */
/*                  ~fromLine and ~toLine are the first and last lines of the */
/*                  resource data (toLine < fromLine when it is empty; they   */
/*                  were not exposed, nor set). The three attributes returned */
/*                  their own names                                           */
/* 20261124         Constant.Directive~childAttributes: its value             */
/*                                                                            */
/******************************************************************************/

.environment ~   Annotate.Directive =   .Annotate.Directive
.environment ~  Attribute.Directive =  .Attribute.Directive
.environment ~      Class.Directive =      .Class.Directive
.environment ~ Coactivity.Directive = .Coactivity.Directive -- Executor
.environment ~   Constant.Directive =   .Constant.Directive
.environment ~  Extension.Directive =  .Extension.Directive
.environment ~     Method.Directive =     .Method.Directive
.environment ~    Options.Directive =    .Options.Directive
.environment ~   Requires.Directive =   .Requires.Directive
.environment ~   Resource.Directive =   .Resource.Directive
.environment ~          R.Directive =          .R.Directive -- Executor
.environment ~    Routine.Directive =    .Routine.Directive

::Requires "BaseClassesAndRoutines.cls"
::Requires "Expressions.cls"

/******************************************************************************/
/* ANNOTATE directive                                                         */
/******************************************************************************/

::Routine Annotate.Directive Public
  -- The target must exist (99.945): see the Annotate routine in
  -- parser.cls, which knows the directives seen so far.

  Use Strict Arg package, begin, end, elements

  ANNOTATE = elements[2]

  ignoreBlanks  = .True
  options.      = .Nil
  options.pairs = Array()

  keyword = TheElementAfter( ANNOTATE )

  If keyword \< .ALL.SYMBOLS Then Signal 20.924

  kValue = keyword~value

  validKeywords = "ATTRIBUTE CLASS CONSTANT METHOD PACKAGE ROUTINE"

  If WordPos(kValue, validKeywords) == 0 Then Signal 25.928

  Call Set.Directive.Keyword keyword

  options.directive = kValue

  If kValue \== "PACKAGE" Then Do
    name = TheElementAfter( keyword )
    If name \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.925
    Select Case kValue
      When "CLASS"   Then Call SetConstantName name, .CLASS.NAME
      When "ROUTINE" Then Call SetConstantName name, .ROUTINE.NAME
      Otherwise           Call SetConstantName name, .METHOD.NAME
    End
    options.["NAME"] = name
    name = TheElementAfter( name, ignoreBlanks )
  End
  Else name = TheElementAfter( keyword )

  Loop
    If name < .EL.END_OF_CLAUSE Then Signal Done
    If name \< .ALL.SYMBOLS Then Signal 20.919
    Call SetConstantName name, .ANNOTATION.NAME
    value = TheElementAfter( name, ignoreBlanks )
    If value < .EL.END_OF_CLAUSE Then Signal 19.924
    If value < .ALL.OPS.ADDITIVE Then Do
      -- A signed value: the sign and a number (ooRexx: a constant symbol,
      -- and then the whole must be a number).
      sign = value
      value = TheElementAfter( sign, ignoreBlanks )
      If value \< .ALL.NUMBERS, value \< .EL.SYMBOL_LITERAL Then
        Call 19.923 sign
      If \DataType( sign~value || value~value, "Number" ) Then
        Call 19.923 Upper( sign~value || value~value )
    End
    Else sign = .Nil
    If value \< .ALL.SYMBOLS_AND_STRINGS Then Call 19.923 value
    Call SetConstantName value, .ANNOTATION.VALUE
    options.pairs~append( (name, sign, value) )
    name = TheElementAfter( value, ignoreBlanks )
  End

Done:
  Return .Annotate.Directive~new( package, begin, end, options. )

-- Symbol or string expected as ::ANNOTATE attribute value; found "&1".
19.923: Syntax( 19.923, ANNOTATE, Arg(1) )

-- Symbol or string expected as ::ANNOTATE attribute value.
19.924: Syntax( 19.924, ANNOTATE )

-- Symbol or string expected after &1 &2 keyword.
19.925: Syntax( 19.925, ANNOTATE, "::ANNOTATE", keyword )

-- Symbol expected as ::ANNOTATE attribute name; found "&1".
20.919: Syntax( 20.919, ANNOTATE, name )

-- Symbol expected for the ::ANNOTATE type.
20.924: Syntax( 20.924, ANNOTATE, keyword )

-- Unknown keyword on ::ANNOTATE directive; found "&1".
25.928: Syntax( 25.928, ANNOTATE, keyword )

--------------------------------------------------------------------------------

::Class Annotate.Directive Public SubClass Rexx.Directive
::Attribute options. Get
::Method init
  Expose options.
  Use Strict Arg package, begin, end, options.
  self~init:super( package, begin, end )

/******************************************************************************/
/* ATTRIBUTE Directive                                                        */
/******************************************************************************/

::Routine Attribute.Directive Public
  -- TODO [LATER] compare keyword handling with DO/LOOP instructions

  Use Strict Arg package, begin, end, elements

  ATTRIBUTE = elements[2]

  name = TheElementAfter( ATTRIBUTE )

  If name \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.914

  If \ DataType(name~value, "Variable") Then Signal 99.925

  Call SetConstantName name, .METHOD.NAME

  options.get          = 1
  options.set          = 1
  options.class        = 0
  options.guarded      = 1
  options.protected    = 0
  options.abstract     = 0
  options.delegateName = ""
  options.external     = .Nil
  options.visibility   = "PUBLIC"

  ignoreBlanks = 1

  keyword = TheElementAfter( name, ignoreBlanks )

  allOptions = .set~of(                                   -
    "GET", "SET", "CLASS", "PUBLIC", "PACKAGE", "PRIVATE", -
    "GUARDED", "UNGUARDED", "UNPROTECTED", "PROTECTED",    -
    "ABSTRACT", "DELEGATE", "EXTERNAL"                     -
  )
  seen = .StringTable~new            -- Directive.Subkeyword (#2048)

  Loop While keyword \< .EL.END_OF_CLAUSE

    -- An option is a symbol (a string is not one, as ooRexx).
    If keyword \< .ALL.SYMBOLS Then Signal 25.925
    Call Directive.Subkeyword "ATTRIBUTE", keyword, seen,  -
      "GET SET", "CLASS", "PUBLIC PACKAGE PRIVATE",          -
      "GUARDED UNGUARDED", "PROTECTED UNPROTECTED",          -
      "ABSTRACT DELEGATE EXTERNAL"
    If \allOptions~hasItem( keyword~value ) Then Signal 25.925

    Call Set.Directive.SubKeyword keyword

    kValue = keyword~value

    allOptions~~remove(kValue)

    Select case kValue
      When "GET", "SET" Then Do
        If kValue == "GET" Then options.set = 0
        Else      /* "SET" */   options.get = 0
        allOptions~~remove("GET")~~remove("SET")
      End
      When "GUARDED", "UNGUARDED" Then Do
        If kValue == "UNGUARDED" Then options.guarded = 0
        allOptions~~remove("GUARDED")~~remove("UNGUARDED")
      End
      When "PROTECTED", "UNPROTECTED" Then Do
        If kValue == "PROTECTED" Then options.protected = 1
        allOptions~~remove("PROTECTED")~~remove("UNPROTECTED")
      End
      When "PUBLIC", "PACKAGE", "PRIVATE" Then Do
        options.visibility = kValue
        allOptions~~remove("PUBLIC")~~remove("PACKAGE")~~remove("PRIVATE")
      End
      When "CLASS"    Then options.kValue = 1  -- In the prolog: 99.905, below
      -- ABSTRACT, DELEGATE and EXTERNAL exclude each other (ooRexx).
      -- The EXTERNAL specification is checked when the body is known
      -- (a body is an error first, as in ooRexx: see parser.cls).
      When "ABSTRACT" Then Do
        options.abstract = 1
        allOptions~~remove("DELEGATE")~~remove("EXTERNAL")
      End
      When "DELEGATE" Then Do
        allOptions~~remove("ABSTRACT")~~remove("EXTERNAL")
        thisDelegateName = TheElementAfter( keyword )
        If thisDelegateName \< .ALL.SYMBOLS Then
          Call 20.926 thisDelegateName
        options.delegateName = thisDelegateName
        keyword = TheElementAfter( thisDelegateName, ignoreBlanks )
        Iterate -- Jump directly to next keyword, if any
      End
      When "EXTERNAL" Then Do
        allOptions~~remove("ABSTRACT")~~remove("DELEGATE")
        spec = TheElementAfter( keyword )
        If spec \< .ALL.STRINGS Then Signal 19.905
        options.external = spec~value
        keyword = TheElementAfter( spec, ignoreBlanks )
        Iterate -- Jump directly to next keyword, if any
      End
    End

    -- Step to next keyword, if any
    keyword = TheElementAfter( keyword, ignoreBlanks )

  End

  -- CLASS in the prolog is said once the options are read, as ooRexx (a
  -- repeated or conflicting keyword after it is 25.936 or 25.937; #2048)
  If options.class == 1, package~inTheProlog Then Signal 99.905

Done:
  Return .Attribute.Directive~new( package, begin, end, name, options. )

-- String expected after EXTERNAL keyword.
19.905: Syntax( 19.905, ATTRIBUTE, spec)

-- String or symbol expected as ::ATTRIBUTE directive name.
19.914: Syntax( 19.914, ATTRIBUTE, name)

-- Unknown keyword on ::ATTRIBUTE directive; found "&1".
25.925: Syntax( 25.925, ATTRIBUTE, keyword )

-- Symbol expected after the DELEGATE keyword.
20.926: Syntax( 20.926, ATTRIBUTE, Arg(1))

-- CLASS keyword on ::METHOD directive requires a matching ::CLASS directive.
99.905: Syntax( 99.905, ATTRIBUTE)

-- An ATTRIBUTE method name must be a valid variable name; found "&1".
99.925: Syntax( 99.925, ATTRIBUTE, name)

--------------------------------------------------------------------------------

::Class Attribute.Directive Public SubClass Rexx.Directive
::Attribute name     Get
::Attribute options. Get
::Attribute attribute
::Method init
  Expose name options.
  Use Strict Arg package, begin, end, name, options.
  self~init:super( package, begin, end )

/******************************************************************************/
/* CLASS Directive                                                            */
/******************************************************************************/

::Routine Class.Directive Public

  Use Strict Arg package, begin, end, elements

  CLASS = elements[2]

  name = TheElementAfter( CLASS )

  NotNamespaced = .False

  Call ExpectClassName name, 19.901, NotNamespaced

  options.             = .Nil
  options.public       = 0

  ignoreBlanks         = 1

  keyword = TheElementAfter( name, ignoreBlanks )

  allOptions = .set~of(                             -
    "METACLASS", "PRIVATE", "PUBLIC", "MIXINCLASS", -
    "SUBCLASS", "ABSTRACT", "INHERIT"               -
  )
  seen = .StringTable~new            -- Directive.Subkeyword (#2048)

  Loop While keyword \< .EL.END_OF_CLAUSE

    -- An option is a symbol (a string is not one, as ooRexx).
    If keyword \< .ALL.SYMBOLS Then Signal 25.901
    Call Directive.Subkeyword "CLASS", keyword, seen, -
      "METACLASS", "PUBLIC PRIVATE", "SUBCLASS MIXINCLASS", "ABSTRACT"
    If \allOptions~hasItem( keyword~value ) Then Signal 25.901

    Call Set.Directive.SubKeyword keyword

    kValue = keyword~value

    allOptions~remove(kValue)

    Select case kValue
      When "METACLASS" Then Do
        metaClass = TheElementAfter( keyword, ignoreBlanks )
        Call ExpectClassName metaClass, 19.906
        options.kValue = ( nameSpace, className )
        keyword = TheElementAfter( className, ignoreBlanks )
      End
      When "SUBCLASS", "MIXINCLASS" Then Do
        option = TheElementAfter( keyword, ignoreBlanks )
        If kValue == "SUBCLASS" Then Call ExpectClassName option, 19.907
        Else                         Call ExpectClassName option, 19.913
        options.kValue = ( nameSpace, className )
        allOptions~~remove("MIXINCLASS")~~remove("SUBCLASS")
        keyword = TheElementAfter( className, ignoreBlanks )
      End
      When "INHERIT" Then Do
        inherit = TheElementAfter( keyword, ignoreBlanks )
        Loop
          Call ExpectClassName inherit, 19.908
          If options.kValue == .Nil Then options.kValue = Array()
          options.kValue~append( ( nameSpace, className ) )
          inherit = TheElementAfter( className, ignoreBlanks )
          If inherit < .EL.END_OF_CLAUSE Then Signal Done
        End
      End
      When "ABSTRACT" Then Do
        options.kValue = 1
        keyword = TheElementAfter( keyword )
      End
      When "PRIVATE", "PUBLIC" Then Do
        options.public = ( kValue == "PUBLIC" )
        allOptions~~remove("PRIVATE")~~remove("PUBLIC")
        keyword = TheElementAfter( keyword )
      End
    End
  End
  Signal Done

Done:
  Exit .Class.Directive~new( package, begin, end, name, options. )

ExpectClassName:
  nameSpace = .Nil
  Use Arg element, code, nameSpaceAllowed = .True
  If nameSpaceAllowed, element < .ALL.SYMBOLS Then Do
    next = TheElementAfter( element )
    If next < .EL.COLON Then Do
      nameSpace = element
      Call SetConstantName element, .NAMESPACE.NAME
      element   = TheElementAfter( next )
      If element \< .ALL.SYMBOLS Then Signal 20.921
    End
  End

  If element \< .ALL.SYMBOLS_AND_STRINGS Then Signal (code)
  Call SetConstantName element, .CLASS.NAME
  className = element
Return

-- String or symbol expected after ::CLASS keyword.
19.901: Syntax( 19.901, CLASS )

-- String or symbol expected after METACLASS keyword.
19.906: Syntax( 19.906, CLASS, element )

-- String or symbol expected after SUBCLASS keyword.
19.907: Syntax( 19.907, CLASS, element )

-- String or symbol expected after INHERIT keyword.
19.908: Syntax( 19.908, CLASS, element )

-- String or symbol expected after MIXINCLASS keyword.
19.913: Syntax( 19.913, CLASS, element )

-- Symbol expected as a class name of qualified class name.
20.921: Syntax( 20.921, CLASS )

-- Unknown keyword on ::CLASS directive; found "&1".
25.901: Syntax( 25.901, CLASS, keyword )

--------------------------------------------------------------------------------

::Class Class.Directive Public SubClass Rexx.Directive
::Attribute name     Get
::Attribute options. Get
::Attribute theClass
::Method init
  Expose name options.
  Use Strict Arg package, begin, end, name, options.
  self~init:super( package, begin, end )

/******************************************************************************/
/* CO Directive                                                               */
/******************************************************************************/

::Routine Co.Directive Public

  Use Strict Arg package, begin, end, elements

  CO = elements[2]

  Return .COACTIVITY.Directive~new( package, begin, end, CO )

/******************************************************************************/
/* COACTIVITY Directive                                                       */
/******************************************************************************/

::Routine Coactivity.Directive Public

  Use Strict Arg package, begin, end, elements

  COACTIVITY = elements[2]

  Return .COACTIVITY.Directive~new( package, begin, end, COACTIVITY )

--------------------------------------------------------------------------------

::Class Coactivity.Directive Public SubClass Rexx.Directive
::Attribute directive Get
::Method init
  Expose         package  begin  end  directive
  Use Strict Arg package, begin, end, directive
  self~init:super( package, begin, end )

/******************************************************************************/
/* CONSTANT Directive                                                         */
/******************************************************************************/

::Routine Constant.Directive Public

  Use Strict Arg package, begin, end, elements

  CONSTANT = elements[2]

  name = TheElementAfter( CONSTANT )

  If name \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.915
  Call SetConstantName name, .METHOD.NAME

  ignoreBlanks = 1

  next = TheElementAfter( name, ignoreBlanks )

  If next < .EL.END_OF_CLAUSE Then Signal NoValue

  If next < .ALL.OPS.ADDITIVE Then Do
    sign = next
    number = TheElementAfter( next, ignoreBlanks )
    If number \< .ALL.NUMBERS Then Call 19.916 number
    Call SetConstantName number, .CONSTANT.VALUE
    If NoMoreDataAfter( number ) Then Signal SignedNumber
  End

  If next < .EL.LEFT_PARENTHESIS Then Do
    -- "()", "(": no expression, 35.936 (as ooRexx)
    first = TheElementAfter( next, ignoreBlanks )
    If first < .EL.END_OF_CLAUSE | first < .EL.RIGHT_PARENTHESIS Then
      Signal 35.936
    Call PrepareExpression package, next, ")"
    expression = SubExpression( package, next, "" )
    element = expression~end
    Call NoMoreDataAfter element
    If package~inTheProlog Then Signal 99.906
    Signal Expression
  End

  If next \< .ALL.SYMBOLS_AND_STRINGS Then Call 19.916 next
  Call SetConstantName next, .CONSTANT.VALUE
  value = next
  If NoMoreDataAfter( value ) Then Signal "VALUE"

NoMoreDataAfter:
  If Arg(1) < .EL.END_OF_CLAUSE Then Return 1
  next = TheElementAfter( Arg(1), ignoreBlanks )
  If next < .EL.END_OF_CLAUSE Then Return 1
  Signal 21.913

NoValue:
  Return .Constant.Directive~new( package, begin, end, name )

Value:
  Return .Constant.Directive~new( package, begin, end, name, value )

Expression:
  Return .Constant.Directive~new( package, begin, end, name, expression )

SignedNumber:
  Return .Constant.Directive~new( package, begin, end, name, (sign, number) )

-- String or symbol expected as ::CONSTANT directive name.
19.915: Syntax( 19.915, CONSTANT, name )

-- Missing expression on ::CONSTANT directive.
35.936: Syntax( 35.936, CONSTANT )

-- String or symbol expected as ::CONSTANT value.
19.916: Syntax( 19.916, CONSTANT, Arg(1) )

-- Data must not follow the ::CONSTANT value; found "&1".
21.913: Syntax( 21.913, CONSTANT, next)

-- A ::CONSTANT directive with an expression requires
-- a matching ::CLASS directive.
99.906: Syntax( 99.906, CONSTANT, name )

--------------------------------------------------------------------------------

::Class Constant.Directive Public SubClass Rexx.Directive
::Constant childAttributes "BODY VALUE"
::Attribute name    Get
::Attribute value   Get
::Attribute constant
::Method init
  Expose name value expression sign number
  Use Strict Arg        -
    package, begin, end, name, value = .Nil
  self~init:super( package, begin, end )

/******************************************************************************/
/* EXTENSION Directive                                                        */
/******************************************************************************/

::Routine Extension.Directive Public

  Use Strict Arg package, begin, end, elements

  EXTENSION = elements[2]

  name = TheElementAfter( EXTENSION )

  NotNamespaced = .False

  Call ExpectClassName name, 19.900, NotNamespaced

  options.             = .Nil

  ignoreBlanks         = 1

  keyword = TheElementAfter( name, ignoreBlanks )

  inherit = .Nil

  If keyword \< .EL.END_OF_CLAUSE Then Do

    If keyword~value \== "INHERIT" Then Signal 25.900

    Call Set.Directive.SubKeyword keyword

    next = TheElementAfter( keyword, ignoreBlanks )
    Loop Until next < .EL.END_OF_CLAUSE
      Call ExpectClassName next, 19.908
      If inherit == .Nil Then inherit = Array()
      inherit~append( ( nameSpace, className ) )
      next = TheElementAfter( className, ignoreBlanks )
    End

  End

  Exit .Extension.Directive~new( package, begin, end, name, options. )

ExpectClassName:
  nameSpace = .Nil
  Use Arg element, code, nameSpaceAllowed = .True
  If nameSpaceAllowed, element < .ALL.SYMBOLS Then Do
    next = TheElementAfter( element )
    If next < .EL.COLON Then Do
      nameSpace = element
      Call SetConstantName element, .NAMESPACE.NAME
      element   = TheElementAfter( next )
      If element \< .ALL.SYMBOLS Then Signal 20.921
    End
  End

  If element \< .ALL.SYMBOLS_AND_STRINGS Then Signal (code)
  Call SetConstantName element, .CLASS.NAME
  className = element
Return

-- String or symbol expected after ::EXTENSION keyword.
19.900: Syntax( 19.900, EXTENSION, "String or symbol expected after ::EXTENSION keyword" )

-- String or symbol expected after INHERIT keyword.
19.908: Syntax( 19.908, EXTENSION, element )

-- Symbol expected as a class name of qualified class name.
20.921: Syntax( 20.921, CLASS )

-- Unknown keyword on ::INHERIT directive; found "&1".
25.900: Syntax( 25.900, CLASS, 'Unknown keyword on ::INHERIT directive; found "'keyword'"' )

--------------------------------------------------------------------------------

::Class Extension.Directive Public SubClass Rexx.Directive
::Attribute name     Get
::Attribute options. Get
::Attribute extension
::Method init
  Expose name options.
  Use Strict Arg package, begin, end, name, options.
  self~init:super( package, begin, end )


/******************************************************************************/
/* METHOD Directive                                                           */
/******************************************************************************/

::Routine Method.Directive Public

  Use Strict Arg package, begin, end, elements

  METHOD = elements[2]

  name = TheElementAfter( METHOD )

  If name \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.902

  Call SetConstantName name, .METHOD.NAME

  options.              = .Nil
  options.isAbstract    = 0
  options.isAttribute   = 0
  options.isClassMethod = 0
  options.isGuarded     = 1
  options.isPackage     = 0
  options.isPrivate     = 0
  options.isProtected   = 0

  ignoreBlanksToTheRight = 1

  keyword = TheElementAfter( name, ignoreBlanksToTheRight )

  allOptions = .set~of(                                   -
    "ATTRIBUTE", "CLASS", "PUBLIC", "PACKAGE", "PRIVATE", -
    "GUARDED", "UNGUARDED", "UNPROTECTED", "PROTECTED",   -
    "ABSTRACT", "DELEGATE", "EXTERNAL"                    -
  )

  If .Options.Experimental Then Do
    allOptions[] = "EXTENDS"
    allOptions[] = "OVERRIDES"
  End

  options.containingClass = package~currentClass
  seen = .StringTable~new            -- Directive.Subkeyword (#2048)

  Loop While keyword \< .EL.END_OF_CLAUSE

    -- An option is a symbol (a string is not one, as ooRexx).
    If keyword \< .ALL.SYMBOLS Then Signal 25.902
    Call Directive.Subkeyword "METHOD", keyword, seen,     -
      "CLASS", "ABSTRACT DELEGATE EXTERNAL", "PUBLIC PACKAGE PRIVATE", -
      "PROTECTED UNPROTECTED", "GUARDED UNGUARDED", "ATTRIBUTE"
    If \allOptions~hasItem( keyword~value ) Then Signal 25.902

    Call Set.Directive.SubKeyword keyword

    kValue = keyword~value

    allOptions~remove(kValue)

    Select case kValue
      When "ATTRIBUTE" Then options.isAttribute = 1
      When "GUARDED", "UNGUARDED" Then Do
        If kValue == "UNGUARDED" Then options.isGuarded = 0
        allOptions~~remove("GUARDED")~~remove("UNGUARDED")
      End
      When "PROTECTED", "UNPROTECTED" Then Do
        If kValue == "PROTECTED" Then options.isProtected = 1
        allOptions~~remove("PROTECTED")~~remove("UNPROTECTED")
      End
      When "PUBLIC", "PACKAGE", "PRIVATE" Then Do
        If      kValue == "PACKAGE" Then options.isPackage = 1
        Else If kValue == "PRIVATE" Then options.isPrivate = 1
        allOptions~~remove("PUBLIC")~~remove("PACKAGE")~~remove("PRIVATE")
      End
      When "CLASS" Then options.isClassMethod = 1 -- In the prolog: 99.905, below
      When "EXTENDS", "OVERRIDES" Then Do -- Experimental
        className = TheElementAfter( keyword )
        Call ExpectClassName className, 20.900
        options.kValue = ( nameSpace, className )
        ignoreBlanksToTheRight = 1
        keyword = TheElementAfter( className, ignoreBlanksToTheRight )
        Iterate -- Jump directly to next keyword, if any
      End
      Otherwise -- "ABSTRACT", "DELEGATE", "EXTERNAL"
        allOptions~~remove("ABSTRACT")~~remove("DELEGATE")~~remove("EXTERNAL")
        Select Case kValue
          When "ABSTRACT" Then Do
            options.isAbstract = 1
            keyword = TheElementAfter( keyword )
          End
          When "DELEGATE" Then Do
            thisDelegateName = TheElementAfter( keyword )
            ignoreBlanksToTheRight = 1
            If thisDelegateName \< .ALL.SYMBOLS Then
              Call 20.926 thisDelegateName
            options.delegateName = thisDelegateName
            keyword = TheElementAfter( thisDelegateName, ignoreBlanksToTheRight )
          End
          When "EXTERNAL" Then Do -- The spec is checked with the body
            spec = TheElementAfter( keyword )
            ignoreBlanksToTheRight = 1
            If spec \< .ALL.STRINGS Then Signal 19.905
            options.external = spec~value
            keyword = TheElementAfter( spec, ignoreBlanksToTheRight )
          End
        End
        Iterate -- Jump directly to next keyword, if any
    End
    -- Step to next keyword, if any
    keyword = TheElementAfter( keyword )
  End

  -- CLASS in the prolog is said once the options are read, as ooRexx (#2048)
  If options.isClassMethod == 1, package~inTheProlog Then Signal 99.905

Done:
  Return .Method.Directive~new( package, begin, end, name, options. )

ExpectClassName:
  nameSpace = .Nil
  Use Arg element, code
  If element < .ALL.SYMBOLS Then Do
    next = TheElementAfter( element )
    If next < .EL.COLON Then Do
      nameSpace = element
      Call SetConstantName element, .NAMESPACE.NAME
      element   = TheElementAfter( next )
      If element \< .ALL.SYMBOLS Then Signal 20.921
    End
  End

  If element \< .ALL.SYMBOLS_AND_STRINGS Then Signal (code)
  Call SetConstantName element, .CLASS.NAME
  className = element
Return

-- String or symbol expected after ::METHOD keyword.
19.902: Syntax( 19.902, METHOD, name )

-- String expected after EXTERNAL keyword.
19.905: Syntax( 19.905, METHOD, spec )

-- Unknown keyword on ::METHOD directive; found "&1".
25.902: Syntax( 25.902, METHOD, keyword )

-- Symbol expected after the XXX keyword.
20.900: Syntax( 20.900, METHOD, "Symbol expected after the" kValue "keyword." )

-- Symbol expected as a class name of qualified class name.
20.921: Syntax( 20.921, METHOD )

-- Symbol expected after the DELEGATE keyword.
20.926: Syntax( 20.926, METHOD, Arg(1) )

-- CLASS keyword on ::METHOD directive requires a matching ::CLASS directive.
99.905: Syntax( 99.905, METHOD )

--------------------------------------------------------------------------------

::Class Method.Directive Public SubClass Rexx.Directive
::Attribute name          Get
::Attribute isAbstract    Get
::Attribute isAttribute   Get
::Attribute isConstant    Get
::Attribute isClassMethod Get
::Attribute isGuarded     Get
::Attribute isPackage     Get
::Attribute isPrivate     Get
::Attribute isProtected   Get
::Attribute delegateName  Get
::Attribute external      Get
::Attribute extends       Get -- Experimental
::Attribute overrides     Get -- Experimental

-- A .Rexx.Method, set when parsing the directive
::Attribute method

::Method init
  Expose name isAbstract isAttribute isClassMethod isGuarded -
    isPackage isPrivate isProtected delegateName external extends overrides -
    isConstant
  Use Strict Arg package, begin, end, name, options.
  isConstant    = 0 -- A ::METHOD is never a constant (see ::CONSTANT)
  isAttribute   = options.isAttribute
  isAbstract    = options.isAbstract
  isClassMethod = options.isClassMethod
  isGuarded     = options.isGuarded
  isPackage     = options.ispackage
  isPrivate     = options.isPrivate
  isProtected   = options.isProtected

  delegateName  = options.delegatename
  external      = options.external
  extends       = options.extends
  overrides     = options.overrides

  self~init:super( package, begin, end )

/******************************************************************************/
/* OPTIONS Directive                                                          */
/******************************************************************************/
-- We keep only the most current values

::Routine Options.Directive Public

  Use Strict Arg package, begin, end, elements

  OPTIONS = elements[2]

  element            = TheElementAfter( OPTIONS )
  options.           = .Nil
  options.commands   = 1
  options.macrospace = 1
  ignoreBlanks       = .True
  -- The settings, in the order they are given (a later one overrides an
  -- earlier one: ALL SYNTAX ERROR CONDITION), as (NAME, value) pairs.
  settings           = .Array~new
  options.["SETTINGS"] = settings

  If element < .EL.END_OF_CLAUSE Then Signal Done

  conditions = "ALL ERROR FAILURE LOSTDIGITS NOSTRING NOTREADY NOVALUE"
  numeric    = "DIGITS FORM FUZZ NUMERIC"
  allOptions = numeric conditions "PROLOG NOPROLOG TRACE"

  If .Options.Executor Then
    allOptions ||= " COMMANDS NOCOMMANDS MACROSPACE NOMACROSPACE"

  Loop While element \< .EL.END_OF_CLAUSE
    If element \< .ALL.SYMBOLS          Then Signal 25.924
    tValue = element~value
    If WordPos(tValue, allOptions) == 0 Then Signal 25.924
    Call Set.Directive.SubKeyword element
    theCondition = element
    element = TheElementAfter( element )
    Select
      When WordPos(tValue, conditions) > 0 Then Do
        condition = tValue
        If element \< .ALL.SYMBOLS Then Signal 20.929
        If tValue = "NOVALUE" Then Do
          -- ::OPTIONS NOVALUE ERROR is deprecated (rexxref B.2.3) but it should work.
          If WordPos( element~value, "CONDITION SYNTAX ERROR") == 0 Then Signal 25.927
        End
        Else Do
          If WordPos( element~value, "CONDITION SYNTAX") == 0 Then Signal 25.927
        End
        options.condition = element~value
        settings~append( .Array~of( condition, element~value ) )
        Call Set.Directive.SubKeyword element
        element = TheElementAfter( element )
      End
      When tValue == "COMMANDS"     Then options.commands   = 1
      When tValue == "NOCOMMANDS"   Then options.commands   = 0
      When tValue == "MACROSPACE"   Then options.macrospace = 1
      When tValue == "NOMACROSPACE" Then options.macrospace = 0
      When tValue == "PROLOG"       Then Do
        options.prolog = 1
        settings~append( .Array~of( "PROLOG", 1 ) )
      End
      When tValue == "NOPROLOG"     Then Do
        options.prolog = 0
        settings~append( .Array~of( "PROLOG", 0 ) )
      End
      When tValue == "FORM"         Then Do
        If element \< .ALL.SYMBOLS Then Signal 20.925
        If WordPos( element~value, "SCIENTIFIC ENGINEERING") == 0 Then Signal 25.938
        options.form = element~value
        settings~append( .Array~of( "FORM", element~value ) )
        Call Set.Directive.SubKeyword element
        element = TheElementAfter( element )
      End
      When tValue == "TRACE" Then Do
        If element \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.919
        tValue = Upper( element~value )
        c = Strip(tValue, "L", "?")[1]
        If c \== "", Pos(c, "ACEFILNOR") == 0 Then Signal 24.001
        If c == "" Then Options.trace = tValue
        Else Do
          Parse Var tValue ? (c)
          If Length( ? ) // 2 Then c = "?"c
          options.trace = c
        End
        settings~append( .Array~of( "TRACE", options.trace ) )
        element = TheElementAfter( element, ignoreBlanks )
      End
      When tValue == "NUMERIC" Then Do
        If element \< .ALL.SYMBOLS Then Signal 20.935
        If WordPos( element~value, "INHERIT NOINHERIT") == 0 Then Signal 25.935
        options.["NUMERIC"] = element~value
        settings~append( .Array~of( "NUMERIC", element~value ) )
        Call Set.Directive.SubKeyword element
        element = TheElementAfter( element )
      End
      Otherwise Do -- DIGITS, FUZZ
        If element \< .ALL.SYMBOLS_AND_STRINGS Then
          If tValue == "FUZZ" Then Signal 19.918; Else Signal 19.917
        value = element~value
        If \DataType(value, "W")             | -
          ( tValue == "FUZZ" & value < 0   ) | -
          ( tValue == "DIGITS" & value < 1 ) Then
          If tValue == "DIGITS" Then Signal 26.005; Else Signal 26.006
        options.tValue = value
        settings~append( .Array~of( tValue, value ) )
        element = TheElementAfter( element, ignoreBlanks )
      End
    End
  End

Done:
  Return .Options.Directive~new( package, begin, end, options. )

-- String or symbol expected as DIGITS value.
19.917: Syntax( 19.917, OPTIONS, element )

-- String or symbol expected as FUZZ value.
19.918: Syntax( 19.918, OPTIONS, element )

-- String or symbol expected as TRACE value.
19.919: Syntax( 19.919, OPTIONS, element )

-- Symbol expected after the FORM keyword.
20.925: Syntax( 20.925, OPTIONS, element )

-- Symbol expected after &1 keyword.
20.929: Syntax( 20.929, OPTIONS, theCondition )

-- Symbol expected after the NUMERIC subdirective keyword.
20.935: Syntax( 20.935, OPTIONS )

-- TRACE request letter must be one of "ACEFILNOR"; found "&1".
24.001: Syntax( 24.001, OPTIONS, c )

-- FORM must be followed by one of the keywords
-- SCIENTIFIC or ENGINEERING; found "&1".
25.938: Syntax( 25.938, OPTIONS, element )

-- Unknown keyword on ::OPTIONS directive; found "&1".
25.924: Syntax( 25.924, OPTIONS, element )

-- Unknown keyword following "&1"; found "&2".
25.927: Syntax( 25.927, OPTIONS, condition, element )

-- Subdirective NUMERIC must be followed by one of the keywords
-- INHERIT or NOINHERIT; found "&1".
25.935: Syntax( 25.935, OPTIONS, element )

-- DIGITS value must be a positive whole number; found "&1".
26.005: Syntax( 26.005, OPTIONS, element )

-- FUZZ value must be zero or a positive whole number; found "&1".
26.006: Syntax( 26.006, OPTIONS, element )

--------------------------------------------------------------------------------

::Class Options.Directive Public SubClass Rexx.Directive
::Attribute Commands   -- Executor only
::Attribute Macrospace -- Executor only
::Attribute Prolog
-- The settings, in the order given: an Array of (NAME, value) pairs. NAME
-- is DIGITS, FUZZ, FORM, TRACE, NUMERIC (INHERIT or NOINHERIT), PROLOG (1 or
-- 0), or a condition (ALL, ERROR, FAILURE, LOSTDIGITS, NOSTRING, NOTREADY,
-- NOVALUE: SYNTAX or CONDITION; NOVALUE ERROR too, the old form of SYNTAX).
::Attribute Settings
::Method init
  Expose Commands Macrospace
  Use Strict Arg package, begin, end, options.

  self~commands   = options.commands
  self~macrospace = options.macrospace
  self~prolog     = options.prolog
  self~settings   = options.settings

  self~init:super( package, begin, end )

/******************************************************************************/
/* REQUIRES Directive                                                         */
/******************************************************************************/

::Routine Requires.Directive Public

  Use Strict Arg package, begin, end, elements

  REQUIRES = elements[2]

  programName  = TheElementAfter( REQUIRES )

  options.     = .Nil
  ignoreBlanks = .True

  If programName \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.904
  Call SetConstantName programName, .REQUIRES.PROGRAM.NAME

  keyword = TheElementAfter( programName, ignoreBlanks )
  If keyword < .EL.END_OF_CLAUSE Then Signal Done

  If keyword \< .ALL.SYMBOLS Then Call 25.904 keyword

  kValue = keyword~value

  If WordPos(kvalue, "LIBRARY NAMESPACE") == 0 Then Call 25.904 keyword

  Call Set.Directive.SubKeyword keyword

  -- One of LIBRARY and NAMESPACE, once: a second is 25.936 or 25.937 (#2048)
  seen = .StringTable~new
  Call Directive.Subkeyword "REQUIRES", keyword, seen, "LIBRARY NAMESPACE"

  If kValue == "LIBRARY" Then Do
    options.library = 1
    element = TheElementAfter( keyword )
    If element < .ALL.SYMBOLS Then
      Call Directive.Subkeyword "REQUIRES", element, seen, "LIBRARY NAMESPACE"
    If element \< .EL.END_OF_CLAUSE Then Call 25.904 element
    Signal Done
  End

  -- kValue == "NAMESPACE"
  element = TheElementAfter( keyword )

  If element \< .ALL.SYMBOLS Then Signal 20.920
  If element~value == "REXX"  Then Signal 99.944
  options.nameSpace = element

  Call SetConstantName element, .NAMESPACE.NAME

  element = TheElementAfter( element, ignoreBlanks )

  If element < .ALL.SYMBOLS Then
    Call Directive.Subkeyword "REQUIRES", element, seen, "LIBRARY NAMESPACE"
  If element \< .EL.END_OF_CLAUSE Then Call 25.904 element

Done:
  Return .Requires.Directive~new( package, begin, end, programName, options. )

-- String or symbol expected after ::REQUIRES keyword.
19.904: Syntax( 19.904, REQUIRES, programName )

-- Symbol expected after NAMESPACE keyword.
20.920: Syntax( 20.920, REQUIRES )

-- Unknown keyword on ::REQUIRES directive; found "&1".
25.904: Syntax( 25.904, REQUIRES, Arg(1) )

-- The REXX name is reserved for the language-provided namespace.
99.944: Syntax( 99.944, REQUIRES )

--------------------------------------------------------------------------------

::Class Requires.Directive Public SubClass Rexx.Directive
::Attribute programName Get
::Attribute nameSpace   Get
::Attribute library     Get
::Method init
  Expose programName nameSpace library
  Use Strict Arg package, begin, end, programName, options.
  nameSpace = options.namespace
  library   = options.library
  self~init:super( package, begin, end )

/******************************************************************************/
/* RESOURCE Directive                                                         */
/*                                                                            */
/*   ::RESOURCE directives are special, in the sense that they break          */
/*   normal parsing rules. An added complication stems from the fact that     */
/*   there is an ambiguity in the syntax regarding possible extra characters  */
/*   after an explicit semicolon after ::RESOURCE. See documentation bug      */
/*   307 for details.                                                         */
/*                                                                            */
/*   The syntax we will be using is the following:                            */
/*                                                                            */
/*      ::RESOURCE name [END delimiter] [; [ignored_data_1] ]                 */
/*      [resource_data]*                                                      */
/*      (::END | delimiter) [ignored_data_2]                                  */
/*                                                                            */
/******************************************************************************/

::Routine Resource.Directive Public

  Use Strict Arg package, begin, end, elements

  RESOURCE     = elements[2]
  ignoreBlanks = .True

  name = TheElementAfter( RESOURCE )
  If name \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.920
  Call SetConstantName name, .RESOURCE.NAME

  end = TheElementAfter( name, ignoreBlanks )

  If end < .EL.END_OF_CLAUSE Then delimiter = "::END"
  Else Do
    keyword = end

    If keyword \< .ALL.SYMBOLS Then Signal 25.926
    If keyword~value \== "END" Then Signal 25.926
    Call Set.Directive.SubKeyword keyword

    delimiterElement = TheElementAfter( keyword )
    If delimiterElement \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.921
    Call SetConstantName delimiterElement, .RESOURCE.DELIMITER.NAME

    delimiter = delimiterElement~value

    end = TheElementAfter( delimiterElement, ignoreBlanks )
    If end \< .EL.END_OF_CLAUSE Then Signal 21.914

  End

  closing = elements[1]~closing
  If closing \== .Nil Then end = closing

  Return .Resource.Directive~new( package, begin, end, name, delimiter )

-- String or symbol expected as ::RESOURCE directive name.
19.920: Syntax( 19.920, RESOURCE, name )

-- String or symbol expected after ::RESOURCE END keyword.
19.921: Syntax( 19.921, RESOURCE, delimiterElement )

-- Data must not follow the ::RESOURCE directive; found "&1".
21.914: Syntax( 21.914, delimiterElement, end )

-- Unknown keyword on ::RESOURCE directive; found "&1".
25.926: Syntax( 25.926, RESOURCE, keyword )

--------------------------------------------------------------------------------

::Class Resource.Directive Public SubClass Rexx.Directive
::Attribute name      Get
::Attribute delimiter Get
::Attribute fromLine  Get
::Attribute toLine    Get
::Method init
  Expose name delimiter fromLine toLine
  Use Strict Arg package, begin, end, name, delimiter
  -- The scanner has already put the resource data element in the
  -- directive: its first and last lines are the lines of the resource.
  fromLine = ""
  toLine   = ""
  element  = begin
  Do While element \== .Nil, element \== end
    If element < .EL.RESOURCE_DATA Then Do
      Parse Value element~from With fromLine .
      Parse Value element~to   With toLine   .
      Leave
    End
    element = element~next
  End
  self~init:super( package, begin, end )

/******************************************************************************/
/* R Directive                                                                */
/******************************************************************************/

::Routine R.Directive Public

  Use Strict Arg package, begin, end, elements

  R = elements[2]

  Return .R.Directive~new( package, begin, end, R )

--------------------------------------------------------------------------------

::Class R.Directive Public SubClass Rexx.Directive
::Attribute directive Get
::Method init
  Expose         package  begin  end  directive
  Use Strict Arg package, begin, end, directive
  self~init:super( package, begin, end )

/******************************************************************************/
/* ROUTINE Directive                                                          */
/******************************************************************************/

::Routine Routine.Directive Public

  Use Strict Arg package, begin, end, elements

  ROUTINE = elements[2]

  If \package~sourceLiteralStack~isEmpty Then Do
    semicolon = InsertSemicolonAfter( ROUTINE )
    Return .R.Directive~new( package, begin, semicolon, ROUTINE )
  End

  ignoreBlanks = .True

  routineName =  TheElementAfter( ROUTINE )

  If routineName \< .ALL.SYMBOLS_AND_STRINGS Then Signal 19.903

  Call SetConstantName routineName, .ROUTINE.NAME

  allOptions = .Set~of("PUBLIC", "PRIVATE", "EXTERNAL")
  possibleKeywords = "public private external"

  private  = 1
  external = 0
  spec     = .Nil

  keyword = TheElementAfter( routineName, ignoreBlanks )
  seen    = .StringTable~new         -- Directive.Subkeyword (#2048)

  Loop While keyword \< .EL.END_OF_CLAUSE

    -- An option is a symbol (a string is not one, as ooRexx).
    If keyword \< .ALL.SYMBOLS Then Signal 25.903
    Call Directive.Subkeyword "ROUTINE", keyword, seen, "PUBLIC PRIVATE", "EXTERNAL"
    If \allOptions~hasItem( keyword~value ) Then Signal 25.903

    Call Set.Directive.SubKeyword keyword

    Select Case keyword~value
      When "PUBLIC", "PRIVATE" Then Do
        allOptions~~remove("PUBLIC")~~remove("PRIVATE")
        If keyword~value == "PUBLIC" Then private = 0
        keyword = TheElementAfter( keyword )
      End
      When "EXTERNAL" Then Do
        external = 1
        allOptions~~remove("EXTERNAL")
        spec = TheElementAfter( keyword )
        If spec \< .ALL.STRINGS Then Signal 19.905
        keyword = TheElementAfter( spec, ignoreBlanks )
     End
    End
  End

  -- The specification is checked once the options are read, as ooRexx (a
  -- second EXTERNAL after a bad one is 25.936; #2048)
  If spec \== .Nil, \ExternalSpecIsValid( spec~value, "LIBRARY REGISTERED" ) Then
    Signal 99.917

  Return .Routine.Directive~new(-
    package, begin, end, routineName, private, spec -
  )

-- String or symbol expected after ::ROUTINE keyword.
19.903: Syntax( 19.903, ROUTINE, routineName )

-- String expected after EXTERNAL keyword.
19.905: Syntax( 19.905, ROUTINE, spec )

-- Unknown keyword on ::ROUTINE directive; found "&1".
25.903: Syntax( 25.903, ROUTINE, keyword )

-- Incorrect external name specification "&1".
99.917: Syntax( 99.917, ROUTINE, spec )

--------------------------------------------------------------------------------

-- The forms of an EXTERNAL specification, as ooRexx: "LIBRARY lib [entry]"
-- for methods and attributes (decodeExternalMethod), and also
-- "REGISTERED lib [entry]" for routines. The first word is caseless.

::Routine ExternalSpecIsValid Public
  Use Strict Arg spec, forms = "LIBRARY"
  Parse Upper Var spec type rest
  If WordPos( type, forms ) == 0 Then Return .False
  n = Words( rest )
  Return n == 1 | n == 2

--------------------------------------------------------------------------------

::Class Routine.Directive Public SubClass Rexx.Directive
::Attribute body
::Attribute routine
::Method init
  Expose name  private  spec
  Use Strict Arg package, begin, end, name, private, spec = .Nil
  body = .Nil
  self~init:super( package, begin, end )
::Attribute name     Get
::Attribute private  Get
::Method external
  Expose spec
  Return spec

/******************************************************************************/
/* Directive.Subkeyword - a keyword of a directive that is one of its        */
/* options, against the keywords already given (ooRexx's checkSubkeyword,    */
/* #2048, r13404). Each group (a string of words) is one option: a second     */
/* keyword of the same group is a duplicate (25.936) if it is the same word, */
/* and a conflict (25.937) if it is another one. `seen` (a StringTable)       */
/* keeps, for each group, the keyword that gave it. A word in no group is    */
/* not an option, and is left for the caller (an unknown keyword).            */
/******************************************************************************/

::Routine Directive.Subkeyword
  Use Strict Arg directive, keyword, seen, group1, ...

  kValue = keyword~value
  Do i = 4 To Arg()
    If WordPos(kValue, Arg(i)) == 0 Then Iterate
    If \seen~hasIndex(i) Then Do
      seen[i] = keyword
      Return
    End
    previous = seen[i]
    If previous~value == kValue Then Signal 25.936
    Signal 25.937
  End
  Return

-- Duplicate keyword on ::&1 directive; found "&2".
25.936: Syntax( 25.936, keyword, directive, keyword )

-- Keyword "&1" on ::&2 directive conflicts with "&3".
25.937: Syntax( 25.937, keyword, keyword, directive, previous )
