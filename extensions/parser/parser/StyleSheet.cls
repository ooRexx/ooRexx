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
/* StyleSheet.cls - (Very limited) stylesheet support                         */
/* ==================================================                         */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Abstract                                                                   */
/* --------                                                                   */
/*                                                                            */
/* HTML highlighting uses a set of CSS stylesheets, and we want to reuse      */
/* these stylesheets with other highlighter drivers, like ANSI or LaTeX.      */
/* To that purpose, we must be able to parse the CSS stylesheet and abstract  */
/* its contents, so that the other drivers can use it. This package contains  */
/* the necessary routines to implement this cross-driver support.             */
/*                                                                            */
/* Generally speaking, CSS parsing is a formidable task. We only support      */
/* rigidly formatted stylesheets, that are enough to produce excellent        */
/* HTML results and at the same time simple enough to be manually parser      */
/* and used by the ANSI and LaTeX drivers.                                    */
/*                                                                            */
/* Where are stylesheets located                                              */
/* -----------------------------                                              */
/*                                                                            */
/* Stylesheets reside in the /css directory. Given a style name <style>,      */
/* the corresponding stylesheet file is "/css/rexx-<style>.css".              */
/* A style of the author's own lives in a file of theirs: see LocalStyle.     */
/*                                                                            */
/* Format of a stylesheet file                                                */
/* ---------------------------                                                */
/*                                                                            */
/* To our parser, a stylesheet is simply a list of (possibly nested) rules.   */
/* Rules that start with ".highlight-rexx-<style>" are recognized and         */
/* processed; all other rules are ignored.                                    */
/*                                                                            */
/* ### Flattening                                                             */
/*                                                                            */
/* The parser first flattens the recognized rules in several simple ways:     */
/*                                                                            */
/*   selector1, ..., selectorn {declaration block}                            */
/*                                                                            */
/* is flattened to                                                            */
/*                                                                            */
/*   selector1 {declaration block}                                            */
/*   ...                                                                      */
/*   selectorn {declaration block}                                            */
/*                                                                            */
/* Similarly,                                                                 */
/*                                                                            */
/*   selector {declaration1; ...; declarationn}                               */
/*                                                                            */
/* is flattened to                                                            */
/*                                                                            */
/*   selector declaration1;                                                   */
/*   ...                                                                      */
/*   selector declarationn;                                                   */
/*                                                                            */
/*  ### Undoing nesting                                                       */
/*                                                                            */
/*  The flattening process eliminates nesting:                                */
/*                                                                            */
/*    .class1 {.class2 rules}                                                 */
/*                                                                            */
/*  flattens to                                                               */
/*                                                                            */
/*    .class1 .class2 rules                                                   */
/*                                                                            */
/*  while                                                                     */
/*                                                                            */
/*    .class1 {&.class2 rules}                                                */
/*                                                                            */
/*  flattens to                                                               */
/*                                                                            */
/*    .class1.class2 rules                                                    */
/*                                                                            */
/*  Other CSS combinators like +, > or ~ are not recognized.                  */
/*                                                                            */
/*  ### Format of the flattened styles                                        */
/*                                                                            */
/*  Flattening is recursive, so that, in the end, we are left up with a       */
/*  list of triples in the format                                             */
/*                                                                            */
/*    (selector, property, value)                                             */
/*                                                                            */
/*  which is excellent to implement the specificity and order algorithms.     */
/*                                                                            */
/* Limitations:                                                               */
/* ------------                                                               */
/*                                                                            */
/* + We only recognize and process rules where selectors consist              */
/*   of class names.                                                          */
/* + A selector must be composed of exactly two words. The first word must be */
/*   ".highlight-rexx-<style>", as described above. The second word must be   */
/*   an abutted list of classes starting with a period,                       */
/*   ".class1.class2...classn". Rules starting with other selectors           */
/*   are ignored.                                                             */
/* + Comment-like constructions inside strings are not supported              */
/* + Braces "{" and "}" inside strings are not supported                      */
/* + We cannot process function line constructs that use commas.              */
/* + For 'font-style', we only support 'italic' and 'normal'.                 */
/* + For 'font-weight', we only support 'bold' and 'normal'.                  */
/* + For 'text-decoration', we only support 'underline', 'none', and a dotted */
/*   underline ('underline dotted', with or without a colour), which is "D". */
/* + For 'color' and 'background-color', we only support #rgb, #rgba,         */
/*   #rrggbb, #rrggbbaa, or a named color.                                    */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20250623    0.2d First version                                             */
/* 20261022         LocalStyle: a style in a file of the author's own;        */
/*                  FlattenedCSS, for md2pdf and md2epub (Rony, 21-Oct)       */
/* 20261031         A dotted underline, "D" (the directives, Rony, 31-Oct)    */
/* 20261121         A style name is looked for in css/, next to this file     */
/*                  (bin/parser/css/, for an installation), and then in       */
/*                  ../../css/, as before                                     */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261122         A style is a file only when it has a "/" or a "\", or     */
/*                  ends in ".css", as in LocalStyle ("my.x" is a name). A    */
/*                  name is looked for in parser/css/, next to us, and then   */
/*                  in the directories of REXX_PARSER_STYLE_PATH, in order    */
/*                  (RexxPub puts its css/ there); no more ../../css/. A      */
/*                  style that is not found is SYNTAX 98.900, which says      */
/*                  where it was looked for, not a HALT. StyleFile: the file  */
/*                  of a style (highlight --css puts it in the page)          */
/*                                                                            */
/******************************************************************************/

-- The styles of the author's own that LocalStyle has seen
.context~package~local["CSS.LOCAL.NAMES"] = .Set~new

::Requires "HTMLColors.cls"

/******************************************************************************/
/* GETHIGHLIGHT                                                               */
/******************************************************************************/

--- The file of a style. A style is a file when it has a "/" or a "\", or
--- ends in ".css" (as in LocalStyle). Otherwise it is a name: a style of
--- the author's own (LocalStyle) is found where it was said; the others,
--- as "rexx-NAME.css" in parser/css/, next to us, and then in each
--- directory of REXX_PARSER_STYLE_PATH, in order (RexxPub puts its css/
--- there, with the styles that do not come with ooRexx). The shipped
--- styles come first: a copy of one elsewhere does not hide it.
---
--- @param style  A style name, like "dark", or the name of a file.
--- @return       The full name of the file. A style that is not found is
---               SYNTAX 98.900, which says where it was looked for.
---

::Routine StyleFile Public

  Use Strict Arg style

  If Pos("/", style) + Pos("\", style) > 0 | -
    style~caselessEndsWith(".css") Then Do
    exists = Stream( style, "C", "Q Exists" )
    If exists == "" Then Raise Syntax 98.900 -
      Array( "The style file '"style"' does not exist" )
    Return exists
  End

  file = .context~package~local["CSS.FILE."style]
  If file \== .Nil Then Do
    exists = Stream( file, "C", "Q Exists" )
    If exists == "" Then Raise Syntax 98.900 -
      Array( "Style '"style"' not found (looked for" file")" )
    Return exists
  End

  dirs  = StyleDirectories()
  where = ""
  Do dir Over dirs
    exists = Stream( dir"rexx-"style".css", "C", "Q Exists" )
    If exists \== "" Then Return exists
    where ||= ", "dir
  End
  Raise Syntax 98.900 Array( "Style '"style"' not found (looked for" -
    "rexx-"style".css in" SubStr(where, 3)")" )

--- Gets the highlighting associated to a style and a set of classes.
---
--- @param style      A Highlighter style, like 'dark' or 'light'.
--- @param myClasses  A blank-separated list of Highlighter classes,
---                   like "rx-const rx-method" (note: no initial periods).
--- @return           A string "b i u f:g", where b can be "B" (bold), "b"
---                   (no bold), or "-" (not specified), and similarly For
---                   i and u, and f and g can be the null string of a
---                   color in the format rrggbbaa.
---

::Routine GetHighlight Public

  Use Strict Arg style, myClasses

  file = StyleFile( style )
  If Pos("/", style) + Pos("\", style) > 0 | -
    style~caselessEndsWith(".css") Then Do
    Parse Value Reverse(style) With "ssc."elyts"-xxer"
    style = Reverse(elyts)
  End

  -- Sort the classes to get a unique key to cache the results
  myClasses = myClasses~makeArray(" ")~sort~makeString("L"," ")

  -- Prepare the cache
  pkgLocal   = .context~package~local
  key        = "CSS.STYLESHEET."style"."myclasses

  -- Check the cache
  highlight = pkgLocal[ key ]

  -- Cached? Return the cached value
  If highlight \== .Nil Then Return highlight

  -- Load and preprocess the stylesheet (it may be cached)
  style = LoadStylesheet( style, file )

  -- A (very) poor's man cascading algorithm
  --
  -- We only have to check specificity and order, and that in a single
  -- style sheet.
  --
  -- Specificity amounts to the number of classes present as selectors,
  -- as we don't process other forms of selectors.
  --
  -- Flattening will have created a different record for every property,
  -- and anyway different properties can appear in different rules.
  --
  specificity. = 0
  value.       = ""

  Do rule over style
    classes  = rule[1]
    property = rule[2]
    value    = rule[3]

    -- We first check all the classes in the rule and ensure that
    -- they are specified
    Do class Over classes~makeArray(" ")
      If WordPos(class, myClasses) == 0 Then Iterate rule
    End

    -- Mini specificity algorithm: we associate a different
    -- specificity value to each property, and update it (and the
    -- property value) only when the specificity is:
    --
    --   + Larger (more specificity), or
    --   + The same (then the order wins).
    --
    oldSpecificity = specificity.property
    If oldSpecificity <= Words(classes) Then Do
      specificity.property = Words(classes)
      value.property = value
    End
  End

  -- We now pick all the collected values, and store
  -- them in a string
  color        =  value.["color"]
  background   =  value.["background-color"]
  bold         =  value.["bold"]
  If bold      == "" Then bold = "-"
  italic       =  value.["italic"]
  If italic    == "" Then italic = "-"
  underline    =  value.["underline"]
  If underline == "" Then underline = "-"

  highlight = bold italic underline color":"background

  -- Cache these results
  pkgLocal[ key ] = highlight

  -- And return them
  Return highlight

/******************************************************************************/
/* LOADSTYLESHEET                                                             */
/******************************************************************************/

---
--- Loads and returns a flattened version of (part of) a stylesheet.
---
--- @param style      A Highlighter style, like 'dark' or 'light'.
---                   File "..\css\rexx-"style".css" is read and processed,
---                   and its contents are cached.
--- @return           A flattened array of styles, respecting the order
---                   found in the original stylesheet. Every element E
---                   contains three items: E[1] is a blank-separated list
---                   of Highlighter classes, like "rx-const rx-method"
---                   (note that we don't use initial periods). E[2] is
---                   one of "bold", "italic", "underline", "color" or
---                   "background-color". E[3] is either "B" (bold) or "b"
---                   (normal) when E[2] is "bold", and similarly for
---                   "italic" and "underline". When E[2] is "color" or
---                   "background-color", E[3] has the form "rrggbbaa".
---

::Routine LoadStylesheet

  Use Strict Arg style, file

  -- Prepare the cache
  pkgLocal = .context~package~local
  key      = "CSS.STYLESHEET."file

  -- Check the cache

  theStyle = pkgLocal[ key ]

  -- Cached? Return the cached version

  If theStyle \== .Nil Then Return theStyle

  -- Read the whole file into an array and close the file

  css = Charin( file,1,Chars(file) )~makeArray
  Call Stream file, "C", "Close"

  -- Now we will process the stylesheet. This array will hold our results
  unflattened = .Array~new

  -- We first transform the array into a string.
  -- Line ends are substituted by blanks

  css = css~makeString("Line"," ")

  -- Substitute double spaces by single spaces. This is not acceptable
  -- in a general CSS parser, but it's OK for our purposes.

  css = Space( css )

  -- Delete all comments
  --
  -- LIMITATION: We don't accept comment-like constructs inside strings.
  --
  -- Given that limitation, and since CSS comments can't be nested,
  -- it's safe to substitute them by a single blank.

  Loop While Pos("/*", css) > 0
    Parse Var css before "/*" . "*/" after
    css = before after
  End

  -- Now parse blocks delimited by braces

  open  = Pos( "{", css )
  Do While open > 0
    Call ParseBlock
    open  = Pos( "{", css )
  End

  -- Flatten everything

  newList = .Array~new
  Call Flatten "", unflattened, newList

  -- Delete cases that don't concern us

  Do i = 1 To newList~items
    classes = newList[i][1]
    Parse Value classes With check rest

    -- The first class must be ".highlight-rexx-"style

    If check \== ".highlight-rexx-"style Then Do
      newList~remove(i)
      Iterate
    End

    -- After a descendant combinator, we only accept compound
    -- selectors. Please note that the uses of the "&" nested selector
    -- will have already been flattened at this point.

    If Words(rest) \== 1 | Strip(rest)[1] \== "." Then Do
      newList~remove(i)
      Iterate
    End

    -- We now process (property, value) pairs.
    -- We only accept a very limited subset of CSS.

    Parse Value Lower(Space(newList[i][2],0)) With property":"value
    Select Case property
      When "font-style" Then Do
        property = "italic"
        Select Case value
          When "italic"    Then value = "I"
          When "normal"    Then value = "i"
          Otherwise
            newList~remove(i)
            Iterate -- We don't support oblique
        End
      End
      When "font-weight" Then Do
        property = "bold"
        Select Case value
          When "bold"      Then value = "B"
          When "normal"    Then value = "b"
          Otherwise
            newList~remove(i)
            Iterate -- We don't support other variants
        End
      End
      When "text-decoration" Then Do
        property = "underline"
        Select
          -- 'underline dotted #RRGGBBAA': the blanks are gone, and its colour
          -- is the token's, half drawn, wherever it is drawn
          When value~startsWith("underline"), value~pos("dotted") > 0 Then
            value = "D"
          When value == "underline" Then value = "U"
          When value == "none"      Then value = "u"
          Otherwise
            newList~remove(i)
            Iterate -- We don't support other variants
        End
      End
      When "color", "background-color" Then Do
        If value[1] == "#", DataType(SubStr(value,2),"X") Then Do
          f  = "f"
          ff = "ff"
          Select Case Length(value)
            When 4 Then Do
              Parse Var value +1 r +1 g +1 b
              value = Space(r r g g b b f f, 0)
            End
            When 5 Then Do
              Parse Var value +1 r +1 g +1 b +1 a
              value = Space(r r g g b b a a , 0)
            End
            When 7 Then Do
              Parse Var value +1 r +2 g +2 b
              value = Space(r g b ff, 0)
            End
            When 9 Then Do
              Parse Var value +1 r +2 g +2 b +2 a
              value = Space(r g b a, 0)
            End
            Otherwise
              newList~remove(i)
              Iterate -- Not supported
          End
        End
        Else Do -- At this point, we only support a named color
          value = .HTML.Color[value]
          If value == .Nil Then Do
            newList~remove(i)
            Iterate
          End
          value = value"ff"
        End
      End
      Otherwise Iterate
    End
    newList[i] = (Strip( ChangeStr(".", rest, " ") ), property, value)
  End

  -- Make the array dense (i.e., non-sparse) again

  newList = newList~makeArray

  -- Cache these results

  pkgLocal[ key ] = newList

  -- And return them

  Return newList

/******************************************************************************/
/* PARSEBLOCK                                                                 */
/*                                                                            */
/* We are at the beginning of a block, i.e., we just scanned a "{" character. */
/******************************************************************************/

ParseBlock: Procedure Expose css unflattened

  ------------------------------------------------------------------------------
  -- Allow constructs like
  --   {
  --     color:blue;
  --     whatever {
  --       nested stuff
  --     }
  --   }
  ------------------------------------------------------------------------------
  Parse Var css defined"{"css

  Do While Pos(";", defined) > 0
    Parse Var defined definition";"defined
    unflattened~append( Strip(definition) )
  End

  ------------------------------------------------------------------------------
  -- Pick what's inside the braces. This has to be recursive, because
  -- we can have braces inside braces.
  ------------------------------------------------------------------------------
  definition = Definition()

  ------------------------------------------------------------------------------
  --
  -- Implement a first form of flattening, by recognizing
  --    a, b, c {whatever}
  -- and expanding it to
  --   a {whatever}
  --   b {whatever}
  --   c {whatever}
  --
  -- LIMITATION: We can't parse function-like constructs that use commas
  ------------------------------------------------------------------------------

  selectors = defined
  Do While Pos(",", selectors) > 0
    Parse Var selectors selector"," selectors
    unflattened~append( (Strip(selector), definition) )
  End

  ------------------------------------------------------------------------------
  -- Store the last definition
  ------------------------------------------------------------------------------

  selector = selectors
  unflattened~append( (Strip(selector), definition) )

Return

--------------------------------------------------------------------------------
-- DEFINITION                                                                 --
--------------------------------------------------------------------------------

Definition: Procedure Expose css

  open  = Pos( "{", css )
  close = Pos( "}", css )

  If (open > 0 & close < open) | (open == 0 & close > 0) Then Do
    Parse Var css res "}" css
    Return Strip(res) -- A string
  End

  unflattened = .Array~new    -- An array
  Do While open > 0, (close == 0 | open < close )

    Call ParseBlock  -- This will add to the "unflattened" array

    open  = Pos("{", css)
    close = Pos("}", css)
  End

  If close == 0 Then Call HALT "Missing '}' in CSS."

  Parse Var css last "}" css
  If last \= "" Then unflattened~append( Strip(last) )

Return unflattened

/******************************************************************************/
/* FLATTEN                                                                    */
/******************************************************************************/

---
--- Flatten a list of rules, given a selector.
---
--- @param selector Outer selector, to be combined with the inner selectors
---                 (nesting).
--- @param rules    The list of rules to flatten.
--- @param newList  A (not necessarily empty) list to accumulate the flattened
---                 rules.
---

::Routine Flatten

  Use Arg selector, rules, newList

  --- Step over the whole list of rules
  Do rule Over rules

    --- Simple flattening: "rule" was a declaration. Just store it.

    If rule~isA(.String) Then Do
      newList~append( (selector, rule ) )
      Iterate
    End

    -- Rule is an array. Rule[1] will contain classes (with some
    -- optional operators), and Rule[2] will be a list of declarations.

    Do declarationList Over rule[2]

      -- Create a new selector
      newSelector = NewSelector(selector, rule[1])

      -- When declarationList is a string (i.e., a real declaration list),
      -- we simply append each declaration to the new list, and we are done.

      If declarationList~isA(.String) Then Do
        Call AppendAll newSelector, declarationList, newList
        Iterate
      End

      -- Now declarationList itself is an array:
      --   the first element is the (nested) selector,...

      nestedSelector = declarationList[1]
      newSelector = NewSelector(newSelector, nestedSelector)

      ---   ...and all other elements are the declarations

      Do i = 2 To declarationList~items
        declarations = declarationList[i]

        -- If declarations is an array, we should flatten it

        If declarations~isA(.Array) Then
          Call Flatten   newSelector, declarations, newList
        Else
          Call AppendAll newSelector, declarations, newList
      End

    End

  End
  Return

NewSelector: Procedure
  Use Arg selector1, selector2
  If selector2[1] == "&" Then Return Strip( selector1 || SubStr(selector2,2) )
  Return Strip( selector1 selector2 )

AppendAll: Procedure
  Use Arg selector, declarations, list
  Do declaration Over Declarations(declarations)
    list~append( (selector, declaration ) )
  End
Return

Declarations: Procedure
  Use Arg list
  declarations = .Array~new

  If Right(list,1) == ";" Then list = Left(list,Length(list)-1)

  Do While Pos(";", list) > 0
    Parse Var list declaration";"list
    declarations~append( Strip(declaration) )
  End

  declaration = list
  declarations~append( Strip(declaration) )
Return declarations

/******************************************************************************/
/* LOCALSTYLE                                                                 */
/******************************************************************************/

--- A style of the author's own, in a file of theirs (Rony, 21-Oct-2026: a
--- variant of vim-light-zellner next to his book.md). "style:" in the YAML
--- (or --style) names a shipped style, "vim-light-zellner", or a file:
--- anything with a "/" or a "\", or ending in ".css". A relative path is
--- relative to baseDir (the directory of the .md, or the current one for
--- --style). The style's name is the one its rules carry, the first
--- ".highlight-rexx-NAME" in the file, so a copy of a shipped sheet can be
--- edited and used as it is, without renaming anything. From then on NAME
--- is found in that file (GetHighlight), also when a shipped style has the
--- same name: the author's wins.
---
--- @param value    What "style:" says.
--- @param baseDir  Where a relative path starts.
--- @return         .Nil when value is a name; an Array (name, file) when
---                 it is a file with a style; otherwise, a String that
---                 says what is wrong.
---

::Routine LocalStyle Public

  Use Strict Arg value, baseDir

  value = Strip(value)
  If Pos("/", value) + Pos("\", value) == 0,                                -
    \value~caselessEndsWith(".css") Then Return .Nil

  file = value
  If file[1] \== "/", file[1] \== "\", SubStr(file, 2, 1) \== ":" Then
    file = Strip(Strip(baseDir, "T", "/"), "T", "\")"/"file
  fileObj = .File~new(file)
  If \fileObj~isFile Then Return "there is no such file ("file")"
  file = fileObj~absolutePath

  css = CharIn(file, 1, Chars(file))
  Call Stream file, "C", "Close"
  css = StrippedCSSComments(css)

  marker = ".highlight-rexx-"
  p = Pos(marker, css)
  If p == 0 Then Return "it has no rule for a '"marker"NAME' class"
  allowed = XRange("ALNUM")"-_"
  name = SubStr(css, p + Length(marker))
  e = Verify(name, allowed)
  If e > 0 Then name = Left(name, e - 1)
  If name == "" Then Return "it has no rule for a '"marker"NAME' class"

  -- What GetHighlight knew of a style with that name is no longer true
  local = .context~package~local
  Do key Over local~allIndexes
    If key~startsWith("CSS.STYLESHEET."name".") Then local~remove(key)
  End
  local["CSS.FILE."name] = file
  local["CSS.LOCAL.NAMES"]~put(name)
  Return .Array~of(name, file)

--- The file of a style of the author's own (LocalStyle), or .Nil.
::Routine LocalStyleFile Public
  Use Strict Arg name
  Return .context~package~local["CSS.FILE."name]

--- The names of the styles of the author's own (LocalStyle), as an Array.
::Routine LocalStyleNames Public
  Return .context~package~local["CSS.LOCAL.NAMES"]~allIndexes

--- The styles of the author's own are forgotten (md2html, md2pdf and md2epub
--- do one .md after another: a file's local style is not the next one's).
::Routine ForgetLocalStyles Public
  local = .context~package~local
  Do name Over local["CSS.LOCAL.NAMES"]
    file = local["CSS.FILE."name]
    local~remove("CSS.FILE."name)
    -- What GetHighlight learned from that file goes with it
    Do key Over local~allIndexes
      If key~startsWith("CSS.STYLESHEET."name".") Then local~remove(key)
    End
    If file \== .Nil Then local~remove("CSS.STYLESHEET."file)
  End
  local["CSS.LOCAL.NAMES"] = .Set~new
  Return

/******************************************************************************/
/* FLATTENEDCSS                                                               */
/******************************************************************************/

--- A nested stylesheet, flattened. paged.js (md2pdf) and the EPUB readers
--- (md2epub) do not understand CSS nesting, so they get the sheets in
--- css/flattened/; a sheet of the author's own (LocalStyle) is flattened
--- here, when it is used.
---
---   .a { color: red; .b, &.c { x: y } }
---
--- becomes ".a { color: red; }", ".a .b, .a.c { x: y; }". A selector list
--- is expanded selector by selector; "&" is the outer selector, and a
--- selector without "&" is a descendant of it. Declarations keep their
--- order: those after a nested rule go in a rule of their own, after it.
--- @media, @supports, @container and @layer keep their place and get
--- their contents flattened; other at-rules (@font-face, @page,
--- @keyframes...) are copied as they are. Comments are left out.
---
--- @param css  The text of the sheet.
--- @return     The text of the flattened sheet.
---

::Routine FlattenedCSS Public

  Use Strict Arg css

  s = .Directory~new
  s~text = StrippedCSSComments(css)
  s~pos  = 1
  items  = CSSBlock(s)

  out = .MutableBuffer~new
  Call CSSEmit items, .Array~new, out
  Return out~string

-- The text, without its comments (not inside strings)
::Routine StrippedCSSComments Public
  Use Strict Arg css
  out = .MutableBuffer~new
  i = 1; n = Length(css)
  Do While i <= n
    c = css[i]
    If c == '"' | c == "'" Then Do
      j = i + 1
      Do While j <= n, css[j] \== c
        If css[j] == "\" Then j = j + 1
        j = j + 1
      End
      out~append(SubStr(css, i, j - i + 1))
      i = j + 1
      Iterate
    End
    If c == "/", SubStr(css, i + 1, 1) == "*" Then Do
      j = Pos("*/", css, i + 2)
      If j == 0 Then Leave
      out~append(" ")
      i = j + 2
      Iterate
    End
    out~append(c)
    i = i + 1
  End
  Return out~string

-- The items of a block, up to its "}" (or the end): declarations (Strings)
-- and rules (Arrays: the selector or at-rule's prelude, and its items).
-- At-rules without a block (@import, @charset) are kept as declarations.
::Routine CSSBlock
  Use Strict Arg s
  text = s~text; n = Length(text)
  items = .Array~new
  buf = .MutableBuffer~new
  depth = 0                                   -- parentheses
  i = s~pos
  Do While i <= n
    c = text[i]
    Select
      When c == '"' | c == "'" Then Do
        j = i + 1
        Do While j <= n, text[j] \== c
          If text[j] == "\" Then j = j + 1
          j = j + 1
        End
        buf~append(SubStr(text, i, j - i + 1))
        i = j + 1
        Iterate
      End
      When c == "(" Then depth = depth + 1
      When c == ")" Then depth = Max(0, depth - 1)
      When c == ";", depth == 0 Then Do
        decl = Space(Translate(buf~string, "  ", "0a0d"x))
        If decl \== "" Then items~append(decl)
        buf = .MutableBuffer~new
        i = i + 1
        Iterate
      End
      When c == "{", depth == 0 Then Do
        prelude = Space(Translate(buf~string, "   ", "0a0d09"x))
        buf = .MutableBuffer~new
        s~pos = i + 1
        inner = CSSBlock(s)
        items~append(.Array~of(prelude, inner))
        i = s~pos
        Iterate
      End
      When c == "}", depth == 0 Then Do
        decl = Space(Translate(buf~string, "  ", "0a0d"x))
        If decl \== "" Then items~append(decl)
        s~pos = i + 1
        Return items
      End
      Otherwise Nop
    End
    buf~append(c)
    i = i + 1
  End
  decl = Space(Translate(buf~string, "  ", "0a0d"x))
  If decl \== "" Then items~append(decl)
  s~pos = n + 1
  Return items

-- The flat rules of a block's items, inside the given selectors
::Routine CSSEmit
  Use Strict Arg items, parents, out
  run = .Array~new
  Do item Over items
    If item~isA(.String) Then Do
      run~append(item)
      Iterate
    End
    Call CSSDeclarations run, parents, out
    run = .Array~new
    prelude = item[1]
    If prelude[1] == "@" Then Do
      Select Case Lower(Word(prelude, 1))
        When "@media", "@supports", "@container", "@layer" Then Do
          out~append(prelude" {"||"0a"x)
          Call CSSEmit item[2], parents, out
          out~append("}"||"0a"x)
        End
        Otherwise
          out~append(prelude" {"||"0a"x, CSSVerbatim(item[2], "  "), "}"||"0a"x)
      End
      Iterate
    End
    Call CSSEmit item[2], CSSSelectors(parents, prelude), out
  End
  Call CSSDeclarations run, parents, out
  Return

-- A rule for a run of declarations
::Routine CSSDeclarations
  Use Strict Arg run, parents, out
  If run~isEmpty Then Return
  If parents~isEmpty Then Do                  -- outside any rule: as is
    Do decl Over run
      out~append(decl";"||"0a"x)
    End
    Return
  End
  out~append(parents~makeString("L", ", ")" {"||"0a"x)
  Do decl Over run
    out~append("  "decl";"||"0a"x)
  End
  out~append("}"||"0a"x)
  Return

-- The selectors of a nested rule: each of its own, inside each outer one
::Routine CSSSelectors
  Use Strict Arg parents, prelude
  own = .Array~new
  buf = ""; depth = 0
  Do i = 1 To Length(prelude)
    c = prelude[i]
    If c == "(" Then depth = depth + 1
    If c == ")" Then depth = depth - 1
    If c == ",", depth == 0 Then Do
      own~append(Strip(buf)); buf = ""
      Iterate
    End
    buf = buf || c
  End
  own~append(Strip(buf))
  If parents~isEmpty Then Return own
  all = .Array~new
  Do p Over parents
    Do sel Over own
      If Pos("&", sel) > 0 Then all~append(ChangeStr("&", sel, p))
      Else                      all~append(p sel)
    End
  End
  Return all

-- An at-rule's block, as it was
::Routine CSSVerbatim
  Use Strict Arg items, indent
  out = .MutableBuffer~new
  Do item Over items
    If item~isA(.String) Then out~append(indent || item";"||"0a"x)
    Else out~append(indent || item[1]" {"||"0a"x,                           -
      CSSVerbatim(item[2], indent"  "), indent"}"||"0a"x)
  End
  Return out~string

/******************************************************************************/
/* Utility routines                                                           */
/******************************************************************************/

-- StyleDirectories - where a style name is looked for: parser/css/, next to
-- us, and then the directories of REXX_PARSER_STYLE_PATH (separated by ";" on
-- Windows and ":" elsewhere, as the PATH), each one ending in a separator.

::Routine StyleDirectories
  sl = .File~separator
  If sl == "\" Then pathSep = ";"
               Else pathSep = ":"
  Parse Source . . thisPgm
  dirs = .Array~of( FileSpec("Location", thisPgm)"css"sl )
  list = Value("REXX_PARSER_STYLE_PATH", , "ENVIRONMENT")
  Do While list \== ""
    Parse Var list dir (pathSep) list
    dir = Strip(dir)
    If dir == "" Then Iterate
    If \dir~endsWith(sl), \dir~endsWith("/") Then dir ||= sl
    dirs~append( dir )
  End
  Return dirs
