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
/* Clauses.cls -- Clone clauses (part of the identity compiler)               */
/* ============================================================               */
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
/* 20261113         CATCH and FINALLY clauses (RFE #31)                       */
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
/* CLAUSES                                                                    */
/******************************************************************************/

::Method "Catch.Clause::compile"
  Use Strict Arg element, stream, context

  -- Look for CATCH keyword
  element = PrepareClause(element, stream, context)

  -- Skip until EOC
  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

::Method "Else.Clause::compile"
  Use Strict Arg element, stream, context

  element = PrepareClause(element, stream, context)

  element = Clone( element, stream, context )

  Return element

::Method "End.Clause::compile"
  Use Strict Arg element, stream, context

  -- Look for END keyword
  element = PrepareClause(element, stream, context)

  -- Clone everything until EOC
  Do Until element < .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

::Method "Finally.Clause::compile"
  Use Strict Arg element, stream, context

  element = PrepareClause(element, stream, context)

  element = Clone( element, stream, context )

  Return element

::Method "If.Clause::compile"
  Use Strict Arg element, stream, context

  -- Look for IF keyword
  element = PrepareClause(element, stream, context)

  -- Clone IF keyword
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile the expressions
  element = self~expressions~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element

::Method "Otherwise.Clause::compile"
  Use Strict Arg element, stream, context

  element = PrepareClause(element, stream, context)

  element = Clone( element, stream, context )

  Return element

::Method "Select.Clause::compile"
  Use Strict Arg element, stream, context

  -- Look for SELECT keyword
  element = PrepareClause(element, stream, context)

  If self~case \== .Nil Then Do
    -- Look for CASE keyword
    Loop Until element <.EL.SUBKEYWORD, element~value == "CASE"
      element = Clone( element, stream, context )
    End
    -- Skip it
    element = Clone( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    element = self~case~compile( element, stream, context )
  End

  -- Clone everything until EOC
  Do While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

::Method "Then.Clause::compile"
  Use Strict Arg element, stream, context

  element = PrepareClause(element, stream, context)

  element = Clone( element, stream, context )

  Return element

::Method "When.Clause::compile"
  Use Strict Arg element, stream, context

  -- Look for WHEN keyword
  element = PrepareClause(element, stream, context)

  -- Clone WHEN keyword
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile the expressions
  element = self~expressions~compile( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Return element
