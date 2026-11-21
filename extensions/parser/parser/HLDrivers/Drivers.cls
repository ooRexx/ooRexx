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
/* Highlighter.Drivers.cls - Rexx highlighter driver system                   */
/* ========================================================                   */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241231    0.1e First version                                             */
/* 20250222    0.2  Add drive to path for requires                            */
/* 20250328         Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250622    0.2d Change fg/bg char separator to ":"                        */
/* 20261031         tags2highlight: underline is 2 when dotted ("D")          */
/* 20261121         Requires ../parser/StyleSheet.cls: "parser/..." was found */
/*                  only with bin/ in the PATH, or already loaded             */
/* 20261121         Moved from bin/HLDrivers/ to bin/parser/HLDrivers/ (RFE   */
/*                  #878: everything but parser.cls lives in parser/)         */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/*                                                                            */
/******************************************************************************/

-- We call all the drivers (the .cls files) in our own directory,
-- except ourselves.

If .environment~Highlighter.Driver == .Nil Then Do
   .environment~Highlighter.Driver =  .Highlighter.Driver
  package    = .context~package
  local      =  package~local
  mypath     =  FileSpec( "Location", package~name )
  myName     =  FileSpec( "Name", package~name )
  drivers    = .File~new(mypath)~list
  Do driver Over drivers
    If driver == myName Then Iterate    -- Don't call ourselves
    If driver~endsWith(".cls") Then     -- Call every driver in turn
      Call (mypath||driver)
  End

  stem. = ""
 .environment~CSS.Cache = stem.

End

::Requires "../StyleSheet.cls"

/******************************************************************************/
/* Base class, should be subclassed by actual drivers                         */
/******************************************************************************/

::Class Highlighter.Driver Public Abstract

-- These two methods are driver-specific, and should be implemented
-- by all subclasses.

::Method Prolog    Abstract
::Method Epilog    Abstract

-- Emit a simple newline by default

::Method startLine
  Use Strict Arg lineNo
  self~say("")

-- Store options. and output

::Method Init
  Expose Options. output

  Use Strict Arg Options., output

  source = Options.!Source

  -- If we are going to print line numbers, we need to compute
  -- the width of the line number area. We use a minimum of
  -- two columns, even in the case that a single digit is enough.
  -- If the user has specified the "numberWidth" option, we use that
  -- value, unless it's incapable to accomodate our numbers.
  -- Otherwise, we use the width of the maximum line number, plus one.

  If options.numberLines Then Do
    nWidth = Max( Length(options.startFrom + source~items - 1), 2) + 1
    If options.numberWidth > nWidth Then nWidth = options.numberWidth
    Options.!numberWidth = nWidth       -- Store in options.
  End

--------------------------------------------------------------------------------
-- CachedCSS                                                                  --
--------------------------------------------------------------------------------

---
--- Return a version of the CSS associated with some tags.
---
--- @param style A style, like "dark" or "light".
--- @parem tags  A list of CSS classes.
---
--- @return A string, "b i u fg':'bg".
---

::Method CachedCSS Public

  Use Strict Arg style, tags

  -- Was "tags" already normalized?
  newTags = .CSS.Cache[style,tags]

  -- It was: return it
  If newTags \== "" Then Return newTags

  -- It was not: normalize it, cache and return the results

  newTags = GetHighlight(style, tags )

 .CSS.Cache[style,tags] = newTags

  Return newTags

--------------------------------------------------------------------------------
-- Conceptually, EMIT Arg(1) is CHAROUT , Arg(1)                              --
--------------------------------------------------------------------------------

::Method Emit
  Expose output
  output[ output~last ] ||= Arg(1)      -- Append to the current line

--------------------------------------------------------------------------------
-- Conceptually, this is SAY Arg(1)                                           --
--------------------------------------------------------------------------------

::Method Say
  Expose output
  self~emit( Arg(1) )
  output~append("")                     -- Start a new, empty line

-- Used by the ANSI and LaTeX highlighter.
-- The HTML Highlighter uses inline <style>s.

::Method tags2highlight
  Expose Options.

  Use Strict Arg tags, extra

  style      = Options.Style

  Parse Value self~CachedCSS(style, tags) With x y z fg":"bg
  attrs = x y z
  color      = ""
  background = ""
  If fg \== "" Then color      = fg
  If bg \== "" Then background = bg

  If extra \== "" Then Do
    Parse Value extra With x y z fg":"bg
    attrs ||= x y z
    If fg \== "" Then color      = fg
    If bg \== "" Then background = bg
  End

  If background = "" Then
    Parse Value self~CachedCSS(style, "rexx") With ":"background

  bold      = .False
  italic    = .False
  underline = .False

  Do attr Over attrs~makeArray("")
    Select Case attr
      When "B" Then bold      = .True
      When "b" Then bold      = .False
      When "I" Then italic    = .True
      When "i" Then italic    = .False
      When "U" Then underline = .True
      When "D" Then underline = 2           -- dotted (an ANSI line is a line)
      When "u" Then underline = .False
      Otherwise Nop
    End
  End

  Return bold italic underline color":"background