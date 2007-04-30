/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* REXX Kernel                                                  ASCIIDBCSStrings.c     */
/*                                                                            */
/* REXX string ASCII DBCS string methods                                      */
/*                                                                            */
/******************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ASCIIDBCSStrings.hpp"
#include "RexxBuiltinFunctions.h"                     /* General purpose BIF Header file   */

extern INT  lookup[];
                                       /* current global settings           */
extern ACTIVATION_SETTINGS *current_settings;

INT DBCS_MemiCmp(
  PUCHAR    DBCS_Table,                /* DBCS validation table             */
  PUCHAR    String1,                   /* first memory location             */
  PUCHAR    String2,                   /* second memory location            */
  size_t    Length )                   /* length of comparison              */
/*********************************************************************/
/*  Function:  Case insensitive DBCS memory compare                  */
/*********************************************************************/
{
  INT       rc;                        /* compare value                     */

  rc = 0;                              /* default compare equal             */
  while ( Length ) {                   /* while length to compare           */
    if (IsDBCS(*String1)) {            /* first string is DBCS              */
      if (IsDBCS(*String2)) {          /* second DBCS                       */
                                       /* compare the two bytes             */
        rc = memcmp(String1, String2, DBCS_BYTELEN);
        if (rc)                        /* mismatch?                         */
          break;                       /* quit if not equal                 */
        String1 += DBCS_BYTELEN;       /* step the pointer                  */
        String2 += DBCS_BYTELEN;       /* step second pointer               */
        Length -= DBCS_BYTELEN;        /* reduce the length                 */
      }
      else {                           /* second is SBCS                    */
        rc = 1;                        /* first is larger                   */
        break;                         /* quit comparing                    */
      }
    }
    else {                             /* first is SBCS                     */
      if (IsDBCS(*String2)) {          /* second DBCS?                      */
        rc = -1;                       /* second is larger                  */
        break;                         /* quit comparing                    */
      }
      else {                           /* both SBCS                         */
                                       /* if characters aren't equal        */
        rc = (tolower(*String1++) - tolower(*String2++));
        if (rc)                        /* mismatch?                         */
          break;                       /* quit the loop                     */
        String1++;                     /* step the pointer                  */
        String2++;                     /* step second pointer               */
        Length--;                      /* reduce the length                 */
      }
    }
  }
  return rc;                           /* return compare result             */
}

UCHAR DBCS_Type(
  RexxString *String )                 /* Input String                      */
/*********************************************************************/
/* Function: Do DBCS string type validation                          */
/*********************************************************************/
{
  PUCHAR   CStr;                       /* current scan location             */
  PUCHAR   EndStr;                     /* end location                      */
  CHAR     rc;                         /* Function return code.             */
  BOOL     ChkFlag;                    /* Invalid string flag.              */
  size_t   StrLen;                     /* Input String length.              */
  size_t   DBCSNum;                    /* Number of DBCS bytes.             */

  CStr = STRPTR(String);               /* point to the string               */
  StrLen = STRLEN(String);             /* get the length                    */
  ChkFlag = FALSE;                     /* still valid                       */
  DBCSNum = 0;                         /* no DBCS bytes                     */
  EndStr = CStr + StrLen;              /* point to end of string            */

  while (CStr < EndStr) {              /* scan until invalid status         */
    if (IsDBCS(*CStr)) {               /* Current byte DBCS 1st?            */
      if (++CStr >= EndStr) {          /* but only byte left?               */
        ChkFlag = TRUE;                /* flag the error                    */
        break;                         /* and quit                          */
      }
      else {
        DBCSNum += 2;                  /* bump the DBCS count               */
      }
    }
    CStr++;                            /* step one more character           */
  }

  if (ChkFlag)                         /* not valid?                        */
    rc = INV_MIXED;                    /* String is invalid.                */
  else {                               /* String is valid                   */
    if (!DBCSNum)                      /* Pure SBCS?                        */
      rc = PURE_SBCS;
    else if (DBCSNum == StrLen)        /* Pure DBCS?                        */
      rc = PURE_DBCS;
    else
      rc = MIXED;                      /* DBCS & SBCS combined.             */
  }
  return  rc;                          /* Return the result.                */
}

size_t RexxString::validDBCS()
/*********************************************************************/
/* Function:         Validate a character string, returning length   */
/*                   in logical characters                           */
/*********************************************************************/
{
  PUCHAR   EndStr;                     /* end location                      */
  PUCHAR   String;                     /* string scan pointer               */
  size_t   Length;                     /* byte length of the string         */
  size_t   CharLength;                 /* Input HugeString length.          */
  CHAR     BadChar[4];                 /* working buffer for errors         */
  CHAR     HexBadChar[4];              /* working buffer for errors         */
  BOOL     HasDBCS;                    /* found a DBCS character            */

  if (NoDBCS(this))                    /* already validated?                */
    return this->length;               /* return quickly                    */

  String = STRPTR(this);               /* get the string pointer            */
  Length = STRLEN(this);               /* and the length                    */
  EndStr = String + Length;            /* point to end of string            */
  CharLength = 0;                      /* no characters yet                 */
  HasDBCS = FALSE;                     /* no DBCS yet                       */

  while (String < EndStr) {            /* scan until invalid status         */
    if (IsDBCS(*String)) {             /* Current byte DBCS 1st?            */
      if (++String >= EndStr) {        /* but only byte left?               */
                                       /* get as character data (and hex)   */
        sprintf(BadChar, "%c", *String);
        sprintf(HexBadChar, "%2X", *String);
                                       /* raise an error                    */
        report_exception2(Error_Invalid_character_char,
                          new_cstring(BadChar),
                          new_cstring(HexBadChar));
      }
      HasDBCS = TRUE;                  /* have DBCS on this                 */
    }
    String++;                          /* step one more byte                */
    CharLength++;                      /* count a character                 */
  }
  if (!HasDBCS)                        /* no DBCS characters found?         */
    this->Attributes |= STRING_NODBCS; /* flag as no DBCS possible          */
  return CharLength;                   /* return the length                 */
}

size_t DBCS_CharacterCount(
  PUCHAR   String,                     /* string scan pointer               */
  size_t   Length )                    /* string byte length                */
/*********************************************************************/
/* Function:         Counts logical characters in a section of a     */
/*                   string                                          */
/*********************************************************************/
{
  PUCHAR   EndStr;                     /* end location                      */
  size_t   CharLength;                 /* Input HugeString length.          */

  EndStr = String + Length;            /* point to end of string            */
  CharLength = 0;                      /* no characters yet                 */

  while (String < EndStr) {            /* scan until invalid status         */
    if (IsDBCS(*String))               /* Current byte DBCS 1st?            */
      String++;                        /* step on extra byte                */
    String++;                          /* step one more byte                */
    CharLength++;                      /* count a character                 */
  }
  return CharLength;                   /* return the length                 */
}

RexxString *RequiredArg(
  RexxString *ArgString,               /* validated argument string         */
  size_t     *Length,                  /* returned argument length          */
  size_t      Position )               /* position of the argument          */
/*********************************************************************/
/* Function:       : Validate a character string, returning length   */
/*                   in logical characters.                          */
/*********************************************************************/
{
                                       /* get the string value              */
  ArgString = get_string(ArgString, Position);
  *Length = ValidDBCS(ArgString);      /* now validate the string           */
  return ArgString;                    /* return the string value           */
}

RexxString *OptionalArg(
  RexxString *ArgString,               /* validated argument string         */
  RexxString *Default,                 /* default value                     */
  size_t     *Length,                  /* returned argument length          */
  size_t      Position)                /* argument position number          */
/*********************************************************************/
/* Function:         Validate a character string, returning length   */
/*                   in logical characters.                          */
/*********************************************************************/
{
  if (ArgString == OREF_NULL) {        /* no argument?                      */
    if (Default == OREF_NULL)
      *Length = 0l;
    else
      *Length = Default->length;
    return Default;                    /* give back the default string      */
  }                                    /* get the string value              */
  ArgString = optional_string(ArgString, Default, Position);
  *Length = ValidDBCS(ArgString);      /* now validate the string           */
  return ArgString;                    /* return the string value           */
}

PUCHAR ValidatePad(
  RexxString *PadString,               /* validated argument string         */
  const PUCHAR Default )               /* default padding character         */
/*********************************************************************/
/* Function:         Validate a a padding character, returning length*/
/*                   in logical characters.                          */
/*********************************************************************/
{
  if (PadString == OREF_NULL)          /* no pad given?                     */
    return (PUCHAR)Default;            /* just give back the default        */
                                       /* validate the string               */
  if (ValidDBCS(PadString) != 1)
    report_exception1(Error_Incorrect_method_pad, PadString);
  return STRPTR(PadString);            /* return pointer to pad             */
}

void DBCS_IncChar(
  PUCHAR   *String,                    /* Input string.                     */
  size_t   *Length,                    /* length of string                  */
  size_t   *CharLen )                  /* Extract length.                   */
/*********************************************************************/
/* Function:    Increment a string pointer by the indicated number   */
/*              of characters                                        */
/*********************************************************************/
{
  PUCHAR    CStr;                      /* current string location           */
  PUCHAR    EndStr;                    /* end of string                     */

  CStr = *String;                      /* point to the string               */
  EndStr = CStr + *Length;             /* point to the end of string        */

  while (*CharLen) {                   /* while more characters             */
    if (CStr >= EndStr)                /* reached the end                   */
      break;                           /* This failed                       */
                                       /* DBCS character?                   */
    if (IsDBCS(*CStr)) {
      CStr += DBCS_BYTELEN;            /* step two characters               */
    }
    else {
      CStr++;                          /* move one characters               */
    }
    (*CharLen)--;                      /* forget one character              */
  }
  *Length -= CStr - *String;           /* reduce the length                 */
  *String = CStr;                      /* set the new pointer               */
  return;                              /* finished                          */
}

void DBCS_IncByte(
  PUCHAR   *String,                    /* Input string.                     */
  size_t   *Length,                    /* length of string                  */
  size_t   *ByteLen )                  /* Extract length.                   */
/*********************************************************************/
/* Function:    Increment a string pointer by the indicated number   */
/*              of bytes.  Will not split a DBCS character           */
/*********************************************************************/
{
  PUCHAR    CStr;                      /* current string location           */
  PUCHAR    EndStr;                    /* end of string                     */

  CStr = *String;                      /* copy the pointer                  */
  EndStr = CStr + *Length;             /* point to the end of string        */

  while (*ByteLen) {                   /* while more characters             */
    if (CStr >= EndStr)                /* reached the end                   */
      break;                           /* This failed                       */

    if (IsDBCS(*CStr)) {               /* DBCS character?                   */
      if (*ByteLen == SBCS_BYTELEN)    /* only a single left?               */
        break;                         /* finished                          */
      CStr += DBCS_BYTELEN;            /* step two characters               */
      (*ByteLen)--;                    /* forget one more character         */
    }
    else {
      CStr++;                          /* move one characters               */
    }
    (*ByteLen)--;                      /* forget one character              */
  }
  *Length -= CStr - *String;           /* reduce the length                 */
  *String = CStr;                      /* set the new pointer               */
  return;                              /* finished                          */
}

void DBCS_SkipBlanks(
  PUCHAR   *String,                    /* point to advance                  */
  size_t   *StringLength )             /* string length                     */
/*********************************************************************/
/*   Function:          Skip leading SBCS/DBCS blanks and decrement  */
/*                      count.                                       */
/*********************************************************************/
{
  PUCHAR   Scan;                       /* scan pointer                      */
  size_t   Length;                     /* length to scan                    */

  Scan = *String;                      /* point to data                     */
  Length = *StringLength;              /* get the length                    */

  while (Length) {                     /* scan entire string                */
    if (*Scan == ch_SPACE) {           /* if SBCS blank                     */
      Length--;                        /* reduce the length by one          */
      Scan++;                          /* step the pointer one              */
    }
    else if (IsDBCSBlank(Scan)) {      /* DBCS blank?                       */
      Length -= DBCS_BYTELEN;          /* reduce length two                 */
      Scan += DBCS_BYTELEN;            /* step pointer also                 */
    }
    else                               /* found a non-blank                 */
      break;                           /* just quit the loop                */
  }
                                       /* fell through, all blanks          */
  *String = Scan;                      /* set pointer one past              */
  *StringLength = Length;              /* update the length                 */
}

void DBCS_SkipNonBlanks(
  PUCHAR   *String,                    /* point to advance                  */
  size_t   *StringLength )             /* string length                     */
/*********************************************************************/
/*   Function:          Skip SBCS/DBCS non-blanks and decrement size.*/
/*********************************************************************/
{
  PUCHAR   Scan;                       /* scan pointer                      */
  size_t   Length;                     /* length to scan                    */

  Scan = *String;                      /* point to data                     */
  Length = *StringLength;              /* get the length                    */

  while (Length) {                     /* scan entire string                */
    if (*Scan == ch_SPACE ||           /* either SBCS or                    */
        IsDBCSBlank(Scan))             /* DBCS blank?                       */
      break;                           /* done                              */
    else if (IsDBCS(*Scan)) {          /* DBCS character?                   */
      Length -= DBCS_BYTELEN;          /* reduce length two                 */
      Scan += DBCS_BYTELEN;            /* step pointer also                 */
    }
    else {                             /* if SBCS blank                     */
      Length--;                        /* reduce the length by one          */
      Scan++;                          /* step the pointer one              */
    }
  }
                                       /* fell through, all blanks          */
  *String = Scan;                      /* set pointer one past              */
  *StringLength = Length;              /* update the length                 */
}

void DBCS_StripBlanks(
  PUCHAR    *String,                   /* point to advance                  */
  size_t    *StringLength )            /* string length                     */
/*********************************************************************/
/*   Function:          Adjust string length, removing blanks        */
/*********************************************************************/
{
  size_t   Count;                      /* size to scan                      */
  PUCHAR   BlankStr;                   /* start of last blank part          */
  PUCHAR   Scan;                       /* scan pointer                      */

  BlankStr = NULL;                     /* null the pointer                  */
  Scan = *String;                      /* copy the pointer                  */
                                       /* loop through entire string        */
  for (Count = *StringLength; Count; ) {

    if (IsDBCSBlank(Scan)) {           /* DBCS blank?                       */
      if (!BlankStr)                   /* first blank?                      */
        BlankStr = Scan;               /* mark spot                         */
      Scan += DBCS_BYTELEN;            /* Increment by DBCS length          */
      Count -= DBCS_BYTELEN;           /* decrement count                   */
    }
    else if (*Scan == SBCS_BLANK) {    /* SBCS blank?                       */
      if (!BlankStr)                   /* first blank?                      */
        BlankStr = Scan;               /* mark spot                         */
      Scan++;                          /* Increment by SBCS length          */
      Count--;                         /* and decrement the count           */
    }
    else if (IsDBCS(*Scan)) {          /* DBCS character?                   */
      BlankStr = NULL;                 /* null the pointer                  */
      Scan += DBCS_BYTELEN;            /* Increment by DBCS length          */
      Count -= DBCS_BYTELEN;           /* decrement count                   */
    }
    else {                             /* is SBCS character                 */
      BlankStr = NULL;                 /* null the pointer                  */
      Scan++;                          /* Increment by SBCS length          */
      Count--;                         /* and decrement the count           */
    }
 }
  if (BlankStr)                        /* trailing blanks?                  */
    Scan = BlankStr;                   /* adjust the pointer                */
                                       /* return the new length             */
  *StringLength = (Scan - *String);
}

INT  DBCS_CaselessCompare(
  PUCHAR    Str1,                      /* String to be compared.            */
  PUCHAR    Str2,                      /* String to be compared.            */
  size_t    Length )                   /* String2 length                    */
/*********************************************************************/
/* Function:    Compare the two strings, ignoring the case.          */
/*********************************************************************/
{
  INT       rc;                        /* Return code for compare           */
  size_t    Cnt1;                      /* Str1 compare length               */
  size_t    Cnt2;                      /* Str2 compare length               */

  rc = COMP_EQUAL;                     /* Comparison default.               */

                                       /* while still characters            */
                                       /* left and they still compare       */
  while (Length && rc == COMP_EQUAL) {
    if (IsDBCS(*Str1))                 /* if DBCS, then                     */
      Cnt1 = DBCS_BYTELEN;             /* compare two bytes                 */
    else
      Cnt1 = SBCS_BYTELEN;             /* else just a single byte           */
    if (IsDBCS(*Str2))                 /* if DBCS, then                     */
      Cnt2 = DBCS_BYTELEN;             /* compare two bytes                 */
    else
      Cnt2 = SBCS_BYTELEN;             /* else just a single byte           */
    if (Cnt1 == Cnt2) {                /* equal length?                     */
      if (Cnt1 == DBCS_BYTELEN) {      /* Comparing DBCS characters?        */
        rc = *Str1++ == *Str2++;       /* do the comparision                */
        if (rc == 0)                   /* compared equal?                   */
          rc = *Str1++ == *Str2++;     /* compare the second bytes          */
      }
      else                             /* compare the uppercase versions    */
        rc = toupper(*Str1++) == toupper(*Str2++);
      Length -= Cnt1;                  /* reduce the length                 */
    }
    else if (Cnt1 == DBCS_BYTELEN)     /* 1st DBCS and 2nd SBCS?            */
      rc = COMP_GREATER;               /* the first is greater              */
    else                               /* 1st SBCS and 2nd DBCS             */
      rc = COMP_LESS;                  /* 2nd is the larger                 */
  }
  return  rc;                          /* Return the result.                */
}

INT  DBCS_CharCompare(
  PUCHAR    Str1,                      /* String to be compared.            */
  size_t    Len1,                      /* String1 length                    */
  PUCHAR    Str2,                      /* String to be compared.            */
  size_t    Len2,                      /* String2 length                    */
  PUCHAR    Pad,                       /* Padding character.                */
  size_t   *Diff )                     /* Unmatched position, 0 origin      */
/*********************************************************************/
/* Function:    Compare the two strings and return the result and the*/
/*              unmatched position.                                  */
/*********************************************************************/
{
  INT       rc;                        /* Return code for compare           */
  size_t    PadSize;                   /* pad character length              */
  size_t    Cnt1;                      /* Str1 compare length               */
  size_t    Cnt2;                      /* Str2 compare length               */
  PUCHAR    Comp1;                     /* compare position 1                */
  PUCHAR    Comp2;                     /* compare position 2                */

  *Diff = 0L;                          /* Different position                */
  rc = COMP_EQUAL;                     /* Comparison default.               */

  if (!Pad)                            /* if no pad, length is 0            */
    PadSize = 0;
  else
    PadSize = strlen((PCHAR)Pad);      /* get pad size                      */
                                       /* while still characters            */
                                       /* left and they still compare       */
  while (((Len1) || (Len2)) && (rc == COMP_EQUAL)) {
    if (Len1) {                        /* anything of str1 left?            */
      Comp1 = Str1;                    /* compare at this point             */

      if (IsDBCS(*Str1)) {             /* if DBCS, then                     */
        Cnt1 = DBCS_BYTELEN;           /* compare two bytes                 */
        Str1 += DBCS_BYTELEN;          /* step the pointer                  */
        Len1 -= DBCS_BYTELEN;          /* reduce the length                 */
      }
      else {
        Cnt1 = SBCS_BYTELEN;           /* else just a single byte           */
        Str1++;                        /* step the pointer                  */
        Len1--;                        /* reduce the length                 */
      }
    }
    else {                             /* Str1 is used up                   */
      if (!PadSize)                    /* No padding?                       */
        break;                         /* finished                          */
      Cnt1 = PadSize;                  /* use the pad length                */
      Comp1 = Pad;                     /* set compare location              */
    }
    if (Len2) {                        /* now repeat for Str2               */
      Comp2 = Str2;                    /* set compare location              */
      if (IsDBCS(*Str2)) {             /* if DBCS, then                     */
        Cnt2 = DBCS_BYTELEN;           /* compare two bytes                 */
        Str2 += DBCS_BYTELEN;          /* step the pointer                  */
        Len2 -= DBCS_BYTELEN;          /* reduce the length                 */
      }
      else {
        Cnt2 = SBCS_BYTELEN;           /* else just a single byte           */
        Str2++;                        /* step the pointer                  */
        Len2--;                        /* reduce the length                 */
      }
    }
    else {
      if (!PadSize)                    /* No padding?                       */
        break;                         /* finished                          */
      Cnt2 = PadSize;                  /* use the pad length                */
      Comp2 = Pad;                     /* set compare location              */
    }
    if (Cnt1 == Cnt2) {                /* equal length?                     */
      if (*Comp1 == *Comp2) {          /* both equal?                       */
        if (Cnt1 == DBCS_BYTELEN) {    /* might be DBCS                     */
          if (*(++Comp1) > *(++Comp2)) /* compare second bytes              */
            rc = COMP_GREATER;
          else if (*Comp1 < *Comp2)    /* might be less                     */
            rc = COMP_LESS;
          else                         /* both bytes equal                  */
            ++(*Diff);                 /* bump char position                */
        }
        else                           /* equal SBCS character              */
          ++(*Diff);                   /* bump char position                */
      }
      else if (*Comp1 > *Comp2)        /* else set the proper return        */
        rc = COMP_GREATER;
      else
        rc = COMP_LESS;
    }
    else if (Cnt1 == DBCS_BYTELEN)     /* 1st DBCS and 2nd SBCS?            */
      rc = COMP_GREATER;               /* the first is greater              */
    else                               /* 1st SBCS and 2nd DBCS             */
      rc = COMP_LESS;                  /* 2nd is the larger                 */
  }
  if (rc == COMP_EQUAL)                /* still equal?                      */
    *Diff = 0;                         /* no difference point               */
  else
    ++(*Diff);                         /* now adjust differnce point        */
  return  rc;                          /* Return the result.                */
}

PUCHAR DBCS_StrStr(
  PUCHAR    Haystack,                  /* haystack pointer                  */
  size_t    HaystackLen,               /* length of haystack                */
  PUCHAR    Needle,                    /* needle pointer                    */
  size_t    NeedleLen )                /* lengthy of needle                 */
/*********************************************************************/
/*  Function:                   A pointer to the beginning           */
/*                              of the first occurrence of needle    */
/*                              as a substring of haystack, if any;  */
/*                              otherwise, a null pointer is         */
/*                              returned                             */
/*********************************************************************/
{
  PUCHAR   End;                        /* end of string                     */
  PUCHAR   Retval;                     /* return value                      */

  Retval = NULL;                       /* set default return value          */
  if (NeedleLen <= HaystackLen) {      /* short enough to search?           */
                                       /* point to end position             */
    End = Haystack + (HaystackLen - NeedleLen);
    while (Haystack < End) {           /* search for the string             */
                                       /* check this position               */
      if (!memcmp(Haystack, Needle, NeedleLen)) {
        Retval = Haystack;             /* copy the pointer                  */
        break;                         /* stop looping                      */
      }
      if (IsDBCS(*Haystack))
        Haystack += DBCS_BYTELEN;      /* step appropriate length           */
      else
        Haystack++;                    /* No, step our search pointer       */
    }
  }
  return Retval;                       /* return match position             */
}

size_t DBCS_ByteLen(
  PUCHAR    String,                    /* Input string.                     */
  size_t    Length,                    /* string length                     */
  size_t    CharLen )                  /* Extract length.                   */
/*********************************************************************/
/* Function:    Return the byte length for a set of characters       */
/*********************************************************************/
{
  PUCHAR    Temp;                      /* temporary pointer                 */

  Temp = String;                       /* copy the pointer                  */
                                       /* step forward chars                */
  DBCS_IncChar(&Temp, &Length, &CharLen);
  return (Temp - String);              /* return the difference             */
}

/*********************************************************************/
/*                                                                   */
/* Function: DBCS_MemChar                                            */
/*                                                                   */
/* Desctiption: Search a string for a character                      */
/*                                                                   */
/*********************************************************************/

size_t  DBCS_MemChar(
  PUCHAR    ch,                        /* search character                  */
  PUCHAR    String,                    /* searched string                   */
  size_t    Length )                   /* size to search                    */
{
  PUCHAR    EndStr;                    /* search end position               */
  size_t    Position;                  /* located position                  */
  size_t    Count;                     /* character count                   */

  EndStr = String + Length;            /* step to the end                   */
  Position = 0;                        /* set default position              */
  Count = 0;                           /* set initial character count       */
  if (IsDBCS(*ch)) {                   /* looking for a DBCS char?          */
    while (String < EndStr) {          /* search entire string              */
      Count++;                         /* count the character               */
      if (IsDBCS(*String)) {           /* DBCS character?                   */
                                       /* one we want?                      */
        if (!memcmp(String, ch, DBCS_BYTELEN)) {
          Position = Count;            /* set position                      */
          break;                       /* get out                           */
        }
        String += DBCS_BYTELEN;        /* step past character               */
      }
      else
        String++;                      /* step a single character           */
    }
  }
  else {
    while (String < EndStr) {          /* search entire string              */
      Count++;                         /* count the character               */
      if (!IsDBCS(*String)) {          /* DBCS character?                   */
        if (*String == *ch) {          /* one we want?                      */
          Position = Count;            /* set position                      */
          break;                       /* get out                           */
        }
        String++;                      /* step past character               */
      }
      else
        String += DBCS_BYTELEN;        /* step a DBCS character             */
    }
  }
  return Position;                     /* return located position           */
}

/*********************************************************************/
/* Description: Return the number of words of input string.          */
/*********************************************************************/

size_t  DBCS_WordLen(
  PUCHAR    String,                    /* Input string.                     */
  size_t    Length )                   /* input length                      */
{
  size_t   Count;                      /* Current count of words            */

  if (!Length)                         /* Nothing there?                    */
    return 0;                          /* ... just return 0                 */
  Count = 0;                           /* start with no words               */
                                       /* skip any leading blanks           */
  DBCS_SkipBlanks(&String, &Length);

  while (Length) {                     /* while still string ...            */
    Count++;                           /* account for this word             */
                                       /* now skip the non-blanks           */
    DBCS_SkipNonBlanks(&String, &Length);
                                       /* skip any trailing blanks          */
    DBCS_SkipBlanks(&String, &Length);
  }
  return Count;                        /* return word count                 */
}

/*********************************************************************/
/*   Function:          searches for the next word in a string from  */
/*                      the current position (the pointer is         */
/*                      unchanged if already at a nonblank           */
/*                      character).  The return value is the updated */
/*                      pointer and the length of the word.          */
/*                      Returns a length of zero if no word was      */
/*                      found.                                       */
/*********************************************************************/

size_t  DBCS_NextWord(
  PUCHAR    *String,                   /* input string                      */
  size_t    *StringLength,             /* string length                     */
  PUCHAR    *NextString )              /* next word position                */
{
  size_t     WordStart;                /* Starting point of word            */

  WordStart = 0;                       /* nothing moved yet                 */
  if (*StringLength) {                 /* Something there?                  */
                                       /* skip any leading blanks           */
    DBCS_SkipBlanks(String, StringLength);

    if (*StringLength) {               /* if still string ...               */
      WordStart = *StringLength;       /* save current length               */
      *NextString = *String;           /* save start position now           */
                                       /* skip the non-blanks               */
      DBCS_SkipNonBlanks(NextString, StringLength);
      WordStart -= *StringLength;      /* adjust the word length            */
    }
  }
  return WordStart;                    /* return word length                */
}

/*********************************************************************/
/* Description: Do string upper of mixed string.                     */
/*********************************************************************/

void DBCS_MemUpper(
  PUCHAR     String,                   /* input string                      */
  size_t     Length )                  /* string length                     */
{
  while (Length) {                     /* While string                      */
    if (IsDBCS(*String)) {             /* is this DBCS?                     */
      Length -= DBCS_BYTELEN;          /* YES,skip DBCS2nd byte             */
      String += DBCS_BYTELEN;
    }                                  /* move pointer to next.             */
    else {                             /* NO. it is SBCS                    */
      *String = toupper(*String);      /* convert to uppercase              */
      Length -= SBCS_BYTELEN;          /* increment counter                 */
      String++;                        /* step past the byte                */
    }
  }
}

/*********************************************************************/
/* Description: Do string lower of mixed string.                     */
/*********************************************************************/

void DBCS_MemLower(
  PUCHAR     String,                   /* input string                      */
  size_t     Length )                  /* string length                     */
{
  while (Length) {                     /* While string                      */
    if (IsDBCS(*String)) {             /* is this DBCS?                     */
      Length -= DBCS_BYTELEN;          /* YES,skip DBCS2nd byte             */
      String += DBCS_BYTELEN;
    }                                  /* move pointer to next.             */
    else {                             /* NO. it is SBCS                    */
      *String = tolower(*String);      /* convert to uppercase              */
      Length -= SBCS_BYTELEN;          /* increment counter                 */
      String++;                        /* step past the byte                */
    }
  }
}

/*********************************************************************/
/* Description: Set a specified number of pad characters at the      */
/*              indicated location.                                  */
/*********************************************************************/

void DBCS_SetPadChar(
  PUCHAR    String,                    /* Input string.                     */
  size_t    NumPad,                    /* Number of pad characters          */
  PUCHAR    PadChar )                  /* Pad character                     */
{
  size_t    i;                         /* loop counter                      */
  size_t    PadSize;                   /* size of padding character         */

  PadSize = strlen((PCHAR)PadChar);    /* get the pad size                  */
  if (PadSize == SBCS_BYTELEN)         /* if padding with singles           */
                                       /* add space characters              */
    memset(String, *PadChar, NumPad);
  else {                               /* DBCS padding                      */
    for (i = 0; i < NumPad; i++) {     /* fill in the gap                   */
      *String++ = *PadChar;            /* first one character               */
      *String++ = *(PadChar + 1);      /* then the second byte              */
    }
  }
}

/*$$N  Do not format these tables when using CPRETTY                 */
/*********************************************************************/
/* The followings are the table for converting between DBCS and SBCS.*/
/*********************************************************************/
/* JAPAN TABLE                                                       */
/*                                                                   */
/* Note: C_INV  For invalid code.                                    */
/*       C_UNDF For undefined code.                                  */
/*       C_DBCS For DBCS code.                                       */
/*                                                                   */
/*********************************************************************/

static USHORT dbcscodeJ[256]={
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*00-07*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*08-0f*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*10-17*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*18-1f*/
0x4081,0x4981, 0x57FA, 0x9481, 0x9081, 0x9381, 0x9581, 0x56FA,/*20-27*/
0x6981,0x6A81, 0x9681, 0x7B81, 0x4381, 0x7C81, 0x4481, 0x5E81,/*28-2f*/
0x4F82,0x5082, 0x5182, 0x5282, 0x5382, 0x5482, 0x5582, 0x5682,/*30-37*/
0x5782,0x5882, 0x4681, 0x4781, 0x8381, 0x8181, 0x8481, 0x4881,/*38-3f*/
0x9781,0x6082, 0x6182, 0x6282, 0x6382, 0x6482, 0x6582, 0x6682,/*40-47*/
0x6782,0x6882, 0x6982, 0x6A82, 0x6B82, 0x6C82, 0x6D82, 0x6E82,/*48-4f*/
0x6F82,0x7082, 0x7182, 0x7282, 0x7382, 0x7482, 0x7582, 0x7682,/*50-57*/
0x7782,0x7882, 0x7982, 0x6D81, 0x8F81, 0x6E81, 0x4F81, 0x5181,/*58-5f*/
0x4D81,0x8182, 0x8282, 0x8382, 0x8482, 0x8582, 0x8682, 0x8782,/*60-67*/
0x8882,0x8982, 0x8A82, 0x8B82, 0x8C82, 0x8D82, 0x8E82, 0x8F82,/*68-6f*/
0x9082,0x9182, 0x9282, 0x9382, 0x9482, 0x9582, 0x9682, 0x9782,/*70-77*/
0x9882,0x9982, 0x9A82, 0x6F81, 0x6281, 0x7081, 0x5081, C_UNDF,/*78-7f*/
C_UNDF,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*80-87*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*88-8f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*90-97*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*98-9f*/
C_UNDF,0x4281, 0x7581, 0x7681, 0x4181, 0x4581, 0x9283, 0x4083,/*a0-a7*/
0x4283,0x4483, 0x4683, 0x4883, 0x8383, 0x8583, 0x8783, 0x6283,/*a8-af*/
0x5B81,0x4183, 0x4383, 0x4583, 0x4783, 0x4983, 0x4A83, 0x4C83,/*b0-b7*/
0x4E83,0x5083, 0x5283, 0x5483, 0x5683, 0x5883, 0x5A83, 0x5C83,/*b8-bf*/
0x5E83,0x6083, 0x6383, 0x6583, 0x6783, 0x6983, 0x6A83, 0x6B83,/*c0-c7*/
0x6C83,0x6D83, 0x6E83, 0x7183, 0x7483, 0x7783, 0x7A83, 0x7D83,/*c8-cf*/
0x7E83,0x8083, 0x8183, 0x8283, 0x8483, 0x8683, 0x8883, 0x8983,/*d0-d7*/
0x8A83,0x8B83, 0x8C83, 0x8D83, 0x8F83, 0x9383, 0x4A81, 0x4B81,/*d8-df*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e0-e7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e8-ef*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*f0-f7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_UNDF, C_UNDF, C_UNDF /*f8-ff*/
};

/*********************************************************************/
/* KOREA TABLE                                                       */
/*                                                                   */
/* Note: C_INV  For invalid code.                                    */
/*       C_UNDF For undefined code.                                  */
/*       C_DBCS For DBCS code.                                       */
/*                                                                   */
/*********************************************************************/

static USHORT dbcscodeK[256]={
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*00-07*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*08-0f*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*10-17*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*18-1f*/
0x4081,0x9F81, 0xA081, 0xA181, 0xA281, 0xA381, 0xA481, 0xA581,/*20-27*/
0xA681,0xA781, 0xA881, 0xA981, 0xAA81, 0xAB81, 0xAC81, 0xAD81,/*28-2f*/
0xAE81,0xAF81, 0xB081, 0xB181, 0xB281, 0xB381, 0xB481, 0xB581,/*30-37*/
0xB681,0xB781, 0xB881, 0xB981, 0xBA81, 0xBB81, 0xBC81, 0xBD81,/*38-3f*/
0xBE81,0xBF81, 0xC081, 0xC181, 0xC281, 0xC381, 0xC481, 0xC581,/*40-47*/
0xC681,0xC781, 0xC881, 0xC981, 0xCA81, 0xCB81, 0xCC81, 0xCD81,/*48-4f*/
0xCE81,0xCF81, 0xD081, 0xD181, 0xD281, 0xD381, 0xD481, 0xD581,/*50-57*/
0xD681,0xD781, 0xD881, 0xD981, 0xDA81, 0xDB81, 0xDC81, 0xDD81,/*58-5f*/
0xDE81,0xDF81, 0xE081, 0xE181, 0xE281, 0xE381, 0xE481, 0xE581,/*60-67*/
0xE681,0xE781, 0xE881, 0xE981, 0xEA81, 0xEB81, 0xEC81, 0xED81,/*68-6f*/
0xEE81,0xEF81, 0xF081, 0xF181, 0xF281, 0xF381, 0xF481, 0xF581,/*70-77*/
0xF681,0xF781, 0xF881, 0xF981, 0xFA81, 0xFB81, 0xFC81, C_UNDF,/*78-7f*/
C_UNDF,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*80-87*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*88-8f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*90-97*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*98-9f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a0-a7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a8-af*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b0-b7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b8-bf*/
0x5F82,0x6082, 0x6182, 0x6282, 0x6382, 0x6482, 0x6582, 0x6682,/*c0-c7*/
0x6782,0x6882, 0x6982, 0x6A82, 0x6B82, 0x6C82, 0x6D82, 0x6E82,/*c8-cf*/
0x6F82,0x7082, 0x7182, 0x7282, 0x7382, 0x7482, 0x7582, 0x7682,/*d0-d7*/
0x7782,0x7882, 0x7982, 0x7A82, 0x7B82, 0x7C82, 0x7D82, C_UNDF,/*d8-df*/
C_UNDF,C_UNDF, 0x8282, 0x8382, 0x8482, 0x8582, 0x8682, 0x8782,/*e0-e7*/
C_UNDF,C_UNDF, 0x8A82, 0x8B82, 0x8C82, 0x8D82, 0x8E82, 0x8F82,/*e8-ef*/
C_UNDF,C_UNDF, 0x9282, 0x9382, 0x9482, 0x9582, 0x9682, 0x9782,/*f0-f7*/
C_UNDF,C_UNDF, 0x9A82, 0x9B82, 0x9C82, C_UNDF, C_UNDF, C_UNDF /*f8-ff*/
};

/*********************************************************************/
/* PEOPLE REPUBLIC OF CHINA TABLE                                    */
/*                                                                   */
/* Note: C_INV  For invalid code.                                    */
/*       C_UNDF For undefined code.                                  */
/*       C_DBCS For DBCS code.                                       */
/*                                                                   */
/*********************************************************************/

static USHORT dbcscodeP[256]={
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*00-07*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*08-0f*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*10-17*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*18-1f*/
0x4081,0x4082, 0x4182, 0x4282, 0x8781, 0x4482, 0x4582, 0x52FA,/*20-27*/
0x4782,0x4882, 0x4982, 0x4A82, 0x4B82, 0x4C82, 0x4D82, 0x4E82,/*28-2f*/
0x4F82,0x5082, 0x5182, 0x5282, 0x5382, 0x5482, 0x5582, 0x5682,/*30-37*/
0x5782,0x5882, 0x5982, 0x5A82, 0x5B82, 0x5C82, 0x5D82, 0x5E82,/*38-3f*/
0x5F82,0x6082, 0x6182, 0x6282, 0x6382, 0x6482, 0x6582, 0x6682,/*40-47*/
0x6782,0x6882, 0x6982, 0x6A82, 0x6B82, 0x6C82, 0x6D82, 0x6E82,/*48-4f*/
0x6F82,0x7082, 0x7182, 0x7282, 0x7382, 0x7482, 0x7582, 0x7682,/*50-57*/
0x7782,0x7882, 0x7982, 0x7A82, 0x4382, 0x7C82, 0x7D82, 0x7E82,/*58-5f*/
0x8082,0x8182, 0x8282, 0x8382, 0x8482, 0x8582, 0x8682, 0x8782,/*60-67*/
0x8882,0x8982, 0x8A82, 0x8B82, 0x8C82, 0x8D82, 0x8E82, 0x8F82,/*68-6f*/
0x9082,0x9182, 0x9282, 0x9382, 0x9482, 0x9582, 0x9682, 0x9782,/*70-77*/
0x9882,0x9982, 0x9A82, 0x9B82, 0x9C82, 0x9D82, 0x9E82, C_UNDF,/*78-7f*/
C_UNDF,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*80-87*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*88-8f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*90-97*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*98-9f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a0-a7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a8-af*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b0-b7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b8-bf*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*c0-c7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*c8-cf*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*d0-d7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*d8-df*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e0-e7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e8-ef*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*f0-f7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_UNDF, C_UNDF, C_UNDF /*f8-ff*/
};

/*********************************************************************/
/* TAIWAN TABLE                                                      */
/*                                                                   */
/* Note: C_INV  For invalid code.                                    */
/*       C_UNDF For undefined code.                                  */
/*       C_DBCS For DBCS code.                                       */
/*                                                                   */
/*********************************************************************/

static USHORT dbcscodeR[256]={
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*00-07*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*08-0f*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*10-17*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*18-1f*/
0x4081,0x4981, 0x5E8B, 0x9481, 0x9081, 0x9381, 0x9581, 0x5D8B,/*20-27*/
0x6981,0x6A81, 0x9681, 0x7B81, 0x4381, 0x7C81, 0x4481, 0x5E81,/*28-2f*/
0x4F82,0x5082, 0x5182, 0x5282, 0x5382, 0x5482, 0x5582, 0x5682,/*30-37*/
0x5782,0x5882, 0x4681, 0x4781, 0x8381, 0x8181, 0x8481, 0x4881,/*38-3f*/
0x9781,0x6082, 0x6182, 0x6282, 0x6382, 0x6482, 0x6582, 0x6682,/*40-47*/
0x6782,0x6882, 0x6982, 0x6A82, 0x6B82, 0x6C82, 0x6D82, 0x6E82,/*48-4f*/
0x6F82,0x7082, 0x7182, 0x7282, 0x7382, 0x7482, 0x7582, 0x7682,/*50-57*/
0x7782,0x7882, 0x7982, 0x6D81, 0x8F81, 0x6E81, 0x4F81, 0x5181,/*58-5f*/
0x4D81,0x8182, 0x8282, 0x8382, 0x8482, 0x8582, 0x8682, 0x8782,/*60-67*/
0x8882,0x8982, 0x8A82, 0x8B82, 0x8C82, 0x8D82, 0x8E82, 0x8F82,/*68-6f*/
0x9082,0x9182, 0x9282, 0x9382, 0x9482, 0x9582, 0x9682, 0x9782,/*70-77*/
0x9882,0x9982, 0x9A82, 0x6F81, 0x6281, 0x7081, 0x5081, C_UNDF,/*78-7f*/
C_UNDF,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*80-87*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*88-8f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*90-97*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*98-9f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a0-a7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a8-af*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b0-b7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b8-bf*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*c0-c7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*c8-cf*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*d0-d7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*d8-df*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e0-e7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e8-ef*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*f0-f7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_UNDF, C_UNDF, C_UNDF /*f8-ff*/
};

/*********************************************************************/
/* JAPAN TABLE      CodePage942                                      */
/*                                                                   */
/* Note: C_INV  For invalid code.                                    */
/*       C_UNDF For undefined code.                                  */
/*       C_DBCS For DBCS code.                                       */
/*                                                                   */
/*********************************************************************/

static USHORT dbcscodeJ942[256]={
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*00-07*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*08-0f*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*10-17*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*18-1f*/
0x4081,0x4981, 0x57FA, 0x9481, 0x9081, 0x9381, 0x9581, 0x56FA,/*20-27*/
0x6981,0x6A81, 0x9681, 0x7B81, 0x4381, 0x7C81, 0x4481, 0x5E81,/*28-2f*/
0x4F82,0x5082, 0x5182, 0x5282, 0x5382, 0x5482, 0x5582, 0x5682,/*30-37*/
0x5782,0x5882, 0x4681, 0x4781, 0x8381, 0x8181, 0x8481, 0x4881,/*38-3f*/
0x9781,0x6082, 0x6182, 0x6282, 0x6382, 0x6482, 0x6582, 0x6682,/*40-47*/
0x6782,0x6882, 0x6982, 0x6A82, 0x6B82, 0x6C82, 0x6D82, 0x6E82,/*48-4f*/
0x6F82,0x7082, 0x7182, 0x7282, 0x7382, 0x7482, 0x7582, 0x7682,/*50-57*/
0x7782,0x7882, 0x7982, 0x6D81, 0x8F81, 0x6E81, 0x4F81, 0x5181,/*58-5f*/
0x4D81,0x8182, 0x8282, 0x8382, 0x8482, 0x8582, 0x8682, 0x8782,/*60-67*/
0x8882,0x8982, 0x8A82, 0x8B82, 0x8C82, 0x8D82, 0x8E82, 0x8F82,/*68-6f*/
0x9082,0x9182, 0x9282, 0x9382, 0x9482, 0x9582, 0x9682, 0x9782,/*70-77*/
0x9882,0x9982, 0x9A82, 0x6F81, 0x6281, 0x7081, 0x5081, C_UNDF,/*78-7f*/
0x9181,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*80-87*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*88-8f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*90-97*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*98-9f*/
0x9281,0x4281, 0x7581, 0x7681, 0x4181, 0x4581, 0x9283, 0x4083,/*a0-a7*/
0x4283,0x4483, 0x4683, 0x4883, 0x8383, 0x8583, 0x8783, 0x6283,/*a8-af*/
0x5B81,0x4183, 0x4383, 0x4583, 0x4783, 0x4983, 0x4A83, 0x4C83,/*b0-b7*/
0x4E83,0x5083, 0x5283, 0x5483, 0x5683, 0x5883, 0x5A83, 0x5C83,/*b8-bf*/
0x5E83,0x6083, 0x6383, 0x6583, 0x6783, 0x6983, 0x6A83, 0x6B83,/*c0-c7*/
0x6C83,0x6D83, 0x6E83, 0x7183, 0x7483, 0x7783, 0x7A83, 0x7D83,/*c8-cf*/
0x7E83,0x8083, 0x8183, 0x8283, 0x8483, 0x8683, 0x8883, 0x8983,/*d0-d7*/
0x8A83,0x8B83, 0x8C83, 0x8D83, 0x8F83, 0x9383, 0x4A81, 0x4B81,/*d8-df*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e0-e7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e8-ef*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*f0-f7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, 0x54FA, 0x5F81, 0x6081 /*f8-ff*/
};

/*********************************************************************/
/* KOREA TABLE      CodePage944                                      */
/*                                                                   */
/* Note: C_INV  For invalid code.                                    */
/*       C_UNDF For undefined code.                                  */
/*       C_DBCS For DBCS code.                                       */
/*                                                                   */
/*********************************************************************/

static USHORT dbcscodeK944[256]={
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*00-07*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*08-0f*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*10-17*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*18-1f*/
0x4081,0x9F81, 0xA081, 0xA181, 0xA281, 0xA381, 0xA481, 0xA581,/*20-27*/
0xA681,0xA781, 0xA881, 0xA981, 0xAA81, 0xAB81, 0xAC81, 0xAD81,/*28-2f*/
0xAE81,0xAF81, 0xB081, 0xB181, 0xB281, 0xB381, 0xB481, 0xB581,/*30-37*/
0xB681,0xB781, 0xB881, 0xB981, 0xBA81, 0xBB81, 0xBC81, 0xBD81,/*38-3f*/
0xBE81,0xBF81, 0xC081, 0xC181, 0xC281, 0xC381, 0xC481, 0xC581,/*40-47*/
0xC681,0xC781, 0xC881, 0xC981, 0xCA81, 0xCB81, 0xCC81, 0xCD81,/*48-4f*/
0xCE81,0xCF81, 0xD081, 0xD181, 0xD281, 0xD381, 0xD481, 0xD581,/*50-57*/
0xD681,0xD781, 0xD881, 0xD981, 0xDA81, 0xDB81, 0xDC81, 0xDD81,/*58-5f*/
0xDE81,0xDF81, 0xE081, 0xE181, 0xE281, 0xE381, 0xE481, 0xE581,/*60-67*/
0xE681,0xE781, 0xE881, 0xE981, 0xEA81, 0xEB81, 0xEC81, 0xED81,/*68-6f*/
0xEE81,0xEF81, 0xF081, 0xF181, 0xF281, 0xF381, 0xF481, 0xF581,/*70-77*/
0xF681,0xF781, 0xF881, 0xF981, 0xFA81, 0xFB81, 0xFC81, C_UNDF,/*78-7f*/
0x6A81,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*80-87*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*88-8f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*90-97*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*98-9f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a0-a7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a8-af*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b0-b7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b8-bf*/
0x5F82,0x6082, 0x6182, 0x6282, 0x6382, 0x6482, 0x6582, 0x6682,/*c0-c7*/
0x6782,0x6882, 0x6982, 0x6A82, 0x6B82, 0x6C82, 0x6D82, 0x6E82,/*c8-cf*/
0x6F82,0x7082, 0x7182, 0x7282, 0x7382, 0x7482, 0x7582, 0x7682,/*d0-d7*/
0x7782,0x7882, 0x7982, 0x7A82, 0x7B82, 0x7C82, 0x7D82, 0x8D81,/*d8-df*/
C_UNDF,C_UNDF, 0x8282, 0x8382, 0x8482, 0x8582, 0x8682, 0x8782,/*e0-e7*/
C_UNDF,C_UNDF, 0x8A82, 0x8B82, 0x8C82, 0x8D82, 0x8E82, 0x8F82,/*e8-ef*/
C_UNDF,C_UNDF, 0x9282, 0x9382, 0x9482, 0x9582, 0x9682, 0x9782,/*f0-f7*/
C_UNDF,C_UNDF, 0x9A82, 0x9B82, 0x9C82, 0x8C81, 0x4B81, 0x4C81 /*f8-ff*/
};

/*********************************************************************/
/* PEOPLE REPUBLIC OF CHINA TABLE   CodePage946                      */
/*                                                                   */
/* Note: C_INV  For invalid code.                                    */
/*       C_UNDF For undefined code.                                  */
/*       C_DBCS For DBCS code.                                       */
/*                                                                   */
/*********************************************************************/

static USHORT dbcscodeP946[256]={
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*00-07*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*08-0f*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*10-17*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*18-1f*/
0x4081,0x4082, 0x4182, 0x4282, 0x8781, 0x4482, 0x4582, 0x52FA,/*20-27*/
0x4782,0x4882, 0x4982, 0x4A82, 0x4B82, 0x4C82, 0x4D82, 0x4E82,/*28-2f*/
0x4F82,0x5082, 0x5182, 0x5282, 0x5382, 0x5482, 0x5582, 0x5682,/*30-37*/
0x5782,0x5882, 0x5982, 0x5A82, 0x5B82, 0x5C82, 0x5D82, 0x5E82,/*38-3f*/
0x5F82,0x6082, 0x6182, 0x6282, 0x6382, 0x6482, 0x6582, 0x6682,/*40-47*/
0x6782,0x6882, 0x6982, 0x6A82, 0x6B82, 0x6C82, 0x6D82, 0x6E82,/*48-4f*/
0x6F82,0x7082, 0x7182, 0x7282, 0x7382, 0x7482, 0x7582, 0x7682,/*50-57*/
0x7782,0x7882, 0x7982, 0x7A82, 0x4382, 0x7C82, 0x7D82, 0x7E82,/*58-5f*/
0x8082,0x8182, 0x8282, 0x8382, 0x8482, 0x8582, 0x8682, 0x8782,/*60-67*/
0x8882,0x8982, 0x8A82, 0x8B82, 0x8C82, 0x8D82, 0x8E82, 0x8F82,/*68-6f*/
0x9082,0x9182, 0x9282, 0x9382, 0x9482, 0x9582, 0x9682, 0x9782,/*70-77*/
0x9882,0x9982, 0x9A82, 0x9B82, 0x9C82, 0x9D82, 0x9E82, C_UNDF,/*78-7f*/
0x8A81,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*80-87*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*88-8f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*90-97*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*98-9f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a0-a7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a8-af*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b0-b7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b8-bf*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*c0-c7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*c8-cf*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*d0-d7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*d8-df*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e0-e7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e8-ef*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*f0-f7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, 0x50FA, 0x7B82, 0x4A81 /*f8-ff*/
};

/*********************************************************************/
/* TAIWAN TABLE     CodePage948                                      */
/*                                                                   */
/* Note: C_INV  For invalid code.                                    */
/*       C_UNDF For undefined code.                                  */
/*       C_DBCS For DBCS code.                                       */
/*                                                                   */
/*********************************************************************/

static USHORT dbcscodeR948[256]={
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*00-07*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*08-0f*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*10-17*/
C_INV ,C_INV , C_INV , C_INV , C_INV , C_INV , C_INV , C_INV ,/*18-1f*/
0x4081,0x4981, 0x5E8B, 0x9481, 0x9081, 0x9381, 0x9581, 0x5D8B,/*20-27*/
0x6981,0x6A81, 0x9681, 0x7B81, 0x4381, 0x7C81, 0x4481, 0x5E81,/*28-2f*/
0x4F82,0x5082, 0x5182, 0x5282, 0x5382, 0x5482, 0x5582, 0x5682,/*30-37*/
0x5782,0x5882, 0x4681, 0x4781, 0x8381, 0x8181, 0x8481, 0x4881,/*38-3f*/
0x9781,0x6082, 0x6182, 0x6282, 0x6382, 0x6482, 0x6582, 0x6682,/*40-47*/
0x6782,0x6882, 0x6982, 0x6A82, 0x6B82, 0x6C82, 0x6D82, 0x6E82,/*48-4f*/
0x6F82,0x7082, 0x7182, 0x7282, 0x7382, 0x7482, 0x7582, 0x7682,/*50-57*/
0x7782,0x7882, 0x7982, 0x6D81, 0x8F81, 0x6E81, 0x4F81, 0x5181,/*58-5f*/
0x4D81,0x8182, 0x8282, 0x8382, 0x8482, 0x8582, 0x8682, 0x8782,/*60-67*/
0x8882,0x8982, 0x8A82, 0x8B82, 0x8C82, 0x8D82, 0x8E82, 0x8F82,/*68-6f*/
0x9082,0x9182, 0x9282, 0x9382, 0x9482, 0x9582, 0x9682, 0x9782,/*70-77*/
0x9882,0x9982, 0x9A82, 0x6F81, 0x6281, 0x7081, 0x5081, C_UNDF,/*78-7f*/
0x9181,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*80-87*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*88-8f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*90-97*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*98-9f*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a0-a7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*a8-af*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b0-b7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*b8-bf*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*c0-c7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*c8-cf*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*d0-d7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*d8-df*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e0-e7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*e8-ef*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS, C_DBCS,/*f0-f7*/
C_DBCS,C_DBCS, C_DBCS, C_DBCS, C_DBCS, 0x5B8B, 0x5C8B, C_UNDF /*f8-ff*/
};

static USHORT *dbcstable[]=
{
        dbcscodeJ,                      /* Japan   TableNum = 0             */
        dbcscodeK,                      /* KOREA   TableNum = 1             */
        dbcscodeP,                      /* PRC     TableNum = 2             */
        dbcscodeR,                      /* ROC     TableNum = 3             */
        dbcscodeJ942,                   /* Japan 942  TableNum = 4          */
        dbcscodeK944,                   /* KOREA 944  TableNum = 5          */
        dbcscodeP946,                   /* PRC   946  TableNum = 6          */
        dbcscodeR948                    /* ROC   948  TableNum = 7          */
};

/*$$Y  Resume CPRETTY code formatting.                               */

/*********************************************************************/
/* Description: Convert to DBCS character that is equal to SBCS      */
/*              character.                                           */
/*********************************************************************/

void DBCS_ConvToDBCS(
  PUCHAR    input,                     /* Converted DBCS.                   */
  PUCHAR   *output )                   /* Target SBCS.                      */
{
  PUCHAR    outspot;                   /* output spot                       */
  union convert {                      /* Use to get DBCS bytes             */
    USHORT convchar;                   /* From USHORT DBCS code.            */
    UCHAR conv[2];                     /* DBCS 1ST and 2ND byte.            */
  }
  convert;
  size_t TableNum;                     /* current country table             */
  USHORT TableChr;                     /* Table number for character.       */

  outspot = *output;                   /* copy the pointer                  */
                                       /* find the county code              */
  switch (current_settings->codepage) {/* Which code page is used?.         */
    case  CP_JAPAN :                   /* Japan 932.                        */
      TableNum = 0;                    /* Table number is 0.                */
      break;
    case  CP_KOREA :                   /* KOREA 934.                        */
      TableNum = 1;                    /* Table number is 1.                */
      break;
    case  CP_PRC :                     /* PRC 936.                          */
      TableNum = 2;                    /* Table number is 2.                */
      break;
    case  CP_ROC :                     /* ROC 938.                          */
      TableNum = 3;                    /* Table number is 3.                */
      break;
    case  CP_JAPAN_942 :               /* Japan 942.                        */
      TableNum = 4;                    /* Tabel number is 4.                */
      break;
    case  CP_KOREA_944 :               /* KOREA 944.                        */
      TableNum = 5;                    /* Table number is 5.                */
      break;
    case  CP_PRC_946 :                 /* PRC 946.                          */
      TableNum = 6;                    /* Table number is 6.                */
      break;
    case  CP_ROC_948 :                 /* ROC 948.                          */
      TableNum = 7;                    /* Table number is 7.                */
      break;
    default  :                         /* Others. move original and         */
                                       /* return.                           */
      *outspot++ = *input;             /* just copy the character           */
      *output = outspot;               /* update the location               */
      return ;
  }
                                       /* see if we need to convert         */
  TableChr = dbcstable[TableNum][*input];

  if ((TableChr == C_INV) ||           /* non-convertable?                  */
      (TableChr == C_UNDF) ||
      (TableChr == C_DBCS)) {
    *outspot++ = *input;               /* just copy the character           */
    *output = outspot;                 /* update the location               */
  }
                                       /* need to convert                   */
  else {
    convert.convchar = TableChr;
    *outspot++ = convert.conv[0];      /* Put DBCS 1st byte.                */
    *outspot++ = convert.conv[1];      /* Put DBCS 2nd byte.                */
    *output = outspot;                 /* update the location               */
  }
}

/*********************************************************************/
/* Description: Convert to SBCS character that is equal to DBCS      */
/*              character.                                           */
/*********************************************************************/

void DBCS_ConvToSBCS(
  PUCHAR    input,                     /* Converted DBCS.                   */
  PUCHAR   *output )                   /* Target SBCS.                      */
{
  PUCHAR    outspot;                   /* output spot                       */
  size_t i;                            /* Use for index of table.           */
  union convert {                      /* Use to get DBCS bytes             */
    USHORT convchar;                   /* From unsigned int DBCS            */
    UCHAR conv[2];                     /* DBCS 1ST and 2ND byte.            */
  }
  convert;
  size_t TableNum;

  outspot = *output;                   /* copy the pointer                  */
  switch (current_settings->codepage) {/* Which code page is used?.         */
    case  CP_JAPAN :                   /* Japan 932.                        */
      TableNum = 0;                    /* Table number is 0.                */
      break;
    case  CP_KOREA :                   /* KOREA 934.                        */
      TableNum = 1;                    /* Table number is 1.                */
      break;
    case  CP_PRC :                     /* PRC 936.                          */
      TableNum = 2;                    /* Table number is 2.                */
      break;
    case  CP_ROC :                     /* ROC 938.                          */
      TableNum = 3;                    /* Table number is 3.                */
      break;
    case  CP_JAPAN_942 :               /* Japan 942.                        */
      TableNum = 4;                    /* Tabel number is 4.                */
      break;
    case  CP_KOREA_944 :               /* KOREA 944.                        */
      TableNum = 5;                    /* Table number is 5.                */
      break;
    case  CP_PRC_946 :                 /* PRC 946.                          */
      TableNum = 6;                    /* Table number is 6.                */
      break;
    case  CP_ROC_948 :                 /* ROC 948.                          */
      TableNum = 7;                    /* Table number is 7.                */
      break;
    default  :                         /* Others. move original and         */
                                       /* return.                           */
      memcpy(outspot, input, DBCS_BYTELEN);
      *output = outspot + DBCS_BYTELEN;/* update output location            */
      return ;
  }

  convert.conv[0] = *input;            /* Get DBCS 1st.                     */
  convert.conv[1] = *(input+1);        /* Get DBCS 2nd.                     */

  if ((convert.convchar != C_UNDF) && (convert.convchar != C_INV)) {
                                       /* need to scan the table            */
    for (i = 0x00; i <= 0xFF; i++) {   /* **** If Found. ****               */

      if (dbcstable[TableNum][i] == convert.convchar) {/* **** Put          */
                                       /* the index num as SBCS code.       */
                                       /* ****                              */
        *outspot++ = (char)i;          /* copy the single character         */
        *output = outspot;             /* update the original               */
        return ;                       /* Length is 1.                      */
      }
    }
  }
                                       /* Move original if not found.       */
  memcpy(outspot, input, DBCS_BYTELEN);
  *output = outspot + DBCS_BYTELEN;    /* update output location            */
}

/*********************************************************************/
/* Descriptive Name: Move the pointer per logical character length.  */
/*********************************************************************/

size_t RexxString::DBCSmovePointer(size_t   Start,
                                   INT    Direction,
                                   size_t   CharLen)
{
  size_t    BaseLen;                   /* length of base string             */
  size_t    TailLen;                   /* length of tail string             */
  PUCHAR    Cpos;                      /* character position                */
  size_t    Result;                    /* final resulting offset            */

  if (CharLen >= 1) {                  /* If Charlen is zero or             */
                                       /* minus, NOP.                       */
    if (Direction < 0) {               /* Left.                             */
                                       /* get front character length        */
      BaseLen = DBCS_CharacterCount(STRPTR(this), Start);
      if (CharLen > BaseLen)           /* going to fall off?                */
        Result = 0;                    /* stop at the front                 */
      else {
        Cpos = STRPTR(this);           /* reset to the front                */
        CharLen = BaseLen - CharLen;   /* adjust to forward movement        */
        TailLen = Start;               /* set final movement length         */
                                       /* increment directly                */
        DBCS_IncChar(&Cpos, &TailLen, &CharLen);
        Result = Cpos - STRPTR(this);  /* calculate final offset            */
      }
    }
    else {                             /* Right.                            */
      TailLen = this->length - Start;  /* get tail length                   */
      Cpos = STRPTR(this) + Start;     /* get the character position        */
                                       /* increment directly                */
      DBCS_IncChar(&Cpos, &TailLen, &CharLen);
      Result = Cpos - STRPTR(this);    /* calculate final offset            */
    }
  }
  else Result = CharLen;
  return Result;
}

/*********************************************************************/
/*   FUNCTION           : Finds the first ocurrence in string1 of any*/
/*                        character from string2.                    */
/*                                                                   */
/*   The strpbrk function finds the first occurrence in string1 of   */
/*   any character from string2.                                     */
/*                                                                   */
/*   DBCS characters must be allowed in string1 and are ignored in   */
/*   string2.                                                        */
/*********************************************************************/

PUCHAR DBCS_strpbrk(
  PUCHAR    String,                    /* searched string                   */
  size_t    StringLength,              /* length of searched string         */
  PCHAR     Reference )                /* set of reference characters       */
{
  PUCHAR    Match;                     /* match location                    */

  Match = NULL;                        /* no hits yet                       */
  while (StringLength--) {             /* loop through string one           */
    if (!IsDBCS(*String)) {            /* current character SBCS?           */
                                       /* have a hit?                       */
      if (*String != '\0' && strchr(Reference, *String)) {
        Match = String;                /* copy pointer                      */
        break;                         /* finished                          */
      }
      String++;                        /* step to next character            */
    }
    else {
      String += DBCS_BYTELEN;          /* step over DBCS                    */
      StringLength--;                  /* reduce the length one more        */
    }
  }
  return Match;                        /* return match location             */
}

/*********************************************************************/
/* DESCRIPTION : Find last occurence of SBCS character in DBCS string*/
/*********************************************************************/

PUCHAR DBCS_strrchr(
  PUCHAR    String,                    /* searched string                   */
  size_t    Length,                    /* string length                     */
  UCHAR     ch )                       /* searched character                */
{
  PUCHAR    Match;                     /* match location                    */

  Match = NULL;                        /* no match yet                      */

  while (Length--) {                   /* while not end character           */
    if (!IsDBCS(*String)) {            /* if not a DBCS character           */
      if (*String == ch)               /* one we want?                      */
        Match = String;                /* remember location                 */
      String++;
    }
    else
      String += DBCS_BYTELEN;          /* else step two bytes               */
  }
  return Match;                        /* return match location             */
}

/*********************************************************************/
/*   FUNCTION           : Skips all occurrences in string1 of any    */
/*                        character from string2.                    */
/*                                                                   */
/*   DBCS characters must be allowed in string1 and are ignored in   */
/*   string2.                                                        */
/*********************************************************************/

PUCHAR DBCS_strspn(
  PUCHAR    String,                    /* searched string                   */
  size_t    StringLength,              /* length of searched string         */
  PCHAR     Reference )                /* set of reference characters       */
{
  PUCHAR    Match;                     /* match location                    */

  Match = NULL;                        /* no hits yet                       */
  while (StringLength--) {             /* loop through string one           */
    if (!IsDBCS(*String)) {            /* current character SBCS?           */
                                       /* mismatch?                         */
      if (*String != '\0' && !strchr(Reference, *String)) {
        Match = String;                /* copy pointer                      */
        break;                         /* finished                          */
      }
      String++;                        /* step to next character            */
    }
    else {
      String += DBCS_BYTELEN;          /* step over DBCS                    */
      StringLength--;                  /* reduce the length one more        */
    }
  }
  return Match;                        /* return match location             */
}

/*********************************************************************/
/* Description: Compare operator.                                    */
/*********************************************************************/

INT RexxString::DBCSstringCompare(RexxString *Right )
{
  size_t    CharNum;                   /* Number of character.              */
  size_t    LeftLength;                /* left string length                */
  size_t    RightLength;               /* right string length               */
  PUCHAR    LeftPtr;                   /* left string pointer               */
  PUCHAR    RightPtr;                  /* right string pointer              */

  ValidDBCS(this);                     /* validate both strings             */
  ValidDBCS(Right);

  LeftPtr = STRPTR(this);              /* get left string pointer and length*/
  LeftLength = STRLEN(this);
  RightPtr = STRPTR(Right);            /* get right string info also        */
  RightLength = STRLEN(Right);

                                       /* skip leading blanks               */
  DBCS_SkipBlanks(&LeftPtr, &LeftLength);
  DBCS_SkipBlanks(&RightPtr, &RightLength);
                                       /* strip trailing blanks now         */
  DBCS_StripBlanks(&LeftPtr, &LeftLength);
  DBCS_StripBlanks(&RightPtr, &RightLength);

                                       /* do the compare                    */
  return DBCS_CharCompare(LeftPtr, LeftLength,
                          RightPtr, RightLength,
                          (PUCHAR)" ", &CharNum);
}

/*********************************************************************/
/* Description: Reverse Built-in function.                           */
/*********************************************************************/

RexxString *RexxString::DBCSreverse()
{
  PUCHAR Sptr;                         /* Map of current bytes.             */
  PUCHAR Dptr;                         /* Map of buffer bytes.              */
  PUCHAR Endptr;                       /* End position for reverse          */
  RexxString *Retval;                  /* return value                      */

  ValidDBCS(this);                     /* validate input string             */
  Retval = raw_string(this->length);   /* get an output string              */
  Sptr = STRPTR(this);                 /* start of input                    */
                                       /* point to output location          */
  Dptr = STRPTR(Retval) + STRLEN(Retval);
  Endptr = Sptr + STRLEN(this);        /* get an end position also          */

  while (Sptr < Endptr) {              /* Do Reverse HugeString             */
    if (IsDBCS(*Sptr)) {               /* current position DBCS?            */
      Dptr -= DBCS_BYTELEN;            /* step back two bytes               */
      memcpy(Dptr, Sptr, DBCS_BYTELEN);/* copy two characters               */
      Sptr += DBCS_BYTELEN;            /* step to next logical              */
    }
    else
      *--Dptr = *Sptr++;               /* just copy one character           */
  }
  Retval->generateHash();              /* done building the string          */
  return Retval;                       /* Return reversed string            */
}

/*********************************************************************/
/* Description: Substr Built-in function.                            */
/*********************************************************************/

RexxString *RexxString::DBCSsubstr(RexxInteger *position,
                                   RexxInteger *strLength,
                                   RexxString  *pad)
{
  size_t   StringChar;                 /* size of input string              */
  size_t   StartPos;                   /* substr start position             */
  size_t   SubstrSize;                 /* size of substring                 */
  size_t   PadSize;                    /* size of pad character             */
  PUCHAR   PadChar;                    /* pointer to pad                    */
  PUCHAR   SubPtr;                     /* pointer to substring              */
  PUCHAR   EndPtr;                     /* pointer to substring end          */
  size_t   StringLength;               /* byte length of input string       */
  size_t   RealSubLen;                 /* byte length of substring          */
  size_t   Length;                     /* requested length                  */
  RexxString *Retval;                  /* function return value             */

                                       /* get size of input string          */
  StringChar = ValidDBCS(this);        /* validate the string               */

  StartPos = get_position(position, ARG_ONE) - 1;
  if (StringChar >= StartPos)          /* pos within the string?            */
    Length = StringChar - StartPos;    /* length is remainder               */
  else
    Length = 0L;                       /* string is used up                 */
                                       /* get the result length             */
  SubstrSize = optional_length(strLength, Length, ARG_TWO);
                                       /* validate the pad character        */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
  PadSize = strlen((PCHAR)PadChar);    /* get the pad size too              */
  if (!SubstrSize)                     /* zero bytes requested?             */
    Retval = OREF_NULLSTRING;          /* this is a null string             */
  else {
    SubPtr = STRPTR(this);             /* point to the data                 */
    StringLength = STRLEN(this);       /* get the byte length               */
                                       /* step to the start                 */
    DBCS_IncChar(&SubPtr, &StringLength, &StartPos);
    if (StartPos) {                    /* nothing left?                     */
                                       /* get padded string                 */
      Retval = raw_string(SubstrSize * PadSize);
                                       /* pad to the length                 */
      DBCS_SetPadChar(STRPTR(Retval), SubstrSize, PadChar);
      Retval->generateHash();          /* done building the string          */
    }
    else {
      EndPtr = SubPtr;                 /* copy the pointer                  */
                                       /* skip over the middle              */
      DBCS_IncChar(&EndPtr, &StringLength, &SubstrSize);
      if (!SubstrSize)                 /* get it all?                       */
                                       /* just extract the piece            */
        Retval = new_string((PCHAR)SubPtr, EndPtr - SubPtr);
      else {                           /* need to pad                       */
                                       /* get length of substring           */
        RealSubLen = EndPtr - SubPtr;
                                       /* allocate a buffer                 */
        Retval = raw_string((SubstrSize * PadSize) + RealSubLen);
                                       /* copy string piece                 */
        memcpy(STRPTR(Retval), SubPtr, RealSubLen);
                                       /* pad to the length                 */
        DBCS_SetPadChar(STRPTR(Retval) + RealSubLen, SubstrSize, PadChar);
        Retval->generateHash();        /* done building the string          */
      }
    }
  }
  return  Retval;                      /* Return the substring              */
}

/*********************************************************************/
/* Description: Delstr Built-in function.                            */
/*********************************************************************/

RexxString *RexxString::DBCSdelstr(RexxInteger *position,
                                   RexxInteger *plength)
{
  size_t   StringChar;                 /* size of input string              */
  size_t   StartPos;                   /* substr start position             */
  size_t   DeleteSize;                 /* size of deletion                  */
  size_t   StringLength;               /* length of input string            */
  size_t   FrontLength;                /* length of front part              */
  size_t   BackLength;                 /* length of back part               */
  PUCHAR   FrontEnd;                   /* end of front part                 */
  PUCHAR   BackEnd;                    /* end of back part                  */
  PUCHAR   BackStart;                  /* start of back part                */
  RexxString *Retval;                  /* return string                     */

  StringChar = ValidDBCS(this);        /* get string length                 */
                                       /* get start string position         */
  StartPos = get_position(position, ARG_ONE);
                                       /* get the length to delete          */
  DeleteSize = optional_length(plength, StringChar - StartPos + 1, ARG_TWO);

  FrontEnd = STRPTR(this);             /* copy string pointer               */
  StringLength = STRLEN(this);         /* get size of string                */
  StartPos--;                          /* make start origin 0               */
                                       /* step to the start                 */
  DBCS_IncChar(&FrontEnd, &StringLength, &StartPos);
  if (StartPos)                        /* skip everything?                  */
    Retval = this;                     /* return string unchanged           */
  else {
    BackStart = FrontEnd;              /* copy the pointer                  */
                                       /* step to the start                 */
    DBCS_IncChar(&BackStart, &StringLength, &DeleteSize);
    if (DeleteSize) {                  /* used it up?                       */
                                       /* just copy the front               */
      Retval = new_string((PCHAR)STRPTR(this), FrontEnd - STRPTR(this));
    }
    else {
                                       /* point to the end                  */
      BackEnd = STRPTR(this) + STRLEN(this);
                                       /* get front length                  */
      FrontLength = (FrontEnd - STRPTR(this));
                                       /* and the back length               */
      BackLength = (BackEnd - BackStart);
                                       /* allocate the return string        */
      Retval = raw_string(BackLength + FrontLength);
                                       /* copy the front and back           */
      memcpy(STRPTR(Retval), STRPTR(this), FrontLength);
      memcpy(STRPTR(Retval) + FrontLength, BackStart, BackLength);
      Retval->generateHash();          /* done building the string          */
    }
  }
  return Retval;                       /* Return the deleted string         */
}

/*********************************************************************/
/* Description: Subword Built-in function.                           */
/*********************************************************************/

RexxString *RexxString::DBCSsubWord(RexxInteger *position,
                                    RexxInteger *plength)
{
  size_t   StringChar;                 /* size of input string              */
  size_t   StartPos;                   /* substr start position             */
  size_t   SubwordSize;                /* size of substring                 */
  size_t   StringLength;               /* byte length of input string       */
  PUCHAR   Scan;                       /* scan pointer                      */
  PUCHAR   SubwordPtr;                 /* pointer to subword part           */
  PUCHAR   WordPtr;                    /* pointer to word end               */
  size_t   WordLength;                 /* length of word start              */
  RexxString *Retval;                  /* return string                     */

  StringChar = ValidDBCS(this);        /* get size of input string          */
                                       /* convert position to binary        */
  StartPos = get_position(position, ARG_ONE);
                                       /* get num of words to delete, the   */
                                       /* default is "a very large number"  */
  SubwordSize = optional_length(plength, MAXNUM, ARG_TWO);

  StringLength = STRLEN(this);         /* get length of string              */
                                       /* if null input string              */
  if (!StringLength || !SubwordSize )  /* or no words requested             */
    Retval = OREF_NULLSTRING;          /* so return a null string           */
  else {                               /* otherwise output something        */
    Scan = STRPTR(this);               /* point to input                    */
                                       /* step over leading words           */
    while (StringLength) {             /* while input                       */
                                       /* skip leading blanks               */
      DBCS_SkipBlanks(&Scan, &StringLength);
      if (StringLength) {              /* non-blank found in line           */
        if (--StartPos == 0)           /* dec count of words to copy        */
          break;                       /* if 0, we are done                 */
                                       /* do the non-blank now              */
        DBCS_SkipNonBlanks(&Scan, &StringLength);
      }
    }
    if (StringLength) {                /* if still have a string            */
      SubwordPtr = Scan;               /* copy start pointer                */
      WordPtr = SubwordPtr;            /* initialize to eliminate compile error */
      while (StringLength &&           /* skip over words to be             */
          SubwordSize) {               /* extracted                         */
                                       /* first skip non-blanks             */
        DBCS_SkipNonBlanks(&Scan, &StringLength);
        WordPtr = Scan;                /* save word end position            */
        SubwordSize--;                 /* count the word                    */
        if (StringLength)              /* still some left?                  */
                                       /* skip interword blanks             */
          DBCS_SkipBlanks(&Scan, &StringLength);
      }
                                       /* get substring size                */
      WordLength = (WordPtr - SubwordPtr);
                                       /* allocate output string            */
      Retval = new_string((PCHAR)SubwordPtr, WordLength);
    }
    else
      Retval = OREF_NULLSTRING;        /* start is past end--NULL           */
  }
  return Retval;                       /* return extracted words            */
}

/*********************************************************************/
/* Description: Delword Built-in function.                           */
/*********************************************************************/

RexxString *RexxString::DBCSdelWord(RexxInteger *position,
                                    RexxInteger *plength)
{
  size_t   StringChar;                 /* size of input string              */
  size_t   StartPos;                   /* substr start position             */
  size_t   DeleteSize;                 /* size of deletion                  */
  size_t   StringLength;               /* length of input string            */
  size_t   FrontLength;                /* length of front part              */
  PUCHAR   BackStart;                  /* start of back part                */
  PUCHAR   FrontPtr;                   /* start of front part               */
  PUCHAR   Scan;                       /* scanning pointer                  */
  PUCHAR   CopyPtr;                    /* pointer for copying strings       */
  RexxString *Retval;                  /* return string                     */

  StringChar = ValidDBCS(this);        /* get string length                 */
                                       /* get start string position         */
  StartPos = get_position(position, ARG_ONE);
                                       /* get the length to delete          */
  DeleteSize = optional_length(plength, StringChar - StartPos + 1, ARG_TWO);
  StringLength = STRLEN(this);         /* get length of string              */

  if (!StringLength)                   /* if null input string              */
    Retval = OREF_NULLSTRING;          /* so return a null string           */
  else if (!DeleteSize)                /* deleting zero words?              */
    Retval = this;                     /* return entire string              */
  else {                               /* otherwise output something        */
    Scan = STRPTR(this);               /* point to input                    */
    FrontPtr = Scan;                   /* save front part                   */
                                       /* step over leading words           */
    while (--StartPos && StringLength) {
                                       /* skip leading blanks               */
      DBCS_SkipBlanks(&Scan, &StringLength);
      if (StringLength) {              /* non-blank found in line           */
                                       /* do the non-blank now              */
        DBCS_SkipNonBlanks(&Scan, &StringLength);
      }
    }
                                       /* keep blanks trailing first part   */
    DBCS_SkipBlanks(&Scan, &StringLength);
    if (StringLength) {                /* if still have a string            */
                                       /* get length of front part          */
      FrontLength = (Scan - FrontPtr);
      BackStart = Scan;                /* set back pointer                  */
      while (StringLength &&           /* skip over words to be             */
          DeleteSize) {                /* deleted                           */
                                       /* skip interword blanks             */
        DBCS_SkipBlanks(&Scan, &StringLength);
                                       /* first skip non-blanks             */
        DBCS_SkipNonBlanks(&Scan, &StringLength);
        DeleteSize--;                  /* count the word                    */
      }
                                       /* skip blanks trailing deleted part */
      DBCS_SkipBlanks(&Scan, &StringLength);
                                       /* allocate output string            */
      BackStart = Scan;                /* save end position                 */
      Retval = raw_string(FrontLength + StringLength);
      CopyPtr = STRPTR(Retval);        /* get string start                  */
      if (FrontLength) {               /* have a front part?                */
                                       /* copy the front                    */
        memcpy(CopyPtr, FrontPtr, FrontLength);
        CopyPtr += FrontLength;        /* step the pointer                  */
      }
      if (StringLength)                /* have a back part?                 */
                                       /* copy the back part                */
        memcpy(CopyPtr, BackStart, StringLength);
      Retval->generateHash();          /* done building the string          */
    }
    else
      Retval = this;                   /* return copy of the string         */
  }
  return Retval;                       /* return extracted words            */
}

/*********************************************************************/
/* Description: Strip Built-in function.                             */
/*********************************************************************/

RexxString *RexxString::DBCSstrip(RexxString *option,
                                  RexxString *stripchar)
{
  PUCHAR   CStr;                       /* scan string                       */
  PUCHAR   EndStr;                     /* end of string                     */
  PUCHAR   CStr2;                      /* second scan string                */
  PUCHAR   StripStart;                 /* start of stripped field           */
  UCHAR    Option;                     /* Strip option.                     */
  size_t   RemSize;                    /* size of removed character         */
  PUCHAR   RemChar;                    /* Removed character                 */

  ValidDBCS(this);                     /* validate the string               */
                                       /* and the pad character             */
  RemChar = ValidatePad(stripchar, (const PUCHAR)" ");
  RemSize = strlen((PCHAR)RemChar);    /* get the character size            */
                                       /* get the option character          */
  Option = option_character(option, STRIP_BOTH, ARG_ONE);
                                       /* must be a valid option            */
  if (Option != STRIP_TRAILING && Option != STRIP_LEADING && Option != STRIP_BOTH )
    report_exception2(Error_Incorrect_method_option, new_string("BLT", 3), option);

  CStr = STRPTR(this);                 /* point to the string               */
  EndStr = CStr + STRLEN(this);        /* point to the end                  */
  StripStart = NULL;                   /* no strip point yet                */
                                       /* need to strip leading?            */
  if ((Option == STRIP_LEADING) || (Option == STRIP_BOTH)) {

    if (RemSize == SBCS_BYTELEN) {     /* single byte?                      */
      while (CStr < EndStr) {          /* while not at the end              */
        if (*CStr != *RemChar)         /* mismatch?                         */
          break;                       /* finished                          */
        CStr++;                        /* go around again                   */
      }
    }
    else {                             /* double byte strip                 */
      while ((CStr + 1) < EndStr) {    /* still two bytes left?             */
        if (*CStr != *RemChar ||       /* mismatch on both bytes            */
           (*(CStr+1) != *(RemChar+1)))/* of the character?                 */
          break;                       /* finished                          */
        CStr += DBCS_BYTELEN;          /* go around again                   */
      }
    }
  }

                                       /* need to stip trailing?            */
  if ((Option == STRIP_TRAILING) || (Option == STRIP_BOTH)) {
    CStr2 = CStr;                      /* start at current position         */
    StripStart = NULL;                 /* haven't found strip part          */

    if (RemSize == SBCS_BYTELEN) {     /* looking for SBCS characters       */
      while (CStr2 < EndStr) {         /* while more string                 */
        if (IsDBCS(*CStr2)) {          /* DBCS character?                   */
          CStr2 += DBCS_BYTELEN;       /* just skip this                    */
          StripStart = NULL;           /* reset the start                   */
        }
        else if (*CStr2 == *RemChar) { /* found a strip character?          */
          if (!StripStart)             /* first one?                        */
            StripStart = CStr2;        /* remember location                 */
          CStr2++;                     /* step over this                    */
        }
        else {                         /* some other character              */
          StripStart = NULL;           /* reset the start                   */
          CStr2++;                     /* skip over this                    */
        }
      }
    }
    else {                             /* stripping DBCS characters         */
      while (CStr2 < EndStr) {         /* while more string                 */
        if (IsDBCS(*CStr2)) {          /* DBCS character?                   */
          if (*CStr2 == *RemChar &&    /* is it a match?                    */
              *(CStr2+1) == *(RemChar+1)) {
            if (!StripStart)           /* first one?                        */
              StripStart = CStr2;      /* remember location                 */
          }
          else
            StripStart = NULL;         /* reset the start                   */

          CStr2 += DBCS_BYTELEN;       /* just skip this                    */
        }
        else {                         /* some other character              */
          StripStart = NULL;           /* reset the start                   */
          CStr2++;                     /* skip over this                    */
        }
      }
    }
  }
  if (!StripStart)                     /* nothing to strip                  */
    StripStart = EndStr;               /* take to the end                   */
                                       /* copy the result part              */
  return new_string((PCHAR)CStr, (StripStart - CStr));
}

/*********************************************************************/
/* Description: Length Built-in function.                            */
/*********************************************************************/

RexxInteger *RexxString::DBCSlength()
{
  size_t tempLen;
  tempLen = ValidDBCS(this);     /* place holder to invoke new_integer */
  return new_integer(tempLen);         /* return character length           */
}

/*********************************************************************/
/* Description: Word position Built-in function.                     */
/*********************************************************************/

RexxInteger *RexxString::DBCSwordPos(RexxString *phrase,
                                    RexxInteger *pstart)
{
  RexxInteger *Retval;                 /* return value                      */
  PUCHAR   Needle;                     /* start of needle string            */
  PUCHAR   Haystack;                   /* current haystack positon          */
  PUCHAR   NextNeedle;                 /* next search position              */
  PUCHAR   NextHaystack;               /* next search position              */
  size_t   Count;                      /* current haystack word pos         */
  size_t   NeedleWords;                /* needle word count                 */
  size_t   HaystackWords;              /* haystack word count               */
  size_t   SearchCount;                /* possible searches to do           */
  size_t   FirstNeedle;                /* length of first needle word       */
  size_t   NeedleLength;               /* length of needle                  */
  size_t   HaystackLength;             /* length of haystack                */
  size_t   NeedleWordLength;           /* length of needle word             */
  size_t   HaystackWordLength;         /* length of haystack word           */
  size_t   NeedleScanLength;           /* temporary scan length             */
  size_t   HaystackScanLength;         /* temporary scan length             */
  PUCHAR   NeedlePosition;             /* temporary pointers for            */
  PUCHAR   HaystackPosition;           /* the searches                      */
  PUCHAR   NextHaystackPtr;            /* pointer to next word              */
  PUCHAR   NextNeedlePtr;              /* pointer to next word              */
  size_t   i;                          /* loop counter                      */
  ValidDBCS(this);                     /* validate haystack                 */
                                       /* get nuber of words in haystack    */
  HaystackWords = DBCS_WordLen(STRDESC(this));
  phrase = get_string(phrase, ARG_ONE);/* get the phrase we are looking for */
  ValidDBCS(phrase);                   /* validate the phrase too           */
                                       /* get nuber of words in needle      */
  NeedleWords = DBCS_WordLen(STRDESC(phrase));
                                       /* get starting position, the default*/
                                       /* is the first word                 */
  Count = optional_position(pstart, 1, ARG_TWO);

  Needle = STRPTR(phrase);             /* get friendly pointer              */
  Haystack = STRPTR(this);             /* and the second also               */
  NeedleLength = STRLEN(phrase);       /* get the needle length             */
  HaystackLength = STRLEN(this);       /* get the haystack length           */
                                       /* if search string is longer        */
                                       /* or no words in search             */
                                       /* or count is longer than           */
                                       /* haystack                          */
  if (NeedleWords > (HaystackWords - Count + 1) ||
      NeedleWords == 0 ||
      Count > HaystackWords) {
    Retval = IntegerZero;              /* can't find anything,              */
  }
  else {                               /* need to search                    */
                                       /* point at first word               */
    HaystackWordLength = DBCS_NextWord(&Haystack, &HaystackLength, &NextHaystack);

    for (i = Count - 1; i; i--) {      /* now skip over count-1             */
      Haystack = NextHaystack;         /* step past current word            */
                                       /* find the next word                */
      HaystackWordLength = DBCS_NextWord(&Haystack, &HaystackLength, &NextHaystack);
    }
                                       /* get number of searches            */
    SearchCount = (HaystackWords - NeedleWords - Count) + 2;
                                       /* position at first needle          */
    FirstNeedle = DBCS_NextWord(&Needle, &NeedleLength, &NextNeedle);
                                       /* loop for the possible times       */
    for (; SearchCount; SearchCount--) {
      NeedleWordLength = FirstNeedle;  /* set the length                    */
      NeedlePosition = Needle;         /* get the start of phrase           */
      HaystackPosition = Haystack;     /* and the target string loop        */
                                       /* for needlewords                   */
      NextHaystackPtr = NextHaystack;  /* copy nextword information         */
      NextNeedlePtr = NextNeedle;
                                       /* including the lengths             */
      HaystackScanLength = HaystackLength;
      NeedleScanLength = NeedleLength;

      for (i = NeedleWords; i; i--) {  /* possible times                    */

        if (HaystackWordLength !=      /* if wrong length, then it          */
            NeedleWordLength)          /* can't be a match...just           */
          break;                       /* leave the loop                    */
                                       /* if the two words don't            */
                                       /* match, then no sense              */
                                       /* checking the rest                 */
        if (memcmp(NeedlePosition, HaystackPosition, NeedleWordLength))
          break;                       /* get out fast.                     */

                                       /* the last words matched, so        */
                                       /* continue searching.               */

                                       /* set new search information        */
        HaystackPosition = NextHaystackPtr;
        NeedlePosition = NextNeedlePtr;
                                       /* Scan off the next word            */
        HaystackWordLength = DBCS_NextWord(&HaystackPosition, &HaystackScanLength, &NextHaystackPtr);
                                       /* repeat for the needle             */
        NeedleWordLength = DBCS_NextWord(&NeedlePosition, &NeedleScanLength, &NextNeedlePtr);
      }

      if (!i)                          /* all words matched, we             */
        break;                         /* found it                          */
      Haystack = NextHaystack;         /* set the search position           */
                                       /* step to next haytack pos          */
      HaystackWordLength = DBCS_NextWord(&Haystack, &HaystackLength, &NextHaystack);
      Count++;                         /* remember the word position        */
    }

    if (SearchCount)                   /* if we haven't scanned the         */
                                       /* entire string                     */
      Retval = new_integer(Count);     /* return the position               */

    else                               /* it wasn't found, just             */
      Retval = IntegerZero;            /* return a zero.                    */
  }
  return Retval;                       /* return the position               */
}

/*********************************************************************/
/* Description: Data type Built-in function.                         */
/*********************************************************************/

RexxInteger *RexxString::DBCSdatatype(INT  DataType )
{
  RexxInteger *Retval;                 /* Return DataValue.                 */
  UCHAR    MixedType;                  /* HugeString attributes.            */

  MixedType = DBCS_Type(this);         /* get type of string                */

                                       /* looking for valid mixed?          */
  if (DataType == DATATYPE_MIXED_DBCS) {
                                       /* If not invalid string, then       */
    if (MixedType != INV_MIXED && MixedType != PURE_SBCS)
      Retval = TheTrueObject;          /* this is true                      */
    else
      Retval = TheFalseObject;         /* this is false                     */
  }
  else {                               /* looking for pure DBCS             */
    if (MixedType == PURE_DBCS)        /* is it pure?                       */
      Retval = TheTrueObject;          /* this is true                      */
    else
      Retval = TheFalseObject;         /* this is false                     */
  }
  return Retval;                       /* Return DBCS_datatype              */
}

/*********************************************************************/
/* Description: Compare Built-in function.                           */
/*********************************************************************/

RexxInteger *RexxString::DBCScompare(RexxString *string2,
                                     RexxString *pad)
{
  PUCHAR   PadChar;                    /* Padding char for CharComp.        */
  size_t   CharNum;                    /* Difference char position.         */

  ValidDBCS(this);                     /* validate first string             */
                                       /* validate second string            */
  string2 = RequiredArg(string2, &CharNum, ARG_ONE);
                                       /* validate the pad character        */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
                                       /* pass to character compare         */
  DBCS_CharCompare(STRDESC(this), STRDESC(string2), PadChar, &CharNum);
  return new_integer(CharNum);         /* format the result                 */
}

/*********************************************************************/
/* Description: Copies Built-in function.                            */
/*********************************************************************/

RexxString *RexxString::DBCScopies(RexxInteger *ncopies)
{
  ValidDBCS(this);                     /* validate arg                      */
  return OREF_NULLSTRING;              /* and just return                   */
}

/*********************************************************************/
/* Description: Abbrev Built-in function.                            */
/*********************************************************************/

RexxInteger *RexxString::DBCSabbrev(RexxString *info,
                                    RexxInteger *plength)
{
  size_t   M1CLen;                     /* arg1 Logical char length          */
  size_t   M2CLen;                     /* arg2 Logical char length          */
  size_t   M1BLen;                     /* arg1 byte length                  */
  size_t   M2BLen;                     /* arg2 byte length                  */
  size_t   MinLen;                     /* Minium Logical char length.       */
  RexxInteger *Retval;                 /* return value                      */

  M1CLen = ValidDBCS(this);            /* validate the first argument       */
                                       /* validate the first argument       */
  info = RequiredArg(info, &M2CLen, ARG_ONE);
  M1BLen = STRLEN(this);               /* get byte lengths also             */
  M2BLen = STRLEN(info);

                                       /* get the optional check length     */
  MinLen = optional_length(plength, M2CLen, ARG_TWO);
                                       /* second longer than ref or         */
                                       /* shorter than required?            */
  if (M1CLen < M2CLen || M1BLen < M2BLen || M2CLen < MinLen)
    Retval = TheFalseObject;           /* this is false                     */
  else {                               /* just want equality                */
    if (!memcmp(STRPTR(this), STRPTR(info), STRLEN(info)))
      Retval = TheTrueObject;          /* this is true                      */
    else
      Retval = TheFalseObject;         /* this is false                     */
  }
  return Retval;                       /* return true/false                 */
}

/*********************************************************************/
/* Description: Space Built-in function.                             */
/*********************************************************************/

RexxString *RexxString::DBCSspace(RexxInteger *space_count,
                                  RexxString *pad)
{
  size_t      Spaces;                  /* requested spacing                 */
  PUCHAR      PadChar;                 /* pad character                     */
  size_t      PadSize;                 /* size of the pad character         */
  PUCHAR      Current;                 /* current pointer position          */
  PUCHAR      Word;                    /* current word pointer              */
  PUCHAR      NextSite;                /* next word                         */
  size_t      Count;                   /* count of words                    */
  size_t      Length;                  /* remaining length                  */
  size_t      WordLength;              /* word size                         */
  RexxString *Retval;                  /* return value                      */
  size_t      WordSize;                /* total size of words               */

  ValidDBCS(this);                     /* validate first argument           */
                                       /* get the spacing count             */
  Spaces = optional_length(space_count, 1, ARG_ONE);
                                       /* get the padding character         */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
  PadSize = strlen((PCHAR)PadChar);    /* get the pad size also             */

  Length = STRLEN(this);               /* get the string length             */
  Count = 0;                           /* no words yet                      */
  WordSize = 0;                        /* no characters either              */
  Word = STRPTR(this);                 /* point to the string               */
                                       /* get the first word                */
  WordLength = DBCS_NextWord(&Word, &Length, &NextSite);

  while (WordLength) {                 /* loop until we reach target        */
    Count++;                           /* count the word                    */
    WordSize += WordLength;            /* add in the word length            */
    Word = NextSite;                   /* copy the start pointer            */
                                       /* get the next word                 */
    WordLength = DBCS_NextWord(&Word, &Length, &NextSite);
  }
  if (!Count)                          /* no words?                         */
    Retval = OREF_NULLSTRING;          /* this is a null string             */
  else {                               /* real words                        */
    Count--;                           /* step back one                     */
                                       /* get space for output              */
    Retval = raw_string(WordSize + Count * (Spaces * PadSize));
    Current = STRPTR(Retval);          /* point to output area              */

    Length = STRLEN(this);             /* recover the length                */
    Word = STRPTR(this);               /* point to the string               */
                                       /* get the first word                */
    WordLength = DBCS_NextWord(&Word, &Length, &NextSite);

    while (Count--) {                  /* loop for each word                */
                                       /* copy the word over                */
      memcpy(Current, Word, WordLength);
      Current += WordLength;           /* step over the word                */
      if (Spaces) {                    /* if have gaps...                   */
                                       /* fill in the pad chars             */
        DBCS_SetPadChar(Current, Spaces, PadChar);
        Current += Spaces * PadSize;   /* step over the pad chars           */
      }
      Word = NextSite;                 /* copy the start pointer            */
                                       /* get the next word                 */
      WordLength = DBCS_NextWord(&Word, &Length, &NextSite);
    }
                                       /* copy the word over                */
    memcpy(Current, Word, WordLength);
    Retval->generateHash();            /* done building the string          */
  }
  return Retval;                       /* return spaced string              */
}

/*********************************************************************/
/* Description: Left Built-in function.                              */
/*********************************************************************/

RexxString *RexxString::DBCSleft(RexxInteger *plength,
                                 RexxString *pad)
{
  PUCHAR   Scan;                       /* scanning pointer                  */
  PUCHAR   StringPtr;                  /* scanning pointer                  */
  size_t   Length;                     /* input string length               */
  size_t   ReqCharLen;                 /* Length of padded new.             */
  size_t   PadSize;                    /* size of pad character             */
  PUCHAR   PadChar;                    /* pad character pointer             */
  RexxString *Retval;                  /* return value                      */

  ValidDBCS(this);                     /* validate first argument           */
                                       /* get the substring length          */
  ReqCharLen = get_length(plength, ARG_ONE);
                                       /* get the padding character         */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
  PadSize = strlen((PCHAR)PadChar);    /* get the pad size too              */

  if (!ReqCharLen)                     /* request zero characters?          */
    Retval = OREF_NULLSTRING;          /* this is a null string             */
  else {                               /* extract characters                */
    StringPtr = STRPTR(this);          /* point to input string             */
    Scan = StringPtr;                  /* copy the pointer                  */
    Length = STRLEN(this);             /* get the length also               */
                                       /* step to the end                   */
    DBCS_IncChar(&Scan, &Length, &ReqCharLen);
    if (!ReqCharLen)                   /* that many available?              */
                                       /* copy the string part              */
      Retval = new_string((PCHAR)StringPtr, (Scan - StringPtr));
    else {                             /* need to pad                       */
      Length = STRLEN(this);           /* get string length                 */
                                       /* allocate the return area          */
      Retval = raw_string(Length + (ReqCharLen * PadSize));
                                       /* copy string part                  */
      memcpy(STRPTR(Retval), StringPtr, Length);
                                       /* fill pad characters               */
      DBCS_SetPadChar(STRPTR(Retval) + Length, ReqCharLen, PadChar);
      Retval->generateHash();          /* done building the string          */
    }
  }
  return Retval;                       /* Return extracted string           */
}

/*********************************************************************/
/* Description: Right Built-in function.                             */
/*********************************************************************/

RexxString *RexxString::DBCSright(RexxInteger *plength,
                                  RexxString *pad)
{
  PUCHAR   Scan;                       /* scanning pointer                  */
  size_t   Length;                     /* input string length               */
  size_t   ReqCharLen;                 /* Length of padded new.             */
  size_t   PadSize;                    /* size of pad character             */
  size_t   CharSize;                   /* character size of input           */
  PUCHAR   PadChar;                    /* pad character pointer             */
  RexxString *Retval;                  /* return value                      */
  size_t   Remain;                     /* remainder size                    */

  CharSize = ValidDBCS(this);          /* validate first argument           */

                                       /* get the substring length          */
  ReqCharLen = get_length(plength, ARG_ONE);
                                       /* get the padding character         */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
  PadSize = strlen((PCHAR)PadChar);    /* get the pad size too              */

  if (!ReqCharLen)                     /* request zero characters?          */
    Retval = OREF_NULLSTRING;          /* this is a null string             */
  else {
    if (CharSize == ReqCharLen)        /* exactly equal?                    */
      Retval = this;                   /* just return a copy                */
    else if (CharSize > ReqCharLen) {  /* input string is longer            */
      Scan = STRPTR(this);             /* copy the pointer                  */
      Length = STRLEN(this);           /* get length also                   */
      Remain = CharSize - ReqCharLen;  /* calculate offset position         */
                                       /* step to string start              */
      DBCS_IncChar(&Scan, &Length, &Remain);
                                       /* copy remaining portion            */
      Retval = new_string((PCHAR)Scan, Length);
    }
    else {                             /* need to pad                       */
      Length = STRLEN(this);           /* get input size                    */
      Remain = ReqCharLen - CharSize;  /* get number of pads needed         */
                                       /* allocate the return               */
      Retval = raw_string(Length + Remain * PadSize);
                                       /* fill pad characters               */
      DBCS_SetPadChar(STRPTR(Retval), Remain, PadChar);
                                       /* copy string part                  */
      memcpy(STRPTR(Retval) + (Remain * PadSize), STRPTR(this), Length);
      Retval->generateHash();          /* done building the string          */
    }
  }
  return  Retval;                      /* extracted string                  */
}

/*********************************************************************/
/* Description: Center Built-in function.                            */
/*********************************************************************/

RexxString *RexxString::DBCScenter(RexxInteger *plength,
                                   RexxString *pad)
{
  size_t   MainCharLen;                /* Character Length of main.         */
  PUCHAR   Scan;                       /* scanning pointer                  */
  PUCHAR   StringPtr;                  /* pointer to string start           */
  size_t   Length;                     /* input string length               */
  size_t   ReqCharLen;                 /* Length of padded new.             */
  size_t   PadSize;                    /* size of pad character             */
  PUCHAR   PadChar;                    /* pad character pointer             */
  RexxString *Retval;                  /* return value                      */
  size_t   Rlen;                       /* right pad size                    */
  size_t   Llen;                       /* left pad size                     */

  MainCharLen = ValidDBCS(this);       /* validate first argument           */
                                       /* get the substring length          */
  ReqCharLen = get_length(plength, ARG_ONE);
                                       /* get the padding character         */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
  PadSize = strlen((PCHAR)PadChar);    /* get the pad size too              */
                                       /* validate first argument           */
  if (!ReqCharLen)                     /* centered in 0 chars?              */
    Retval = OREF_NULLSTRING;          /* this is a null string             */
  else {                               /* have to really center             */
    if (MainCharLen == ReqCharLen)     /* if length is requested            */
      Retval = this;                   /* just return a copy                */
                                       /* shorter than request?             */
    else if (MainCharLen < ReqCharLen) {
                                       /* needs to be padded                */
                                       /* calculate pad amounts             */
      Llen = (ReqCharLen - MainCharLen) / 2;
      Rlen = (ReqCharLen - MainCharLen) - Llen;
      Length = STRLEN(this);           /* calculate result size             */
                                       /* allocate result string            */
      Retval = raw_string((Llen + Rlen) * PadSize + Length);
      Scan = STRPTR(Retval);           /* copy the pointer                  */
                                       /* fill leading pad characters       */
      DBCS_SetPadChar(Scan, Llen, PadChar);
      Scan += Llen * PadSize;          /* step over pads                    */
                                       /* copy input string                 */
      memcpy(Scan, STRPTR(this), Length);
      Scan += Length;                  /* step to last pad location         */
                                       /* fill trailing pad chars           */
      DBCS_SetPadChar(Scan, Rlen, PadChar);
      Retval->generateHash();          /* done building the string          */
    }
    else {                             /* needs truncation                  */
                                       /* calculate locations               */
      Llen = (MainCharLen - ReqCharLen) / 2;
      Rlen = (MainCharLen - ReqCharLen) - Llen;
                                       /* calculate the middle size         */
      Rlen = MainCharLen - (Llen + Rlen);
      Length = STRLEN(this);           /* calculate length of input string  */
      Scan = STRPTR(this);             /* copy input pointer                */
                                       /* step over leading part            */
      DBCS_IncChar(&Scan, &Length, &Llen);
      StringPtr = Scan;                /* copy position                     */
                                       /* step over middle part             */
      DBCS_IncChar(&Scan, &Length, &Rlen);
                                       /* copy the middle portion           */
      Retval = new_string((PCHAR)StringPtr, (Scan - StringPtr));
    }
  }
  return Retval;                       /* Return centered string            */
}

/*********************************************************************/
/* Description: Insert Built-in function.                            */
/*********************************************************************/

RexxString *RexxString::DBCSinsert(RexxString *newStr,
                                   RexxInteger *position,
                                   RexxInteger *plength,
                                   RexxString *pad)
{
  size_t   MainCharLen;                /* Character Length of main.         */
  size_t   NewCharLen;                 /* Character Length of new           */
  size_t   StartPos;                   /* Character Length of new           */
  PUCHAR   Scan;                       /* scanning pointer                  */
  PUCHAR   Target;                     /* pointer to string start           */
  size_t   Length;                     /* input string length               */
  size_t   ReqCharLen;                 /* Length of padded new.             */
  size_t   PadSize;                    /* size of pad character             */
  PUCHAR   PadChar;                    /* pad character pointer             */
  RexxString *Retval;                  /* return value                      */
  size_t   TargetSize;                 /* size of target string             */
  size_t   NewSize;                    /* size of new string                */
  size_t   ReqLeadPad;                 /* leading pad characters            */
  size_t   ReqPadChar;                 /* trailing pad characters           */
  size_t   FCharLen;                   /* front character length            */
  size_t   BCharLen;                   /* back character length             */
  size_t   BuffSize;                   /* size of return string             */

  MainCharLen = ValidDBCS(this);       /* validate the string               */
                                       /* validate the new string           */
  newStr = RequiredArg(newStr, &NewCharLen, ARG_ONE);
                                       /* use optional_length for starting  */
                                       /* position becase a value of 0 IS   */
                                       /* valid for INSERT                  */
  StartPos = optional_length(position, 0, ARG_TWO);
                                       /* get the optional length, using the*/
                                       /* needle length as the defaul       */
  ReqCharLen = optional_length(plength, NewCharLen, ARG_THREE);
                                       /* get the padding character         */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
  PadSize = strlen((PCHAR)PadChar);    /* get the pad size too              */
  TargetSize = STRLEN(this);           /* get target byte size              */

  if (StartPos == 0) {                 /* inserting at the front?           */
    ReqLeadPad = 0;                    /* no leading pads                   */
    FCharLen = 0;                      /* no front part                     */
    BCharLen = MainCharLen;            /* trailer is entire target          */
  }
  else if (StartPos >= MainCharLen) {  /* need leading pads?                */
                                       /* calculate needed                  */
    ReqLeadPad = (StartPos - MainCharLen);
    FCharLen = MainCharLen;            /* front part is all                 */
    BCharLen = 0;                      /* trailer is nothing                */
  }
  else {                               /* have a split                      */
    ReqLeadPad = 0;                    /* no leading pad                    */
    FCharLen = StartPos;               /* NChar front chars                 */
    BCharLen = MainCharLen - StartPos; /* and some trailer too              */
  }
                                       /* truncate new, if needed           */
  NewCharLen = min(NewCharLen, ReqCharLen);
                                       /* get new byte size                 */
  NewSize = DBCS_ByteLen(STRDESC(newStr), NewCharLen);
  ReqPadChar = ReqCharLen - NewCharLen;/* calculate pad chars               */
                                       /* calculate result size             */
  BuffSize = NewSize + TargetSize + ((ReqPadChar + ReqLeadPad) * PadSize);
  Retval = raw_string(BuffSize);       /* allocate the result               */

  Scan = STRPTR(Retval);               /* point to start                    */
  Target = STRPTR(this);               /* point to target start             */

  if (FCharLen) {                      /* have leading chars                */
                                       /* get the byte length               */
    Length = DBCS_ByteLen(Target, STRLEN(this), FCharLen);
    memcpy(Scan, Target, Length);      /* copy the characters               */
    Scan += Length;                    /* step output pointer               */
    Target += Length;                  /* and input pointer                 */
  }

  if (ReqLeadPad) {                    /* if required leading pads          */
                                       /* fill in the pads                  */
    DBCS_SetPadChar(Scan, ReqLeadPad, PadChar);
    Scan += ReqLeadPad * PadSize;      /* step output pointer               */
  }

  if (NewSize) {                       /* new string to copy?               */
                                       /* copy the new part                 */
    memcpy(Scan, STRPTR(newStr), NewSize);
    Scan += NewSize;                   /* step to next position             */
  }

  if (ReqPadChar) {                    /* if required trailing pads         */
                                       /* fill in the pads                  */
    DBCS_SetPadChar(Scan, ReqPadChar, PadChar);
    Scan += ReqPadChar * PadSize;      /* step output pointer               */
  }

  if (BCharLen) {                      /* have trailing chars               */
                                       /* copy the data                     */
    memcpy(Scan, Target, (STRLEN(this) - (Target - STRPTR(this))));
  }
  Retval->generateHash();              /* done building the string          */
  return Retval;                       /* Return inserted string            */
}

/*********************************************************************/
/* Description: Overlay Built-in function.                           */
/*********************************************************************/

RexxString *RexxString::DBCSoverlay(RexxString *newStr,
                                    RexxInteger *position,
                                    RexxInteger *plength,
                                    RexxString *pad)
{
  size_t   MainCharLen;                /* Character Length of main.         */
  size_t   NewCharLen;                 /* Character Length of new           */
  size_t   StartPos;                   /* Character Length of new           */
  PUCHAR   Scan;                       /* scanning pointer                  */
  size_t   ReqCharLen;                 /* Length of padded new.             */
  size_t   PadSize;                    /* size of pad character             */
  PUCHAR   PadChar;                    /* pad character pointer             */
  RexxString *Retval;                  /* return value                      */
  size_t   NewSize;                    /* size of new string                */
  size_t   PadFNum;                    /* leading pad characters            */
  size_t   PadBNum;                    /* trailing pad characters           */
  size_t   FCharLen;                   /* front character length            */
  size_t   BCharLen;                   /* back character length             */
  size_t   BuffSize;                   /* size of return string             */
  PUCHAR   BackPtr;                    /* point to back part                */
  size_t   FrontSize;                  /* byte size of front part           */
  size_t   BackSize;                   /* byte length of back part          */

  MainCharLen = ValidDBCS(this);       /* validate the string               */
                                       /* validate the new string           */
  newStr = RequiredArg(newStr, &NewCharLen, ARG_ONE);
                                       /* use optional_length for starting  */
                                       /* position, 1 will be the default   */
                                       /* value                             */
  StartPos = optional_position(position, 1, ARG_TWO) - 1;
                                       /* get the optional length, using the*/
                                       /* needle length as the default      */
  ReqCharLen = optional_length(plength, NewCharLen, ARG_THREE);
                                       /* get the padding character         */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
  PadSize = strlen((PCHAR)PadChar);    /* get the pad size too              */
                                       /* now truncate NCharLen             */
  NewCharLen = min(NewCharLen, ReqCharLen);
                                       /* number of front chars             */
  FCharLen = min(StartPos, MainCharLen);
  if (FCharLen)                        /* have front characters?            */
                                       /* get string byte size              */
    FrontSize = DBCS_ByteLen(STRDESC(this), FCharLen);
  else                                 /* yes, front part                   */
    FrontSize = 0;                     /* no front part                     */
                                       /* calculate back portion size       */
  if (FCharLen + ReqCharLen < MainCharLen) {
    BCharLen = FCharLen + ReqCharLen;
    BackPtr = STRPTR(this);            /* get front pointer                 */
    BackSize = STRLEN(this);           /* get string length                 */
                                       /* step to location                  */
    DBCS_IncChar(&BackPtr, &BackSize, &BCharLen);
  }
  else {
    BCharLen = 0;                      /* no back portion needed            */
    BackSize = 0;                      /* no size needed                    */
  }

  if (StartPos > MainCharLen)          /* start past end of target?         */
    PadFNum = StartPos - MainCharLen;  /* need front padding                */
  else
    PadFNum = 0;                       /* no padding needed                 */

  if (NewCharLen < ReqCharLen)         /* need back padding too?            */
    PadBNum = ReqCharLen - NewCharLen; /* calculate the size                */
  else
    PadBNum = 0;                       /* no padding needed                 */

                                       /* get new byte size                 */
  NewSize = DBCS_ByteLen(STRDESC(newStr), NewCharLen);
                                       /* calculate result size             */
  BuffSize = NewSize + FrontSize + BackSize + ((PadFNum + PadBNum) * PadSize);
  Retval = raw_string(BuffSize);       /* allocate the result string        */

  Scan = STRPTR(Retval);               /* point to start                    */

  if (FCharLen) {                      /* have leading chars                */
                                       /* copy the front part               */
    memcpy(Scan, STRPTR(this), FrontSize);
    Scan += FrontSize;                 /* step to next position             */
  }

  if (PadFNum) {                       /* if required leading pads          */
                                       /* fill in the pads                  */
    DBCS_SetPadChar(Scan, PadFNum, PadChar);
    Scan += PadFNum * PadSize;         /* step output pointer               */
  }

  if (NewSize) {                       /* new string to copy?               */
                                       /* copy the new part                 */
    memcpy(Scan, STRPTR(newStr), NewSize);
    Scan += NewSize;                   /* step to next position             */
  }

  if (PadBNum) {                       /* if required trailing pads         */
                                       /* fill in the pads                  */
    DBCS_SetPadChar(Scan, PadBNum, PadChar);
    Scan += PadBNum * PadSize;         /* step output pointer               */
  }

  if (BackSize)                        /* have trailing chars               */
    memcpy(Scan, BackPtr, BackSize);   /* copy the trailing part            */
  Retval->generateHash();              /* done building the string          */
  return Retval;                       /* Return overlayed string           */
}

/*********************************************************************/
/* Description: Verify Built-in function.                            */
/*********************************************************************/

RexxInteger *RexxString::DBCSverify(RexxString *ref,
                                    RexxString *option,
                                    RexxInteger *pstart)
{
  size_t   CPos;                       /* Character position.               */
  PUCHAR   CStr;                       /* scan pointer                      */
  PUCHAR   EndStr;                     /* end pointer                       */
  size_t   StartPos;                   /* Start position.                   */
  size_t   Length;                     /* string length                     */
  RexxInteger *Retval;                 /* returned value                    */
  INT      Option;                     /* verify option                     */

  ValidDBCS(this);                     /* validate string and               */
                                       /* reference string                  */
  ref = RequiredArg(ref, &Length, ARG_ONE);
                                       /* get the option, default 'Nomatch' */
  Option = option_character(option, VERIFY_NOMATCH, ARG_TWO);
  if (Option != VERIFY_MATCH &&        /* options are 'Match' and           */
      Option != VERIFY_NOMATCH)        /* 'NoMatch'                         */
                                       /* not that either, then its an error*/
    report_exception2(Error_Incorrect_method_option, new_string("MN", 2), option);

                                       /* get starting position             */
  StartPos = optional_position(pstart, 1, ARG_THREE) - 1;
  CStr = STRPTR(this);                 /* copy search string pointer        */
  Length = STRLEN(this);               /* get input length                  */
  CPos = StartPos;                     /* start character position          */
  Retval = IntegerZero;                /* default to not found              */
  if (StartPos)                        /* have a start position?            */
    DBCS_IncChar(&CStr, &Length, &StartPos);

  if (!Length)                         /* all used up?                      */
    Retval = IntegerZero;              /* nothing found                     */
  else {
    EndStr = CStr + Length;            /* point to end                      */

    while (CStr < EndStr) {            /* check entire string               */
      CPos++;                          /* step char position                */
                                       /* search the string                 */
      if (DBCS_MemChar(CStr, STRDESC(ref))) {
        if (Option == VERIFY_MATCH) {  /* is this match?                    */
          Retval = new_integer(CPos);  /* format return postion             */
          break;                       /* leave                             */
        }
      }
      else {                           /* wasn't found                      */
                                       /* this what we want?                */
        if ((Option == VERIFY_NOMATCH)) {
          Retval = new_integer(CPos);  /* format return postion             */
          break;                       /* leave                             */
        }
      }
                                       /* step the correct number           */
      if (!IsDBCS(*CStr))              /* SBCS?                             */
        CStr++;                        /* step one character                */
      else
        CStr += DBCS_BYTELEN;          /* step a DBCS character             */
    }
  }
  return Retval;                       /* Return match position             */
}

/*********************************************************************/
/* Description: Words Built-in function.                             */
/*********************************************************************/

RexxInteger *RexxString::DBCSwords()
{
  size_t tempCount;

  ValidDBCS(this);                     /* validate the string               */
                                       /* place holder to invoke new_integer*/
  tempCount = DBCS_WordLen(STRDESC(this));
  return new_integer(tempCount);       /* return count of words             */
}

/*********************************************************************/
/* Description: Pos Built-in function.                               */
/*********************************************************************/

size_t RexxString::DBCSpos(RexxString *needle,
                         size_t      StartPos)
{
  PUCHAR   Haystack;                   /* haystack pointer                  */
  PUCHAR   Needle;                     /* needle pointer                    */
  size_t   CharStart;                  /* Start position.                   */
  size_t   Position;                   /* current search position           */
  size_t   NeedleLength;               /* length of needle                  */
  size_t   HaystackLength;             /* length of haystack                */
  PUCHAR   End;                        /* Last end position                 */

  ValidDBCS(this);                     /* validate both haystack...         */
                                       /* and the needle                    */
  needle = RequiredArg(needle, &NeedleLength, ARG_ONE);

  Needle = STRPTR(needle);             /* get Needle pointer                */
  NeedleLength = STRLEN(needle);       /* and length                        */
  Haystack = STRPTR(this);             /* also get haystack pointer         */
  HaystackLength = STRLEN(this);       /* and length                        */

  CharStart = StartPos;                /* set start position                */
  StartPos++;                          /* take start position back 1 origin */
                                       /* and advance the pointer           */
  DBCS_IncChar(&Haystack, &HaystackLength, &CharStart);

  if (NeedleLength > HaystackLength || /* no match possible?                */
      NeedleLength == 0)
    CharStart = 0;                     /* return a zero                     */
  else {
                                       /* set end marker                    */
    End = Haystack + HaystackLength - NeedleLength + 1;

    Position = StartPos;               /* set search position               */
    CharStart = 0;                     /* set default match                 */
    while (Haystack < End) {           /* while the string is found         */
                                       /* search the string                 */
                                       /* check this position               */
      if (!memcmp(Haystack, Needle, NeedleLength)) {
                                       /* return hit position               */
        CharStart = Position;          /* remember position                 */
        break;                         /* we've got what we want            */
      }
      if (IsDBCS(*Haystack))           /* DBCS character?                   */
        Haystack += DBCS_BYTELEN;      /* step appropriate length           */
      else
        Haystack++;                    /* No, step our search pointer       */
      Position++;                      /* update the char position          */
    }
  }
  return CharStart;                    /* Return match position             */
}

/*********************************************************************/
/* Description: DBCS caseless search primitive function              */
/*********************************************************************/

size_t RexxString::DBCScaselessPos(RexxString *needle,
                                 size_t      StartPos)
{
  PUCHAR   Haystack;                   /* haystack pointer                  */
  PUCHAR   Needle;                     /* needle pointer                    */
  size_t   CharStart;                  /* Start position.                   */
  size_t   Position;                   /* current search position           */
  size_t   NeedleLength;               /* length of needle                  */
  size_t   HaystackLength;             /* length of haystack                */
  PUCHAR   End;                        /* Last end position                 */

  ValidDBCS(this);                     /* validate both haystack...         */
                                       /* and the needle                    */
  needle = RequiredArg(needle, &NeedleLength, ARG_ONE);
  StartPos++;                          /* take start position back 1 origin */

  Haystack = STRPTR(needle);           /* get haystack pointer              */
  HaystackLength = STRLEN(needle);     /* and length                        */
  Needle = STRPTR(this);               /* also get needle pointer           */
  NeedleLength = STRLEN(this);         /* and length                        */

  CharStart = StartPos;                /* set start position                */
  StartPos++;                          /* take start position back 1 origin */
                                       /* and advance the pointer           */
  DBCS_IncChar(&Haystack, &HaystackLength, &CharStart);

  if (NeedleLength > HaystackLength || /* no match possible?                */
      NeedleLength == 0)
    CharStart = 0;                     /* return a zero                     */
  else {
                                       /* set end marker                    */
    End = Haystack + HaystackLength - NeedleLength + 1;

    Position = StartPos;               /* set search position               */
    CharStart = 0;                     /* set default match                 */
    while (Haystack < End) {           /* while the string is found         */
                                       /* search the string                 */
                                       /* check this position               */
      if (!DBCS_CaselessCompare(Haystack, Needle, NeedleLength)) {
                                       /* return hit position               */
        CharStart = Position;          /* remember position                 */
        break;                         /* we've got what we want            */
      }
      if (IsDBCS(*Haystack))           /* DBCS character?                   */
        Haystack += DBCS_BYTELEN;      /* step appropriate length           */
      else
        Haystack++;                    /* No, step our search pointer       */
      Position++;                      /* update the char position          */
    }
  }
  return CharStart;                    /* Return match position             */
}

/*********************************************************************/
/* Description: Lastpos Built-in function.                           */
/*********************************************************************/

RexxInteger *RexxString::DBCSlastPos(RexxString *needle,
                                    RexxInteger *pstart)
{
  PUCHAR   Haystack;                   /* pointer for haystack              */
  size_t   LastMatched;                /* last match position               */
  size_t   CharStart;                  /* Start position.                   */
  size_t   EndOffSet;                  /* Last possible match pos           */
  size_t   Position;                   /* current search position           */
  size_t   NeedleLength;               /* length of needle                  */
  size_t   HaystackLength;             /* length of haystack                */
  PUCHAR   Needle;                     /* needle value                      */
  PUCHAR   Current;                    /* current compare location          */
  PUCHAR   End;                        /* Last end position                 */
  RexxInteger *Retval;                 /* function return value             */

  HaystackLength = ValidDBCS(this);    /* validate both haystack...         */
                                       /* and the needle                    */
  needle = RequiredArg(needle, &NeedleLength, ARG_ONE);
  NeedleLength = STRLEN(needle);       /* get pointers and lengths          */
  Needle = STRPTR(needle);
  Haystack = STRPTR(this);
                                       /* get the starting position         */
  CharStart = optional_position(pstart, HaystackLength, ARG_TWO);

  Current = Haystack;                  /* copy the pointer                  */
  EndOffSet = STRLEN(this);            /* copy the length                   */
                                       /* and advance the pointer           */
  DBCS_IncChar(&Current, &EndOffSet, &CharStart);
                                       /* find last position                */
  EndOffSet = (Current - Haystack);
  EndOffSet -= NeedleLength;           /* back up by needle length          */
                                       /* no match possible?                */
  if (NeedleLength > HaystackLength || NeedleLength == 0)
    Retval = IntegerZero;              /* return a zero                     */
  else {                               /* need to search                    */
    End = Haystack + EndOffSet + 1;    /* set end marker                    */

    LastMatched = 0;                   /* set initial match position        */
    Position = 1;                      /* set search position               */
    Current = Haystack;                /* copy the pointer                  */

    while (Current < End) {            /* while the string is found         */
                                       /* search the string                 */
                                       /* check this position               */
      if (!memcmp(Current, Needle, NeedleLength))
        LastMatched = Position;        /* remember this spot                */
      if (IsDBCS(*Current))            /* DBCS character?                   */
        Current += DBCS_BYTELEN;       /* step appropriate length           */
      else
        Current++;                     /* No, step our search pointer       */
      Position++;                      /* update the char position          */
    }
    Retval = new_integer(LastMatched); /* return last hit position          */
  }
  return Retval;                       /* return match position             */
}

/*********************************************************************/
/* Description: Wordindex Built-in function.                         */
/*********************************************************************/

RexxInteger *RexxString::DBCSwordIndex(RexxInteger *position)
{
  size_t     Length;                   /* length of the string              */
  size_t     Count;                    /* required word count               */
  PUCHAR     NextSite;                 /* next word                         */
  PUCHAR     Word;                     /* word pointer                      */
  size_t     WordLength;               /* length of word                    */
  RexxInteger *Retval;                 /* function return value             */
  size_t     tempCount;

  ValidDBCS(this);                     /* validate the string               */
  Word = STRPTR(this);                 /* get pointer to string             */
  Length = STRLEN(this);               /* length of arg string              */
                                       /* convert count to binary           */
  Count = get_position(position , ARG_ONE);

  if (!Length)                         /* if null input string              */
    Retval = IntegerZero;              /* so return a null string           */
  else {                               /* otherwise output something        */
                                       /* get the first word                */
    WordLength = DBCS_NextWord(&Word, &Length, &NextSite);
    while (--Count && WordLength) {    /* loop until we reach target        */
      Word = NextSite;                 /* copy the start pointer            */
                                       /* get the next word                 */
      WordLength = DBCS_NextWord(&Word, &Length, &NextSite);
    }
                                       /* so output position of word        */
    if (!WordLength)                   /* past the end?                     */
      Retval = IntegerZero;            /* this is zero                      */
    else {                                                                                                        /* else give the offset              */
                                       /* return a real count               */
      tempCount = DBCS_CharacterCount(STRPTR(this), (Word - STRPTR(this))) + 1;
      Retval = new_integer(tempCount); /* as an integer                     */
    }
  }
  return Retval;                       /* normal the index                  */
}

/*********************************************************************/
/* Description: Wordlength Built-in function.                        */
/*********************************************************************/

RexxInteger *RexxString::DBCSwordLength(RexxInteger *position)
{
  size_t     Length;                   /* length of the string              */
  size_t     Count;                    /* required word count               */
  PUCHAR     NextSite;                 /* next word                         */
  PUCHAR     Word;                     /* word pointer                      */
  size_t     WordLength;               /* length of word                    */
  RexxInteger *Retval;                 /* function return value             */
  size_t     tempCount;

  ValidDBCS(this);                     /* validate the string               */
  Word = STRPTR(this);                 /* get pointer to string             */
  Length = STRLEN(this);               /* length of arg string              */
                                       /* convert count to binary           */
  Count = get_position(position , ARG_ONE);

  if (!Length)                         /* if null input string              */
    Retval = IntegerZero;              /* so return a null string           */
  else {                               /* otherwise output something        */
                                       /* get the first word                */
    WordLength = DBCS_NextWord(&Word, &Length, &NextSite);
    while (--Count && WordLength) {    /* loop until we reach target        */
      Word = NextSite;                 /* copy the start pointer            */
                                       /* get the next word                 */
      WordLength = DBCS_NextWord(&Word, &Length, &NextSite);
    }
    if (!WordLength)                   /* past the end?                     */
      Retval = IntegerZero;            /* this is zero                      */
    else {                             /* else give the length              */
                                       /* temporary place holder to invoke  */
                                       /*new_integer                        */
      tempCount = DBCS_CharacterCount(Word, WordLength);
      Retval = new_integer(tempCount); /* now return as an integer          */
    }
  }
  return Retval;                       /* normal the index                  */
}

/*********************************************************************/
/* Description: Word Built-in function.                              */
/*********************************************************************/

RexxString *RexxString::DBCSword(RexxInteger *position)
{
  PUCHAR      Word;                    /* current word pointer              */
  PUCHAR      NextSite;                /* next word                         */
  size_t      Count;                   /* needed word position              */
  size_t      Length;                  /* remaining length                  */
  size_t      WordLength;              /* word size                         */
  RexxString *Retval;                  /* return value                      */

  ValidDBCS(this);                     /* validate the string               */
                                       /* convert count to binary           */
  Count = get_position(position , ARG_ONE);
  Word = STRPTR(this);                 /* get string pointer                */
  Length = STRLEN(this);               /* get length of string              */

  if (!Length)                         /* if null input string              */
    Retval = OREF_NULLSTRING;          /* so return a null string           */
  else {                               /* otherwise output something        */
                                       /* get the first word                */
    WordLength = DBCS_NextWord(&Word, &Length, &NextSite);
    while (--Count && WordLength) {    /* loop until we reach target        */
      Word = NextSite;                 /* copy the start pointer            */
                                       /* get the next word                 */
      WordLength = DBCS_NextWord(&Word, &Length, &NextSite);
    }
    if (WordLength)                    /* have a word                       */
                                       /* extract the string                */
      Retval = new_string((PCHAR)Word, WordLength);
    else
      Retval = OREF_NULLSTRING;        /* no word, return a null            */
  }
  return Retval;                       /* return extracted string           */
}

/*********************************************************************/
/* Description: Translate Built-in function.                         */
/*********************************************************************/

RexxString *RexxString::DBCStranslate(RexxString *tableo,
                                      RexxString *tablei,
                                      RexxString *pad)
{
  PUCHAR   PadChar;                    /* Padding character                 */
  PUCHAR   OutTable;                   /* Output table.                     */
  PUCHAR   InTable;                    /* Input table.                      */
  size_t   PadSize;                    /* size of pad character             */
  PUCHAR   InputStr;                   /* input pointer                     */
  PUCHAR   EndStr;                     /* end of input string               */
  size_t   InputLength;                /* length of input string            */
  RexxString *Retval;                  /* returned string                   */
  PUCHAR   CStr;                       /* output pointer                    */
  size_t   OldCharLen;                 /* length of translated char         */
  size_t   BPos;                       /* position in translate table       */
  PUCHAR   OutPtr;                     /* output table position             */
  size_t   OutByteLength;              /* output byte length                */
  size_t   TempLength;                 /* temporary output length           */
  size_t   InByteLength;               /* input byte length                 */
  size_t   OutTableLength;             /* output table length in characters */
  size_t   InTableLength;              /* input table length in characters  */

  ValidDBCS(this);                     /* validate first argument           */

                                       /* just a simple uppercase?          */
  if (tableo == OREF_NULL && tablei == OREF_NULL && pad == OREF_NULL) {
                                       /* copy the argument                 */
    Retval = (RexxString *)this->copy();
                                       /* uppercase the string              */
    DBCS_MemUpper(STRPTR(Retval), STRLEN(Retval));
  }
  else {                               /* translation to perform            */
                                       /* validate the tables               */
    tableo = OptionalArg(tableo, OREF_NULLSTRING, &OutTableLength, ARG_ONE);
    tablei = OptionalArg(tablei, OREF_NULLSTRING, &InTableLength, ARG_TWO);
    OutByteLength = STRLEN(tableo);    /* get byte length                   */
    InByteLength = STRLEN(tablei);     /* get table size                    */
    OutTable = STRPTR(tableo);         /* copy the pointer                  */
    InTable = STRPTR(tablei);          /* copy the pointer                  */
                                       /* get the padding character         */
    PadChar = ValidatePad(pad, (const PUCHAR)" ");
    PadSize = strlen((PCHAR)PadChar);  /* get the pad size too              */
    InputStr = STRPTR(this);           /* copy the pointer                  */
    InputLength = STRLEN(this);        /* get the length                    */
    EndStr = InputStr + InputLength;   /* set the end pointer               */

    if (!InputLength)                  /* Null input?                       */
      Retval = OREF_NULLSTRING;        /* return a null string              */
    else {
                                       /* allow for worst case, each        */
                                       /* byte increased to 2 bytes         */
      Retval = raw_string(InputLength * 2);
      CStr = STRPTR(Retval);           /* get the copy location             */

      while (InputStr < EndStr) {      /* Loop through input string         */
        if (IsDBCS(*InputStr))         /* is first character DBCS?          */
          OldCharLen = DBCS_BYTELEN;   /* set the byte length               */
        else
          OldCharLen = SBCS_BYTELEN;   /* First character is SBCS.          */
        if (tablei == OREF_NULL) {     /* Omitted input table?              */
                                       /*  SBCS, use ASCII char code        */
          if (OldCharLen == SBCS_BYTELEN) {
            BPos = (UCHAR)*InputStr;   /* InTable is character code         */
            BPos++;                    /* make origin 1                     */
          }
          else
            BPos = 0;                  /* DBCS not in default InTable       */
        }
        else                           /* have an input table               */
                                       /* search for the character          */
          BPos = DBCS_MemChar(InputStr, InTable, InByteLength);
        if (!BPos) {                   /* Not found?                        */
                                       /* copy character directly           */
          memcpy(CStr, InputStr, OldCharLen);
          CStr += OldCharLen;          /* step past character               */
        }
        else {                         /* Get character from output         */
                                       /* table that is same                */
                                       /* character position with           */
                                       /* input table.                      */
          if (BPos > OutTableLength) { /* out side of table?                */
                                       /* use the pad character             */
            memcpy(CStr, PadChar, PadSize);
            CStr += PadSize;           /* step output position              */
          }
          else {                       /* extract from the table            */
            BPos--;                    /* make origin 0                     */
            OutPtr = OutTable;         /* get front of the table            */
                                       /* copy the length                   */
            TempLength = OutByteLength;/* step to needed character          */
            DBCS_IncChar(&OutPtr, &TempLength, &BPos);
            if (IsDBCS(*OutPtr)) {     /* is this DBCS?                     */
                                       /* copy two characters               */
              memcpy(CStr, OutPtr, DBCS_BYTELEN);
              CStr += DBCS_BYTELEN;    /* step past the two                 */
            }
            else {
              *CStr = *OutPtr;         /* copy one character                */
              CStr++;                  /* step the pointer                  */
            }
          }
        }
        InputStr += OldCharLen;        /* step around                       */
      }
      Retval->generateHash();          /* done building the string          */
                                       /* produce final result string       */
      Retval = new_string((PCHAR)STRPTR(Retval), CStr - STRPTR(Retval));
    }
  }
  return Retval;                       /* Return translated string          */
}

/*********************************************************************/
/* Description: DBLeft DBCS unique routine.                          */
/*********************************************************************/

RexxString *RexxString::dbLeft(RexxInteger *plength,
                               RexxString *pad,
                               RexxString *option)
{
  PUCHAR       PadChar;                /* pad character                     */
  size_t       PadSize;                /* size of pad character             */
  PUCHAR       DBCSPad;                /* DBCS pad character                */
  PUCHAR       SBCSPad;                /* SBCS pad character                */
  size_t       DBCSPadNum;             /* number of DBCS pads               */
  size_t       SBCSPadNum;             /* number of SBCS pads               */
  size_t       ReqBytes;               /* requested bytes                   */
  PUCHAR       String;                 /* input string pointer              */
  size_t       Length;                 /* string length                     */
  UCHAR        Option;                 /* specified option                  */
  size_t       RemBytes;               /* remainder bytes                   */
  RexxString  *Retval;                 /* function return value             */

  ValidDBCS(this);                     /* validate the string               */
                                       /* get the substring length          */
  ReqBytes = get_length(plength, ARG_ONE);
                                       /* get the padding character         */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
  PadSize = strlen((PCHAR)PadChar);    /* get the pad size too              */
                                       /* get the option character          */
  Option = option_character(option, DBCS_YES, ARG_THREE);
                                       /* must be a valid option            */
  if (Option != DBCS_YES && Option != DBCS_NO)
    report_exception2(Error_Incorrect_method_option, new_string("NY", 2), option);

  if (PadSize == SBCS_BYTELEN)         /* single byte pad?                  */
    SBCSPad = PadChar;                 /* set SBCS pad.                     */
  else {                               /* Padding char is DBCS,             */
    DBCSPad = PadChar;                 /* Set DBCS padd.                    */
    SBCSPad = (PUCHAR)" ";             /* Set Default pad as SBCS           */
  }
  if (ReqBytes == 0)                   /* requested null?                   */
    Retval = OREF_NULLSTRING;          /* this is easy                      */
  else {
    String = STRPTR(this);             /* copy start pointer                */
    Length = STRLEN(this);             /* get string length                 */
    RemBytes = ReqBytes;               /* copy size                         */
                                       /* move requested bytes              */
    DBCS_IncByte(&String, &Length, &RemBytes);
    ReqBytes -= RemBytes;              /* adjust extracted size             */

    if (RemBytes > 0) {                /* do we need to pad?                */
      if (PadSize == SBCS_BYTELEN) {   /* single byte pad?                  */
        SBCSPadNum = RemBytes;         /* Set SBCS pad length.              */
        DBCSPadNum = 0;                /* And DBCS is zero.                 */
      }
      else {                           /* When Pad char is DBCS,            */
                                       /* calculate DBCS pad and            */
        DBCSPadNum = RemBytes / DBCS_BYTELEN;
                                       /* SBCS padding                      */
        SBCSPadNum = RemBytes % DBCS_BYTELEN;
      }
    }
    else {
      SBCSPadNum = 0;                  /* no padding needed                 */
      DBCSPadNum = 0;
    }
                                       /* get the return value              */
    Retval = raw_string(ReqBytes + RemBytes);
                                       /* copy string part                  */
    memcpy(STRPTR(Retval), STRPTR(this), ReqBytes);

    if (DBCSPadNum > 0)                /* DBCS padding?                     */
                                       /* add them                          */
      DBCS_SetPadChar(STRPTR(Retval) + ReqBytes, DBCSPadNum, DBCSPad);

    if (SBCSPadNum > 0)                /* SBCS padding?                     */
      DBCS_SetPadChar(STRPTR(Retval) + ReqBytes + (DBCSPadNum * DBCS_BYTELEN), SBCSPadNum, SBCSPad);
    Retval->generateHash();            /* done building the string          */
  }
  return Retval;                       /* return extracted string           */
}

/*********************************************************************/
/* Description: DBRight DBCS unique routine.                         */
/*********************************************************************/

RexxString *RexxString::dbRight(RexxInteger *plength,
                                RexxString *pad,
                                RexxString *option)
{
  PUCHAR       PadChar;                /* pad character                     */
  PUCHAR       DBCSPad;                /* DBCS pad character                */
  PUCHAR       SBCSPad;                /* SBCS pad character                */
  size_t       DBCSPadNum;             /* number of DBCS pads               */
  size_t       SBCSPadNum;             /* number of SBCS pads               */
  size_t       ReqBytes;               /* requested bytes                   */
  size_t       PadSize;                /* size of pad character             */
  PUCHAR       String;                 /* input string pointer              */
  size_t       Length;                 /* string length                     */
  UCHAR        Option;                 /* specified option                  */
  size_t       RemBytes;               /* remainder bytes                   */
  RexxString  *Retval;                 /* function return value             */

  ValidDBCS(this);                     /* validate the string               */
                                       /* get the substring length          */
  ReqBytes = get_length(plength, ARG_ONE);
                                       /* get the padding character         */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
  PadSize = strlen((PCHAR)PadChar);    /* get the pad size too              */
                                       /* get the option character          */
  Option = option_character(option, DBCS_YES, ARG_THREE);
                                       /* must be a valid option            */
  if (Option != DBCS_YES && Option != DBCS_NO)
    report_exception2(Error_Incorrect_method_option, new_string("NY", 2), option);

  if (PadSize == SBCS_BYTELEN)         /* single byte pad?                  */
    SBCSPad = PadChar;                 /* set SBCS pad.                     */
  else {                               /* Padding char is DBCS,             */
    DBCSPad = PadChar;                 /* Set DBCS padd.                    */
    SBCSPad = (PUCHAR)" ";             /* Set Default pad as SBCS           */
  }
  if (ReqBytes == 0)                   /* requested null?                   */
    Retval = OREF_NULLSTRING;          /* this is easy                      */
  else {
    String = STRPTR(this);             /* copy start pointer                */
    Length = STRLEN(this);             /* get string length                 */
    if (ReqBytes > Length) {           /* longer than string?               */
      RemBytes = ReqBytes - Length;    /* get remainder part                */
      ReqBytes = Length;               /* adjust string part                */
    }
    else {
      RemBytes = Length - ReqBytes;    /* target move amount                */
                                       /* move requested bytes              */
      DBCS_IncByte(&String, &Length, &RemBytes);
      if (RemBytes) {                  /* split character?                  */
        RemBytes = 1;                  /* need one pad char                 */
        ReqBytes--;                    /* decrement length                  */
        String += DBCS_BYTELEN;        /* step over DBCS character          */
      }
    }
    if (RemBytes > 0) {                /* do we need to pad?                */
      if (PadSize == SBCS_BYTELEN) {   /* SBCS padding?                     */
        SBCSPadNum = RemBytes;         /* Set SBCS pad length.              */
        DBCSPadNum = 0;                /* And DBCS is zero.                 */
      }
      else {                           /* When Pad char is DBCS,            */
                                       /* calculate DBCS pad and            */
        DBCSPadNum = RemBytes / DBCS_BYTELEN;
                                       /* SBCS padding                      */
        SBCSPadNum = RemBytes % DBCS_BYTELEN;
      }
    }
    else {                             /* Need not to pad                   */
      SBCSPadNum = 0;
      DBCSPadNum = 0;
    }
                                       /* get the return value              */
    Retval = raw_string(ReqBytes + RemBytes);
    if (SBCSPadNum)                    /* SBCS padding?                     */
                                       /* fill in SBCS padding              */
      DBCS_SetPadChar(STRPTR(Retval), SBCSPadNum, SBCSPad);
    if (DBCSPadNum)                    /* DBCS padding?                     */
                                       /* fill in DBCS padding              */
      DBCS_SetPadChar(STRPTR(Retval) + SBCSPadNum, DBCSPadNum, DBCSPad);
                                       /* now copy string part              */
    memcpy(STRPTR(Retval) + SBCSPadNum + (DBCSPadNum * DBCS_BYTELEN), String, ReqBytes);
    Retval->generateHash();            /* done building the string          */
  }
  return Retval;                       /* return extracted string           */
}

/*********************************************************************/
/* Description: DBCenter DBCS unique routine.                        */
/*********************************************************************/

RexxString *RexxString::dbCenter(RexxInteger *plength,
                                 RexxString *pad,
                                 RexxString *option)
{
  PUCHAR   PadChar;                    /* Pad character                     */
  size_t   PadSize;                    /* size of pad character             */
  size_t   DBCSLPadNum;                /* left side DBCS padding            */
  size_t   SBCSLPadNum;                /* left side DBCS padding            */
  size_t   DBCSRPadNum;                /* right side DBCS padding           */
  size_t   SBCSRPadNum;                /* right side DBCS padding           */
  size_t   LeftPad;                    /* left padding                      */
  size_t   RightPad;                   /* right padding                     */
  size_t   Length;                     /* length of input string            */
  PUCHAR   String;                     /* pointer to string                 */
  PUCHAR   End;                        /* end string pointer                */
  RexxString *Retval;                  /* result copy pointer               */
  PUCHAR   CStr;                       /* string pointer                    */
  PUCHAR   SBCSPad;                    /* SBCSPad character                 */
  PUCHAR   DBCSPad;                    /* DBCSPad character                 */
  UCHAR    Option;                     /* SO/SI counting option             */
  size_t   ReqBytes;                   /* requested bytes                   */

  ValidDBCS(this);                     /* validate the string               */
                                       /* get the substring length          */
  ReqBytes = get_length(plength, ARG_ONE);
                                       /* get the padding character         */
  PadChar = ValidatePad(pad, (const PUCHAR)" ");
  PadSize = strlen((PCHAR)PadChar);    /* get the pad size too              */
                                       /* get the option character          */
  Option = option_character(option, DBCS_YES, ARG_THREE);
                                       /* must be a valid option            */
  if (Option != DBCS_YES && Option != DBCS_NO)
    report_exception2(Error_Incorrect_method_option, new_string("NY", 2), option);

  if (PadSize == SBCS_BYTELEN)         /* single byte pad?                  */
    SBCSPad = PadChar;                 /* set SBCS pad.                     */
  else {                               /* Padding char is DBCS,             */
    DBCSPad = PadChar;                 /* Set DBCS padd.                    */
    SBCSPad = (PUCHAR)" ";             /* Set Default pad as SBCS           */
  }
  Length = STRLEN(this);               /* get argument length               */
  String = STRPTR(this);               /* get data pointer                  */

  if (ReqBytes == Length)              /* if input length and               */
                                       /* requested are  the same           */
    Retval = this;                     /* then copy input                   */
  else if (!ReqBytes)                  /* asking for nothing?               */
    Retval = OREF_NULLSTRING;          /* return a null string              */
  else {
    if (ReqBytes > Length) {           /* otherwise                         */
                                       /* if requested larger               */
                                       /* get left pad count                */
      LeftPad = (ReqBytes - Length + 1) / 2;
                                       /* and right pad count               */
      RightPad = (ReqBytes - Length) - LeftPad;
      Retval = raw_string(ReqBytes);   /* allocate space                    */
      CStr = STRPTR(Retval);           /* get pointer                       */
      if (PadSize == SBCS_BYTELEN) {
        SBCSLPadNum = LeftPad;         /* Set SBCS pad length.              */
        DBCSLPadNum = 0;               /* And DBCS is zero.                 */
        SBCSRPadNum = RightPad;        /* Set SBCS pad length.              */
        DBCSRPadNum = 0;               /* And DBCS is zero.                 */
      }
      else {                           /* When Pad char is DBCS,            */
                                       /* calculate DBCS pad and            */
        DBCSLPadNum = LeftPad / DBCS_BYTELEN;
                                       /* SBCS padding                      */
        SBCSLPadNum = LeftPad % DBCS_BYTELEN;
                                       /* calculate DBCS pad and            */
        DBCSRPadNum = RightPad / DBCS_BYTELEN;
                                       /* SBCS padding                      */
        SBCSRPadNum = RightPad % DBCS_BYTELEN;
      }
      if (SBCSRPadNum) {               /* SBCS padding?                     */
        DBCS_SetPadChar(CStr, SBCSRPadNum, SBCSPad);
        CStr += SBCSRPadNum;           /* step the pointer                  */
      }
      if (DBCSRPadNum) {               /* DBCS padding?                     */
        DBCS_SetPadChar(CStr, DBCSRPadNum, DBCSPad);
        CStr += DBCSRPadNum * DBCS_BYTELEN;
      }
      if (Length) {                    /* length to copy?                   */
                                       /* copy the data                     */
        memcpy(CStr, String, Length);
        CStr += Length;                /* step pointer                      */
      }
      if (DBCSLPadNum) {               /* DBCS padding?                     */
        DBCS_SetPadChar(CStr, DBCSLPadNum, DBCSPad);
        CStr += DBCSLPadNum * DBCS_BYTELEN;
      }
      if (SBCSLPadNum) {               /* SBCS padding?                     */
        DBCS_SetPadChar(CStr, SBCSLPadNum, SBCSPad);
        CStr += SBCSLPadNum;           /* step the pointer                  */
      }
      Retval->generateHash();          /* done building the string          */
    }
    else {                             /* requested smaller than            */
                                       /* input                             */
      Retval = raw_string(ReqBytes);   /* allocate space                    */
      CStr = STRPTR(Retval);           /* copy the pointer                  */
      LeftPad = (Length - ReqBytes)/2; /* get left truncate count           */
                                       /* move requested bytes              */
      DBCS_IncByte(&String, &Length, &LeftPad);
      if (LeftPad) {                   /* split a DBCS char?                */
        *CStr++ = *SBCSPad;            /* copy a pad character              */
        String += DBCS_BYTELEN;        /* step over DBCS char               */
        ReqBytes--;                    /* reduce copy length by 1           */
      }
      LeftPad = ReqBytes;              /* copy remaining size               */
      End = String;                    /* copy position                     */
                                       /* move requested bytes              */
      DBCS_IncByte(&End, &Length, &LeftPad);
      if (LeftPad) {                   /* split backend DBCS?               */
                                       /* copy data                         */
        memcpy(CStr, String, (ReqBytes - 1));
                                       /* bump the pointer                  */
        *(CStr + ReqBytes - 1) = *SBCSPad;
      }
      else                             /* copy k bytes of data              */
        memcpy(CStr, String, ReqBytes);
      Retval->generateHash();          /* done building the string          */
    }
  }
  return Retval;                       /* return centered string            */
}

/*********************************************************************/
/* Description: Returns the remeinder from DBleft                    */
/*********************************************************************/

RexxString *RexxString::dbRleft(RexxInteger *plength,
                                RexxString *option)
{
  size_t   Length;                     /* length of input string            */
  PUCHAR   String;                     /* pointer to string                 */
  RexxString *Retval;                  /* result copy pointer               */
  UCHAR    Option;                     /* SO/SI counting option             */
  size_t   ReqBytes;                   /* requested bytes                   */

  ValidDBCS(this);                     /* validate the string               */
                                       /* get the substring length          */
  ReqBytes = get_length(plength, ARG_ONE);
                                       /* get the option character          */
  Option = option_character(option, DBCS_YES, ARG_TWO);
                                       /* must be a valid option            */
  if (Option != DBCS_YES && Option != DBCS_NO)
    report_exception2(Error_Incorrect_method_option, new_string("NY", 2), option);

  if (ReqBytes == 0)                   /* requested null?                   */
    Retval = this;                     /* just copy the string              */
  else {
    String = STRPTR(this);             /* copy start pointer                */
    Length = STRLEN(this);             /* get string length                 */
                                       /* move requested bytes              */
    DBCS_IncByte(&String, &Length, &ReqBytes);
                                       /* copy the remainder                */
    Retval = new_string((PCHAR)String, Length);
  }
  return Retval;                       /* return remainder string           */
}

/*********************************************************************/
/* Description: DBRight DBCS unique routine.                         */
/*********************************************************************/

RexxString *RexxString::dbRright(RexxInteger *plength,
                                 RexxString *option)
{
  size_t   Length;                     /* length of input string            */
  PUCHAR   String;                     /* pointer to string                 */
  RexxString *Retval;                  /* result copy pointer               */
  UCHAR    Option;                     /* SO/SI counting option             */
  size_t   ReqBytes;                   /* requested bytes                   */
  size_t   RemBytes;                   /* remainder bytes                   */

  ValidDBCS(this);                     /* validate the string               */
                                       /* get the substring length          */
  ReqBytes = get_length(plength, ARG_ONE);
                                       /* get the option character          */
  Option = option_character(option, DBCS_YES, ARG_TWO);
                                       /* must be a valid option            */
  if (Option != DBCS_YES && Option != DBCS_NO)
    report_exception2(Error_Incorrect_method_option, new_string("NY", 2), option);

  if (ReqBytes == 0)                   /* requested null?                   */
    Retval = this;                     /* just copy the string              */
  else {
    String = STRPTR(this);             /* copy start pointer                */
    Length = STRLEN(this);             /* get string length                 */

    if (ReqBytes > Length)             /* longer than string?               */
      Retval = OREF_NULLSTRING;        /* zero length string                */
    else {
      RemBytes = Length - ReqBytes;    /* target move amount                */
                                       /* move requested bytes              */
      DBCS_IncByte(&String, &Length, &RemBytes);
      if (RemBytes) {                  /* split character?                  */
        RemBytes = 1;                  /* need one pad char                 */
        ReqBytes--;                    /* decrement length                  */
        String += DBCS_BYTELEN;        /* step over DBCS character          */
      }
                                       /* extract string part               */
      Retval = new_string((PCHAR)STRPTR(this), (String - STRPTR(this)));
    }
  }
  return Retval;                       /* return extracted string           */
}

/*********************************************************************/
/* Description: DBToDBCS DBCS unique routine.                        */
/*********************************************************************/

RexxString *RexxString::dbToDbcs()
{
  PUCHAR   CStr;                       /* output string                     */
  PUCHAR   IStr;                       /* input string                      */
  PUCHAR   EndStr;                     /* end of input string               */
  size_t   InLen;                      /* input length                      */
  RexxString *Retval;                  /* function return value             */

  ValidDBCS(this);                     /* validate the string               */
  InLen = STRLEN(this);                /* get input length                  */
  if (!InLen)                          /* If input string length is         */
    Retval = OREF_NULLSTRING;          /* just return a null string         */
  else {
    Retval = raw_string(InLen * 2);    /* get largest needed string         */
    IStr = STRPTR(this);               /* point to input string             */
    EndStr = IStr + InLen;             /* set input end point               */
    CStr = STRPTR(Retval);             /* set output location               */

    while (IStr < EndStr) {            /* Do until end of string            */
      if (IsDBCS(*IStr)) {             /* DBCS character?                   */
                                       /* copy directly                     */
        memcpy(CStr, IStr, DBCS_BYTELEN);
        CStr += DBCS_BYTELEN;          /* bump output                       */
        IStr += DBCS_BYTELEN;          /* bump and input                    */
      }
      else {                           /* might need to convert             */
                                       /* convert this                      */
        DBCS_ConvToDBCS(IStr, &CStr);
        IStr++;                        /* bump the input one                */
      }
    }
                                       /* resize the string                 */
    Retval = new_string((PCHAR)STRPTR(Retval), CStr - STRPTR(Retval));
  }
  return Retval;                       /* return converted string           */
}

/*********************************************************************/
/* Description: DBToSBCS DBCS unique routine.                        */
/*********************************************************************/

RexxString *RexxString::dbToSbcs()
{
  PUCHAR   CStr;                       /* output string                     */
  PUCHAR   IStr;                       /* input string                      */
  PUCHAR   EndStr;                     /* end of input string               */
  size_t   InLen;                      /* input length                      */
  RexxString *Retval;                  /* function return value             */

  ValidDBCS(this);                     /* validate the string               */
  InLen = STRLEN(this);                /* get input length                  */
  if (!InLen)                          /* If input string length is         */
    Retval = OREF_NULLSTRING;          /* just return a null string         */
  else {
    Retval = raw_string(InLen);        /* get largest needed string         */
    IStr = STRPTR(this);               /* point to input string             */
    EndStr = IStr + InLen;             /* set input end point               */
    CStr = STRPTR(Retval);             /* set output location               */

    while (IStr < EndStr) {            /* Do until end of string            */
      if (IsDBCS(*IStr)) {             /* DBCS character?                   */
        DBCS_ConvToSBCS(IStr, &CStr);
        IStr += DBCS_BYTELEN;          /* bump input                        */
      }
      else                             /* might need to convert             */
        *CStr++ = *IStr++;             /* copy the single character         */
    }
                                       /* shorten the string                */
    Retval = new_string((PCHAR)STRPTR(Retval), CStr - STRPTR(Retval));
  }
  return Retval;                       /* return converted string           */
}

/*********************************************************************/
/* Description: Validate Mixed string                                */
/*********************************************************************/

RexxInteger *RexxString::dbValidate(RexxString *option)
{
  RexxInteger *Retval;                 /* return value                      */
  UCHAR    Option;                     /* SO/SI counting option             */

                                       /* get the option character          */
  Option = option_character(option, DBCS_COUNT, ARG_ONE);
  if (Option != DBCS_COUNT)            /* must be a valid option            */
    report_exception2(Error_Incorrect_method_option, new_string("C", 1), option);
  if (DBCS_Type(this) != INV_MIXED)    /* valid string?                     */
    Retval = TheTrueObject;            /* return true                       */
  else
    Retval = TheFalseObject;           /* return false                      */
  return Retval;                       /* Normal Return                     */
}

/*********************************************************************/
/* Description: Length of string.                                    */
/*********************************************************************/

RexxInteger *RexxString::dbWidth(RexxString *option)
{
  UCHAR Option;                        /* function option                   */
  size_t slength;

  ValidDBCS(this);                     /* validate the string               */
                                       /* get the option character          */
  Option = option_character(option, DBCS_YES, ARG_ONE);
                                       /* must be a valid option            */
  if (Option != DBCS_YES && Option != DBCS_NO)
    report_exception2(Error_Incorrect_method_option, new_string("NY", 2), option);
  slength = STRLEN(this);              /* return string size                */
  return new_integer(slength);         /* as an integer object              */
}

/*********************************************************************/
/* Description: Adjusts contiguous SI-SO and SO-SI                   */
/*********************************************************************/

RexxString *RexxString::dbAdjust(RexxString *option)
{
  UCHAR    Option;                     /* function option character         */

  ValidDBCS(this);                     /* validate the string               */
                                       /* get the option character          */
  Option = option_character(option, DBCS_REMOVE,  ARG_ONE);
                                       /* must be a valid option            */
  if (Option != DBCS_BLANK && Option != DBCS_REMOVE)
    report_exception2(Error_Incorrect_method_option, new_string("NY", 2), option);
  return this;                         /* return the string unchanged       */
}

/*********************************************************************/
/* Description: Changes DBCS string to bracketed DBCS string         */
/*********************************************************************/

RexxString *RexxString::dbBracket()
{
  return this;                         /* just return the string            */
}

/*********************************************************************/
/* Description: Changes bracket DBCS string to DBCS string           */
/*********************************************************************/

RexxString *RexxString::dbUnBracket()
{
  return this;                         /* just return the string            */
}
