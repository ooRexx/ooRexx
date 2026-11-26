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
/* compile.cls -- Clone a program using the Tree API                          */
/* =================================================                          */
/*                                                                            */
/* This class implements a skeleton identity compiler. It compiles a ooRexx   */
/* program to a perfect copy of itself.                                       */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20250707    0.2d First version                                             */
/* 20251102    0.2e Rename to "compile.cls" (was "clonetree")                 */
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

  -- Elements to skip to get to the beginning of a clause
  pkgLocal~SKIP.BEFORE.CLAUSE =    -
   .EL.END_OF_CLAUSE    ||         -    -- ends Of clauses (implied or not )
   .ALL.WHITESPACE_LIKE ||         -    -- Including continuations
   .ALL.COMMENTS        ||         -    -- Including doc-comments
   .EL.TAKEN_CONSTANT   ||         -    -- Labels
   .EL.COLON                            -- Label colons


::Requires "../../BaseClassesAndRoutines.cls"

/******************************************************************************/
/* CLONE -- Clone a single element                                            */
/******************************************************************************/

::Routine Clone Public
  Use Strict Arg element, stream, context

  -- Inserted elements are zero-length and do not need to be cloned
  If \element~isInserted Then Do
    If element < .ALL.COMMENTS, -- Special case: multi-line block comments
      element~from~word(1) \== element~to~word(1) Then Do
      a = element~sourceLines
      n = a~items
      Loop Counter c l Over a
        If c < n Then stream~Say(l)
        Else          stream~charOut(l)
      End
    End
    Else stream~charOut( element~source )
  End

  -- Get next element
  next = element~next

  -- Take care of newlines
  If element~to~word(1) \== next~from~word(1) Then stream~Say

  -- Return next element
  Return next

/******************************************************************************/
/* PREPARECLAUSE -- Skip whitespace, end-of-clause, comments and labels       */
/******************************************************************************/

::Routine PrepareClause Public
  Use Strict Arg element, stream, context

  -- Clone initial end-of-clauses, whitespace, comments,
  -- doc-comments and labels
  Do While element < .SKIP.BEFORE.CLAUSE
    If element < .EL.TAKEN_CONSTANT Then Do
      next = TheElementAfter( element, 1 )
      If next \< .EL.COLON Then Leave
    End
    element = Clone( element, stream, context )
  End

  Return element

/******************************************************************************/
/* CODE BODIES                                                                */
/******************************************************************************/

::Method "Code.Body::compile"
  Use Strict Arg element, stream, context

  Do instruction Over self~instructions
    element = instruction~compile( element, stream, context )
  End

  Return element

/******************************************************************************/
/* PACKAGES                                                                   */
/******************************************************************************/

::Method "Rexx.Package::compile"
  Use Strict Arg element, stream, context

  element = self~prolog~body~compile( element, stream, context )

  Loop directive Over self~directives
    element = directive~compile( element, stream, context )
  End

  Return element~next

