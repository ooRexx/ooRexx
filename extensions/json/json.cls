/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2010-2023 Rexx Language Association. All rights reserved.    */
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
    An ooRexx utility class to encode and decode ooRexx objects in JSON (RFC 4627).
*/

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
 * Constructor, initilizes the instance.
 */
::method init
    expose eJS uJS ctrl crlf
    use strict arg      -- no arguments allowed

    eJS = .directory~new() -- escape Javascript
    eJS['08'x] = '\b'
    eJS['09'x] = '\t'
    eJS['0A'x] = '\n'
    eJS['0C'x] = '\f'
    eJS['0D'x] = '\r'
    eJS['"'] = '\"'
    eJS['\'] = '\\'
    eJS['/'] = '\/'

    uJS = .directory~new() -- unescape Javascript
    do index over eJS
        uJS[eJS[index]] = index
    end

    -- chars that end a value
    ctrl = .Set~of(' ', '}', ']', ',', '09'x, '0a'x, '0d'x)

    crlf = .rexxInfo~endOfLine   -- get the platform's endOfLine character(s)


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
  use strict arg fn, rexxObject, legible=.false -- TODO: adjust test and documentation! 202302-06
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


/**
 * Converts a Rexx object to JSON formatting
 *
 * @param  rexxObject   The object to convert.  Accepts MapCollection, OrderedCollections,
 *                      or String objects, JsonBoolean objects or nil.  Otherwise, it calls
 *                      the makeArray method for the object, returning
 *                      "null" if no makeArray method is available.
 */
::method toJSON
    expose buffer
    use strict arg rexxObject, legible=(.false) -- default to minimized JSON

  if arg()=2 then       -- check supplied argument
     .Validate~logical("legible",legible) -- test for logical value 0 or 1

    buffer = .mutablebuffer~new()
    if legible then  -- human readable (adds ignorable whitespace)
       self~parseRexxObjectLegible(rexxObject)
    else             -- default: minimized version (no ignorable whitespace)
       self~parseRexxObject(rexxObject)

    return buffer~string

::method parseRexxObject private
    expose buffer
    use strict arg rexxObject

    select
        when rexxObject~isA(.string) then do
            self~parseRexxString(rexxObject)
        end
        when rexxObject~isA(.OrderedCollection) then do
            self~parseRexxArray(rexxObject)
        end
        when rexxObject~isA(.MapCollection) then do
            self~parseRexxDirectory(rexxObject)
        end
        when rexxObject~isNil then do
            buffer~append('null')
        end
        when rexxObject~hasMethod('makejson') then do    -- object can render itself as a JSON string; handles .JsonBoolean as well
           buffer~append(rexxObject~makeJson)
        end
        when rexxObject~hasMethod('makearray') then do
            self~parseRexxObject(rexxObject~makearray())
        end
        when rexxObject~hasMethod('makestring') then do  -- object can render itself as string
           buffer~append('"', rexxObject~makeString~changeStr('"','\"'), '"')
        end
        otherwise
            -- buffer~append('null') -- original implementation eradicated any information
            -- run the object's string method and use that value which may help debugging, if needed
            buffer~append('"', rexxObject~string~changeStr('"','\"'), '"')
    end

::method parseRexxArray private
    expose buffer
    use strict arg rexxObject

    buffer~append('[')
    if rexxObject~items() == 0 then buffer~append(']')
    else do
        do item over rexxObject
            self~parseRexxObject(item)
            buffer~append(',')
        end
        buffer~overlay(']', buffer~length)
    end

::method parseRexxDirectory private
    expose buffer
    use strict arg rexxObject

    buffer~append('{')
    if rexxObject~items == 0 then buffer~append('}')
    else do
        do index over rexxObject~allindexes~sort -- index in JSON must be a quoted string
            if index~isA(.string) then self~parseRexxString(index, .true)
            else buffer~append('"'index~class'"')
            buffer~append(':')
            self~parseRexxObject(rexxObject[index])
            buffer~append(',')
        end
        buffer~overlay('}', buffer~length)
    end

::method parseRexxString private
    expose buffer eJS
    use strict arg rexxObject, quoted = .false

    if rexxObject~length == 0 then buffer~append('""')
    else do
        if rexxObject~dataType('n') then do
            if quoted then buffer~append('"'rexxObject'"')
            else buffer~append(rexxObject)
        end
        else do
            buffer~append('"')
            do i = 1 to rexxObject~length
                char = rexxObject~substr(i, 1)
                if char = "\", rexxObject~match(i + 1, "u"), rexxObject~substr(i + 2, 4)~dataType("x") then do
                    -- balanced with our special handling of \uXXXX escape
                    -- sequences (keep as-is), we also keep them as-is here
                    buffer~append(rexxObject~substr(i, 6))
                    i += 5
                end
                else if eJS~hasIndex(char) then buffer~append(eJS[char])
                else if char <= '1f'x then buffer~append("\u00" || char~c2x)
                else buffer~append(char)
            end
            buffer~append('"')
        end
    end


/* ========================================================================== */
/* legible (human oriented) versions of creating JSON string: use whitespace
   to make structure and name(key)/value pairs easier to read for humans
*/

::method parseRexxObjectLegible private
    expose buffer leadin
    use strict arg rexxObject, level=0, leadin="   "
    select
        when rexxObject~isA(.string) then do
            self~parseRexxStringLegible(rexxObject,,level)
        end
        when rexxObject~isA(.OrderedCollection) then do
            self~parseRexxOrderedCollectionLegible(rexxObject,level)
        end
        when rexxObject~isA(.MapCollection) then do
            self~parseRexxMapCollectionLegible(rexxObject,level)
        end
        when rexxObject~isNil then do
            buffer~append('null')
        end
        when rexxObject~hasMethod('makejson') then do    -- object can render itself as a JSON string; handles .JsonBoolean as well
           buffer~append(rexxObject~makeJson)
        end
        when rexxObject~hasMethod('makearray') then do
            self~parseRexxObjectLegible(rexxObject~makearray, level+1)
        end
        when rexxObject~hasMethod('makestring') then do  -- object can render itself as string
           buffer~append('"', rexxObject~makeString~changeStr('"','\"'), '"')
        end
        otherwise
            -- buffer~append('null') -- original implementation eradicated any information
            -- run the object's string method and use that value which may help debugging, if needed
            buffer~append('"', rexxObject~string~changeStr('"','\"'), '"')
    end

::method parseRexxOrderedCollectionLegible private
    expose buffer leadin crlf
    use strict arg rexxObject, level

    buffer~append('[')
    level+=1   -- indent
    items=rexxObject~items
    if items == 0 then buffer~append(']')
    else do
        buffer~append(crlf, leadin~copies(level))
        do counter c item over rexxObject
            self~parseRexxObjectLegible(item, level)
            if c<>items then buffer~append(',', crlf, leadin~copies(level))
                        else buffer~append(     crlf, leadin~copies(level-1), ']')
        end
    end
    level-=1   -- outdent

::method parseRexxMapCollectionLegible private
    expose buffer leadin crlf
    use strict arg rexxObject, level

    buffer~append('{')
    level+=1   -- indent
    items=rexxObject~items
    if items == 0 then buffer~append('}')
    else do
        buffer~append(crlf, leadin~copies(level))
        do counter c index over rexxObject~allIndexes~sort -- index in JSON must be a quoted string
            if index~isA(.string) then self~parseRexxStringLegible(index, .true, level)
            else buffer~append('"'index~class'"')     -- TODO: just leave "index" there?
            buffer~append(': ')
            self~parseRexxObjectLegible(rexxObject[index], level)
            if c<>items then buffer~append(',', crlf, leadin~copies(level) )
                        else buffer~append(     crlf, leadin~copies(level-1), '}')
        end
    end
    level-=1   -- outdent

::method parseRexxStringLegible private
    expose buffer leadin eJS crlf
    use strict arg rexxObject, quoted = .false, level

    if rexxObject~length == 0 then buffer~append('""')
    else do
        if rexxObject~dataType('n') then do
            if quoted then buffer~append('"'rexxObject'"')
            else buffer~append(rexxObject)
        end
        else do
            buffer~append('"')
            do i = 1 to rexxObject~length
                char = rexxObject~substr(i, 1)
                if char = "\", rexxObject~match(i + 1, "u"), rexxObject~substr(i + 2, 4)~dataType("x") then do
                    -- balanced with our special handling of \uXXXX escape
                    -- sequences (keep as-is), we also keep them as-is here
                    buffer~append(rexxObject~substr(i, 6))
                    i += 5
                end
                else if eJS~hasIndex(char) then buffer~append(eJS[char])
                else if char <= '1f'x then buffer~append("\u00" || char~c2x)
                else buffer~append(char)
            end
            buffer~append('"')
        end
    end
/* ========================================================================== */

/**
 * Recursively converts a JSON text to Rexx objects.
 *
 * @param  jsonString   A JSON text.
 */
::method fromJSON
    expose jsonString jsonPos jsonStringLength
    signal on user parseError
    use strict arg jsonString

    jsonPos = 1
    jsonStringLength = jsonString~length
    self~trimLeadingWhitespace()
    rexxObject = self~parseJSONvalue()
    if jsonPos > jsonStringLength then return rexxObject
    else do
        self~trimLeadingWhitespace()
        if jsonPos > jsonStringLength then return rexxObject
        message = 'Expected end of input'
        signal extraChars
    end
return .nil

parseError:
    c = condition('o')
    message = c['ADDITIONAL'][1]
extraChars:
    raise syntax 93.900 array(message 'at' jsonString~substr(jsonPos, 25)~strip("t"))
return .nil

/**
 * Determines type of value.
 *
 */
::method parseJSONvalue private
    expose jsonString jsonPos
    signal on user parseError

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
parseError: raise propagate

/**
 * Converts a JSON object into a Rexx directory object.
 *
 */
::method parseJSONobject private
    expose jsonString jsonPos
    signal on user parseError

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
            otherwise raise user parseError array('Expected end of an object or new value')
        end
    end
return
parseError: raise propagate

/**
 * Converts JSON name:value pairs into a Rexx directory item@index.
 *
 * @param  rexxDirectory   A Rexx directory object.
 */
::method parseJSONobjectValue private
    expose jsonString jsonPos
    signal on user parseError
    use strict arg rexxDirectory

    self~trimLeadingWhitespace()
    parse value jsonString with =(jsonPos) char +1
    if char == '"' then do
        jsonPos = jsonPos + 1
        index = self~parseJSONstring()
    end
    else raise user parseError array('Name must be a quoted string')

    self~trimLeadingWhitespace()
    parse value jsonString with =(jsonPos) char +1
    if char == ':' then do
        jsonPos = jsonPos + 1
        self~trimLeadingWhitespace()
        rexxDirectory[index] = self~parseJSONvalue()
    end
    else raise user parseError array('Expected colon separating object name and value')
return
parseError: raise propagate

/**
 * Converts a JSON array into a Rexx array object.
 *
 */
::method parseJSONarray private
    expose jsonString jsonPos
    signal on user parseError

    rexxArray = .array~new()

    self~trimLeadingWhitespace()
    parse value jsonString with =(jsonPos) char +1
    if char == ']' then do
        jsonPos = jsonPos + 1
        return rexxArray
    end
    else self~parseJSONarrayValue(rexxArray)

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
                self~parseJSONarrayValue(rexxArray)
            end
            otherwise raise user parseError array('Expected end of an array or new value')
        end
    end
return
parseError: raise propagate

/**
 * Converts a JSON array values into Rexx array items.
 *
 * @param  rexxArray   A Rexx array object.
 */
::method parseJSONarrayValue private
    expose jsonString
    signal on user parseError
    use strict arg rexxArray

    self~trimLeadingWhitespace()
    index = rexxArray~last
    if .nil == index then index = 0
    rexxArray[index + 1] = self~parseJSONvalue()
return
parseError: raise propagate

/**
 * Converts a quoted JSON string into a Rexx string object.
 *
 */
::method parseJSONstring private
    expose jsonString uJS jsonPos jsonStringLength
    signal on user parseError

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
                if hex~length = 2, hex~dataType("x") then do
                    jsonPos = jsonPos + 6
                    rexxString~append(hex~x2c)
                end
                else
                    raise user parseError array("Invalid escape sequence")
            end
            else if jsonString~match(jsonPos, "\u") then do
                -- in general \uXXXX escape sequences are not supported
                -- as ooRexx has no Unicode support
                -- short of failing, we just just keep any \uXXXX as-is
                hex = jsonString[jsonPos + 2, 4]
                if hex~length = 4, hex~dataType("x") then do
                    jsonPos = jsonPos + 6
                    rexxString~append("\u", hex)
                end
                else
                    raise user parseError array("Invalid escape sequence")
            end
            else do
                raise user parseError array("Invalid escape sequence")
            end
        end
        else if char == '"' then do
            jsonPos = jsonPos + 1
            return rexxString~string
        end
        else do
            -- append to the string up to the next quote or backslash
            stop = jsonString~verify('\"', "match", jsonPos)
            if stop == 0
                then stop = jsonStringLength + 1
            rexxString~append(jsonString[jsonPos, stop - jsonPos])
            jsonPos = stop
        end
        if jsonPos > jsonStringLength then raise user parseError array('Expected end of a quoted string')
    end
return
parseError: raise propagate

/**
 * Converts other JSON types into Rexx objects.
 *
 */
::method parseJSONother private
    expose jsonString ctrl jsonPos jsonStringLength
    signal on user parseError

    length = jsonStringLength + 1
    do i = jsonPos while i \== length
        parse value jsonString with =(i) char +1
        if ctrl~hasIndex(char) then leave
    end
    parse value jsonString with =(jsonPos) string +(i - jsonPos)
    if string~datatype('n') then do
        jsonPos = jsonPos + string~length
        return string
    end
    else do
        select
            when string == 'false' then do
                jsonPos = jsonPos + string~length
                return .JsonBoolean~false
                -- return .false
            end
            when string == 'true' then do
                jsonPos = jsonPos + string~length
                return .JsonBoolean~true
                -- return .true
            end
            when string == 'null' then do
                jsonPos = jsonPos + string~length
                return .nil
            end
            otherwise nop
        end
    end
raise user parseError array('Invalid JSON value')
return
parseError: raise propagate

/**
 * Skips allowed whitespace between values.
 *
 */
::method trimLeadingWhitespace private
    expose jsonString jsonPos jsonStringLength
    jsonPos = jsonString~verify('20 09 0d 0a'x, , jsonPos)
    if jsonPos == 0
        then jsonPos = jsonStringLength + 1


/* ========================================================================= */

/** An ooRexx class to represent a JSON boolean (logical) value. It inherits from
    the ooRexx mixinclass <code>Comparable</code> and therefore implements its abstract
    method <code>compareTo</code>.
    To get access to the <code>JsonBoolean</code> <code>true</code> and <code>false</code>
    sentinels it is advised to use the JSON class attributes
    <code>true</code> and <code>false</code>.
*/
::class "JSONBoolean" public inherit comparable

/** Class getter attribute method that refers to the proxy object that represents the value <code>.true</code>.
*/
::attribute true  get class  unguarded -- true proxy, class getter method

/** Class getter attribute method that refers to the proxy object that represents the value <code>.false</code>.
*/
::attribute false get class  unguarded -- false proxy, class getter method


/** Make sure that only the JsonBoolean class can create instances.
*/
::method new class private
  forward class (super)

/** Finalizes the class initialization by creating the two proxy class attribute values <code>true</code>
*   and <code>false</code>.
*/
::method    activate  class   -- initialization of class object complete, we now can use everything
  expose true false
  true =self~new(.true)       -- create and store true proxy value
  false=self~new(.false)      -- create and store false proxy value

/** Constructor that saves argument in its attribute 'value'.
* @param value mandatory Rexx string representing the logical value
*/
::method init           -- make constructor method inaccessible to other classes, own metaclass is allowed to access directly
  expose value
  use strict arg value  -- assign boolean value

/** Forwards message to the string value of the JBoolean.
*/
::method unknown unguarded
  expose value          -- the string value of this JsonBoolean
  use arg msg, args
  forward to (value) message (msg) arguments (args)  -- maybe a method of the string value, forward it


/** Equal comparison method, needs to be overriden otherwise .Object's method gets run instead.
* @param other the other object representing a Boolean/logical value
* @return <code>.true</code>, if this object and <code>other</code> can be reqarded to be equal, <code>.false</code> else
*/
::method "=" unguarded           -- equal method
  expose value
  use strict arg other  -- other must be a Boolean value
  return value~compareTo(other)=0

/** Unequal comparison method.
* @param other the other object representing a Boolean/logical value
* @return <code>.true</code>, if this object and <code>other</code> cannot be reqarded to be equal, <code>.false</code> else
*/
::method "\=" unguarded          -- unequal method
  expose value
  use strict arg other  -- other must be a Boolean value
  return value~compareTo(other)\=0

/** Unequal comparison method, forwarding to method <code>&quot;\=&quot;</code>.
*/
::method "<>" unguarded          -- synonym for "\="
  forward message ("\=")

/** Unequal comparison method, forwarding to method <code>&quot;\=&quot;</code>.
*/
::method "><" unguarded          -- synonym for "\="
  forward message ("\=")

/** Implements the abstract method inherited from the mixinclass <code>Comparable</code>.
*/
::method compareTo unguarded  -- implementation for .orderable class: must return -1 if other greater, 0 if same, 1 otherwise
  expose value
  use strict arg other  -- other must be a Boolean value

  if other~isA(.JsonBoolean) then  -- get Rexx string representing logical value
     otherValue=other~value
  else
     otherValue=other~request("string") -- request the string value

  if otherValue=.nil then
     raise syntax 88.900 array ("Argument ""other"" ["other"] has no 'MAKESTRING' method")

  if value < otherValue then return -1    -- self smaller than other
  if value = otherValue then return  0    -- self equal to other
  return                             1    -- self greater than other

/** Renders the object as a Rexx string representing its logical value.
* @return a Rexx string representing its logical value, either &quot;<code>0</code>&quot; or
*         &quot;<code>1</code>&quot;
*/
::method makeString unguarded -- allow instances of this class to be a plug in replacement for Rexx logical values
  expose value
  return value

/** Renders the object as a JSON string representing its logical value.
* @return a string representing its logical value JSON encoded, either &quot;<code>false</code>&quot;
*         or &quot;<code>true</code>&quot;
*/
::method makeJSON unguarded   -- creates the string "true" or "false", depending on the attribute "value"
  expose value
  if value=.true then return "true"
  return "false"

