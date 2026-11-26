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
/* Directives.cls -- Clone directives (part of the identity compiler)         */
/* ==================================================================         */
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
/* 20251212         (Executor) Implement the ::EXTENSION directive            */
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
/*                                                                            */
/******************************************************************************/

  Call "../Load.Parser.Module.rex"

  pkgLocal = .context~package~local

  -- Set to 1 to activate debug
  pkgLocal~DEBUG = 0

::Requires "compile.cls"

/******************************************************************************/
/******************************************************************************/
/* DIRECTIVES                                                                 */
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* ANNOTATE DIRECTIVE                                                         */
/******************************************************************************/

::Method "Annotate.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Clone everything until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- Compile empty code body
  element = self~body~compile( element, stream, context )

  Return element

/******************************************************************************/
/* ATTRIBUTE DIRECTIVE                                                        */
/******************************************************************************/

::Method "Attribute.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Clone everything until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- Compile possibly empty code body
  element = self~body~compile( element, stream, context )

  Return element

/******************************************************************************/
/* CLASS DIRECTIVE                                                            */
/******************************************************************************/

::Method "Class.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Clone everything until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- Compile empty code body
  element = self~body~compile( element, stream, context )

  Return element

/******************************************************************************/
/* COACTIVITY DIRECTIVE                                                       */
/******************************************************************************/

::Method "Coactivity.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Clone everything until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- No body for ::co

  Return element

/******************************************************************************/
/* CONSTANT DIRECTIVE                                                         */
/******************************************************************************/

::Method "Constant.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End
  element = Clone( element, stream, context )

  -- Clone ignorables
  Do while element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone CONSTANT
  element = Clone( element, stream, context )

  -- Clone ignorables
  Do while element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone constant name
  element = Clone( element, stream, context )

  -- Clone ignorables
  Do while element~ignorable
    element = Clone( element, stream, context )
  End

  Select
    When element < .EL.END_OF_CLAUSE Then Nop
    When element < .ALL.OPS.ADDITIVE Then Do
      -- Clone sign
      element = Clone( element, stream, context )
      -- Clone ignorables
      Do while element~ignorable
        element = Clone( element, stream, context )
      End
      -- Clone integer
      element = Clone( element, stream, context )
      -- Clone ignorables
      Do while element~ignorable
        element = Clone( element, stream, context )
      End
    End
    When element < .EL.LEFT_PARENTHESIS Then Do
      -- Compile (expression)
      element = self~value~compile( element, stream, context )
    End
    Otherwise
      element = Clone( element, stream, context )
  End

  -- Clone ignorables
  Do while element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile empty code body
  element = self~body~compile( element, stream, context )

  Return element


/******************************************************************************/
/* EXTENSION DIRECTIVE                                                        */
/******************************************************************************/

::Method "Extension.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Clone everything until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- Compile empty code body
  element = self~body~compile( element, stream, context )

  Return element

/******************************************************************************/
/* METHOD DIRECTIVE                                                           */
/******************************************************************************/

::Method "Method.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Clone everything until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- Compile code body
  element = self~body~compile( element, stream, context )

  Return element

/******************************************************************************/
/* OPTIONS DIRECTIVE                                                          */
/******************************************************************************/

::Method "Options.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Clone everything until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- Compile empty code body
  element = self~body~compile( element, stream, context )

  Return element

/******************************************************************************/
/* REQUIRES DIRECTIVE                                                         */
/******************************************************************************/

::Method "Requires.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Clone everything until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- Compile empty code body
  element = self~body~compile( element, stream, context )

  Return element

/******************************************************************************/
/* RESOURCE DIRECTIVE                                                         */
/******************************************************************************/

::Method "Resource.Directive::compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Now clone until the resource data starts
  Do While element \< .EL.RESOURCE_DATA
    element = Clone( element, stream, context )
  End

  -- Clone the resource data itself
  element = element~compile( element, stream, context )

  -- Now clone the end marker
  Do While element \== self~directiveClause~lastElement
    element = Clone( element, stream, context )
  End

  -- Ignorable stuff until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- An empty code body
  element = self~body~compile( element, stream, context )

  Return element

::Method "Resource.Data::compile"
  Use Strict Arg element, stream, context

  Do line Over self~sourceLines
    stream~Say( line )
  End

  Return element~next

/******************************************************************************/
/* R DIRECTIVE                                                                */
/******************************************************************************/

::Method "R.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Clone everything until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- No body for ::r

  Return element

/******************************************************************************/
/* ROUTINE DIRECTIVE                                                          */
/******************************************************************************/

::Method "Routine.Directive::Compile"
  Use Strict Arg element, stream, context

  -- Clone up to "::"
  Do While element \< .EL.DIRECTIVE_START
    element = Clone( element, stream, context )
  End

  -- Ignorable stuff until EOC
  Do while element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  -- Compile code body
  element = self~body~compile( element, stream, context )

  Return element
