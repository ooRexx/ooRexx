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
/* Globals.cls - element categories, category sets, and taken constants       */
/* ====================================================================       */
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
/* 20241209    0.1b Add .EL.SHEBANG                                           */
/*                  New call system                                           */
/* 20241224    0.1d Add .EL.DOC_COMMENT and .EL.DOC_COMMENT_MARKDOWN          */
/* 20250103    0.1f Add TUTOR-flavored Unicode support                        */
/* 20250116         Add EL.PARSE_PERIOD                                       */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250406         Rename fractional numbers to decimal                      */
/* 20250529    0.2c Add support for parts of a string                         */
/* 20250601         Add ALL.STRING_DELIMITERS and ALL.STRING_COMPONENTS       */
/* 20250629         Add support for doc-comment subelements                   */
/* 20250722    0.2d Add .EL.DIRECTIVE_SUBKEYWORD and .ALL.DIRECTIVE_KEYWORDS  */
/* 20250729         Add .ALL.EXPOSED_VARIABLES                                */
/* 20250808         Add support for length positional patterns                */
/* 20251125    0.3a Add support for Executor implicit abuttal                 */
/* 20251129         (Executor) Create elements for curly brackets             */
/* 20251213         (Executor) Create .NAMEs for named arguments              */
/* 20251226    0.4a Send error messages to .error, not .output                */
/* 20251226         Add support for "=" and "==" at end of expression         */
/* 20261113         KeywordInstructionSet: CATCH and FINALLY (RFE #31), not   */
/*                  under CMS or Executor                                     */
/* 20261118         KeywordInstructionSet: THROW (RFE #31, proposed), not     */
/*                  under CMS or Executor                                     */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/*                                                                            */
/******************************************************************************/

  -- The purpose of this package is to load a set of global constants into
  -- the environment

  -- Check if we are already loaded, and exit if so
  If .environment~Parser.Constants.Loaded == 1 Then Exit

  -- Load all the constants...
  Call LoadElementConstants
  -- ...and remember the fact
 .environment~Parser.Constants.Loaded = 1

/******************************************************************************/
/* CategoryName(Arg(1)) returns the readable name of category "Arg(1)"        */
/******************************************************************************/

::Routine CategoryName Public
  Return .Parser.CategoryName[Arg(1)]

/******************************************************************************/
/* ConstantName(name) returns the readable name of subCategory "name"         */
/******************************************************************************/

::Routine ConstantName Public
  Return .Parser.ConstantName[Arg(1)]

/******************************************************************************/
/* LoadElementConstants -- Load global constants into the environment         */
/******************************************************************************/

::Routine LoadElementConstants Private

/******************************************************************************/
/*                                                                            */
/* List of category classes                                                   */
/* =====================                                                      */
/*                                                                            */
/* "Elements" are proper Rexx tokens (e.g., symbols, strings, operator        */
/* characters, and so on), other character sequences, like whitespace and     */
/* comments, and certain combinations of elements which have special          */
/* significance, like compound operators, extended assignment sequences,      */
/* and the "::" directive start sequence.                                     */
/*                                                                            */
/* The list below contains all the possible element categories, which are     */
/* stored as part of each element, in the attribute element~category.         */
/* Some of these categories are created and assigned in the first parts       */
/* of the scanning process, and some others are discovered, altered,          */
/* or inserted later.                                                         */
/*                                                                            */
/* We will dynamically (i.e., on each run) assign a 1-byte value to each of   */
/* the constants in the categories array. This will allow us to construct     */
/* set values by concatenating these 1-byte values and using the POS() BIF,   */
/* the Contains String method, or similar techniques, to determine if the     */
/* class of a certain element belongs or not to a particular set.             */
/*                                                                            */
/* As an application of this idea, the .Element class overloads               */
/* the "<" operator, so that the construct                                    */
/*                                                                            */
/*   element < category                                                       */
/*                                                                            */
/* means element~category = category, while                                   */
/*                                                                            */
/*   element < categorySet                                                    */
/*                                                                            */
/* means categorySet~contains(element~category).                              */
/*                                                                            */
/* NOTE: All references to rexxref below refer to the version distributed     */
/* with the 5.2.0 beta version of ooRexx, and the references to               */
/* the ANSI standard will always be in the form a.b.c.d                       */
/*                                                                            */
/******************************************************************************/

  categories                      = .Array~new
  takenConstant                   = .Array~new
 .environment~Parser.CategoryName = .Stem~new
 .environment~Parser.ConstantName = .Stem~new

--------------------------------------------------------------------------------
-- We place the internal routines here, so that we don't need to scroll down  --
-- to take a look at them.                                                    --
--------------------------------------------------------------------------------
  Signal SkipOverTheInternalRoutines -- Jump over them
--------------------------------------------------------------------------------

-- NewElement defines a new element category
-- NewElements defines several element categories at once.

NewElement :  -- Allow "Call NewElement" syntax
NewElements:  -- Allow "Call NewElements" syntax
  Procedure Expose categories
  Do i = 1 To Arg()
    name = Arg(i)
    index = categories~append( name )
    If index > 255 Then
      Call Halt "The Parser.CategoryName array has more than 255 elements"
    value = X2C(D2X( index ))
   .environment[ name ] = value
   .Parser.CategoryName[ value  ] = name
  End
Return

-- NewSet defined named sets of element categories

NewSet:
  setName = Arg(1)
  classes = Arg(2,"A")~makeString("Line","")
 .environment[setName] = classes
Return

-- NewConstant defines a new taken constant subcategory

NewConstant: Procedure Expose takenConstant
  Do i = 1 To Arg()
    name = Arg(i)
    index = takenConstant~append( name )
    If index > 255 Then
      Call Halt "The Parser.ConstantName array has more than 255 elements"
    value = X2C(D2X( index ))
   .environment[ name  ] = value
   .Parser.ConstantName[ value ] = name
  End
Return

-- Implement global synonyms

Synonym:
  newName = Arg(1)
  value   = Arg(2)
 .environment[ newName ] = value
Return

Halt:
 .Error~Say( Arg(1) )
  Raise Halt

--------------------------------------------------------------------------------
-- End of internal routines                                                   --
--------------------------------------------------------------------------------

SkipOverTheInternalRoutines:            -- Main code continues here

  ------------------------------------------------------------------------------
  -- Syntax errors                                                            --
  ------------------------------------------------------------------------------
  Call NewElement EL.SYNTAX_ERROR       --

  ------------------------------------------------------------------------------
  -- Shebangs                                                                 --
  ------------------------------------------------------------------------------
  -- Shebangs are not really elements, but they need to be recognized by
  -- the parser
  Call NewElement  EL.SHEBANG             -- A line starting with "#!"

  ------------------------------------------------------------------------------
  -- Markers                                                                  --
  ------------------------------------------------------------------------------
  --
  -- The end-of-clause marker
  -- ------------------------
  --
  -- The parser inserts end-of-clause markers (implied semicolons) at the end
  -- of each line (except after a continuation, inside a classic comment, or
  -- inside a resource). It also inserts a dummy end-of-clause marker at the
  -- beginning of the element stream. This guarantees that the following
  -- invariant is always true:
  --
  -- INVARIANT 1:
  --
  --   All clauses are enclosed (that is, preceded and ended) between
  --   end-of-clause markers.
  --
  Call NewElement  EL.END_OF_CLAUSE       -- ";" (explicit or implied)
  --
  -- The implicit EXIT marker
  -- ------------------------
  --
  -- The Rexx language assumes an implicit EXIT instruction at the end of
  -- every code body [a "code body" is the collective name for the prolog,
  -- other ::Routines, and ::Methods].
  --
  -- This implicit exit instruction is provided, as a zero-width element,
  -- by the parser, so that the following invariants holds:
  --
  -- INVARIANT 2:
  --
  --   A Rexx program (a "package") consists of a prolog (a code body),
  --   followed by zero or more directives. Every directive is followed
  --   by a (possibly empty -- see below) code body.
  --
  -- INVARIANT 3:
  --
  --   All code bodies have at least one instruction, namely the
  --   implicit EXIT instructions, which always ends a body.
  --
  -- We say that a code body is empty when it consists only of null clauses
  -- followed by the implicit EXIT instruction. Please note that a code body
  -- containing any label is not empty.
  --
  -- The implicit EXIT instructions also provides an insertion point for null
  -- clauses, and for labels appearing after all instructions in a body.
  --
  -- Please note that, being an instruction, and therefore a clause, the
  -- implicit EXIT instruction will be flanked, as all clauses are, by
  -- (inserted) end-of-clause markers.
  --
  Call NewElement EL.IMPLICIT_EXIT
  --
  -- The end-of-source marker
  -- ------------------------
  --
  -- The parser inserts an additional pseudo-instruction after the last
  -- implicit EXIT instruction, the end-of-source marker. This is a special
  -- element that ends the element stream. As all instructions,
  -- it is flanked by end-of-clause markers, so that the following
  -- invariant holds:
  --
  -- INVARIANT 4:
  --
  --   All element streams end with the sequence
  --
  --     <EOC, implicit EXIT, EOC, end-of-source, EOC>.
  --
  --   where EOC is an end-of-clause marker. All these elements
  --   are inserted and generated by the parser. The end-of-source element
  --   appears in no other position in the element stream.
  --
  Call NewElement EL.END_OF_SOURCE

  ------------------------------------------------------------------------------
  -- Whitespace                                                               --
  ------------------------------------------------------------------------------
  --
  -- blank             := ' ' | other_blank_character                  (6.2.2.8)
  -- bo                := [blank+]                                     (6.2.2.9)
  --
  -- Will be promoted to EL.OP.BLANK in certain contexts.
  Call NewElement EL.WHITESPACE         -- "20"X and "09"X
  --
  Call NewElement EL.CONTINUATION       -- "," and "-" at end of line
  --
  Call NewSet ALL.WHITESPACE_LIKE, .EL.WHITESPACE, .EL.CONTINUATION

  ------------------------------------------------------------------------------
  -- Comments and documentation comments (doc-comments)                       --
  ------------------------------------------------------------------------------
  --
  -- comment           := '/' '*' [commentpart+] ['*'+] ('*' '/'      (6.2.2.18)
  --                      | EOS Msg6.1)
  --
  -- Line comments are not part of the ANSI standard
  --
  Call NewElements EL.STANDARD_COMMENT, EL.LINE_COMMENT

  -- A EL.DOC_COMMENT is a standard ("block") comment that starts with "/**"
  -- but not with "/***" and ends with "*/" but not with "**/".
  --
  -- A EL.DOC_COMMENT_MARKDOWN is a set of contiguous line comments that
  -- start with "---" but not with "----".
  Call NewElements EL.DOC_COMMENT, EL.DOC_COMMENT_MARKDOWN

  -- Doc-comments are returned as a single element, but some of the finer
  -- structure of the comment is also available via the "parts" method.
  -- The following are the doc-comment (sub-)element categories.

  Call NewElements -
    EL.DOC_COMMENT_ARMATURE,         -  -- The enclosing outer block
    EL.DOC_COMMENT_WHITESPACE,       -  -- Internal to the doc-comment contents
    EL.DOC_COMMENT_SUMMARY,          -  -- The first statement
    EL.DOC_COMMENT_MAIN_DESCRIPTION, -  -- Before the first @tag
    EL.DOC_COMMENT_TAG,              -  -- Like @param. Includes the initial @
    EL.DOC_COMMENT_TAG_VALUE,        -  -- E.g., the parameter name in @param
    EL.DOC_COMMENT_TAG_DESCRIPTION      -- E.g., the param. desc. in @param

  Call NewSet ALL.CLASSIC_DOC_COMMENTS,  -
    .EL.DOC_COMMENT,                     -
    .EL.DOC_COMMENT_ARMATURE,            -
    .EL.DOC_COMMENT_WHITESPACE,          -
    .EL.DOC_COMMENT_SUMMARY,             -
    .EL.DOC_COMMENT_MAIN_DESCRIPTION,    -
    .EL.DOC_COMMENT_TAG,                 -
    .EL.DOC_COMMENT_TAG_VALUE,           -
    .EL.DOC_COMMENT_TAG_DESCRIPTION
  Call NewSet ALL.MARKDOWN_DOC_COMMENTS, -
    .EL.DOC_COMMENT_MARKDOWN,            -
    .EL.DOC_COMMENT_ARMATURE,            -
    .EL.DOC_COMMENT_WHITESPACE,          -
    .EL.DOC_COMMENT_SUMMARY,             -
    .EL.DOC_COMMENT_MAIN_DESCRIPTION,    -
    .EL.DOC_COMMENT_TAG,                 -
    .EL.DOC_COMMENT_TAG_VALUE,           -
    .EL.DOC_COMMENT_TAG_DESCRIPTION
  Call NewSet ALL.DOC_COMMENTS,          -
    .EL.DOC_COMMENT,                     -
    .EL.DOC_COMMENT_MARKDOWN,            -
    .EL.DOC_COMMENT_ARMATURE,            -
    .EL.DOC_COMMENT_WHITESPACE,          -
    .EL.DOC_COMMENT_SUMMARY,             -
    .EL.DOC_COMMENT_MAIN_DESCRIPTION,    -
    .EL.DOC_COMMENT_TAG,                 -
    .EL.DOC_COMMENT_TAG_VALUE,           -
    .EL.DOC_COMMENT_TAG_DESCRIPTION
  Call NewSet ALL.NON_DOC_COMMENTS,  .EL.STANDARD_COMMENT, .EL.LINE_COMMENT
  Call NewSet ALL.COMMENTS,          .ALL.DOC_COMMENTS,    .ALL.NON_DOC_COMMENTS

  ------------------------------------------------------------------------------
  -- NUMBERS                                                                  --
  ------------------------------------------------------------------------------
  --
  -- Numbers are technically const_symbols. We handle them separately.
  --
  -- number            := plain_number [exponent]                     (6.2.2.35)
  -- plain_number      := ['.'] digit+ | digit+ '.' [digit+]          (6.2.2.36)
  -- exponent          := ('e' | 'E') ['+' | '-'] digit+              (6.2.2.37)
  --
  Call NewElements -
    EL.DECIMAL_NUMBER,  EL.INTEGER_NUMBER,  EL.EXPONENTIAL_NUMBER
  Call NewSet ALL.NUMBERS,            -
   .EL.DECIMAL_NUMBER, .EL.INTEGER_NUMBER, .EL.EXPONENTIAL_NUMBER

  -- Decimal and exponential numbers can be viewed as a single element,
  -- or as a sequence of smaller elements. The following categories
  -- are never assigned directly by the Parser, but they can be used by
  -- other tools, like the Highlighter.

  Call NewElements                      -
    EL.NUMBER_SIGN,                     - -- "+" or "-", inside strings only
    EL.INTEGER_PART,                    - -- Digits before the decimal point
    EL.DECIMAL_POINT,                   - -- Not a TAIL_SEPARATOR nor a PERIOD.
    EL.FRACTIONAL_PART,                 - -- Digits after the decimal point
    EL.EXPONENT_MARK,                   - -- "E" or "e"
    EL.EXPONENT_SIGN,                   - -- "+" or "-", optional
    EL.EXPONENT                           -- 1 or more digits

  ------------------------------------------------------------------------------
  -- STRINGS                                                                  --
  ------------------------------------------------------------------------------
  --
  -- string_literal    := Hex_string | Binary_string | String         (6.2.2.21)
  -- String            := quoted_string                               (6.2.2.22)
  -- Hex_string        := quoted_string RADIX ('x' | 'X')             (6.2.2.23)
  -- Binary_string     := quoted_string RADIX ('b' | 'B')             (6.2.2.24)
  --
  Call NewElements                      -
    EL.STRING,                          -
    EL.HEX_STRING,                      -
    EL.BINARY_STRING,                   -
    EL.BYTES_STRING,                    - -- TUTOR-flavored Unicode
    EL.CODEPOINTS_STRING,               - -- TUTOR-flavored Unicode
    EL.GRAPHEMES_STRING,                - -- TUTOR-flavored Unicode
    EL.TEXT_STRING,                     - -- TUTOR-flavored Unicode
    EL.UNICODE_STRING                     -- TUTOR-flavored Unicode
  Call NewSet ALL.STRINGS,              -
   .EL.STRING,                          -
   .EL.HEX_STRING,                      -
   .EL.BINARY_STRING,                   -
   .EL.BYTES_STRING,                    -
   .EL.CODEPOINTS_STRING,               -
   .EL.GRAPHEMES_STRING,                -
   .EL.TEXT_STRING,                     -
   .EL.UNICODE_STRING

  --
  -- Strings can be viewed as a single element,
  -- but also as a sequence of parts:
  --
  --   string  :=  delimiter contents delimiter [suffix]
  --
  -- the "contents" part will inherit one of the string categories above.
  --
  -- NOTICE: The Rexx Parser will never assign any of the following
  -- categories to an element. They are provided here for the benefit of
  -- tools like the Rexx Highlighter.
  --

  Call NewElements                      -
    EL.STRING_OPENING_DELIMITER,        - -- "'" or '"'
    EL.STRING_CLOSING_DELIMITER,        - -- "'" or '"'
    EL.STRING_SUFFIX                      -- One of "BXYPGT"

  Call NewSet ALL.STRING_DELIMITERS,    - -- To handle both quotes at once
   .EL.STRING_OPENING_DELIMITER,        -
   .EL.STRING_CLOSING_DELIMITER

  Call NewSet ALL.STRING_COMPONENTS,    - -- To handle all components of all strings
   .ALL.STRINGS,                        -
   .ALL.STRING_DELIMITERS,              -
   .EL.STRING_SUFFIX

  ------------------------------------------------------------------------------
  -- VAR_SYMBOLS                                                              --
  ------------------------------------------------------------------------------
  --
  -- Var_symbol        := general_letter [var_symbol_char+]           (6.2.2.29)
  -- var_symbol_char   := general_letter | digit | '.'                (6.2.2.30)
  --
  Call NewElements                        -
    EL.SIMPLE_VARIABLE,  EL.COMPOUND_VARIABLE,  EL.STEM_VARIABLE
  -- The parser understands the difference between local and exposed variables
  Call NewElements                      -
    EL.EXPOSED_SIMPLE_VARIABLE,         -
    EL.EXPOSED_COMPOUND_VARIABLE,       -
    EL.EXPOSED_STEM_VARIABLE
  Call NewSet ALL.EXPOSED_VARIABLES,    -
   .EL.EXPOSED_SIMPLE_VARIABLE,         -
   .EL.EXPOSED_COMPOUND_VARIABLE,       -
   .EL.EXPOSED_STEM_VARIABLE
  -- All forms of variable
  Call NewSet ALL.VAR_SYMBOLS,          -
    .EL.SIMPLE_VARIABLE,                -
    .EL.COMPOUND_VARIABLE,              -
    .EL.STEM_VARIABLE,                  -
    .EL.EXPOSED_SIMPLE_VARIABLE,        -
    .EL.EXPOSED_COMPOUND_VARIABLE,      -
    .EL.EXPOSED_STEM_VARIABLE
  -- A simple variable, irrespective of whether it is exposed or not
  Call NewSet ALL.SIMPLE_VARIABLES,     -
   .EL.SIMPLE_VARIABLE,                 -
   .EL.EXPOSED_SIMPLE_VARIABLE
  -- A stem variable, irrespective of whether it is exposed or not
  Call NewSet ALL.STEM_VARIABLES,       -
   .EL.STEM_VARIABLE,                   -
   .EL.EXPOSED_STEM_VARIABLE
  -- A compound variable, irrespective of whether it is exposed or not
  Call NewSet ALL.COMPOUND_VARIABLES,   -
    .EL.COMPOUND_VARIABLE,              -
    .EL.EXPOSED_COMPOUND_VARIABLE
  -- Symbols that can be referenced with the ">" and "<" reference operators
  Call NewSet ALL.REFERENCED_SYMBOLS, .EL.SIMPLE_VARIABLE, .EL.STEM_VARIABLE

  ------------------------------------------------------------------------------
  -- Keywords                                                                 --
  ------------------------------------------------------------------------------
  --
  -- The parser marks the appropriate var_symbols symbols as keywords,
  -- or as directive keywords.
  --
  Call NewElements                     -
    EL.KEYWORD,                        -
    EL.SUBKEYWORD,                     -
    EL.DIRECTIVE_KEYWORD,              -
    EL.DIRECTIVE_SUBKEYWORD

  Call NewSet ALL.KEYWORDS,            -
   .EL.KEYWORD,                        -
   .EL.SUBKEYWORD
  Call NewSet ALL.DIRECTIVE_KEYWORDS,  -
   .EL.DIRECTIVE_KEYWORD,              -
   .EL.DIRECTIVE_SUBKEYWORD
  --
  -- An expression is terminated by an end-of-clause, or by a keyword.
  -- The PrepareExpression routine assigns the EL.KEYWORD element category
  -- to the appropriate symbols.
  --
  Call NewSet ALL.EXPRESSION_TERMINATORS, .ALL.KEYWORDS, .EL.END_OF_CLAUSE

  ------------------------------------------------------------------------------
  -- CONST_SYMBOLS                                                            --
  ------------------------------------------------------------------------------
  --
  -- Const_symbol      := (digit | '.') [const_symbol_char+]          (6.2.2.31)
  -- const_symbol_char := var_symbol_char                             (6.2.2.32)
  --                      | EXPONENT_SIGN ('+' | '-')
  --
  -- The definition of a constant symbol is outdated: it is unable to account
  -- for environment symbols, which are not constant. We use the term
  -- "symbol literal" (which mimicks "string literal") for "real" constant
  -- symbols, and the outdated category "CONST_SYMBOLS" to encompass both
  -- symbol literals, periods and environment symbols.
  --
  Call NewElements                        -
    EL.PERIOD,                          -
    EL.SYMBOL_LITERAL,                  -
    EL.ENVIRONMENT_SYMBOL

  Call NewSet ALL.CONST_SYMBOLS,        -
   .EL.PERIOD,                          -
   .EL.SYMBOL_LITERAL,                  -
   .EL.ENVIRONMENT_SYMBOL

  -- Set of possible message scope elements
  Call NewSet ALL.MESSAGE_SCOPE_ELEMENTS, -
   .ALL.VAR_SYMBOLS, .EL.ENVIRONMENT_SYMBOL

  ------------------------------------------------------------------------------
  -- Periods in parsing templates                                             --
  ------------------------------------------------------------------------------
  --
  -- A EL.PERIOD in a parsing template will mutate to become
  -- a EL.PARSE_PERIOD
  --
  Call NewElement    EL.PARSE_PERIOD

  ------------------------------------------------------------------------------
  -- Symbols                                                                  --
  ------------------------------------------------------------------------------
  --
  -- symbol            := VAR_SYMBOL | CONST_SYMBOL | NUMBER          (6.3.2.96)
  --
  -- Var_symbols, const_symbols, and numbers
  --
  Call NewSet ALL.SYMBOLS, .ALL.VAR_SYMBOLS, .ALL.CONST_SYMBOLS, .ALL.NUMBERS

  -- Symbols or keywords
  Call NewSet ALL.SYMBOLS_AND_KEYWORDS, -
   .ALL.SYMBOLS, .ALL.KEYWORDS, .ALL.DIRECTIVE_KEYWORDS

  -- Variables or keywords
  Call NewSet ALL.VARIABLES_AND_KEYWORDS, .ALL.VAR_SYMBOLS, .ALL.KEYWORDS

  ------------------------------------------------------------------------------
  -- TAKEN_CONSTANTS                                                          --
  ------------------------------------------------------------------------------
  --
  -- taken_constant    := symbol | STRING                             (6.2.2.22)
  --
  -- A "taken constant", is "a string or a symbol taken as a constant".
  -- The Rexx parser detects taken constants and assigns them a special
  -- element category, EL.TAKEN_CONSTANT, and, additionally,
  -- an element subcategory.
  --
  -- This subcategory may be very useful in certain highlighting contexts.
  -- For example, it will allow to highlight BIFs, internal routines,
  -- external routines and ::ROUTINE names differently.
  --
  -- We also include some elements that have to be symbols (i.e., they cannot
  -- be strings), like namespace names.
  --
  Call NewElement    EL.TAKEN_CONSTANT

  Drop name value

  --
  -- The first set of constants allow us to model in detail (a) function and
  -- (b) subroutine calls. For both (a) and (b), we should distinguish,
  -- (in the following order, which closely models Rexxref 7.2.1, "Search
  -- order), between:
  --
  -- * Internal calls (i.e., calls that refer to a label local to the current
  --   code body.
  -- * Built-in functions.
  -- * Local ::ROUTINEs, that is, ::ROUTINEs defined in the same package.
  -- * Non local ::ROUTINEs, defined in another package that has been
  --   ::REQUIREd or loaded by an equivalent mechanism.
  --
  --   [Note: since we are not fetching ::REQUIREd packages, we only
  --   recognize as non-local ::ROUTINES those that are namespaced]
  --
  -- * Other routines. These are all external.
  --

  -- Built-in functions
  Call NewConstant BUILTIN.FUNCTION.NAME
  Call NewConstant BUILTIN.SUBROUTINE.NAME

  Call NewSet  BUILTIN.NAME,            -
              .BUILTIN.FUNCTION.NAME,   -
              .BUILTIN.SUBROUTINE.NAME

  -- Internal calls: a locally defined label
  Call NewConstant INTERNAL.FUNCTION.NAME
  Call NewConstant INTERNAL.SUBROUTINE.NAME
  -- ::ROUTINE calls for ::ROUTINEs defined in the same package
  Call NewConstant PACKAGE.FUNCTION.NAME
  Call NewConstant PACKAGE.SUBROUTINE.NAME
  -- Namespace-qualified routine names
  Call NewConstant EXTERNAL.PACKAGE.FUNCTION.NAME
  Call NewConstant EXTERNAL.PACKAGE.SUBROUTINE.NAME
  -- External functions
  Call NewConstant EXTERNAL.FUNCTION.NAME
  Call NewConstant EXTERNAL.SUBROUTINE.NAME

  --
  -- Other names and values
  --

  Call NewConstant ANNOTATION.NAME
  Call NewConstant BLOCK.INSTRUCTION.NAME -- DO, LOOP or SELECT
  Call NewConstant CLASS.NAME
  Call NewConstant ENVIRONMENT.NAME       -- In an ADDRESS instruction
  Call NewConstant LABEL.NAME             -- Labels, and SIGNAL targets
  Call NewConstant NAMESPACE.NAME
  Call NewConstant METHOD.NAME
  Call NewConstant REQUIRES.PROGRAM.NAME
  Call NewConstant RESOURCE.NAME
  Call NewConstant RESOURCE.DELIMITER.NAME
  Call NewConstant ROUTINE.NAME
  Call NewConstant USER.CONDITION.NAME
  Call NewConstant ARGUMENT.NAME          -- For Executor named arguments

  -- Experimental names
  Call NewConstant TABLE.FIELD.NAME       -- The name of a table field

  -- Constant that are not names, but values
  Call NewConstant ANNOTATION.VALUE       -- Not including the optional sign
  Call NewConstant CONSTANT.VALUE         -- Value of a ::CONSTANT (no sign)

  ------------------------------------------------------------------------------
  -- Symbol and string combinations                                           --
  ------------------------------------------------------------------------------
  --
  -- Many contexts expect a symbol or a string
  --
  Call NewSet ALL.SYMBOLS_AND_STRINGS, .ALL.SYMBOLS, .ALL.STRINGS
  --
  -- Constant symbols or strings (see for example the FORWARD instruction)
  Call NewSet ALL.CONSTANT_SYMBOLS_AND_STRINGS, -
   .ALL.STRINGS, .ALL.CONST_SYMBOLS, .ALL.NUMBERS

  ------------------------------------------------------------------------------
  -- Special characters                                                       --
  ------------------------------------------------------------------------------
  --
  -- Special chars, as defined in rexxref 1.10.4.7. Special Characters,
  -- minus "~", which we consider an operator.
  --
  -- Technically, the semicolon belongs here. It is defined above,
  -- under the name EL.END_OF_CLAUSE, because it includes the cases
  -- where a semicolon is implied.
  --
  Call NewElements                      -
    EL.COMMA,                         - -- ","
    EL.COLON,                         - -- ":"
    EL.LEFT_PARENTHESIS,              - -- "("
    EL.RIGHT_PARENTHESIS,             - -- ")"
    EL.LEFT_BRACKET,                  - -- "["
    EL.RIGHT_BRACKET,                 - -- "]"
    EL.LEFT_CURLY_BRACKET,            - -- "{"
    EL.RIGHT_CURLY_BRACKET              -- "}"

  -- We include here the ooRexx-only "::" directive start sequence.
  -- It is the only compound construct made of special characters.
  Call NewElements EL.DIRECTIVE_START

  -- We also include here the ellipsis construct, "...". Technically,
  -- this is an environment symbol (e.g., .environment[".."]), but, when
  -- it appears at the end of an argument list, it has a different meaning.
  Call NewElements EL.ELLIPSIS

  -- Dots appearing inside a compound variable tail (i.e., not the first dot,
  -- which is part of the stem name) work, in fact, as syntactical separators,
  -- like commas in an argument list, but with a different syntax (no blanks
  -- allowed, etc.).
  Call NewElements EL.TAIL_SEPARATOR

  -- The set of left braces
  Call NewSet ALL.LEFT_BRACES,  -
   .EL.LEFT_PARENTHESIS,  .EL.LEFT_BRACKET,  .EL.LEFT_CURLY_BRACKET

  -- The set of right braces
  Call NewSet ALL.RIGHT_BRACES, -
   .EL.RIGHT_PARENTHESIS, .EL.RIGHT_BRACKET, .EL.RIGHT_CURLY_BRACKET

  -- We provide this set, which we don't use, as a convenience.
  -- We don't include .EL.DIRECTIVE_START here, as it normally is
  -- treated (i.e., highlighted) as part of a directive.
  Call NewSet ALL.SPECIAL_CHARS,      -
   .EL.COLON,                         -
   .EL.COMMA,                         -
   .EL.LEFT_PARENTHESIS,              -
   .EL.RIGHT_PARENTHESIS,             -
   .EL.LEFT_BRACKET,                  -
   .EL.RIGHT_BRACKET,                 -
   .EL.END_OF_CLAUSE,                 -
   .EL.TAIL_SEPARATOR

  ------------------------------------------------------------------------------
  -- Operators                                                                --
  ------------------------------------------------------------------------------
  --
  -- Simple operator chars, i.e., 1-character operators, plus "~".
  -- See rexxref 1.10.4.6. Operator Characters and 1.11.3. Parentheses
  -- and Operator Precedence.
  --
  -- 1.10.4.6 also defines "¬", but we do not add it here, because it is
  -- a two-byte character when using the UTF8 encoding.
  --
  Call NewElements                        -
    EL.OP.AND,                          - -- "&"
    EL.OP.NEGATION,                     - -- "\"
    EL.OP.EQUAL,                        - -- "="
    EL.OP.GREATER_THAN,                 - -- ">"
    EL.OP.LOWER_THAN,                   - -- "<"
    EL.OP.MINUS,                        - -- "-"
    EL.OP.OR,                           - -- "|"
    EL.OP.PLUS,                         - -- "+"
    EL.OP.INTEGER_DIVISION,             - -- "%"
    EL.OP.DIVISION,                     - -- "/"
    EL.OP.MULTIPLICATION,               - -- "*"
    EL.OP.MESSAGE                         -- "~"

  -- prefix_expression   := ('+' | '-' | '\') prefix_expression      (6.3.2.112)
  --                        | term | Msg35.1
  --
  Call NewElements                        -
    EL.OP.PREFIX.PLUS,                  - -- "+"
    EL.OP.PREFIX.MINUS                    -- "-"

  --
  -- "<" and ">" appear twice, both as comparison operators,
  -- and as prefix reference operators (see rexxref 1.11.7.
  -- Variable Reference Term).
  --
  Call NewElements                        -
    EL.OP.REFERENCE.GREATER_THAN,       - -- ">"
    EL.OP.REFERENCE.LOWER_THAN            -- "<"

  --
  -- Compound operator ("sequences"):
  -- see rexxref 1.10.4.6. Operator Characters, plus "~~".
  --
  -- Operators using the "¬" character are omitted for the same reasons
  -- detailed above.
  --
  -- Non-strict operators
  --
  Call NewElements                        -
    EL.OP.CASCADING_MESSAGE,            - -- "~~"
    EL.OP.CONCATENATION,                - -- "||"
    EL.OP.GREATER_OR_EQUAL,             - -- ">="
    EL.OP.GREATER_OR_LOWER_THAN,        - -- "><"
    EL.OP.LOWER_OR_EQUAL,               - -- "<="
    EL.OP.LOWER_OR_GREATER_THAN,        - -- "<>"
    EL.OP.NOT_EQUAL,                    - -- "\="
    EL.OP.NOT_GREATER_THAN,             - -- "\>"
    EL.OP.NOT_LOWER_THAN,               - -- "\<"
    EL.OP.POWER,                        - -- "**"
    EL.OP.REMAINDER,                    - -- "//"
    EL.OP.XOR                             -- "&&"
  --
  -- Strict operators
  --
  Call NewElements                        -
   EL.OP.STRICT.EQUAL,                  - --"=="
   EL.OP.STRICT.GREATER_OR_EQUAL,       - --">>="
   EL.OP.STRICT.GREATER_THAN,           - --">>"
   EL.OP.STRICT.LOWER_OR_EQUAL,         - --"<<="
   EL.OP.STRICT.LOWER_THAN,             - --"<<"
   EL.OP.STRICT.NOT_EQUAL,              - --"\=="
   EL.OP.STRICT.NOT_GREATER_THAN,       - --"\>>"
   EL.OP.STRICT.NOT_LOWER_THAN            --"\<<"
  --
  -- In addition to the explicit "||" concatenation operator, Rexx has two
  -- operators more: the blank " " operator and the "" abuttal operator.
  -- Whitespace, in certain contexts, will be transformed
  -- into a " " operator.
  --
  Call NewElements                        -
     EL.OP.BLANK,                       - -- " " (when it is not whitespace)
     EL.OP.ABUTTAL                        -- ""  (always an implicit element)
  --
  -- JLF's Executor has a high-priority abuttal operator
  -- which is automatically inserted when a constant symbol with a syntax of
  -- <number><extra> is found.
  --
  Call NewElement EL.OP.EXECUTOR.ABUTTAL  -- ""  (always implicit)

  ------------------------------------------------------------------------------
  -- Sets of operators                                                        --
  ------------------------------------------------------------------------------
  --
  -- These are used in the recursive descent parsing of expressions
  --

  -- The set of all additive operators ("+" and "-")
  Call NewSet ALL.OPS.ADDITIVE, .EL.OP.PLUS, .EL.OP.MINUS

  -- The set of all multiplicative operators ("*", "/", "%" and "//")
  Call NewSet ALL.OPS.MULTIPLICATIVE,   -
   .EL.OP.MULTIPLICATION,               -
   .EL.OP.DIVISION,                     -
   .EL.OP.INTEGER_DIVISION,             -
   .EL.OP.REMAINDER

  -- The set of all prefix operators ("+", "-", "\")
  -- Prefix .EL.OP.PLUS and .EL.OP.MINUS will be converted to
  -- .EL.OP.PREFIX.PLUS and .EL.OP.PREFIX.MINUS by the expression
  -- parser.
  Call NewSet PREFIX_OPERATORS,         -
   .EL.OP.PLUS, .EL.OP.MINUS, .EL.OP.NEGATION

  -- The set of all concatenation operators ("||", " " and "")
  Call NewSet ALL.OPS.CONCATENATION,    -
    .EL.OP.CONCATENATION, .EL.OP.BLANK, .EL.OP.ABUTTAL

  -- The set of all logical alternative operators ("|" and "&&")
  Call NewSet ALL.OPS.ALTERNATIVE, .EL.OP.OR, .EL.OP.XOR

  -- The set of all logical conjunctive operators ("&")
  Call NewSet ALL.OPS.CONJUNCTIVE, .EL.OP.AND

  -- The set of all logical operators (|, &&, & and \)
  Call NewSet ALL.OPS.LOGICAL, -
   .ALL.OPS.ALTERNATIVE, .EL.OP.AND, .EL.OP.NEGATION

  -- The set of all comparison operators
  Call NewSet ALL.OPS.COMPARISON,       -
   .EL.OP.EQUAL,                        -
   .EL.OP.STRICT.EQUAL,                 -
   .EL.OP.NOT_EQUAL,                    -
   .EL.OP.STRICT.NOT_EQUAL,             -
   .EL.OP.GREATER_OR_LOWER_THAN,        -
   .EL.OP.GREATER_THAN,                 -
   .EL.OP.STRICT.GREATER_THAN,          -
   .EL.OP.NOT_GREATER_THAN,             -
   .EL.OP.STRICT.NOT_GREATER_THAN,      -
   .EL.OP.GREATER_OR_EQUAL,             -
   .EL.OP.STRICT.GREATER_OR_EQUAL,      -
   .EL.OP.LOWER_OR_GREATER_THAN,        -
   .EL.OP.LOWER_THAN,                   -
   .EL.OP.STRICT.LOWER_THAN,            -
   .EL.OP.NOT_LOWER_THAN,               -
   .EL.OP.STRICT.NOT_LOWER_THAN,        -
   .EL.OP.LOWER_OR_EQUAL,               -
   .EL.OP.STRICT.LOWER_OR_EQUAL

  -- The set of all message send operators ("~", "~~", and "[")
  Call NewSet ALL.OPS.MESSAGE_SEND, -
   .EL.LEFT_BRACKET, .EL.OP.MESSAGE, .EL.OP.CASCADING_MESSAGE

  -- The set of all reference operators
  Call NewSet ALL.OPS.REFERENCE,        -
   .EL.OP.REFERENCE.LOWER_THAN,         -
   .EL.OP.REFERENCE.GREATER_THAN

  -- Set of three-characters operators.
  -- This is needed by the parser to determine what element categories
  -- to assign to the constituent characters.
  Call NewSet ALL.OPS.3CHARS,           -
   .EL.OP.STRICT.NOT_EQUAL,             -
   .EL.OP.STRICT.NOT_GREATER_THAN,      -
   .EL.OP.STRICT.GREATER_OR_EQUAL,      -
   .EL.OP.STRICT.NOT_LOWER_THAN,        -
   .EL.OP.STRICT.LOWER_OR_EQUAL

  -- The set of all operators
  Call NewSet ALL.OPERATORS,            -
   .ALL.OPS.ADDITIVE,                   -
   .ALL.OPS.MULTIPLICATIVE,             -
   .ALL.OPS.CONCATENATION,              -
   .ALL.OPS.LOGICAL,                    -
   .ALL.OPS.COMPARISON,                 -
   .ALL.OPS.MESSAGE_SEND,               -
   .ALL.OPS.REFERENCE,                  -
   .EL.OP.POWER

  ------------------------------------------------------------------------------
  -- Assignment operators (standard and compound)                             --
  ------------------------------------------------------------------------------
  --
  -- See rexxref 1.12.5.1. Extended Assignments.
  --
  Call NewElements                        -
    EL.ASG.PLUS,                        - -- "+="
    EL.ASG.MINUS,                       - -- "-="
    EL.ASG.MULTIPLY,                    - -- "*="
    EL.ASG.DIVIDE,                      - -- "/="
    EL.ASG.INTEGER_DIVISION,            - -- "%="
    EL.ASG.AND,                         - -- "&="
    EL.ASG.OR,                          - -- "|="
    EL.ASG.REMAINDER,                   - -- "//="
    EL.ASG.CONCATENATION,               - -- "||="
    EL.ASG.XOR,                         - -- "&&="
    EL.ASG.POWER,                       - -- "**="
    EL.ASG.EQUAL                          -- "=", in an assignment position

  -- The set of all assignment sequences.
  Call NewSet ALL.ASSIGNMENTS,          -
   .EL.ASG.EQUAL,                       -
   .EL.ASG.PLUS,                        -
   .EL.ASG.MINUS,                       -
   .EL.ASG.MULTIPLY,                    -
   .EL.ASG.DIVIDE,                      -
   .EL.ASG.INTEGER_DIVISION,            -
   .EL.ASG.REMAINDER,                   -
   .EL.ASG.CONCATENATION,               -
   .EL.ASG.AND,                         -
   .EL.ASG.OR,                          -
   .EL.ASG.XOR,                         -
   .EL.ASG.POWER

  -- Set of three-characters assignment sequences
  -- This is needed by the parser to determine what element categories
  -- to assign to the constituent characters.
  Call NewSet ALL.3CHARS_ASSIGNMENT_SEQUENCES, -
    .EL.ASG.REMAINDER,                  -
    .EL.ASG.CONCATENATION,              -
    .EL.ASG.XOR,                        -
    .EL.ASG.POWER

  ------------------------------------------------------------------------------
  -- Executor "=" and "==" at the end of a (no)command expression             --
  ------------------------------------------------------------------------------

  Call NewElements EL.EXECUTOR.FINAL.EQUAL, EL.EXECUTOR.FINAL.DOUBLE.EQUAL

  Call NewSet ALL.EXECUTOR.PREFINAL, .EL.OP.EQUAL, .EL.OP.STRICT.EQUAL

  Call NewSet ALL.EXECUTOR.FINAL,       -
    .EL.EXECUTOR.FINAL.EQUAL,           -
    .EL.EXECUTOR.FINAL.DOUBLE.EQUAL

  ------------------------------------------------------------------------------
  -- Continuation characters                                                  --
  ------------------------------------------------------------------------------
  --
  -- Set of continuation characters. A continuation character will be mutated
  -- to the EL.CONTINUATION element category.
  --
  Call NewSet ALL.CONTINUATION_CHARACTERS, .EL.COMMA, .EL.OP.MINUS

  ------------------------------------------------------------------------------
  -- Templates                                                                --
  ------------------------------------------------------------------------------
  --
  -- Set of position prefixes (for PARSE templates)
  --
  Call NewSet ALL.POSITION_PREFIXES,    -
    .EL.OP.EQUAL,                       -
    .EL.OP.PLUS,                        -
    .EL.OP.MINUS,                       -
    .EL.OP.GREATER_THAN,                -
    .EL.OP.LOWER_THAN                   -

  ------------------------------------------------------------------------------
  -- Resources                                                                --
  ------------------------------------------------------------------------------
  --
  Call NewElement EL.RESOURCE_DATA        -- The lines that define the resource
  --
  -- Any data after the end delimiter is ignored. Similarly, data found
  -- in the same logical line, after the resource directive (that is,
  -- after an explicit semicolon) is also ignored (see documentation
  -- bug no. 307).
  --
  Call NewElement EL.RESOURCE_IGNORED_DATA

  ------------------------------------------------------------------------------
  -- Ignore whitespace automatically after...                                 --
  ------------------------------------------------------------------------------
  --
  Call NewSet IGNORE_WHITESPACE_AFTER,  -
    .ALL.OPERATORS,                     - -- An operator (compound or not)
    .ALL.ASSIGNMENTS,                   - -- An (extended) assignment
    .ALL.WHITESPACE_LIKE,               - -- Previous whitespace & continuations
    .EL.END_OF_CLAUSE,                  - -- At the beginning of a clause
    .EL.DIRECTIVE_START,                - -- "::"
    .EL.LEFT_PARENTHESIS,               - -- "("
    .EL.LEFT_BRACKET,                   - -- "["
    .EL.LEFT_CURLY_BRACKET,             - -- "{"
    .EL.COLON,                          - -- A colon ":"
    .EL.COMMA                             -- A comma ","

/******************************************************************************/
/* Keyword-instruction and directive-keyword vocabularies                     */
/* ========================================================================== */
/*                                                                            */
/* Single source of truth for the two closed, dialect-sensitive lists a       */
/* prose mention needs BEFORE it pays for a parse: the keyword instructions   */
/* (so `` `Do`{.rexx} `` colours as a keyword without the parser shouting     */
/* "Incomplete DO") and the directive keywords (so `` `::Method`{.rexx} ``    */
/* is recognised by its `::` and keyword alone). PreClauser builds the very   */
/* same lists in its own init from these routines, so the vocabulary lives    */
/* in ONE place and never drifts between the parser and the prose facade.     */
/*                                                                            */
/* Dialect is read from an options stem, NOT from the global .Features (which */
/* reflect the last parse, unreliable in the prose path that runs before any  */
/* highlighter is instantiated). We accept both index conventions a caller    */
/* may hold: the tail-symbol form (Options.cms / Options.executor, produced   */
/* by ParseOptions) and the string form (Options.["CMS"] / Options.["EXECUTOR"] */
/* set by the block-level language-tag path).                                 */
/*                                                                            */
/* The lists mirror PreClauser.cls exactly:                                   */
/*   - the ANSI base is always present;                                       */
/*   - the ooRexx-only instructions (expose forward guard loop raise reply    */
/*     use) are present unless CMS;                                           */
/*   - the CATCH and FINALLY clauses, and THROW, unless CMS or Executor;     */
/*   - UPPER is present under CMS or Executor;                                */
/*   - the directive EXTENSION is present under Executor.                     */
/*                                                                            */
/* Both return a Set of upper-cased names, so the caller matches with a       */
/* single Upper() on the (trimmed) mention content.                          */

::Routine KeywordInstructionSet Public
  Use Strict Arg options.

  cms      = DialectFlag( options., "cms",      "CMS"      )
  executor = DialectFlag( options., "executor", "EXECUTOR" )

  s = .Set~new

  -- ANSI base (always present). These are the instruction keywords, which
  -- INCLUDE the pure clauses (do end if select when then else otherwise): a
  -- clause keyword is still a keyword for colouring purposes.
  Do kw Over ( "ADDRESS", "ARG", "CALL", "DO", "DROP", "ELSE", "END",     -
               "EXIT", "IF", "INTERPRET", "ITERATE", "LEAVE", "NOP",      -
               "NUMERIC", "OPTIONS", "OTHERWISE", "PARSE", "PROCEDURE",   -
               "PULL", "PUSH", "QUEUE", "RETURN", "SAY", "SELECT",        -
               "SIGNAL", "THEN", "TRACE", "WHEN" )
    s~put( kw )
  End

  -- ooRexx-only (absent under CMS).
  If \cms Then
    Do kw Over ( "EXPOSE", "FORWARD", "GUARD", "LOOP", "RAISE", "REPLY", "USE" )
      s~put( kw )
    End

  -- CATCH and FINALLY (RFE #31): absent under CMS and Executor.
  If \cms, \executor Then Do
    s~put( "CATCH" )
    s~put( "FINALLY" )
    s~put( "THROW" ) -- RFE #31, proposed
  End

  -- Executor and CMS only.
  If cms | executor Then s~put( "UPPER" )

  Return s

::Routine DirectiveKeywordSet Public
  Use Strict Arg options.

  executor = DialectFlag( options., "executor", "EXECUTOR" )

  s = .Set~new

  Do kw Over ( "ANNOTATE", "ATTRIBUTE", "CLASS", "CONSTANT", "METHOD", -
               "OPTIONS", "REQUIRES", "RESOURCE", "ROUTINE" )
    s~put( kw )
  End

  If executor Then s~put( "EXTENSION" )

  Return s

/* DialectFlag -- read a dialect flag from an options stem, tolerating both   */
/* the tail-symbol convention (options.name) and the string-index convention  */
/* (options.["STRINGKEY"]). Returns 1 if either is truthy, 0 otherwise.       */

::Routine DialectFlag Private
  Use Strict Arg options., tailName, stringKey

  If options.~hasIndex( tailName ),  options.tailName == 1       Then Return 1
  If options.~hasIndex( stringKey ), options.[ stringKey ] == 1  Then Return 1
  Return 0
