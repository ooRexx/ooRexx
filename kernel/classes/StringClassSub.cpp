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
/* REXX Kernel                                                  okbsubs.c     */
/*                                                                            */
/* substring oriented REXX string methods                                     */
/*                                                                            */
/******************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ASCIIDBCSStrings.hpp"
#include "RexxBuiltinFunctions.h"                          /* Gneral purpose BIF Header file       */

extern INT  lookup[];
extern RexxActivity *CurrentActivity;  /* expose current activity object    */
                                       /* current global settings           */
extern ACTIVATION_SETTINGS *current_settings;

/* the CENTER function (and the CENTRE function) */
/******************************************************************************/
/* Arguments:  String len, string pad character                               */
/*                                                                            */
/*  Returned:  string                                                         */
/******************************************************************************/
RexxString *RexxString::center(RexxInteger *length,
                               RexxString  *pad)
{
  CHAR     PadChar;                    /* pad character                     */
  size_t   LeftPad;                    /* required left pads                */
  size_t   RightPad;                   /* required right pads               */
  size_t   Space;                      /* result string size                */
  size_t   Width;                      /* centering width                   */
  size_t   Len;                        /* string length                     */
  RexxString *Retval;                  /* return string                     */

  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return (RexxString *)this->DBCScenter(length, pad);
                                       /* see how long result should be     */
  Width = get_length(length, ARG_ONE);

                                       /* Get pad character (optional) blank*/
                                       /*  is used if omitted.              */
  PadChar = get_pad(pad, ' ', ARG_TWO);
  Len = this->length;                  /* get length of input to center     */
  if (Width == Len)                    /* if input length and               */
                                       /* requested are  the same           */
    Retval = this;                     /* then copy input                   */

  else if (!Width)                     /* centered in zero?                 */
    Retval = OREF_NULLSTRING;          /* return a null string              */
  else {
    if (Width > Len) {                 /* otherwise                         */
                                       /* if requested larger               */
      LeftPad = (Width - Len) / 2;     /* get left pad count                */
      RightPad = (Width - Len)-LeftPad;/* and right pad count               */
      Space = RightPad + LeftPad + Len;/* total space required              */
                                       /* allocate space                    */
      Retval = (RexxString *)raw_string(Space);
                                       /* set left pad characters           */
      memset(Retval->stringData, PadChar, LeftPad);
      if (Len)                         /* something to copy?                */
                                       /* copy the string                   */
        memcpy(Retval->stringData + LeftPad,
               (PCHAR)this->stringData,
               Len);
                                       /* now the trailing pad chars        */
      memset(Retval->stringData + LeftPad + Len,
             PadChar,
             RightPad);
                                       /* done building the string          */
      Retval->generateHash();
    }
    else {                             /* requested smaller than            */
                                       /* input                             */
      LeftPad = (Len - Width) / 2;     /* get left truncate count           */
                                       /* copy the data                     */
      Retval = (RexxString *)new_string((PCHAR)this->stringData + LeftPad, Width);
    }
  }
  return Retval;                       /* done, return output buffer        */
}

/* the DELSTR function */
/******************************************************************************/
/* Arguments:  Starting position of string to be deleted                      */
/*             length of string to be deleted                                 */
/*  Returned:  string                                                         */
/******************************************************************************/
RexxString *RexxString::delstr(RexxInteger *position,
                               RexxInteger *length)
{
  RexxString *Retval;                  /* return value:                     */
  size_t   BackLen;                    /* end string section                */
  size_t   StringLen;                  /* original string length            */
  size_t   DeleteLen;                  /* deleted length                    */
  size_t   DeletePos;                  /* delete position                   */
  PCHAR    Current;                    /* current copy position             */

  if (DBCS_SELF)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return this->DBCSdelstr(position, length);

  StringLen = this->length;            /* get string length                 */
                                       /* get start string position         */
  DeletePos = get_position(position, ARG_ONE);
                                       /* get the length to delete          */
  DeleteLen = optional_length(length, StringLen - DeletePos + 1, ARG_TWO);

  if (DeletePos > StringLen)           /* beyond string bounds?             */
    Retval = this;                     /* return string unchanged           */

  else {                               /* need to actually delete           */
    DeletePos--;                       /* make position origin zero         */
                                       /* deleting more than string?        */
    if (DeleteLen >= (StringLen - DeletePos))
      BackLen = 0;                     /* no back part                      */
    else                               /* find length to delete             */
      BackLen = StringLen - (DeletePos + DeleteLen);
                                       /* allocate result string            */
    Retval = (RexxString *)raw_string(DeletePos + BackLen);
                                       /* point to string part              */
    Current = Retval->stringData;
    if (DeletePos) {                   /* have a front part?                */
                                       /* copy it                           */
      memcpy(Current, (PCHAR)this->stringData, DeletePos);
      Current += DeletePos;            /* step past the front               */
    }

    if (BackLen) {                     /* have a trailing part              */
                                       /* copy that over                    */
      memcpy(Current,
             (PCHAR)this->stringData + DeletePos + DeleteLen,
             BackLen);
    }
                                       /* done building the string          */
    Retval->generateHash();
  }
  return Retval;                       /* return the new string             */
}

/* the INSERT function */
/******************************************************************************/
/* Arguments:  string to be inserted                                          */
/*             position in self to place new string                           */
/*             length of new string to insert, padded if necessary            */
/*             pad character to use.                                          */
/*  Returned:  string                                                         */
/******************************************************************************/
RexxString *RexxString::insert(RexxString  *newStrObj,
                               RexxInteger *position,
                               RexxInteger *length,
                               RexxString  *pad)
{
  RexxString *Retval;                  /* return string                     */
  RexxString *newStr;                  /* return string                     */
  CHAR     PadChar;                    /* HugeString for Padding char       */
  size_t   ReqLenChar;                 /* Actual req char len of new.       */
  size_t   ReqPadChar;                 /* Actual req char len of new.       */
  size_t   ReqLeadPad;                 /* Actual req char len of new.       */
  size_t   TargetSize;                 /* byte size of target string        */
  size_t   NCharLen;                   /* Char len of new HugeString.       */
  size_t   TCharLen;                   /* Char len of target HugeStr.       */
  size_t   FCharLen;                   /* Char len of front portion.        */
  size_t   BCharLen;                   /* Char len of back portion.         */
  size_t   BuffSiz;                    /* Estimated result area size.       */
  size_t   NChar;                      /* Character position.               */
  PCHAR    Current;                    /* current copy location             */

  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return this->DBCSinsert(newStrObj, position, length, pad);

  TCharLen = this->length;             /* get the target string length      */
                                       /* get the needle string (and length)*/
  newStr = (RexxString *)get_string(newStrObj, ARG_ONE);
  NCharLen = newStr->length;
                                       /* use optional_length for starting  */
                                       /* position becase a value of 0 IS   */
                                       /* valid for INSERT                  */
  NChar = optional_length(position, 0, ARG_TWO);
                                       /* get the optional length, using the*/
                                       /* needle length as the defaul       */
  ReqLenChar = optional_length(length, NCharLen, ARG_THREE);

                                       /*  is used if omitted.              */
  PadChar = get_pad(pad, ' ', ARG_FOUR);
  ReqLeadPad = 0;                      /* set lead pad to zero              */
  TargetSize = TCharLen;               /* copy the target size              */

  if (NChar == 0) {                    /* inserting at the front?           */
    ReqLeadPad = 0;                    /* no leading pads                   */
    FCharLen = 0;                      /* no front part                     */
    BCharLen = TCharLen;               /* trailer is entire target          */
  }
  else if (NChar >= TCharLen) {        /* need leading pads?                */
    ReqLeadPad = (NChar - TCharLen);   /* calculate needed                  */
    FCharLen = TCharLen;               /* front part is all                 */
    BCharLen = 0;                      /* trailer is nothing                */
  }
  else {                               /* have a split                      */
    ReqLeadPad = 0;                    /* no leading pad                    */
    FCharLen = NChar;                  /* NChar front chars                 */
    BCharLen = TCharLen - NChar;       /* and some trailer too              */
  }
  NCharLen = min(NCharLen, ReqLenChar);/* truncate new, if needed           */
  ReqPadChar = ReqLenChar - NCharLen;  /* calculate pad chars               */
                                       /* calculate result size             */
  BuffSiz = NCharLen + TargetSize + ReqPadChar + ReqLeadPad;
  Retval = raw_string(BuffSiz);        /* allocate the result               */
  Current = Retval->stringData;        /* point to start                    */

  if (FCharLen) {                      /* have leading chars                */
                                       /* copy the leading part             */
    memcpy(Current, (PCHAR)this->stringData, FCharLen);
    Current += FCharLen;               /* step copy location                */
  }
  if (ReqLeadPad) {                    /* if required leading pads          */
                                       /* add the pads now                  */
    memset(Current, PadChar, ReqLeadPad);
    Current += ReqLeadPad;             /* step the output pointer           */
  }

  if (NCharLen) {                      /* new string to copy?               */
                                       /* copy the inserted part            */
    memcpy(Current, newStr->stringData, NCharLen);
    Current += NCharLen;               /* step copy location                */
  }

  if (ReqPadChar) {                    /* if required trailing pads         */
                                       /* add the pads now                  */
    memset(Current, PadChar, ReqPadChar);
    Current += ReqPadChar;             /* step the output pointer           */
  }

  if (BCharLen) {                      /* have trailing chars               */
                                       /* copy the leading part             */
    memcpy(Current, (PCHAR)this->stringData + FCharLen, BCharLen);
  }
  Retval->generateHash();              /* done building the string          */
  return Retval;                       /* Return the new string             */
}

/* the LEFT function */
/******************************************************************************/
/* Arguments:  String len, string pad character                               */
/*                                                                            */
/*  Returned:  string                                                         */
/******************************************************************************/
RexxString *RexxString::left(RexxInteger *length,
                             RexxString  *pad)
{
  CHAR      PadChar;                   /* pad character                     */
  size_t    Size;                      /* requested size                    */
  size_t    Length;                    /* string length                     */
  RexxString *Retval;                  /* returned result                   */
  PCHAR     Current;                   /* current copy location             */
  size_t    CopyLength;                /* length to copy                    */

  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return this->DBCSleft(length, pad);

                                       /* get the target length             */
  Size = get_length(length, ARG_ONE);

                                       /*  is used if omitted.              */
  PadChar = get_pad(pad, ' ', ARG_TWO);
  Length = this->length;               /* get input length                  */

  if (!Size)                           /* requesting zero bytes?            */
    Retval = OREF_NULLSTRING;          /* return a null string              */
  else {
    Retval = raw_string(Size);         /* allocate a result string          */
    CopyLength = min(Length, Size);    /* adjust the length                 */
                                       /* point to data part                */
    Current = Retval->stringData;
    if (CopyLength) {                  /* have real data?                   */
                                       /* copy it                           */
      memcpy(Current, (PCHAR)this->stringData, CopyLength);
      Current += CopyLength;           /* bump the pointer                  */
    }
    if (Size > Length)                 /* need to pad?                      */
                                       /* pad the string                    */
      memset(Current, PadChar, Size - Length);
                                       /* done building the string          */
    Retval->generateHash();
  }
  return Retval;                       /* return string piece               */
}

/******************************************************************************/
/* Function:  Process the OVERLAY function/method                             */
/******************************************************************************/
RexxString *RexxString::overlay(
    RexxString  *newStrObj,            /* overlayed string                  */
    RexxInteger *position,             /* overlay position                  */
    RexxInteger *length,               /* overlay length                    */
    RexxString  *pad)                  /* pad character to use.             */
{
  RexxString *Retval;                  /* return string                     */
  RexxString *newStr;                  /* return string                     */
  size_t   OverlayPos;                 /* overlay position                  */
  size_t   OverlayLen;                 /* overlay length                    */
  size_t   NewLen;                     /* length of overlay string          */
  size_t   TargetLen;                  /* target length                     */
  size_t   FrontLen;                   /* front length                      */
  size_t   BackLen;                    /* back length                       */
  size_t   FrontPad;                   /* front pad length                  */
  size_t   BackPad;                    /* back pad length                   */
  CHAR     PadChar;                    /* pad character                     */
  PCHAR    Current;                    /* current copy location             */

  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return this->DBCSoverlay(newStrObj, position, length, pad);

  TargetLen = this->length;            /* get the haystack length           */
                                       /* get the overlay string value      */
  newStr = (RexxString *)get_string(newStrObj, ARG_ONE);
  NewLen = newStr->length;
                                       /* get the overlay position          */
  OverlayPos = optional_position(position, 1, ARG_TWO);
                                       /* get final overlay length          */
  OverlayLen = optional_length(length, NewLen, ARG_THREE);
                                       /*  is used if omitted.              */
  PadChar = get_pad(pad, ' ', ARG_FOUR);

  if (OverlayLen > NewLen)             /* need to pad?                      */
    BackPad = OverlayLen - NewLen;     /* get the pad size                  */
  else {                               /* need to truncate                  */
    NewLen = OverlayLen;               /* used specified length             */
    BackPad = 0;                       /* no back padding                   */
  }

  if (OverlayPos > TargetLen) {        /* overlaying past the end?          */
                                       /* get front padding                 */
    FrontPad = OverlayPos - TargetLen - 1;
    FrontLen = TargetLen;              /* copy entire string                */
  }
  else {                               /* overlay is within bounds          */
    FrontPad = 0;                      /* no padding here                   */
    FrontLen = OverlayPos - 1;         /* just copy the front part          */
  }
                                       /* fall off the back side?           */
  if (OverlayPos + OverlayLen > TargetLen)
    BackLen = 0;                       /* no back part                      */
  else
                                       /* calculate the back part           */
    BackLen = TargetLen - (OverlayPos + OverlayLen - 1);
                                       /* allocate result string            */
  Retval = raw_string(FrontLen + BackLen + FrontPad + OverlayLen);

  Current = Retval->stringData;        /* get copy location                 */

  if (FrontLen) {                      /* something in front?               */
                                       /* copy the front part               */
    memcpy(Current, (PCHAR)this->stringData, FrontLen);
    Current += FrontLen;               /* step the pointer                  */
  }

  if (FrontPad) {                      /* padded in front?                  */
    memset(Current, PadChar, FrontPad);/* set the pad characters            */
    Current += FrontPad;               /* step the pointer                  */
  }

  if (NewLen) {                        /* non-null new string?              */
                                       /* copy the string                   */
    memcpy(Current, newStr->stringData, NewLen);
    Current += NewLen;                 /* step the pointer                  */
  }

  if (BackPad) {                       /* padded in back?                   */
                                       /* set the pad characters            */
    memset(Current, PadChar, BackPad);
    Current += BackPad;                /* step the pointer                  */
  }

  if (BackLen) {                       /* trailing part?                    */
                                       /* copy the string                   */
    memcpy(Current, (PCHAR)this->stringData + OverlayPos + OverlayLen - 1, BackLen);
  }
  Retval->generateHash();              /* done building the string          */
  return Retval;                       /* return new string                 */
}

/* the REVERSE function */
/******************************************************************************/
/* Arguments:  none                                                           */
/*                                                                            */
/*  Returned:  string reversed.                                               */
/******************************************************************************/
RexxString *RexxString::reverse()
{
  RexxString *Retval;                  /* temp pointer for reversal       */
  size_t     Length;                   /* string length                   */
  PCHAR      String;                   /* current location                */
  PCHAR      End;                      /* string end position             */

  if (DBCS_SELF)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return this->DBCSreverse();

  Length = this->length;               /* get first argument              */
  if (Length) {                        /* if really data                  */
    Retval = raw_string(Length);       /* get result storage              */
                                       /* get new string pointer          */
    String = Retval->stringData;
                                       /* point to end of original        */
    End = (PCHAR)this->stringData + Length - 1;

    while (Length--)                   /* reverse entire string           */
      *String++ = *End--;              /* copy a single char              */
                                       /* done building the string          */
    Retval->generateHash();
  }
  else                                 /* if null input                     */
    Retval = OREF_NULLSTRING;          /* return null output                */
  return Retval;                       /* return the reversed string        */
}

/* the RIGHT function */
/******************************************************************************/
/* Arguments:  length of result                                               */
/*             pad character to use if needed.                                */
/*                                                                            */
/*  Returned:  string right justified.                                        */
/******************************************************************************/
RexxString *RexxString::right(RexxInteger *length,
                              RexxString  *pad)
{
  CHAR      PadChar;                   /* pad character                     */
  size_t    Size;                      /* requested size                    */
  size_t    Length;                    /* string length                     */
  RexxString *Retval;                  /* returned result                   */
  PCHAR     Current;                   /* current copy location             */
  size_t    CopyLength;                /* length to copy                    */

  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return this->DBCSright(length, pad);

                                       /* get the target length             */
  Size = get_length(length, ARG_ONE);

                                       /*  is used if omitted.              */
  PadChar = get_pad(pad, ' ', ARG_TWO);
  Length = this->length;               /* get input length                  */

  if (!Size)                           /* requesting zero bytes?            */
                                       /* return a null string              */
    Retval = OREF_NULLSTRING;
  else {
    Retval = raw_string(Size);         /* allocate a result string          */
    CopyLength = min(Length, Size);    /* adjust the length                 */
                                       /* point to data part                */
    Current = Retval->stringData;
    if (Size > Length) {               /* need to pad?                      */
                                       /* pad the string                    */
      memset(Current, PadChar, Size - Length);
      Current += Size - Length;        /* bump the pointer                  */
    }
    if (CopyLength)                    /* have real data?                   */
                                       /* copy it                           */
      memcpy(Current,
             (PCHAR)this->stringData + Length - CopyLength,
             CopyLength);
                                       /* done building the string          */
    Retval->generateHash();
  }
  return Retval;                       /* return string piece               */
}

/* the STRIP function */
/******************************************************************************/
/* Arguments:  option, where to strip the characters                          */
/*             the character to strip.                                        */
/*                                                                            */
/*  Returned:  string                                                         */
/******************************************************************************/
RexxString *RexxString::strip(RexxString *option,
                              RexxString *stripchar)
{
    PCHAR       Back;                    /* pointer to back part              */
    PCHAR       Front;                   /* pointer to front part             */
    size_t      Length;                  /* length of the string              */
    CHAR        RemoveChar;              /* character to remove               */
    CHAR        Option;                  /* strip option                      */
    RexxString *Retval;                  /* return value                      */

    if (DBCS_MODE)                       /* need to use DBCS?                 */
                                         /* do the DBCS version               */
        return this->DBCSstrip(option, stripchar);

    /* get the option character          */
    Option = option_character(option, STRIP_BOTH, ARG_ONE);
    if (Option != STRIP_TRAILING &&      /* must be a valid option            */
        Option != STRIP_LEADING &&
        Option != STRIP_BOTH )
        report_exception2(Error_Incorrect_method_option,
                          new_string("BLT", 3),
                          option);
    // get the strip character.  This is a phony default, as the
    // real default strips the entire set of recognized whitespace characters.
    RemoveChar = (UCHAR) get_pad(stripchar, ' ', ARG_TWO);
    // and get a special processing flag
    bool stripWhite = stripchar == OREF_NULL;

    Front = (PCHAR)this->stringData;     /* point to string start             */
    Length = this->length;               /* get the length                    */

                                         /* need to strip leading?            */
    if (Option == STRIP_LEADING || Option == STRIP_BOTH)
    {
        // stripping all white space?  need multiple checks
        if (stripWhite)
        {
            while (Length > 0)
            {
                // stop of not a blank or a tab
                if (*Front != ch_BLANK && *Front != ch_TAB)
                {
                    break;
                }
                Front++;                         /* step the pointer                  */
                Length--;                        /* reduce the length                 */
            }

        }
        else
        {
            while (Length > 0)
            {                   /* while more string                 */
                if (*Front != RemoveChar)        /* done stripping?                   */
                    break;                         /* quit                              */
                Front++;                         /* step the pointer                  */
                Length--;                        /* reduce the length                 */
            }
        }
    }

    /* need to strip trailing?           */
    if (Option == STRIP_TRAILING || Option == STRIP_BOTH)
    {
        Back = Front + Length - 1;         /* point to the end                  */
        if (stripWhite)
        {
            while (Length > 0)
            {
                if (*Back != ch_BLANK && *Back != ch_TAB)
                {
                    break;
                }
                Back--;                          /* step the pointer back             */
                Length--;                        /* reduce the length                 */
            }

        }
        else
        {
            while (Length > 0)
            {                   /* while more string                 */
                if (*Back != RemoveChar)         /* done stripping?                   */
                    break;                         /* quit                              */
                Back--;                          /* step the pointer back             */
                Length--;                        /* reduce the length                 */
            }
        }
    }

    if (Length > 0)                      /* have anything left?               */
        Retval = new_string(Front, Length);/* extract remaining piece           */
    else
        Retval = OREF_NULLSTRING;          /* nothing left, us a null           */
    return Retval;                       /* return stripped string            */
}

/* the SUBSTR function */
/******************************************************************************/
/* Arguments:  String position for substr                                     */
/*             requested length of new string                                 */
/*             pad character to use, if necessary                             */
/*                                                                            */
/*  Returned:  string, sub string of original.                                */
/******************************************************************************/
RexxString *RexxString::substr(RexxInteger *position,
                               RexxInteger *length,
                               RexxString  *pad)
{
  UCHAR    PadChar;                    /* pad character                     */
  size_t   Position;                   /* start position                    */
  size_t   Length;                     /* required length                   */
  size_t   StringLength;               /* input string length               */
  size_t   SubstrLength;               /* length plucked from string        */
  size_t   PadCount;                   /* number of pad characters          */
  RexxString *Retval;                  /* returned value                    */

  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return this->DBCSsubstr(position, length, pad);

                                       /* get starting position             */
  Position = get_position(position, ARG_ONE) - 1;
  StringLength = this->length;         /* get the string length             */
  if (StringLength == 0)               /* string of zerolength              */
    Length = 0L;                       /* no partial to handle              */
  else if (StringLength >= Position)   /* pos within the string?            */
    Length = StringLength - Position;  /* length is remainder               */
  else
    Length = 0L;                       /* string is used up                 */
                                       /* go get length or use default if   */
                                       /* not supplied.                     */
  Length = optional_length(length, Length, ARG_TWO);
                                       /* go get optional pad character,    */
                                       /*  is used if omitted.              */
  PadChar = get_pad(pad, ' ', ARG_THREE);

  if (Length == 0)                     /* nothing to return?                */
    Retval = OREF_NULLSTRING;          /* return a null string              */
  else {
    if (Position > StringLength) {     /* start past the string?            */
      SubstrLength = 0;                /* nothing to copy                   */
      PadCount = Length;               /* just pad characters               */
    }
    else {                             /* have something to copy            */
                                       /* get string size                   */
      SubstrLength = min(Length, StringLength - Position);
                                       /* and number of pad chars           */
      PadCount = Length - SubstrLength;
    }
    Retval = raw_string(Length);       /* get result string                 */
    if (SubstrLength)                  /* data to copy?                     */
                                       /* yes, copy over                    */
      memcpy(Retval->stringData,
             (PCHAR)this->stringData + Position,
             SubstrLength);
    if (PadCount)                      /* padding needed?                   */
                                       /* add the pad characters            */
      memset(Retval->stringData + SubstrLength,
             PadChar,
             PadCount);
                                       /* done building the string          */
    Retval->generateHash();
  }
  return Retval;                       /* return extracted string           */
}


/**
 * Extract a single character from a string object.
 * Returns a null string if the specified position is
 * beyond the bounds of the string.
 *
 * @param positionArg
 *               The position of the target  character.  Must be a positive
 *               whole number.
 *
 * @return Returns the single character at the target position.
 *         Returns a null string if the position is beyond the end
 *         of the string.
 */
RexxString *RexxString::subchar(RexxInteger *positionArg)
{
    // the starting position isn't optional
    size_t position = get_position(positionArg, ARG_ONE) - 1;
    size_t stringLength = this->length;

    // beyond the bounds, this is a null string
    if (position >= stringLength)
    {
        return OREF_NULLSTRING;
    }
    // return the single character
    return this->extract(position, 1);
}

