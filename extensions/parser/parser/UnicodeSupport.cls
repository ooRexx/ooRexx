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
/* UnicodeSupport.cls - Optional Unicode support for the Rexx Parser          */
/* =================================================================          */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20250103    0.1f First public release                                      */
/* 20250328    0.2  Main dir is now rexx-parser instead of rexx[.]parser      */
/*                  Move "resources" dir under "bin"                          */
/* 20251226         Send error messages to .error, not .output                */
/* 20261121         Unicode 18.0.0: DerivedName.txt (with NameAliases.txt)    */
/*                  instead of UnicodeData.txt. The algorithmic names (CJK,   */
/*                  Tangut, Jurchen, Small Seal, Egyptian...) and the labels  */
/*                  come from the data, not from ranges written here. Fixes:  */
/*                  Hangul syllables were never found (HANGULSYLLABE), most   */
/*                  <reserved-...> were refused, and unassigned compatibility */
/*                  ideographs (FA6E, FA6F) were accepted                     */
/* 20261121         Well.Formed.UString: a name of a surrogate (<surrogate-   */
/*                  D800>) is refused, as the code D800 was: it cannot be     */
/*                  encoded in UTF-8                                          */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/*                                                                            */
/******************************************************************************/

package     = .context~package
local       =  package~local
myName      =  package~name
myPath      =  FileSpec( "Location", myName )

-- The version of the Unicode Character Database that we use: the two files
-- in resources/ carry it in their names. To move to a new version, put its
-- DerivedName.txt (from the extracted/ directory of the UCD) and its
-- NameAliases.txt there, with the version in their names, and change this.
UnicodeVersion = "18.0.0"

-- We will ensure that these two files exist, but we will not load them
-- until they are really needed, because loading them is expensive.
DerivedName =  "DerivedName-"UnicodeVersion".txt"
NameAliases =  "NameAliases-"UnicodeVersion".txt"

Do file Over DerivedName, NameAliases
  If Stream(myPath"resources/"file, "c", "query exists") == "" Then Do
   .Error~Say( "File" file "not found in the 'resources' subdirectory." )
    Raise Halt
  End
End

-- Remember the names of our two files...
local~UnicodeVersion     = UnicodeVersion
local~DerivedName        = myPath"resources/"DerivedName
local~NameAliases        = myPath"resources/"NameAliases

-- ...and the fact that we haven't loaded them yet
local~UnicodeData.Loaded = 0

::Requires "BaseClassesAndRoutines.cls"

/******************************************************************************/
/* Name2Code(name)                                                            */
/*   Returns "" if the name is not found, else the corresponding              */
/*   Unicode codepoint. NAME can be a character name, a name alias, an        */
/*   algorithmic name (CJK UNIFIED IDEOGRAPH-4E00, TANGUT IDEOGRAPH-17000...) */
/*   or a label (<control-0007>, <reserved-0378>, <private-use-E000>,         */
/*   <surrogate-D800> or <noncharacter-FFFE>).                                */
/******************************************************************************/

::Routine Name2Code

  Call LoadNames

  NOT_FOUND = ""

  Use Strict Arg name
  nameIsALabel = 0

  -- Preprocess labels
  If name[1] == "<" Then Do
    -- A malformed label
    If name[Length(name)] \== ">" Then Return NOT_FOUND
    nameIsALabel = 1
    -- Strip name of the enclosing "<" and ">" characters first
    Parse Var name "<"name">"
  End

  -- Handle non-medial dashes
  name = ChangeStr("- ",name,">")
  name = ChangeStr(" -",name,"<")
  name = Upper(ChangeStr("_",Space(name,0),""))
  If name \== "HANGULJUNGSEONGO-E" Then name = ChangeStr("-", name,"")
  name = ChangeStr(">",name,"- ")
  name = ChangeStr("<",name," -")

  -- Restore "<" and ">" for labels
  If nameIsALabel Then name = "<"name">"

  -- Do we have a code? Return it
  code = .Name2Code[name]
  If code \== "" Then Return code

  -- Nope. A label, then, or an algorithmic name

  -- A label: the code has to be of that kind
  If name[1] == "<" Then Do
    Parse Var name "<"element">"extra
    If extra \== "" Then Return NOT_FOUND
    labels = "CONTROL PRIVATEUSE RESERVED SURROGATE NONCHARACTER"
    Do word Over labels~makeArray(" ")
      If element~startsWith(word) Then Do
        code = SubStr(element,Length(word)+1)
        If \ValidCode()             Then Return NOT_FOUND
        If LabelOf(code) \== word   Then Return NOT_FOUND
        Return code
      End
    End
    Return NOT_FOUND
  End

  -- An algorithmic name: the prefix of a family (DerivedName.txt), and a
  -- code in one of its ranges
  Do family Over .Name.Families
    prefix = family[1]
    If \name~startsWith(prefix) Then Iterate
    code = SubStr(name,Length(prefix)+1)
    If \ValidCode()                 Then Iterate
    n = X2D(code)
    If n >= family[2], n <= family[3] Then Return code
  End

  Return NOT_FOUND

ValidCode:
  -- A code has from 4 to 6 hexadecimal digits, and no leading zeros
  -- when it has more than 4
  If \DataType(code,"X")              Then Return 0
  If Length(code) < 4                 Then Return 0
  If Length(code) > 6                 Then Return 0
  If Length(code) > 4, code[1] == "0" Then Return 0
  If X2D(code) > 1114111              Then Return 0
  Return 1

LabelOf: Procedure
  -- The label of a code, or "" when it is an assigned character with a name
  Use Strict Arg code
  n = X2D(code)
  If n <= 31                          Then Return "CONTROL"      -- 0000..001F
  If n >= 127,   n <= 159             Then Return "CONTROL"      -- 007F..009F
  If n >= 55296, n <= 57343           Then Return "SURROGATE"    -- D800..DFFF
  If n >= 64976, n <= 65007           Then Return "NONCHARACTER" -- FDD0..FDEF
  If n // 65536 >= 65534              Then Return "NONCHARACTER" -- xxFFFE/F
  If n >= 57344, n <= 63743           Then Return "PRIVATEUSE"   -- E000..F8FF
  If n >= 983040                      Then Return "PRIVATEUSE"   -- Planes 15-16
  If .Assigned[code]                  Then Return ""
  Do family Over .Name.Families
    If n >= family[2], n <= family[3] Then Return ""
  End
  Return "RESERVED"

/******************************************************************************/
/* LoadNames                                                                  */
/*   Read DerivedName.txt and NameAliases.txt and build a stem storing all    */
/*   the compressed names and their corresponding codes, a stem with the      */
/*   assigned codes that have a name, and an array with the families of       */
/*   algorithmic names (their compressed prefix and each of their ranges).    */
/******************************************************************************/

-------------------------------------------------------------------------------
--
-- See The Unicode® Standard. Version 15.0 – Core Specification, p. 181
-- https://www.unicode.org/versions/Unicode15.0.0/UnicodeStandard-15.0.pdf
-- (the rule is UAX44-LM2 in Unicode Standard Annex #44, and it has not
-- changed since)
--
-- "Character Name Matching. When matching identifiers transposed from
-- character names, it is possible to ignore case, whitespace, and all
-- medial hyphen-minus characters (or any “_” replacing a hyphen-minus),
-- except for the hyphen-minus in U+1180 HANGUL JUNGSEONG O-E, and still
-- result in a unique match. For example, “ZERO WIDTH SPACE” is equivalent
-- to “zero-width-space” or “ZERO_WIDTH_SPACE” or “ZeroWidthSpace”. However,
-- “TIBETAN LETTER A” should not match “TIBETAN LETTER -A”, because in that
-- instance the hyphen-minus is not medial between two letters, but is
-- instead preceded by a space. For more information on character name
-- matching, see Section 5.9, “Matching Rules” in Unicode Standard Annex #44,
-- “Unicode Character Database.”
-------------------------------------------------------------------------------

::Routine LoadNames

  -- Run once and cache results in .context~package~local
  If .UnicodeData.Loaded Then Return

  name2Code. = ""          -- (Compressed) names to Unicode codepoints
  assigned.  = 0           -- The codes that DerivedName.txt names one by one
  families   = .Array~new  -- (prefix, first, last), first and last in decimal

  -- We know that these two files exist because we checked it in the prolog
  DerivedName = File2Array(.DerivedName)
  NameAliases = File2Array(.NameAliases)

  -- DerivedName.txt: "code ; NAME", or "first..last ; PREFIX-*" for a range
  -- of algorithmic names ("4E00..9FFF ; CJK UNIFIED IDEOGRAPH-*")
  Do line Over DerivedName
    If line == "" Then Iterate
    If line[1] == "#" Then Iterate
    Parse Var line codes";"name
    codes = Strip(codes)
    name  = Strip(name)
    If name~endsWith("-*") Then Do
      Parse Var codes first".."last
      If last == "" Then last = first
      prefix = Compress( Left( name, Length(name) - 2 ) )
      families~append( .Array~of( prefix, X2D(first), X2D(last) ) )
      Iterate
    End

    -- An exceptional name
    If name = "HANGUL JUNGSEONG O-E" Then key = "HANGULJUNGSEONGO-E"
    -- The standard case
    Else key = Compress(name)

    -- Store in our stems
    name2Code.key  = codes
    assigned.codes = 1
  End

  -- Now parse NameAliases.txt

  Do line Over NameAliases
    If line[1] == "#" Then Iterate
    If line     = ""  Then Iterate
    Parse Var line code";"name";"
    key = Compress(name)
    name2Code.key = code
  End

  local = .context~package~local
  local~UnicodeData.Loaded = 1
  local~name2Code          = name2Code.
  local~assigned           = assigned.
  local~Name.Families      = families

Return

--------------------------------------------------------------------------------

Compress:
  key = ChangeStr("- ",Arg(1),">") -- Preserve non-medial dash-blank sequences
  key = ChangeStr(" -",key, "<")   -- Preserve non-medial blank-dash sequences
  key = ChangeStr("-" ,key, "")    -- Delete all dashed (will all be medial)
  key = Space(key,0)               -- Delete all blanks too
  key = ChangeStr(">",key,"- ")    -- Restore non-medial dash-blank sequences..
  key = ChangeStr("<",key," -")    -- ..and non-medial blank-slash sequences too
Return key

/******************************************************************************/
/* Well.Formed.UString(string)                                                */
/*   Returns an array containing two items. The first item is the null string */
/*   when string contains a well-formed U string, and then the second item    */
/*   is the interpreted (i.e., translated) string value. If the U string is   */
/*   not well-formed, then the first item in the array is a custom error      */
/*   message (for syntax error 22.900), and the second item can be discarded. */
/******************************************************************************/

::Routine Well.Formed.UString Public

  Use Strict Arg string
  res = ""
  Do While string \= " "
    string = Strip(string)
    If string[1] == "(" Then Do
      Parse var string "("name")"extra
      If extra == "" Then If \string~endsWith(")") Then Exit BadName("("name)
      If \.UnicodeData.Loaded Then Call LoadNames
      string = Strip(extra)
      code = Name2Code(name)
      If code = "" Then Exit BadName(name)
      -- A surrogate has a name (a label), but it cannot be encoded
      If X2D(code) >= X2D(D800), X2D(code) <= X2D(DFFF)
                                   Then Exit BadSurrogate(name, code)
      res ||= IUTF8(code)
      Iterate
    End
    Parse Var string word string
    If Upper(word) == "U+"         Then Exit BadCodepoint(word)
    If Upper(Left(word,2)) == "U+" Then word = SubStr(word,3)
    If \DataType(word,"X")         Then Exit BadCodepoint(word)
    If X2D(word) > X2D(10FFFF)     Then Exit BadCodepoint(word)
    If X2D(word) >= X2D(D800), X2D(word) <= X2D(DFFF)
                                   Then Exit BadCodepoint(word)
    res ||= IUTF8(word)
  End

  Return "", res

BadCodepoint: Return .Array~of("Invalid Unicode codepoint '"Arg(1)"'")
BadName:      Return .Array~of("Invalid Unicode name '"Arg(1)"'")
BadSurrogate: Return .Array~of("Invalid Unicode name '"Arg(1)"':" -
                "U+"Arg(2) "is a surrogate, which cannot be encoded")

IUTF8: Procedure -- Internal, fast
  Use Arg code
  If code~length < 4 Then code = Right(code,4,0)
  Do While code~length > 4, code[1] == 0
    code = Substr(code,2)
  End
  n = X2D(code)
  b = X2B(code)
  If b~length == 20 Then b = "0000"||b
  If b~length == 8, n >= 128 Then b = "0000"||b
  Select
    When n <= 127   Then Return X2C(code[3,2])
    When n <= 2047  Then Return X2C(B2X("110"SubStr(b,6,5)"10"Right(b,6)))
    When n <= 65535 Then Return X2C(B2X("1110"Left(b,4)"10"SubStr(b,5,6)"10"Right(b,6)))
    Otherwise            Return X2C(B2X("11110"SubStr(b,4,3) "10"SubStr(b,7,6) "10"SubStr(b,13,6) "10"Right(b,6)))
  End



/******************************************************************************/
/* Well.Formed.UTF8(string)                                                   */
/*   Returns the null string when string is a well-formed UTF-8 string,       */
/*   and the first offending character sequence when it is not.               */
/******************************************************************************/

-- See The Unicode® Standard. Version 15.0 – Core Specification
-- https://www.unicode.org/versions/Unicode15.0.0/UnicodeStandard-15.0.pdf,
-- table 3-7, p. 125.

::Routine Well.Formed.UTF8 Public

  Use Strict Arg string

  ASCII       =  XRange("00"x,"7F"x)

  length      =  Length(string)
  pos         =  0

  WELL_FORMED =  ""

--------------------------------------------------------------------------------

NextChar: -- Look for the first character that is not ASCII
  pos = Verify(string, ASCII,, pos+1)

  -- All ASCII? That's a well-formed UTF-8 string
  If pos == 0 Then Exit WELL_FORMED

  -- See what kind of char we have
  ch   = string[pos]

  -- Lone continuations, or illegal characters
  If ch << "C2"X Then Exit ch
  If ch >> "F4"X Then Exit ch

  -- "C2".."DF"? We need exactly one continuation character
  If ch << "E0"X Then Do
    next   = string[pos+1]
    If \ContinuationChar( next )   Then Signal Error2
    pos += 1
    Signal NextChar
  End

  -- "E0".."EF"? We need exactly two continuation characters
  If ch << "F0"X Then Do
    next1   = string[pos+1]
    next2   = string[pos+2]
    If \ContinuationChar( next1 )  Then Signal Error3
    If \ContinuationChar( next2 )  Then Signal Error3
    -- Special cases
    If ch == "E0"X, next1 << "A0"X Then Signal Error3
    Else
    If ch == "ED"X, next1 >> "9F"X Then Signal Error3
    pos += 2
    Signal NextChar
  End

  -- "F0".."F4"? We need exactly three continuation characters
  Do
    next1   = string[pos+1]
    next2   = string[pos+2]
    next3   = string[pos+3]
    If \ContinuationChar( next1 )  Then Signal Error4
    If \ContinuationChar( next2 )  Then Signal Error4
    If \ContinuationChar( next3 )  Then Signal Error4
    -- Special cases
    If ch == "F0"X, next1 << "90"X Then Signal Error4
    Else
    If ch == "F4"X, next1 >> "8F"X Then Signal Error4
    pos += 3
    Signal NextChar
  End

--------------------------------------------------------------------------------

Error2: Exit ch || next
Error3: Exit ch || next1 || next2
Error4: Exit ch || next1 || next2 || next3

--------------------------------------------------------------------------------

ContinuationChar:
  If Arg(1) >= "80"X, Arg(1) <= "BF"X Then Return 1
  Return 0