# The Rexx Parser and Highlighter

The Rexx Parser is a full parser for ooRexx (with options for CMS and the
Executor dialect), written in ooRexx. The Highlighter, built on it,
highlights Rexx programs as HTML, ANSI terminal output or DocBook, using CSS
style sheets; FencedCode highlights the Rexx fenced code blocks of a Markdown
or HTML file. See RFE #878.

Upstream, with its documentation, its history and the RexxPub publishing
framework that uses it: <https://rexx.epbcn.com/rexx-parser/>.

## Usage

```rexx
::Requires "parser.cls"
```

is all a program needs. `parser.cls` is the only file that has to be found in
the search path; it finds everything else in its `parser/` directory, next to
it. The version is `.Rexx.Parser~version` (a release and a refresh date,
`"0.7 20261121"`).

The parser needs ooRexx 5.3.0 r13562 or later (THROW).

## Installation's `bin` Directory

These are the files that should be deployed in installations:

- `parser.cls`          ... `bin` directory or equivalent
- `parser/`             ... a `parser` directory next to `parser.cls`, with
                            its three subdirectories:
  - `parser/HLDrivers/` ... the highlighter's drivers (ANSI, HTML, DocBook)
  - `parser/css/`       ... the style sheets (`rexx-<style>.css`)
  - `parser/resources/` ... the Unicode Character Database files used by the
                            TUTOR-flavoured Unicode support

## Utilities

In `utilities/`, installed in the `bin` directory, next to `rexxtry.rex`:

- `highlight.rex`  ... highlights a Rexx program (ANSI, HTML or DocBook)
- `fencedcode.rex` ... highlights the Rexx fenced code blocks of a Markdown or
                       HTML file
- `rexxcheck.rex`  ... checks a Rexx program for syntax errors, running the
                       parser with all its early checks

In `samples/`, installed with the samples:

- `elements.rex` ... lists the elements of a program, as the parser returns
                     them
- `elident.rex`  ... checks that the elements put back together are the
                     program
- `rxcomp.rex`   ... compares two programs element by element

Their first line, `#!@OOREXX_SHEBANG_PROGRAM@`, is adjusted by the build, as
the one of `rexxtry.rex`.

## ooRexx Test Suite

The tests (ooTest `.testGroup` files: the parser, the highlighter and
FencedCode) go to the `test/trunk/ooRexx/extensions/parser` directory.

## Licenses

The files of this directory are under the Common Public License 1.0, as the
rest of ooRexx, copyright of Josep Maria Blasco and of the Rexx Language
Association, with these exceptions:

- `parser/resources/DerivedName-18.0.0.txt` and
  `parser/resources/NameAliases-18.0.0.txt` are part of the Unicode Character
  Database 18.0.0, Copyright Unicode, Inc., under the Unicode License V3
  (`parser/resources/Unicode-LICENSE.txt`, and
  <https://www.unicode.org/license.txt>).
- The colours of the `parser/css/rexx-vim-*.css` style sheets are taken from
  the Vim colour schemes of the same names, distributed under the Vim license.
