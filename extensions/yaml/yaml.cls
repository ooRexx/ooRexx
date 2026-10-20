/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2024-2026 Rexx Language Association. All rights reserved.    */
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
/** yaml.cls &ndash; Comprehensive YAML 1.2 (core schema) parser/emitter for ooRexx.
 *
 *  <p>Covers the vast majority of real-world YAML:</p>
 *  <ul>
 *    <li>Block mappings and sequences (nested to arbitrary depth)</li>
 *    <li>Flow mappings {&hellip;} and flow sequences [&hellip;] (nested)</li>
 *    <li>Block scalars: literal (|) and folded (&gt;) with chomping (+/-)</li>
 *    <li>Quoted strings: single and double with escape sequences</li>
 *    <li>Plain (unquoted) scalars, including multi-line</li>
 *    <li>Anchors (&amp;name) and aliases (*name)</li>
 *    <li>Merge key (&lt;&lt;) support</li>
 *    <li>Multi-document streams (--- / ...)</li>
 *    <li>Core schema type resolution: null, bool, int, float, string</li>
 *    <li>Comment stripping</li>
 *    <li>Serialisation (ooRexx objects &rarr; YAML string)</li>
 *    <li>XML round-trip via XSD/DTD</li>
 *  </ul>
 *
 *  <p><b>Representation:</b></p>
 *  <table>
 *    <tr><td>mapping</td> <td>&rarr; .Table</td>
 *        <td>sequence</td><td>&rarr; .Array</td></tr>
 *    <tr><td>null</td>    <td>&rarr; .nil</td>
 *        <td>boolean</td> <td>&rarr; .YamlBoolean</td></tr>
 *    <tr><td>integer</td> <td>&rarr; ooRexx String</td>
 *        <td>float</td>   <td>&rarr; ooRexx String</td></tr>
 *    <tr><td>string</td>  <td>&rarr; ooRexx String</td>
 *        <td></td><td></td></tr>
 *  </table>
 *
 *  <p><b>Example 1 &ndash; Build an ooRexx tree and emit YAML:</b></p>
 *  <pre>
 *    doc = .table~new
 *    doc["title"]  = "My Application"
 *    doc["version"] = 2
 *    authors = .array~of("Alice", "Bob")
 *    doc["authors"] = authors
 *    say .Yaml~toYaml(doc)
 *    -- Output:
 *    --   title: My Application
 *    --   version: 2
 *    --   authors:
 *    --     - Alice
 *    --     - Bob
 *
 *    ::requires "yaml.cls"
 *  </pre>
 *
 *  <p><b>Example 2 &ndash; Convert YAML to XML:</b></p>
 *  <pre>
 *    parser = .Yaml~new
 *    doc = parser~parseString("name: Alice" || "0a"x || "age: 30")
 *    xml = .Yaml~yamlToXml(doc)
 *    say xml
 *    -- Output (XSD-valid YAML-in-XML representation):
 *    --   &lt;?xml version="1.0" ...?&gt;
 *    --   &lt;yaml xmlns="..."&gt;
 *    --     &lt;map&gt;
 *    --       &lt;mapping&gt;
 *    --         &lt;scalar&gt;name&lt;/scalar&gt;
 *    --         &lt;scalar&gt;Alice&lt;/scalar&gt;
 *    --       &lt;/mapping&gt;
 *    --       ...
 *    --     &lt;/map&gt;
 *    --   &lt;/yaml&gt;
 *
 *    ::requires "yaml.cls"
 *  </pre>
 *
 *  <p><b>Example 3 &ndash; Convert XML-encoded YAML back to YAML:</b></p>
 *  <pre>
 *    parser = .Yaml~new
 *    doc = parser~parseXml(xmlString)   -- xmlString is the XML from Example 2
 *    yaml = .Yaml~toYaml(doc)
 *    say yaml
 *    -- Output:
 *    --   name: Alice
 *    --   age: 30
 *
 *    ::requires "yaml.cls"
 *  </pre>
 *
 */

/*============================================================================*/
/*  YamlError                                                                 */
/*============================================================================*/

/** Represents an error encountered during YAML parsing.  Instances are raised
 *  via <code>Raise Syntax</code> and carry the error message, source location,
 *  and the offending input line for diagnostic purposes.
 *
 *  @see Yaml
 */
::requires "logicalwrapper.cls"   -- the LogicalWrapper base class (JsonBoolean and YamlBoolean)

::class YamlError public

/** The human-readable error description. */
::attribute message
/** The 1-based line number where the error was detected (0 if unknown). */
::attribute lineNumber
/** The 1-based column where the error was detected (0 if unknown). */
::attribute column
/** The source line that triggered the error (empty string if unavailable). */
::attribute contextLine

/** Creates a new YamlError instance.
 *
 *  @param message      the error description
 *  @param lineNumber   optional 1-based line number (default 0)
 *  @param column       optional 1-based column (default 0)
 *  @param contextLine  optional source line text (default "")
 */
::method init
  expose message lineNumber column contextLine
  use strict arg message, lineNumber = 0, column = 0, contextLine = ""

/** Returns a human-readable representation of the error, including the line
 *  number and context line when available.
 *
 *  @return a formatted error string
 */
::method makeString
  expose message lineNumber column contextLine
  s = "YamlError:" message
  if lineNumber > 0 then s = s "(line" lineNumber")"
  if contextLine \== "" then s = s || "0A"x || "  >" contextLine~strip
  return s

/*============================================================================*/
/*  YamlBoolean                                                               */
/*============================================================================*/

/** Sentinel class for YAML boolean values, a subclass of the class
 *  <code>LogicalWrapper</code> (<code>logicalwrapper.cls</code>), which provides the two instances
 *  (<code>.YamlBoolean~true</code> and <code>.YamlBoolean~false</code>), their
 *  behaviour as <code>1</code> and <code>0</code>, and the comparison methods.
 *  They can be detected with <code>isA(.YamlBoolean)</code> for type-aware
 *  serialisation.
 *
 *  @see Yaml
 *  @see YamlEmitter
 */
::class "YamlBoolean" public subclass LogicalWrapper

/** Renders this object as a YAML string representing its logical value.
 *
 *  @return <code>"true"</code> or <code>"false"</code>
 */
::method makeYAML unguarded
  if self~value then return "true"
  return "false"

/*============================================================================*/
/*  YamlTagged                                                                */
/*============================================================================*/

/** Wraps a YAML value together with its explicit tag string.
 *
 *  <p>When the parser's <code>preserveTags</code> option is enabled,
 *  any node that carries an explicit tag in the source YAML (e.g.
 *  <code>!!str</code>, <code>!custom</code>, <code>!&lt;verbatim&gt;</code>)
 *  is represented as a <code>YamlTagged</code> instance instead of a bare
 *  value.  Nodes without tags are unaffected.
 *
 *  <p>The <code>value</code> attribute holds the resolved ooRexx object
 *  (string, integer, .Table, .Array, .YamlBoolean, or .nil) and the
 *  <code>tag</code> attribute holds the original tag text.
 *
 *  @see Yaml
 */
::class YamlTagged public

/** Creates a new YamlTagged wrapper.
 *
 *  @param tag   the tag string (e.g. <code>"!!str"</code>,
 *               <code>"!custom"</code>, <code>"!&lt;tag:yaml.org,2002:str&gt;"</code>)
 *  @param value the resolved ooRexx value
 */
::method init
  expose tag value
  use strict arg tag, value

/** Returns the tag string.
 *
 *  @return the tag string
 */
::attribute tag get

/** Returns the wrapped value.
 *
 *  @return the resolved ooRexx value
 */
::attribute value get

/** Returns the string representation of the wrapped value, so that
 *  <code>YamlTagged</code> can be used transparently in string contexts.
 *
 *  @return the string representation of the value
 */
::method makeString
  expose value
  if value == .nil then return ""
  return value~string

/*============================================================================*/
/*  Yaml                                                                      */
/*============================================================================*/

/** Comprehensive YAML 1.2 (core schema) parser and emitter for ooRexx.
 *
 *  <p>Parsing is performed through instance methods on a <code>Yaml</code>
 *  object; serialisation is provided as class methods.
 *
 *  <p><b>Representation model:</b>
 *  <pre>
 *    mapping  &rarr; .Table       sequence &rarr; .Array
 *    null     &rarr; .nil               boolean  &rarr; .YamlBoolean
 *    integer  &rarr; ooRexx String      float    &rarr; ooRexx String
 *    string   &rarr; ooRexx String
 *  </pre>
 *
 *  <p><b>Quick start:</b>
 *  <pre>
 *    parser = .Yaml~new
 *    doc    = parser~parseFile("config.yml")
 *    say doc["title"]
 *    yaml   = .Yaml~toYaml(doc)
 *  </pre>
 *
 *  @see YamlBoolean
 *  @see YamlEmitter
 *  @see YamlXmlEmitter
 *  @see YamlXmlParser
 */
::class Yaml public

/** Returns the <code>.YamlBoolean</code> proxy for <code>.true</code>.
 *  It can be used interchangeably with ooRexx' <code>.true</code> or
 *  <code>1</code> values.
 *
 *  @return the YamlBoolean value for <code>.true</code>
 */
::attribute true  get class unguarded
  return .YamlBoolean~true

/** Returns the <code>.YamlBoolean</code> proxy for <code>.false</code>.
 *  It can be used interchangeably with ooRexx' <code>.false</code> or
 *  <code>0</code> values.
 *
 *  @return the YamlBoolean value for <code>.false</code>
 */
::attribute false get class unguarded
  return .YamlBoolean~false

/* Class-level lookup tables (internal) */
::attribute escMap    class
::attribute boolTrue  class
::attribute boolFalse class
::attribute nullWords class

/** Initialises the class-level lookup tables for escape sequences, boolean
 *  words, and null words according to the YAML 1.2 core schema.
 */
::method init class
  expose escMap boolTrue boolFalse nullWords

  /* Double-quote escape map (YAML 1.2 sec. 5.7) */
  escMap = .table~new
  escMap["0"] = "00"x
  escMap["a"] = "07"x
  escMap["b"] = "08"x
  escMap["t"] = "09"x
  escMap["09"x] = "09"x              -- \<TAB> is also valid (spec [45])
  escMap["n"] = "0A"x
  escMap["v"] = "0B"x
  escMap["f"] = "0C"x
  escMap["r"] = "0D"x
  escMap["e"] = "1B"x
  escMap[" "] = " "
  escMap['"'] = '"'
  escMap["/"] = "/"
  escMap["\"] = "\"
  /* YAML 1.2 sec. 5.7 — Unicode escape shortcuts */
  escMap["N"] = "C285"x          -- U+0085 Next Line
  escMap["_"] = "C2A0"x          -- U+00A0 No-Break Space
  escMap["L"] = "E280A8"x        -- U+2028 Line Separator
  escMap["P"] = "E280A9"x        -- U+2029 Paragraph Separator

  boolTrue = .table~new
  do w over .array~of("true","True","TRUE","yes","Yes","YES","on","On","ON")
    boolTrue[w] = 1
  end

  boolFalse = .table~new
  do w over .array~of("false","False","FALSE","no","No","NO","off","Off","OFF")
    boolFalse[w] = 1
  end

  nullWords = .table~new
  do w over .array~of("null","Null","NULL","~")
    nullWords[w] = 1
  end

/** Creates a new Yaml parser instance.  Each instance maintains its own
 *  parsing state, including anchor and merge-source metadata from the
 *  most recent parse operation.
 *
 *  @param unescapeUnicode  optional flag controlling Unicode escape handling.
 *                          When <code>.true</code> (the default),
 *                          <code>\uXXXX</code> and <code>\UXXXXXXXX</code>
 *                          escape sequences in double-quoted strings are
 *                          converted to their UTF-8 byte equivalents.  When
 *                          <code>.false</code>, they are stored literally
 *                          (e.g. as the six characters
 *                          <code>\u00E9</code>).  Use <code>.false</code>
 *                          when exact byte-level round-tripping of escape
 *                          sequences is required.
 *  @param preserveTags     optional flag controlling tag preservation.
 *                          When <code>.true</code>, nodes with explicit tags
 *                          are wrapped in <code>YamlTagged</code> objects
 *                          that carry both the tag string and the resolved
 *                          value.  When <code>.false</code> (the default),
 *                          tags are stripped and only the resolved value is
 *                          stored (the v8o behaviour).
 */
::method init
  expose lines pos anchors anchorMap mergeSourceMap unescapeUnicode preserveTags directivesMap currentDirectives flowMinIndent curLine lineOffset
  use strict arg unescapeUnicode = .true, preserveTags = .false
  lineOffset        = 0
  lines             = .array~new
  pos               = 1
  curLine           = 0
  anchors           = .table~new
  anchorMap         = .identityTable~new
  mergeSourceMap    = .identityTable~new
  directivesMap     = .identityTable~new
  currentDirectives = .nil
  flowMinIndent     = 0

/** Returns the anchor map from the most recent parse.  The map is an
 *  <code>.IdentityTable</code> that associates each anchored ooRexx object
 *  with its anchor name.  Pass this to the <code>toYaml*</code> methods to
 *  preserve anchors and aliases during round-tripping.
 *
 *  @return an .IdentityTable mapping objects to anchor name strings
 */
::attribute anchorMap get

/** Returns the merge-source map from the most recent parse.  The map is an
 *  <code>.IdentityTable</code> that associates each target mapping
 *  (<code>.Table</code>) that contained merge keys
 *  (<code>&lt;&lt;</code>) with an <code>.Array</code> of the source mapping
 *  objects that were merged into it.  Pass this to the
 *  <code>toYaml*</code> methods to reconstruct merge keys during
 *  round-tripping.
 *
 *  @return an .IdentityTable mapping target Tables to Arrays of
 *          source Tables
 */
::attribute mergeSourceMap get

/** Returns the directives map from the most recent parse.  The map is an
 *  <code>.IdentityTable</code> that associates each parsed document's root
 *  object with its directives <code>.Table</code>.  The directives table
 *  has two optional entries: <code>"yamlVersion"</code> (a string like
 *  <code>"1.2"</code>) and <code>"tagHandles"</code> (a <code>.Table</code>
 *  mapping handle strings like <code>"!!"</code> to URI prefixes).
 *  Documents with no directives are not present in the map.  Pass this to
 *  the <code>toYaml*</code> methods to preserve directives during
 *  round-tripping.
 *
 *  @return an .IdentityTable mapping root objects to directives Tables
 */
::attribute directivesMap get

/*============================================================================*/
/*  PUBLIC PARSING API                                                        */
/*============================================================================*/

/** Parses a single YAML document from a string.  If the input contains
 *  multiple documents, only the first one is parsed.
 *
 *  @param input a YAML string (may be a multi-line string or an .Array of
 *               lines)
 *  @return the ooRexx object tree representing the parsed document
 */
::method parseString
  expose lines pos anchors anchorMap
  use strict arg input
  self~loadInput(input)
  return self~parseOneDoc

/** Parses a single YAML document from a file.
 *
 *  @param path the file system path to read
 *  @return the ooRexx object tree representing the parsed document
 */
::method parseFile
  use strict arg path
  return self~parseString(self~readFileLines(path))

/** Parses all YAML documents from a multi-document string (separated by
 *  <code>---</code> document-start markers).
 *
 *  @param input a YAML string containing one or more documents
 *  @return an .Array of ooRexx object trees, one per document
 */
::method parseAll
  expose lines pos anchors anchorMap
  use strict arg input
  self~loadInput(input)
  return self~parseAllDocs

/** Parses all YAML documents from a multi-document file.
 *
 *  @param path the file system path to read
 *  @return an .Array of ooRexx object trees, one per document
 */
::method parseAllFile
  use strict arg path
  return self~parseAll(self~readFileLines(path))

/** Extracts and parses the YAML front-matter block from a text that uses
 *  the <code>---</code> / <code>---</code> convention (e.g. Jekyll,
 *  Hugo, Markdown documents).  The line numbers of parse errors are lines
 *  of the whole <code>input</code>, where the opening <code>---</code> is
 *  line 1.
 *
 *  @param input the full text containing front-matter
 *  @return the ooRexx object tree of the front-matter, or <code>.nil</code>
 *          if no front-matter block is found
 */
::method parseFrontMatter
  expose lineOffset
  use strict arg input
  yamlLines = self~extractFM(input)
  if yamlLines == .nil then return .nil
  self~loadInput(yamlLines)
  lineOffset = 1                /* the opening "---" is line 1 */
  return self~parseOneDoc

/** Extracts and parses YAML front-matter from a file.
 *
 *  @param path the file system path to read
 *  @return the ooRexx object tree of the front-matter, or <code>.nil</code>
 *          if no front-matter block is found
 */
::method parseFrontMatterFile
  use strict arg path
  return self~parseFrontMatter(self~readFileLines(path))

/** Parses a single YAML document from an <code>.Array</code> of lines.
 *
 *  @param anArray an .Array where each item is one line of YAML text
 *  @return the ooRexx object tree representing the parsed document
 */
::method parseArray
  use strict arg anArray
  if \anArray~isA(.array) then
    raise syntax 93.900 additional("parseArray requires an Array argument")
  return self~parseString(anArray)

/*============================================================================*/
/*  PUBLIC SERIALISATION (class methods)                                      */
/*============================================================================*/

/** Serialises an ooRexx object tree to a YAML string.
 *
 *  @param obj             the ooRexx object tree to serialise
 *  @param indentSize      optional number of spaces per indentation level
 *                         (default 2)
 *  @param anchorMap       optional .IdentityTable from a previous parse to
 *                         preserve anchors/aliases
 *  @param mergeSourceMap  optional .IdentityTable from a previous parse to
 *                         reconstruct merge keys
 *  @param directivesMap   optional .IdentityTable from a previous parse to
 *                         preserve YAML directives (%YAML, %TAG)
 *  @return a YAML-formatted string
 */
::method toYaml class
  use strict arg obj, indentSize = 2, anchorMap = .nil, mergeSourceMap = .nil, directivesMap = .nil
  return .YamlEmitter~new(indentSize, anchorMap, mergeSourceMap, directivesMap)~emit(obj)

/** Serialises an array of ooRexx object trees to a multi-document YAML
 *  string, with each document preceded by a <code>---</code> marker.
 *
 *  @param docs            an .Array of ooRexx object trees
 *  @param indentSize      optional indentation size (default 2)
 *  @param anchorMap       optional anchor map
 *  @param mergeSourceMap  optional merge-source map
 *  @param directivesMap   optional directives map
 *  @return a multi-document YAML string
 */
::method toYamlAll class
  use strict arg docs, indentSize = 2, anchorMap = .nil, mergeSourceMap = .nil, directivesMap = .nil
  emitter = .YamlEmitter~new(indentSize, anchorMap, mergeSourceMap, directivesMap)
  parts = .array~new
  do doc over docs
    /* Only emit bare --- separator if emitter did not produce directives */
    emitted = emitter~emit(doc)
    if emitted~left(1) \== "%" then
      parts~append("---")
    parts~append(emitted)
  end
  return parts~makeString('L', "0A"x)

/** Serialises an ooRexx object tree and writes it to a YAML file using
 *  platform-independent LF line endings.
 *
 *  @param obj             the ooRexx object tree to serialise
 *  @param path            the file system path to write
 *  @param indentSize      optional indentation size (default 2)
 *  @param anchorMap       optional anchor map
 *  @param mergeSourceMap  optional merge-source map
 *  @param directivesMap   optional directives map
 */
::method toYamlFile class
  use strict arg obj, path, indentSize = 2, anchorMap = .nil, mergeSourceMap = .nil, directivesMap = .nil
  arr = self~toYamlArray(obj, indentSize, anchorMap, mergeSourceMap, directivesMap)
  s = .stream~new(path)~~open("WRITE REPLACE")
  if s~state \== "READY" then
    raise syntax 93.900 additional("Cannot open file for writing:" path)
  s~charOut(arr~makeString('L', "0A"x))
  s~close

/** Serialises an ooRexx object tree to an <code>.Array</code> of YAML
 *  lines (one element per line, without trailing newlines).
 *
 *  @param obj             the ooRexx object tree to serialise
 *  @param indentSize      optional indentation size (default 2)
 *  @param anchorMap       optional anchor map
 *  @param mergeSourceMap  optional merge-source map
 *  @param directivesMap   optional directives map
 *  @return an .Array of strings, one per YAML output line
 */
::method toYamlArray class
  use strict arg obj, indentSize = 2, anchorMap = .nil, mergeSourceMap = .nil, directivesMap = .nil
  yamlStr = self~toYaml(obj, indentSize, anchorMap, mergeSourceMap, directivesMap)
  return yamlStr~strip("T", "0A"x)~makeArray("0A"x)

/** Serialises an ooRexx object tree and writes it to a file wrapped in
 *  front-matter delimiters (<code>---</code> before and after), suitable for
 *  Markdown/Jekyll/Hugo documents.
 *
 *  @param obj             the ooRexx object tree to serialise
 *  @param path            the file system path to write
 *  @param indentSize      optional indentation size (default 2)
 *  @param anchorMap       optional anchor map
 *  @param mergeSourceMap  optional merge-source map
 *  @param directivesMap   optional directives map
 */
::method toYamlFMFile class
  use strict arg obj, path, indentSize = 2, anchorMap = .nil, mergeSourceMap = .nil, directivesMap = .nil
  arr = self~toYamlArray(obj, indentSize, anchorMap, mergeSourceMap, directivesMap)
  s = .stream~new(path)~~open("WRITE REPLACE")
  if s~state \== "READY" then
    raise syntax 93.900 additional("Cannot open file for writing:" path)
  mb = .mutableBuffer~new
  mb~append("---", "0A"x)
  mb~append(arr~makeString('L', "0A"x))
  mb~append("0A"x, "---", "0A"x)
  s~charOut(mb~string)
  s~close

/*============================================================================*/
/*  PUBLIC XML SERIALISATION (class methods)                                  */
/*============================================================================*/

/** Serialises an ooRexx object tree to an XML string conforming to either
 *  <code>yaml.xsd</code> (with namespace) or <code>yaml.dtd</code>
 *  (with DOCTYPE).
 *
 *  @param obj             the ooRexx object tree to serialise
 *  @param schema          optional schema type: <code>"xsd"</code> (default) or
 *                         <code>"dtd"</code>
 *  @param anchorMap       optional anchor map from a previous parse
 *  @param directivesMap   optional directives map from a previous parse
 *  @param mergeSourceMap  optional merge-source map from a previous parse
 *  @return an XML string
 */
::method yamlToXml class
  use strict arg obj, schema = "xsd", anchorMap = .nil, directivesMap = .nil, mergeSourceMap = .nil
  return .YamlXmlEmitter~new(schema, anchorMap, directivesMap, mergeSourceMap)~emit(obj)

/** Serialises an ooRexx object tree to XML and writes it to a file.
 *
 *  @param obj             the ooRexx object tree to serialise
 *  @param path            the file system path to write
 *  @param schema          optional schema type: <code>"xsd"</code> (default) or
 *                         <code>"dtd"</code>
 *  @param anchorMap       optional anchor map from a previous parse
 *  @param directivesMap   optional directives map from a previous parse
 *  @param mergeSourceMap  optional merge-source map from a previous parse
 */
::method yamlToXmlFile class
  use strict arg obj, path, schema = "xsd", anchorMap = .nil, directivesMap = .nil, mergeSourceMap = .nil
  xml = self~yamlToXml(obj, schema, anchorMap, directivesMap, mergeSourceMap)
  s = .stream~new(path)~~open("WRITE REPLACE")
  if s~state \== "READY" then
    raise syntax 93.900 additional("Cannot open file for writing:" path)
  s~charOut(xml)
  s~close

/*============================================================================*/
/*  PUBLIC XML PARSING (instance methods)                                     */
/*============================================================================*/

/** Parses an XML string (conforming to <code>yaml.xsd</code> or
 *  <code>yaml.dtd</code>) back into an ooRexx object tree.
 *
 *  @param input an XML string or an .Array of lines
 *  @return the ooRexx object tree
 */
::method parseXml
  expose directivesMap mergeSourceMap anchorMap
  use strict arg input
  select
    when input~isA(.string) then
      t = input~changeStr("0D0A"x, "0A"x)~changeStr("0D"x, "0A"x)
    when input~isA(.array) then
      t = input~makeString('L', "0A"x)
    otherwise
      raise syntax 93.900 additional("parseXml: input must be a String or Array")
  end
  xmlParser = .YamlXmlParser~new
  node = xmlParser~parse(t)
  /* Merge XML parser's directivesMap into ours */
  xmlDirMap = xmlParser~directivesMap
  if xmlDirMap \== .nil then do
    sup = xmlDirMap~supplier
    do while sup~available
      directivesMap[sup~index] = sup~item
      sup~next
    end
  end
  /* Merge XML parser's mergeSourceMap into ours */
  xmlMsm = xmlParser~mergeSourceMap
  if xmlMsm \== .nil then do
    sup = xmlMsm~supplier
    do while sup~available
      mergeSourceMap[sup~index] = sup~item
      sup~next
    end
  end
  /* Build anchorMap from XML parser's xmlAnchors (name→object → object→name) */
  xmlAnch = xmlParser~xmlAnchors
  if xmlAnch \== .nil then do
    sup = xmlAnch~supplier
    do while sup~available
      anchorMap[sup~item] = sup~index
      sup~next
    end
  end
  return node

/** Parses an XML file (conforming to <code>yaml.xsd</code> or
 *  <code>yaml.dtd</code>) back into an ooRexx object tree.
 *
 *  @param path the file system path to read
 *  @return the ooRexx object tree
 */
::method parseXmlFile
  use strict arg path
  s = .stream~new(path)~~open("READ")
  if s~state \== "READY" then
    raise syntax 93.900 additional("Cannot open file:" path)
  arr = s~arrayIn
  s~close
  return self~parseXml(arr)

/*============================================================================*/
/*  INPUT LOADING (private)                                                   */
/*============================================================================*/

/** Initialises parser state for a new parse operation.
 *  Normalises line endings (CRLF/CR to LF), splits input into lines,
 *  and resets the position cursor, anchors table, anchorMap, and
 *  mergeSourceMap.
 *
 *  @param input a .String or .Array of lines to parse
 */
::method loadInput private
  expose lines pos anchors anchorMap mergeSourceMap curLine lineOffset
  use strict arg input
  select
    when input~isA(.array) then lines = input~copy
    when input~isA(.string) then do
      t = input~changeStr("0D0A"x, "0A"x)~changeStr("0D"x, "0A"x)
      lines = t~makeArray("0A"x)
    end
    otherwise
      raise syntax 93.900 additional("Input must be a String or Array")
  end
  pos            = 1
  curLine        = 0
  lineOffset     = 0
  anchors        = .table~new
  anchorMap      = .identityTable~new
  mergeSourceMap = .identityTable~new

/** Reads the contents of a file into an .Array of lines.
 *
 *  @param path the file path to read
 *  @return an .Array of lines
 */
::method readFileLines
  use strict arg path
  s = .stream~new(path)~~open("READ")
  if s~state \== "READY" then
    raise syntax 93.900 additional("Cannot open file:" path)
  arr = s~arrayIn
  s~close
  return arr

/** Extracts the YAML front-matter block from a document.
 *  Front matter is delimited by a leading '---' and a closing
 *  '---' or '...'.  Returns the YAML lines (without delimiters)
 *  as an .Array, or .nil if no valid front matter is found.
 *
 *  @param input a .String or .Array containing the full document
 *  @return an .Array of YAML lines, or .nil
 */
::method extractFM private
  use strict arg input
  select
    when input~isA(.array) then allLines = input
    when input~isA(.string) then do
      t = input~changeStr("0D0A"x, "0A"x)~changeStr("0D"x, "0A"x)
      allLines = t~makeArray("0A"x)
    end
    otherwise
      raise syntax 93.900 additional("Input must be a String or Array")
  end
  if allLines~items = 0 then return .nil
  if allLines[1]~strip \== "---" then return .nil
  yamlLines = .array~new
  do i = 2 to allLines~items
    stripped = allLines[i]~strip
    if stripped == "---" | stripped == "..." then return yamlLines
    yamlLines~append(allLines[i])
  end
  return .nil

/*============================================================================*/
/*  DOCUMENT HANDLING (private)                                               */
/*============================================================================*/

/** Parses a single YAML document from the current position.
 *  Handles directives, the document-start marker, block node
 *  parsing, and the document-end marker.  Stores any directives
 *  in directivesMap keyed by the returned document object.
 *
 *  @return the parsed document (a .Table, .Array, or scalar)
 */
::method parseOneDoc private
  expose lines pos directivesMap currentDirectives
  directives = self~parseDirectives
  currentDirectives = directives
  docStartResidual = self~skipDocStart
  if docStartResidual \== .false, docStartResidual \== "" then
    doc = self~blockNode(-1)
  else
    doc = self~blockNode(0)
  self~consumeDocEnd
  if doc == .nil then doc = .table~new
  if directives \== .nil then directivesMap[doc] = directives
  return doc

/** Parses all YAML documents from the current input.
 *  Iterates through the input, consuming document-start and
 *  document-end markers, directives, and block nodes.  Enforces
 *  the requirement for '...' before directives between documents.
 *
 *  @return an .Array of parsed documents
 */
::method parseAllDocs private
  expose lines pos directivesMap currentDirectives
  docs = .array~new
  hadDocEnd = .false
  do while pos <= lines~items
    self~skipBlanksOnly
    if pos > lines~items then leave
    savedPos = pos
    directives = self~parseDirectives
    currentDirectives = directives
    /* Directives require an explicit document-start marker '---' */
    if directives \== .nil then do
      /* Between documents, directives require a preceding '...'
         document-end marker (YAML 1.2, §9.2).  Without it, the
         directive is ambiguous — it could be scalar content. */
      if docs~items > 0 & \hadDocEnd then
        self~raiseError("Missing document-end marker '...' before directive", pos)
      self~skipBlanksOnly
      hasDocStart = .false
      if pos <= lines~items then do
        peek = lines[pos]~strip
        if peek~left(3) == "---" then do
          if peek~length == 3 then hasDocStart = .true
          else if peek[4] == " " | peek[4] == "09"x then hasDocStart = .true
        end
      end
      if \hasDocStart then do
        /* If this is the first document, it's a real error.
           Otherwise, the "directive" might be scalar content that
           blockNode didn't consume — revert and stop parsing. */
        if docs~items == 0 then
          self~raiseError("Directive without document start marker '---'", pos)
        else do
          pos = savedPos
          leave  /* stop multi-doc loop — remaining content is ambiguous */
        end
      end
    end
    /* Check for trailing content: if we already have a document,
       there was no document-end marker '...', no directives, and the
       current line is not a document-start marker '---', then this is
       invalid trailing content after the document. */
    if docs~items > 0 & \hadDocEnd & directives == .nil then do
      hasDocStart = .false
      if pos <= lines~items then do
        peek = lines[pos]~strip
        if peek~left(3) == "---" then do
          if peek~length == 3 then hasDocStart = .true
          else if peek[4] == " " | peek[4] == "09"x then hasDocStart = .true
        end
      end
      if \hasDocStart then
        self~raiseError("Invalid trailing content after document:" lines[pos]~strip, pos)
    end
    docStartResidual = self~skipDocStart
    /* When --- has content on the same line (e.g. "--- >", "--- !tag"),
       the residual is placed at indent 0.  Block scalars and other
       constructs need parentIndent = -1 so that indent 0 content is
       accepted as belonging to the document root. */
    if docStartResidual \== .false, docStartResidual \== "" then
      doc = self~blockNode(-1)
    else
      doc = self~blockNode(0)
    if doc == .nil then doc = .table~new
    if directives \== .nil then directivesMap[doc] = directives
    docs~append(doc)
    hadDocEnd = self~consumeDocEnd
  end
  if docs~items = 0 then docs~append(.table~new)
  return docs

/** Skips a document-start marker ('---') if present.
 *  If the marker has trailing content (e.g. '--- !tag'), the
 *  residual is left in the lines array for the caller to process.
 *
 *  @return "" if bare '---' consumed, the residual string if
 *          '---' had trailing content, or .false if no marker found
 */
::method skipDocStart private
  expose lines pos
  self~skipBlanksOnly
  if pos <= lines~items then do
    s = lines[pos]~strip
    if s == "---" then do
      pos = pos + 1
      return ""
    end
    else if s~left(4) == "--- " | s~left(4) == "---"||"09"x then do
      /* Content after ---: replace the line with just the residual */
      residual = s~substr(4)~strip
      lines[pos] = residual
      return residual
    end
  end
  return .false

/** Parses %YAML and %TAG directives before a document-start marker.
 *  Returns a .Table with "yamlVersion" and/or "tagHandles" entries
 *  if any directives were found, or .nil if none were present.
 */
::method parseDirectives private
  expose lines pos
  yamlVersion = .nil
  tagHandles  = .nil
  self~skipBlanksOnly
  do while pos <= lines~items
    stripped = lines[pos]~strip
    if stripped == "" | stripped~left(1) == "#" then do
      pos = pos + 1; iterate
    end
    if stripped~left(6) == "%YAML " then do
      if yamlVersion \== .nil then
        self~raiseError("Duplicate %YAML directive", pos)
      versionStr = stripped~substr(7)~strip
      /* Strip comment (must be preceded by space) */
      sp = versionStr~pos(" ")
      if sp > 0 then do
        after = versionStr~substr(sp + 1)~strip
        if after~left(1) == "#" then
          versionStr = versionStr~left(sp - 1)
        else
          self~raiseError("Extra content on %YAML directive:" stripped, pos)
      end
      /* Validate no '#' without preceding space */
      if versionStr~pos("#") > 0 then
        self~raiseError("Invalid %YAML directive:" stripped, pos)
      yamlVersion = versionStr
      pos = pos + 1; iterate
    end
    if stripped~left(5) == "%TAG " then do
      rest = stripped~substr(6)~strip
      sp = rest~pos(" ")
      if sp > 0 then do
        handle = rest~left(sp - 1)
        prefix = rest~substr(sp + 1)~strip
        if tagHandles == .nil then tagHandles = .table~new
        tagHandles[handle] = prefix
      end
      pos = pos + 1; iterate
    end
    /* Any other % directive — skip (unknown directive) */
    if stripped~left(1) == "%" then do
      pos = pos + 1; iterate
    end
    leave
  end
  /* Build result */
  if yamlVersion == .nil, tagHandles == .nil then return .nil
  dirTable = .table~new
  if yamlVersion \== .nil then dirTable["yamlVersion"] = yamlVersion
  if tagHandles \== .nil then dirTable["tagHandles"] = tagHandles
  return dirTable

/** Skips blank lines and comment lines only (not directives).
 *  Used before parseDirectives to avoid consuming directives
 *  in skipBlanks.
 */
::method skipBlanksOnly private
  expose lines pos
  do while pos <= lines~items
    stripped = lines[pos]~strip
    if stripped == "" | stripped~left(1) == "#" then pos = pos + 1
    else leave
  end

/** Consumes a document-end marker ('...') if present.
 *  Also validates that no non-comment content follows the marker
 *  on the same line.
 *
 *  @return .true if a document-end marker was consumed, .false otherwise
 */
::method consumeDocEnd private
  expose lines pos
  self~skipBlanksOnly
  if pos <= lines~items then do
    s = lines[pos]~strip
    if s~left(3) == "..." then do
      if s~length == 3 then do
        pos = pos + 1
        return .true
      end
      else if s~substr(4,1) == " " | s~substr(4,1) == "09"x then do
        rest = s~substr(4)~strip
        if rest == "" | rest~left(1) == "#" then do
          pos = pos + 1
          return .true
        end
        else
          self~raiseError("Invalid content after document end marker:" s, pos)
      end
      /* else: ...x style — not a doc-end marker, leave it */
    end
  end
  return .false

/*============================================================================*/
/*  BLOCK NODE DISPATCH (private)                                             */
/*============================================================================*/

/** Main dispatch method for parsing a YAML block node.
 *  Examines the current line to determine the node type (anchor,
 *  alias, tag, mapping, sequence, flow collection, or scalar) and
 *  delegates to the appropriate specialised method.
 *
 *  @param minIndent minimum indentation required for this node;
 *         -1 indicates content on the document-start line
 *  @param scalarParentIndent optional parent indent passed to
 *         block scalar parsing for correct indentation detection
 *  @return the parsed node (.Table, .Array, scalar, or .nil)
 */
::method blockNode private
  expose lines pos anchors anchorMap flowMinIndent curLine
  use strict arg minIndent, scalarParentIndent = (.nil)

  self~skipBlanks
  if pos > lines~items then return .nil

  line    = lines[pos]
  indent  = self~indentOf(line)
  if indent < minIndent then return .nil
  curLine = pos
  content = line~strip

  /* Anchor prefix */
  if content~left(1) == "&" then do
    parse var content aTag rest
    aName = aTag~substr(2)
    /* An alias cannot be the target of an anchor (§7.1) */
    if rest~strip \== "", rest~strip~left(1) == "*" then
      self~raiseError("An alias cannot be anchored")
    /* Block indicators (- ? :) cannot follow an anchor on the same
       line — they must appear at the start of a line (after
       indentation only). §8.2.1 */
    if rest~strip \== "" then do
      r = rest~strip
      if r~left(2) == "- " | r == "-" | -
         (r~left(1) == "-" & r~length > 1 & r[2] == "09"x) then
        self~raiseError("Block sequence indicator cannot follow anchor on same line")
    end
    lines[pos] = copies(" ", indent) || rest~strip
    if rest~strip == "" then do
      pos = pos + 1
      self~skipBlanks
      if pos > lines~items then do
        anchors[aName] = .nil
        return .nil
      end
      /* We moved past the original line; the doc-start-line restriction
         (minIndent = -1) no longer applies to the new line. */
      if minIndent < 0 then minIndent = 0
    end
    anchors[aName] = .nil              /* pre-register for forward refs */
    node = self~blockNode(minIndent, scalarParentIndent)
    /* Reject double anchor: if the recursive call already anchored
       this same scalar node, two anchors target it (§6.9.2).
       For collections, a sub-node anchor is legitimate. */
    if node \== .nil, anchorMap~hasIndex(node), -
       \node~isA(.table), \node~isA(.array) then
      self~raiseError("Only one anchor is allowed per node")
    anchors[aName] = node              /* update with actual value      */
    if node \== .nil then anchorMap[node] = aName
    return node
  end

  /* Alias */
  if content~left(1) == "*" then do
    aName = self~extractAliasName(content~substr(2))
    rest  = content~substr(aName~length + 2)~strip
    /* Strip trailing comment */
    if rest == "" | rest~left(1) == "#" then do
      pos = pos + 1
      return self~getAnchor(aName)
    end
    /* Alias used as mapping key: *alias : value → route to blockMap */
    if rest~left(2) == ": " | rest == ":" then do
      node = self~blockMap(indent)
      return node
    end
    /* Just an alias with trailing content — treat as alias */
    pos = pos + 1
    return self~getAnchor(aName)
  end

  /* Tag prefix — strip and ignore (core schema only) */
  tagStr = ""
  if content~left(1) == "!" then do
    stripped = self~stripTag(content)
    if stripped~isA(.array) then do
      tagStr  = stripped[1]
      content = stripped[2]
    end
    else
      content = stripped
    lines[pos] = copies(" ", indent) || content
    if content == "" then do
      pos = pos + 1
      self~skipBlanks
      if pos > lines~items then do
        if tagStr \== "" then return .YamlTagged~new(tagStr, .nil)
        return .nil
      end
      /* We moved past the original line; the doc-start-line restriction
         (minIndent = -1) no longer applies to the new line. */
      if minIndent < 0 then minIndent = 0
      /* A standalone tag may be at a higher indent than the node it
         tags. After consuming the tag, use the actual indent of the
         next line if it is lower than minIndent (but still valid).
         Example (M5C3): tag at indent 3, block scalar at indent 2. */
      nextIndent = self~indentOf(lines[pos])
      node = self~blockNode(min(minIndent, nextIndent), scalarParentIndent)
      if tagStr \== "" then return .YamlTagged~new(tagStr, node)
      return node
    end
  end

  /* Decide node type */
  if content~left(2) == "? " | content == "?" then do
    /* Block mapping cannot start on the document-start line (§9.1.2) */
    if minIndent < 0 then
      self~raiseError("Block mapping is not allowed on the document-start line")
    node = self~blockMap(indent)
    if tagStr \== "" then return .YamlTagged~new(tagStr, node)
    return node
  end

  /* '?' followed by tab: the '?' IS a complex key indicator (tab is
     white space per YAML spec), but the content after tab would use
     tab as indentation, which is forbidden (§6.1).  E.g. "?\t-"
     (Y79Y/006), "?\tkey:" (Y79Y/008). */
  if content~left(1) == "?" & content~length > 1 & content[2] == "09"x then
    self~raiseError("Tab character in indentation (YAML 1.2 forbids tabs for indentation)")

  if content~left(2) == "- " | content == "-" | -
     (content~left(1) == "-" & content~length > 1 & content[2] == "09"x) then do
    /* Block sequence cannot start on the document-start line (§9.1.2) */
    if minIndent < 0 then
      self~raiseError("Block sequence is not allowed on the document-start line")
    node = self~blockSeq(indent)
    if tagStr \== "" then return .YamlTagged~new(tagStr, node)
    return node
  end

  if content~left(1) == "[" then do
    ep = self~matchBracket(content, "[", "]")
    if ep < content~length then do
      /* Closing bracket found on same line — check residual */
      afterBracket = content~substr(ep + 1)
      residual = afterBracket~strip
      if residual \== "" then do
        /* Flow collection used as block mapping key: [...]: value */
        if residual~left(1) == ":" then do
          node = self~blockMap(indent)
          if tagStr \== "" then return .YamlTagged~new(tagStr, node)
          return node
        end
        /* '#' is only a comment if preceded by whitespace */
        if residual~left(1) == "#" then do
          if afterBracket~left(1) \== " " & afterBracket~left(1) \== "09"x then
            self~raiseError("Invalid content after flow sequence:" residual)
        end
        else
          self~raiseError("Invalid content after flow sequence:" residual)
      end
      fval = self~flowSeq(content~left(ep))
      pos = pos + 1
    end
    else do
      /* Multiline flow sequence — let flowSeq accumulate lines */
      pos = pos + 1
      fval = self~flowSeq(content)
    end
    if tagStr \== "" then return .YamlTagged~new(tagStr, fval)
    return fval
  end
  if content~left(1) == "{" then do
    ep = self~matchBracket(content, "{", "}")
    if ep < content~length then do
      /* Closing brace found on same line — check residual */
      afterBrace = content~substr(ep + 1)
      residual = afterBrace~strip
      if residual \== "" then do
        /* Flow collection used as block mapping key: {...}: value */
        if residual~left(1) == ":" then do
          node = self~blockMap(indent)
          if tagStr \== "" then return .YamlTagged~new(tagStr, node)
          return node
        end
        if residual~left(1) == "#" then do
          if afterBrace~left(1) \== " " & afterBrace~left(1) \== "09"x then
            self~raiseError("Invalid content after flow mapping:" residual)
        end
        else
          self~raiseError("Invalid content after flow mapping:" residual)
      end
      fval = self~flowMap(content~left(ep))
      pos = pos + 1
    end
    else do
      /* Multiline flow mapping — let flowMap accumulate lines */
      pos = pos + 1
      fval = self~flowMap(content)
    end
    if tagStr \== "" then return .YamlTagged~new(tagStr, fval)
    return fval
  end

  if self~findMapColon(content) > 0 then do
    /* Block mapping cannot start on the document-start line (§9.1.2) */
    if minIndent < 0 then
      self~raiseError("Block mapping is not allowed on the document-start line")
    node = self~blockMap(indent)
    if tagStr \== "" then return .YamlTagged~new(tagStr, node)
    return node
  end

  /* When minIndent is -1 (content on --- line), block scalars need
     parentIndent = -1 so that indent-0 content lines are accepted. */
  if minIndent < 0 then
    node = self~blockScalarOrPlain(minIndent)
  else if scalarParentIndent \== .nil, self~isBlockIndicator(content) then
    node = self~blockScalarOrPlain(scalarParentIndent)
  else
    node = self~blockScalarOrPlain(indent)
  if tagStr \== "" then return .YamlTagged~new(tagStr, node)
  return node

/*============================================================================*/
/*  BLOCK MAPPING (private)                                                   */
/*============================================================================*/

/** Parses a block mapping at the given indentation level.
 *  Handles simple keys (key: value), complex keys (? key),
 *  alias keys, tag-prefixed keys, inline and nested values,
 *  and merge keys (<<).  Applies merges in reverse order
 *  (low priority) and records merge sources in mergeSourceMap.
 *
 *  @param blockIndent the column at which mapping keys must appear
 *  @return a .Table representing the mapping
 */
::method blockMap private
  expose lines pos anchors anchorMap mergeSourceMap curLine
  use strict arg blockIndent

  map    = .table~new
  merges = .array~new

  do while pos <= lines~items
    self~skipBlanks
    if pos > lines~items then leave

    line   = lines[pos]
    indent = self~indentOf(line)
    if indent \== blockIndent then leave
    curLine = pos
    self~checkTabIndent(line)

    content = line~strip

    anchorName = ""
    if content~left(1) == "&" then do
      parse var content aTag content
      anchorName = aTag~substr(2)
      content = content~strip
      /* An alias cannot be the target of an anchor (§7.1) */
      if content~left(1) == "*" then
        self~raiseError("An alias cannot be anchored")
    end

    /* Complex key indicator: ? */
    if content~left(2) == "? " | content == "?" then do
      afterQ = ""
      if content~length > 2 then afterQ = content~substr(3)~strip
      pos = pos + 1

      /* Parse the key node */
      if afterQ \== "" then do
        /* Inline key after "? ": could be flow collection, anchor, alias, or scalar */
        if afterQ~left(1) == "[" then
          key = self~flowSeq(afterQ)
        else if afterQ~left(1) == "{" then
          key = self~flowMap(afterQ)
        else if afterQ~left(1) == "&" then do
          parse var afterQ kATag kRest
          kAnchor = kATag~substr(2)
          kRest = kRest~strip
          if kRest == "" then key = .nil
          else                key = self~resolve(self~stripComment(kRest))
          anchors[kAnchor] = key
          if key \== .nil then anchorMap[key] = kAnchor
        end
        else if afterQ~left(1) == "*" then do
          kAName = self~extractAliasName(afterQ~substr(2))
          key = self~getAnchor(kAName)
        end
        else if self~isBlockIndicator(afterQ) then do
          /* Block scalar as explicit key: ? | or ? > */
          key = self~parseBlockScalar(afterQ, blockIndent)
        end
        else if afterQ~left(2) == "- " | afterQ == "-" then do
          /* Inline block seq as explicit key: ? - item */
          virtualIndent = blockIndent + 2
          lines[pos - 1] = copies(" ", virtualIndent) || afterQ
          pos = pos - 1
          key = self~blockSeq(virtualIndent)
        end
        else if afterQ~left(1) == "#" then do
          /* Comment after ?: treat as bare ? (key on next lines) */
          nop  /* fall through to afterQ == "" branch below */
        end
        else do
          /* Plain scalar — may have multiline continuation */
          parts = .array~new
          stripped = self~stripComment(afterQ)
          parts~append(stripped)
          if stripped \== afterQ then
            key = self~resolve(stripped)
          else
            key = self~plainContinuation(parts, blockIndent + 1)
        end
      end

      if afterQ == "" | afterQ~left(1) == "#" then do
        /* Key on next line(s) — parse as a block node */
        self~skipBlanks
        if pos > lines~items then key = .nil
        else do
          ni = self~indentOf(lines[pos])
          if ni > blockIndent then key = self~blockNode(ni)
          else if ni == blockIndent then do
            /* At same indent: block collections (- or ?) are part of the key */
            nc = lines[pos]~strip
            if nc~left(2) == "- " | nc == "-" then
              key = self~blockSeq(ni)
            else
              key = .nil
          end
          else key = .nil
        end
      end

      /* Now expect ": " at blockIndent for the value */
      self~skipBlanks
      value = .nil
      if pos <= lines~items then do
        vLine   = lines[pos]
        vIndent = self~indentOf(vLine)
        vContent = vLine~strip
        if vIndent == blockIndent, vContent~left(1) == ":" then curLine = pos
        /* Tab after ':' indicator is invalid */
        if vIndent == blockIndent, vContent~left(1) == ":", -
           vContent~length >= 2, vContent[2] == "09"x then
          self~raiseError("Tab character after ':' indicator (YAML 1.2 requires space)")
        if vIndent == blockIndent, -
           (vContent~left(2) == ": " | vContent == ":") then do
          valPart = ""
          if vContent~length > 2 then do
            raw = vContent~substr(3)
            stripped = raw~strip
            if stripped \== "", stripped~left(1) \== "#" then
              valPart = self~stripComment(raw)~strip
          end
          pos = pos + 1
          value = self~mapValue(valPart, blockIndent, .true)
        end
      end

      if anchorName \== "" then do
        anchors[anchorName] = value
        if value \== .nil then anchorMap[value] = anchorName
      end

      map[key] = value
      iterate
    end

    colonPos = self~findMapColon(content)
    if colonPos = 0 then leave

    key     = content~left(colonPos - 1)~strip
    valPart = content~substr(colonPos + 1)~strip
    /* A '#' at the start of valPart is always a comment: the mapping
       colon was followed by space (findMapColon guarantees ': '), so
       the '#' is preceded by whitespace in the original line. */
    if valPart \== "", valPart~left(1) == "#" then valPart = ""
    /* Strip tag from key if present */
    keyTag = ""
    if key~left(1) == "!" then do
      stripped = self~stripTag(key)
      if stripped~isA(.array) then do
        keyTag = stripped[1]
        key    = stripped[2]
      end
      else
        key = stripped
    end
    self~checkPlainKey(key)
    key     = self~unquoteIfNeeded(key)
    /* Resolve alias keys: *name → anchored value */
    if key~left(1) == "*" then
      key = self~getAnchor(self~extractAliasName(key~substr(2)))
    if keyTag \== "" then key = .YamlTagged~new(keyTag, key)
    pos     = pos + 1

    /* Reject nested implicit mapping on same line: "a: b: c" (ZCZ6).
       In block context, a second ": " in the inline value part would
       start a nested mapping, which is not allowed on the same line
       as the parent key.  Only applies to plain scalar values — skip
       anchors, aliases, tags, quoted strings, and flow collections. */
    self~checkNoNestedMapping(valPart)

    value   = self~mapValue(valPart, indent)

    if anchorName \== "" then do
      anchors[anchorName] = value
      if value \== .nil then anchorMap[value] = anchorName
    end

    /* Merge key */
    if key == "<<" then do
      if value~isA(.array) then do item over value
        if item~isA(.table) then merges~append(item)
      end
      else if value~isA(.table) then merges~append(value)
    end
    else
      map[key] = value
  end

  /* Apply merges (low priority) */
  do i = merges~items to 1 by -1
    sup = merges[i]~supplier
    do while sup~available
      if \map~hasIndex(sup~index) then map[sup~index] = sup~item
      sup~next
    end
  end

  /* Record merge sources for this mapping (used by emitter) */
  if merges~items > 0 then
    mergeSourceMap[map] = merges

  return map

/** Parses the value portion of a mapping entry.
 *  Called after the key and colon have been consumed.  Handles
 *  inline values (scalars, flow collections, aliases, anchors),
 *  block values on subsequent lines, block scalars, multi-line
 *  quoted strings, compact block sequences at the same indent,
 *  and inline block sequences in explicit-key context.
 *
 *  @param valPart the inline value text after ':' (may be empty)
 *  @param parentIndent the indentation of the containing mapping
 *  @param allowInlineSeq .true if inline block sequence (: - item)
 *         is permitted (explicit key context only)
 *  @return the parsed value node
 */
::method mapValue private
  expose lines pos anchors anchorMap preserveTags flowMinIndent curLine
  use strict arg valPart, parentIndent, allowInlineSeq = .false

  /* Tag prefix — strip and resolve */
  valTag = ""
  if valPart~left(1) == "!" then do
    stripped = self~stripTag(valPart)
    if stripped~isA(.array) then do
      valTag  = stripped[1]
      valPart = stripped[2]
    end
    else
      valPart = stripped
  end

  if self~isBlockIndicator(valPart) then do
    node = self~parseBlockScalar(valPart, parentIndent)
    if valTag \== "" then return .YamlTagged~new(valTag, node)
    return node
  end
  /* Starts with | or > but isBlockIndicator returned false → malformed */
  if valPart \== "", valPart~left(1) == "|" | valPart~left(1) == ">" then
    self~raiseError("Invalid block scalar indicator:" valPart)

  if valPart == "" then do
    self~skipBlanks
    if pos > lines~items then do
      if valTag \== "" then return .YamlTagged~new(valTag, .nil)
      return .nil
    end
    ni = self~indentOf(lines[pos])
    /* Standalone anchor line: consume it and re-evaluate the actual node.
       This allows patterns like:
         key:
          &anchor
         - a        # compact seq at parentIndent */
    valAnchor = ""
    if ni > parentIndent then do
      anchorLine = lines[pos]~strip
      if anchorLine~left(1) == "&" then do
        parse var anchorLine aTag aRest
        aRest = aRest~strip
        if aRest == "" | aRest~left(1) == "#" then do
          valAnchor = aTag~substr(2)
          anchors[valAnchor] = .nil
          curLine = pos
          pos = pos + 1
          self~skipBlanks
          if pos > lines~items then do
            if valTag \== "" then return .YamlTagged~new(valTag, .nil)
            return .nil
          end
          ni = self~indentOf(lines[pos])
        end
      end
    end
    if ni > parentIndent then do
      node = self~blockNode(ni, parentIndent)
      if valAnchor \== "" then do
        if node \== .nil, anchorMap~hasIndex(node), -
           \node~isA(.table), \node~isA(.array) then
          self~raiseError("Only one anchor is allowed per node")
        anchors[valAnchor] = node
        if node \== .nil then anchorMap[node] = valAnchor
      end
      if valTag \== "" then return .YamlTagged~new(valTag, node)
      return node
    end
    /* Compact block sequence at same indentation as mapping key:
       key:
       - item1
       - item2
       Per YAML spec, a sequence at the same indent as the mapping key
       is the value of that key (compact notation). */
    if ni == parentIndent then do
      nextContent = lines[pos]~strip
      if nextContent~left(2) == "- " | nextContent == "-" then do
        node = self~blockSeq(ni)
        if valAnchor \== "" then do
          anchors[valAnchor] = node
          if node \== .nil then anchorMap[node] = valAnchor
        end
        if valTag \== "" then return .YamlTagged~new(valTag, node)
        return node
      end
    end
    if valTag \== "" then return .YamlTagged~new(valTag, .nil)
    return .nil
  end

  if valPart~left(1) == "[" then do
    ep = self~matchBracket(valPart, "[", "]")
    afterBracket = valPart~substr(ep + 1)
    residual = afterBracket~strip
    if residual \== "" then do
      if residual~left(1) == "#" then do
        if afterBracket~left(1) \== " " & afterBracket~left(1) \== "09"x then
          self~raiseError("Invalid content after flow sequence:" residual)
      end
      else
        self~raiseError("Invalid content after flow sequence:" residual)
    end
    flowMinIndent = parentIndent + 1
    node = self~flowSeq(valPart~left(ep))
    if valTag \== "" then return .YamlTagged~new(valTag, node)
    return node
  end
  if valPart~left(1) == "{" then do
    ep = self~matchBracket(valPart, "{", "}")
    afterBrace = valPart~substr(ep + 1)
    residual = afterBrace~strip
    if residual \== "" then do
      if residual~left(1) == "#" then do
        if afterBrace~left(1) \== " " & afterBrace~left(1) \== "09"x then
          self~raiseError("Invalid content after flow mapping:" residual)
      end
      else
        self~raiseError("Invalid content after flow mapping:" residual)
    end
    flowMinIndent = parentIndent + 1
    node = self~flowMap(valPart~left(ep))
    if valTag \== "" then return .YamlTagged~new(valTag, node)
    return node
  end

  if valPart~left(1) == "*" then do
    aName = self~extractAliasName(valPart~substr(2))
    return self~getAnchor(aName)
  end

  if valPart~left(1) == "&" then do
    parse var valPart aTag rest
    aName = aTag~substr(2)
    rest = rest~strip
    /* Check if rest is a comment (# preceded by space in original) */
    if rest \== "", rest~left(1) == "#" then rest = ""
    if rest == "" then do
      self~skipBlanks
      if pos > lines~items then do
        anchors[aName] = .nil
        return .nil
      end
      ni = self~indentOf(lines[pos])
      if ni > parentIndent then do
        val = self~blockNode(ni, parentIndent)
        /* Reject double anchor: if blockNode already anchored this
           node (e.g. &a\n  &b scalar), it's two anchors on the
           same node — YAML 1.2 forbids this (§6.9.2). */
        if val \== .nil, anchorMap~hasIndex(val), -
           \val~isA(.table), \val~isA(.array) then
          self~raiseError("Only one anchor is allowed per node")
        anchors[aName] = val
        if val \== .nil then anchorMap[val] = aName
        return val
      end
      anchors[aName] = .nil
      return .nil
    end
    /* Reject &anchor *alias or &anchor &anchor2 on same line */
    if rest~left(1) == "*" then
      self~raiseError("Anchor cannot precede alias:" valPart)
    if rest~left(1) == "&" then
      self~raiseError("Multiple anchors on same node:" valPart)
    val = self~resolve(self~stripComment(rest))
    anchors[aName] = val
    if val \== .nil then anchorMap[val] = aName
    return val
  end

  /* Inline block sequence: ": - item" on the same line */
  /* Only allowed in explicit key context (? key : - value) */
  if allowInlineSeq, -
     (valPart~left(2) == "- " | valPart~left(2) == "-" || "09"x | valPart == "-") then do
    /* Create a virtual line at parentIndent + 2 and reparse as blockSeq */
    virtualIndent = parentIndent + 2
    lines[pos - 1] = copies(" ", virtualIndent) || valPart
    pos = pos - 1
    node = self~blockSeq(virtualIndent)
    if valTag \== "" then return .YamlTagged~new(valTag, node)
    return node
  end

  /* Multi-line quoted string */
  if valPart~left(1) == '"' | valPart~left(1) == "'" then do
    pair = self~multiLineQuoted(valPart, parentIndent + 1)
    node = pair[1]
    rest = pair[2]~strip
    if rest \== "" then do
      if rest~left(1) \== "#" then
        self~raiseError("Invalid trailing content after quoted scalar:" rest)
      /* Comment # must be preceded by at least one space/tab */
      restRaw = pair[2]
      p = restRaw~verify(" " || "09"x)
      if p > 1, restRaw[p] == "#" then nop  /* valid: space(s) then # */
      else
        self~raiseError("Invalid trailing content after quoted scalar:" rest)
    end
    if valTag \== "" then return .YamlTagged~new(valTag, node)
    return node
  end

  /* Plain scalar — may have multiline continuation */
  self~checkPlainFirst(valPart)
  parts = .array~new
  stripped = self~stripComment(valPart)
  parts~append(stripped)
  /* A trailing comment on the value line terminates the plain scalar
     immediately — no continuation lines are consumed (YAML §7.3.3). */
  if stripped \== valPart then do
    node = self~resolve(stripped)
    if valTag \== "" then return .YamlTagged~new(valTag, node)
    return node
  end
  node = self~plainContinuation(parts, parentIndent + 1)
  if valTag \== "" then return .YamlTagged~new(valTag, node)
  return node

/*============================================================================*/
/*  BLOCK SEQUENCE (private)                                                  */
/*============================================================================*/

/** Parses a block sequence at the given indentation level.
 *  Consumes '- ' entries, handling inline values (scalars, flow
 *  collections, anchors, aliases, compact mappings, nested
 *  sequences), block values on subsequent lines, block scalars,
 *  multi-line quoted strings, and tags on sequence items.
 *
 *  @param blockIndent the column at which '-' indicators must appear
 *  @return an .Array of sequence items
 */
::method blockSeq private
  expose lines pos anchors anchorMap flowMinIndent curLine
  use strict arg blockIndent

  arr = .array~new

  do while pos <= lines~items
    self~skipBlanks
    if pos > lines~items then leave

    line   = lines[pos]
    indent = self~indentOf(line)
    if indent \== blockIndent then leave
    curLine = pos
    self~checkTabIndent(line)

    content = line~strip
    if content~left(2) \== "- " & content~left(2) \== "-" || "09"x & content \== "-" then leave

    if content == "-" then afterDash = ""
    else do
      rawAfterDash = content~substr(2)          -- everything after the '-'
      afterDash    = rawAfterDash~strip
      /* If the separation between '-' and the next content contains a tab,
         and the next content is a block indicator (- or ?), then tab is
         acting as indentation for the nested block structure → error.
         E.g. "-\t-" (Y79Y/004), "- \t-" (Y79Y/005).
         But "-\t-1" (Y79Y/010) and "- -\tc" (A2M4) are OK.
         We check only the whitespace BETWEEN the outer '-' and the start
         of afterDash — i.e. the leading whitespace of rawAfterDash. */
      sepEnd = rawAfterDash~verify(" " || "09"x)
      if sepEnd == 0 then sepEnd = rawAfterDash~length + 1
      separation = rawAfterDash~left(sepEnd - 1)
      if separation~pos("09"x) > 0 then do
        if afterDash == "-" | afterDash == "?" | -
           afterDash~left(2) == "- " | afterDash~left(2) == "? " then
          self~raiseError("Tab character in indentation (YAML 1.2 forbids tabs for indentation)")
      end
    end

    /* Tag prefix — strip and resolve */
    seqTag = ""
    if afterDash \== "", afterDash~left(1) == "!" then do
      stripped = self~stripTag(afterDash)
      if stripped~isA(.array) then do
        seqTag    = stripped[1]
        afterDash = stripped[2]
      end
      else
        afterDash = stripped
    end
    /* Comment after tag: treat as standalone tag (value on next lines) */
    if afterDash \== "", afterDash~left(1) == "#" then afterDash = ""

    pos = pos + 1

    select
      when afterDash == "" then do
        self~skipBlanks
        if pos <= lines~items then do
          ni = self~indentOf(lines[pos])
          if ni > blockIndent then do
            node = self~blockNode(ni)
            if seqTag \== "" then arr~append(.YamlTagged~new(seqTag, node))
            else                  arr~append(node)
          end
          else do
            if seqTag \== "" then arr~append(.YamlTagged~new(seqTag, .nil))
            else                  arr~append(.nil)
          end
        end
        else do
          if seqTag \== "" then arr~append(.YamlTagged~new(seqTag, .nil))
          else                  arr~append(.nil)
        end
      end

      when self~isBlockIndicator(afterDash) then do
        node = self~parseBlockScalar(afterDash, indent)
        if seqTag \== "" then arr~append(.YamlTagged~new(seqTag, node))
        else                  arr~append(node)
      end

      when afterDash~left(1) == "[" then do
        ep = self~matchBracket(afterDash, "[", "]")
        afterBracket = afterDash~substr(ep + 1)
        residual = afterBracket~strip
        if residual \== "" then do
          if residual~left(1) == "#" then do
            if afterBracket~left(1) \== " " & afterBracket~left(1) \== "09"x then
              self~raiseError("Invalid content after flow sequence:" residual)
          end
          else
            self~raiseError("Invalid content after flow sequence:" residual)
        end
        flowMinIndent = blockIndent + 1
        node = self~flowSeq(afterDash~left(ep))
        if seqTag \== "" then arr~append(.YamlTagged~new(seqTag, node))
        else                  arr~append(node)
      end

      when afterDash~left(1) == "{" then do
        ep = self~matchBracket(afterDash, "{", "}")
        afterBrace = afterDash~substr(ep + 1)
        residual = afterBrace~strip
        if residual \== "" then do
          if residual~left(1) == "#" then do
            if afterBrace~left(1) \== " " & afterBrace~left(1) \== "09"x then
              self~raiseError("Invalid content after flow mapping:" residual)
          end
          else
            self~raiseError("Invalid content after flow mapping:" residual)
        end
        flowMinIndent = blockIndent + 1
        node = self~flowMap(afterDash~left(ep))
        if seqTag \== "" then arr~append(.YamlTagged~new(seqTag, node))
        else                  arr~append(node)
      end

      when self~findMapColon(afterDash) > 0 then do
        node = self~compactMap(afterDash, indent + 2)
        if seqTag \== "" then arr~append(.YamlTagged~new(seqTag, node))
        else                  arr~append(node)
      end

      when afterDash~left(1) == "&" then do
        parse var afterDash aTag rest
        aName = aTag~substr(2)
        rest = rest~strip
        if rest == "" then do
          self~skipBlanks
          if pos <= lines~items then do
            ni = self~indentOf(lines[pos])
            if ni > blockIndent then do
              val = self~blockNode(ni)
              anchors[aName] = val
              if val \== .nil then anchorMap[val] = aName
              arr~append(val)
            end
            else do; anchors[aName] = .nil; arr~append(.nil); end
          end
          else do; anchors[aName] = .nil; arr~append(.nil); end
        end
        else do
          val = self~resolve(self~stripComment(rest))
          anchors[aName] = val
          if val \== .nil then anchorMap[val] = aName
          arr~append(val)
        end
      end

      when afterDash~left(1) == "*" then do
        aName = afterDash~substr(2)
        cp = aName~pos(" #")
        if cp > 0 then aName = aName~left(cp - 1)~strip
        arr~append(self~getAnchor(aName~strip))
      end

      when afterDash~left(2) == "- " | afterDash == "-" then do
        virtualIndent = indent + 2
        lines[pos - 1] = copies(" ", virtualIndent) || afterDash
        pos = pos - 1
        node = self~blockSeq(virtualIndent)
        if seqTag \== "" then arr~append(.YamlTagged~new(seqTag, node))
        else                  arr~append(node)
      end

      otherwise do
        if afterDash~left(1) == '"' | afterDash~left(1) == "'" then do
          pair = self~multiLineQuoted(afterDash, blockIndent + 1)
          node = pair[1]
          rest = pair[2]~strip
          if rest \== "" then do
            if rest~left(1) \== "#" then
              self~raiseError("Invalid trailing content after quoted scalar:" rest)
            restRaw = pair[2]
            p = restRaw~verify(" " || "09"x)
            if p > 1, restRaw[p] == "#" then nop
            else
              self~raiseError("Invalid trailing content after quoted scalar:" rest)
          end
        end
        else do
          self~checkPlainFirst(afterDash)
          parts = .array~new
          stripped = self~stripComment(afterDash)
          parts~append(stripped)
          if stripped \== afterDash then
            node = self~resolve(stripped)
          else
            node = self~plainContinuation(parts, indent + 1, indent)
        end
        if seqTag \== "" then arr~append(.YamlTagged~new(seqTag, node))
        else                  arr~append(node)
      end
    end
  end

  return arr

/** Parses a compact mapping (inline after '- ' in a sequence).
 *  The first key: value pair is provided as text; subsequent pairs
 *  are read from the input at virtualIndent or deeper.
 *
 *  @param firstLine the text of the first key: value pair
 *  @param virtualIndent the indentation level for continuation entries
 *  @return a .Table representing the compact mapping
 */
::method compactMap private
  expose lines pos curLine
  use strict arg firstLine, virtualIndent

  map = .table~new

  colonPos = self~findMapColon(firstLine)
  key     = self~unquoteIfNeeded(firstLine~left(colonPos - 1)~strip)
  valPart = firstLine~substr(colonPos + 1)~strip
  /* "? key" and ": value" are explicit entries: a compact mapping may
     follow them on the same line (Spec Example 8.19, V9D5) */
  if firstLine~left(1) \== "?" then do
    self~checkPlainKey(key)
    self~checkNoNestedMapping(valPart)
  end
  map[key] = self~mapValue(valPart, virtualIndent - 2)

  do while pos <= lines~items
    self~skipBlanks
    if pos > lines~items then leave
    line   = lines[pos]
    indent = self~indentOf(line)
    if indent < virtualIndent then leave
    content = line~strip
    colonPos = self~findMapColon(content)
    if colonPos = 0 then leave
    curLine = pos
    key     = self~unquoteIfNeeded(content~left(colonPos - 1)~strip)
    valPart = content~substr(colonPos + 1)~strip
    if content~left(1) \== "?", content~left(1) \== ":" then do
      self~checkPlainKey(key)
      self~checkNoNestedMapping(valPart)
    end
    pos     = pos + 1
    map[key] = self~mapValue(valPart, indent)
  end

  return map

/*============================================================================*/
/*  BLOCK SCALARS (private)                                                   */
/*============================================================================*/

/** Tests whether a text string is a valid block scalar indicator.
 *  A block scalar indicator starts with '|' or '>', optionally
 *  followed by a chomp indicator (+/-) and/or an indent digit (1-9),
 *  and optionally a comment (space + #).  Invalid combinations
 *  (e.g. two digits, two chomp indicators) return .false.
 *
 *  @param text the text to test
 *  @return .true if valid block scalar indicator, .false otherwise
 */
::method isBlockIndicator private
  use strict arg text
  if text == "" then return .false
  ch = text~left(1)
  if ch \== "|" & ch \== ">" then return .false
  /* Strip comment (space + #) from the header */
  header = text
  do i = 2 to text~length
    c = text[i]
    if c == " " | c == "09"x then do
      rest = text~substr(i)~strip
      if rest == "" | rest~left(1) == "#" then do
        header = text~left(i - 1)
        leave
      end
      /* Non-comment text after space — not a valid block indicator */
      return .false
    end
    if c \== "+" & c \== "-" & \c~datatype("9") then do
      /* Invalid character — '#' without space means invalid header */
      return .false
    end
  end
  /* Validate: at most one digit (1-9), at most one chomp indicator */
  digitCount = 0; chompCount = 0
  do i = 2 to header~length
    c = header[i]
    select
      when c~datatype("9") then do
        digitCount = digitCount + 1
        if c == "0" | digitCount > 1 then return .false
      end
      when c == "+" | c == "-" then do
        chompCount = chompCount + 1
        if chompCount > 1 then return .false
      end
      otherwise return .false
    end
  end
  return .true

/** Parses a block scalar (literal '|' or folded '>').
 *  Reads the header for chomp and explicit indent indicators, then
 *  collects body lines at the determined indentation level.  Applies
 *  literal or folded line-joining rules and the chomping behaviour
 *  (strip, clip, or keep) to produce the final scalar value.
 *
 *  @param indicator the block scalar header string (e.g. "|+", ">2")
 *  @param parentIndent the indentation of the parent structure
 *  @return the assembled scalar string
 */
::method parseBlockScalar private
  expose lines pos
  use strict arg indicator, parentIndent

  style = indicator~left(1)
  chomp = "clip"
  explicitIndent = 0
  do i = 2 to indicator~length
    c = indicator[i]
    select
      when c == "+" then chomp = "keep"
      when c == "-" then chomp = "strip"
      when c~datatype("9") then explicitIndent = c
      otherwise nop
    end
  end

  bodyLines    = .array~new
  scalarIndent = 0
  firstContent = .true
  maxLeadingSpaces = 0   -- track max indent of leading all-space lines

  do while pos <= lines~items
    line = lines[pos]
    if line~strip == "" then do
      /* A line consisting solely of tab(s) — with no leading spaces —
         means the tab is being used as indentation, which YAML 1.2
         forbids (§6.1).  Lines with leading space(s) before a tab are
         OK: the space provides the indentation and the tab is content. */
      if line~pos("09"x) > 0 & self~indentOf(line) == 0 then
        self~raiseError("Tab character in indentation (YAML 1.2 forbids tabs for indentation)", pos)
      /* Track indent of leading all-space lines (non-empty, spaces only) */
      if firstContent & line \== "" & line~verify(" ") == 0 then do
        if line~length > maxLeadingSpaces then maxLeadingLine = pos
        maxLeadingSpaces = max(maxLeadingSpaces, line~length)
      end
      bodyLines~append("")
      pos = pos + 1
      iterate
    end
    lineIndent = self~indentOf(line)
    if firstContent then do
      if explicitIndent > 0 then scalarIndent = parentIndent + explicitIndent
      else                       scalarIndent = lineIndent
      if scalarIndent <= parentIndent then leave
      /* Leading all-space lines must not have more spaces than scalarIndent */
      if maxLeadingSpaces > scalarIndent then
        self~raiseError("Leading all-space line has too many spaces ("maxLeadingSpaces") for block scalar indentation level" scalarIndent, maxLeadingLine)
      firstContent = .false
    end
    if lineIndent < scalarIndent then leave
    bodyLines~append(line~substr(scalarIndent + 1))
    pos = pos + 1
  end

  /* Count trailing blanks */
  trailing = 0
  do i = bodyLines~items to 1 by -1
    if bodyLines[i] == "" then trailing = trailing + 1
    else leave
  end
  contentEnd = bodyLines~items - trailing

  /* Build output */
  if style == "|" then do
    parts = .array~new
    do i = 1 to contentEnd
      parts~append(bodyLines[i])
    end
    out = parts~makeString('L', "0A"x)
  end
  else do
    mb = .mutableBuffer~new
    prevBlank = .false
    do i = 1 to contentEnd
      ln = bodyLines[i]
      if ln == "" then do
        mb~append("0A"x)
        prevBlank = .true
      end
      else do
        if ln~left(1) == " " then do
          if mb~length \= 0 & \prevBlank then mb~append("0A"x)
          mb~append(ln, "0A"x)
          prevBlank = .false
          iterate
        end
        if mb~length = 0 then mb~append(ln)
        else if prevBlank then mb~append("0A"x, ln)
        else mb~append(" ", ln)
        prevBlank = .false
      end
    end
    out = mb~string
  end

  select case chomp
    when "strip" then nop
    when "keep"  then
      out = out || copies("0A"x, trailing + 1)
    otherwise
      out = out || "0A"x
  end

  return out

/** Parses the current line as either a block scalar or a plain scalar.
 *  If the line starts with a block indicator ('|' or '>'), delegates
 *  to parseBlockScalar.  If quoted, delegates to multiLineQuoted.
 *  Otherwise treats it as a plain scalar with possible continuation
 *  lines via plainContinuation.  Also rejects reserved indicators
 *  (%, @, `) that cannot start a plain scalar.
 *
 *  @param blockIndent the indentation level for block scalar detection
 *  @return the parsed scalar value
 */
::method blockScalarOrPlain private
  expose lines pos
  use strict arg blockIndent

  line    = lines[pos]
  content = line~strip
  fc = content~left(1)

  if self~isBlockIndicator(content) then do
    pos = pos + 1
    return self~parseBlockScalar(content, blockIndent)
  end
  /* Starts with | or > but isBlockIndicator returned false → malformed */
  if fc == "|" | fc == ">" then
    self~raiseError("Invalid block scalar indicator:" content)

  if fc == '"' | fc == "'" then do
    pos = pos + 1
    pair = self~multiLineQuoted(content, max(blockIndent, 0))
    return pair[1]
  end

  /* c-directive (%) and c-reserved (@ `) cannot start a plain scalar
     (YAML §7.3.3, ns-plain-first). */
  if fc == "%" | fc == "@" | fc == "`" then
    self~raiseError("Reserved indicator" fc "cannot start a plain scalar")

  parts = .array~new
  stripped = self~stripComment(content)
  parts~append(stripped)
  pos = pos + 1

  /* A trailing comment on the first line terminates the plain scalar
     immediately — no continuation lines are consumed (YAML §7.3.3). */
  if stripped \== content then
    return self~resolve(stripped)

  return self~plainContinuation(parts, blockIndent)

/*----------------------------------------------------------------------------*/
/* plainContinuation — accumulate multiline plain scalar continuation lines.  */
/* 'parts' is an array with the first line already appended. 'minIndent' is   */
/* the minimum indentation required for continuation lines.                   */
/* Returns the resolved scalar value.                                         */
/*                                                                            */
/* YAML rules for plain scalar folding:                                       */
/*   - Non-empty continuation lines join with a single space.                 */
/*   - A single blank line becomes a literal newline (\n).                    */
/*   - Each additional consecutive blank line adds another newline.           */
/*   - Blank lines only count as part of the scalar if a valid continuation   */
/*     line follows them (lookahead required).                                */
/*----------------------------------------------------------------------------*/

/** Accumulates multi-line plain scalar continuation lines.
 *  Applies YAML 1.2 folding rules: non-empty continuation lines
 *  join with a space, a single blank line becomes a newline, and
 *  each additional blank line adds another newline.  Stops at
 *  document markers, block indicators, mapping colons, comments,
 *  or insufficient indentation.
 *
 *  @param parts an .Array with the first line already appended
 *  @param minIndent minimum indentation for continuation lines
 *  @param seqIndent optional sequence indent to detect sequence
 *         entries that terminate the scalar
 *  @return the resolved scalar value
 */
::method plainContinuation private
  expose lines pos
  use strict arg parts, minIndent, seqIndent = .nil

  do while pos <= lines~items
    nLine = lines[pos]

    /* Blank line — lookahead to see if the scalar continues */
    if nLine~strip == "" then do
      blankCount = 0
      lookahead = pos
      do while lookahead <= lines~items
        if lines[lookahead]~strip \== "" then leave
        blankCount = blankCount + 1
        lookahead = lookahead + 1
      end
      /* If no more lines, or next non-blank doesn't qualify → stop */
      if lookahead > lines~items then leave
      laLine = lines[lookahead]
      if self~indentOf(laLine) < minIndent then leave
      lac = laLine~strip
      if lac == "---" | lac~left(4) == "--- " | lac~left(4) == "---" || "09"x then leave
      if lac == "..." | lac~left(4) == "... " | lac~left(4) == "..." || "09"x then leave
      if (lac~left(2) == "- " | lac == "-"), -
         (seqIndent == .nil | self~indentOf(laLine) == seqIndent) then leave
      if self~findMapColon(lac) > 0 then leave
      if lac~left(1) == "#" then leave
      /* Valid continuation after blanks — insert newline separators */
      do blankCount
        parts~append("0A"x)
      end
      pos = lookahead
      iterate
    end

    if self~indentOf(nLine) < minIndent then leave
    nc = nLine~strip
    if nc == "---" | nc~left(4) == "--- " | nc~left(4) == "---" || "09"x then leave
    if nc == "..." | nc~left(4) == "... " | nc~left(4) == "..." || "09"x then leave
    if (nc~left(2) == "- " | nc == "-"), -
       (seqIndent == .nil | self~indentOf(nLine) == seqIndent) then leave
    if self~findMapColon(nc) > 0 then leave
    if nc~left(1) == "#" then leave
    stripped = self~stripComment(nc)
    parts~append(stripped)
    pos = pos + 1
    /* A trailing comment on this continuation line terminates the
       plain scalar — no further continuation lines are consumed.
       (YAML §7.3.3: the comment ends the line content.) */
    if stripped \== nc then leave
  end

  /* Join parts: regular parts with space, but \n markers stay as-is */
  out = parts[1]
  do i = 2 to parts~items
    p = parts[i]
    if p == "0A"x then do
      out = out || "0A"x
      /* Skip consecutive \n markers and the next text part joins directly */
      do while i + 1 <= parts~items, parts[i + 1] == "0A"x
        out = out || "0A"x
        i = i + 1
      end
      /* Next non-blank part joins without space (newline replaces space) */
      if i + 1 <= parts~items then do
        i = i + 1
        out = out || parts[i]
      end
    end
    else
      out = out || " " || p
  end

  return self~resolve(out)

/*============================================================================*/
/*  FLOW COLLECTIONS (private)                                                */
/*============================================================================*/

/** Parses a flow sequence ([ ... ]).
 *  Accumulates continuation lines (with line-boundary markers),
 *  validates indentation and document markers, then delegates
 *  item extraction to nextFlowVal.  Handles multi-line detection
 *  for implicit mapping keys within sequences.
 *
 *  @param text the text starting with '[' (may span multiple lines)
 *  @return an .Array of sequence items
 */
::method flowSeq private
  expose lines pos flowMinIndent curLine
  use strict arg text

  startLine = curLine           /* where the collection opens */

  full = self~stripComment(text)
  do while self~countUnquoted(full, "[") > self~countUnquoted(full, "]")
    if pos > lines~items then self~raiseError("Unterminated flow sequence", startLine)
    /* Document markers at column 0 are invalid inside flow collections */
    nLine = lines[pos]
    curLine = pos
    if nLine~length >= 3 then do
      prefix3 = nLine~left(3)
      if prefix3 == "---" | prefix3 == "..." then
        if nLine~length == 3 | nLine[4] == " " | nLine[4] == "09"x then
          self~raiseError("Document marker inside flow sequence")
    end
    stripped = nLine~strip
    pos = pos + 1
    /* Full-line comment — insert comment-boundary marker */
    if stripped~left(1) == "#" then do
      full = full || "03"x
      iterate
    end
    /* Skip blank lines */
    if stripped == "" then iterate
    /* Check indentation of non-blank content line */
    if self~indentOf(nLine) < flowMinIndent then
      self~raiseError("Flow sequence continuation line not indented enough")
    contLine = self~stripComment(stripped)
    full = full || "02"x || contLine
  end

  /* Use matchBracket to find the true closing ']' */
  full = full~strip("Both", " " || "02"x || "03"x)
  if full~left(1) \== "[" then self~raiseError("Expected '[' at start of flow sequence")
  ep = self~matchBracket(full, "[", "]")

  /* Check for residual content after closing bracket (e.g., ]: 42).
     If the residual contains a value indicator ':', the flow sequence
     is being used as a mapping key — this is invalid when the sequence
     spans multiple lines (contains line-boundary markers). */
  if ep < full~length then do
    residual = self~flowStrip(full~substr(ep + 1))
    if residual \== "" then do
      if residual~left(1) == ":" then do
        /* Flow collection key spans multiple lines */
        if full~pos("02"x) > 0 | full~pos("03"x) > 0 then
          self~raiseError("Flow collection used as mapping key spans multiple lines")
      end
    end
  end

  inner = full~substr(2, ep - 2)~strip("Both", " " || "02"x || "03"x)
  if inner == "" then return .array~new

  /* Leading comma is invalid */
  if inner~left(1) == "," then
    self~raiseError("Invalid leading comma in flow sequence")

  arr = .array~new
  do while inner \== ""
    pair = self~nextFlowVal(inner)
    arr~append(pair[1])
    /* Check for implicit key spanning multiple lines: if the rest
       after a value starts with line-boundary markers followed by
       a value indicator ':', the key and ':' are on different lines.
       We check if there is a marker BEFORE the ':' in the rest. */
    rest = pair[2]
    restClean = rest~strip("Leading", " " || "02"x || "03"x)
    if restClean \== "" then do
      if restClean~left(1) == ":" then do
        /* Find the colon position and check if markers precede it */
        colonIdx = rest~pos(":")
        beforeColon = rest~left(colonIdx - 1)
        if beforeColon~pos("02"x) > 0 | beforeColon~pos("03"x) > 0 then
          self~raiseError("Implicit flow mapping key spans multiple lines")
      end
    end
    inner = self~flowStrip(rest)
    if inner~left(1) == "," then do
      inner = self~flowStrip(inner~substr(2))
      /* Double comma (empty entry) is invalid */
      if inner~left(1) == "," then
        self~raiseError("Invalid extra comma in flow sequence")
    end
  end
  return arr

/** Parses a flow mapping ({ ... }).
 *  Accumulates continuation lines (with line-boundary markers),
 *  validates indentation and document markers.  Handles flow
 *  collection keys, alias keys, anchor prefixes on keys, and
 *  regular scalar keys with colon separators.
 *
 *  @param text the text starting with '{' (may span multiple lines)
 *  @return a .Table of key-value pairs
 */
::method flowMap private
  expose lines pos flowMinIndent curLine
  use strict arg text

  startLine = curLine           /* where the collection opens */

  full = self~stripComment(text)
  do while self~countUnquoted(full, "{") > self~countUnquoted(full, "}")
    if pos > lines~items then self~raiseError("Unterminated flow mapping", startLine)
    /* Document markers at column 0 are invalid inside flow collections */
    nLine = lines[pos]
    curLine = pos
    if nLine~length >= 3 then do
      prefix3 = nLine~left(3)
      if prefix3 == "---" | prefix3 == "..." then
        if nLine~length == 3 | nLine[4] == " " | nLine[4] == "09"x then
          self~raiseError("Document marker inside flow mapping")
    end
    stripped = nLine~strip
    pos = pos + 1
    /* Full-line comment — insert comment-boundary marker */
    if stripped~left(1) == "#" then do
      full = full || "03"x
      iterate
    end
    /* Skip blank lines */
    if stripped == "" then iterate
    /* Check indentation of non-blank content line */
    if self~indentOf(nLine) < flowMinIndent then
      self~raiseError("Flow mapping continuation line not indented enough")
    contLine = self~stripComment(stripped)
    full = full || "02"x || contLine
  end

  /* Use matchBracket to find the true closing '}' */
  full = full~strip("Both", " " || "02"x || "03"x)
  if full~left(1) \== "{" then self~raiseError("Expected '{' at start of flow mapping")
  ep = self~matchBracket(full, "{", "}")

  inner = full~substr(2, ep - 2)~strip("Both", " " || "02"x || "03"x)
  if inner == "" then return .table~new

  map = .table~new
  do while inner \== ""
    /* Anchor prefix on key: &anchor key: value */
    keyAnchor = ""
    if inner~left(1) == "&" then do
      parse var inner kaTag inner
      keyAnchor = kaTag~substr(2)
      inner = self~flowStrip(inner)
    end
    fc = inner~left(1)
    /* Flow collection as key: [...]  or {...} */
    if fc == "[" | fc == "{" then do
      if fc == "[" then do
        kep = self~matchBracket(inner, "[", "]")
        key = self~flowSeq(inner~left(kep))
      end
      else do
        kep = self~matchBracket(inner, "{", "}")
        key = self~flowMap(inner~left(kep))
      end
      if keyAnchor \== "" then self~registerAnchor(keyAnchor, key)
      inner = self~flowStrip(inner~substr(kep + 1))
      /* Expect colon after the collection key */
      if inner~left(1) == ":" then do
        inner = self~flowStrip(inner~substr(2))
        pair = self~nextFlowVal(inner)
        map[key] = pair[1]
        inner = self~flowStrip(pair[2])
      end
      else
        map[key] = .nil
      if inner~left(1) == "," then inner = self~flowStrip(inner~substr(2))
      else if inner \== "" then
        self~raiseError("Missing comma between flow mapping entries")
      iterate
    end
    /* Alias as key: *alias : value */
    if fc == "*" then do
      aName = self~extractAliasName(inner~substr(2))
      key = self~getAnchor(aName)
      inner = self~flowStrip(inner~substr(aName~length + 2))
      if keyAnchor \== "" then self~registerAnchor(keyAnchor, key)
      if inner~left(1) == ":" then do
        inner = self~flowStrip(inner~substr(2))
        pair = self~nextFlowVal(inner)
        map[key] = pair[1]
        inner = self~flowStrip(pair[2])
      end
      else
        map[key] = .nil
      if inner~left(1) == "," then inner = self~flowStrip(inner~substr(2))
      else if inner \== "" then
        self~raiseError("Missing comma between flow mapping entries")
      iterate
    end
    colonPos = self~findFlowColon(inner)
    if colonPos = 0 then do
      parse value self~nextFlowTok(inner) with . "01"x inner
      inner = self~flowStrip(inner)
      if inner~left(1) == "," then inner = self~flowStrip(inner~substr(2))
      else if inner \== "" then
        self~raiseError("Missing comma between flow mapping entries")
      iterate
    end
    key   = self~unquoteIfNeeded(self~flowClean(inner~left(colonPos - 1)~strip))
    if keyAnchor \== "" then self~registerAnchor(keyAnchor, key)
    inner = self~flowStrip(inner~substr(colonPos + 1))
    pair = self~nextFlowVal(inner)
    map[key] = pair[1]
    inner = self~flowStrip(pair[2])
    if inner~left(1) == "," then inner = self~flowStrip(inner~substr(2))
    else if inner \== "" then
      self~raiseError("Missing comma between flow mapping entries")
  end
  return map

/** Extracts the next value from a flow collection context.
 *  Handles tag prefixes, anchor prefixes, nested flow collections,
 *  quoted strings, aliases, and plain scalars.  Validates flow
 *  scalar rules (bare '-', '#' at start, comment-boundary markers).
 *
 *  @param text the remaining flow content to parse
 *  @return a two-element .Array: [value, remainingText]
 */
::method nextFlowVal private
  use strict arg text
  text = self~flowStrip(text)
  fc = text~left(1)

  /* Tag prefix — strip and resolve */
  flowTag = ""
  if fc == "!" then do
    stripped = self~stripTag(text, .true)
    if stripped~isA(.array) then do
      flowTag = stripped[1]
      text    = stripped[2]
    end
    else
      text = stripped
    text = self~flowStrip(text)
    fc = text~left(1)
  end

  /* Anchor prefix on value: &anchor value */
  flowAnchor = ""
  if fc == "&" then do
    parse var text faTag text
    flowAnchor = faTag~substr(2)
    text = self~flowStrip(text)
    fc = text~left(1)
  end

  if fc == "[" then do
    ep = self~matchBracket(text, "[", "]")
    node = self~flowSeq(text~left(ep))
    if flowTag \== "" then node = .YamlTagged~new(flowTag, node)
    if flowAnchor \== "" then self~registerAnchor(flowAnchor, node)
    return .array~of(node, text~substr(ep + 1))
  end
  if fc == "{" then do
    ep = self~matchBracket(text, "{", "}")
    node = self~flowMap(text~left(ep))
    if flowTag \== "" then node = .YamlTagged~new(flowTag, node)
    if flowAnchor \== "" then self~registerAnchor(flowAnchor, node)
    return .array~of(node, text~substr(ep + 1))
  end
  if fc == '"' | fc == "'" then do
    parse value self~extractQuoted(text) with val "01"x rest
    if flowTag \== "" then val = .YamlTagged~new(flowTag, val)
    if flowAnchor \== "" then self~registerAnchor(flowAnchor, val)
    return .array~of(val, rest)
  end
  if fc == "*" then do
    parse value self~nextFlowTok(text) with tok "01"x rest
    return .array~of(self~getAnchor(tok~substr(2)~strip), rest)
  end
  /* '#' is a c-indicator — cannot start a plain scalar */
  if fc == "#" then
    self~raiseError("Invalid '#' at start of flow value (comment requires preceding space):" text)
  /* YAML 1.2 §7.3.1: '-' can start a plain scalar in flow context
     only if followed by an ns-plain-safe(flow) character, i.e. not
     followed by a flow indicator, whitespace, or end-of-string. */
  if fc == "-" then do
    if text~length == 1 then
      self~raiseError("Invalid flow scalar — bare '-' is not allowed in flow context")
    nc = text[2]
    if nc == " " | nc == "09"x | nc == "," | nc == "[" | nc == "]" | -
       nc == "{" | nc == "}" then
      self~raiseError("Invalid flow scalar — '-' must be followed by a safe character in flow context")
  end
  parse value self~nextFlowTok(text) with tok "01"x rest
  /* A comment-boundary marker ("03"x) inside a plain scalar token
     means a full-line comment appeared between parts of what would
     otherwise be a single token.  In flow context, a comment terminates
     a plain scalar, so the content after the comment is a separate
     entry — a missing comma.  (CML9: [ word1\n#xxx\n  word2 ])
     However, a comment BEFORE a comma is valid (7TMG: [ word1\n#comment\n, word2]).
     Only flag if there is plain content after the last comment marker. */
  if tok~pos("03"x) > 0 then do
    lastCB = tok~lastPos("03"x)
    afterCB = tok~substr(lastCB + 1)~strip("Both", " " || "02"x || "03"x)
    if afterCB \== "" then
      self~raiseError("Missing comma between flow entries (comment interrupts plain scalar)")
  end
  node = self~resolve(self~flowClean(tok~strip))
  if flowTag \== "" then node = .YamlTagged~new(flowTag, node)
  if flowAnchor \== "" then self~registerAnchor(flowAnchor, node)
  return .array~of(node, rest)

/** Tokenises the next element in a flow collection.
 *  Scans forward respecting quote and bracket nesting, stopping
 *  at an unquoted comma, closing bracket/brace, or value indicator
 *  (': ').  Also detects implicit keys that span multiple lines.
 *  Uses SOH ("01"x) as separator between token and remainder.
 *
 *  @param text the text to tokenise
 *  @return a string in the form "token || '01'x || rest"
 */
::method nextFlowTok private
  use strict arg text
  depth = 0; inSingle = .false; inDouble = .false
  do i = 1 to text~length
    ch = text[i]
    select
      when inSingle then do
        if ch == "'" then inSingle = .false
      end
      when inDouble then do
        if ch == "\" then do; i = i + 1; iterate; end
        if ch == '"' then inDouble = .false
      end
      otherwise do
        if ch == "'"      then inSingle = .true
        else if ch == '"' then inDouble = .true
        else if ch == "[" | ch == "{" then depth = depth + 1
        else if ch == "]" | ch == "}" then do
          if depth > 0 then depth = depth - 1
          else return text~left(i - 1) || "01"x || text~substr(i)
        end
        else if ch == "," & depth = 0 then
          return text~left(i - 1) || "01"x || text~substr(i)
        /* In flow context, ': ' (colon followed by space, tab,
           flow indicator, or end of string) terminates a plain
           scalar — it marks a mapping value indicator.
           YAML 1.2 §7.3.3: ns-plain(flow) cannot contain ': '. */
        else if ch == ":" & depth = 0 & i > 1 then do
          if i == text~length then do
            /* Check implicit key does not span multiple lines.
               Explicit keys (starting with '?') MAY span lines. */
            keyPart = text~left(i - 1)
            if text~strip("Leading")~left(1) \== "?" then
              if keyPart~pos("02"x) > 0 | keyPart~pos("03"x) > 0 then
                self~raiseError("Implicit flow mapping key spans multiple lines")
            return text~left(i - 1) || "01"x || text~substr(i)
          end
          nch = text[i + 1]
          if nch == " " | nch == "09"x | nch == "," | -
             nch == "[" | nch == "]" | nch == "{" | nch == "}" then do
            /* Check implicit key does not span multiple lines.
               Explicit keys (starting with '?') MAY span lines. */
            keyPart = text~left(i - 1)
            if text~strip("Leading")~left(1) \== "?" then
              if keyPart~pos("02"x) > 0 | keyPart~pos("03"x) > 0 then
                self~raiseError("Implicit flow mapping key spans multiple lines")
            return text~left(i - 1) || "01"x || text~substr(i)
          end
        end
      end
    end
  end
  return text || "01"x || ""

/** Extracts a quoted string value from text.
 *  Handles both single-quoted (with doubled-quote escaping) and
 *  double-quoted (with backslash escape sequences including \x,
 *  \u, and \U Unicode escapes).  Uses SOH ("01"x) as separator
 *  between the extracted value and the remainder.
 *
 *  @param text the text starting with a quote character
 *  @return a string in the form "value || '01'x || rest"
 */
::method extractQuoted private
  expose unescapeUnicode
  use strict arg text
  quote = text~left(1)
  i = 2; mb = .mutableBuffer~new
  do while i <= text~length
    ch = text[i]
    select
      when quote == '"' & ch == "\" then do
        i = i + 1
        if i <= text~length then do
          esc = text[i]
          if esc == "x" & (i + 2) <= text~length then do
            hex = text~substr(i + 1, 2)
            if hex~datatype("X") then do
              mb~append(x2c(hex)); i = i + 2
            end
            else do
              mapped = .Yaml~escMap~at(esc)
              if mapped \== .nil then mb~append(mapped)
              else self~raiseError("Invalid escape sequence: \" || esc)
            end
          end
          else if esc == "U" & (i + 8) <= text~length then do
            hex8 = text~substr(i + 1, 8)
            if hex8~datatype("X") then do
              if unescapeUnicode then
                mb~append(self~unicodeToUtf8(hex8))
              else
                mb~append("\U", hex8)
              i = i + 8
            end
            else do
              mapped = .Yaml~escMap~at(esc)
              if mapped \== .nil then mb~append(mapped)
              else self~raiseError("Invalid escape sequence: \" || esc)
            end
          end
          else if esc == "u" & (i + 4) <= text~length then do
            hex4 = text~substr(i + 1, 4)
            if hex4~datatype("X") then do
              if unescapeUnicode then
                mb~append(self~unicodeToUtf8(hex4))
              else
                mb~append("\u", hex4)
              i = i + 4
            end
            else do
              mapped = .Yaml~escMap~at(esc)
              if mapped \== .nil then mb~append(mapped)
              else self~raiseError("Invalid escape sequence: \" || esc)
            end
          end
          else do
            mapped = .Yaml~escMap~at(esc)
            if mapped \== .nil then mb~append(mapped)
            else self~raiseError("Invalid escape sequence: \" || esc)
          end
        end
      end
      when quote == "'" & ch == "'" then do
        if i < text~length, text[i + 1] == "'" then do
          mb~append("'"); i = i + 1
        end
        else return mb~string || "01"x || text~substr(i + 1)
      end
      when quote == '"' & ch == '"' then
        return mb~string || "01"x || text~substr(i + 1)
      otherwise
        mb~append(ch)
    end
    i = i + 1
  end
  return mb~string || "01"x || ""

/* Multi-line quoted string support.  Assembles continuation lines from
   the input array, applies YAML 1.2 line-folding rules, and delegates
   escape processing to extractQuoted.

   YAML 1.2 line folding for quoted scalars:
   - A line break between two non-empty lines is replaced by a space.
   - An empty continuation line (blank line) becomes a literal newline.
   - For double-quoted strings, a "\" at end of line (escape-newline)
     means "join the next line without any separator".
   - Leading white space on continuation lines is trimmed (it is
     indentation, not content).
*/
/** Assembles a multi-line quoted string from the input array.
 *  Reads continuation lines, applies YAML 1.2 line-folding rules
 *  (fold to space, blank lines to newlines, escape-newline for
 *  double-quoted), and delegates final processing to extractQuoted.
 *
 *  @param text the first line of the quoted string
 *  @param minIndent minimum indentation for continuation lines
 *  @return a two-element .Array: [value, trailingRest]
 */
::method multiLineQuoted private
  expose lines pos unescapeUnicode curLine
  use strict arg text, minIndent = 0

  quote = text~left(1)
  startLine = curLine           /* where the scalar opens */

  /* Check whether the quote is closed in the initial text */
  if self~isQuoteClosed(text) then do
    parse value self~extractQuoted(text) with val "01"x rest
    return .array~of(val, rest)
  end

  /* Quote is not closed — accumulate continuation lines.
     Build a single-line equivalent by applying folding rules.

     YAML 1.2 folding for quoted scalars:
     - A line break between two non-empty lines → a space.
     - One or more blank continuation lines: the line break before the
       first blank line PLUS the blank lines produce N newlines (where
       N = number of blank lines), replacing the fold-space entirely.
     - For double-quoted: "\" at end of line → join next line with no
       separator (escape-newline).
  */
  mb = .mutableBuffer~new(text)
  blankCount = 0

  do while pos <= lines~items
    nLine = lines[pos]
    curLine = pos

    /* Document markers at column 0 terminate even inside quoted scalars */
    if nLine~length >= 3 then do
      prefix3 = nLine~left(3)
      if prefix3 == "---" | prefix3 == "..." then
        if nLine~length == 3 | nLine[4] == " " | nLine[4] == "09"x then
          self~raiseError("Unterminated" quote "quoted string before document marker")
    end

    pos = pos + 1
    stripped = nLine~strip

    /* Blank continuation line — count it */
    if stripped == "" then do
      blankCount = blankCount + 1
      iterate
    end

    /* Non-blank continuation line */
    /* Check indentation: continuation must be indented past the
       containing block's indent level */
    if self~indentOf(nLine) < minIndent then
      self~raiseError("Quoted string continuation line not indented enough")

    if blankCount > 0 then do
      /* Blank lines seen: emit blankCount newlines (replacing the
         fold-space for the line break before the first blank). */
      if quote == '"' then
        mb~append(copies("\n", blankCount))
      else
        mb~append(copies("0A"x, blankCount))
      blankCount = 0
    end
    else do
      /* No blank lines: normal fold (line break → space).
         For double-quoted: "\" at end of line → join without space. */
      curText = mb~string
      if quote == '"', curText~right(1) == "\" then
        mb = .mutableBuffer~new(curText~left(curText~length - 1))
      else
        mb~append(" ")
    end

    mb~append(stripped)

    /* Check if the quote is now closed */
    if self~isQuoteClosed(mb~string) then do
      parse value self~extractQuoted(mb~string) with val "01"x rest
      return .array~of(val, rest)
    end
  end

  /* If we reach here, the quote was never closed */
  self~raiseError("Unterminated" quote "quoted string", startLine)

/* Checks whether a quoted string (starting with its quote character)
   has a matching closing quote, taking into account escape sequences
   (for double-quoted) and doubled quotes (for single-quoted). */
/** Checks whether a quoted string has a matching closing quote.
 *  Accounts for escaped characters in double-quoted strings and
 *  doubled quotes in single-quoted strings.
 *
 *  @param text the string starting with its opening quote character
 *  @return .true if the closing quote is found, .false otherwise
 */
::method isQuoteClosed private
  use strict arg text
  quote = text~left(1)
  do i = 2 to text~length
    ch = text[i]
    if quote == '"', ch == "\" then do
      i = i + 1   /* skip escaped character */
      iterate
    end
    if ch == quote then do
      if quote == "'", i < text~length, text[i + 1] == "'" then do
        i = i + 1  /* doubled single-quote — skip */
        iterate
      end
      return .true
    end
  end
  return .false

/* Converts a Unicode code point (given as a hex string of 4 or 8 characters)
   to its UTF-8 byte sequence.
   - U+0000..U+007F    → 1 byte   (0xxxxxxx)
   - U+0080..U+07FF    → 2 bytes  (110xxxxx 10xxxxxx)
   - U+0800..U+FFFF    → 3 bytes  (1110xxxx 10xxxxxx 10xxxxxx)
   - U+10000..U+10FFFF → 4 bytes  (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
*/
/** Converts a Unicode code point (hex string) to its UTF-8 encoding.
 *  Supports the full Unicode range: U+0000 to U+10FFFF, producing
 *  1 to 4 bytes as needed.  Invalid code points are returned as
 *  literal \u or \U escape strings.
 *
 *  @param hexStr a 4 or 8 character hexadecimal string
 *  @return the UTF-8 byte sequence as a string
 */
::method unicodeToUtf8 private
  use strict arg hexStr
  cp = hexStr~x2d
  if cp <= 127 then
    return d2c(cp)
  if cp <= 2047 then do
    b1 = d2c(cp % 64 + 192)
    b2 = d2c(cp // 64 + 128)
    return b1 || b2
  end
  if cp <= 65535 then do
    b1 = d2c(cp % 4096 + 224)
    b2 = d2c((cp // 4096) % 64 + 128)
    b3 = d2c(cp // 64 + 128)
    return b1 || b2 || b3
  end
  if cp <= 1114111 then do
    b1 = d2c(cp % 262144 + 240)
    b2 = d2c((cp // 262144) % 4096 + 128)
    b3 = d2c((cp // 4096) % 64 + 128)
    b4 = d2c(cp // 64 + 128)
    return b1 || b2 || b3 || b4
  end
  /* Invalid code point — store literally */
  if hexStr~length = 4 then return "\u" || hexStr
  return "\U" || hexStr

/** Finds the matching closing bracket for an opening bracket.
 *  Respects single-quote, double-quote, and nested bracket depth.
 *
 *  @param text the text to scan (must start with openBr)
 *  @param openBr the opening bracket character ('[' or '{')
 *  @param closeBr the closing bracket character (']' or '}')
 *  @return the 1-based position of the matching closeBr, or
 *          text~length if not found
 */
::method matchBracket private
  use strict arg text, openBr, closeBr
  depth = 0; inSingle = .false; inDouble = .false
  do i = 1 to text~length
    ch = text[i]
    select
      when inSingle then do; if ch == "'" then inSingle = .false; end
      when inDouble then do
        if ch == "\" then do; i = i + 1; iterate; end
        if ch == '"' then inDouble = .false
      end
      otherwise do
        if ch == "'"      then inSingle = .true
        else if ch == '"' then inDouble = .true
        else if ch == openBr  then depth = depth + 1
        else if ch == closeBr then do
          depth = depth - 1
          if depth = 0 then return i
        end
      end
    end
  end
  return text~length

/*============================================================================*/
/*  SCALAR RESOLUTION (private)                                               */
/*============================================================================*/

/** Resolves a plain scalar string to its typed ooRexx value.
 *  Recognises null words, boolean literals, integers (decimal,
 *  hex 0x, octal 0o), floats, special values (.inf, .nan), and
 *  quoted strings.  Unrecognised values are returned as strings.
 *
 *  @param text the plain scalar text to resolve
 *  @return the resolved value (.nil, .YamlBoolean, number, or string)
 */
::method resolve private
  use strict arg text

  fc = text~left(1)
  if fc == '"' | fc == "'" then return self~unquoteStr(text)

  if .Yaml~nullWords~at(text) \== .nil then return .nil
  if .Yaml~boolTrue~at(text) \== .nil  then return .YamlBoolean~true
  if .Yaml~boolFalse~at(text) \== .nil then return .YamlBoolean~false

  plain = text~changeStr("_", "")
  if plain~datatype("W") then return plain + 0

  if plain~length > 2 then do
    pf = plain~left(2)~lower
    if pf == "0x" then do
      hex = plain~substr(3)
      if hex \== "" & hex~datatype("X") then return x2d(hex)
    end
    if pf == "0o" then do
      oct = plain~substr(3)
      if self~isOctal(oct) then return self~oct2dec(oct)
    end
  end

  lp = plain~lower
  if lp == ".inf" | lp == "+.inf" then return ".inf"
  if lp == "-.inf"                then return "-.inf"
  if lp == ".nan"                 then return ".nan"

  if plain~datatype("N") then return plain + 0

  return text

/*============================================================================*/
/*  TEXT UTILITIES (private)                                                   */
/*============================================================================*/

/** Unquotes a quoted string by delegating to extractQuoted.
 *  If the text does not start with a quote character, returns it
 *  unchanged.
 *
 *  @param text a potentially quoted string
 *  @return the unquoted string value
 */
::method unquoteStr private
  use strict arg text
  fc = text~left(1)
  if fc == '"' | fc == "'" then do
    parse value self~extractQuoted(text) with val "01"x .
    return val
  end
  return text

/** Unquotes a string only if it starts with a quote character.
 *  Convenience wrapper around unquoteStr.
 *
 *  @param text a string that may or may not be quoted
 *  @return the unquoted value if quoted, or the original string
 */
::method unquoteIfNeeded private
  use strict arg text
  fc = text~left(1)
  if fc == '"' | fc == "'" then return self~unquoteStr(text)
  return text

/** Strips whitespace and flow line-boundary markers ("02"x, "03"x)
 *  from both ends of a string. Used during flow collection processing
 *  to clean up accumulated multi-line content without losing internal
 *  boundary markers needed for validation.
 */
::method flowStrip private
  use strict arg text
  return text~strip("Both", " " || "02"x || "03"x)

/** Cleans all flow line-boundary markers from a string, replacing them
 *  with spaces. Used when extracting final scalar values from flow
 *  content that was accumulated with boundary markers.
 */
::method flowClean private
  use strict arg text
  return text~changeStr("02"x, " ")~changeStr("03"x, " ")

/** Strips a trailing comment from a line of YAML text.
 *  A comment starts with ' #' (space followed by hash) outside
 *  of any quoted string context.  Respects single and double
 *  quote nesting and backslash escapes in double-quoted strings.
 *
 *  @param text the text to strip
 *  @return the text with any trailing comment removed
 */
::method stripComment private
  use strict arg text
  inSingle = .false; inDouble = .false
  do i = 1 to text~length
    ch = text[i]
    select
      when inSingle then do; if ch == "'" then inSingle = .false; end
      when inDouble then do
        if ch == "\" then do; i = i + 1; iterate; end
        if ch == '"' then inDouble = .false
      end
      otherwise do
        if ch == "'"      then inSingle = .true
        else if ch == '"' then inDouble = .true
        else if ch == "#", i > 1, text[i - 1] == " " then
          return text~left(i - 2)~strip
      end
    end
  end
  return text

/* Strips a YAML tag prefix (e.g., !!str, !!int, !custom, !!map) from the
   beginning of a text string.  Returns the remainder after the tag and any
   following whitespace.  Tags are stored as-is (shorthand form); use
   resolveTagHandle for explicit resolution against %TAG directives.
   Tags have the form:
     !word   — primary tag (e.g., !custom)
     !!word  — secondary tag (e.g., !!str, !!int, !!map)
     !handle!suffix — named tag handle (e.g., !e!person)
     !<uri>  — verbatim tag
   Without preserveTags, the tag is discarded. */
/** Strips a YAML tag prefix from the beginning of a text string.
 *  Handles verbatim tags (!<uri>), secondary tags (!!word), primary
 *  tags (!word), and named handles (!handle!suffix).  Tags are
 *  stored as-is (shorthand form); use resolveTagHandle for resolution.
 *  When preserveTags is enabled, returns a two-element .Array
 *  [tagString, remainder]; otherwise returns just the remainder.
 *
 *  @param text the text starting with '!'
 *  @param flowContext .true if parsing inside a flow collection
 *         (comma terminates tag); default .false
 *  @return the remainder string, or .Array of [tag, remainder]
 */
::method stripTag private
  expose preserveTags currentDirectives
  use strict arg text, flowContext = .false
  if text == "" then return ""
  if text~left(1) \== "!" then return text
  /* Verbatim tag: !<uri> */
  if text~length > 1, text[2] == "<" then do
    ep = text~pos(">")
    if ep > 0 then do
      tag = text~left(ep)
      val = text~substr(ep + 1)~strip
      if preserveTags then return .array~of(tag, val)
      return val
    end
    return text
  end
  /* !!tag or !handle!suffix or !tag — skip to first space or flow indicator */
  tagEnd = 0
  Do i = 2 To text~length
    ch = text[i]
    if ch == " " | ch == "09"x | ch == "02"x | ch == "03"x then do
      tagEnd = i; leave
    end
    /* Flow indicators {} [] are invalid in tag names */
    if pos(ch, "{}[]") > 0 then
      self~raiseError("Invalid character '"ch"' in tag:" text~word(1))
    /* Comma: terminates tag in flow context, invalid in block context.
       The comma itself is a flow separator and must remain in the
       returned text for the caller to process. */
    if ch == "," then do
      if flowContext then do
        tag = text~left(i - 1)
        self~validateTagHandle(tag)
        val = text~substr(i)  /* keep comma for caller */
        if preserveTags then return .array~of(tag, val)
        return val
      end
      else self~raiseError("Invalid character ',' in tag:" text~word(1))
    end
  End
  if tagEnd > 0 then do
    tag = text~left(tagEnd - 1)
    self~validateTagHandle(tag)
    val = text~substr(tagEnd + 1)~strip
    if preserveTags then return .array~of(tag, val)
    return val
  end
  /* Tag with no value after it */
  self~validateTagHandle(text)
  if preserveTags then return .array~of(text, "")
  return ""

/** Validates that a named tag handle (e.g., !prefix!suffix) is defined
 *  in the current document's %TAG directives.  The secondary handle !!
 *  and the primary handle ! are always valid.  Named handles (!word!)
 *  must have been declared via %TAG in the current document.
 */
::method validateTagHandle private
  expose currentDirectives
  use strict arg tag
  /* !! (secondary) and !word (primary, no second !) are always valid */
  if tag~left(2) == "!!" then return
  secondBang = tag~pos("!", 2)
  if secondBang == 0 then return  /* !word — primary tag, always valid */
  /* Named handle: !handle!suffix — validate that handle is declared */
  handle = tag~left(secondBang)
  tagHandles = .nil
  if currentDirectives \== .nil then
    tagHandles = currentDirectives~at("tagHandles")
  if tagHandles \== .nil then
    if tagHandles~at(handle) \== .nil then return
  self~raiseError("Undeclared tag handle '"handle"' in tag:" tag)

/** Resolves a tag handle using a tag-handles table from %TAG directives.
 *  For example, with %TAG !e! tag:example.com,2000: the tag
 *  !e!person becomes !&lt;tag:example.com,2000:person&gt;.
 *  If no matching handle is found, the tag is returned unchanged.
 *  This method supports lazy resolution: tags are stored as-is during
 *  parsing and can be resolved on demand by the caller.
 *
 *  @param tag        the tag string to resolve (e.g. "!e!person", "!!int")
 *  @param tagHandles a .Table mapping handle strings to URI prefixes,
 *                    typically obtained from directivesMap[doc]["tagHandles"]
 *  @return the resolved tag as a verbatim tag, or the original if no
 *          matching handle is found
 */
::method resolveTagHandle
  use strict arg tag, tagHandles
  if tagHandles == .nil then return tag
  /* Try to match a tag handle: !!, !x!, or ! */
  /* Check named handles first: !x!suffix */
  secondBang = tag~pos("!", 2)
  if secondBang > 0 then do
    handle = tag~left(secondBang)
    suffix = tag~substr(secondBang + 1)
    prefix = tagHandles~at(handle)
    if prefix \== .nil then
      return "!<" || prefix || suffix || ">"
  end
  /* Check primary handle: !suffix (handle is "!") */
  if tag~left(2) \== "!!" then do
    prefix = tagHandles~at("!")
    if prefix \== .nil then
      return "!<" || prefix || tag~substr(2) || ">"
  end
  return tag

/** Returns the number of leading space characters in a line.
 *  Tab characters are not counted as indentation (YAML 1.2 §6.1).
 *
 *  @param line the line to measure
 *  @return the number of leading spaces
 */
::method indentOf private
  use strict arg line
  do i = 1 to line~length
    if line[i] \== " " then return i - 1
  end
  return line~length

/** Checks that no tab character appears between the leading spaces and the
 *  first content character.  Call this where block structure indentation
 *  matters (blockMap / blockSeq iteration).
 */
::method checkTabIndent private
  use strict arg line
  indent = self~indentOf(line)
  nextPos = indent + 1
  if nextPos <= line~length then do
    if line[nextPos] == "09"x then
      self~raiseError("Tab character in indentation (YAML 1.2 forbids tabs for indentation)")
  end

/** Rejects a nested implicit mapping on the same line as its key:
 *  "a: b: c" (ZCZ6).  In block context, a second ": " in the inline value
 *  would start a nested mapping, which is not allowed on the same line as
 *  the parent key.  Only applies to plain scalar values: anchors, aliases,
 *  tags, quoted strings, and flow collections are skipped.  Used by
 *  blockMap and by compactMap ("- a: b: c").
 *
 *  @param valPart the inline value, after the key's ": "
 */
::method checkNoNestedMapping private
  use strict arg valPart
  if valPart == "" then return
  vpc = valPart~left(1)
  if vpc == "&" | vpc == "*" | vpc == "!" | vpc == "'" | vpc == '"' | -
     vpc == "[" | vpc == "{" then return
  if self~findMapColon(valPart) > 0 then
    self~raiseError("Mapping value is not allowed in this context:" valPart)

/** Rejects a plain scalar that starts with a reserved indicator.
 *  c-directive (%) and c-reserved (@ `) cannot start a plain scalar
 *  (YAML 1.2 §7.3.3, ns-plain-first), in any position: a key, an inline
 *  value, or a sequence item.
 *
 *  @param text the scalar as written (quoted text is not checked)
 */
::method checkPlainFirst private
  use strict arg text
  fc = text~left(1)
  if fc == "%" | fc == "@" | fc == "`" then
    self~raiseError("Reserved indicator" fc "cannot start a plain scalar")

/** As checkPlainFirst, for an implicit mapping key.  Keys that are not
 *  plain scalars (quoted, aliases, tags, flow collections, "?") are not
 *  checked.
 *
 *  @param key the key as written
 */
::method checkPlainKey private
  use strict arg key
  if key~isA(.string) then self~checkPlainFirst(key)

/** Advances the position past blank lines and comment-only lines.
 *  Stops at the first line that has non-comment content.
 */
::method skipBlanks private
  expose lines pos
  do while pos <= lines~items
    stripped = lines[pos]~strip
    if stripped == "" | stripped~left(1) == "#" then pos = pos + 1
    else leave
  end

/** Finds the position of a mapping value indicator ': ' in block context.
 *  Skips colons inside quoted strings and flow collections.
 *  Also recognises a trailing colon at end of line.  Returns 0 if
 *  a comment is encountered before any colon, or if no valid colon
 *  is found.
 *
 *  @param text the text to search
 *  @return the 1-based position of ':', or 0 if not found
 */
::method findMapColon private
  use strict arg text
  inSingle = .false; inDouble = .false; flowDepth = 0
  do i = 1 to text~length
    ch = text[i]
    select
      when inSingle then do; if ch == "'" then inSingle = .false; end
      when inDouble then do
        if ch == "\" then do; i = i + 1; iterate; end
        if ch == '"' then inDouble = .false
      end
      otherwise do
        /* Only enter quote mode at position 1 (quoted key) or when
           flowDepth > 0 (inside a flow collection used as key).
           Mid-key quotes in plain scalars are literal characters. */
        if ch == "'", (i == 1 | flowDepth > 0) then inSingle = .true
        else if ch == '"', (i == 1 | flowDepth > 0) then inDouble = .true
        else if ch == "[" | ch == "{" then flowDepth = flowDepth + 1
        else if ch == "]" | ch == "}" then do
          if flowDepth > 0 then flowDepth = flowDepth - 1
        end
        else if ch == "#", i > 1, text[i - 1] == " " then do
          if flowDepth == 0 then return 0
        end
        else if ch == ":", flowDepth == 0 then do
          if i == text~length then return i
          nch = text[i + 1]
          if nch == " " | nch == "09"x then return i
        end
      end
    end
  end
  return 0

/** Finds the position of a value indicator ':' in flow context.
 *  Similar to findMapColon but uses flow-context termination rules:
 *  colon followed by space, comma, closing brace, or tab.
 *  Respects quote and bracket nesting.
 *
 *  @param text the text to search
 *  @return the 1-based position of ':', or 0 if not found
 */
::method findFlowColon private
  use strict arg text
  depth = 0; inSingle = .false; inDouble = .false
  do i = 1 to text~length
    ch = text[i]
    select
      when inSingle then do; if ch == "'" then inSingle = .false; end
      when inDouble then do
        if ch == "\" then do; i = i + 1; iterate; end
        if ch == '"' then inDouble = .false
      end
      otherwise do
        if ch == "'"      then inSingle = .true
        else if ch == '"' then inDouble = .true
        else if ch == "[" | ch == "{" then depth = depth + 1
        else if ch == "]" | ch == "}" then depth = depth - 1
        else if ch == ":" & depth = 0 then do
          if i == text~length then return i
          nch = text[i + 1]
          if nch == " " | nch == "," | nch == "}" | nch == "09"x then return i
        end
      end
    end
  end
  return 0

/** Counts occurrences of a target character outside quoted strings.
 *  Used to determine bracket balance in flow collections.
 *
 *  @param text the text to scan
 *  @param target the character to count
 *  @return the count of unquoted occurrences
 */
::method countUnquoted private
  use strict arg text, target
  count = 0; inSingle = .false; inDouble = .false
  do i = 1 to text~length
    ch = text[i]
    select
      when inSingle then do; if ch == "'" then inSingle = .false; end
      when inDouble then do
        if ch == "\" then do; i = i + 1; iterate; end
        if ch == '"' then inDouble = .false
      end
      otherwise do
        if ch == "'"       then inSingle = .true
        else if ch == '"'  then inDouble = .true
        else if ch == target then count = count + 1
      end
    end
  end
  return count

/** Tests whether a string consists entirely of octal digits (0-7).
 *
 *  @param text the string to test
 *  @return .true if all characters are octal digits, .false otherwise
 */
::method isOctal private
  use strict arg text
  if text == "" then return .false
  do i = 1 to text~length
    ch = text[i]
    if ch < "0" | ch > "7" then return .false
  end
  return .true

/** Converts an octal string to its decimal integer value.
 *
 *  @param text a string of octal digits
 *  @return the decimal integer value
 */
::method oct2dec private
  use strict arg text
  val = 0
  do i = 1 to text~length
    val = val * 8 + text[i]
  end
  return val

/** Extracts an alias or anchor name from text.
 *  Scans until whitespace or a flow indicator character
 *  ([ ] { } ,) is found.
 *
 *  @param text the text after the '*' or '&' character
 *  @return the anchor/alias name string
 */
::method extractAliasName private
  use strict arg text
  /* Alias/anchor names end at whitespace, flow indicators, or end of string.
     Per YAML 1.2: ns-anchor-char ::= ns-char - c-flow-indicator
     Flow indicators: [ ] { } ,                                              */
  do i = 1 to text~length
    ch = text[i]
    if ch == " " | ch == "09"x | ch == "[" | ch == "]" | -
       ch == "{" | ch == "}" | ch == "," then
      return text~left(i - 1)
  end
  return text

/** Retrieves the value of a previously defined anchor.
 *  Raises a parse error if the anchor name is not defined.
 *
 *  @param name the anchor name to look up
 *  @return the anchored value
 */
::method getAnchor private
  expose anchors anchorMap
  use strict arg name
  if \anchors~hasIndex(name) then self~raiseError("Unknown alias: *" || name)
  return anchors~at(name)

/** Registers an anchor name and its associated node.
 *  Stores the mapping in both the anchors table (name to value)
 *  and anchorMap (value to name, via identity).
 *
 *  @param name the anchor name
 *  @param node the node object to associate
 */
::method registerAnchor private
  expose anchors anchorMap
  use strict arg name, node
  anchors[name] = node
  if node \== .nil then anchorMap[node] = name

/** Raises a YAML parse error with context information.
 *  Constructs a YamlError with the message, current line number,
 *  and the content of the current line (if available).
 *
 *  @param msg the error description
 */
::method raiseError private
  expose lines pos curLine lineOffset
  use strict arg msg, line = .nil
  /* The line to report: the one given, else the line whose text is being
     parsed (curLine), else the current position.  'pos' alone is not
     enough: most checks run after their line has been consumed, when
     'pos' already points at the NEXT line. */
  if line == .nil then line = curLine
  if line == 0 then line = pos
  if line > lines~items then line = lines~items
  ctx = ""
  if line >= 1 then do
    ctx  = lines[line]
    line = line + lineOffset    /* parseFrontMatter: lines of the document */
  end
  else line = 0
  raise syntax 93.900 additional(.YamlError~new(msg, line, 0, ctx)~makeString)

/*============================================================================*/
/*  yaml.deepEqual                                                                 */
/*============================================================================*/

/** Recursively compares two ooRexx objects for semantic equality.
 *  <code>YamlTagged</code> objects are compared by tag (strict) and
 *  by recursive value comparison.  <code>.Table</code> objects are
 *  compared by keys and values (order-independent),
 *  <code>.Array</code> objects by length and element-wise equality
 *  (order-dependent), and scalars by strict comparison
 *  (<code>==</code>).
 *
 *  @param expected the expected object (may be <code>.nil</code>,
 *           .YamlTagged, .Table, .Array, or a scalar)
 *  @param actual the actual object
 *  @return <code>.true</code> if the two objects are semantically equal,
 *          <code>.false</code> otherwise
 */
::routine yaml.deepEqual public
  use arg expected, actual
  /* Both .nil */
  if expected == .nil, actual == .nil then return .true
  if expected == .nil | actual == .nil then return .false
  /* YamlTagged: same tag, same value */
  if expected~isA(.YamlTagged) then do
    if \actual~isA(.YamlTagged) then return .false
    if expected~tag \== actual~tag then return .false
    return yaml.deepEqual(expected~value, actual~value)
  end
  if actual~isA(.YamlTagged) then return .false
  /* Table: same keys, same values */
  if expected~isA(.table) then do
    if \actual~isA(.table) then return .false
    if expected~items \= actual~items then return .false
    sup = expected~supplier
    do while sup~available
      k = sup~index
      /* String keys: direct lookup */
      if k~isA(.string) then do
        if \actual~hasIndex(k) then return .false
        if \yaml.deepEqual(sup~item, actual[k]) then return .false
      end
      /* Non-string keys: find matching key in actual by deep comparison */
      else do
        found = .false
        bSup = actual~supplier
        do while bSup~available
          if yaml.deepEqual(k, bSup~index) then do
            if \yaml.deepEqual(sup~item, bSup~item) then return .false
            found = .true
            leave
          end
          bSup~next
        end
        if \found then return .false
      end
      sup~next
    end
    return .true
  end
  /* Array: same length, same elements in order */
  if expected~isA(.array) then do
    if \actual~isA(.array) then return .false
    if expected~items \= actual~items then return .false
    do i = 1 to expected~items
      if \yaml.deepEqual(expected[i], actual[i]) then return .false
    end
    return .true
  end
  /* Scalar comparison */
  return expected == actual

/*============================================================================*/
/*  YamlEmitter                                                               */
/*============================================================================*/

/** Serialises an ooRexx object tree to a YAML block-style string.
 *
 *  <p>This class is used internally by the <code>Yaml~toYaml*</code> class
 *  methods, but can also be used directly for finer control over emission.
 *
 *  <p>Supports anchors/aliases and merge-key reconstruction when the
 *  corresponding metadata tables from a previous parse are supplied.
 *
 *  @see Yaml#toYaml
 */
::class YamlEmitter public

/** Creates a new YamlEmitter instance.
 *
 *  @param indentSize      optional number of spaces per indentation level
 *                         (default 2)
 *  @param anchorMap       optional .IdentityTable mapping objects to anchor
 *                         names; if <code>.nil</code>, an empty table is used
 *  @param mergeSourceMap  optional .IdentityTable mapping target mappings to
 *                         arrays of merge sources; if <code>.nil</code>, an
 *                         empty table is used
 *  @param directivesMap   optional .IdentityTable mapping root objects to
 *                         directives Tables (from <code>Yaml~directivesMap</code>);
 *                         if <code>.nil</code>, an empty table is used
 */
::method init
  expose indent anchorMap emitted mergeSourceMap directivesMap
  use strict arg indentSize = 2, anchorMap = .nil, mergeSourceMap = .nil, directivesMap = .nil
  indent = indentSize
  if anchorMap == .nil then anchorMap = .identityTable~new
  if mergeSourceMap == .nil then mergeSourceMap = .identityTable~new
  if directivesMap == .nil then directivesMap = .identityTable~new
  emitted = .identityTable~new

/** Emits the given ooRexx object tree as a YAML string.
 *
 *  @param obj the root object to serialise (.Table, .Array, scalar,
 *             or <code>.nil</code>)
 *  @return a YAML-formatted string terminated by a single newline
 */
::method emit
  expose indent directivesMap
  use strict arg obj
  mb = .mutableBuffer~new
  /* Emit directives if present for this root object */
  directives = directivesMap~at(obj)
  if directives \== .nil then do
    yamlVersion = directives~at("yamlVersion")
    if yamlVersion \== .nil then
      mb~append("%YAML ", yamlVersion, "0A"x)
    tagHandles = directives~at("tagHandles")
    if tagHandles \== .nil then do
      sup = tagHandles~supplier
      do while sup~available
        mb~append("%TAG ", sup~index, " ", sup~item, "0A"x)
        sup~next
      end
    end
    mb~append("---", "0A"x)
  end
  nodeOut = self~emitNode(obj, 0)
  out = nodeOut~strip("T", "0A"x)
  mb~append(out, "0A"x)
  return mb~string

/** Recursively serialises an ooRexx object to YAML block style.
 *  Handles mappings (with merge key reconstruction and anchor
 *  ordering), sequences (with compact notation for mapping items),
 *  and scalars.  Detects already-emitted objects to generate
 *  aliases (*name) instead of duplicating content.
 *
 *  @param obj the object to emit
 *  @param level the current indentation level (in spaces)
 *  @return the YAML string fragment for this node
 */
::method emitNode private
  expose indent anchorMap emitted mergeSourceMap
  use strict arg obj, level

  /* YamlTagged wrapper — emit tag prefix, then the inner value */
  if obj~isA(.YamlTagged) then do
    tag = obj~tag
    inner = obj~value
    child = self~emitNode(inner, level)
    /* Inline child (no newlines) — tag + space + child */
    if child~pos("0A"x) = 0 then return tag || " " || child
    /* Block scalar child (| or >) — tag on the same line as the header,
       e.g. "!custom >\n  content\n". */
    if child~left(1) == "|" | child~left(1) == ">" then
      return tag || " " || child
    /* Block collection child — tag on its own, child on next lines */
    return tag || "0A"x || child
  end

  if obj == .nil then return "null"

  /* Check for alias (already emitted anchored object) */
  anchorName = anchorMap~at(obj)
  if anchorName \== .nil then do
    if emitted~at(obj) \== .nil then
      return "*" || anchorName
    emitted[obj] = anchorName
  end

  /* Anchor prefix for this node (empty string if none) */
  aPrefix = ""
  if anchorName \== .nil then aPrefix = "&" || anchorName || " "

  /* Mapping */
  if obj~isA(.MapCollection) then do
    if obj~items = 0 then return aPrefix || "{}"

    /* Try flow style for small, simple, nested mappings (not top-level) */
    mergeSources = mergeSourceMap~at(obj)
    if level > 0, aPrefix == "", mergeSources == .nil, -
       self~canFlowStyle(obj) then do
      flow = self~emitFlowMap(obj)
      if flow \== .nil then return flow
    end

    mb = .mutableBuffer~new

    /* Determine which keys came from merges (if any) */
    mergedKeys = .table~new
    if mergeSources \== .nil then do
      /* A key is "merged" if it exists in a merge source with the same
         value.  For scalars we compare by value (=); for collections
         we compare by object identity (same reference). A key that was
         overridden by the target mapping is NOT considered merged. */
      do src over mergeSources
        sup = src~supplier
        do while sup~available
          k = sup~index
          if obj~hasIndex(k) then do
            objVal = obj[k]
            srcVal = sup~item
            if objVal~isA(.MapCollection) | objVal~isA(.OrderedCollection) then do
              if objVal~identityHash == srcVal~identityHash then
                mergedKeys[k] = .true
            end
            else if objVal = srcVal then
              mergedKeys[k] = .true
          end
          sup~next
        end
      end
    end

    /* Order keys so that anchor definitions come before aliases.
       This prevents "unknown alias" errors when re-parsing the dump. */
    keys = self~orderedKeys(obj)

    /* Emit merge key(s) first (if any) — mirrors typical YAML style */
    if mergeSources \== .nil then do
      pad = copies(" ", level)
      if mergeSources~items = 1 then do
        /* Single merge: <<: *alias */
        src = mergeSources[1]
        srcAnchor = anchorMap~at(src)
        if srcAnchor \== .nil then
          mb~append(pad, "<<: *", srcAnchor, "0A"x)
      end
      else do
        /* Multiple merges: <<: [*a, *b, ...] */
        refs = .array~new
        do src over mergeSources
          srcAnchor = anchorMap~at(src)
          if srcAnchor \== .nil then
            refs~append("*" || srcAnchor)
        end
        if refs~items > 0 then
          mb~append(pad, "<<: [", refs~makeString('L', ", "), "]", "0A"x)
      end
    end

    /* Emit own (non-merged) keys */
    do key over keys
      if mergedKeys~hasIndex(key) then iterate
      pad  = copies(" ", level)
      val  = obj[key]

      /* Non-string key — emit using explicit key indicator (?) */
      if \key~isA(.string) then do
        keyChild = self~emitNode(key, level + indent)
        child    = self~emitNode(val, level + indent)
        /* Inline key (flow collection or scalar) */
        if keyChild~pos("0A"x) = 0 then
          mb~append(pad, "? ", keyChild, "0A"x)
        /* Block key — on next line */
        else
          mb~append(pad, "?", "0A"x, keyChild)
        /* Value */
        if child~pos("0A"x) = 0 then
          mb~append(pad, ": ", child, "0A"x)
        else if self~isBlockChild(child) then
          mb~append(pad, ": ", child)
        else do
          valAnchor = self~anchorPrefix(val)
          mb~append(pad, ":", valAnchor, "0A"x, child)
        end
        iterate
      end

      qkey = self~quoteKey(key)
      child = self~emitNode(val, level + indent)
      /* If child is inline (no newlines) — alias, scalar, or flow collection */
      if child~pos("0A"x) = 0 then
        mb~append(pad, qkey, ": ", child, "0A"x)
      /* Block scalar (starts with | or >, possibly with tag) — emit inline after ": " */
      else if self~isBlockChild(child) then
        mb~append(pad, qkey, ": ", child)
      /* Block collection — emit on next line with indentation */
      else do
        valAnchor = self~anchorPrefix(val)
        mb~append(pad, qkey, ":", valAnchor, "0A"x, child)
      end
    end

    return mb~string
  end

  /* Sequence */
  if obj~isA(.OrderedCollection) then do
    if obj~items = 0 then return aPrefix || "[]"
    /* Try flow style for small, simple, nested sequences (not top-level) */
    if level > 0, aPrefix == "", self~canFlowStyle(obj) then do
      flow = self~emitFlowSeq(obj)
      if flow \== .nil then return flow
    end
    mb = .mutableBuffer~new
    do item over obj
      pad = copies(" ", level)
      if item == .nil then do
        mb~append(pad, "- null", "0A"x)
        iterate
      end
      child = self~emitNode(item, level + 2)
      /* Inline child (scalar, alias, flow collection) — no newlines */
      if child~pos("0A"x) = 0 then
        mb~append(pad, "- ", child, "0A"x)
      /* Block mapping child — compact notation: first key on same line as "- " */
      else if item~isA(.MapCollection) then do
        itemAnchor = self~anchorPrefix(item)
        firstLine = ""
        restLines = ""
        nlPos = child~pos("0A"x)
        if nlPos > 0 then do
          firstLine = child~left(nlPos - 1)
          restLines = child~substr(nlPos)
        end
        else firstLine = child
        mb~append(pad, "- ", itemAnchor, firstLine, "0A"x)
        if restLines \== "" then mb~append(restLines)
      end
      /* Block sequence child — nested on next line */
      else if item~isA(.OrderedCollection) then
        mb~append(pad, "-", "0A"x, child)
      /* Block scalar (starts with | or >) — inline after "- " */
      else
        mb~append(pad, "- ", child)
    end
    return mb~string
  end

  /* Scalar */
  return aPrefix || self~emitScalar(obj, level)

/* Returns the anchor prefix " &name " for an object, if it was just registered
   as anchored (i.e., it's in emitted). Returns "" otherwise. */
/** Returns an anchor prefix string for a value that has already been
 *  emitted (i.e. is an alias target).  Returns " &name" if the
 *  value appears in emitted, or "" otherwise.
 *
 *  @param obj the object to check
 *  @return the anchor prefix string, or ""
 */
::method anchorPrefix private
  expose anchorMap emitted
  use strict arg obj
  if obj == .nil then return ""
  anchorName = emitted~at(obj)
  if anchorName \== .nil then return " &" || anchorName
  return ""

/* Returns keys of a Table ordered so that anchor definitions come
   before aliases.  Keys whose values define an anchor (in anchorMap but
   not yet emitted) come first; keys whose values are aliases (already
   emitted) come second; other keys in their natural hash order. */
/** Returns the keys of a mapping ordered so that anchor definitions
 *  come before aliases.  This prevents "unknown alias" errors when
 *  re-parsing the emitted YAML.  Keys whose values define anchors
 *  appear first, then regular keys, then alias keys.
 *
 *  @param map the .MapCollection to order keys for
 *  @return an .Array of keys in emission order
 */
::method orderedKeys private
  expose anchorMap emitted
  use strict arg map
  /* If no anchorMap in use, return keys in hash order */
  if anchorMap~items = 0 then do
    keys = .array~new
    sup = map~supplier
    do while sup~available
      keys~append(sup~index)
      sup~next
    end
    return keys
  end
  /* Separate keys into anchor-definers and alias-users.
     Multiple keys can point to the same object (via aliases).  The first
     key encountered for a given object is the "anchor definer"; subsequent
     keys with the same object identity are aliases. We use a local
     IdentityTable to track which objects we have already assigned. */
  anchorKeys = .array~new
  aliasKeys  = .array~new
  otherKeys  = .array~new
  seen       = .identityTable~new
  sup = map~supplier
  do while sup~available
    val = sup~item
    aName = anchorMap~at(val)
    if aName \== .nil then do
      if seen~at(val) \== .nil then
        aliasKeys~append(sup~index)
      else do
        seen[val] = .true
        anchorKeys~append(sup~index)
      end
    end
    else
      otherKeys~append(sup~index)
    sup~next
  end
  /* Anchor definitions first, then other keys, then aliases */
  orderedList = .array~new
  do k over anchorKeys
    orderedList~append(k)
  end
  do k over otherKeys
    orderedList~append(k)
  end
  do k over aliasKeys
    orderedList~append(k)
  end
  return orderedList

/** Serialises a scalar value to its YAML string representation.
 *  Selects the most appropriate quoting style: unquoted for simple
 *  values, single-quoted for strings with special characters but no
 *  control chars, double-quoted for strings needing escapes, block
 *  literal (|) or folded (>) for multi-line strings with trailing
 *  newlines, and quoted for reserved words.
 *
 *  @param value the scalar to serialise
 *  @param bodyIndent indentation level for block scalar body lines
 *  @return the YAML scalar string
 */
::method emitScalar private
  expose indent
  use strict arg value, bodyIndent = 2
  pad = copies(" ", bodyIndent)
  /* Explicit indent indicator for block scalars — needed when any
     content line starts with a space or tab, so the parser knows
     where the indentation ends and content begins. */
  indentIndicator = ""
  if value == .nil then return "null"
  if value~isA(.LogicalWrapper) then return .YamlBoolean~of(value)~makeYAML
  if \value~isA(.string) then return value~string
  select case value
    when "" then return '""'
    when "null", "Null", "NULL", "~",  -
         "true", "True", "TRUE",       -
         "false", "False", "FALSE",    -
         "yes", "Yes", "YES",          -
         "no", "No", "NO",            -
         "on", "On", "ON",            -
         "off", "Off", "OFF" then return "'" || value || "'"
    otherwise nop
  end
  if value~pos("0A"x) > 0 then do
    /* String contains newlines — decide between block scalar and double-quoted.
       If the string does NOT end with a newline, use double-quoted with \n
       escapes so the round-trip is exact (no trailing newline added).
       If it ends with newline(s), use block scalar with chomp indicator.

       Choose between literal (|) and folded (>):
         - Folded (>) is used when the content (excluding trailing newlines)
           is a single line of text — the emitter wraps it into multiple
           indented lines that the parser will fold back.
         - Literal (|) is used when the content has multiple distinct lines
           (embedded newlines), since those would be lost by folding.
    */
    if value~right(1) \== "0A"x then
      return '"' || self~escapeStr(value) || '"'
    /* Count trailing newlines */
    trailingNLs = 0
    do i = value~length to 1 by -1
      if value[i] == "0A"x then trailingNLs = trailingNLs + 1
      else leave
    end
    /* Content without trailing newlines */
    contentLen = value~length - trailingNLs
    content = value~left(contentLen)

    /* Decide chomp suffix */
    if trailingNLs > 1 then chompSuffix = "+"
    else                    chompSuffix = ""

    /* If any content line starts with a space or tab, we need an explicit
       indent indicator so the parser can distinguish indentation from
       content.  The indicator value is the relative indent (indent step). */
    contentLines = content~makeArray("0A"x)
    do cl over contentLines
      if cl \== "", cl~left(1) == " " | cl~left(1) == "09"x then do
        indentIndicator = indent
        leave
      end
    end

    /* Use folded (>) when content is a single line (no embedded newlines)
       and does not start with a space (which would trigger more-indented
       literal blocks in folded mode). */
    if content~pos("0A"x) = 0, content~left(1) \== " " then do
      mb = .mutableBuffer~new
      mb~append(">" || chompSuffix, "0A"x)
      self~emitFoldedLines(mb, content, pad)
      /* Emit trailing blank lines for keep chomp — must be indented
         so that emit()'s trailing-newline normalization does not
         strip them. */
      do i = 2 to trailingNLs
        mb~append(pad, "0A"x)
      end
      return mb~string
    end

    /* Otherwise use literal (|) */
    mb = .mutableBuffer~new
    mb~append("|" || indentIndicator || chompSuffix, "0A"x)
    do vl over value~makeArray("0A"x)
      mb~append(pad, vl, "0A"x)
    end
    return mb~string
  end
  if self~needsQuoting(value) then do
    /* Prefer single quotes when the string has no characters that
       need escape sequences (control chars, backslash).  Single-quoted
       strings only need to double any embedded single quotes. */
    if self~canSingleQuote(value) then
      return "'" || value~changeStr("'", "''") || "'"
    return '"' || self~escapeStr(value) || '"'
  end
  return value

/* Wraps a single line of text into folded block scalar body lines,
   breaking at word boundaries to keep lines within a reasonable width.
   Each output line is indented with <code>pad</code> and terminated
   with a newline. */
/** Wraps a single line of text into folded block scalar body lines.
 *  Breaks at word boundaries to keep lines within a reasonable
 *  width (~78 chars minus indentation).  Each output line is indented
 *  with pad and terminated with a newline.
 *
 *  @param mb the .MutableBuffer to append to
 *  @param text the text to fold
 *  @param pad the indentation prefix string
 */
::method emitFoldedLines private
  use strict arg mb, text, pad = "  "
  maxWidth = 78 - pad~length
  if maxWidth < 20 then maxWidth = 20
  if text~length <= maxWidth then do
    mb~append(pad, text, "0A"x)
    return
  end
  rest = text
  do while rest~length > maxWidth
    /* Find last space at or before maxWidth */
    breakPos = rest~lastPos(" ", maxWidth + 1)
    if breakPos = 0 then do
      /* No space found — find first space after maxWidth */
      breakPos = rest~pos(" ", maxWidth + 1)
      if breakPos = 0 then leave  /* no space at all — emit as-is */
    end
    mb~append(pad, rest~left(breakPos - 1), "0A"x)
    rest = rest~substr(breakPos + 1)
  end
  if rest \== "" then
    mb~append(pad, rest, "0A"x)

/* Returns .true if the emitted <code>child</code> string starts with a
   block scalar header (<code>|</code> or <code>&gt;</code>), possibly
   preceded by a YAML tag (e.g. <code>!custom &gt;\n  body\n</code>).
   Used by emitNode to decide whether to inline block scalars after
   <code>:</code> or <code>-</code>. */
/** Tests whether an emitted child string starts with a block scalar
 *  header ('|' or '>'), possibly preceded by a YAML tag.  Used by
 *  emitNode to decide whether to inline block scalars after ':' or '-'.
 *
 *  @param child the emitted child string to test
 *  @return .true if the child is a block scalar, .false otherwise
 */
::method isBlockChild private
  use strict arg child
  fc = child~left(1)
  if fc == "|" | fc == ">" then return .true
  /* Tag prefix: starts with "!" — check if the first line contains
     a block scalar indicator after the tag. */
  if fc == "!" then do
    nlPos = child~pos("0A"x)
    if nlPos = 0 then return .false
    firstLine = child~left(nlPos - 1)~strip("T")
    /* Find the space separating the tag from the rest */
    spPos = firstLine~pos(" ")
    if spPos > 0 then do
      afterTag = firstLine~substr(spPos + 1)~strip("L")
      atc = afterTag~left(1)
      if atc == "|" | atc == ">" then return .true
    end
  end
  return .false

/* Returns .true if a string can be safely represented in single quotes.
   This is possible when the string contains no control characters
   (which would need \-escapes) and no backslashes (which would be literal
   in single-quoted strings but may confuse round-tripping). */
/** Tests whether a string can be safely single-quoted.
 *  Returns .false if the string contains backslashes or control
 *  characters (which need double-quote escape sequences).
 *
 *  @param text the string to test
 *  @return .true if single-quoting is safe, .false otherwise
 */
::method canSingleQuote private
  use strict arg text
  if text~pos("\") > 0 then return .false
  do i = 1 to text~length
    c = text[i]~c2d
    if c < 32 | c = 127 then return .false
  end
  return .true

/** Tests whether a scalar value requires quoting in YAML output.
 *  Returns .true for empty strings, strings starting with YAML
 *  indicators, strings containing ': ', ' #', leading/trailing
 *  spaces, quotes, backslashes, or control characters.
 *
 *  @param text the string to test
 *  @return .true if quoting is required, .false otherwise
 */
::method needsQuoting private
  use strict arg text
  if text == "" then return .true
  fc = text~left(1)
  if "&*[]{}|>!%@`#,?-:'"~pos(fc) > 0 then return .true
  if text~pos(": ") > 0 | text~pos(" #") > 0 then return .true
  if text~left(1) == " " | text~right(1) == " " then return .true
  if text~pos('"') > 0 | text~pos("\") > 0 then return .true
  /* Check for control characters that need escaping */
  do i = 1 to text~length
    c = text[i]~c2d
    if c < 32 | c = 127 then return .true
  end
  return .false

/** Quotes a mapping key if necessary.
 *  Applies needsQuoting and isReservedWord checks.  Prefers
 *  single quotes when possible; falls back to double quotes
 *  for strings needing escape sequences.
 *
 *  @param key the key string to quote
 *  @return the quoted or unquoted key string
 */
::method quoteKey private
  use strict arg key
  if self~needsQuoting(key) then do
    if self~canSingleQuote(key) then
      return "'" || key~changeStr("'", "''") || "'"
    return '"' || self~escapeStr(key) || '"'
  end
  /* Quote keys that look like YAML reserved words */
  if self~isReservedWord(key) then
    return "'" || key~changeStr("'", "''") || "'"
  return key

/** Tests whether a string is a YAML reserved word that requires
 *  quoting when used as a mapping key.  Includes null, boolean,
 *  and yes/no/on/off variants, as well as numeric-looking strings.
 *
 *  @param text the string to test
 *  @return .true if the string is a reserved word, .false otherwise
 */
::method isReservedWord private
  use strict arg text
  select case text
    when "null", "Null", "NULL", "~",  -
         "true", "True", "TRUE",       -
         "false", "False", "FALSE",    -
         "yes", "Yes", "YES",          -
         "no", "No", "NO",            -
         "on", "On", "ON",            -
         "off", "Off", "OFF" then return .true
    otherwise nop
  end
  /* Also quote keys that look like numbers */
  plain = text~changeStr("_", "")
  if plain~datatype("N") then return .true
  return .false

/* Returns .true if a collection is small/simple enough for flow style.
   Criteria: no anchors, all values are simple scalars, <= 4 items,
   and the resulting flow string would be <= 80 chars. */
/** Tests whether a collection is small and simple enough for flow
 *  style emission.  Criteria: no anchored values, all values are
 *  simple scalars (no nested collections, no tags, no multi-line
 *  strings), and at most 4 items.
 *
 *  @param obj the .MapCollection or .OrderedCollection to test
 *  @return .true if flow style is appropriate, .false otherwise
 */
::method canFlowStyle private
  expose anchorMap
  use strict arg obj
  if obj~isA(.MapCollection) then do
    if obj~items = 0 | obj~items > 4 then return .false
    sup = obj~supplier
    do while sup~available
      /* Non-string keys cannot be represented in flow style */
      if \sup~index~isA(.string) then return .false
      if sup~index~isA(.YamlTagged) then return .false
      if anchorMap~at(sup~item) \== .nil then return .false
      if sup~item == .nil then do; sup~next; iterate; end
      if sup~item~isA(.YamlTagged) then return .false
      if sup~item~isA(.MapCollection) | sup~item~isA(.OrderedCollection) then return .false
      if sup~item~isA(.string), sup~item~pos("0A"x) > 0 then return .false
      sup~next
    end
    return .true
  end
  if obj~isA(.OrderedCollection) then do
    if obj~items = 0 | obj~items > 4 then return .false
    do item over obj
      if anchorMap~at(item) \== .nil then return .false
      if item == .nil then iterate
      if item~isA(.YamlTagged) then return .false
      if item~isA(.MapCollection) | item~isA(.OrderedCollection) then return .false
      if item~isA(.string), item~pos("0A"x) > 0 then return .false
    end
    return .true
  end
  return .false

/** Emits a sequence in flow style: [item1, item2, ...].
 *  Returns the flow string if it fits within 80 characters,
 *  or .nil if it would be too long.
 *
 *  @param obj the .OrderedCollection to emit
 *  @return the flow sequence string, or .nil if too long
 */
::method emitFlowSeq private
  use strict arg obj
  mb = .mutableBuffer~new
  mb~append("[")
  first = .true
  do item over obj
    if \first then mb~append(", ")
    if item == .nil then mb~append("null")
    else                 mb~append(self~emitScalar(item))
    first = .false
  end
  mb~append("]")
  result = mb~string
  if result~length > 80 then return .nil
  return result

/** Emits a mapping in flow style: {key1: val1, key2: val2, ...}.
 *  Returns the flow string if it fits within 80 characters,
 *  or .nil if it would be too long.
 *
 *  @param obj the .MapCollection to emit
 *  @return the flow mapping string, or .nil if too long
 */
::method emitFlowMap private
  use strict arg obj
  mb = .mutableBuffer~new
  mb~append("{")
  first = .true
  sup = obj~supplier
  do while sup~available
    if \first then mb~append(", ")
    mb~append(self~quoteKey(sup~index), ": ")
    if sup~item == .nil then mb~append("null")
    else                     mb~append(self~emitScalar(sup~item))
    first = .false
    sup~next
  end
  mb~append("}")
  result = mb~string
  if result~length > 80 then return .nil
  return result

/** Escapes a string for double-quoted YAML representation.
 *  Replaces special characters (quotes, backslashes, control
 *  characters) with their YAML backslash escape sequences.
 *
 *  @param text the string to escape
 *  @return the escaped string (without surrounding quotes)
 */
::method escapeStr private
  use strict arg text
  mb = .mutableBuffer~new
  do i = 1 to text~length
    ch = text[i]
    select
      when ch == '"'  then mb~append('\"')
      when ch == "\"  then mb~append("\\")
      when ch == "00"x then mb~append("\0")
      when ch == "07"x then mb~append("\a")
      when ch == "08"x then mb~append("\b")
      when ch == "09"x then mb~append("\t")
      when ch == "0A"x then mb~append("\n")
      when ch == "0B"x then mb~append("\v")
      when ch == "0C"x then mb~append("\f")
      when ch == "0D"x then mb~append("\r")
      when ch == "1B"x then mb~append("\e")
      otherwise mb~append(ch)
    end
  end
  return mb~string

/*============================================================================*/
/*  YamlXmlEmitter                                                            */
/*============================================================================*/

/** Serialises an in-memory ooRexx object tree to XML conforming to either
 *  <code>yaml.xsd</code> (with namespace) or <code>yaml.dtd</code>
 *  (with DOCTYPE).
 *
 *  <p>Used internally by <code>Yaml~yamlToXml</code> and
 *  <code>Yaml~yamlToXmlFile</code>.
 *
 *  @see Yaml#yamlToXml
 *  @see YamlXmlParser
 */
::class YamlXmlEmitter public

/** Creates a new YamlXmlEmitter instance.
 *
 *  @param schema          optional schema type: <code>"xsd"</code> (default) or
 *                         <code>"dtd"</code>
 *  @param anchorMap       optional .IdentityTable mapping objects to anchor names
 *  @param directivesMap   optional .IdentityTable mapping root objects to
 *                         directives Tables
 *  @param mergeSourceMap  optional .IdentityTable mapping target mappings to
 *                         arrays of merge sources
 */
::method init
  expose schema anchorMap emitted directivesMap mergeSourceMap
  use strict arg schema = "xsd", anchorMap = .nil, directivesMap = .nil, mergeSourceMap = .nil
  schema = schema~lower
  if schema \== "xsd" & schema \== "dtd" then
    raise syntax 93.900 additional("schema must be 'xsd' or 'dtd'")
  if anchorMap == .nil then anchorMap = .identityTable~new
  if directivesMap == .nil then directivesMap = .identityTable~new
  if mergeSourceMap == .nil then mergeSourceMap = .identityTable~new
  emitted = .identityTable~new

/** Emits the given ooRexx object tree as an XML string.
 *
 *  @param obj the root object to serialise
 *  @return a well-formed XML string
 */
::method emit
  expose schema directivesMap
  use strict arg obj
  mb = .mutableBuffer~new
  mb~append('<?xml version="1.0" encoding="UTF-8"?>', "0A"x)
  if schema == "xsd" then
    mb~append('<yaml xmlns="urn:yaml:xml:1.0" version="1.2">', "0A"x)
  else do
    mb~append('<!DOCTYPE yaml SYSTEM "yaml.dtd">', "0A"x)
    mb~append('<yaml version="1.2">', "0A"x)
  end
  /* Emit document element with optional directives */
  directives = directivesMap~at(obj)
  if directives \== .nil then do
    yamlVersion = directives~at("yamlVersion")
    if yamlVersion \== .nil then
      mb~append('  <document yaml-version="', self~xmlEscape(yamlVersion), '">', "0A"x)
    else
      mb~append("  <document>", "0A"x)
    tagHandles = directives~at("tagHandles")
    if tagHandles \== .nil then do
      sup = tagHandles~supplier
      do while sup~available
        mb~append('    <tag-directive handle="', self~xmlEscape(sup~index), -
                  '" prefix="', self~xmlEscape(sup~item), '"/>', "0A"x)
        sup~next
      end
    end
  end
  else
    mb~append("  <document>", "0A"x)
  self~emitNode(mb, obj, 4)
  mb~append("  </document>", "0A"x)
  mb~append("</yaml>", "0A"x)
  return mb~string

/** Recursively serialises an ooRexx object to XML elements.
 *  Emits <mapping>, <sequence>, and <scalar> elements with
 *  appropriate attributes (anchor, tag, type).  Handles merge
 *  key reconstruction via <merge> elements and alias references.
 *
 *  @param mb the .MutableBuffer to append XML to
 *  @param obj the object to emit
 *  @param level the current indentation level (in spaces)
 */
::method emitNode private
  expose schema anchorMap emitted mergeSourceMap
  use strict arg mb, obj, level, tagAttr = ""

  pad = copies(" ", level)

  /* YamlTagged wrapper — unwrap tag and pass it down */
  if obj~isA(.YamlTagged) then do
    tAttr = ' tag="' || self~xmlEscape(obj~tag) || '"'
    self~emitNode(mb, obj~value, level, tAttr)
    return
  end

  /* Null */
  if obj == .nil then do
    mb~append(pad, '<scalar type="null"', tagAttr, '/>', "0A"x)
    return
  end

  /* Check for alias (already emitted anchored object) */
  anchorName = anchorMap~at(obj)
  if anchorName \== .nil then do
    if emitted~at(obj) \== .nil then do
      mb~append(pad, '<alias anchor="', self~xmlEscape(anchorName), '"/>', "0A"x)
      return
    end
    emitted[obj] = anchorName
  end

  /* Build anchor attribute fragment */
  anchorAttr = ""
  if anchorName \== .nil then
    anchorAttr = ' anchor="' || self~xmlEscape(anchorName) || '"'
  /* Append tag attribute (from YamlTagged wrapper, if any) */
  anchorAttr = anchorAttr || tagAttr

  /* Mapping */
  if obj~isA(.MapCollection) then do
    mergeSources = mergeSourceMap~at(obj)
    /* Determine which keys came from merges (if any) */
    mergedKeys = .table~new
    if mergeSources \== .nil then do
      do src over mergeSources
        srcSup = src~supplier
        do while srcSup~available
          k = srcSup~index
          if k~isA(.string) then do
            /* A key is merged only if the target has the same value */
            targetVal = obj~at(k)
            if targetVal \== .nil then do
              if srcSup~item = targetVal then
                mergedKeys[k] = .true
            end
          end
          srcSup~next
        end
      end
    end
    /* Count own (non-merged) entries */
    ownCount = obj~items - mergedKeys~items
    if ownCount = 0, mergeSources == .nil then do
      mb~append(pad, "<mapping" || anchorAttr || "/>", "0A"x)
      return
    end
    mb~append(pad, "<mapping" || anchorAttr || ">", "0A"x)
    /* Emit <merge> elements */
    if mergeSources \== .nil then do
      do src over mergeSources
        srcAnchor = anchorMap~at(src)
        if srcAnchor \== .nil then
          mb~append(pad, '  <merge anchor="', self~xmlEscape(srcAnchor), '"/>', "0A"x)
      end
    end
    /* Emit own (non-merged) entries */
    sup = obj~supplier
    do while sup~available
      if mergedKeys~hasIndex(sup~index) then do
        sup~next
        iterate
      end
      mb~append(pad, "  <entry>", "0A"x)
      mb~append(pad, "    <key>", "0A"x)
      self~emitNode(mb, sup~index, level + 6)
      mb~append(pad, "    </key>", "0A"x)
      mb~append(pad, "    <value>", "0A"x)
      self~emitNode(mb, sup~item, level + 6)
      mb~append(pad, "    </value>", "0A"x)
      mb~append(pad, "  </entry>", "0A"x)
      sup~next
    end
    mb~append(pad, "</mapping>", "0A"x)
    return
  end

  /* Sequence */
  if obj~isA(.OrderedCollection) then do
    if obj~items = 0 then do
      mb~append(pad, "<sequence" || anchorAttr || "/>", "0A"x)
      return
    end
    mb~append(pad, "<sequence" || anchorAttr || ">", "0A"x)
    do item over obj
      mb~append(pad, "  <item>", "0A"x)
      self~emitNode(mb, item, level + 4)
      mb~append(pad, "  </item>", "0A"x)
    end
    mb~append(pad, "</sequence>", "0A"x)
    return
  end

  /* Scalar */
  self~emitScalarNode(mb, obj, level, anchorAttr)

/** Emits a scalar value as an XML <scalar> element with a type attribute.
 *  Null values produce self-closing elements; other values are
 *  XML-escaped and wrapped in <scalar type="...">...</scalar>.
 *
 *  @param mb the .MutableBuffer to append to
 *  @param value the scalar value to emit
 *  @param level the indentation level
 *  @param anchorAttr optional anchor attribute string
 */
::method emitScalarNode private
  use strict arg mb, value, level, anchorAttr = ""
  pad = copies(" ", level)
  sType = self~scalarType(value)
  if sType == "null" then do
    mb~append(pad, '<scalar type="null"', anchorAttr, '/>', "0A"x)
    return
  end
  if sType == "bool" then do
    if value then escaped = "true"
    else          escaped = "false"
  end
  else
    escaped = self~xmlEscape(value~string)
  mb~append(pad, '<scalar type="', sType, '"', anchorAttr, '>', escaped, "</scalar>", "0A"x)

/** Determines the XML scalar type for a value.
 *  Returns "null", "bool", "int", "float", or "str" based on the
 *  value's ooRexx type (nil, YamlBoolean, integer, number, or string).
 *
 *  @param value the value to classify
 *  @return the type name string
 */
::method scalarType private
  use strict arg value
  if value == .nil then return "null"
  if value~isA(.LogicalWrapper) then return "bool"
  if \value~isA(.string) then return "str"
  if value == "" then return "str"
  /* Check numeric types */
  if value~datatype("W") then return "int"
  if value~datatype("N") then return "float"
  if value == ".inf" | value == "+.inf" | value == "-.inf" then return "float"
  if value == ".nan" then return "float"
  return "str"

/** Escapes special XML characters in a text string.
 *  Replaces &, <, >, ", and ' with their XML entity equivalents.
 *
 *  @param text the text to escape
 *  @return the XML-escaped string
 */
::method xmlEscape private
  use strict arg text
  mb = .mutableBuffer~new
  do i = 1 to text~length
    ch = text[i]
    select
      when ch == "&" then mb~append("&amp;")
      when ch == "<" then mb~append("&lt;")
      when ch == ">" then mb~append("&gt;")
      when ch == '"' then mb~append("&quot;")
      when ch == "'" then mb~append("&apos;")
      otherwise mb~append(ch)
    end
  end
  return mb~string

/*============================================================================*/
/*  YamlXmlParser                                                             */
/*============================================================================*/

/** Parses XML conforming to <code>yaml.xsd</code> or <code>yaml.dtd</code>
 *  back into ooRexx objects.  This is a purpose-built parser for the YAML XML
 *  vocabulary &mdash; it is not a general-purpose XML parser.
 *
 *  <p>Used internally by <code>Yaml~parseXml</code> and
 *  <code>Yaml~parseXmlFile</code>.
 *
 *  @see Yaml#parseXml
 *  @see YamlXmlEmitter
 */
::class YamlXmlParser public

/** Returns the directives map from the most recent parse.
 *  Same structure as <code>Yaml~directivesMap</code>.
 *
 *  @return an .IdentityTable mapping root objects to directives Tables
 */
::attribute directivesMap get

/** Returns the merge-source map from the most recent parse.
 *  Same structure as <code>Yaml~mergeSourceMap</code>.
 *
 *  @return an .IdentityTable mapping target mappings to arrays of merge sources
 */
::attribute mergeSourceMap get

/** Returns the anchor map from the most recent parse.
 *  Maps anchor names to their resolved ooRexx objects.
 *
 *  @return a .Table mapping anchor name strings to objects
 */
::attribute xmlAnchors get

/** Parses an XML string into an ooRexx object tree.
 *
 *  @param input a well-formed XML string conforming to the YAML XML vocabulary
 *  @return the ooRexx object tree
 */
::method parse
  expose text pos len xmlAnchors directivesMap mergeSourceMap pendingMerges
  use strict arg input
  text = input
  len  = text~length
  pos  = 1
  xmlAnchors = .table~new
  directivesMap = .identityTable~new
  mergeSourceMap = .identityTable~new
  pendingMerges = .array~new
  /* Skip XML declaration and DOCTYPE if present */
  self~skipProlog
  /* Expect <yaml ...> */
  self~skipWS
  tag = self~readTag
  if tag["name"] \== "yaml" then
    raise syntax 93.900 additional("Expected <yaml> root element, got <" || tag["name"] || ">")
  /* Read <document> element */
  self~skipWS
  tag = self~readTag
  if tag["name"] \== "document" then
    raise syntax 93.900 additional("Expected <document> element")
  /* Collect directives from document attributes and child elements */
  directives = .nil
  docAttrs = tag["attrs"]
  yamlVersion = docAttrs~at("yaml-version")
  if yamlVersion \== .nil then do
    if directives == .nil then directives = .table~new
    directives["yamlVersion"] = yamlVersion
  end
  /* Read optional <tag-directive> elements before the content node */
  self~skipWS
  do while self~peekTagName == "tag-directive"
    tdTag = self~readTag
    tdAttrs = tdTag["attrs"]
    handle = tdAttrs~at("handle")
    prefix = tdAttrs~at("prefix")
    if handle \== .nil, prefix \== .nil then do
      if directives == .nil then directives = .table~new
      tagHandles = directives~at("tagHandles")
      if tagHandles == .nil then do
        tagHandles = .table~new
        directives["tagHandles"] = tagHandles
      end
      tagHandles[handle] = prefix
    end
    self~skipWS
  end
  node = self~readNode
  /* Resolve any forward-referenced merges now that all anchors are known */
  self~resolvePendingMerges
  if directives \== .nil then directivesMap[node] = directives
  self~skipWS
  self~expectCloseTag("document")
  self~skipWS
  self~expectCloseTag("yaml")
  return node

/** Peeks at the next tag name without consuming input.
 *  Returns the tag name string, or "" if the next token is not
 *  an opening tag.
 */
::method peekTagName private
  expose text pos len
  if pos > len then return ""
  if text[pos] \== "<" then return ""
  if pos + 1 <= len, text[pos + 1] == "/" then return ""
  /* Scan tag name */
  i = pos + 1
  do while i <= len
    ch = text[i]
    if ch == " " | ch == "09"x | ch == ">" | ch == "/" then leave
    i = i + 1
  end
  return text~substr(pos + 1, i - pos - 1)

/** Resolves forward-referenced merge anchors after the full document has
 *  been parsed and all anchors are available in xmlAnchors.  For each
 *  pending merge, looks up the anchor name, adds the source object to
 *  the mergeSourceMap, and copies the merged keys into the target mapping.
 */
::method resolvePendingMerges private
  expose xmlAnchors mergeSourceMap pendingMerges
  do entry over pendingMerges
    map          = entry[1]
    pendingNames = entry[2]
    merges       = entry[3]
    do anchorName over pendingNames
      srcObj = xmlAnchors~at(anchorName)
      if srcObj \== .nil, srcObj~isA(.table) then do
        merges~append(srcObj)
        /* Apply merged keys (low priority — don't overwrite existing) */
        srcSup = srcObj~supplier
        do while srcSup~available
          if \map~hasIndex(srcSup~index) then
            map[srcSup~index] = srcSup~item
          srcSup~next
        end
      end
    end
    if merges~items > 0 then
      mergeSourceMap[map] = merges
  end

/** Skips the XML prolog (declaration, DOCTYPE, comments) at the
 *  beginning of an XML document.  Advances the position past any
 *  processing instructions (<?...?>), comments (<!--...-->), and
 *  DOCTYPE declarations (<!DOCTYPE...>).
 */
::method skipProlog private
  expose text pos len
  /* Skip <?xml ...?>, <!DOCTYPE ...>, <!-- comments --> */
  do forever
    self~skipWS
    if pos > len then return
    if text[pos] \== "<" then return
    if pos + 1 > len then return
    nch = text[pos + 1]
    if nch == "?" then do
      /* Processing instruction — skip to ?> */
      ep = text~pos("?>", pos)
      if ep == 0 then
        raise syntax 93.900 additional("Unterminated processing instruction")
      pos = ep + 2
    end
    else if nch == "!" then do
      if pos + 3 <= len, text~substr(pos + 2, 2) == "--" then do
        /* Comment: skip to --> */
        ep = text~pos("-->", pos)
        if ep == 0 then
          raise syntax 93.900 additional("Unterminated XML comment")
        pos = ep + 3
      end
      else do
        /* DOCTYPE: skip to > */
        ep = text~pos(">", pos)
        if ep == 0 then
          raise syntax 93.900 additional("Unterminated DOCTYPE declaration")
        pos = ep + 1
      end
    end
    else return
  end

/** Advances the position past XML whitespace characters
 *  (space, tab, LF, CR).
 */
::method skipWS private
  expose text pos len
  do while pos <= len
    ch = text[pos]
    if ch \== " ", ch \== "09"x, ch \== "0A"x, ch \== "0D"x then return
    pos = pos + 1
  end

/** Reads an XML opening tag with its attributes.
 *  Parses the tag name and any name="value" attribute pairs.
 *  Detects self-closing tags (/>).
 *
 *  @return a .Table with keys: "name" (string), "attrs" (.Table),
 *          "selfClose" (.true or .false)
 */
::method readTag private
  expose text pos len
  /* Reads <name attr="val" ...> or <name/>. Returns a Table. */
  if text[pos] \== "<" then
    raise syntax 93.900 additional("Expected '<' at position" pos)
  pos = pos + 1
  self~skipWS
  /* Read tag name */
  startName = pos
  do while pos <= len
    ch = text[pos]
    if ch == " " | ch == "09"x | ch == ">" | ch == "/" then leave
    pos = pos + 1
  end
  tagName = text~substr(startName, pos - startName)
  /* Read attributes */
  attrs = .table~new
  self~skipWS
  do while pos <= len
    ch = text[pos]
    if ch == ">" then do; pos = pos + 1; leave; end
    if ch == "/" then do
      pos = pos + 1
      if pos <= len, text[pos] == ">" then pos = pos + 1
      tag = .table~new
      tag["name"] = tagName
      tag["attrs"] = attrs
      tag["selfClose"] = .true
      return tag
    end
    /* Read attribute name */
    aStart = pos
    do while pos <= len
      if text[pos] == "=" then leave
      pos = pos + 1
    end
    aName = text~substr(aStart, pos - aStart)~strip
    pos = pos + 1 /* skip = */
    self~skipWS
    /* Read attribute value */
    if pos <= len then do
      quote = text[pos]
      pos = pos + 1
      vStart = pos
      do while pos <= len
        if text[pos] == quote then leave
        pos = pos + 1
      end
      aVal = text~substr(vStart, pos - vStart)
      pos = pos + 1 /* skip closing quote */
      attrs[aName] = aVal
    end
    self~skipWS
  end
  tag = .table~new
  tag["name"] = tagName
  tag["attrs"] = attrs
  tag["selfClose"] = .false
  return tag

/** Expects and consumes a closing XML tag.
 *  Raises a parse error if the closing tag does not match the
 *  expected element name.
 *
 *  @param expected the expected element name
 */
::method expectCloseTag private
  expose text pos len
  use strict arg expected
  self~skipWS
  if pos > len | text[pos] \== "<" then
    raise syntax 93.900 additional("Expected </" || expected || ">")
  if pos + 1 > len | text[pos + 1] \== "/" then
    raise syntax 93.900 additional("Expected </" || expected || ">, got opening tag")
  pos = pos + 2
  startName = pos
  do while pos <= len
    ch = text[pos]
    if ch == ">" | ch == " " then leave
    pos = pos + 1
  end
  tagName = text~substr(startName, pos - startName)
  if tagName \== expected then
    raise syntax 93.900 additional("Expected </" || expected || ">, got </" || tagName || ">")
  do while pos <= len
    if text[pos] == ">" then do; pos = pos + 1; return; end
    pos = pos + 1
  end

/** Reads the next XML node element and converts it to an ooRexx object.
 *  Dispatches based on the element name: <mapping>, <sequence>,
 *  <scalar>, or <alias>.  Handles tag and anchor attributes.
 *  Alias elements resolve to previously anchored objects.
 *
 *  @return the parsed ooRexx object
 */
::method readNode private
  expose text pos len xmlAnchors
  self~skipWS
  if pos > len then return .nil
  tag = self~readTag
  select case tag["name"]
    when "mapping"  then do
      node = self~readMapping(tag)
      yamlTag = tag["attrs"]~at("tag")
      if yamlTag \== .nil then node = .YamlTagged~new(self~xmlUnescape(yamlTag), node)
      anchorName = tag["attrs"]~at("anchor")
      if anchorName \== .nil then xmlAnchors[anchorName] = node
      return node
    end
    when "sequence" then do
      node = self~readSequence(tag)
      yamlTag = tag["attrs"]~at("tag")
      if yamlTag \== .nil then node = .YamlTagged~new(self~xmlUnescape(yamlTag), node)
      anchorName = tag["attrs"]~at("anchor")
      if anchorName \== .nil then xmlAnchors[anchorName] = node
      return node
    end
    when "scalar"   then do
      node = self~readScalar(tag)
      yamlTag = tag["attrs"]~at("tag")
      if yamlTag \== .nil then node = .YamlTagged~new(self~xmlUnescape(yamlTag), node)
      anchorName = tag["attrs"]~at("anchor")
      if anchorName \== .nil, node \== .nil then xmlAnchors[anchorName] = node
      return node
    end
    when "alias"    then do
      anchorName = tag["attrs"]~at("anchor")
      if anchorName == .nil then
        raise syntax 93.900 additional("alias element missing anchor attribute")
      if \xmlAnchors~hasIndex(anchorName) then
        raise syntax 93.900 additional("alias references unknown anchor:" anchorName)
      return xmlAnchors[anchorName]
    end
    otherwise
      raise syntax 93.900 additional("Unexpected element: <" || tag["name"] || ">")
  end

/** Reads a <mapping> element and its <entry> and <merge> children.
 *  Processes merge elements by looking up anchored source mappings
 *  and applying their keys at low priority.  Forward-referenced
 *  merge anchors are deferred to resolvePendingMerges.
 *
 *  @param tag the already-parsed opening <mapping> tag
 *  @return a .Table representing the mapping
 */
::method readMapping private
  expose text pos len xmlAnchors mergeSourceMap pendingMerges
  use strict arg tag
  map = .table~new
  if tag["selfClose"] then return map
  merges = .array~new
  pendingNames = .array~new
  do forever
    self~skipWS
    if pos > len then leave
    if text[pos] == "<", pos + 1 <= len, text[pos + 1] == "/" then leave
    /* Peek to see if this is <merge> or <entry> */
    nextName = self~peekTagName
    if nextName == "merge" then do
      mergeTag = self~readTag
      mergeAnchor = mergeTag["attrs"]~at("anchor")
      if mergeAnchor \== .nil then do
        srcObj = xmlAnchors~at(mergeAnchor)
        if srcObj \== .nil, srcObj~isA(.table) then do
          merges~append(srcObj)
          /* Apply merged keys (low priority — don't overwrite existing) */
          srcSup = srcObj~supplier
          do while srcSup~available
            if \map~hasIndex(srcSup~index) then
              map[srcSup~index] = srcSup~item
            srcSup~next
          end
        end
        else do
          /* Forward reference — anchor not yet parsed; defer resolution */
          pendingNames~append(mergeAnchor)
        end
      end
      iterate
    end
    /* Expect <entry> */
    entryTag = self~readTag
    if entryTag["name"] \== "entry" then
      raise syntax 93.900 additional("Expected <entry> or <merge>, got <" || entryTag["name"] || ">")
    /* <key> */
    self~skipWS
    keyTag = self~readTag
    if keyTag["name"] \== "key" then
      raise syntax 93.900 additional("Expected <key>")
    self~skipWS
    key = self~readNode
    self~skipWS
    self~expectCloseTag("key")
    /* <value> */
    self~skipWS
    valTag = self~readTag
    if valTag["name"] \== "value" then
      raise syntax 93.900 additional("Expected <value>")
    self~skipWS
    val = self~readNode
    self~skipWS
    self~expectCloseTag("value")
    /* </entry> */
    self~skipWS
    self~expectCloseTag("entry")
    /* Store entry — .Table supports any object as key */
    if key == .nil then
      map[""] = val
    else
      map[key] = val
  end
  self~expectCloseTag("mapping")
  /* Record merge sources for this mapping */
  if merges~items > 0 then
    mergeSourceMap[map] = merges
  /* Record pending forward-reference merges for deferred resolution */
  if pendingNames~items > 0 then
    pendingMerges~append(.array~of(map, pendingNames, merges))
  return map

/** Reads a <sequence> element and its <item> children.
 *
 *  @param tag the already-parsed opening <sequence> tag
 *  @return an .Array of items
 */
::method readSequence private
  expose text pos len
  use strict arg tag
  arr = .array~new
  if tag["selfClose"] then return arr
  do forever
    self~skipWS
    if pos > len then leave
    if text[pos] == "<", pos + 1 <= len, text[pos + 1] == "/" then leave
    /* Expect <item> */
    itemTag = self~readTag
    if itemTag["name"] \== "item" then
      raise syntax 93.900 additional("Expected <item>, got <" || itemTag["name"] || ">")
    self~skipWS
    val = self~readNode
    self~skipWS
    self~expectCloseTag("item")
    arr~append(val)
  end
  self~expectCloseTag("sequence")
  return arr

/** Reads a <scalar> element and resolves its typed value.
 *  Uses the type attribute (null, bool, int, float, str) to
 *  convert the text content to the appropriate ooRexx object.
 *
 *  @param tag the already-parsed opening <scalar> tag
 *  @return the resolved scalar value
 */
::method readScalar private
  expose text pos len
  use strict arg tag
  sType = tag["attrs"]~at("type")
  if sType == .nil then sType = "str"
  /* Self-closing scalar: null or empty */
  if tag["selfClose"] then do
    if sType == "null" then return .nil
    return ""
  end
  /* Read text content until </scalar> */
  startContent = pos
  do while pos <= len
    if text[pos] == "<" then leave
    pos = pos + 1
  end
  content = self~xmlUnescape(text~substr(startContent, pos - startContent))
  self~expectCloseTag("scalar")
  /* Resolve type */
  select case sType
    when "null"  then return .nil
    when "bool"  then do
      if content~lower == "true" then return .YamlBoolean~true
      return .YamlBoolean~false
    end
    when "int"   then return content + 0
    when "float" then do
      lc = content~lower
      if lc == ".inf" | lc == "+.inf" then return ".inf"
      if lc == "-.inf" then return "-.inf"
      if lc == ".nan" then return ".nan"
      return content + 0
    end
    otherwise return content
  end

/** Unescapes XML entities in a text string.
 *  Replaces &lt;, &gt;, &quot;, &apos;, and &amp; with their
 *  literal character equivalents.  The &amp; replacement is done
 *  last to avoid double-unescaping.
 *
 *  @param text the XML-escaped text
 *  @return the unescaped string
 */
::method xmlUnescape private
  use strict arg text
  text = text~changeStr("&lt;", "<")
  text = text~changeStr("&gt;", ">")
  text = text~changeStr("&quot;", '"')
  text = text~changeStr("&apos;", "'")
  text = text~changeStr("&amp;", "&")  /* must be last */
  return text
