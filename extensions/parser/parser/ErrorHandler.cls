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
/* ErrorHandler.cls -- Common error handler routine                           */
/* ================================================                           */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20251221    0.4a  First public release                                     */
/* 20251226          Send error messages to .error, not .output               */
/* 20260313    0.5   Add ReportParseError for pipeline error handling         */
/* 20260930          Show the clause in error, not the whole line (issue #8)  */
/* 20261018          IsAParseError: a FencedCode 88.900 (bad option...) is    */
/*                   said in one line, without a traceback                    */
/*                   Show the hint, when the error has one (issue #39)        */
/* 20261115    0.7   IsAParseError: the parser's extra information is a       */
/*                   Directory, whose entries are not methods, so             */
/*                   hasMethod("position") never found a parse error, and a   */
/*                   batch without --continue stopped with a raw ooRexx error */
/* 20261119          The messages come from ERRORTEXT, which substitutes      */
/*                   (ooRexx r13480, RFE 766), not from ANSI.ErrorText.cls,   */
/*                   which is gone                                            */
/* 20261119          The errors of the parser are a Rexx.Parser.Error, thrown */
/*                   as USER REXX.PARSER.ERROR (THROW, RFE #31): ErrorHandler */
/*                   takes its condition object, and shows the error's        */
/*                   report. New ReportParseError, for the pipelines.         */
/*                   IsAParseError only says the 88.900 that FencedCode       */
/*                   raises for a mistake in the document                     */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* ErrorHandler(name, source, co, itrace) -- Standard error handler           */
/*                                                                            */
/* For use by standalone utilities (rexxcheck, highlight...), in a CATCH      */
/* clause of the condition USER REXX.PARSER.ERROR: co is its condition        */
/* object, and its ADDITIONAL the Rexx.Parser.Error. Displays the error as    */
/* ooRexx does, with "name" as the program's name (unless it is ""), and      */
/* exits with -major, following the ooRexx convention.                        */
/*                                                                            */
/* Usage: Exit ErrorHandler( ... )                                            */
/*                                                                            */
/******************************************************************************/

::Routine ErrorHandler Public
  Use Strict Arg name, source, co, itrace
  error = co~additional
  If name \== "" Then error~in( name )
  error~withSource( source )
  Call ReportParseError error, itrace
  Exit - error~major


/******************************************************************************/
/*                                                                            */
/* ReportParseError(error, itrace) -- Report a Rexx.Parser.Error              */
/*                                                                            */
/* For the RexxPub pipelines (CGI, md2pdf, md2html...), which keep control    */
/* after the error (to go on in batch mode, say), and for ErrorHandler.       */
/* Displays the error on .Error as ooRexx does, and, if itrace, the stack     */
/* frames where the parser found it.                                          */
/*                                                                            */
/******************************************************************************/

::Routine ReportParseError Public
  Use Strict Arg error, itrace
  Do line Over error~report
   .Error~Say( line )
  End
  If itrace Then Do
   .Error~Say
   .Error~Say( "Trace follows:"         )
   .Error~Say( Copies("-",80)           )
   .Error~Say( error~stackFrames~makeArray )
  End


/******************************************************************************/
/*                                                                            */
/* IsAParseError(co, itrace) -- An error that FencedCode found in a document  */
/*                                                                            */
/* For the RexxPub pipelines, in a CATCH clause of SYNTAX. A SYNTAX 88.900    */
/* raised by FencedCode is a mistake in the document (a bad option, a block   */
/* with no end), not a bug: it is said in one line, without a traceback       */
/* (Rony, 18/10/2026), and the routine returns 1. Anything else returns 0,    */
/* and the caller should propagate it. (The errors of the parser are not      */
/* SYNTAX any more: see ReportParseError.)                                    */
/*                                                                            */
/* Usage:                                                                     */
/*    If \IsAParseError( ... ) Then Raise Propagate -- Application error      */
/*    -- Error already handled                                                */
/*                                                                            */
/******************************************************************************/

::Routine IsAParseError Public
  Use Strict Arg co, itrace

  If co~code \== "88.900" Then Return 0
  If \co~program~caselessEndsWith("FencedCode.cls") Then Return 0
 .Error~Say( "Error:" co~additional~lastItem )
  If itrace Then Do
   .Error~Say
   .Error~Say( "Trace follows:"         )
   .Error~Say( Copies("-",80)           )
   .Error~Say( co~stackFrames~makeArray )
  End
  Return 1
