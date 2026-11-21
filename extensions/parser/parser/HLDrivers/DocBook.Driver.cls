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
/* DocBook.Driver.cls - DocBook highlighter driver                            */
/* ===============================================                            */
/*                                                                            */
/* Emits a DocBook <phrase role="..."> element around every highlighted       */
/* token.  The role attribute carries the highlighter's CSS class string      */
/* verbatim -- exactly what the HTML driver puts in class=:                   */
/*                                                                            */
/*   "rx-kw"                       -> <phrase role="rx-kw">                   */
/*   "rx-op rx-add"                -> <phrase role="rx-op rx-add">            */
/*   "rx-const rx-method rx-oquo"  -> <phrase role="rx-const rx-method        */
/*                                       rx-oquo">                            */
/*                                                                            */
/* Two properties of this markup matter, and both are deliberate:             */
/*                                                                            */
/*   1. It is valid DocBook.  <phrase role="..."> is the standard idiom for   */
/*      a semantically tagged inline, so the emitted XML validates against    */
/*      the DocBook 4.5 DTD and survives any validating step of the build.    */
/*      It also means the stock DocBook XSL already knows what to do with     */
/*      it: the XHTML stylesheets turn <phrase role="X"> into <span           */
/*      class="X"> unaided, so the HTML branch needs no per-token templates   */
/*      at all.                                                               */
/*                                                                            */
/*   2. It is style-independent.  The markup emitted for a given token is     */
/*      identical whatever style is in effect; the style is carried by the    */
/*      container element, not by the tokens.  This is the same invariant     */
/*      the HTML driver relies on, and it is what allows one highlighted      */
/*      document to be restyled -- by CSS in HTML, by the generated           */
/*      templates in FO -- without parsing the source again.                  */
/*                                                                            */
/* Whitespace is emitted as bare escaped text, with no wrapping element.      */
/* The content is XML-escaped.  No prolog or epilog is emitted; the           */
/* container is supplied by Parser.DocBook.cls, which owns the enclosing      */
/* <programlisting>.                                                          */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20260403    0.5  First version                                             */
/* 20260729    0.6  Emit <phrase role="..."> instead of <rexx_STYLE_*>        */
/*                  elements.  The style no longer appears in the token       */
/*                  markup; Tags2Element and StripPrefix are gone.            */
/* 20261121         Moved from bin/HLDrivers/ to bin/parser/HLDrivers/ (RFE   */
/*                  #878: everything but parser.cls lives in parser/)         */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/*                                                                            */
/******************************************************************************/


If \.environment~hasIndex( DocBook.Highlighter ) Then Do
  .environment~DocBook.Highlighter = .DocBook.Highlighter
End

::Class DocBook.Highlighter SubClass Highlighter.Driver Public

::Method Init
  Expose Options. output
  Use Strict Arg Options., output
  self~init:super(Options., output)

::Method Prolog
  Nop

::Method startLine
  Use Strict Arg lineNo
  self~say("")

::Method Highlight
  Use Strict Arg category, subCategory, variant, tags, string

  -- Whitespace: emit as-is (XML-escaped, no element wrapper).  A
  -- <phrase> around a run of spaces would carry no information and
  -- would bloat the document for no gain.
  If tags == "rx-ws" Then Do
    self~emit( XMLEscape(string) )
    Return
  End

  -- An untagged token is emitted bare as well: an empty role would be
  -- meaningless in DocBook and would produce <span class=""> in HTML.
  If tags == "" Then Do
    self~emit( XMLEscape(string) )
    Return
  End

  self~emit( '<phrase role="'tags'">'XMLEscape(string)'</phrase>' )

::Method Epilog
  Nop

/******************************************************************************/
/* XMLESCAPE: Escape XML special characters in content                        */
/******************************************************************************/

::Routine XMLEscape

  Use Strict Arg text

  text = text~changeStr("&",  "&amp;")
  text = text~changeStr("<",  "&lt;")
  text = text~changeStr(">",  "&gt;")
  text = text~changeStr('"',  "&quot;")
  text = text~changeStr("'",  "&apos;")

  Return text
