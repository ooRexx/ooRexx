/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026 Josep Maria Blasco. All rights reserved.           */
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
/******************************************************************************/
/*                                                                            */
/* DocumentError.cls -- A mistake in a document                               */
/* ============================================                               */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20261126    1.0  First version: the errors that FencedCode finds in a      */
/*                  Markdown document (a bad option, a block with no end, a   */
/*                  bad style patch) are a Rexx.Parser.DocumentError, thrown  */
/*                  as USER REXX.PARSER.DOCUMENTERROR, and no longer a        */
/*                  SYNTAX 88.900                                             */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Rexx.Parser.DocumentError -- a mistake in a document                       */
/*                                                                            */
/*   Not an error in a program (that is a Rexx.Parser.Error), but in the      */
/*   document that holds it: a code block with no end, an option that does   */
/*   not exist, a style patch that cannot be read. It is thrown (~throw) as   */
/*   the USER condition REXX.PARSER.DOCUMENTERROR, with itself as the         */
/*   ADDITIONAL of the condition and its description as the DESCRIPTION.      */
/*   ReportDocumentError (ErrorHandler.cls) displays it.                      */
/*                                                                            */
/*   message      what is wrong                                               */
/*   line         the line of the document, or "" when there is none (the   */
/*                default options of the document, say)                       */
/*   name         the document, or ""                                         */
/*   sourceLine   the line in error (a fence, say), or ""                     */
/*   stackFrames  the stack frames where the error was found (for the        */
/*                internal traces of the utilities)                           */
/*                                                                            */
/******************************************************************************/

::Class Rexx.Parser.DocumentError Public

::Attribute message     Get
::Attribute line        Get
::Attribute name        Get
::Attribute sourceLine  Get
::Attribute stackFrames Get

::Method init
  Expose message line name sourceLine stackFrames
  Use Strict Arg message, line = "", name = "", sourceLine = ""
  stackFrames = .context~stackFrames

-- "x.md:3: Invalid option 'numberlines'", as compilers say it: the
-- DESCRIPTION of the condition, and the string value of the error
::Method description
  Expose message line name
  where = name
  If line \== "" Then Do
    If where == "" Then where = "line" line
    Else where = where":"line
  End
  If where == "" Then Return message
  Return where":" message

::Method makeString
  Return self~description

::Method string
  Return self~description

-- The report of the error, one line per item: the description, and the
-- line in error, if any, indented
::Method report
  Expose sourceLine
  report = .Array~of( self~description )
  If sourceLine \== "" Then report~append( "    "Strip(sourceLine) )
  Return report

-- The error is in "name", "offset" lines below where its line says (a
-- document inside another one: a cell of a grid table, say)
::Method in
  Expose name line
  Use Strict Arg name, offset = 0
  If line \== "" Then line += offset
  Return self

-- Throw the error: USER REXX.PARSER.DOCUMENTERROR, with this object as
-- ADDITIONAL
::Method throw
  Throw User Rexx.Parser.DocumentError Description (self~description) -
    Additional (self)
