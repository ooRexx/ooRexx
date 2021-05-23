/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2010-2021 Rexx Language Association. All rights reserved.    */
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

-- A class to encode and decode ooRexx objects in JSON (RFC 4627)
::class json public

/**
 * Initilizes the class
 */
::method init
    expose eJS uJS ctrl

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

/**
 * Converts a Rexx object to JSON formatting
 *
 * @param  rexxObject   The object to convert.  Accepts directory, array,
 *                      or string objects, or nil.  Otherwise, it calls
 *                      the makeArray method for the object, returning
 *                      "null" if no makeArray method is available.
 */
::method toJSON
    expose buffer
    use strict arg rexxObject

    buffer = .mutablebuffer~new()
    self~parseRexxObject(rexxObject)
return buffer~string

::method parseRexxObject private
    expose buffer
    use strict arg rexxObject

    select
        when .array~id == rexxObject~class~id then do
            self~parseRexxArray(rexxObject)
        end
        when .directory~id == rexxObject~class~id then do
            self~parseRexxDirectory(rexxObject)
        end
        when .string~id == rexxObject~class~id then do
            self~parseRexxString(rexxObject)
        end
        when .nil == rexxObject then do
            buffer~append('null')
        end
        otherwise
            if rexxObject~hasMethod('makearray') then do
                self~parseRexxObject(rexxObject~makearray())
            end
            else buffer~append('null')
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
        do index over rexxObject -- index in JSON must be a quoted string
            if .string~id == index~class~id then self~parseRexxString(index, .true)
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

/**
 * Recursively converts a JSON text to Rexx objects
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
                return .false
            end
            when string == 'true' then do
                jsonPos = jsonPos + string~length
                return .true
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
