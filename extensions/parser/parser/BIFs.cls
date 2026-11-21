/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2025-2026 Josep Maria Blasco. All rights reserved.           */
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
/* BIFs.cls - Built-in function support                                       */
/* ====================================                                       */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20250414    0.2  First version                                             */
/* 20250421    0.2b Major rewrite, add BIF early checking                     */
/* 20251116    0.3a Add support for the new GC BIF                            */
/* 20251226    0.4a Send error messages to .error, not .output                */
/* 20260521    0.5  CMS: Fix BIF list                                         */
/* 20260601    0.6  Implement a fine-grained feature system                   */
/* 20260923    0.7  STRIP: the ooRexx third argument is a character set (ANY) */
/* 20260924         VAR: the argument is rANY, not rSYM (ooRexx returns 0     */
/*                  for a non-symbol, it does not raise an error).            */
/* 20261105         ooRexx r13363: BIF argument errors are now 40.x, not      */
/*                  93.x (ooRexx bug 2136).                                   */
/* 20261106         ooRexx r13375 (bug 2009): BEEP checks its frequency       */
/*                  first, and a non-whole number is 88.903.                  */
/* 20261110         ooRexx r13437 (RFE 766): ERRORTEXT takes an error code    */
/*                  nn.nnn too; a bad one is 40.926 (type ERRCODE).           */
/* 20261113         ooRexx r13480 (RFE 766): ERRORTEXT has an optional second */
/*                  argument, the values of the substitution markers.         */
/* 20261115         Use CATCH instead of SIGNAL ON (RFE #31; needs ooRexx     */
/*                  r13514 or later)                                          */
/* 20261117         ooRexx r13531 (RFE 890): BEEP, DIRECTORY and FILESPEC are */
/*                  built-in functions; their argument errors are 40.x (they  */
/*                  were 88.9xx), with the new 40.927 for the ranges of BEEP  */
/* 20261117         40.904 shows what ooRexx shows (BadOption): the whole     */
/*                  value for most functions; the first character for         */
/*                  FILESPEC, and uppercased for DATE, TIME and DATATYPE (""  */
/*                  when there is none; a NUL for DATATYPE, unreported)       */
/* 20261118         ooRexx r13538: DATATYPE shows "" for an empty option, as  */
/*                  DATE and TIME (the NUL is fixed)                          */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261121         .rexxcheck.debug, as rxcheck.rex is now rexxcheck.rex     */
/*                                                                            */
/******************************************************************************/

  -- When writing this package, we have uncovered a large number of ooRexx
  -- bugs. Many of them are documented in the code below
  --
  -- See also https://rexx.epbcn.com/rexx-parser/doc/ref/classes/rexx.parser/early-check/#bugs

  If .environment~Parser.BIFInfo.Loaded = 1 Then Return

 .environment~Parser.BIFInfo.Loaded = 1

::Requires "BaseClassesAndRoutines.cls"

/******************************************************************************/
/* LoadBIFInfo -- Load BIF information structures                             */
/*                                                                            */
/*   The purpose of this routine is to load a set of global constants into    */
/*   the environment.                                                         */
/*                                                                            */
/******************************************************************************/

::Routine LoadBIFInfo Public

  -- ANSI Rexx does not define LOWER and UPPER

  BIFARGS.ABBREV       = "rANY rANY oWHOLE>=0"
  BIFARGS.ABS          = "rNUM"
  BIFARGS.ADDRESS      = "oEINO"
  BIFARGS.ARG          = "oWHOLE>0 oENO"
  BIFARGS.B2X          = "rBIN"
  BIFARGS.BITAND       = "rANY oANY oPAD"
  BIFARGS.BITOR        = "rANY oANY oPAD"
  BIFARGS.BITXOR       = "rANY oANY oPAD"
  BIFARGS.C2D          = "rANY oWHOLE>=0"
  BIFARGS.C2X          = "rANY"
  BIFARGS.CENTER       = "rANY rWHOLE>=0 oPAD"
  BIFARGS.CENTRE       = "rANY rWHOLE>=0 oPAD"
  BIFARGS.CHANGESTR    = "rANY rANY rANY"
  BIFARGS.CHARIN       = "oSTREAM oWHOLE>0 oWHOLE>=0"
  BIFARGS.CHAROUT      = "oSTREAM oANY oWHOLE>0"
  BIFARGS.CHARS        = "oSTREAM oCN"
  BIFARGS.COMPARE      = "rANY rANY oPAD"
  BIFARGS.CONDITION    = "oCDEIS"
  BIFARGS.COPIES       = "rANY rWHOLE>=0"
  BIFARGS.COUNTSTR     = "rANY rANY"
  BIFARGS.D2C          = "rD2 oWHOLE>=0"
  -- If Arg(2,"E") Then rD2 = "rD2WHOLE>=0"
  -- Else               rD2 = "rD2WHOLE"
  BIFARGS.D2X          = "rD2 oWHOLE>=0"
  -- If Arg(2,"E") Then rD2 = "rD2WHOLE>=0"
  -- Else               rD2 = "rD2WHOLE"
  BIFARGS.DATATYPE     = "rANY oABLMNSUWX"
  BIFARGS.DATE         = "oBDEMNOSUW oANY oBDENOSU"
  BIFARGS.DELSTR       = "rANY rWHOLE>0 oWHOLE>=0"
  BIFARGS.DELWORD      = "rANY rWHOLE>0 oWHOLE>=0"
  BIFARGS.DIGITS       = ""
  BIFARGS.ERRORTEXT    = "r0_90 oSN"
  BIFARGS.FORM         = ""
  BIFARGS.FORMAT       = "rNUM oWHOLE>=0 oWHOLE>=0 oWHOLE>=0 oWHOLE>=0"
  BIFARGS.FUZZ         = ""
  BIFARGS.INSERT       = "rANY rANY oWHOLE>=0 oWHOLE>=0 oPAD"
  BIFARGS.LASTPOS      = "rANY rANY oWHOLE>0"
  BIFARGS.LEFT         = "rANY rWHOLE>=0 oPAD"
  BIFARGS.LENGTH       = "rANY"
  BIFARGS.LINEIN       = "oSTREAM oWHOLE>0 oWHOLE>=0"
  BIFARGS.LINEOUT      = "oSTREAM oANY oWHOLE>0"
  BIFARGS.LINES        = "oSTREAM oCN"
  BIFARGS.MAX          = "rMAX" -- Strip(Copies("rNUM ",Arg())), Arg() > 0
  BIFARGS.MIN          = "rMIN" -- Strip(Copies("rNUM ",Arg())), Arg() > 0
  BIFARGS.OVERLAY      = "rANY rANY oWHOLE>0 oWHOLE>=0 oPAD"
  BIFARGS.POS          = "rANY rANY oWHOLE>0"
  BIFARGS.QUALIFY      = "oSTREAM"
  BIFARGS.QUEUED       = ""
  BIFARGS.RANDOM       = "oWHOLE>=0 oWHOLE>=0 oWHOLE>=0"
  BIFARGS.REVERSE      = "rANY"
  BIFARGS.RIGHT        = "rANY rWHOLE>=0 oPAD"
  BIFARGS.SIGN         = "rNUM"
  BIFARGS.SOURCELINE   = "oWHOLE>0"
  BIFARGS.SPACE        = "rANY oWHOLE>=0 oPAD"
  BIFARGS.STREAM       = "rSTREAM oCDS oANY" -- Special
  -- Third argument is only correct with 'C'
  --   If Arg(2,"E"), Translate(Left(Arg(2)),1) == "C" Then "rSTREAM rCDS rANY"
  --                                                   Else "rSTREAM oCDS"
  BIFARGS.STRIP        = "rANY oBLT oPAD"
  BIFARGS.SUBSTR       = "rANY rWHOLE>0 oWHOLE>=0 oPAD"
  BIFARGS.SUBWORD      = "rANY rWHOLE>0 oWHOLE>=0"
  BIFARGS.SYMBOL       = "rANY" -- Special. See ANSI 9.8.4
  BIFARGS.TIME         = "oCEHLMNORS oANY oCHLMNS"
  BIFARGS.TRACE        = "oACEFILNOR"     -- Should first check for "?" prefixes
  BIFARGS.TRANSLATE    = "rANY oANY oANY oPAD"
  BIFARGS.TRUNC        = "rNUM oWHOLE>=0"
  -- VALUE: IF the third arg exists, then the first is rSYM, not rANY
  BIFARGS.VALUE        = "rANY oANY oANY"
  BIFARGS.VERIFY       = "rANY rANY oMN oWHOLE>0"
  BIFARGS.WORD         = "rANY rWHOLE>0"
  BIFARGS.WORDINDEX    = "rANY rWHOLE>0"
  BIFARGS.WORDLENGTH   = "rANY rWHOLE>0"
  BIFARGS.WORDPOS      = "rANY rANY oWHOLE>0"
  BIFARGS.WORDS        = "rANY"
  BIFARGS.X2B          = "rHEX"
  BIFARGS.X2C          = "rHEX"
  BIFARGS.X2D          = "rHEX oWHOLE>=0"
  BIFARGS.XRANGE       = "oPAD oPAD"

  -- ooRexx additions and ooRexx-specific functions
  --
  -- NOTE: BEEP, DIRECTORY and FILESPEC are built-in functions since
  -- ooRexx r13531 (RFE 890). Before, the interpreter treated them as if
  -- they were part of RexxUtil (bug 1885), and their errors were 88.9xx.
  --
  -- BIFs marked SPECIAL are handled in SecondPass.cls
  --
  -- Created new categories LENGTH, POS, SEP
  -- TODO Check SEP HEX

  BIFARGS.ABBREV       = "rANY rANY oLENGTH"
  BIFARGS.ADDRESS      = ""
  BIFARGS.ARG          = "oWHOLE>0 oAENO"    -- Special
  BIFARGS.BEEP         = "rWHOLE>0 rWHOLE>=0"
  BIFARGS.C2D          = "rANY oLENGTH"
  BIFARGS.CENTER       = "rANY rLENGTH oPAD"
  BIFARGS.CENTRE       = "rANY rLENGTH oPAD"
  BIFARGS.CHANGESTR    = "rANY rANY rANY oWHOLE>=0"
  BIFARGS.CHARS        = "oSTREAM"
  BIFARGS.CHARIN       = "oSTREAM oWHOLE>0 oLENGTH"
  BIFARGS.CONDITION    = "oACDEIORS"
  BIFARGS.D2C          = "rD2 oLENGTH"       -- Special
  -- If Arg(2,"E") Then rD2 = "rD2WHOLE>=0"
  -- Else               rD2 = "rD2WHOLE"
  BIFARGS.D2X          = "rD2 oLENGTH"       -- Special
  -- If Arg(2,"E") Then rD2 = "rD2WHOLE>=0"
  -- Else               rD2 = "rD2WHOLE"
  BIFARGS.DATATYPE     = "rANY oABILMNOSUVWX9"
  BIFARGS.DATE         = "oBDEFILMNOSTUW oANY oBDEFINOSTU oSEP oSEP"
  BIFARGS.DELSTR       = "rANY oPOS oLENGTH"
  BIFARGS.DELWORD      = "rANY rPOS oLENGTH"
  BIFARGS.DIRECTORY    = "oANY"
  BIFARGS.ENDLOCAL     = ""                  -- Linux only
  BIFARGS.ERRORTEXT    = "rERRCODE oANY"    -- r13437, r13480 (RFE 766)
  BIFARGS.FILESPEC     = "rDELNP rANY"
  BIFARGS.GC           = "oF"
  -- INSERT: ooRexx bug 2012 fixed in r13112. Arg(3) error is now 40.013.
  BIFARGS.INSERT       = "rANY rANY oWHOLE>=0 oLENGTH oPAD"
  BIFARGS.LASTPOS      = "rANY rANY oPOS oLENGTH"
  BIFARGS.LEFT         = "rANY rLENGTH oPAD"
  BIFARGS.LOWER        = "rANY oPOS oLENGTH"
  BIFARGS.OVERLAY      = "rANY rANY oPOS oLENGTH oPAD"
  BIFARGS.POS          = "rANY rANY oPOS oLENGTH"
  BIFARGS.RANDOM       = "oWHOLE oWHOLE oWHOLE>=0"
  BIFARGS.RIGHT        = "rANY rLENGTH oPAD"
  BIFARGS.RXFUNCADD    = "rANY rANY oANY"
  BIFARGS.RXFUNCDROP   = "rANY"
  BIFARGS.RXFUNCQUERY  = "rANY"
  BIFARGS.RXQUEUE      = "rCDEGOS oANY"      -- Special TODO
  BIFARGS.SETLOCAL     = ""                  -- Linux only
  BIFARGS.SPACE        = "rANY oLENGTH oPAD"
  -- STREAM: Order of Arg(2) is different in ooRexx
  BIFARGS.STREAM       = "rSTREAM oSDC oANY"
  -- STRIP: in ooRexx the third argument is a SET of characters (any string,
  -- the null string included), not a single pad character as in ANSI.
  BIFARGS.STRIP        = "rANY oBLT oANY"
  BIFARGS.SUBSTR       = "rANY rPOS oLENGTH oPAD"
  BIFARGS.SUBWORD      = "rANY rPOS oLENGTH"
  BIFARGS.QUALIFY      = "rSTREAM"
  BIFARGS.TIME         = "oCEFHLMNORST oANY oCFHLMNOST"
  BIFARGS.TRANSLATE    = "rANY oANY oANY oPAD oPOS oLENGTH"
  BIFARGS.UPPER        = "rANY oPOS oLENGTH"
  BIFARGS.USERID       = ""
  BIFARGS.VAR          = "rANY"
  BIFARGS.VERIFY       = "rANY rANY oMN oPOS oLENGTH"
  BIFARGS.WORD         = "rANY rPOS"
  BIFARGS.WORDINDEX    = "rANY rPOS"
  BIFARGS.WORDLENGTH   = "rANY rPOS"
  BIFARGS.WORDPOS      = "rANY rANY oPOS"
  BIFARGS.X2D          = "rHEX oLENGTH"      -- TODO
  BIFARGS.XRANGE       = "oRANGE"            -- Special

  If .Options.CMS Then Do
    -- CMS-specific additions
    BIFARGS.EXTERNALS  = ""
    BIFARGS.FIND       = "rANY rANY"
    BIFARGS.INDEX      = "rANY rANY oPOS"
    BIFARGS.JUSTIFY    = "rANY rLENGTH oPAD"
    BIFARGS.LINESIZE   = ""

    -- CMS signature overrides (different from ooRexx)
    BIFARGS.DATE       = "oBCDJEMNOSUW oANY oBCDJEMNOSUW oSEP oSEP"
    BIFARGS.DELSTR     = "rANY rPOS oLENGTH"           -- n is required in CMS
    BIFARGS.LASTPOS    = "rANY rANY oPOS"              -- no length arg
    BIFARGS.LINES      = "oSTREAM"                     -- no C/N arg
    BIFARGS.POS        = "rANY rANY oPOS"              -- no length arg
    BIFARGS.STREAM     = "rSTREAM oSCD oANY"
    BIFARGS.STRIP      = "rANY oBLT oPAD"             -- a single character
    BIFARGS.TIME       = "oCEHLMNRS"                   -- 1 arg, CMS option set
    BIFARGS.TRANSLATE  = "rANY oANY oANY oPAD"         -- no start/length
    BIFARGS.VERIFY     = "rANY rANY oMN oPOS"          -- no length arg

    -- ooRexx BIFs not in CMS
    Do name Over BEEP,    CHANGESTR, COUNTSTR,   DIRECTORY,   ENDLOCAL, FILESPEC, GC,     LOWER, -
                 QUALIFY, RXFUNCADD, RXFUNCDROP, RXFUNCQUERY, RXQUEUE,  SETLOCAL, UPPER,  VAR
      Drop BIFARGS.name
    End
  End

  -- TUTOR additions. Types "UNI" and "UTF8" added

  -- [Only checked for required an optional args at the moment]
  If .Options.Unicode Then Do

    BIFARGS.BYTES      = "rANY"
    BIFARGS.C2U        = "rANY oANY" -- Arg(2): one of CODES, UTF32, U+ or NAmes
    BIFARGS.CODEPOINTS = "rUTF8"
    BIFARGS.DECODE     = "rANY rANY oANY oANY"
    BIFARGS.ENCODE     = "rANY rANY oANY"
    BIFARGS.GRAPHEMES  = "rUTF8"
    BIFARGS.N2P        = "rANY"
    BIFARGS.P2N        = "rANY"
    BIFARGS.STRINGTYPE = "rUTF8"
    BIFARGS.TEXT       = "rUTF8"
    BIFARGS.U2C        = "rUNI"
    BIFARGS.UNICODE    = "rANY rANY oANY" -- See the docs
    BIFARGS.UTF8       = "rANY oANY oANY oANY"

  End

  -- We now construct .Parser.BIF, a set with all the BIF names, and
  -- .Parser.BIFInfo, a directory indexed by these BIF names.
  -- Every indexed item is also a directory, containing three entries:
  --   minArgs: The minimum number of args
  --   maxArgs: The maximum number of args
  --   argType: An array with maxArgs elements defining the respective
  --     argument types.
  --   BIFNumber: A sequential number identifying the BIF.

  BIF = .Set~new
 .environment~Parser.BIF     = BIF
  BIFInfo = .Directory~new
 .environment~Parser.BIFInfo = BIFInfo
  Do Counter c bifName Over BIFARGS.~allIndexes~sort
    BIF[] = bifName
    BIFInfo[bifName] = .Directory~new
    args  = BIFARGS.bifName
    maxArgs = Words(args)
    If args == "(special)" Then Iterate
    If WordPos(bifName, "MAX MIN XRANGE") > 0 Then maxArgs = "*"
    BIFInfo[bifName]~maxArgs = maxArgs
    BIFInfo[bifName]~argType = .Array~new
    BIFInfo[bifName]~BIFNumber = c
    required      = 0
    foundOptional = 0
    Do word Over args~makeArray(" ")
      Parse var word required? +1 type
      Select Case required?
        When "r" Then Do
          If foundOptional Then Call InternalError "required argument after optional argument in definition of" bifName
          required += 1
        End
        When "o" Then foundOptional = 1
        Otherwise Call InternalError "'r' or 'o' expected, found '"required?"' in '"word"'"
      End
      BIFInfo[bifName]~argType~append(type)
    End
    BIFInfo[bifName]~minArgs = required
  End

Exit

InternalError:
 .Error~Say( "Internal error in BIFs definitions:" Arg(1)"." )
  Raise Halt

/******************************************************************************/
/* CheckBIFArgs -- Check BIF signatures and constant terms                    */
/******************************************************************************/

::Routine CheckBIFArgs Public

  Use Strict Arg label, arguments

If .rexxcheck.debug = 1 Then Trace ?a

  BIFName = label~value

  BIFInfo = .Parser.BIFInfo[BIFName]

  nArgs = arguments~arg()

  -- Check if there are more args than the BIF allows
  maxArgs = BIFInfo~maxArgs
  If maxArgs \== "*", nArgs > maxArgs Then Signal 40.004

  -- Check if there are less args than the minimum required by the BIF
  minArgs = BIFInfo~minArgs
  If nArgs < minArgs Then Signal 40.003

  -- Now check that all required args are present

  Select Case BIFName -- Special cases
    When "ARG" Then if nArgs > 0 Then minArgs = 1
    Otherwise Nop
  End

  checkUpTo = minArgs - 1 + (nArgs > minArgs)
  If maxArgs == "*", BIFName \== "XRANGE" Then checkUpTo = nArgs
  Do argN = 1 To checkUpTo
    If arguments~arg(argN,"O") Then Signal 40.005
  End

  -- Now check statically specified arguments against their declared types
  Do argN = 1 To nArgs
    If arguments~arg(argN,"O") Then Iterate
    arg = arguments~arg(argN)
    Select Case arg~class
      When .Literal.String.Term Then value = arg~theString~value
      When .Symbol.Term         Then Do
        symbol   = arg~symbol
        If symbol \< .ALL.NUMBERS Then Iterate
        value = symbol~value
      End
      When .Prefix.Expression Then Do -- Allow negative numbers
        term = arg~term
        If \term~isA( .Symbol.Term ) Then Iterate
        symbol   = term~symbol
        If symbol \< .ALL.NUMBERS Then Iterate
        prefixes = arg~prefixes -- An array of element "+" and "-"
        minus = 0
        Do sign Over prefixes
          If sign~value == "-" Then minus = \minus
          -- TODO What to do if sign is "\"?
        End
        value = symbol~value
        digits = Digits()
        -- We may need many digits when the type is NUM
        Numeric Digits Length(value) + 2
        If minus Then value = -value
        Numeric Digits digits
      End
      Otherwise Iterate
    End

    -- (ooRexx r13531, RFE 890) BEEP: a whole number, and then its range
    -- (40.927). The message shows the number as an integer: "1E5" is
    -- "100000", and "12.0" is "12".
    If BIFName == "BEEP" Then Do
      If \DataType(value,"Internal") Then Signal 40.012
      If argN == 1 Then Parse Value 37 32767 With min max
      Else              Parse Value  0 60000 With min max
      If value < min | value > max Then Do
        Numeric Digits 18
        value = value % 1
        Signal 40.927
      End
      Iterate
    End

    -- Special cases
    Select Case BIFName
      When "MIN", "MAX" Then type = "NUM"
      When "D2C", "D2X" Then Do
        If argN == 1 Then
          If nArgs == 2 Then type = "D2WHOLE"
          Else               type = "D2WHOLE>=0"
        Else type  = BIFInfo~argType[argN]
      End
      When "XRANGE" Then type = "XRANGE"
      Otherwise type  = BIFInfo~argType[argN]
    End

    Select Case type
      When "ANY" Then Iterate
      When "ACEFILNOR" Then Do
        newValue = Strip(value,"L","?")
        If Length(newValue) > 0 Then Do
          letter = Left(newValue, 1)
          If Pos(Upper(letter), type) == 0 Then Signal 24.001
        End
      End
      When "F" Then Do
        letter = Strip(Left(value, 1))
        If Pos(Upper(letter), type) == 0 Then Do
          letter = value
          type = '"force", "Force", "f", "F"'
          Signal 40.904
        End
      End
      When "AENO", "ACDEIORS", "BCDJEMNOSUW", "BDEFILMNOSTUW", "BDEFINOSTU", "CEHLMNRS", "CEFHLMNORST", "CFHLMNOST", "CN", "CDEGOS", "DELNP", "SDC" Then Do
        letter = Left(value, 1)
        If Pos(Upper(letter), type) == 0 Then Signal BadOption
        -- TODO RXQUEUE and STREAM special casess
      End
      When "ABILMNOSUVWX9", "BLT", "MN" Then Do
        letter = Left(value, 1)
        If Pos(Upper(letter), type) == 0 Then Signal BadOption
      End
      When "WHOLE", "D2WHOLE", "D2WHOLE>=0" Then Do
        If BIFName == "RANDOM" Then Do
          If \DataType(value, "Whole") Then Signal 40.012
          Iterate
        End
        If \DataType(value,"Number") | Pos("E", value) > 0 | Pos(" ", value) > 0 | Pos(".", value) > 0 Then
          Select Case BIFName
            When "D2X"    Then           Signal 93.928
            When "D2C"    Then           Signal 93.929
          End
        -- For D2C and D2X, value is a number. See if it is also an integer
        -- We do not accept literal strings, exponents or decimals
        If BIFName \== "RANDOM", nArgs == 1, Left(value,1) = "-" Then Signal 93.927
      End
      When "LENGTH", "POS", "WHOLE", "WHOLE>0", "WHOLE>=0" Then Do
        If \DataType(value,"Internal") Then Signal 40.012
        Select Case Type
          When "LENGTH"   Then If value <  0 Then
            If WordPos(BIFName, "CHARIN") > 0 Then Nop -- See bug 2011
            Else                                   Signal 40.013
          When "POS"      Then If value <= 0 Then  Signal 40.014
          When "WHOLE>0"  Then Do
            If value <= 0 Then
              If WordPos(BIFName, "CHAROUT LINEOUT") > 0    Then Nop -- TODO That would be Signal 93.907, but arg numbers are shifted by two :/
              Else If WordPos(BIFName, "CHARIN LINEIN") > 0 Then Signal 93.907
              Else                                               Signal 40.014
          End
          When "WHOLE>=0" Then Do
            If value <  0 Then Do
              If BIFName == "RANDOM"                                   Then Signal 40.013
              If WordPos(BIFName, "CHANGESTR COPIES FORMAT INSERT TRUNC") > 0 Then Signal 40.013 -- See bugs 2010, 2012
            End
            Else If BIFName == "RANDOM", value > 999999999             Then Signal 40.013
          End
        End
      End
      When "NUM" Then Do
        If \DataType(value,"Number")   Then Signal 40.011
      End
      When "XRANGE" Then Do
        If Length(value) > 1, -
          WordPos(Upper(value),"ALNUM ALPHA BLANK CNTRL DIGIT GRAPH LOWER PRINT PUNCT SPACE UPPER XDIGIT") == 0 Then
          Signal 40.028
      End
      When "BIN" Then Do
        If \DataType(value,"Binary") Then Do
          -- Only 0,1 and whitespace
          pos = Verify(value,"09"X"01 ")
          If pos > 0 Then do
            char = value[pos]
            Signal 93.934
          End
          -- Groups of four
          pos = 1
          If Pos(value[pos],"01") == 0 Then Signal 93.932
          pos = Length(value)
          If Pos(value[pos],"01") == 0 Then Signal 93.932
          Do i = Words(value) To 2 By -1
            If Length(Word(value,i)) // 4 \== 0 Then  Signal 93.977
          End
        End
      End
      When "HEX" Then Do
        If \DataType(value,"X") Then Do -- Find out why
          -- Only 0-9, A-F1 and whitespace
          pos = Verify(value,"09"X" "XRange("AlNum") )
          If pos > 0 Then do
            char = value[pos]
            Signal 93.933
          End
          -- No whitespace at the extremes
          pos = 1
          If Pos(value[pos],"2009"X) > 0 Then Signal 93.931
          pos = Length(value)
          If Pos(value[pos],"2009"X) > 0 Then Signal 93.931
          -- Groups of two
          Do i = Words(value) To 2 By -1
            If Length(Word(value,i)) // 2 \== 0 Then  Signal 93.976
          End
        End
      End
      When "SEP" Then Do
        If Length(value) > 1 Then Signal 40.043
      End
      When "PAD" Then Do
        If Length(value) \== 1 Then Signal 40.023
      End
      When "0_99" Then Do
        If \DataType(value,"Internal") Then Signal 40.012
        If value < 0 | value > 99      Then Signal 40.903
      End
      -- ERRORTEXT since ooRexx r13437 (RFE 766): a whole number is an error
      -- number, 0-99, as before; anything else with a "." is a code nn.nnn,
      -- its two parts read as written (nn 0-99, nnn 0-999), or 40.926.
      When "ERRCODE" Then Do
        If \DataType(value,"W"), Pos(".", value) > 0 Then Do
          Parse Var value major "." minor
          If \DataType(major,"W") | \DataType(minor,"W") Then Signal 40.926
          If major < 0 | major > 99 | minor < 0 | minor > 999 Then Signal 40.926
        End
        Else Do
          If \DataType(value,"Internal") Then Signal 40.012
          If value < 0 | value > 99      Then Signal 40.903
        End
      End
      Otherwise Nop
    End
  End

Return

-- TRACE request letter must be one of "ACEFILNOR"; found "&1".
24.001: Syntax( 24.001, label, letter )

-- Not enough arguments in invocation of &1; minimum expected is &2.
40.003: Syntax( 40.003, label, BIFName, minArgs )

-- Too many arguments in invocation of &1; maximum expected is &2.
40.004: Syntax( 40.004, label, BIFName, maxArgs )

-- Missing argument in invocation of &1; argument &2 is required.
40.005: Syntax( 40.005, label, BIFName, argN )

-- &1 argument &2 must be a number; found "&3".
40.011: Syntax( 40.011, label, BIFName, argN, value )

-- &1 argument &2 must be a whole number; found "&3".
40.012: Syntax( 40.012, label, BIFName, argN, value )

-- &1 argument &2 must be zero or positive; found "&3".
40.013: Syntax( 40.013, label, BIFName, argN, value )

-- &1 argument &2 must be positive; found "&3".
40.014: Syntax( 40.014, label, BIFName, argN, value )

-- &1 argument &2 must be a single character; found "&3".
40.023: Syntax( 40.023, label, BIFName, argN, value )

-- &1 argument &2 must be a character class name or a single character; found "&3".
40.028: Syntax( 40.028, label, BIFName, argN, value )

-- &1 argument &2 must be a single non-alphanumeric character or the null string; found "&3".
40.043: Syntax( 40.043, label, BIFName, argN, value )

-- &1 argument &2 must be in the range 0-99; found "&3".
40.903: Syntax( 40.903, label, BIFName, argN, value )

-- &1 argument &2 must be an error code nn.nnn, with nn in the range 0-99
-- and nnn in the range 0-999; found "&3".
40.926: Syntax( 40.926, label, BIFName, argN, value )

-- An option that is not one of its letters. The first letter is the one
-- checked, but what 40.904 shows as "&4" depends on the function, as in
-- ooRexx: the first character for FILESPEC, and uppercased for DATE, TIME
-- and DATATYPE, or nothing when the value is empty (DATATYPE showed a NUL
-- for an empty value up to r13535, unreported; fixed in r13538); the whole
-- value for the others.
BadOption:
  first = Left(value, Min(1, Length(value)))
  Select Case BIFName
    When "DATE", "TIME", "DATATYPE" Then letter = Upper(first)
    When "FILESPEC"                 Then letter = first
    Otherwise                            letter = value
  End
  Signal 40.904

-- &1 argument &2 must be one of &3; found "&4".
40.904: Syntax( 40.904, label, BIFName, argN, type, letter )

-- &1 argument &2 must be in the range &3 to &4; found "&5".
40.927: Syntax( 40.927, label, BIFName, argN, min, max, value )

-- Method argument &1 must be a positive whole number; found "&2".
93.907: Syntax( 93.907, label, argN-1, value )

-- Length must be specified to convert a negative value.
93.927: Syntax( 93.927, label  )

-- D2X value must be a valid whole number; found "&1".
93.928: Syntax( 93.928, label, value  )

-- D2C value must be a valid whole number; found "&1".
93.929: Syntax( 93.929, label, value  )

-- Incorrect location of whitespace character in position &1 in hexadecimal string.
93.931: Syntax( 93.931, label, pos  )

-- Incorrect location of whitespace character in position &1 in binary string.
93.932: Syntax( 93.932, label, pos  )

-- Only 0-9, a-f, A-F, and whitespace characters are valid in a hexadecimal string; character found "&1".
93.933: Syntax( 93.933, label, char  )

-- Only 0, 1, and whitespace characters are valid in a binary string; character found "&1".
93.934: Syntax( 93.934, label, char  )

-- Hexadecimal strings must be grouped in units that are multiples of two characters.
93.976: Syntax( 93.976, label )

-- Binary strings must be grouped in units that are multiples of four characters.
93.977: Syntax( 93.977, label )
