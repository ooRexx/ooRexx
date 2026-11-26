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
/* Parsing.cls -- Parse-related instructions (part of the identity compiler)  */
/* =========================================================================  */
/*                                                                            */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20251119    0.3a First version                                             */
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
/* ARG INSTRUCTIONS                                                           */
/******************************************************************************/

::Method "Arg.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for ARG keyword
  element = PrepareClause( element, stream, context )

  -- Skip ARG
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile templates
  element = self~templates~compile( element, stream, context )

  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* PARSE INSTRUCTIONS                                                         */
/******************************************************************************/

::Method "Parse.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for PARSE keyword
  element = PrepareClause( element, stream, context )

  -- Skip PARSE
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  Loop While WordPos(element~value, "UPPER LOWER CASELESS") > 0
    -- Skip subkeyword
    element = Clone( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End

  -- Skip variant subkeyword
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  If self~variant == "VAR" Then Do
    -- Compile the variable
    element = self~variable~compile( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End
  Else If self~variant == "VALUE" Then Do
    -- Compile expression
    element = self~expression~compile( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    -- Skip WITH
    element = Clone( element, stream, context )
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
  End

  -- Compile templates
  element = self~templates~compile( element, stream, context )

  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* ARG INSTRUCTIONS                                                           */
/******************************************************************************/

::Method "Pull.Instruction::compile"
  Use Strict Arg element, stream, context

  -- Look for PULL keyword
  element = PrepareClause( element, stream, context )

  -- Skip PULL
  element = Clone( element, stream, context )

  -- Skip ignorables
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Compile templates
  element = self~templates~compile( element, stream, context )

  Loop While element \< .EL.END_OF_CLAUSE
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* TEMPLATE LIST                                                              */
/******************************************************************************/

::Method "Template.List::Compile"
  Use Arg element, stream, context

  nTemplates = self~items
  Loop Counter c template Over self
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    If template \== .Nil Then Do
      element = template~compile( element, stream, context )
      -- Skip ignorables
      Loop While element~ignorable
        element = Clone( element, stream, context )
      End
    End
    If c < nTemplates Then Do
      -- Skip comma
      element = Clone( element, stream, context )
      -- Skip ignorables
      Loop While element~ignorable
        element = Clone( element, stream, context )
      End
    End
  End

  Return element

/******************************************************************************/
/* TEMPLATE                                                                   */
/******************************************************************************/

::Method "Template::Compile"
  Use Arg element, stream, context

  nComponents = self~items
  Loop Counter c component Over self
    -- Skip ignorables
    Loop While element~ignorable
      element = Clone( element, stream, context )
    End
    element = component~compile( element, stream, context )
  End

  Return element

/******************************************************************************/
/* PREFIXED TEMPLATE                                                          */
/******************************************************************************/

::Method "Prefixed.Template::Compile"
  Use Arg element, stream, context

  -- Skip ignorables until prefix
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  -- Clone prefix
  element = Clone( element, stream, context )

  -- Skip ignorables again
  Loop While element~ignorable
    element = Clone( element, stream, context )
  End

  element = self~elementOrSubExpression~compile( element, stream, context )

  Return element
