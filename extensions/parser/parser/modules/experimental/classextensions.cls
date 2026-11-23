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
/* ClassExtension.cls -- Implement ::METHOD EXTENDS and OVERRIDES             */
/* ==============================================================             */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20251112    0.3a First version                                             */
/* 20261123         The identity compiler is the one that comes with ooRexx   */
/*                  (parser/modules/identity/compile.cls, RFE #878)           */
/* 20261123         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261123         Moved from bin/modules/experimental/ to                   */
/*                  bin/parser/modules/experimental/ (RFE #878); its requires */
/*                  and the call to the loader, relative to this file         */
/* 20261123         The new name of the method, from the value of its name    */
/*                  ("a STRINGORSYMBOL.ELEMENT<+>THING" without print.cls,    */
/*                  which gave the element a string method)                   */
/*                                                                            */
/******************************************************************************/

  Overrides = .True
  Call "../Load.Parser.Module.rex" , Overrides

::Requires "../identity/compile.cls"

::Method "Method.Directive::Compile"

  Use Strict Arg element, stream, context

  -- No EXTENDS or OVERRIDES? Return the source, unaltered

  If self~extends == .Nil, self~overrides == .Nil Then Do
    element = PrepareClause( element, stream, context )
    Call CloneUntilElementIs .EL.END_OF_CLAUSE
    element = self~body~compile( element, stream, context )
    Return element
  End

  -- EXTENDS or OVERRIDES has been specified

  --
  -- We first compute the new method name as
  --
  --   methodName'<'sep'>'[nameSpace':']classname,
  --
  -- where sep is "+" for EXTENDS, and "*" for OVERRIDES
  --

  extends = self~extends \== .Nil

  If extends Then array = self~extends
  Else            array = self~overrides

  nameSpace = array[1]
  className = array[2]

  If extends Then separator = "<+>"
  Else            separator = "<*>"

  If nameSpace \== .Nil Then
    newName = self~name~value || separator || nameSpace~value":"className~value
  Else
    newName = self~name~value || separator || className~value

  newName = '"'ChangeStr('"',newName,'""')'"'

  -- We now carefully clone the directive, subtituting the
  -- method name, and commenting out the EXTENDS or OVERRIDES
  -- phrases.

  element = PrepareClause( element, stream, context )

  -- Look for a method name taken_constant and comment it out
  Call CloneUntilNameIsAndThenCommentOut .METHOD.NAME

  stream~charOut(newName)               -- Insert new name

  -- Clone everything until we find EXTENDS or OVERRIDES
  Loop
    Call CloneUntilElementIs .EL.DIRECTIVE_SUBKEYWORD
  If WordPos(element~value,"EXTENDS OVERRIDES") > 0 Then Leave
    element = Clone( element, stream, context )
  End

  Call CommentOut                       -- Comment out subkeyword

  If nameSpace \== .Nil Then Do
    -- Look for a namespace taken_constant and comment it out
    Call CloneUntilNameIsAndThenCommentOut .NAMESPACE.NAME
    -- Now look for a colon element and comment it out
    Call CloneUntilElementIsAndThenCommentOut .EL.COLON
  End

  -- Look for a class name taken_constant and comment it out
  Call CloneUntilNameIsAndThenCommentOut .CLASS.NAME

  Call CloneUntilElementIs .EL.END_OF_CLAUSE

  element = self~body~compile( element, stream, context )

  Return element

CloneUntilElementIs:
  Do While element \< Arg(1)
    element = Clone( element, stream, context )
  End
Return

CloneUntilElementIsAndThenCommentOut:
  -- Now look for a Arg(1) element...
  Do While element \< Arg(1)
    element = Clone( element, stream, context )
  End
  -- ...and comment it out
  Call CommentOut
Return

CloneUntilNameIsAndThenCommentOut:
  -- Look for an Arg(1) taken_constant...
  Do While element \<< Arg(1)
    element = Clone( element, stream, context )
  End
  -- ...and comment it out
  Call CommentOut
Return

CommentOut:
  stream~charOut("/*")
  element = Clone(element,stream,context)
  stream~charOut("*/")
Return
