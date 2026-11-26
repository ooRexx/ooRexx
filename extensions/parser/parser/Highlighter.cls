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
/* Highlighter.cls - Rexx highlighter class                                   */
/* ========================================                                   */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241206    0.1  First public release                                      */
/* 20241208    0.1a Add support for extraletters option                       */
/*                  c/CLASSIC_COMMENT/STANDARD_COMMENT/                       */
/* 20241222    0.1c Add support for .numberLines, numberWidth, startFrom      */
/*                  and pad                                                   */
/* 20241225    0.1d Add support for doc-comments                              */
/* 20241229    0.1e Add support for ANSI highlighing                          */
/*                  Rename to Rexx.Highlighter                                */
/* 20241231         Use new driver system                                     */
/* 20250102    0.1f Add "prolog" option                                       */
/* 20250103         Add "unicode" and "tutor" options                         */
/* 20250106         Rename to Highlighter, move to cls directory              */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250519    0.2b Fix error computing mypath, add dependencies              */
/* 20250520         Ensure that options. == 0                                 */
/* 20250529    0.2c Add support for detailed string and number highlighing    */
/* 20250622    0.2d Change fg/bg char separator to ":"                        */
/* 20250630         Add support for doc-comment subelements                   */
/* 20251114    0.3a Add support for Experimental features                     */
/* 20251125         Add support for Executor                                  */
/* 20251226    0.4a Send error messages to .error, not .output                */
/* 20260620         CMS: Implement multi-line strings                         */
/* 20260911         %SPLIT: honour a configurable marker char (metachar=)     */
/* 20260912         Drop %SPLIT entirely: RemoveSplitMarkers, the metachar    */
/*                  default and Options.!Splits. FencedCode cuts a program=   */
/*                  listing cold, after emission, so no marker is needed.     */
/* 20260930         An error's sourceline is the clause in error, as ooRexx   */
/*                  shows it (issue #8), not the whole line                   */
/* 20261025         Such a continuation is shown as a continuation            */
/* 20261115         Use CATCH instead of SIGNAL ON (RFE #31; needs ooRexx     */
/*                  r13514 or later)                                          */
/* 20261119         A syntax error in the source is a Rexx.Parser.Error,      */
/*                  thrown (THROW, RFE #31): it gets the name, and is thrown  */
/*                  again (it was a SYNTAX 98.900, raised again with the code */
/*                  of the error)                                             */
/* 20261121         Loads parser.cls itself: it uses .Rexx.Parser, and had it */
/*                  only when FencedCode.cls was loaded too                   */
/* 20261121         LATEX mode without the LaTeX driver (RexxPub's, maybe     */
/*                  not installed): a 98.900 that says so, not a 97.1         */
/* 20261121         The drivers are in parser/HLDrivers/ (RFE #878)           */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261121         The LaTeX driver is looked for in the search path, as     */
/*                  rexxpub/Latex.Driver.cls (RFE #878)                       */
/* 20261121         The parser gets the name of the program (it got the       */
/*                  unset variable FILE: the error had the right name only    */
/*                  because it was renamed when it went up)                   */
/* 20261122         The LaTeX driver only in the search path (no              */
/*                  ../rexxpub/); no Options.!UniqueID (nobody used it: the   */
/*                  HTML of a patch uses the block id)                        */
/* 20261123         No Catch of SYNTAX in parse: it wrote an error header to  */
/*                  .ERROR even when the caller caught the condition, and a   */
/*                  utility that prints its own printed it twice              */
/* 20261125         The cut of the tree API: the names, types and attributes  */
/*                  decided in the review of the Element and Tree APIs; the   */
/*                  attributes that only the parser writes are read-only      */
/*                  (set... methods)                                          */
/*                                                                            */
/******************************************************************************/

-- Make our class known to everybody,

.environment~Highlighter =  .Highlighter

package      = .context~package
pkgLocal     =  package~local
myName       =  package~name
mypath       =  FileSpec( "Location", myName )
pkgLocal ~ . = .File~new( mypath"../" )~absolutePath

/******************************************************************************/
/* Dependencies                                                               */
/******************************************************************************/

package~loadPackage(pkgLocal~."/parser.cls")
package~loadPackage(mypath"HLDrivers/Drivers.cls")

-- The LaTeX driver is RexxPub's: it lives in the rexxpub/ directory of
-- RexxPub's package, which may not be installed. When the search path finds
-- it, we call it as Drivers.cls calls the drivers next to it
latexDriver = package~findProgram("rexxpub/Latex.Driver.cls")
If latexDriver \== .Nil Then Call (latexDriver)

::Requires "BaseClassesAndRoutines.cls"
::Requires "StylePatch.cls"
::Requires "HTMLClasses.cls"

/******************************************************************************/
/*                                                                            */
/* This is the main highlighting class. To create an instance, we need        */
/* a name, a source array, and an options stem. The Parse method, which       */
/* accepts an optional StylePatch argument, retuns a new array                */
/* containing the highlighted source code.                                    */
/*                                                                            */
/******************************************************************************/

::Class Highlighter Public

--------------------------------------------------------------------------------
-- INIT method                                                                --
--------------------------------------------------------------------------------

::Method init
  Expose name source options.

  Use Strict Arg name, source, options. = (.Nil)

 .Validate~classType( "name",   name,   .String )
 .Validate~classType( "source", source, .Array  )

  -- Set some sane default for Options.
  If options. == .Nil Then Options. = 0

  If options.[] \== 0 Then Do
   .Error~Say( ">>> Default value for options argument to Highlighter~new is not 0. <<<" )
    Raise Halt
  End

  Call Default mode,        "HTML"
  Call Default docComments, "detailed"
  Call Default operator,    "group"
  Call Default assignment,  "group"
  Call Default special,     "group"
  Call Default constant,    "full"
  Call Default classprefix, "rx-"
  Call Default style,       "dark"
  Call Default startFrom,   1

  If Verify(Options.style, XRange(AlNum)".-_") > 0 Then
    Raise Halt -
      Array("Style names are limited to letters, numbers, '.', '-' and '_'.")

  Do tail Over Options.~allIndexes
    -- Text the author wrote, not a keyword: caption= is shown, spot= is
    -- searched for in the listing (spot='trap:["DATE"()]', Rony 28-Sep).
    If tail == "CAPTION" | tail == "SPOT" Then Iterate
    Options.tail = Lower( options.tail )
  End
  Options.mode = Upper( Options.mode )

  If Options.mode == "ANSI" Then Call Default width, 80

  Options.!Source    = source

  Return

Default:
  Use Arg key, value
  If \ Options.~hasIndex(key)   Then Options.key   = value
Return

--------------------------------------------------------------------------------
-- PARSE method                                                               --
--------------------------------------------------------------------------------

::Method parse
  Expose name source options. HTMLClass

  numberLines = options.numberLines

  output = .Array~of("")

  Select Case Options.mode
    When "HTML"  Then
      Options.!Driver =  .HTML.Highlighter~new( Options., output )
    When "HTML-INLINE" Then
      Options.!Driver =  .HTML.Inline.Highlighter~new( Options., output )
    When "ANSI" Then
      Options.!Driver =  .ANSI.Highlighter~new( Options., output )
    When "LATEX" Then Do
      -- The LaTeX driver is RexxPub's, and may not be installed
      If \.environment~hasIndex("LATEX.HIGHLIGHTER") Then
        Raise Syntax 98.900 Array( "LaTeX highlighting needs the LaTeX" -
          "driver (rexxpub/Latex.Driver.cls), which is part of RexxPub" )
      Options.!Driver = .LaTeX.Highlighter~new( Options., output )
    End
    When "DOCBOOK" Then
      Options.!Driver = .DocBook.Highlighter~new( Options., output )
    Otherwise Nop
  End

  Use Strict arg patch = (.Nil)

  If patch == .Nil Then Do
    If Options.~hasIndex(additional) Then
      patch = Options.additional
  End
  If      patch~isA(.String) Then patch = .StylePatch~of( patch )
  Else If patch~isA(.Array)  Then patch = .StylePatch~of( patch )
  Else If patch \== .Nil     Then
    .Validate~classType("patch", patch, .StylePatch)
  Options.!Patch = patch

  HTMLClass = HTMLClasses( options. )
  Options.!HTMLClass = HTMLClass

  Options.!inc = 1

  parserOptions = .Array~new
  If options.~hasIndex(extraletters) Then
    parserOptions~append( (extraletters, options.extraletters) )
  If options.unicode == 1 | options.tutor == 1 Then
    parserOptions~append( (unicode, 1) )
  If options.experimental == 1 Then
    parserOptions~append( (experimental, 1) )
  If options.executor == 1 Then
    parserOptions~append( (executor, 1) )
  If options.cms == 1 Then
    parserOptions~append( (cms, 1) )

  -- Instantiate a parser, parse the program, and get the first element
  parser  = .Rexx.Parser~new(name, source, parserOptions)
  package = parser~package
  element = package~firstElement

  -- Emit a prolog
  Options.!Driver~Prolog

  -- Process all elements in turn until the element chain is exhausted

  currentLine = 1
  last = .Nil
  Loop Until element == .Nil
    -- Print an element...
    Call Print source, output, element, Options., >currentLine, HTMLClass
    -- ... and fetch the next one.
    last  = element
    element = element~next
  End

  -- Emit our epilog
  Options.!Driver~Epilog

  Return output

/******************************************************************************/
/* PRINT -- Process an element: tag it appropriately and append it            */
/*          to the output array.                                              */
/******************************************************************************/

::Routine Print

  Use arg source, output, element, Options., >currentLine, HTMLClass

  -- Since we are skipping zero-width elements, ...
  -- ... we will have to take special care about line ends
  Parse Value element~from With line .
  Do While Options.!inc < line
    Call NewLine Options.!inc + Options.startFrom
    currentLine += 1
  End

  -- If this is an inserted element, there is nothing to print
  If element~from == element~to          Then Return

  -- Strings or symbols taken as a constant require special handling
  constant = .EL.TAKEN_CONSTANT
  If element < constant                  Then Signal TakenConstant

  -- Compound variables are displayed highlighting all the
  -- elements in the tail.
  If element < .ALL.COMPOUND_VARIABLES   Then Signal CompoundVariable

  If element < .ALL.STRINGS Then Do
    -- Strings that are numbers and are not X, B or U get detailed number
    -- highlighting -- but only when single-line. A multi-line numeric
    -- string falls through to plain (multi-line) string highlighting,
    -- since digit-by-digit highlighting across a line break is moot.
    If \ element~isMultiLine, -
      element \< .EL.HEX_STRING || .EL.BINARY_STRING || .EL.UNICODE_STRING, -
      DataType(element~value) == "NUM"  Then Signal Numbers
    -- Detailed string highlighting
    Signal String
  End

  -- Detailed highlighting of numbers
  If element < .ALL.NUMBERS               Then Signal Numbers

  -- Classic comments and doc-comments may be multi-line,
  -- in which case they also require special handling
  If element < .EL.STANDARD_COMMENT       Then Signal BlockComments
  If options.docComments == "detailed"    Then Do
    If element < .EL.DOC_COMMENT          Then Signal DocComment
    If element < .EL.DOC_COMMENT_MARKDOWN Then Signal DocComment
  End
  Else Do
    If element < .EL.DOC_COMMENT          Then Signal BlockComments
    If element < .EL.DOC_COMMENT_MARKDOWN Then Signal BlockComments
  End

  -- Process ::Resources (probably multi-line)
  If element < .EL.RESOURCE_DATA          Then Signal ResourceData

  -- A continuation character that is the blank operator too ("say 1 -",
  -- then "2"): shown as the continuation it is in the source
  If element < .EL.OP.BLANK, element~continuation == 1 Then Do
    tag = HTMLClass[ .EL.CONTINUATION ]
    Call Highlight .EL.CONTINUATION, "", "", tag, element~source
    Exit
  End

  -- Standard case
  tag = HTMLClass[ element~category ]
  Call Highlight element~category, "", "", tag, element~source

Exit

Highlight:
  Options.!Driver~highlight( Arg(1), Arg(2), Arg(3), Arg(4), Arg(5) )
Return

NewLine:
  Options.!Driver~startLine( Arg(1) )
  options.!inc += 1
Return

TakenConstant:
  If element~isAString Then Signal String
  tag = HTMLClass[ constant, element~subCategory ]
  Call Highlight constant, element~subCategory, "", tag, element~source
Exit

--------------------------------------------------------------------------------
-- RESOURCE DATA                                                              --
--------------------------------------------------------------------------------

ResourceData:
  start   = element~from~word(1)
  end     = element~  to~word(1)
  tag     = HTMLClass[ element~category ]
  padding = options.pad
  Do Counter c line = start to end
    If c > 1 Then Do
      displayLine = currentLine + (c-1) + options.startFrom - 1
      Call NewLine displayLine
    End
    sourceLine = source[line]
    If padding > 0, padding > sourceLine~length Then
      sourceLine = Left(sourceLine, padding)
    Call Highlight element~category, "", "", tag, sourceLine
  End
  displayLine = currentLine + c + options.startFrom - 1
  Call NewLine displayLine
  currentLine += end - start + 1
Exit

--------------------------------------------------------------------------------
-- BLOCK COMMENTS                                                             --
--------------------------------------------------------------------------------

BlockComments:
  comment = element~sourceLines
  lines   = comment~items
  tag = HTMLClass[ element~category ]
  padding = options.pad
  Do Counter c line Over comment
    If c > 1 Then Do
      currentline += 1
      displayLine = currentLine + options.startFrom - 1
      Call NewLine displayLine
    End
    Call Highlight element~category, "", "", tag, line
  End
Exit

--------------------------------------------------------------------------------
-- NUMBERS                                                                    --
--------------------------------------------------------------------------------

Numbers:
  Call ComputeSourceCatsOpenCloseAndSuffix

  -- Highlight the opening quote, if any
  If q \== "" Then
    Call Highlight .EL.STRING_OPENING_DELIMITER, "", "", open,  q

  -- That's the number we have to deal with
  If      q == ""   Then Nop
  Else If hasSuffix Then eSource = SubStr( eSource, 2, Length(eSource) - 3 )
  Else                   eSource = SubStr( eSource, 2, Length(eSource) - 2 )

  -- Compute the default tag(s)
  tag = HTMLClass[ cat ]

  -- Initial blanks (--> that's a string)
  p = Verify(eSource, " ")
  If p > 1 Then Call Highlight cat, subCat, "", tag, Left( eSource, p - 1 )

  -- Optional sign (for strings also)
  s = eSource[p]
  If s == "+" | s == "-" Then Do
    nSign = HTMLClass[ .EL.NUMBER_SIGN ]
    Call Highlight .EL.NUMBER_SIGN, "", "", tag nSign, s
    p += 1
  End

  -- There may be some blanks after the sign
  p2 = Verify(eSource, " ",, p)
  If p2 > p Then Do
    len = p2 - p
    Call Highlight .EL.INTEGER_PART, "", "", tag, SubStr( eSource, p, len )
    p = p2
  End

  -- Integer part
  p2 = Verify( eSource, "0123456789",, p )
  If p2 == 0 | p2 > p Then Do
    If p2 == 0 Then len = Length( eSource ) - p + 1
    Else            len = p2 - p
    intPart = HTMLClass[ .EL.INTEGER_PART ]
    Call Highlight .EL.INTEGER_PART, "", "", tag intPart, SubStr( eSource, p, len )
    p = p2
  End
  If p == 0 Then Signal NumberDone

  -- Decimal point
  If eSource[p] == "." Then Do
    decPoint = HTMLClass[ .EL.DECIMAL_POINT ]
    Call Highlight .EL.DECIMAL_POINT, "", "", tag decPoint, "."
    p = p + 1
  End

  -- Fractional part
  p2 = Verify( eSource, "0123456789",, p )
  If p2 == 0 | p2 > p Then Do
    If p2 == 0 Then len = Length( eSource ) - p + 1
    Else            len = p2 - p
    fracPart = HTMLClass[ .EL.FRACTIONAL_PART ]
    Call Highlight .EL.FRACTIONAL_PART, "", "", tag fracPart, SubStr( eSource, p, len )
    p = p2
  End
  If p == 0 Then Signal NumberDone

  -- Exponent mark
  e = eSource[p]
  eMark = HTMLClass[ .EL.EXPONENT_MARK ]
  Call Highlight .EL.EXPONENT_MARK, "", "", tag eMark, e
  p = p + 1

  -- Exponent sign
  s = eSource[p]
  If s == "-" | s == "+" Then Do
    eSign = HTMLClass[ .EL.EXPONENT_SIGN ]
    Call Highlight .EL.EXPONENT_SIGN, "", "", tag eSign, s
    p = p + 1
  End

  -- The exponent itself
  p2 = Verify( eSource, "0123456789",, p )
  If p2 == 0 | p2 > p Then Do
    If p2 == 0 Then len = Length( eSource ) - p + 1
    Else            len = p2 - p
    expo = HTMLClass[ .EL.EXPONENT ]
    Call Highlight .EL.EXPONENT, "", "", tag expo, SubStr( eSource, p, len )
    p = p2
  End

  -- Final whitespace (in the case of strings)
  If p \== 0 Then Call Highlight cat, subCat, "", tag, SubStr( eSource, p )

NumberDone:
  -- Highligth the closing quote and suffix, if any
  If q \== "" Then Do
    Call Highlight .EL.STRING_CLOSING_DELIMITER, "", "", close, q
    If hasSuffix Then
      Call Highlight .EL.STRING_SUFFIX,          "", "", string_suffix_tag, suffix
  End

Exit

ComputeSourceCatsOpenCloseAndSuffix:

  eSource               = element~source
  q                     = eSource[1]
  If q \== "'", q \== '"' Then q = ""
  cat                   = element~category
  If element < constant Then subcat = element~subCategory
  Else                       subcat = ""
  hasSuffix             =  q \== "" & eSource~right(1) \== q

  If q == "" Then Return

  If element < constant Then
    open  = HTMLClass[ constant, subcat, .EL.STRING_OPENING_DELIMITER ]
  Else
    open  = HTMLClass[ cat, .EL.STRING_OPENING_DELIMITER ]

  If element < constant Then
    close = HTMLClass[ constant, subcat, .EL.STRING_CLOSING_DELIMITER ]
  Else
    close = HTMLClass[ cat, .EL.STRING_CLOSING_DELIMITER ]

  If hasSuffix Then Do
    If element < constant Then
      string_suffix_tag = HTMLClass[ constant, subcat, .EL.STRING_SUFFIX ]
    Else
      string_suffix_tag = HTMLClass[ cat, .EL.STRING_SUFFIX ]
    suffix              = Right( eSource, 1 )
  End

Return

--------------------------------------------------------------------------------
-- STRINGS                                                                    --
--------------------------------------------------------------------------------

String:
  If element~isMultiLine Then Signal MultiLineString

  Call ComputeSourceCatsOpenCloseAndSuffix

  If element < constant Then tag = HTMLClass[ constant, subCat ]
  Else                       tag = HTMLClass[ cat ]

  If hasSuffix Then eSource = SubStr( eSource, 2, Length(eSource) - 3 )
  Else              eSource = SubStr( eSource, 2, Length(eSource) - 2 )

  Call Highlight .EL.STRING_OPENING_DELIMITER, "",     "", open,  q
  Call Highlight cat,                          subcat, "", tag,   eSource
  Call Highlight .EL.STRING_CLOSING_DELIMITER, "",     "", close, q
  If hasSuffix Then
    Call Highlight .EL.STRING_SUFFIX,          "",     "", string_suffix_tag, suffix

Exit

--------------------------------------------------------------------------------
-- MULTI-LINE STRINGS (CMS)                                                   --
--------------------------------------------------------------------------------
-- A string whose opening and closing quotes are on different lines. The
-- source layout is reproduced faithfully: opening quote at the start of
-- the first line, closing quote (and suffix, if any) at the end of the
-- last one, each line's contents in between, with a line break wherever
-- the source had one. The string "value" has no line breaks, but its
-- rendering does, mirroring the original program.

MultiLineString:
  Call ComputeSourceCatsOpenCloseAndSuffix

  If element < constant Then tag = HTMLClass[ constant, subCat ]
  Else                       tag = HTMLClass[ cat ]

  lines = element~sourceLines
  last  = lines~items

  Do Counter c lineSource Over lines
    If c > 1 Then Do
      currentline += 1
      Call NewLine currentLine + options.startFrom - 1
    End
    line = lineSource
    -- Strip the opening quote off the first line...
    If c == 1     Then line = SubStr( line, 2 )
    -- ...and the closing quote (plus suffix) off the last one.
    If c == last  Then Do
      If hasSuffix Then line = Left( line, Length(line) - 2 )
      Else              line = Left( line, Length(line) - 1 )
    End
    -- Opening delimiter (first line only)
    If c == 1     Then Call Highlight .EL.STRING_OPENING_DELIMITER, "", "", open, q
    -- This line's contents
    Call Highlight cat, subcat, "", tag, line
    -- Closing delimiter and suffix (last line only)
    If c == last  Then Do
      Call Highlight .EL.STRING_CLOSING_DELIMITER, "", "", close, q
      If hasSuffix Then
        Call Highlight .EL.STRING_SUFFIX,          "", "", string_suffix_tag, suffix
    End
  End

Exit

--------------------------------------------------------------------------------
-- COMPOUND VARIABLES                                                         --
--------------------------------------------------------------------------------

CompoundVariable:
  Do part Over element~parts
    tag = HTMLClass[ part~category ]
    Call Highlight part~category, "", "", tag, part~source
  End
Exit

--------------------------------------------------------------------------------
-- DOC-COMMENTS                                                               --
--------------------------------------------------------------------------------

DocComment:
  padding = options.pad
  lineNo = ""
  lineLength = 0
  elCat = element~category
  Do Counter partNo part Over element~parts
    newlineNo = part[1]
    If lineNo \== newLineNo Then Do
      If lineNo \== "" Then Do
        currentline += 1
        displayLine = currentLine + options.startFrom - 1
        Call NewLine displayLine
        lineLength = 0
      End
      lineNo = newLineNo
    End
    cat    = part[2]
    string = part[3]
    lineLength += Length( string )
    tag = HTMLClass[ elCat, cat ]
    Call Highlight elementCat elCat cat, "", "", tag, string
    If padding > 0, lineLength < padding Then Do
      If partNo == element~parts~items Then pad = 1
      Else If partNo < element~parts~items, newLineNo < element~parts[partNo+1][1] Then pad = 1
      Else pad = 0
      If pad Then Do
        cat = .EL.DOC_COMMENT_WHITESPACE
        tag = HTMLClass[ elCat, cat ]
        Call Highlight elementCat elCat cat, "", "", tag, Copies(" ", padding - lineLength )
      End
    End
  End
  currentline += 1
  displayLine = currentLine + options.startFrom - 1
  Call NewLine displayLine
Exit
