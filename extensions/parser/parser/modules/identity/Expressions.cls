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
/* Expressions.cls -- Clone expressions (part of the identity compiler)       */
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
/* 20251116    0.3a First version                                             */
/* 20251209         (Executor) Implement named arguments                      */
/* 20251212         (Executor) Implement message terms                        */
/* 20261123         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261123         Moved from bin/modules/identity/ to                       */
/*                  bin/parser/modules/identity/ (RFE #878: everything but    */
/*                  parser.cls lives in parser/); its requires and the call   */
/*                  to the loader, relative to this file                      */
/*                                                                            */
/******************************************************************************/

  Call "../Load.Parser.Module.rex"

  pkgLocal = .context~package~local

  -- Set to 1 to activate debug
  pkgLocal~DEBUG = 0

::Requires "compile.cls"

/******************************************************************************/
/* ADDITION EXPRESSIONS                                                       */
/******************************************************************************/

::Method "Addition.Expression::Compile"
  Use Strict Arg element, stream, context

  args  = self~args
  nArgs = args~items

  Loop i = 1 By 2 To nArgs
    -- Compile args[i]
    element = args[i]~compile( element, stream, context )
    -- Now if i < nArgs, args[i+1] is an addition operator
    If i < nArgs Then Do
      -- Clone everything until the start of the next argument
      begin = args[i+2]~begin
      Loop While element \== begin
        element = Clone( element, stream, context )
      End
    End
  End

  Return element

/******************************************************************************/
/* AND EXPRESSIONS                                                            */
/******************************************************************************/

::Method "And.Expression::Compile"
  Use Strict Arg element, stream, context

  args  = self~args
  nArgs = args~items

  Loop i = 1 By 2 To nArgs
    -- Compile args[i]
    element = args[i]~compile( element, stream, context )
    -- Now if i < nArgs, args[i+1] is an and operator
    If i < nArgs Then Do
      -- Clone everything until the start of the next argument
      begin = args[i+2]~begin
      Loop While element \== begin
        element = Clone( element, stream, context )
      End
    End
  End

  Return element

/******************************************************************************/
/* ARGUMENTS                                                                  */
/******************************************************************************/

::Method "Arguments::Compile"
  Use Strict Arg element, stream, context

  namedArguments = self~namedArguments

  If self~isEmpty, namedArguments~isEmpty == .Nil Then Return element

  first = 1
  Loop arg Over self~args
    first = 0
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    If arg~isA( .Special.Character.Sequence ) Then
      element = Clone( element, stream, context )
    Else element = arg~compile( element, stream, context )
  End

  If \namedArguments~isEmpty Then Do
    If \first Then -- Clone a comma
      element = Clone( element, stream, context )
    nNamed = namedArguments~items
    Loop Counter C named Over namedArguments

      Loop While element~ignorable
        element = Clone( element, stream, context )
      End

      -- name
      name       = named[1]
      If name \== .Nil Then Do
        element = name~compile( element, stream, context )
        Loop While element~ignorable
          element = Clone( element, stream, context )
        End
      End

      -- ":"
      element = Clone( element, stream, context )

      Loop While element~ignorable
        element = Clone( element, stream, context )
      End

      expression = named[2]
      If expression \== .Nil Then Do
        element = expression~compile( element, stream, context )
        Loop While element~ignorable
          element = Clone( element, stream, context )
        End
      End

      If c < nNamed Then Do -- Clone ","
        element = Clone( element, stream, context )
        Loop While element~ignorable
          element = Clone( element, stream, context )
        End
      End
    End
  End

  Return element

/******************************************************************************/
/* ARRAY TERMS                                                                */
/******************************************************************************/

::Method "Array.Term::Compile"
  Use Strict Arg element, stream, context

  Return self~expression_list~compile( element, stream, context )

/******************************************************************************/
/* COMPOUND VARIABLE TERMS                                                    */
/******************************************************************************/

::Method "Compound.Variable.Term::Compile"
  Use Strict Arg element, stream, context

  element = Clone( element, stream, context )

  Return element

/******************************************************************************/
/* COMPARISON EXPRESSIONS                                                     */
/******************************************************************************/

::Method "Comparison.Expression::Compile"
  Use Strict Arg element, stream, context

  args  = self~args
  nArgs = args~items

  Loop i = 1 By 2 To nArgs
    -- Compile args[i]
    element = args[i]~compile( element, stream, context )
    -- Now if i < nArgs, args[i+1] is a comparison operator
    If i < nArgs Then Do
      -- Clone everything until the start of the next argument
      begin = args[i+2]~begin
      Loop While element \== begin
        element = Clone( element, stream, context )
      End
    End
  End

  Return element

/******************************************************************************/
/* CONCATENATION EXPRESSIONS                                                  */
/******************************************************************************/

::Method "Concatenation.Expression::Compile"
  Use Strict Arg element, stream, context

  args  = self~args
  nArgs = args~items

  Loop i = 1 By 2 To nArgs
    -- Compile args[i]
    element = args[i]~compile( element, stream, context )
    -- Now if i < nArgs, args[i+1] is a concatenation operator
    If i < nArgs Then Do
      -- Clone everything until the start of the next argument
      begin = args[i+2]~begin
      Loop While element \== begin
        element = Clone( element, stream, context )
      End
    End
  End

  Return element

/******************************************************************************/
/* EXPRESSION LISTS                                                           */
/******************************************************************************/

::Method "Expression.List::Compile"
  Use Strict Arg element, stream, context

  If self~isEmpty Then Return element

  Loop arg Over self~args
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    If arg~isA( .Special.Character.Sequence ) Then
      element = Clone( element, stream, context )
    Else element = arg~compile( element, stream, context )
  End

  Return element

/******************************************************************************/
/* FUNCTION CALL TERMS                                                        */
/******************************************************************************/

::Method "Function.Call.Term::Compile"

  Use Strict Arg element, stream, context

  name          = self~name
  argumentList  = self~argumentList
  trailingBlock = self~trailingBlock

  -- Name
  element = name~compile( element, stream, context )

  If argumentList \== .Nil Then Do
    -- "("
    element = Clone( element, stream, context )
    -- Ignorable stuff
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    -- Argument list
    element = argumentList~compile( element, stream, context )
    -- More ignorable stuff
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    -- ")"
    element = Clone( element, stream, context )
  End

  If trailingBlock \== .Nil Then Do
    element = trailingBlock~compile( element, stream, context )
  End

  Return element

/******************************************************************************/
/* INDEXED OR INVOKED TERMS                                                   */
/******************************************************************************/

::Method "Indexed.Or.Invoked.Term::Compile"
  Use Strict Arg element, stream, context

  term      = self~term
  bracket   = self~bracket
  arguments = self~arguments

  -- term
  element = term~compile( element, stream, context )
  -- Left bracket
  element = Clone( element, stream, context )
  -- Arguments
  element = arguments~compile( element, stream, context )
  -- Right bracket
  element = Clone( element, stream, context )

  Return element

/******************************************************************************/
/* LITERAL STRING TERMS                                                       */
/******************************************************************************/

::Method "Literal.String.Term::Compile"
  Use Strict Arg element, stream, context

  element = Clone( element, stream, context )

  Return element

/******************************************************************************/
/* MESSAGE TERMS                                                              */
/******************************************************************************/

::Method "Message.Term::Compile"
  Use Strict Arg element, stream, context

  term          = self~term
  messageName   = self~messageName
  scope         = self~scope
  arguments     = self~arguments
  trailingBlock = self~trailingBlock

  element = term~compile( element, stream, context )

  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- ~ or ~~
  element = Clone( element, stream, context )

  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  If messageName \== .Nil Then Do
    Loop While element \== messageName
      element = Clone( element, stream, context )
    End

    element = messageName~compile( element, stream, context )
  End

  If scope \== .Nil Then Do
    Loop While element \< .EL.COLON
      element = Clone( element, stream, context )
    End
    element = Clone( element, stream, context )
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    element = scope~compile( element, stream, context )
  End

  If element < .EL.LEFT_PARENTHESIS Then Do
    element = Clone( element, stream, context )
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    element = arguments~compile( element, stream, context )
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    element = Clone( element, stream, context )
  End

  If element < .EL.LEFT_CURLY_BRACKET Then Do
    element = trailingBlock~compile( element, stream, context )
  End

  Return element

/******************************************************************************/
/* MULTIPLICATION EXPRESSIONS                                                 */
/******************************************************************************/

::Method "Multiplication.Expression::Compile"
  Use Strict Arg element, stream, context

  args  = self~args
  nArgs = args~items

  Loop i = 1 By 2 To nArgs
    -- Compile args[i]
    element = args[i]~compile( element, stream, context )
    -- Now if i < nArgs, args[i+1] is a multiplication operator
    If i < nArgs Then Do
      -- Clone everything until the start of the next argument
      begin = args[i+2]~begin
      Loop While element \== begin
        element = Clone( element, stream, context )
      End
    End
  End

  Return element

/******************************************************************************/
/* NAMESPACE QUALIFIED CLASSNAME                                              */
/******************************************************************************/

::Method "Namespace.Qualified.Classname::Compile"
  Use Strict Arg element, stream, context

  classname     = self~classname
  nameSpace     = self~nameSpace

  -- Namespace
  element = nameSpace~compile( element, stream, context )

  -- Ignorable stuff
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- ":"
  element = Clone( element, stream, context )

  -- Ignorable stuff
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Class name
  element = classname~compile( element, stream, context )

  Return element

/******************************************************************************/
/* NAMESPACE QUALIFIED FUNCTION CALLS                                         */
/******************************************************************************/

::Method "Namespace.Qualified.Function.Call::Compile"
  Use Strict Arg element, stream, context

  name          = self~name
  nameSpace     = self~nameSpace
  arguments     = self~arguments
  trailingBlock = self~trailingBlock

  -- Namespace
  element = nameSpace~compile( element, stream, context )

  -- Ignorable stuff
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- ":"
  element = Clone( element, stream, context )

  -- Ignorable stuff
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Name
  element = name~compile( element, stream, context )

  If arguments \== .Nil Then Do
    -- "("
    element = Clone( element, stream, context )
    -- Ignorable stuff
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    -- Argument list
    element = arguments~compile( element, stream, context )
    -- More ignorable stuff
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    -- ")"
    element = Clone( element, stream, context )
  End

  If trailingBlock \== .Nil Then Do
    element = trailingBlock~compile( element, stream, context )
  End

  Return element

/******************************************************************************/
/* OR EXPRESSIONS                                                             */
/******************************************************************************/

::Method "Or.Expression::Compile"
  Use Strict Arg element, stream, context

  args  = self~args
  nArgs = args~items

  Loop i = 1 By 2 To nArgs
    -- Compile args[i]
    element = args[i]~compile( element, stream, context )
    -- Now if i < nArgs, args[i+1] is a or operator
    If i < nArgs Then Do
      -- Clone everything until the start of the next argument
      begin = args[i+2]~begin
      Loop While element \== begin
        element = Clone( element, stream, context )
      End
    End
  End

  Return element

/******************************************************************************/
/* POWER EXPRESSIONS                                                          */
/******************************************************************************/

::Method "Power.Expression::Compile"
  Use Strict Arg element, stream, context

  args  = self~args
  nArgs = args~items

  Loop i = 1 By 2 To nArgs
    -- Compile args[i]
    element = args[i]~compile( element, stream, context )
    -- Now if i < nArgs, args[i+1] is a power operator
    If i < nArgs Then Do
      -- Clone everything until the start of the next argument
      begin = args[i+2]~begin
      Loop While element \== begin
        element = Clone( element, stream, context )
      End
    End
  End

  Return element

/******************************************************************************/
/* PREFIX EXPRESSIONS                                                         */
/******************************************************************************/

::Method "Prefix.Expression::Compile"
  Use Strict Arg element, stream, context

  term  = self~term
  begin = term~begin

  -- Skip over prefixes
  Loop While element \== begin
    element = Clone( element, stream, context )
  End

  element = term~compile( element, stream, context )

  Return element

/******************************************************************************/
/* SOURCE LITERALS                                                            */
/******************************************************************************/

::Method "Source.Literal::Compile"

  Use Strict Arg element, stream, context

  begin        = self~begin
  end          = self~end
  instructions = self~instructions

  element = Clone( begin,   stream, context)  -- "{" -> (inserted) ";"
  element = Clone( element, stream, context ) -- Clone ";"

  Loop instruction Over instructions
    element = instruction~compile( element, stream, context )
  End

  Loop While element \== end
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* STEM VARIABLE TERMS                                                        */
/******************************************************************************/

::Method "Stem.Variable.Term::Compile"
  Use Strict Arg element, stream, context

  element = Clone( element, stream, context )

  Return element

/******************************************************************************/
/* STRING OR SYMBOL ELEMENTS                                                  */
/******************************************************************************/

::Method "StringOrSymbol.Element::Compile"
  Use Strict Arg element, stream, context

  element = Clone( element, stream, context )

  Return element

/******************************************************************************/
/* SUBEXPRESSIONS                                                             */
/******************************************************************************/

::Method "SubExpression::Compile"
  Use Strict Arg element, stream, context

  subExpression = self~subExpression

  -- "("
  element = Clone( element, stream, context )

  -- Skip until the subexpression proper begins
  begin = subExpression~begin
  Loop While element \== begin
    element = Clone( element, stream, context )
  End

  -- Compile the subexpression
  element = subExpression~compile( element, stream, context )

  -- Skip until ")"
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- ")"
  element = Clone( element, stream, context )

  Return element

/******************************************************************************/
/* SYMBOL TERMS                                                               */
/******************************************************************************/

::Method "Symbol.Term::Compile"
  Use Strict Arg element, stream, context

  element = Clone( element, stream, context )

  Return element

/******************************************************************************/
/* VARIABLE REFERENCE TERMS                                                   */
/******************************************************************************/

::Method "Variable.Reference.Term::Compile"
  Use Strict Arg element, stream, context

  -- Clone the operator
  element = Clone( element, stream, context )

  -- Skip all ignorables until we find the variable
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone the variable
  element = Clone( element, stream, context )

  Return element
