/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2010-2026 Rexx Language Association. All rights reserved.    */
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


/**
    An ooRexx utility class to encode and decode ooRexx objects in JSON
    (RFC 8259, https://www.rfc-editor.org/rfc/rfc8259).

    <p>Any JSON value (object, array, string, number, boolean, null) is
    accepted at the top level, as permitted by RFC 8259.</p>

    <p><b>Encoding (ooRexx &rarr; JSON):</b></p>
    <ul>
      <li>MapCollection &rarr; JSON object (keys sorted alphabetically)</li>
      <li>OrderedCollection &rarr; JSON array</li>
      <li>.nil &rarr; <code>null</code></li>
      <li>.JsonBoolean &rarr; <code>true</code> / <code>false</code></li>
      <li>.JsonString &rarr; always quoted, even if numeric</li>
      <li>Plain string &rarr; number if numeric, quoted string otherwise</li>
      <li>Objects with makeJSON, makeArray, or makeString are handled</li>
    </ul>

    <p><b>Decoding (JSON &rarr; ooRexx):</b></p>
    <ul>
      <li>JSON object &rarr; .Directory</li>
      <li>JSON array &rarr; .Array</li>
      <li>JSON string &rarr; .JsonString</li>
      <li>JSON number &rarr; plain String</li>
      <li>JSON boolean &rarr; .JsonBoolean</li>
      <li>JSON null &rarr; .nil</li>
    </ul>

    <p><b>\uXXXX handling:</b> Since ooRexx has no native Unicode support,
    <code>\u00XX</code> sequences are decoded to the corresponding single-byte
    character.  Non-<code>\u00XX</code> sequences (e.g. <code>\u4E16</code>)
    are kept as literal <code>\uXXXX</code> text.  During encoding, any
    literal <code>\uXXXX</code> pattern already present in a source string
    is passed through unchanged.  This means a Rexx string containing the
    six characters <code>\u0041</code> will encode as <code>\u0041</code>
    (decoded as <code>A</code> by a JSON consumer), rather than escaping
    the backslash.  This is a deliberate trade-off for ooRexx environments
    where \uXXXX sequences are used to represent Unicode characters that
    cannot be stored natively.</p>

    <p><b>Example:</b></p>
    <pre>
    json = .json~new
    doc  = json~fromJSON('{"name": "Alice", "age": 30, "active": true}')
    say doc["name"]           -- Alice
    say doc["age"]            -- 30
    say doc["active"]         -- 1  (a JsonBoolean singleton)
    say .json~toJSON(doc)     -- {"active":true,"age":30,"name":"Alice"}

    ::requires "json.cls"
    </pre>
*/

::requires "logicalwrapper.cls"   -- the LogicalWrapper base class (JsonBoolean and YamlBoolean)

::class "JSON" public

/** Returns the .JsonBoolean proxy for <code>.true</code>, which ensures a proper JSON encoding.
    It can be used interchangeably with ooRexx' <code>.true</code> or <code>1</code> values.

    @return the .JsonBoolean value for <code>.true</code>
*/
::attribute true     get class unguarded
  return .jsonBoolean~true

/** Returns the .JsonBoolean proxy for <code>.false</code>, which ensures a proper JSON encoding.
    It can be used interchangeably with ooRexx' <code>.false</code> or <code>0</code> values.

    @return the .JsonBoolean value for <code>.false</code>
*/
::attribute false    get class unguarded
  return .jsonBoolean~false



/**
 * Constructor, initializes the instance.
 */
::method init
    expose eJS uJS ctrl crlf
    use strict arg      -- no arguments allowed

    eJS = .directory~new() -- escape table: character -> JSON escape sequence
    eJS['08'x] = '\b'
    eJS['09'x] = '\t'
    eJS['0A'x] = '\n'
    eJS['0C'x] = '\f'
    eJS['0D'x] = '\r'
    eJS['"'] = '\"'
    eJS['\'] = '\\'
    eJS['/'] = '\/'

    uJS = .directory~new() -- unescape table: JSON escape sequence -> character
    do index over eJS
        uJS[eJS[index]] = index
    end

    -- chars that end a value
    ctrl = .Set~of(' ', '}', ']', ',', '09'x, '0a'x, '0d'x)

    crlf = .rexxInfo~endOfLine   -- get the platform's endOfLine character(s)

    -- Store in package-local variables so the string2json routine can access them
    pkgLocal=.context~package~local
    pkgLocal~js.eJS  = eJS
    pkgLocal~js.uJS  = uJS
    pkgLocal~js.ctrl = ctrl
    pkgLocal~js.crlf = crlf


/* ========================================================================== */
/*  Public API methods                                                        */
/* ========================================================================== */

/** Utility class method to ease reading JSON files into an ooRexx object.
*
<p>Example:
<pre>
   rexxObject = .json~fromJsonFile('some.json')
</pre>
*
*  @param fn file name or file object to read JSON data from
*
* @return rexxObject ooRexx object representing the JSON text
*/
::method fromJsonFile class
  use strict arg fn

  s=.stream~new(fn)~~open("read")   -- open stream for reading
  jsonText=s~charin(1,s~chars)      -- read all chars, close stream
  s~close
  json=self~new                     -- create json instance
  return json~fromJson(jsonText)    -- use it to create ooRexx representation


/** Utility class method to ease creating minimized JSON files from an ooRexx object.
*   If a legible (with ignorable whitespace to ease reading for humans) JSON file
*   is desired instead, supply <code>.true</code> as the third argument.
*
<p>Example:
<pre>
   .json~toJsonFile('some.json',someRexxObject)
</pre>
*
*  @param fn file name or file object to write produced JSON text to
*  @param rexxObject rexxObject to encode as JSON
*  @param isLegible optional logical value, defaults to <code>.false</code> (create minimized JSON string)
*
*/
::method toJsonFile class
  use strict arg fn, rexxObject, legible=.false
  if arg()=3 then       -- check supplied argument
     .Validate~logical("legible",legible) -- test for logical value 0 or 1

  json=self~new                  -- create json instance
  jsonText=json~toJSON(rexxObject, legible)  -- create JSON text representing the rexxObject

   -- open stream for writing, delete existing file, if any, write json text to it
  .stream~new(fn)~~open("write replace")~~charout(jsonText)~~close

::method toJSON class unguarded     -- make available via class object
  j=self~new                        -- create JSON instance
  forward to (j)                    -- forward message to JSON instance

::method fromJSON class unguarded   -- make available via class object
  j=self~new                        -- create JSON instance
  forward to (j)                    -- forward message to JSON instance

/** Serialises an ooRexx object tree to XML conforming to either
 *  <code>json.xsd</code> (with namespace) or <code>json.dtd</code>
 *  (with DOCTYPE).
 *
 *  @param obj     the ooRexx object tree to serialise
 *  @param schema  optional schema type: <code>"xsd"</code> (default) or
 *                 <code>"dtd"</code>
 *  @return a well-formed XML string
 *  @see JsonXmlEmitter
 *  @see JsonXmlParser
 */
::method jsonToXml class
  use strict arg obj, schema = "xsd"
  return .JsonXmlEmitter~new(schema)~emit(obj)

/** Serialises an ooRexx object tree to XML and writes it to a file.
 *
 *  @param obj     the ooRexx object tree to serialise
 *  @param path    the file system path to write
 *  @param schema  optional schema type: <code>"xsd"</code> (default) or
 *                 <code>"dtd"</code>
 */
::method jsonToXmlFile class
  use strict arg obj, path, schema = "xsd"
  xml = self~jsonToXml(obj, schema)
  s = .stream~new(path)~~open("WRITE REPLACE")
  if s~state \== "READY" then
    raise syntax 93.900 additional("Cannot open file for writing:" path)
  s~charOut(xml)
  s~close


/* ========================================================================== */
/*  Encoding: ooRexx objects -> JSON text                                     */
/* ========================================================================== */

/**
 * Converts a Rexx object to JSON formatting.
 *
 * @param  rexxObject   The object to convert.  Accepts MapCollection, OrderedCollections,
 *                      or String objects, JsonBoolean objects or nil.  Otherwise, it calls
 *                      the makeJSON, makeArray, or makeString method for the object.
 * @param  legible      Optional logical value (default .false).  If .true, the output
 *                      includes ignorable whitespace for human readability.
 */
::method toJSON
    expose buffer legible leadin crlf
    use strict arg rexxObject, legible=(.false) -- default to minimized JSON

    if arg()=2 then       -- check supplied argument
       .Validate~logical("legible",legible) -- test for logical value 0 or 1

    buffer = .mutablebuffer~new()
    leadin = "   "        -- 3-space indentation for legible output
    self~emitValue(rexxObject, 0)

    return buffer~string


/** Emits a JSON value for any ooRexx object.  Dispatches based on object type.
 *  Handles both minimized and legible output via the 'legible' instance variable.
 *
 *  @param rexxObject  the object to encode
 *  @param level       current indentation level (used only in legible mode)
 */
::method emitValue private
    expose buffer legible
    use strict arg rexxObject, level

    select
        when rexxObject~isA(.string) then do
           call string2json buffer, rexxObject
        end
        when rexxObject~isA(.OrderedCollection) then do
            self~emitArray(rexxObject, level)
        end
        when rexxObject~isA(.MapCollection) then do
            self~emitObject(rexxObject, level)
        end
        when rexxObject~isNil then do
            buffer~append('null')
        end
        when rexxObject~isA(.LogicalWrapper) then do
            -- any logical value that knows it is one (.JsonBoolean, a bridge's, ...)
            if rexxObject~value then buffer~append('true')
            else buffer~append('false')
        end
        when rexxObject~hasMethod('makejson') then do
            -- object can render itself as a JSON string; handles .JsonBoolean as well
           buffer~append(rexxObject~makeJson)
        end
        when rexxObject~hasMethod('makearray') then do
            self~emitValue(rexxObject~makearray, level)
        end
        when rexxObject~hasMethod('makestring') then do
            -- object can render itself as string; use proper JSON escaping
           call string2json buffer, rexxObject~makeString
        end
        otherwise
            -- use the object's string method; may help debugging if needed
            call string2json buffer, rexxObject~string
    end


/** Emits a JSON array from an OrderedCollection.
 *
 *  @param rexxObject  the OrderedCollection to encode
 *  @param level       current indentation level
 */
::method emitArray private
    expose buffer legible leadin crlf
    use strict arg rexxObject, level

    buffer~append('[')
    items = rexxObject~items
    if items == 0 then buffer~append(']')
    else do
        level += 1
        if legible then buffer~append(crlf, leadin~copies(level))
        do counter c item over rexxObject
            self~emitValue(item, level)
            if c <> items then do
                buffer~append(',')
                if legible then buffer~append(crlf, leadin~copies(level))
            end
            else do
                if legible then buffer~append(crlf, leadin~copies(level - 1))
                buffer~append(']')
            end
        end
        level -= 1
    end


/** Emits a JSON object from a MapCollection.  Keys are sorted alphabetically
 *  for reproducible output.
 *
 *  @param rexxObject  the MapCollection to encode
 *  @param level       current indentation level
 */
::method emitObject private
    expose buffer legible leadin crlf
    use strict arg rexxObject, level

    buffer~append('{')
    items = rexxObject~items
    if items == 0 then buffer~append('}')
    else do
        level += 1
        if legible then buffer~append(crlf, leadin~copies(level))
        do counter c index over rexxObject~allIndexes~sort
            -- encode the key: must be a quoted string in JSON
            if index~isA(.string) then
                call string2json buffer, index, .true   -- force quoted
            else
                call string2json buffer, index~string, .true  -- convert to string, force quoted
            buffer~append(':')
            if legible then buffer~append(' ')
            self~emitValue(rexxObject[index], level)
            if c <> items then do
                buffer~append(',')
                if legible then buffer~append(crlf, leadin~copies(level))
            end
            else do
                if legible then buffer~append(crlf, leadin~copies(level - 1))
                buffer~append('}')
            end
        end
        level -= 1
    end


/* ========================================================================== */
/*  Public XML parsing (instance methods)                                     */
/* ========================================================================== */

/** Parses an XML string (conforming to <code>json.xsd</code> or
 *  <code>json.dtd</code>) back into an ooRexx object tree.
 *
 *  @param input an XML string or an .Array of lines
 *  @return the ooRexx object tree
 *  @see JsonXmlParser
 *  @see JsonXmlEmitter
 */
::method parseXml
  use strict arg input
  select
    when input~isA(.string) then
      t = input~changeStr("0D0A"x, "0A"x)~changeStr("0D"x, "0A"x)
    when input~isA(.array) then
      t = input~makeString('L', "0A"x)
    otherwise
      raise syntax 93.900 additional("parseXml: input must be a String or Array")
  end
  xmlParser = .JsonXmlParser~new
  return xmlParser~parse(t)

/** Parses an XML file (conforming to <code>json.xsd</code> or
 *  <code>json.dtd</code>) back into an ooRexx object tree.
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


/* ========================================================================== */
/*  Decoding: JSON text -> ooRexx objects                                     */
/* ========================================================================== */

/**
 * Recursively converts a JSON text to Rexx objects.
 *
 * @param  jsonString   A JSON text.
 */
::method fromJSON
    expose jsonString jsonPos jsonStringLength
    use strict arg jsonString

    -- strip UTF-8 BOM if present
    if jsonString~left(3) == 'EFBBBF'x then
        jsonString = jsonString~substr(4)

    jsonPos = 1
    jsonStringLength = jsonString~length
    self~trimLeadingWhitespace()
    rexxObject = self~parseJSONvalue()
    if jsonPos > jsonStringLength then return rexxObject
    self~trimLeadingWhitespace()
    if jsonPos > jsonStringLength then return rexxObject
    self~raiseError('Expected end of input')
return .nil

/**
 * Determines type of value.
 *
 */
::method parseJSONvalue private
    expose jsonString jsonPos

    parse value jsonString with =(jsonPos) char +1
    select
        when char == '{' then do
            jsonPos = jsonPos + 1
            return self~parseJSONobject()
        end
        when char == '[' then do
            jsonPos = jsonPos + 1
            return self~parseJSONarray()
        end
        when char == '"' then do
            jsonPos = jsonPos + 1
            return self~parseJSONstring()
        end
        otherwise return self~parseJSONother()
    end
return

/**
 * Converts a JSON object into a Rexx directory object.
 *
 */
::method parseJSONobject private
    expose jsonString jsonPos

    rexxDirectory = .directory~new()

    self~trimLeadingWhitespace()
    parse value jsonString with =(jsonPos) char +1
    if char == '}' then do
        jsonPos = jsonPos + 1
        return rexxDirectory
    end
    else self~parseJSONobjectValue(rexxDirectory)

    do forever
        self~trimLeadingWhitespace()
        parse value jsonString with =(jsonPos) char +1
        select
            when char == '}' then do
                jsonPos = jsonPos + 1
                return rexxDirectory
            end
            when char == ',' then do
                jsonPos = jsonPos + 1
                self~parseJSONobjectValue(rexxDirectory)
            end
            otherwise self~raiseError('Expected end of an object or new value')
        end
    end
return

/**
 * Converts JSON name:value pairs into a Rexx directory item@index.
 *
 * @param  rexxDirectory   A Rexx directory object.
 */
::method parseJSONobjectValue private
    expose jsonString jsonPos
    use strict arg rexxDirectory

    self~trimLeadingWhitespace()
    parse value jsonString with =(jsonPos) char +1
    if char == '"' then do
        jsonPos = jsonPos + 1
        index = self~parseJSONstring()
    end
    else self~raiseError('Name must be a quoted string')

    self~trimLeadingWhitespace()
    parse value jsonString with =(jsonPos) char +1
    if char == ':' then do
        jsonPos = jsonPos + 1
        self~trimLeadingWhitespace()
        rexxDirectory[index] = self~parseJSONvalue()
    end
    else self~raiseError('Expected colon separating object name and value')
return

/**
 * Converts a JSON array into a Rexx array object.
 *
 */
::method parseJSONarray private
    expose jsonString jsonPos

    rexxArray = .array~new()

    self~trimLeadingWhitespace()
    parse value jsonString with =(jsonPos) char +1
    if char == ']' then do
        jsonPos = jsonPos + 1
        return rexxArray
    end
    else do
        self~trimLeadingWhitespace()
        rexxArray~append(self~parseJSONvalue())
    end

    do forever
        self~trimLeadingWhitespace()
        parse value jsonString with =(jsonPos) char +1
        select
            when char == ']' then do
                jsonPos = jsonPos + 1
                return rexxArray
            end
            when char == ',' then do
                jsonPos = jsonPos + 1
                self~trimLeadingWhitespace()
                rexxArray~append(self~parseJSONvalue())
            end
            otherwise self~raiseError('Expected end of an array or new value')
        end
    end
return

/**
 * Converts a quoted JSON string into a Rexx string object.
 *
 */
::method parseJSONstring private
    expose jsonString uJS jsonPos jsonStringLength

    rexxString = .mutablebuffer~new()
    do forever
        parse value jsonString with =(jsonPos) char +1
        if char == '\' then do
            parse value jsonString with =(jsonPos) char2 +2
            if uJS~hasIndex(char2) then do
                -- two-character escape sequences \" \\ \/ \b \f \n \r \t
                jsonPos = jsonPos + 2
                rexxString~append(uJS[char2])
            end
            else if jsonString~match(jsonPos, "\u00") then do
                -- \u00XX escape sequence is supported
                hex = jsonString[jsonPos + 4, 2]
                if hex~length == 2, hex~dataType("x") then do
                    jsonPos = jsonPos + 6
                    rexxString~append(hex~x2c)
                end
                else
                    self~raiseError("Invalid escape sequence")
            end
            else if jsonString~match(jsonPos, "\u") then do
                -- in general \uXXXX escape sequences are not supported
                -- as ooRexx has no Unicode support
                -- short of failing, we just keep any \uXXXX as-is
                hex = jsonString[jsonPos + 2, 4]
                if hex~length == 4, hex~dataType("x") then do
                    jsonPos = jsonPos + 6
                    rexxString~append("\u", hex)
                end
                else
                    self~raiseError("Invalid escape sequence")
            end
            else do
                self~raiseError("Invalid escape sequence")
            end
        end
        else if char == '"' then do
            jsonPos = jsonPos + 1
            return .jsonString~new(rexxString~string)
        end
        else do
            -- reject raw control characters U+0000..U+001F per RFC 8259 §7
            if char <<= '1F'x then
                self~raiseError("Unescaped control character in string")
            -- append to the string up to the next quote, backslash, or control char
            stop = jsonString~verify('"\'||'000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f'x, "match", jsonPos)
            if stop == 0
                then stop = jsonStringLength + 1
            rexxString~append(jsonString[jsonPos, stop - jsonPos])
            jsonPos = stop
        end
        if jsonPos > jsonStringLength then self~raiseError('Expected end of a quoted string')
    end
return

/**
 * Converts other JSON types (numbers, booleans, null) into Rexx objects.
 *
 */
::method parseJSONother private
    expose jsonString ctrl jsonPos jsonStringLength

    length = jsonStringLength + 1
    do i = jsonPos while i \== length
        parse value jsonString with =(i) char +1
        if ctrl~hasIndex(char) then leave
    end
    parse value jsonString with =(jsonPos) string +(i - jsonPos)
    if self~isJsonNumber(string) then do
        jsonPos = jsonPos + string~length
        return string
    end
    else do
        select
            when string == 'false' then do
                jsonPos = jsonPos + string~length
                return .JsonBoolean~false
            end
            when string == 'true' then do
                jsonPos = jsonPos + string~length
                return .JsonBoolean~true
            end
            when string == 'null' then do
                jsonPos = jsonPos + string~length
                return .nil
            end
            otherwise nop
        end
    end
self~raiseError('Invalid JSON value')
return

/**
 * Skips allowed whitespace between values.
 *
 */
::method trimLeadingWhitespace private
    expose jsonString jsonPos jsonStringLength
    jsonPos = jsonString~verify('20 09 0d 0a'x, , jsonPos)
    if jsonPos == 0
        then jsonPos = jsonStringLength + 1

/** Raises a JSON parse error with line, column, and context information.
 *  Computes the 1-based line number and column from the current character
 *  position in the JSON string.
 *
 *  @param msg the error description
 */
::method raiseError private
    expose jsonString jsonPos jsonStringLength
    use strict arg msg
    -- compute line and column from jsonPos
    lineNum = 1
    lineStart = 1
    do i = 1 to min(jsonPos, jsonStringLength) - 1
       char = jsonString~substr(i, 1)
       if char == '0A'x then do
          lineNum += 1
          lineStart = i + 1
       end
       else if char == '0D'x then do
          lineNum += 1
          -- handle CR+LF as a single newline
          if i < jsonStringLength, jsonString~substr(i + 1, 1) == '0A'x then
             i += 1
          lineStart = i + 1
       end
    end
    col = jsonPos - lineStart + 1

    -- extract context line (from lineStart to end of line or string)
    lineEnd = jsonString~pos('0A'x, lineStart)
    if lineEnd == 0 then lineEnd = jsonStringLength + 1
    -- also check for CR
    crPos = jsonString~pos('0D'x, lineStart)
    if crPos > 0, crPos < lineEnd then lineEnd = crPos
    contextLine = jsonString~substr(lineStart, lineEnd - lineStart)

    raise syntax 93.900 additional(.JsonError~new(msg, lineNum, col, contextLine)~makeString)

/**
 * Validates a string against the JSON number grammar (RFC 8259):
 *
 *   number = [ "-" ] int [ frac ] [ exp ]
 *   int    = "0" / ( digit1-9 *DIGIT )
 *   frac   = "." 1*DIGIT
 *   exp    = ( "e" / "E" ) [ "+" / "-" ] 1*DIGIT
 *
 * Returns .true if the string is a valid JSON number, .false otherwise.
 *
 */
::method isJsonNumber private
    use arg string
    return isJsonNumber(string)


/* ========================================================================= */
/*  JsonError                                                                */
/* ========================================================================= */

/** Represents an error encountered during JSON parsing.  Instances carry the
 *  error message, source location (line and column), and the offending input
 *  line for diagnostic purposes.
 *
 *  @see Json
 */
::class JsonError public

/** The human-readable error description. */
::attribute message
/** The 1-based line number where the error was detected (0 if unknown). */
::attribute lineNumber
/** The 1-based column where the error was detected (0 if unknown). */
::attribute column
/** The source line that triggered the error (empty string if unavailable). */
::attribute contextLine

/** Creates a new JsonError instance.
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
 *  number, column, and context line when available.
 *
 *  @return a formatted error string
 */
::method makeString
  expose message lineNumber column contextLine
  s = "JsonError:" message
  if lineNumber > 0 then do
    s = s "(line" lineNumber
    if column > 0 then s = s", column" column
    s = s")"
  end
  if contextLine \== "" then s = s || "0A"x || "  >" contextLine~strip
  return s


/* ========================================================================= */
/*  JsonBoolean                                                              */
/* ========================================================================= */

/** An ooRexx class to represent a JSON boolean (logical) value. It is a subclass
    of the class <code>LogicalWrapper</code> (<code>logicalwrapper.cls</code>), which provides the two instances
    (<code>.JsonBoolean~true</code> and <code>.JsonBoolean~false</code>), their
    behaviour as the Rexx logical values <code>1</code> and <code>0</code>, and
    the comparison methods. To get access to the <code>JsonBoolean</code>
    <code>true</code> and <code>false</code> sentinels it is advised to use the
    JSON class attributes <code>true</code> and <code>false</code>.
*/
::class "JSONBoolean" public subclass LogicalWrapper

/** Renders the object as a JSON string representing its logical value.
* @return a string representing its logical value JSON encoded, either &quot;<code>false</code>&quot;
*         or &quot;<code>true</code>&quot;
*/
::method makeJSON unguarded   -- creates the string "true" or "false", depending on the logical value
  if self~value then return "true"
  return "false"


/* ========================================================================= */
/*  JsonString                                                               */
/* ========================================================================= */

/** An ooRexx class that represents a JSON string value. To force numeric values
*   to be encoded as JSON strings, use this class, e.g.
*   <code>jstring=.JsonString~new("123456789")</code>
*/
::class "JsonString" subclass string  public
::method makeJSON unguarded
  buffer=.MutableBuffer~new
  call string2json buffer, self
  return buffer~string


/* ========================================================================= */
/*  JsonXmlEmitter                                                           */
/* ========================================================================= */

/** Serialises an in-memory ooRexx object tree to XML conforming to either
 *  <code>json.xsd</code> (with namespace) or <code>json.dtd</code>
 *  (with DOCTYPE).
 *
 *  <p>Used internally by <code>Json~jsonToXml</code> and
 *  <code>Json~jsonToXmlFile</code>.</p>
 *
 *  @see Json#jsonToXml
 *  @see JsonXmlParser
 */
::class JsonXmlEmitter public

/** Creates a new JsonXmlEmitter instance.
 *
 *  @param schema optional schema type: <code>"xsd"</code> (default) or
 *                <code>"dtd"</code>
 */
::method init
  expose schema
  use strict arg schema = "xsd"
  schema = schema~lower
  if schema \== "xsd" & schema \== "dtd" then
    raise syntax 93.900 additional("schema must be 'xsd' or 'dtd'")

/** Emits the given ooRexx object tree as an XML string.
 *
 *  @param obj the root object to serialise
 *  @return a well-formed XML string
 */
::method emit
  expose schema
  use strict arg obj
  mb = .mutableBuffer~new
  mb~append('<?xml version="1.0" encoding="UTF-8"?>', "0A"x)
  if schema == "xsd" then
    mb~append('<json xmlns="urn:json:xml:1.0">', "0A"x)
  else do
    mb~append('<!DOCTYPE json SYSTEM "json.dtd">', "0A"x)
    mb~append('<json>', "0A"x)
  end
  self~emitNode(mb, obj, 2)
  mb~append('</json>', "0A"x)
  return mb~string

/** Recursively serialises an ooRexx object to XML elements.
 *
 *  @param mb    the .MutableBuffer to append XML to
 *  @param obj   the object to emit
 *  @param level the current indentation level (in spaces)
 */
::method emitNode private
  use strict arg mb, obj, level

  pad = copies(" ", level)

  /* Null */
  if obj == .nil then do
    mb~append(pad, '<null/>', "0A"x)
    return
  end

  /* LogicalWrapper — check before string because a LogicalWrapper has makeString */
  if obj~isA(.LogicalWrapper) then do
    if obj~value then mb~append(pad, '<boolean>true</boolean>', "0A"x)
    else              mb~append(pad, '<boolean>false</boolean>', "0A"x)
    return
  end

  /* Object (MapCollection — Directory, Table, etc.) */
  if obj~isA(.MapCollection) then do
    if obj~items = 0 then do
      mb~append(pad, '<object/>', "0A"x)
      return
    end
    mb~append(pad, '<object>', "0A"x)
    /* Sort keys for deterministic output, matching toJSON behaviour */
    keys = .array~new
    sup = obj~supplier
    do while sup~available
      keys~append(sup~index)
      sup~next
    end
    keys = keys~sort
    do key over keys
      mb~append(pad, '  <entry>', "0A"x)
      mb~append(pad, '    <n>', self~xmlEscape(key), '</n>', "0A"x)
      mb~append(pad, '    <value>', "0A"x)
      self~emitNode(mb, obj~at(key), level + 6)
      mb~append(pad, '    </value>', "0A"x)
      mb~append(pad, '  </entry>', "0A"x)
    end
    mb~append(pad, '</object>', "0A"x)
    return
  end

  /* Array (OrderedCollection) */
  if obj~isA(.OrderedCollection) then do
    if obj~items = 0 then do
      mb~append(pad, '<array/>', "0A"x)
      return
    end
    mb~append(pad, '<array>', "0A"x)
    do item over obj
      mb~append(pad, '  <item>', "0A"x)
      self~emitNode(mb, item, level + 4)
      mb~append(pad, '  </item>', "0A"x)
    end
    mb~append(pad, '</array>', "0A"x)
    return
  end

  /* Scalar — string or number */
  self~emitScalarNode(mb, obj, level)

/** Emits a scalar value as an XML element.
 *  JsonString values are always emitted as <string>.
 *  Numeric strings are emitted as <number>.
 *  All other strings are emitted as <string>.
 *
 *  @param mb    the .MutableBuffer to append to
 *  @param value the scalar value to emit
 *  @param level the indentation level
 */
::method emitScalarNode private
  use strict arg mb, value, level
  pad = copies(" ", level)
  /* JsonString — always emit as <string> */
  if value~isA(.JsonString) then do
    mb~append(pad, '<string>', self~xmlEscape(value~string), '</string>', "0A"x)
    return
  end
  /* Numeric — emit as <number> */
  if value~isA(.string), value~dataType('N') then do
    if isJsonNumber(value) then
      mb~append(pad, '<number>', value, '</number>', "0A"x)
    else do
      numeric digits value~length
      mb~append(pad, '<number>', value + 0, '</number>', "0A"x)
    end
    return
  end
  /* Default — string */
  mb~append(pad, '<string>', self~xmlEscape(value~string), '</string>', "0A"x)

/** Escapes special XML characters in a text string.
 *  Replaces &amp;, &lt;, &gt;, &quot;, and &apos; with their XML entity
 *  equivalents.
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
      when ch == '08'x then mb~append("\b")    -- JSON \b, illegal in XML 1.0
      when ch == '0C'x then mb~append("\f")    -- JSON \f, illegal in XML 1.0
      when ch == '0D'x then mb~append("\r")    -- JSON \r, XML normalises CR to LF
      when ch~c2d < 32, ch \== '09'x, ch \== '0A'x then
        mb~append("\u00", ch~c2x)              -- other XML-illegal control chars
      when ch == "\" then mb~append("\\")      -- escape backslash for XSL round-trip
      otherwise mb~append(ch)
    end
  end
  return mb~string


/* ========================================================================= */
/*  JsonXmlParser                                                            */
/* ========================================================================= */

/** Parses XML conforming to <code>json.xsd</code> or <code>json.dtd</code>
 *  back into ooRexx objects.  This is a purpose-built parser for the JSON XML
 *  vocabulary &mdash; it is not a general-purpose XML parser.
 *
 *  <p>Used internally by <code>Json~parseXml</code> and
 *  <code>Json~parseXmlFile</code>.</p>
 *
 *  @see Json#parseXml
 *  @see JsonXmlEmitter
 */
::class JsonXmlParser public

/** Parses an XML string into an ooRexx object tree.
 *
 *  @param input a well-formed XML string conforming to the JSON XML vocabulary
 *  @return the ooRexx object tree
 */
::method parse
  expose text pos len
  use strict arg input
  text = input
  len  = text~length
  pos  = 1
  /* Skip XML declaration and DOCTYPE if present */
  self~skipProlog
  /* Expect <json ...> */
  self~skipWS
  tag = self~readTag
  if tag["name"] \== "json" then
    raise syntax 93.900 additional("Expected <json> root element, got <" || tag["name"] || ">")
  self~skipWS
  node = self~readNode
  self~skipWS
  self~expectCloseTag("json")
  return node

/** Skips the XML prolog (declaration, DOCTYPE, comments) at the
 *  beginning of an XML document.
 */
::method skipProlog private
  expose text pos len
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
 *
 *  @return a .Table with keys: "name" (string), "attrs" (.Table),
 *          "selfClose" (.true or .false)
 */
::method readTag private
  expose text pos len
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
 *  Dispatches based on the element name.
 *
 *  @return the parsed ooRexx object
 */
::method readNode private
  expose text pos len
  self~skipWS
  if pos > len then return .nil
  tag = self~readTag
  select case tag["name"]
    when "object"  then return self~readObject(tag)
    when "array"   then return self~readArray(tag)
    when "string"  then return self~readString(tag)
    when "number"  then return self~readNumber(tag)
    when "boolean" then return self~readBoolean(tag)
    when "null"    then return .nil
    otherwise
      raise syntax 93.900 additional("Unexpected element: <" || tag["name"] || ">")
  end

/** Reads an <object> element and its <entry> children.
 *
 *  @param tag the already-parsed opening <object> tag
 *  @return a .Directory representing the object
 */
::method readObject private
  expose text pos len
  use strict arg tag
  dir = .directory~new
  if tag["selfClose"] then return dir
  do forever
    self~skipWS
    if pos > len then leave
    if text[pos] == "<", pos + 1 <= len, text[pos + 1] == "/" then leave
    /* Expect <entry> */
    entryTag = self~readTag
    if entryTag["name"] \== "entry" then
      raise syntax 93.900 additional("Expected <entry>, got <" || entryTag["name"] || ">")
    /* <name> */
    self~skipWS
    nameTag = self~readTag
    if nameTag["name"] \== "n" then
      raise syntax 93.900 additional("Expected <n>, got <" || nameTag["name"] || ">")
    /* Read text content of <name> */
    nameContent = self~readTextContent
    self~expectCloseTag("n")
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
    dir[nameContent] = val
  end
  self~expectCloseTag("object")
  return dir

/** Reads an <array> element and its <item> children.
 *
 *  @param tag the already-parsed opening <array> tag
 *  @return an .Array of items
 */
::method readArray private
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
  self~expectCloseTag("array")
  return arr

/** Reads a <string> element.
 *
 *  @param tag the already-parsed opening <string> tag
 *  @return a .JsonString
 */
::method readString private
  use strict arg tag
  if tag["selfClose"] then return .JsonString~new("")
  content = self~readTextContent
  self~expectCloseTag("string")
  return .JsonString~new(content)

/** Reads a <number> element.
 *
 *  @param tag the already-parsed opening <number> tag
 *  @return a numeric string
 */
::method readNumber private
  use strict arg tag
  if tag["selfClose"] then return 0
  content = self~readTextContent
  self~expectCloseTag("number")
  return content + 0

/** Reads a <boolean> element.
 *
 *  @param tag the already-parsed opening <boolean> tag
 *  @return a .JsonBoolean
 */
::method readBoolean private
  use strict arg tag
  if tag["selfClose"] then return .JsonBoolean~false
  content = self~readTextContent
  self~expectCloseTag("boolean")
  if content~lower == "true" then return .JsonBoolean~true
  return .JsonBoolean~false

/** Reads text content from the current position until the next '<'.
 *  Unescapes XML entities.
 *
 *  @return the unescaped text content
 */
::method readTextContent private
  expose text pos len
  startContent = pos
  do while pos <= len
    if text[pos] == "<" then leave
    pos = pos + 1
  end
  return self~xmlUnescape(text~substr(startContent, pos - startContent))

/** Unescapes XML entities in a text string.
 *
 *  @param text the XML-escaped text
 *  @return the unescaped string
 */
::method xmlUnescape private
  use strict arg text
  /* First, decode XML entities */
  text = text~changeStr("&lt;", "<")
  text = text~changeStr("&gt;", ">")
  text = text~changeStr("&quot;", '"')
  text = text~changeStr("&apos;", "'")
  text = text~changeStr("&amp;", "&")  /* must be last of the XML entities */
  /* Then, decode JSON escape sequences for XML-illegal characters.
   * Process character by character to avoid ambiguity between
   * \\ (escaped backslash) and \b/\f/\u00XX (control char escapes). */
  mb = .mutableBuffer~new
  i = 1
  do while i <= text~length
    ch = text[i]
    if ch == "\" then do
      if i < text~length then do
        next = text[i + 1]
        select
          when next == "\" then do    -- \\ -> literal backslash
            mb~append("\")
            i = i + 2
          end
          when next == "b" then do    -- \b -> backspace
            mb~append('08'x)
            i = i + 2
          end
          when next == "f" then do    -- \f -> formfeed
            mb~append('0C'x)
            i = i + 2
          end
          when next == "r" then do    -- \r -> carriage return
            mb~append('0D'x)
            i = i + 2
          end
          when next == "u" then do    -- \u00XX -> control char
            if i + 5 <= text~length then do
              hex = text~substr(i + 4, 2)
              prefix = text~substr(i + 2, 2)
              if prefix == "00", hex~dataType("x") then do
                ch2 = hex~x2c
                if ch2~c2d < 32, ch2 \== '09'x, ch2 \== '0A'x, ch2 \== '0D'x then do
                  mb~append(ch2)
                  i = i + 6
                end
                else do               -- valid \u00XX but not a control char
                  mb~append("\")
                  i = i + 1
                end
              end
              else do                 -- not \u00XX pattern
                mb~append("\")
                i = i + 1
              end
            end
            else do                   -- not enough characters for \u00XX
              mb~append("\")
              i = i + 1
            end
          end
          otherwise do                -- backslash + other char: pass through
            mb~append("\")
            i = i + 1
          end
        end
      end
      else do                         -- trailing backslash
        mb~append("\")
        i = i + 1
      end
    end
    else do
      mb~append(ch)
      i = i + 1
    end
  end
  return mb~string


/* ========================================================================= */
/*  string2json routine                                                      */
/* ========================================================================= */

/** Encodes a Rexx string as a JSON value and appends it to a MutableBuffer.
*
*   If the string is numeric and is a plain .String (not a .JsonString), it is
*   appended unquoted as a JSON number.  If it is a .JsonString, the numeric
*   value is quoted.  Non-numeric strings are always quoted with proper JSON
*   escaping of control characters, backslashes, and double quotes.
*
*   The optional <code>forceQuoted</code> parameter, when .true, forces the
*   value to be quoted regardless of whether it is numeric.  This is used
*   when encoding object keys, which must always be quoted strings in JSON.
*
*   @param buffer      the MutableBuffer to append to
*   @param rexxObject  the string to encode
*   @param forceQuoted optional; if .true, always quote (used for object keys)
*/
::routine string2json public
   use arg buffer, rexxObject, forceQuoted = .false

   if \forceQuoted, rexxObject~dataType('n') then
   do
      if rexxObject~isA(.jsonString) then buffer~append('"',rexxObject,'"')   -- as a string (quoted)
      else if isJsonNumber(rexxObject) then buffer~append(rexxObject)         -- already valid JSON number
      else do
         numeric digits rexxObject~length                                     -- avoid truncation
         buffer~append(rexxObject + 0)                                        -- normalize to JSON form
      end
   end
   else
   do
      eJS=.js.eJS    -- save environment lookups from now on
      buffer~append('"')
      do i = 1 to rexxObject~length
         char = rexxObject~substr(i, 1)
         if char == "\", rexxObject~match(i + 1, "u"), rexxObject~substr(i + 2, 4)~dataType("x") then do
            -- \uXXXX sequences already present in the source string are passed
            -- through unchanged (see class documentation for rationale)
            buffer~append(rexxObject~substr(i, 6))
            i += 5
         end
         else if eJS~hasIndex(char) then buffer~append(eJS[char])
         else if char <= '1f'x      then buffer~append("\u00", char~c2x)
         else                            buffer~append(char)
      end
      buffer~append('"')
   end
   return


/* ========================================================================= */
/*  isJsonNumber routine                                                     */
/* ========================================================================= */

/** Validates a string against the JSON number grammar (RFC 8259 §6):
*
*   number = [ "-" ] int [ frac ] [ exp ]
*   int    = "0" / ( digit1-9 *DIGIT )
*   frac   = "." 1*DIGIT
*   exp    = ( "e" / "E" ) [ "+" / "-" ] 1*DIGIT
*
*   Returns .true if the string is a valid JSON number, .false otherwise.
*   This is a package-level routine so that both the Json class (via its
*   private isJsonNumber method) and the string2json routine can use it.
*
*   @param string  the string to validate
*/
::routine isJsonNumber public
    use arg string
    pos = 1
    len = string~length
    if len == 0 then return .false

    -- optional leading minus
    if string~substr(pos, 1) == '-' then do
       pos += 1
       if pos > len then return .false
    end

    -- integer part
    char = string~substr(pos, 1)
    if char == '0' then
       pos += 1                              -- bare "0"
    else if char >= '1', char <= '9' then do
       pos += 1
       do while pos <= len                   -- additional digits
          char = string~substr(pos, 1)
          if char < '0' | char > '9' then leave
          pos += 1
       end
    end
    else return .false                       -- no valid integer start

    if pos > len then return .true           -- integer only, valid

    -- optional fractional part
    if string~substr(pos, 1) == '.' then do
       pos += 1
       if pos > len then return .false       -- trailing dot, not valid
       char = string~substr(pos, 1)
       if char < '0' | char > '9' then return .false  -- need >=1 digit
       pos += 1
       do while pos <= len
          char = string~substr(pos, 1)
          if char < '0' | char > '9' then leave
          pos += 1
       end
    end

    if pos > len then return .true           -- number with fraction, valid

    -- optional exponent part
    char = string~substr(pos, 1)
    if char == 'e' | char == 'E' then do
       pos += 1
       if pos > len then return .false       -- trailing e, not valid
       char = string~substr(pos, 1)
       if char == '+' | char == '-' then do
          pos += 1
          if pos > len then return .false    -- trailing sign, not valid
       end
       char = string~substr(pos, 1)
       if char < '0' | char > '9' then return .false  -- need >=1 digit
       pos += 1
       do while pos <= len
          char = string~substr(pos, 1)
          if char < '0' | char > '9' then leave
          pos += 1
       end
    end

    return pos > len                         -- valid only if fully consumed
