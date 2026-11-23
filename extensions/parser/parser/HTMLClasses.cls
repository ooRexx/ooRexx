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
/* HTMLClasses.cls - element category/subcategory to html class translation   */
/* ========================================================================   */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241206    0.1  First public release                                      */
/* 20241208    0.1a c/CLASSIC_COMMENT/STANDARD_COMMENT/                       */
/* 20241209    0.1b Add shebang support                                       */
/*                  New call system                                           */
/* 20241225    0.1d Add doc-comment support                                   */
/* 20250103    0.1e Add TUTOR-flavored Unicode classes                        */
/* 20250116    0.1f Add support for EL.SUBKEYWORD                             */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250406         Rename fractional numbers to decimal                      */
/* 20250528    0.2c Add support for detailed string highlighting              */
/* 20250630    0.2d Add support for doc-comment subelements                   */
/* 20250722         Add "dskw" for .EL.DIRECTIVE_SUBKEYWORD                   */
/* 20251213         Add support for curly brackets and argument names         */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261123         No TABLE.FIELD.NAME: no table constructors                */
/*                                                                            */
/******************************************************************************/

::Routine HTMLClasses Public

--
-- ARGUMENT
-- --------
--
--   "Options.", a stem.
--

  Use Strict Arg Options.

--
-- The following tails are examined:
--
-- * Options.assignment,
--     for (extended) assignment operator characters and character sequences.
-- * Options.constant,
--     for strings or symbols taken as a constant.
-- * Options.operator,
--     for operator characters and operator character sequences.
-- * Options.special,
--     for special characters and special character sequences.
-- * Options.classprefix,
--     a prefix prepended to every HTML class. Default is "".
--
-- The code assigns one or two HTML classes to every element category, and,
-- in the case of symbols taken as a constant, to every element subcategory.
--
-- When two classes are assigned, the first one is generic (for instance,
-- "spe" for special characters), and the second one identifies the
-- corresponding element (for example, "comma", or "colon").
--
-- The value of the corresponding compound variables determine the class
-- associated with every element category and subcategory,
-- in the following way:
--
-- * When the value is "group", only the generic class is assigned.
-- * When the value is "detail", only the detailed class is assigned.
-- * When the value is "full", two classes are assigned, the generic
--   one and the detailed one (in this order).
--
-- RETURNS
-- -------
--
-- A stem ("HTMLClass") mapping element categories and subcategories to html
-- classes. The default HTML class is "rexx".
--

  prefix      = options.classprefix

  HTMLClass   = .Stem~new  --
  HTMLClass[] = "rexx"     -- The default HTML class is 'rexx'

--
-- We will assign HTML classes to a number of element categories
-- using the "Assign" internal routine.
--
-- We include it here and skip over its code for readability.
--
--------------------------------------------------------------------------------

  Signal SkipOverAssign

Assign:
  value = ""                            -- An accumulator
  classes = Arg(2)~makeArray(" ")
  Do Counter c class Over classes
    If c > 1 Then value ||= " "
    value ||= prefix || class           -- Prepend the prefix to each class
  End
  HTMLClass[ Arg(1) ] = value
Return

SkipOverAssign:

/******************************************************************************/
/* SHEBANGS                                                                   */
/******************************************************************************/

  Call Assign   .EL.SHEBANG,                     "shb"

/******************************************************************************/
/* SYMBOLS                                                                    */
/******************************************************************************/

--------------------------------------------------------------------------------
-- Keywords                                                                   --
--------------------------------------------------------------------------------
--
-- Directive keywords and subkeywordshave a special element category, different
-- from instruction keywords, and, correspondingly, they can be assigned
-- a different HTML class, if so desired.
--

  Call Assign   .EL.KEYWORD,                     "kw"
  Call Assign   .EL.SUBKEYWORD,                  "skw"
  Call Assign   .EL.DIRECTIVE_KEYWORD,           "dkw"
  Call Assign   .EL.DIRECTIVE_SUBKEYWORD,        "dskw"

--------------------------------------------------------------------------------
-- Variables                                                                  --
--------------------------------------------------------------------------------
--
-- The parser is able to differentiate exposed (instance) variables from
-- local variables.
--

  Call Assign   .EL.SIMPLE_VARIABLE,             "var"
  Call Assign   .EL.COMPOUND_VARIABLE,           "cmp"
  Call Assign   .EL.STEM_VARIABLE,               "stem"

  Call Assign   .EL.EXPOSED_SIMPLE_VARIABLE,     "xvar"
  Call Assign   .EL.EXPOSED_COMPOUND_VARIABLE,   "xcmp"
  Call Assign   .EL.EXPOSED_STEM_VARIABLE,       "xstem"

--------------------------------------------------------------------------------
-- Environment symbols                                                        --
--------------------------------------------------------------------------------

  Call Assign   .EL.ENVIRONMENT_SYMBOL,          "env"

--------------------------------------------------------------------------------
-- Pure constant symbols                                                      --
--------------------------------------------------------------------------------
--
-- That is, constant symbols that are not environment symbols or numbers
--

  Call Assign   .EL.SYMBOL_LITERAL,              "lit"
  Call Assign   .EL.PERIOD,                      "lit"

--------------------------------------------------------------------------------
-- Parse placeholders and explicit end-of-clause                              --
--------------------------------------------------------------------------------
--
-- .EL.PARSE_PERIOD is the "." placeholder in PARSE templates.
-- .EL.END_OF_CLAUSE is the explicit ";" clause terminator.
-- Both are treated as special characters for highlighting purposes.
--

  Call Assign   .EL.PARSE_PERIOD,                "spe"
  Call Assign   .EL.END_OF_CLAUSE,               "spe"

/******************************************************************************/
/* COMMENTS AND DOC-COMMENTS                                                  */
/******************************************************************************/

  Call Assign   .EL.LINE_COMMENT,                "lncm"
  Call Assign   .EL.DOC_COMMENT_MARKDOWN,        "doc-comment-markdown"
  Call Assign   .EL.STANDARD_COMMENT,            "cm"
  Call Assign   .EL.DOC_COMMENT,                 "doc-comment"


  Signal SkipOverDocComms

DocComms:
  Call Assign Arg(1), Arg(2)

  pa2  = prefix||Arg(2)
  doc1 = prefix"doc-comment"
  doc2 = prefix"doc-comment-markdown"

  HTMLClass[ .EL.DOC_COMMENT,          Arg(1)   ] = doc1 pa2
  HTMLClass[ .EL.DOC_COMMENT_MARKDOWN, Arg(1)   ] = doc2 pa2

  Return

SkipOverDocComms:

  Call DocComms .EL.DOC_COMMENT_ARMATURE,        "doc-comment-armature"
  Call DocComms .EL.DOC_COMMENT_WHITESPACE,      "doc-comment-whitespace"
  Call DocComms .EL.DOC_COMMENT_SUMMARY,         "doc-comment-summary"
  Call DocComms .EL.DOC_COMMENT_MAIN_DESCRIPTION,"doc-comment-main-description"
  Call DocComms .EL.DOC_COMMENT_TAG,             "doc-comment-tag"
  Call DocComms .EL.DOC_COMMENT_TAG_VALUE,       "doc-comment-tag-value"
  Call DocComms .EL.DOC_COMMENT_TAG_DESCRIPTION, "doc-comment-tag-description"

/******************************************************************************/
/* RESOURCES                                                                  */
/******************************************************************************/
--
-- See also the .RESOURCE.NAME and .RESOURCE.DELIMITER.NAME
-- subcategories below.
--

  Call Assign   .EL.RESOURCE_DATA,               "res-data"
  Call Assign   .EL.RESOURCE_IGNORED_DATA,       "res-ignore"

/******************************************************************************/
/* WHITESPACE AND CONTINUATIONS                                               */
/******************************************************************************/

  Call Assign   .EL.WHITESPACE,                  "ws"
  Call Assign   .EL.CONTINUATION,                "cont"

/******************************************************************************/
/* OPERATORS                                                                  */
/******************************************************************************/

  Signal SkipOverOperator

Operator:
  -- Valid values ("group", "full", "detail") are sanitized by callers
  -- (FencedCode.cls ParseOptions). Otherwise Nop is defensive.
  Select Case Options.operator
    When "group"  Then Call Assign Arg(1), "op"
    When "full"   Then Call Assign Arg(1), "op" Arg(2)
    When "detail" Then Call Assign Arg(1), Arg(2)
    Otherwise Nop
  End
Return

SkipOverOperator:

--------------------------------------------------------------------------------
-- Single-character operators                                                 --
--------------------------------------------------------------------------------

  Call Operator .EL.OP.AND,                      "and"
  Call Operator .EL.OP.DIVISION,                 "div"
  Call Operator .EL.OP.EQUAL,                    "eq"
  Call Operator .EL.OP.GREATER_THAN,             "gt"
  Call Operator .EL.OP.INTEGER_DIVISION,         "idiv"
  Call Operator .EL.OP.LOWER_THAN,               "lt"
  Call Operator .EL.OP.MINUS,                    "sub"
  Call Operator .EL.OP.MESSAGE,                  "msg"
  Call Operator .EL.OP.MULTIPLICATION,           "mul"
  Call Operator .EL.OP.NEGATION,                 "not"
  Call Operator .EL.OP.OR,                       "or"
  Call Operator .EL.OP.PLUS,                     "add"
  Call Operator .EL.OP.REFERENCE.LOWER_THAN,     "ref-lt"
  Call Operator .EL.OP.REFERENCE.GREATER_THAN,   "ref-gt"

--------------------------------------------------------------------------------
-- Prefix operators                                                           --
--------------------------------------------------------------------------------
--
-- "\" is always a prefix operator
--

  Call Operator .EL.OP.PREFIX.MINUS,             "prf-sub"
  Call Operator .EL.OP.PREFIX.PLUS,              "prf-add"

--------------------------------------------------------------------------------
-- Multi-character operator sequences                                         --
--------------------------------------------------------------------------------
--
-- There may be some whitespace, comments and continuations inside
-- the sequence; only the relevant characters will be tagged.
--

  Call Operator .EL.OP.CASCADING_MESSAGE,        "cmsg"
  Call Operator .EL.OP.CONCATENATION,            "cat"
  Call Operator .EL.OP.GREATER_OR_EQUAL,         "ge"
  Call Operator .EL.OP.GREATER_OR_LOWER_THAN,    "glt"
  Call Operator .EL.OP.LOWER_OR_EQUAL,           "le"
  Call Operator .EL.OP.LOWER_OR_GREATER_THAN,    "lgt"
  Call Operator .EL.OP.NOT_GREATER_THAN,         "ngt"
  Call Operator .EL.OP.NOT_LOWER_THAN,           "nlt"
  Call Operator .EL.OP.NOT_EQUAL,                "ne"
  Call Operator .EL.OP.POWER,                    "pow"
  Call Operator .EL.OP.REMAINDER,                "rem"
  Call Operator .EL.OP.XOR,                      "xor"

--------------------------------------------------------------------------------
-- Strict comparison operator sequences                                       --
--------------------------------------------------------------------------------
--
-- All are multi-character
--

  Call Operator .EL.OP.STRICT.LOWER_THAN,        "st-lt"
  Call Operator .EL.OP.STRICT.GREATER_OR_EQUAL,  "st-ge"
  Call Operator .EL.OP.STRICT.GREATER_THAN,      "st-gt"
  Call Operator .EL.OP.STRICT.NOT_EQUAL,         "st-ne"
  Call Operator .EL.OP.STRICT.NOT_LOWER_THAN,    "st-nlt"
  Call Operator .EL.OP.STRICT.NOT_GREATER_THAN,  "st-ngt"
  Call Operator .EL.OP.STRICT.LOWER_OR_EQUAL,    "st-le"
  Call Operator .EL.OP.STRICT.EQUAL,             "st-eq"

--------------------------------------------------------------------------------
-- Blank concatenation                                                        --
--------------------------------------------------------------------------------
--
-- The abuttal operator is zero-length, but blank concatenation is not.
--

  Call Operator .EL.OP.BLANK,                    "blank"

/******************************************************************************/
/* ASSIGNMENTS                                                                */
/******************************************************************************/

  Signal SkipOverAssignment

Assignment:
  -- Valid values ("group", "full", "detail") are sanitized by callers
  -- (FencedCode.cls ParseOptions). Otherwise Nop is defensive.
  Select Case Options.assignment
    When "group"  Then Call Assign Arg(1), "asg"
    When "full"   Then Call Assign Arg(1), "asg" Arg(2)
    When "detail" Then Call Assign Arg(1), Arg(2)
    Otherwise Nop
  End
Return

SkipOverAssignment:

--------------------------------------------------------------------------------

  Call Assignment .EL.ASG.EQUAL,                 "asg-equal"

  Call Assignment .EL.ASG.PLUS,                  "asg-add"
  Call Assignment .EL.ASG.MINUS,                 "asg-sub"
  Call Assignment .EL.ASG.MULTIPLY,              "asg-mul"
  Call Assignment .EL.ASG.DIVIDE,                "asg-div"
  Call Assignment .EL.ASG.INTEGER_DIVISION,      "asg-idiv"
  Call Assignment .EL.ASG.AND,                   "asg-and"
  Call Assignment .EL.ASG.OR,                    "asg-or"
  Call Assignment .EL.ASG.REMAINDER,             "asg-rem"
  Call Assignment .EL.ASG.CONCATENATION,         "asg-cat"
  Call Assignment .EL.ASG.XOR,                   "asg-xor"
  Call Assignment .EL.ASG.POWER,                 "asg-pow"

/******************************************************************************/
/* SPECIAL CHARACTERS                                                         */
/******************************************************************************/

  Signal SkipOverSpecial

Special:
  -- Valid values ("group", "full", "detail") are sanitized by callers
  -- (FencedCode.cls ParseOptions). Otherwise Nop is defensive.
  Select Case Options.special
    When "group"  Then Call Assign Arg(1), "spe"
    When "full"   Then Call Assign Arg(1), "spe" Arg(2)
    When "detail" Then Call Assign Arg(1), Arg(2)
    Otherwise Nop
  End
Return

SkipOverSpecial:

--------------------------------------------------------------------------------

  Call Special .EL.COMMA,                        "comma"
  Call Special .EL.COLON,                        "colon"
  Call Special .EL.LEFT_PARENTHESIS,             "paren"
  Call Special .EL.RIGHT_PARENTHESIS,            "paren"
  Call Special .EL.LEFT_BRACKET,                 "bracket"
  Call Special .EL.RIGHT_BRACKET,                "bracket"
  Call Special .EL.LEFT_CURLY_BRACKET,           "curly"
  Call Special .EL.RIGHT_CURLY_BRACKET,          "curly"


--
-- Executor final "=" and "==" in command instruction expressions
--
-- We handle them as special characters so that they can be highlighted
-- differently from assignment and comparison operator tokens.

  Call Special .EL.EXECUTOR.FINAL.EQUAL,         "xtr-final-equal"
  Call Special .EL.EXECUTOR.FINAL.DOUBLE.EQUAL,  "xtr-final-double-equal"

--
-- Directive start. Although technically this is a sequence of special
-- characters, we assign a new class using "Assign" instead of "Special",
-- since we will probably want to highlight "::" differently from other
-- specials and special .
--

  Call Assign  .EL.DIRECTIVE_START,              "dir"

-- The period as a compound variable tail separator is a pseudo-special, ...

  Call Special .EL.TAIL_SEPARATOR,               "period"

  -- ... as is the "..." construct at the end of an argument list.

  Call Special .EL.ELLIPSIS,                     "ellipsis"

/******************************************************************************/
/* NUMBERS                                                                    */
/******************************************************************************/

--------------------------------------------------------------------------------
-- Numbers                                                                    --
--------------------------------------------------------------------------------

  Call Assign   .EL.EXPONENTIAL_NUMBER,          "exp"
  Call Assign   .EL.DECIMAL_NUMBER,              "deci"
  Call Assign   .EL.INTEGER_NUMBER,              "int"

  Call Assign   .EL.NUMBER_SIGN,                 "nsign"
  Call Assign   .EL.INTEGER_PART,                "ipart"
  Call Assign   .EL.DECIMAL_POINT,               "dpoint"
  Call Assign   .EL.FRACTIONAL_PART,             "fpart"
  Call Assign   .EL.EXPONENT_MARK,               "emark"
  Call Assign   .EL.EXPONENT_SIGN,               "esign"
  Call Assign   .EL.EXPONENT,                    "expon"

/******************************************************************************/
/* STRINGS                                                                    */
/******************************************************************************/

  Call String   .EL.BINARY_STRING,               "bstr"
  Call String   .EL.STRING,                      "str"
  Call String   .EL.HEX_STRING,                  "xstr"
  Call String   .EL.BYTES_STRING,                "ystr"
  Call String   .EL.CODEPOINTS_STRING,           "pstr"
  Call String   .EL.GRAPHEMES_STRING,            "gstr"
  Call String   .EL.TEXT_STRING,                 "tstr"
  Call String   .EL.UNICODE_STRING,              "ustr"

  Signal SkipOverStrings

String:
  Call Assign Arg(1), Arg(2)
  pa2 = prefix||Arg(2)

  HTMLClass[ Arg(1), .EL.STRING_OPENING_DELIMITER ] = pa2 prefix"oquo"
  HTMLClass[ Arg(1), .EL.STRING_CLOSING_DELIMITER ] = pa2 prefix"cquo"
  HTMLClass[ Arg(1), .EL.STRING_SUFFIX            ] = pa2 prefix"ssuf"

  Return

SkipOverStrings:

  HTMLClass[ .EL.STRING_OPENING_DELIMITER ] = prefix"oquo"
  HTMLClass[ .EL.STRING_CLOSING_DELIMITER ] = prefix"cquo"
  HTMLClass[ .EL.STRING_SUFFIX            ] = prefix"ssuf"


/******************************************************************************/
/* STRINGS OR SYMBOLS TAKEN AS A CONSTANT                                     */
/******************************************************************************/

  Signal SkipOverConstant

Constant:
  -- Valid values ("group", "full", "detail") are sanitized by callers
  -- (FencedCode.cls ParseOptions). Otherwise Nop is defensive.
  Select Case Options.["CONSTANT"]
    When "group"  Then value = prefix"const"
    When "full"   Then value = prefix"const" prefix||Arg(2)
    When "detail" Then value = prefix||Arg(2)
    Otherwise          value = prefix"const"
  End
  HTMLClass[ constant, Arg(1) ] = value

  open   = .EL.STRING_OPENING_DELIMITER
  close  = .EL.STRING_CLOSING_DELIMITER
  suffix = .EL.STRING_SUFFIX

  HTMLClass[ constant, Arg(1), open   ] = value prefix"oquo"
  HTMLClass[ constant, Arg(1), close  ] = value prefix"cquo"
  HTMLClass[ constant, Arg(1), suffix ] = value prefix"ssuf"

Return

SkipOverConstant:

--------------------------------------------------------------------------------

  constant = .EL.TAKEN_CONSTANT

-- Function and subroutine call constants

  Call Constant .INTERNAL.FUNCTION.NAME,           "int-func"
  Call Constant .INTERNAL.SUBROUTINE.NAME,         "int-proc"
  Call Constant .BUILTIN.FUNCTION.NAME,            "bif-func"
  Call Constant .BUILTIN.SUBROUTINE.NAME,          "bif-proc"
  Call Constant .PACKAGE.FUNCTION.NAME,            "pkg-func"
  Call Constant .PACKAGE.SUBROUTINE.NAME,          "pkg-proc"
  Call Constant .EXTERNAL.PACKAGE.FUNCTION.NAME,   "ext-pkg-func"
  Call Constant .EXTERNAL.PACKAGE.SUBROUTINE.NAME, "ext-pkg-proc"
  Call Constant .EXTERNAL.FUNCTION.NAME,           "ext-func"
  Call Constant .EXTERNAL.SUBROUTINE.NAME,         "ext-proc"

-- Other constants

  Call Constant .ANNOTATION.NAME,                  "annotation"
  Call Constant .ARGUMENT.NAME,                    "argument-name"
  Call Constant .BLOCK.INSTRUCTION.NAME,           "block"
  Call Constant .CLASS.NAME,                       "class"
  Call Constant .ENVIRONMENT.NAME,                 "environment"
  Call Constant .LABEL.NAME,                       "label"
  Call Constant .METHOD.NAME,                      "method"
  Call Constant .NAMESPACE.NAME,                   "namespace"
  Call Constant .ROUTINE.NAME,                     "routine"
  Call Constant .REQUIRES.PROGRAM.NAME,            "requires"
  Call Constant .RESOURCE.NAME,                    "resource"
  Call Constant .RESOURCE.DELIMITER.NAME,          "res-delimiter"
  Call Constant .USER.CONDITION.NAME,              "user-condition"

  Call Constant .ANNOTATION.VALUE,                 "annotation-value"
  Call Constant .CONSTANT.VALUE,                   "constant-value"

--------------------------------------------------------------------------------

Return HTMLClass

/******************************************************************************/
/* PROSE FACADE - public vocabulary for inline mentions in prose              */
/* ============================================================               */
/*                                                                            */
/* A prose author does not know, and should not need to know, the internal    */
/* rx-* class taxonomy (which is compound, and shaped by the Options. that    */
/* were active when the fenced blocks around it were highlighted). Instead,   */
/* the author writes `` `Bark`{.rexx method} `` and we translate the public   */
/* name to whatever HTMLClasses would have emitted for that element           */
/* category under the SAME Options. - one source of truth, so a future        */
/* change to classprefix, or to "constant" mode, or to the rx-* taxonomy      */
/* itself, keeps prose mentions in sync without touching this table.          */
/*                                                                            */
/* This is a lookup into a category (or constant/subcategory pair), never     */
/* a hardcoded string: ProseElementTags below always re-derives the tags      */
/* from the HTMLClass stem the caller already built with HTMLClasses().       */
/*                                                                            */
/******************************************************************************/

--
-- ProseModifierTable returns the unified {.rexx MODIFIER} vocabulary table.
--
-- ONE table, ONE source of truth for every modifier a prose mention may carry.
-- It replaces the three vocabularies v175 kept apart (IsProseMode's mode list,
-- IsProseAssertion's semantic names, and the ProseTable of element categories).
-- Collapsing them removes the last hyphens from the vocabulary: a modifier that
-- used to be a hyphenated atom (`internal-function`, `constant-value`,
-- `directive-keyword`) is now COMPOSITION -- two or three space-separated words
-- the tag parser matches longest-first (3 -> 2 -> 1). See ScanProseMentions.
--
-- INDEX: the space form ("internal function", "external package function").
-- VALUE: an Array whose FIRST item is the modifier TYPE, and whose remaining
--        items are that type's payload:
--
--   ("MODE")                    -- a parse mode refining an auto-parse. The
--                                  index IS the mode name; ModeChip consumes it.
--                                  Domain-checked, shouts in situ if misapplied.
--   ("OPERATOR", category)      -- an operator NAMED because its glyph cannot be
--                                  written as span content. Today only "blank
--                                  concatenation" (glyph is a space). Resolves
--                                  through SymbolChip, exactly like a glyph hit.
--   ("ELEMENT", category)             -- a semantic name -> one element category.
--   ("ELEMENT", category, subcategory)-- ... -> .EL.TAKEN_CONSTANT + subcategory.
--                                  What the author asserts a word IS. The class
--                                  HTMLClasses() emits for the (sub)category is
--                                  the single source of truth, so a mentioned
--                                  name and a fenced token share their colour.
--   ("KLUDGE")                  -- a name with no single category, handled
--                                  specially in the by-element path ("directive"
--                                  = `::`+keyword; "exposed" = auto-parse then
--                                  relabel). The index IS the kludge name.
--
-- To publish a new modifier: add one line here. Nowhere else changes.
--
-- LONGEST-MATCH is unambiguous because no real fence attribute (style,
-- numberLines, classprefix...) is ever the 2nd or 3rd component of an entry:
-- attributes carry "=value" or are single words the parser never treats as a
-- modifier's tail. So a 2- or 3-word modifier can never be mistaken for a
-- 1-word modifier followed by an attribute.

::Routine ProseModifierTable Public

  Table. = .Nil

  --
  -- MODES -- refine an auto-parse. "blank concatenation" is NOT here: it is an
  -- OPERATOR (below), not a mode -- a named operator whose glyph is a space.
  --
  Table.["prefix"]     = .Array~of( "MODE" )
  Table.["reference"]  = .Array~of( "MODE" )
  Table.["comparison"] = .Array~of( "MODE" )
  Table.["assignment"] = .Array~of( "MODE" )
  Table.["expression"] = .Array~of( "MODE" )

  --
  -- NAMED OPERATORS -- operators reached by name because their glyph cannot be
  -- span content. The blank concatenation operator (.EL.OP.BLANK) is the sibling
  -- of "||" (.EL.OP.CONCATENATION): a genuine operator, not whitespace. Its glyph
  -- is a single space, the one glyph that cannot be abbreviated as content, so it
  -- is the only operator that must be named instead of written.
  --
  Table.["blank concatenation"] = .Array~of( "OPERATOR", .EL.OP.BLANK )

  --
  -- ELEMENTS -- the keyword family. Named SEMANTICALLY -- by what the author
  -- asserts the word IS, not by the syntactic role it would take inside a
  -- construct the author did not write.
  --
  -- "variable" (-> the simple-variable class) is the override for a plain
  -- variable whose name is ALSO a keyword instruction (`parse`, `say`...): a
  -- one-token fragment of such a name would be captured as a keyword clause and
  -- fail to parse under {.rexx}, so the author publishes "variable" to say "I
  -- mean the variable, not the keyword".
  --
  Table.["keyword"]              = .Array~of( "ELEMENT", .EL.KEYWORD )
  Table.["subkeyword"]           = .Array~of( "ELEMENT", .EL.SUBKEYWORD )
  Table.["directive keyword"]    = .Array~of( "ELEMENT", .EL.DIRECTIVE_KEYWORD )
  Table.["directive subkeyword"] = .Array~of( "ELEMENT", .EL.DIRECTIVE_SUBKEYWORD )
  Table.["variable"]             = .Array~of( "ELEMENT", .EL.SIMPLE_VARIABLE )

  -- The old "directive-start" name (a lone `::`) is gone: `::` colours by GLYPH
  -- lookup (ProseSymbolTable), no author assertion needed. The compound
  -- "directive" (`::Method`) survives as a kludge (below), being a two-element
  -- sequence with no single category.

  --
  -- ELEMENTS -- taken constants: names and markers. Each maps to
  -- .EL.TAKEN_CONSTANT plus the constant subcategory the engine assigns.
  --
  Table.["class"]              = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .CLASS.NAME              )
  Table.["method"]             = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .METHOD.NAME             )
  Table.["routine"]            = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .ROUTINE.NAME            )
  Table.["label"]              = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .LABEL.NAME              )
  Table.["namespace"]          = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .NAMESPACE.NAME          )
  Table.["annotation"]         = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .ANNOTATION.NAME         )
  Table.["argument"]           = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .ARGUMENT.NAME           )
  Table.["block"]              = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .BLOCK.INSTRUCTION.NAME  )
  Table.["environment"]        = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .ENVIRONMENT.NAME        )
  Table.["requires"]           = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .REQUIRES.PROGRAM.NAME   )
  Table.["resource"]           = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .RESOURCE.NAME           )
  Table.["resource delimiter"] = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .RESOURCE.DELIMITER.NAME )
  Table.["user condition"]     = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .USER.CONDITION.NAME     )
  Table.["constant value"]     = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .CONSTANT.VALUE          )
  Table.["annotation value"]   = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .ANNOTATION.VALUE        )

  -- Exposed variables USED to be three rows (exposed-variable / exposed-compound
  -- / exposed-stem). They are now the SINGLE "exposed" KLUDGE (below): the author
  -- asserts only the extra-textual fact "this is exposed"; WHICH kind (simple /
  -- compound / stem) the auto-parse deduces from the text. See ExposedChip.

  --
  -- ELEMENTS -- calls (function / subroutine axis). These are where the hyphens
  -- died into composition. The engine's term is SUBROUTINE, not "procedure": a
  -- function must return a value, a subroutine need not. The bare 1-word form
  -- ("internal") means the subroutine sense -- the commoner one in prose.
  -- "external package" (2 words) is the same subroutine shortcut for the
  -- external-package origin; "external package function" (3 words) is the only
  -- entry that needs three components.
  --
  Table.["internal function"]           = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .INTERNAL.FUNCTION.NAME           )
  Table.["internal subroutine"]         = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .INTERNAL.SUBROUTINE.NAME         )
  Table.["internal"]                    = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .INTERNAL.SUBROUTINE.NAME         )
  Table.["builtin function"]            = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .BUILTIN.FUNCTION.NAME            )
  Table.["builtin subroutine"]          = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .BUILTIN.SUBROUTINE.NAME          )
  Table.["builtin"]                     = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .BUILTIN.SUBROUTINE.NAME          )
  Table.["package function"]            = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .PACKAGE.FUNCTION.NAME            )
  Table.["package subroutine"]          = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .PACKAGE.SUBROUTINE.NAME          )
  Table.["package"]                     = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .PACKAGE.SUBROUTINE.NAME          )
  Table.["external function"]           = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .EXTERNAL.FUNCTION.NAME           )
  Table.["external subroutine"]         = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .EXTERNAL.SUBROUTINE.NAME         )
  Table.["external"]                    = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .EXTERNAL.SUBROUTINE.NAME         )
  Table.["external package function"]   = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .EXTERNAL.PACKAGE.FUNCTION.NAME   )
  Table.["external package subroutine"] = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .EXTERNAL.PACKAGE.SUBROUTINE.NAME )
  Table.["external package"]            = .Array~of( "ELEMENT", .EL.TAKEN_CONSTANT, .EXTERNAL.PACKAGE.SUBROUTINE.NAME )

  --
  -- KLUDGES -- names with no single category, handled specially in the
  -- by-element path of ScanProseMentions. "directive" (`::Method`) is a
  -- two-element `::`+keyword sequence; "exposed" auto-parses to find the variable
  -- kind and relabels the base to its exposed colour.
  --
  Table.["directive"] = .Array~of( "KLUDGE" )
  Table.["exposed"]   = .Array~of( "KLUDGE" )

  --
  -- Glyph operators, assignment operators and special characters are NOT named
  -- here: a glyph is coloured by writing the glyph itself inside {.rexx},
  -- resolved by LOOKUP against ProseSymbolTable (glyph -> category, below). The
  -- few glyphs a bare lookup cannot disambiguate are reached by a MODE modifier
  -- above ("=" -> comparison/assignment; "+"/"-" -> prefix; "<"/">" -> reference).

Return Table.

--
-- ProseSymbolTable returns the glyph -> element-category table.
--
-- The counterpart of ProseTable for the OTHER prose axis: where ProseTable maps
-- a semantic NAME an author asserts ("method", "class"), this maps a Rexx
-- GLYPH directly to its category, so `` `*`{.rexx} `` colours without any name.
-- FencedCode.cls's ScanProseMentions consults this BEFORE handing a bare
-- {.rexx} fragment to the auto-parser: a lone glyph does not sustain itself as a
-- parseable unit, so the parser would shout; the lookup answers first.
--
-- Every glyph is copied VERBATIM from the engine's own Scanner (Scanner.cls),
-- so the two never drift: if the Scanner recognises "\==" as strict-not-equal,
-- so do we, with the same category.
--
-- DELIBERATELY ABSENT (they have no single default -- a modifier resolves them,
-- see ScanProseMentions and the note at the end of ProseTable):
--   "="  -- OP.EQUAL vs ASG.EQUAL: bare {.rexx} shouts, modifier picks.
--   the unary "+"/"-", the reference "<"/">", and the blank operator: the
--   binary/comparison senses ARE the defaults and live here; the other senses
--   are modifier-only, so no glyph row would be unambiguous.
--
-- To publish a new glyph: add one line here. Nowhere else changes.
--

::Routine ProseSymbolTable Public

  Sym. = .Nil

  -- Operators (Scanner.cls single- and multi-char operator tables).
  Sym.["+"]   = .EL.OP.PLUS
  Sym.["-"]   = .EL.OP.MINUS
  Sym.["*"]   = .EL.OP.MULTIPLICATION
  Sym.["/"]   = .EL.OP.DIVISION
  Sym.["%"]   = .EL.OP.INTEGER_DIVISION
  Sym.["//"]  = .EL.OP.REMAINDER
  Sym.["**"]  = .EL.OP.POWER
  Sym.["||"]  = .EL.OP.CONCATENATION
  Sym.["&"]   = .EL.OP.AND
  Sym.["|"]   = .EL.OP.OR
  Sym.["&&"]  = .EL.OP.XOR
  Sym.["\"]   = .EL.OP.NEGATION
  Sym.["~"]   = .EL.OP.MESSAGE
  Sym.["~~"]  = .EL.OP.CASCADING_MESSAGE

  -- Non-strict comparison. ">" and "<" default here; the reference sense is
  -- {.rexx reference}. "=" is absent (ambiguous, modifier-only).
  Sym.["\="]  = .EL.OP.NOT_EQUAL
  Sym.[">"]   = .EL.OP.GREATER_THAN
  Sym.["<"]   = .EL.OP.LOWER_THAN
  Sym.[">="]  = .EL.OP.GREATER_OR_EQUAL
  Sym.["<="]  = .EL.OP.LOWER_OR_EQUAL
  Sym.["><"]  = .EL.OP.GREATER_OR_LOWER_THAN
  Sym.["<>"]  = .EL.OP.LOWER_OR_GREATER_THAN
  Sym.["\>"]  = .EL.OP.NOT_GREATER_THAN
  Sym.["\<"]  = .EL.OP.NOT_LOWER_THAN

  -- Strict comparison.
  Sym.["=="]  = .EL.OP.STRICT.EQUAL
  Sym.["\=="] = .EL.OP.STRICT.NOT_EQUAL
  Sym.[">>"]  = .EL.OP.STRICT.GREATER_THAN
  Sym.["<<"]  = .EL.OP.STRICT.LOWER_THAN
  Sym.[">>="] = .EL.OP.STRICT.GREATER_OR_EQUAL
  Sym.["<<="] = .EL.OP.STRICT.LOWER_OR_EQUAL
  Sym.["\>>"] = .EL.OP.STRICT.NOT_GREATER_THAN
  Sym.["\<<"] = .EL.OP.STRICT.NOT_LOWER_THAN

  -- (Extended) assignment operators. The bare "=" assignment is modifier-only
  -- ({.rexx assignment}); the compound forms are unambiguous and live here.
  Sym.["+="]  = .EL.ASG.PLUS
  Sym.["-="]  = .EL.ASG.MINUS
  Sym.["*="]  = .EL.ASG.MULTIPLY
  Sym.["/="]  = .EL.ASG.DIVIDE
  Sym.["%="]  = .EL.ASG.INTEGER_DIVISION
  Sym.["//="] = .EL.ASG.REMAINDER
  Sym.["||="] = .EL.ASG.CONCATENATION
  Sym.["&="]  = .EL.ASG.AND
  Sym.["|="]  = .EL.ASG.OR
  Sym.["&&="] = .EL.ASG.XOR
  Sym.["**="] = .EL.ASG.POWER

  -- Special characters (rexxref 1.10.4.7, minus "~" -- an operator above -- and
  -- ";"). "::" is here as a plain glyph: it is the directive start, and a lone
  -- `::` in prose colours as rx-dir by lookup, with no modifier -- the old
  -- "directive-start" semantic name is gone (a `::` needs no author assertion,
  -- its glyph is unambiguous). Must precede ":" is irrelevant (exact-match
  -- lookup, not longest-prefix), but we keep "::" adjacent to ":" for clarity.
  Sym.["::"]  = .EL.DIRECTIVE_START
  Sym.[","]   = .EL.COMMA
  Sym.[":"]   = .EL.COLON
  Sym.["("]   = .EL.LEFT_PARENTHESIS
  Sym.[")"]   = .EL.RIGHT_PARENTHESIS
  Sym.["["]   = .EL.LEFT_BRACKET
  Sym.["]"]   = .EL.RIGHT_BRACKET
  Sym.["{"]   = .EL.LEFT_CURLY_BRACKET
  Sym.["}"]   = .EL.RIGHT_CURLY_BRACKET

Return Sym.

--
-- ProseElementTags - translate a semantic modifier name to its HTML class string
--
-- ARGUMENTS
-- ---------
--   HTMLClass:  The stem returned by HTMLClasses(Options.), already built
--               against the Options. in effect for the surrounding document.
--   PublicName: An ELEMENT modifier from ProseModifierTable, in its space form
--               (e.g. "method", "internal function"; the "rexx-" prefix already
--               stripped by the caller).
--
-- RETURNS
-- -------
--   The HTML class string (e.g. "rx-const rx-method"), exactly what a
--   fenced block would emit for that category today. .Nil if PublicName is
--   not a published ELEMENT name — vocabulary validation is the caller's job
--   (FencedCode.cls), so a typo can be reported with the source position,
--   which we don't have here. A modifier that IS published but is not of type
--   ELEMENT (a MODE, OPERATOR or KLUDGE) also returns .Nil: those do not resolve
--   to a flat element class and are handled by their own paths in the caller.
--

::Routine ProseElementTags Public
  Use Strict Arg HTMLClass, PublicName

  Table. = ProseModifierTable()
  entry  = Table.[ PublicName ]
  If entry == .Nil Then Return .Nil
  If entry[1] \== "ELEMENT" Then Return .Nil

  -- entry is ("ELEMENT", category) or ("ELEMENT", category, subcategory)
  If entry~items == 2 Then Return HTMLClass[ entry[2] ]
  Return HTMLClass[ entry[2], entry[3] ]